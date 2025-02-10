#include "SBPlayerController.h"
#include "C_SB.h"

#include "SBMyPlayer.h"
#include "Component/SBMovementComponent.h"

#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"

#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

#include "Engine/World.h"

ASBPlayerController::ASBPlayerController() :
	ShortPressThreshold(0.3f),
	_CachedDestination(FVector::ZeroVector),
	_bIsTouch(false),
	_FollowTime(0.f)
{
	DefaultMouseCursor = EMouseCursor::Default;
	bShowMouseCursor = true;
}

void ASBPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 마우스
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &ASBPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &ASBPlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &ASBPlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &ASBPlayerController::OnSetDestinationReleased);

		// 스크린 터치
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &ASBPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &ASBPlayerController::OnTouchTriggered);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &ASBPlayerController::OnTouchReleased);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &ASBPlayerController::OnTouchReleased);
	}
	else
	{
		UE_LOG(LogSB, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void ASBPlayerController::OnInputStarted()
{
	StopMovement();
}

void ASBPlayerController::OnSetDestinationTriggered()
{
	_FollowTime += GetWorld()->GetDeltaSeconds();

	// 선택 지점 충돌 검사
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (_bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	if (bHitSuccessful)
	{
		_CachedDestination = Hit.Location;
	}

	// 방향 움직임 추가
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (_CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void ASBPlayerController::OnSetDestinationReleased()
{
	ASBMyPlayer* ControlledPlayer = GetPawn<ASBMyPlayer>();
	if (ControlledPlayer == nullptr)
	{
		return;
	}

	// 일정 Threadhold 이하로 드래그 했을 경우에만 길 탐색
	if (_FollowTime <= ShortPressThreshold)
	{
		ControlledPlayer->SimpleMoveToLocation(_CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, _CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}
	else
	{
		ControlledPlayer->GetSBMovement()->NotifyStop();
	}

	_FollowTime = 0.f;
}

void ASBPlayerController::OnTouchTriggered()
{
	_bIsTouch = true;
	OnSetDestinationTriggered();
}

void ASBPlayerController::OnTouchReleased()
{
	_bIsTouch = false;
	OnSetDestinationReleased();
}
