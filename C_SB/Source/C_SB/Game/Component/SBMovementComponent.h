#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "PriorityQueue.h"

#include "protocol.pb.h"

#include "Components/ActorComponent.h"
#include "SBMovementComponent.generated.h"

USING_SHARED_PTR_IN_NAMESPACE(Protocol, MovementRecord)
DECLARE_DELEGATE_RetVal_OneParam(float, FOnGetVelocity, const Protocol::MoveState)

UCLASS( ClassGroup=Movement, meta=(BlueprintSpawnableComponent) )
class C_SB_API USBMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USBMovementComponent();
	virtual ~USBMovementComponent();

	virtual void											TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void											BeginPlay() override;

public:	
	const Protocol::MoveState*								GetState();
	void													SetState(const Protocol::MoveState& MoveState);

	/* 레코드 함수 */

	void													AddRecord(const Protocol::PosInfo& PosInfo, const Protocol::DestInfo& DestInfo, const Protocol::MoveState& State);
	void													AddReservedRecord(const uint64 ReservedTime, const Protocol::PosInfo& PosInfo, const Protocol::DestInfo& DestInfo, const Protocol::MoveState& State);

	// 초기 SBObject 생성시, 움직임 처리
	void													AddInitMoveRecord(const Protocol::PosInfo& PosInfo);

	// 특정 시간대 이후의 예약된 레코드 제거
	// TPriorityQueue형식의 Record 저장소 형식 따라 큰 비용이 소모
	void													RemoveRecordsAfterTime(uint32 TimeStamp);

	/* 조작 캐릭터 움직임 등록 함수 */

	void													NotifyStop();
	void													NotifyMoveInput(FVector MoveInputDir);
	void													NotifyPathFollowing(FVector DestLocation);

protected:
	/* MovePacket 제작 함수 */

	virtual void											ReplicateToServer(const uint64 CurrentTime, float DeltaTime);

	/* MovePacket 시뮬레이션 함수 */

	void													ApplyRecords(const uint64 CurrentTime);
	void													CalcRevisedMove(const uint64 CurrentTime, const FVector CurrentLocation, const FVector ServerLocation);

public:
	FOnGetVelocity											OnGetVelocity;

protected:
	/* 움직임 복제 옵션값들 */

	UPROPERTY(Category = "SBMovement: Replicated", EditDefaultsOnly, BlueprintReadOnly, AdvancedDisplay, meta = (ClampMin = "0.1", ClampMax = "100.0", UIMin = "0.1", UIMax = "100.0"))
	float													_ReplicatedLocationAccuracy;

	UPROPERTY(Category = "SBMovement: Replicated", EditDefaultsOnly, BlueprintReadOnly, AdvancedDisplay, meta = (ClampMin = "0.1", ClampMax = "5.0", UIMin = "0.1", UIMax = "5.0"))
	float													_ReplicatedYawAccuracy;

	UPROPERTY(Category = "SBMovement: Replicated", EditDefaultsOnly, BlueprintReadOnly, AdvancedDisplay, meta = (ClampMin = "0.01", ClampMax = "0.2", UIMin = "0.01", UIMax = "0.2"))
	float													_PacketUrgencySensitivity;

	UPROPERTY(Category = "SBMovement: Replicated", EditDefaultsOnly, BlueprintReadOnly, AdvancedDisplay, meta = (ClampMin = "0.1", ClampMax = "1.0", UIMin = "0.1", UIMax = "1.0"))
	float													_MovePacketSendDelay;

protected:
	/* 패킷 전송 계산 값 */

	bool													_bIsUrgentPacket;
	float													_MovePacketSendTimer;

	/* 움직임 기록들 */

	TPriorityQueue<Protocol::MovementRecordRef, uint64>		_Records;
	uint64													_LatestTimeStamp;

	/* 특정 방향의 Input 움직임 계산 값 */

	bool													_bMoveToInputDir;
	FVector													_MoveDir;

protected:
	Protocol::MoveState*									_State;
};
