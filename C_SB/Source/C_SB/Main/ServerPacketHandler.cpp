#include "ServerPacketHandler.h"
#include "C_SB.h"

#include "SBGameInstance.h"
#include "SBNetworkManager.h"
#include "PacketSession.h"

#include "SBWebNetworkManager.h"
#include "ServerHttpPacketHandler.h"

DEFINE_LOG_CATEGORY(LogTcpHandler);

/*************************
	ServerPacketHandler
*************************/

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& Session, BYTE* Buffer, int32 Len)
{
	PacketHeader* Header = reinterpret_cast<PacketHeader*>(Buffer);
	// TODO : 헤더 Log
	return false;
}

bool Handle_S_PING(PacketSessionRef& Session, Protocol::S_PING& Pkt)
{
	USBNetworkManager* GameNetworkManager = ServerPacketHandler::GetNetworkManager();
	if (GameNetworkManager == nullptr)
		return false;

	GameNetworkManager->HandlePingPong(Pkt);

	return true;
}

bool Handle_S_LOGIN(PacketSessionRef& Session, Protocol::S_LOGIN& Pkt)
{
	USBNetworkManager* GameNetworkManager = ServerPacketHandler::GetNetworkManager();
	if (GameNetworkManager == nullptr)
		return false;

	USBWebNetworkManager* WebNetworkManager = ServerHttpPacketHandler::GetWebNetworkManager();
	if (WebNetworkManager == nullptr)
		return false;

	switch (Pkt.result())
	{
	case Protocol::LOGIN_RESULT_SUCCESS:
	{
		GameNetworkManager->HandleLogin(Pkt);
		break;
	}
	case Protocol::LOGIN_RESULT_ERROR_FULL_SERVER:
	{
		GameNetworkManager->ErrorFromLoginPkt(TEXT("서버 인원 초과"));
		WebNetworkManager->RequestToRecheckServer();
		break;
	}
	case Protocol::LOGIN_RESULT_ERROR_INVALID_TOKEN:
	{
		GameNetworkManager->ErrorFromLoginPkt(TEXT("잘못된 토큰"), true);
		WebNetworkManager->ResponseToLogOut();
		break;
	}
	case Protocol::LOGIN_RESULT_ERROR_ACCOUNT_EXIST:
	{
		GameNetworkManager->ErrorFromLoginPkt(TEXT("중복 로그인"), true);
		WebNetworkManager->ResponseToLogOut();
		break;
	}
	}

	return true;
}

bool Handle_S_CREATE_PLAYER(PacketSessionRef& session, Protocol::S_CREATE_PLAYER& pkt)
{
	USBNetworkManager* GameNetworkManager = ServerPacketHandler::GetNetworkManager();
	if (GameNetworkManager == nullptr)
		return false;

	if (pkt.success() == false)
	{
		GameNetworkManager->ErrorFromCreatePkt(TEXT("잘못된 패킷"));
	}
	GameNetworkManager->HandleChangePlayer(pkt);

	return true;
}

bool Handle_S_DELETE_PLAYER(PacketSessionRef& session, Protocol::S_DELETE_PLAYER& pkt)
{
	USBNetworkManager* GameNetworkManager = ServerPacketHandler::GetNetworkManager();
	if (GameNetworkManager == nullptr)
		return false;

	if (pkt.success() == false)
	{
		GameNetworkManager->ErrorFromDeletePkt(TEXT("잘못된 패킷"));
	}
	GameNetworkManager->HandleChangePlayer(pkt);

	return true;
}

bool Handle_S_CHANGE_MAP(PacketSessionRef& Session, Protocol::S_CHANGE_MAP& Pkt)
{
	USBNetworkManager* GameNetworkManager = ServerPacketHandler::GetNetworkManager();
	if (GameNetworkManager == nullptr)
		return false;

	if (Pkt.success() == false)
	{
		// TODO : 맵 이동 에러 처리

		return false;
	}
	// TODO: NextMapId 로 이동
	// 방 이동 함수();
	GameNetworkManager->HandleSpawn(Pkt);

	return true;
}

bool Handle_S_LEAVE_GAME(PacketSessionRef& Session, Protocol::S_LEAVE_GAME& Pkt)
{
	USBNetworkManager* GameNetworkManager = ServerPacketHandler::GetNetworkManager();
	if (GameNetworkManager == nullptr)
		return false;

	GameNetworkManager->HandleLeave();

	return true;
}

bool Handle_S_SPAWN(PacketSessionRef& Session, Protocol::S_SPAWN& Pkt)
{
	USBNetworkManager* GameNetworkManager = ServerPacketHandler::GetNetworkManager();
	if (GameNetworkManager == nullptr)
		return false;

	GameNetworkManager->HandleSpawn(Pkt);

	return true;
}

bool Handle_S_DESPAWN(PacketSessionRef& Session, Protocol::S_DESPAWN& Pkt)
{
	USBNetworkManager* GameNetworkManager = ServerPacketHandler::GetNetworkManager();
	if (GameNetworkManager == nullptr)
		return false;

	GameNetworkManager->HandleDespawn(Pkt);

	return true;
}

bool Handle_S_UPDATE_ROOM(PacketSessionRef& session, Protocol::S_UPDATE_ROOM& pkt)
{
	return true;
}

bool Handle_S_SHOW_TEAM_MATCH(PacketSessionRef& session, Protocol::S_SHOW_TEAM_MATCH& pkt)
{
	return true;
}

bool Handle_S_INIT_TEAM_MATCH(PacketSessionRef& session, Protocol::S_INIT_TEAM_MATCH& pkt)
{
	return true;
}

bool Handle_S_UPDATE_TEAM_MATCH(PacketSessionRef& session, Protocol::S_UPDATE_TEAM_MATCH& pkt)
{
	return true;
}

bool Handle_S_MOVE(PacketSessionRef& Session, Protocol::S_MOVE& Pkt)
{
	USBNetworkManager* GameNetworkManager = ServerPacketHandler::GetNetworkManager();
	if (GameNetworkManager == nullptr)
		return false;

	GameNetworkManager->HandleMove(Pkt);

	return true;
}

bool Handle_S_CHAT(PacketSessionRef& Session, Protocol::S_CHAT& Pkt)
{
	std::cout << Pkt.msg() << std::endl;
	return true;
}

USBNetworkManager* const ServerPacketHandler::GetNetworkManager()
{
	if (const UGameInstance* GameInstance = Utils::GetGameInstance())
	{
		return GameInstance->GetSubsystem<USBNetworkManager>();
	}
	return nullptr;
}
