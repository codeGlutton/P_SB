#include "Component/SBMovementComponent.h"
#include "C_SB.h"

#include "SBObject.h"
#include "ObjectUtils.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "SBNetworkManager.h"
#include "ServerPacketHandler.h"
#include "PacketSession.h"

USBMovementComponent::USBMovementComponent() : 
	_ReplicatedLocationAccuracy(10.f),
	_ReplicatedYawAccuracy(1.f),
	_PacketUrgencySensitivity(0.1f),
	_MovePacketSendDelay(0.4f),
	_bIsUrgentPacket(false),
	_MovePacketSendTimer(_MovePacketSendDelay),
	_LatestTimeStamp(0ull),
	_bMoveToInputDir(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;

	_State = xnew<Protocol::MoveState>();
}

USBMovementComponent::~USBMovementComponent()
{
	xdelete(_State);
	_State = nullptr;
}

void USBMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ISBObject* OwnSBObject = GetOwner<ISBObject>();
	check(OwnSBObject);
	APawn* OwnPawn = GetOwner<APawn>();
	check(OwnPawn);

	const uint64 CurrentTime = Utils::MakeTimeStamp();

	if (OwnSBObject->IsMyPlayer())
	{
		ReplicateToServer(CurrentTime, DeltaTime);
	}
	else
	{
		ApplyRecords(CurrentTime);

		if (_bMoveToInputDir)
		{
			OwnPawn->AddMovementInput(_MoveDir);
		}
	}
}

void USBMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	ISBObject* OwnSBObject = GetOwner<ISBObject>();
	check(OwnSBObject);

	if (OwnSBObject->IsMyPlayer())
	{
		*_State = Protocol::MOVE_STATE_IDLE;
	}
	else
	{
		AddInitMoveRecord(*OwnSBObject->GetPosInfo());
	}
}

const Protocol::MoveState* USBMovementComponent::GetState()
{
	return _State;
}

void USBMovementComponent::SetState(const Protocol::MoveState& MoveState)
{
	*_State = MoveState;
}

void USBMovementComponent::AddRecord(const Protocol::PosInfo& PosInfo, const Protocol::DestInfo& DestInfo, const Protocol::MoveState& State)
{
	AddReservedRecord(Utils::MakeTimeStamp(), PosInfo, DestInfo, State);
}

void USBMovementComponent::AddReservedRecord(const uint64 ReservedTime, const Protocol::PosInfo& PosInfo, const Protocol::DestInfo& DestInfo, const Protocol::MoveState& State)
{
	Protocol::MovementRecordRef NewRecord = ObjectPool<Protocol::MovementRecord>::MakeXShared();
	NewRecord->set_time_stamp(ReservedTime);
	NewRecord->mutable_pos_info()->CopyFrom(PosInfo);
	NewRecord->mutable_dest_info()->CopyFrom(DestInfo);
	NewRecord->set_move_state(State);

	_Records.Push(NewRecord, NewRecord->time_stamp());
}

void USBMovementComponent::AddInitMoveRecord(const Protocol::PosInfo& PosInfo)
{
	if (*_State == Protocol::MOVE_STATE_IDLE || *_State == Protocol::MOVE_STATE_NONE)
		return;

	if (*_State == Protocol::MOVE_STATE_FOLLOW_PATH)
		*_State = Protocol::MOVE_STATE_WALK;

	AddRecord(PosInfo, Protocol::DestInfo(), *_State);
}

void USBMovementComponent::RemoveRecordsAfterTime(uint32 TimeStamp)
{
	TPriorityQueue<Protocol::MovementRecordRef, uint64> NewRecords;

	while (_Records.IsEmpty() == false)
	{
		if (_Records.Peek()->time_stamp() > TimeStamp)
		{
			break;
		}
		NewRecords.PushNode(_Records.PopNode());
	}

	_Records = NewRecords;
}

void USBMovementComponent::NotifyStop()
{
	ISBObject* OwnSBObject = GetOwner<ISBObject>();
	check(OwnSBObject);

	AddRecord(*OwnSBObject->GetPosInfo(), Protocol::DestInfo(), Protocol::MOVE_STATE_IDLE);
	_bMoveToInputDir = false;

	_bIsUrgentPacket = _MoveDir.Equals(FVector::Zero(), _PacketUrgencySensitivity) == false;
}

void USBMovementComponent::NotifyMoveInput(FVector MoveInputDir)
{
	if (MoveInputDir.IsNearlyZero())
		return;

	_bMoveToInputDir = true;

	// 이전 전송 움직임 방향과 차이가 있어 빠른 전송이 필요한가
	_bIsUrgentPacket = _MoveDir.Equals(MoveInputDir, _PacketUrgencySensitivity) == false;
}

void USBMovementComponent::NotifyPathFollowing(FVector DestLocation)
{
	ISBObject* OwnSBObject = GetOwner<ISBObject>();
	check(OwnSBObject);

	Protocol::PosInfo Pos = *OwnSBObject->GetPosInfo();

	Protocol::DestInfo Dest;
	Dest.set_x(DestLocation.X);
	Dest.set_y(DestLocation.Y);

	AddRecord(Pos, Dest, Protocol::MOVE_STATE_FOLLOW_PATH);
	_bMoveToInputDir = false;

	// 이전 전송 움직임 방향과 차이가 있어 빠른 전송이 필요한가
	FVector PathFollowingDir = FVector(Dest.x(), Dest.y(), 0.) - PosUtils::ExtractVectorFromPos(Pos);
	PathFollowingDir.Normalize();
	_bIsUrgentPacket = _MoveDir.Equals(PathFollowingDir, _PacketUrgencySensitivity) == false;
}

