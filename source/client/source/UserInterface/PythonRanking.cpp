#include "StdAfx.h"
#include "PythonRanking.h"
#include "AbstractPlayer.h"

CPythonRanking::CPythonRanking(void)
{
	TRankingInfoMap Map;
	for (int i = 0; i < PARTY_RK_CATEGORY_MAX + SOLO_RK_CATEGORY_MAX; ++i)
		m_RanksInfoMap.insert(std::make_pair(i, Map));
}

CPythonRanking::~CPythonRanking(void)
{
	m_RanksInfoMap.clear();
}

const TRankingInfo& CPythonRanking::GetHighRankingInfo(int iCategory, int iLine)
{
	TRankingInfoMap::iterator it = m_RanksInfoMap[iCategory].find(iLine);

	if (it != m_RanksInfoMap[iCategory].end())
		return it->second;

	return m_NullMap;
}

const TRankingInfo& CPythonRanking::GetMyRankingInfo(int iCategory)
{
	IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();

	for (TRankingInfoMap::iterator it = m_RanksInfoMap[iCategory].begin(); it != m_RanksInfoMap[iCategory].end(); ++it)
		if (!strcmp(it->second.szCharName, rPlayer.GetName()))
			return it->second;

	return m_NullMap;
}

const char* CPythonRanking::GetPartyMemberName(int iCategory, int iLine)
{
	TRankingInfoMap::iterator it = m_RanksInfoMap[iCategory].find(iLine);

	if (it != m_RanksInfoMap[iCategory].end())
		return it->second.szMembersName;

	return m_NullMap.szMembersName;
}

const char* CPythonRanking::GetMyPartyMemberName(int iCategory)
{
	IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();

	for (TRankingInfoMap::iterator it = m_RanksInfoMap[iCategory].begin(); it != m_RanksInfoMap[iCategory].end(); ++it)
		if (!strcmp(it->second.szCharName, rPlayer.GetName()))
			return it->second.szMembersName;

	return m_NullMap.szMembersName;
}

void CPythonRanking::AddRankingInfo(int iCategory, int iLine, TRankingInfo Info)
{
	m_RanksInfoMap[iCategory].insert(std::make_pair(iLine, Info));
}

