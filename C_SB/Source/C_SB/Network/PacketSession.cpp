#include "PacketSession.h"
#include "C_SB.h"

#include "NetworkWorker.h"
#include "ServerPacketHandler.h"

#include "Sockets.h"
#include "Common/TcpSocketBuilder.h"
#include "Serialization/ArrayWriter.h"
#include "SocketSubsystem.h"

#include "SBNetworkManager.h"

/************************
	  PacketSession
*************************/

PacketSession::PacketSession() : State(EState::READY)
{
	_CurServerInfo = xnew<Protocol::ServerInfo>();
	_LoginPkt = xnew<Protocol::C_LOGIN>();
	_Socket = nullptr;
}

PacketSession::~PacketSession()
{
	Disconnect();

	xdelete(_CurServerInfo);
	_CurServerInfo = nullptr;
	xdelete(_LoginPkt);
	_LoginPkt = nullptr;
}

void PacketSession::HandleRecvPackets()
{
	while (true)
	{
		TArray<uint8> Packet;
		if (RecvPacketQueue.Dequeue(OUT Packet) == false)
		{
			break;
		}
		if (Packet.Num() == 0)
		{
			// 게임 서버에서 Disconnect로 강제 종료
			UKismetSystemLibrary::QuitGame(Utils::GetWorld(), nullptr, EQuitPreference::Quit, false);
			break;
		}

		PacketSessionRef ThisPtr = AsShared();
		ServerPacketHandler::HandlePacket(ThisPtr, Packet.GetData(), Packet.Num());
	}
}

void PacketSession::SendPacket(SendBufferRef SendBuffer)
{
	SendPacketQueue.Enqueue(SendBuffer);
}

bool PacketSession::Connect(const Protocol::ServerInfo& ServerInfo, OUT FString& SocketError)
{
	if (State != EState::READY)
	{
		SocketError = TEXT("State Error");
		return false;
	}

	_CurServerInfo->CopyFrom(ServerInfo);

	FString IpAddress;
	Utils::UTF8To16(_CurServerInfo->ip_address(), OUT IpAddress);
	int16 Port = std::stoi(_CurServerInfo->port());

	FSocket* NewSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(TEXT("Stream"), TEXT("Client Socket"));

	FIPv4Address Ip;
	FIPv4Address::Parse(IpAddress, Ip);

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	TSharedRef<FInternetAddr> InternetAddr = SocketSubsystem->CreateInternetAddr();
	InternetAddr->SetIp(Ip.Value);
	InternetAddr->SetPort(Port);

	if (NewSocket->Connect(*InternetAddr) == false)
	{
		SocketError = FString(SocketSubsystem->GetSocketError(SocketSubsystem->GetLastErrorCode()));
		return false;
	}

	_Socket = NewSocket;
	State = EState::CONNECTED;

	return true;
}

void PacketSession::Run()
{
	if (State != EState::CONNECTED)
		return;

	check(_Socket->GetConnectionState() == ESocketConnectionState::SCS_Connected);

	_RecvWorkerThread = MakeXShared<RecvWorker>(_Socket, AsShared());
	_SendWorkerThread = MakeXShared<SendWorker>(_Socket, AsShared());
}

void PacketSession::Disconnect()
{
	if (_RecvWorkerThread != nullptr)
	{
		_RecvWorkerThread->Destroy();
		_RecvWorkerThread->GetThread()->WaitForCompletion();

		/* 초기화 */

		_RecvWorkerThread = nullptr;
		RecvPacketQueue.Empty();
	}

	if (_SendWorkerThread != nullptr)
	{
		_SendWorkerThread->Destroy();
		_SendWorkerThread->GetThread()->WaitForCompletion();

		/* 초기화 */

		_SendWorkerThread = nullptr;
		SendPacketQueue.Empty();
	}

	if (_Socket != nullptr)
	{
		ISocketSubsystem::Get()->DestroySocket(_Socket);
		_Socket = nullptr;
	}

	if (_CurServerInfo != nullptr)
	{
		_CurServerInfo->Clear();
	}

	State = EState::READY;
}

const Protocol::ServerInfo& PacketSession::GetCurrentServerInfo()
{
	return *_CurServerInfo;
}

const Protocol::C_LOGIN& PacketSession::GetLoginPkt()
{
	return *_LoginPkt;
}

Protocol::C_LOGIN* PacketSession::GetMutableLoginPkt()
{
	return _LoginPkt;
}
