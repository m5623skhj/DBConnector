#include "PreCompile.h"
#include "ODBCMetaData.h"
#include "ODBCConnector.h"
#include "ODBCConst.h"

#include <sql.h>
#include <sqlext.h>
#include <cstring>

ColumnInfo::ColumnInfo(SQLTCHAR* inName, SQLSMALLINT inType,
	SQLSMALLINT inDataType, SQLTCHAR* inDataTypeName, SQLULEN inColumnSize)
	: name(reinterpret_cast<WCHAR*>(inName))
	, type(inType)
	, dataType(inDataType)
	, dataTypeName(reinterpret_cast<WCHAR*>(inDataTypeName))
	, columnSize(inColumnSize)
{
}

ColumnInfo::ColumnInfo(SQLTCHAR* inName, SQLSMALLINT inType,
	SQLSMALLINT inDataType, std::wstring inDataTypeName, SQLULEN inColumnSize)
	: name(reinterpret_cast<WCHAR*>(inName))
	, type(inType)
	, dataType(inDataType)
	, dataTypeName(inDataTypeName)
	, columnSize(inColumnSize)
{
}

ODBCMetaData::ODBCMetaData(const std::wstring& inCatalogName)
	: catalogName(inCatalogName)
{
}

bool ODBCMetaData::GetProcedureNameFromDB(ODBCConnector& connector, WCHAR* schemaName, OUT std::set<std::string>& procedureNameList)
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
	ZeroMemory(procedureName, sizeof(procedureName));

	SQLRETURN ret;
	while (SQLFetch(stmtHandle) == SQL_SUCCESS)
	{
		ret = SQLGetData(stmtHandle, COLUMN_NUMBER::PROCEDURE_NAME, SQL_C_CHAR, procedureName, sizeof(procedureName), nullptr);
		if (ret != SQL_SUCCESS)
		{
			return false;
		}

		procedureNameList.insert(reinterpret_cast<const char*>(procedureName));
	}

	SQLCloseCursor(stmtHandle);

	return true;
}

bool ODBCMetaData::MakeProcedureColumnInfoFromDB(ODBCConnector& connector, const std::set<std::string>& procedureNameList)
{
	auto stmtHandle = connector.GetStmtHandle();

	for (const auto& procedureName : procedureNameList)
	{
		auto procedureInfo = std::make_shared<ProcedureInfo>();
		WCHAR procedureNameBuffer[64];
		ZeroMemory(procedureNameBuffer, sizeof(procedureNameBuffer));

		if (UTF8ToUTF16(procedureName.c_str(), procedureNameBuffer, sizeof(procedureNameBuffer)) == false)
		{
			return false;
		}

		if (MakeInputColumnToProcedureInfo(stmtHandle, procedureName, procedureNameBuffer, procedureInfo) == false)
		{
			return false;
		}

		auto ret = SQLPrepare(stmtHandle, (SQLWCHAR*)procedureInfo->sql.c_str(), SQL_NTS);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
		{
			std::cout << "SQLPrepare failed : " << procedureName << std::endl;
			return false;
		}

		if (MakeOutputColumnToProcedureInfo(stmtHandle, procedureName, procedureInfo) == false)
		{
			return false;
		}

		procedureInfoMap.insert({ procedureName, procedureInfo });
	}

	return true;
}

