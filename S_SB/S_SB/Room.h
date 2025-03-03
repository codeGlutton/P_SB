#pragma once

#include <sqltypes.h>

USING_SHARED_PTR(GameMatchRoom);

USING_SHARED_PTR(TeamMatchRoom);
USING_SHARED_PTR(MeetingRoom);

USING_SHARED_PTR(AthleteInstance);
USING_SHARED_PTR(TeamInstance);
USING_SHARED_PTR(TeamMatchInstance);
USING_SHARED_PTR(LeagueInstance);

namespace Protocol
{
	class C_MOVE;
	class S_SPAWN;
}

struct AthleteConstructionParameters;

/*********************
	   RoomBase
*********************/

// 오브젝트의 관리를 위한 구분용 객체
class RoomBase : public JobQueue
{
public:
	RoomBase() : _parentRoom(std::weak_ptr<RoomBase>()), _level(0)
	{
	}

	RoomBase(RoomBaseRef parentRoom) : _parentRoom(parentRoom)
	{
		_level = parentRoom->GetLevel() + 1;
	}

	/* thread-safe */

	// 넓은 범위의 방이 존재하는지
	const RoomBaseRef					GetParentRoom() { return _parentRoom.lock(); }
	const uint8&						GetLevel() { return _level; }
	RoomBaseRef							GetRoomBaseRef();

	/* thread-unsafe (jobQ로 보호)  */

	// 단순 부모를 따라 모든 방을 나가기. 오브젝트의 ownerRoom 포인터는 변동되지 않음
	virtual void						LeaveToRoot(ObjectRef object) {}
	// (직접적인 호출 금지) 경로를 따라 순차적 방 나가기
	virtual	void						LeaveAlongPath(ObjectRef object, uint8 leaveCount, xStack<RoomBaseRef> enterPath) {}
	// (직접적인 호출 금지) 경로를 따라 순차적 방 진입
	virtual	void						EnterAlongPath(ObjectRef object, xStack<RoomBaseRef> enterPath) {}

	// 맵 bytes로 전송된 데이터를 분석해 방 진입
	virtual	void						ProcessMapBytes(ObjectRef moveObject, uint64 mapBytes) {}

	// (방 참여자들에게) 브로드캐스트
	virtual void						Broadcast(SendBufferRef sendBuffer);
	// (방 참여자들에게) 특정 id 제외 브로드캐스트
	virtual void						ExceptionalBroadcast(SendBufferRef sendBuffer, uint64 exceptionId);

protected:
	// 진입 가능한 방인지 확인
	virtual bool						IsOpen() { return true; }
	// 진입 불가시 대안으로 이동할 경로
	virtual void						GetRollbackPath(OUT uint8& leaveCount, OUT xStack<RoomBaseRef>& enterPath) { return; }

protected:
	// 해당 방을 감싸는 더 넓은 범위의 방 객체
	std::weak_ptr<RoomBase>				_parentRoom;
	// 룸 깊이
	uint8								_level;

	// 방 참여자들
	xUnorderedMap<uint64, ObjectRef>	_objects;
};

/*********************
		Room
*********************/

// 실제 캐릭터 공간 구분용 객체
class Room : public RoomBase
{
public:
	Room() : RoomBase()
	{
	}

	Room(RoomBaseRef parentRoom) : RoomBase(parentRoom)
	{
	}

	/* thread-safe */

	RoomRef								GetRoomRef();

	/* thread-unsafe (jobQ로 보호)  */

	// 단순 오브젝트 입장 함수. 오브젝트의 ownerRoom 포인터는 변동되지 않음
	virtual bool						Enter(ObjectRef object, bool randPos);
	// 단순 오브젝트 퇴장 함수. 오브젝트의 ownerRoom 포인터는 변동되지 않음
	virtual bool						Leave(ObjectRef object);
	// 플레이어 방 변경 함수
	virtual void						LeaveAndEnter(ObjectRef object, RoomRef nextRoom);

	void								LeaveToRoot(ObjectRef object) override;
	void								LeaveAlongPath(ObjectRef object, uint8 leaveCount, xStack<RoomBaseRef> enterPath) override;
	void								EnterAlongPath(ObjectRef object, xStack<RoomBaseRef> enterPath) override;

	virtual void						MoveObject(ObjectRef object, Protocol::C_MOVE pkt);

private:
	// 진입할 방 경로 탐색
	void								FindPathToRoom(RoomRef& targetRoom, OUT xQueue<RoomBaseRef>& leavePath, OUT xStack<RoomBaseRef>& enterPath);
};

/*********************
	ConnectionRoom
*********************/

// 관리용 전 플레이어 룸
class ConnectionRoom final : public RoomBase
{
public:
	ConnectionRoom() : RoomBase(), activeSessionCount(0)
	{
	}

	/* thread-unsafe (jobQ로 보호)  */

	void								TryToVerification(GameSessionRef gameSession, int32 accountId, xString tokenValue);
	void								LoadPlayerDatas(GameSessionRef gameSession, int32 accountId, xVector<Protocol::ObjectInfo> ObjectInfos);
	void								SelectPlayer(PlayerDataProtectorRef playerData, uint64 index);
	void								DeleteLocalData(PlayerDataProtectorRef playerData);

	void								StartHeartBeat(GameSessionRef gameSession);
	void								RestartHeartBeat(GameSessionRef gameSession);

	void								UpdateServerDencityTick();

