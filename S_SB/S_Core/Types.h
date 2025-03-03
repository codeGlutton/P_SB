#pragma once

#include <mutex>
#include <atomic>

// 코드 수정 용이성을 위해 UE 타입 네이밍과 최대한 호환되도록 맞춰주기

/* 기본 타입형 */

using BYTE					= unsigned char;
using int8					= __int8;
using int16					= __int16;
using int32					= __int32;
using int64					= __int64;
using uint8					= unsigned __int8;
using uint16				= unsigned __int16;
using uint32				= unsigned __int32;
using uint64				= unsigned __int64;

/* 멀티스레드 관련 타입 */

template<typename T>
using Atomic				= std::atomic<T>;
using Mutex					= std::mutex;
using LockGuard				= std::lock_guard<std::mutex>;

/* SharedPtr 타입형 */

#define USING_SHARED_PTR(name) using name##Ref = std::shared_ptr<class name>
#define USING_SHARED_PTR_IN_NAMESPACE(space, name) namespace space { USING_SHARED_PTR(name); }
#define USING_SHARED_PTR_IN_TWO_NAMESPACE(space1, space2, name) namespace space1 { USING_SHARED_PTR_IN_NAMESPACE(space2, name) }

#define USING_CONST_SHARED_PTR(name) using name##ConstRef = std::shared_ptr<const class name>
#define USING_CONST_SHARED_PTR_IN_NAMESPACE(space, name) namespace space { USING_CONST_SHARED_PTR(name); }
#define USING_CONST_SHARED_PTR_IN_TWO_NAMESPACE(space1, space2, name) namespace space1 { USING_CONST_SHARED_PTR_IN_NAMESPACE(space2, name); }

USING_SHARED_PTR(IocpCore);
USING_SHARED_PTR(IocpObject);
USING_SHARED_PTR(Session);
USING_SHARED_PTR(PacketSession);
USING_SHARED_PTR(Listener);
USING_SHARED_PTR(ServerService);
USING_SHARED_PTR(ClientService);
USING_SHARED_PTR(SendBuffer);
USING_SHARED_PTR(SendBufferChunk);
USING_SHARED_PTR(Job);
USING_SHARED_PTR(JobQueue);
USING_SHARED_PTR(ProducerConsumerQueue);