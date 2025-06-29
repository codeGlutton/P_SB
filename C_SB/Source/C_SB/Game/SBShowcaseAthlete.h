#pragma once

#include "CoreMinimal.h"
#include "SBCustomizing.h"
#include "GameFramework/Character.h"
#include "SBShowcaseAthlete.generated.h"

/**
 * 쇼케이스 Athlete 캐릭터
 */
UCLASS()
class C_SB_API ASBShowcaseAthlete : public ACharacter, public ISBCustomizing
{
	GENERATED_BODY()

public:
	ASBShowcaseAthlete();

protected:
	virtual void												BeginPlay() override;
	virtual void												OnConstruction(const FTransform& Transform) override;

public:
	virtual void												ChangeOutfit(const ESBTableRowType Type, TSoftObjectPtr<USkeletalMesh> SkeletalMesh, TSoftObjectPtr<UMaterialInstance> MaterialInstance) override;
	virtual UTextureRenderTarget2D*								Get2DCustomizingRenderTarget_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneCaptureComponent2D>					_SceneCapture2DComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<ESBTableRowType, TObjectPtr<USkeletalMeshComponent>>	_CostumeMeshComps;
};
