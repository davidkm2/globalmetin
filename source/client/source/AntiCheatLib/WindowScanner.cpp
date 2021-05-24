#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"

void NTAPI OnWindowWatcherThreadLost(PVOID pParam, BOOLEAN)
{
	CCheatDetectQueueMgr::Instance().AppendDetection(WINDOW_WATCHER_THREAD_LOST, 0, "");
}


inline static void OnWatcherWindowScan(HWND hWnd)
{
	auto dwProcessId = 0UL;
	auto dwThreadId = GetWindowThreadProcessId(hWnd, &dwProcessId);

	if (dwProcessId == GetCurrentProcessId())
	{
		char szFileName[MAX_PATH] = { 0 };
		GetWindowModuleFileNameA(hWnd, szFileName, sizeof(szFileName));

		char szExeName[MAX_PATH] = { 0 };
		GetModuleFileNameA(nullptr, szExeName, sizeof(szExeName));

		auto strWndFileName = std::string(szFileName);
		std::transform(strWndFileName.begin(), strWndFileName.end(), strWndFileName.begin(), tolower);

		auto strExeName = std::string(szExeName);
		std::transform(strExeName.begin(), strExeName.end(), strExeName.begin(), tolower);

		if (strWndFileName != strExeName)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_GAME_WINDOW, 0, strWndFileName);
		}
	}
}

void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD dwEventCode, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (hwnd)
	{
		switch (dwEventCode)
		{
			case EVENT_OBJECT_REORDER:		// window create
			case EVENT_SYSTEM_MINIMIZEEND:	// window maximize
			{
				OnWatcherWindowScan(hwnd);
			} break;
		}
	}
}

DWORD WINAPI WindowCheckRoutine(LPVOID)
{
	auto hWndHandlerHook = SetWinEventHook(EVENT_MIN, EVENT_MAX, nullptr, HandleWinEvent, 0, 0, WINEVENT_OUTOFCONTEXT);
	if (!hWndHandlerHook)
	{
		return 0;
	}
	CAnticheatManager::Instance().RegisterWindowHookHandler(hWndHandlerHook);

	MSG message;
	while (GetMessageA(&message, nullptr, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}

	return 0;
}


bool CAnticheatManager::InitializeWindowWatcherThread()
{
	m_hWndCheckThread = CreateThread(0, 0, WindowCheckRoutine, 0, 0, &m_dwWndCheckThreadId);
	if (!m_hWndCheckThread || m_hWndCheckThread == INVALID_HANDLE_VALUE)
		return false;

	RegisterWaitForSingleObject(&m_hWndThreadWaitObj, m_hWndCheckThread, OnWindowWatcherThreadLost, nullptr, INFINITE, WT_EXECUTEONLYONCE);

	return true;
}

bool CAnticheatManager::DestroyWindowWatcher()
{
	if (m_hWndHandlerHook)
	{
		if (m_hWndThreadWaitObj && m_hWndThreadWaitObj != INVALID_HANDLE_VALUE)
			UnregisterWait(m_hWndThreadWaitObj);

		if (!UnhookWinEvent(m_hWndHandlerHook))
			return false;

		if (m_hWndCheckThread && m_hWndCheckThread != INVALID_HANDLE_VALUE)
			TerminateThread(m_hWndCheckThread, EXIT_SUCCESS);
	}

	return true;
}


