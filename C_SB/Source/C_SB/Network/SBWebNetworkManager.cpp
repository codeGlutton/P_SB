#include "SBWebNetworkManager.h"
#include "C_SB.h"

#include "ServerHttpPacketHandler.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "ILoginFlowModule.h"
#include "LoginFlowManager.h"

#ifndef GOOGLE_EX_SUBSYSTEM
const FName GOOGLE_EX_SUBSYSTEM(TEXT("GOOGLEEX"));
#endif

DEFINE_LOG_CATEGORY(LogWebManager);

/*************************
   USBWebNetworkManager
*************************/

USBWebNetworkManager::USBWebNetworkManager() :
	_GoogleOnlineSubSysytem(nullptr),
	_GoogleLoginFlowManager(nullptr),
	_GoogleLoginHandle(FDelegateHandle()),
	_bIsLoggedIn(false),
	_WaitingResTypeFlags(ESBHttpPktTypeFlag::NONE),
	_AccountIdPattern(TEXT("^[a-zA-Z0-9ㄱ-ㅎㅏ-ㅣ가-힣]{1,20}$")),
	_PasswordPattern(TEXT("^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[~!@#$%^&*_\\-+=`|\\\\:;\"',.?\\/])[a-zA-Z0-9~!@#$%^&*_\\-+=`|\\\\:;\"',.?\\/]{8,20}$")),
	_LoginPkt(),
	_ServerInfos()
{
}

void USBWebNetworkManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	_LoginPkt = xnew<Protocol::C_LOGIN>();

	/* 구글 로그인 설정 초기화 */

	FName GoogleIdentifier = Online::GetUtils()->GetOnlineIdentifier(GetWorld(), GOOGLE_EX_SUBSYSTEM);
	_GoogleOnlineSubSysytem = IOnlineSubsystem::Get(GoogleIdentifier);
	if (_GoogleOnlineSubSysytem != nullptr)
	{
		_GoogleOnlineSubSysytem->Init();

		/* 로그인 flow 생성 */

		ILoginFlowModule& LoginFlowModule = ILoginFlowModule::Get();
		_GoogleLoginFlowManager = LoginFlowModule.CreateLoginFlowManager();

		bool IsSuccess = _GoogleLoginFlowManager->AddLoginFlow(
			GoogleIdentifier,
			ILoginFlowManager::FOnDisplayPopup::CreateUObject(this, &USBWebNetworkManager::OpenGooglePopupWidget),
			ILoginFlowManager::FOnDisplayPopup::CreateUObject(this, &USBWebNetworkManager::OpenGoogleSingUpPopupWidget)
		);
		if (IsSuccess == false)
		{
			UE_LOG(LogWebManager, Warning, TEXT("Google Login flow can't be added"));
			return;
		}

		/* 로그인 결과 대리자 연결 */

		IOnlineIdentityPtr Identity = _GoogleOnlineSubSysytem->GetIdentityInterface();
		if (Identity == nullptr)
		{
			UE_LOG(LogWebManager, Warning, TEXT("Google Identity is none"));
			return;
		}
		_GoogleLoginHandle = Identity->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateLambda(
			[this](int32 LocalUserNumber, bool WasSuccessful, const FUniqueNetId& UserId, const FString& Error)
			{
				if (WasSuccessful == true)
				{
					Protocol::REQ_LOGIN_GOOGLE_ACCOUNT LoginGooglePkt;

					std::string Ut8AuthCode;
					Utils::UTF16To8(UserId.ToString(), OUT Ut8AuthCode);
					LoginGooglePkt.set_auth_code(Ut8AuthCode);

					SEND_REQ_PACKET(LoginGooglePkt);
				}
				else
				{
					UE_LOG(LogWebManager, Warning, TEXT("%s"), *Error);
					ErrorFromLogin(TEXT("로그인 실패"));
				}
			}
		));
	}
}

