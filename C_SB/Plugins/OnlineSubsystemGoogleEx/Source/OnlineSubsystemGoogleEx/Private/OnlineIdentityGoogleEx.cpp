#include "OnlineIdentityGoogleEx.h"
#include "OnlineSubsystemGoogleExPrivate.h"
#include "OnlineSubsystemGoogleExTypes.h"
#include "OnlineSubsystemGoogleEx.h"
#include "OnlineExternalUIGoogleEx.h"
#include "OnlineError.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Base64.h"

bool FJsonWebTokenGoogleEx::Parse(const FString& InJWTStr)
{
	bool bSuccess = false;

	TArray<FString> Tokens;
	InJWTStr.ParseIntoArray(Tokens, TEXT("."));
	if (Tokens.Num() == 3)
	{
		/* Base64 패딩 적용이 요구되는지 파악 */

		static const TCHAR* const Padding = TEXT("==");
		int32 Padding1 = (4 - (Tokens[0].Len() % 4)) % 4;
		int32 Padding2 = (4 - (Tokens[1].Len() % 4)) % 4;
		int32 Padding3 = (4 - (Tokens[2].Len() % 4)) % 4;
		if (Padding1 < 3 && Padding2 < 3 && Padding3 < 3)
		{
			Tokens[0].AppendChars(Padding, Padding1);
			Tokens[1].AppendChars(Padding, Padding2);
			Tokens[2].AppendChars(Padding, Padding3);

			// JWT 헤더 디코딩
			FString HeaderStr;
			if (FBase64::Decode(Tokens[0], HeaderStr))
			{
				// 헤더 데이터로 파싱
				if (Header.FromJson(HeaderStr))
				{
					// JWT 페이로드 디코딩
					FString PayloadStr;
					if (FBase64::Decode(Tokens[1], PayloadStr))
					{
						// 페이로드 데이터로 파싱
						if (Payload.FromJson(PayloadStr))
						{
							/* iss 일치 검증 */

							static const FString Issuer1 = TEXT("https://accounts.google.com");
							static const FString Issuer2 = TEXT("accounts.google.com");
							if ((Payload.ISS == Issuer1) || (Payload.ISS == Issuer2))
							{
								/* 클라이언트 id 일치 검증 */

								FOnlineSubsystemGoogleEx* GoogleExSubsystem = static_cast<FOnlineSubsystemGoogleEx*>(IOnlineSubsystem::Get(GOOGLE_EX_SUBSYSTEM));
								if (ensure(GoogleExSubsystem))
								{
									if (Payload.Aud == GoogleExSubsystem->GetAppId())
									{
										/* 토큰 만기 시간 유효성 검증 */

										FDateTime ExpiryTime = FDateTime::FromUnixTimestamp(Payload.EXP);
										FDateTime IssueTime = FDateTime::FromUnixTimestamp(Payload.IAT);
										if ((ExpiryTime - IssueTime) <= FTimespan(ETimespan::TicksPerHour) && ExpiryTime > FDateTime::UtcNow())
										{
											bSuccess = true;
										}
										else
										{
											UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Google auth: Expiry Time inconsistency"));
											UE_LOG_ONLINE_IDENTITY(Warning, TEXT("	Expiry: %s"), *ExpiryTime.ToString());
											UE_LOG_ONLINE_IDENTITY(Warning, TEXT("	Issue: %s"), *IssueTime.ToString());
										}
									}
									else
									{
										UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Google auth: Audience inconsistency"));
										UE_LOG_ONLINE_IDENTITY(Warning, TEXT("	Payload: %s"), *Payload.Aud);
										UE_LOG_ONLINE_IDENTITY(Warning, TEXT("	ClientId: %s"), *GoogleExSubsystem->GetAppId());
									}
								}
								else
								{
									UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Google auth: missing OSS"));
								}
							}
							else
							{
								UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Google auth: Issuer inconsistency"));
								UE_LOG_ONLINE_IDENTITY(Warning, TEXT("	ISS: %s"), *Payload.ISS);
							}
						}
						else
						{
							UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Google auth: Payload data inconsistency"));
						}
					}
					else
					{
						UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Google auth: Payload format inconsistency"));
					}
				}
				else
				{
					UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Google auth: Header data inconsistency"));
				}
			}
			else
			{
				UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Google auth: Header format inconsistency"));
			}
		}
		else
		{
			UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Google auth: JWT format inconsistency"));
		}
	}

	return bSuccess;
}

