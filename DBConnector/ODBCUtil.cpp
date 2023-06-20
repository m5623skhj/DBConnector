#include "PreCompile.h"

#include <sql.h>
#include <sqlext.h>

namespace ODBCUtil
{
	void PrintSQLErrorMessage(SQLHSTMT stmtHandle)
	{
		SQLWCHAR SqlState[6];
		SQLWCHAR Msg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER NativeError;
		SQLSMALLINT MsgLen;
		SQLRETURN rc;

		rc = SQLGetDiagRec(SQL_HANDLE_STMT, stmtHandle, 1, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen);
		std::wstring errorMessage = Msg;
		if (SQL_SUCCEEDED(rc)) {
			std::wcout << L"SQL error message : " << errorMessage << std::endl;
		}
	}

	bool SQLIsSuccess(SQLRETURN returnValue)
	{
		if (returnValue == SQL_SUCCESS || returnValue == SQL_SUCCESS_WITH_INFO)
		{
			return true;
		}

		return false;
	}

	bool IsSameType(const std::string& lhs, const std::string& rhs)
	{
		if (lhs == "FString")
		{
			if (rhs == "varchar")
			{
				return true;
			}
		}
		else if (rhs == "FString")
		{
			if (lhs == "varchar")
			{
				return true;
			}
		}

		return false;
	}

	std::wstring GetDataTypeName(SQLSMALLINT inDataType)
	{
		switch (inDataType)
		{
		case SQL_BIGINT:
			return L"int64";
		case SQL_NUMERIC:
		case SQL_DECIMAL:
		case SQL_INTEGER:
			return L"int";
		case SQL_FLOAT:
		case SQL_REAL:
			return L"float";
		case SQL_VARCHAR:
			return L"string";
		case SQL_BIT:
			return L"bool";
		default:
			return L"";
		}
	}

	bool DBSendQuery(const std::wstring& query, SQLHSTMT& stmtHandle)
	{
		if (SQLExecute(stmtHandle) != SQL_SUCCESS)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}

	bool DBSendQueryDirect(const std::wstring& query, SQLHSTMT& stmtHandle)
	{
		if (SQLExecDirect(stmtHandle, (SQLWCHAR*)query.c_str(), SQL_NTS) != SQL_SUCCESS)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}

	bool DBSendQueryWithPrepare(const std::wstring& query, SQLHSTMT& stmtHandle)
	{
		if (SQLPrepare(stmtHandle, (SQLWCHAR*)query.c_str(), SQL_NTS) != SQL_SUCCESS)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		if (SQLExecute(stmtHandle) != SQL_SUCCESS)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}

	void GetDBResult(SQLHSTMT& stmtHandle)
	{
		while (SQLFetch(stmtHandle) == SQL_SUCCESS)
		{

		}

		SQLCloseCursor(stmtHandle);
	}
}