#pragma once

#include <set>
#include <map>
#include <vector>
#include <sqltypes.h>
#include <memory>
#include "ODBCUtil.h"

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

	bool SettingSPMaker(SQLHSTMT stmtHandle) const
	{
		return true;
	}

	template <typename T>
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const T& input) const
	{
		return ODBCUtil::SettingSPMaker(stmtHandle, parameterLocation, input);
	}

	template <typename T, typename... Args>
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const T& input, Args... args) const
	{
		if (SettingSPMaker(stmtHandle, parameterLocation, input) == false)
		{
			return false;
		}

		int nextParameterLocation = parameterLocation + 1;
		return SettingSPMaker(stmtHandle, nextParameterLocation, args...);
	}

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
	bool GetProcedureNameFromDB(ODBCConnector& connector, WCHAR* schemaName, OUT std::set<ProcedureName>& procedureNameList);
	bool MakeProcedureColumnInfoFromDB(ODBCConnector& connector, const std::set<ProcedureName>& procedureNameList);

private:
	bool MakeInputColumnToProcedureInfo(SQLHSTMT stmtHandle, const ProcedureName& procedureName, const WCHAR* procedureNameBuffer, OUT std::shared_ptr<ProcedureInfo> outProcdureInfo);
	bool MakeOutputColumnToProcedureInfo(SQLHSTMT stmtHandle, const ProcedureName& procedureName, OUT std::shared_ptr<ProcedureInfo> procdureInfo);

public:
	const ProcedureInfo * const GetProcedureInfo(ProcedureName procedureName) const;

private:
	std::wstring catalogName;
	std::map<ProcedureName, std::shared_ptr<ProcedureInfo>> procedureInfoMap;
};

std::wstring GetDataTypeName(SQLSMALLINT inDataType);
