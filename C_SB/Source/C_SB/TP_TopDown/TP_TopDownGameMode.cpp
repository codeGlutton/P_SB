// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_TopDownGameMode.h"
#include "TP_TopDownPlayerController.h"
#include "TP_TopDownCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATP_TopDownGameMode::ATP_TopDownGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ATP_TopDownPlayerController::StaticClass();
	DefaultPawnClass = ATP_TopDownCharacter::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<ATP_TopDownCharacter> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter.BP_TopDownCharacter_C"));
	if (PlayerPawnBPClass.Succeeded())
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<ATP_TopDownPlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController.BP_TopDownPlayerController_C"));
	if(PlayerControllerBPClass.Succeeded())
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}