void FAuthTokenGoogleEx::AddAuthAttributes(const TSharedPtr<FJsonObject>& JsonUser)
{
	for (auto It = JsonUser->Values.CreateConstIterator(); It; ++It)
	{
		if (It.Value().IsValid())
		{
			if (It.Value()->Type == EJson::String)
			{
				AuthData.Add(It.Key(), It.Value()->AsString());
			}
			else if (It.Value()->Type == EJson::Boolean)
			{
				AuthData.Add(It.Key(), It.Value()->AsBool() ? TEXT("true") : TEXT("false"));
			}
			else if (It.Value()->Type == EJson::Number)
			{
				AuthData.Add(It.Key(), FString::Printf(TEXT("%f"), (double)It.Value()->AsNumber()));
			}
		}
	}
}

bool FAuthTokenGoogleEx::Parse(const FString& InJsonStr, const FAuthTokenGoogleEx& InOldAuthToken)
{
	bool bSuccess = false;
	if ((InOldAuthToken.AuthType == EGoogleExAuthTokenType::RefreshToken) && Parse(InJsonStr))
	{
		RefreshToken = InOldAuthToken.RefreshToken;
		AuthData.Add(TEXT("refresh_token"), InOldAuthToken.RefreshToken);
		bSuccess = true;
	}

	return bSuccess;
}

bool FAuthTokenGoogleEx::Parse(const FString& InJsonStr)
{
	bool bSuccess = false;

	if (!InJsonStr.IsEmpty())
	{
		TSharedPtr<FJsonObject> JsonAuth;
		TSharedRef< TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(InJsonStr);

		if (FJsonSerializer::Deserialize(JsonReader, JsonAuth) &&
			JsonAuth.IsValid())
		{
			bSuccess = Parse(JsonAuth);
		}
	}
	else
	{
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("FAuthTokenGoogleEx: Empty Json string"));
	}

	return bSuccess;
}

bool FAuthTokenGoogleEx::Parse(TSharedPtr<FJsonObject> InJsonObject)
{
	bool bSuccess = false;

	if (InJsonObject.IsValid())
	{
		if (FromJson(InJsonObject))
		{
			if (!AccessToken.IsEmpty())
			{
				if (IdTokenJWT.Parse(IdToken))
				{
					AddAuthAttributes(InJsonObject);
					AuthType = EGoogleExAuthTokenType::AccessToken;
					ExpiresInUTC = FDateTime::UtcNow() + FTimespan(ExpiresIn * ETimespan::TicksPerSecond);
					bSuccess = true;
				}
			}
		}
	}
	else
	{
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("FAuthTokenGoogleEx: Invalid Json pointer"));
	}

	return bSuccess;
}

