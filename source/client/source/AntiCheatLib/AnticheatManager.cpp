#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "../EterBase/SimpleTimer.h"
#include "../EterBase/MappedFile.h"
#include "../UserInterface/Locale_inc.h"

#include <XORstr.h>
#include <Wtsapi32.h>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/property_tree/ptree_serialization.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/json_parser/detail/read.hpp>

#include "DataBuffer.h"
#include "md5.h"
#include "Exception.h"
#include "ProtectionMacros.h"

static auto gs_pWatchdogCheckTimer = std::make_unique <CSimpleTimer <std::chrono::milliseconds> >();

void NTAPI OnSyncWait(PVOID pParam, BOOLEAN)
{
	static auto bBlocked = false;
	if (bBlocked)
		return;

	if (gs_pWatchdogCheckTimer->diff() > 30000)
	{
		if (CAnticheatManager::Instance().GetWatchdogStatus() == false)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(WATCHDOG_SIGNAL_LOST, 0, "");
			bBlocked = true;
		}
		CAnticheatManager::Instance().SendWatchdogSignal(false);

		gs_pWatchdogCheckTimer->reset();
	}
}


CAnticheatManager::CAnticheatManager() :
	m_hWndCheckThread(INVALID_HANDLE_VALUE), m_hWndThreadWaitObj(INVALID_HANDLE_VALUE),
	m_hSyncProc(INVALID_HANDLE_VALUE), m_hSyncWaitObj(INVALID_HANDLE_VALUE),
	m_pDumpGuardMem(nullptr)
{
}

void CAnticheatManager::InitializeAnticheatRoutines(HINSTANCE hInstance, HWND hWnd)
{
#ifndef ENABLE_ANTICHEAT
	return;
#endif

#if !defined(_DEBUG)
#if USE_ENIGMA_SDK == 1
	if (EP_CheckupIsProtected() && EP_CheckupIsEnigmaOk() == FALSE)
#elif USE_VMPROTECT_SDK == 1
	if (VMProtectIsValidImageCRC() == FALSE)
#elif USE_SHIELDEN_SDK == 1
	if (SECheckProtection() == FALSE)
#else
	if (false)
#endif
	{
		ExitByAnticheat(PROTECTION_CRACK, 0, 0, true);
		return;
	}
#endif

	// InitializeBuiltinModuleList

	m_bAggressiveMode = ENABLE_ANTICHEAT_AGGRESSIVE_MODE;

	m_pQuerantineMgr = std::make_shared<CCheatQuarentineMgr>();
	m_pWmiMonitorMgr = std::make_shared<CWMI_Monitor>();

	m_hInstance = hInstance;
	m_hWnd = hWnd;
	m_dwMainThreadId = GetCurrentThreadId();
	m_wndpOldProc = nullptr;
	m_hkMessageHook = nullptr;

 	m_mapKeyStatusList[0] = false;
 	m_mapKeyStatusList[1] = false;
 	m_mapKeyStatusList[2] = false;

	SetupSectionMonitorContainer();

#if !defined(_DEBUG) && !defined(ENABLE_DEBUGGER_ALLOW_BY_ANTICHEAT)
	SetAntiTrace(GetCurrentThread()); // TODO: Check return
	CheckExceptionTrap();
	SimpleDebugCheck();
	DebugCheckBugControl();
	DebugObjectHandleCheck();
	SharedUserDataCheck();
	GlobalFlagsCheck();
#endif

	CheckMainFolderFiles();
	CheckMilesFolderForMilesPlugins();
	CheckLibFolderForPythonLibs();
	CheckYmirFolder();

	CheckMappedFiles();

	BlockAccess(GetCurrentProcess());
	BlockAccess(GetCurrentThread());
	DenyProcessAccess();
	DenyThreadAccess();

	InitializeExceptionFilters();
///	InitializeDumpProtection();
	InitializeAnticheatWatchdog();

	/*
	// TODO: Add minimize and restore messages for block keyboard stuck
	InitializeWindowMessageHook();
	InitializeRawWindowHook();
	*/

	LoadDllFilter();

	ParentProcessCheck(""); // TODO: Patcher name

	InitializeWindowWatcherThread();
	m_pWmiMonitorMgr->InitializeWMIWatcherThread();

	InitSectionHashes();
	CreateProcessHashList();

	m_hSyncProc = OpenProcess(SYNCHRONIZE, FALSE, GetCurrentProcessId());
	if (m_hSyncProc && m_hSyncProc != INVALID_HANDLE_VALUE)
	{
		gs_pWatchdogCheckTimer->reset();

		auto bRegisterWaitRet = RegisterWaitForSingleObject(&m_hSyncWaitObj, m_hSyncProc, OnSyncWait, nullptr, 12000, WT_EXECUTEDEFAULT);
		if (bRegisterWaitRet == false)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(SYNC_PROC_REGISTER_FAIL, GetLastError(), "");
		}
	}
}

