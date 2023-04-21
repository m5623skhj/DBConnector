#pragma once

#include <sql.h>
#include <sqlext.h>

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
	bool MakeProcedureName();
	bool MakeProcedureMetaData();

private:


private:
	bool OptionParsing(const std::wstring& optionFileName);

public:
	SQLHSTMT GetStmtHandle();

private:
	SQLHENV enviromentHandle;
	SQLHDBC dbcHandle;
	SQLHSTMT stmtHandle;

public:
	WCHAR* GetSchemaName() { return schemaName; }

private:
	WCHAR dsn[16];
	WCHAR uid[16];
	WCHAR password[16];
	WCHAR schemaName[16];
};