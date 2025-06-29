#pragma once

#include "CoreMinimal.h"
#include "Wrappers.h"
#include "SBCustomizing.h"
#include "SBTableRow.h"
#include "UObject/NoExportTypes.h"
#include "MVVMViewModelBase.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SBPlayerSetupViewModel.generated.h"

/************************
 USBPlayerSelectViewModel
*************************/

UCLASS(BlueprintType)
class C_SB_API USBPlayerSelectViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/* FieldNotify Getter/Setter */

	void SetManagerCharacter(const TScriptInterface<ISBCustomizing>& NewManagerCharacter)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(ManagerCharacter, NewManagerCharacter))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Get2DManagerTexture);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Get2DManagerRenderTarget);
			UpdateManagerOutfit();
		}
	}

	void SetPlayerSelectInfo(const FSBPlayerSelectInfo& NewPlayerSelectInfo)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(PlayerSelectInfo, NewPlayerSelectInfo))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsEmptyPlayer);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSimpleCreatedDate);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetPlayerName);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetTier);
			UpdateManagerOutfit();
		}
	}

	const TScriptInterface<ISBCustomizing>& GetManagerCharacter() const
	{
		return ManagerCharacter;
	}

	const FSBPlayerSelectInfo& GetPlayerSelectInfo() const
	{
		return PlayerSelectInfo;
	}

public:
	/* FieldNotify Function */

	UFUNCTION(BlueprintPure, FieldNotify)
	UTexture2D* Get2DManagerTexture() const
	{
		if (ManagerCharacter == nullptr)
			return nullptr;
		return ManagerCharacter->Get2DCustomizingTexture();
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	UTextureRenderTarget2D* Get2DManagerRenderTarget() const
	{
		if (ManagerCharacter == nullptr)
			return nullptr;
		//FSlateBrush Brush;
		//Brush.SetResourceObject(ManagerCharacter->Get2DCustomizingRenderTarget());
		return ManagerCharacter->Get2DCustomizingRenderTarget();
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	bool IsEmptyPlayer() const
	{
		return PlayerSelectInfo.Name.IsEmpty();
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	const FString& GetPlayerName() const
	{
		return PlayerSelectInfo.Name;
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	ESBTier GetTier() const
	{
		return USBFunctionLibrary::GetTier(PlayerSelectInfo.Score);
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	FString GetSimpleCreatedDate() const
	{
		return PlayerSelectInfo.CreatedTime.ToString(TEXT("%Y.%m.%d"));
	}

public:
	/* 수동 실행 함수 */

	UFUNCTION(BlueprintCallable)
	void											InitViewModel(const TScriptInterface<ISBCustomizing>& NewManagerCharacter);

private:
	// 매니저 캐릭터의 스켈레톤과 매테리얼에 PlayerSelectInfo 반영
	void											UpdateManagerOutfit();

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	TScriptInterface<ISBCustomizing>				ManagerCharacter;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	FSBPlayerSelectInfo								PlayerSelectInfo;
};

/************************
 USBPlayerCreateViewModel
*************************/

UCLASS(BlueprintType)
class C_SB_API USBPlayerCreateViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/* FieldNotify Getter/Setter */

	void SetManagerCharacter(const TScriptInterface<ISBCustomizing>& NewManagerCharacter)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(ManagerCharacter, NewManagerCharacter))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Get2DManagerTexture);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Get2DManagerRenderTarget);
			UpdateManagerOutfit();
		}
	}

	void SetAthleteCharacter(const TScriptInterface<ISBCustomizing>& NewAthleteCharacter)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(AthleteCharacter, NewAthleteCharacter))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Get2DAthleteTexture);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Get2DAthleteRenderTarget);
			UpdateAthleteOutfit(ESBTableRowType::UNIFORM_COSTUME_TABLE);
			// TODO
		}
	}

	void SetName(const FText& NewName)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(Name, NewName))
		{
		}
	}

	void SetManagerTableId(const int32& NewManagerTableId)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(ManagerTableId, NewManagerTableId))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetManagerTableName);
			UpdateManagerOutfit();
		}
	}

	void SetUniformTableId(const int32& NewUniformTableId)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(UniformTableId, NewUniformTableId))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetUniformTableName);
			UpdateAthleteOutfit(ESBTableRowType::UNIFORM_COSTUME_TABLE);
		}
	}

	const TScriptInterface<ISBCustomizing>& GetManagerCharacter() const
	{
		return ManagerCharacter;
	}

	const TScriptInterface<ISBCustomizing>& GetAthleteCharacter() const
	{
		return AthleteCharacter;
	}

	const FText& GetName() const
	{
		return Name;
	}

	const int32& GetManagerTableId() const
	{
		return ManagerTableId;
	}

	const int32& GetUniformTableId() const
	{
		return UniformTableId;
	}

