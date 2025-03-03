#pragma once

#ifdef _DEBUG
#pragma comment(lib, "S_Core\\Debug\\S_Core.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#pragma comment(lib, "Hiredis\\Debug\\hiredis.lib")
#pragma comment(lib, "RedisCpp\\Debug\\redis++_static.lib")
#pragma comment(lib, "Openssl\\Debug\\capi.lib")
#pragma comment(lib, "Openssl\\Debug\\dasync.lib")
#pragma comment(lib, "Openssl\\Debug\\libcrypto.lib")
#pragma comment(lib, "Openssl\\Debug\\libssl.lib")
#pragma comment(lib, "Openssl\\Debug\\openssl.lib")
#pragma comment(lib, "Openssl\\Debug\\ossltest.lib")
#pragma comment(lib, "Openssl\\Debug\\padlock.lib")
#else
#pragma comment(lib, "S_Core\\Release\\S_Core.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#pragma comment(lib, "Hiredis\\Release\\hiredis.lib")
#pragma comment(lib, "RedisCpp\\Release\\redis++_static.lib")
#pragma comment(lib, "Openssl\\Release\\capi.lib")
#pragma comment(lib, "Openssl\\Release\\dasync.lib")
#pragma comment(lib, "Openssl\\Release\\libcrypto.lib")
#pragma comment(lib, "Openssl\\Release\\libssl.lib")
#pragma comment(lib, "Openssl\\Release\\openssl.lib")
#pragma comment(lib, "Openssl\\Release\\ossltest.lib")
#pragma comment(lib, "Openssl\\Release\\padlock.lib")
#endif

#include "CorePch.h"
#include "Struct.pb.h"
#include "Utils.h"

/* Protobuf 전송 */

#define SEND_S_PACKET(session, pkt)										\
{																		\
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);			\
	session->Send(sendBuffer);											\
}

#define BROADCAST_S_PACKET(room, pkt)									\
{																		\
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);			\
	room->DoAsync(&Room::Broadcast, sendBuffer);						\
}

#define EXCEPTIONAL_BROADCAST_S_PACKET(room, pkt, exceptionId)			\
{																		\
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);			\
	room->DoAsync(&Room::ExceptionalBroadcast, sendBuffer, exceptionId);\
}

/* SharedPtr 타입형 */

USING_SHARED_PTR(Object);
USING_SHARED_PTR(Component);
USING_SHARED_PTR(Player);
USING_SHARED_PTR(GameSession);
USING_SHARED_PTR(PlayerDataProtector);
USING_SHARED_PTR(RoomBase);
USING_SHARED_PTR(Room);
USING_SHARED_PTR(ConnectionRoom);