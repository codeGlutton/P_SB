#pragma once

#include "CoreMinimal.h"
#include <atomic>

/* 멀티스레드 관련 타입 */

template<typename T>
using Atomic = std::atomic<T>;

/* SharedPtr 타입형 */

#define USING_SHARED_PTR(name) using name##Ref = TSharedPtr<class name>
#define USING_SHARED_PTR_IN_NAMESPACE(space, name) namespace space { USING_SHARED_PTR(name); }
#define USING_CONST_SHARED_PTR(name) using name##ConstRef = TSharedPtr<const class name>
#define USING_CONST_SHARED_PTR_IN_NAMESPACE(space, name) namespace space { USING_CONST_SHARED_PTR(name); }

USING_SHARED_PTR(PacketSession);
USING_SHARED_PTR(SendBuffer);
USING_SHARED_PTR(SendBufferChunk);
USING_SHARED_PTR(SendWorker);
USING_SHARED_PTR(RecvWorker);