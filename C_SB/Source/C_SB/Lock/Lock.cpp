#include "Lock.h"
#include "C_SB.h"
#include "HAL/PlatformProcess.h"

/**********************
	RecursiveRWLock
**********************/

void RecursiveRWLock::WriteLock()
{
	const uint32 LThreadId = FPlatformTLS::GetCurrentThreadId();
	const uint32 LockThreadId = (_LockFlag.load() & WRITE_THREAD_MASK) >> 16;
	// 이미 write lock을 가진 스레드일 걍우 바로 통과
	if (LThreadId == LockThreadId) 
	{
		_WriterCount++;
		return;
	}

	const int64 BeginTick = GetTickCount64();
	const uint32 Desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	while (true)
	{
		for (uint32 SpinCount = 0; SpinCount < MAX_SPIN_COUNT; SpinCount++)
		{
			uint32 Expected = EMPTY_FLAG;
			if (_LockFlag.compare_exchange_strong(OUT Expected, Desired))
			{
				_WriterCount++;
				return;
			}
		}
		if (::GetTickCount64() - BeginTick >= ACQUIRE_TIMEOUT_TICK)
			check("LOCK_TIMEOUT");

		FPlatformProcess::YieldThread();
	}
}

void RecursiveRWLock::WriteUnlock()
{
	// 읽기 카운터 0일 때, 오류 방지
	if ((_LockFlag.load() & READ_COUNT_MASK) != 0)
		check("INVALID_UNLOCK_ORDER");

	const int32 LockCount = --_WriterCount;
	if (LockCount == 0)
		_LockFlag.store(EMPTY_FLAG);
}

void RecursiveRWLock::ReadLock()
{
	const uint32 LThreadId = FPlatformTLS::GetCurrentThreadId();
	const uint32 lockThreadId = (_LockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId) 
	{
		_LockFlag.fetch_add(1); // 비록 소유하고 있는 스레드일지라도, 여러 스레드 접근 주의
		return;
	}

	const int64 BeginTick = GetTickCount64();
	while (true)
	{
		for (uint32 SpinCount = 0; SpinCount < MAX_SPIN_COUNT; SpinCount++)
		{
			uint32 Expected = (_LockFlag.load() & READ_COUNT_MASK);
			// Threads fight each other for adding 1 to read lock count
			if (_LockFlag.compare_exchange_strong(OUT Expected, Expected + 1))
				return;
		}

		if (::GetTickCount64() - BeginTick >= ACQUIRE_TIMEOUT_TICK)
			check("LOCK_TIMEOUT");

		FPlatformProcess::YieldThread();
	}
}

void RecursiveRWLock::ReadUnlock()
{
	// 읽기 카운터 0일 때, 오류 방지
	if ((_LockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		check("MULTIPLE_UNLOCK");
}