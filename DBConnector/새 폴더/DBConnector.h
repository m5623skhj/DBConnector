#pragma once
#include <string>
#include <mysql.h>
#include <map>
#include <memory>

class DBConnector
{
public:
	struct DBResultRows
	{
		const char* operator[](const std::string& columnName)
		{
			auto findIter = itemMap.find(columnName);
			if (findIter == itemMap.end())
			{
				return nullptr;
			}

			if (rows == nullptr)
			{
				return nullptr;
			}

			return rows->data[findIter->second];
		}

		MYSQL_ROWS* rows;
		std::map<std::string, int> itemMap;
	};

public:
	DBConnector();
	~DBConnector() = default;

	DBConnector(const DBConnector&) = delete;
	DBConnector& operator=(const DBConnector&) = delete;

public:
	bool ConnectDB();
	void CloseDB();
	
	void DBSendQuery(std::string query);
	void DBStoreResult();
	void FreeResult();
	void FreeResult(DBResultRows* deleteRow);

	DBResultRows* GetDBArr();
	void DBArray();

public:
	const MYSQL_ROWS* GetResult();

private :
	std::unique_ptr<MYSQL, decltype(&mysql_close)> connection;
	std::shared_ptr<MYSQL_RES> response;
	std::map<std::string, int> rowMap;
};