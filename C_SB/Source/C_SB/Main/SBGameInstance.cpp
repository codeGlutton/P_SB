#include "SBGameInstance.h"
#include "SBPlayer.h"
#include "ServerPacketHandler.h"

USBGameInstance::USBGameInstance() : UGameInstance()
{
}

void USBGameInstance::Init()
{
	Super::Init();
	ServerPacketHandler::Init();
}

