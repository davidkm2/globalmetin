#include "stdafx.h"
#include "groupmatchmanager.h"
#include "constants.h"
#include "config.h"
#include "item_manager.h"
#include "item.h"
#include "dungeon.h"
#include "log.h"
#include "char.h"
#include "char_manager.h"
#include "party.h"
#include "questlua.h"
#include "questmanager.h"

#ifdef __PARTY_MATCH__
using namespace std;
static set<int> yasakli_haritalar;

//configs:
bool wait_time = false;
int enter_0 = 2;
int enter_1 = 2;
int enter_2 = 2;
int enter_3 = 2;
int enter_4 = 2;
int enter_5 = 2;
int d2_min_shaman = 1;
int d2_min_assassin = 1;
int search_wait = 10;
long d0_need_1 = 30190;
long d0_need_2 = 30179;
int d0_need_1_count = 1;
int d0_need_2_count = 3;
long d1_need_1 = 71095;
int d1_need_1_count = 1;
long d3_need_1 = 30613;
int d3_need_1_count = 1;
long d5_need_1 = 71095;
int d5_need_1_count = 1;
long d0_need_lv = 75;
long d1_need_lv = 100;
long d2_need_lv = 100;
long d3_need_lv = 95;
long d4_need_lv = 95;
long d5_need_lv = 75;
//end of configs

const char* EnglishTranslate[] = {
	"Empty",
	"You can't this, because you have a party.",
	"You're dead, you can't do that.",
	"You don't have enough: %s",
	"Search canceled.",
	"You're a shop, you can't do that.",
	"You're already in a dungeon, you can't do that.",
	"You can't do that in this map.",
	"You must wait %d seconds.",
	"You must be %d level for group search.",
	"<Group Search> deactivated.",//10
	"<Group Search> activated.",
	"<Group Search> Notices activated.",
	"<Group Search> Notices deactivated."
	
};

//Don't change anything here
const char* dungeonname[] = {
	"grouup 0",
	"grouup 1",
	"grouup 2",
	"grouup 3",
	"grouup 4",
	"grouup 5"
};

const int SpawnDungeons[DUNGEON_MAX_INDEX+1][2] =
{
	{1808, 12207},
	{7766, 6719},
	{4321, 1649},
	{8279, 14182},
	{12773, 17336},
	{5979, 7002}
};

//not using now
const int BlockMapIdx[] = {
	OX_MAP_IDX//
};

namespace quest
{	
	int __PARTY_MATCH___ayarlar_giris(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:1");
			return 0;
		}
		
		int black = (int) lua_tonumber(L, 1);
		
