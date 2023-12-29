#include "PreCompile.h"
#include "DBServer.h"
#include "LanServerSerializeBuf.h"
#include "StoredProcedure.h"
#include "ODBCConnector.h"
#include "../../RIOServerTest/RIO_Test/Protocol.h"

using namespace std;

void DBServer::InsertBatchJob(DBJobKey jobKey, const DBJobStart& job)
{
	std::lock_guard lock(batchedDBJobMapLock);
	batchedDBJobMap.insert({ jobKey, make_shared<BatchedDBJob>(job.batchSize, job.sessionId) });
}

void DBServer::HandlePacket(UINT64 requestSessionId, PACKET_ID packetId, CSerializationBuf* recvBuffer)
{
	DBJobKey key = INVALID_DB_JOB_KEY;
	*recvBuffer >> key;

	if (packetId == PACKET_ID::GAME2DB_BATCHED_DB_JOB)
	{
		DBJobStart job;
		*recvBuffer >> job.sessionId >> job.batchSize;
		InsertBatchJob(key, job);
	}
	else
	{
		if (key == 0)
		{
			ProcedureHandleImpl(requestSessionId, packetId, recvBuffer);
		}

		if (IsBatchJobWaitingJob(key) == true)
		{
			AddItemForJobStart(requestSessionId, key, packetId, recvBuffer);
		}
		else
		{
			std::cout << "Receive invalid job. Request session id : " << requestSessionId 
				<< " Packet id : " << static_cast<UINT>(packetId) << std::endl;
		}
	}
}

void DBServer::AddItemForJobStart(UINT64 requestSessionId, DBJobKey jobKey, PACKET_ID packetId, CSerializationBuf* recvBuffer)
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
	bool isSuccess = true;

	ODBCConnector& connector = ODBCConnector::GetInst();
	auto conn = connector.GetConnection();
	if (conn == nullopt)
	{
		g_Dump.Crash();
	}

	if (ODBCUtil::SQLIsSuccess(
		SQLSetConnectAttr(conn.value().dbcHandle, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, 0)) == false)
	{
		g_Dump.Crash();
	}

	for (auto& job : batchedJob->bufferList)
	{
		auto result = DBJobHandleImpl(requestSessionId, batchedJob->sessionId, static_cast<PACKET_ID>(job.first), conn.value(), job.second);
		CSerializationBuf::Free(job.second);

		if (isSuccess == false)
		{
			break;
		}
	}

	CSerializationBuf* resultPacket = CSerializationBuf::Alloc();
	*resultPacket << jobKey << isSuccess;

	if (isSuccess == true)
	{
		if (ODBCUtil::SQLIsSuccess(SQLEndTran(SQL_HANDLE_DBC, conn.value().dbcHandle, SQL_COMMIT)) == false)
		{
			g_Dump.Crash();
		}
	}
	else
	{
		if (ODBCUtil::SQLIsSuccess(SQLEndTran(SQL_HANDLE_DBC, conn.value().dbcHandle, SQL_ROLLBACK)) == false)
		{
			g_Dump.Crash();
		}
	}
	SendPacket(requestSessionId, resultPacket);
	SQLSetConnectAttr(conn.value().dbcHandle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
	connector.FreeConnection(conn.value());
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

ProcedureResult DBServer::ProcedureHandleImpl(UINT64 requestSessionId, PACKET_ID packetId, CSerializationBuf* recvBuffer)
{
	CSerializationBuf* packet = CSerializationBuf::Alloc();
	bool isSuccess = false;
	ODBCConnector& connector = ODBCConnector::GetInst();
	auto conn = connector.GetConnection();
	if (conn == nullopt)
	{
		g_Dump.Crash();
	}

	switch (packetId)
	{
	case PACKET_ID::GAME2DB_SELECT_TEST_2:
	{
		auto procedure = connector.GetProcedureInfo("SELECT_TEST_2");
		if (procedure == nullptr)
		{
			break;
		}

		CallSelectTest2ProcedurePacket item;
		*recvBuffer >> item.ownerSessionId >> item.id;

		SP::SELECT_TEST_2 s;
		s.id = item.id;

		if (connector.CallSPDirectWithSPObject(conn.value().stmtHandle, procedure, s) == false)
		{
			break;
		}

		auto results = connector.GetSPResult<SP::SELECT_TEST_2::ResultType>(conn.value().stmtHandle);
		if (results == nullopt)
		{
			break;
		}

		UINT sendPackeId = static_cast<UINT>(PACKET_ID::DB2GAME_SELECT_TEST_2);
		*packet << sendPackeId << item.ownerSessionId;

		for (const auto& result : results.value())
		{
			*packet << result.no;
			packet->WriteBuffer((char*)(result.tablename.GetCString()), sizeof(result.tablename));
		}
		isSuccess = true;
		break;
	}
	}

	connector.FreeConnection(conn.value());
	return ProcedureResult(isSuccess, packet);
}

bool DBServer::DBJobHandleImpl(UINT64 requestSessionId, UINT64 userSessionId, PACKET_ID packetId, DBConnection& conn, CSerializationBuf* recvBuffer)
{
	bool isSuccess = false;

	ODBCConnector& connector = ODBCConnector::GetInst();
	switch (packetId)
	{
	case PACKET_ID::GAME2DB_TEST:
	{
		auto procedure = connector.GetProcedureInfo("test");
		if (procedure == nullptr)
		{
			break;
		}

		SP::test t;
		*recvBuffer >> t.id3;
		recvBuffer->ReadBuffer((char*)t.teststring.GetCString(), sizeof(t.teststring));

		if (connector.CallSPDirectWithSPObject(conn.stmtHandle, procedure, t) == false)
		{
			break;
		}

		break;
	}
	default:
		cout << "Invalid packet id : " << static_cast<UINT>(packetId) << endl;
		break;
	}

	return isSuccess;
}