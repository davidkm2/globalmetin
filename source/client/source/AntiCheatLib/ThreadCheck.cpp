#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "NTDefinations.h"
#include "../eterPack/EterPackManager.h"

#include <XORstr.h>
#include <Python/Python.h>

#if 0

// https://github.com/mq1n/DLLThreadInjectionDetector/blob/master/DLLInjectionDetector/ThreadCheck.cpp

bool CAnalyser::OnThreadCreated(DWORD dwThreadID, LPVOID lpStartAddress, LPVOID lpEIP, LPDWORD pdwErrorType)
{
#if 0
	// Check module owner name with file scan wrapper
	// Start address + mbi.regionsize section scan wrapper
	// Check start address is in text section range
	// Check eip register is in text section range
	// check mbi.type, is it mem_image or what
	// check priority
	// enum modules check thread's module, if is not pointed to any module range, kill
	// check stack
	// teb -> tib -> ArbitraryUserPointer -> file analyse
#endif

	HOOK_LOG(LL_SYS, "Thread check started! TID: %u Start address: %p", dwThreadID, lpStartAddress);

	if (g_nmApp->ThreadManagerInstance()->GetThreadFromId(dwThreadID))
		return true;

	auto pThread = std::make_unique<CThread>(dwThreadID);
	if (!IS_VALID_SMART_PTR(pThread)|| !pThread->IsCorrect())
	{
		if (pdwErrorType) *pdwErrorType = (DWORD)-1;
		return false;
	}

	auto _pStartAddress = lpStartAddress;
	if (!_pStartAddress)
		_pStartAddress = (LPVOID)pThread->GetStartAddress();

	auto _pEIP = (DWORD)lpEIP;
	if (!_pEIP)
	{
		auto pContext = pThread->GetContext();
		if (pContext && pContext.get())
		{
#ifndef _M_X64
			_pEIP = pContext->Eip;
#else
			_pEIP = pContext->Rip;
#endif
		}
	}

	if (!_pStartAddress)
	{
		if (pdwErrorType) *pdwErrorType = 100;

		auto szTempName = g_nmApp->FunctionsInstance()->GetModuleOwnerName(_pStartAddress);
		Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n").crypt_get(), szTempName.c_str());

		return false;
	}

	auto dwReturnCode = 0UL;
	auto bIsCallerLegit = IsCallerAddressLegit(_pStartAddress, CHECK_TYPE_THREAD, "", &dwReturnCode);
	if (!bIsCallerLegit)
	{
		if (pdwErrorType) *pdwErrorType = dwReturnCode;

		auto szTempName = g_nmApp->FunctionsInstance()->GetModuleOwnerName(_pStartAddress);
		Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n").crypt_get(), szTempName.c_str());

		return false;
	}

	if (pThread->IsItAlive() == false)
		return true;

	auto dwStartAddress = pThread->GetStartAddress(); 
	if (dwStartAddress != (DWORD)_pStartAddress)  // spoofed address?
	{
		if (pdwErrorType) *pdwErrorType = 101;

		auto szTempName = g_nmApp->FunctionsInstance()->GetModuleOwnerName(_pStartAddress);
		Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n").crypt_get(), szTempName.c_str());

		return false;
	}

	if (pThread->IsRemoteThread()) 
	{
		if (pdwErrorType) *pdwErrorType = 102;

		auto szTempName = g_nmApp->FunctionsInstance()->GetModuleOwnerName(_pStartAddress);
		Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n").crypt_get(), szTempName.c_str());

		return false;
	}

	if (pThread->HasSuspend()) 
	{
		if (pdwErrorType) *pdwErrorType = 103;

		auto szTempName = g_nmApp->FunctionsInstance()->GetModuleOwnerName(_pStartAddress);
		Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n").crypt_get(), szTempName.c_str());

		return false;
	}

	if (g_nmApp->DynamicWinapiInstance()->IsLoadedAddress(dwStartAddress)) // unlinked or mmapped
	{ 
		if (pdwErrorType) *pdwErrorType = 104;

		auto szTempName = g_nmApp->FunctionsInstance()->GetModuleOwnerName(_pStartAddress);
		Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n").crypt_get(), szTempName.c_str());

		return false;
	}

	auto dwDbgUiRemoteBreakin = (DWORD)g_winapiApiTable->GetProcAddress(g_winapiModuleTable->hNtdll, xorstr("DbgUiRemoteBreakin").crypt_get());
	if (_pEIP && _pEIP == dwDbgUiRemoteBreakin && !dwStartAddress) 
	{
		if (pdwErrorType) *pdwErrorType = 105;

		auto szTempName = g_nmApp->FunctionsInstance()->GetModuleOwnerName(_pStartAddress);
		Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n").crypt_get(), szTempName.c_str());

		return false;
	}
		
	if (pThread->HasDebugRegisters())
		pThread->ClearDebugRegisters();

	MEMORY_BASIC_INFORMATION mbi = { 0 };
	if (g_winapiApiTable->VirtualQuery(_pStartAddress, &mbi, sizeof(mbi)))
	{
		if (mbi.AllocationBase && mbi.AllocationBase == _pStartAddress) /* If thread started in page's allocated base aka. from page EP */
		{
			if (pdwErrorType) *pdwErrorType = 106;

			auto szTempName = g_nmApp->FunctionsInstance()->GetModuleOwnerName(_pStartAddress);
			Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n").crypt_get(), szTempName.c_str());

			return false;
		}

#if 0
		HMODULE hOwner = nullptr;
		g_winapiApiTable->GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)dwStartAddress, &hOwner);
		if (!hOwner && mbi.Type == MEM_IMAGE)
		{
			SSectionEnumerateInfos sectionEnumerateInfos;
			sectionEnumerateInfos.dwBase			= (DWORD)mbi.BaseAddress;
			sectionEnumerateInfos.dwAllocationBase	= (DWORD)mbi.AllocationBase;
			sectionEnumerateInfos.ulSize			= mbi.RegionSize;
			sectionEnumerateInfos.dwProtect			= mbi.Protect;
			sectionEnumerateInfos.dwState			= mbi.State;
			sectionEnumerateInfos.dwType			= mbi.Type;
			sectionEnumerateInfos.bCanAccess		= true;

			ScanSection(&sectionEnumerateInfos);
		}