		if (black < 0 || black > DUNGEON_MAX_INDEX)
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:2");
			return 0;
		}
		if (black == 0)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().enter_0);
		}
		else if (black == 1)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().enter_1);
		}
		else if (black == 2)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().enter_2);
		}
		else if (black == 3)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().enter_3);
		}
		else if (black == 4)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().enter_4);
		}
		else if (black == 5)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().enter_5);
		}
		
		return 1;
	}
	int __PARTY_MATCH___ayarlar_giris_levelleri(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:1");
			return 0;
		}
		
		int black = (int) lua_tonumber(L, 1);
		
		if (black < 0 || black > DUNGEON_MAX_INDEX)
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:2");
			return 0;
		}
		if (black == 0)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d0_need_lv);
		}
		else if (black == 1)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d1_need_lv);
		}
		else if (black == 2)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d2_need_lv);
		}
		else if (black == 3)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d3_need_lv);
		}
		else if (black == 4)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d4_need_lv);
		}
		else if (black == 5)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d5_need_lv);
		}
		
		return 1;
	}
	int __PARTY_MATCH___ayarlar_itemler(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:1");
			return 0;
		}
		
		int black = (int) lua_tonumber(L, 1);
		
		if (black < 0 || black > DUNGEON_MAX_INDEX + 1)
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:2");
			return 0;
		}
		if (black == 0)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d0_need_1);
		}
		else if (black == 1)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d1_need_1);
		}
		#ifdef NEW_NEED_ITEMS
		else if (black == 2)
		{
			lua_pushnumber(L, D2_ITEM_1_VNUM);
		}
		else if (black == 4)
		{
			lua_pushnumber(L, D4_ITEM_1_VNUM);
		}
		#endif
		else if (black == 3)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d3_need_1);
		}
		else if (black == 5)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d5_need_1);
		}
		else if (black == 6)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d0_need_2);
		}
		
		return 1;
	}
	int __PARTY_MATCH___ayarlar_itemler_count(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:1");
			return 0;
		}
		
		int black = (int) lua_tonumber(L, 1);
		
		if (black < 0 || black > DUNGEON_MAX_INDEX + 1)
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:2");
			return 0;
		}
		if (black == 0)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d0_need_1_count);
		}
		else if (black == 1)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d1_need_1_count);
		}
		#ifdef NEW_NEED_ITEMS
		else if (black == 2)
		{
			lua_pushnumber(L, D2_ITEM_1_COUNT);
		}
		else if (black == 4)
		{
			lua_pushnumber(L, D4_ITEM_1_COUNT);
		}
		#endif
		else if (black == 3)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d3_need_1_count);
		}
		else if (black == 5)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d5_need_1_count);
		}
		else if (black == 6)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().d0_need_2_count);
		}
		
		return 1;
	}
	int is_new_need_items(lua_State* L)
	{
		#ifdef NEW_NEED_ITEMS
			lua_pushnumber(L, 1);
		#else
			lua_pushnumber(L, 0);
		#endif
		return 1;
	}
	int is_send_shout(lua_State* L)
	{
		lua_pushnumber(L, 0);
		return 1;
	}
	int dungeon_name(lua_State* L)
	{
		int black = (int) lua_tonumber(L, 1);
		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:4");
			return 0;
		}
		if (black < 0 || black > DUNGEON_MAX_INDEX)
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:3");
			return 0;
		}
		
		lua_pushstring(L, LC_TEXT(dungeonname[black]));
		return 1;
	}
	int kayitlar(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:4");
			return 0;
		}
		
		int black = (int) lua_tonumber(L, 1);
		
		if (black < 0 || black > DUNGEON_MAX_INDEX)
		{
			sys_err("QUEST ERROR : <GroupMatchManager>:3");
			return 0;
		}
		if (black == 0)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().ArayanlarinSayisi0());
		}
		else if (black == 1)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().ArayanlarinSayisi1());
		}
		else if (black == 2)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().ArayanlarinSayisi2());
		}
		else if (black == 3)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().ArayanlarinSayisi3());
		}
		else if (black == 4)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().ArayanlarinSayisi4());
		}
		else if (black == 5)
		{
			lua_pushnumber(L, CGroupMatchManager::instance().ArayanlarinSayisi5());
		}
		
		return 1;
	}
	void RegisterGroupMatchFunctionTable()
	{
		luaL_reg GroupSearchFunctions[] =
		{
			{"ayarlar_giris",__PARTY_MATCH___ayarlar_giris},
			{"ayarlar_itemler",__PARTY_MATCH___ayarlar_itemler},
			{"ayarlar_itemler_count",__PARTY_MATCH___ayarlar_itemler_count},
			{"ayarlar_kayitlar",kayitlar},
			{"is_new_need_items",is_new_need_items},
			{"is_send_shout",is_send_shout},
			{"dungeon_name",dungeon_name},
			{"giris_levelleri",__PARTY_MATCH___ayarlar_giris_levelleri},
			{NULL,NULL}
		};

		CQuestManager::instance().AddLuaFunctionTable("__PARTY_MATCH__", GroupSearchFunctions);
	}
}

bool CGroupMatchManager::Initialize()
{
	first0 = 0;
	first1 = 0;
	first2 = 0;
	first3 = 0;
	first4 = 0;
	first5 = 0;
	have_shaman = 0;
	have_ninja = 0;
	
	Destroy();
	return true;
}

void CGroupMatchManager::Destroy()
{
	arayanlar0.clear();
	arayanlar1.clear();
	arayanlar2.clear();
	arayanlar3.clear();
	arayanlar4.clear();
	arayanlar5.clear();
}

