#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "../EterBase/WinVerHelper.h"

#include <XORstr.h>

typedef BOOL(WINAPI* TGetWindowDisplayAffinity)(HWND hWnd, DWORD* pdwAffinity);
static TGetWindowDisplayAffinity pGetWindowDisplayAffinity = nullptr;
typedef BOOL(WINAPI* TSetWindowDisplayAffinity)(HWND hWnd, DWORD dwAffinity);
static TSetWindowDisplayAffinity pSetWindowDisplayAffinity = nullptr;

bool CAnticheatManager::GetScreenProtectionStatus()
{
	if (m_mScreenProtectionStatus.find(m_hWnd) != m_mScreenProtectionStatus.end())
		return m_mScreenProtectionStatus[m_hWnd];
	return false;
}

void CAnticheatManager::CheckScreenProtection()
{
	if (!IsWindows7SP1OrGreater())
		return;

	if (!GetScreenProtectionStatus())
		return;

	auto dwAffinity = 0UL;
	auto bRet = pGetWindowDisplayAffinity(m_hWnd, &dwAffinity);
#ifdef _DEBUG
	Tracenf("hWnd: %p Current protection status ret: %d affinity: %u", bRet, dwAffinity);
#endif

	if (!bRet)
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(WINDOW_PROTECTION_CHECK_FAIL, GetLastError(), "");
		return;
	}

	if (dwAffinity != WDA_MONITOR)
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(WINDOW_PROTECTION_CORRUPTED, GetLastError(), "");
		return;
	}
}

bool CAnticheatManager::ActivateScreenProtection(bool bEnabled)
{
	auto bRet = pSetWindowDisplayAffinity(m_hWnd, bEnabled ? WDA_MONITOR : WDA_NONE) ? true : false;
#ifdef _DEBUG
	Tracenf("ActivateScreenProtection: hWnd: %p New status: %d Protection ret: %d Last error: %u", m_hWnd, bEnabled, bRet, GetLastError());
#endif

	m_mScreenProtectionStatus[m_hWnd] = bEnabled;
	return bRet;
}

void CAnticheatManager::InitScreenProtection()
{
	if (!IsWindows7SP1OrGreater())
		return;

	if (!m_hWnd || !IsWindow(m_hWnd))
	{
#ifdef _DEBUG
		TraceError("InitScreenProtection: Target window: %p is NOT valid!", m_hWnd);
#endif
		return;
	}

	auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
	if (!hNtdll)
		return;

	pGetWindowDisplayAffinity = (TGetWindowDisplayAffinity)GetProcAddress(hNtdll, XOR("GetWindowDisplayAffinity"));
	pSetWindowDisplayAffinity = (TSetWindowDisplayAffinity)GetProcAddress(hNtdll, XOR("SetWindowDisplayAffinity"));

	if (!pGetWindowDisplayAffinity || !pSetWindowDisplayAffinity)
		return;

#ifdef _DEBUG
	char szClass[MAX_PATH] = { 0x0 };
	GetClassNameA(m_hWnd, szClass, MAX_PATH);

	char szTitle[MAX_PATH] = { 0x0 };
	GetWindowTextA(m_hWnd, szTitle, MAX_PATH);

	Tracenf("InitScreenProtection: Target window found! %p | %s-%s", m_hWnd, szTitle, szClass);
#endif

	if (!ActivateScreenProtection(true))
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(WINDOW_PROTECTION_CAN_NOT_ENABLED, GetLastError(), "");
		return;
	}
}

void CAnticheatManager::RemoveScreenProtection()
{
	if (!IsWindows7SP1OrGreater())
		return;

	if (!m_hWnd || !IsWindow(m_hWnd))
	{
#ifdef _DEBUG
		TraceError("RemoveScreenProtection: Target window: %p is NOT valid!", m_hWnd);
#endif
		return;
	}

#ifdef _DEBUG
	char szClass[MAX_PATH] = { 0x0 };
	GetClassNameA(m_hWnd, szClass, MAX_PATH);

	char szTitle[MAX_PATH] = { 0x0 };
	GetWindowTextA(m_hWnd, szTitle, MAX_PATH);

	Tracenf("RemoveScreenProtection: Target window found! %p | %s-%s", m_hWnd, szTitle, szClass);
#endif

	if (!ActivateScreenProtection(false))
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(WINDOW_PROTECTION_CAN_NOT_DISABLED, GetLastError(), "");
		return;
	}
}
