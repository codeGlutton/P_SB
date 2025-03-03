#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FOnlineSubsystemGoogleExModule : public IModuleInterface
{
public:
	FOnlineSubsystemGoogleExModule() :
		GoogleExFactory(nullptr)
	{
	}

	virtual ~FOnlineSubsystemGoogleExModule()
	{
	}

public:
	virtual void					StartupModule() override;
	virtual void					ShutdownModule() override;
	virtual bool					SupportsDynamicReloading() override
	{
		return false;
	}
	virtual bool					SupportsAutomaticShutdown() override
	{
		return false;
	}

public:
	// 구글 서브 시스템을 생성하는 단일 팩토리
	class FOnlineFactoryGoogleEx*	GoogleExFactory;
};