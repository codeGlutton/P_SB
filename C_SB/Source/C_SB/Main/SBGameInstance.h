#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "SBGameInstance.generated.h"

UCLASS()
class C_SB_API USBGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USBGameInstance();

	virtual void							Init() override;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ASBPlayer>			OtherPlayerClass;
};