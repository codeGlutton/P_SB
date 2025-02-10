#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"

#include "Room.h"
#include "RoomManager.h"

/************************
	   GameSession
*************************/

void GameSession::OnConnected()
{
	GSessionManager->Add(std::static_pointer_cast<GameSession>(shared_from_this()));
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
