#pragma once
#include "CoreMacro.h"

/******************
	BaseAllocator
*******************/

class BaseAllocator
{
public:
	static void*	Alloc(int32 size);
	static void		Release(void* ptr);
};

/******************
	StompAllocator
*******************/

// 페이징 기법과 VirtualAlloc을 이용한 디버깅 Allocator 클래스
class StompAllocator
{
	enum { PAGE_SIZE = 0x1000 };
public:
	static void*	Alloc(int32 size);
	static void		Release(void* ptr);
};

/******************
	PoolAllocator
*******************/

// 메모리풀을 이용한 Allocator 클래스 (#define _STOMP 정의시, StompAllocator로 구동)
class PoolAllocator
{
public:
	static void*	Alloc(int32 size);
	static void		Release(void* ptr);
};

/******************
	STL Allocator
*******************/

// xAlloc, xRelease을 이용한 stl Allocator 클래스
template<typename T>
class StlAllocator
{
public:
	using value_type = T;

	StlAllocator() { }

	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) { }

	T* allocate(size_t count)
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(xAlloc(size));
	}

	void deallocate(T* ptr, size_t count)
	{
		xRelease(ptr);
	}
};
