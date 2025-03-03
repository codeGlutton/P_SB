#pragma once
 
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemGoogleExTypes.h"
#include "OnlineJsonSerializer.h"
#include "Interfaces/IHttpRequest.h"
#include "PlatformHttp.h"

#define GOOGLE_EX_PERM_PUBLIC_PROFILE "https://www.googleapis.com/auth/userinfo.profile"
#define AUTH_TYPE_GOOGLE_EX TEXT("google_ex")

class FOnlineSubsystemGoogleEx;
class FUserOnlineAccountGoogleEx;

// identitiy URL 디테일 정보
struct FGoogleLoginURL
{
	FGoogleLoginURL() : 
		RedirectPort(9000),
		AccessType(TEXT("online"))
	{
	}

public:
	bool							IsValid() const
	{
		return !LoginRedirectUrl.IsEmpty() && !ClientId.IsEmpty() && !RedirectPath.IsEmpty() && (RedirectPort > 0) && (ScopeFields.Num() > 0);
	}

	// State 값 랜덤으로 채우기
	FString							GenerateNonce()
	{
		State = FString::FromInt(FMath::Rand() % 100000);
		return State;
	}

	FString							GetRedirectURL() const
	{
		return FString::Printf(TEXT("%s:%d/%s"), *LoginRedirectUrl, RedirectPort, *RedirectPath);
	}

	// 초기 구글 소셜 로그인 연결 URL
	FString							GetURL() const
	{
		FString Scopes = FString::Join(ScopeFields, TEXT(" "));

		const FString Redirect = GetRedirectURL();

		const FString ParamsString = FString::Printf(TEXT("redirect_uri=%s&scope=%s&response_type=code&client_id=%s&state=%s&access_type=%s"),
			*Redirect, *FPlatformHttp::UrlEncode(Scopes), *ClientId, *State, *AccessType);

		// auth url to spawn in browser
		const FString URLString = FString::Printf(TEXT("%s?%s"),
			*LoginUrl, *ParamsString);

		return URLString;
	}

public:
	// 구글 exchange 토큰 end point url
	FString							LoginUrl;
	// 구글 요청 완료시 리다이렉트 될 url
	FString							LoginRedirectUrl;
	// 구글 요청 완료시 리다이렉트 될 port
	int32							RedirectPort;
	// 구글 요청 완료시 리다이렉트 될 path
	FString							RedirectPath;
	// OnlineSubsystemGoogleEx에서 받아온 client id
	FString							ClientId;
	// 로그인 시 사용할 허가 범위 리스트
	TArray<FString>					ScopeFields;
	// Replay 공격 완화를 위한 랜덤 값 (송수신 시 일치 확인)
	FString							State;
	// Online or Offline (디버깅에서만)
	FString							AccessType;
};

// 온라인 identity 인터페이스의 구글 서비스 구현
class FOnlineIdentityGoogleEx :
	public IOnlineIdentity
{
public:
	FOnlineIdentityGoogleEx(FOnlineSubsystemGoogleEx* InSubsystem);

	virtual ~FOnlineIdentityGoogleEx()
	{
	}

protected:
	typedef TFunction<void(bool)>						PendingLoginRequestCb;

public:
	/* 온라인 identitiy interface */

	virtual bool										Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials) override;
	virtual bool										Logout(int32 LocalUserNum) override;
	virtual bool										AutoLogin(int32 LocalUserNum) override;
	virtual TSharedPtr<FUserOnlineAccount>				GetUserAccount(const FUniqueNetId& UserId) const override;
	virtual TArray<TSharedPtr<FUserOnlineAccount> >		GetAllUserAccounts() const override;
	virtual FUniqueNetIdPtr								GetUniquePlayerId(int32 LocalUserNum) const override;
	virtual FUniqueNetIdPtr								CreateUniquePlayerId(uint8* Bytes, int32 Size) override;
	virtual FUniqueNetIdPtr								CreateUniquePlayerId(const FString& Str) override;
	virtual ELoginStatus::Type							GetLoginStatus(int32 LocalUserNum) const override;
	virtual ELoginStatus::Type							GetLoginStatus(const FUniqueNetId& UserId) const override;
	virtual FString										GetPlayerNickname(int32 LocalUserNum) const override;
	virtual FString										GetPlayerNickname(const FUniqueNetId& UserId) const override;
	virtual FString										GetAuthToken(int32 LocalUserNum) const override;
	virtual void										GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate) override;
	virtual FPlatformUserId								GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const override;
	virtual FString										GetAuthType() const override;
	virtual void										RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate) override;

public:
	// 로그인 시도 (토큰 존재 or 빈 토큰)
	void												Login(int32 LocalUserNum, const FAuthTokenGoogleEx& InToken, const FOnLoginCompleteDelegate& InCompletionDelegate);

	// auth code를 통해 실제 access 토큰과 refresh 토큰으로 교환
	void												ExchangeCode(int32 LocalUserNum, const FAuthTokenGoogleEx& InExchangeToken, const FOnLoginCompleteDelegate& InCompletionDelegate);

	const FGoogleLoginURL&								GetLoginURLDetails() const { return LoginURLDetails; }

protected:

	// 구글 인증 서비스 end points 검색 요청
	void												RetrieveDiscoveryDocument(PendingLoginRequestCb&& LoginCb);

	// 구글 인증 서비스 end points 검색 완료시 처리 함수
	void												DiscoveryRequest_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, PendingLoginRequestCb LoginCb);

private:
	// 모든 로그인 시도가 완료시 처리 함수
	void												OnLoginAttemptComplete(FUniqueNetIdPtr NetId, int32 LocalUserNum, const FString& ErrorStr);

	// 로그인 외부 UI와 함께 결과를 받았을때 처리 함수
	void												OnExternalUILoginComplete(FUniqueNetIdPtr NetId, const int ControllerIndex, const FOnlineError& Error);

protected:
	// 해당 객체가 속한 구글 subsystem
	FOnlineSubsystemGoogleEx*							GoogleExSubsystem;
	// 구글 서비스에서 받아온 end point 설정 정보 객체
	FGoogleExOpenIDConfiguration						Endpoints;

	// 구글 API와 통신하기 위한 URL 상수 값들
	FGoogleLoginURL										LoginURLDetails;
	// 등록이 진행중인지 여부
	bool												bHasLoginOutstanding;
	// 쿠키 관리를 위해 보관되는 로그인시 사용되는 도메인 리스트
	TArray<FString>										LoginDomains;
};

typedef TSharedPtr<FOnlineIdentityGoogleEx, ESPMode::ThreadSafe> FOnlineIdentityGoogleExPtr;
