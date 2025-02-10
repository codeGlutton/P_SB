#pragma once

extern thread_local uint32							LThreadId;
// 네트워크 처리 및 게임 로직 처리 + 글로벌 큐 작업 결정까지의 최대 시간
extern thread_local uint64							LEndTickCount;

// 현재 스레드에서 쌓인 락
extern thread_local std::stack<int32>				LLockStack;
// 현재 스레드에서 진행중인 JobQ
extern thread_local JobQueue*						LCurrentJobQueue;
// 현재 스레드에서 진행중인 ProducerConsumerQ
extern thread_local ProducerConsumerQueue*			LCurrentProducerConsumerQueue;
extern thread_local SendBufferChunkRef				LSendBufferChunk;