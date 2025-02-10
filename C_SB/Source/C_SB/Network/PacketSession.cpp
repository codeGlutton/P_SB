#include "PacketSession.h"
#include "C_SB.h"

#include "NetworkWorker.h"
#include "ServerPacketHandler.h"
#include "Sockets.h"
#include "SBNetworkManager.h"

/************************
	  PacketSession
*************************/

PacketSession::PacketSession(USBNetworkManager* Owner, FSocket* Socket) : Owner(Owner), Socket(Socket)
{
}

PacketSession::~PacketSession()
{
	Disconnect();
}

void PacketSession::HandleRecvPackets()
{
	while (true)
	{
		TArray<uint8> Packet;
		if (RecvPacketQueue.Dequeue(OUT Packet) == false)
			break;

		PacketSessionRef ThisPtr = AsShared();
		ServerPacketHandler::HandlePacket(ThisPtr, Packet.GetData(), Packet.Num());
	}
}

void PacketSession::SendPacket(SendBufferRef SendBuffer)
{
	SendPacketQueue.Enqueue(SendBuffer);
}

void PacketSession::Run()
{
	check(Socket->GetConnectionState() == ESocketConnectionState::SCS_Connected);

	RecvWorkerThread = MakeXShared<RecvWorker>(Socket, AsShared());
	SendWorkerThread = MakeXShared<SendWorker>(Socket, AsShared());

	// Å×½ºÆ®
	{
		Protocol::C_LOGIN Pkt;

		SEND_C_PACKET(this, Pkt);
	}
}

void PacketSession::Disconnect()
{
	if (RecvWorkerThread)
	{
		RecvWorkerThread->Destroy();
		RecvWorkerThread = nullptr;
	}

	if (SendWorkerThread)
	{
		SendWorkerThread->Destroy();
		SendWorkerThread = nullptr;
	}
}

void PacketSession::OnConnected()
{
	
}
