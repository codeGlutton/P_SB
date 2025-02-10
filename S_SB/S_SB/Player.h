#pragma once

#include "Object.h"

struct PlayerClassTableRow;

USING_SHARED_PTR(TeamInstance);

namespace Protocol
{
	class PlayerDetailInfo;
}

class Player : public Object
{
public:
	Player();
	virtual ~Player() override;

	virtual void										MakeObjectInfo(OUT Protocol::ObjectInfo& outInfo) override;

public:
	Protocol::PlayerDetailInfo*							playerDetailInfo;

	GameSessionRef										ownerSession;
	std::atomic<std::weak_ptr<class GameMatchRoom>>		ownerMatchRoom;
};

