#include "SBShowcaseManager.h"
#include "C_SB.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

ASBShowcaseManager::ASBShowcaseManager()
{
	PrimaryActorTick.bCanEverTick = true;

	auto* SceneCapture2D = CreateDefaultSubobject<USceneCaptureComponent2D>(FName("SceneCapture2DComp"));
	if (SceneCapture2D)
	{
		SceneCapture2D->SetupAttachment(GetMesh());
	}
}

void ASBShowcaseManager::BeginPlay()
{
	Super::BeginPlay();
}

void ASBShowcaseManager::ChangeOutfit(const ESBTableRowType Type, TSoftObjectPtr<USkeletalMesh> SkeletalMesh, TSoftObjectPtr<UMaterialInstance> MaterialInstance)
{
	if (SkeletalMesh.IsValid() == true)
	{
		GetMesh()->SetSkeletalMesh(SkeletalMesh.Get());
	}
	if (MaterialInstance.IsValid() == true)
	{
		GetMesh()->SetMaterial(0, MaterialInstance.Get());
	}
}

UTextureRenderTarget2D* ASBShowcaseManager::Get2DCustomizingRenderTarget_Implementation()
{
	return _SceneCapture2DComp->TextureTarget;
}
