#pragma once

USING_SHARED_PTR(DBEnterTaskExecutor);
USING_SHARED_PTR(DBUpdateTaskExecutor);

USING_SHARED_PTR_IN_TWO_NAMESPACE(sw, redis, Redis)

namespace sw { namespace redis { class Redis; } }

class DBManager
{
public:
	DBManager();

	void								Init();

	const DBEnterTaskExecutorRef		GetEnterTaskExecutor();
	const DBUpdateTaskExecutorRef		GetUpdateTaskExecutor();
	const sw::redis::RedisRef			GetRedis();

private:
	const std::string					GetRedisConStr();

private:
	DBEnterTaskExecutorRef				_enterTaskExecutor;
	DBUpdateTaskExecutorRef				_updateTaskExecutor;
	sw::redis::RedisRef					_redis;
};

extern DBManager* GDBManager;
