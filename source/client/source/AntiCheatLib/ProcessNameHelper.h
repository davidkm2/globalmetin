#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <string>
#include <algorithm>

#include <XORstr.h>

static std::string DosDevicePath2LogicalPath(LPCSTR lpszDosPath)
{
	std::string strResult;
	char szTemp[MAX_PATH];
	szTemp[0] = '\0';

	if (lpszDosPath == nullptr || strlen(lpszDosPath) == 0 || !GetLogicalDriveStringsA(_countof(szTemp) - 1, szTemp))
		return strResult;

	char szName[MAX_PATH];
	char szDrive[3] = " :";
	BOOL bFound = FALSE;
	char* p = szTemp;

	do {
		// Copy the drive letter to the template string
		*szDrive = *p;

		// Look up each device name
		if (QueryDosDeviceA(szDrive, szName, _countof(szName)))
		{
			uint32_t uNameLen = (uint32_t)strlen(szName);

			if (uNameLen < MAX_PATH)
			{
				bFound = strncmp(lpszDosPath, szName, uNameLen) == 0;

				if (bFound) {
					// Reconstruct pszFilename using szTemp
					// Replace device path with DOS path
					char szTempFile[MAX_PATH];
					sprintf_s(szTempFile, XOR("%s%s"), szDrive, lpszDosPath + uNameLen);
					strResult = szTempFile;
				}
			}
		}

		// Go to the next nullptr character.
		while (*p++);
	} while (!bFound && *p); // end of string

	return strResult;
}

static std::string GetProcessFullName(HANDLE hProcess)
{
	CHAR processPath[MAX_PATH] = { 0 };
	if (GetProcessImageFileNameA(hProcess, processPath, _countof(processPath)))
		return processPath;

	memset(&processPath, 0, MAX_PATH);
	if (GetModuleFileNameExA(hProcess, nullptr, processPath, _countof(processPath)))
		return processPath;

	return std::string("");
}

static std::string GetProcessName(HANDLE hProcess)
{
	auto szDosName = GetProcessFullName(hProcess);
	if (szDosName.empty()) return std::string("");

	auto szProcessName = DosDevicePath2LogicalPath(szDosName.c_str());
	if (szProcessName.empty()) return std::string("");

	std::transform(szProcessName.begin(), szProcessName.end(), szProcessName.begin(), tolower);
	return szProcessName;
}
