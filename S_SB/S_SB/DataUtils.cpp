#include "pch.h"
#include "DataUtils.h"

#include "AthleteInstance.h"
#include "TableRow.h"

AthleteInstanceRef DataUtils::CreatedAthleteInstance(const xWString name)
{
	AthleteTableRow& athleteTableRow = GDataTable->AthleteTable[name];
	AthleteInstanceRef athleteInstance = MakeXShared<AthleteInstance>(athleteTableRow);

	return athleteInstance;
}
