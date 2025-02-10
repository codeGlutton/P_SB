#pragma once

#include "CoreMinimal.h"
#include <string>

class Utils
{
public:
	/* 유니코드 변환 */

	static void UTF16To8(const FString& Source, std::string& Target)
	{
		Target = StringCast<ANSICHAR>(StringCast<UTF8CHAR>(*Source).Get()).Get();
	}

	static void UTF16To8(const FString& Source, FString& Target)
	{
		Target = StringCast<UTF8CHAR>(*Source).Get();
	}

	static void UTF8To16(const std::string& Source, FString& Target)
	{
		Target = StringCast<TCHAR>(StringCast<UTF8CHAR>(Source.c_str()).Get()).Get();
	}

	static void UTF8To16(const FString& Source, FString& Target)
	{
		Target = StringCast<TCHAR>(StringCast<UTF8CHAR>(*Source).Get()).Get();
	}

	/* 타임 스탬프 */

	static uint64						MakeTimeStamp();

	static UGameInstance* const			GetGameInstance();
};
