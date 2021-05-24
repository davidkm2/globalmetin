#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include <XORstr.h>
#include <TlHelp32.h>
#include "ProcessNameHelper.h"

inline std::string WinPath()
{
	char szBuffer[MAX_PATH] = { 0 };
	GetWindowsDirectoryA(szBuffer, MAX_PATH);
	return szBuffer;
}
inline std::string ExeNameWithPath()
{
	char szBuffer[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szBuffer, MAX_PATH);
	return szBuffer;
}

inline std::string GetPathFromProcessName(const std::string& stBuffer)
{
	auto szCopyBuffer = stBuffer;
	auto pos = szCopyBuffer.find_last_of("\\/");
	return szCopyBuffer.substr(0, pos);
}
inline std::string GetNameFromPath(const std::string& stBuffer)
{
	auto szCopyBuffer = stBuffer;
	auto iLastSlash = szCopyBuffer.find_last_of("\\/");
	szCopyBuffer = szCopyBuffer.substr(iLastSlash + 1, szCopyBuffer.length() - iLastSlash);
	return szCopyBuffer;
}

inline std::string ExePath()
{
	auto szBuffer = ExeNameWithPath();
	return GetPathFromProcessName(szBuffer);
}

inline DWORD GetProcessParentProcessId(DWORD dwMainProcessId)
{
	auto hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (!IS_VALID_HANDLE(hSnap))
	{
		Tracenf("CreateToolhelp32Snapshot fail! Error: %u", GetLastError());
		return 0;
	}

	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnap, &pe)) 
	{
		do {
			if (pe.th32ProcessID == dwMainProcessId) 
			{
				CloseHandle(hSnap);
				return pe.th32ParentProcessID;
			}
		} while (Process32Next(hSnap, &pe));
	}

	CloseHandle(hSnap);
	return 0;
}

inline bool ProcessIsItAlive(DWORD dwProcessId)
{
	auto hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (!IS_VALID_HANDLE(hSnap))
	{
		Tracenf("CreateToolhelp32Snapshot fail! Error: %u", GetLastError());
		return 0;
	}

	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnap, &pe))
	{
		do {
			if (pe.th32ProcessID == dwProcessId)
			{
				CloseHandle(hSnap);
				return true;
			}
		} while (Process32Next(hSnap, &pe));
	}

	CloseHandle(hSnap);
	return false;
}

inline bool GetExplorerPIDbyProgman(LPDWORD pdwExplorerPID)
{
	auto hWnd = FindWindowExA(NULL, NULL, "Progman", NULL);
	if (!hWnd)
		return false;

	auto dwPID = 0UL;
	auto dwTID = GetWindowThreadProcessId(hWnd, &dwPID);
	if (!dwTID || !dwPID)
		return false;

	*pdwExplorerPID = dwPID;
	return true;
}

inline bool GetExplorerPIDbyShellWindow(LPDWORD pdwExplorerPID)
{
	auto hWnd = GetShellWindow();
	if (!hWnd)
		return false;

	auto dwPID = 0UL;
	auto dwTID = GetWindowThreadProcessId(hWnd, &dwPID);
	if (!dwTID || !dwPID)
		return false;

	*pdwExplorerPID = dwPID;
	return true;
}


