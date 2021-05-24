#pragma once
#include "../EterBase/Singleton.h"
#include "CheatQuarentineManager.h"
#include "WMI_Monitor.h"
#include "NTDefinations.h"
#include <string>
#include <map>

enum ENetworkScanIDs
{
	NET_SCAN_ID_NULL,
	NET_SCAN_ID_PROCESS_HASH,
	NET_SCAN_ID_MAX
};

enum EMonitoredModules
{
	MMODULE_NULL,
	MMODULE_BASE,
	MMODULE_KERNEL32,
	MMODULE_KERNELBASE,
	MMODULE_NTDLL,
	MMODULE_USER32,
	MMODULE_WIN32U,
	MMODULE_PYTHON27
};

typedef struct _MODULE_SECTIONS_HASH_DATA
{
	uint32_t dwModuleBase;
	WCHAR 	 wszModuleName[1024];

	uint32_t dwPeHeaderHash;
	uint32_t dwTextHash;
	uint32_t dwRDataHash;
	uint32_t dwEDataHash;
	uint32_t dwRsrcHash;
	uint32_t dwRelocHash;
} SModuleSectionsHashCtx, *PModuleSectionsHashCtx;

class CAnticheatManager : public CSingleton <CAnticheatManager>
{
public:
	CAnticheatManager();
	virtual ~CAnticheatManager() = default;

	uint32_t ServiceMessageBox(const std::string & szTitle, const std::string & szMessage, uint16_t wType);
	bool IsFileExist(const std::string & szFileName);
	bool IsDirExist(const std::string & szDirName);
	
	std::string GetMd5(const uint8_t* pData, std::size_t nSize);
	std::string GetFileMd5(const std::string & szName);

	void InitializeAnticheatRoutines(HINSTANCE hInstance, HWND hWnd);
	void FinalizeAnticheatRoutines();

	void InitializeExceptionFilters();
	void RemoveExceptionHandlers();

	void InitScreenProtection();
	void RemoveScreenProtection();
	void CheckScreenProtection();

	bool IsBlockedStartAddress(uint32_t dwThreadId, uint32_t dwStartAddress, LPDWORD pdwViolationID, std::string * pstrViolationOwnerModule);

	void InitializeAnticheatWatchdog();
	void RemoveAnticheatWatchdog();
	UINT_PTR GetAnticheatWatchdogTimerID() const { return m_iAnticheatWatchdogTimerID; };
	void SendWatchdogSignal(bool bStat) { m_bWatchdogStatus = bStat; };
	bool GetWatchdogStatus() const { return m_bWatchdogStatus; };

	bool InitializeMemoryWatchdogs();
	bool IsMemoryWatchdogsTriggered();

	void InitializeDumpProtection();
	bool IsMemoryDumped();

	bool BlockAccess(HANDLE hTarget);
	bool DenyProcessAccess();
	bool DenyThreadAccess();

	bool SimpleDebugCheck();
	bool DebugCheckBugControl();
	bool DebugObjectHandleCheck();
	bool SharedUserDataCheck();
	bool GlobalFlagsCheck();

	bool InitializeWindowWatcherThread();
	bool DestroyWindowWatcher();
	void RegisterWindowHookHandler(HWINEVENTHOOK hook) { m_hWndHandlerHook = hook; };
	uint32_t GetWindowWatcherThreadId() { return m_dwWndCheckThreadId; };

	void CheckMainFolderFiles();
	void CheckMilesFolderForMilesPlugins();
	void CheckLibFolderForPythonLibs();
	void CheckYmirFolder();

	void CheckMappedFiles();

	void CreateProcessHashList();
	void AppendProcessHandle(HANDLE hProcess);
	void ScanProcess(HANDLE hProcess);
	void CloseUselessProcessHandles();
	void ParseCheatBlacklist(const std::string & szContent);

	void InitializeRawWindowHook();
	void DestroyRawWindowHook();

	void InitializeWindowMessageHook();
	void DestroyWindowMessageHook();

	void OnKeyPress(int32_t iKeyIdx, bool bStatus);
	bool GetKeyStatus(int32_t iKeyIdx);

	HWND GetMainWindow() { return m_hWnd; };
	HINSTANCE GetMainInstance() { return m_hInstance; };
	WNDPROC GetWindowMsgProc() { return m_wndpOldProc; };
	DWORD GetMainThreadId() { return m_dwMainThreadId; };

	void SaveModuleSectionHashList(std::shared_ptr <SModuleSectionsHashCtx> vHashList)
	{
		m_vModuleSectionHashContainer.push_back(vHashList);
	}
	std::vector < std::shared_ptr <SModuleSectionsHashCtx> > GetModuleSectionHashContainer()
	{
		return m_vModuleSectionHashContainer;
	}

	void InitSectionHashes();
	bool HasCorruptedModuleSection();
	std::string GetModuleOwnerName(LPVOID pModuleBase);

	std::vector < HMODULE > GetMonitoredModuleContainer()
	{
		return m_vMonitoredModules;
	}

	bool IsAggressiveMode() const { return m_bAggressiveMode; }
	void ExitByAnticheat(uint32_t dwErrorCode, uint32_t dwSubCode1 = 0, uint32_t dwSubCode2 = 0, bool bForceClose = false, const std::string& stSubMessage = "");

	bool IsDllFilterHookCorrupted();

	bool SetAntiTrace(HANDLE hThread);

protected:
	bool ActivateScreenProtection(bool bEnabled);
	bool GetScreenProtectionStatus();

	bool IsPackedExecutable(const std::string & szName);
	void SetupSectionMonitorContainer();

	bool LoadDllFilter();
	void ParentProcessCheck(const std::string& stPatcherName);

	size_t CheckExceptionTrap();
	
private:
	bool m_bAggressiveMode;

	std::shared_ptr < CCheatQuarentineMgr > m_pQuerantineMgr;
	std::shared_ptr < CWMI_Monitor >		m_pWmiMonitorMgr;

	HINSTANCE	m_hInstance;
	HWND		m_hWnd;
	DWORD		m_dwMainThreadId;
	WNDPROC		m_wndpOldProc;
	HHOOK		m_hkMessageHook;

	std::map <int32_t /* iKeyIdx */, bool /* bStatus */> m_mapKeyStatusList;

	std::vector < std::shared_ptr <SModuleSectionsHashCtx> > m_vModuleSectionHashContainer;
	std::vector < HMODULE > m_vMonitoredModules;

	UINT_PTR		m_iAnticheatWatchdogTimerID;
	bool			m_bWatchdogStatus;

	HWINEVENTHOOK	m_hWndHandlerHook;
	HANDLE			m_hWndCheckThread;
	DWORD			m_dwWndCheckThreadId;
	HANDLE			m_hWndThreadWaitObj;

	HANDLE			m_hSyncProc;
	HANDLE			m_hSyncWaitObj;

	std::map <HWND, bool>	m_mScreenProtectionStatus;

	LPVOID					m_pDumpGuardMem;
	std::vector < LPVOID >	m_pMemoryWatchdogs;
	std::vector < LPVOID >	m_pMemoryDummyPages;

	std::vector < HANDLE >	m_vProcessHandles;
};

