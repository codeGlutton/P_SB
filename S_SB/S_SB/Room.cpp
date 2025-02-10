#include "pch.h"
#include "Room.h"
#include "RoomManager.h"

#include "Athlete.h"
#include "Player.h"

#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "ObjectUtils.h"

#include "MovementComponent.h"

#include "DBManager.h"
#include "DBTaskExecutor.h"

#include "AthleteInstance.h"
#include "TeamInstance.h"
#include "TeamMatchInstance.h"
#include "LeagueInstance.h"

USING_SHARED_PTR(MovementComponent);

/*********************
	   RoomBase
*********************/

RoomBaseRef RoomBase::GetRoomBaseRef()
{
	return std::static_pointer_cast<RoomBase>(shared_from_this());
}

void RoomBase::Broadcast(SendBufferRef sendBuffer)
{
	for (auto& object : _objects)
	{
		if (object.second->IsPlayer() == false)
			continue;
		PlayerRef player = std::static_pointer_cast<Player>(object.second);
		player->ownerSession->Send(sendBuffer);
	}
}

void RoomBase::ExceptionalBroadcast(SendBufferRef sendBuffer, uint64 exceptionId)
{
	for (auto& object : _objects)
	{
		if (object.second->IsPlayer() == false)
			continue;
		PlayerRef player = std::static_pointer_cast<Player>(object.second);
		if (player->objectBaseInfo->object_id() == exceptionId)
			continue;

		player->ownerSession->Send(sendBuffer);
	}
}

/*********************
		Room
*********************/

RoomRef Room::GetRoomRef()
{
	return std::static_pointer_cast<Room>(shared_from_this());
}

bool Room::Enter(ObjectRef object, bool randPos)
{
	if (_objects.find(object->objectBaseInfo->object_id()) != _objects.end())
		return false;

	// 랜덤 위치
	if (randPos)
	{
		object->posInfo->set_x(Utils::GetRandom(0.f, 500.f));
		object->posInfo->set_y(Utils::GetRandom(0.f, 500.f));
		object->posInfo->set_z(0.f);
		object->posInfo->set_pitch(0.f);
		object->posInfo->set_yaw(Utils::GetRandom(0.f, 100.f));
		object->posInfo->set_roll(0.f);
	}

	if (object->IsPlayer() == true)
	{
		PlayerRef player = std::static_pointer_cast<Player>(object);
		GameSessionRef session = player->ownerSession;

		// 대상 클라에 방 이동 및 본인 캐릭터 스폰 명령
		{
			Protocol::S_CHANGE_MAP changeMapPkt;
			changeMapPkt.set_success(true);
			// TODO: 로비 맵 아이디 넣어주기
			//changeMapPkt.set_map_id(lobbyId);
			Protocol::ObjectInfo* objectInfo = changeMapPkt.mutable_object_info();
			player->MakeObjectInfo(*objectInfo);

			SEND_S_PACKET(session, changeMapPkt);
		}

		// 대상 클라에 타 오브젝트들 복제
		{
			Protocol::S_SPAWN spawnPkt;
			for (auto& o : _objects)
			{
				Protocol::ObjectInfo* spawnObject = spawnPkt.add_object_infos();
				o.second->MakeObjectInfo(*spawnObject);
			}

			SEND_S_PACKET(session, spawnPkt);
		}
	}

	_objects[object->objectBaseInfo->object_id()] = object;

	// 나머지 클라이언트들에게 입장한 오브젝트 복제
	{
		Protocol::S_SPAWN spawnPkt;

		Protocol::ObjectInfo* spawnObject = spawnPkt.add_object_infos();
		object->MakeObjectInfo(*spawnObject);

		EXCEPTIONAL_BROADCAST_S_PACKET(this, spawnPkt, object->objectBaseInfo->object_id());
	}

	return true;
}

bool Room::Leave(ObjectRef object)
{
	if (_objects.find(object->objectBaseInfo->object_id()) == _objects.end())
		return false;
	_objects.erase(object->objectBaseInfo->object_id());

	const uint64 objectId = object->objectBaseInfo->object_id();

	// 방 참여자들에게 특정 클라이언트 캐릭터 방 나감을 알림
	{
		Protocol::S_DESPAWN despawnPkt;
		despawnPkt.add_object_id(objectId);
		BROADCAST_S_PACKET(this, despawnPkt);
	}

	return true;
}

