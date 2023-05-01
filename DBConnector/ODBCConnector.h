#pragma once

#include <sql.h>
#include <sqlext.h>
#include <memory>

class ODBCMetaData;

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

private:
	bool MakeProcedureFromDB();
	bool MakeProcedureMetaData();

private:


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
