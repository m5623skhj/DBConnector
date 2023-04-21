#include "DBConnector.h"
#include <iostream>
#include <memory>

#pragma comment(lib, "libmySQL.lib")

DBConnector::DBConnector()
	: connection(nullptr, mysql_close)
{

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
}

void DBConnector::DBStoreResult()
{
	response.reset(mysql_store_result(connection.get()));
}

void DBConnector::FreeResult()
{
	if (response == nullptr)
	{
		return;
	}

	mysql_free_result(response.get());
}

void DBConnector::FreeResult(DBResultRows* deleteRow)
{
	if (deleteRow == nullptr)
	{
		delete deleteRow;
		deleteRow = nullptr;
	}

	if (response == nullptr)
	{
		return;
	}

	mysql_free_result(response.get());
}

DBConnector::DBResultRows* DBConnector::GetDBArr()
{
	auto result = GetResult();
	if (result == nullptr)
	{
		return nullptr;
	}

	std::shared_ptr<DBResultRows> rows = std::make_shared<DBResultRows>();
	rows->rows = mysql_row_tell(response.get());

	return rows.get();
}

void DBConnector::DBArray()
{
	if (response == nullptr)
	{
		return;
	}

	MYSQL_FIELD* field = mysql_fetch_fields(response.get());
	int numOfFields = mysql_num_fields(response.get());
	for (int i = 0; i < numOfFields; ++i)
	{
		rowMap.insert({ field[i].name, i });
	}
}

const MYSQL_ROWS* DBConnector::GetResult()
{
	if (response != nullptr)
	{
		return nullptr;
	}

	return mysql_row_tell(response.get());
}
