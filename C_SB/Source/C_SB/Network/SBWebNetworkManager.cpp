#include "SBWebNetworkManager.h"
#include "C_SB.h"

#include "ServerHttpPacketHandler.h"

DEFINE_LOG_CATEGORY(LogWebManager);

/*************************
   USBWebNetworkManager
*************************/

//PasswordPattern(TEXT("^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[~!@#$%^&*_\\-+=`|\\\\(){}[\\]:;\"'<>,.?\\/])[a-zA-Z0-9~!@#$%^&*_\\-+=`|\\\\(){}[\\]:;\"'<>,.?\\/]{8,20}$"))

USBWebNetworkManager::USBWebNetworkManager() :
	bIsLoggedIn(false),
	WaitingResTypeFlags(ESBHttpPktTypeFlag::NONE),
	PasswordPattern(TEXT("^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[~!@#$%^&*_\\-+=`|\\\\:;\"',.?\\/])[a-zA-Z0-9~!@#$%^&*_\\-+=`|\\\\:;\"',.?\\/]{8,20}$"))
{
}

void USBWebNetworkManager::RequestToCheckUsableId(const FText& Id)
{
	if (EnumHasAllFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT) == true)
	{
		UE_LOG(LogWebManager, Warning, TEXT("New requests cannot be send while waiting for a response packet"));
		return;
	}
	EnumAddFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT);

	Protocol::REQ_CHECK_EXISTS_ACCOUNT CheckPkt;
	{
		std::string Ut8Id;
		Utils::UTF16To8(Id.ToString(), OUT Ut8Id);
		CheckPkt.set_account_name(Ut8Id);
	}

	if (ServerHttpPacketHandler::SendPkt(CheckPkt) == false)
	{
		ErrorFromCheckUsableId(TEXT("패킷 오류"));
	}
}

void USBWebNetworkManager::RequestToSignUp(const FText& Id, const FText& Password)
{
	if (EnumHasAllFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT) == true)
	{
		UE_LOG(LogWebManager, Warning, TEXT("New requests cannot be send while waiting for a response packet"));
		return;
	}
	EnumAddFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT);

	Protocol::REQ_CREATE_ACCOUNT CreatePkt;
	{
		std::string Ut8Id;
		Utils::UTF16To8(Id.ToString(), OUT Ut8Id);
		CreatePkt.set_account_name(Ut8Id);
	}
	{
		std::string Ut8Password;
		Utils::UTF16To8(Password.ToString(), OUT Ut8Password);
		CreatePkt.set_password(Ut8Password);
	}

	if (ServerHttpPacketHandler::SendPkt(CreatePkt) == false)
	{
		ErrorFromSignUp(TEXT("패킷 오류"));
	}
}

void USBWebNetworkManager::RequestToLogin(const FText& Id, const FText& Password)
{
	if (bIsLoggedIn == true)
	{
		UE_LOG(LogWebManager, Warning, TEXT("Already logged in"));
		return;
	}
	if (EnumHasAllFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT) == true)
	{
		UE_LOG(LogWebManager, Warning, TEXT("New requests cannot be send while waiting for a response packet"));
		return;
	}
	EnumAddFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT);

	Protocol::REQ_LOGIN_ACCOUNT LoginPkt;
	{
		std::string Ut8Id;
		Utils::UTF16To8(Id.ToString(), OUT Ut8Id);
		LoginPkt.set_account_name(Ut8Id);
	}
	{
		std::string Ut8Password;
		Utils::UTF16To8(Password.ToString(), OUT Ut8Password);
		LoginPkt.set_password(Ut8Password);
	}

	if (ServerHttpPacketHandler::SendPkt(LoginPkt) == false)
	{
		ErrorFromLogin(TEXT("패킷 오류"));
	}
}

void USBWebNetworkManager::ResponseToCheckUsableId(const Protocol::RES_CHECK_EXISTS_ACCOUNT& CreatePkt)
{
	OnIdCheckResRecved.Broadcast(true, "");

	EnumRemoveFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ResponseToSignUp(const Protocol::RES_CREATE_ACCOUNT& CreatePkt)
{
	OnSignUpResRecved.Broadcast(true, "");

	EnumRemoveFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ResponseToLogin(const Protocol::RES_LOGIN_ACCOUNT& LoginPkt)
{
	OnLoginResRecved.Broadcast(true, "");
	bIsLoggedIn = true;

	EnumRemoveFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ErrorFromCheckUsableId(FString ErrStr)
{
	UE_LOG(LogWebManager, Log, TEXT("Account pkt is failed by %s"), *ErrStr);
	
	OnIdCheckResRecved.Broadcast(false, ErrStr);

	EnumRemoveFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ErrorFromSignUp(FString ErrStr)
{
	UE_LOG(LogWebManager, Log, TEXT("Account pkt is failed by %s"), *ErrStr);

	OnSignUpResRecved.Broadcast(false, ErrStr);

	EnumRemoveFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT);
}

void USBWebNetworkManager::ErrorFromLogin(FString ErrStr)
{
	UE_LOG(LogWebManager, Log, TEXT("Account pkt is failed by %s"), *ErrStr);

	OnLoginResRecved.Broadcast(false, ErrStr);

	EnumRemoveFlags(WaitingResTypeFlags, ESBHttpPktTypeFlag::ACCOUNT);
}

bool USBWebNetworkManager::CheckUsablePassword(const FText& Password)
{
	FRegexMatcher Matcher(PasswordPattern.Pattern, Password.ToString());
	return Matcher.FindNext();
}

bool USBWebNetworkManager::IsWaitingSpecificRes(const ESBHttpPktTypeFlag HttpOktTypeFlag)
{
	return EnumHasAllFlags(WaitingResTypeFlags, HttpOktTypeFlag);
}
