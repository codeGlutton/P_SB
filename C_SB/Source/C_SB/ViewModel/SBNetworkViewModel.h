#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "SBBaseWidgetViewModel.h"
#include "Wrappers.h"
#include "SBNetworkViewModel.generated.h"

/************************
 USBGoogleWidgetViewModel
*************************/

UCLASS()
class C_SB_API USBGoogleWidgetViewModel : public USBViewModelBase
{
	GENERATED_BODY()

public:
	USBGoogleWidgetViewModel() : WidgetVisibility(ESlateVisibility::Collapsed), _GoogleWidget(nullptr)
	{
	}

public:
	/* FieldNotify Getter/Setter */

	void SetWidgetVisibility(ESlateVisibility NewWidgetVisibility)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(WidgetVisibility, NewWidgetVisibility))
		{
		}
	}

	ESlateVisibility GetWidgetVisibility() const
	{
		return WidgetVisibility;
	}

private:
	/* 수동 실행 함수 */

	UFUNCTION(BlueprintCallable)
	void											PreInit(UWidget* GoogleWidget);

	virtual void									Initialize_Implementation() override;
	virtual void									Deinitialize_Implementation() override;

public:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	ESlateVisibility								WidgetVisibility;

private:
	TObjectPtr<class UGoogleWidget>					_GoogleWidget;
};

/************************
	USBLoginViewModel
*************************/

namespace ELoginPanelSwitcherState
{
	enum Type
	{
		Login = 0,
		Register = 1
	};
}

UCLASS()
class C_SB_API USBLoginViewModel : public USBViewModelBase
{
	GENERATED_BODY()

public:
	USBLoginViewModel() : 
		Id(FText::GetEmpty()), 
		Password(FText::GetEmpty()), 
		bIsWaitingAccountPkt(false),
		RegisterPassword(FText::GetEmpty()),
		RewritePassword(FText::GetEmpty()),
		LoginPanelSwitcherIndex(ELoginPanelSwitcherState::Login),
		bIsLoggedIn(false),
		NoticeViewModel(nullptr),
		RegisterIdPostTextViewModel(nullptr),
		PasswordSwithcerViewModel(nullptr),
		RewritePasswordSwithcerViewModel(nullptr),
		GoogleWidgetViewModel(nullptr)
	{
	}

public:
	/* FieldNotify Getter/Setter */

	void SetId(const FText& NewId)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(Id, NewId))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsLoginAttemptAllowed);
		}
	}

	void SetPassword(const FText& NewPassword)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(Password, NewPassword))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsLoginAttemptAllowed);
		}
	}

	void SetIsWaitingAccountPkt(bool bNewIsWaitingAccountPkt)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(bIsWaitingAccountPkt, bNewIsWaitingAccountPkt))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsLoginAttemptAllowed);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSignUpAttemptAllowed);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCircularThrobberVisibility);
		}
	}

	void SetRegisterPassword(const FText& NewRegisterPassword)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(RegisterPassword, NewRegisterPassword))
		{
			UpdateRegisterPassword();
		}
	}

	void SetRewritePassword(const FText& NewRewritePassword)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(RewritePassword, NewRewritePassword))
		{
			UpdateRewritePassword();
		}
	}

	void SetLoginPanelSwitcherIndex(int32 NewLoginPanelSwitcherIndex)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(LoginPanelSwitcherIndex, NewLoginPanelSwitcherIndex))
		{
			if (LoginPanelSwitcherIndex == ELoginPanelSwitcherState::Login)
			{
				ResetRegisterPanel();
			}
			else
			{
				ResetLoginPanel();
			}
		}
	}

	void SetIsLoggedIn(bool bNewIsLoggedIn)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(bIsLoggedIn, bNewIsLoggedIn))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetWidgetVisibility);
		}
	}

	void SetWantToClose(bool bNewWantToClose)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(bWantToClose, bNewWantToClose))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetWidgetVisibility);
		}
	}

	const FText& GetId() const
	{
		return Id;
	}

	const FText& GetPassword() const
	{
		return Password;
	}

	bool GetIsWaitingAccountPkt() const
	{
		return bIsWaitingAccountPkt;
	}

	const FText& GetRegisterPassword() const
	{
		return RegisterPassword;
	}

	const FText& GetRewritePassword() const
	{
		return RewritePassword;
	}

	int32 GetLoginPanelSwitcherIndex() const
	{
		return LoginPanelSwitcherIndex;
	}

	bool GetIsLoggedIn() const
	{
		return bIsLoggedIn;
	}

	bool GetWantToClose() const
	{
		return bWantToClose;
	}

