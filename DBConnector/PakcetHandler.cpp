#include "PreCompile.h"
#include "DBServer.h"
#include "LanServerSerializeBuf.h"
#include "StoredProcedure.h"
#include "ODBCConnector.h"
#include "../../RIOServerTest/RIO_Test/Protocol.h"

using namespace std;

void DBServer::InsertBatchJob(DBJobKey jobKey, const DBJobStart& job)
{
	std::lock_guard lock(batchedDbJobMapLock);
	batchedDbJobMap.insert({ jobKey, make_shared<BatchedDBJob>(job.batchSize, job.sessionId) });
}

void DBServer::HandlePacket(const UINT64 requestSessionId, PACKET_ID packetId, CSerializationBuf* recvBuffer)
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
				<< " Packet id : " << static_cast<UINT>(packetId) << '\n';
		}
	}
}

void DBServer::AddItemForJobStart(const UINT64 requestSessionId, const DBJobKey jobKey, PACKET_ID packetId, CSerializationBuf* recvBuffer)
{
	std::shared_ptr<BatchedDBJob> batchedJob = nullptr;
	{
		std::lock_guard lock(batchedDbJobMapLock);
		const auto& itor = batchedDbJobMap.find(jobKey);
		if (itor == batchedDbJobMap.end())
		{
			return;
		}

		batchedJob = itor->second;
		batchedDbJobMap.erase(jobKey);
	}

	if (batchedJob == nullptr)
	{
		return;
	}

	CSerializationBuf::AddRefCount(recvBuffer);
	batchedJob->bufferList.emplace_back(std::make_pair(packetId, recvBuffer));
	if (batchedJob->batchSize == batchedJob->bufferList.size())
	{
		DoBatchedJob(requestSessionId, jobKey, batchedJob);
	}
}

void DBServer::DoBatchedJob(const UINT64 requestSessionId, const DBJobKey jobKey, const std::shared_ptr<BatchedDBJob>& batchedJob)
{
	bool isSuccess = true;

	ODBCConnector& connector = ODBCConnector::GetInst();
	auto conn = connector.GetConnection();
	if (conn == nullopt)
	{
		g_Dump.Crash();
	}

	if (ODBCUtil::SQLIsSuccess(
		SQLSetConnectAttr(conn.value().dbcHandle, SQL_ATTR_AUTOCOMMIT, nullptr, 0)) == false)
	{
		g_Dump.Crash();
	}

	for (const auto& [packetId, buffer] : batchedJob->bufferList)
	{
		isSuccess = DbJobHandleImpl(requestSessionId, batchedJob->sessionId, packetId, conn.value(), buffer);
		CSerializationBuf::Free(buffer);

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
	SQLSetConnectAttr(conn.value().dbcHandle, SQL_ATTR_AUTOCOMMIT, reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_ON), 0);
	connector.FreeConnection(conn.value());
}

bool DBServer::IsBatchJobWaitingJob(const DBJobKey jobKey)
{
	std::lock_guard lock(batchedDbJobMapLock);
	if (const auto& itor = batchedDbJobMap.find(jobKey); itor == batchedDbJobMap.end())
	{
		return false;
	}

	return true;
}

ProcedureResult DBServer::ProcedureHandleImpl(UINT64 requestSessionId, const PACKET_ID packetId, CSerializationBuf* recvBuffer)
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

		const auto results = connector.GetSPResult<SP::SELECT_TEST_2::ResultType>(conn.value().stmtHandle);
		if (results == nullopt)
		{
			break;
		}

		constexpr UINT sendPacketId = static_cast<UINT>(PACKET_ID::DB2GAME_SELECT_TEST_2);
		*packet << sendPacketId << item.ownerSessionId;

		for (const auto& result : results.value())
		{
			*packet << result.no;
			packet->WriteBuffer(reinterpret_cast<char*>(const_cast<WCHAR*>(result.tablename.GetCString())), sizeof(result.tablename));
		}
		isSuccess = true;
		break;
	}
	}

	connector.FreeConnection(conn.value());
	return { isSuccess, packet };
}

bool DBServer::DbJobHandleImpl(UINT64 requestSessionId, UINT64 userSessionId, PACKET_ID packetId, DBConnection& conn, CSerializationBuf* recvBuffer)
{
	bool isSuccess = false;

	ODBCConnector& connector = ODBCConnector::GetInst();
	switch (packetId)
	{
	case PACKET_ID::GAME2DB_TEST:
	{
		const auto procedure = connector.GetProcedureInfo("test");
		if (procedure == nullptr)
		{
			break;
		}

		SP::test t;
		*recvBuffer >> t.id3;
		t.teststring.resize(recvBuffer->GetUseSize());
		recvBuffer->ReadBuffer(reinterpret_cast<char*>(const_cast<WCHAR*>(t.teststring.GetCString())), recvBuffer->GetUseSize());

		if (connector.CallSPDirectWithSPObject(conn.stmtHandle, procedure, t) == false)
		{
			break;
		}

		isSuccess = true;
	}
	default:
		cout << "Invalid packet id : " << static_cast<UINT>(packetId) << '\n';
		break;
	}

	return isSuccess;
}
