#include "pch.h"
#include "DBManager.h"
#include "DBTaskExecutor.h"
#include <sw/redis++/redis++.h>

DBManager* GDBManager = nullptr;

DBManager::DBManager()
{
	_enterTaskExecutor = MakeXShared<DBEnterTaskExecutor>();
	_updateTaskExecutor = MakeXShared<DBUpdateTaskExecutor>();
	_redis = MakeXShared<sw::redis::Redis>(GetRedisConStr());
}

void DBManager::Init()
{
}

const DBEnterTaskExecutorRef DBManager::GetEnterTaskExecutor()
{
	return _enterTaskExecutor;
}

const DBUpdateTaskExecutorRef DBManager::GetUpdateTaskExecutor()
{
	return _updateTaskExecutor;
}

const sw::redis::RedisRef DBManager::GetRedis()
{
	return _redis;
}

const std::string DBManager::GetRedisConStr()
{
	std::string redisConStr;
	char buffer[256];
	DWORD size = GetEnvironmentVariableA("RedisConnectionStrings", buffer, sizeof(buffer));

	if (size > 0) 
	{
		redisConStr = buffer;
	}
	else 
	{
		CRASH("Redis Env val is empty");
	}

	return redisConStr;
}
