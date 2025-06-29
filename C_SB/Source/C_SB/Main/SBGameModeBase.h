#pragma once

#include "CoreMinimal.h"
#include "SBTableRow.h"
#include "GameFramework/GameModeBase.h"
#include "SBGameModeBase.generated.h"

/**
 * Seamless travel로 인해 선택적으로 클래스를 유지할 GameModeBase
 */
UCLASS()
class C_SB_API ASBGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASBGameModeBase();
	virtual void										Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<ESBTableRowType, TObjectPtr<UDataTable>>		SpawnableTables;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<ESBTableRowType, TObjectPtr<UDataTable>>		CostumeTables;
};