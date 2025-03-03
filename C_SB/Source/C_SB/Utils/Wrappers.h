#pragma once

#include "CoreMinimal.h"
#include "Wrappers.generated.h"

/************************
   FRegexPatternWrapper
*************************/

USTRUCT(BlueprintType)
struct FRegexPatternWrapper
{
    GENERATED_BODY()

public:
    FRegexPatternWrapper() : Pattern(TEXT(".*")) {}
    FRegexPatternWrapper(const FString& SourceString, ERegexPatternFlags Flags = ERegexPatternFlags::None) : Pattern(FRegexPattern(SourceString, Flags)) {}
    FRegexPatternWrapper(const FRegexPattern& Other) : Pattern(Other) {}
    FRegexPatternWrapper(FRegexPattern&& Other) : Pattern(MoveTemp(Other)) {}

    FRegexPatternWrapper& operator=(const FRegexPattern& Other);
    FRegexPatternWrapper& operator=(FRegexPattern&& Other);

public:
    FRegexPattern Pattern;
};