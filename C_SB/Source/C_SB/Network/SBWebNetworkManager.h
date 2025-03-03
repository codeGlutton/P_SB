#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"

#include "Types.h"

#include "Protocol.pb.h"
#include "HttpProtocol.pb.h"
#include "Wrappers.h"

#include "GoogleWidget.h"

#include "SBWebNetworkManager.generated.h"

USING_SHARED_PTR(ILoginFlowManager);

DECLARE_LOG_CATEGORY_EXTERN(LogWebManager, Log, All);

DECLARE_DELEGATE(FOnPopupDismissed);
DECLARE_MULTICAST_DELEGATE(FOnGetGoogleDismissedWidget);

DECLARE_DELEGATE_RetVal_OneParam(FOnPopupDismissed, FOnDisplayPopup, const TSharedRef<SWidget>& /*LoginWidget*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGetGoogleWidget, const TSharedRef<SWidget>& /*LoginWidget*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHttpResRecved, bool, bIsSuccess, FString, ErrStr);

/**
 * Http 패킷이 어떤 카테고리의 정보를 다루는지
 */
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

	virtual void					Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void					Deinitialize() override;
	
public:
	/**
	 * 사용가능한 아이디인지 중복 검사 Http 요청
	 * @param Id 검증할 아이디
	 */
	UFUNCTION(BlueprintCallable)
	void							RequestToCheckUsableName(const FText& AccountName);
	
	/**
	 * 계정 가입 Http 요청
	 * @param Id 새 아이디
	 * @param Password 새 비밀번호
	 */
	UFUNCTION(BlueprintCallable)
	void							RequestToSignUp(const FText& AccountName, const FText& Password);

	/**
	 * 계정 로그인 Http 요청
	 * @param Id 아이디
	 * @param Password 비밀번호
	 */
	UFUNCTION(BlueprintCallable)
	void							RequestToLogin(const FText& AccountName, const FText& Password);

	/**
	 * 계정 구글 로그인 Http 요청
	 */
	UFUNCTION(BlueprintCallable)
	void							RequestToGoogleLogin();

public:
	/**
	 * 사용 가능한 아이디인지 검증
	 * @param AccountId 검증할 아이디
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable)
	bool							CheckUsableAccountName(const FText& AccountName);

	/**
	 * 사용 가능한 비밀번호인지 검증
	 * @param Password 검증할 비밀번호
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable)
	bool							CheckUsablePassword(const FText& Password);

	/**
	 * 로그인 여부 
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable)
	bool							IsLoggedIn() { return _bIsLoggedIn; }
	
	/**
	 * 어떤 응답 패킷 종류들을 대기중인지
	 * @return 응답 패킷 Flags
	 */
	UFUNCTION(BlueprintCallable)
	ESBHttpPktTypeFlag				GetWaitingResTypeFlags() { return _WaitingResTypeFlags; }

	/**
	 * 특정 패킷 종류가 대기중인지
	 * @param HttpOktTypeFlag 원하는 패킷 종류
	 * @return 응답 중인지 여부
	 */
	UFUNCTION(BlueprintCallable)
	bool							IsWaitingSpecificRes(const ESBHttpPktTypeFlag HttpOktTypeFlag);

	/**
	 * 구글 소셜 로그인시, 팝업으로 등장할 위젯을 바인딩
	 * @param GoogleWidget 연결할 구글 위젯
	 */
	UFUNCTION(BlueprintCallable)
	void							BindGoogleWidget(UGoogleWidget* GoogleWidget);

	/**
	 * 구글 소셜 로그인에 사용한 팝업 위젯 언바인딩
	 * @param GoogleWidget 끊을 구글 위젯
	 */
	UFUNCTION(BlueprintCallable)
	void							UnbindGoogleWidget(UGoogleWidget* GoogleWidget);

public:
	void							RequestToRecheckServer(const int32& AccountId, const std::string& TokenValue);

	void							ResponseToCheckUsableName(const Protocol::RES_CHECK_EXISTS_ACCOUNT& CheckPkt);
	void							ResponseToSignUp(const Protocol::RES_CREATE_ACCOUNT& CreatePkt);
	void							ResponseToLogin(const Protocol::RES_LOGIN_ACCOUNT& LoginPkt);
	void							ResponseToRecheck(const Protocol::RES_RECHECK_SERVER& RecheckPkt);

	void							ResponseToLogOut();

	void							ErrorFromCheckUsableId(FString ErrStr);
	void							ErrorFromSignUp(FString ErrStr);
	void							ErrorFromLogin(FString ErrStr);
	void							ErrorFromRecheck(FString ErrStr);

private:
	FOnPopupDismissed				OpenGooglePopupWidget(const TSharedRef<SWidget>& LoginWidget);
	FOnPopupDismissed				OpenGoogleSingUpPopupWidget(const TSharedRef<SWidget>& SignUpWidget);

public:
	/* 블루프린트 Notify */

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnHttpResRecved				OnIdCheckResRecved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnHttpResRecved				OnSignUpResRecved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnHttpResRecved				OnLoginResRecved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnHttpResRecved				OnRecheckResRecved;

private:
	/* 구글 위젯과 연동용 대리자 */

	FOnGetGoogleWidget				_OnGetGoogleLoginWidget;
	FOnGetGoogleWidget				_OnGetGoogleSignUpWidget;
	FOnGetGoogleDismissedWidget		_OnGetGoogleLoginDismissedWidget;
	FOnGetGoogleDismissedWidget		_OnGetGoogleSignUpDismissedWidget;

private:
	class IOnlineSubsystem*			_GoogleOnlineSubSysytem;
	ILoginFlowManagerRef			_GoogleLoginFlowManager;
	FDelegateHandle					_GoogleLoginHandle;

	bool							_bIsLoggedIn;

	// 응답 대기 중인 Http 패킷 타입들
	// (같은 카테고리 패킷에 대해서 다중 요청이 불가)
	ESBHttpPktTypeFlag				_WaitingResTypeFlags;

	FRegexPatternWrapper			_AccountIdPattern;
	FRegexPatternWrapper			_PasswordPattern;
};
