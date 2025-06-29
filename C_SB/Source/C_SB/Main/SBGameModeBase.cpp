#include "SBGameModeBase.h"
#include "C_SB.h"
#include "SBNetworkManager.h"

ASBGameModeBase::ASBGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASBGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	auto* NetworkManager = GetGameInstance()->GetSubsystem<USBNetworkManager>();
	if (NetworkManager == nullptr)
		return;

	NetworkManager->HandleRecvPackets();
}