void Room::LeaveAndEnter(ObjectRef object, RoomRef nextRoom)
{
	if (_objects.find(object->objectBaseInfo->object_id()) == _objects.end())
		return;

	/* 이동을 원하는 방과의 경로 파악 */

	xQueue<RoomBaseRef> leavePath;
	xStack<RoomBaseRef> enterPath;
	FindPathToRoom(nextRoom, leavePath, enterPath);

	// 실제 방 퇴장 처리 시작
	LeaveAlongPath(object, leavePath.size(), enterPath);
}

void Room::LeaveToRoot(ObjectRef object)
{
	if (Leave(object) == false)
		return;

	/* 부모의 방도 연속해서 나가기 */

	RoomBaseRef parentRoom = _parentRoom.lock();
	if (_parentRoom.lock() == nullptr)
		return;

	parentRoom->DoAsync(&RoomBase::LeaveToRoot, object);
}

void Room::LeaveAlongPath(ObjectRef object, uint8 leaveCount, xStack<RoomBaseRef> enterPath)
{
	if (Leave(object) == false)
		return;

	// 나가야하는 방은 모두 나간 경우
	if (leaveCount == 0)
	{
		/* Player의 현재 소속 방을 나타내는 ownerRoom을 변경 */

		auto connectionRoom = GRoomManager->GetConnectionRoom();
		connectionRoom->DoAsync(&ConnectionRoom::ChangeRoomData, object, enterPath);
	}
	else
	{
		/* 부모의 방도 연속해서 나가기 */

		RoomBaseRef nextRoom = _parentRoom.lock();
		if (nextRoom == nullptr)
			return;

		nextRoom->DoAsync(&RoomBase::LeaveAlongPath, object, --leaveCount, enterPath);
	}
}

void Room::EnterAlongPath(ObjectRef object, xStack<RoomBaseRef> enterPath)
{
	if (IsOpen() == false)
	{
		// 방 닫힘으로 인해 이동 실패 전송
		if (object->IsPlayer() == true)
		{
			Protocol::S_CHANGE_MAP changeMapPkt;
			changeMapPkt.set_success(false);
			SEND_S_PACKET(std::static_pointer_cast<Player>(object)->ownerSession, changeMapPkt);
		}

		/* 대안으로 로비 방으로 이동 */

		uint8 tmpLeaveCount;
		xStack<RoomBaseRef> tmpEnterPath;
		GetRollbackPath(OUT tmpLeaveCount, OUT tmpEnterPath);
		LeaveAlongPath(object, tmpLeaveCount, tmpEnterPath);

		return;
	}

	if (Enter(object, false) == false)
		return;
}

void Room::MoveObject(ObjectRef object, Protocol::C_MOVE pkt)
{
	const uint64 objectId = object->objectBaseInfo->object_id();
	if (_objects.find(objectId) == _objects.end())
		return;

	MovementComponentRef movementComponent = object->FindComponent<MovementComponent>();
	if (movementComponent == nullptr)
		return;

	// TODO : 이동 변화량 검사

	object->posInfo->CopyFrom(pkt.pos_info());

	Protocol::S_MOVE movePkt;
	movePkt.set_object_id(objectId);
	movePkt.mutable_pos_info()->CopyFrom(*object->posInfo);
	movePkt.mutable_dest_info()->CopyFrom(pkt.dest_info());
	movePkt.set_move_state(*movementComponent->state);

	BROADCAST_S_PACKET(this, movePkt);
}

void Room::FindPathToRoom(RoomRef& targetRoom, OUT xQueue<RoomBaseRef>& leavePath, OUT xStack<RoomBaseRef>& enterPath)
{
	leavePath.push(GetRoomRef());
	enterPath.push(targetRoom);

	for (uint8 levelDelta; leavePath.front()->GetParentRoom() != enterPath.top()->GetParentRoom(); )
	{
		levelDelta = leavePath.front()->GetLevel() - enterPath.top()->GetLevel();
		if (levelDelta >= 0)
		{
			leavePath.push(leavePath.front()->GetParentRoom());
		}
		if (levelDelta <= 0)
		{
			enterPath.push(enterPath.top()->GetParentRoom());
		}
	}

	leavePath.pop();
}

/*********************
	ConnectionRoom
*********************/

const float ConnectionRoom::MAX_PING = 500.f;

void ConnectionRoom::FindDBData(GameSessionRef gameSession, int32 dbId)
{
	PlayerDataProtectorRef playerData = gameSession->playerDataProtector;
	if (playerData->_state != PlayerDataProtector::STATE::EMPTY)
		return;

	// DB Request 전송
	DBPlayerTaskExecutorRef playerTaskExecutor = GDBManager->GetPlayerTaskExecutor();
	playerTaskExecutor->DoAsync(&DBPlayerTaskExecutor::GetSelectablePlayers, gameSession, dbId);
}

