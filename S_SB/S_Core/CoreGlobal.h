#pragma once

/* 베이스 전역 변수 */

extern class ThreadManager*		GThreadManager;
extern class Memory*			GMemory;

/* 작업 처리 */

extern class SendBufferManager*	GSendBufferManager;
extern class GlobalQueue*		GGlobalQueue;
extern class JobTimer*			GJobTimer;

/* 디버깅 */

extern class DeadLockProfiler*	GDeadLockProfiler; 
extern class ConsoleLog*		GConsoleLogger;

/* DB */

extern class DBConnectionPool*	GDBConnectionPool;
