#pragma once

#include "LanServer.h"
#include <string>
#include <map>
#include <list>
#include <mutex>

struct BatchedDBJob
{
	explicit BatchedDBJob(unsigned char size)
		: batchSize(size)
	{
	}

	unsigned char batchSize = 0;
	std::list<std::pair<UINT, CSerializationBuf*>> bufferList;
	std::atomic_bool isRunning = false;
};

using ProcedureResult = std::pair<bool, CSerializationBuf*>;

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
	void InsertBatchJob(UINT64 sessionId, unsigned char batchSize);

private:
	void HandlePacket(UINT64 requestSessionId, UINT packetId, CSerializationBuf* recvBuffer);
	bool IsBatchJobWaitingUser(UINT64 userSessionId);
	void AddItemForJobStart(UINT64 requestSessionId, UINT64 userSessionId, UINT packetId, CSerializationBuf* recvBuffer);
	void DoBatchedJob(UINT64 requestSessionId, UINT64 userSessionId, std::shared_ptr<BatchedDBJob> batchedJob);
	ProcedureResult HandleImpl(UINT64 requestSessionId, UINT64 userSessionId, UINT packetId, CSerializationBuf* recvBuffer);

#pragma region BatchedDBJob
private:
	std::mutex batchedDBJobMapLock;
	std::map<UINT64, std::shared_ptr<BatchedDBJob>> batchedDBJobMap;
#pragma endregion BatchedDBJob
};