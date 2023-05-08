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

	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const std::string& input)
	{
		if (ODBCUtil::SQLIsSuccess(SQLBindParameter(stmtHandle, parameterLocation, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_VARCHAR, 0, 0, (SQLPOINTER)(&input), 0, NULL)) == false)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}

	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const int& input)
	{
		if (ODBCUtil::SQLIsSuccess(SQLBindParameter(stmtHandle, parameterLocation, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&input), 0, NULL)) == false)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}
	
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const INT64& input)
	{
		if (ODBCUtil::SQLIsSuccess(SQLBindParameter(stmtHandle, parameterLocation, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_BIGINT, 0, 0, (SQLPOINTER)(&input), 0, NULL)) == false)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}
	
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const float& input)
	{
		if (ODBCUtil::SQLIsSuccess(SQLBindParameter(stmtHandle, parameterLocation, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_FLOAT, 0, 0, (SQLPOINTER)(&input), 0, NULL)) == false)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}
	
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const double& input)
	{
		if (ODBCUtil::SQLIsSuccess(SQLBindParameter(stmtHandle, parameterLocation, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_REAL, 0, 0, (SQLPOINTER)(&input), 0, NULL)) == false)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}
	
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const bool& input)
	{
		if (ODBCUtil::SQLIsSuccess(SQLBindParameter(stmtHandle, parameterLocation, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_BIT, 0, 0, (SQLPOINTER)(&input), 0, NULL)) == false)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}
}