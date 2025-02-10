#pragma once

struct AthleteTableRow;

class AthleteInstance
{
public:
	AthleteInstance(const AthleteTableRow& table);
	~AthleteInstance();

	void											MakeAthleteAnnualUpdateInfo(OUT Protocol::AthleteAnnualUpdateInfo& outInfo);
	void											MakeAthleteInitInfo(OUT Protocol::AthleteInitInfo& outInfo);

public:
	/* 초기 정보 */

	// 시드로 정해진 값
	const AthleteTableRow&							seedInfo;
	uint32											age;

	/* 업데이트 정보 */

	Protocol::AthletePublicInfo*					athletePublicInfo;
	Protocol::AthletePrivateInfo*					athletePrivateInfo;

protected:
	// TODO: 계약정보에 weak_ptr<TeamInsatnce> or uint64 ownerPlayerId 필요. 팀이 없더라도 FA 팀
};

