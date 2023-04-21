#include "PreCompile.h"
#include "ODBCMetaData.h"
#include "ODBCConnector.h"
#include <sql.h>
#include <sqlext.h>
#include "ODBCConst.h"

ODBCMetaData::ODBCMetaData(const std::wstring& inCatalogName)
	: catalogName(inCatalogName)
{

}

ODBCMetaData::~ODBCMetaData()
{

}

bool ODBCMetaData::GetProcedureNameFromDB(ODBCConnector& connector, WCHAR* schemaName, OUT std::vector<std::string>& procedureNameList)
{
	auto stmtHandle = connector.GetStmtHandle();
	if (stmtHandle == nullptr)
	{
		return false;
	}

	SQLWCHAR* catalog_name = (SQLWCHAR*)L"SQL_ALL_CATALOGS";
	if (SQLProcedures(stmtHandle, (SQLWCHAR*)catalog_name, static_cast<SQLSMALLINT>(wcslen(catalog_name)), schemaName, wcslen(schemaName), NULL, NULL) != SQL_SUCCESS)
	{
		return false;
	}

	SQLCHAR procedureName[256];
	SQLRETURN ret;
	while (SQLFetch(stmtHandle) == SQL_SUCCESS)
	{
		ret = SQLGetData(stmtHandle, COLUMN_NUMBER::PROCEDURE_NAME, SQL_C_CHAR, procedureName, sizeof(procedureName), nullptr);
		if (ret != SQL_SUCCESS)
		{
			return false;
		}

		procedureNameList.emplace_back(reinterpret_cast<const char*>(procedureName));
	}

	return true;
}