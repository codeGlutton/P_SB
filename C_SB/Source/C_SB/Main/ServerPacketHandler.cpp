#include "ServerPacketHandler.h"
#include "C_SB.h"

#include "SBGameInstance.h"
#include "SBNetworkManager.h"
#include "PacketSession.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& Session, BYTE* Buffer, int32 Len)
{
	PacketHeader* Header = reinterpret_cast<PacketHeader*>(Buffer);
	// TODO : 헤더 Log
	return false;
}

bool Handle_S_PING(PacketSessionRef& Session, Protocol::S_PING& Pkt)
{
	if (auto* GameNetworkManager = Session->Owner)
	{
		GameNetworkManager->Rtt = Pkt.rtt();
	}

	Protocol::C_PING PingPkt;
	SEND_C_PACKET(Session, PingPkt);

	return true;
}

bool Handle_S_LOGIN(PacketSessionRef& Session, Protocol::S_LOGIN& Pkt)
{
	if (Pkt.success() == false)
	{
		// TODO : 로그인 실패 에러처리

		return false;
	}

	// 캐릭터 없으면 생성창 이동
	if (Pkt.players().size() == 0)
	{
		// TODO : 생성창
	}

	for (auto& Player : Pkt.players())
	{
		FString NameUTF16;
		Utils::UTF8To16(Player.name(), NameUTF16);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("OBJ_ID[%llu] Name[%s]\n"), Player.object_id(), *NameUTF16));
	}

	// (임시) 캐릭터 0번 선택
	Protocol::C_ENTER_GAME EnterGamePkt;
	EnterGamePkt.set_player_index(0ull);

	SEND_C_PACKET(Session, EnterGamePkt);

	return true;
}

bool Handle_S_CHANGE_MAP(PacketSessionRef& Session, Protocol::S_CHANGE_MAP& Pkt)
{
	if (Pkt.success() == false)
	{
		// TODO : 맵 이동 에러 처리

		return false;
	}

	if (auto* GameNetworkManager = Session->Owner)
	{
		// TODO: NextMapId 로 이동
		// 방 이동 함수();
		GameNetworkManager->HandleSpawn(Pkt);
	}

	return true;
}

bool Handle_S_LEAVE_GAME(PacketSessionRef& Session, Protocol::S_LEAVE_GAME& Pkt)
{
	if (auto* GameNetworkManager = Session->Owner)
	{
		// TODO : 메인 메뉴 혹은 게임 종료
	}

	return true;
}

bool Handle_S_SPAWN(PacketSessionRef& Session, Protocol::S_SPAWN& Pkt)
{
	if (auto* GameNetworkManager = Session->Owner)
	{
		GameNetworkManager->HandleSpawn(Pkt);
	}

	return true;
}

bool Handle_S_DESPAWN(PacketSessionRef& Session, Protocol::S_DESPAWN& Pkt)
{
	if (auto* GameNetworkManager = Session->Owner)
	{
		GameNetworkManager->HandleDespawn(Pkt);
	}

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
	if (auto* GameNetworkManager = Session->Owner)
	{
		GameNetworkManager->HandleMove(Pkt);
	}

	return true;
}

bool Handle_S_CHAT(PacketSessionRef& Session, Protocol::S_CHAT& Pkt)
{
	std::cout << Pkt.msg() << std::endl;
	return true;
}
