#pragma once

// 데드락 탐지 프로파일 클래스 
class DeadLockProfiler
{
public:
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void Dfs(int32 here);

private:
	// 기록된 락 사용 클래스 이름 -> 아이디
	std::unordered_map<const char*, int32>	_nameToId;
	// 기록된 락 사용 클래스 아이디 -> 이름
	std::unordered_map<int32, const char*>	_idToName;
	// 특정 락 에서 다른 락 접근 기록
	std::map<int32, std::set<int32>>		_lockHistory;

	Mutex									_lock;

private:
	// 발견 여부 및 순서 표시
	std::vector<int32>						_discoveredOrder;
	// 발견 순서 표시용 카운트
	int32									_discoveredCount = 0;
	// 연결된 노드 검사를 모두 마친 노드인가
	std::vector<bool>						_finished;
	// 첫 발견시 어떤 노드에서 접근했나
	std::vector<int32>						_parent;
};

