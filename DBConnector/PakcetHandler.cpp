#include "PreCompile.h"
#include "DBServer.h"
#include "DBServerProtocol.h"
#include "LanServerSerializeBuf.h"
#include "StoredProcedure.h"

using namespace std;

void DBServer::HandlePacket(WORD packetId, CSerializationBuf& recvBuffer)
{
	switch (packetId)
	{
	case DBServerProtocol::PACKET_ID::TEST:
	{
		test t;
		recvBuffer >> t.id3;
		recvBuffer.ReadBuffer((char*)t.teststring.GetCString(), sizeof(t.teststring));
	}
	case DBServerProtocol::PACKET_ID::INPUT_TEST:
	{
		input_test i;
		recvBuffer >> i.item >> i.item2;
	}
	case DBServerProtocol::PACKET_ID::SELECT_TEST:
	{
		SELECT_TEST s;
		recvBuffer >> s.id;
	}
	case DBServerProtocol::PACKET_ID::SELECT_TEST_2:
	{
		SELECT_TEST_2 s;
		recvBuffer >> s.id;
	}
	case DBServerProtocol::PACKET_ID::SELECT_TEST_3:
	{
		SELECT_TEST_3 s;
	}
	case DBServerProtocol::PACKET_ID::STRING_TEST_PROC:
	{
		string_test_proc s;
		recvBuffer.ReadBuffer((char*)s.test.GetCString(), sizeof(s.test));
	}
	case DBServerProtocol::PACKET_ID::UPDATE_TEST:
	{
		update_test u;
		recvBuffer >> u._id;
	}

	default:
		cout << "Invalid packet id : " << packetId << endl;
		break;
	}
}