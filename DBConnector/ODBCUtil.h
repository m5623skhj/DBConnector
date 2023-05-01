#pragma once
#include <sqlext.h>

namespace ODBCUtil
{
	void PrintSQLErrorMessage(SQLHSTMT stmtHandle);
	bool SQLIsSuccess(SQLRETURN returnValue);
	bool IsSameType(const std::string& lhs, const std::string& rhs);
}
