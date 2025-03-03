#include "OnlineExternalUIGoogleEx.h"
#include "OnlineSubsystemGoogleEx.h"
#include "OnlineIdentityGoogleEx.h"
#include "OnlineError.h"

#define GOOGLE_EX_STATE_TOKEN TEXT("state")
#define GOOGLE_EX_ACCESS_TOKEN TEXT("code")
#define GOOGLE_EX_ERRORCODE_TOKEN TEXT("error")
#define GOOGLE_EX_ERRORCODE_DENY TEXT("access_denied")

bool FOnlineExternalUIGoogleEx::ShowLoginUI(const int ControllerIndex, bool bShowOnlineOnly, bool bShowSkipButton, const FOnLoginUIClosedDelegate& Delegate)
{
	bool bStarted = false;
	FString ErrorStr;
	if (ControllerIndex >= 0 && ControllerIndex < MAX_LOCAL_PLAYERS)
	{
		FOnlineIdentityGoogleExPtr IdentityInt = StaticCastSharedPtr<FOnlineIdentityGoogleEx>(GoogleExSubsystem->GetIdentityInterface());
		if (IdentityInt.IsValid())
		{
			const FGoogleLoginURL& URLDetails = IdentityInt->GetLoginURLDetails();
			if (URLDetails.IsValid())
			{
				/* 대리자 등록과 구글 로그인 Widget 실행 */

				const FString RequestedURL = URLDetails.GetURL();
				bool bShouldContinueLoginFlow = false;
				FOnLoginRedirectURL OnRedirectURLDelegate = FOnLoginRedirectURL::CreateRaw(this, &FOnlineExternalUIGoogleEx::OnLoginRedirectURL);
				FOnLoginFlowComplete OnExternalLoginFlowCompleteDelegate = FOnLoginFlowComplete::CreateRaw(this, &FOnlineExternalUIGoogleEx::OnExternalLoginFlowComplete, ControllerIndex, Delegate);
				TriggerOnLoginFlowUIRequiredDelegates(RequestedURL, OnRedirectURLDelegate, OnExternalLoginFlowCompleteDelegate, bShouldContinueLoginFlow);
				bStarted = bShouldContinueLoginFlow;
			}
			else
			{
				ErrorStr = TEXT("ShowLoginUI: Url Details not properly configured");
			}
		}
		else
		{
			ErrorStr = TEXT("ShowLoginUI: Missing identity interface");
		}
	}
	else
	{
		ErrorStr = FString::Printf(TEXT("ShowLoginUI: Invalid controller index (%d)"), ControllerIndex);
	}

	if (!bStarted)
	{
		UE_LOG_ONLINE_EXTERNALUI(Warning, TEXT("%s"), *ErrorStr);

		FOnlineError Error;
		Error.SetFromErrorCode(MoveTemp(ErrorStr));

		GoogleExSubsystem->ExecuteNextTick([ControllerIndex, Delegate, Error = MoveTemp(Error)]()
			{
				Delegate.ExecuteIfBound(nullptr, ControllerIndex, FOnlineError(EOnlineErrorResult::Unknown));
			});
	}

	return bStarted;
}

bool FOnlineExternalUIGoogleEx::ShowFriendsUI(int32 LocalUserNum)
{
	return false;
}

bool FOnlineExternalUIGoogleEx::ShowInviteUI(int32 LocalUserNum, FName SessionName)
{
	return false;
}

bool FOnlineExternalUIGoogleEx::ShowAchievementsUI(int32 LocalUserNum)
{
	return false;
}

bool FOnlineExternalUIGoogleEx::ShowLeaderboardUI(const FString& LeaderboardName)
{
	return false;
}

bool FOnlineExternalUIGoogleEx::ShowWebURL(const FString& Url, const FShowWebUrlParams& ShowParams, const FOnShowWebUrlClosedDelegate& Delegate)
{
	return false;
}

bool FOnlineExternalUIGoogleEx::CloseWebURL()
{
	return false;
}

bool FOnlineExternalUIGoogleEx::ShowProfileUI(const FUniqueNetId& Requestor, const FUniqueNetId& Requestee, const FOnProfileUIClosedDelegate& Delegate)
{
	return false;
}

bool FOnlineExternalUIGoogleEx::ShowAccountUpgradeUI(const FUniqueNetId& UniqueId)
{
	return false;
}

bool FOnlineExternalUIGoogleEx::ShowStoreUI(int32 LocalUserNum, const FShowStoreParams& ShowParams, const FOnShowStoreUIClosedDelegate& Delegate)
{
	return false;
}

bool FOnlineExternalUIGoogleEx::ShowSendMessageUI(int32 LocalUserNum, const FShowSendMessageParams& ShowParams, const FOnShowSendMessageUIClosedDelegate& Delegate)
{
	return false;
}

