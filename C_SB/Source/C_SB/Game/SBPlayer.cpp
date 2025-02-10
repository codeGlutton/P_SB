#include "SBPlayer.h"
#include "C_SB.h"
#include "PosUtils.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/SBMovementComponent.h"

#include "NavAreas/NavArea_Obstacle.h"

ASBPlayer::ASBPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	AutoPossessAI = EAutoPossessAI::Spawned;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200.f;
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->MaxAcceleration = 1000.f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.f;
	GetCharacterMovement()->SetFixedBrakingDistance(200.f);

	GetCapsuleComponent()->SetAreaClassOverride(UNavArea_Obstacle::StaticClass());

	_ObjectBaseInfo = xnew<Protocol::ObjectBaseInfo>();
	_PosInfo = xnew<Protocol::PosInfo>();
	_PlayerDetailInfo = xnew<Protocol::PlayerDetailInfo>();

	_SBMovement = CreateDefaultSubobject<USBMovementComponent>(FName("CharSBMoveComp"));
}

ASBPlayer::~ASBPlayer()
{
	xdelete(_ObjectBaseInfo);
	_ObjectBaseInfo = nullptr;
	xdelete(_PosInfo);
	_PosInfo = nullptr;
	xdelete(_PlayerDetailInfo);
	_PlayerDetailInfo = nullptr;
}

void ASBPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void ASBPlayer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PosUtils::ApplyVectorToPos(*_PosInfo, GetActorLocation());
	PosUtils::ApplyRotatorToPos(*_PosInfo, GetActorRotation());
}

void ASBPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	_SBMovement->PredictedVelocityDelegate.BindUObject(this, &ASBPlayer::GetVelocityByState);
}

void ASBPlayer::GetInfo(OUT Protocol::ObjectInfo& OutInfo)
{
	OutInfo.mutable_object_base_info()->CopyFrom(*_ObjectBaseInfo);
	OutInfo.mutable_object_base_info()->CopyFrom(*_PosInfo);
	OutInfo.mutable_object_base_info()->CopyFrom(*_PlayerDetailInfo);
	OutInfo.set_move_state(*_SBMovement->GetState());
}

void ASBPlayer::SetInfo(const Protocol::ObjectInfo& InInfo)
{
	if (_ObjectBaseInfo->object_id() != 0ull)
	{
		check(_ObjectBaseInfo->object_id() == InInfo.object_base_info().object_id());
	}

	_ObjectBaseInfo->CopyFrom(InInfo.object_base_info());
	_PosInfo->CopyFrom(InInfo.pos_info());
	_PlayerDetailInfo->CopyFrom(InInfo.player_detail_info());
}

void ASBPlayer::ApplyPos(const Protocol::PosInfo& InPos)
{
	_PosInfo->CopyFrom(InPos);

	SetActorLocation(PosUtils::Pos2Vector(InPos));
	SetActorRotation(PosUtils::Pos2Rotator(InPos));
}

const Protocol::ObjectBaseInfo* ASBPlayer::GetObjectBaseInfo()
{
	return _ObjectBaseInfo;
}

const Protocol::PosInfo* ASBPlayer::GetPosInfo()
{
	return _PosInfo;
}

const Protocol::PlayerDetailInfo* ASBPlayer::GetPlayerDetailInfo()
{
	return _PlayerDetailInfo;
}

float ASBPlayer::GetVelocityByState(const Protocol::MoveState State)
{
	float Velocity = 0.f;

	switch (State)
	{
	case Protocol::MOVE_STATE_WALK:
		Velocity = GetCharacterMovement()->MaxWalkSpeed;
		break;
	}

	return Velocity;
}

const TObjectPtr<class USBMovementComponent> ASBPlayer::GetSBMovement()
{
	return _SBMovement;
}
