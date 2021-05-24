#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "ProcessNameHelper.h"
#include "CheatQuarentineManager.h"

#include "../EterBase/WinVerHelper.h"

#include <XORstr.h>
#include <psapi.h>
#include <tlhelp32.h>

inline static bool IsValidHandle(HANDLE hTarget)
{
	auto dwInfo = 0UL;
	if (GetHandleInformation(hTarget, &dwInfo) == false)
		return false;
	return true;
}

inline static bool SafeCloseHandle(HANDLE hTarget)
{
	__try
	{
		if (!IsValidHandle(hTarget))
			return false;

		return CloseHandle(hTarget) ? true : false;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
	return true;
}


void CAnticheatManager::AppendProcessHandle(HANDLE hProcess)
{
	m_vProcessHandles.push_back(hProcess);
}

void CAnticheatManager::CloseUselessProcessHandles()
{
	if (m_vProcessHandles.size() <= 0)
		return;

	for (auto i = 0UL; i < m_vProcessHandles.size(); ++i)
		SafeCloseHandle(m_vProcessHandles[i]);
	m_vProcessHandles.clear();
}

void CAnticheatManager::ScanProcess(HANDLE hProcess)
{
	if (!hProcess || hProcess == INVALID_HANDLE_VALUE)
		return;

	auto strProcessName = GetProcessName(hProcess);
	if (strProcessName.empty())
		return;

	if (IsFileExist(strProcessName) == false)
		return;

	auto strMd5 = GetFileMd5(strProcessName);
	if (strMd5.empty())
		return;

	CCheatQuarentineMgr::Instance().AppendProcessMd5(strProcessName, strMd5, false);
}

DWORD WINAPI CreateProcessHashListRoutine(LPVOID)
{
	if (IsWindowsVistaOrGreater())
	{
		typedef NTSTATUS(NTAPI* TNtGetNextProcess)(HANDLE ProcessHandle, ACCESS_MASK DesiredAccess, ULONG HandleAttributes, ULONG Flags, PHANDLE NewProcessHandle);

		auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
		if (!hNtdll)
			return 0;

		auto pNtGetNextProcess = GetProcAddress(hNtdll, XOR("NtGetNextProcess"));
		if (!pNtGetNextProcess)
			return 0;
		auto NtGetNextProcess = (TNtGetNextProcess)pNtGetNextProcess;


		auto hCurrent = HANDLE(nullptr);

		while (NT_SUCCESS(NtGetNextProcess(hCurrent, PROCESS_QUERY_INFORMATION, 0, 0, &hCurrent)))
		{
			CAnticheatManager::Instance().ScanProcess(hCurrent);
			CAnticheatManager::Instance().AppendProcessHandle(hCurrent);
			Sleep(10);
		}

		CAnticheatManager::Instance().CloseUselessProcessHandles();
	}
	else
	{
		auto hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap && hProcessSnap != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 pe32;
			pe32.dwSize = sizeof(PROCESSENTRY32);

			if (Process32First(hProcessSnap, &pe32))
			{
				do
				{
					HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
					if (hProcess && hProcess != INVALID_HANDLE_VALUE)
					{
						CAnticheatManager::Instance().ScanProcess(hProcess);
						Sleep(10);
						CloseHandle(hProcess);
					}
				} while (Process32Next(hProcessSnap, &pe32));
			}
		}
		CloseHandle(hProcessSnap);
	}

	return 0;
}

void CAnticheatManager::CreateProcessHashList()
{
	auto hThread = CreateThread(0, 0, CreateProcessHashListRoutine, 0, 0, 0); // It's moved to new thread because takes int32_t time on client start
	if (hThread && hThread != INVALID_HANDLE_VALUE)
		CloseHandle(hThread);
}

