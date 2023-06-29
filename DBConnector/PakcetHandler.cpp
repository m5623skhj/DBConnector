#include "DBServer.h"
#include "DBServerProtocol.h"
#include "LanServerSerializeBuf.h"
#include <iostream>

using namespace std;
using namespace DBServerProtocol;

void DBServer::HandlePacket(WORD packetId, CSerializationBuf& recvBuffer)
{
	switch (packetId)
	{
	case PACKET_ID::TEST:
	{
		test t;
		recvBuffer >> t.id3;
		recvBuffer.ReadBuffer((char*)t.teststring, sizeof(t.teststring));
	}
	case PACKET_ID::INPUT_TEST:
	{
		input_test i;
		recvBuffer >> i.item >> i.item2;
	}
	case PACKET_ID::SELECT_TEST:
	{
		select_test s;
		recvBuffer >> s.id;
	}
	case PACKET_ID::SELECT_TEST_2:
	{
		select_test_2 s;
		recvBuffer >> s.id;
	}
	case PACKET_ID::SELECT_TEST_3:
	{

	}
	case PACKET_ID::STRING_TEST_PROC:
	{
		string_test_proc s;
		recvBuffer.ReadBuffer((char*)s.test, sizeof(s.test));
	}
	case PACKET_ID::UPDATE_TEST:
	{
		update_test u;
		recvBuffer >> u._id;
	}

	default:
		cout << "Invalid packet id : " << packetId << endl;
		break;
	}
}