void USBWebNetworkManager::Deinitialize()
{
	Super::Deinitialize();
	xdelete(_LoginPkt);
	_LoginPkt = nullptr;

	if (_GoogleLoginHandle.IsValid() == true)
	{
		_GoogleLoginHandle.Reset();
	}

	if (_GoogleOnlineSubSysytem != nullptr)
	{
		{
			IOnlineIdentityPtr identity = _GoogleOnlineSubSysytem->GetIdentityInterface();
			identity->Logout(0);
		}

		FName GoogleIdentifier = Online::GetUtils()->GetOnlineIdentifier(GetWorld(), GOOGLE_EX_SUBSYSTEM);
		IOnlineSubsystem::Destroy(GoogleIdentifier);
		_GoogleOnlineSubSysytem = nullptr;
	}
}

void USBWebNetworkManager::RequestToCheckUsableName(const FText& AccountName)
{
	if (CanSendReqPkt(ESBHttpPktTypeFlag::ACCOUNT) == false)
		return;

	Protocol::REQ_CHECK_EXISTS_ACCOUNT CheckPkt;
	{
		std::string Ut8Id;
		Utils::UTF16To8(AccountName.ToString(), OUT Ut8Id);
		CheckPkt.set_account_name(Ut8Id);
	}

	if (SEND_REQ_PACKET(CheckPkt) == false)
	{
		ErrorFromCheckUsableName(TEXT("패킷 오류"));
	}
}

void USBWebNetworkManager::RequestToSignUp(const FText& AccountName, const FText& Password, const FText& RewirtePassword)
{
	if (CanSendReqPkt(ESBHttpPktTypeFlag::ACCOUNT) == false)
		return;

	if (CheckUsableAccountName(AccountName) == false)
	{
		ErrorFromSignUp(TEXT("부적절한 아이디"));
	}
	if (CheckUsablePassword(Password) == false)
	{
		ErrorFromSignUp(TEXT("부적절한 비밀번호"));
	}
	if (Password.ToString() != RewirtePassword.ToString())
	{
		ErrorFromSignUp(TEXT("비밀번호 불일치"));
	}

	Protocol::REQ_CREATE_ACCOUNT CreatePkt;
	{
		std::string Ut8Id;
		Utils::UTF16To8(AccountName.ToString(), OUT Ut8Id);
		CreatePkt.set_account_name(Ut8Id);
	}
	{
		std::string Ut8Password;
		Utils::UTF16To8(Password.ToString(), OUT Ut8Password);
		CreatePkt.set_password(Ut8Password);
	}

	if (SEND_REQ_PACKET(CreatePkt) == false)
	{
		ErrorFromSignUp(TEXT("패킷 오류"));
	}
}

void USBWebNetworkManager::RequestToLogin(const FText& AccountName, const FText& Password)
{
	if (_bIsLoggedIn == true)
	{
		UE_LOG(LogWebManager, Warning, TEXT("Already logged in"));
		return;
	}
	if (CanSendReqPkt(ESBHttpPktTypeFlag::ACCOUNT) == false)
		return;

	Protocol::REQ_LOGIN_ACCOUNT LoginPkt;
	{
		std::string Ut8Id;
		Utils::UTF16To8(AccountName.ToString(), OUT Ut8Id);
		LoginPkt.set_account_name(Ut8Id);
	}
	{
		std::string Ut8Password;
		Utils::UTF16To8(Password.ToString(), OUT Ut8Password);
		LoginPkt.set_password(Ut8Password);
	}

	if (SEND_REQ_PACKET(LoginPkt) == false)
	{
		ErrorFromLogin(TEXT("패킷 오류"));
	}
}

void USBWebNetworkManager::RequestToGoogleLogin()
{
	if (_GoogleOnlineSubSysytem == nullptr)
	{
		UE_LOG(LogWebManager, Warning, TEXT("Google online subsystem is nullptr"));
		return;
	}
	if (_bIsLoggedIn == true)
	{
		UE_LOG(LogWebManager, Warning, TEXT("Already logged in"));
		return;
	}
	if (CanSendReqPkt(ESBHttpPktTypeFlag::ACCOUNT) == false)
		return;

	IOnlineIdentityPtr Identity = _GoogleOnlineSubSysytem->GetIdentityInterface();
	if (Identity == nullptr || Identity->Login(0, FOnlineAccountCredentials{}) == false)
	{
		UE_LOG(LogWebManager, Warning, TEXT("Some google login settings are incorrect"));
		ErrorFromLogin(TEXT("로그인 실패"));
		return;
	}
}

