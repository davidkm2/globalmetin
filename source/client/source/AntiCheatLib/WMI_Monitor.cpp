#include "StdAfx.h"
#include "WMI_Monitor.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "ProcessNameHelper.h"

#include <XORstr.h>

#include <OleAuto.h>

// FIXME: In some client the thread terminate itself due than unknown reasons(s) check it
// TODO: Check WMI service status before than initilization

void NTAPI OnWMIMonitorThreadLost(PVOID pParam, BOOLEAN)
{
	CCheatDetectQueueMgr::Instance().AppendDetection(WMI_MONITOR_THREAD_LOST, 0, "");
}


bool DumpWMIClassObject(int32_t iAnalyseType, IWbemClassObject * pClassObject, int32_t iIndentationLevel)
{
	auto mDataMap = std::map<std::string, std::string>();

	SAFEARRAY * pStrNames;
	auto hError = pClassObject->GetNames(nullptr, WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, nullptr, &pStrNames);
	if (FAILED(hError))
		return false;

	auto lLowerBound = 0L;
	hError = SafeArrayGetLBound(pStrNames, 1, &lLowerBound);
	if (FAILED(hError))
		return false;

	auto lUpperBound = 0L;
	hError = SafeArrayGetUBound(pStrNames, 1, &lUpperBound);
	if (FAILED(hError))
		return false;

	auto lElementCount = lUpperBound - lLowerBound + 1;
	if (!lElementCount)
		return false;

	for (auto i = 0L; i < lElementCount; i++)
	{
		auto bszName = ((BSTR*)pStrNames->pvData)[i];

		VARIANT varVal = { 0 };
		CIMTYPE cymType = 0;
		hError = pClassObject->Get(bszName, 0, &varVal, &cymType, nullptr);
		if (SUCCEEDED(hError))
		{
			WCHAR __TargetInstance[] = { L'T', L'a', L'r', L'g', L'e', L't', L'I', L'n', L's', L't', L'a', L'n', L'c', L'e', L'\0' }; // TargetInstance
			if (wcscmp(bszName, __TargetInstance) == 0)
			{
				DumpWMIClassObject(iAnalyseType, reinterpret_cast<IWbemClassObject*>(varVal.uintVal), iIndentationLevel + 1);
			}

			else if (cymType == CIM_STRING)
			{
				if (varVal.bstrVal)
				{
					std::wstring wszName(bszName, SysStringLen(bszName));
					std::string szName(wszName.begin(), wszName.end());

					std::wstring wszValue(varVal.bstrVal, SysStringLen(varVal.bstrVal));
					std::string szValue(wszValue.begin(), wszValue.end());

					mDataMap[szName] = szValue;
				}
			}

			else
			{
				std::wstring wszName(bszName, SysStringLen(bszName));
				std::string szName(wszName.begin(), wszName.end());

				mDataMap[szName] = std::to_string(varVal.uintVal);
			}
		}
	}

	CWMI_Monitor::Instance().OnWMIMessage(iAnalyseType, mDataMap);
	return true;
}



CWMI_Monitor::CWMI_Monitor() :
	m_pWbemServices(nullptr), m_bTerminated(false)
{
}
CWMI_Monitor::~CWMI_Monitor()
{
}

