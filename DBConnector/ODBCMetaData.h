#pragma once

#include <map>
#include <vector>

class ODBCConnector;

class ODBCMetaData
{
public:
	explicit ODBCMetaData(const std::wstring& inCatalogName);
	~ODBCMetaData();

	ODBCMetaData(const ODBCMetaData&) = delete;
	ODBCMetaData& operator=(const ODBCMetaData&) = delete;

public:
	bool GetProcedureNameFromDB(ODBCConnector& connector, OUT std::vector<std::wstring>& );

private:
	std::wstring catalogName;
};