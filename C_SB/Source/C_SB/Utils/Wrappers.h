#pragma once

#include "CoreMinimal.h"
#include "Struct.pb.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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

/*************************
	FSBServerSelectInfo
*************************/

/**
 * 서버 선택창에 표기될 일부 서버 정보 구조체
 */
USTRUCT(BlueprintType)
struct C_SB_API FSBServerSelectInfo
{
	GENERATED_USTRUCT_BODY()
public:
	FSBServerSelectInfo() : ServerId(0), Name(), Dencity(0.f) {}
	FSBServerSelectInfo(const int32& ServerId, const FString& Name, const float& Dencity) : ServerId(ServerId), Name(Name), Dencity(Dencity) {}
	FSBServerSelectInfo(int32&& ServerId, FString&& Name, float&& Dencity) : ServerId(MoveTemp(ServerId)), Name(MoveTemp(Name)), Dencity(MoveTemp(Dencity)) {}
	FSBServerSelectInfo(const Protocol::ServerSelectInfo& ServerSeclectInfo);
	FSBServerSelectInfo(Protocol::ServerSelectInfo&& ServerSeclectInfo);
	FSBServerSelectInfo(const Protocol::ServerInfo& ServerInfo);
	FSBServerSelectInfo(Protocol::ServerInfo&& ServerInfo);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 ServerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float Dencity;

public:
	bool operator==(FSBServerSelectInfo const& Other) const;
};

/*************************
	FSBPlayerSelectInfo
*************************/

/**
 * 캐릭터 선택창에 표기될 일부 캐릭터 정보 구조체
 */
USTRUCT(BlueprintType)
struct C_SB_API FSBPlayerSelectInfo
{
	GENERATED_USTRUCT_BODY()
public:
	FSBPlayerSelectInfo() : PlayerDbId(0), PlayerTableId(0), Name(), TeamUniform(0), TeamShoes(0), TeamAccessories(0), Score(0), CreatedTime() {}
	FSBPlayerSelectInfo(const int32& PlayerDbId, const int32& PlayerTableId, const FString& Name, const int32& TeamUniform, const int32& TeamShoes, const int32& TeamAccessories, const int32& Score, const FDateTime& CreatedTime) :
		PlayerDbId(PlayerDbId),
		PlayerTableId(PlayerTableId),
		Name(Name),
		TeamUniform(TeamUniform),
		TeamShoes(TeamShoes),
		TeamAccessories(TeamAccessories),
		Score(Score),
		CreatedTime(CreatedTime)
	{
	}
	FSBPlayerSelectInfo(int32&& PlayerDbId, int32&& PlayerTableId, FString&& Name, int32&& TeamUniform, int32&& TeamShoes, int32&& TeamAccessories, int32&& Score, FDateTime&& CreatedTime) :
		PlayerDbId(MoveTemp(PlayerDbId)),
		PlayerTableId(PlayerTableId),
		Name(MoveTemp(Name)),
		TeamUniform(MoveTemp(TeamUniform)),
		TeamShoes(MoveTemp(TeamShoes)),
		TeamAccessories(MoveTemp(TeamAccessories)),
		Score(MoveTemp(Score)),
		CreatedTime(MoveTemp(CreatedTime))
	{
	}
	FSBPlayerSelectInfo(const int32& PlayerDbId, const int32& PlayerTableId, const FString& Name, const int32& CostumeSetting, const int32& Score, const google::protobuf::Timestamp& CreatedTime);
	FSBPlayerSelectInfo(int32&& PlayerDbId, int32& PlayerTableId, FString&& Name, int32&& CostumeSetting, int32&& Score, google::protobuf::Timestamp&& CreatedTime);
	FSBPlayerSelectInfo(const Protocol::PlayerSelectInfo& PlayerSeclectInfo);
	FSBPlayerSelectInfo(Protocol::PlayerSelectInfo&& PlayerSeclectInfo);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 PlayerDbId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 PlayerTableId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 TeamUniform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 TeamShoes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 TeamAccessories;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 Score;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FDateTime CreatedTime;

public:
	bool operator==(FSBPlayerSelectInfo const& Other) const;
};

/************************
          Enums
*************************/

UENUM(BlueprintType)
enum class ESBTier : uint8
{
	BRONZE = 0 UMETA(DisplayName = "Bronze"),
	SILVER UMETA(DisplayName = "Silver"),
    GOLD UMETA(DisplayName = "Gold"),
	PLATINUM UMETA(DisplayName = "Platinum"),
	DIAMOND UMETA(DisplayName = "Diamond")
};

/************************
        Functions
*************************/

UCLASS()
class C_SB_API USBFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "BP Wrapper")
    static ESBTier GetTier(const int32& Score);
};