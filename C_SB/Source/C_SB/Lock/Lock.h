#pragma once
#include "Types.h"

/**********************
    RecursiveRWLock
**********************/

class RecursiveRWLock
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
	void							WriteLock();
	void							WriteUnlock();
	void							ReadLock();
	void							ReadUnlock();

private:
	// 상위 16비트는 Thread id, 하위 16비트는 걸린 read lock 수
	Atomic<uint32>					_LockFlag = EMPTY_FLAG; 
	// 얼마나 많은 write lock이 걸려있는지
	uint16							_WriterCount = 0;
};

/**********************
	  LockGuards
**********************/

class ReadLockGuard
{
public:
	ReadLockGuard(RecursiveRWLock& RWLock) : _Lock(RWLock) { RWLock.ReadLock(); }
	~ReadLockGuard() { _Lock.ReadUnlock(); }

private:
	RecursiveRWLock&				_Lock;
};

class WriteLockGuard
{
public:
	WriteLockGuard(RecursiveRWLock& RWLock) : _Lock(RWLock) { RWLock.WriteLock(); }
	~WriteLockGuard() { _Lock.WriteUnlock(); }

private:
	RecursiveRWLock&				_Lock;
};