#include "pch.h"
#include "JobTimer.h"
#include "JobQueue.h"

/***********************
		JobTimer
***********************/

void JobTimer::Reserve(uint64 tickAfter, std::weak_ptr<JobQueue> owner, JobRef job)
{
	const uint64 executeTick = GetTickCount64() + tickAfter;
	JobData* jobData = ObjectPool<JobData>::Pop(owner, job);

	WRITE_LOCK;
	_items.push(TimerItem{ executeTick, jobData});
}

void JobTimer::Distribute(uint64 now)
{
	// 단일 스레드만 접근 허용
	if (_distributing.exchange(true) == true)
		return;

	// 타이머 초과된 멤버들만 추출
	xVector<TimerItem> items;
	{
		WRITE_LOCK;

		while (_items.empty() == false)
		{
			const TimerItem& timerItem = _items.top();
			if (now < timerItem.executeTick)
				break;

			items.push_back(timerItem);
			_items.pop();
		}
	}

	// Job 분배
	for (TimerItem& item : items)
	{
		if (JobQueueRef owner = item.jobData->owner.lock())
			owner->Push(item.jobData->job, true);
		
		ObjectPool<JobData>::Push(item.jobData);
	}

	_distributing.store(false);
}

void JobTimer::Clear()
{
	WRITE_LOCK;

	while (_items.empty() == false)
	{
		const TimerItem& timerItem = _items.top();
		ObjectPool<JobData>::Push(timerItem.jobData);
		_items.pop();
	}
}
