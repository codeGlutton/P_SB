#include "MemoryPool.h"
#include "C_SB.h"

#if defined(__GNUC__) || defined(__clang__)		// 리눅스 기반

/******************
	MemoryPool
******************/

MemoryPool::MemoryPool(int32 AllocSize) : _AllocSize(AllocSize)
{
	SLIST_INIT(&_Header);
}

MemoryPool::~MemoryPool()
{
	while (!SLIST_EMPTY(&_Header)) 
	{
		MemoryHeader* Memory = SLIST_FIRST(&_Header);
		SLIST_REMOVE_HEAD(&_Header, Entries);
		::_aligned_free(Memory);
	}
}

void MemoryPool::Push(MemoryHeader* Ptr)
{
	Ptr->AllocSize = 0;

	SLIST_INSERT_HEAD(&_Header, Ptr, Entries);

	_UseCount.fetch_sub(1);
	_ReserveCount.fetch_add(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* Memory = nullptr;
	if (SLIST_EMPTY(&_Header))
	{
		Memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(_AllocSize, SLIST_ALIGNMENT));
	}
	else
	{
		Memory = SLIST_FIRST(&_Header);
		SLIST_REMOVE_HEAD(&_Header, Entries);

		check(Memory->AllocSize == 0);
		_ReserveCount.fetch_sub(1);
	}

	_UseCount.fetch_add(1);

	return Memory;
}

#elif defined(_MSC_VER) 						// 윈도우 기반

/******************
	MemoryPool
******************/

MemoryPool::MemoryPool(int32 AllocSize) : _AllocSize(AllocSize)
{
	::InitializeSListHead(&_Header);
}

MemoryPool::~MemoryPool()
{
	while (MemoryHeader* Memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_Header)))
		::_aligned_free(Memory);
}

void MemoryPool::Push(MemoryHeader* Ptr)
{
	Ptr->AllocSize = 0;

	::InterlockedPushEntrySList(&_Header, static_cast<PSLIST_ENTRY>(Ptr));

	_UseCount.fetch_sub(1);
	_ReserveCount.fetch_add(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* Memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_Header));

	if (Memory == nullptr)
	{
		Memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(_AllocSize, SLIST_ALIGNMENT));
	}
	else
	{
		// 기존 쓰던 동적 변수 사이즈 0으로 정상 반납 됬는가 체크
		check(Memory->AllocSize == 0);
		_ReserveCount.fetch_sub(1);
	}

	_UseCount.fetch_add(1);

	return Memory;
}

#else

#endif