#include "stdafx.h"
#include "winminidump.h"

#ifdef _WIN32
#include <DbgHelp.h>

// Custom minidump callback 
BOOL CALLBACK MiniDumpCallback(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput)
{
	BOOL bRet = FALSE;

	if (!pInput || !pOutput)
		return FALSE;

	switch (pInput->CallbackType)
	{
		case IncludeModuleCallback:
		{
			bRet = TRUE;
		} break;

		case IncludeThreadCallback:
		{
			bRet = TRUE;
		} break;

		case ModuleCallback:
		{
			if (!(pOutput->ModuleWriteFlags & ModuleReferencedByMemory)) 
			{
				pOutput->ModuleWriteFlags &= (~ModuleWriteModule);
			}
			bRet = TRUE;
		} break;

		case ThreadCallback:
		{
			bRet = TRUE;
		} break;

		case ThreadExCallback:
		{
			bRet = TRUE;
		} break;

		case MemoryCallback:
		{
			bRet = FALSE;
		} break;

		case CancelCallback:
			break;
	}

	return bRet;
}

bool CreateMiniDump(EXCEPTION_POINTERS* pExceptionInfo)
{
	fprintf(stderr, "Exception handled: %p\n", pExceptionInfo);
	if (IsDebuggerPresent())
		DebugBreak();

	char szProcessName[MAX_PATH];
	GetModuleFileNameA(NULL, szProcessName, MAX_PATH);
	std::string strFileName = std::string(szProcessName);
	if (strFileName.size() > 0)
	{
		size_t iLastSlash = strFileName.find_last_of("\\/");
		strFileName = strFileName.substr(iLastSlash + 1, strFileName.length() - iLastSlash);
	}

	time_t t;
	time(&t);
	struct tm *tinfo;
	tinfo = localtime(&t);

	char szDumpName[128];
	strftime(szDumpName, sizeof(szDumpName), "dump%Y%m%d_%H%M%S.dmp", tinfo);

	char szDumpPath[256];
	sprintf(szDumpPath, "%s_%s", strFileName.c_str(), szDumpName);

	HANDLE hFile = CreateFileA(szDumpPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (!hFile || hFile == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Exception dump file is not created. Error code: %u Path: %s\n", GetLastError(), szDumpPath);
		return false;
	}

	// Create the minidump 
	MINIDUMP_EXCEPTION_INFORMATION mdei;
	mdei.ThreadId = GetCurrentThreadId();
	mdei.ExceptionPointers = pExceptionInfo;
	mdei.ClientPointers = FALSE;

	MINIDUMP_CALLBACK_INFORMATION mci;
	mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
	mci.CallbackParam = 0;

	MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

	BOOL rv = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, (pExceptionInfo != 0) ? &mdei : 0, 0, &mci);
	if (!rv)
	{
		fprintf(stderr, "Exception dump can not created. Error code: %u\n", GetLastError());
	}
	else
	{
		fprintf(stderr, "Exception dump successfully created.\n");
	}

	// Close the file 
	CloseHandle(hFile);
	return true;
}

LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS * pExceptionInfo)
{
	if (pExceptionInfo && pExceptionInfo->ExceptionRecord)
	{
		if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
		{
			HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CreateMiniDump, pExceptionInfo, 0, NULL);
			if (hThread && hThread != INVALID_HANDLE_VALUE)
			{
				WaitForSingleObject(hThread, INFINITE);
				CloseHandle(hThread);
			}
		}
		else
		{
			CreateMiniDump(pExceptionInfo);
		}
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

bool setup_minidump_generator()
{
	if (SetUnhandledExceptionFilter(ExceptionFilter))
	{
		fprintf(stderr, "Mini dump generator succesfully created!\n");
		return true;
	}
	fprintf(stderr, "Mini dump generator can NOT created! Error code: %u\n", GetLastError());
	return false;
}

#else

bool setup_minidump_generator()
{
	return true;
}

#endif