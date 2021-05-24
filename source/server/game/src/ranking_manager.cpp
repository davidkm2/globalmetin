#include "stdafx.h"
#include "ranking_manager.h"
#include "db.h"
#include "packet.h"
#include "char.h"

CRankingManager::CRankingManager() /// Unutamadım......................................................................
{
	TRankingInfoMap Map;
	for (int i = 0; i < PARTY_RK_CATEGORY_MAX + SOLO_RK_CATEGORY_MAX; ++i)
		m_RanksInfoMap.insert(std::make_pair(i, Map));
}

CRankingManager::~CRankingManager()
{
	m_RanksInfoMap.clear();
}

void CRankingManager::Initialize()
{
	std::unique_ptr<SQLMsg> pkMsg(DBManager::instance().DirectQuery("SELECT * FROM player.ranking"));
	SQLResult * pRes = pkMsg->Get();

	MYSQL_ROW data;

	if (pRes->uiNumRows)
	{
		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TRankingInfo Info;

			int col = 0;

			str_to_number(Info.bCategory, data[col++]);
			str_to_number(Info.iRankingIdx, data[col++]);
			strlcpy(Info.szCharName, data[col++], sizeof(Info.szCharName));
			strlcpy(Info.szMembersName, data[col++], sizeof(Info.szMembersName));
			str_to_number(Info.iRecord0, data[col++]);
			str_to_number(Info.iRecord1, data[col++]);
			str_to_number(Info.iTime, data[col++]);
			str_to_number(Info.bEmpire, data[col++]);

			m_RanksInfoMap[Info.bCategory].insert(std::make_pair(Info.iRankingIdx, Info));
		}
	}
}

const TRankingInfo& CRankingManager::GetHighRankingInfo(int iCategory, int iLine)
{
	std::map<int, TRankingInfo>::iterator it = m_RanksInfoMap[iCategory].find(iLine);

	if (it != m_RanksInfoMap[iCategory].end())
		return it->second;

	return m_NullMap;
}

bool CRankingManager::FindHighRankingInfo(int iCategory, int iLine)
{
	std::map<int, TRankingInfo>::iterator it = m_RanksInfoMap[iCategory].find(iLine);

	if (it != m_RanksInfoMap[iCategory].end())
		return true;

	return false;
}

const TRankingInfo& CRankingManager::GetMyRankingInfo(int iCategory, LPCHARACTER pChar)
{
	for (TRankingInfoMap::iterator it = m_RanksInfoMap[iCategory].begin(); it != m_RanksInfoMap[iCategory].end(); ++it)
		if (!strcmp(it->second.szCharName, pChar->GetName()))
			return it->second;

	return m_NullMap;
}

const char* CRankingManager::GetPartyMemberName(int iCategory, int iLine)
{
	std::map<int, TRankingInfo>::iterator it = m_RanksInfoMap[iCategory].find(iLine);

	if (it != m_RanksInfoMap[iCategory].end())
		return it->second.szMembersName;

	return m_NullMap.szMembersName;
}

const char* CRankingManager::GetMyPartyMemberName(int iCategory, LPCHARACTER pChar)
{
	for (TRankingInfoMap::iterator it = m_RanksInfoMap[iCategory].begin(); it != m_RanksInfoMap[iCategory].end(); ++it)
		if (!strcmp(it->second.szCharName, pChar->GetName()))
			return it->second.szMembersName;

	return m_NullMap.szMembersName;
}

void CRankingManager::AddRankingInfo(int iCategory, int iLine, TRankingInfo Info)
{
	DBManager::instance().DirectQuery("INSERT INTO player.ranking VALUES(%d, %d, '%s', '%s', %d, %d, %d, %d)", iCategory, Info.iRankingIdx, Info.szCharName, Info.szMembersName, Info.iRecord0, Info.iRecord1, Info.iTime, Info.bEmpire);
	m_RanksInfoMap[iCategory].insert(std::make_pair(iLine, Info));
}

void CRankingManager::UpdateRankingInfo(int iCategory, int iLine, TRankingInfo Info)
{
	DBManager::instance().DirectQuery("UPDATE player.ranking SET category = %d, index = %d, name = '%s', members = '%s', record0 = %d, record1 = %d, time = %d, empire = %d WHERE category = %d and index = %d and name = '%s'", iCategory, Info.iRankingIdx, Info.szCharName, Info.szMembersName, Info.iRecord0, Info.iRecord1, Info.iTime, Info.bEmpire, iCategory, Info.iRankingIdx, Info.szCharName);
	m_RanksInfoMap[iCategory].erase(iLine);
	m_RanksInfoMap[iCategory].insert(std::make_pair(iLine, Info));
}

int CRankingManager::GetSpaceLine(int iCategory)
{
	TRankingInfoMap::size_type i = m_RanksInfoMap[iCategory].size();

	return (int)i;
}

