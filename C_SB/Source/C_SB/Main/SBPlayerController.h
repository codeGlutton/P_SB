#pragma once

#include "C_SB.h"
#include "GameFramework/PlayerController.h"
#include "SBPlayerController.generated.h"

class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;

UCLASS()
class ASBPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASBPlayerController();

	virtual void					SetupInputComponent() override;
	virtual void					BeginPlay();

protected:
	void							OnInputStarted();

	/* 마우스 조작 */

	// 마우스 누르는 동안 틱마다 실행
	void							OnSetDestinationTriggered();
	void							OnSetDestinationReleased();

	/* 터치 스크린 조작 */

	// 터치 스크린 누르는 동안 틱마다 실행
	void							OnTouchTriggered();
	void							OnTouchReleased();

public:
	// 빠른 클릭과 드래그를 구분하는 threadhold 값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float							ShortPressThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UNiagaraSystem*					FXCursor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext*			DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction*					SetDestinationClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction*					SetDestinationTouchAction;

protected:
	uint32							_bMoveToMouseCursor : 1;

private:
	FVector							_CachedDestination;

	bool							_bIsTouch;
	float							_FollowTime;
};