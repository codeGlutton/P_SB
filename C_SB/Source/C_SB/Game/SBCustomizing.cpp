#include "SBCustomizing.h"
#include "C_SB.h"
#include "Engine/AssetManager.h"

void ISBCustomizing::ChangeOutfit(const ESBTableRowType Type, TSoftObjectPtr<USkeletalMesh> SkeletalMesh, TSoftObjectPtr<UMaterialInstance> MaterialInstance, bool bNeedToLoad)
{
	if (bNeedToLoad == false)
	{
		ChangeOutfit(Type, SkeletalMesh, MaterialInstance);
		return;
	}

	TArray<FSoftObjectPath> LoadPaths{ SkeletalMesh.ToSoftObjectPath(), MaterialInstance.ToSoftObjectPath()};

	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(LoadPaths, FStreamableDelegate::CreateLambda([this, Type, SkeletalMesh, MaterialInstance]()
		{
			ChangeOutfit(Type, SkeletalMesh, MaterialInstance);
		}
	));
}
