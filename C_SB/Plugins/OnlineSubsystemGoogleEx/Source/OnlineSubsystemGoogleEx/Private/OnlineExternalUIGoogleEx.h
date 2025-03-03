// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "OnlineSubsystemGoogleEx.h"
#include "Interfaces/OnlineExternalUIInterface.h"

class FOnlineSubsystemGoogleEx;
class IHttpRequest;

// 구글 외부 UI 구현 클래스
class FOnlineExternalUIGoogleEx : public IOnlineExternalUI
{
public:
	explicit FOnlineExternalUIGoogleEx(FOnlineSubsystemGoogleEx* InSubsystem) :
		GoogleExSubsystem(InSubsystem)
	{
	}

	virtual ~FOnlineExternalUIGoogleEx()
	{
	}

public:
	/* 온라인 외부 UI interface */

	virtual bool				ShowLoginUI(const int ControllerIndex, bool bShowOnlineOnly, bool bShowSkipButton, const FOnLoginUIClosedDelegate& Delegate = FOnLoginUIClosedDelegate()) override;
	virtual bool				ShowAccountCreationUI(const int ControllerIndex, const FOnAccountCreationUIClosedDelegate& Delegate = FOnAccountCreationUIClosedDelegate()) override { return false; }
	virtual bool				ShowFriendsUI(int32 LocalUserNum) override;
	virtual bool				ShowInviteUI(int32 LocalUserNum, FName SessionName = NAME_GameSession) override;
	virtual bool				ShowAchievementsUI(int32 LocalUserNum) override;
	virtual bool				ShowLeaderboardUI(const FString& LeaderboardName) override;
	virtual bool				ShowWebURL(const FString& Url, const FShowWebUrlParams& ShowParams, const FOnShowWebUrlClosedDelegate& Delegate = FOnShowWebUrlClosedDelegate()) override;
	virtual bool				CloseWebURL() override;
	virtual bool				ShowProfileUI(const FUniqueNetId& Requestor, const FUniqueNetId& Requestee, const FOnProfileUIClosedDelegate& Delegate = FOnProfileUIClosedDelegate()) override;
	virtual bool				ShowAccountUpgradeUI(const FUniqueNetId& UniqueId) override;
	virtual bool				ShowStoreUI(int32 LocalUserNum, const FShowStoreParams& ShowParams, const FOnShowStoreUIClosedDelegate& Delegate = FOnShowStoreUIClosedDelegate()) override;
	virtual bool				ShowSendMessageUI(int32 LocalUserNum, const FShowSendMessageParams& ShowParams, const FOnShowSendMessageUIClosedDelegate& Delegate = FOnShowSendMessageUIClosedDelegate()) override;

private:
	// LoginFlow에서 받은 URL 리다이렉션 정보를 넘길 떼, 정보를 보고 리다이렉션을 허가할지 결정하는 함수
	FLoginFlowResult			OnLoginRedirectURL(const FString& RedirectURL);

	// LoginFlow가 종료될 때 호출되는 함수
	void						OnExternalLoginFlowComplete(const FLoginFlowResult& Result, int ControllerIndex, const FOnLoginUIClosedDelegate Delegate);

	// identitiy에서 로그인 처리가 완료되고 나서 ExternalUI를 정리하기 위해 호출되는 함수 (이후 identity의 OnExternalUILoginComplete() 호출)
	void						OnAccessTokenLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error, FOnLoginUIClosedDelegate Delegate);

public:
	FOnlineSubsystemGoogleEx*	GoogleExSubsystem;
};

typedef TSharedPtr<FOnlineExternalUIGoogleEx, ESPMode::ThreadSafe> FOnlineExternalUIGoogleExPtr;

