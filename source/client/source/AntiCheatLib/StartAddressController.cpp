#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "../EterBase/WinVerHelper.h"
#include "ProcessNameHelper.h"

#include <XORstr.h>
#include <psapi.h>

enum EBlacklistedShellCodes
{
	ShellCodeNull,
	LdrLoadShellCode1,
	ManualMapShellCode1,
	ManualMapShellCode2,
	ManualMapShellCode3,
	ReflectiveShellCode1,
	ManualLoadShellCode1,
	ThreadHijackShellCode1,
	ThreadHijackShellCode2,
	ThreadHijackShellCode3,
	CreateRemoteThreadExShellCode1,
	CodeInjectionShellCode1,
	AutoInjectorLalakerShellCode,
	EaxLoadLibraryA,
	EaxLoadLibraryW,
	EaxLoadLibraryExA,
	EaxLoadLibraryExW,
	EaxLoadLibraryA_KBase,
	EaxLoadLibraryW_KBase,
	EaxLoadLibraryExA_KBase,
	EaxLoadLibraryExW_KBase,
	EaxLdrLoadDll,
	EaxSetWindowHookEx,
	EaxPython,
	EaxRtlUserThreadStart,
	EaxSetWindowHookEx2,
	EaxNtCreateThread,
	EaxNtCreateThreadEx,
	EaxRtlCreateUserThread,
	EaxUnknownState,
	EaxBadPointerType,
	EaxBadProtectType,
	QueryWorkingSetExFail,
	QueryWorkingSetExNotValid,
	NullCharacteristics,
	EaxMainProcess,
	EaxMappedCode,
};

inline int32_t FilterShellcode(LPVOID lpTargetAddress)
{
	auto pMemory = (uint8_t*)lpTargetAddress;
	uint8_t shellLdrLoad[5] = { 0x55, 0x8B, 0xEC, 0x8D, 0x5 };
	uint8_t shellManualMp[6] = { 0x55, 0x8B, 0xEC, 0x51, 0x53, 0x8B };
	uint8_t shellReflective[8] = { 0x55, 0x89, 0xE5, 0x53, 0x83, 0xEC, 0x54, 0x8B };
	uint8_t shellMLoad[8] = { 0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x20, 0x53, 0x56 };
	uint8_t shellhijack[10] = { 0x68, 0xCC, 0xCC, 0xCC, 0xCC, 0x60, 0x9C, 0xBB, 0xCC, 0xCC };
	uint8_t shellhijack2[10] = { 0x60, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x5B, 0x81, 0xEB, 0x06 };
	uint8_t shellhijack3[10] = { 0x56, 0x8B, 0x35, 0x00, 0xC0, 0x27, 0x6A, 0x57, 0x8B, 0x3D };
	uint8_t shellcreateremotethreadex[10] = { 0xE8, 0x1D, 0x00, 0x00, 0x00, 0x50, 0x68, 0x58, 0x58, 0xC3 };
	uint8_t shellcodeinjectrosdevil[8] = { 0x68, 0xAC, 0xCE, 0xEA, 0xAC, 0x9C, 0x60, 0x68 };
	uint8_t shellcodeLalakerAuto[8] = { 0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x5D, 0xFF, 0x25 };

	// LdrLoadDll, LdrpLoadDll, ManualMap1
	if (memcmp(lpTargetAddress, &shellLdrLoad, 5) == 0)
		return LdrLoadShellCode1;

	// ManualMap2
	if (memcmp(lpTargetAddress, &shellManualMp, 6) == 0)
		return ManualMapShellCode1;

	// ManualMap3 ++
	if (*pMemory == 0x68 && *(pMemory + 5) == 0x68)
	{
		if (*(pMemory + 10) == 0xB8)
			return ManualMapShellCode2;
		else if (*(pMemory + 10) == 0x68)
			return ManualMapShellCode3;
	}

	// Reflective
	if (memcmp(lpTargetAddress, &shellReflective, sizeof(shellReflective)) == 0)
		return ReflectiveShellCode1;

	// Manual Load
	if (memcmp(lpTargetAddress, &shellMLoad, sizeof(shellMLoad)) == 0)
		return ManualLoadShellCode1;

	// Thread hijack 1
	if (memcmp(lpTargetAddress, &shellhijack, sizeof(shellhijack)) == 0)
		return ThreadHijackShellCode1;

	// Thread hijack 2
	if (memcmp(lpTargetAddress, &shellhijack2, sizeof(shellhijack2)) == 0)
		return ThreadHijackShellCode2;

	// Thread hijack 3
	if (memcmp(lpTargetAddress, &shellhijack3, sizeof(shellhijack3)) == 0)
		return ThreadHijackShellCode3;

	// Createremotethreadex 1
	if (memcmp(lpTargetAddress, &shellcreateremotethreadex, sizeof(shellcreateremotethreadex)) == 0)
		return CreateRemoteThreadExShellCode1;

	// Code injection 1
	if (memcmp(lpTargetAddress, &shellcodeinjectrosdevil, sizeof(shellcodeinjectrosdevil)) == 0)
		return CodeInjectionShellCode1;

	// Lalaker auto injector
	if (memcmp(lpTargetAddress, &shellcodeLalakerAuto, sizeof(shellcodeLalakerAuto)) == 0)
		return AutoInjectorLalakerShellCode;

	return ShellCodeNull;
}

