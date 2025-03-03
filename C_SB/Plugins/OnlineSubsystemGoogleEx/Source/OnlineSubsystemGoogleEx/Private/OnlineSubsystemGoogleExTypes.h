#pragma once

#include "OnlineSubsystemTypes.h"
#include "OnlineJsonSerializer.h"
#include "OnlineSubsystemGoogleExPrivate.h"

class FAuthTokenGoogleEx;

TEMP_UNIQUENETIDSTRING_SUBCLASS(FUniqueNetIdGoogleEx, GOOGLE_EX_SUBSYSTEM);

// 지원하는 구글 토큰 타입
enum class EGoogleExAuthTokenType : uint8
{
	// auth 승인 코드를 통해 엑세스 토큰을 발급시 사용
	ExchangeToken,
	// 구글 refresh 토큰
	RefreshToken,
	// 구글 access 토큰
	AccessToken
};

class FJsonWebTokenGoogleEx
{

public:

	// 구글 JWT id 토큰 Str을 파싱
	bool		Parse(const FString& InJWTStr);

private:

	class FHeader : public FOnlineJsonSerializable
	{

	public:
		// 토큰 서명 알고리즘
		FString		Alg;
		// 키 아이디
		FString		Kid;
		BEGIN_ONLINE_JSON_SERIALIZER
			ONLINE_JSON_SERIALIZE("alg", Alg);
			ONLINE_JSON_SERIALIZE("kid", Kid);
		END_ONLINE_JSON_SERIALIZER
	};

	class FPayload : public FOnlineJsonSerializable
	{

	public:

		// 요청 수행자 id
		FString		Sub;
		// 유저 성
		FString		FirstName;
		// 유저 이름
		FString		LastName;
		// 유저 퓰 네임
		FString		RealName;
		// 토큰 발급인
		FString		ISS;
		// 토큰 발급시간
		double		IAT;
		// 토큰 만기시간
		double		EXP;
		// 엑세스 토큰 보안 해시
		FString		ATHash;
		// 토큰 사용 대상
		FString		Aud;
		// 이메일 인증 여부
		bool		bEmailVerified;
		// 토큰 발급 대상
		FString		AZP;
		// 유저 이메일
		FString		Email;
		// 유저 프로필 사진
		FString		Picture;
		// 유저 언어
		FString		Locale;

		BEGIN_ONLINE_JSON_SERIALIZER
			ONLINE_JSON_SERIALIZE("sub", Sub);
			ONLINE_JSON_SERIALIZE("given_name", FirstName);
			ONLINE_JSON_SERIALIZE("family_name", LastName);
			ONLINE_JSON_SERIALIZE("name", RealName);
			ONLINE_JSON_SERIALIZE("iss", ISS);
			ONLINE_JSON_SERIALIZE("iat", IAT);
			ONLINE_JSON_SERIALIZE("exp", EXP);
			ONLINE_JSON_SERIALIZE("at_hash", ATHash);
			ONLINE_JSON_SERIALIZE("aud", Aud);
			ONLINE_JSON_SERIALIZE("email_verified", bEmailVerified);
			ONLINE_JSON_SERIALIZE("azp", AZP);
			ONLINE_JSON_SERIALIZE("email", Email);
			ONLINE_JSON_SERIALIZE("picture", Picture);
			ONLINE_JSON_SERIALIZE("locale", Locale);
		END_ONLINE_JSON_SERIALIZER
	};

	FHeader		Header;
	FPayload	Payload;
};

enum EGoogleExExchangeToken : uint8 { GoogleExchangeToken };
enum EGoogleExRefreshToken : uint8 { GoogleRefreshToken };