void ConnectionRoom::DeliverDBData(GameSessionRef gameSession, xVector<int32> ids, xVector<xString> names, xVector<TIMESTAMP_STRUCT> dates)
{
	PlayerDataProtectorRef playerData = gameSession->playerDataProtector;
	if (playerData->_state != PlayerDataProtector::STATE::EMPTY)
		return;

	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);

	for (int32 i = 0; i < ids.size(); i++)
	{
		{
			/* 캐릭터 정보 저장 (휘발성) */

			PlayerRef playerRef = ObjectUtils::CreatedPlayer(gameSession, ids[0]);
			playerData->_players.push_back(playerRef);
			playerRef->playerDetailInfo->set_name(std::string(names[0]));

			// TODO : 초기 로비 위치 설정
			playerRef->posInfo->set_x(0.f);
			playerRef->posInfo->set_y(0.f);
			playerRef->posInfo->set_z(0.f);
			playerRef->posInfo->set_pitch(0.f);
			playerRef->posInfo->set_yaw(0.f);
			playerRef->posInfo->set_roll(0.f);

			/* 긁어온 캐릭터 정보 패킷에 넣어주기 */

			auto player = loginPkt.add_players();
			player->set_object_id(playerRef->objectBaseInfo->object_id());
			player->set_name(playerRef->playerDetailInfo->name());
		}
	}

	playerData->_state = PlayerDataProtector::STATE::LOADED;
	SEND_S_PACKET(gameSession, loginPkt);
}

void ConnectionRoom::SelectPlayer(PlayerDataProtectorRef playerData, uint64 index)
{
	if (playerData->_state != PlayerDataProtector::STATE::LOADED)
		return;

	PlayerRef currentPlayer = playerData->_players[index];
	playerData->currentPlayer.store(currentPlayer);
	playerData->_players.clear();
	if (Enter(currentPlayer) == false)
		return;

	LobbyRoomRef lobbyRoom = GRoomManager->GetLobby();
	currentPlayer->ownerRoom.store(std::weak_ptr<Room>(lobbyRoom));

	ObjectRef currentObject = std::static_pointer_cast<Object>(currentPlayer);
	lobbyRoom->DoAsync(&LobbyRoom::Enter, currentObject, false);

	playerData->_state = PlayerDataProtector::STATE::FULL;
	StartHeartBeat(currentPlayer->ownerSession);
}

void ConnectionRoom::DeleteLocalData(PlayerDataProtectorRef playerData)
{
	if (playerData->_state == PlayerDataProtector::STATE::DELETED)
		return;
	
	/* 순환 참조를 끊기 위해 룸의 Player Contanier의 멤버를 제거 */

	PlayerRef currentPlayer = playerData->currentPlayer.exchange(nullptr);
	if (currentPlayer)
	{
		if (Leave(currentPlayer) == false)
			return;

		RoomRef room = currentPlayer->ownerRoom.exchange(std::weak_ptr<Room>()).lock();
		if (room)
		{
			ObjectRef currentObject = std::static_pointer_cast<Object>(currentPlayer);
			room->DoAsync(&Room::LeaveToRoot, currentObject);
		}

		// 클라이언트에게 종료 알림
		{
			Protocol::S_LEAVE_GAME leaveGamePkt;
			leaveGamePkt.set_object_id(currentPlayer->objectBaseInfo->object_id());
			SEND_S_PACKET(currentPlayer->ownerSession, leaveGamePkt);
		}
	}

	/* 순환 참조를 끊기 위해 멤버 비우기 */ 

	playerData->_players.clear();
	playerData->_state = PlayerDataProtector::STATE::DELETED;
}

void ConnectionRoom::StartHeartBeat(GameSessionRef gameSession)
{
	PlayerDataProtectorRef& playerData = gameSession->playerDataProtector;
	if (playerData->_state != PlayerDataProtector::STATE::FULL)
		return;
	if (playerData->_isMeasuringPing == true)
		return;
	playerData->_isMeasuringPing = true;
	playerData->_startPingTime = std::chrono::high_resolution_clock::now();

	Protocol::S_PING pingPkt;
	pingPkt.set_rtt(playerData->rtt);

	SEND_S_PACKET(gameSession, pingPkt);
}

