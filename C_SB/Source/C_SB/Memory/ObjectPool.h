#pragma once

#include "CoreMinimal.h"
#include "MemoryPool.h"

template<typename Type>
class ObjectPool
{
public:
	template<typename...ArgTypes>
	static Type* Pop(ArgTypes&&... Args)
	{
		Type* NewMemory = static_cast<Type*>(MemoryHeader::AttachHeader(S_Pool.Pop(), S_AllocSize));
		new(NewMemory)Type(std::forward<ArgTypes>(Args)...);
		return NewMemory;
	}

	static void Push(Type* Obj)
	{
		Obj->~Type();
		S_Pool.Push(MemoryHeader::DetachHeader(Obj));
	}

	template<typename... ArgTypes>
	static TSharedPtr<Type> MakeXShared(ArgTypes&&... Args)
	{
		TSharedPtr<Type> SharedPtr = { Pop(std::forward<ArgTypes>(Args)...), [](Type* Obj) { Push(Obj); } };
		return SharedPtr;
	}

private:
	static int32		S_AllocSize;
	static MemoryPool	S_Pool;
};

template<typename Type>
int32 ObjectPool<Type>::S_AllocSize = sizeof(Type) + sizeof(MemoryHeader);

template<typename Type>
MemoryPool ObjectPool<Type>::S_Pool{ S_AllocSize };