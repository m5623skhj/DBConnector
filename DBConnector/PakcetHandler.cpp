#include "PreCompile.h"
#include "DBServer.h"
#include "LanServerSerializeBuf.h"
#include "StoredProcedure.h"
#include "ODBCConnector.h"
#include "../../RIOServerTest/RIO_Test/Protocol.h"

using namespace std;

FORCEINLINE bool IsSuccess(const ProcedureResult& result)
{
	return result.first;
}

void DBServer::InsertBatchJob(DBJobKey jobKey, const DBJobStart& job)
{
	std::lock_guard lock(batchedDBJobMapLock);
	batchedDBJobMap.insert({ jobKey, make_shared<BatchedDBJob>(job.batchSize, job.sessionId) });
}

void DBServer::HandlePacket(UINT64 requestSessionId, UINT packetId, CSerializationBuf* recvBuffer)
{
	DBJobKey key = INVALID_DB_JOB_KEY;
	*recvBuffer >> key;

	if (packetId == static_cast<UINT>(PACKET_ID::BATCHED_DB_JOB))
	{
		DBJobStart job;
		*recvBuffer >> job.sessionId >> job.batchSize;
		InsertBatchJob(key, job);
	}
	else
	{
		if (IsBatchJobWaitingJob(key) == true)
		{
			AddItemForJobStart(requestSessionId, key, packetId, recvBuffer);
		}
		else
		{
			std::cout << "Receive invalid job. Request session id : " << requestSessionId 
				<< " Packet id : " << packetId << std::endl;
		}
	}
}

void DBServer::AddItemForJobStart(UINT64 requestSessionId, DBJobKey jobKey, UINT packetId, CSerializationBuf* recvBuffer)
{
	std::shared_ptr<BatchedDBJob> batchedJob = nullptr;
	{
		std::lock_guard lock(batchedDBJobMapLock);
		const auto& iter = batchedDBJobMap.find(jobKey);
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
		DoBatchedJob(requestSessionId, jobKey, batchedJob);
	}
}

void DBServer::DoBatchedJob(UINT64 requestSessionId, DBJobKey jobKey, std::shared_ptr<BatchedDBJob> batchedJob)
{
	std::list<CSerializationBuf*> resultList;
	bool isSuccess = true;
	for (auto& job : batchedJob->bufferList)
	{
		auto result = HandleImpl(requestSessionId, batchedJob->sessionId, static_cast<PACKET_ID>(job.first), job.second);
		CSerializationBuf::Free(job.second);

		if (IsSuccess(result) == true)
		{
			resultList.push_back(result.second);
		}
		else
		{
			isSuccess = false;
			break;
		}
	}

	CSerializationBuf* resultPacket = CSerializationBuf::Alloc();
	*resultPacket << jobKey << isSuccess;

	if (isSuccess == true)
	{
		SendPacket(requestSessionId, resultPacket);

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

		SendPacket(requestSessionId, resultPacket);
	}
}

bool DBServer::IsBatchJobWaitingJob(DBJobKey jobKey)
{
	std::lock_guard lock(batchedDBJobMapLock);
	const auto& iter = batchedDBJobMap.find(jobKey);
	if (iter == batchedDBJobMap.end())
	{
		return false;
	}

	return true;
}

ProcedureResult DBServer::HandleImpl(UINT64 requestSessionId, UINT64 userSessionId, PACKET_ID packetId, CSerializationBuf* recvBuffer)
{
	CSerializationBuf* packet = CSerializationBuf::Alloc();
	bool isSuccess = false;
	ODBCConnector& connector = ODBCConnector::GetInst();
	auto conn = connector.GetConnection();

	switch (packetId)
	{
	case PACKET_ID::TEST:
	{
		auto procedure = connector.GetProcedureInfo("test");
		if (procedure == nullptr)
		{
			break;
		}

		SP::test t;
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
	case PACKET_ID::SELECT_TEST_2:
	{
		auto procedure = connector.GetProcedureInfo("SELECT_TEST_2");
		if (procedure == nullptr)
		{
			break;
		}

		SP::SELECT_TEST_2 s;
		*recvBuffer >> s.id;

		if (connector.CallSPDirectWithSPObject(conn.value().stmtHandle, procedure, s) == false)
		{
			break;
		}

		auto results = connector.GetSPResult<SP::SELECT_TEST_2::ResultType>(conn.value().stmtHandle);
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
		cout << "Invalid packet id : " << static_cast<UINT>(packetId) << endl;
		break;
	}

	connector.FreeConnection(conn.value());
	return ProcedureResult(isSuccess, packet);
}