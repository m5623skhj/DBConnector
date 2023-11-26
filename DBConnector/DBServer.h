#pragma once

#include "LanServer.h"
#include <string>
#include <map>
#include <list>
#include <mutex>
#include "../../RIOServerTest/RIO_Test/EnumType.h"
#include "../../RIOServerTest/RIO_Test/DefineType.h"

class DBJobStart;

struct BatchedDBJob
{
	explicit BatchedDBJob(unsigned char inBatchSize, SessionId inSessionId)
		: batchSize(inBatchSize)
		, sessionId(inSessionId)
	{
	}

	unsigned char batchSize = 0;
	SessionId sessionId = INVALID_SESSION_ID;
	std::list<std::pair<PACKET_ID, CSerializationBuf*>> bufferList;
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
	void InsertBatchJob(DBJobKey jobKey, const DBJobStart& job);

private:
	void HandlePacket(UINT64 requestSessionId, PACKET_ID packetId, CSerializationBuf* recvBuffer);
	bool IsBatchJobWaitingJob(DBJobKey jobKey);
	void AddItemForJobStart(UINT64 requestSessionId, DBJobKey jobKey, PACKET_ID packetId, CSerializationBuf* recvBuffer);
	void DoBatchedJob(UINT64 requestSessionId, DBJobKey jobKey, std::shared_ptr<BatchedDBJob> batchedJob);
	ProcedureResult ProcedureHandleImpl(UINT64 requestSessionId, PACKET_ID packetId, CSerializationBuf* recvBuffer);
	bool DBJobHandleImpl(UINT64 requestSessionId, UINT64 userSessionId, PACKET_ID packetId, CSerializationBuf* recvBuffer);

#pragma region BatchedDBJob
private:
	std::mutex batchedDBJobMapLock;
	std::map<DBJobKey, std::shared_ptr<BatchedDBJob>> batchedDBJobMap;
#pragma endregion BatchedDBJob
};