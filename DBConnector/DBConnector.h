#pragma once
#include <string>
#include <mysql.h>
#include <map>
#include <memory>

class DBConnector
{
public:
	class DBResultRows
	{
	public:
		const char* operator[](const std::string& columnName)
		{
			if (resultRow == nullptr)
			{
				return nullptr;
			}

			auto findIter = columnIndexMap.find(columnName);
			if (findIter == columnIndexMap.end())
			{
				return nullptr;
			}

			return resultRow->data[findIter->second];
		}

		void Clear()
		{
			resultRow = nullptr;
			columnIndexMap.clear();
		}

		void NextRow()
		{
			resultRow = resultRow->next;
		}

		const MYSQL_ROWS* GetRow()
		{
			return resultRow;
		}

		bool IsEndRow()
		{
			return resultRow == nullptr;
		}

	private:
		friend class DBConnector;

	private:
		MYSQL_ROWS* resultRow;
		std::map<std::string, int> columnIndexMap;
	};

public:
	DBConnector();
	~DBConnector();

	DBConnector(const DBConnector&) = delete;
	DBConnector& operator=(const DBConnector&) = delete;

public:
	bool ConnectDB();
	void CloseDB();
	
	void DBSendQuery(std::string query);
	void FreeResult();

	const DBResultRows& GetRow();

private:
	void DBStoreResult();
	void MakeResultRows();

	MYSQL_ROWS* GetResults();

private :
	std::unique_ptr<MYSQL, decltype(&mysql_close)> connection;
	MYSQL_RES* response;
	DBResultRows rows;
};