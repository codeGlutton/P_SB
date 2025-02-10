#pragma once
#include "Types.h"

/**********************
	RecursiveRWLock
**********************/

class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		READ_COUNT_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0x0000'0000
	};

public:
	void			WriteLock(const char* name);
	void			WriteUnlock(const char* name);
	void			ReadLock(const char* name);
	void			ReadUnlock(const char* name);

private:
	// 상위 16비트는 Thread id, 하위 16비트는 걸린 read lock 수
	Atomic<uint32>	_lockFlag = EMPTY_FLAG; 
	// 얼마나 많은 write lock이 걸려있는지
	uint16			_writerCount = 0;
};

/********************
	LockGuards
********************/

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& _lock, const char* name) : _lock(_lock), _name(name) { _lock.ReadLock(name); }
	~ReadLockGuard() { _lock.ReadUnlock(_name); }

private:
	Lock&			_lock;
	const char*		_name;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& _lock, const char* name) : _lock(_lock), _name(name) { _lock.WriteLock(name); }
	~WriteLockGuard() { _lock.WriteUnlock(_name); }

private:
	Lock&			_lock;
	const char*		_name;
};