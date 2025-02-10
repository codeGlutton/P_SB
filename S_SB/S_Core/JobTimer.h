#pragma once

// 추가될 Job 정보
struct JobData
{
	JobData(std::weak_ptr<JobQueue> owner, JobRef job) : owner(owner), job(job)
	{

	}

	std::weak_ptr<JobQueue>													owner;
	JobRef																	job;
};

// 일정 시간 뒤에 JobData 추가를 위한 정보
struct TimerItem
{
	bool operator<(const TimerItem& other) const
	{
		return executeTick < other.executeTick;
	}
	bool operator>(const TimerItem& other) const
	{
		return executeTick > other.executeTick;
	}

	uint64																	executeTick = 0;
	JobData*																jobData = nullptr;
};

/***********************
		JobTimer
***********************/

class JobTimer
{
public:
	// 타이머 예약
	void																	Reserve(uint64 tickAfter, std::weak_ptr<JobQueue> owner, JobRef job);
	// 타이머 체크와 타이머 초과된 Job을 분배 (다중 접근 불가)
	void																	Distribute(uint64 now);
	void																	Clear();

private:
	USE_LOCK;

	xPriorityQueue<TimerItem, xVector<TimerItem>, std::greater<TimerItem>>	_items;
	// 현재 Distribute() 작업 실행 여부
	Atomic<bool>															_distributing = false;
};

