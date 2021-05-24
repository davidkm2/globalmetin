#pragma once
#include <Windows.h>
//#include <winternl.h>

#pragma warning (disable : 4191)

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status)					(((NTSTATUS)(Status)) >= 0)
#endif

#define STATUS_PORT_NOT_SET					((NTSTATUS)0xC0000353L)
#define STATUS_INFO_LENGTH_MISMATCH			((NTSTATUS)0xC0000004)

namespace
{
	enum THREAD_STATE_MY
	{
		Running = 2,
		Waiting = 5,
	};

	enum KWAIT_REASON_MY
	{
		Suspended = 5,
	};

	typedef struct _CLIENT_ID_MY {
		HANDLE UniqueProcess;
		HANDLE UniqueThread;
	} CLIENT_ID_MY;

	typedef struct _UNICODE_STRING_MY {
		USHORT Length;
		USHORT MaximumLength;
		PWSTR  Buffer;
	} UNICODE_STRING_MY;
	typedef UNICODE_STRING_MY *PUNICODE_STRING_MY;
	typedef const UNICODE_STRING_MY *PCUNICODE_STRING_MY;

	typedef int32_t KPRIORITY_MY;

	typedef struct _KSYSTEM_TIME
	{
		ULONG LowPart;
		int32_t High1Time;
		int32_t High2Time;
	} KSYSTEM_TIME, *PKSYSTEM_TIME;

	typedef enum _NT_PRODUCT_TYPE
	{
		NtProductWinNt = 1,
		NtProductLanManNt = 2,
		NtProductServer = 3
	} NT_PRODUCT_TYPE;

	typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE
	{
		StandardDesign = 0,
		NEC98x86 = 1,
		EndAlternatives = 2
	} ALTERNATIVE_ARCHITECTURE_TYPE;

	typedef struct _KUSER_SHARED_DATA
	{
		ULONG TickCountLowDeprecated;
		ULONG TickCountMultiplier;
		KSYSTEM_TIME InterruptTime;
		KSYSTEM_TIME SystemTime;
		KSYSTEM_TIME TimeZoneBias;
		uint16_t ImageNumberLow;
		uint16_t ImageNumberHigh;
		WCHAR NtSystemRoot[260];
		ULONG MaxStackTraceDepth;
		ULONG CryptoExponent;
		ULONG TimeZoneId;
		ULONG LargePageMinimum;
		ULONG Reserved2[7];
		NT_PRODUCT_TYPE NtProductType;
		UCHAR ProductTypeIsValid;
		ULONG NtMajorVersion;
		ULONG NtMinorVersion;
		UCHAR ProcessorFeatures[64];
		ULONG Reserved1;
		ULONG Reserved3;
		ULONG TimeSlip;
		ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;
		LARGE_INTEGER SystemExpirationDate;
		ULONG SuiteMask;
		UCHAR KdDebuggerEnabled;
	} KUSER_SHARED_DATA, *PKUSER_SHARED_DATA;

	typedef struct _RTL_USER_PROCESS_PARAMETERS_MY {
		uint8_t Reserved1[16];
		PVOID Reserved2[10];
		UNICODE_STRING_MY ImagePathName;
		UNICODE_STRING_MY CommandLine;
	} RTL_USER_PROCESS_PARAMETERS_MY, *PRTL_USER_PROCESS_PARAMETERS_MY;

	typedef
		VOID
		(NTAPI *PPS_POST_PROCESS_INIT_ROUTINE_MY) (
			VOID
			);

	typedef struct _PEB_LDR_DATA_MY {
		uint8_t Reserved1[8];
		PVOID Reserved2[3];
		LIST_ENTRY InMemoryOrderModuleList;
	} PEB_LDR_DATA_MY, *PPEB_LDR_DATA_MY;

	typedef struct _PEB_MY {
		uint8_t Reserved1[2];
		uint8_t BeingDebugged;
		uint8_t Reserved2[1];
		PVOID Reserved3[2];
		PPEB_LDR_DATA_MY Ldr;
		PRTL_USER_PROCESS_PARAMETERS_MY ProcessParameters;
		PVOID Reserved4[3];
		PVOID AtlThunkSListPtr;
		PVOID Reserved5;
		ULONG Reserved6;
		PVOID Reserved7;
		ULONG Reserved8;
		ULONG AtlThunkSListPtr32;
		PVOID Reserved9[45];
		uint8_t Reserved10[96];
		PPS_POST_PROCESS_INIT_ROUTINE_MY PostProcessInitRoutine;
		uint8_t Reserved11[128];
		PVOID Reserved12[1];
		ULONG SessionId;
	} PEB_MY, *PPEB_MY;

	typedef struct _TEB_MY {
		PVOID Reserved1[12];
		PPEB_MY ProcessEnvironmentBlock;
		PVOID Reserved2[399];
		uint8_t Reserved3[1952];
		PVOID TlsSlots[64];
		uint8_t Reserved4[8];
		PVOID Reserved5[26];
		PVOID ReservedForOle;  // Windows 2000 only
		PVOID Reserved6[4];
		PVOID TlsExpansionSlots;
	} TEB_MY, *PTEB_MY;

	typedef enum _SYSTEM_INFORMATION_CLASS_MY {
		SystemBasicInformation = 0,
		SystemPerformanceInformation = 2,
		SystemTimeOfDayInformation = 3,
		SystemProcessInformation = 5,
		SystemProcessorPerformanceInformation = 8,
		SystemInterruptInformation = 23,
		SystemExceptionInformation = 33,
		SystemRegistryQuotaInformation = 37,
		SystemLookasideInformation = 45,
		SystemPolicyInformation = 134,
	} SYSTEM_INFORMATION_CLASS_MY;

	typedef struct _LDR_DATA_TABLE_ENTRY_MY {
		PVOID Reserved1[2];
		LIST_ENTRY InMemoryOrderLinks;
		PVOID Reserved2[2];
		PVOID DllBase;
		PVOID Reserved3[2];
		UNICODE_STRING_MY FullDllName;
		uint8_t Reserved4[8];
		PVOID Reserved5[3];
#pragma warning(push)
#pragma warning(disable: 4201) // we'll always use the Microsoft compiler
		union {
			ULONG CheckSum;
			PVOID Reserved6;
		} DUMMYUNIONNAME;
#pragma warning(pop)
		ULONG TimeDateStamp;
	} LDR_DATA_TABLE_ENTRY_MY, *PLDR_DATA_TABLE_ENTRY_MY;
}

static TEB_MY* GetCurrentTeb()
{
	return reinterpret_cast<TEB_MY*>(
#ifdef _M_X64
		__readgsqword(offsetof(NT_TIB, Self))
#elif _M_IX86
		__readfsdword(offsetof(NT_TIB, Self))
#else
#error "architecture unsupported"
#endif
		);
}
static PEB_MY* GetCurrentPEB()
{
	return reinterpret_cast<PEB_MY*>(GetCurrentTeb()->ProcessEnvironmentBlock);
}

