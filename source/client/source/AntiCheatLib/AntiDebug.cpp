#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "NTDefinations.h"

#include <XORstr.h>

#if 0
	if (g_nmApp->SelfHooksInstance()->BlockAPI(xorstr("ntdll.dll").crypt_get(), xorstr("DbgUiIssueRemoteBreakin").crypt_get(), RET_HOOK))
	{
		g_nmApp->OnCloseRequest(EXIT_ERR_HOOK_PATCH_6_FAIL, g_winapiApiTable->GetLastError());
		return false;
	}
	if (g_nmApp->SelfHooksInstance()->BlockAPI(xorstr("ntdll.dll").crypt_get(), xorstr("DbgUiRemoteBreakin").crypt_get(), RET_HOOK))
	{
		g_nmApp->OnCloseRequest(EXIT_ERR_HOOK_PATCH_7_FAIL, g_winapiApiTable->GetLastError());
		return false;
	}
	if (g_nmApp->SelfHooksInstance()->BlockAPI(xorstr("ntdll.dll").crypt_get(), xorstr("DbgBreakPoint").crypt_get(), NOP_HOOK))
	{
		g_nmApp->OnCloseRequest(EXIT_ERR_HOOK_PATCH_8_FAIL, g_winapiApiTable->GetLastError());
		return false;
	}
	if (g_nmApp->SelfHooksInstance()->BlockAPI(xorstr("ntdll.dll").crypt_get(), xorstr("DbgUserBreakPoint").crypt_get(), NOP_HOOK))
	{
		g_nmApp->OnCloseRequest(EXIT_ERR_HOOK_PATCH_9_FAIL, g_winapiApiTable->GetLastError());
		return false;
	}
#endif

bool CAnticheatManager::GlobalFlagsCheck()
{
	auto pImageBase = (PBYTE)m_hInstance;
	if (!pImageBase)
		return false;

	auto pIDH = (PIMAGE_DOS_HEADER)pImageBase;
	if (!pIDH || pIDH->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	auto pINH = (PIMAGE_NT_HEADERS)(pImageBase + pIDH->e_lfanew);
	if (!pINH || pINH->Signature != IMAGE_NT_SIGNATURE)
		return false;

	auto pImageLoadConfigDirectory = (PIMAGE_LOAD_CONFIG_DIRECTORY)(pImageBase + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress);
	if (pImageLoadConfigDirectory->GlobalFlagsClear)
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(DEBUG_DETECT_GLOBALFLAGS, 0, "");
		ExitByAnticheat(DEBUG_DETECT_GLOBALFLAGS, 0);
		return true;
	}

	return false;
}

bool CAnticheatManager::SharedUserDataCheck()
{
	auto pUserSharedData = (KUSER_SHARED_DATA *)0x7FFE0000; // The fixed user mode address of KUSER_SHARED_DATA

	BOOLEAN KdDebuggerEnabled = (pUserSharedData->KdDebuggerEnabled & 0x1) == 0x1;
	BOOLEAN KdDebuggerNotPresent = (pUserSharedData->KdDebuggerEnabled & 0x2) == 0x0;

	if (KdDebuggerEnabled || !KdDebuggerNotPresent)
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(DEBUG_DETECT_KERNEL, 0, "");
		ExitByAnticheat(DEBUG_DETECT_KERNEL, 0);
		return true;
	}

	return false;
}

bool CAnticheatManager::DebugObjectHandleCheck()
{
	typedef NTSTATUS(WINAPI* TNtQueryInformationProcess)(HANDLE a, ULONG b, PVOID c, ULONG d, PULONG e);

	auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
	if (!hNtdll)
		return false;

	auto pNtQueryInformationProcess = GetProcAddress(hNtdll, XOR("NtQueryInformationProcess"));
	if (!pNtQueryInformationProcess)
		return false;
	auto NtQueryInformationProcess = (TNtQueryInformationProcess)pNtQueryInformationProcess;

	auto hDebugObject = HANDLE(INVALID_HANDLE_VALUE);
	auto ntStatus = NtQueryInformationProcess(GetCurrentProcess(), /* ProcessDebugObjectHandle */ 30, &hDebugObject, sizeof(HANDLE), nullptr);
	if (!NT_SUCCESS(ntStatus))
		return false;

	if (ntStatus != STATUS_PORT_NOT_SET)
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(DEBUG_DETECT_DBGOBJ_1, 0, "");
		ExitByAnticheat(DEBUG_DETECT_DBGOBJ_1, 0);
		return true;
	}

	if (ntStatus == STATUS_PORT_NOT_SET && hDebugObject)
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(DEBUG_DETECT_DBGOBJ_2, 0, "");
		ExitByAnticheat(DEBUG_DETECT_DBGOBJ_2, 0);
		return true;
	}

	return false;
}

