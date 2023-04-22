#include "PreCompile.h"
#include "ODBCConnector.h"
#include "ODBCConst.h"
#include "ODBCMetaData.h"

#include "Parse.h"

#include <Windows.h>
#include <iostream>
#include <set>

ODBCConnector::ODBCConnector()
{

}

ODBCConnector::~ODBCConnector()
{

}

bool ODBCConnector::ConnectDB(const std::wstring& optionFileName)
{
	if (OptionParsing(optionFileName) == false)
	{
		std::cout << "OptionParsing() Failed" << std::endl;
		return false;
	}

	SQLRETURN sqlReturn;

	sqlReturn = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &enviromentHandle);
	if (sqlReturn != SQL_SUCCESS)
	{
		std::cout << "env handle alloc Failed : " << sqlReturn << std::endl;
		return false;
	}

	sqlReturn = SQLSetEnvAttr(enviromentHandle, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
	if (sqlReturn != SQL_SUCCESS)
	{
		std::cout << "SQLSetEnvAttr() Failed : " << sqlReturn << std::endl;
		return false;
	}

	sqlReturn = SQLAllocHandle(SQL_HANDLE_DBC, enviromentHandle, &dbcHandle);
	if (sqlReturn != SQL_SUCCESS)
	{
		std::cout << "dbc handle alloc Failed : " << sqlReturn << std::endl;
		return false;
	}

	if (ConnectSQLDriver() == false)
	{
		std::cout << "SQLDriverConnect() Failed : " << sqlReturn << std::endl;
		return false;
	}

	sqlReturn = SQLAllocHandle(SQL_HANDLE_STMT, dbcHandle, &stmtHandle);
	if (sqlReturn != SQL_SUCCESS)
	{
		std::cout << "stmt handle alloc Failed : " << sqlReturn << std::endl;
		return false;
	}

	return true;
}

void ODBCConnector::DisconnectDB()
{
	SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
	SQLDisconnect(dbcHandle);
	SQLFreeHandle(SQL_HANDLE_DBC, dbcHandle);
	SQLFreeHandle(SQL_HANDLE_ENV, enviromentHandle);
}

bool ODBCConnector::InitDB()
{
	metaData.reset(new ODBCMetaData(schemaName));

	if (MakeProcedureFromDB() == false)
	{
		std::cout << "MakeProcedureName Failed : " << std::endl;
		return false;
	}

	return true;
}

bool ODBCConnector::DBSendQuery(std::wstring query)
{
	if (SQLPrepare(stmtHandle, (SQLWCHAR*)query.c_str(), SQL_NTS) != SQL_SUCCESS)
	{
		return false;
	}

	if (SQLExecute(stmtHandle) != SQL_SUCCESS)
	{
		return false;
	}

	return true;
}

bool ODBCConnector::MakeProcedureFromDB()
{
	std::set<std::string> procedureNameList;
	if (metaData == nullptr)
	{
		return false;
	}

	if (metaData->GetProcedureNameFromDB(*this, schemaName, procedureNameList) == false)
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

SQLHSTMT ODBCConnector::GetStmtHandle()
{
	return stmtHandle;
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

	return true;
}

bool ODBCConnector::ConnectSQLDriver()
{
	std::wstring connectString;
	connectString += L"DSN=";
	connectString += dsn;
	connectString += L";UID=";
	connectString += uid;
	connectString += L";PWD=";
	connectString += password;
	SQLRETURN sqlReturn = SQLDriverConnect(dbcHandle, NULL, (SQLWCHAR*)connectString.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
	if (sqlReturn != SQL_SUCCESS)
	{
		return false;
	}

	return true;
}
