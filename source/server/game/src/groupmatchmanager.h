#ifdef __PARTY_MATCH__
#include "char.h"
#include "utils.h"

/**Test syslogs */
// #define __PARTY_MATCH___TEST_SYSLOG

/**You can add need items for dungeon(2&3) also you must add to python */
// #define NEW_NEED_ITEMS

/**You can add wait time for search */
#define ENABLE_WAIT_TIME

/****************************/

	enum ayarlar
	{
		DUNGEON_MAX_INDEX = 5, //Official 5
		OX_MAP_IDX = 113, //Official 113
		DUNGEON_MAP_VALUE = 100, //Default 100
	};

	// enum dungeon0_need_items
	// {
		// D0_ITEM_1_VNUM = 30190,
		// D0_ITEM_2_VNUM = 30179,
		// D0_ITEM_1_COUNT = 1,
		// D0_ITEM_2_COUNT = 3,
	// };

	// enum dungeon1_need_items
	// {
		// D1_ITEM_1_VNUM = 71095,
		// D1_ITEM_1_COUNT = 1,
	// };
#ifdef NEW_NEED_ITEMS
	enum dungeon2_need_items
	{
		D2_ITEM_1_VNUM = 299,
		D2_ITEM_1_COUNT = 1,
	};
	enum dungeon4_need_items
	{
		D4_ITEM_1_VNUM = 289,
		D4_ITEM_1_COUNT = 1,
	};
#endif
	// enum dungeon3_need_items
	// {
		// D3_ITEM_1_VNUM = 30613,
		// D3_ITEM_1_COUNT = 1,
	// };

	// enum dungeon5_need_items
	// {
		// D5_ITEM_1_VNUM = 71095,
		// D5_ITEM_1_COUNT = 1,
	// };
/****************************/
class CGroupMatchManager : public singleton<CGroupMatchManager>
{		
	private:
		std::map<DWORD, DWORD> arayanlar0;
		std::map<DWORD, DWORD> arayanlar1;
		std::map<DWORD, DWORD> arayanlar2;
		std::map<DWORD, DWORD> arayanlar3;
		std::map<DWORD, DWORD> arayanlar4;
		std::map<DWORD, DWORD> arayanlar5;
		
		DWORD first0;
		DWORD first1;
		DWORD first2;
		DWORD first3;
		DWORD first4;
		DWORD first5;
		
		int have_shaman;
		int have_ninja;
		
	public:
		bool Initialize();
		void Destroy();
		
		//configs:
		int enter_0;
		int enter_1;
		int enter_2;
		int enter_3;
		int enter_4;
		int enter_5;
		int d2_min_shaman;
		int d2_min_assassin;
		int search_wait;
		bool wait_time;
		int d0_need_1;
		int d0_need_2;
		int d0_need_1_count;
		int d0_need_2_count;
		int d1_need_1;
		int d1_need_1_count;
		int d3_need_1;
		int d3_need_1_count;
		int d5_need_1;
		int d5_need_1_count;
		int d0_need_lv;
		int d1_need_lv;
		int d2_need_lv;
		int d3_need_lv;
		int d4_need_lv;
		int d5_need_lv;
		//end
		
		void AddToControl(LPCHARACTER ch, int index);
		void CheckPlayers(int index);
		void SendShoutForInfo(LPCHARACTER ch, int index);
		void AramayiDurdur(DWORD id);
		
		//configs:
		void group_search_config_init();
		bool group_search_map_allow_find(int mapIndex);
		void group_search_map_allow_add(int mapIndex);
		
#ifdef ENABLE_WAIT_TIME
		int GetSearchLoadTime(LPCHARACTER ch)
		{
			return ch->GetQuestFlag("group.cooltime");
		}
		void SetSearchLoadTime(LPCHARACTER ch)
		{
			ch->SetQuestFlag("group.cooltime", get_global_time());
		}
#endif
			
		DWORD ArayanlarinSayisi0() { return arayanlar0.size(); }
		DWORD ArayanlarinSayisi1() { return arayanlar1.size(); }
		DWORD ArayanlarinSayisi2() { return arayanlar2.size(); }
		DWORD ArayanlarinSayisi3() { return arayanlar3.size(); }
		DWORD ArayanlarinSayisi4() { return arayanlar4.size(); }
		DWORD ArayanlarinSayisi5() { return arayanlar5.size(); }
};
#endif

