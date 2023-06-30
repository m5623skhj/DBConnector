#pragma once

#include "LanServer.h"
#include <string>

class DBServer : public CLanServer
{
public:
	DBServer() = delete;
	explicit DBServer(const std::wstring& optionFile);
	virtual ~DBServer();

public:
	void StopServer();

protected:
	virtual void OnClientJoin(UINT64 OutClientID);
	virtual void OnClientLeave(UINT64 ClientID);
	virtual bool OnConnectionRequest();

	virtual void OnRecv(UINT64 ReceivedSessionID, CSerializationBuf* OutReadBuf);
	virtual void OnSend(UINT64 ClientID, int sendsize);

	virtual void OnWorkerThreadBegin();
	virtual void OnWorkerThreadEnd();
	virtual void OnError(st_Error* OutError);

private:
	void HandlePacket(WORD packetId, CSerializationBuf& recvBuffer);
};