void ConnectionRoom::RestartHeartBeat(GameSessionRef gameSession)
{
	PlayerDataProtectorRef& playerData = gameSession->playerDataProtector;
	if (playerData->_isMeasuringPing == false)
		return;

	std::chrono::duration<float, std::milli> rtt = std::chrono::high_resolution_clock::now() - playerData->_startPingTime;

	if (playerData->rtt < MAX_PING)
	{
		if (playerData->rtt <= 0.f)
		{
			playerData->rtt = rtt.count();
		}
		else
		{
			playerData->rtt = 0.75f * playerData->rtt + 0.25f * rtt.count();
		}
	}
	playerData->_isMeasuringPing = false;

	this->DoTimer(500llu, &ConnectionRoom::StartHeartBeat, gameSession);
}

void ConnectionRoom::ChangeRoomData(ObjectRef object, xStack<RoomBaseRef> enterPath)
{
	if (object->IsPlayer() == false)
		return;
	PlayerRef player = std::static_pointer_cast<Player>(object);

	GameSessionRef gameSession = player->ownerSession;
	if (gameSession == nullptr)
		return;
	if (gameSession->playerDataProtector->_state != PlayerDataProtector::STATE::FULL)
		return;

	/* 방 정보 교환 */ 

	RoomRef targetRoom = std::static_pointer_cast<Room>(enterPath._Get_container().front());
	if (targetRoom == nullptr)
		return;
	RoomRef room = player->ownerRoom.exchange(std::weak_ptr<Room>(targetRoom)).lock();
	if (room == nullptr)
		return;

	/* 방 입장 시작 */

	RoomBaseRef nextRoom = enterPath.top();
	enterPath.pop();
	nextRoom->DoAsync(&RoomBase::EnterAlongPath, object, enterPath);
}

void ConnectionRoom::ChangeMatchRoomData(ObjectRef object, GameMatchRoomRef nextRoom)
{
	if (object->IsPlayer() == false)
		return;
	PlayerRef player = std::static_pointer_cast<Player>(object);

	GameSessionRef gameSession = player->ownerSession;
	if (gameSession == nullptr)
		return;
	if (gameSession->playerDataProtector->_state != PlayerDataProtector::STATE::FULL)
		return;

	// 게임 방 정보 교환
	player->ownerMatchRoom.exchange(nextRoom);
}

bool ConnectionRoom::Enter(ObjectRef object)
{
	if (object->IsPlayer() == false)
		return false;
	if (_objects.find(object->objectBaseInfo->object_id()) != _objects.end())
		return false;
	_objects[object->objectBaseInfo->object_id()] = object;
	return true;
}

bool ConnectionRoom::Leave(ObjectRef object)
{
	if (object->IsPlayer() == false)
		return false;
	if (_objects.find(object->objectBaseInfo->object_id()) == _objects.end())
		return false;
	_objects.erase(object->objectBaseInfo->object_id());
	return true;
}

/*********************
	 GameMatchRoom
*********************/

const uint8 GameMatchRoom::MAX_PLAYER = 8;

GameMatchRoom::GameMatchRoom(const uint32 id, const xString name) : RoomBase(), id(id), name(name), _isClosure(false), _teamIdGenerator(0ul)
{
	_meetingRoom = MakeXShared<MeetingRoom>(std::static_pointer_cast<GameMatchRoom>(GetRoomBaseRef()));
	_leagueInstance = MakeXShared<LeagueInstance>();

	_playerCount.store(0);
}

const uint8 GameMatchRoom::GetPlayerCount()
{
	return _playerCount.load();
}

void GameMatchRoom::MakeSelectInfo(OUT Protocol::RoomSelectInfo& info)
{
	info.set_id(id);
	info.set_name(name.c_str());
	info.set_count(_playerCount.load());
}

const MeetingRoomRef GameMatchRoom::GetMeetingRoom()
{
	return _meetingRoom;
}

const TeamMatchRoomRef GameMatchRoom::GetTeamMatchRoom(uint32 id)
{
	auto iter = _teamMatchRooms.find(id);
	if (iter == _teamMatchRooms.end())
	{
		return TeamMatchRoomRef();
	}
	return iter->second;
}

bool GameMatchRoom::Enter(ObjectRef object)
{
	if (object->IsPlayer() == false)
		return false;
	if (_objects.find(object->objectBaseInfo->object_id()) != _objects.end())
		return false;

	GRoomManager->GetConnectionRoom()->DoAsync(&ConnectionRoom::ChangeMatchRoomData, object, std::static_pointer_cast<GameMatchRoom>(GetRoomBaseRef()));
	_objects[object->objectBaseInfo->object_id()] = object;
	_playerCount.fetch_add(1);

	return true;
}