void CGroupMatchManager::AramayiDurdur(DWORD id)
{
	itertype(arayanlar0) iter0 = arayanlar0.begin();
	LPCHARACTER arayan0 = NULL;
	for (; iter0 != arayanlar0.end(); ++iter0)
	{
		LPCHARACTER arayan0 = CHARACTER_MANAGER::instance().FindByPID(id);
		if (arayan0 != NULL)
		{
			arayanlar0.erase(id);
			if (first0 == arayan0->GetPlayerID())
			{
				first0 = 0;
			}
			arayan0->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
			#ifdef __PARTY_MATCH___TEST_SYSLOG
			sys_log(0, "CGroupMatchManager::AramayiDurdur: arayanlar0\n index:%d\n id:%u", index, id);
			#endif
		}
	}
	
	itertype(arayanlar1) iter1 = arayanlar1.begin();
	LPCHARACTER arayan1 = NULL;
	for (; iter1 != arayanlar1.end(); ++iter1)
	{
		LPCHARACTER arayan1 = CHARACTER_MANAGER::instance().FindByPID(id);
		if (arayan1 != NULL)
		{
			arayanlar1.erase(id);
			if (first1 == arayan1->GetPlayerID())
			{
				first1 = 0;
			}
			arayan1->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
			#ifdef __PARTY_MATCH___TEST_SYSLOG
			sys_log(0, "CGroupMatchManager::AramayiDurdur: arayanlar1\n index:%d\n id:%u", index, id);
			#endif
		}
	}
	
	itertype(arayanlar2) iter2 = arayanlar2.begin();
	LPCHARACTER arayan2 = NULL;
	for (; iter2 != arayanlar2.end(); ++iter2)
	{
		LPCHARACTER arayan2 = CHARACTER_MANAGER::instance().FindByPID(id);
		if (arayan2 != NULL)
		{
			arayanlar2.erase(id);
			if (first2 == arayan2->GetPlayerID())
			{
				first2 = 0;
			}
			if (arayan2->GetJob() == 3 || arayan2->GetJob() == 7)
			{
				have_shaman = have_shaman - 1;
			}
			if (arayan2->GetJob() == 1 || arayan2->GetJob() == 5)
			{
				have_ninja = have_ninja - 1;
			}
			arayan2->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
			#ifdef __PARTY_MATCH___TEST_SYSLOG
			sys_log(0, "CGroupMatchManager::AramayiDurdur: arayanlar2\n index:%d\n id:%u", index, id);
			sys_log(0, "CGroupMatchManager::AramayiDurdur: arayanlar2\n have_shaman:%d\n have_ninja:%u", have_ninja, have_shaman);
			#endif
		}
	}
	
	itertype(arayanlar3) iter3 = arayanlar3.begin();
	LPCHARACTER arayan3 = NULL;
	for (; iter3 != arayanlar3.end(); ++iter3)
	{
		LPCHARACTER arayan3 = CHARACTER_MANAGER::instance().FindByPID(id);

		if (arayan3 != NULL)
		{
			arayanlar3.erase(id);
			if (first3 == arayan3->GetPlayerID())
			{
				first3 = 0;
			}
			arayan3->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
			#ifdef __PARTY_MATCH___TEST_SYSLOG
			sys_log(0, "CGroupMatchManager::AramayiDurdur: arayanlar3\n index:%d\n id:%u", index, id);
			#endif
		}
	}
	
	itertype(arayanlar4) iter4 = arayanlar4.begin();
	LPCHARACTER arayan4 = NULL;
	for (; iter4 != arayanlar4.end(); ++iter4)
	{
		LPCHARACTER arayan4 = CHARACTER_MANAGER::instance().FindByPID(id);

		if (arayan4 != NULL)
		{
			arayanlar4.erase(id);
			if (first4 == arayan4->GetPlayerID())
			{
				first4 = 0;
			}
			arayan4->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
			#ifdef __PARTY_MATCH___TEST_SYSLOG
			sys_log(0, "CGroupMatchManager::AramayiDurdur: arayanlar4\n index:%d\n id:%u", index, id);
			#endif
		}
	}
	
	itertype(arayanlar5) iter5 = arayanlar5.begin();
	LPCHARACTER arayan5 = NULL;
	for (; iter5 != arayanlar5.end(); ++iter5)
	{
		LPCHARACTER arayan5 = CHARACTER_MANAGER::instance().FindByPID(id);

		if (arayan5 != NULL)
		{
			arayanlar5.erase(id);
			if (first5 == arayan5->GetPlayerID())
			{
				first5 = 0;
			}
			arayan5->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
			#ifdef __PARTY_MATCH___TEST_SYSLOG
			sys_log(0, "CGroupMatchManager::AramayiDurdur: arayanlar5\n index:%d\n id:%u", index, id);
			#endif
		}
	}
}

