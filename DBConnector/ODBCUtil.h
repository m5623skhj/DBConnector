#pragma once
#include <sqlext.h>

namespace ODBCUtil
{
	void PrintSQLErrorMessage(SQLHSTMT stmtHandle);
	bool SQLIsSuccess(SQLRETURN returnValue);
	bool IsSameType(const std::string& lhs, const std::string& rhs);

	std::wstring GetDataTypeName(SQLSMALLINT inDataType);

	template <typename T>
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, SQLSMALLINT dataType, const T& input)
	{
		if (ODBCUtil::SQLIsSuccess(SQLBindParameter(stmtHandle, parameterLocation, SQL_PARAM_INPUT, SQL_C_SLONG, dataType
			, 0, 0, (SQLPOINTER)(&input), 0, NULL)) == false)
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		return true;
	}
}
