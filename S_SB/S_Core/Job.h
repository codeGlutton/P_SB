#pragma once
#include <functional>

using CallBackType = std::function<void()>;

// 람다와 Funtion을 활용한 Job 클래스
class Job
{
public:
	// 이미 생성된 콜백 클로저를 활용할 경우 생성자
	Job(CallBackType&& callback) : _callback(std::move(callback))
	{
	}

	// 메소드 콜백을 활용하는 Job 생성자
	template<typename T, typename Ret, typename... Args>
	Job(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args... args)
	{
		_callback = [owner, memFunc, args...]()
			{
				(owner.get()->*memFunc)(args...);
			};
	}

	void Execute()
	{
		_callback();
	}

private:
	CallBackType _callback;
};
