#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"

#include "Player.h"
#include "Room.h"
#include "RoomManager.h"

#include "DBTaskExecutor.h"
#include "DBManager.h"

#include "MovementComponent.h"

#include "DataUtils.h"

/*************************
	ClientPacketHandler
*************************/

// 프로토콜에 따른 핸들러 함수 배열
PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool CheckVerifiedClient(const GameSessionRef& session)
{
	return session->playerDataProtector->isVerified.load();
}

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

	GRoomManager->GetConnectionRoom()->DoAsync(&ConnectionRoom::RestartHeartBeat, gameSession);

	return true;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	
	GRoomManager->GetConnectionRoom()->DoAsync(&ConnectionRoom::TryToVerification, gameSession, pkt.account_id(), xString(pkt.token_value()));

	return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	if (CheckVerifiedClient(gameSession))
		return false;

	uint64 index = pkt.player_index();

	// TODO : Validation 체크

	GRoomManager->GetConnectionRoom()->DoAsync(&ConnectionRoom::SelectPlayer, gameSession->playerDataProtector, index);

	return true;
}

bool Handle_C_CHANGE_MAP(PacketSessionRef& session, Protocol::C_CHANGE_MAP& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	if (CheckVerifiedClient(gameSession))
		return false;

	ObjectRef currentObject = std::static_pointer_cast<Object>(gameSession->playerDataProtector->currentPlayer.load());
	if (currentObject == nullptr)
		return false;
	
	return GRoomManager->ProcessMapBytes(currentObject, pkt.map_id());
}

bool Handle_C_LEAVE_GAME(PacketSessionRef& session, Protocol::C_LEAVE_GAME& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	if (CheckVerifiedClient(gameSession))
		return false;

	// 클라이언트에게 종료 알림
	{
		Protocol::S_LEAVE_GAME leaveGamePkt;

		SEND_S_PACKET(session, leaveGamePkt);
	}

	session->Disconnect(L"Reserved disconnection");
	//GRoomManager->GetConnectionRoom()->DoTimer(2000ull, ([session] { session->Disconnect(L"Reserved disconnection"); }));

	return true;
}

bool Handle_C_SHOW_TEAM_MATCH(PacketSessionRef& session, Protocol::C_SHOW_TEAM_MATCH& pkt)
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(session);
	if (CheckVerifiedClient(gameSession))
		return false;

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
	if (CheckVerifiedClient(gameSession))
		return false;

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
	if (CheckVerifiedClient(gameSession))
		return false;

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
