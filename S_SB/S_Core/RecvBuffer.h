#pragma once

class RecvBuffer
{
	enum { BUFFER_COUNT = 10 };

public:
	RecvBuffer(int32 bufferSize);
	~RecvBuffer();

	void			Clean();
	bool			OnRead(int32 numOfBytes);
	bool			OnWrite(int32 numOfBytes);

	BYTE*			ReadPos() { return &_buffer[_readPos]; }
	BYTE*			WritePos() { return &_buffer[_writePos]; }
	// 읽기 대기 중인 데이터 크기
	int32			DataSize() { return _writePos - _readPos; }
	// 남은 버퍼 빈칸 크기
	int32			FreeSize() { return _capacity - _writePos; }

private:
	// 여유 공간 + 초기 버퍼 크기 (실제 버퍼 사용 크기)
	int32			_capacity = 0;
	// 초기 버퍼 크기
	int32			_bufferSize = 0;
	int32			_readPos = 0;
	int32			_writePos = 0;
	xVector<BYTE>	_buffer;
};