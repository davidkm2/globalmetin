#include "StdAfx.h"
#include "CheatQueueManager.h"

#include <nb30.h>
#include <XORstr.h>

CCheatDetectQueueMgr::CCheatDetectQueueMgr() :
	m_pNetReportCallbackFunc(nullptr)
{
	m_pCheckTimer.reset();
}

void CCheatDetectQueueMgr::Initialize()
{
	m_kQueCheatQuarentine = std::make_shared< CQueue <std::shared_ptr <SCheatQueueCtx> > >();
	m_kQueCheatQuarentine->BindCallback(std::bind(&CCheatDetectQueueMgr::OnQueueCall, this, std::placeholders::_1));
}

void CCheatDetectQueueMgr::RegisterReportFunction(TNetworkReportCallback NetReportFunc)
{
	m_pNetReportCallbackFunc = NetReportFunc;
}

std::string CCheatDetectQueueMgr::GetHardwareIdentifier()
{
	char szMacAddr[128] = { 0 };

	typedef struct _ASTAT_
	{
		ADAPTER_STATUS adapt;
		NAME_BUFFER NameBuff[30];
	} ASTAT, *PASTAT;
	ASTAT pAdapter = { 0 };

	LANA_ENUM laLanaEnum = { 0 };

	NCB ncb = { 0 };
	ncb.ncb_command = NCBENUM;
	ncb.ncb_buffer = (LPBYTE)&laLanaEnum;
	ncb.ncb_length = sizeof(laLanaEnum);

	auto pRet = Netbios(&ncb);
	if (pRet == NRC_GOODRET)
	{
		for (int32_t lana = 0; lana < laLanaEnum.length; lana++)
		{
			ncb.ncb_command = NCBRESET;
			ncb.ncb_lana_num = laLanaEnum.lana[lana];
			pRet = Netbios(&ncb);
			if (pRet == NRC_GOODRET)
				break;
		}
		if (pRet == NRC_GOODRET)
		{
			memset(&ncb, 0, sizeof(ncb));

			ncb.ncb_command = NCBASTAT;
			ncb.ncb_lana_num = laLanaEnum.lana[0];
			strcpy((char*)ncb.ncb_callname, "*");
			ncb.ncb_buffer = (LPBYTE)&pAdapter;
			ncb.ncb_length = sizeof(pAdapter);

			pRet = Netbios(&ncb);
			if (pRet == NRC_GOODRET)
			{
				sprintf(szMacAddr, XOR("%02X-%02X-%02X-%02X-%02X-%02X"),
					pAdapter.adapt.adapter_address[0],
					pAdapter.adapt.adapter_address[1],
					pAdapter.adapt.adapter_address[2],
					pAdapter.adapt.adapter_address[3],
					pAdapter.adapt.adapter_address[4],
					pAdapter.adapt.adapter_address[5]
				);
			}
		}
	}
	return szMacAddr;
}

bool CCheatDetectQueueMgr::AppendDetection(uint32_t dwViolationID, uint32_t dwSystemErrorCode, const std::string & strMessage)
{
#ifdef _DEBUG
	TraceError("AppendDetection: Violation %u-%u Message %s", dwViolationID, dwSystemErrorCode, strMessage.c_str());
#endif

	if (m_vSentDetections.size() > 0)
	{
		for (const auto & pCurrSentItem : m_vSentDetections)
		{
			if (dwViolationID == pCurrSentItem->dwViolation && dwSystemErrorCode == pCurrSentItem->dwErrorCode && strMessage == pCurrSentItem->szMessage)
				return true;
		}
	}

	auto pCheatDetectQueueData = std::make_shared<SCheatQueueCtx>();
	if (!pCheatDetectQueueData || !pCheatDetectQueueData.get())
	{
#ifdef _DEBUG
		TraceError("Cheat queue data container can NOT allocated!");
#endif
		return false;
	}

	pCheatDetectQueueData->dwViolation = dwViolationID;
	pCheatDetectQueueData->dwErrorCode = dwSystemErrorCode;
	strcpy_s(pCheatDetectQueueData->szMessage, strMessage.c_str());

	if (m_kQueCheatQuarentine->InsertObject(pCheatDetectQueueData) == false)
	{
#ifdef _DEBUG
		TraceError("Cheat object is already attached to queue!");
#endif
		return true;
	}

	m_vSentDetections.push_back(pCheatDetectQueueData);

#ifdef _DEBUG
	Tracen("Notification attached to queue!");
#endif
	return true;
}


void CCheatDetectQueueMgr::OnTick()
{
	if (m_pCheckTimer.diff() > 2000)
	{
		m_kQueCheatQuarentine->ProcessFirstObject();

		m_pCheckTimer.reset();
	}
}

void CCheatDetectQueueMgr::OnQueueCall(std::shared_ptr <SCheatQueueCtx> pData)
{
	assert(m_pNetReportCallbackFunc);

	if (pData && pData.get()) 
	{
#ifdef _DEBUG
		TraceError("Current cheat queue obj: %u-%u", pData->dwErrorCode, pData->dwViolation);
#endif

		std::string strCheatMsg = std::to_string(pData->dwViolation) + "[" + std::to_string(pData->dwErrorCode) + "]";

		if (false == m_pNetReportCallbackFunc(strCheatMsg.c_str(), pData->szMessage, GetHardwareIdentifier().c_str()))
		{
#ifdef _DEBUG
			TraceError("Current cheat queue obj: '%s' can not send to server!", strCheatMsg.c_str());
#endif
		}
	}
}

