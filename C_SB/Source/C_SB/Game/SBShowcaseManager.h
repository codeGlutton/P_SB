#pragma once

#include "CoreMinimal.h"
#include "SBCustomizing.h"
#include "GameFramework/Character.h"
#include "SBShowcaseManager.generated.h"

/**
 * 쇼케이스 Manager 캐릭터
 */
UCLASS()
class C_SB_API ASBShowcaseManager : public ACharacter, public ISBCustomizing
{
	GENERATED_BODY()

public:
	ASBShowcaseManager();

protected:
	virtual void						BeginPlay() override;

public:	
	virtual void						ChangeOutfit(const ESBTableRowType Type, TSoftObjectPtr<USkeletalMesh> SkeletalMesh, TSoftObjectPtr<UMaterialInstance> MaterialInstance) override;
	virtual UTextureRenderTarget2D*		Get2DCustomizingRenderTarget_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneCaptureComponent2D>					_SceneCapture2DComp;
};