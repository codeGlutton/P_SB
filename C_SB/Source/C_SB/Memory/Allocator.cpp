#include "Allocator.h"
#include "C_SB.h"

/******************
	BaseAllocator
*******************/

void* BaseAllocator::Alloc(int32 Size)
{
	return ::malloc(Size);
}

void BaseAllocator::Release(void* Ptr)
{
	::free(Ptr);
}

/******************
	PoolAllocator
*******************/

void* PoolAllocator::Alloc(int32 Size)
{
	return GMemory->Allocate(Size);
}

void PoolAllocator::Release(void* Ptr)
{
	GMemory->Release(Ptr);
}