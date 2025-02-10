#include "pch.h"
#include "ProducerConsumerQueue.h"
#include "GlobalQueue.h"

void ProducerConsumerQueue::Execute()
{
	LCurrentProducerConsumerQueue = this;

	while (true)
	{
		// 가능하대로 빼내 처리 (도중에 새로운 Job 추가 가능성 있음)
		xVector<JobRef> jobs;
		_jobs.PopAll(OUT jobs);

		// 처리가 확정된 job 갯수
		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++)
			jobs[i]->Execute();

		// 남은 일감이 없는 경우 (중도에 들어오지 않았거나, 들어올 예정이 없는 경우)
		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			LCurrentProducerConsumerQueue = nullptr;
			return;
		}

		// 일감이 너무 몰려서 특정 시간을 초과한 경우
		const uint64 now = GetTickCount64();
		if (now >= LEndTickCount)
		{
			// 타 스레드에 GlobalQueue로 명시
			LCurrentProducerConsumerQueue = nullptr;
			GGlobalQueue->PushProdConsQ(shared_from_this());
			return;
		}
	}
}

void ProducerConsumerQueue::Push(JobRef& job)
{
	// 실제 job 추가 전에 Count 증가로 PushProdConsQ 예고
	const int32 preCount = _jobCount.fetch_add(1);
	_jobs.Push(job);

	if (preCount == 0)
	{
		GGlobalQueue->PushProdConsQ(shared_from_this());
	}
}

void ProducerConsumerQueue::Push(JobRef&& job)
{
	// 실제 job 추가 전에 Count 증가로 PushProdConsQ 예고
	const int32 preCount = _jobCount.fetch_add(1);
	_jobs.Push(job);

	if (preCount == 0)
	{
		GGlobalQueue->PushProdConsQ(shared_from_this());
	}
}