// 구글 인증 토큰 
class FAuthTokenGoogleEx :
	public FOnlineJsonSerializable
{
public:
	FAuthTokenGoogleEx()
		: AuthType(EGoogleExAuthTokenType::ExchangeToken)
		, ExpiresIn(0)
		, ExpiresInUTC(0)
	{
	}

	explicit FAuthTokenGoogleEx(const FString& InExchangeToken, EGoogleExExchangeToken)
		: AuthType(EGoogleExAuthTokenType::ExchangeToken)
		, AccessToken(InExchangeToken)
		, ExpiresIn(0)
		, ExpiresInUTC(0)
	{
	}

	explicit FAuthTokenGoogleEx(const FString& InRefreshToken, EGoogleExRefreshToken)
		: AuthType(EGoogleExAuthTokenType::RefreshToken)
		, ExpiresIn(0)
		, RefreshToken(InRefreshToken)
		, ExpiresInUTC(0)
	{
	}

	FAuthTokenGoogleEx(FAuthTokenGoogleEx&&) = default;
	FAuthTokenGoogleEx(const FAuthTokenGoogleEx&) = default;
	FAuthTokenGoogleEx& operator=(FAuthTokenGoogleEx&&) = default;
	FAuthTokenGoogleEx& operator=(const FAuthTokenGoogleEx&) = default;

	// 구글 응답 string을 받아 데이터로 파싱하는 함수
	bool							Parse(const FString& InJsonStr);

	// json형태의 값들를 받아 데이터로 파싱하는 함수
	bool							Parse(TSharedPtr<FJsonObject> InJsonObject);

	// 오래된 Refresh 포함 토큰 데이터를 참조하여 구글 응답 string을 데이터로 파싱하는 함수
	bool							Parse(const FString& InJsonStr, const FAuthTokenGoogleEx& InOldAuthToken);

	// 토큰 종류에 맞추어 데이터가 유효한가
	bool							IsValid() const
	{
		if (AuthType == EGoogleExAuthTokenType::ExchangeToken)
		{
			return !AccessToken.IsEmpty() && RefreshToken.IsEmpty();
		}
		else if (AuthType == EGoogleExAuthTokenType::RefreshToken)
		{
			return AccessToken.IsEmpty() && !RefreshToken.IsEmpty();
		}
		else
		{
			return !AccessToken.IsEmpty() && !RefreshToken.IsEmpty();
		}
	}

	// access 토큰이 만기되었는지
	bool							IsExpired() const
	{
		if (AuthType == EGoogleExAuthTokenType::AccessToken)
		{
			return (FDateTime::UtcNow() > ExpiresInUTC);
		}

		return false;
	}

	// 토큰 기타 데이터 값 얻기
	inline bool						GetAuthData(const FString& Key, FString& OutVal) const
	{
		const FString* FoundVal = AuthData.Find(Key);
		if (FoundVal != NULL)
		{
			OutVal = *FoundVal;
			return true;
		}
		return false;
	}

private:
	void							AddAuthAttributes(const TSharedPtr<FJsonObject>& JsonUser);

public:
	// 해당 클래스가 다루는 토큰 타입
	EGoogleExAuthTokenType			AuthType;
	// access 토큰 str 혹은 exchange 토큰 str(auth code)
	FString							AccessToken;
	// 토큰 타입 (access 토큰에서만 유효)
	FString							TokenType;
	// 만기 시간까지 유효한 시간 초
	double							ExpiresIn;
	// Refresh 토큰 str
	FString							RefreshToken;
	// JWT Id 토큰 str
	FString							IdToken;
	// IdToken를 파싱한 데이터들
	FJsonWebTokenGoogleEx			IdTokenJWT;
	// UTC 기준 만기시간
	FDateTime						ExpiresInUTC;

private:
	// 토큰과 관련된 주요 값들 외 기타 데이터 모음
	FJsonSerializableKeyValueMap	AuthData;

	BEGIN_ONLINE_JSON_SERIALIZER
		ONLINE_JSON_SERIALIZE("access_token", AccessToken);
		ONLINE_JSON_SERIALIZE("token_type", TokenType);
		ONLINE_JSON_SERIALIZE("expires_in", ExpiresIn);
		ONLINE_JSON_SERIALIZE("refresh_token", RefreshToken);
		ONLINE_JSON_SERIALIZE("id_token", IdToken);
	END_ONLINE_JSON_SERIALIZER
};

// 구글 API 서비스와 관련된 모든 설정 클래스
class FGoogleExOpenIDConfiguration :
	public FOnlineJsonSerializable
{
public:

	FGoogleExOpenIDConfiguration()
		: bInitialized(false)
		, AuthEndpoint(TEXT("https://accounts.google.com/o/oauth2/v2/auth"))
		, TokenEndpoint(TEXT("https://www.googleapis.com/oauth2/v4/token"))
		, UserInfoEndpoint(TEXT("https://www.googleapis.com/oauth2/v2/userinfo")) //"https://www.googleapis.com/userinfo/v2/me"
		, RevocationEndpoint(TEXT("https://accounts.google.com/o/oauth2/revoke"))
	{
	}

	bool							IsValid() const { return bInitialized; }

	// 구글 설정에 관한 응답 str을 파싱하여 저장
	bool							Parse(const FString& InJsonStr)
	{
		if (!InJsonStr.IsEmpty() && FromJson(InJsonStr))
		{
			bInitialized = true;
		}

		return bInitialized;
	}

	// 구글 설정 정보를 받아 초기화했는지
	bool							bInitialized;
	// 설정 정보의 발급자
	FString							Issuer;
	// 로그인을 위한 인증 end point
	FString							AuthEndpoint;
	// 토큰 auth code와 access token 교환을 위한 end point
	FString							TokenEndpoint;
	// 유저 프로필 요청 end point
	FString							UserInfoEndpoint;
	// 권한 취소 endpoint
	FString							RevocationEndpoint;
	// JWT 인증서 endpoint 
	FString							JWKSURI;

private:

	BEGIN_ONLINE_JSON_SERIALIZER
		ONLINE_JSON_SERIALIZE("issuer", Issuer);
		ONLINE_JSON_SERIALIZE("authorization_endpoint", AuthEndpoint);
		ONLINE_JSON_SERIALIZE("token_endpoint", TokenEndpoint);
		ONLINE_JSON_SERIALIZE("userinfo_endpoint", UserInfoEndpoint);
		ONLINE_JSON_SERIALIZE("revocation_endpoint", RevocationEndpoint);
		ONLINE_JSON_SERIALIZE("jwks_uri", JWKSURI);
	END_ONLINE_JSON_SERIALIZER
};

// 구글 JWP 페이로드의 에러 데이터
class FErrorGoogleEx :
	public FOnlineJsonSerializable
{
public:

	FErrorGoogleEx()
	{
	}

	FString							ToDebugString() const { return FString::Printf(TEXT("%s [Desc:%s]"), *Error, *Error_Description); }

public:
	FString							Error;
	FString							Error_Description;

	BEGIN_ONLINE_JSON_SERIALIZER
		ONLINE_JSON_SERIALIZE("error", Error);
		ONLINE_JSON_SERIALIZE("error_description", Error_Description);
	END_ONLINE_JSON_SERIALIZER
};
