#include "PreCompile.h"
#include "ODBCConnector.h"
#include "ODBCUtil.h"

#include "Parse.h"

#include <Windows.h>
#include <iostream>
#include <set>

DBConnectionPool::~DBConnectionPool()
{
	Cleanup();
}

bool DBConnectionPool::Initialize(const std::wstring& inConnectionString, int inPoolSize)
{
	connectionString = inConnectionString;
	poolSize = inPoolSize;

	if (poolSize == 0)
	{
		poolSize = 1;
	}

	return Initialize();
}

void DBConnectionPool::Cleanup()
{
	std::lock_guard lock(connectionLock);

	while (not connectionStack.empty())
	{
		const auto& [dbcHandle, stmtHandle] = connectionStack.top();

		if (stmtHandle != SQL_NULL_HSTMT)
		{
			SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
		}

		if (dbcHandle != SQL_NULL_HDBC)
		{
			SQLDisconnect(dbcHandle);
			SQLFreeHandle(SQL_HANDLE_DBC, dbcHandle);
		}

		connectionStack.pop();
	}

	if (environmentHandle != SQL_NULL_HENV)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, environmentHandle);
		environmentHandle = SQL_NULL_HENV;
	}
}

std::optional<DBConnection> DBConnectionPool::GetConnection()
{
	std::lock_guard lock(connectionLock);

	if (connectionStack.empty())
	{
		return std::nullopt;
	}

	DBConnection connection = connectionStack.top();
	connectionStack.pop();

	return connection;
}

void DBConnectionPool::FreeConnection(DBConnection& connection)
{
	std::lock_guard lock(connectionLock);
	connectionStack.push(connection);
}

bool DBConnectionPool::Initialize() 
{
	SQLRETURN sqlReturn = SQLAllocHandle(SQL_HANDLE_ENV, nullptr, &environmentHandle);
	if (sqlReturn != SQL_SUCCESS)
	{
		std::cout << "env handle alloc Failed : " << sqlReturn << '\n';
		return false;
	}

	sqlReturn = SQLSetEnvAttr(environmentHandle, SQL_ATTR_ODBC_VERSION, reinterpret_cast<void*>(SQL_OV_ODBC3), 0);
	if (sqlReturn != SQL_SUCCESS)
	{
		std::cout << "SQLSetEnvAttr() Failed : " << sqlReturn << '\n';
		Cleanup();
		return false;
	}

	for (int i = 0; i < poolSize; ++i) 
	{
		DBConnection conn;

		sqlReturn = SQLAllocHandle(SQL_HANDLE_DBC, environmentHandle, &conn.dbcHandle);
		if (sqlReturn != SQL_SUCCESS)
		{
			std::cout << "dbc handle alloc Failed : " << sqlReturn << '\n';
			Cleanup();
			return false;
		}

		sqlReturn = SQLDriverConnect(conn.dbcHandle, nullptr, const_cast<SQLWCHAR*>(connectionString.c_str()), SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE);
		if (ODBCUtil::SQLIsSuccess(sqlReturn) == false)
		{
			std::cout << "ConnectSQLDriver() failed" << '\n';
			SQLFreeHandle(SQL_HANDLE_DBC, conn.dbcHandle);
			Cleanup();
			return false;
		}

		sqlReturn = SQLAllocHandle(SQL_HANDLE_STMT, conn.dbcHandle, &conn.stmtHandle);
		if (sqlReturn != SQL_SUCCESS)
		{
			std::cout << "stmt handle alloc Failed : " << sqlReturn << '\n';
			SQLDisconnect(conn.dbcHandle);
			SQLFreeHandle(SQL_HANDLE_DBC, conn.dbcHandle);
			Cleanup();
			return false;
		}

		connectionStack.push(conn);
	}

	return true;
}

ODBCConnector& ODBCConnector::GetInst()
{
	static ODBCConnector instance;
	return instance;
}

