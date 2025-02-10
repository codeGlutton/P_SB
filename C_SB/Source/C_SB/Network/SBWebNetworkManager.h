#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "HttpProtocol.pb.h"
#include "Wrappers.h"

#include "SBWebNetworkManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWebManager, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIdCheckResRecved, bool, bIsSuccess, FString, ErrStr);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSignUpResRecved, bool, bIsSuccess, FString, ErrStr);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoginResRecved, bool, bIsSuccess, FString, ErrStr);

/// <summary>
/// Http 패킷이 어떤 카테고리의 정보를 다루는지
/// </summary>
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ESBHttpPktTypeFlag : uint8
{
	NONE = 0 UMETA(DisplayName = "None"),

	ACCOUNT = 1 << 0 UMETA(DisplayName = "Account"),
	RANKING = 1 << 1 UMETA(DisplayName = "Ranking")
};

/*************************
   USBWebNetworkManager
*************************/

UCLASS()
class C_SB_API USBWebNetworkManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USBWebNetworkManager();
	
public:
	/* HTTP 패킷 송수신 함수들 */
	
	/**
	 * 사용가능한 아이디인지 중복 검사 Http 요청
	 * @param Id 검증할 아이디
	 */
	UFUNCTION(BlueprintCallable)
	void						RequestToCheckUsableId(const FText& Id);
	
	/**
	 * 계정 가입 Http 요청
	 * @param Id 새 아이디
	 * @param Password 새 비밀번호
	 */
	UFUNCTION(BlueprintCallable)
	void						RequestToSignUp(const FText& Id, const FText& Password);

	/**
	 * 계정 로그인 Http 요청
	 * @param Id 아이디
	 * @param Password 비밀번호
	 */
	UFUNCTION(BlueprintCallable)
	void						RequestToLogin(const FText& Id, const FText& Password);

	void						ResponseToCheckUsableId(const Protocol::RES_CHECK_EXISTS_ACCOUNT& CreatePkt);
	void						ResponseToSignUp(const Protocol::RES_CREATE_ACCOUNT& CreatePkt);
	void						ResponseToLogin(const Protocol::RES_LOGIN_ACCOUNT& LoginPkt);

	void						ErrorFromCheckUsableId(FString ErrStr);
	void						ErrorFromSignUp(FString ErrStr);
	void						ErrorFromLogin(FString ErrStr);

	/* 로컬 확인 함수들 */

	/**
	 * 사용 가능한 비밀번호인지 검증
	 * @param Password 검증할 비밀번호
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable)
	bool						CheckUsablePassword(const FText& Password);

	/**
	 * 로그인 여부 
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable)
	bool						IsLoggedIn() { return bIsLoggedIn; }
	
	/**
	 * 어떤 응답 패킷 종류들을 대기중인지
	 * @return 응답 패킷 Flags
	 */
	UFUNCTION(BlueprintCallable)
	ESBHttpPktTypeFlag			GetWaitingResTypeFlags() { return WaitingResTypeFlags; }

	/**
	 * 특정 패킷 종류가 대기중인지
	 * @param HttpOktTypeFlag 원하는 패킷 종류
	 * @return 응답 중인지 여부
	 */
	UFUNCTION(BlueprintCallable)
	bool						IsWaitingSpecificRes(const ESBHttpPktTypeFlag HttpOktTypeFlag);

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnIdCheckResRecved			OnIdCheckResRecved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSignUpResRecved			OnSignUpResRecved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnLoginResRecved			OnLoginResRecved;

private:
	bool						bIsLoggedIn;

	// 응답 대기 중인 Http 패킷 타입들
	// (같은 카테고리 패킷에 대해서 다중 요청이 불가)
	ESBHttpPktTypeFlag			WaitingResTypeFlags;

	FRegexPatternWrapper		PasswordPattern;
};