FOnlineIdentityGoogleEx::FOnlineIdentityGoogleEx(FOnlineSubsystemGoogleEx* InSubsystem)
	: GoogleExSubsystem(InSubsystem)
	, bHasLoginOutstanding(false)
{
	if (!GConfig->GetString(TEXT("OnlineSubsystemGoogleEx.OnlineIdentityGoogleEx"), TEXT("LoginRedirectUrl"), LoginURLDetails.LoginRedirectUrl, GEngineIni))
	{
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Missing LoginRedirectUrl= in [OnlineSubsystemGoogleEx.OnlineIdentityGoogleEx] of DefaultEngine.ini"));
	}
	if (!GConfig->GetInt(TEXT("OnlineSubsystemGoogleEx.OnlineIdentityGoogleEx"), TEXT("RedirectPort"), LoginURLDetails.RedirectPort, GEngineIni))
	{
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Missing RedirectPort= in [OnlineSubsystemGoogleEx.OnlineIdentityGoogleEx] of DefaultEngine.ini"));
	}
	if (!GConfig->GetString(TEXT("OnlineSubsystemGoogleEx.OnlineIdentityGoogleEx"), TEXT("RedirectPath"), LoginURLDetails.RedirectPath, GEngineIni))
	{
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Missing RedirectPort= in [OnlineSubsystemGoogleEx.OnlineIdentityGoogleEx] of DefaultEngine.ini"));
	}

	GConfig->GetArray(TEXT("OnlineSubsystemGoogleEx.OnlineIdentityGoogleEx"), TEXT("LoginDomains"), LoginDomains, GEngineIni);

	LoginURLDetails.ClientId = InSubsystem->GetAppId();

	GConfig->GetArray(TEXT("OnlineSubsystemGoogleEx.OnlineIdentityGoogleEx"), TEXT("ScopeFields"), LoginURLDetails.ScopeFields, GEngineIni);
	// profile 권한은 상시 필요
	LoginURLDetails.ScopeFields.AddUnique(TEXT(GOOGLE_EX_PERM_PUBLIC_PROFILE));

#if UE_BUILD_SHIPPING == 0
	GConfig->GetString(TEXT("OnlineSubsystemGoogleEx.OnlineIdentityGoogleEx"), TEXT("AccessType"), LoginURLDetails.AccessType, GEngineIni);
#endif
}

bool FOnlineIdentityGoogleEx::Login(int32 LocalUserNum, const FOnlineAccountCredentials& AccountCredentials)
{
	FString ErrorStr;

	if (bHasLoginOutstanding)
	{
		ErrorStr = FString::Printf(TEXT("Registration already pending for user"));
	}
	else if (!LoginURLDetails.IsValid())
	{
		ErrorStr = FString::Printf(TEXT("OnlineSubsystemGoogle is improperly configured in DefaultEngine.ini LoginRedirectUrl=%s RedirectPort=%d ClientId=%s"),
			*LoginURLDetails.LoginRedirectUrl, LoginURLDetails.RedirectPort, *LoginURLDetails.ClientId);
	}
	else
	{
		if (LocalUserNum < 0 || LocalUserNum >= MAX_LOCAL_PLAYERS)
		{
			ErrorStr = FString::Printf(TEXT("Invalid LocalUserNum=%d"), LocalUserNum);
		}
		else
		{
			PendingLoginRequestCb PendingLoginFn = [this, LocalUserNum](bool bWasSuccessful)
				{
					if (bWasSuccessful)
					{
						/* exchange 토큰을 얻기위해 로그인 외부 UI 띄우기 */

						LoginURLDetails.LoginUrl = Endpoints.AuthEndpoint;
						IOnlineExternalUIPtr OnlineExternalUI = GoogleExSubsystem->GetExternalUIInterface();
						if (ensure(OnlineExternalUI.IsValid()))
						{
							LoginURLDetails.GenerateNonce();
							FOnLoginUIClosedDelegate CompletionDelegate = FOnLoginUIClosedDelegate::CreateRaw(this, &FOnlineIdentityGoogleEx::OnExternalUILoginComplete);
							OnlineExternalUI->ShowLoginUI(LocalUserNum, true, false, CompletionDelegate);
						}
					}
					else
					{
						const FString ErrorStr = TEXT("Error retrieving discovery service");
						OnLoginAttemptComplete(FUniqueNetIdGoogleEx::EmptyId(), LocalUserNum, ErrorStr);
					}
				};

			/* 우선 endpoints 탐색 */

			bHasLoginOutstanding = true;
			RetrieveDiscoveryDocument(MoveTemp(PendingLoginFn));
		}
	}

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG_ONLINE_IDENTITY(Error, TEXT("FOnlineIdentityGoogleEx::Login() failed: %s"), *ErrorStr);
		OnLoginAttemptComplete(FUniqueNetIdGoogleEx::EmptyId(), LocalUserNum, ErrorStr);
		return false;
	}
	return true;
}

bool FOnlineIdentityGoogleEx::Logout(int32 LocalUserNum)
{
	GoogleExSubsystem->ExecuteNextTick([this, LocalUserNum]()
		{
			TriggerOnLogoutCompleteDelegates(LocalUserNum, false);
		});

	return true;
}

