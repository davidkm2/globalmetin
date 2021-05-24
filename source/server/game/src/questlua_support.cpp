#include "stdafx.h"
 
#include "questlua.h"
#include "questmanager.h"
#include "horsename_manager.h"
#include "char.h"
#include "affect.h"
#include "config.h"
#include "utils.h"
#include "../../common/defines.h"
#include "supportsystem.h"

#undef sys_err
#ifndef _WIN32
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

extern int (*check_name) (const char * str);

namespace quest
{

#ifdef __SUPPORT__
	// syntax in LUA: support.summon(mob_vnum, support's name, (bool)run to me from far away)
	int support_summon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CSupportSystem* supportSystem = ch->GetSupportSystem();
		LPITEM pItem = CQuestManager::instance().GetCurrentItem();
		if (!ch || !supportSystem || !pItem)
		{
			lua_pushnumber (L, 0);
			return 1;
		}

		if (0 == supportSystem)
		{
			lua_pushnumber (L, 0);
			return 1;
		}

		// 소환수의 vnum
		DWORD mobVnum= lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		// 소환수의 이름
		const char* supportName = lua_isstring(L, 2) ? lua_tostring(L, 2) : 0;

		// 소환하면 멀리서부터 달려오는지 여부
		bool bFromFar = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false;

		CSupportActor* support = supportSystem->Summon(mobVnum, pItem, supportName, bFromFar);

		if (support != NULL)
			lua_pushnumber (L, support->GetVID());
		else
			lua_pushnumber (L, 0);

		return 1;
	}

	// syntax: support.unsummon(mob_vnum)
	int support_unsummon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CSupportSystem* supportSystem = ch->GetSupportSystem();

		if (0 == supportSystem)
			return 0;

		// 소환수의 vnum
		DWORD mobVnum= lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		supportSystem->Unsummon(mobVnum);
		return 1;
	}

	// syntax: support.unsummon(mob_vnum)
	int support_count_summoned(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CSupportSystem* supportSystem = ch->GetSupportSystem();

		lua_Number count = 0;

		if (0 != supportSystem)
			count = (lua_Number)supportSystem->CountSummoned();

		lua_pushnumber(L, count);

		return 1;
	}

	// syntax: support.is_summon(mob_vnum)
	int support_is_summon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CSupportSystem* supportSystem = ch->GetSupportSystem();

		if (0 == supportSystem)
			return 0;

		// 소환수의 vnum
		DWORD mobVnum= lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		CSupportActor* supportActor = supportSystem->GetByVnum(mobVnum);

		if (NULL == supportActor)
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, supportActor->IsSummoned());

		return 1;
	}

	int support_spawn_effect(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CSupportSystem* supportSystem = ch->GetSupportSystem();

		if (0 == supportSystem)
			return 0;

		DWORD mobVnum = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		CSupportActor* supportActor = supportSystem->GetByVnum(mobVnum);
		if (NULL == supportActor)
			return 0;
		LPCHARACTER support_ch = supportActor->GetCharacter();
		if (NULL == support_ch)
			return 0;

		if (lua_isstring(L, 2))
		{
			support_ch->SpecificEffectPacket (lua_tostring(L, 2));
		}
		return 0;
	}

	void RegisterSupportFunctionTable()
	{
		luaL_reg support_functions[] =
		{
			{ "summon",			support_summon			},
			{ "unsummon",		support_unsummon		},
			{ "is_summon",		support_is_summon		},
			{ "count_summoned",	support_count_summoned	},
			{ "spawn_effect",	support_spawn_effect	},
			{ NULL,				NULL				}
		};

		CQuestManager::instance().AddLuaFunctionTable("supports", support_functions);
	}
#endif

}

