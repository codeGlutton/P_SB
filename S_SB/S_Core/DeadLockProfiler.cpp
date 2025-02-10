#include "pch.h"
#include "DeadLockProfiler.h"

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	int32 lockId = 0;

	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end())
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{
		lockId = findIt->second;
	}

	if (LLockStack.empty() == false)
	{
		const int32 prevId = LLockStack.top();
		// 동일 클래스의 락 중첩은 허용
		if (lockId != prevId)
		{
			// 이전 락 사용 클래스가 처음 샤로운 락 사용 클래스을 접근할 경우, 기록판 생성. 아니면 불러오기
			std::set<int32>& history = _lockHistory[prevId];
			// 들어온 락 사용 클래스를 이전에 건 락 사용 클래스 다음으로 걸어본 것은
			// 처음으로 시도해보 것인가?
			if (history.find(lockId) == history.end())
			{
				history.insert(lockId);
				CheckCycle();
			}
		}
	}

	LLockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	if (LLockStack.empty())
		CRASH("MUTIPLE_UNLOCK");

	int32 lockId = _nameToId[name];
	if (LLockStack.top() != lockId)
		CRASH("INVALID_UNLOCK");

	LLockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = std::vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = std::vector<bool>(lockCount, false);
	_parent = std::vector<int32>(lockCount, -1);

	for (int32 lockId = 0; lockId < lockCount; lockId++)
		Dfs(lockId);

	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] != -1)
		return;

	_discoveredOrder[here] = _discoveredCount++;

	auto findIt = _lockHistory.find(here);
	// 락 걸고 바로 해제했거나, 방금 새로 생긴 락 사용 클래스인 경우
	if (findIt == _lockHistory.end())
	{
		_finished[here] = true;
		return;
	}

	std::set<int32>& nextSet = findIt->second;
	for (int32 there : nextSet)
	{
		// 첫 방문
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// 순방향 (접근이 처음)
		if (_discoveredOrder[here] < _discoveredOrder[there])
			continue;

		// 인근 노드 탐색이 끝나기도 전에 다시 접근했으므로, 순환인 역방향 접근
		if (_finished[there] == false)
		{
			printf("%s -> %s", _idToName[here], _idToName[there]);

			int32 now = here;
			while (true)
			{
				printf("%s -> %s", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];
				if (now == there)
					break;
			}

			CRASH("DEADLOCK_DETECTED");
		}
	}

	_finished[here] = true;
}
