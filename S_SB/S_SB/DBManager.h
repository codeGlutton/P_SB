#pragma once

USING_SHARED_PTR(DBPlayerTaskExecutor);
USING_SHARED_PTR(DBAchvTaskExecutor);

USING_SHARED_PTR_IN_TWO_NAMESPACE(sw, redis, Redis)

namespace sw { namespace redis { class Redis; } }

class DBManager
{
public:
	DBManager();

	void								Init();

	const DBPlayerTaskExecutorRef		GetPlayerTaskExecutor();
	const DBAchvTaskExecutorRef			GetAchvTaskExecutor();
	const sw::redis::RedisRef			GetRedis();

private:
	const std::string					GetRedisConStr();

private:
	DBPlayerTaskExecutorRef				_playerTaskExecutor;
	DBAchvTaskExecutorRef				_achvTaskExecutor;
	sw::redis::RedisRef					_redis;
};

extern DBManager* GDBManager;
