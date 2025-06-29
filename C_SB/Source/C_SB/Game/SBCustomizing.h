#pragma once

#include "CoreMinimal.h"
#include "SBTableRow.h"
#include "UObject/Interface.h"
#include "SBCustomizing.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBCustomizing : public UInterface
{
	GENERATED_BODY()
};

/**
 * 커스텀 마이징이 가능한 대상
 */
class C_SB_API ISBCustomizing
{
	GENERATED_BODY()

public:
	virtual void			ChangeOutfit(const ESBTableRowType Type, TSoftObjectPtr<USkeletalMesh> SkeletalMesh, TSoftObjectPtr<UMaterialInstance> MaterialInstance) = 0;
	virtual void			ChangeOutfit(const ESBTableRowType Type, TSoftObjectPtr<USkeletalMesh> SkeletalMesh, TSoftObjectPtr<UMaterialInstance> MaterialInstance, bool bNeedToLoad);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UTexture2D*	Get2DCustomizingTexture();
	virtual UTexture2D* Get2DCustomizingTexture_Implementation() { return nullptr; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UTextureRenderTarget2D* Get2DCustomizingRenderTarget();
	virtual UTextureRenderTarget2D* Get2DCustomizingRenderTarget_Implementation() { return nullptr; }
};