bool GameMatchRoom::Leave(ObjectRef object)
{
	if (object->IsPlayer() == false)
		return false;
	if (_objects.find(object->objectBaseInfo->object_id()) == _objects.end())
		return false;
	_objects.erase(object->objectBaseInfo->object_id());

	/* 룸 인원 체크 및 폐쇠 */

	uint8 curPlayer = _playerCount.fetch_sub(1) - 1;
	if (curPlayer == 0)
	{
		_isClosure = true;
		GRoomManager->Remove(std::static_pointer_cast<GameMatchRoom>(GetRoomBaseRef()));
	}

	GRoomManager->GetConnectionRoom()->DoAsync(&ConnectionRoom::ChangeMatchRoomData, object, GameMatchRoomRef(nullptr));

	return true;
}

void GameMatchRoom::StartGameMatch()
{
	if (_isClosure == true)
		return;
	_isClosure = true;

	// 팀 데이터 생성
	for (auto& object : _objects)
	{
		_leagueInstance->teamInstances.emplace(object.first, MakeXShared<TeamInstance>());
	}

	// TODO : 시작 루틴
}

void GameMatchRoom::ShowTeamMatchs(PlayerRef targetPlayer)
{
	Protocol::S_SHOW_TEAM_MATCH showTeamMatchPkt;
	for (auto& teamMatchRoom : _teamMatchRooms)
	{
		teamMatchRoom.second->MakeSelectInfo(*showTeamMatchPkt.add_team_match_select_infos());
	}
	SEND_S_PACKET(targetPlayer->ownerSession, showTeamMatchPkt);
}

void GameMatchRoom::CreateTeamMatch(uint64 homePlayerId, uint64 otherPlayerId)
{
	auto iter = _objects.find(homePlayerId);
	if (iter == _objects.end())
		return;
	ObjectRef homeObject = iter->second;

	iter = _objects.find(otherPlayerId);
	if (iter == _objects.end())
		return;
	ObjectRef otherObject = iter->second;

	if (homeObject->IsPlayer() == false || otherObject->IsPlayer() == false)
		return;

	/* 방 생성 */

	TeamMatchRoomRef newTeamMatchRoom = MakeXShared<TeamMatchRoom>(
		std::static_pointer_cast<GameMatchRoom>(GetRoomBaseRef()),
		_teamIdGenerator,
		std::static_pointer_cast<Player>(homeObject),
		std::static_pointer_cast<Player>(otherObject)
	);
	_teamMatchRooms.emplace(_teamIdGenerator, newTeamMatchRoom);
	_teamIdGenerator++;

	// 선수 액터 초기 데이터 생성 작업
	InitTeamMatch(newTeamMatchRoom, homePlayerId, otherPlayerId);

	/* 방으로 이동 예약 */

	RoomRef newRoom = std::static_pointer_cast<Room>(newTeamMatchRoom);

	RoomRef homePlayerRoom = homeObject->ownerRoom.load().lock();
	if (homePlayerRoom != nullptr)
	{
		homePlayerRoom->DoAsync(&Room::LeaveAndEnter, homeObject, newRoom);
	}

	RoomRef otherPlayerRoom = otherObject->ownerRoom.load().lock();
	if (otherPlayerRoom != nullptr)
	{
		otherPlayerRoom->DoAsync(&Room::LeaveAndEnter, homeObject, newRoom);
	}
}

void GameMatchRoom::DeleteTeamMatch(uint32 teamMatchId)
{
	if (_teamMatchRooms.find(teamMatchId) == _teamMatchRooms.end())
		return;
	_teamMatchRooms.erase(teamMatchId);
}

void GameMatchRoom::UpdateComputedDatas(TeamMatchRoomRef target, xVector<uint64> homeObjectIds, xVector<uint64> otherObjectIds)
{
	/* 홈 팀 선수 업데이트 정보 채우기 */
	
	xVector<Protocol::AthleteUpdateInfo> honeUpdateInfos;
	for (uint64& homeObjectId : homeObjectIds)
	{
		auto iter = _leagueInstance->athleteInstances.find(reinterpret_cast<IdConvertor*>(&homeObjectId)->tableId);
		if (iter == _leagueInstance->athleteInstances.end())
			continue;

		Protocol::AthleteUpdateInfo homeUpdateInfo;
		homeUpdateInfo.set_object_id(homeObjectId);
		ComputeData(iter->second, *homeUpdateInfo.mutable_athlete_computed_info());

		honeUpdateInfos.push_back(homeUpdateInfo);
	}

	/* 어웨이 팀 선수 업데이트 정보 채우기 */

	xVector<Protocol::AthleteUpdateInfo> otherUpdateInfos;
	for (uint64& otherObjectId : otherObjectIds)
	{
		auto iter = _leagueInstance->athleteInstances.find(reinterpret_cast<IdConvertor*>(&otherObjectId)->tableId);
		if (iter == _leagueInstance->athleteInstances.end())
			continue;

		Protocol::AthleteUpdateInfo otherUpdateInfo;
		otherUpdateInfo.set_object_id(otherObjectId);
		ComputeData(iter->second, *otherUpdateInfo.mutable_athlete_computed_info());

		otherUpdateInfos.push_back(otherUpdateInfo);
	}

	// 해당 데이터로 업데이트 명령
	target->DoAsync(&TeamMatchRoom::UpdateTeamMatch, honeUpdateInfos, otherUpdateInfos);
}

