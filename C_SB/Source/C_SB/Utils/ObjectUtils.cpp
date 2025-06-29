#include "ObjectUtils.h"
#include "C_SB.h"
#include "Component/SBMovementComponent.h"

bool PosUtils::IsRoughlySamePos(const Protocol::PosInfo& Source, const Protocol::PosInfo& Target, float LocationAccuracy, float YawAccuracy)
{
	if (FMath::IsNearlyEqual(Source.x(), Target.x(), LocationAccuracy) == false)
		return false;
	if (FMath::IsNearlyEqual(Source.y(), Target.y(), LocationAccuracy) == false)
		return false;
	if (FMath::IsNearlyEqual(Source.z(), Target.z(), LocationAccuracy) == false)
		return false;
	if (FMath::IsNearlyEqual(Source.yaw(), Target.yaw(), YawAccuracy) == false)
		return false;

	// pitch와 roll은 생략

	return true;
}

FVector PosUtils::ExtractVectorFromPos(const Protocol::PosInfo& PosInfo)
{
	return FVector(PosInfo.x(), PosInfo.y(), PosInfo.z());
}

FRotator PosUtils::ExtractRotatorFromPos(const Protocol::PosInfo& PosInfo)
{
	return FRotator(PosInfo.pitch(), PosInfo.yaw(), PosInfo.roll());
}

void PosUtils::ApplyVectorToPos(OUT Protocol::PosInfo& PosInfo, const FVector& Location)
{
	PosInfo.set_x(Location.X);
	PosInfo.set_y(Location.Y);
	PosInfo.set_z(Location.Z);
}

void PosUtils::ApplyRotatorToPos(OUT Protocol::PosInfo& PosInfo, const FRotator& Rotation)
{
	PosInfo.set_pitch(Rotation.Pitch);
	PosInfo.set_yaw(Rotation.Yaw);
	PosInfo.set_roll(Rotation.Roll);
}

ISBObject* ObjectUtils::SpawnSBObject(UWorld* World, const Protocol::ObjectInfo& Info, const FSBSpawnableTableRow& TableRow)
{
	UClass* ObjectClass = TableRow.ClientBasePath.Get();
	if (ObjectClass == nullptr || ObjectClass->ImplementsInterface(USBObject::StaticClass()) == false)
		return nullptr;

	FTransform SpawnTransform(FRotator(0., Info.pos_info().yaw(), 0.), FVector(Info.pos_info().x(), Info.pos_info().y(), 0.));

	// 초기 정보 적용

	ISBObject* Object = World->SpawnActorDeferred<ISBObject>(ObjectClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (Object == nullptr)
		return nullptr;
	Object->SetInfo(Info);
	Object->ApplyTableRow(TableRow);

	// 초기 움직임 적용

	AActor* Actor = Cast<AActor>(Object);
	Actor->FinishSpawning(SpawnTransform, true);

	return Object;
}
