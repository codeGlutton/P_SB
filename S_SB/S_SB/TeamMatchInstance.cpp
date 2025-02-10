#include "pch.h"
#include "TeamMatchInstance.h"

#include "Player.h"

TeamMatchInstance::TeamMatchInstance(const PlayerRef homePlayer, const PlayerRef otherPlayer)
{
	teamMatchInitInfo = xnew<Protocol::TeamMatchInitInfo>();
	teamMatchDetailInfo = xnew<Protocol::TeamMatchDetailInfo>();

	teamMatchInitInfo->set_home_player_id(homePlayer->objectBaseInfo->object_id());
	teamMatchInitInfo->set_other_player_id(otherPlayer->objectBaseInfo->object_id());

	teamMatchInitInfo->set_home_costume_setting(homePlayer->playerDetailInfo->costume_setting());
	teamMatchInitInfo->set_other_costume_setting(otherPlayer->playerDetailInfo->costume_setting());
}

TeamMatchInstance::~TeamMatchInstance()
{
	xdelete(teamMatchInitInfo);
	xdelete(teamMatchDetailInfo);

	teamMatchInitInfo = nullptr;
	teamMatchDetailInfo = nullptr;
}

void TeamMatchInstance::MakeTeamMatchInfo(OUT Protocol::TeamMatchInfo& outInfo)
{
	outInfo.mutable_init_info()->CopyFrom(*teamMatchInitInfo);
	outInfo.mutable_detail_info()->CopyFrom(*teamMatchDetailInfo);
}
