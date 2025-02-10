#pragma once
#include "DBConnection.h"

class DBConnectionPool
{
public:
	DBConnectionPool();
	~DBConnectionPool();

	bool					Connect(int32 connectionCount, const WCHAR* connectionString);
	void					Clear();

	DBConnection*			Pop();
	void					Push(DBConnection* connection);

private:
	USE_LOCK;

	// 공용 환경 핸들
	SQLHENV					_environment = SQL_NULL_HANDLE;
	xVector<DBConnection*>	_connections;
};