	// (직접적인 호출 금지) 플레이어의 현재 방 정보를 교환 및 입장 시작
	void								ChangeRoomData(ObjectRef object, xStack<RoomBaseRef> enterPath);
	// (직접적인 호출 금지) 플레이어의 현재 게임 방 정보 교환
	void								ChangeMatchRoomData(ObjectRef object, GameMatchRoomRef nextRoom);

protected:
	bool								Enter(ObjectRef object);
	bool								Leave(ObjectRef object);

	float								GetDecity();

protected:
	// 인증을 마친 세션 수 (캐릭터 선택창 인원 + 인 게임 내 인원)
	uint64								activeSessionCount;

protected:
	// RTT 핑 연산에서 계산에 활용되지 않을 이상치 기준
	static const float					MAX_PING;

	static const uint64					MAX_PLAYER_NUM;

	static const uint64					HEART_BEAT_WAIT_MS;
	static const uint64					DENCITY_TICK_MS;
};

/*********************
	 GameMatchRoom
*********************/

// 매치를 위한 구분용 룸
class GameMatchRoom : public RoomBase
{
public:
	GameMatchRoom(const uint32 id, const xString name);

	/* thread-safe */

	// 현재 입장 인원 수
	const uint8							GetPlayerCount();
	void								MakeSelectInfo(OUT Protocol::RoomSelectInfo& info);

	const MeetingRoomRef				GetMeetingRoom();
	const TeamMatchRoomRef				GetTeamMatchRoom(uint32 id);

	/* thread-unsafe (jobQ로 보호)  */

	bool								Enter(ObjectRef object);
	bool								Leave(ObjectRef object);

	void								StartGameMatch();

	void								ShowTeamMatchs(PlayerRef targetPlayer);

	void								CreateTeamMatch(uint64 homePlayerId, uint64 otherPlayerId);
	void								DeleteTeamMatch(uint32 teamMatchId);

	void								UpdateComputedDatas(TeamMatchRoomRef target, xVector<uint64> homeObjectIds, xVector<uint64> otherObjectIds);

	void								LeaveToRoot(ObjectRef object) override;
	void								LeaveAlongPath(ObjectRef object, uint8 leaveCount, xStack<RoomBaseRef> enterPath) override;
	void								EnterAlongPath(ObjectRef object, xStack<RoomBaseRef> enterPath) override;

	void								ProcessMapBytes(ObjectRef moveObject, uint64 mapBytes) override;

protected:
	void								InitTeamMatch(TeamMatchRoomRef target, uint64 homePlayerId, uint64 otherPlayerId);

	void								ComputeData(AthleteInstanceRef& athleteInstance, OUT Protocol::AthleteComputedInfo& athleteComputeInfo);


	/* 입장 연관 */

	bool								IsOpen() override;
	void								GetRollbackPath(OUT uint8& leaveCount, OUT xStack<RoomBaseRef>& enterPath) override;

public:
	// GRoomManager 관리 및 네트워크 전송시 id
	const uint32						id;
	const xString						name;
	static const uint8					MAX_PLAYER;

protected:
	// 팀 매치 id와 팀 매치 매니저 간의 Map
	xMap<uint32, TeamMatchRoomRef>		_teamMatchRooms;
	MeetingRoomRef						_meetingRoom;

	LeagueInstanceRef					_leagueInstance;

	// UI에 보일 방 인원 수 변수
	std::atomic<uint8>					_playerCount;
	bool								_isClosure;

	uint32								_teamIdGenerator;
};

/*********************
	  LobbyRoom
*********************/

// 사용자 접속 시 마주할 광장 룸
class LobbyRoom : public Room
{
public:
	LobbyRoom();

	/* thread-unsafe (jobQ로 보호)  */

	virtual void						UpdateRoomTick();

private:
	static const uint64					ROOM_TICK_MS;
};

/*********************
	  LeagueRoom
*********************/

// 리그 관련 방
class LeagueRoom : public Room
{
public:
	LeagueRoom(GameMatchRoomRef matchRoom) : Room(matchRoom)
	{
	}

	/* thread-safe */

	GameMatchRoomRef					GetGameMatchRoom();

	/* thread-unsafe (jobQ로 보호)  */
};

/*********************
	TeamMatchRoom
*********************/

class TeamMatchRoom : public LeagueRoom
{
public:
	TeamMatchRoom(GameMatchRoomRef matchRoom, uint32 id, const PlayerRef homePlayer, const PlayerRef otherPlayer);

	/* thread-safe */

	void								MakeSelectInfo(OUT Protocol::TeamMatchSelectInfo& info);

	/* thread-unsafe (jobQ로 보호)  */

	virtual bool						Enter(ObjectRef object, bool randPos) override;
	void								Init(xVector<AthleteConstructionParameters> homeAthleteParameters, xVector<AthleteConstructionParameters> otherAthleteParameters);

	// GameMatchRoom에 업데이트 데이터 요청
	void								RequestToUpdateComputedData();
	void								UpdateTeamMatch(xVector<Protocol::AthleteUpdateInfo> homeUpdateInfos, xVector<Protocol::AthleteUpdateInfo> otherUpdateInfos);
	
	void								EndTeamMatch();

protected:
	/* 입장 연관 */

	bool								IsOpen() override;
	void								GetRollbackPath(OUT uint8& leaveCount, OUT xStack<RoomBaseRef>& enterPath) override;

	// 입장 시, TeamMatch의 모든 초기 정보 전달
	void								ShareTeamMatchInfo(const PlayerRef newPlayer);

public:
	const uint32						id;

protected:
	TeamMatchInstanceRef				_teamMatchInstance;
	bool								_isClosure;
};

/*********************
	 MeetingRoom
*********************/

class MeetingRoom : public LeagueRoom
{
public:
	MeetingRoom(GameMatchRoomRef matchRoom);
};