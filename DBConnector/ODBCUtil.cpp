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
}