bool ODBCMetaData::MakeInputColumnToProcedureInfo(SQLHSTMT stmtHandle, const std::string& procedureName, const WCHAR* procedureNameBuffer, OUT std::shared_ptr<ProcedureInfo> outProcdureInfo)
{
	auto ret = SQLProcedureColumns(stmtHandle, NULL, 0, NULL, 0, (SQLWCHAR*)procedureNameBuffer, SQL_NTS, NULL, 0);
	if (ret != SQL_SUCCESS)
	{
		return false;
	}

	struct InputColumnInfo
	{
		SQLTCHAR name[64];
		SQLSMALLINT type = 0;
		SQLSMALLINT dataType = 0;
		SQLTCHAR dataTypeName[64];
		SQLULEN columnSize = 0;

		bool BindColumn(SQLHSTMT stmtHandle)
		{
			if (SQLBindCol(stmtHandle, COLUMN_NUMBER::COLUMN_NAME, SQL_C_TCHAR, name, sizeof(name), nullptr) != SQL_SUCCESS)
			{
				return false;
			}
			if (SQLBindCol(stmtHandle, COLUMN_NUMBER::COLUMN_TYPE, SQL_C_SHORT, &type, sizeof(type), nullptr) != SQL_SUCCESS)
			{
				return false;
			}
			if (SQLBindCol(stmtHandle, COLUMN_NUMBER::DATA_TYPE, SQL_C_SHORT, &dataType, sizeof(dataType), nullptr) != SQL_SUCCESS)
			{
				return false;
			}
			if (SQLBindCol(stmtHandle, COLUMN_NUMBER::DATA_TYPE_NAME, SQL_C_TCHAR, dataTypeName, sizeof(dataTypeName), nullptr) != SQL_SUCCESS)
			{
				return false;
			}
			if (SQLBindCol(stmtHandle, COLUMN_NUMBER::COLUMN_SIZE, SQL_C_LONG, &columnSize, sizeof(columnSize), nullptr) != SQL_SUCCESS)
			{
				return false;
			}

			return true;
		}
	};

	InputColumnInfo columnInfo;
	ZeroMemory(&columnInfo, sizeof(columnInfo));
	if (columnInfo.BindColumn(stmtHandle) == false)
	{
		return false;
	}

	outProcdureInfo->sql = L"{CALL ";
	outProcdureInfo->sql += procedureNameBuffer;
	outProcdureInfo->sql += L"(";

	bool isFirstParam = true;
	while (true)
	{
		if (SQLFetch(stmtHandle) != SQL_SUCCESS)
		{
			break;
		}
		
		if (isFirstParam)
		{
			isFirstParam = false;
		}
		else
		{
			outProcdureInfo->sql += L", ";
		}

		if (columnInfo.type == SQL_PARAM_INPUT)
		{
			outProcdureInfo->inputColumnInfoList.emplace_back(ColumnInfo(
				columnInfo.name, columnInfo.type, columnInfo.dataType, columnInfo.dataTypeName, columnInfo.columnSize));

			outProcdureInfo->sql += L"?";
		}
	}
	outProcdureInfo->sql += L")}";

	SQLCloseCursor(stmtHandle);

	return true;
}

bool ODBCMetaData::MakeOutputColumnToProcedureInfo(SQLHSTMT stmtHandle, const std::string& procedureName, OUT std::shared_ptr<ProcedureInfo> procdureInfo)
{
	SQLSMALLINT columnCount = 0;
	SQLRETURN ret = SQLNumResultCols(stmtHandle, &columnCount);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		std::cout << "SQLNumResultCols failed : " << procedureName << std::endl;
		return false;
	}

	struct ResultColumnInfo
	{
		SQLWCHAR name[64];
		SQLSMALLINT nameLength = 0;
		SQLSMALLINT dataType = 0;
		SQLULEN columnSize = 0;
		SQLSMALLINT decimalDigits = 0;
		SQLSMALLINT nullable = 0;
	};

	for (int i = 1; i <= columnCount; ++i)
	{
		ResultColumnInfo resultColumn;
		if (SQLDescribeCol(stmtHandle, static_cast<SQLUSMALLINT>(i)
			, resultColumn.name
			, _countof(resultColumn.name)
			, &resultColumn.nameLength
			, &resultColumn.dataType
			, &resultColumn.columnSize
			, &resultColumn.decimalDigits
			, &resultColumn.nullable) != SQL_SUCCESS)
		{
			std::cout << "SQLDescribeCol failed : " << procedureName << ", " << i << std::endl;
			return false;
		}

		procdureInfo->resultColumnInfoList.emplace_back(ColumnInfo(
			resultColumn.name, resultColumn.dataType, resultColumn.dataType, GetDataTypeName(resultColumn.dataType), resultColumn.columnSize));
	}
	SQLCloseCursor(stmtHandle);

	return true;
}

std::wstring GetDataTypeName(SQLSMALLINT inDataType)
{
	switch (inDataType)
	{
	case SQL_BIGINT:
		return L"int64";
	case SQL_NUMERIC:
	case SQL_DECIMAL:
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
