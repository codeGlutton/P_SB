#pragma once

#include "SBTableRow.h"
#include "Engine/DataTable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SBDataTable.generated.h"

UCLASS()
class USBDataTable : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USBDataTable()
	{
		static ConstructorHelpers::FObjectFinder<UDataTable> DT_ATHLETETABLE(TEXT("DataTable'/Game/SB/Data/DataTables/DT_SBAthleteTable.DT_SBAthleteTable'"));
		if (DT_ATHLETETABLE.Succeeded())
			AthleteTable = DT_ATHLETETABLE.Object.Get();

		_Tables.Add(1000ull / 1000ull, AthleteTable);
	}

	const FSBTableRow* FindTableRow(uint64 ObjectId);

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UDataTable* AthleteTable;

private:

	UPROPERTY()
	TMap<uint64, UDataTable*> _Tables;
};