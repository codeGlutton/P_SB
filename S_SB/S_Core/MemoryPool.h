#pragma once

enum
{
	SLIST_ALIGNMENT = 16
};

/******************
	MemoryHeader
******************/

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY
{
	MemoryHeader(int32 size) : allocSize(size) {}

	// 전체 메모리 할당 후, 앞에 헤더 생성해주기
	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		new(header)MemoryHeader(size);
		return reinterpret_cast<void*>(++header);
	}

	// 헤더 뒤 본 데이터 부분을 담당하는 포인터를 넘겨주면, 해당 헤더 부분을 리턴
	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}

	int32 allocSize;
};

/******************
	MemoryPool
******************/

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	// 동적 변수 반납시
	void			Push(MemoryHeader* ptr);
	// 동적 변수 재시용시
	MemoryHeader*	Pop();

private:
	SLIST_HEADER		_header;
	// 관리 데이터 최대 사이즈
	int32				_allocSize = 0;
	// 사용중인 갯수
	std::atomic<int32>	_useCount = 0;
	// 반납된 갯수
	std::atomic<int32>	_reserveCount = 0;
};