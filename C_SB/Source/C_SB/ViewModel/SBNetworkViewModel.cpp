#include "ViewModel/SBNetworkViewModel.h"
#include "SBNetworkManager.h"
#include "SBWebNetworkManager.h"
#include "GoogleWidget.h"
#include "C_SB.h"

/************************
 USBGoogleWidgetViewModel
*************************/

void USBGoogleWidgetViewModel::PreInit(UWidget* GoogleWidget)
{
	_GoogleWidget = Cast<UGoogleWidget>(GoogleWidget);
}

void USBGoogleWidgetViewModel::Initialize_Implementation()
{
	Super::Initialize_Implementation();

	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		if (IsValid(_GoogleWidget) == true)
		{
			WebNetworkManager->BindGoogleWidget(_GoogleWidget);
		}
	}
}

void USBGoogleWidgetViewModel::Deinitialize_Implementation()
{
	Super::Deinitialize_Implementation();
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		if (IsValid(_GoogleWidget) == true)
		{
			WebNetworkManager->UnbindGoogleWidget(_GoogleWidget);
		}
	}
}

/************************
    USBLoginViewModel
*************************/

void USBLoginViewModel::Notify_RegisterId(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
	UpdateRegisterId();
}

void USBLoginViewModel::Notify_IsPossibleId(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSignUpAttemptAllowed);
}

void USBLoginViewModel::Notify_NoticeTitle(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
}

void USBLoginViewModel::Notify_NoticeMsg(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
}

void USBLoginViewModel::Notify_IsPossiblePassword(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSignUpAttemptAllowed);
}

void USBLoginViewModel::Notify_IsPossibleRewritePassword(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSignUpAttemptAllowed);
}

void USBLoginViewModel::PreInit(USBNoticeViewModel* Notice, USBPostTextViewModel* RegisterIdPost, USBCheckSwitcherViewModel* PasswordSwitcher, USBCheckSwitcherViewModel* RewritePasswordSwitcher, USBGoogleWidgetViewModel* GoogleWidget)
{
	NoticeViewModel = Notice;
	RegisterIdPostTextViewModel = RegisterIdPost;
	PasswordSwithcerViewModel = PasswordSwitcher;
	RewritePasswordSwithcerViewModel = RewritePasswordSwitcher;
	GoogleWidgetViewModel = GoogleWidget;
}

void USBLoginViewModel::Initialize_Implementation()
{
	Super::Initialize_Implementation();

	UE_MVVM_BIND_ON_VALUE_CHANGED(USBPostTextViewModel, RegisterIdPostTextViewModel, TargetText, RegisterId);
	UE_MVVM_BIND_ON_VALUE_CHANGED(USBPostTextViewModel, RegisterIdPostTextViewModel, bIsSuccessed, IsPossibleId);
	UE_MVVM_BIND_ON_VALUE_CHANGED(USBCheckSwitcherViewModel, PasswordSwithcerViewModel, SwitcherIndex, IsPossiblePassword);
	UE_MVVM_BIND_ON_VALUE_CHANGED(USBCheckSwitcherViewModel, RewritePasswordSwithcerViewModel, SwitcherIndex, IsPossibleRewritePassword);
	UE_MVVM_BIND_ON_VALUE_CHANGED(USBNoticeViewModel, NoticeViewModel, NoticeTitle, NoticeTitle);
	UE_MVVM_BIND_ON_VALUE_CHANGED(USBNoticeViewModel, NoticeViewModel, NoticeMsg, NoticeMsg);

	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		WebNetworkManager->OnLoginResRecved.AddDynamic(this, &USBLoginViewModel::ProcessLoginResult);
		WebNetworkManager->OnIdCheckResRecved.AddDynamic(this, &USBLoginViewModel::ProcessRegisterIdCheckResult);
		WebNetworkManager->OnSignUpResRecved.AddDynamic(this, &USBLoginViewModel::ProcessSignUpResult);
		WebNetworkManager->OnLogOutResRecved.AddDynamic(this, &USBLoginViewModel::ProcessLogOutResult);
		WebNetworkManager->OnChangeReqPktFlags.AddDynamic(this, &USBLoginViewModel::UpdateWatingState);
	}
}