void GameMatchRoom::LeaveToRoot(ObjectRef object)
{
	if (Leave(object) == false)
		return;

	// 부모의 방도 연속해서 나가기
	RoomBaseRef parentRoom = _parentRoom.lock();
	if (_parentRoom.lock() == nullptr)
		return;

	parentRoom->DoAsync(&RoomBase::LeaveToRoot, object);
}

void GameMatchRoom::LeaveAlongPath(ObjectRef object, uint8 leaveCount, xStack<RoomBaseRef> enterPath)
{
	if (Leave(object) == false)
		return;

	// 나가야하는 방은 모두 나간 경우
	if (leaveCount == 0)
	{
		// Player의 현재 소속 방을 나타내는 ownerRoom을 변경
		auto connectionRoom = GRoomManager->GetConnectionRoom();
		connectionRoom->DoAsync(&ConnectionRoom::ChangeRoomData, object, enterPath);
	}
	else
	{
		/* 부모의 방도 연속해서 나가기 */ 

		RoomBaseRef nextRoom = _parentRoom.lock();
		if (nextRoom == nullptr)
			return;

		nextRoom->DoAsync(&RoomBase::LeaveAlongPath, object, --leaveCount, enterPath);
	}
}

void GameMatchRoom::EnterAlongPath(ObjectRef object, xStack<RoomBaseRef> enterPath)
{
	if (IsOpen() == false)
	{
		// 방 닫힘으로 인해 이동 실패 전송
		if (object->IsPlayer() == true)
		{
			Protocol::S_CHANGE_MAP changeMapPkt;
			changeMapPkt.set_success(false);
			SEND_S_PACKET(std::static_pointer_cast<Player>(object)->ownerSession, changeMapPkt);
		}

		/* 대안으로 로비 방으로 이동 */

		uint8 tmpLeaveCount;
		xStack<RoomBaseRef> tmpEnterPath;
		GetRollbackPath(OUT tmpLeaveCount, OUT tmpEnterPath);
		LeaveAlongPath(object, tmpLeaveCount, tmpEnterPath);

		return;
	}

	if (Enter(object) == false)
		return;

	// 진입해야할 방은 모두 들어간 경우
	if (enterPath.empty() == true)
		return;

	/* 연달아서 진입 */

	RoomBaseRef nextRoom = enterPath.top();
	enterPath.pop();
	nextRoom->DoAsync(&RoomBase::EnterAlongPath, object, enterPath);
}

void GameMatchRoom::ProcessMapBytes(ObjectRef moveObject, uint64 mapBytes)
{
	RoomIdConvertor* mapIndex = reinterpret_cast<RoomIdConvertor*>(mapBytes);

	RoomRef nextRoom = GetTeamMatchRoom(mapIndex->teamMatchIndex);
	if (nextRoom == nullptr)
		return;
	RoomRef ownerRoom = moveObject->ownerRoom.load().lock();
	if (ownerRoom == nullptr)
		return;

	ownerRoom->DoAsync(&Room::LeaveAndEnter, moveObject, nextRoom);
}

