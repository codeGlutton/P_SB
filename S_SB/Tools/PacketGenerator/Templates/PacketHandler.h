#pragma once
#include "Protocol.pb.h"

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
#include "C_SB.h"
#endif

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

// S_ : From Server, C_ : From Client
enum : uint16
{
{%- for pkt in parser.total_pkt %}
	PKT_{{pkt.name}} = {{pkt.id}},
{%- endfor %}
};

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
{%- for pkt in parser.recv_pkt %}
bool Handle_{{pkt.name}}(PacketSessionRef& session, Protocol::{{pkt.name}}& pkt);
{%- endfor %}

/*************************
	{{output}}
*************************/

class {{output}}
{
public:
	// (수신) 초기 핸들러 함수 연결
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;

{%- for pkt in parser.recv_pkt %}
		GPacketHandler[PKT_{{pkt.name}}] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::{{pkt.name}}>(Handle_{{pkt.name}}, session, buffer, len); };
{%- endfor %}
	}

	// (수신) 패킷 프로토콜 id로 해석 및 처리
	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}

{%- for pkt in parser.send_pkt %}
	static SendBufferRef MakeSendBuffer(Protocol::{{pkt.name}}& pkt) { return MakeSendBuffer(pkt, PKT_{{pkt.name}}); }
{%- endfor %}

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