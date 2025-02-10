#include "SendBuffer.h"
#include "C_SB.h"

/**********************
	   SendBuffer
**********************/

SendBuffer::SendBuffer(SendBufferChunkRef Owner, BYTE* Buffer, uint32 AllocSize)
	: _Owner(Owner), _Buffer(Buffer), _AllocSize(AllocSize)
{
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::Close(uint32 WriteSize)
{
	check(_AllocSize >= WriteSize);
	_WriteSize = WriteSize;
	_Owner->Close(WriteSize);
}

/**********************
	SendBufferChunk
**********************/

SendBufferChunk::SendBufferChunk()
{
	_Buffer.AddZeroed(SEND_BUFFER_CHUNK_SIZE);
}

SendBufferChunk::~SendBufferChunk()
{
}

void SendBufferChunk::Reset()
{
	_bOpen = false;
	_UsedSize = 0;
}

SendBufferRef SendBufferChunk::Open(uint32 AllocSize)
{
	check(AllocSize <= SEND_BUFFER_CHUNK_SIZE);
	check(_bOpen == false);

	if (AllocSize > FreeSize())
		return nullptr;

	_bOpen = true;

	return ObjectPool<SendBuffer>::MakeXShared(AsShared(), Buffer(), AllocSize);
}

void SendBufferChunk::Close(uint32 WriteSize)
{
	check(_bOpen == true);
	_bOpen = false;
	_UsedSize += WriteSize;
}

/**********************
   SendBufferManager
**********************/

SendBufferRef SendBufferManager::Open(uint32 Size)
{
	if (_CurSendBufferChunk == nullptr)
	{
		_CurSendBufferChunk = Pop();
		_CurSendBufferChunk->Reset();
	}

	check(_CurSendBufferChunk->IsOpen() == false);

	if (_CurSendBufferChunk->FreeSize() < Size)
	{
		_CurSendBufferChunk = Pop();
		_CurSendBufferChunk->Reset();
	}

	return _CurSendBufferChunk->Open(Size);
}

SendBufferChunkRef SendBufferManager::Pop()
{
	{
		WRITE_LOCK;
		if (_SendBufferChunks.IsEmpty() == false)
		{
			SendBufferChunkRef NewSendBufferChunk = _SendBufferChunks.Pop();
			return NewSendBufferChunk;
		}
	}
	return SendBufferChunkRef(xnew<SendBufferChunk>(), [](SendBufferChunk* Obj) { GSendBufferManager->PushGlobal(Obj); });
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_SendBufferChunks.Emplace(buffer);
}

void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	UE_LOG(LogTemp, Log, TEXT("Push Chunk : SEND_BUFFER"));
	GSendBufferManager->Push(SendBufferChunkRef(buffer, [](SendBufferChunk* Obj) { GSendBufferManager->PushGlobal(Obj); }));
}