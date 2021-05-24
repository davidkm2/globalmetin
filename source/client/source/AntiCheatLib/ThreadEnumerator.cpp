#include "StdAfx.h"
#include "ThreadEnumerator.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "NTDefinations.h"

#include <XORstr.h>

CThreadEnumerator::CThreadEnumerator(uint32_t dwProcessId) :
	m_dwProcessId(dwProcessId)
{
	m_Cap = InitializeQuery();
}

CThreadEnumerator::~CThreadEnumerator()
{
	m_dwProcessId = 0;

	if (m_Cap)
		free(m_Cap);
	m_Cap = nullptr;
}

uint8_t * CThreadEnumerator::InitializeQuery()
{
	auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
	if (!hNtdll)
		return nullptr;

	auto pNtQuerySystemInformation = GetProcAddress(hNtdll, XOR("NtQuerySystemInformation"));
	if (!pNtQuerySystemInformation)
		return nullptr;

	typedef NTSTATUS(NTAPI* lpNtQuerySystemInformation)(SYSTEM_INFORMATION_CLASS_MY SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
	auto NtQuerySystemInformation = (lpNtQuerySystemInformation)pNtQuerySystemInformation;

	uint8_t * mp_Data;
	uint32_t mu32_DataSize = 1024 * 1024;

	while (true)
	{
		mp_Data = (uint8_t*)malloc(mu32_DataSize);
		if (!mp_Data)
			break;

		ULONG ntNeeded = 0;
		auto ntStat = NtQuerySystemInformation(SystemProcessInformation, mp_Data, mu32_DataSize, &ntNeeded);

		if (ntStat == STATUS_INFO_LENGTH_MISMATCH)
		{
			mu32_DataSize *= 2;
			mp_Data = (uint8_t*)realloc((PVOID)mp_Data, mu32_DataSize);
			continue;
		}

		return mp_Data;
	}

//	printf("CThreadEnumerator::InitializeQuery fail!");
	return nullptr;
}

SYSTEM_PROCESS_INFORMATION_MY * CThreadEnumerator::GetProcInfo()
{
	auto pk_Proc = (SYSTEM_PROCESS_INFORMATION_MY*)m_Cap;
	if (!pk_Proc)
		return nullptr;

	while (true)
	{
		if ((uint32_t)pk_Proc->UniqueProcessId == m_dwProcessId)
			return pk_Proc;

		if (!pk_Proc->NextEntryOffset)
			return nullptr;

		pk_Proc = (SYSTEM_PROCESS_INFORMATION_MY*)((uint8_t*)pk_Proc + pk_Proc->NextEntryOffset);
	}

	return nullptr;
}

SYSTEM_THREAD_INFORMATION_MY * CThreadEnumerator::GetThreadList(SYSTEM_PROCESS_INFORMATION_MY * procInfo)
{
	auto pk_Thread = procInfo->Threads;
	return pk_Thread;
}

uint32_t CThreadEnumerator::GetThreadCount(SYSTEM_PROCESS_INFORMATION_MY * procInfo)
{
	return procInfo->NumberOfThreads;
}

SYSTEM_THREAD_INFORMATION_MY * CThreadEnumerator::FindThread(SYSTEM_PROCESS_INFORMATION_MY * procInfo, uint32_t dwThreadId)
{
	auto pk_Thread = procInfo->Threads;
	if (!pk_Thread)
		return nullptr;

	for (uint32_t i = 0; i < procInfo->NumberOfThreads; i++)
	{
		if ((uint32_t)pk_Thread->ClientId.UniqueThread == dwThreadId)
			return pk_Thread;

		pk_Thread++;
	}

	return nullptr;
}

