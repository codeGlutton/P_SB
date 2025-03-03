#include "Utils.h"
#include "C_SB.h"
#include <chrono>

uint64 Utils::MakeTimeStamp()
{
    auto Now = std::chrono::system_clock::now();
    auto Duration = Now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(Duration).count();
}

UWorld* const Utils::GetWorld()
{
#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST >= 1
	if (GEngine)
	{
		if (FWorldContext* CurWorld = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport))
		{
			return CurWorld->World();
		}
	}
#else
	return GWorld;
#endif
	return nullptr;
}

UGameInstance* const Utils::GetGameInstance()
{
	if (UWorld* CurWorld = GetWorld())
	{
		return CurWorld->GetGameInstance();
	}
	return nullptr;
}
