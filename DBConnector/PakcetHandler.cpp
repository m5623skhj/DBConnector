#include "PreCompile.h"
#include "DBServer.h"
#include "DBServerProtocol.h"
#include "LanServerSerializeBuf.h"
#include "StoredProcedure.h"
#include "ODBCConnector.h"
#include "../../RIOServerTest/RIO_Test/Protocol.h"
#include "../../RIOServerTest/RIO_Test/EnumType.h"

using namespace std;

FORCEINLINE bool IsSuccess(const ProcedureResult& result)
{
	return result.first;
}

void DBServer::InsertBatchJob(UINT64 userSessionId, unsigned char batchSize)
{
	std::lock_guard lock(batchedDBJobMapLock);
	batchedDBJobMap.insert({ userSessionId, make_shared<BatchedDBJob>(batchSize)});
}

void DBServer::HandlePacket(UINT64 requestSessionId, UINT packetId, CSerializationBuf* recvBuffer)
{
	UINT64 userSessionId = 0;

	*recvBuffer >> userSessionId;

	if (packetId == DBServerProtocol::PACKET_ID::BATCHED_DB_JOB)
	{
		unsigned char batchSize = 0;
		*recvBuffer >> batchSize;

		InsertBatchJob(userSessionId, batchSize);
	}
	else
	{
		if (IsBatchJobWaitingUser(userSessionId) == true)
		{
			AddItemForJobStart(requestSessionId, userSessionId, packetId, recvBuffer);
		}
		else
		{
			auto result = HandleImpl(requestSessionId, userSessionId, packetId, recvBuffer);
			if (IsSuccess(result) == true)
			{
				SendPacket(requestSessionId, result.second);
			}
			else
			{
				CSerializationBuf::Free(result.second);
			}
		}
	}
}

void DBServer::AddItemForJobStart(UINT64 requestSessionId, UINT64 userSessionId, UINT packetId, CSerializationBuf* recvBuffer)
{
	std::shared_ptr<BatchedDBJob> batchedJob = nullptr;
	{
		std::lock_guard lock(batchedDBJobMapLock);
		const auto& iter = batchedDBJobMap.find(userSessionId);
		if (iter == batchedDBJobMap.end())
		{
			return;
		}

		batchedJob = iter->second;
	}

	if (batchedJob == nullptr)
	{
		return;
	}

	CSerializationBuf::AddRefCount(recvBuffer);
	batchedJob->bufferList.push_back(std::make_pair(packetId, recvBuffer));
	if (batchedJob->batchSize == batchedJob->bufferList.size())
	{
		DoBatchedJob(requestSessionId, userSessionId, batchedJob);
	}
}

void DBServer::DoBatchedJob(UINT64 requestSessionId, UINT64 userSessionId, std::shared_ptr<BatchedDBJob> batchedJob)
{
	std::list<CSerializationBuf*> resultList;
	bool isError = false;
	for (auto& job : batchedJob->bufferList)
	{
		auto result = HandleImpl(requestSessionId, userSessionId, job.first, job.second);
		CSerializationBuf::Free(job.second);

		if (IsSuccess(result) == true)
		{
			resultList.push_back(result.second);
		}
		else
		{
			isError = true;
			break;
		}
	}

	if (isError == false)
	{
		for (auto& result : resultList)
		{
			SendPacket(requestSessionId, result);
		}
	}
	else
	{
		auto reverseIter = resultList.rbegin();
		if (reverseIter == resultList.rend())
		{
			// First job was failed
			// To do what?
		}

		++reverseIter;
		for (; reverseIter != resultList.rend(); ++reverseIter)
		{
			// TODO
			// Rollback committed procedure
		}
	}
}

bool DBServer::IsBatchJobWaitingUser(UINT64 userSessionId)
{
	std::lock_guard lock(batchedDBJobMapLock);
	const auto& iter = batchedDBJobMap.find(userSessionId);
	if (iter == batchedDBJobMap.end())
	{
		return false;
	}

	return true;
}

ProcedureResult DBServer::HandleImpl(UINT64 requestSessionId, UINT64 userSessionId, UINT packetId, CSerializationBuf* recvBuffer)
{
	CSerializationBuf* packet = CSerializationBuf::Alloc();
	bool isSuccess = false;
	ODBCConnector& connector = ODBCConnector::GetInst();
	auto conn = connector.GetConnection();

	switch (packetId)
	{
	case DBServerProtocol::PACKET_ID::TEST:
	{
		auto procedure = connector.GetProcedureInfo("test");
		if (procedure == nullptr)
		{
			break;
		}

		test t;
		*recvBuffer >> t.id3;
		recvBuffer->ReadBuffer((char*)t.teststring.GetCString(), sizeof(t.teststring));

		if (connector.CallSPDirectWithSPObject(conn.value().stmtHandle, procedure, t) == false)
		{
			break;
		}

		UINT sendPackeId = static_cast<UINT>(PACKET_ID::CALL_TEST_PROCEDURE_PACKET_REPLY);
		*packet << sendPackeId << userSessionId;
		isSuccess = true;
		break;
	}
	case DBServerProtocol::PACKET_ID::SELECT_TEST_2:
	{
		auto procedure = connector.GetProcedureInfo("SELECT_TEST_2");
		if (procedure == nullptr)
		{
			break;
		}

		SELECT_TEST_2 s;
		*recvBuffer >> s.id;

		if (connector.CallSPDirectWithSPObject(conn.value().stmtHandle, procedure, s) == false)
		{
			break;
		}

		auto results = connector.GetSPResult<SELECT_TEST_2::ResultType>(conn.value().stmtHandle);
		if (results == nullopt)
		{
			break;
		}

		UINT sendPackeId = static_cast<UINT>(PACKET_ID::CALL_SELECT_TEST_2_PROCEDURE_PACKET_REPLY);
		*packet << sendPackeId << userSessionId;

		for (const auto& result : results.value())
		{
			*packet << result.no;
			packet->WriteBuffer((char*)(result.tablename.GetCString()), sizeof(result.tablename));
		}
		isSuccess = true;
		break;
	}

	default:
		cout << "Invalid packet id : " << packetId << endl;
		break;
	}

	connector.FreeConnection(conn.value());
	return ProcedureResult(isSuccess, packet);
}