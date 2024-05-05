#include "mysql-api.h"

//init descriptor
MySQL_API::MySQL_API(const char* dbName, const char* dbHost, const char* dbAdmin, const char* dbPass) : 
	_result(),
	_row(), 
	_dbName(dbName),
	_dbHost(dbHost),
	_dbAdmin(dbAdmin),
	_dbPass(dbPass),
	_query()
{
	mysql_init(&_mysql);
	if (&_mysql == NULL)
	{
		std::cerr << "Error: can't creat MySQL - descriptor" << std::endl;
		return;
	}
	mysql_set_character_set(&_mysql, "utf8mb4"); // utf8mb3 do not store emojies
	connectDatabase();
}

void MySQL_API::connectDatabase()
{
	if (mysql_real_connect(&_mysql, _dbHost, _dbAdmin, _dbPass, NULL, 0, NULL, 0))
	{
		_query = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = \'" + std::string(_dbName) + "\'";
		mysql_query(&_mysql, _query.c_str());
		_query.clear();
		_result = mysql_store_result(&_mysql);
		if (!(_row = mysql_fetch_row(_result)))
		{
			_query = "CREATE DATABASE IF NOT EXISTS " + std::string(_dbName);
			mysql_query(&_mysql, _query.c_str());
			_query.clear();
			std::cout << "Database was created" << std::endl;

			_query = "USE " + std::string(_dbName);
			mysql_query(&_mysql, _query.c_str());
			_query.clear();

			_query = "CREATE TABLE accounts (login VARCHAR(20), password VARCHAR(20), CHECK (login <> '' AND password <> ''))";
			mysql_query(&_mysql, _query.c_str());
			_query.clear();
			std::cout << "Table \'accounts\' was created" << std::endl;
		}
		else
		{
			_query = "USE " + std::string(_dbName);
			mysql_query(&_mysql, _query.c_str());
			_query.clear();
		}
		mysql_free_result(_result);
	}
	else
	{
		std::cerr << "Connect to database failed!" << std::endl;
		return;
	}
}

bool MySQL_API::promptToDB(const std::string& prompt)
{
	if (mysql_query(&_mysql, prompt.c_str()) == 0)
	{
		return true;
	}
	else
	{
		std::cerr << "Error executing query: " << mysql_error(&_mysql) << std::endl;
		return false;
	}
}

bool MySQL_API::selectFromDB(const std::string& select)
{
	//making query
	if (mysql_query(&_mysql, select.c_str()) != 0)
	{
		std::cerr << "Error executing query: " << mysql_error(&_mysql) << std::endl;
		return false;
	}

	//storing result
	_result = mysql_store_result(&_mysql);
	if (!_result)
	{
		std::cerr << "Error storing result: " << mysql_error(&_mysql) << std::endl;
		return false;
	}

	//handle query
	if (_row = mysql_fetch_row(_result))
	{
		mysql_free_result(_result);
		return true;
	}
	else
	{
		mysql_free_result(_result);
		return false;
	}
}

//destructor
MySQL_API::~MySQL_API()
{
	mysql_close(&_mysql);
}