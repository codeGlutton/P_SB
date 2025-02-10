#pragma once
#include "Allocator.h"

class MemoryPool;

class Memory
{
	enum
	{
		// 메모리풀 할당 크기 
		// ~1024 byte		: 32 byte 단위 구분 
		// 1024~2048 byte	: 128 byte 단위 구분 
		// 2048~4096 byte	: 256 byte 단위 구분 
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void*						Allocate(int32 size);
	void						Release(void* ptr);

private:
	std::vector<MemoryPool*>	_pools;

	// 할당 원하는 메모리 크기 -> 적절한 메모리 풀 포인터
	MemoryPool*					_poolTable[MAX_ALLOC_SIZE + 1];
};

// xAlloc을 이용한 new (기본 메모리 풀)
template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(xAlloc(sizeof(Type)));
	
	new(memory)Type(std::forward<Args>(args)...);
	
	return memory;
}

// xRelease을 이용한 delete (기본 메모리 풀)
template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();
	xRelease(obj);
}

template<typename Type, typename... Args>
std::shared_ptr<Type> MakeXShared(Args&&... args)
{
	return std::shared_ptr<Type> { xnew<Type>(std::forward<Args>(args)...), xdelete<Type> };
}