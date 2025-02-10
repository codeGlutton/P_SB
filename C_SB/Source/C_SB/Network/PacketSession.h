#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "Containers/Queue.h"

/************************
	   FPacketHeader
*************************/

struct C_SB_API FPacketHeader
{
	FPacketHeader() : PacketSize(0), PacketId(0)
	{
	}

	FPacketHeader(uint16 PacketSize, uint16 PacketId) : PacketSize(PacketSize), PacketId(PacketId)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FPacketHeader& Header)
	{
		Ar << Header.PacketSize;
		Ar << Header.PacketId;
		return Ar;
	}

	uint16 PacketSize;
	uint16 PacketId;
};

/************************
	  PacketSession
*************************/

class C_SB_API PacketSession : public TSharedFromThis<PacketSession>
{
public:
	PacketSession(class USBNetworkManager* Owner, class FSocket* Socket);
	virtual ~PacketSession();

public:
	void							HandleRecvPackets();

	void							SendPacket(SendBufferRef SendBuffer);

	void							Run();
	void							Disconnect();

	virtual void					OnConnected();

public:
	class USBNetworkManager*		Owner;
	class FSocket*					Socket;
	RecvWorkerRef					RecvWorkerThread;
	SendWorkerRef					SendWorkerThread;
	
	TQueue<TArray<uint8>>			RecvPacketQueue;
	TQueue<SendBufferRef>			SendPacketQueue;
};