bool CAnticheatManager::DebugCheckBugControl()
{
	typedef NTSTATUS(WINAPI* TNtQueryInformationProcess)(HANDLE a, ULONG b, PVOID c, ULONG d, PULONG e);

	auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
	if (!hNtdll)
		return false;

	auto pNtQueryInformationProcess = GetProcAddress(hNtdll, XOR("NtQueryInformationProcess"));
	if (!pNtQueryInformationProcess)
		return false;
	auto NtQueryInformationProcess = (TNtQueryInformationProcess)pNtQueryInformationProcess;

	auto dwBugCheck = 0UL;
	auto ntStatus = NtQueryInformationProcess(GetCurrentProcess(), /* ProcessDebugObjectHandle */ 30, (PVOID)&dwBugCheck, sizeof(dwBugCheck), &dwBugCheck);
	if (ntStatus == STATUS_PORT_NOT_SET && dwBugCheck != 4)
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(DEBUG_DETECT_BUGCHECK, 0, "");
		ExitByAnticheat(DEBUG_DETECT_BUGCHECK, 0);
		return true;
	}

	return false;
}

bool CAnticheatManager::SimpleDebugCheck()
{
	typedef NTSTATUS(WINAPI* TNtQueryInformationProcess)(HANDLE a, ULONG b, PVOID c, ULONG d, PULONG e);

	auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
	if (!hNtdll)
		return false;

	auto pNtQueryInformationProcess = GetProcAddress(hNtdll, XOR("NtQueryInformationProcess"));
	if (!pNtQueryInformationProcess)
		return false;
	auto NtQueryInformationProcess = (TNtQueryInformationProcess)pNtQueryInformationProcess;

	auto pPEB = GetCurrentPEB();
	if (pPEB && pPEB->BeingDebugged)
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(DEBUG_DETECT_PEB, 0, "");
		ExitByAnticheat(DEBUG_DETECT_PEB, 0);
		return true;
	}

	auto dwFlags = 0UL;
	auto dwReturnSize = 0UL;
	if (NT_SUCCESS(NtQueryInformationProcess(GetCurrentProcess(), /* ProcessDebugFlags */ 31, &dwFlags, sizeof(dwFlags), &dwReturnSize)))
	{
		if (dwReturnSize != sizeof(dwFlags) || dwFlags == 0)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(DEBUG_DETECT_FLAGS, 0, "");
			ExitByAnticheat(DEBUG_DETECT_FLAGS, 0);
			return true;
		}
	}

	return false;
}

#define ThreadHideFromDebugger 0x11
bool CAnticheatManager::SetAntiTrace(HANDLE hThread)
{
	typedef NTSTATUS(NTAPI* TNtSetInformationThread)(HANDLE ThreadHandle, ULONG ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength);
	typedef NTSTATUS(NTAPI* TNtQueryInformationThread)(HANDLE ThreadHandle, ULONG ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength, PULONG ReturnLength);

	if (!IS_VALID_HANDLE(hThread))
		return false;

	auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
	if (!hNtdll)
		return false;

	auto pNtSetInformationThread = GetProcAddress(hNtdll, XOR("NtSetInformationThread"));
	if (!pNtSetInformationThread)
		return false;
	auto NtSetInformationThread = (TNtSetInformationThread)pNtSetInformationThread;

	auto PNtQueryInformationThread = GetProcAddress(hNtdll, XOR("NtQueryInformationThread"));
	if (!PNtQueryInformationThread)
		return false;
	auto NtQueryInformationThread = (TNtQueryInformationThread)PNtQueryInformationThread;

	BOOL bCheckStat = FALSE;

	auto ntReturnStat = NtSetInformationThread(hThread, ThreadHideFromDebugger, &bCheckStat, sizeof(ULONG));
	if (NT_SUCCESS(ntReturnStat)) 
	{
//		DEBUG_LOG(LL_SYS, "Anti trace failed on Step1!");
		return false;
	}

	auto ntFakeStat1 = NtSetInformationThread(hThread, ThreadHideFromDebugger, &bCheckStat, sizeof(UINT));
	if (NT_SUCCESS(ntFakeStat1))
	{
//		DEBUG_LOG(LL_SYS, "Anti trace failed on Step2!");
		return false;
	}

	auto ntFakeStat2 = NtSetInformationThread((HANDLE)0xFFFFF, ThreadHideFromDebugger, 0, 0);
	if (NT_SUCCESS(ntFakeStat2)) 
	{
//		DEBUG_LOG(LL_SYS, "Anti trace failed on Step3!");
		return false;
	}

	auto ntCorrectStat = NtSetInformationThread(hThread, ThreadHideFromDebugger, 0, 0);
	if (!NT_SUCCESS(ntCorrectStat)) 
	{
//		DEBUG_LOG(LL_SYS, "Anti trace failed on Step4, Status: %p", ntCorrectStat);
		return false;
	}

	auto ntCorrectStat2 = NtQueryInformationThread(hThread, ThreadHideFromDebugger, &bCheckStat, sizeof(BOOLEAN), 0);
	if (!NT_SUCCESS(ntCorrectStat))
	{
//		DEBUG_LOG(LL_SYS, "Anti trace failed on Step5, Status: %p", ntCorrectStat);
		return false;
	}

	if (!bCheckStat) 
	{
//		DEBUG_LOG(LL_SYS, "Anti trace failed on Step6!");
		return false;
	}

	return true;
}

size_t CAnticheatManager::CheckExceptionTrap()
{
	CSimpleTimer <std::chrono::milliseconds> checkTimer;

	Sleep(1000);
	__try {
		int* p = 0; // access violation
		*p = 0;
	}
	__except (1) { }
	Sleep(1000);

	return checkTimer.diff();
}
