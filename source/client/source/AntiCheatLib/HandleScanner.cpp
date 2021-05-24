#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "NTDefinations.h"

#include <XORstr.h>

// https://github.com/kruk3t/BetaShield/blob/master/BetaStatic/EnumerateHandles.cpp

#if 0
typedef NTSTATUS(NTAPI* lpNtQuerySystemInformation)(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
typedef NTSTATUS(NTAPI* lpNtQueryObject)(HANDLE ObjectHandle, ULONG ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength);


std::wstring GetObjectName(HANDLE hObject)
{
	auto ulReturnLength = 0UL;
	OBJECT_NAME_INFORMATION pName[4096] = { 0 };
	auto ntStat = NtQueryObject(hObject, ObjectNameInformation, &pName, sizeof(pName), &ulReturnLength);
	if (NT_SUCCESS(ntStat))
	{
		if (pName && pName->Name.Buffer && pName->Name.Length)
		{
			return pName->Name.Buffer;
		}
	}
	return L"";
}

void IScanner::OnScanHandle(HANDLE hSourcePid, HANDLE hHandle, PVOID pObject, USHORT uTypeIndex)
{
	printf("PID: %d\t", (int)hSourcePid);
	printf("Object %p\t", pObject);
	printf("Handle %p\r\n", hHandle);

	HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, (DWORD)hSourcePid);
	if (hProcess != INVALID_HANDLE_VALUE)
	{ // TODO: Kernel Wrapper
		HANDLE hObject = NULL;
		if (DuplicateHandle(hProcess, hHandle, GetCurrentProcess(), &hObject, STANDARD_RIGHTS_REQUIRED, FALSE, 0) != FALSE)
		{ // TODO: Kernel Wrapper
			auto lpwsName = GetObjectName(hObject).c_str();
			if (lpwsName != NULL)
			{
				wprintf(L"\nobject Name:\t%s ,object type:\t%d", lpwsName, uTypeIndex);
			}
			CloseHandle(hObject);
		}

		CloseHandle(hProcess);
	}
}

bool ScanHandles()
{


	auto dwHandleInfoSize = 2000;
	auto ntStat = NTSTATUS(0x0);

	auto pHandleInfo = (PSYSTEM_HANDLE_INFORMATION_EX)malloc(dwHandleInfoSize);
	ZeroMemory(pHandleInfo, dwHandleInfoSize);

	while ((ntStat = NtQuerySystemInformation(SystemExtendedHandleInformation, pHandleInfo, dwHandleInfoSize, NULL)) == STATUS_INFO_LENGTH_MISMATCH)
	{
		dwHandleInfoSize *= 2;
		pHandleInfo = (PSYSTEM_HANDLE_INFORMATION_EX)realloc(pHandleInfo, dwHandleInfoSize);
	}

	if (!NT_SUCCESS(ntStat))
	{
		DEBUG_LOG(LL_ERR, "NtQuerySystemInformation failed! Error code: %u Ntstatus: %u", GetLastError(), ntStat);

		free(pHandleInfo);
		return false;
	}

	for (std::size_t i = 0; i < pHandleInfo->HandleCount; i++) 
	{
		auto hCurrHandle = pHandleInfo->Handles[i];

		OnScanHandle(hCurrHandle.ProcessId, hCurrHandle.Handle, hCurrHandle.Object, hCurrHandle.ObjectTypeIndex);
	}

	free(pHandleInfo);

	return true;
}
#endif
