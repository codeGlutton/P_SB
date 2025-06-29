#pragma once
#include "Protocol.pb.h"

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
#include "C_SB.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTcpHandler, Log, All);
#endif

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

// S_ : From Server, C_ : From Client
enum : uint16
{
	PKT_C_PING = 1000,
	PKT_S_PING = 1001,
	PKT_C_LOGIN = 1002,
	PKT_S_LOGIN = 1003,
	PKT_C_CREATE_PLAYER = 1004,
	PKT_S_CREATE_PLAYER = 1005,
	PKT_C_DELETE_PLAYER = 1006,
	PKT_S_DELETE_PLAYER = 1007,
	PKT_C_ENTER_GAME = 1008,
	PKT_C_CHANGE_MAP = 1009,
	PKT_S_CHANGE_MAP = 1010,
	PKT_C_LEAVE_GAME = 1011,
	PKT_S_LEAVE_GAME = 1012,
	PKT_S_SPAWN = 1013,
	PKT_S_DESPAWN = 1014,
	PKT_S_UPDATE_ROOM = 1015,
	PKT_C_SHOW_TEAM_MATCH = 1016,
	PKT_S_SHOW_TEAM_MATCH = 1017,
	PKT_S_INIT_TEAM_MATCH = 1018,
	PKT_S_UPDATE_TEAM_MATCH = 1019,
	PKT_C_MOVE = 1020,
	PKT_S_MOVE = 1021,
	PKT_C_CHAT = 1022,
	PKT_S_CHAT = 1023,
};

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_PING(PacketSessionRef& session, Protocol::C_PING& pkt);
bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt);
bool Handle_C_CREATE_PLAYER(PacketSessionRef& session, Protocol::C_CREATE_PLAYER& pkt);
bool Handle_C_DELETE_PLAYER(PacketSessionRef& session, Protocol::C_DELETE_PLAYER& pkt);
bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt);
bool Handle_C_CHANGE_MAP(PacketSessionRef& session, Protocol::C_CHANGE_MAP& pkt);
bool Handle_C_LEAVE_GAME(PacketSessionRef& session, Protocol::C_LEAVE_GAME& pkt);
bool Handle_C_SHOW_TEAM_MATCH(PacketSessionRef& session, Protocol::C_SHOW_TEAM_MATCH& pkt);
bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt);
bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt);

/*************************
	ClientPacketHandler
*************************/

class ClientPacketHandler
{
public:
	// (수신) 초기 핸들러 함수 연결
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_C_PING] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_PING>(Handle_C_PING, session, buffer, len); };
		GPacketHandler[PKT_C_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_LOGIN>(Handle_C_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_C_CREATE_PLAYER] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_CREATE_PLAYER>(Handle_C_CREATE_PLAYER, session, buffer, len); };
		GPacketHandler[PKT_C_DELETE_PLAYER] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_DELETE_PLAYER>(Handle_C_DELETE_PLAYER, session, buffer, len); };
		GPacketHandler[PKT_C_ENTER_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ENTER_GAME>(Handle_C_ENTER_GAME, session, buffer, len); };
		GPacketHandler[PKT_C_CHANGE_MAP] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_CHANGE_MAP>(Handle_C_CHANGE_MAP, session, buffer, len); };
		GPacketHandler[PKT_C_LEAVE_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_LEAVE_GAME>(Handle_C_LEAVE_GAME, session, buffer, len); };
		GPacketHandler[PKT_C_SHOW_TEAM_MATCH] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_SHOW_TEAM_MATCH>(Handle_C_SHOW_TEAM_MATCH, session, buffer, len); };
		GPacketHandler[PKT_C_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_MOVE>(Handle_C_MOVE, session, buffer, len); };
		GPacketHandler[PKT_C_CHAT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_CHAT>(Handle_C_CHAT, session, buffer, len); };
	}

	// (수신) 패킷 프로토콜 id로 해석 및 처리
	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_PING& pkt) { return MakeSendBuffer(pkt, PKT_S_PING); }
	static SendBufferRef MakeSendBuffer(Protocol::S_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_S_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CREATE_PLAYER& pkt) { return MakeSendBuffer(pkt, PKT_S_CREATE_PLAYER); }
	static SendBufferRef MakeSendBuffer(Protocol::S_DELETE_PLAYER& pkt) { return MakeSendBuffer(pkt, PKT_S_DELETE_PLAYER); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CHANGE_MAP& pkt) { return MakeSendBuffer(pkt, PKT_S_CHANGE_MAP); }
	static SendBufferRef MakeSendBuffer(Protocol::S_LEAVE_GAME& pkt) { return MakeSendBuffer(pkt, PKT_S_LEAVE_GAME); }
	static SendBufferRef MakeSendBuffer(Protocol::S_SPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_SPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_DESPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_DESPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_UPDATE_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_S_UPDATE_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::S_SHOW_TEAM_MATCH& pkt) { return MakeSendBuffer(pkt, PKT_S_SHOW_TEAM_MATCH); }
	static SendBufferRef MakeSendBuffer(Protocol::S_INIT_TEAM_MATCH& pkt) { return MakeSendBuffer(pkt, PKT_S_INIT_TEAM_MATCH); }
	static SendBufferRef MakeSendBuffer(Protocol::S_UPDATE_TEAM_MATCH& pkt) { return MakeSendBuffer(pkt, PKT_S_UPDATE_TEAM_MATCH); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_S_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_S_CHAT); }

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
	static class USBNetworkManager* const GetNetworkManager();
#endif

private:
	// (수신) 특정 핸들러 함수를 통해 패킷 역직렬화 후 처리
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	// (송신) 임의의 전송 프로토콜 버퍼를 통해 제작된 직렬화 데이터에 헤더를 씌어 SendBuffer로 넘겨주는 함수
	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);

		/* 패킷 헤더 입력 */

		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;

		// 프로토콜 버퍼 직렬화 
#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
		check(pkt.SerializeToArray(&header[1], dataSize));
#else
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
#endif

		sendBuffer->Close(packetSize);
		return sendBuffer;
	}
};