void USBWebNetworkManager::RequestToConnectGameServer(const int32& ServerId)
{
	if (_bIsLoggedIn != true)
	{
		UE_LOG(LogWebManager, Warning, TEXT("Only account logged in can connect to game server"));
		return;
	}
	if (CanSendReqPkt(ESBHttpPktTypeFlag::ACCOUNT) == false)
		return;

	Protocol::REQ_CONNECT_GAME_SERVER ConnectPkt;
	ConnectPkt.set_account_id(_LoginPkt->account_id());
	ConnectPkt.set_server_id(ServerId);

	FString TokenValue;
	Utils::UTF8To16(_LoginPkt->token_value(), OUT TokenValue);
	if (SEND_REQ_PACKET_WITH_TOKEN(ConnectPkt, TokenValue) == false)
	{
		ErrorFromRecheck(TEXT("패킷 오류"));
	}
}

bool USBWebNetworkManager::CheckUsableAccountName(const FText& AccountName)
{
	FRegexMatcher Matcher(_AccountIdPattern.Pattern, AccountName.ToString());
	return Matcher.FindNext();
}

bool USBWebNetworkManager::CheckUsablePassword(const FText& Password)
{
	FRegexMatcher Matcher(_PasswordPattern.Pattern, Password.ToString());
	return Matcher.FindNext();
}

bool USBWebNetworkManager::IsWaitingSpecificRes(const ESBHttpPktTypeFlag HttpPktTypeFlag)
{
	return EnumHasAllFlags(_WaitingResTypeFlags, HttpPktTypeFlag);
}

void USBWebNetworkManager::GetServerInfos(TArray<FSBServerSelectInfo>& ServerInfos)
{
	for (auto& server : _ServerInfos)
	{
		ServerInfos.Add(FSBServerSelectInfo(server));
	}
}

void USBWebNetworkManager::BindGoogleWidget(UGoogleWidget* GoogleWidget)
{
	_OnGetGoogleLoginWidget.AddUObject(GoogleWidget, &UGoogleWidget::SetLoginWidget);
	_OnGetGoogleSignUpWidget.AddUObject(GoogleWidget, &UGoogleWidget::SetSignUpWidget);
	_OnGetGoogleLoginDismissedWidget.AddUObject(GoogleWidget, &UGoogleWidget::SetLoginDismissedWidget);
	_OnGetGoogleSignUpDismissedWidget.AddUObject(GoogleWidget, &UGoogleWidget::SetSignUpDismissedWidget);
}

void USBWebNetworkManager::UnbindGoogleWidget(UGoogleWidget* GoogleWidget)
{
	_OnGetGoogleLoginWidget.RemoveAll(GoogleWidget);
	_OnGetGoogleSignUpWidget.RemoveAll(GoogleWidget);
	_OnGetGoogleLoginDismissedWidget.RemoveAll(GoogleWidget);
	_OnGetGoogleSignUpDismissedWidget.RemoveAll(GoogleWidget);
}

void USBWebNetworkManager::RequestToRecheckServer()
{
	if (_bIsLoggedIn == false)
	{
		UE_LOG(LogWebManager, Warning, TEXT("Can't recheck before login"));
		return;
	}
	if (CanSendReqPkt(ESBHttpPktTypeFlag::ACCOUNT) == false)
		return;

	Protocol::REQ_RECHECK_SERVER RecheckPkt;
	{
		RecheckPkt.set_account_id(_LoginPkt->account_id());
	}

	FString TokenValue;
	Utils::UTF8To16(_LoginPkt->token_value(), OUT TokenValue);
	if (SEND_REQ_PACKET_WITH_TOKEN(RecheckPkt, TokenValue) == false)
	{
		ErrorFromRecheck(TEXT("패킷 오류"));
	}
}

