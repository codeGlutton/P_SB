#include "Memory.h"
#include "C_SB.h"
#include "MemoryPool.h"

Memory::Memory()
{
	int32 Size = 0;
	int32 TableIndex = 0;

	for (Size = 32; Size <= 1024; Size += 32)
	{
		MemoryPool* Pool = new MemoryPool(Size);
		_Pools.Emplace(Pool);

		while (TableIndex <= Size)
		{
			_PoolTable[TableIndex] = Pool;
			TableIndex++;
		}
	}

	for (; Size <= 2048; Size += 128)
	{
		MemoryPool* Pool = new MemoryPool(Size);
		_Pools.Emplace(Pool);

		while (TableIndex <= Size)
		{
			_PoolTable[TableIndex] = Pool;
			TableIndex++;
		}
	}

	for (; Size <= 4096; Size += 256)
	{
		MemoryPool* Pool = new MemoryPool(Size);
		_Pools.Emplace(Pool);

		while (TableIndex <= Size)
		{
			_PoolTable[TableIndex] = Pool;
			TableIndex++;
		}
	}
}

Memory::~Memory()
{
	for (MemoryPool* Pool : _Pools)
		delete Pool;

	_Pools.Empty();
}

void* Memory::Allocate(int32 Size)
{
	MemoryHeader* Header = nullptr;
	const int32 AllocSize = Size + sizeof(MemoryHeader);
	
	if (AllocSize > MAX_ALLOC_SIZE)
	{
		Header = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(AllocSize, SLIST_ALIGNMENT));
	}
	else
	{
		Header = _PoolTable[AllocSize]->Pop();
	}

	return MemoryHeader::AttachHeader(Header, AllocSize);
}

void Memory::Release(void* Ptr)
{
	MemoryHeader* Header = MemoryHeader::DetachHeader(Ptr);

	const int32 AllocSize = Header->AllocSize;
	check(AllocSize > 0);

	if (AllocSize > MAX_ALLOC_SIZE)
	{
		::_aligned_free(Header);
	}
	else
	{
		_PoolTable[AllocSize]->Push(Header);
	}
}
