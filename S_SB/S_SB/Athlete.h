#pragma once

#include "Object.h"

struct AthleteConstructionParameters
{
	Protocol::AthleteComputedInfo	athleteComputedInfo;
	uint32							costumeBytes;
	uint64							physicalBytes;
};

class Athlete : public Object
{
public:
	Athlete(const AthleteConstructionParameters& parameters, bool isHomePlayer);
	virtual ~Athlete() override;

	virtual void									MakeObjectInfo(OUT Protocol::ObjectInfo& outInfo) override;
	void											MakeAthleteUpdateInfo(OUT Protocol::AthleteUpdateInfo& outInfo);

public:
	Protocol::AthleteComputedInfo*					athleteComputedInfo;
	const bool										isHomePlayer;
};