#include "SBMyPlayer.h"
#include "C_SB.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Component/SBMovementComponent.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"

ASBMyPlayer::ASBMyPlayer()
{
	_CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	_CameraBoom->SetupAttachment(RootComponent);
	_CameraBoom->SetUsingAbsoluteRotation(true);
	_CameraBoom->TargetArmLength = 800.f;
	_CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	_CameraBoom->bDoCollisionTest = false;

	_TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	_TopDownCameraComponent->SetupAttachment(_CameraBoom, USpringArmComponent::SocketName);
	_TopDownCameraComponent->bUsePawnControlRotation = false;
}

void ASBMyPlayer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ASBMyPlayer::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
{
	Super::AddMovementInput(WorldDirection, ScaleValue, bForce);

	GetSBMovement()->NotifyMoveInput(WorldDirection);
}

void ASBMyPlayer::SimpleMoveToLocation(const FVector& GoalLocation)
{
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), GoalLocation);

	GetSBMovement()->NotifyPathFollowing(GoalLocation);
}