#include "PreCompile.h"
#include "ODBCMetaData.h"
#include "ODBCConnector.h"
#include "ODBCConst.h"

#include <sql.h>
#include <sqlext.h>
#include <cstring>

ColumnInfo::ColumnInfo(const SQLTCHAR* inName, const SQLSMALLINT inType,
	const SQLINTEGER inDataType, const SQLTCHAR* inDataTypeName, const SQLULEN inColumnSize)
	: name(inName)
	, columnType(inType)
	, dataType(inDataType)
	, dataTypeName(inDataTypeName)
	, columnSize(inColumnSize)
{
}

ColumnInfo::ColumnInfo(const SQLTCHAR* inName, const SQLSMALLINT inType,
	const SQLINTEGER inDataType, std::wstring inDataTypeName, const SQLULEN inColumnSize)
	: name(inName)
	, columnType(inType)
	, dataType(inDataType)
	, dataTypeName(std::move(inDataTypeName))
	, columnSize(inColumnSize)
{
}

bool ProcedureInfo::SettingDefaultSPMaker(const SQLHSTMT stmtHandle) const
{
	int cnt = 1;
	for (const auto& inputColumn : inputColumnInfoList)
	{
		auto defaultColumn = GetDefaultValue(inputColumn.dataType);
		if (defaultColumn == nullptr)
		{
			return false;
		}

		if (not ODBCUtil::SQLIsSuccess(SQLBindParameter(stmtHandle, cnt, SQL_PARAM_INPUT, SQL_C_LONG, inputColumn.dataType, 0, 0, defaultColumn.get(), 0, nullptr)))
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		++cnt;
	}

	return true;
}

std::shared_ptr<void> ProcedureInfo::GetDefaultValue(const short dataType)
{
	if (dataType == SQL_NUMERIC || dataType == SQL_DECIMAL || dataType == SQL_INTEGER)
	{
		return std::make_shared<int>(0);
	}
	else if (dataType == SQL_BIGINT)
	{
		return std::make_shared<INT64>(0);
	}
	else if (dataType == SQL_FLOAT || dataType == SQL_REAL)
	{
		return std::make_shared<float>(0.f);
	}
	else if (dataType == SQL_WVARCHAR)
	{
		return std::make_shared<const WCHAR*>(L" ");
	}
	else if (dataType == SQL_VARCHAR)
	{
		return std::make_shared<const char*>(" ");
	}
	else if (dataType == SQL_BIT)
	{
		return std::make_shared<bool>(false);
	}

	return nullptr;
}

ODBCMetaData::ODBCMetaData(std::wstring inCatalogName)
	: catalogName(std::move(inCatalogName))
{
}

bool ODBCMetaData::GetProcedureNameFromDB(ODBCConnector& connector, WCHAR* catalogName, WCHAR* schemaName, OUT std::set<ProcedureName>& procedureNameList)
{
	const auto stmtHandle = connector.GetDefaultStmtHandle();
	if (stmtHandle == nullptr)
	{
		return false;
	}

	if (SQLProcedures(stmtHandle, catalogName, static_cast<SQLSMALLINT>(wcslen(catalogName)), schemaName, wcslen(schemaName), nullptr, NULL) != SQL_SUCCESS)
	{
		return false;
	}

	SQLCHAR procedureName[256];
	ZeroMemory(procedureName, sizeof(procedureName));

	while (SQLFetch(stmtHandle) == SQL_SUCCESS)
	{
		const SQLRETURN ret = SQLGetData(stmtHandle, PROCEDURE_NAME, SQL_C_CHAR, procedureName,
		                           sizeof(procedureName), nullptr);
		if (not ODBCUtil::SQLIsSuccess(ret))
		{
			return false;
		}

		std::string procInfo = reinterpret_cast<const char*>(procedureName);
		const auto pos = procInfo.find(';');
		std::string procName = procInfo.substr(0, pos);

		procedureNameList.insert(procName.c_str());
	}

	SQLCloseCursor(stmtHandle);

	return true;
}

bool ODBCMetaData::MakeProcedureColumnInfoFromDB(ODBCConnector& connector, const std::set<ProcedureName>& procedureNameList)
{
	auto stmtHandle = connector.GetDefaultStmtHandle();
	// auto commit mode off
	// SQLExecute()를 모두 롤백하기 위해서 설정
	auto ret = SQLSetConnectAttr(connector.GetDefaultDBCHandle(), SQL_ATTR_AUTOCOMMIT, nullptr, 0);
	if (not ODBCUtil::SQLIsSuccess(ret))
	{
		std::cout << "SQLSetConnectAttr failed : " << '\n';
		ODBCUtil::PrintSQLErrorMessage(stmtHandle);
		return false;
	}

	for (const auto& procedureName : procedureNameList)
	{
		auto procedureInfo = std::make_shared<ProcedureInfo>();
		WCHAR procedureNameBuffer[64];
		ZeroMemory(procedureNameBuffer, sizeof(procedureNameBuffer));

		if (not UTF8ToUTF16(procedureName.c_str(), procedureNameBuffer, sizeof(procedureNameBuffer)))
		{
			return false;
		}

		if (not MakeInputColumnToProcedureInfo(stmtHandle, procedureName, procedureNameBuffer, procedureInfo))
		{
			return false;
		}

		ret = SQLPrepare(stmtHandle, const_cast<SQLWCHAR*>(procedureInfo->sql.c_str()), SQL_NTS);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
		{
			std::cout << "SQLPrepare failed : " << procedureName << '\n';
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}
		procedureInfo->SettingDefaultSPMaker(stmtHandle);

		// 반드시 먼저 SQLExecute() 혹은 SQLExecDirect()를 먼저 호출해야 함
		// 호출하지 않으면, SQLNumResultCols()를 호출할 때, coulmnCount가 0을 반환함
		ret = SQLExecute(stmtHandle);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
		{
			std::cout << "SQLExecute failed : " << procedureName << '\n';
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		if (MakeOutputColumnToProcedureInfo(stmtHandle, procedureName, procedureInfo) == false)
		{
			return false;
		}

		// 실제로 호출된 sp가 적용되면 안되므로 롤백시킴
		SQLEndTran(SQL_HANDLE_DBC, connector.GetDefaultDBCHandle(), SQL_ROLLBACK);
		
		procedureInfoMap.insert({ procedureName, procedureInfo });
	}

	// auto commit mode on
	ret = SQLSetConnectAttr(connector.GetDefaultDBCHandle(), SQL_ATTR_AUTOCOMMIT, reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_ON), 0);
	if (not ODBCUtil::SQLIsSuccess(ret))
	{
		std::cout << "SQLSetStmtAttr() failed" << '\n';
		ODBCUtil::PrintSQLErrorMessage(stmtHandle);
		return false;
	}

	return true;
}

