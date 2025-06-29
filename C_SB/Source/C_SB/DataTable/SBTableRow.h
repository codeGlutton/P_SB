#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SBTableRow.generated.h"

UENUM(BlueprintType)
enum class ESBTableRowType : uint8
{
	NONE = 0 UMETA(Hidden),
	UNIFORM_COSTUME_TABLE = 1 UMETA(DisplayName = "UniformCostumeTable"),
	ATHLETE_TABLE = 1 UMETA(DisplayName = "AthleteTable"),
	MANAGER_TABLE = 2 UMETA(DisplayName = "ManagerTable"),
};

/**********************
	  FSBTableRow
**********************/

USTRUCT(Blueprintable)
struct FSBTableRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	FSBTableRow() :Id(), ClientName() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ClientName;
};

/**********************
	FSBCostumeTableRow
**********************/

USTRUCT(Blueprintable)
struct FSBCostumeTableRow : public FSBTableRow
{
	GENERATED_USTRUCT_BODY()

public:
	FSBCostumeTableRow() : FSBTableRow(), ClientMesh(), ClientMaterial() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USkeletalMesh> ClientMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInstance> ClientMaterial;
};

/**********************
	FSBSpawnableTableRow
**********************/

USTRUCT(Blueprintable)
struct FSBSpawnableTableRow : public FSBTableRow
{
	GENERATED_USTRUCT_BODY()

public:
	FSBSpawnableTableRow() : FSBTableRow(), ClientBasePath(), ClientMesh(), ClientMaterial() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MustImplement = "SBObject"))
	TSoftClassPtr<UObject> ClientBasePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USkeletalMesh> ClientMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInstance> ClientMaterial;
};

/**********************
	FSBAthleteTableRow
**********************/

USTRUCT(Blueprintable)
struct FSBAthleteTableRow : public FSBSpawnableTableRow
{
	GENERATED_USTRUCT_BODY()

public:
	FSBAthleteTableRow() : FSBSpawnableTableRow(), Race(), Positioin(), Skin() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Race;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Positioin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Skin;
};