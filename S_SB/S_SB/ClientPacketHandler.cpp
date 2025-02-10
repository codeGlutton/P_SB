#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"

#include "Player.h"
#include "Room.h"
#include "RoomManager.h"

#include "MovementComponent.h"

#include "DataUtils.h"

// 프로토콜에 따른 핸들러 함수 배열
PacketHandlerFunc GPacketHandler[UINT16_MAX];

/* 실제 프로토콜별 작업 함수 */

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : 헤더 Log
	return false;
}

bool Handle_C_PING(PacketSessionRef& session, Protocol::C_PING& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	GRoomManager->GetConnectionRoom()->RestartHeartBeat(gameSession);

	return true;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);

	// TODO : Validation 체크

	// TODO : id 해쉬 값 전달 받기
	//GRoomManager->GetConnectionRoom()->DoAsync(&ConnectionRoom::FindDBData, gameSession, pkt.id());

	return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	uint64 index = pkt.player_index();

	// TODO : Validation 체크

	GRoomManager->GetConnectionRoom()->DoAsync(&ConnectionRoom::SelectPlayer, gameSession->playerDataProtector, index);

	return true;
}

bool Handle_C_CHANGE_MAP(PacketSessionRef& session, Protocol::C_CHANGE_MAP& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	ObjectRef currentObject = std::static_pointer_cast<Object>(gameSession->playerDataProtector->currentPlayer.load());
	if (currentObject == nullptr)
		return false;
	
	return GRoomManager->ProcessMapBytes(currentObject, pkt.map_id());
}

bool Handle_C_LEAVE_GAME(PacketSessionRef& session, Protocol::C_LEAVE_GAME& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	GRoomManager->GetConnectionRoom()->DoAsync(&ConnectionRoom::DeleteLocalData, gameSession->playerDataProtector);

	return true;
}

bool Handle_C_SHOW_TEAM_MATCH(PacketSessionRef& session, Protocol::C_SHOW_TEAM_MATCH& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	PlayerRef currentPlayer = gameSession->playerDataProtector->currentPlayer.load();
	if (currentPlayer == nullptr)
		return false;
	GameMatchRoomRef currentMatchRoom = currentPlayer->ownerMatchRoom.load().lock();
	if (currentMatchRoom == nullptr)
		return false;

	currentMatchRoom->DoAsync(&GameMatchRoom::ShowTeamMatchs, currentPlayer);

	return true;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	ObjectRef currentObject = std::static_pointer_cast<Object>(gameSession->playerDataProtector->currentPlayer.load());
	if (currentObject == nullptr)
		return false;
	if (currentObject->FindComponent<MovementComponent>() == nullptr)
		return false;
	RoomRef ownerRoom = currentObject->ownerRoom.load().lock();
	if (ownerRoom == nullptr)
		return false;

	ownerRoom->DoAsync(&Room::MoveObject, currentObject, pkt);

	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	PlayerRef currentPlayer = gameSession->playerDataProtector->currentPlayer.load();
	if (currentPlayer == nullptr)
		return false;
	RoomRef ownerRoom = currentPlayer->ownerRoom.load().lock();
	if (ownerRoom == nullptr)
		return false;

	GConsoleLogger->WriteStdOut(Color::WHITE, L"[%s] : %s\n", currentPlayer->playerDetailInfo->name(), pkt.msg());

	Protocol::S_CHAT chatPkt;
	chatPkt.set_msg(pkt.msg());

	BROADCAST_S_PACKET(ownerRoom, chatPkt);

	return true;
}
