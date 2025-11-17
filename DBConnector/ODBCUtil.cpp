#include "PreCompile.h"

#include <sql.h>
#include <sqlext.h>
#include "StoredProcedure.h"

namespace ODBCUtil
{
	void PrintSQLErrorMessage(const SQLHSTMT stmtHandle)
	{
		SQLWCHAR sqlState[6];
		SQLWCHAR msg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER nativeError;
		SQLSMALLINT msgLen;

		const SQLRETURN rc = SQLGetDiagRec(SQL_HANDLE_STMT, stmtHandle, 1, sqlState, &nativeError, msg, sizeof(msg), &msgLen);
		const std::wstring errorMessage = msg;

		if (SQL_SUCCEEDED(rc)) 
		{
			std::wcout << L"SQL error message : " << errorMessage << std::endl;
		}
	}

	bool SQLIsSuccess(const SQLRETURN returnValue)
	{
		if (returnValue == SQL_SUCCESS || returnValue == SQL_SUCCESS_WITH_INFO)
		{
			return true;
		}

		return false;
	}

	bool IsSameType(const std::string& lhs, const std::string& rhs)
	{
		// string
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

		// wstring
		if (lhs == "FWString")
		{
			if (rhs == "nvarchar")
			{
				return true;
			}
		}
		else if (rhs == "FWString")
		{
			if (lhs == "nvarchar")
			{
				return true;
			}
		}

		// int64
		if (lhs == "__int64")
		{
			if (rhs == "bigint")
			{
				return true;
			}
		}
		else if (rhs == "bigint")
		{
			if (lhs == "__int64")
			{
				return true;
			}
		}

		return false;
	}

	std::wstring GetDataTypeName(const SQLSMALLINT inDataType)
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
		case SQL_WVARCHAR:
			return L"nvarchar";
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
		if (ODBCUtil::SQLIsSuccess(SQLExecDirect(stmtHandle, (SQLWCHAR*)query.c_str(), SQL_NTS)) == false)
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
}