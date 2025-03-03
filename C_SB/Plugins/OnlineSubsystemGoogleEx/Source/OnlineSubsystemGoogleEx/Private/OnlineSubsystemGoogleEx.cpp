#include "OnlineSubsystemGoogleEx.h"
#include "OnlineSubsystemGoogleExPrivate.h"
#include "OnlineIdentityGoogleEx.h"
#include "OnlineExternalUIGoogleEx.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CommandLine.h"
#include "Stats/Stats.h"

#define GOOGLE_EX_CLIENTAUTH_ID TEXT("ClientId")

FOnlineSubsystemGoogleEx::FOnlineSubsystemGoogleEx(FName InInstanceName)
	: FOnlineSubsystemImpl(GOOGLE_EX_SUBSYSTEM, InInstanceName)
{

}

FOnlineSubsystemGoogleEx::~FOnlineSubsystemGoogleEx()
{
}

bool FOnlineSubsystemGoogleEx::Init()
{
	static FString ConfigSection(TEXT("OnlineSubsystemGoogleEx"));
	if (!GConfig->GetString(*ConfigSection, GOOGLE_EX_CLIENTAUTH_ID, ClientId, GEngineIni))
	{
		UE_LOG_ONLINE(Warning, TEXT("Missing ClientId= in [%s] of DefaultEngine.ini"), *ConfigSection);
	}

	GoogleExIdentity = MakeShareable(new FOnlineIdentityGoogleEx(this));
	GoogleExExternalUI = MakeShareable(new FOnlineExternalUIGoogleEx(this));

	return true;
}

bool FOnlineSubsystemGoogleEx::Shutdown()
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemGoogleEx::Shutdown()"));

	FOnlineSubsystemImpl::Shutdown();

	/* 소유한 ExternalUI와 Identity Ref가 마지막인지 확인*/

#define DESTRUCT_INTERFACE(Interface) \
	if (Interface.IsValid()) \
	{ \
		ensure(Interface.IsUnique()); \
		Interface = nullptr; \
	}

	DESTRUCT_INTERFACE(GoogleExExternalUI);
	DESTRUCT_INTERFACE(GoogleExIdentity);

#undef DESTRUCT_INTERFACE

	return true;
}

bool FOnlineSubsystemGoogleEx::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	if (FOnlineSubsystemImpl::Exec(InWorld, Cmd, Ar))
	{
		return true;
	}
	return false;
}

IOnlineSessionPtr FOnlineSubsystemGoogleEx::GetSessionInterface() const
{
	return nullptr;
}

IOnlineFriendsPtr FOnlineSubsystemGoogleEx::GetFriendsInterface() const
{
	return nullptr;
}

IOnlinePartyPtr FOnlineSubsystemGoogleEx::GetPartyInterface() const
{
	return nullptr;
}

IOnlineGroupsPtr FOnlineSubsystemGoogleEx::GetGroupsInterface() const
{
	return nullptr;
}

IOnlineSharedCloudPtr FOnlineSubsystemGoogleEx::GetSharedCloudInterface() const
{
	return nullptr;
}

IOnlineUserCloudPtr FOnlineSubsystemGoogleEx::GetUserCloudInterface() const
{
	return nullptr;
}

IOnlineLeaderboardsPtr FOnlineSubsystemGoogleEx::GetLeaderboardsInterface() const
{
	return nullptr;
}

IOnlineVoicePtr FOnlineSubsystemGoogleEx::GetVoiceInterface() const
{
	return nullptr;
}

IOnlineExternalUIPtr FOnlineSubsystemGoogleEx::GetExternalUIInterface() const
{
	return GoogleExExternalUI;
}

IOnlineTimePtr FOnlineSubsystemGoogleEx::GetTimeInterface() const
{
	return nullptr;
}

IOnlineIdentityPtr FOnlineSubsystemGoogleEx::GetIdentityInterface() const
{
	return GoogleExIdentity;
}

IOnlineTitleFilePtr FOnlineSubsystemGoogleEx::GetTitleFileInterface() const
{
	return nullptr;
}

IOnlineEntitlementsPtr FOnlineSubsystemGoogleEx::GetEntitlementsInterface() const
{
	return nullptr;
}

IOnlineEventsPtr FOnlineSubsystemGoogleEx::GetEventsInterface() const
{
	return nullptr;
}

IOnlineAchievementsPtr FOnlineSubsystemGoogleEx::GetAchievementsInterface() const
{
	return nullptr;
}

IOnlineSharingPtr FOnlineSubsystemGoogleEx::GetSharingInterface() const
{
	return nullptr;
}

IOnlineUserPtr FOnlineSubsystemGoogleEx::GetUserInterface() const
{
	return nullptr;
}

IOnlineMessagePtr FOnlineSubsystemGoogleEx::GetMessageInterface() const
{
	return nullptr;
}

IOnlinePresencePtr FOnlineSubsystemGoogleEx::GetPresenceInterface() const
{
	return nullptr;
}

IOnlineChatPtr FOnlineSubsystemGoogleEx::GetChatInterface() const
{
	return nullptr;
}

IOnlineStatsPtr FOnlineSubsystemGoogleEx::GetStatsInterface() const
{
	return nullptr;
}

IOnlineTurnBasedPtr FOnlineSubsystemGoogleEx::GetTurnBasedInterface() const
{
	return nullptr;
}

IOnlineTournamentPtr FOnlineSubsystemGoogleEx::GetTournamentInterface() const
{
	return nullptr;
}

FText FOnlineSubsystemGoogleEx::GetOnlineServiceName() const
{
	return NSLOCTEXT("OnlineSubsystemGoogleEx", "OnlineServiceName", "GoogleEx");
}

FString FOnlineSubsystemGoogleEx::GetAppId() const
{
	return ClientId;
}

bool FOnlineSubsystemGoogleEx::Tick(float DeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FOnlineSubsystemGoogleEx_Tick);

	if (!FOnlineSubsystemImpl::Tick(DeltaTime))
	{
		return false;
	}

	return true;
}