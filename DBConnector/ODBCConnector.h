#pragma once

#include <sql.h>
#include <sqlext.h>
#include <memory>
#include "ODBCMetaData.h"
#include "ODBCConst.h"
#include <list>
#include <mutex>
#include <optional>

struct ProcedureInfo;

struct DBConnection
{
	SQLHDBC dbcHandle;
	SQLHSTMT stmtHandle;
};

class DBConnectionPool
{
public:
	DBConnectionPool() = default;
	~DBConnectionPool();

public:
	bool Initialize(const std::wstring& inConnectionString, int inPoolSize);
	void Cleanup();

public:
	std::optional<DBConnection> GetConnection();

private:
	bool Initialize();

private:
	std::list<DBConnection> connectionList;
	std::mutex connectionLock;

private:
	int poolSize = 0;
	std::wstring connectionString;
};

class ODBCConnector
{
private:
	ODBCConnector();
	~ODBCConnector();

	ODBCConnector(const ODBCConnector&) = delete;
	ODBCConnector& operator=(const ODBCConnector&) = delete;

public:
	static ODBCConnector& GetInst();

public:
	bool ConnectDB(const std::wstring& optionFileName);
	void DisconnectDB();

	bool InitDB();

	template <typename... Args>
	bool CallStoredProcedure(const ProcedureName& procedureName, SQLHSTMT& stmtHandle, Args... args)
	{
		auto procedureInfo = GetProcedureInfo(procedureName);
		if (procedureInfo == nullptr)
		{
			return false;
		}

		procedureInfo->SettingSPMaker(stmtHandle, SP_PARAMETER_LOCATION, args...);
		if (ODBCUtil::DBSendQuery(procedureInfo->sql, stmtHandle) == false)
		{
			return false;
		}

		return true;
	}

	template <typename... Args>
	bool CallStoredProcedureDirect(const ProcedureName& procedureName, SQLHSTMT& stmtHandle, Args... args)
	{
		auto procedureInfo = GetProcedureInfo(procedureName);
		if (procedureInfo == nullptr)
		{
			return false;
		}

		procedureInfo->SettingSPMaker(stmtHandle, SP_PARAMETER_LOCATION, args...);
		if (ODBCUtil::DBSendQueryDirect(procedureInfo->sql, stmtHandle) == false)
		{
			return false;
		}

		return true;
	}

	template <typename... Args>
	bool CallStoredProcedureDirect(const ProcedureInfo* procedureInfo, SQLHSTMT& stmtHandle, Args... args)
	{
		if (procedureInfo == nullptr)
		{
			return false;
		}

		procedureInfo->SettingSPMaker(stmtHandle, SP_PARAMETER_LOCATION, args...);
		if (ODBCUtil::DBSendQueryDirect(procedureInfo->sql, stmtHandle) == false)
		{
			return false;
		}

		return true;
	}

private:
	bool MakeProcedureFromDB();
	bool MakeProcedureMetaData();

private:
	bool OptionParsing(const std::wstring& optionFileName);
	const std::wstring GetDBConnectionString();

public:
	SQLHSTMT GetDefaultStmtHandle();
	SQLHDBC GetDefaultDBCHandle();
	std::optional<DBConnection> GetConnection();

private:
	DBConnectionPool connectionPool;
	DBConnection defaultConnection;

public:
	const ProcedureInfo * const GetProcedureInfo(ProcedureName procedureName) const;

private:
	std::unique_ptr<ODBCMetaData> metaData;

public:
	WCHAR* GetSchemaName() { return schemaName; }

private:
	WCHAR dsn[16];
	WCHAR uid[16];
	WCHAR password[16];
	WCHAR schemaName[16];

	int connectionPoolSize = 4;
};
