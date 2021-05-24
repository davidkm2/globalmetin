#pragma once
#include "../EterBase/Singleton.h"
#include "../EterBase/WorkQueue.h"
#include "../EterBase/SimpleTimer.h"

#include <memory>
#include <vector>
#include <functional>

enum ECheatViolationIDs : uint32_t
{
	CHEAT_VIOLATION_BASE = 0, //1605888
	CHEAT_PY_LOAD,
	CHEAT_PY_API_REQUEST_UNKNOWN_REFERENCE,
	CHEAT_PY_API_REQUEST_UNKNOWN_API,
	CHEAT_PY_API_UNKNOWN_REFERENCE,
	CHEAT_ATTACK_SPEED_OVERFLOW,
	CHEAT_MOVE_SPEED_OVERFLOW,
	VEH_SINGLE_STEP_EXCEPTION,
	VEH_BREAKPOINT_EXCEPTION,
	VEH_PAGEGUARD_EXCEPTION,
	SEH_SINGLE_STEP_EXCEPTION,
	SEH_BREAKPOINT_EXCEPTION,
	SEH_PAGEGUARD_EXCEPTION,
	VEH_FILTER_INIT_FAIL,
	SEH_FILTER_INIT_FAIL,
	WINDOW_PROTECTION_CHECK_FAIL,
	WINDOW_PROTECTION_CORRUPTED,
	WINDOW_PROTECTION_CAN_NOT_ENABLED,
	WINDOW_PROTECTION_CAN_NOT_DISABLED,
	BLOCKED_THREAD_START_ADDRESS,
	WATCHDOG_CREATE_FAIL,
	CHEAT_MEMORY_DUMP,
	CHEAT_MEMORY_WATCHDOG,
	DEBUG_DETECT_PEB,
	DEBUG_DETECT_FLAGS,
	DEBUG_DETECT_BUGCHECK,
	DEBUG_DETECT_DBGOBJ_1,
	DEBUG_DETECT_DBGOBJ_2,
	DEBUG_DETECT_KERNEL,
	DEBUG_DETECT_GLOBALFLAGS,
	UNKNOWN_GAME_WINDOW,
	WINDOW_WATCHER_THREAD_LOST,
	BAD_PROCESS_DETECT,
	SYNC_PROC_REGISTER_FAIL,
	WATCHDOG_SIGNAL_LOST,
	WMI_MONITOR_THREAD_LOST,
	WINDOW_WATCHER_THREAD_SUSPEND,
	WMI_MONITOR_THREAD_SUSPEND,
	MAIN_THREAD_SUSPEND,
	LMOUSE_INJECTION,
	MMOUSE_INJECTION,
	RMOUSE_INJECTION,
	WMINPUT_NULL_DEVICE,
	MODULE_SECTION_INTEGRITY_CORRUPTED,
	UNKNOWN_MAPPED_FILE_PATH_NAME,
	UNKNOWN_MONITORED_MODULE,
	MAPPED_FILE_VIOLATION,
	PROTECTION_CRACK,
	DLL_INJECTION,
	DLL_FILTER_CORRUPTED,
	PARENT_CHECK_ACCESS_FAIL,
	PARENT_CHECK_UNKNOWN_EXPLORER,
	PARENT_CHECK_UNKNOWN_PARENT,
	PARENT_CHECK_STARTUP_VIOLATION
};

typedef struct _CHEAT_QUEUE_CONTEXT
{
	uint32_t	dwViolation;
	uint32_t	dwErrorCode;
	char		szMessage[255];
}SCheatQueueCtx, *PCheatQueueCtx;

class CCheatDetectQueueMgr : public CSingleton <CCheatDetectQueueMgr>
{
	using TNetworkReportCallback = std::function<bool(const char*, const char*, const char*)>;

public:
	CCheatDetectQueueMgr();
	virtual ~CCheatDetectQueueMgr() = default;

	void Initialize();
	void RegisterReportFunction(TNetworkReportCallback NetReportFunc);

	std::string GetHardwareIdentifier();
	bool AppendDetection(uint32_t dwViolationID, uint32_t dwSystemErrorCode, const std::string & strMessage);

	void OnTick();
	void OnQueueCall(std::shared_ptr <SCheatQueueCtx> pData);

private:
	std::vector < std::shared_ptr <SCheatQueueCtx> >				m_vSentDetections;
	std::shared_ptr <CQueue <std::shared_ptr <SCheatQueueCtx> > >	m_kQueCheatQuarentine;
	CSimpleTimer <std::chrono::milliseconds>						m_pCheckTimer;
	TNetworkReportCallback											m_pNetReportCallbackFunc;
};

