#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "ThreadEnumerator.h"
#include "../EterBase/SimpleTimer.h"

static auto gs_pSectionChecksumTimer = std::make_unique <CSimpleTimer <std::chrono::milliseconds> >();

VOID CALLBACK AnticheatWatchdogRoutine(HWND hwnd, UINT message, UINT_PTR iCurrTimerID, DWORD dwTime)
{
	if (CAnticheatManager::Instance().GetAnticheatWatchdogTimerID() != iCurrTimerID)
		return;

	// ...

	CAnticheatManager::Instance().SendWatchdogSignal(true);

	static bool bMemDumpStatus = false;
	if (bMemDumpStatus == false && CAnticheatManager::Instance().IsMemoryDumped())
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(CHEAT_MEMORY_DUMP, GetLastError(), "");
		bMemDumpStatus = true;
	}

	static bool bMemWatchdogStatus = false;
	if (bMemWatchdogStatus == false && CAnticheatManager::Instance().IsMemoryWatchdogsTriggered())
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(CHEAT_MEMORY_WATCHDOG, GetLastError(), "");
		bMemWatchdogStatus = true;
	}

	static bool bDllFilterStatus = false;
	if (bDllFilterStatus == false && CAnticheatManager::Instance().IsDllFilterHookCorrupted())
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(DLL_FILTER_CORRUPTED, GetLastError(), "");
		CAnticheatManager::Instance().ExitByAnticheat(DLL_FILTER_CORRUPTED, 0, 0, true);
		bDllFilterStatus = true;
	}

#if !defined(_DEBUG) && !defined(ENABLE_DEBUGGER_ALLOW_BY_ANTICHEAT)
	static bool bRuntimeDebuggerStatus = false;
	if (bRuntimeDebuggerStatus == false)
	{
		if (CAnticheatManager::Instance().DebugObjectHandleCheck() || CAnticheatManager::Instance().DebugCheckBugControl())
		{
			bRuntimeDebuggerStatus = true;
		}
	}
#endif

	static bool bSectionViolationStatus = false;
	if (bSectionViolationStatus == false && gs_pSectionChecksumTimer->diff() > 40000)
	{
		gs_pSectionChecksumTimer->reset();

		if (CAnticheatManager::Instance().HasCorruptedModuleSection())
		{
			bSectionViolationStatus = true;
		}
	}

	static bool bThreadsWaitingStatus = false;
	if (bMemWatchdogStatus == false)
	{
		auto threadEnumerator = std::make_unique<CThreadEnumerator>(GetCurrentProcessId());
		if (threadEnumerator && threadEnumerator.get())
		{
			auto systemThreadOwnerProcInfo = threadEnumerator->GetProcInfo();
			if (systemThreadOwnerProcInfo)
			{
				auto systemThreadInfo = threadEnumerator->FindThread(systemThreadOwnerProcInfo, CAnticheatManager::Instance().GetWindowWatcherThreadId());
				if (systemThreadInfo)
				{
					if (systemThreadInfo->ThreadState == Waiting && systemThreadInfo->WaitReason == Suspended)
					{
						CCheatDetectQueueMgr::Instance().AppendDetection(WINDOW_WATCHER_THREAD_SUSPEND, GetLastError(), "");
						bThreadsWaitingStatus = true;
					}
				}

				systemThreadInfo = threadEnumerator->FindThread(systemThreadOwnerProcInfo, CWMI_Monitor::Instance().GetWMIMonitorThreadId());
				if (systemThreadInfo)
				{
					if (systemThreadInfo->ThreadState == Waiting && systemThreadInfo->WaitReason == Suspended)
					{
						CCheatDetectQueueMgr::Instance().AppendDetection(WMI_MONITOR_THREAD_SUSPEND, GetLastError(), "");
						bThreadsWaitingStatus = true;
					}
				}

				systemThreadInfo = threadEnumerator->FindThread(systemThreadOwnerProcInfo, CAnticheatManager::Instance().GetMainThreadId());
				if (systemThreadInfo)
				{
					if (systemThreadInfo->ThreadState == Waiting && systemThreadInfo->WaitReason == Suspended)
					{
						CCheatDetectQueueMgr::Instance().AppendDetection(MAIN_THREAD_SUSPEND, GetLastError(), "");
						CAnticheatManager::Instance().ExitByAnticheat(MAIN_THREAD_SUSPEND, GetLastError(), 0, true);
						bThreadsWaitingStatus = true;
					}
				}
			}
		}
	}
}

void CAnticheatManager::InitializeAnticheatWatchdog()
{
	gs_pSectionChecksumTimer->reset();

	m_iAnticheatWatchdogTimerID = SetTimer(nullptr, 0, 8000, AnticheatWatchdogRoutine);
	if (!m_iAnticheatWatchdogTimerID)
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(WATCHDOG_CREATE_FAIL, GetLastError(), "");
		return;
	}
}

void CAnticheatManager::RemoveAnticheatWatchdog()
{
	KillTimer(nullptr, m_iAnticheatWatchdogTimerID);
}

