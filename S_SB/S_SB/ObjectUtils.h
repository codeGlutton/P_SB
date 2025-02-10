#pragma once

USING_SHARED_PTR(Athlete);
USING_SHARED_PTR(AthleteInstance);

struct IdConvertor
{
	IdConvertor(uint64 objId) : objId(objId)
	{
	}

	IdConvertor(uint32 gameId, uint16 tableId)
	{
		objId = ((uint64)tableId << 48) + (uint64)gameId;
	}

	IdConvertor(uint32 gameId, uint32 dbId, uint16 tableId)
	{
		objId = ((uint64)tableId << 48) + ((uint64)dbId << 24) + (uint64)gameId;
	}

	union
	{
		struct
		{
			uint64					gameId	: 24;
			uint64					dbId	: 24;
			uint64					tableId	: 16;
		};
		uint64						objId;
	};
};

/************************
	   ObjectUtils
*************************/

struct AthleteConstructionParameters;

class ObjectUtils
{
public:
	static PlayerRef			CreatedPlayer(GameSessionRef session, int32 dbId);
	static AthleteRef			CreatedAthlete(const AthleteConstructionParameters& parameters, bool isHomePlayer);

private:
	static std::atomic<uint32>	s_idGenerator;
};

