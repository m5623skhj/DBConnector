#pragma once

#include "LanServer.h"
#include <string>
#include <map>
#include <list>
#include <mutex>
#include "../../RIOServerTest/RIO_Test/EnumType.h"
#include "../../RIOServerTest/RIO_Test/DefineType.h"

class DBJobStart;
struct DBConnection;

struct BatchedDBJob
{
	explicit BatchedDBJob(const unsigned char inBatchSize, const SessionId inSessionId)
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

class DBServer final : public CLanServer
{
public:
	DBServer() = delete;
	explicit DBServer(const std::wstring& optionFile);
	~DBServer() override;

public:
	void StopServer();

protected:
	void OnClientJoin(UINT64 clientId) override;
	void OnClientLeave(UINT64 clientId) override;
	bool OnConnectionRequest() override;

	void OnRecv(UINT64 receivedSessionId, CSerializationBuf* outReadBuf) override;
	void OnSend(UINT64 clientId, int sendSize) override;

	void OnWorkerThreadBegin() override;
	void OnWorkerThreadEnd() override;
	void OnError(st_Error* outError) override;

private:
	void InsertBatchJob(DBJobKey jobKey, const DBJobStart& job);

private:
	void HandlePacket(UINT64 requestSessionId, PACKET_ID packetId, CSerializationBuf* recvBuffer);
	bool IsBatchJobWaitingJob(DBJobKey jobKey);
	void AddItemForJobStart(UINT64 requestSessionId, DBJobKey jobKey, PACKET_ID packetId, CSerializationBuf* recvBuffer);
	void DoBatchedJob(UINT64 requestSessionId, DBJobKey jobKey, const std::shared_ptr<BatchedDBJob>& batchedJob);
	static ProcedureResult ProcedureHandleImpl(UINT64 requestSessionId, PACKET_ID packetId, CSerializationBuf* recvBuffer);
	static bool DbJobHandleImpl(UINT64 requestSessionId, UINT64 userSessionId, PACKET_ID packetId, DBConnection& conn, CSerializationBuf* recvBuffer);

#pragma region BatchedDBJob
private:
	std::mutex batchedDbJobMapLock;
	std::map<DBJobKey, std::shared_ptr<BatchedDBJob>> batchedDbJobMap;
#pragma endregion BatchedDBJob
};