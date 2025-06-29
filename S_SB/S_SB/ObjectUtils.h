#pragma once

USING_SHARED_PTR(Player);
USING_SHARED_PTR(Athlete);
USING_SHARED_PTR(GameSession);

/************************
	   ObjectUtils
*************************/

struct AthleteConstructionParameters;

class ObjectUtils
{
public:
	static PlayerRef			CreatedPlayer(GameSessionRef session, uint32 dbId, uint16 tableId);
	static AthleteRef			CreatedAthlete(const AthleteConstructionParameters& parameters, bool isHomePlayer);

private:
	static std::atomic<uint32>	s_idGenerator;
};

