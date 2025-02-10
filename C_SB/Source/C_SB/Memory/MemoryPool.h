#pragma once

#include "CoreMinimal.h"

enum
{
	SLIST_ALIGNMENT = 16
};

#if defined(__GNUC__) || defined(__clang__)		// 리눅스 기반
#define ALIGN(x) __attribute__ ((aligned(x)))
#include "sys/queue.h"

/******************
	MemoryHeader
******************/

struct ALIGN(SLIST_ALIGNMENT) MemoryHeader
{
	MemoryHeader(int32 Size) : AllocSize(Size) {}

	static void* AttachHeader(MemoryHeader* Header, int32 Size)
	{
		new(header)MemoryHeader(Size);
		return reinterpret_cast<void*>(++Header);
	}

	static MemoryHeader* DetachHeader(void* Ptr)
	{
		MemoryHeader* Header = reinterpret_cast<MemoryHeader*>(Ptr) - 1;
		return Header;
	}

	SLIST_ENTRY(MemoryHeader)	Entries;
	int32						AllocSize;
};
SLIST_HEAD(SLIST_HEADER, MemoryHeader);

/******************
	MemoryPool
******************/

class ALIGN(SLIST_ALIGNMENT) MemoryPool
{
public:
	MemoryPool(int32 AllocSize);
	~MemoryPool();

	void						Push(MemoryHeader * Ptr);
	MemoryHeader*				Pop();

private:
	SLIST_HEADER				_Header;
	int32						_AllocSize = 0;
	std::atomic<int32>			_UseCount = 0;
	std::atomic<int32>			_ReserveCount = 0;
};

#elif defined(_MSC_VER)							// 윈도우 기반
#define ALIGN(x) __declspec(align(x))

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

/******************
	MemoryHeader
******************/

struct ALIGN(SLIST_ALIGNMENT) MemoryHeader : public SLIST_ENTRY
{
	MemoryHeader(int32 Size) : AllocSize(Size) {}

	static void* AttachHeader(MemoryHeader* Header, int32 Size)
	{
		new(Header)MemoryHeader(Size);
		return reinterpret_cast<void*>(++Header);
	}

	static MemoryHeader* DetachHeader(void* Ptr)
	{
		MemoryHeader* Header = reinterpret_cast<MemoryHeader*>(Ptr) - 1;
		return Header;
	}

	int32						AllocSize;
};

/******************
	MemoryPool
******************/

class ALIGN(SLIST_ALIGNMENT) MemoryPool
{
public:
	MemoryPool(int32 AllocSize);
	~MemoryPool();

	void						Push(MemoryHeader * Ptr);
	MemoryHeader*				Pop();

private:
	SLIST_HEADER				_Header;
	int32						_AllocSize = 0;
	std::atomic<int32>			_UseCount = 0;
	std::atomic<int32>			_ReserveCount = 0;
};

#else
#error "Unknown compiler"
#endif