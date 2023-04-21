#include "PreCompile.h"
#include "ODBCConnector.h"
#include <Windows.h>
#include <iostream>
#include "Parse.h"
#include "ODBCConst.h"

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

	std::wstring connectString;
	connectString += L"DSN=";
	connectString += dsn;
	connectString += L";UID=";
	connectString += uid;
	connectString += L";PWD=";
	connectString += password;
	sqlReturn = SQLDriverConnect(dbcHandle, NULL, (SQLWCHAR*)connectString.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
	if(sqlReturn != SQL_SUCCESS)
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
	if (MakeProcedureName() == false)
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

bool ODBCConnector::MakeProcedureName()
{
	SQLWCHAR* catalog_name = (SQLWCHAR*)L"SQL_ALL_CATALOGS";
	if (SQLProcedures(stmtHandle, (SQLWCHAR*)catalog_name, static_cast<SQLSMALLINT>(wcslen(catalog_name)), schemaName, wcslen(schemaName), NULL, NULL) != SQL_SUCCESS)
	{
		return false;
	}

	SQLCHAR procedureName[256];
	SQLRETURN ret;
	// SQLFetch 함수를 먼저 호출
	while (SQLFetch(stmtHandle) == SQL_SUCCESS)
	{
		ret = SQLGetData(stmtHandle, COLUMN_NUMBER::PROCEDURE_NAME, SQL_C_CHAR, procedureName, sizeof(procedureName), nullptr);
		if (ret != SQL_SUCCESS)
		{
			return false;
		}
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
	_wsetlocale(LC_ALL, L"Korean");

	CParser parser;
	WCHAR cBuffer[BUFFER_MAX];

	FILE* fp;
	_wfopen_s(&fp, optionFileName.c_str(), L"rt, ccs=UNICODE");
	if (fp == NULL)
		return false;

	int iJumpBOM = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int iFileSize = ftell(fp);
	fseek(fp, iJumpBOM, SEEK_SET);
	int FileSize = (int)fread_s(cBuffer, BUFFER_MAX, sizeof(WCHAR), BUFFER_MAX / 2, fp);
	int iAmend = iFileSize - FileSize; // 개행 문자와 파일 사이즈에 대한 보정값
	fclose(fp);

	cBuffer[iFileSize - iAmend] = '\0';
	WCHAR* pBuff = cBuffer;

	if (!parser.GetValue_String(pBuff, L"ODBC", L"DSN", dsn))
		return false;
	if (!parser.GetValue_String(pBuff, L"ODBC", L"UID", uid))
		return false;
	if (!parser.GetValue_String(pBuff, L"ODBC", L"PWD", password))
		return false;

	return true;
}