bool FOnlineIdentityGoogleEx::AutoLogin(int32 LocalUserNum)
{
	return false;
}

TSharedPtr<FUserOnlineAccount> FOnlineIdentityGoogleEx::GetUserAccount(const FUniqueNetId& UserId) const
{
	return TSharedPtr<FUserOnlineAccount>();
}

TArray<TSharedPtr<FUserOnlineAccount> > FOnlineIdentityGoogleEx::GetAllUserAccounts() const
{
	return TArray<TSharedPtr<FUserOnlineAccount>>();
}

FUniqueNetIdPtr FOnlineIdentityGoogleEx::GetUniquePlayerId(int32 LocalUserNum) const
{
	return nullptr;
}

FUniqueNetIdPtr FOnlineIdentityGoogleEx::CreateUniquePlayerId(uint8* Bytes, int32 Size)
{
	return nullptr;
}

FUniqueNetIdPtr FOnlineIdentityGoogleEx::CreateUniquePlayerId(const FString& Str)
{
	return FUniqueNetIdPtr();
}

ELoginStatus::Type FOnlineIdentityGoogleEx::GetLoginStatus(int32 LocalUserNum) const
{
	return ELoginStatus::NotLoggedIn;
}

ELoginStatus::Type FOnlineIdentityGoogleEx::GetLoginStatus(const FUniqueNetId& UserId) const
{
	return ELoginStatus::NotLoggedIn;
}

FString FOnlineIdentityGoogleEx::GetPlayerNickname(int32 LocalUserNum) const
{
	return TEXT("");
}

FString FOnlineIdentityGoogleEx::GetPlayerNickname(const FUniqueNetId& UserId) const
{
	return TEXT("");
}

FString FOnlineIdentityGoogleEx::GetAuthToken(int32 LocalUserNum) const
{
	return FString();
}

void FOnlineIdentityGoogleEx::GetUserPrivilege(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, const FOnGetUserPrivilegeCompleteDelegate& Delegate)
{
	Delegate.ExecuteIfBound(UserId, Privilege, (uint32)EPrivilegeResults::NoFailures);
}

FPlatformUserId FOnlineIdentityGoogleEx::GetPlatformUserIdFromUniqueNetId(const FUniqueNetId& UniqueNetId) const
{
	return PLATFORMUSERID_NONE;
}

FString FOnlineIdentityGoogleEx::GetAuthType() const
{
	return AUTH_TYPE_GOOGLE_EX;
}

void FOnlineIdentityGoogleEx::RevokeAuthToken(const FUniqueNetId& UserId, const FOnRevokeAuthTokenCompleteDelegate& Delegate)
{
	UE_LOG_ONLINE_IDENTITY(Display, TEXT("FOnlineIdentityGoogleEx::RevokeAuthToken not implemented"));
	FUniqueNetIdRef UserIdRef(UserId.AsShared());
	GoogleExSubsystem->ExecuteNextTick([UserIdRef, Delegate]()
		{
			Delegate.ExecuteIfBound(*UserIdRef, FOnlineError(FString(TEXT("RevokeAuthToken not implemented"))));
		});
}

void FOnlineIdentityGoogleEx::RetrieveDiscoveryDocument(PendingLoginRequestCb&& LoginCb)
{
	if (!Endpoints.IsValid())
	{
		/* end points를 얻기 위한 Http 요청 시작 */

		static const FString DiscoveryURL = TEXT("https://accounts.google.com/.well-known/openid-configuration");
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

		HttpRequest->OnProcessRequestComplete().BindRaw(this, &FOnlineIdentityGoogleEx::DiscoveryRequest_HttpRequestComplete, LoginCb);
		HttpRequest->SetURL(DiscoveryURL);
		HttpRequest->SetVerb(TEXT("GET"));
		HttpRequest->ProcessRequest();
	}
	else
	{
		LoginCb(true);
	}
}