protected:
	/* Notify From Inner ViewModel */

	UE_MVVM_ADD_NOTIFY(RegisterId);
	UE_MVVM_ADD_NOTIFY(IsPossibleId);

	UE_MVVM_ADD_NOTIFY(IsPossiblePassword);
	UE_MVVM_ADD_NOTIFY(IsPossibleRewritePassword);

	UE_MVVM_ADD_NOTIFY(NoticeTitle);
	UE_MVVM_ADD_NOTIFY(NoticeMsg);

public:
	/* FieldNotify Function */

	UFUNCTION(BlueprintPure, FieldNotify)
	bool IsLoginAttemptAllowed() const
	{
		return !bIsWaitingAccountPkt && !Id.IsEmpty() && !Password.IsEmpty();
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	bool IsSignUpAttemptAllowed() const
	{
		if (IsValid(PasswordSwithcerViewModel) == false || IsValid(RewritePasswordSwithcerViewModel) == false || IsValid(RegisterIdPostTextViewModel) == false)
		{
			return false;
		}
		return PasswordSwithcerViewModel->IsSuccess() && RewritePasswordSwithcerViewModel->IsSuccess() && !bIsWaitingAccountPkt && RegisterIdPostTextViewModel->GetIsSuccessed();
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	ESlateVisibility GetCircularThrobberVisibility() const
	{
		return bIsWaitingAccountPkt == true ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	ESlateVisibility GetWidgetVisibility() const
	{
		return (bIsLoggedIn == true || bWantToClose == true) ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible;
	}

private:
	/* 수동 실행 함수 */

	UFUNCTION(BlueprintCallable)
	void											PreInit(USBNoticeViewModel* Notice, USBPostTextViewModel* RegisterIdPost, USBCheckSwitcherViewModel* PasswordSwitcher, USBCheckSwitcherViewModel* RewritePasswordSwitcher, USBGoogleWidgetViewModel* GoogleWidget);

	virtual void									Initialize_Implementation() override;
	virtual void									Deinitialize_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void											UpdateRegisterId();

	UFUNCTION(BlueprintCallable)
	void											UpdateRegisterPassword();

	UFUNCTION(BlueprintCallable)
	void											UpdateRewritePassword();

	UFUNCTION(BlueprintCallable)
	void											ResetAllPanel();

	UFUNCTION(BlueprintCallable)
	void											ResetLoginPanel();

	UFUNCTION(BlueprintCallable)
	void											ResetRegisterPanel();

	UFUNCTION(BlueprintCallable)
	void											RequestLogin();

	UFUNCTION(BlueprintCallable)
	void											RequestGoogleLogin();

	UFUNCTION(BlueprintCallable)
	void											RequestCheckUsableId();

	UFUNCTION(BlueprintCallable)
	void											RequestSignUp();

private:
	/* 콜백 함수 */

	UFUNCTION()
	void											ProcessLoginResult(bool bIsSuccess, FString ErrStr);

	UFUNCTION()
	void											ProcessLogOutResult();

	UFUNCTION()
	void											ProcessRegisterIdCheckResult(bool bIsSuccess, FString ErrStr);

	UFUNCTION()
	void											ProcessSignUpResult(bool bIsSuccess, FString ErrStr);

	UFUNCTION()
	void											UpdateWatingState();

private:
	/* 로그인 Flow */

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	FText											Id;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	FText											Password;

private:
	/* 회원가입 Flow */

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = "SetIsWaitingAccountPkt", Getter = "GetIsWaitingAccountPkt", meta = (AllowPrivateAccess))
	bool											bIsWaitingAccountPkt;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	FText											RegisterPassword;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	FText											RewritePassword;

private:
	/* 공용 Flow */

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	int32											LoginPanelSwitcherIndex;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = "SetIsLoggedIn", Getter = "GetIsLoggedIn", meta = (AllowPrivateAccess))
	bool											bIsLoggedIn;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = "SetWantToClose", Getter = "GetWantToClose", meta = (AllowPrivateAccess))
	bool											bWantToClose;

private:
	/* 하위 ViewModel */

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<USBNoticeViewModel>					NoticeViewModel;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<USBPostTextViewModel>				RegisterIdPostTextViewModel;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<USBCheckSwitcherViewModel>			PasswordSwithcerViewModel;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<USBCheckSwitcherViewModel>			RewritePasswordSwithcerViewModel;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<USBGoogleWidgetViewModel>			GoogleWidgetViewModel;
};

/************************
 USBServerConnectViewModel
*************************/

