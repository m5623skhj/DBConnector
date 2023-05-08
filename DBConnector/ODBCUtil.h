#pragma once
#include <sqlext.h>

namespace ODBCUtil
{
	void PrintSQLErrorMessage(SQLHSTMT stmtHandle);
	bool SQLIsSuccess(SQLRETURN returnValue);
	bool IsSameType(const std::string& lhs, const std::string& rhs);

	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const std::string& input);
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const int& input);
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const INT64& input);
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const float& input);
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const double& input);
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const bool& input);
}
