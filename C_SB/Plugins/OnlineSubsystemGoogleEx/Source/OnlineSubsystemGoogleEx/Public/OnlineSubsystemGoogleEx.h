#pragma once

#include "OnlineSubsystem.h"
#include "OnlineSubsystemImpl.h"
#include "OnlineJsonSerializer.h"

typedef TSharedPtr<class FOnlineIdentityGoogleEx, ESPMode::ThreadSafe> FOnlineIdentityGoogleExPtr;
typedef TSharedPtr<class FOnlineFriendsGoogleEx, ESPMode::ThreadSafe> FOnlineFriendsGoogleExPtr;
typedef TSharedPtr<class FOnlineSharingGoogleEx, ESPMode::ThreadSafe> FOnlineSharingGoogleExPtr;
typedef TSharedPtr<class FOnlineUserGoogleEx, ESPMode::ThreadSafe> FOnlineUserGoogleExPtr;
typedef TSharedPtr<class FOnlineExternalUIGoogleEx, ESPMode::ThreadSafe> FOnlineExternalUIGoogleExPtr;

class ONLINESUBSYSTEMGOOGLEEX_API FOnlineSubsystemGoogleEx
	: public FOnlineSubsystemImpl
{
public:
	FOnlineSubsystemGoogleEx() = delete;
	explicit FOnlineSubsystemGoogleEx(FName InInstanceName);
	virtual ~FOnlineSubsystemGoogleEx();

public:
	/* 온라인 서브시스템 interface */

	virtual bool						Init() override;
	virtual bool						Shutdown() override;
	virtual bool						Exec(class UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

	virtual IOnlineSessionPtr			GetSessionInterface() const override;
	virtual IOnlineFriendsPtr			GetFriendsInterface() const override;
	virtual IOnlinePartyPtr				GetPartyInterface() const override;
	virtual IOnlineGroupsPtr			GetGroupsInterface() const override;
	virtual IOnlineSharedCloudPtr		GetSharedCloudInterface() const override;
	virtual IOnlineUserCloudPtr			GetUserCloudInterface() const override;
	virtual IOnlineLeaderboardsPtr		GetLeaderboardsInterface() const override;
	virtual IOnlineVoicePtr				GetVoiceInterface() const override;
	virtual IOnlineExternalUIPtr		GetExternalUIInterface() const override;
	virtual IOnlineTimePtr				GetTimeInterface() const override;
	virtual IOnlineIdentityPtr			GetIdentityInterface() const override;
	virtual IOnlineTitleFilePtr			GetTitleFileInterface() const override;
	virtual IOnlineEntitlementsPtr		GetEntitlementsInterface() const override;
	virtual IOnlineStoreV2Ptr			GetStoreV2Interface() const override { return nullptr; }
	virtual IOnlinePurchasePtr			GetPurchaseInterface() const override { return nullptr; }
	virtual IOnlineEventsPtr			GetEventsInterface() const override;
	virtual IOnlineAchievementsPtr		GetAchievementsInterface() const override;
	virtual IOnlineSharingPtr			GetSharingInterface() const override;
	virtual IOnlineUserPtr				GetUserInterface() const override;
	virtual IOnlineMessagePtr			GetMessageInterface() const override;
	virtual IOnlinePresencePtr			GetPresenceInterface() const override;
	virtual IOnlineChatPtr				GetChatInterface() const override;
	virtual IOnlineStatsPtr				GetStatsInterface() const override;
	virtual IOnlineTurnBasedPtr			GetTurnBasedInterface() const override;
	virtual IOnlineTournamentPtr		GetTournamentInterface() const override;
	virtual FText						GetOnlineServiceName() const override;
	virtual FString						GetAppId() const override;

	virtual bool						Tick(float DeltaTime) override;

	FString								GetClientId() const { return ClientId; }

protected:

	// 사용할 구글 클라이언트 RegisterId
	FString								ClientId;

	// 구글 identitiy 인터페이스 구현 객체
	FOnlineIdentityGoogleExPtr			GoogleExIdentity;

	// 구글 외부 UI 구현 객체
	FOnlineExternalUIGoogleExPtr		GoogleExExternalUI;
};

typedef TSharedPtr<FOnlineSubsystemGoogleEx, ESPMode::ThreadSafe> FOnlineSubsystemGoogleExPtr;