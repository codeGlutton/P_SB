#include "PosUtils.h"
#include "C_SB.h"

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

FVector PosUtils::Pos2Vector(const Protocol::PosInfo& PosInfo)
{
	return FVector(PosInfo.x(), PosInfo.y(), PosInfo.z());
}

FRotator PosUtils::Pos2Rotator(const Protocol::PosInfo& PosInfo)
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
