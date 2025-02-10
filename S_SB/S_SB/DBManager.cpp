#include "pch.h"
#include "DBManager.h"
#include "DBTaskExecutor.h"

DBManager* GDBManager = nullptr;

DBManager::DBManager()
{
	_playerTaskExecutor = MakeXShared<DBPlayerTaskExecutor>();
	_achvTaskExecutor = MakeXShared<DBAchvTaskExecutor>();
}
