#include "PreCompile.h"
#include "DBServer.h"
#include "LanServerSerializeBuf.h"

DBServer::DBServer(const std::wstring& optionFile)
{
	if (Start(optionFile.c_str()) == false)
	{
		g_Dump.Crash();
	}
	
	std::cout << "---------------------" << '\n';
	std::cout << "DBServer On" << '\n';
	std::cout << "---------------------" << '\n';
}

DBServer::~DBServer()
{
	Stop();
}

void DBServer::StopServer()
{
	Stop();
}

void DBServer::OnClientJoin(UINT64 clientId)
{

}

void DBServer::OnClientLeave(UINT64 clientId)
{

}

bool DBServer::OnConnectionRequest()
{
	return true;
}

void DBServer::OnRecv(const UINT64 receivedSessionId, CSerializationBuf* outReadBuf)
{
	UINT packetId = 0;
	*outReadBuf >> packetId;

	HandlePacket(receivedSessionId, static_cast<PACKET_ID>(packetId), outReadBuf);
}

void DBServer::OnSend(UINT64 clientId, int sendSize)
{

}

void DBServer::OnWorkerThreadBegin()
{

}

void DBServer::OnWorkerThreadEnd()
{

}

void DBServer::OnError(st_Error* outError)
{

}