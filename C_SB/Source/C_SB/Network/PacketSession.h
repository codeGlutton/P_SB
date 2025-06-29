#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "Containers/Queue.h"

namespace Protocol { class ServerInfo; class C_LOGIN; }

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
	PacketSession();
	virtual ~PacketSession();

public:
	void								HandleRecvPackets();

	void								SendPacket(SendBufferRef SendBuffer);

	bool								Connect(const Protocol::ServerInfo& ServerInfo, OUT FString& SocketError);
	void								Run();
	void								Disconnect();

	const Protocol::ServerInfo&			GetCurrentServerInfo();
	const Protocol::C_LOGIN&			GetLoginPkt();

	Protocol::C_LOGIN*					GetMutableLoginPkt();

public:
	float								Rtt;

	TQueue<TArray<uint8>>				RecvPacketQueue;
	TQueue<SendBufferRef>				SendPacketQueue;

	enum EState : uint8
	{
		EMPTY,
		READY,
		CONNECTED,
		VERIFIED,
		PLAYED
	}									State;

private:
	Protocol::ServerInfo*				_CurServerInfo;
	Protocol::C_LOGIN*					_LoginPkt;

	class FSocket*						_Socket;
	RecvWorkerRef						_RecvWorkerThread;
	SendWorkerRef						_SendWorkerThread;
};
