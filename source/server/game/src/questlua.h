#ifndef __HEADER_QUEST_LUA__
#define __HEADER_QUEST_LUA__

#include "quest.h"
#include "buffer_manager.h"
#ifdef __MOUNT__
#include "mountsystem.h"
#endif
namespace quest
{
	extern void RegisterPCFunctionTable();
	extern void RegisterNPCFunctionTable();
	extern void RegisterTargetFunctionTable();
	extern void RegisterAffectFunctionTable();
	extern void RegisterBuildingFunctionTable();
	extern void RegisterMarriageFunctionTable();
	extern void RegisterITEMFunctionTable();
	extern void RegisterDungeonFunctionTable();
	extern void RegisterQuestFunctionTable();
	extern void RegisterPartyFunctionTable();
	extern void RegisterHorseFunctionTable();
	extern void RegisterPetFunctionTable();
	extern void RegisterGuildFunctionTable();
	extern void RegisterGameFunctionTable();
	extern void RegisterArenaFunctionTable();
	extern void RegisterGlobalFunctionTable(lua_State* L);
	extern void RegisterOXEventFunctionTable();
	extern void RegisterMgmtFunctionTable();
	extern void RegisterDanceEventFunctionTable();
	extern void RegisterDragonLairFunctionTable();
	extern void RegisterDragonSoulFunctionTable();
#ifdef __MOUNT__
	extern void RegisterMountFunctionTable();
#endif
#ifdef __GROWTH_PET__
	extern void RegisterNewPetFunctionTable();
#endif
#ifdef __GUILD_DRAGONLAIR__
	extern void RegisterMeleyLairFunctionTable();
#endif
#ifdef __TEMPLE_OCHAO__
	extern void RegisterTempleOchaoFunctionTable();
#endif
#ifdef __PARTY_MATCH__
	extern void RegisterGroupMatchFunctionTable();
#endif
#ifdef __SUPPORT__
	extern void RegisterSupportFunctionTable();
#endif
	extern void combine_lua_string(lua_State* L, std::ostringstream &s);

	struct FSetWarpLocation
	{
		long map_index;
		long x;
		long y;

		FSetWarpLocation (long _map_index, long _x, long _y) :
			map_index (_map_index), x (_x), y (_y)
		{}
		void operator () (LPCHARACTER ch);
	};

	struct FSetQuestFlag
	{
		std::string flagname;
		int value;

		void operator () (LPCHARACTER ch);
	};

	struct FPartyCheckFlagLt
	{
		std::string flagname;
		int value;

		bool operator () (LPCHARACTER ch);
	};

	struct FPartyChat
	{
		int iChatType;
		const char* str;

		FPartyChat(int ChatType, const char* str);
		void operator() (LPCHARACTER ch);
	};

	struct FPartyClearReady
	{
		void operator() (LPCHARACTER ch);
	};

	struct FSendPacket
	{
		TEMP_BUFFER buf;

		void operator() (LPENTITY ent);
	};

	struct FSendPacketToEmpire
	{
		TEMP_BUFFER buf;
		BYTE bEmpire;

		void operator() (LPENTITY ent);
	};

	struct FWarpEmpire
	{
		BYTE m_bEmpire;
		long m_lMapIndexTo;
		long m_x;
		long m_y;

		void operator() (LPENTITY ent);
	};

	EVENTINFO(warp_all_to_map_my_empire_event_info)
	{
		BYTE 	m_bEmpire;
		long	m_lMapIndexFrom;
		long 	m_lMapIndexTo;
		long 	m_x;
		long	m_y;

		warp_all_to_map_my_empire_event_info()
		: m_bEmpire( 0 )
		, m_lMapIndexFrom( 0 )
		, m_lMapIndexTo( 0 )
		, m_x( 0 )
		, m_y( 0 )
		{
		}
	};

	EVENTFUNC(warp_all_to_map_my_empire_event);

	struct FBuildLuaGuildWarList
	{
		lua_State * L;
		int m_count;

		FBuildLuaGuildWarList(lua_State * L);
		void operator() (DWORD g1, DWORD g2);
	};
}
#endif /*__HEADER_QUEST_LUA__*/