void CGroupMatchManager::CheckPlayers(int index)
{
	if (index > DUNGEON_MAX_INDEX)
	{
		#ifdef __PARTY_MATCH___TEST_SYSLOG
		sys_log(0, "CGroupMatchManager::CheckPlayers: over index:%d", index);
		#endif
		return;
	}
	#ifdef __PARTY_MATCH___TEST_SYSLOG
	sys_log(0, "CGroupMatchManager::CheckPlayers: index:%d", index);
	#endif
	if (index == 0)
	{
		if (ArayanlarinSayisi0() == enter_0)
		{
			itertype(arayanlar0) iter0 = arayanlar0.begin();
			LPCHARACTER arayan0 = NULL;
			for (; iter0 != arayanlar0.end(); ++iter0)
			{
				LPCHARACTER arayan0 = CHARACTER_MANAGER::instance().FindByPID(iter0->second);
				LPCHARACTER FirstPlayer_0 = CHARACTER_MANAGER::instance().FindByPID(first0);
				if (arayan0 != NULL && arayan0)
				{
					if (arayan0->CountSpecifyItem(d0_need_1) < d0_need_1_count || arayan0->CountSpecifyItem(d0_need_2) < d0_need_2_count)
					{
						arayan0->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Arama yapan oyuncularda yeterli item olmadigindan arama iptal edildi."));
						arayan0->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						AramayiDurdur(arayan0->GetPlayerID());
						return;
					}
					else if (arayan0->GetParty() != NULL)
					{
						arayan0->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));
						arayan0->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						AramayiDurdur(arayan0->GetPlayerID());
						return;
					}
					else
					{
						arayan0->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						LPPARTY pParty = CPartyManager::instance().CreateParty(FirstPlayer_0);
						pParty->Join(arayan0->GetPlayerID());
						arayan0->WarpSet(SpawnDungeons[index][0]*DUNGEON_MAP_VALUE, SpawnDungeons[index][1]*DUNGEON_MAP_VALUE);
						#ifdef __PARTY_MATCH___TEST_SYSLOG
						sys_log(0, "CGroupMatchManager::CheckPlayers: arayan0\n id:%u", arayan0->GetPlayerID());
						#endif
					}
				}
			}
		}
	}
	if (index == 1)
	{
		if (ArayanlarinSayisi1() == enter_1)
		{
			itertype(arayanlar1) iter1 = arayanlar1.begin();
			LPCHARACTER arayan1 = NULL;
			for (; iter1 != arayanlar1.end(); ++iter1)
			{
				LPCHARACTER arayan1 = CHARACTER_MANAGER::instance().FindByPID(iter1->second);
				LPCHARACTER FirstPlayer_1 = CHARACTER_MANAGER::instance().FindByPID(first1);
				if (arayan1 != NULL && arayan1)
				{
					if (arayan1->CountSpecifyItem(d1_need_1) < d1_need_1_count)
					{
						arayan1->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Arama yapan oyuncularda yeterli item olmadigindan arama iptal edildi."));
						arayan1->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						AramayiDurdur(arayan1->GetPlayerID());
						return;
					}
					else if (arayan1->GetParty() != NULL)
					{
						arayan1->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));
						arayan1->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						AramayiDurdur(arayan1->GetPlayerID());
						return;
					}
					else
					{
						arayan1->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						LPPARTY pParty = CPartyManager::instance().CreateParty(FirstPlayer_1);
						pParty->Join(arayan1->GetPlayerID());
						long lMapIndex = 351;
						LPDUNGEON pDungeon = CDungeonManager::instance().Create(lMapIndex);
						if (pDungeon)
						{	
							arayan1->WarpSet(SpawnDungeons[index][0]*DUNGEON_MAP_VALUE, SpawnDungeons[index][1]*DUNGEON_MAP_VALUE);
						}
						#ifdef __PARTY_MATCH___TEST_SYSLOG
						sys_log(0, "CGroupMatchManager::CheckPlayers: arayan1\n id:%u", arayan1->GetPlayerID());
						#endif
					}
				}
			}
		}
	}
	if (index == 2)
	{
		if (ArayanlarinSayisi2() >= enter_2)
		{
			itertype(arayanlar2) iter2 = arayanlar2.begin();
			LPCHARACTER arayan2 = NULL;
			for (; iter2 != arayanlar2.end(); ++iter2)
			{
				LPCHARACTER arayan2 = CHARACTER_MANAGER::instance().FindByPID(iter2->second);
				LPCHARACTER FirstPlayer_2 = CHARACTER_MANAGER::instance().FindByPID(first2);

				if (arayan2 != NULL && arayan2)
				{
					if (arayan2->GetParty() != NULL)
					{
						arayan2->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));
						arayan2->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						AramayiDurdur(arayan2->GetPlayerID());
						return;
					}
					if (have_shaman >= d2_min_shaman && have_ninja >= d2_min_assassin)
					{
						arayan2->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						LPPARTY pParty = CPartyManager::instance().CreateParty(FirstPlayer_2);
						pParty->Join(arayan2->GetPlayerID());
						long lMapIndex = 352;
						LPDUNGEON pDungeon = CDungeonManager::instance().Create(lMapIndex);
						if (pDungeon)
						{
							arayan2->WarpSet(SpawnDungeons[index][0]*DUNGEON_MAP_VALUE, SpawnDungeons[index][1]*DUNGEON_MAP_VALUE);
						}
						#ifdef __PARTY_MATCH___TEST_SYSLOG
						sys_log(0, "CGroupMatchManager::CheckPlayers: arayan2\n id:%u", arayan2->GetPlayerID());
						#endif
					}
					#ifdef __PARTY_MATCH___TEST_SYSLOG
					sys_log(0, "CGroupMatchManager::CheckPlayers: arayan2\n have_shaman:%d, have_assassin", have_shaman, have_ninja);
					#endif
				}
			}
		}
	}
	if (index == 3)
	{
		if (ArayanlarinSayisi3() == enter_3)
		{
			itertype(arayanlar3) iter3 = arayanlar3.begin();
			LPCHARACTER arayan3 = NULL;
			for (; iter3 != arayanlar3.end(); ++iter3)
			{
				LPCHARACTER arayan3 = CHARACTER_MANAGER::instance().FindByPID(iter3->second);
				LPCHARACTER FirstPlayer_3 = CHARACTER_MANAGER::instance().FindByPID(first3);
				if (arayan3 != NULL && arayan3)
				{
					if (arayan3->CountSpecifyItem(d3_need_1) < d3_need_1_count)
					{
						arayan3->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Arama yapan oyuncularda yeterli item olmadigindan arama iptal edildi."));
						arayan3->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						AramayiDurdur(arayan3->GetPlayerID());
						return;
					}
					else if (arayan3->GetParty() != NULL)
					{
						arayan3->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));

						arayan3->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						AramayiDurdur(arayan3->GetPlayerID());
						return;
					}
					else
					{
						arayan3->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						LPPARTY pParty = CPartyManager::instance().CreateParty(FirstPlayer_3);
						pParty->Join(arayan3->GetPlayerID());
						arayan3->WarpSet(SpawnDungeons[index][0]*DUNGEON_MAP_VALUE, SpawnDungeons[index][1]*DUNGEON_MAP_VALUE);
						#ifdef __PARTY_MATCH___TEST_SYSLOG
						sys_log(0, "CGroupMatchManager::CheckPlayers: arayan3\n id:%u", arayan3->GetPlayerID());
						#endif
					}
				}
			}
		}
	}
	if (index == 4)
	{
		if (ArayanlarinSayisi4() == enter_4)
		{
			itertype(arayanlar4) iter4 = arayanlar4.begin();
			LPCHARACTER arayan4 = NULL;
			for (; iter4 != arayanlar4.end(); ++iter4)
			{
				LPCHARACTER arayan4 = CHARACTER_MANAGER::instance().FindByPID(iter4->second);
				LPCHARACTER FirstPlayer_4 = CHARACTER_MANAGER::instance().FindByPID(first4);

				if (arayan4 != NULL && arayan4)
				{
					if (arayan4->GetParty() != NULL)
					{
						arayan4->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));
						arayan4->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						AramayiDurdur(arayan4->GetPlayerID());
						return;
					}
					arayan4->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
					LPPARTY pParty = CPartyManager::instance().CreateParty(FirstPlayer_4);
					pParty->Join(arayan4->GetPlayerID());
					arayan4->WarpSet(SpawnDungeons[index][0]*DUNGEON_MAP_VALUE, SpawnDungeons[index][1]*DUNGEON_MAP_VALUE);
					#ifdef __PARTY_MATCH___TEST_SYSLOG
					sys_log(0, "CGroupMatchManager::CheckPlayers: arayan4\n id:%u", arayan4->GetPlayerID());
					#endif
				}
			}
		}
	}
	if (index == 5)
	{
		if (ArayanlarinSayisi5() == enter_5)
		{
			itertype(arayanlar5) iter5 = arayanlar5.begin();
			LPCHARACTER arayan5 = NULL;
			for (; iter5 != arayanlar5.end(); ++iter5)
			{
				LPCHARACTER arayan5 = CHARACTER_MANAGER::instance().FindByPID(iter5->second);
				LPCHARACTER FirstPlayer_5 = CHARACTER_MANAGER::instance().FindByPID(first5);
				if (arayan5 != NULL && arayan5)
				{
					if (arayan5->CountSpecifyItem(d5_need_1) < d5_need_1_count)
					{
						arayan5->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Arama yapan oyuncularda yeterli item olmadigindan arama iptal edildi."));
						arayan5->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						AramayiDurdur(arayan5->GetPlayerID());
						return;
					}
					else if (arayan5->GetParty() != NULL)
					{
						arayan5->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));
						arayan5->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						AramayiDurdur(arayan5->GetPlayerID());
						return;
					}
					else
					{
						arayan5->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 0 %d", index);
						LPPARTY pParty = CPartyManager::instance().CreateParty(FirstPlayer_5);
						pParty->Join(arayan5->GetPlayerID());
						arayan5->WarpSet(SpawnDungeons[index][0]*DUNGEON_MAP_VALUE, SpawnDungeons[index][1]*DUNGEON_MAP_VALUE);
						#ifdef __PARTY_MATCH___TEST_SYSLOG
						sys_log(0, "CGroupMatchManager::CheckPlayers: arayan5\n id:%u", arayan5->GetPlayerID());
						#endif
					}
				}
			}
		}
	}
}

