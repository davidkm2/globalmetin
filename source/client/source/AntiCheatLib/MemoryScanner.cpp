#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"

#include "../EterBase/WinVerHelper.h"
#include "../EterBase/Random.h"

#include <Psapi.h>

inline LPVOID CreateMemoryPage(uint32_t dwRegionSize, uint32_t dwProtection)
{
	LPVOID pMemBase = nullptr;

	__try
	{
		pMemBase = VirtualAlloc(0, dwRegionSize, MEM_COMMIT | MEM_RESERVE, dwProtection);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }

	return pMemBase;
}

bool CAnticheatManager::InitializeMemoryWatchdogs()
{
	if (!IsWindowsVistaOrGreater())
		return true;

	m_pMemoryWatchdogs.clear();

	// 1
	for (auto i = 0; i <= random_range(1, 15); i++) // Create fake pages pre
	{
		auto pCurrMem = CreateMemoryPage(0x10000, PAGE_READWRITE);
		if (pCurrMem) m_pMemoryDummyPages.push_back(pCurrMem);
	}

	auto pFirstWatchdog = CreateMemoryPage(0x10000, PAGE_READWRITE); // Create watchdog page
	if (!pFirstWatchdog)
	{
#ifdef _DEBUG
		TraceError("First watchdog create fail! Error: %u", GetLastError());
#endif
		return false;
	}
	m_pMemoryWatchdogs.push_back(pFirstWatchdog);

	for (auto i = 0; i <= random_range(1, 15); i++) // Create fake pages post
	{
		auto pCurrMem = CreateMemoryPage(0x10000, PAGE_READWRITE);
		if (pCurrMem) m_pMemoryDummyPages.push_back(pCurrMem);
	}

	// 2
	for (auto i = 0; i <= random_range(1, 15); i++) // Create fake pages pre
	{
		auto pCurrMem = CreateMemoryPage(0x10000, PAGE_READONLY);
		if (pCurrMem) m_pMemoryDummyPages.push_back(pCurrMem);
	}

	auto pSecondWatchdog = CreateMemoryPage(0x10000, PAGE_READONLY); // Create watchdog page
	if (!pSecondWatchdog)
	{
#ifdef _DEBUG
		TraceError("Second watchdog create fail! Error: %u", GetLastError());
#endif
		return false;
	}
	m_pMemoryWatchdogs.push_back(pSecondWatchdog);

	for (auto i = 0; i <= random_range(1, 15); i++) // Create fake pages post
	{
		auto pCurrMem = CreateMemoryPage(0x10000, PAGE_READONLY);
		if (pCurrMem) m_pMemoryDummyPages.push_back(pCurrMem);
	}

	// 3
	for (auto i = 0; i <= random_range(1, 15); i++) // Create fake pages pre
	{
		auto pCurrMem = CreateMemoryPage(0x10000, PAGE_NOACCESS);
		if (pCurrMem) m_pMemoryDummyPages.push_back(pCurrMem);
	}

	auto pThirdWatchdog = CreateMemoryPage(0x10000, PAGE_NOACCESS); // Create watchdog page
	if (!pThirdWatchdog)
	{
#ifdef _DEBUG
		TraceError("Third watchdog create fail! Error: %u", GetLastError());
#endif
		return false;
	}
	m_pMemoryWatchdogs.push_back(pThirdWatchdog);

	for (auto i = 0; i <= random_range(1, 15); i++) // Create fake pages post
	{
		auto pCurrMem = CreateMemoryPage(0x10000, PAGE_NOACCESS);
		if (pCurrMem) m_pMemoryDummyPages.push_back(pCurrMem);
	}

#ifdef _DEBUG
	Tracenf("Memory watchdog pages succesfully created! Watchdog count: %u Dummy page count: %u", m_pMemoryWatchdogs.size(), m_pMemoryDummyPages.size());
#endif
	return true;
}

bool CAnticheatManager::IsMemoryWatchdogsTriggered()
{
	if (m_pMemoryWatchdogs.empty())
		return false;

	if (!IsWindowsVistaOrGreater())
		return false;

	for (const auto & pCurrWatchdog : m_pMemoryWatchdogs)
	{
//		Tracenf("Current watchdog page: %p", pCurrWatchdog);
		
		/*
		if (IsBadCodePtr((FARPROC)pCurrWatchdog))
		{
#ifdef _DEBUG
			TraceError("CheckMemoryWatchdogs: Memory page is corrupted!");
#endif
			return false;
		}
		*/

		MEMORY_BASIC_INFORMATION mbi = { 0 };
		if (!VirtualQuery(pCurrWatchdog, &mbi, sizeof(mbi)))
		{
#ifdef _DEBUG
			TraceError("CheckMemoryWatchdogs: VirtualQuery fail! Error: %u", GetLastError());
#endif
			return false;
		}

		if (mbi.State == MEM_FREE)
		{
#ifdef _DEBUG
			TraceError("CheckMemoryWatchdogs: Memory page is free'd!");
#endif
			return false;
		}

		PSAPI_WORKING_SET_EX_INFORMATION pworkingSetExInformation = { pCurrWatchdog, 0 };
		if (!QueryWorkingSetEx(GetCurrentProcess(), &pworkingSetExInformation, sizeof(pworkingSetExInformation)))
		{
#ifdef _DEBUG
			TraceError("CheckMemoryWatchdogs: QueryWorkingSetEx fail! Error: %u", GetLastError());
#endif
			return false;
		}

		if (pworkingSetExInformation.VirtualAttributes.Valid)
		{
#ifdef _DEBUG
			TraceError("CheckMemoryWatchdogs: Memory scan detected at watchdog: %p", pCurrWatchdog);
#endif
			return false;
		}
	}
	return true;
}
