#include "pch.h"
#include "RoomManager.h"
#include "Room.h"

#include "Object.h"

#include "Protocol.pb.h"

RoomManager* GRoomManager = nullptr;

RoomManager::RoomManager()
{
	_connectionRoom = MakeXShared<ConnectionRoom>();
	_lobby = MakeXShared<LobbyRoom>();
}

void RoomManager::Init()
{
	_connectionRoom->DoAsync(&ConnectionRoom::UpdateServerDencityTick);
	_lobby->DoAsync(&LobbyRoom::UpdateRoomTick);
}

GameMatchRoomRef RoomManager::Create(const xString name)
{
	GameMatchRoomRef newRoom;
	{
		WRITE_LOCK;
		newRoom = MakeXShared<GameMatchRoom>(_matchRoomIdGenerator, name);
		_gameMatchRooms.insert({ _matchRoomIdGenerator, newRoom });
		_matchRoomIdGenerator++;
	}

	return newRoom;
}

void RoomManager::Remove(GameMatchRoomRef room)
{
	WRITE_LOCK;
	_gameMatchRooms.erase(room->id);
}

bool RoomManager::ProcessMapBytes(ObjectRef moveObject, uint64 mapBytes)
{
	RoomIdConvertor* mapIndex = reinterpret_cast<RoomIdConvertor*>(mapBytes);
	switch (mapIndex->type)
	{
	case Protocol::ROOM_TYPE_TEAMMATCH:
	{
		GameMatchRoomRef targetMatchRoom = GetGameMatchRoom(mapIndex->gameMatchIndex);
		if (targetMatchRoom == nullptr)
			return false;

		targetMatchRoom->DoAsync(&GameMatchRoom::ProcessMapBytes, moveObject, mapBytes);

		break;
	}
	case Protocol::ROOM_TYPE_LOBBY:
	{
		RoomRef nextRoom = GetLobby();
		if (nextRoom == nullptr)
			return false;
		RoomRef ownerRoom = moveObject->ownerRoom.load().lock();
		if (ownerRoom == nullptr)
			return false;

		ownerRoom->DoAsync(&Room::LeaveAndEnter, moveObject, nextRoom);

		break;
	}
	case Protocol::ROOM_TYPE_MEETING:
	{
		GameMatchRoomRef targetMatchRoom = GetGameMatchRoom(mapIndex->gameMatchIndex);
		if (targetMatchRoom == nullptr)
			return false;
		RoomRef nextRoom = targetMatchRoom->GetMeetingRoom();
		RoomRef ownerRoom = moveObject->ownerRoom.load().lock();
		if (ownerRoom == nullptr)
			return false;

		ownerRoom->DoAsync(&Room::LeaveAndEnter, moveObject, nextRoom);

		break;
	}
	}

	return true;
}

const GameMatchRoomRef RoomManager::GetGameMatchRoom(uint32 id)
{
	READ_LOCK;
	auto iter = _gameMatchRooms.find(id);
	if (iter == _gameMatchRooms.end())
	{
		return GameMatchRoomRef();
	}
	return iter->second;
}

void RoomManager::GetGameMatchRoomInfos(OUT Protocol::S_UPDATE_ROOM& infos)
{
	{
		READ_LOCK;
		for (auto& tmpRoomPair : _gameMatchRooms)
		{
			tmpRoomPair.second->MakeSelectInfo(*(infos.add_room_select_infos()));
		}
	}
}
