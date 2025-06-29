#include "pch.h"
#include "DataUtils.h"

#include "AthleteInstance.h"
#include "TableRow.h"

AthleteInstanceRef DataUtils::CreatedAthleteInstance(const int32 id)
{
	auto& AthleteTable = *GDataTableManager->SpawnableTables[TableRowType::ATHLETE_TABLE];
	auto iter = AthleteTable.find(id);
	if (iter == AthleteTable.end())
		return nullptr;

	AthleteInstanceRef athleteInstance = MakeXShared<AthleteInstance>(*reinterpret_cast<AthleteTableRow*>(iter->second));
	return athleteInstance;
}