UCLASS()
class C_SB_API USBServerConnectViewModel : public USBViewModelBase
{
	GENERATED_BODY()

public:
	USBServerConnectViewModel() : ServerName(FText::GetEmpty()), Percent(0.f), ServerId(0)
	{
	}

public:
	/* FieldNotify Getter/Setter */

	void SetServerName(const FText& NewServerName)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(ServerName, NewServerName))
		{
		}
	}

	void SetPercent(float NewPercent)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(Percent, NewPercent))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetWidgetVisibility);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetProgressBarFillColorAndOpacity);
		}
	}

	void SetServerId(int32 NewServerId)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(ServerId, NewServerId))
		{
		}
	}

	const FText& GetServerName() const
	{
		return ServerName;
	}

	float GetPercent() const
	{
		return Percent;
	}

	int32 GetServerId() const
	{
		return ServerId;
	}

public:
	/* FieldNotify Function */

	UFUNCTION(BlueprintPure, FieldNotify)
	ESlateVisibility GetWidgetVisibility() const
	{
		return Percent >= 1.f ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible;
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	FLinearColor GetProgressBarFillColorAndOpacity() const
	{
		return FLinearColor(1.f, 1.f - Percent, 1.f - Percent, 1.f);
	}

private:
	/* 수동 실행 함수 */

	UFUNCTION(BlueprintCallable)
	void											SetServerData(const FSBServerSelectInfo& ServerSelectInfo);

	UFUNCTION(BlueprintCallable)
	void											RequestConnectToGameServer();

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	FText											ServerName;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	float											Percent;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	int32											ServerId;
};

/************************
 USBServerSelectViewModel
*************************/

UCLASS()
class C_SB_API USBServerSelectViewModel : public USBViewModelBase
{
	GENERATED_BODY()

public:
	USBServerSelectViewModel() :
		bIsVerified(false),
		bWantToClose(false),
		bNeedUpdateServers(false),
		bIsWaitingAccountPkt(false),
		bIsWaitingEnterFlowPkt(false)
	{
	}

public:
	/* FieldNotify Getter/Setter */

	void SetIsVerified(bool bNewIsVerified)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(bIsVerified, bNewIsVerified))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetWidgetVisibility);
		}
	}

	void SetWantToClose(bool bNewWantToClose)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(bWantToClose, bNewWantToClose))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetWidgetVisibility);
		}
	}

	void SetNeedUpdateServers(bool bNewNeedUpdateServers)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(bNeedUpdateServers, bNewNeedUpdateServers))
		{
		}
	}

	void SetIsWaitingAccountPkt(bool bNewIsWaitingAccountPkt)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(bIsWaitingAccountPkt, bNewIsWaitingAccountPkt))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCircularThrobberVisibility);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetIsWaitingPkts);
		}
	}

	void SetIsWaitingEnterFlowPkt(bool bNewIsWaitingEnterFlowPkt)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(bIsWaitingEnterFlowPkt, bNewIsWaitingEnterFlowPkt))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCircularThrobberVisibility);
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetIsWaitingPkts);
		}
	}

	bool GetIsVerified() const
	{
		return bIsVerified;
	}

	bool GetWantToClose() const
	{
		return bWantToClose;
	}
	
	bool GetNeedUpdateServers() const
	{
		return bNeedUpdateServers;
	}

	bool GetIsWaitingAccountPkt() const
	{
		return bIsWaitingAccountPkt;
	}

	bool GetIsWaitingEnterFlowPkt() const
	{
		return bIsWaitingEnterFlowPkt;
	}

protected:
	/* Notify From Inner ViewModel */

	UE_MVVM_ADD_NOTIFY(NoticeTitle);
	UE_MVVM_ADD_NOTIFY(NoticeMsg);

