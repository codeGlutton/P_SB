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
{%- for parser in parsers %}
		static ConstructorHelpers::FObjectFinder<UDataTable> DT_{{ parser.origin | upper }}(TEXT("DataTable'/Game/SB/Data/DataTables/DT_{{ parser.fileName }}.DT_{{ parser.fileName }}'"));
		if (DT_{{ parser.origin | upper }}.Succeeded())
			{{ parser.origin }} = DT_{{ parser.origin | upper }}.Object.Get();
{%- endfor %}
{% for parser in parsers %}
		_Tables.Add({{ parser.prefixId }}ull / 1000ull, {{ parser.origin }});
{%- endfor %}
	}

	const FSBTableRow* FindTableRow(uint64 ObjectId);

public:
{%- for parser in parsers %}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UDataTable* {{ parser.origin }};
{%- endfor %}

private:

	UPROPERTY()
	TMap<uint64, UDataTable*> _Tables;
};
