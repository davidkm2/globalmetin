#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"

LONG WINAPI vehFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
	if (ExceptionInfo && ExceptionInfo->ExceptionRecord)
	{
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(VEH_SINGLE_STEP_EXCEPTION, GetLastError(), "");
			CAnticheatManager::Instance().ExitByAnticheat(VEH_SINGLE_STEP_EXCEPTION, GetLastError());
		}

		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(VEH_BREAKPOINT_EXCEPTION, GetLastError(), "");
			CAnticheatManager::Instance().ExitByAnticheat(VEH_BREAKPOINT_EXCEPTION, GetLastError());
		}

		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_GUARD_PAGE)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(VEH_PAGEGUARD_EXCEPTION, GetLastError(), "");
			CAnticheatManager::Instance().ExitByAnticheat(VEH_PAGEGUARD_EXCEPTION, GetLastError());
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI sehFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
	if (ExceptionInfo && ExceptionInfo->ExceptionRecord)
	{
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION)
		{
#ifndef _M_X64
			ExceptionInfo->ContextRecord->Eip++;
#else
			ExceptionInfo->ContextRecord->Rip++;
#endif
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(SEH_SINGLE_STEP_EXCEPTION, GetLastError(), "");
			CAnticheatManager::Instance().ExitByAnticheat(SEH_SINGLE_STEP_EXCEPTION, GetLastError());
		}

		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(SEH_BREAKPOINT_EXCEPTION, GetLastError(), "");
			CAnticheatManager::Instance().ExitByAnticheat(SEH_BREAKPOINT_EXCEPTION, GetLastError());
		}

		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_GUARD_PAGE)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(SEH_PAGEGUARD_EXCEPTION, GetLastError(), "");
			CAnticheatManager::Instance().ExitByAnticheat(SEH_PAGEGUARD_EXCEPTION, GetLastError());
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

__forceinline bool InitializeSEH()
{
	return (SetUnhandledExceptionFilter(sehFilter) != nullptr);
}

__forceinline bool InitializeVEH()
{
	return (AddVectoredExceptionHandler(0, vehFilter) != nullptr);
}

void CAnticheatManager::InitializeExceptionFilters()
{
#ifdef _DEBUG
	Tracen("Exception handlers initilization skipped on debug build!");
	return;
#endif

#if 0
	if (!InitializeSEH())
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(SEH_FILTER_INIT_FAIL, GetLastError(), "");
	}
#endif

	if (!InitializeVEH())
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(VEH_FILTER_INIT_FAIL, GetLastError(), "");
	}
}

void CAnticheatManager::RemoveExceptionHandlers()
{
	RemoveVectoredExceptionHandler(vehFilter);
}
