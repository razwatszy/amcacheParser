#include "parser.h"

DWORD subKeys;
DWORD MaxSubKeyLen;
DWORD MaxClassLen;
DWORD Values;
DWORD MaxValueNameLen;
DWORD MaxValueLen;
HKEY rootKey;
subKeysInfo rootInfo;

static void readValues(HKEY key, subKeysInfo& info) {
	DWORD valueCount = 0;
	DWORD maxValueNameLen = 0;
	DWORD maxValueLen = 0;
	LONG STATUS = RegQueryInfoKeyW(key, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &valueCount, &maxValueNameLen, &maxValueLen, nullptr, nullptr);
	if (STATUS != ERROR_SUCCESS)
		return;

	for (DWORD k = 0; k < valueCount; k++) {
		std::wstring valueName(maxValueNameLen + 1, L'\0');
		DWORD valueNameSize = maxValueNameLen + 1;

		regValue value;
		value.data.resize(maxValueLen);
		DWORD dataSize = maxValueLen;

		STATUS = RegEnumValueW(key, k, &valueName[0], &valueNameSize, nullptr, &value.type, value.data.data(), &dataSize);
		if (STATUS != ERROR_SUCCESS) {
			std::wcerr << L"failed to enumerate value " << k << L": " << STATUS << std::endl;
			continue;
		}

		valueName.resize(valueNameSize);
		value.data.resize(dataSize);
		value.name = valueName;
		info.values.push_back(std::move(value));
	}
}




void loadAmcacheHive(const wchar_t* hivePath) {

	HKEY hKey;
	LONG STATUS = RegLoadAppKeyW(hivePath, &hKey, KEY_READ, 0, 0);

	if (STATUS != ERROR_SUCCESS) {
		std::wcerr << L"failed to load hive " << STATUS << std::endl;
		return;
	}

	
	STATUS = RegOpenKeyExW(hKey, L"Root", 0, KEY_READ, &rootKey);
	if (STATUS != ERROR_SUCCESS) {
	std::wcerr << L"failed to open Root key " << STATUS << std::endl;
		RegCloseKey(hKey);
		return;
	}

	STATUS = RegQueryInfoKeyW(rootKey, nullptr, nullptr, nullptr, &subKeys, &MaxSubKeyLen, &MaxClassLen, &Values, &MaxValueNameLen, &MaxValueLen, nullptr, nullptr);
	if (STATUS != ERROR_SUCCESS) {
		std::wcerr << L"failed to query Root key " << STATUS << std::endl;
		RegCloseKey(hKey);
		RegCloseKey(rootKey);
		return;
	}

	
	rootInfo.name = L"Root";
	rootInfo.subKeyCount = subKeys;
	rootInfo.values.clear();
	rootInfo.RootSubKeys.clear();
	readValues(rootKey, rootInfo);


	for (DWORD i = 0; i < subKeys; i++) {
		wchar_t name[512];
		DWORD namesize = _countof(name);
		STATUS = RegEnumKeyExW(rootKey, i, name, &namesize, nullptr, nullptr, nullptr, nullptr);
		if (STATUS != ERROR_SUCCESS) {
			std::wcerr << L"failed to enumerate subkey " << i << L": " << STATUS << std::endl;
			break;
		}

		std::wcout << L"Subkey " << i << L": " << name << std::endl;

		HKEY currentKey = nullptr;
		STATUS = RegOpenKeyExW(rootKey, name, 0, KEY_READ, &currentKey);
		if (STATUS != ERROR_SUCCESS) {
			std::wcerr << L"failed to open subkey " << name << L": " << STATUS << std::endl;
			continue;
		}

		DWORD currentSubkeys = 0;
		DWORD currentMaxSubKeyLen = 0;
		DWORD currentMaxClassLen = 0;
		DWORD currentValues = 0;
		DWORD currentMaxValueNameLen = 0;
		DWORD currentMaxValueLen = 0;
		STATUS = RegQueryInfoKeyW(currentKey, nullptr, nullptr, nullptr, &currentSubkeys, &currentMaxSubKeyLen, &currentMaxClassLen, &currentValues, &currentMaxValueNameLen, &currentMaxValueLen, nullptr, nullptr);
		if (STATUS != ERROR_SUCCESS) {
			std::wcerr << L"failed to query subkey " << name << L": " << STATUS << std::endl;
			RegCloseKey(currentKey);
			continue;
		}

		std::wcout << L"subkeys in " << name << L": " << currentSubkeys << std::endl;

		subKeysInfo currentInfo;
		currentInfo.name = name;
		currentInfo.subKeyCount = currentSubkeys;
		readValues(currentKey, currentInfo);

		for (DWORD j = 0; j < currentSubkeys; j++) {
			wchar_t subName[512];
			DWORD subNameSize = _countof(subName);
			STATUS = RegEnumKeyExW(currentKey, j, subName, &subNameSize, nullptr, nullptr, nullptr, nullptr);
			if (STATUS != ERROR_SUCCESS) {
				std::wcerr << L"failed to enumerate subkey " << j << L": " << STATUS << std::endl;
				continue;
			}

			std::wcout << L"  Subkey " << j << L": " << subName << std::endl;

			subKeysInfo childInfo;
			childInfo.name = subName;
			childInfo.subKeyCount = 0;

			HKEY subcurrentKey = nullptr;
			STATUS = RegOpenKeyExW(currentKey, subName, 0, KEY_READ, &subcurrentKey);
			if (STATUS != ERROR_SUCCESS) {
				std::wcerr << L"failed to open subkey " << subName << L": " << STATUS << std::endl;
			}
			else {
				readValues(subcurrentKey, childInfo);
				RegCloseKey(subcurrentKey);
			}

			currentInfo.RootSubKeys.push_back(childInfo);
		}

		rootInfo.RootSubKeys.push_back(currentInfo);
		RegCloseKey(currentKey);
	}

	

	RegCloseKey(rootKey);
	RegCloseKey(hKey);
}