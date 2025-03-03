#pragma once
#include "Job.h"
#include "LockQueue.h"
#include "JobTimer.h"

// 콜백 LockQueue<JobRef>를 활용 용이하도록 packing한 추상 클래스
class JobQueue : public std::enable_shared_from_this<JobQueue>
{
public:
	// 이미 생성된 콜백 클로저를 활용해 콜백할 경우 생성자
	void DoAsync(CallBackType&& callback)
	{
		Push(ObjectPool<Job>::MakeXShared(std::move(callback)));
	}

	// 해당 객체의 메소드를 콜백할 경우 생성자
	template<typename T, typename Ret>
	void DoAsync(Ret(T::* memFunc)())
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		Push(ObjectPool<Job>::MakeXShared(owner, memFunc));
	}
	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::* memFunc)(Args...), const Args&... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		Push(ObjectPool<Job>::MakeXShared(owner, memFunc, args...));
	}
	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::* memFunc)(Args...), Args&&... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		Push(ObjectPool<Job>::MakeXShared(owner, memFunc, std::forward<Args>(args)...));
	}

	// 이미 생성된 콜백 클로저를 활용해 일정 시간 후 콜백할 경우 생성자
	void DoTimer(uint64 timeAfter, CallBackType&& callback)
	{
		JobRef job = ObjectPool<Job>::MakeXShared(std::move(callback));
		GJobTimer->Reserve(timeAfter, shared_from_this(), job);
	}

	// 해당 객체의 메소드를 일정 시간 후 콜백할 경우 생성자
	template<typename T, typename Ret>
	void DoTimer(uint64 timeAfter, Ret(T::* memFunc)())
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		JobRef job = ObjectPool<Job>::MakeXShared(owner, memFunc);
		GJobTimer->Reserve(timeAfter, shared_from_this(), job);
	}
	template<typename T, typename Ret, typename... Args>
	void DoTimer(uint64 timeAfter, Ret(T::* memFunc)(Args...), const Args&... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		JobRef job = ObjectPool<Job>::MakeXShared(owner, memFunc, args...);
		GJobTimer->Reserve(timeAfter, shared_from_this(), job);
	}
	template<typename T, typename Ret, typename... Args>
	void DoTimer(uint64 timeAfter, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		JobRef job = ObjectPool<Job>::MakeXShared(owner, memFunc, std::forward<Args>(args)...);
		GJobTimer->Reserve(timeAfter, shared_from_this(), job);
	}

	// JobQueue가 더 이상 필요하지 않을 경우, 순환 참조 끊어주기 위함
	void				ClearJob() { _jobs.Clear(); }
	void				Execute();

	void				Push(JobRef& job, bool pushOnly = false);
	void				Push(JobRef&& job, bool pushOnly = false);

protected:
	LockQueue<JobRef>	_jobs;
	Atomic<int32>		_jobCount = 0;
};

