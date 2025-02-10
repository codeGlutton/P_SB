#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "Memory.h"
#include "DeadLockProfiler.h"
#include "GlobalQueue.h"
#include "SocketUtils.h"
#include "SendBuffer.h"
#include "JobTimer.h"
#include "DBConnectionPool.h"

ThreadManager*		GThreadManager = nullptr;
Memory*				GMemory = nullptr;

SendBufferManager*	GSendBufferManager = nullptr;
GlobalQueue*		GGlobalQueue = nullptr;
JobTimer*			GJobTimer = nullptr;

DeadLockProfiler*	GDeadLockProfiler = nullptr;
ConsoleLog*			GConsoleLogger = nullptr;

DBConnectionPool*	GDBConnectionPool = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		// Note specific manager class's generation order
		GThreadManager = new ThreadManager();
		GMemory = new Memory();
		GSendBufferManager = new SendBufferManager();
		GGlobalQueue = new GlobalQueue();
		GJobTimer = new JobTimer();
		GDeadLockProfiler = new DeadLockProfiler();
		GConsoleLogger = new ConsoleLog();
		GDBConnectionPool = new DBConnectionPool();
		SocketUtils::Init();
	}
	~CoreGlobal()
	{
		delete GThreadManager;
		delete GMemory;
		delete GSendBufferManager;
		delete GGlobalQueue;
		delete GJobTimer;
		delete GDeadLockProfiler;
		delete GConsoleLogger;
		delete GDBConnectionPool;
		SocketUtils::Clear();
	}
} GCoreGlocal;