#pragma once

class TeamMatchInstance
{
public:
	TeamMatchInstance(const PlayerRef homePlayer, const PlayerRef otherPlayer);
	~TeamMatchInstance();

	void											MakeTeamMatchInfo(OUT Protocol::TeamMatchInfo& outInfo);

public:
	Protocol::TeamMatchInitInfo*					teamMatchInitInfo;
	Protocol::TeamMatchDetailInfo*					teamMatchDetailInfo;
};