public:
	/* FieldNotify Function */

	UFUNCTION(BlueprintPure, FieldNotify)
	ESlateVisibility GetCircularThrobberVisibility() const
	{
		return (bIsWaitingAccountPkt || bIsWaitingAccountPkt) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	bool GetIsWaitingPkts() const
	{
		return bIsWaitingAccountPkt || bIsWaitingAccountPkt;
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	ESlateVisibility GetWidgetVisibility() const
	{
		return (bIsVerified == true || bWantToClose == true) ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible;
	}

private:
	/* 수동 실행 함수 */

	UFUNCTION(BlueprintCallable)
	void											PreInit(USBNoticeViewModel* Notice);

	virtual void									Initialize_Implementation() override;
	virtual void									Deinitialize_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void											GetServerInfos(TArray<FSBServerSelectInfo>& ServerSelectInfos);

private:
	/* 콜백 함수 */

	UFUNCTION()
	void											ProcessLoginResult(bool bIsSuccess, FString ErrStr);

	UFUNCTION()
	void											ProcessLogOutResult();

	UFUNCTION()
	void											ProcessConnectToGameServersResult(bool bIsSuccess, FString ErrStr);

	UFUNCTION()
	void											ProcessUpdateGameServersResult(bool bIsSuccess, FString ErrStr);

	UFUNCTION()
	void											ProcessGameServerValidationResult(bool bIsSuccess, FString ErrStr);

	UFUNCTION()
	void											UpdateAccoutWatingState();

	UFUNCTION()
	void											UpdateEnterFlowWatingState();

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = "SetIsVerified", Getter = "GetIsVerified", meta = (AllowPrivateAccess))
	bool											bIsVerified;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = "SetWantToClose", Getter = "GetWantToClose", meta = (AllowPrivateAccess))
	bool											bWantToClose;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = "SetNeedUpdateServers", Getter = "GetNeedUpdateServers", meta = (AllowPrivateAccess))
	bool											bNeedUpdateServers;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = "SetIsWaitingAccountPkt", Getter = "GetIsWaitingAccountPkt", meta = (AllowPrivateAccess))
	bool											bIsWaitingAccountPkt;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = "SetIsWaitingEnterFlowPkt", Getter = "GetIsWaitingEnterFlowPkt", meta = (AllowPrivateAccess))
	bool											bIsWaitingEnterFlowPkt;

private:
	/* 하위 ViewModel */

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<USBNoticeViewModel>					NoticeViewModel;
};

/************************
  USBTitleSceneViewModel
*************************/

namespace EEnterPanelSwitcherState
{
	enum Type
	{
		Login,
		ServerSelect
	};
}

UCLASS()
class C_SB_API USBTitleSceneViewModel : public USBViewModelBase
{
	GENERATED_BODY()

public:
	USBTitleSceneViewModel() :
		EnterPanelSwitcherIndex(0),
		LoginViewModel(nullptr),
		ServerSelectViewModel(nullptr)
	{
	}

public:
	/* FieldNotify Getter/Setter */

	void SetEnterPanelSwitcherIndex(int32 NewEnterPanelSwitcherIndex)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(EnterPanelSwitcherIndex, NewEnterPanelSwitcherIndex))
		{
		}
	}

	int32 GetEnterPanelSwitcherIndex() const
	{
		return EnterPanelSwitcherIndex;
	}

public:
	/* FieldNotify Function */

	UFUNCTION(BlueprintPure, FieldNotify)
	ESlateVisibility GetEnterWidgetsVisibility() const
	{
		if (IsValid(LoginViewModel) == false || IsValid(ServerSelectViewModel) == false)
		{
			return ESlateVisibility::SelfHitTestInvisible;
		}
		if (LoginViewModel->GetWantToClose() || ServerSelectViewModel->GetWantToClose() || ServerSelectViewModel->GetIsVerified())
		{
			return ESlateVisibility::Collapsed;
		}
		return ESlateVisibility::SelfHitTestInvisible;
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	ESlateVisibility GetLoginTrialVisibility() const
	{
		if (IsValid(ServerSelectViewModel) == false)
		{
			return ESlateVisibility::Visible;
		}
		return ServerSelectViewModel->GetIsVerified() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	ESlateVisibility GetPlayGameTrialVisibility() const
	{
		if (IsValid(ServerSelectViewModel) == false)
		{
			return ESlateVisibility::Collapsed;
		}
		return ServerSelectViewModel->GetIsVerified() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	}

protected:
	/* Notify From Inner ViewModel */

	UE_MVVM_ADD_NOTIFY(LoginWantToClose);
	UE_MVVM_ADD_NOTIFY(ServerSelectWantToClose);
	UE_MVVM_ADD_NOTIFY(IsLoggedIn);
	UE_MVVM_ADD_NOTIFY(IsVerified);

private:
	/* 수동 실행 함수 */

	UFUNCTION(BlueprintCallable)
	void											PreInit(USBLoginViewModel* Login, USBServerSelectViewModel* ServerSelect);

	virtual void									Initialize_Implementation() override;
	virtual void									Deinitialize_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void											OpenEnterPanels();

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess))
	int32											EnterPanelSwitcherIndex;

private:
	/* 하위 ViewModel */

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<USBLoginViewModel>					LoginViewModel;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<USBServerSelectViewModel>			ServerSelectViewModel;
};