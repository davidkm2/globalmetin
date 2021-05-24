#pragma once

#include <map>
#include <set>
#include "Peer.h"

typedef struct SItemAward
{
    DWORD	dwID;
    char	szLogin[LOGIN_MAX_LEN+1];
    DWORD	dwVnum;
    DWORD	dwCount;
    DWORD	dwSocket0;
    DWORD	dwSocket1;
    DWORD	dwSocket2;
	DWORD	dwSocket3;
    char	szWhy[ITEM_AWARD_WHY_MAX_LEN+1];
    bool	bTaken;
    bool	bMall;
} TItemAward;

class ItemAwardManager : public singleton<ItemAwardManager>
{
    public:
		ItemAwardManager();
		virtual ~ItemAwardManager();

		void				RequestLoad();
		void				Load(SQLMsg * pMsg);
		std::set<TItemAward *> *	GetByLogin(const char * c_pszLogin);

		void				Taken(DWORD dwAwardID, DWORD dwItemID);
		std::map<DWORD, TItemAward *>& GetMapAward();
		std::map<std::string, std::set<TItemAward *> >& GetMapkSetAwardByLogin();

	private:
		std::map<DWORD, TItemAward *>			m_map_award;
		std::map<std::string, std::set<TItemAward *> >	m_map_kSetAwardByLogin;
};

