#include "SBShowcaseAthlete.h"
#include "C_SB.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

ASBShowcaseAthlete::ASBShowcaseAthlete()
{
	PrimaryActorTick.bCanEverTick = true;

	auto* UniformMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("UniformMeshComp"));
	if (UniformMesh)
	{
		_CostumeMeshComps.Add(ESBTableRowType::UNIFORM_COSTUME_TABLE, UniformMesh);
		UniformMesh->SetupAttachment(GetCapsuleComponent());
	}
	auto* SceneCapture2D = CreateDefaultSubobject<USceneCaptureComponent2D>(FName("SceneCapture2DComp"));
	if (SceneCapture2D)
	{
		SceneCapture2D->SetupAttachment(GetMesh());
	}

	/*_CostumeMeshComps.Add(ESBTableRowType::UNIFORM_COSTUME_TABLE, CreateDefaultSubobject<USkeletalMeshComponent>(FName("UniformMeshComp")));
	_CostumeMeshComps.Add(ESBTableRowType::UNIFORM_COSTUME_TABLE, CreateDefaultSubobject<USkeletalMeshComponent>(FName("UniformMeshComp")));*/
}

void ASBShowcaseAthlete::BeginPlay()
{
	Super::BeginPlay();
}

void ASBShowcaseAthlete::OnConstruction(const FTransform& Transform)
{
	for (auto& CostumeMeshComp : _CostumeMeshComps)
	{
		CostumeMeshComp.Value->SetLeaderPoseComponent(GetMesh());
	}
}

void ASBShowcaseAthlete::ChangeOutfit(const ESBTableRowType Type, TSoftObjectPtr<USkeletalMesh> SkeletalMesh, TSoftObjectPtr<UMaterialInstance> MaterialInstance)
{
	TObjectPtr<USkeletalMeshComponent>* CostumeMeshComp = _CostumeMeshComps.Find(Type);
	if (CostumeMeshComp == nullptr)
		return;

	if (SkeletalMesh.IsValid() == true)
	{
		(*CostumeMeshComp)->SetSkeletalMesh(SkeletalMesh.Get());
	}
	if (MaterialInstance.IsValid() == true)
	{
		(*CostumeMeshComp)->SetMaterial(0, MaterialInstance.Get());
	}
}

UTextureRenderTarget2D* ASBShowcaseAthlete::Get2DCustomizingRenderTarget_Implementation()
{
	return _SceneCapture2DComp->TextureTarget;
}


