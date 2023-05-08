#pragma once

#include <sql.h>
#include <sqlext.h>
#include <memory>
#include "ODBCMetaData.h"
#include "ODBCConst.h"

struct ProcedureInfo;

class ODBCConnector
{
public:
	ODBCConnector();
	~ODBCConnector();

	ODBCConnector(const ODBCConnector&) = delete;
	ODBCConnector& operator=(const ODBCConnector&) = delete;

public:
	bool ConnectDB(const std::wstring& optionFileName);
	void DisconnectDB();

	bool InitDB();
	bool DBSendQuery(std::wstring query);
	bool DBSendQueryWithPrepare(std::wstring query);

	template <typename... Args>
	bool CallStoredProcedure(const ProcedureName& procedureName, Args... args)
	{
		auto procedureInfo = GetProcedureInfo(procedureName);
		if (procedureInfo == nullptr)
		{
			return false;
		}

		procedureInfo->SettingSPMaker(stmtHandle, SP_PARAMETER_LOCATION, args...);
		if (DBSendQuery(procedureInfo->sql) == false)
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
	bool ConnectSQLDriver();

public:
	SQLHSTMT GetStmtHandle();
	SQLHDBC GetDBCHandle();

private:
	SQLHENV enviromentHandle;
	SQLHDBC dbcHandle;
	SQLHSTMT stmtHandle;

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
};