bool ODBCMetaData::MakeInputColumnToProcedureInfo(const SQLHSTMT stmtHandle, const ProcedureName& procedureName, const WCHAR* procedureNameBuffer, OUT const std::shared_ptr<ProcedureInfo>& outProcedureInfo)
{
	if (not ODBCUtil::SQLIsSuccess(SQLProcedureColumns(stmtHandle, nullptr, 0, nullptr, 0, const_cast<SQLWCHAR*>(procedureNameBuffer), SQL_NTS, nullptr, 0)))
	{
		return false;
	}

	struct InputColumnInfo
	{
		SQLTCHAR name[64];
		SQLSMALLINT columnType = 0;
		SQLINTEGER dataType = 0;
		SQLTCHAR dataTypeName[64];
		SQLULEN columnSize = 0;

		bool BindColumn(const SQLHSTMT stmtHandle)
		{
			if (SQLBindCol(stmtHandle, COLUMN_NUMBER::COLUMN_NAME, SQL_C_TCHAR, name, sizeof(name), nullptr) != SQL_SUCCESS)
			{
				return false;
			}
			if (SQLBindCol(stmtHandle, COLUMN_NUMBER::COLUMN_TYPE, SQL_C_SHORT, &columnType, sizeof(columnType), nullptr) != SQL_SUCCESS)
			{
				return false;
			}
			if (SQLBindCol(stmtHandle, COLUMN_NUMBER::DATA_TYPE, SQL_INTEGER, &dataType, sizeof(dataType), nullptr) != SQL_SUCCESS)
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
	if (not columnInfo.BindColumn(stmtHandle))
	{
		return false;
	}

	outProcedureInfo->sql = L"{CALL ";
	outProcedureInfo->sql += procedureNameBuffer;
	outProcedureInfo->sql += L"(";

	bool isFirstParam = true;
	while (true)
	{
		if (not ODBCUtil::SQLIsSuccess(SQLFetch(stmtHandle)))
		{
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			break;
		}
		
		if (columnInfo.columnType == SQL_PARAM_INPUT)
		{
			if (isFirstParam)
			{
				isFirstParam = false;
			}
			else
			{
				outProcedureInfo->sql += L", ";
			}

			outProcedureInfo->inputColumnInfoList.emplace_back(
				columnInfo.name, columnInfo.columnType, columnInfo.dataType, columnInfo.dataTypeName, columnInfo.columnSize);

			outProcedureInfo->sql += L"?";
		}
	}
	outProcedureInfo->sql += L")}";

	SQLCloseCursor(stmtHandle);

	return true;
}

bool ODBCMetaData::MakeOutputColumnToProcedureInfo(SQLHSTMT stmtHandle, const ProcedureName& procedureName, OUT const std::shared_ptr<ProcedureInfo>& procedureInfo)
{
	SQLSMALLINT columnCount = 0;
	if (not ODBCUtil::SQLIsSuccess(SQLNumResultCols(stmtHandle, &columnCount)))
	{
		std::cout << "SQLNumResultCols failed : " << procedureName << '\n';
		ODBCUtil::PrintSQLErrorMessage(stmtHandle);
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
		if (ODBCUtil::SQLIsSuccess(SQLDescribeCol(stmtHandle, static_cast<SQLUSMALLINT>(i)
			, resultColumn.name
			, _countof(resultColumn.name)
			, &resultColumn.nameLength
			, &resultColumn.dataType
			, &resultColumn.columnSize
			, &resultColumn.decimalDigits
			, &resultColumn.nullable)) == false)
		{
			std::cout << "SQLDescribeCol failed : " << procedureName << ", " << i << '\n';
			ODBCUtil::PrintSQLErrorMessage(stmtHandle);
			return false;
		}

		procedureInfo->resultColumnInfoList.emplace_back(
			resultColumn.name, resultColumn.dataType, resultColumn.dataType, ODBCUtil::GetDataTypeName(resultColumn.dataType), resultColumn.columnSize);
	}
	SQLCloseCursor(stmtHandle);

	return true;
}

const ProcedureInfo* ODBCMetaData::GetProcedureInfo(const ProcedureName& procedureName) const
{
	const auto it = procedureInfoMap.find(procedureName);
	if (it == procedureInfoMap.end())
	{
		return nullptr;
	}

	return it->second.get();
}