void USBLoginViewModel::Deinitialize_Implementation()
{
	Super::Deinitialize_Implementation();

	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBPostTextViewModel, RegisterIdPostTextViewModel, TargetText, RegisterId);
	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBPostTextViewModel, RegisterIdPostTextViewModel, bIsSuccessed, IsPossibleId);
	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBCheckSwitcherViewModel, PasswordSwithcerViewModel, SwitcherIndex, IsPossiblePassword);
	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBCheckSwitcherViewModel, RewritePasswordSwithcerViewModel, SwitcherIndex, IsPossibleRewritePassword);
	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBNoticeViewModel, NoticeViewModel, NoticeTitle, NoticeTitle);
	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBNoticeViewModel, NoticeViewModel, NoticeMsg, NoticeMsg);

	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		WebNetworkManager->OnLoginResRecved.RemoveDynamic(this, &USBLoginViewModel::ProcessLoginResult);
		WebNetworkManager->OnIdCheckResRecved.RemoveDynamic(this, &USBLoginViewModel::ProcessRegisterIdCheckResult);
		WebNetworkManager->OnSignUpResRecved.RemoveDynamic(this, &USBLoginViewModel::ProcessSignUpResult);
		WebNetworkManager->OnLogOutResRecved.RemoveDynamic(this, &USBLoginViewModel::ProcessLogOutResult);
		WebNetworkManager->OnChangeReqPktFlags.RemoveDynamic(this, &USBLoginViewModel::UpdateWatingState);
	}
}

void USBLoginViewModel::UpdateRegisterId()
{
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		RegisterIdPostTextViewModel->SetIsSuccessed(WebNetworkManager->CheckUsableAccountName(RegisterIdPostTextViewModel->GetTargetText()));
	}
}

void USBLoginViewModel::UpdateRegisterPassword()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSignUpAttemptAllowed);
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		PasswordSwithcerViewModel->SetSwitcherIndex(WebNetworkManager->CheckUsablePassword(RegisterPassword));
		UpdateRewritePassword();
	}
}

void USBLoginViewModel::UpdateRewritePassword()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSignUpAttemptAllowed);
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		RewritePasswordSwithcerViewModel->SetSwitcherIndex(RegisterPassword.IsEmpty() == false && RegisterPassword.EqualTo(RewritePassword) ? ECheckSwitcherState::Success : ECheckSwitcherState::Fail);
	}
}

void USBLoginViewModel::ResetAllPanel()
{
	ResetLoginPanel();
	ResetRegisterPanel();
	SetLoginPanelSwitcherIndex(ELoginPanelSwitcherState::Login);
}

void USBLoginViewModel::ResetLoginPanel()
{
	SetId(FText::GetEmpty());
	SetPassword(FText::GetEmpty());
}

void USBLoginViewModel::ResetRegisterPanel()
{
	RegisterIdPostTextViewModel->SetTargetText(FText::GetEmpty());
	SetRegisterPassword(FText::GetEmpty());
	SetRewritePassword(FText::GetEmpty());
}

void USBLoginViewModel::RequestLogin()
{
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		WebNetworkManager->RequestToLogin(Id, Password);
	}
}

void USBLoginViewModel::RequestGoogleLogin()
{
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		WebNetworkManager->RequestToGoogleLogin();
	}
}

void USBLoginViewModel::RequestCheckUsableId()
{
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		WebNetworkManager->RequestToCheckUsableName(RegisterIdPostTextViewModel->GetTargetText());
	}
}

void USBLoginViewModel::RequestSignUp()
{
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		WebNetworkManager->RequestToSignUp(RegisterIdPostTextViewModel->GetTargetText(), RegisterPassword, RewritePassword);
	}
}

void USBLoginViewModel::ProcessLoginResult(bool bIsSuccess, FString ErrStr)
{
	if (bIsSuccess == true)
	{
		SetIsLoggedIn(true);
		ResetLoginPanel();
	}
	else
	{
		NoticeViewModel->SetNoticeTitle(FText::FromString(TEXT("에러")));
		NoticeViewModel->SetNoticeMsg(FText::FromString(ErrStr));
		SetPassword(FText::GetEmpty());
	}
}

void USBLoginViewModel::ProcessLogOutResult()
{
	SetIsLoggedIn(false);
	SetWantToClose(false);
}

void USBLoginViewModel::ProcessRegisterIdCheckResult(bool bIsSuccess, FString ErrStr)
{
	if (bIsSuccess == true)
	{
		NoticeViewModel->SetNoticeTitle(FText::FromString(TEXT("알림")));
		NoticeViewModel->SetNoticeMsg(FText::FromString(TEXT("사용가능한 ID")));
	}
	else
	{
		NoticeViewModel->SetNoticeTitle(FText::FromString(TEXT("에러")));
		NoticeViewModel->SetNoticeMsg(FText::FromString(ErrStr));
	}
}

void USBLoginViewModel::ProcessSignUpResult(bool bIsSuccess, FString ErrStr)
{
	if (bIsSuccess == true)
	{
		NoticeViewModel->SetNoticeTitle(FText::FromString(TEXT("알림")));
		NoticeViewModel->SetNoticeMsg(FText::FromString(TEXT("회원가입 성공")));
		SetLoginPanelSwitcherIndex(ELoginPanelSwitcherState::Login);
	}
	else
	{
		NoticeViewModel->SetNoticeTitle(FText::FromString(TEXT("에러")));
		NoticeViewModel->SetNoticeMsg(FText::FromString(ErrStr));
	}
}

