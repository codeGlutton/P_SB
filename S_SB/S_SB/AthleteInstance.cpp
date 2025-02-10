#include "pch.h"
#include "AthleteInstance.h"

#include "TableRow.h"

AthleteInstance::AthleteInstance(const AthleteTableRow& table) : seedInfo(table)
{
	athletePublicInfo = xnew<Protocol::AthletePublicInfo>();
	athletePrivateInfo = xnew<Protocol::AthletePrivateInfo>();
}

AthleteInstance::~AthleteInstance()
{
	xdelete(athletePublicInfo);
	xdelete(athletePrivateInfo);

	athletePublicInfo = nullptr;
	athletePrivateInfo = nullptr;
}

void AthleteInstance::MakeAthleteAnnualUpdateInfo(OUT Protocol::AthleteAnnualUpdateInfo& outInfo)
{
	outInfo.set_athlete_id(seedInfo.Id);
	outInfo.mutable_athlete_private_info()->CopyFrom(*athletePrivateInfo);
	outInfo.mutable_athlete_public_info()->CopyFrom(*athletePublicInfo);
}

void AthleteInstance::MakeAthleteInitInfo(OUT Protocol::AthleteInitInfo& outInfo)
{
	outInfo.set_athlete_id(seedInfo.Id);
	outInfo.set_age(age);
	outInfo.mutable_athlete_private_info()->CopyFrom(*athletePrivateInfo);
	outInfo.mutable_athlete_public_info()->CopyFrom(*athletePublicInfo);
}