void CGroupMatchManager::AddToControl(LPCHARACTER ch, int index)
{
	if (!ch || !ch->IsPC() || index > DUNGEON_MAX_INDEX)
	{
		#ifdef __PARTY_MATCH___TEST_SYSLOG
		sys_log(0, "CGroupMatchManager::AddToControl: error: %d", index);
		#endif
		return;
	}
	if (quest::CQuestManager::instance().GetEventFlag("__PARTY_MATCH___giris") == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("group girisleri kapatildi"));
		return;
	}	

	if (!group_search_map_allow_find(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup map"));
		return;
	}	

	#ifdef ENABLE_WAIT_TIME
	if (wait_time && (get_global_time() - GetSearchLoadTime(ch)) < search_wait)
	{	
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup bekle %d"), (search_wait + GetSearchLoadTime(ch)) - get_global_time());
		return;
	}
	#endif	
	if (ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("group olu"));
		return;
	}
	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("group zindan"));
		return;
	}	
	if (ch->GetExchange() || ch->IsOpenSafebox() || ch->IsCubeOpen() || ch->GetShop() || ch->GetMyShop()
#ifdef ENABLE_ITEM_COMBINATION
	|| ch->IsCombOpen()
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
    || ch->IsAcceOpen()
#endif
#ifdef ENABLE_OFFLINE_SHOP
    || ch->GetOfflineShop() || ch->GetOfflineShopOwner()
#endif
    )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("group penceree"));
		return;
	}
	DWORD id = ch->GetPlayerID();
	if (index == 0)
	{
		if (ch->GetLevel() < d0_need_lv)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %d level."), d0_need_lv);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		if (ch->CountSpecifyItem(d0_need_1) < d0_need_1_count)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %s yok."), ITEM_MANAGER::instance().GetTable(d0_need_1)->szLocaleName);

			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		if (ch->CountSpecifyItem(d0_need_2) < d0_need_2_count)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %s yok."), ITEM_MANAGER::instance().GetTable(d0_need_2)->szLocaleName);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		if (ch->GetParty() != NULL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));
			return;
		}
		if (first0 == 0)
		{
			first0 = id;
		}
		arayanlar0.insert(std::make_pair(id, id));
	}
	else if (index == 1)
	{
		if (ch->GetLevel() < d1_need_lv)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %d level."), d1_need_lv);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		if (ch->CountSpecifyItem(d1_need_1) < d1_need_1_count)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %s yok."), ITEM_MANAGER::instance().GetTable(d1_need_1)->szLocaleName);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		if (ch->GetParty() != NULL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));
			return;
		}
		if (first1 == 0)
		{
			first1 = id;
		}
		arayanlar1.insert(std::make_pair(id, id));
	}
	else if (index == 2)
	{
		if (ch->GetLevel() < d2_need_lv)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %d level."), d2_need_lv);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		#ifdef NEW_NEED_ITEMS
		if (ch->CountSpecifyItem(D2_ITEM_1_VNUM) < D2_ITEM_1_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %s yok."), ITEM_MANAGER::instance().GetTable(D2_ITEM_1_VNUM)->szLocaleName);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		#endif
		if (ch->GetParty() != NULL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));
			return;
		}
		if (first2 == 0)
		{
			first2 = id;
		}
		if (ch->GetJob() == 3 || ch->GetJob() == 7)
		{
			have_shaman = have_shaman + 1;
		}
		if (ch->GetJob() == 1 || ch->GetJob() == 5)
		{
			have_ninja = have_ninja + 1;
		}
		arayanlar2.insert(std::make_pair(id, id));
	}
	else if (index == 3)
	{
		if (ch->GetLevel() < d3_need_lv)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %d level."), d3_need_lv);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		if (ch->CountSpecifyItem(d3_need_1) < d3_need_1_count)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %s yok."), ITEM_MANAGER::instance().GetTable(d3_need_1)->szLocaleName);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		if (ch->GetParty() != NULL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));
			return;
		}
		if (first3 == 0)
		{
			first3 = id;
		}
		arayanlar3.insert(std::make_pair(id, id));
	}
	else if (index == 4)
	{
		if (ch->GetLevel() < d4_need_lv)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %d level."), d4_need_lv);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		#ifdef NEW_NEED_ITEMS
		if (ch->CountSpecifyItem(D4_ITEM_1_VNUM) < D4_ITEM_1_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %s yok."), ITEM_MANAGER::instance().GetTable(D4_ITEM_1_VNUM)->szLocaleName);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		#endif
		if (ch->GetParty() != NULL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));
			return;
		}
		if (first4 == 0)
		{
			first4 = id;
		}
		arayanlar4.insert(std::make_pair(id, id));
	}
	else if (index == 5)
	{
		if (ch->GetLevel() < d5_need_lv)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %d level."), d5_need_lv);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		if (ch->CountSpecifyItem(d5_need_1) < d5_need_1_count)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yeterli %s yok."), ITEM_MANAGER::instance().GetTable(d5_need_1)->szLocaleName);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 61 %d", index);
			return;
		}
		if (ch->GetParty() != NULL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("grup varken yapamazsınız."));
			return;
		}
		if (first5 == 0)
		{
			first5 = id;
		}
		arayanlar5.insert(std::make_pair(id, id));
	}
	ch->ChatPacket(CHAT_TYPE_COMMAND, "gorup_match_search 1 %d", index);
	CheckPlayers(index);
	#ifdef ENABLE_WAIT_TIME
	if (wait_time)
	{
		SetSearchLoadTime(ch);
	}
	#endif
}

