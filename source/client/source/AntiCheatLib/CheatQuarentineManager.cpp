#include "StdAfx.h"
#include "CheatQuarentineManager.h"
#include "CheatQueueManager.h"
#include "AnticheatManager.h"

CCheatQuarentineMgr::CCheatQuarentineMgr()
{
	m_vProcessMd5List.clear();
}


void CCheatQuarentineMgr::ProcessQuarentineList(int32_t iIdx, bool bSingle)
{
	if (iIdx == NET_SCAN_ID_PROCESS_HASH)
	{
		CheckProcessMd5Hashes(bSingle);
	}
}


void CCheatQuarentineMgr::AppendProcessMd5(const std::string & strProcessName, const std::string & strMd5, bool bBlacklisted)
{
	if (bBlacklisted)
	{
		for (const auto & pCurrBlacklistItem : m_vProcessMd5Blacklist)
		{
			if (!strcmp(pCurrBlacklistItem->szProcessMd5, strMd5.c_str()))
			{
				return;
			}
		}
	}

	auto pScanData = std::make_shared<SProcessMd5ScanData>();
	if (!pScanData || !pScanData.get())
		return;

	strcpy_s(pScanData->szProcessName, strProcessName.c_str());
	strcpy_s(pScanData->szProcessMd5, strMd5.c_str());

	if (bBlacklisted)
		m_vProcessMd5Blacklist.push_back(pScanData);
	else
		m_vProcessMd5List.push_back(pScanData);
}


void CCheatQuarentineMgr::CheckProcessMd5Hashes(bool bSingle)
{
	if (m_vProcessMd5Blacklist.empty())
	{
//		Tracef("Process md5 scan list is null!");
		return;
	}

//	Tracenf("Process md5 scan process has been started! Size: %u", m_vProcessMd5Blacklist.size());

	for (const auto & pCurrScanCtx : m_vProcessMd5Blacklist)
	{
		if (pCurrScanCtx && pCurrScanCtx.get())
		{
//			Tracenf("Current process md5 scan list object: '%s'-'%s' List size: %u", pCurrScanCtx->szProcessName, pCurrScanCtx->szProcessMd5, m_vProcessMd5Blacklist.size());

			for (const auto & pCurrProcessCtx : m_vProcessMd5List)
			{
				if (pCurrProcessCtx && pCurrProcessCtx.get())
				{
					if (!strcmp(pCurrScanCtx->szProcessMd5, pCurrProcessCtx->szProcessMd5))
					{
//						Tracenf("Blacklist'd object found! Data: '%s'-'%s'", pCurrScanCtx->szProcessName, pCurrScanCtx->szProcessMd5);

						// Send violation msg
						CCheatDetectQueueMgr::Instance().AppendDetection(BAD_PROCESS_DETECT, 0 /* std::stoul(pCurrScanCtx->szProcessName) */, pCurrScanCtx->szProcessMd5);

						// Delete already processed object from quarentine db
						m_vProcessMd5Blacklist.erase(std::remove(m_vProcessMd5Blacklist.begin(), m_vProcessMd5Blacklist.end(), pCurrScanCtx), m_vProcessMd5Blacklist.end());
						m_vProcessMd5List.erase(std::remove(m_vProcessMd5List.begin(), m_vProcessMd5List.end(), pCurrProcessCtx), m_vProcessMd5List.end());

						break;
					}
				}
			}
		}
	}

	if (bSingle == false) // If current check type is not single check it's mean all stored processes compared with blacklisted ones, we don't need anymore
	{
		m_vProcessMd5List.clear();
	}
}


