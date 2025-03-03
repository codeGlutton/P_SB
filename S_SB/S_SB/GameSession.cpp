#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"

#include "Room.h"
#include "RoomManager.h"

/************************
	   GameSession
*************************/

const uint64 GameSession::AUTH_TIMEOUT_MS = 10000ull;

void GameSession::OnConnected()
{
	GameSessionRef gameSession = std::static_pointer_cast<GameSession>(shared_from_this());
	GSessionManager->Add(gameSession);
	GRoomManager->GetConnectionRoom()->DoTimer(AUTH_TIMEOUT_MS, ([gameSession] {
		if (gameSession->playerDataProtector->isVerified.load() == false)
			gameSession->Disconnect(L"Authentication timeout");
		})
	);
}

void GameSession::OnDisconnected()
{
	GSessionManager->Remove(std::static_pointer_cast<GameSession>(shared_from_this()));
	GRoomManager->GetConnectionRoom()->DoAsync(&ConnectionRoom::DeleteLocalData, playerDataProtector);
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	
	// TODO : 통신 서버 확인 절차를 위해 header id 영역대 확인 필요

	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{
}