void USBLoginViewModel::UpdateWatingState()
{
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		SetIsWaitingAccountPkt(WebNetworkManager->IsWaitingSpecificRes(ESBHttpPktTypeFlag::ACCOUNT));
	}
}

/************************
 USBServerConnectViewModel
*************************/

void USBServerConnectViewModel::SetServerData(const FSBServerSelectInfo& ServerSelectInfo)
{
	SetServerName(FText::FromString(ServerSelectInfo.Name));
	SetPercent(ServerSelectInfo.Dencity);
	SetServerId(ServerSelectInfo.ServerId);
}

void USBServerConnectViewModel::RequestConnectToGameServer()
{
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		WebNetworkManager->RequestToConnectGameServer(ServerId);
	}
}

/************************
 USBServerSelectViewModel
*************************/

void USBServerSelectViewModel::Notify_NoticeTitle(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
}

void USBServerSelectViewModel::Notify_NoticeMsg(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
}

void USBServerSelectViewModel::PreInit(USBNoticeViewModel* Notice)
{
	NoticeViewModel = Notice;
}

void USBServerSelectViewModel::Initialize_Implementation()
{
	Super::Initialize_Implementation();

	UE_MVVM_BIND_ON_VALUE_CHANGED(USBNoticeViewModel, NoticeViewModel, NoticeTitle, NoticeTitle);
	UE_MVVM_BIND_ON_VALUE_CHANGED(USBNoticeViewModel, NoticeViewModel, NoticeMsg, NoticeMsg);

	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		WebNetworkManager->OnLoginResRecved.AddDynamic(this, &USBServerSelectViewModel::ProcessLoginResult);
		WebNetworkManager->OnLogOutResRecved.AddDynamic(this, &USBServerSelectViewModel::ProcessLogOutResult);
		WebNetworkManager->OnConnectResRecved.AddDynamic(this, &USBServerSelectViewModel::ProcessConnectToGameServersResult);
		WebNetworkManager->OnRecheckResRecved.AddDynamic(this, &USBServerSelectViewModel::ProcessUpdateGameServersResult);
		WebNetworkManager->OnChangeReqPktFlags.AddDynamic(this, &USBServerSelectViewModel::UpdateAccoutWatingState);
	}

	auto* NetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBNetworkManager>();
	if (IsValid(NetworkManager) == true)
	{
		NetworkManager->OnLoginPktRecved.AddDynamic(this, &USBServerSelectViewModel::ProcessGameServerValidationResult);
		NetworkManager->OnChangeReqPktFlags.AddDynamic(this, &USBServerSelectViewModel::UpdateEnterFlowWatingState);
	}
}

void USBServerSelectViewModel::Deinitialize_Implementation()
{
	Super::Deinitialize_Implementation();

	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBNoticeViewModel, NoticeViewModel, NoticeTitle, NoticeTitle);
	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBNoticeViewModel, NoticeViewModel, NoticeMsg, NoticeMsg);

	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		WebNetworkManager->OnLoginResRecved.RemoveDynamic(this, &USBServerSelectViewModel::ProcessLoginResult);
		WebNetworkManager->OnLogOutResRecved.RemoveDynamic(this, &USBServerSelectViewModel::ProcessLogOutResult);
		WebNetworkManager->OnConnectResRecved.RemoveDynamic(this, &USBServerSelectViewModel::ProcessConnectToGameServersResult);
		WebNetworkManager->OnRecheckResRecved.RemoveDynamic(this, &USBServerSelectViewModel::ProcessUpdateGameServersResult);
		WebNetworkManager->OnChangeReqPktFlags.RemoveDynamic(this, &USBServerSelectViewModel::UpdateAccoutWatingState);
	}

	auto* NetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBNetworkManager>();
	if (IsValid(NetworkManager) == true)
	{
		NetworkManager->OnLoginPktRecved.RemoveDynamic(this, &USBServerSelectViewModel::ProcessGameServerValidationResult);
		NetworkManager->OnChangeReqPktFlags.RemoveDynamic(this, &USBServerSelectViewModel::UpdateEnterFlowWatingState);
	}
}

void USBServerSelectViewModel::GetServerInfos(TArray<FSBServerSelectInfo>& ServerSelectInfos)
{
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		WebNetworkManager->GetServerInfos(ServerSelectInfos);
	}
	SetNeedUpdateServers(false);
}

void USBServerSelectViewModel::ProcessLoginResult(bool bIsSuccess, FString ErrStr)
{
	if (bIsSuccess == true)
	{
		SetNeedUpdateServers(true);
	}
	else
	{
		NoticeViewModel->SetNoticeTitle(FText::FromString(TEXT("에러")));
		NoticeViewModel->SetNoticeMsg(FText::FromString(ErrStr));
	}
}