void USBMovementComponent::ReplicateToServer(const uint64 CurrentTime, float DeltaTime)
{
	_MovePacketSendTimer -= DeltaTime;

	float SendTimerThreadHold = _bIsUrgentPacket ? _MovePacketSendDelay / 2.f : 0.f;
	if (_MovePacketSendTimer <= SendTimerThreadHold)
	{
		_MovePacketSendTimer = _MovePacketSendDelay;
		_bIsUrgentPacket = false;

		// Input 방향 움직임 중에는 SendTimerThreadHold에 맞추어 이동 방향 주기적 전송
		if (_bMoveToInputDir)
		{
			_bMoveToInputDir = false;

			ISBObject* OwnSBObject = GetOwner<ISBObject>();
			check(OwnSBObject);

			Protocol::PosInfo NewPosInfo;
			NewPosInfo.CopyFrom(*OwnSBObject->GetPosInfo());
			PosUtils::ApplyRotatorToPos(NewPosInfo, _MoveDir.Rotation());
			
			AddRecord(NewPosInfo, Protocol::DestInfo(), *_State);
		}

		// 마지막 움직임 전송
		Protocol::MovementRecordRef NextRecord = nullptr;
		while (_Records.IsEmpty() == false)
		{
			NextRecord = _Records.Pop();
		}

		if (NextRecord != nullptr)
		{
			// 전송될 움직임 방향 기록
			if (NextRecord->move_state() == Protocol::MOVE_STATE_FOLLOW_PATH)
			{
				FVector NewMoveDir = FVector(NextRecord->dest_info().x(), NextRecord->dest_info().y(), 0.) - PosUtils::ExtractVectorFromPos(NextRecord->pos_info());
				NewMoveDir.Normalize();

				_MoveDir = NewMoveDir;
			}
			else
			{
				_MoveDir = FRotationMatrix(PosUtils::ExtractRotatorFromPos(NextRecord->pos_info())).GetUnitAxis(EAxis::X);
			}

			// 전송 시작
			{
				Protocol::C_MOVE MovePkt;

				MovePkt.mutable_pos_info()->CopyFrom(NextRecord->pos_info());
				MovePkt.mutable_dest_info()->CopyFrom(NextRecord->dest_info());
				MovePkt.set_move_state(NextRecord->move_state());

				USBNetworkManager* NetworkManager = GetOwner()->GetGameInstance()->GetSubsystem<USBNetworkManager>();
				if (NetworkManager == nullptr)
					return;

				PacketSessionRef session = NetworkManager->GameServerSession;
				SEND_C_PACKET(session, MovePkt);
			}
		}
		else
		{
			_MoveDir = FVector::Zero();
		}
	}
}

void USBMovementComponent::ApplyRecords(const uint64 CurrentTime)
{
	ISBObject* OwnSBObject = GetOwner<ISBObject>();
	check(OwnSBObject);
	APawn* OwnPawn = GetOwner<APawn>();
	check(OwnPawn);

	while (_Records.IsEmpty() == false)
	{
		Protocol::MovementRecordRef NextRecord = _Records.Peek();
		if (NextRecord->time_stamp() > CurrentTime)
		{
			break;
		}
		_Records.Pop();

		const Protocol::PosInfo& NextPosInfo = NextRecord->pos_info();
		const Protocol::MoveState& NextState = NextRecord->move_state();

		// 움직임 보정 여부
		bool bNeedMovementRevision = true;

		// 움직임 상태 처리
		// 시뮬레이션 캐릭터의 위치가 조작 클라이언트와 차이가 최대치 허용범위보다 클 경우 (클라이언트 측 RTT 증가) 위치 수정
		if (PosUtils::IsRoughlySamePos(NextPosInfo, *OwnSBObject->GetPosInfo(), _ReplicatedLocationAccuracy, _ReplicatedYawAccuracy) == false || NextState == Protocol::MOVE_STATE_PHYSICS)
		{
			OwnSBObject->ApplyPos(NextPosInfo);
			bNeedMovementRevision = false;
		}
		*_State = NextState;

		_bMoveToInputDir = false;
		_MoveDir = FVector();

		// 길찾기 움직임과 기본 움직임 구분
		if (*_State == Protocol::MOVE_STATE_FOLLOW_PATH)
		{
			const Protocol::DestInfo& NextDestInfo = NextRecord->dest_info();
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(OwnPawn->GetController(), FVector(NextDestInfo.x(), NextDestInfo.y(), 0.));
		}
		else if (*_State != Protocol::MOVE_STATE_IDLE && *_State != Protocol::MOVE_STATE_NONE)
		{
			_MoveDir = FRotator(0., NextPosInfo.yaw(), 0.).RotateVector(FVector(1., 0., 0.));

			// 움직임 보정 처리 시작
			if (bNeedMovementRevision)
			{
				CalcRevisedMove(CurrentTime, PosUtils::ExtractVectorFromPos(*OwnSBObject->GetPosInfo()), PosUtils::ExtractVectorFromPos(NextPosInfo));
			}

			if (_MoveDir.IsNearlyZero() == false)
			{
				_bMoveToInputDir = true;
			}
		}

		_LatestTimeStamp = NextRecord->time_stamp();
	}
}

void USBMovementComponent::CalcRevisedMove(const uint64 CurrentTime, const FVector CurrentLocation, const FVector ServerLocation)
{
	check(OnGetVelocity.IsBound());
	float Velocity = OnGetVelocity.Execute(*_State);

	FVector PredictedDest = ((CurrentTime - _LatestTimeStamp) * 1000 * Velocity) * _MoveDir + ServerLocation;
	FVector RevisedMoveDir = PredictedDest - CurrentLocation;
	RevisedMoveDir.Normalize();

	_MoveDir = RevisedMoveDir;
}



