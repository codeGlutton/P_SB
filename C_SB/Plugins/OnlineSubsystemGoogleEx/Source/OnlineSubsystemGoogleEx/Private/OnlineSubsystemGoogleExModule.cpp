// Copyright Epic Games, Inc. All Rights Reserved.

#include "OnlineSubsystemGoogleExModule.h"
#include "OnlineSubsystemGoogleExPrivate.h"

IMPLEMENT_MODULE(FOnlineSubsystemGoogleExModule, OnlineSubsystemGoogleEx);

// 구글 서브 시스템 EX 생성 팩토리
class FOnlineFactoryGoogleEx : public IOnlineFactory
{
public:

	FOnlineFactoryGoogleEx() {}
	virtual ~FOnlineFactoryGoogleEx() {}

	virtual IOnlineSubsystemPtr CreateSubsystem(FName InstanceName)
	{
		FOnlineSubsystemGoogleExPtr OnlineSub = MakeShared<FOnlineSubsystemGoogleEx, ESPMode::ThreadSafe>(InstanceName);
		if (OnlineSub->IsEnabled())
		{
			if(!OnlineSub->Init())
			{
				UE_LOG_ONLINE(Warning, TEXT("Google Ex API failed to initialize!"));
				OnlineSub->Shutdown();
				OnlineSub = nullptr;
			}
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("Google Ex API disabled!"));
			OnlineSub->Shutdown();
			OnlineSub = nullptr;
		}

		return OnlineSub;
	}
};

void FOnlineSubsystemGoogleExModule::StartupModule()
{
	UE_LOG_ONLINE(Log, TEXT("Google Ex Startup!"));

	GoogleExFactory = new FOnlineFactoryGoogleEx();

	// Create and register our singleton factory with the main online subsystem for easy access
	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.RegisterPlatformService(GOOGLE_EX_SUBSYSTEM, GoogleExFactory);
}

void FOnlineSubsystemGoogleExModule::ShutdownModule()
{
	UE_LOG_ONLINE(Log, TEXT("Google Ex Shutdown!"));

	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.UnregisterPlatformService(GOOGLE_EX_SUBSYSTEM);

	delete GoogleExFactory;
	GoogleExFactory = nullptr;
}