void CAnticheatManager::ParentProcessCheck(const std::string& stPatcherName)
{
	std::string szLowerPatcherName = stPatcherName;
	std::transform(szLowerPatcherName.begin(), szLowerPatcherName.end(), szLowerPatcherName.begin(), tolower);

	// Process informations
	STARTUPINFOA si	= { 0 };
	si.cb			= sizeof(si);;
	GetStartupInfoA(&si);

	auto dwCurrentPID		= GetCurrentProcessId();
	auto dwParentPID		= GetProcessParentProcessId(dwCurrentPID);

	auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwParentPID);
	if (!IS_VALID_HANDLE(hProcess)) 
	{
		Tracenf("Parent process: %u can not open! Error: %u", dwParentPID, GetLastError());
		
		if (ProcessIsItAlive(dwParentPID))
			ExitByAnticheat(PARENT_CHECK_ACCESS_FAIL, GetLastError(), 0, true);
		return;
	}

	auto stParentName = GetProcessName(hProcess);
	if (stParentName.empty())
	{
		Tracenf("Parent process name not found!");

		CloseHandle(hProcess);
		return;
	}
	std::transform(stParentName.begin(), stParentName.end(), stParentName.begin(), tolower);

	auto stParentProcessPath	= GetPathFromProcessName(stParentName);
	auto stParentProcessName	= GetNameFromPath(stParentName);
	CloseHandle(hProcess);

	Tracenf("Parent process: %s(%u)", stParentName.c_str(), dwParentPID);

	// Windows informations
	auto stWindowsPath = WinPath();
	if (stWindowsPath.empty())
	{
		Tracenf("Windows path not found!");
		return;
	}
	std::transform(stWindowsPath.begin(), stWindowsPath.end(), stWindowsPath.begin(), tolower);

	auto dwExplorerPID1 = 0UL;
	if (!GetExplorerPIDbyShellWindow(&dwExplorerPID1))
	{
		Tracenf("Explorer.exe pid not found by shell! Last Error: %u", GetLastError());
		return;
	}
	Tracenf("Shell/Explorer pid: %u", dwExplorerPID1);

	auto dwExplorerPID2 = 0UL;
	if (!GetExplorerPIDbyShellWindow(&dwExplorerPID2))
	{
		Tracenf("Explorer.exe pid not found by progman! Last Error: %u", GetLastError());
		return;
	}
	Tracenf("Progman/Explorer pid: %u", dwExplorerPID2);

	if (dwExplorerPID1 != dwExplorerPID2)
		ExitByAnticheat(PARENT_CHECK_UNKNOWN_EXPLORER, dwExplorerPID1, dwExplorerPID2, true);

	// Have not patcher, so executed from directly
	if (szLowerPatcherName.empty())
	{
		if (!strcmp(stParentProcessName.c_str(), XOR("devenv.exe")) || !strcmp(stParentProcessName.c_str(), XOR("vsdbg.exe")))
			return;

		if (stParentProcessPath != stWindowsPath ||
			strcmp(stParentProcessName.c_str(), XOR("explorer.exe")) ||
			dwExplorerPID1 != dwParentPID)
		{
			ExitByAnticheat(PARENT_CHECK_UNKNOWN_PARENT, 0, 0, true, stParentProcessName);
		}

		// check startup params
		if (si.dwX != 0 || si.dwY != 0 ||
			si.dwXCountChars != 0 || si.dwYCountChars != 0 ||
			si.dwFillAttribute != 0 ||
			si.dwXSize != 0 || si.dwYSize != 0)
		{
			Tracenf("Startup info: %u-%u %u-%u %u %u-%u", si.dwX, si.dwY, si.dwXCountChars, si.dwYCountChars, si.dwFillAttribute, si.dwXSize, si.dwYSize);
			ExitByAnticheat(PARENT_CHECK_STARTUP_VIOLATION, 0, 0, true, stParentProcessName);
		}
	}
	else // Executed from a patcher
	{
		auto stExePath = ExePath();
		std::transform(stExePath.begin(), stExePath.end(), stExePath.begin(), tolower);

		Tracenf("Exe path: %s Parent path: %s Patcher name: %s Parent name: %s",
			stExePath.c_str(), stParentProcessPath.c_str(), stPatcherName.c_str(), stParentProcessName.c_str());

		if (stExePath != stParentProcessPath || stPatcherName != stParentProcessName)
			ExitByAnticheat(PARENT_CHECK_UNKNOWN_PARENT, 0, 0, true, stParentProcessName);
	}
}
