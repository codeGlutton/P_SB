#pragma once

#include "CoreMinimal.h"
#include "Struct.pb.h"
#include "SBObject.h"
#include "SBTableRow.h"

class PosUtils
{
public:
	static bool			IsRoughlySamePos(const Protocol::PosInfo& Source, const Protocol::PosInfo& Target, float LocationAccuracy = 10.f, float YawAccuracy = 1.f);
	static FVector		ExtractVectorFromPos(const Protocol::PosInfo& PosInfo);
	static FRotator		ExtractRotatorFromPos(const Protocol::PosInfo& PosInfo);
	static void			ApplyVectorToPos(OUT Protocol::PosInfo& PosInfo, const FVector& Location);
	static void			ApplyRotatorToPos(OUT Protocol::PosInfo& PosInfo, const FRotator& Rotation);
};

class ObjectUtils
{
public:
	static ISBObject*	SpawnSBObject(UWorld* World, const Protocol::ObjectInfo& Info, const FSBSpawnableTableRow& TableRow);
};