void CGroupMatchManager::group_search_config_init()
{
	if (!g_bAuthServer)
	{
		FILE * file;

		char buf[256], token_string[256], value_string[256];

		if (!(file = fopen("GROUP_SEARCH_CONFIG", "r")))
		{
			fprintf(stderr, "Can not open [GROUP_SEARCH_CONFIG]\n");
			exit(1);
		}

		while (fgets(buf, 256, file))
		{
			parse_token(buf, token_string, value_string);

			TOKEN("D0_ENTER_NUM")
			{
				str_to_number(enter_0, value_string);
			}	
			TOKEN("D1_ENTER_NUM")
			{
				str_to_number(enter_1, value_string);
			}
			TOKEN("D2_ENTER_NUM")
			{
				str_to_number(enter_2, value_string);
			}
			TOKEN("D3_ENTER_NUM")
			{
				str_to_number(enter_3, value_string);
			}			
			TOKEN("D4_ENTER_NUM")
			{
				str_to_number(enter_4, value_string);
			}			
			TOKEN("D5_ENTER_NUM")
			{
				str_to_number(enter_5, value_string);
			}
			
			//settings
			TOKEN("D2_MIN_SHAMAN")
			{
				str_to_number(d2_min_shaman, value_string);
			}		
			TOKEN("D2_MIN_ASSASSIN")
			{
				str_to_number(d2_min_assassin, value_string);
			}		
			//need items 0 
			TOKEN("D0_NEED_1_ITEM_VNUM")
			{
				str_to_number(d0_need_1, value_string);
			}		
			TOKEN("D0_NEED_1_ITEM_COUNT")
			{
				str_to_number(d0_need_1_count, value_string);
			}			
			TOKEN("D0_NEED_2_ITEM_VNUM")
			{
				str_to_number(d0_need_2, value_string);
			}
			TOKEN("D0_NEED_2_ITEM_COUNT")
			{
				str_to_number(d0_need_2_count, value_string);
			}	
			
			//need items 1
			TOKEN("D1_NEED_1_ITEM_VNUM")
			{
				str_to_number(d1_need_1, value_string);
			}			
			TOKEN("D1_NEED_1_ITEM_COUNT")
			{
				str_to_number(d1_need_1_count, value_string);
			}
			
			//need items 3
			TOKEN("D3_NEED_1_ITEM_VNUM")
			{
				str_to_number(d3_need_1, value_string);
			}			
			TOKEN("D3_NEED_1_ITEM_COUNT")
			{
				str_to_number(d3_need_1_count, value_string);
			}
			
			//need items 5
			TOKEN("D5_NEED_1_ITEM_VNUM")
			{
				str_to_number(d5_need_1, value_string);
			}		
			TOKEN("D5_NEED_1_ITEM_COUNT")
			{
				str_to_number(d5_need_1_count, value_string);
			}
			
			//need levels
			TOKEN("D0_NEED_LEVEL")
			{
				str_to_number(d0_need_lv, value_string);
			}		
			TOKEN("D1_NEED_LEVEL")
			{
				str_to_number(d1_need_lv, value_string);
			}
			TOKEN("D2_NEED_LEVEL")
			{
				str_to_number(d2_need_lv, value_string);
			}
			TOKEN("D3_NEED_LEVEL")
			{
				str_to_number(d3_need_lv, value_string);
			}		
			TOKEN("D4_NEED_LEVEL")
			{
				str_to_number(d4_need_lv, value_string);
			}			
			TOKEN("D5_NEED_LEVEL")
			{
				str_to_number(d5_need_lv, value_string);
			}
			#ifdef ENABLE_WAIT_TIME
			TOKEN("SEARCH_WAIT_TIME")
			{
				char arg1[256];
				one_argument(value_string, arg1, sizeof(arg1));

				if (!*arg1)
				{
					fprintf(stderr, "SEARCH_WAIT_TIME syntax: <disable or enable> <time>");
					exit(1);
				}

				if (!strcmp(arg1, "enable"))
					wait_time = true;
				else if (!strcmp(arg1, "disable"))
					wait_time = false;
				else if (isnhdigit(*arg1))
					str_to_number(search_wait, value_string);
			}
			#endif
			TOKEN("GROUP_SEARCH_BLOCK_MAP_ALLOW")
			{
				char * p = value_string;
				string stNum;

				for(; *p; p++)
				{
					if (isnhspace(*p))
					{
						if (stNum.length())
						{
							int index = 0;
							str_to_number(index, stNum.c_str());
							group_search_map_allow_add(index);
							stNum.clear();
						}
					}
					else
						stNum += *p;
				}
				if (stNum.length())
				{
					int index = 0;
					str_to_number(index, stNum.c_str());
					group_search_map_allow_add(index);
				}
				continue;
			}
		}
	}
}
   
bool CGroupMatchManager::group_search_map_allow_find(int mapIndex)
{	
	if (yasakli_haritalar.find(mapIndex) == yasakli_haritalar.end())
		return true;
	
	return false;
}

void CGroupMatchManager::group_search_map_allow_add(int mapIndex)
{
	if (!group_search_map_allow_find(mapIndex))
	{
		fprintf(stderr, "<GROUP SEARCH> Multiple block map allow detected!");
		exit(1);
	}
	
	fprintf(stdout, "GROUP SEARCH BLOCK MAP ALLOW: %d\n", mapIndex);
	yasakli_haritalar.insert(mapIndex);
}
#endif