public:
	/* FieldNotify Function */

	UFUNCTION(BlueprintPure, FieldNotify)
	UTexture2D* Get2DManagerTexture() const
	{
		if (ManagerCharacter == nullptr)
			return nullptr;
		return ManagerCharacter->Get2DCustomizingTexture();
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	UTextureRenderTarget2D* Get2DManagerRenderTarget() const
	{
		if (ManagerCharacter == nullptr)
			return nullptr;
		return ManagerCharacter->Get2DCustomizingRenderTarget();
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	UTexture2D* Get2DAthleteTexture() const
	{
		if (AthleteCharacter == nullptr)
			return nullptr;
		return AthleteCharacter->Get2DCustomizingTexture();
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	UTextureRenderTarget2D* Get2DAthleteRenderTarget() const
	{
		if (AthleteCharacter == nullptr)
			return nullptr;
		return AthleteCharacter->Get2DCustomizingRenderTarget();
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetManagerTableName() const
	{
		if (ManagerTableRows.IsEmpty() == true)
			return FText();
		return FText::FromString(ManagerTableRows[ManagerTableIndex]->ClientName);
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetUniformTableName() const
	{
		if (UniformTableRows.IsEmpty() == true)
			return FText();
		return FText::FromString(UniformTableRows[UniformTableIndex]->ClientName);
	}

public:
	/* 수동 실행 함수 */

	UFUNCTION(BlueprintCallable)
	void											InitViewModel(const TScriptInterface<ISBCustomizing>& NewManagerCharacter, const TScriptInterface<ISBCustomizing>& NewAthleteCharacter, bool bUseCostumizing);

	UFUNCTION(BlueprintCallable)
	void											CreateNewPlayer(const int32& EmptyPlayerDBId);

	UFUNCTION(BlueprintCallable)
	void											GetNewPlayerInfo(FSBPlayerSelectInfo& NewPlayerInfo);

	UFUNCTION(BlueprintCallable)
	void											ChangeManagerTableId(const int32& Offset);

	UFUNCTION(BlueprintCallable)
	void											ChangeUniformTableId(const int32& Offset);

	// TODO

private:
	// 매니저 캐릭터의 스켈레톤과 매테리얼에 Model 정보 반영
	void											UpdateManagerOutfit();

	// 선수 캐릭터의 스켈레톤과 매테리얼에 Model 정보 반영
	void											UpdateAthleteOutfit(const ESBTableRowType& CostumeType);

	// 커스텀마이징 테이블 인덱싱 로드
	void											LoadTableRows();

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	TScriptInterface<ISBCustomizing>				ManagerCharacter;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	TScriptInterface<ISBCustomizing>				AthleteCharacter;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	FText											Name;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	int32											ManagerTableId;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	int32											UniformTableId;

	int32											PlayerDBId;

	int32											ManagerTableIndex;
	int32											UniformTableIndex;
	// TODO

	TArray<FSBTableRow*>							ManagerTableRows;
	TArray<FSBTableRow*>							UniformTableRows;
	// TODO
};

/************************
 USBPlayerSetupViewModel
*************************/

/**
 * PlayerSelect UI에서 사용할 뷰 모델
 */
UCLASS(BlueprintType)
class C_SB_API USBPlayerSetupViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	USBPlayerSetupViewModel();

public:
	/* FieldNotify Getter/Setter */

	void SetPlayerSelectViewModel1(USBPlayerSelectViewModel* NewPlayerSelectViewModel)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(PlayerSelectViewModel1, NewPlayerSelectViewModel))
		{
		}
	}

	void SetPlayerSelectViewModel2(USBPlayerSelectViewModel* NewPlayerSelectViewModel)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(PlayerSelectViewModel2, NewPlayerSelectViewModel))
		{
		}
	}

	void SetPlayerSelectViewModel3(USBPlayerSelectViewModel* NewPlayerSelectViewModel)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(PlayerSelectViewModel3, NewPlayerSelectViewModel))
		{
		}
	}

	void SetPlayerCreateViewModel(USBPlayerCreateViewModel* NewPlayerCreateViewModel)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(PlayerCreateViewModel, NewPlayerCreateViewModel))
		{
		}
	}

	USBPlayerSelectViewModel* GetPlayerSelectViewModel1() const
	{
		return PlayerSelectViewModel1;
	}

	USBPlayerSelectViewModel* GetPlayerSelectViewModel2() const
	{
		return PlayerSelectViewModel2;
	}

	USBPlayerSelectViewModel* GetPlayerSelectViewModel3() const
	{
		return PlayerSelectViewModel3;
	}

	USBPlayerCreateViewModel* GetPlayerCreateViewModel() const
	{
		return PlayerCreateViewModel;
	}

public:
	/* 수동 실행 함수 */

	UFUNCTION(BlueprintCallable)
	void											InitViewModel(TArray<TScriptInterface<ISBCustomizing>> SelectManagers, TScriptInterface<ISBCustomizing> CreateManager, TScriptInterface<ISBCustomizing> CreateAthlete, bool bUseCustomizing);

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	TObjectPtr<USBPlayerSelectViewModel>			PlayerSelectViewModel1;
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	TObjectPtr<USBPlayerSelectViewModel>			PlayerSelectViewModel2;
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	TObjectPtr<USBPlayerSelectViewModel>			PlayerSelectViewModel3;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	TObjectPtr<USBPlayerCreateViewModel>			PlayerCreateViewModel;
};