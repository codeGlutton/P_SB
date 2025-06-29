#pragma once

#include "SBObject.h"
#include "GameFramework/Character.h"
#include "SBPlayer.generated.h"

UCLASS()
class ASBPlayer : public ACharacter, public ISBObject
{
	GENERATED_BODY()

public:
	ASBPlayer();
	virtual	~ASBPlayer() override;

	virtual void										BeginPlay() override;
	virtual void										Tick(float DeltaSeconds) override;
	virtual void										PostInitializeComponents() override;

public:
	virtual void										GetInfo(OUT Protocol::ObjectInfo& OutInfo) override;
	virtual void										SetInfo(const Protocol::ObjectInfo& InInfo) override;
	virtual void										ApplyPos(const Protocol::PosInfo& InPos) override;
	virtual void										ApplyTableRow(const FSBSpawnableTableRow& TableRow) override;

	virtual const Protocol::ObjectBaseInfo*				GetObjectBaseInfo() override;
	virtual const Protocol::PosInfo*					GetPosInfo() override;
	const Protocol::PlayerDetailInfo*					GetPlayerDetailInfo();

	virtual float										GetVelocityByState(const Protocol::MoveState State);

	const TObjectPtr<class USBMovementComponent>		GetSBMovementComp();
	//const TObjectPtr<class USBCostumeMergingComponent>	GetSBCostumeMergingComp();

protected:
	Protocol::ObjectBaseInfo*							_ObjectBaseInfo;
	Protocol::PosInfo*									_PosInfo;
	Protocol::PlayerDetailInfo*							_PlayerDetailInfo;

private:
	UPROPERTY(Category = "Movement", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USBMovementComponent>				_SBMovementComp;
	/*UPROPERTY(Category = "Costume", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USBCostumeMergingComponent>		_SBCostumeMergingComp;*/
};