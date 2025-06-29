#include "pch.h"
#include "ObjectUtils.h"
#include "ByteConverters.h"

#include "GameSession.h"
#include "Player.h"
#include "Athlete.h"

#include "AthleteInstance.h"

/************************
	   ObjectUtils
*************************/

std::atomic<uint32> ObjectUtils::s_idGenerator = 0;

PlayerRef ObjectUtils::CreatedPlayer(GameSessionRef session, uint32 dbId, uint16 tableId)
{
	const uint32 newId = s_idGenerator.fetch_add(1);

	PlayerRef player = MakeXShared<Player>();
	IdConvertor idConvertor(newId, dbId, tableId);
	player->objectBaseInfo->set_object_id(idConvertor.objId);
	player->ownerSession = session;

	return player;
}

AthleteRef ObjectUtils::CreatedAthlete(const AthleteConstructionParameters& parameters, bool isHomePlayer)
{
	const uint32 newId = s_idGenerator.fetch_add(1);

	AthleteRef athlete = MakeXShared<Athlete>(parameters, isHomePlayer);
	IdConvertor idConvertor(newId, parameters.athleteComputedInfo.athlete_id());
	athlete->objectBaseInfo->set_object_id(idConvertor.objId);

	return athlete;
}
