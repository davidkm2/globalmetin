#pragma once
#include "../../common/defines.h"
#include "../../common/tables.h"
#include "packet.h"

class CRankingManager : public singleton<CRankingManager>
{
	public:
		CRankingManager();
		virtual ~CRankingManager();
		void Initialize();

		const TRankingInfo& GetHighRankingInfo(int iCategory, int iLine);
		const TRankingInfo& GetMyRankingInfo(int iCategory, LPCHARACTER pChar);
		const char* GetPartyMemberName(int iCategory, int iLine);
		const char* GetMyPartyMemberName(int iCategory, LPCHARACTER pChar);

		void AddRankingInfo(int iCategory, int iLine, TRankingInfo Info);
		bool FindHighRankingInfo(int iCategory, int iLine);
		void UpdateRankingInfo(int iCategory, int iLine, TRankingInfo Info);

		TInfoMap GetRankingInfo() { return m_RanksInfoMap; }
		int GetSpaceLine(int iCategory);


	protected:
		TInfoMap m_RanksInfoMap;
		TRankingInfo m_NullMap;
};

