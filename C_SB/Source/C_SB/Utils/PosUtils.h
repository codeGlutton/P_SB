#pragma once

#include "CoreMinimal.h"
#include "Struct.pb.h"

class PosUtils
{
public:
	static bool		IsRoughlySamePos(const Protocol::PosInfo& Source, const Protocol::PosInfo& Target, float LocationAccuracy = 10.f, float YawAccuracy = 1.f);
	static FVector	Pos2Vector(const Protocol::PosInfo& PosInfo);
	static FRotator	Pos2Rotator(const Protocol::PosInfo& PosInfo);
	static void		ApplyVectorToPos(OUT Protocol::PosInfo& PosInfo, const FVector& Location);
	static void		ApplyRotatorToPos(OUT Protocol::PosInfo& PosInfo, const FRotator& Rotation);
};