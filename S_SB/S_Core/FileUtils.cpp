#include "pch.h"
#include "FileUtils.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

xVector<BYTE> FileUtils::ReadFile(const WCHAR* path)
{
	xVector<BYTE> ret;
	fs::path filePath{ path };

	// 파일 크기 구하기
	const uint32 fileSize = static_cast<uint32>(fs::file_size(filePath));
	ret.resize(fileSize);

	// 모두 읽기
	std::basic_ifstream<BYTE> inputStream{ filePath };
	inputStream.read(&ret[0], fileSize);

	return ret;
}

xWString FileUtils::Convert(std::string str)
{
	const int32 srcLen = static_cast<int32>(str.size());

	xWString ret;
	if (srcLen == 0)
		return ret;

	// UTF-8 에서 UTF-16으로 변환
	const int32 retLen = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, NULL, 0);
	ret.resize(retLen);
	::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, &ret[0], retLen);

	return ret;
}
