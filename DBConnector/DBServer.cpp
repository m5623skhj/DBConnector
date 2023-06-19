#include "PreCompile.h"
#include "DBServer.h"

DBServer::DBServer(const std::wstring& optionFile)
{
	if (Start(optionFile.c_str()) == false)
	{
		g_Dump.Crash();
	}
}

DBServer::~DBServer()
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