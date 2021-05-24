#pragma once
#include "Packet.h"

class CPythonRanking : public CSingleton<CPythonRanking>
{
	public:
		CPythonRanking(void);
		virtual ~CPythonRanking(void);

	public:
		const TRankingInfo& GetHighRankingInfo(int iCategory, int iLine);
		const TRankingInfo& GetMyRankingInfo(int iCategory);
		const char* GetPartyMemberName(int iCategory, int iLine);
		const char* GetMyPartyMemberName(int iCategory);

		void AddRankingInfo(int iCategory, int iLine, TRankingInfo Info);
		TInfoMap GetRankingInfo() { return m_RanksInfoMap; }

	protected:
		TInfoMap m_RanksInfoMap;
		TRankingInfo m_NullMap;
};

