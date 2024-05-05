#pragma once
#include <iostream>
#include "mysql.h"

class MySQL_API
{
public:
	MySQL_API(const char* dbName, const char* dbHost, const char* dbAdmin, const char* dbPass);
	~MySQL_API();

	bool promptToDB(const std::string& prompt);
	bool selectFromDB(const std::string& select);

private:
	MYSQL _mysql;
	MYSQL_RES* _result;
	MYSQL_ROW _row;

	const char* _dbName;
	const char* _dbHost;
	const char* _dbAdmin;
	const char* _dbPass;

	std::string _query;

	void connectDatabase();
};