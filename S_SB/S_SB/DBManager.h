#pragma once

USING_SHARED_PTR(DBPlayerTaskExecutor);
USING_SHARED_PTR(DBAchvTaskExecutor);

class DBManager
{
public:
	DBManager();

	const DBPlayerTaskExecutorRef		GetPlayerTaskExecutor() { return _playerTaskExecutor; }
	const DBAchvTaskExecutorRef			GetAchvTaskExecutor() { return _achvTaskExecutor; }

private:
	DBPlayerTaskExecutorRef				_playerTaskExecutor;
	DBAchvTaskExecutorRef				_achvTaskExecutor;
};

extern DBManager* GDBManager;
