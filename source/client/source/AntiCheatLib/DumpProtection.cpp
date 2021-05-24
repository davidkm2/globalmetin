#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "../EterBase/WinVerHelper.h"
#include "../EterBase/Random.h"

#include <psapi.h>

#define BUFFER_SIZE 0x1000

inline LPVOID CreateSafeMemoryPage(uint32_t dwRegionSize, uint32_t dwProtection)
{
	LPVOID pMemBase = nullptr;

	__try
	{
		pMemBase = VirtualAlloc(0, dwRegionSize, MEM_COMMIT | MEM_RESERVE, dwProtection);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }

	return pMemBase;
}

void CAnticheatManager::InitializeDumpProtection()
{
	if (!IsWindowsVistaOrGreater())
		return;

	for (auto i = 0; i < random_range(20, 50); i++)
		CreateSafeMemoryPage(BUFFER_SIZE, PAGE_READWRITE);

	m_pDumpGuardMem = CreateSafeMemoryPage(BUFFER_SIZE, PAGE_READWRITE);

	for (auto i = 0; i < random_range(20, 50); i++)
		CreateSafeMemoryPage(BUFFER_SIZE, PAGE_READWRITE);

	auto hTargetModule = m_hInstance;

	auto pDOS = (IMAGE_DOS_HEADER *)hTargetModule;
	if (pDOS->e_magic != IMAGE_DOS_SIGNATURE)
		return;

	auto pINH = (IMAGE_NT_HEADERS *)(pDOS + pDOS->e_lfanew);
	//	if (pINH->Signature != IMAGE_NT_SIGNATURE)
	//		return;

	auto pISH = (PIMAGE_SECTION_HEADER)(pINH + 1);
	if (!pISH)
		return;

	auto dwOldProtect = 0UL;
	VirtualProtect((LPVOID)pISH, sizeof(LPVOID), PAGE_READWRITE, &dwOldProtect);

	pISH[0].VirtualAddress = reinterpret_cast<DWORD_PTR>(m_pDumpGuardMem);

	VirtualProtect((LPVOID)pISH, sizeof(LPVOID), dwOldProtect, &dwOldProtect);

	return;
}

bool CAnticheatManager::IsMemoryDumped()
{
	if (!IsWindowsVistaOrGreater())
		return false;

	if (!m_pDumpGuardMem)
		return false;

	PSAPI_WORKING_SET_EX_INFORMATION pworkingSetExInformation = { m_pDumpGuardMem, 0 };

	if (!QueryWorkingSetEx(GetCurrentProcess(), &pworkingSetExInformation, sizeof(pworkingSetExInformation)))
	{
#ifdef _DEBUG
		TraceError("IsMemoryDumped: QueryWorkingSetEx fail! Error: %u", GetLastError());
#endif
		return false;
	}

	if (pworkingSetExInformation.VirtualAttributes.Valid)
		return true;

	return false;
}

