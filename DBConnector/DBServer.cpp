#include "PreCompile.h"
#include "DBServer.h"
#include "LanServerSerializeBuf.h"

DBServer::DBServer(const std::wstring& optionFile)
{
	if (Start(optionFile.c_str()) == false)
	{
		g_Dump.Crash();
	}
	
	std::cout << "---------------------" << std::endl;
	std::cout << "DBServer On" << std::endl;
	std::cout << "---------------------" << std::endl;
}

DBServer::~DBServer()
{
	Stop();
}

void DBServer::StopServer()
{
	Stop();
}

void DBServer::OnClientJoin(UINT64 OutClientID)
{

}

void DBServer::OnClientLeave(UINT64 ClientID)
{

}

bool DBServer::OnConnectionRequest()
{
	return true;
}

void DBServer::OnRecv(UINT64 ReceivedSessionID, CSerializationBuf* OutReadBuf)
{
	UINT packetId = 0;
	*OutReadBuf >> packetId;

	HandlePacket(ReceivedSessionID, packetId, OutReadBuf);
}

void DBServer::OnSend(UINT64 ClientID, int sendsize)
{

}

void DBServer::OnWorkerThreadBegin()
{

}

void DBServer::OnWorkerThreadEnd()
{

}

void DBServer::OnError(st_Error* OutError)
{

}