void GameMatchRoom::InitTeamMatch(TeamMatchRoomRef target, uint64 homePlayerId, uint64 otherPlayerId)
{
	auto iter = _leagueInstance->teamInstances.find(homePlayerId);
	if (iter == _leagueInstance->teamInstances.end())
		return;
	TeamInstanceRef homeTeam = iter->second;

	iter = _leagueInstance->teamInstances.find(otherPlayerId);
	if (iter == _leagueInstance->teamInstances.end())
		return;
	TeamInstanceRef otherTeam = iter->second;

	/* 생성자 파라미터 수집 */

	xVector<AthleteConstructionParameters> homeParameters;
	for (auto homeAthleteIter = homeTeam->athleteIds.begin(); homeAthleteIter != homeTeam->athleteIds.end(); homeAthleteIter++)
	{
		AthleteInstanceRef athleteInstance = _leagueInstance->athleteInstances.at(*homeAthleteIter);
		Protocol::AthleteComputedInfo athleteComputedInfo;
		ComputeData(athleteInstance, athleteComputedInfo);

		homeParameters.push_back(AthleteConstructionParameters{ athleteComputedInfo, 0, athleteInstance->athletePublicInfo->physical_info() });
	}

	xVector<AthleteConstructionParameters> otherParameters;
	for (auto otherAthleteIter = otherTeam->athleteIds.begin(); otherAthleteIter != otherTeam->athleteIds.end(); otherAthleteIter++)
	{
		AthleteInstanceRef athleteInstance = _leagueInstance->athleteInstances.at(*otherAthleteIter);
		Protocol::AthleteComputedInfo athleteComputedInfo;
		ComputeData(athleteInstance, athleteComputedInfo);

		otherParameters.push_back(AthleteConstructionParameters{ athleteComputedInfo, 0, athleteInstance->athletePublicInfo->physical_info() });
	}

	// 생성 요청
	target->DoAsync(&TeamMatchRoom::Init, homeParameters, otherParameters);
}

void GameMatchRoom::ComputeData(AthleteInstanceRef& athleteInstance, OUT Protocol::AthleteComputedInfo& athleteComputeInfo)
{
	// TODO
}

bool GameMatchRoom::IsOpen()
{
	return _isClosure == false && _playerCount.load() < MAX_PLAYER;
}

void GameMatchRoom::GetRollbackPath(OUT uint8& leaveCount, OUT xStack<RoomBaseRef>& enterPath)
{
	leaveCount = 0;
	enterPath.push(GRoomManager->GetLobby());
}

/*********************
	  LobbyRoom
*********************/

const uint64 LobbyRoom::ROOM_TICK_MS = 200;

LobbyRoom::LobbyRoom() : Room()
{
}

void LobbyRoom::UpdateRoomTick()
{
	{
		Protocol::S_UPDATE_ROOM updateRoomPkt;
		GRoomManager->GetGameMatchRoomInfos(updateRoomPkt);
		BROADCAST_S_PACKET(this, updateRoomPkt);
	}

	DoTimer(ROOM_TICK_MS, &LobbyRoom::UpdateRoomTick);
}

/*********************
	  LeagueRoom
*********************/

GameMatchRoomRef LeagueRoom::GetGameMatchRoom()
{
	return std::static_pointer_cast<GameMatchRoom>(_parentRoom.lock());
}

/*********************
	 TeamMatchRoom
*********************/

TeamMatchRoom::TeamMatchRoom(GameMatchRoomRef matchRoom, uint32 id, const PlayerRef homePlayer, const PlayerRef otherPlayer) : LeagueRoom(matchRoom), id(id), _isClosure(false)
{
	_teamMatchInstance = MakeXShared<TeamMatchInstance>(homePlayer, otherPlayer);
}

void TeamMatchRoom::MakeSelectInfo(OUT Protocol::TeamMatchSelectInfo& info)
{
	info.set_team_match_id(id);
	info.set_home_player_id(_teamMatchInstance->teamMatchInitInfo->home_player_id());
	info.set_other_player_id(_teamMatchInstance->teamMatchInitInfo->other_player_id());
}

bool TeamMatchRoom::Enter(ObjectRef object, bool randPos)
{
	if (LeagueRoom::Enter(object, randPos) == false)
		return false;

	if (object->IsPlayer() == true)
	{
		ShareTeamMatchInfo(std::static_pointer_cast<Player>(object));
	}
	return true;
}

void TeamMatchRoom::Init(xVector<AthleteConstructionParameters> homeAthleteParameters, xVector<AthleteConstructionParameters> otherAthleteParameters)
{
	for (auto& homeAthleteParameter : homeAthleteParameters)
	{
		homeAthleteParameter.costumeBytes = _teamMatchInstance->teamMatchInitInfo->home_costume_setting();
		Enter(std::static_pointer_cast<Object>(ObjectUtils::CreatedAthlete(homeAthleteParameter, true)), false);
	}
	for (auto& otherAthleteParameter : otherAthleteParameters)
	{
		otherAthleteParameter.costumeBytes = _teamMatchInstance->teamMatchInitInfo->other_costume_setting();
		Enter(std::static_pointer_cast<Object>(ObjectUtils::CreatedAthlete(otherAthleteParameter, false)), false);
	}
}

