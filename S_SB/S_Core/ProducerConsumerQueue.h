#pragma once
#include "Job.h"
#include "LockQueue.h"

// 명확한 Producer threads과 Consumer threads를 가지는 JobQueue
class ProducerConsumerQueue : public std::enable_shared_from_this<ProducerConsumerQueue>
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

	// ProducerConsumerQueue가 더 이상 필요하지 않을 경우, 순환 참조 끊어주기 위함
	void				ClearJob() { _jobs.Clear(); }
	void				Execute();

	void				Push(JobRef& job);
	void				Push(JobRef&& job);

protected:
	LockQueue<JobRef>	_jobs;
	Atomic<int32>		_jobCount = 0;
};

