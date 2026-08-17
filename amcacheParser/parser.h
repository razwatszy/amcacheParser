#pragma once
#include "include.h"

void loadAmcacheHive(const wchar_t* hivePath);

struct regValue {
	std::wstring name;
	DWORD type;
	std::vector<BYTE> data;
};

struct subKeysInfo {
	std::wstring name;
	DWORD subKeyCount;
	std::vector<regValue> values;
	std::vector<subKeysInfo> RootSubKeys;
};

extern subKeysInfo rootInfo;