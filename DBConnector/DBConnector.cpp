#include "DBConnector.h"
#include <iostream>
#include <memory>

#pragma comment(lib, "libmySQL.lib")

DBConnector::DBConnector()
	: connection(nullptr, mysql_close)
{
}

DBConnector::~DBConnector()
{
	connection.release();
}

bool DBConnector::ConnectDB()
{
	connection = std::unique_ptr<MYSQL, decltype(&mysql_close)>(mysql_init(nullptr), mysql_close);
	if (!mysql_real_connect(connection.get(), "127.0.0.1", "root", "1234", "testschema", 3306, NULL, 0))
	{
		std::cout << mysql_error(connection.get());
		return false;
	}

	return true;
}

void DBConnector::CloseDB()
{
	mysql_close(connection.get());
}

void DBConnector::DBSendQuery(std::string query)
{
	if (query.empty() == true)
	{
		// log - empty query
		return;
	}

	if(mysql_query(connection.get(), query.c_str()) != 0)
	{
		std::cout << mysql_error(connection.get());
	}
	
	DBStoreResult();
}

void DBConnector::DBStoreResult()
{
	response = mysql_store_result(connection.get());
	MakeResultRows();
}

void DBConnector::FreeResult()
{
	if (response == nullptr)
	{
		return;
	}

	rows.Clear();
	mysql_free_result(response);
}

const DBConnector::DBResultRows& DBConnector::GetRow()
{
	return rows;
}

void DBConnector::MakeResultRows()
{
	if (response == nullptr)
	{
		return;
	}

	MYSQL_FIELD* field = mysql_fetch_fields(response);
	int numOfFields = mysql_num_fields(response);
	for (int i = 0; i < numOfFields; ++i)
	{
		rows.columnIndexMap.insert({ field[i].name, i });
	}

	rows.resultRow = GetResults();
}

MYSQL_ROWS* DBConnector::GetResults()
{
	if (response == nullptr)
	{
		return nullptr;
	}

	return mysql_row_tell(response);
}
