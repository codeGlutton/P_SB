#pragma once

#include "CoreMinimal.h"
#include "SBObject.h"
#include "Engine/DataTable.h"
#include "SBTableRow.generated.h"

/**********************
	  FSBTableRow
**********************/

USTRUCT(Blueprintable)
struct FSBTableRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	FSBTableRow() :Id(), ClientName(), ClientBasePath() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ClientName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MustImplement = "SBObject"))
	TSoftClassPtr<UObject> ClientBasePath;
};

/**********************
	FSBAthleteTableRow
**********************/

USTRUCT(Blueprintable)
struct FSBAthleteTableRow : public FSBTableRow
{
	GENERATED_USTRUCT_BODY()

public:
	FSBAthleteTableRow() : FSBTableRow(), Race(), Positioin(), Skin() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Race;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Positioin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Skin;
};