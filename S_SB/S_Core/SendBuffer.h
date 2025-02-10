#pragma once

/**********************
	   SendBuffer
**********************/

class SendBuffer : public std::enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize);
	~SendBuffer();

	BYTE*									Buffer() { return _buffer; }
	uint32									AllocSize() { return _allocSize; }
	uint32									WriteSize() { return _writeSize; }
	// TLS 청크에 실제 사용 크기 확정
	void									Close(uint32 writeSize);

private:
	// 여유 공간을 포함한 버퍼
	BYTE*									_buffer;
	// 여유 공간을 포함한 할당 크기
	uint32									_allocSize = 0;
	// 본래 전송 데이터 크기
	uint32									_writeSize = 0;
	// TLS 내 메모리 청크 Ref
	SendBufferChunkRef						_owner;
};

/**********************
	SendBufferChunk
**********************/

class SendBufferChunk : public std::enable_shared_from_this<SendBufferChunk>
{
	enum
	{
		SEND_BUFFER_CHUNK_SIZE = 6000
	};

public:
	SendBufferChunk();
	~SendBufferChunk();

	// 청크 쓰기 위치 0 초기화
	void									Reset();
	// 여유 공간을 할당받고 우선 SendBuffer 생산 (인덱스 수정 없음)
	SendBufferRef							Open(int32 allocSize);
	// 새로운 버퍼의 실제 크기 반영 (인덱스 수정)
	void									Close(uint32 writeSize);
	bool									IsOpen() { return _open; }
	// 지금까지 사용하여 남은 공간의 포인터
	BYTE*									Buffer() { return &_buffer[_usedSize]; }
	// 남은 공간 크기
	uint32									FreeSize() { return static_cast<uint32>(_buffer.size() - _usedSize); }

private:
	xArray<BYTE, SEND_BUFFER_CHUNK_SIZE>	_buffer = {};
	bool									_open = false;
	// 지금까지 사용하여 남은 공간 인덱스
	uint32									_usedSize = 0;
};

/**********************
   SendBufferManager
**********************/

class SendBufferManager
{
public:
	// 스레드 사용자의 요청에 따라 TLS의 청크 공간을 확인하고 사용 가능한 버퍼를 반환
	// @param size: 여유 공간 포함하여 요청 크기
	SendBufferRef							Open(uint32 size);

private:
	// Open 내부 함수로 여분 혹은 새로운 청크 할당 
	SendBufferChunkRef						Pop();
	// PushGlobal의 내부 함수
	void									Push(SendBufferChunkRef buffer);

	// 사용하지 않는 청크 자동 처리 함수 called by shared_ptr
	static void								PushGlobal(SendBufferChunk* buffer);

private:
	USE_LOCK;

	// 글로벌 여분 청크 보관소
	xVector<SendBufferChunkRef>				_sendBufferChunks;
};