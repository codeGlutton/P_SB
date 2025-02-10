#pragma once
#include <random>

class Utils
{
public:
	template<typename T>
	static T GetRandom(T min, T max)
	{
		std::random_device randomDevice;
		std::mt19937 generator(randomDevice());

		if constexpr (std::is_integral_v<T>)
		{
			std::uniform_int_distribution<T> distribution(min, max);
			return distribution(generator);
		}
		else
		{
			std::uniform_real_distribution<T> distribution(min, max);
			return distribution(generator);
		}
	}

	static void UTF16To8(const WCHAR* source, char* target)
	{
		int32 sourceLen = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, source, -1, target, sourceLen, NULL, NULL);
	}

	static void UTF8To16(const char* source, WCHAR* target)
	{
		int32 sourceLen = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, NULL);
		MultiByteToWideChar(CP_UTF8, 0, source, -1, target, sourceLen);
	}
};
