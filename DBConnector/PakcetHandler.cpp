#include "PreCompile.h"
#include "DBServer.h"
#include "DBServerProtocol.h"
#include "LanServerSerializeBuf.h"
#include "StoredProcedure.h"
#include "ODBCConnector.h"
#include "../../RIOServerTest/RIO_Test/Protocol.h"
#include "../../RIOServerTest/RIO_Test/EnumType.h"

using namespace std;

void DBServer::HandlePacket(UINT64 requestSessionId, UINT packetId, CSerializationBuf& recvBuffer)
{
	ODBCConnector& connector = ODBCConnector::GetInst();
	auto conn = connector.GetConnection();
	UINT64 sessionId = 0;

	recvBuffer >> sessionId;

	switch (packetId)
	{
	case DBServerProtocol::PACKET_ID::TEST:
	{
		auto procedure = connector.GetProcedureInfo("test");
		if (procedure == nullptr)
		{
			// 무슨 처리를 해야하나?
			return;
		}

		test t;
		recvBuffer >> t.id3;
		recvBuffer.ReadBuffer((char*)t.teststring.GetCString(), sizeof(t.teststring));

		if (connector.CallSPDirectWithSPObject(conn.value().stmtHandle, procedure, t) == false)
		{
			// 무슨 처리를 해야하나?
			return;
		}

		CSerializationBuf& packet = *CSerializationBuf::Alloc();
		UINT sendPackeId = static_cast<UINT>(PACKET_ID::CALL_TEST_PROCEDURE_PACKET_REPLY);
		packet << sendPackeId << sessionId;

		SendPacket(requestSessionId, &packet);
		break;
	}
	case DBServerProtocol::PACKET_ID::INPUT_TEST:
	{
		input_test i;
		recvBuffer >> i.item >> i.item2;
		break;
	}
	case DBServerProtocol::PACKET_ID::SELECT_TEST:
	{
		SELECT_TEST s;
		recvBuffer >> s.id;
		break;
	}
	case DBServerProtocol::PACKET_ID::SELECT_TEST_2:
	{
		auto procedure = connector.GetProcedureInfo("SELECT_TEST_2");
		if (procedure == nullptr)
		{
			// 무슨 처리를 해야하나?
			return;
		}

		SELECT_TEST_2 s;
		recvBuffer >> s.id;

		if (connector.CallSPDirectWithSPObject(conn.value().stmtHandle, procedure, s) == false)
		{
			// 무슨 처리를 해야하나?
			return;
		}

		auto results = connector.GetSPResult<SELECT_TEST_2::ResultType>(conn.value().stmtHandle);
		if (results == nullopt)
		{
			// 무슨 처리를 해야하나?
			return;
		}

		CSerializationBuf& packet = *CSerializationBuf::Alloc();
		UINT sendPackeId = static_cast<UINT>(PACKET_ID::CALL_SELECT_TEST_2_PROCEDURE_PACKET_REPLY);
		packet << sendPackeId << sessionId;

		for (const auto& result : results.value())
		{
			packet << result.no;
			packet.WriteBuffer((char*)(result.tablename.GetCString()), sizeof(result.tablename));
		}

		SendPacket(requestSessionId, &packet);
		break;
	}
	case DBServerProtocol::PACKET_ID::SELECT_TEST_3:
	{
		SELECT_TEST_3 s;
		break;
	}
	case DBServerProtocol::PACKET_ID::STRING_TEST_PROC:
	{
		string_test_proc s;
		recvBuffer.ReadBuffer((char*)s.test.GetCString(), sizeof(s.test));
		break;
	}
	case DBServerProtocol::PACKET_ID::UPDATE_TEST:
	{
		update_test u;
		recvBuffer >> u._id;
		break;
	}

	default:
		cout << "Invalid packet id : " << packetId << endl;
		break;
	}

	// 커넥션 반환
	connector.FreeConnection(conn.value());
}