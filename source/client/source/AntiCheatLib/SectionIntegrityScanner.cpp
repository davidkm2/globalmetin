#include "StdAfx.h"
#include "ThreadEnumerator.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "ProcessNameHelper.h"

typedef uint32_t(NTAPI* TRtlComputeCrc32)(uint32_t dwInitial, const uint8_t* pData, int32_t iLen);
static TRtlComputeCrc32 RtlComputeCrc32 = nullptr;

float GetEntropy(uint8_t* byBuffer, uint32_t dwLength)
{
	uint32_t dwSize = 0;
	int32_t lBuff[0xFF + 1] = { 0 };
	float fTemp, fEntropy = 0;

	for (uint32_t i = 0; i < dwLength; i++)
	{
		lBuff[byBuffer[i]]++;
		dwSize++;
	}

	for (uint32_t i = 0; i < 256; i++)
	{
		if (lBuff[i])
		{
			fTemp = (float)lBuff[i] / (float)dwSize;
			fEntropy += (-fTemp * log2(fTemp));
		}
	}

	return fEntropy;
}

bool CAnticheatManager::IsPackedExecutable(const std::string & szName)
{
	bool bIsPacked = false;

	auto hFile = CreateFileA(szName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (!hFile || hFile == INVALID_HANDLE_VALUE)
	{
		return bIsPacked;
	}

	auto dwFileLen = GetFileSize(hFile, nullptr);
	if (!dwFileLen || dwFileLen == INVALID_FILE_SIZE)
	{
		CloseHandle(hFile);
		return bIsPacked;
	}

	auto pImage = reinterpret_cast<uint8_t*>(malloc(dwFileLen));
	if (!pImage)
	{
		CloseHandle(hFile);
		return bIsPacked;
	}

	DWORD dwReadedBytes;
	BOOL readRet = ReadFile(hFile, pImage, dwFileLen, &dwReadedBytes, nullptr);
	if (!readRet || dwReadedBytes != dwFileLen)
	{
		CloseHandle(hFile);
		free(pImage);
		return bIsPacked;
	}
	CloseHandle(hFile);

	float fEntropy = GetEntropy(pImage, dwFileLen);

	free(pImage);
	return (fEntropy > 7.5f);
}


std::string CAnticheatManager::GetModuleOwnerName(LPVOID pModuleBase)
{
	char szFileName[2048] = { 0 };
	if (!GetMappedFileNameA(GetCurrentProcess(), pModuleBase, szFileName, sizeof(szFileName)))
		return std::string("");

	auto strRealName = DosDevicePath2LogicalPath(szFileName);
	if (strRealName.empty())
		return std::string("");

	std::transform(strRealName.begin(), strRealName.end(), strRealName.begin(), tolower);
	return strRealName;
}

bool GetSectionInformation(const std::string & szSectionName, uint32_t dwBaseAddress, LPDWORD dwOffset, LPDWORD dwLen)
{
	auto pImageDosHeader = (PIMAGE_DOS_HEADER)dwBaseAddress;
	if (!pImageDosHeader || pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	auto pImageNtHeaders = (PIMAGE_NT_HEADERS)(dwBaseAddress + pImageDosHeader->e_lfanew);
	if (!pImageNtHeaders || pImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		return false;

	auto pImageSectionHeader = IMAGE_FIRST_SECTION(pImageNtHeaders);
	if (!pImageSectionHeader)
		return false;

	for (std::size_t i = 0; i < pImageNtHeaders->FileHeader.NumberOfSections; ++i)
	{
		if (!strcmp(szSectionName.c_str(), (char*)pImageSectionHeader[i].Name))
		{
			*dwOffset = uint32_t(dwBaseAddress + pImageSectionHeader[i].VirtualAddress);
			*dwLen = pImageSectionHeader[i].SizeOfRawData;
			return true;
		}
	}
	return false;
}

uint32_t CalculateChecksum(uint32_t dwSectionStart, uint32_t dwSectionLen)
{
	auto dwChecksumResult = 0UL;
	__try
	{
		dwChecksumResult = RtlComputeCrc32(0, (const uint8_t*)dwSectionStart, dwSectionLen);
	}
	__except (1) { }

	return dwChecksumResult;
}


inline bool GetSectionHash(uint32_t dwModuleBase, const std::string & szSectionName, uint32_t * pdwSectionHash)
{
	auto dwSectionBase = 0UL, dwSectionSize = 0UL;
	if (szSectionName == XOR("PEHEADER"))
	{
		dwSectionBase = dwModuleBase;
		dwSectionSize = 0x1000;	
	}
	else
	{
		bool bFindSection = GetSectionInformation(szSectionName, dwModuleBase, &dwSectionBase, &dwSectionSize);
		if (!bFindSection)
		{
#ifdef _DEBUG
			TraceError("Section: %s can not found for: %p", szSectionName.c_str(), dwModuleBase);
#endif
			return false;
		}
	}

	auto szModuleOwner = CAnticheatManager::Instance().GetModuleOwnerName(reinterpret_cast<LPVOID>(dwModuleBase));
	if (szModuleOwner.empty())
	{
#ifdef _DEBUG
		TraceError("Module: %p informations not found! Last error: %u", dwModuleBase, GetLastError());
#endif
		return false;
	}

	if (!dwSectionBase || !dwSectionSize)
	{
#ifdef _DEBUG
		TraceError("Section base address or size is null! Target Module base: %p Section: %s Module: %s",
			dwModuleBase, szSectionName.c_str(), szModuleOwner.c_str());
#endif
		return false;
	}
	
	/*
	if (IsBadReadPtr((LPCVOID)dwSectionBase, dwSectionSize))
	{
#ifdef _DEBUG
		TraceError("Section is not at readable area! Base: %p Size: %p", dwSectionBase, dwSectionSize);
#endif
		return false;
	}
	*/

	MEMORY_BASIC_INFORMATION mbi;
	if (!VirtualQuery((LPCVOID)dwSectionBase, &mbi, sizeof(mbi)))
	{
#ifdef _DEBUG
		TraceError("VirtualQuery fail! Last error: %u", GetLastError());
#endif
		return false;
	}

	if (!(mbi.State & MEM_COMMIT) || (mbi.State & MEM_RELEASE) || (mbi.Protect == PAGE_NOACCESS) || (mbi.Protect & PAGE_GUARD))
	{
#ifdef _DEBUG
		TraceError("Section is not a accesible memory!");
#endif
		return false;
	}

	auto dwOldProtect = 0UL;
	if (!VirtualProtect((LPVOID)dwSectionBase, dwSectionSize, PAGE_EXECUTE_READWRITE, &dwOldProtect))
	{
#ifdef _DEBUG
		TraceError("VirtualProtect(pre) fail! Last error: %u", GetLastError());
#endif
		return false;
	}

	auto dwChecksum = CalculateChecksum(dwSectionBase, dwSectionSize);
	if (!dwChecksum)
	{
#ifdef _DEBUG
		TraceError("Section checksum can not calculated! Last error: %u", GetLastError());
#endif
		return false;
	}

	if (!VirtualProtect((LPVOID)dwSectionBase, dwSectionSize, dwOldProtect, &dwOldProtect))
	{
#ifdef _DEBUG
		TraceError("VirtualProtect(post) fail! Last error: %u", GetLastError());
#endif
		return false;
	}

//	TraceError("Module: %p Section: %s checksum calculated: %p - %p -> (%p)", dwModuleBase, szSectionName.c_str(), dwSectionBase, dwSectionSize, dwChecksum);
	if (pdwSectionHash) *pdwSectionHash = dwChecksum;
	return true;
}


bool InitSectionHash(uint32_t dwModuleBase, std::shared_ptr < SModuleSectionsHashCtx> & pModuleSectionHashContainer)
{
//	TraceError("Section checksum create init started! Target module: %p", dwModuleBase);

	// missing params for check routine
	if (!pModuleSectionHashContainer->dwModuleBase)
		pModuleSectionHashContainer->dwModuleBase = dwModuleBase;

	// identify sections
	if (GetSectionHash(dwModuleBase, XOR("PEHEADER"), &pModuleSectionHashContainer->dwPeHeaderHash) == false)
	{
//		TraceError("GetSectionHash(PEHEADER) fail!");
		if (pModuleSectionHashContainer->dwPeHeaderHash) pModuleSectionHashContainer->dwPeHeaderHash = 0;
	}
	if (GetSectionHash(dwModuleBase, XOR(".text"), &pModuleSectionHashContainer->dwTextHash) == false)
	{
//		TraceError("GetSectionHash(.text) fail!");
		if (pModuleSectionHashContainer->dwTextHash) pModuleSectionHashContainer->dwTextHash = 0;
	}
	if (GetSectionHash(dwModuleBase, XOR(".rdata"), &pModuleSectionHashContainer->dwRDataHash) == false)
	{
//		TraceError("GetSectionHash(.rdata) fail!");
		if (pModuleSectionHashContainer->dwRDataHash) pModuleSectionHashContainer->dwRDataHash = 0;
	}
	if (GetSectionHash(dwModuleBase, XOR(".edata"), &pModuleSectionHashContainer->dwEDataHash) == false)
	{
//		TraceError("GetSectionHash(.edata) fail!");
		if (pModuleSectionHashContainer->dwEDataHash) pModuleSectionHashContainer->dwEDataHash = 0;
	}
	if (GetSectionHash(dwModuleBase, XOR(".rsrc"), &pModuleSectionHashContainer->dwRsrcHash) == false)
	{
//		TraceError("GetSectionHash(.rsrc) fail!");
		if (pModuleSectionHashContainer->dwRsrcHash) pModuleSectionHashContainer->dwRsrcHash = 0;
	}
	if (GetSectionHash(dwModuleBase, XOR(".reloc"), &pModuleSectionHashContainer->dwRelocHash) == false)
	{
//		TraceError("GetSectionHash(.reloc) fail!");
		if (pModuleSectionHashContainer->dwRelocHash) pModuleSectionHashContainer->dwRelocHash = 0;
	}

#if 0
	TraceError("Section checksum calculate completed! result: PE(%p)Text(%p)Rdata(%p)Edata(%p)Rsrc(%p)Reloc(%p)", 
		pModuleSectionHashContainer->dwPeHeaderHash,
		pModuleSectionHashContainer->dwTextHash,
		pModuleSectionHashContainer->dwRDataHash,
		pModuleSectionHashContainer->dwEDataHash,
		pModuleSectionHashContainer->dwRsrcHash,
		pModuleSectionHashContainer->dwRelocHash
	);
#endif

	return true;
}

typedef struct _ldr_enum_context
{
	bool bCheck;
	int32_t iViolationIndex;
	WCHAR wszViolationOwnModule[1024];
} SLdrEnumContext, *PLdrEnumContext;
VOID NTAPI LdrEnumCallBack(IN PLDR_DATA_TABLE_ENTRY_MY DataTableEntry, IN PVOID Context, IN OUT BOOLEAN *StopEnumeration)
{
	auto pldrContext = (PLdrEnumContext)Context;
	if (!pldrContext)
	{
		*StopEnumeration = TRUE;
		return;
	}

	auto vMonitoredModules = CAnticheatManager::Instance().GetMonitoredModuleContainer();
	if (std::find(vMonitoredModules.begin(), vMonitoredModules.end(), reinterpret_cast<HMODULE>(DataTableEntry->DllBase)) == vMonitoredModules.end())
		return;

	if (pldrContext->bCheck == false) // First step: Initilization
	{
		auto pModuleSectionHashContainer = std::make_shared<SModuleSectionsHashCtx>();
		if (InitSectionHash((uint32_t)DataTableEntry->DllBase, pModuleSectionHashContainer))
		{
//			TraceError("Module: %ls(%p) section hash list succesfully generated!", DataTableEntry->FullDllName.Buffer, DataTableEntry->DllBase);

			pModuleSectionHashContainer->dwModuleBase = (uint32_t)DataTableEntry->DllBase;
			wcscpy_s(pModuleSectionHashContainer->wszModuleName, DataTableEntry->FullDllName.Buffer);

			CAnticheatManager::Instance().SaveModuleSectionHashList(pModuleSectionHashContainer);
		}
	}
	else
	{
		auto pCurrentHashList = std::make_shared<SModuleSectionsHashCtx>();
		if (InitSectionHash((uint32_t)DataTableEntry->DllBase, pCurrentHashList))
		{
			auto pCorrectHashContainer = CAnticheatManager::Instance().GetModuleSectionHashContainer();
			for (const auto & pCorrectHashList : pCorrectHashContainer)
			{
				if (pCorrectHashList->dwModuleBase == (uint32_t)DataTableEntry->DllBase)
				{
					if (pCurrentHashList->dwPeHeaderHash && pCorrectHashList->dwPeHeaderHash && pCurrentHashList->dwPeHeaderHash != pCorrectHashList->dwPeHeaderHash)
					{
#ifdef _DEBUG
						TraceError("PE Header checksum violation! Curr: %p - Corr: %p", pCurrentHashList->dwPeHeaderHash, pCorrectHashList->dwPeHeaderHash);
#endif
						pldrContext->iViolationIndex = 1;
						wcscpy_s(pldrContext->wszViolationOwnModule, pCorrectHashList->wszModuleName);
					}

					else if (pCurrentHashList->dwTextHash && pCorrectHashList->dwTextHash && pCurrentHashList->dwTextHash != pCorrectHashList->dwTextHash)
					{
#ifdef _DEBUG
						TraceError("Text section checksum violation! Curr: %p - Corr: %p", pCurrentHashList->dwTextHash, pCorrectHashList->dwTextHash);
#endif
						pldrContext->iViolationIndex = 2;
						wcscpy_s(pldrContext->wszViolationOwnModule, pCorrectHashList->wszModuleName);
					}

					else if (pCurrentHashList->dwRDataHash && pCorrectHashList->dwRDataHash && pCurrentHashList->dwRDataHash != pCorrectHashList->dwRDataHash)
					{
#ifdef _DEBUG
						TraceError("rdata section checksum violation! Curr: %p - Corr: %p", pCurrentHashList->dwRDataHash, pCorrectHashList->dwRDataHash);
#endif
						pldrContext->iViolationIndex = 3;
						wcscpy_s(pldrContext->wszViolationOwnModule, pCorrectHashList->wszModuleName);
					}

					else if (pCurrentHashList->dwEDataHash && pCorrectHashList->dwEDataHash && pCurrentHashList->dwEDataHash != pCorrectHashList->dwEDataHash)
					{
#ifdef _DEBUG
						TraceError("edata section checksum violation! Curr: %p - Corr: %p", pCurrentHashList->dwEDataHash, pCorrectHashList->dwEDataHash);
#endif
						pldrContext->iViolationIndex = 4;
						wcscpy_s(pldrContext->wszViolationOwnModule, pCorrectHashList->wszModuleName);
					}

					else if (pCurrentHashList->dwRsrcHash && pCorrectHashList->dwRsrcHash && pCurrentHashList->dwRsrcHash != pCorrectHashList->dwRsrcHash)
					{
#ifdef _DEBUG
						TraceError("rsrc section checksum violation! Curr: %p - Corr: %p", pCurrentHashList->dwRsrcHash, pCorrectHashList->dwRsrcHash);
#endif
						pldrContext->iViolationIndex = 5;
						wcscpy_s(pldrContext->wszViolationOwnModule, pCorrectHashList->wszModuleName);
					}

					else if (pCurrentHashList->dwRelocHash && pCorrectHashList->dwRelocHash && pCurrentHashList->dwRelocHash != pCorrectHashList->dwRelocHash)
					{
#ifdef _DEBUG
						TraceError("reloc section checksum violation! Curr: %p - Corr: %p", pCurrentHashList->dwRelocHash, pCorrectHashList->dwRelocHash);
#endif
						pldrContext->iViolationIndex = 6;
						wcscpy_s(pldrContext->wszViolationOwnModule, pCorrectHashList->wszModuleName);
					}

					else
					{
						pldrContext->iViolationIndex = 0;
					}
				}

//				TraceError("Section integrity scan completed for: %p(%ls)", pCorrectHashList->dwModuleBase, pCorrectHashList->wszModuleName);
			}
		}
	}
}

void CAnticheatManager::InitSectionHashes()
{
	auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
	if (!hNtdll)
		return;

	if (!RtlComputeCrc32)
	{
		auto pRtlComputeCrc32 = GetProcAddress(hNtdll, XOR("RtlComputeCrc32"));
		if (pRtlComputeCrc32)
			RtlComputeCrc32 = (TRtlComputeCrc32)pRtlComputeCrc32;
	}
	if (!RtlComputeCrc32)
		return;

	typedef VOID(NTAPI LDR_ENUM_CALLBACK)(_In_ PLDR_DATA_TABLE_ENTRY_MY ModuleInformation, _In_ PVOID Parameter, _Out_ BOOLEAN *Stop);
	typedef NTSTATUS(NTAPI* TLdrEnumerateLoadedModules)(IN BOOLEAN ReservedFlag, IN LDR_ENUM_CALLBACK* EnumProc, IN PVOID Context);


	auto pLdrEnumerateLoadedModules = GetProcAddress(hNtdll, XOR("LdrEnumerateLoadedModules"));
	if (!pLdrEnumerateLoadedModules)
		return;
	auto LdrEnumerateLoadedModules = (TLdrEnumerateLoadedModules)pLdrEnumerateLoadedModules;

	SLdrEnumContext	ldrEnumContext;
	ldrEnumContext.bCheck = false;
	LdrEnumerateLoadedModules(0, LdrEnumCallBack, &ldrEnumContext);
}

bool CAnticheatManager::HasCorruptedModuleSection()
{
	typedef VOID(NTAPI LDR_ENUM_CALLBACK)(_In_ PLDR_DATA_TABLE_ENTRY_MY ModuleInformation, _In_ PVOID Parameter, _Out_ BOOLEAN *Stop);
	typedef NTSTATUS(NTAPI* TLdrEnumerateLoadedModules)(IN BOOLEAN ReservedFlag, IN LDR_ENUM_CALLBACK* EnumProc, IN PVOID Context);

	static TLdrEnumerateLoadedModules LdrEnumerateLoadedModules = nullptr;
	if (!LdrEnumerateLoadedModules)
	{
		auto hNtdll = LoadLibraryA(XOR("ntdll.dll"));
		if (hNtdll)
		{
			auto pLdrEnumerateLoadedModules = GetProcAddress(hNtdll, XOR("LdrEnumerateLoadedModules"));
			if (pLdrEnumerateLoadedModules)
				LdrEnumerateLoadedModules = (TLdrEnumerateLoadedModules)pLdrEnumerateLoadedModules;
		}
	}
	if (!LdrEnumerateLoadedModules)
		return false;

	SLdrEnumContext	ldrEnumContext;
	ldrEnumContext.bCheck = true;
	LdrEnumerateLoadedModules(0, LdrEnumCallBack, &ldrEnumContext);

	if (ldrEnumContext.iViolationIndex > 0)
	{
		auto wstrViolationOwner = std::wstring(ldrEnumContext.wszViolationOwnModule);
		auto strViolationOwner = std::string(wstrViolationOwner.begin(), wstrViolationOwner.end());

		CCheatDetectQueueMgr::Instance().AppendDetection(MODULE_SECTION_INTEGRITY_CORRUPTED, ldrEnumContext.iViolationIndex, strViolationOwner);
		ExitByAnticheat(MODULE_SECTION_INTEGRITY_CORRUPTED, ldrEnumContext.iViolationIndex, 0, true, strViolationOwner);
		return true;
	}

	return false;
}

inline std::string GetCurrentPath()
{
	char szCurrentProcessName[MAX_PATH] = { 0 };
	GetModuleFileNameA(nullptr, szCurrentProcessName, MAX_PATH);

	auto strCurrentProcessName = std::string(szCurrentProcessName);
	auto pos = strCurrentProcessName.find_last_of("\\/");
	return strCurrentProcessName.substr(0, pos);
}

void CAnticheatManager::SetupSectionMonitorContainer()
{
	char szWinPath[MAX_PATH] = { 0 };
	GetWindowsDirectoryA(szWinPath, MAX_PATH);

	std::string strLowerWinPath(szWinPath);
	std::transform(strLowerWinPath.begin(), strLowerWinPath.end(), strLowerWinPath.begin(), tolower);

	std::string strLowerCurrentPath = GetCurrentPath();
	std::transform(strLowerCurrentPath.begin(), strLowerCurrentPath.end(), strLowerCurrentPath.begin(), tolower);

	auto hProcessBase = GetModuleHandleA(0);
	if (hProcessBase)
	{
		auto strModuleName = GetModuleOwnerName(hProcessBase);

		if (strModuleName.empty())
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_BASE, XOR("1"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_BASE, 1, true);
		}
		else if (IsPackedExecutable(strModuleName))
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_BASE, XOR("2"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_BASE, 2, true);
		}
		else
		{
			m_vMonitoredModules.push_back(hProcessBase);
		}
	}

	auto hKernel32 = GetModuleHandleA(XOR("kernel32.dll"));
	if (hKernel32)
	{
		auto strModuleName = GetModuleOwnerName(hKernel32);

		if (strModuleName.empty())
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_KERNEL32, XOR("1"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_KERNEL32, 1, true);
		}
		else if (IsPackedExecutable(strModuleName))
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_KERNEL32, XOR("2"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_KERNEL32, 2, true);
		}
		else if (strModuleName.find(strLowerWinPath) == std::string::npos)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_KERNEL32, XOR("3"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_KERNEL32, 3, true);
		}
		else
		{
			m_vMonitoredModules.push_back(hKernel32);
		}
	}

	auto hKernelBase = GetModuleHandleA(XOR("kernelbase.dll"));
	if (hKernelBase)
	{
		auto strModuleName = GetModuleOwnerName(hKernelBase);

		if (strModuleName.empty())
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_KERNELBASE, XOR("1"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_KERNELBASE, 1, true);
		}
		else if (IsPackedExecutable(strModuleName))
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_KERNELBASE, XOR("2"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_KERNELBASE, 2, true);
		}
		else if (strModuleName.find(strLowerWinPath) == std::string::npos)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_KERNELBASE, XOR("3"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_KERNELBASE, 3, true);
		}
		else
		{
			m_vMonitoredModules.push_back(hKernelBase);
		}
	}

	auto hNtdll = GetModuleHandleA(XOR("ntdll.dll"));
	if (hNtdll)
	{
		auto strModuleName = GetModuleOwnerName(hNtdll);

		if (strModuleName.empty())
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_NTDLL, XOR("1"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_NTDLL, 1, true);
		}
		else if (IsPackedExecutable(strModuleName))
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_NTDLL, XOR("2"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_NTDLL, 2, true);
		}
		else if (strModuleName.find(strLowerWinPath) == std::string::npos)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_NTDLL, XOR("3"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_NTDLL, 3, true);
		}
		else
		{
			m_vMonitoredModules.push_back(hNtdll);
		}
	}

	auto hUser32 = GetModuleHandleA(XOR("user32.dll"));
	if (hUser32)
	{
		auto strModuleName = GetModuleOwnerName(hUser32);

		if (strModuleName.empty())
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_USER32, XOR("1"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_USER32, 1, true);
		}
		else if (IsPackedExecutable(strModuleName))
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_USER32, XOR("2"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_USER32, 2, true);
		}
		else if (strModuleName.find(strLowerWinPath) == std::string::npos)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_USER32, XOR("3"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_USER32, 3, true);
		}
		else
		{
			m_vMonitoredModules.push_back(hUser32);
		}
	}

	auto hWin32u = GetModuleHandleA(XOR("win32u.dll"));
	if (hWin32u)
	{
		auto strModuleName = GetModuleOwnerName(hWin32u);

		if (strModuleName.empty())
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_WIN32U, XOR("1"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_WIN32U, 1, true);
		}
		else if (IsPackedExecutable(strModuleName))
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_WIN32U, XOR("2"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_WIN32U, 2, true);
		}
		else if (strModuleName.find(strLowerWinPath) == std::string::npos)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_WIN32U, XOR("3"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_WIN32U, 3, true);
		}
		else
		{
			m_vMonitoredModules.push_back(hWin32u);
		}
	}

	auto hPython27 = GetModuleHandleA(XOR("python27.dll"));
	if (hPython27)
	{
		auto strModuleName = GetModuleOwnerName(hPython27);

		if (strModuleName.empty())
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_PYTHON27, XOR("1"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_PYTHON27, 1, true);
		}
		else if (IsPackedExecutable(strModuleName))
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(UNKNOWN_MONITORED_MODULE, MMODULE_PYTHON27, XOR("2"));
			ExitByAnticheat(UNKNOWN_MONITORED_MODULE, MMODULE_PYTHON27, 2, true);
		}
		else
		{
			m_vMonitoredModules.push_back(hPython27);
		}
	}
}

