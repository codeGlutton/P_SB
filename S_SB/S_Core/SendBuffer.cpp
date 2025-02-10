#include "pch.h"
#include "SendBuffer.h"

class SendBufferChunk;

/**********************
	   SendBuffer
**********************/

SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize)
	: _owner(owner), _buffer(buffer), _allocSize(allocSize)
{
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::Close(uint32 writeSize)
{
	ASSERT_CRASH(_allocSize >= writeSize);
	_writeSize = writeSize;
	_owner->Close(writeSize);
}

/**********************
	SendBufferChunk
**********************/

SendBufferChunk::SendBufferChunk()
{
}

SendBufferChunk::~SendBufferChunk()
{
}

void SendBufferChunk::Reset()
{
	_open = false;
	_usedSize = 0;
}

SendBufferRef SendBufferChunk::Open(int32 allocSize)
{
	// allocSize는 1차적으로 이미 TLS에서 걸러져 FreeSize 크기 이하로 전달
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);
	ASSERT_CRASH(_open == false);

	if (allocSize > FreeSize())
		return nullptr;

	_open = true;

	// SendBuffer 객체는 Object 풀에서 관리하고, SendBuffer 객체가 가리키는 실제 Buffer는 SendBufferChunk가 관리
	return ObjectPool<SendBuffer>::MakeXShared(shared_from_this(), Buffer(), allocSize);
}

void SendBufferChunk::Close(uint32 writeSize)
{
	ASSERT_CRASH(_open == true);
	_open = false;
	_usedSize += writeSize;
}

/**********************
   SendBufferManager
**********************/

SendBufferRef SendBufferManager::Open(uint32 size)
{
	if (LSendBufferChunk == nullptr)
	{
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}

	ASSERT_CRASH(LSendBufferChunk->IsOpen() == false);

	if (LSendBufferChunk->FreeSize() < size)
	{
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}

	return LSendBufferChunk->Open(size);
}

SendBufferChunkRef SendBufferManager::Pop()
{
	{
		WRITE_LOCK;
		if (_sendBufferChunks.empty() == false)
		{
			SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back();
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}
	// 새로운 SendBufferChunk는 삭제 시기에 delete가 아닌 Global에 PushJobQ 해둔다.
	return SendBufferChunkRef(xnew<SendBufferChunk>(), PushGlobal);
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_sendBufferChunks.push_back(buffer);
}

void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	std::cout << "Push Chunk : SEND_BUFFER" << std::endl;
	GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal));
}