void CWMI_Monitor::OnWMIMessage(int32_t analyseType, std::map <std::string /* szType */, std::string /* szValue */> mDataMap)
{
#if 0
	TraceError("Analyse type: %d", analyseType);

	for (const auto & currentElement : mDataMap)
	{
		auto szType = std::string(currentElement.first.begin(), currentElement.first.end());
		auto szValue = std::string(currentElement.second.begin(), currentElement.second.end());

		TraceError("Type: %s Value: %s", szType.c_str(), szValue.c_str());
	}
#endif

	switch (analyseType)
	{
		case EAnalyseTypes::ANALYSE_PROCESS:
		{
			//"SessionId"
			//"ThreadCount"
			//"CreationClassName"
			//"CommandLine"
			//"ExecutablePath"
			//"Name"

			auto it_dwProcessID = mDataMap.find(XOR("ProcessId"));

			if (it_dwProcessID != mDataMap.end())
			{
				auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, std::stoul(it_dwProcessID->second));
				if (hProcess && hProcess != INVALID_HANDLE_VALUE)
				{
					auto strProcessName = GetProcessName(hProcess);
					if (strProcessName.size() > 0)
					{
						if (CAnticheatManager::Instance().IsFileExist(strProcessName))
						{
							auto strMd5 = CAnticheatManager::Instance().GetFileMd5(strProcessName);
							if (strMd5.size() > 0)
							{
								CCheatQuarentineMgr::Instance().AppendProcessMd5(strProcessName, strMd5, false);
								CCheatQuarentineMgr::Instance().ProcessQuarentineList(NET_SCAN_ID_PROCESS_HASH, true);
							}
						}
					}

					CloseHandle(hProcess);
				}
			}

		} break;

		case EAnalyseTypes::ANALYSE_THREAD:
		{
			//"WaitMode"

			auto it_dwTID			= mDataMap.find(XOR("ThreadID"));
			auto it_dwPID			= mDataMap.find(XOR("ProcessID"));
			auto it_dwStartAddress	= mDataMap.find(XOR("Win32StartAddr"));

			if (it_dwTID != mDataMap.end() && it_dwPID != mDataMap.end() && it_dwStartAddress != mDataMap.end())
			{
				if (GetCurrentProcessId() == std::stoul(it_dwPID->second))
				{
					auto dwViolationID = 0UL;
					auto strViolationOwner = std::string("");
					if (CAnticheatManager::Instance().IsBlockedStartAddress(std::stoul(it_dwTID->second), std::stoul(it_dwStartAddress->second), &dwViolationID, &strViolationOwner))
					{
						CCheatDetectQueueMgr::Instance().AppendDetection(BLOCKED_THREAD_START_ADDRESS, dwViolationID, strViolationOwner);
					}
				}
			}

		} break;

		case EAnalyseTypes::ANALYSE_MODULE:
		{
			//"ProcessID"
			//"ImageBase"
			//"ImageSize"
			//"FileName"

		} break;

		case EAnalyseTypes::ANALYSE_DRIVER:
		{
			//"Name"
			//"PathName"
			//"State"
			//"ServiceType"
			//"Started"
		} break;

		case EAnalyseTypes::ANALYSE_SERVICE:
		{

		} break;
	}
}

DWORD CWMI_Monitor::ThreadRoutine(void)
{
	std::string szWQL = XOR("WQL");
	auto wszWQL = std::wstring(szWQL.begin(), szWQL.end());
	auto wszWQL2 = (wchar_t*)wszWQL.c_str();

	std::string szRootCim = XOR("root\\cimv2");
	auto wszRootCim = std::wstring(szRootCim.begin(), szRootCim.end());
	auto wszRootCim2 = (wchar_t*)wszRootCim.c_str();

	std::string szProcessCreateQuery = XOR("SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'");
	auto wszProcessCreateQuery = std::wstring(szProcessCreateQuery.begin(), szProcessCreateQuery.end());
	auto wszProcessCreateQuery2 = (wchar_t*)wszProcessCreateQuery.c_str();

	std::string szThreadLoadQuery = XOR("SELECT * FROM Win32_ThreadTrace");
	auto wszThreadLoadQuery = std::wstring(szThreadLoadQuery.begin(), szThreadLoadQuery.end());
	auto wszThreadLoadQuery2 = (wchar_t*)wszThreadLoadQuery.c_str();

	std::string szModuleLoadQuery = XOR("SELECT * FROM Win32_ModuleLoadTrace");
	auto wszModuleLoadQuery = std::wstring(szModuleLoadQuery.begin(), szModuleLoadQuery.end());
	auto wszModuleLoadQuery2 = (wchar_t*)wszModuleLoadQuery.c_str();

	std::string szDriverLoadQuery = XOR("SELECT * FROM __InstanceCreationEvent WITHIN 5 WHERE TargetInstance ISA 'Win32_SystemDriver'");
	auto wszDriverLoadQuery = std::wstring(szDriverLoadQuery.begin(), szDriverLoadQuery.end());
	auto wszDriverLoadQuery2 = (wchar_t*)wszDriverLoadQuery.c_str();

	std::string szServiceLoadQuery = XOR("SELECT * FROM __InstanceCreationEvent WITHIN 5 WHERE TargetInstance ISA 'Win32_Service'");
	auto wszServiceLoadQuery = std::wstring(szServiceLoadQuery.begin(), szServiceLoadQuery.end());
	auto wszServiceLoadQuery2 = (wchar_t*)wszServiceLoadQuery.c_str();
	
	// ---

	auto hError = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hError))
	{
#ifdef _DEBUG
		TraceError("CoInitializeEx fail! hError: %p Last error: %u", hError, GetLastError());
#endif
		return 0;
	}

	hError = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT /* RPC_C_AUTHN_LEVEL_PKT */, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
	if (FAILED(hError))
	{
#ifdef _DEBUG
		TraceError("CoInitializeSecurity fail! hError: %p Last error: %u", hError, GetLastError());
#endif
		return 0;
	}

	IWbemLocator * pLoc = nullptr;
	hError = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hError))
	{
#ifdef _DEBUG
		TraceError("CoCreateInstance fail! hError: %p Last error: %u", hError, GetLastError());
#endif
		return 0;
	}

	hError = pLoc->ConnectServer(wszRootCim2, nullptr, nullptr, nullptr, WBEM_FLAG_CONNECT_USE_MAX_WAIT /* 0 */, nullptr, nullptr, &m_pWbemServices);
	if (FAILED(hError))
	{
#ifdef _DEBUG
		TraceError("ConnectServer fail! hError: %p Last error: %u", hError, GetLastError());
#endif
		return 0;
	}

