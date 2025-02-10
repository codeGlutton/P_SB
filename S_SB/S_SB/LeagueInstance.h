#pragma once
#include "DataUtils.h"

USING_SHARED_PTR(TeamInstance);

// 플레이어들과의 게임 매치 내내 전체적인 정보를 관리하는 객체
// (클라이언트 측의 서브시스템과 정보 교환)
class LeagueInstance
{
public:
	LeagueInstance() : year(0), athleteInstances(), teamInstances()
	{
	}

public:
	int32								year;

	// 선수 id와 선수 정보 간의 Map
	xMap<uint32, AthleteInstanceRef>	athleteInstances;
	// 플레이어 id와 팀 간의 Map
	xMap<uint64, TeamInstanceRef>		teamInstances;
};
