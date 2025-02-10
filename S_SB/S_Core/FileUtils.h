#pragma once

class FileUtils
{
public:
	static xVector<BYTE>	ReadFile(const WCHAR* path);
	static xWString			Convert(std::string str);
};