void FOnlineIdentityGoogleEx::DiscoveryRequest_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, PendingLoginRequestCb LoginCb)
{
	if (bSucceeded &&
		HttpResponse.IsValid())
	{
		FString ResponseStr = HttpResponse->GetContentAsString();
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			UE_LOG_ONLINE_IDENTITY(Verbose, TEXT("Discovery request complete. url=%s code=%d response=%s"),
				*HttpRequest->GetURL(), HttpResponse->GetResponseCode(), *ResponseStr);
			if (!Endpoints.Parse(ResponseStr))
			{
				UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Failed to parse Google discovery endpoint"));
			}
		}
		else
		{
			UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Bad response from Google discovery endpoint"));
		}
	}
	else
	{
		UE_LOG_ONLINE_IDENTITY(Warning, TEXT("Google discovery endpoint failure"));
	}

	LoginCb(Endpoints.IsValid());

	if (Endpoints.IsValid())
	{
		LoginURLDetails.LoginUrl = Endpoints.AuthEndpoint;
	}
}

void FOnlineIdentityGoogleEx::Login(int32 LocalUserNum, const FAuthTokenGoogleEx& InToken, const FOnLoginCompleteDelegate& InCompletionDelegate)
{
	if (InToken.AuthType == EGoogleExAuthTokenType::ExchangeToken)
	{
		ExchangeCode(LocalUserNum, InToken, InCompletionDelegate);
	}
	else
	{
		InCompletionDelegate.ExecuteIfBound(LocalUserNum, false, *FUniqueNetIdGoogleEx::EmptyId(), TEXT("Non supported token type"));
	}
}

void FOnlineIdentityGoogleEx::ExchangeCode(int32 LocalUserNum, const FAuthTokenGoogleEx& InExchangeToken, const FOnLoginCompleteDelegate& InCompletionDelegate)
{
	FString ErrorStr;
	bool bStarted = false;
	if (LocalUserNum >= 0 && LocalUserNum < MAX_LOCAL_PLAYERS)
	{
		if (Endpoints.IsValid() && !Endpoints.TokenEndpoint.IsEmpty())
		{
			if (InExchangeToken.IsValid())
			{
				check(InExchangeToken.AuthType == EGoogleExAuthTokenType::ExchangeToken);
				bStarted = true;
				InCompletionDelegate.ExecuteIfBound(LocalUserNum, true, *FUniqueNetIdGoogleEx::Create(InExchangeToken.AccessToken), ErrorStr);
			}
			else
			{
				ErrorStr = TEXT("No access token specified");
			}
		}
		else
		{
			ErrorStr = TEXT("Invalid Google endpoint");
		}
	}
	else
	{
		ErrorStr = TEXT("Invalid local user num");
	}

	if (!bStarted)
	{
		InCompletionDelegate.ExecuteIfBound(LocalUserNum, false, *FUniqueNetIdGoogleEx::EmptyId(), ErrorStr);
	}
}

void FOnlineIdentityGoogleEx::OnLoginAttemptComplete(FUniqueNetIdPtr NetId, int32 LocalUserNum, const FString& ErrorStr)
{
	const FString ErrorStrCopy(ErrorStr);

	bHasLoginOutstanding = false;
	if (NetId.IsValid() == true && NetId->ToString().IsEmpty() == false)
	{
		UE_LOG_ONLINE_IDENTITY(Display, TEXT("Google login was successful"));

		GoogleExSubsystem->ExecuteNextTick([this, NetId, LocalUserNum, ErrorStrCopy]()
			{
				TriggerOnLoginCompleteDelegates(LocalUserNum, true, *NetId, ErrorStrCopy);
				TriggerOnLoginStatusChangedDelegates(LocalUserNum, ELoginStatus::NotLoggedIn, ELoginStatus::LoggedIn, *NetId);
			});
	}
	else
	{
		GoogleExSubsystem->ExecuteNextTick([this, LocalUserNum, ErrorStrCopy]()
			{
				TriggerOnLoginCompleteDelegates(LocalUserNum, false, *FUniqueNetIdGoogleEx::EmptyId(), ErrorStrCopy);
			});
	}
}

void FOnlineIdentityGoogleEx::OnExternalUILoginComplete(FUniqueNetIdPtr NetId, const int ControllerIndex, const FOnlineError& Error)
{
	const FString& ErrorStr = Error.GetErrorCode();
	OnLoginAttemptComplete(NetId, ControllerIndex, ErrorStr);
}