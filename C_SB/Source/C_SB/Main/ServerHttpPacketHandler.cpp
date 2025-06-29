#include "ServerHttpPacketHandler.h"
#include "C_SB.h"
#include "SBWebNetworkManager.h"

#include "SBNetworkManager.h"
#include "ServerPacketHandler.h"

DEFINE_LOG_CATEGORY(LogHttpHandler);

/*************************
  ServerHttpPacketHandler
*************************/

const FString ServerHttpPacketHandler::s_BaseUrl = TEXT("https://localhost:5202/account");

USBWebNetworkManager* const ServerHttpPacketHandler::GetWebNetworkManager()
{
	if (const UGameInstance* GameInstance = Utils::GetGameInstance())
	{
		return GameInstance->GetSubsystem<USBWebNetworkManager>();
	}
	return nullptr;
}

bool ServerHttpPacketHandler::DebugHttpFailPkt(const FHttpRequestPtr& Request, const FHttpResponsePtr& Response)
{
	if (Request == nullptr || Request->DoesSharedInstanceExist() == false)
	{
		UE_LOG(LogHttpHandler, Warning, TEXT("Http request does not exists."));
		return true;
	}

	if (Response == nullptr)
	{
		UE_LOG(LogHttpHandler, Warning, TEXT("Http response is nullptr."));
		return true;
	}

	if (EHttpResponseCodes::IsOk(Response->GetResponseCode()) == false)
	{
		UE_LOG(LogHttpHandler, Warning, TEXT("Http response fail code: %d"), Response->GetResponseCode());
		return true;
	}

	UE_LOG(LogHttpHandler, Warning, TEXT("Http response failed for unknown reason"));
	return false;
}

void ServerHttpPacketHandler::RecvReqCheckExistsAccount(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
	USBWebNetworkManager* WebNetworkManager = GetWebNetworkManager();
	if (WebNetworkManager == nullptr)
		return;

	Protocol::RES_CHECK_EXISTS_ACCOUNT ResPkt;
	FString ErrStr;
	if (PreprocessHttpRecv(Request, Response, WasSuccessful, OUT ResPkt, OUT ErrStr) == false)
	{
		WebNetworkManager->ErrorFromCheckUsableName(ErrStr);
		return;
	}
	if (ResPkt.success() == false)
	{
		WebNetworkManager->ErrorFromCheckUsableName(TEXT("존재하거나 불가능한 ID"));
		return;
	}

	WebNetworkManager->ResponseToCheckUsableName(ResPkt);
}

void ServerHttpPacketHandler::RecvReqCreateAccount(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
	USBWebNetworkManager* WebNetworkManager = GetWebNetworkManager();
	if (WebNetworkManager == nullptr)
		return;

	Protocol::RES_CREATE_ACCOUNT ResPkt;
	FString ErrStr;
	if (PreprocessHttpRecv(Request, Response, WasSuccessful, OUT ResPkt, OUT ErrStr) == false)
	{
		WebNetworkManager->ErrorFromSignUp(ErrStr);
		return;
	}
	if (ResPkt.success() == false)
	{
		WebNetworkManager->ErrorFromSignUp(TEXT("부적절한 회원정보"));
		return;
	}

	WebNetworkManager->ResponseToSignUp(ResPkt);
}

void ServerHttpPacketHandler::RecvReqLoginAccount(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
	USBWebNetworkManager* WebNetworkManager = GetWebNetworkManager();
	if (WebNetworkManager == nullptr)
		return;

	Protocol::RES_LOGIN_ACCOUNT ResPkt;
	FString ErrStr;
	if (PreprocessHttpRecv(Request, Response, WasSuccessful, OUT ResPkt, OUT ErrStr) == false)
	{
		WebNetworkManager->ErrorFromLogin(ErrStr);
		return;
	}
	if (ResPkt.success() == false)
	{
		WebNetworkManager->ErrorFromLogin(TEXT("존재하지 않는 회원"));
		return;
	}

	WebNetworkManager->ResponseToLogin(ResPkt);

	USBNetworkManager* NetworkManager = ServerPacketHandler::GetNetworkManager();
	if (NetworkManager == nullptr)
		return;

	NetworkManager->HandleReadyToConnect(ResPkt);
}

void ServerHttpPacketHandler::RecvReqLoginGoogleAccount(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
	RecvReqLoginAccount(Request, Response, WasSuccessful);
}

void ServerHttpPacketHandler::RecvReqConnectGameServer(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
	USBWebNetworkManager* WebNetworkManager = GetWebNetworkManager();
	if (WebNetworkManager == nullptr)
		return;

	Protocol::RES_CONNECT_GAME_SERVER ResPkt;
	FString ErrStr;
	if (PreprocessHttpRecv(Request, Response, WasSuccessful, OUT ResPkt, OUT ErrStr) == false)
	{
		WebNetworkManager->ErrorFromConnectGameServer(ErrStr);
		return;
	}
	if (ResPkt.success() == false)
	{
		WebNetworkManager->ErrorFromConnectGameServer(TEXT("계정 재로그인 필요"));
		WebNetworkManager->ResponseToLogOut();
		return;
	}
	WebNetworkManager->ResponseToConnectGameServer(ResPkt);

	USBNetworkManager* NetworkManager = ServerPacketHandler::GetNetworkManager();
	if (NetworkManager == nullptr)
		return;
	NetworkManager->ConnectToGameServer(ResPkt.server_info());
}

void ServerHttpPacketHandler::RecvReqRecheckServer(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
	USBWebNetworkManager* WebNetworkManager = GetWebNetworkManager();
	if (WebNetworkManager == nullptr)
		return;

	Protocol::RES_RECHECK_SERVER ResPkt;
	FString ErrStr;
	if (PreprocessHttpRecv(Request, Response, WasSuccessful, OUT ResPkt, OUT ErrStr) == false)
	{
		WebNetworkManager->ErrorFromRecheck(ErrStr);
		return;
	}
	if (ResPkt.success() == false)
	{
		WebNetworkManager->ErrorFromRecheck(TEXT("계정 재로그인 필요"));
		WebNetworkManager->ResponseToLogOut();
		return;
	}
	WebNetworkManager->ResponseToRecheck(ResPkt);
}
