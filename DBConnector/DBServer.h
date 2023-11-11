#pragma once

#include "LanServer.h"
#include <string>
#include <map>
#include <mutex>
#include "../../RIOServerTest/RIO_Test/EnumType.h"

struct BatchedDBJob
{
	unsigned char batchSize = 0;
	std::map<unsigned char, CSerializationBuf*> bufferList;
};

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
	void HandlePacket(UINT64 requestSessionId, PACKET_ID packetId, CSerializationBuf* recvBuffer);

#pragma region BatchedDBJob
private:
	std::mutex batchedDBJobMapLock;
	std::map<UINT64, BatchedDBJob> batchedDBJobMap;
#pragma endregion BatchedDBJob
};