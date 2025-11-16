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
	ColumnInfo(const SQLTCHAR* inName, SQLSMALLINT inType,
		SQLINTEGER inDataType, const SQLTCHAR* inDataTypeName, SQLULEN inColumnSize);
	ColumnInfo(const SQLTCHAR* inName, SQLSMALLINT inType,
		SQLINTEGER inDataType, std::wstring inDataTypeName, SQLULEN inColumnSize);

	std::wstring name;
	short columnType = 0;
	int dataType = 0;
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

	bool SettingDefaultSPMaker(SQLHSTMT stmtHandle) const;

	bool SettingSPMaker(SQLHSTMT stmtHandle) const
	{
		return true;
	}

	template <typename T>
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const T& input) const
	{
		int index = parameterLocation - 1;
		if (inputColumnInfoList.size() < index)
		{
			return false;
		}

		return ODBCUtil::SettingSPMaker(stmtHandle, parameterLocation
			, inputColumnInfoList[index].dataType, inputColumnInfoList[index].columnType, input);
	}

	template <typename T, typename... Args>
	bool SettingSPMaker(SQLHSTMT stmtHandle, int parameterLocation, const T& input, Args&... args) const
	{
		if (SettingSPMaker(stmtHandle, parameterLocation, input) == false)
		{
			return false;
		}

		int nextParameterLocation = parameterLocation + 1;
		return SettingSPMaker(stmtHandle, nextParameterLocation, args...);
	}

private:
	static std::shared_ptr<void> GetDefaultValue(short dataType);
};

class ODBCMetaData
{
public:
	explicit ODBCMetaData(std::wstring inCatalogName);
	~ODBCMetaData() = default;

	ODBCMetaData(const ODBCMetaData&) = delete;
	ODBCMetaData& operator=(const ODBCMetaData&) = delete;

public:
	static bool GetProcedureNameFromDB(ODBCConnector& connector, WCHAR* catalogName, WCHAR* schemaName, OUT std::set<ProcedureName>& procedureNameList);
	bool MakeProcedureColumnInfoFromDB(ODBCConnector& connector, const std::set<ProcedureName>& procedureNameList);

private:
	static bool MakeInputColumnToProcedureInfo(SQLHSTMT stmtHandle, const ProcedureName& procedureName, const WCHAR* procedureNameBuffer, OUT const std::shared_ptr<ProcedureInfo>& outProcedureInfo);
	static bool MakeOutputColumnToProcedureInfo(SQLHSTMT stmtHandle, const ProcedureName& procedureName, OUT const std::shared_ptr<ProcedureInfo>& procedureInfo);

public:
	const ProcedureInfo* GetProcedureInfo(const ProcedureName& procedureName) const;

private:
	std::wstring catalogName;
	std::map<ProcedureName, std::shared_ptr<ProcedureInfo>> procedureInfoMap;
};