bool CAnticheatManager::IsBlockedStartAddress(uint32_t dwThreadId, uint32_t dwStartAddress, LPDWORD pdwViolationID, std::string * pstrViolationOwnerModule)
{
	typedef NTSTATUS(WINAPI* TNtQueryInformationThread)(HANDLE, int32_t, PVOID, ULONG, PULONG);
	
	auto hKernel32 = GetModuleHandleA(XOR("kernel32.dll"));
	if (!hKernel32)
	{
//		TraceError("!hKernel32");
		return false;
	}

	auto hUser32 = LoadLibraryA(XOR("user32.dll"));
	if (!hUser32)
	{
//		TraceError("!hUser32");
		return false;
	}

	auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
	if (!hNtdll)
	{
//		TraceError("!hNtdll");
		return false;
	}

	auto hPython27 = GetModuleHandleA(XOR("python27.dll"));
	if (!hPython27)
	{
//		TraceError("!hPython27");
		return false;
	}

	uint32_t dwThreadStartAddress = 0;

	auto hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwThreadId);
	if (hThread && hThread != INVALID_HANDLE_VALUE)
	{
		auto pNtQueryInformationThread = GetProcAddress(hNtdll, XOR("NtQueryInformationThread"));
		if (pNtQueryInformationThread)
		{
			auto NtQueryInformationThread = reinterpret_cast<TNtQueryInformationThread>(pNtQueryInformationThread);

			auto ntStatus = NtQueryInformationThread(hThread, /* ThreadQuerySetWin32StartAddress */ 9, &dwThreadStartAddress, sizeof(dwThreadStartAddress), nullptr);
		}
	}

	if (!dwThreadStartAddress)
		dwThreadStartAddress = dwStartAddress;

	if (!dwThreadStartAddress)
	{
//		TraceError("!dwThreadStartAddress");
		return false;
	}

	char szFileName[1024] = { 0 };
	GetMappedFileNameA(GetCurrentProcess(), (LPVOID)dwThreadStartAddress, szFileName, sizeof(szFileName));

	auto strFileName = std::string(szFileName);
	if (strFileName.size() > 0)
	{
		auto iLastSlash = strFileName.find_last_of("\\/");
		strFileName = strFileName.substr(iLastSlash + 1, strFileName.length() - iLastSlash);
	}

	auto dwDetectedType = 0UL;

	auto dwLoadLibraryA = (uint32_t)GetProcAddress(hKernel32, XOR("LoadLibraryA"));
	auto dwLoadLibraryW = (uint32_t)GetProcAddress(hKernel32, XOR("LoadLibraryW"));
	auto dwLoadLibraryExA = (uint32_t)GetProcAddress(hKernel32, XOR("LoadLibraryExA"));
	auto dwLoadLibraryExW = (uint32_t)GetProcAddress(hKernel32, XOR("LoadLibraryExW"));

	auto dwLdrLoadDll = (uint32_t)GetProcAddress(hNtdll, XOR("LdrLoadDll"));

	//

	MODULEINFO user32ModInfo = { 0 };
	if (!GetModuleInformation(GetCurrentProcess(), hUser32, &user32ModInfo, sizeof(user32ModInfo)))
	{
//		TraceError("!user32ModInfo");
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return false;
	}

	auto dwUser32Low = (uint32_t)user32ModInfo.lpBaseOfDll;
	auto dwUser32Hi = (uint32_t)user32ModInfo.lpBaseOfDll + user32ModInfo.SizeOfImage;

	MODULEINFO pythonModInfo = { 0 };
	if (!GetModuleInformation(GetCurrentProcess(), hPython27, &pythonModInfo, sizeof(pythonModInfo)))
	{
//		TraceError("!pythonModInfo");
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return false;
	}

	auto dwPythonLow = (uint32_t)pythonModInfo.lpBaseOfDll;
	auto dwPythonHi = (uint32_t)pythonModInfo.lpBaseOfDll + pythonModInfo.SizeOfImage;

	MEMORY_BASIC_INFORMATION mbiCaller = { 0 };
	if (!VirtualQuery((LPCVOID)dwThreadStartAddress, &mbiCaller, sizeof(mbiCaller)))
	{
//		TraceError("!VirtualQuery");
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return false;
	}

	int32_t iShellInjectionRet = FilterShellcode((LPVOID)dwThreadStartAddress);
	if (iShellInjectionRet)
	{
		if (pdwViolationID) *pdwViolationID = iShellInjectionRet;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return true;
	}

	if (dwThreadStartAddress == dwLoadLibraryA)
	{
		if (pdwViolationID) *pdwViolationID = EaxLoadLibraryA;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return true;
	}
	else if (dwThreadStartAddress == dwLoadLibraryW)
	{
		if (pdwViolationID) *pdwViolationID = EaxLoadLibraryW;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return true;
	}
	else if (dwThreadStartAddress == dwLoadLibraryExA)
	{
		if (pdwViolationID) *pdwViolationID = EaxLoadLibraryExA;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return true;
	}
	else if (dwThreadStartAddress == dwLoadLibraryExW)
	{
		if (pdwViolationID) *pdwViolationID = EaxLoadLibraryExW;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return true;
	}
	else if (dwThreadStartAddress == dwLdrLoadDll)
	{
		if (pdwViolationID) *pdwViolationID = EaxLdrLoadDll;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return true;
	}
	else if (dwThreadStartAddress >= dwUser32Low && dwThreadStartAddress <= dwUser32Hi)
	{
		if (pdwViolationID) *pdwViolationID = EaxSetWindowHookEx;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return true;
	}
	else if (dwThreadStartAddress >= dwPythonLow && dwThreadStartAddress <= dwPythonHi)
	{
		if (pdwViolationID) *pdwViolationID = EaxPython;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return true;
	}

	if (IsWindowsVistaOrGreater())
	{
		auto dwRtlUserThreadStart = (uint32_t)GetProcAddress(hNtdll, XOR("RtlUserThreadStart"));
		if (dwThreadStartAddress == dwRtlUserThreadStart)
		{
			if (pdwViolationID) *pdwViolationID = EaxRtlUserThreadStart;
			if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
			if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
			return true;
		}

		auto dwNtCreateThread = (uint32_t)GetProcAddress(hNtdll, XOR("NtCreateThread"));
		if (dwThreadStartAddress == dwNtCreateThread)
		{
			if (pdwViolationID) *pdwViolationID = EaxNtCreateThread;
			if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
			if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
			return true;
		}

		auto dwNtCreateThreadEx = (uint32_t)GetProcAddress(hNtdll, XOR("NtCreateThreadEx"));
		if (dwThreadStartAddress == dwNtCreateThreadEx)
		{
			if (pdwViolationID) *pdwViolationID = EaxNtCreateThreadEx;
			if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
			if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
			return true;
		}

		auto dwRtlCreateUserThread = (uint32_t)GetProcAddress(hNtdll, XOR("RtlCreateUserThread"));
		if (dwThreadStartAddress == dwRtlCreateUserThread)
		{
			if (pdwViolationID) *pdwViolationID = EaxRtlCreateUserThread;
			if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
			if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
			return true;
		}
	}

	auto hKernelbase = LoadLibraryA(XOR("kernelbase.dll"));
	if (hKernelbase)
	{
		auto dwLoadLibraryA_KBase = (uint32_t)GetProcAddress(hKernelbase, XOR("LoadLibraryA"));
		if (dwLoadLibraryA_KBase && dwThreadStartAddress == dwLoadLibraryA_KBase)
		{
			if (pdwViolationID) *pdwViolationID = EaxLoadLibraryA_KBase;
			if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
			if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
			return true;
		}

		auto dwLoadLibraryW_KBase = (uint32_t)GetProcAddress(hKernelbase, XOR("LoadLibraryW"));
		if (dwLoadLibraryW_KBase && dwThreadStartAddress == dwLoadLibraryW_KBase)
		{
			if (pdwViolationID) *pdwViolationID = EaxLoadLibraryW_KBase;
			if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
			if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
			return true;
		}

		auto dwLoadLibraryExA_KBase = (uint32_t)GetProcAddress(hKernelbase, XOR("LoadLibraryExA"));
		if (dwLoadLibraryExA_KBase && dwThreadStartAddress == dwLoadLibraryExA_KBase)
		{
			if (pdwViolationID) *pdwViolationID = EaxLoadLibraryExA_KBase;
			if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
			if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
			return true;
		}

		auto dwLoadLibraryExW_KBase = (uint32_t)GetProcAddress(hKernelbase, XOR("LoadLibraryExW"));
		if (dwLoadLibraryExW_KBase && dwThreadStartAddress == dwLoadLibraryExW_KBase)
		{
			if (pdwViolationID) *pdwViolationID = EaxLoadLibraryExW_KBase;
			if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
			if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
			return true;
		}
	}

	if (mbiCaller.State != MEM_COMMIT)
	{
		if (pdwViolationID) *pdwViolationID = EaxUnknownState;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return true;
	}

	if (mbiCaller.AllocationProtect == PAGE_EXECUTE_READWRITE)
	{
		if (pdwViolationID) *pdwViolationID = EaxBadProtectType;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
		return true;
	}

	if (IsWindowsVistaOrGreater())
	{
		PSAPI_WORKING_SET_EX_INFORMATION pworkingSetExInformation = { 0 };
		pworkingSetExInformation.VirtualAddress = (LPVOID)dwThreadStartAddress;

		if (FALSE == QueryWorkingSetEx(GetCurrentProcess(), &pworkingSetExInformation, sizeof(pworkingSetExInformation)))
		{
			if (pdwViolationID) *pdwViolationID = QueryWorkingSetExFail;
			if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
			if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
			return true;
		}

		if (!pworkingSetExInformation.VirtualAttributes.Valid)
		{
			if (pdwViolationID) *pdwViolationID = QueryWorkingSetExNotValid;
			if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
			if (pstrViolationOwnerModule) *pstrViolationOwnerModule = strFileName;
			return true;
		}
	}

	auto szLowerFileName = std::string(szFileName);
	std::transform(szLowerFileName.begin(), szLowerFileName.end(), szLowerFileName.begin(), tolower);

	auto szLowerConvertedName = szLowerFileName;
	if (szLowerConvertedName.size() > 0)
		szLowerConvertedName = DosDevicePath2LogicalPath(szLowerFileName.c_str());

	char szExeName[MAX_PATH] = { 0 };
	GetModuleFileNameA(nullptr, szExeName, sizeof(szExeName));

	auto strLowerExeName = std::string(szExeName);
	std::transform(strLowerExeName.begin(), strLowerExeName.end(), strLowerExeName.begin(), tolower);

	if (szFileName && szLowerConvertedName == strLowerExeName)
	{
		if (pdwViolationID) *pdwViolationID = EaxMainProcess;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = szFileName;
		return true;
	}

	if (szLowerConvertedName.empty() && mbiCaller.Type == MEM_PRIVATE && mbiCaller.RegionSize == 0x1000)
	{
		if (pdwViolationID) *pdwViolationID = EaxMappedCode;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = szFileName;
		return true;
	}

	/*
	auto pCurrentSecHdr = (IMAGE_SECTION_HEADER*)dwThreadStartAddress;
	if (pCurrentSecHdr && !pCurrentSecHdr->Characteristics)
	{
		if (pdwViolationID) *pdwViolationID = NullCharacteristics;
		if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
		if (pstrViolationOwnerModule) *pstrViolationOwnerModule = szFileName;
		return true;
	}
	*/

	if (hThread && hThread != INVALID_HANDLE_VALUE) CloseHandle(hThread);
	return false;
}