void TeamMatchRoom::RequestToUpdateComputedData()
{
	GameMatchRoomRef ownerMatchRoom = GetGameMatchRoom();
	if (ownerMatchRoom == nullptr)
		return;

	xVector<uint64> homeObjectIds;
	xVector<uint64> otherObjectIds;
	for (auto& objectPair : _objects)
	{
		ObjectRef& object = objectPair.second;

		if (object->objectBaseInfo->object_type() != Protocol::OBJECT_TYPE_ATHLETE)
			continue;

		if (std::static_pointer_cast<Athlete>(object)->isHomePlayer == true)
		{
			homeObjectIds.push_back(object->objectBaseInfo->object_id());
		}
		else
		{
			otherObjectIds.push_back(object->objectBaseInfo->object_id());
		}
	}

	// GameMatchRoom에 요청
	ownerMatchRoom->DoAsync(&GameMatchRoom::UpdateComputedDatas, std::static_pointer_cast<TeamMatchRoom>(GetRoomRef()), homeObjectIds, otherObjectIds);

	// TODO : TeamMatchDetailInfo 부분의 업데이트 
}

void TeamMatchRoom::UpdateTeamMatch(xVector<Protocol::AthleteUpdateInfo> homeUpdateInfos, xVector<Protocol::AthleteUpdateInfo> otherUpdateInfos)
{
	Protocol::S_UPDATE_TEAM_MATCH updateTeamMatchPkt;

	for (Protocol::AthleteUpdateInfo& homeUpdateInfo : homeUpdateInfos)
	{
		auto homeObjectPair = _objects.find(homeUpdateInfo.object_id());
		if (homeObjectPair == _objects.end())
			continue;

		// 서버의 업데이트
		std::static_pointer_cast<Athlete>(homeObjectPair->second)->athleteComputedInfo->CopyFrom(homeUpdateInfo.athlete_computed_info());

		// 패킷에 내용 추가
		updateTeamMatchPkt.add_home_athlete_update_infos()->CopyFrom(homeUpdateInfo);
	}
	for (Protocol::AthleteUpdateInfo& otherUpdateInfo : otherUpdateInfos)
	{
		auto otherObjectPair = _objects.find(otherUpdateInfo.object_id());
		if (otherObjectPair == _objects.end())
			continue;

		// 서버의 업데이트
		std::static_pointer_cast<Athlete>(otherObjectPair->second)->athleteComputedInfo->CopyFrom(otherUpdateInfo.athlete_computed_info());

		// 패킷에 내용 추가
		updateTeamMatchPkt.add_other_athlete_update_infos()->CopyFrom(otherUpdateInfo);
	}
	updateTeamMatchPkt.mutable_team_match_detail_info()->CopyFrom(*_teamMatchInstance->teamMatchDetailInfo);

	BROADCAST_S_PACKET(this, updateTeamMatchPkt);
}

void TeamMatchRoom::EndTeamMatch()
{
	if (_isClosure == true)
		return;
	_isClosure = true;

	GameMatchRoomRef ownerMatchRoom = GetGameMatchRoom();
	if (ownerMatchRoom == nullptr)
		return;
	
	/* 룸 폐쇠 전에 모든 인원 방출 */

	auto objects = _objects;
	for (auto& objectPair : objects)
	{
		if (objectPair.second->IsPlayer())
		{
			LeaveAndEnter(objectPair.second, ownerMatchRoom->GetMeetingRoom());
		}
	}

	ownerMatchRoom->DoAsync(&GameMatchRoom::DeleteTeamMatch, id);
}

bool TeamMatchRoom::IsOpen()
{
	return _isClosure == false;
}

void TeamMatchRoom::GetRollbackPath(OUT uint8& leaveCount, OUT xStack<RoomBaseRef>& enterPath)
{
	leaveCount = 0;

	GameMatchRoomRef ownerMatchRoom = GetGameMatchRoom();
	if (ownerMatchRoom == nullptr)
		return;

	enterPath.push(ownerMatchRoom->GetMeetingRoom());
}

void TeamMatchRoom::ShareTeamMatchInfo(const PlayerRef newPlayer)
{
	Protocol::S_INIT_TEAM_MATCH initTeamMatchPkt;

	// 팀 매칭 룸 정보 모두 기록
	_teamMatchInstance->MakeTeamMatchInfo(*initTeamMatchPkt.mutable_team_match_info());

	SEND_S_PACKET(newPlayer->ownerSession, initTeamMatchPkt);
}

/*********************
	 MeetingRoom
*********************/

MeetingRoom::MeetingRoom(GameMatchRoomRef matchRoom) : LeagueRoom(matchRoom)
{

}