bool ODBCConnector::ConnectDB(const std::wstring& optionFileName)
{
	if (OptionParsing(optionFileName) == false)
	{
		std::cout << "OptionParsing() Failed" << '\n';
		return false;
	}

	SQLRETURN sqlReturn = SQLAllocHandle(SQL_HANDLE_ENV, nullptr, &environmentHandle);
	if (sqlReturn != SQL_SUCCESS)
	{
		std::cout << "env handle alloc Failed : " << sqlReturn << '\n';
		return false;
	}

	sqlReturn = SQLSetEnvAttr(environmentHandle, SQL_ATTR_ODBC_VERSION, reinterpret_cast<void*>(SQL_OV_ODBC3), 0);
	if (sqlReturn != SQL_SUCCESS)
	{
		std::cout << "SQLSetEnvAttr() Failed : " << sqlReturn << '\n';
		DisconnectDB();
		return false;
	}

	sqlReturn = SQLAllocHandle(SQL_HANDLE_DBC, environmentHandle, &defaultConnection.dbcHandle);
	if (sqlReturn != SQL_SUCCESS)
	{
		std::cout << "dbc handle alloc Failed : " << sqlReturn << '\n';
		DisconnectDB();
		return false;
	}

	sqlReturn = SQLDriverConnect(defaultConnection.dbcHandle, nullptr, const_cast<SQLWCHAR*>(GetDBConnectionString().c_str()), SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE);
	if (ODBCUtil::SQLIsSuccess(sqlReturn) == false)
	{
		std::cout << "ConnectSQLDriver() failed" << '\n';
		DisconnectDB();
		return false;
	}

	sqlReturn = SQLAllocHandle(SQL_HANDLE_STMT, defaultConnection.dbcHandle, &defaultConnection.stmtHandle);
	if (sqlReturn != SQL_SUCCESS)
	{
		std::cout << "stmt handle alloc Failed : " << sqlReturn << '\n';
		DisconnectDB();
		return false;
	}

	if (connectionPool.Initialize(GetDBConnectionString(), connectionPoolSize) == false)
	{
		std::cout << "Initialize connection pool failed" << '\n';
		DisconnectDB();
		return false;
	}

	return true;
}

void ODBCConnector::DisconnectDB()
{
	connectionPool.Cleanup();

	if (defaultConnection.stmtHandle != SQL_NULL_HSTMT)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, defaultConnection.stmtHandle);
		defaultConnection.stmtHandle = SQL_NULL_HSTMT;
	}

	if (defaultConnection.dbcHandle != SQL_NULL_HDBC)
	{
		SQLDisconnect(defaultConnection.dbcHandle);
		SQLFreeHandle(SQL_HANDLE_DBC, defaultConnection.dbcHandle);
		defaultConnection.dbcHandle = SQL_NULL_HDBC;
	}

	if (environmentHandle != SQL_NULL_HENV)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, environmentHandle);
		environmentHandle = SQL_NULL_HENV;
	}
}

bool ODBCConnector::InitDB()
{
	metaData.reset(new ODBCMetaData(schemaName));

	if (MakeProcedureFromDB() == false)
	{
		std::cout << "MakeProcedureName Failed : " << '\n';
		return false;
	}

	return true;
}

bool ODBCConnector::MakeProcedureFromDB()
{
	std::set<ProcedureName> procedureNameList;
	if (metaData == nullptr)
	{
		return false;
	}

	if (metaData->GetProcedureNameFromDB(*this, catalogName, schemaName, procedureNameList) == false)
	{
		return false;
	}

	if (metaData->MakeProcedureColumnInfoFromDB(*this, procedureNameList) == false)
	{
		return false;
	}

	return true;
}

bool ODBCConnector::MakeProcedureMetaData()
{
	return true;
}

SQLHSTMT ODBCConnector::GetDefaultStmtHandle() const
{
	return defaultConnection.stmtHandle;
}

SQLHDBC ODBCConnector::GetDefaultDBCHandle() const
{
	return defaultConnection.dbcHandle;
}

std::optional<DBConnection> ODBCConnector::GetConnection()
{
	return connectionPool.GetConnection();
}

void ODBCConnector::FreeConnection(DBConnection& connection)
{
	connectionPool.FreeConnection(connection);
}

bool ODBCConnector::OptionParsing(const std::wstring& optionFileName)
{
	WCHAR buffer[BUFFER_MAX];
	LoadParsingText(buffer, optionFileName.c_str(), BUFFER_MAX);

	if (g_Paser.GetValue_String(buffer, L"ODBC", L"DSN", dsn) == false)
		return false;
	if (g_Paser.GetValue_String(buffer, L"ODBC", L"UID", uid) == false)
		return false;
	if (g_Paser.GetValue_String(buffer, L"ODBC", L"PWD", password) == false)
		return false;
	if (g_Paser.GetValue_String(buffer, L"ODBC", L"Schema", schemaName) == false)
		return false;
	if (g_Paser.GetValue_String(buffer, L"ODBC", L"CATALOG_NAME", catalogName) == false)
		return false;
	if (g_Paser.GetValue_String(buffer, L"ODBC", L"DRIVER", driver) == false)
		return false;
	if (g_Paser.GetValue_Short(buffer, L"ODBC", L"PORT", &port) == false)
		return false;
	if (g_Paser.GetValue_Int(buffer, L"CONNECTION_POOL", L"SIZE", &connectionPoolSize) == false)
		return false;

	return true;
}

std::wstring ODBCConnector::GetDBConnectionString() const
{
	std::wstring connectString;
	connectString += L"DRIVER=";
	connectString += driver;
	connectString += L";SERVER=";
	connectString += dsn;
	connectString += L";DATABASE=";
	connectString += catalogName;
	connectString += L";UID=";
	connectString += uid;
	connectString += L";PWD=";
	connectString += password;

	return connectString;
}

const ProcedureInfo* ODBCConnector::GetProcedureInfo(const ProcedureName& procedureName) const
{
	return metaData->GetProcedureInfo(procedureName);
}