#ifdef _DEBUG
	Tracef("WMI Watcher created!");
#endif

	EventSink eventProcCreated(EAnalyseTypes::ANALYSE_PROCESS);
	hError = m_pWbemServices->ExecNotificationQueryAsync(wszWQL2, wszProcessCreateQuery2, WBEM_FLAG_SEND_STATUS, nullptr, &eventProcCreated);
	if (FAILED(hError))
	{
#ifdef _DEBUG
		TraceError("ExecNotificationQueryAsync(%d) fail! hError: %p Last error: %u", EAnalyseTypes::ANALYSE_PROCESS, hError, GetLastError());
#endif
		return 0;
	}

	EventSink eventThreadCreated(EAnalyseTypes::ANALYSE_THREAD);
	hError = m_pWbemServices->ExecNotificationQueryAsync(wszWQL2, wszThreadLoadQuery2, 0, nullptr, &eventThreadCreated);
	if (FAILED(hError))
	{
#ifdef _DEBUG
		TraceError("ExecNotificationQueryAsync(%d) fail! hError: %p Last error: %u", EAnalyseTypes::ANALYSE_THREAD, hError, GetLastError());
#endif
		return 0;
	}

#if 0
	EventSink eventModuleLoaded(EAnalyseTypes::ANALYSE_MODULE);
	hError = m_pWbemServices->ExecNotificationQueryAsync(wszWQL2, wszModuleLoadQuery2, 0, nullptr, &eventModuleLoaded);
	if (FAILED(hError))
	{
#ifdef _DEBUG
		TraceError("ExecNotificationQueryAsync(%d) fail! hError: %p Last error: %u", EAnalyseTypes::ANALYSE_MODULE, hError, GetLastError());
#endif
		return 0;
	}

	EventSink eventDriverLoaded(EAnalyseTypes::ANALYSE_DRIVER);
	hError = m_pWbemServices->ExecNotificationQueryAsync(wszWQL2, wszDriverLoadQuery2, 0, nullptr, &eventDriverLoaded);
	if (FAILED(hError))
	{
#ifdef _DEBUG
		TraceError("ExecNotificationQueryAsync(%d) fail! hError: %p Last error: %u", EAnalyseTypes::ANALYSE_DRIVER, hError, GetLastError());
#endif
		return 0;
	}

	EventSink eventServiceLoaded(EAnalyseTypes::ANALYSE_SERVICE);
	hError = m_pWbemServices->ExecNotificationQueryAsync(wszWQL2, wszServiceLoadQuery2, 0, nullptr, &eventServiceLoaded);
	if (FAILED(hError))
	{
#ifdef _DEBUG
		TraceError("ExecNotificationQueryAsync(%d) fail! hError: %p Last error: %u", EAnalyseTypes::ANALYSE_SERVICE, hError, GetLastError());
#endif
		return 0;
	}
#endif

	MSG msg;
	while (GetMessageA(&msg, nullptr, 0, 0))
	{
		if (m_bTerminated)
			break;

		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	m_pWbemServices->Release();
	pLoc->Release();
	CoUninitialize();
	return 0;
}

DWORD WINAPI CWMI_Monitor::StartThreadRoutine(LPVOID lpParam)
{
	auto This = (CWMI_Monitor*)lpParam;
	return This->ThreadRoutine();
}

void CWMI_Monitor::DestroyWMIMonitor()
{
	if (m_hWMIWaitObj && m_hWMIWaitObj != INVALID_HANDLE_VALUE)
		UnregisterWait(m_hWMIWaitObj);

	if (m_hWMIThread && m_hWMIThread != INVALID_HANDLE_VALUE)
	{
		m_bTerminated = true;

		if (WaitForSingleObject(m_hWMIThread, 400) == WAIT_TIMEOUT)
			TerminateThread(m_hWMIThread, EXIT_SUCCESS);
	}
}

bool CWMI_Monitor::InitializeWMIWatcherThread()
{
	m_hWMIThread = CreateThread(0, 0, StartThreadRoutine, (void*)this, 0, &m_dwWMIThreadId);
	if (!m_hWMIThread || m_hWMIThread == INVALID_HANDLE_VALUE)
		return false;

	RegisterWaitForSingleObject(&m_hWMIWaitObj, m_hWMIThread, OnWMIMonitorThreadLost, nullptr, INFINITE, WT_EXECUTEONLYONCE);

	return true;
}
