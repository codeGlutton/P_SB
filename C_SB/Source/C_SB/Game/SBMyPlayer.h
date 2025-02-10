#pragma once

#include "SBPlayer.h"
#include "SBMyPlayer.generated.h"

UCLASS()
class ASBMyPlayer : public ASBPlayer
{
	GENERATED_BODY()

public:
	ASBMyPlayer();

	virtual void								Tick(float DeltaSeconds) override;
	virtual void								AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;

public:
	FORCEINLINE class UCameraComponent*			GetTopDownCameraComponent() const { return _TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent*		GetCameraBoom() const { return _CameraBoom; }

	void										SimpleMoveToLocation(const FVector& GoalLocation);

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent*						_TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent*					_CameraBoom;
};

