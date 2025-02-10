#include "pch.h"
#include "Athlete.h"
#include "TableRow.h"

#include "DataUtils.h"

Athlete::Athlete(const AthleteConstructionParameters& parameters, bool isHomePlayer) : isHomePlayer(isHomePlayer)
{
	athleteComputedInfo = xnew<Protocol::AthleteComputedInfo>();
	athleteComputedInfo->CopyFrom(parameters.athleteComputedInfo);

	objectBaseInfo->set_object_type(Protocol::OBJECT_TYPE_ATHLETE);

	// TODO : 윙스펜 키 정보와 (참고로 public 정보. 전달안해도 됨.)
	// TODO : TeamMatchInstanceRef 의 코스튬 설정과
	// TODO : athleteId로 DataTable 가지고와서 외견 생성
}

Athlete::~Athlete()
{
	xdelete(athleteComputedInfo);
	athleteComputedInfo = nullptr;
}

void Athlete::MakeObjectInfo(OUT Protocol::ObjectInfo& outInfo)
{
	Object::MakeObjectInfo(OUT outInfo);

	outInfo.mutable_athlete_computed_info()->CopyFrom(*athleteComputedInfo);
}

void Athlete::MakeAthleteUpdateInfo(OUT Protocol::AthleteUpdateInfo& outInfo)
{
	outInfo.set_object_id(objectBaseInfo->object_id());
	outInfo.mutable_athlete_computed_info()->CopyFrom(*athleteComputedInfo);
}
