#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "CoreMacro.h"

#include "Lock.h"

// 서버 코드 호환용
struct PacketHeader
{
	uint16						size;
	uint16						id;
};

/**********************
	   SendBuffer
**********************/

class SendBuffer : public TSharedFromThis<SendBuffer>
{
public:
	SendBuffer(SendBufferChunkRef Owner, BYTE* Buffer, uint32 AllocSize);
	~SendBuffer();

	BYTE*						Buffer() { return _Buffer; }
	uint32						AllocSize() { return _AllocSize; }
	uint32						WriteSize() { return _WriteSize; }

	void						Close(uint32 WriteSize);

private:
	SendBufferChunkRef			_Owner;
	BYTE*						_Buffer;
	uint32						_AllocSize = 0;
	uint32						_WriteSize = 0;
};

/**********************
	SendBufferChunk
**********************/

class SendBufferChunk : public TSharedFromThis<SendBufferChunk>
{
	enum
	{
		SEND_BUFFER_CHUNK_SIZE = 6000
	};

public:
	SendBufferChunk();
	~SendBufferChunk();

	void						Reset();
	SendBufferRef				Open(uint32 AllocSize);
	void						Close(uint32 WriteSize);

	bool						IsOpen() { return _bOpen; }
	BYTE*						Buffer() { return &_Buffer[_UsedSize]; }
	uint32						FreeSize() { return static_cast<uint32>(_Buffer.Num() - _UsedSize); }

private:
	TArray<BYTE>				_Buffer;
	bool						_bOpen = false;
	uint32						_UsedSize = 0;
};

/**********************
   SendBufferManager
**********************/

class SendBufferManager
{
public:
	SendBufferRef				Open(uint32 Size);

private:
	SendBufferChunkRef			Pop();
	void						Push(SendBufferChunkRef Buffer);

	static void					PushGlobal(SendBufferChunk* Buffer);

private:
	USE_LOCK;

	SendBufferChunkRef			_CurSendBufferChunk = nullptr;
	TArray<SendBufferChunkRef>	_SendBufferChunks;
};
