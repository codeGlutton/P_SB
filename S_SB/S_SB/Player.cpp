#include "pch.h"
#include "Player.h"
#include "Room.h"

#include "LeagueInstance.h"
#include "TeamInstance.h"

Player::Player()
{
	playerDetailInfo = xnew<Protocol::PlayerDetailInfo>();
	objectBaseInfo->set_object_type(Protocol::OBJECT_TYPE_PLAYER);
	ownerSession = nullptr;
	_isPlayer = true;
}

Player::~Player()
{
	xdelete(playerDetailInfo);
	playerDetailInfo = nullptr;
}

void Player::MakeObjectInfo(OUT Protocol::ObjectInfo& outInfo)
{
	Object::MakeObjectInfo(OUT outInfo);

	outInfo.mutable_player_detail_info()->CopyFrom(*playerDetailInfo);
}
