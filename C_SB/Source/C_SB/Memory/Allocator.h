#pragma once

#include "CoreMacro.h"

/******************
	BaseAllocator
*******************/

class BaseAllocator
{
public:
	static void*	Alloc(int32 Size);
	static void		Release(void* Ptr);
};

/******************
	PoolAllocator
*******************/

class PoolAllocator
{
public:
	static void*	Alloc(int32 Size);
	static void		Release(void* Ptr);
};

/******************
	STL Allocator
*******************/

template<typename T>
class StlAllocator
{
public:
	using value_type = T;

	StlAllocator() { }

	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) { }

	T* allocate(size_t Count)
	{
		const int32 Size = static_cast<int32>(Count * sizeof(T));
		return static_cast<T*>(xAlloc(Size));
	}

	void deallocate(T* ptr, size_t Count)
	{
		xRelease(ptr);
	}
};
