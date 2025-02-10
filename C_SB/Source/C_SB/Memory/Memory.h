#pragma once

#include "CoreMinimal.h"
#include "Allocator.h"

class MemoryPool;

class Memory
{
	enum
	{
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void*						Allocate(int32 Size);
	void						Release(void* Ptr);

public:
	TArray<MemoryPool*>			_Pools;
	MemoryPool*					_PoolTable[MAX_ALLOC_SIZE + 1];
};

template<typename Type, typename... ArgTypes>
Type* xnew(ArgTypes&&... Args)
{
	Type* NewMemory = static_cast<Type*>(xAlloc(sizeof(Type)));
	
	new(NewMemory)Type(std::forward<ArgTypes>(Args)...);
	
	return NewMemory;
}

template<typename Type>
void xdelete(Type* Obj)
{
	Obj->~Type();
	xRelease(Obj);
}

template<typename Type, typename... ArgTypes>
TSharedPtr<Type> MakeXShared(ArgTypes&&... Args)
{
	return TSharedPtr<Type> { xnew<Type>(std::forward<ArgTypes>(Args)...), [](Type* Obj) { xdelete<Type>(Obj); } };
}