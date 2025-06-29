#pragma once

USING_SHARED_PTR(AthleteInstance);

/**********************
	   DataUtils
***********************/

class DataUtils
{
public:
	// 정해진 시드의 값으로 선수 인스턴스 생성
	static AthleteInstanceRef CreatedAthleteInstance(const int32 id);
};
