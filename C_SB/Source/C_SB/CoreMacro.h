#pragma once

#include "CoreMinimal.h"

/*******************
	 C++ Helper
********************/

#define OUT

#define NAMESPACE_BEGIN(name)					namespace name {
#define NAMESPACE_END							}

// For C type array
#define ARRAY_LEN(arr)							sizeof(arr)/sizeof(arr[0])

/*******************
   RecursiveRWLock
********************/

#define USE_MANY_LOCKS(count)					RecursiveRWLock _Locks[count]
#define USE_LOCK								USE_MANY_LOCKS(1)
#define READ_LOCK_IDX(idx)						ReadLockGuard ReadLockGuard_##idx(_Locks[idx])
#define READ_LOCK								READ_LOCK_IDX(0)
#define WRITE_LOCK_IDX(idx)						WriteLockGuard WriteLockGuard_##idx(_Locks[idx])
#define WRITE_LOCK								WRITE_LOCK_IDX(0)

/*******************
		Memory
********************/

#if UE_BUILD_SHIPPING != 1
#define xAlloc(size)							PoolAllocator::Alloc(size)
#define xRelease(ptr)							PoolAllocator::Release(ptr)
#else
#define xAlloc(size)							PoolAllocator::Alloc(size)
#define xRelease(ptr)							PoolAllocator::Release(ptr)
#endif

/*******************
	  Protobuf
********************/

#define SEND_C_PACKET(session, pkt)															\
{																							\
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);								\
	if (session)																			\
		session->SendPacket(sendBuffer);													\
}

#define SEND_REQ_PACKET(pkt)					ServerHttpPacketHandler::SendPkt(pkt)
#define SEND_REQ_PACKET_WITH_TOKEN(pkt, token)	ServerHttpPacketHandler::SendPkt(pkt, token)