void USBServerSelectViewModel::ProcessLogOutResult()
{
	SetIsVerified(false);
	SetWantToClose(false);
}

void USBServerSelectViewModel::ProcessConnectToGameServersResult(bool bIsSuccess, FString ErrStr)
{
	if (bIsSuccess == false)
	{
		NoticeViewModel->SetNoticeTitle(FText::FromString(TEXT("에러")));
		NoticeViewModel->SetNoticeMsg(FText::FromString(ErrStr));
	}
}

void USBServerSelectViewModel::ProcessUpdateGameServersResult(bool bIsSuccess, FString ErrStr)
{
	if (bIsSuccess == true)
	{
		SetNeedUpdateServers(true);
	}
	else
	{
		NoticeViewModel->SetNoticeTitle(FText::FromString(TEXT("에러")));
		NoticeViewModel->SetNoticeMsg(FText::FromString(ErrStr));
	}
}

void USBServerSelectViewModel::ProcessGameServerValidationResult(bool bIsSuccess, FString ErrStr)
{
	if (bIsSuccess == true)
	{
		SetIsVerified(true);
	}
	else
	{
		NoticeViewModel->SetNoticeTitle(FText::FromString(TEXT("에러")));
		NoticeViewModel->SetNoticeMsg(FText::FromString(ErrStr));
	}
}

void USBServerSelectViewModel::UpdateAccoutWatingState()
{
	auto* WebNetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBWebNetworkManager>();
	if (IsValid(WebNetworkManager) == true)
	{
		SetIsWaitingAccountPkt(WebNetworkManager->IsWaitingSpecificRes(ESBHttpPktTypeFlag::ACCOUNT));
	}
}

void USBServerSelectViewModel::UpdateEnterFlowWatingState()
{
	auto* NetworkManager = GetWorld()->GetGameInstance()->GetSubsystem<USBNetworkManager>();
	if (IsValid(NetworkManager) == true)
	{
		SetIsWaitingAccountPkt(NetworkManager->IsWaitingSpecificPkt(ESBTcpPktTypeFlag::ENTER_FLOW));
	}
}

/************************
  USBTitleSceneViewModel
*************************/

void USBTitleSceneViewModel::Notify_LoginWantToClose(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetEnterWidgetsVisibility);
}

void USBTitleSceneViewModel::Notify_ServerSelectWantToClose(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetEnterWidgetsVisibility);
}

void USBTitleSceneViewModel::Notify_IsLoggedIn(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
	if (LoginViewModel->GetIsLoggedIn() == true)
	{
		SetEnterPanelSwitcherIndex(EEnterPanelSwitcherState::ServerSelect);
	}
	else
	{
		SetEnterPanelSwitcherIndex(EEnterPanelSwitcherState::Login);
	}
}

void USBTitleSceneViewModel::Notify_IsVerified(UObject* Obj, UE::FieldNotification::FFieldId FieIdId)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetEnterWidgetsVisibility);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetLoginTrialVisibility);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetPlayGameTrialVisibility);
}

void USBTitleSceneViewModel::PreInit(USBLoginViewModel* Login, USBServerSelectViewModel* ServerSelect)
{
	LoginViewModel = Login;
	ServerSelectViewModel = ServerSelect;
}

void USBTitleSceneViewModel::Initialize_Implementation()
{
	Super::Initialize_Implementation();

	UE_MVVM_BIND_ON_VALUE_CHANGED(USBLoginViewModel, LoginViewModel, bWantToClose, LoginWantToClose);
	UE_MVVM_BIND_ON_VALUE_CHANGED(USBServerSelectViewModel, ServerSelectViewModel, bWantToClose, ServerSelectWantToClose);
	UE_MVVM_BIND_ON_VALUE_CHANGED(USBLoginViewModel, LoginViewModel, bIsLoggedIn, IsLoggedIn);
	UE_MVVM_BIND_ON_VALUE_CHANGED(USBServerSelectViewModel, ServerSelectViewModel, bIsVerified, IsVerified);
}

void USBTitleSceneViewModel::Deinitialize_Implementation()
{
	Super::Deinitialize_Implementation();

	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBLoginViewModel, LoginViewModel, bWantToClose, LoginWantToClose);
	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBServerSelectViewModel, ServerSelectViewModel, bWantToClose, ServerSelectWantToClose);
	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBLoginViewModel, LoginViewModel, bIsLoggedIn, IsLoggedIn);
	UE_MVVM_UNBIND_ON_VALUE_CHANGED(USBServerSelectViewModel, ServerSelectViewModel, bIsVerified, IsVerified);
}

void USBTitleSceneViewModel::OpenEnterPanels()
{
	LoginViewModel->SetWantToClose(false);
	ServerSelectViewModel->SetWantToClose(false);
}
