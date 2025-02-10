#include "NetworkWorker.h"
#include "C_SB.h"

#include "Sockets.h"
#include "Serialization/ArrayWriter.h"
#include "Engine/Engine.h"

#include "PacketSession.h"

/************************
		RecvWorker
*************************/

RecvWorker::RecvWorker(FSocket* Socket, PacketSessionRef Session) : _Socket(Socket), _SessionRef(Session)
{
	_Thread = FRunnableThread::Create(this, TEXT("RecvWorker Thread"));
}

RecvWorker::~RecvWorker()
{
}

bool RecvWorker::Init()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Recv Thread Init")));

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

	if (ReceiveDesiredBytes(OutPacket.GetData(), HeaderSize) == false)
		return false;

	FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(OutPacket.GetData());
	{
		UE_LOG(LogTemp, Log, TEXT("Recv Packet ID : %d, PacketSize : %d"), Header->PacketId, Header->PacketSize);
	}

	const int32 PayloadSize = Header->PacketSize - HeaderSize;
	OutPacket.AddZeroed(PayloadSize);

	if (PayloadSize == 0)
		return true;

	if (ReceiveDesiredBytes(&OutPacket[HeaderSize], PayloadSize) == false)
		return false;

	return true;
}

bool RecvWorker::ReceiveDesiredBytes(uint8* Results, int32 Size)
{
	uint32 PendingDataSize;
	if (_Socket->HasPendingData(PendingDataSize) == false || PendingDataSize <= 0)
		return false;

	int32 Offset = 0;
	while (Size > 0)
	{
		int32 NumRead = 0;
		_Socket->Recv(Results + Offset, Size, OUT NumRead);
		check(NumRead <= Size);

		if (NumRead <= 0)
			return false;

		Offset += NumRead;
		Size -= NumRead;
	}

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
}

bool SendWorker::Init()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Send Thread Init")));

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
