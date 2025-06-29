#include "Component/SBCostumeMergingComponent.h"
#include "C_SB.h"
#include "Engine/AssetManager.h"
#include "SkeletalMergingLibrary.h"
#include "GameFramework/Character.h"

USBCostumeMergingComponent::USBCostumeMergingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bMergeCostumeOnInit = true;
}

void USBCostumeMergingComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bMergeCostumeOnInit == true)
	{
		ChangeCostume();
	}
}

void USBCostumeMergingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USBCostumeMergingComponent::ChangeCostume()
{
	if (OnGetCostumeMeshes.IsBound() == false)
		return;

	TArray<TSoftObjectPtr<USkeletalMesh>> SkeletalMeshPtrs = OnGetCostumeMeshes.Execute();
	TArray<FSoftObjectPath> LoadItems;
	for (auto& SkeletalMeshPtr : SkeletalMeshPtrs)
	{
		LoadItems.Add(SkeletalMeshPtr.ToSoftObjectPath());
	}
	
	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(LoadItems, FStreamableDelegate::CreateLambda([this, SkeletalMeshPtrs]()
		{
			TArray<TObjectPtr<USkeletalMesh>> SkeletalMeshes;
			for (auto& SkeletalMeshPtr : SkeletalMeshPtrs)
			{
				SkeletalMeshes.Add(SkeletalMeshPtr.Get());
			}
			ChangeCostume(SkeletalMeshes);
		}
	));
}

void USBCostumeMergingComponent::ChangeCostume(const TArray<TObjectPtr<USkeletalMesh>>& SkeletalMeshes)
{
	if (SkeletalMeshes.IsEmpty() == true)
		return;

	auto* OwnerCharacter = GetOwner<ACharacter>();
	if (OwnerCharacter == nullptr)
		return;

	FSkeletalMeshMergeParams SkeletalMeshMergeParams;
	SkeletalMeshMergeParams.Skeleton = OwnerCharacter->GetMesh()->GetSkeletalMeshAsset()->GetSkeleton();
	SkeletalMeshMergeParams.MeshesToMerge = SkeletalMeshes;
	USkeletalMesh* MergedSkeletalMesh = USkeletalMergingLibrary::MergeMeshes(SkeletalMeshMergeParams);
	if (MergedSkeletalMesh == nullptr)
		return;

	OwnerCharacter->GetMesh()->SetSkeletalMesh(MergedSkeletalMesh);
}

