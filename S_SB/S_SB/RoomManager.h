#pragma once

namespace Protocol
{
	class S_UPDATE_ROOM;
}

USING_SHARED_PTR(LobbyRoom);
USING_SHARED_PTR(GameMatchRoom);

struct RoomIdConvertor
{
	RoomIdConvertor(uint64 inBytes)
	{
		bytes = inBytes;
	}

	union
	{
		struct
		{
			Protocol::RoomType		type			: 16;
			uint32					teamMatchIndex	: 16;
			uint32					gameMatchIndex	: 32;
		};
		uint64						bytes;
	};
};

class RoomManager
{
public:
	RoomManager();

	void							Init();

	GameMatchRoomRef				Create(const xString name);
	void							Remove(GameMatchRoomRef room);

	bool							ProcessMapBytes(ObjectRef moveObject, uint64 mapBytes);

	const LobbyRoomRef				GetLobby() { return _lobby; }
	const ConnectionRoomRef			GetConnectionRoom() { return _connectionRoom; }
	const GameMatchRoomRef			GetGameMatchRoom(uint32 id);

	void							GetGameMatchRoomInfos(OUT Protocol::S_UPDATE_ROOM& infos);

private:
	USE_LOCK;

	uint32							_matchRoomIdGenerator = 0;
	ConnectionRoomRef				_connectionRoom;
	LobbyRoomRef					_lobby;
	xMap<uint32, GameMatchRoomRef>	_gameMatchRooms;
};

extern RoomManager*					GRoomManager;