#endif
	}
	else
	{
		// TODO: block it, non accessable or what? 
	}

	auto szModuleName = g_nmApp->FunctionsInstance()->GetModuleOwnerName(reinterpret_cast<LPVOID>(dwStartAddress));
	if (szModuleName.empty())
	{
		if (pdwErrorType) *pdwErrorType = 107;
		return false;
	}
	else
	{
		// TODO: Check thread owner module path
		auto szLowerModuleName = g_nmApp->FunctionsInstance()->szLower(szModuleName);

		auto szAntiFilename = g_nmApp->DataInstance()->GetAntiFileName();
		auto szLowerAntiFilename = g_nmApp->FunctionsInstance()->szLower(szAntiFilename);

		auto hOwnerModule = g_winapiApiTable->GetModuleHandleA_o(szLowerModuleName.c_str());
		if (!hOwnerModule)
		{
			if (!strstr(szLowerModuleName.c_str(), szLowerAntiFilename.c_str()))
			{
				if (pdwErrorType) *pdwErrorType = 108;
				Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n").crypt_get(), szLowerModuleName.c_str());
				return false;
			}
		}

		MODULEINFO currentModInfo = { 0 };
		if (g_winapiApiTable->GetModuleInformation(NtCurrentProcess, hOwnerModule, &currentModInfo, sizeof(currentModInfo)))
		{
			auto dwModuleLow	= (DWORD)currentModInfo.lpBaseOfDll;
			auto dwModuleHi		= (DWORD)currentModInfo.lpBaseOfDll + currentModInfo.SizeOfImage;

			if (dwStartAddress < dwModuleLow || dwStartAddress > dwModuleHi) // out of module area
			{
				if (!strstr(szLowerModuleName.c_str(), szLowerAntiFilename.c_str()))
				{
					if (pdwErrorType) *pdwErrorType = 109;
					Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n").crypt_get(), szLowerModuleName.c_str());
					return false;
				}
			}

#if 0
			// main threadde tetikleniyor, eipin modülünü loglay?p whitelist olu?tur
			auto szEIPModuleName = g_nmApp->FunctionsInstance()->GetModuleOwnerName(_pEIP);
			if (_pEIP && (_pEIP < dwModuleLow || _pEIP > dwModuleHi))
			{
				if (pdwErrorType) *pdwErrorType = 110;
				Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n"), szEIPModuleName.c_str());
				return false;
			}
#endif
		}

#if 0
		if (!strstr(szLowerModuleName.c_str(), szLowerAntiFilename.c_str()))
		{
			std::wstring wszModuleName(szModuleName.begin(), szModuleName.end());

			if (!g_nmApp->ScannerInstance()->IsFileDigitalSigned(wszModuleName.c_str()))
			{
				if (pdwErrorType) *pdwErrorType = 111;
				Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n"), szLowerModuleName.c_str());
				return false;
			}

			if (!g_nmApp->ScannerInstance()->isValidFileCertificate(wszModuleName.c_str()))
			{
				if (pdwErrorType) *pdwErrorType = 112;
				Logf(CUSTOM_LOG_FILENAME, xorstr("Illegal thread owner: %s\n"), szLowerModuleName.c_str());
				return false;
			}
		}
#endif
	}

	if (pThread->HasDebugRegisters()) // already cleaned but still exist
	{
		if (pdwErrorType) *pdwErrorType = 113;
		return false;
	}

	// TODO: Context scan

	return true;
}


void NTAPI LdrInitializeThunkDetour(PCONTEXT NormalContext, PVOID SystemArgument1, PVOID SystemArgument2)
{
//	HOOK_LOG(LL_CRI, "LdrInitializeThunk triggered! Ctx: %p Arg: %p", NormalContext, SystemArgument1);
	printf("LdrInitializeThunk triggered! Ctx: %p Arg: %p", NormalContext, SystemArgument1);

#if 0
	//test
	void * _ecx = NULL;
	void * _edx = NULL;

	_asm 
	{
		mov _ecx, ecx;
		mov _edx, edx;
	}

#ifdef _M_IX86
	if (((DWORD)_edx) >= 0x70000000)
	{
		HOOK_LOG(LL_CRI, "test1");
	}
#endif

	if (!IsBadWritePtr(_edx, 1))
	{
		HOOK_LOG(LL_CRI, "test2");
	}

	__asm 
	{
		mov  ecx, _ecx;
		mov  edx, _edx;
	}
#endif

#if 0
	auto pThreadHookData = std::make_shared<SThreadHookQueueData>();
	if (IS_VALID_SMART_PTR(pThreadHookData))
	{
		pThreadHookData->dwThreadID		= g_winapiApiTable->GetCurrentThreadId();
		pThreadHookData->lpStartAddress = NormalContext ? (LPVOID)NormalContext->Eax : nullptr;
		pThreadHookData->lpEIP			= NormalContext ? (LPVOID)NormalContext->Eip : nullptr;

		g_nmApp->SelfHooksInstance()->ThreadHookCallback(pThreadHookData);
	}
#endif

	return hLdrInitializeThunk.Call(NormalContext, SystemArgument1, SystemArgument2);
}
#endif