void CAnticheatManager::FinalizeAnticheatRoutines()
{
#ifndef ENABLE_ANTICHEAT
	return;
#endif

	if (m_hSyncWaitObj && m_hSyncWaitObj != INVALID_HANDLE_VALUE)
		UnregisterWait(m_hSyncWaitObj);
	if (m_hSyncProc && m_hSyncProc != INVALID_HANDLE_VALUE)
		CloseHandle(m_hSyncProc);

	RemoveAnticheatWatchdog();
	RemoveExceptionHandlers();

	DestroyWindowWatcher();
	if (m_pWmiMonitorMgr)
		m_pWmiMonitorMgr->DestroyWMIMonitor();

	/*
	DestroyWindowMessageHook();
	DestroyRawWindowHook();
	*/
}


void CAnticheatManager::ParseCheatBlacklist(const std::string & szContent)
{
	int32_t iScanID = -1;

	try
	{
		std::istringstream iss(szContent);

		boost::property_tree::ptree root;
		boost::property_tree::json_parser::read_json(iss, root);

		/*
		for (ptree::const_iterator v = root.begin(); v != root.end(); ++v)
		{
			std::string key = v->first;
			auto value = v->second.get_value<std::string>();
			TraceError("key: %s value %s", key.c_str(), value.c_str());
		}
		*/

		auto headerTree = root.get_child(XOR("Header"));
		if (headerTree.empty())
		{
#ifdef _DEBUG
			TraceError("Blacklist header not found!");
#endif
			return;
		}

		auto scanId = headerTree.get<std::string>(XOR("ScanID"));
		auto rowCount = headerTree.get<std::string>(XOR("RowCount"));
		auto fieldCount = headerTree.get<std::string>(XOR("FieldCount"));

		if (scanId.empty() || rowCount.empty() || fieldCount.empty())
		{
#ifdef _DEBUG
			TraceError("Blacklist header context not found! ID: %s Row: %s Field: %s", scanId.c_str(), rowCount.c_str(), fieldCount.c_str());
#endif
			return;
		}

#ifdef _DEBUG
		TraceError("Blacklist process started! ScanID: %s Row Count: %s Field Count: %s", scanId.c_str(), rowCount.c_str(), fieldCount.c_str());
#endif

		iScanID = std::stoi(scanId);
		if (!(iScanID > NET_SCAN_ID_NULL && iScanID < NET_SCAN_ID_MAX))
		{
#ifdef _DEBUG
			TraceError("Unallowed scan ID: %d", iScanID);
#endif
			return;
		}

		auto nRowCount = std::stoi(rowCount);
		for (auto i = 1; i <= nRowCount; ++i)
		{
//			TraceError("Current row idx: %d", i);

			auto strCurrNode = std::to_string(i);
			auto currNode = root.get_child(strCurrNode);
			if (currNode.empty())
			{
#ifdef _DEBUG
				TraceError("Current node: %s is empty!", strCurrNode.c_str());
#endif
				continue;
			}

			switch (iScanID)
			{
				case NET_SCAN_ID_PROCESS_HASH:
				{
					auto strMd5Hash = currNode.get<std::string>(XOR("Md5Hash"));

	//				TraceError("Node process started! ScanID: %d Row: %u Hash: %s", iScanID, i, strMd5Hash.c_str());

					CCheatQuarentineMgr::Instance().AppendProcessMd5(strCurrNode, strMd5Hash, true);

	//				TraceError("Current node appended to quarentine!");

				} break;

				default:
	#ifdef _DEBUG
					TraceError("Unknown scan ID: %d", iScanID);
	#endif
					break;
			}
		}
	}
	catch (const std::exception & e)
	{
#ifdef _DEBUG
		TraceError("Blacklist json parse exception: %s", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}


	if (iScanID != -1)
	{
		CCheatQuarentineMgr::Instance().ProcessQuarentineList(iScanID, false);
	}
}


uint32_t CAnticheatManager::ServiceMessageBox(const std::string & szTitle, const std::string & szMessage, uint16_t wType)
{
	BOOL bRet = FALSE;
	auto dwResponse = 0UL;

	bRet = WTSSendMessageA(
		WTS_CURRENT_SERVER_HANDLE,							// hServer
		WTSGetActiveConsoleSessionId(),						// ID for the console seesion (1)
		const_cast<LPSTR>(szTitle.c_str()),					// MessageBox Caption
		static_cast<uint32_t>(strlen(szTitle.c_str())),		// Length of caption
		const_cast<LPSTR>(szMessage.c_str()),				// MessageBox Text
		static_cast<uint32_t>(strlen(szMessage.c_str())),		// Length of text
		wType,												// Buttons, etc
		0,													// Timeout period in seconds (0 = infinite)
		&dwResponse,										// What button was clicked (if bWait == TRUE)
		FALSE												// bWait - Blocks until user click
	);

	return dwResponse;
}

bool CAnticheatManager::IsFileExist(const std::string & szFileName)
{
	auto dwAttrib = GetFileAttributesA(szFileName.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool CAnticheatManager::IsDirExist(const std::string & szDirName)
{
	auto dwAttrib = GetFileAttributesA(szDirName.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


std::string CAnticheatManager::GetMd5(const uint8_t* pData, std::size_t nSize)
{
	auto strHash = std::string("");

	try
	{
		MD5 md5buf;
		md5buf.add(pData, nSize);
		md5buf.calculate();

		strHash = md5buf.get_hash();
	}
	catch (Exception & e)
	{
#ifdef _DEBUG
		TraceError("CMd5|BuildHash exception! Data: %p Size: %u Reason: %s", pData, nSize, e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
	catch (...)
	{
#ifdef _DEBUG
		TraceError("CMd5|BuildHash unhandled exception!");
#endif
	}

	std::transform(strHash.begin(), strHash.end(), strHash.begin(), tolower);
	return strHash;
}

std::string CAnticheatManager::GetFileMd5(const std::string & szName)
{
	std::string szOutput;

	__PROTECTOR_START__("GetMd5")
	CMappedFile pMappedFile;
	LPCVOID pMappedFileData;
	if (!pMappedFile.Create(szName.c_str(), &pMappedFileData, 0, 0))
		return szOutput;

	szOutput = GetMd5((const uint8_t*)pMappedFileData, pMappedFile.Size());
	__PROTECTOR_END__("GetMd5")

	pMappedFile.Destroy();
	return szOutput;
}

void CAnticheatManager::ExitByAnticheat(uint32_t dwErrorCode, uint32_t dwSubCode1, uint32_t dwSubCode2, bool bForceClose, const std::string& stSubMessage)
{
	__PROTECTOR_START__("ExitByAnticheat")
	if (m_bAggressiveMode || bForceClose)
	{
		auto stMessage = XOR("Anticheat violation detected! Error code: ") +
			std::to_string(dwErrorCode) + "-" + std::to_string(dwSubCode1) + "-" + std::to_string(dwSubCode2) +
			" " + stSubMessage;

		TraceError(stMessage.c_str());
		ServiceMessageBox(XOR("Anticheat"), stMessage, MB_ICONERROR);
		abort();
	}
	__PROTECTOR_END__("ExitByAnticheat")
}