FLoginFlowResult FOnlineExternalUIGoogleEx::OnLoginRedirectURL(const FString& RedirectURL)
{
	FLoginFlowResult Result;
	FOnlineIdentityGoogleExPtr IdentityInt = StaticCastSharedPtr<FOnlineIdentityGoogleEx>(GoogleExSubsystem->GetIdentityInterface());
	if (IdentityInt.IsValid())
	{
		const FGoogleLoginURL& URLDetails = IdentityInt->GetLoginURLDetails();
		if (URLDetails.IsValid())
		{
			// 구글 로그인시 거쳐가는 리다이렉션이 모두 완료되어 RedirectURL이 나타날 때 정지
			if (!RedirectURL.Contains(FPlatformHttp::UrlEncode(URLDetails.LoginUrl)) && RedirectURL.StartsWith(URLDetails.LoginRedirectUrl))
			{
				TMap<FString, FString> ParamsMap;

				{
					FString URLPrefix;
					FString ParamsOnly;
					if (!RedirectURL.Split(TEXT("?"), &URLPrefix, &ParamsOnly))
					{
						ParamsOnly = RedirectURL;
					}

					if (ParamsOnly[ParamsOnly.Len() - 1] == TEXT('#'))
					{
						ParamsOnly[ParamsOnly.Len() - 1] = TEXT('\0');
					}

					TArray<FString> Params;
					ParamsOnly.ParseIntoArray(Params, TEXT("&"));
					for (FString& Param : Params)
					{
						FString Key, Value;
						if (Param.Split(TEXT("="), &Key, &Value))
						{
							ParamsMap.Add(Key, Value);
						}
					}
				}

				/* 구글 로그인 repsonse 분석 및 저장 */

				const FString* State = ParamsMap.Find(GOOGLE_EX_STATE_TOKEN);
				if (State)
				{
					if (URLDetails.State == *State)
					{
						const FString* AccessToken = ParamsMap.Find(GOOGLE_EX_ACCESS_TOKEN);
						if (AccessToken)
						{
							Result.Error.bSucceeded = true;
							Result.Token = *AccessToken;
						}
						else
						{
							const FString* ErrorCode = ParamsMap.Find(GOOGLE_EX_ERRORCODE_TOKEN);
							if (ErrorCode)
							{
								if (*ErrorCode == GOOGLE_EX_ERRORCODE_DENY)
								{
									Result.Error.ErrorRaw = LOGIN_CANCELLED;
									Result.Error.ErrorMessage = FText::FromString(LOGIN_CANCELLED);
									Result.Error.ErrorCode = LOGIN_CANCELLED;
									Result.Error.ErrorMessage = NSLOCTEXT("GoogleAuth", "GoogleAuthDeny", "Google Auth Denied");
									Result.NumericErrorCode = -1;
								}
								else
								{
									Result.Error.ErrorRaw = RedirectURL;
									Result.Error.ErrorCode = *ErrorCode;
									// there is no descriptive error text
									Result.Error.ErrorMessage = NSLOCTEXT("GoogleAuth", "GoogleAuthError", "Google Auth Error");
									// there is no error code
									Result.NumericErrorCode = 0;
								}
							}
							else
							{
								// Set some default in case parsing fails
								Result.Error.ErrorRaw = LOGIN_ERROR_UNKNOWN;
								Result.Error.ErrorMessage = FText::FromString(LOGIN_ERROR_UNKNOWN);
								Result.Error.ErrorCode = LOGIN_ERROR_UNKNOWN;
								Result.NumericErrorCode = -2;
							}
						}
					}
				}
			}
		}
	}

	return Result;
}

void FOnlineExternalUIGoogleEx::OnExternalLoginFlowComplete(const FLoginFlowResult& Result, int ControllerIndex, const FOnLoginUIClosedDelegate Delegate)
{
	UE_LOG_ONLINE_EXTERNALUI(Log, TEXT("OnExternalLoginFlowComplete %s"), *Result.ToDebugString());

	bool bStarted = false;
	if (Result.IsValid())
	{
		FOnlineIdentityGoogleExPtr IdentityInt = StaticCastSharedPtr<FOnlineIdentityGoogleEx>(GoogleExSubsystem->GetIdentityInterface());
		if (IdentityInt.IsValid())
		{
			bStarted = true;

			FOnLoginCompleteDelegate CompletionDelegate;
			CompletionDelegate = FOnLoginCompleteDelegate::CreateRaw(this, &FOnlineExternalUIGoogleEx::OnAccessTokenLoginComplete, Delegate);

			FAuthTokenGoogleEx AuthToken(Result.Token, EGoogleExExchangeToken::GoogleExchangeToken);
			IdentityInt->Login(ControllerIndex, AuthToken, CompletionDelegate);
		}
	}

	if (!bStarted)
	{
		FOnlineError LoginFlowError = Result.Error;
		GoogleExSubsystem->ExecuteNextTick([ControllerIndex, LoginFlowError, Delegate]()
			{
				Delegate.ExecuteIfBound(nullptr, ControllerIndex, LoginFlowError);
			});
	}
}

void FOnlineExternalUIGoogleEx::OnAccessTokenLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error, FOnLoginUIClosedDelegate Delegate)
{
	FUniqueNetIdPtr StrongUserId = UserId.AsShared();
	GoogleExSubsystem->ExecuteNextTick([StrongUserId, LocalUserNum, bWasSuccessful, Delegate]()
		{
			Delegate.ExecuteIfBound(StrongUserId, LocalUserNum, FOnlineError(bWasSuccessful));
		});
}
