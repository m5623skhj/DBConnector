#pragma once

#include <set>
#include <map>
#include <vector>
#include <sqltypes.h>
#include <memory>

class ODBCConnector;

struct ColumnInfo
{
	ColumnInfo() = default;
	ColumnInfo(SQLTCHAR* inName, SQLSMALLINT inType,
		SQLSMALLINT inDataType, SQLTCHAR* inDataTypeName, SQLULEN inColumnSize);
	ColumnInfo(SQLTCHAR* inName, SQLSMALLINT inType,
		SQLSMALLINT inDataType, std::wstring inDataTypeName, SQLULEN inColumnSize);

	std::wstring name;
	short type = 0;
	short dataType = 0;
	std::wstring dataTypeName;
	UINT64 columnSize = 0;
};

struct ProcedureInfo
{
	ProcedureInfo() = default;
	~ProcedureInfo() = default;

	std::vector<ColumnInfo> inputColumnInfoList;
	std::vector<ColumnInfo> resultColumnInfoList;

	std::wstring sql;

	bool SettingDefaultSPMaker(SQLHSTMT stmtHandle);

private:
	std::shared_ptr<void> GetDefaultValue(short dataType);
};

class ODBCMetaData
{
public:
	explicit ODBCMetaData(const std::wstring& inCatalogName);
	~ODBCMetaData() = default;

	ODBCMetaData(const ODBCMetaData&) = delete;
	ODBCMetaData& operator=(const ODBCMetaData&) = delete;

public:
	bool GetProcedureNameFromDB(ODBCConnector& connector, WCHAR* schemaName, OUT std::set<std::string>& procedureNameList);
	bool MakeProcedureColumnInfoFromDB(ODBCConnector& connector, const std::set<std::string>& procedureNameList);

private:
	bool MakeInputColumnToProcedureInfo(SQLHSTMT stmtHandle, const std::string& procedureName, const WCHAR* procedureNameBuffer, OUT std::shared_ptr<ProcedureInfo> outProcdureInfo);
	bool MakeOutputColumnToProcedureInfo(SQLHSTMT stmtHandle, const std::string& procedureName, OUT std::shared_ptr<ProcedureInfo> procdureInfo);

private:
	std::wstring catalogName;
	std::map<std::string, std::shared_ptr<ProcedureInfo>> procedureInfoMap;
};

std::wstring GetDataTypeName(SQLSMALLINT inDataType);
