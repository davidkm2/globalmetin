#pragma once
#include "NTDefinations.h"

typedef struct _SYSTEM_THREAD_INFORMATION_MY
{
	LARGE_INTEGER KernelTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER CreateTime;
	ULONG WaitTime;
	PVOID StartAddress;
	CLIENT_ID_MY ClientId;
	KPRIORITY_MY Priority;
	int32_t BasePriority;
	ULONG ContextSwitchCount;
	ULONG ThreadState;
	KWAIT_REASON_MY WaitReason;
#ifdef _WIN64
	ULONG Reserved[4];
#endif
}SYSTEM_THREAD_INFORMATION_MY, *PSYSTEM_THREAD_INFORMATION_MY;

typedef struct _SYSTEM_PROCESS_INFORMATION_MY
{
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	LARGE_INTEGER SpareLi1;
	LARGE_INTEGER SpareLi2;
	LARGE_INTEGER SpareLi3;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING_MY ImageName;
	KPRIORITY_MY BasePriority;
	HANDLE UniqueProcessId;
	HANDLE InheritedFromUniqueProcessId;
	ULONG HandleCount;
	ULONG SessionId;
	ULONG_PTR PageDirectoryBase;
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER ReadOperationCount;
	LARGE_INTEGER WriteOperationCount;
	LARGE_INTEGER OtherOperationCount;
	LARGE_INTEGER ReadTransferCount;
	LARGE_INTEGER WriteTransferCount;
	LARGE_INTEGER OtherTransferCount;
	SYSTEM_THREAD_INFORMATION_MY Threads[1];
} SYSTEM_PROCESS_INFORMATION_MY, *PSYSTEM_PROCESS_INFORMATION_MY;
	
class CThreadEnumerator
{
public:
	CThreadEnumerator(uint32_t dwProcessId);
	virtual ~CThreadEnumerator();

	SYSTEM_PROCESS_INFORMATION_MY * GetProcInfo();
	SYSTEM_THREAD_INFORMATION_MY  * GetThreadList(SYSTEM_PROCESS_INFORMATION_MY * procInfo);
	uint32_t				    	GetThreadCount(SYSTEM_PROCESS_INFORMATION_MY * procInfo);

	SYSTEM_THREAD_INFORMATION_MY  * FindThread(SYSTEM_PROCESS_INFORMATION_MY * procInfo, uint32_t dwThreadId);

protected:
	uint8_t * InitializeQuery();

private:
	uint32_t  m_dwProcessId;
	uint8_t * m_Cap;
};

