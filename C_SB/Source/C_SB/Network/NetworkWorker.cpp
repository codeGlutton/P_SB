#include "NetworkWorker.h"
#include "C_SB.h"

#include "Sockets.h"
#include "Serialization/ArrayWriter.h"
#include "Engine/Engine.h"

#include "PacketSession.h"

DEFINE_LOG_CATEGORY(LogNetWorker);

/************************
		RecvWorker
*************************/

RecvWorker::RecvWorker(FSocket* Socket, PacketSessionRef Session) : _Socket(Socket), _SessionRef(Session)
{
	_Thread = FRunnableThread::Create(this, TEXT("RecvWorker Thread"));
}

RecvWorker::~RecvWorker()
{
	delete _Thread;
}

bool RecvWorker::Init()
{
	UE_LOG(LogNetWorker, Log, TEXT("Recv Thread Init"));

	return true;
}

uint32 RecvWorker::Run()
{
	while (_bRunning)
	{
		TArray<uint8> Packet;

		if (ReceivePacket(OUT Packet))
		{
			if (PacketSessionRef Session = _SessionRef.Pin())
			{
				Session->RecvPacketQueue.Enqueue(Packet);
			}
		}
	}

	return 0; 
}

void RecvWorker::Exit()
{
}

void RecvWorker::Destroy()
{
	_bRunning = false;
}

bool RecvWorker::ReceivePacket(TArray<uint8>& OutPacket)
{
	OutPacket.Empty();

	const int32 HeaderSize = sizeof(FPacketHeader);
	OutPacket.AddZeroed(HeaderSize);

	int32 RecvHeaderSize;
	if (ReceiveDesiredBytes(OutPacket.GetData(), HeaderSize, RecvHeaderSize) == false)
	{
		return false;
	}
	// Disconnect Pkt
	if (RecvHeaderSize == 0)
	{
		OutPacket.Empty();
		return true;
	}

	FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(OutPacket.GetData());
	{
		UE_LOG(LogNetWorker, Log, TEXT("Recv Packet ID : %d, PacketSize : %d"), Header->PacketId, Header->PacketSize);
	}

	const int32 PayloadSize = Header->PacketSize - HeaderSize;
	OutPacket.AddZeroed(PayloadSize);

	if (PayloadSize == 0)
	{
		return true;
	}

	int32 RecvPayloadSize;
	if (ReceiveDesiredBytes(&OutPacket[HeaderSize], PayloadSize, RecvPayloadSize) == false)
	{
		return false;
	}

	return true;
}

bool RecvWorker::ReceiveDesiredBytes(uint8* Results, int32 Size, OUT int32& RecvSize)
{
	uint32 PendingDataSize;
	if (_Socket->HasPendingData(PendingDataSize) == false || PendingDataSize <= 0)
	{
		RecvSize = 0;
		return false;
	}

	int32 Offset = 0;
	while (Size > 0)
	{
		int32 NumRead = 0;
		_Socket->Recv(Results + Offset, Size, OUT NumRead);
		check(NumRead <= Size);

		// Disconnect Pkt
		if (NumRead <= 0)
		{
			RecvSize = 0;
			return true;
		}

		Offset += NumRead;
		Size -= NumRead;
	}

	RecvSize = Offset;
	return true;
}

/************************
		SendWorker
*************************/

SendWorker::SendWorker(FSocket* Socket, PacketSessionRef Session) : _Socket(Socket), _SessionRef(Session)
{
	_Thread = FRunnableThread::Create(this, TEXT("SendWorker Thread"));
}

SendWorker::~SendWorker()
{
	delete _Thread;
}

bool SendWorker::Init()
{
	UE_LOG(LogNetWorker, Log, TEXT("Send Thread Init"));

	return true;
}

uint32 SendWorker::Run()
{
	while (_bRunning)
	{
		SendBufferRef SendBuffer;

		if (PacketSessionRef Session = _SessionRef.Pin())
		{
			if (Session->SendPacketQueue.Dequeue(OUT SendBuffer))
			{
				SendPacket(SendBuffer);
			}
		}
	}

	return 0;
}

void SendWorker::Exit()
{
}

bool SendWorker::SendPacket(SendBufferRef SendBuffer)
{
	if (SendDesiredBytes(SendBuffer->Buffer(), SendBuffer->WriteSize()) == false)
		return false;

	return true;
}

void SendWorker::Destroy()
{
	_bRunning = false;
}

bool SendWorker::SendDesiredBytes(const uint8* Buffer, int32 Size)
{
	while (Size > 0)
	{
		int32 ByteSent = 0;
		if (_Socket->Send(Buffer, Size, ByteSent) == false)
			return false;

		Size -= ByteSent;
		Buffer += ByteSent;
	}

	return true;
}