void USBWebNetworkManager::ResponseToCheckUsableName(const Protocol::RES_CHECK_EXISTS_ACCOUNT& CheckPkt)
{
	ResponseToRes(OnIdCheckResRecved, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ResponseToSignUp(const Protocol::RES_CREATE_ACCOUNT& CreatePkt)
{
	ResponseToRes(OnSignUpResRecved, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ResponseToLogin(const Protocol::RES_LOGIN_ACCOUNT& LoginPkt)
{
	_LoginPkt->set_account_id(LoginPkt.account_id());
	_LoginPkt->set_token_value(LoginPkt.token_value());
	UpdateServerInfos(LoginPkt.server_list());
	_bIsLoggedIn = true;
	ResponseToRes(OnLoginResRecved, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ResponseToConnectGameServer(const Protocol::RES_CONNECT_GAME_SERVER& ConnectPkt)
{
	ResponseToRes(OnConnectResRecved, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ResponseToRecheck(const Protocol::RES_RECHECK_SERVER& RecheckPkt)
{
	UpdateServerInfos(RecheckPkt.server_list());
	ResponseToRes(OnRecheckResRecved, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ResponseToLogOut()
{
	_LoginPkt->Clear();
	_ServerInfos.Empty();
	_bIsLoggedIn = false;
	OnLogOutResRecved.Broadcast();
}

void USBWebNetworkManager::ErrorFromCheckUsableName(FString ErrStr)
{
	ErrorFromRes(ErrStr, OnIdCheckResRecved, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ErrorFromSignUp(FString ErrStr)
{
	ErrorFromRes(ErrStr, OnSignUpResRecved, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ErrorFromLogin(FString ErrStr)
{
	ErrorFromRes(ErrStr, OnLoginResRecved, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ErrorFromConnectGameServer(FString ErrStr)
{
	ErrorFromRes(ErrStr, OnConnectResRecved, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ErrorFromRecheck(FString ErrStr)
{
	ErrorFromRes(ErrStr, OnRecheckResRecved, ESBHttpPktTypeFlag::ACCOUNT);
}

bool USBWebNetworkManager::CanSendReqPkt(const ESBHttpPktTypeFlag PktType)
{
	if (EnumHasAllFlags(_WaitingResTypeFlags, PktType) == true)
	{
		UE_LOG(LogWebManager, Warning, TEXT("New requests cannot be send while waiting for a response packet"));
		return false;
	}
	EnumAddFlags(_WaitingResTypeFlags, PktType);
	OnChangeReqPktFlags.Broadcast();
	return true;
}

void USBWebNetworkManager::ResponseToRes(const FOnHttpResRecved& InDelegate, const ESBHttpPktTypeFlag PktType)
{
	UE_LOG(LogWebManager, Log, TEXT("Web pkt is successed"));
	InDelegate.Broadcast(true, "");
	EnumRemoveFlags(_WaitingResTypeFlags, PktType);
	OnChangeReqPktFlags.Broadcast();
}

void USBWebNetworkManager::ErrorFromRes(const FString& ErrStr, const FOnHttpResRecved& InDelegate, const ESBHttpPktTypeFlag PktType)
{
	UE_LOG(LogWebManager, Log, TEXT("Web pkt is failed by %s"), *ErrStr);
	InDelegate.Broadcast(false, ErrStr);
	EnumRemoveFlags(_WaitingResTypeFlags, PktType);
	OnChangeReqPktFlags.Broadcast();
}

void USBWebNetworkManager::UpdateServerInfos(const google::protobuf::RepeatedPtrField<Protocol::ServerSelectInfo>& ServerInfos)
{
	_ServerInfos.Empty();
	if (ServerInfos.empty() == true)
	{
		return;
	}

	for (auto& ServerInfo : ServerInfos)
	{
		_ServerInfos.Add(ServerInfo);
	}
	_ServerInfos.Sort([](const Protocol::ServerSelectInfo& Op1, const Protocol::ServerSelectInfo& Op2) {
		return Op1.server_id() < Op2.server_id();
	});
}

FOnPopupDismissed USBWebNetworkManager::OpenGooglePopupWidget(const TSharedRef<SWidget>& LoginWidget)
{
	_OnGetGoogleLoginWidget.Broadcast(LoginWidget);

	return FOnPopupDismissed::CreateLambda([this] {
		_OnGetGoogleLoginDismissedWidget.Broadcast();
		});
}

FOnPopupDismissed USBWebNetworkManager::OpenGoogleSingUpPopupWidget(const TSharedRef<SWidget>& SignUpWidget)
{
	_OnGetGoogleSignUpWidget.Broadcast(SignUpWidget);

	return FOnPopupDismissed::CreateLambda([this] {
		_OnGetGoogleSignUpDismissedWidget.Broadcast();
		});
}
