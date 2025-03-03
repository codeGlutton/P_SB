#include "pch.h"
#include "DBManager.h"
#include "DBTaskExecutor.h"
#include <sw/redis++/redis++.h>

DBManager* GDBManager = nullptr;

DBManager::DBManager()
{
	_playerTaskExecutor = MakeXShared<DBPlayerTaskExecutor>();
	_achvTaskExecutor = MakeXShared<DBAchvTaskExecutor>();
	_redis = MakeXShared<sw::redis::Redis>(GetRedisConStr());
}

void DBManager::Init()
{
}

const DBPlayerTaskExecutorRef DBManager::GetPlayerTaskExecutor()
{
	return _playerTaskExecutor;
}

const DBAchvTaskExecutorRef DBManager::GetAchvTaskExecutor()
{
	return _achvTaskExecutor;
}

const sw::redis::RedisRef DBManager::GetRedis()
{
	return _redis;
}

const std::string DBManager::GetRedisConStr()
{
	std::string redisConStr;
	char buffer[32767];
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
