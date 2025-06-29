#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SBCostumeMergingComponent.generated.h"

DECLARE_DELEGATE_RetVal(TArray<TSoftObjectPtr<USkeletalMesh>>, FOnGetCostumeMeshes)

UCLASS(ClassGroup = Costume, meta = (BlueprintSpawnableComponent))
class C_SB_API USBCostumeMergingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USBCostumeMergingComponent();

protected:
	virtual void						BeginPlay() override;

public:
	virtual void						TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void								ChangeCostume();
	void								ChangeCostume(const TArray<TObjectPtr<USkeletalMesh>>& SkeletalMeshes);

public:
	FOnGetCostumeMeshes					OnGetCostumeMeshes;

	// BeginPlay() 시기에 초기 정보로 Merged Skeleton 생성 및 적용 여부
	bool								bMergeCostumeOnInit;
};
