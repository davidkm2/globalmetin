#include "stdafx.h"
#include "questlua.h"
#include "questmanager.h"
#include "horsename_manager.h"
#include "char.h"
#include "affect.h"
#include "config.h"
#include "utils.h"
#include "db.h"

#include "mountsystem.h"

#undef sys_err
#ifndef _WIN32
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

extern int(*check_name) (const char * str);

namespace quest
{

#ifdef __MOUNT__
	int mount_summon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CMountSystem* MountSystem = ch->GetMountSystem();
		if (!ch || !MountSystem)
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		if (0 == MountSystem)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		DWORD mobVnum = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;
		
		CMountActor* MState = MountSystem->Summon(mobVnum, ch->GetName());
		
		if (MState != NULL)
			lua_pushnumber(L, 1);
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int mount_unsummon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CMountSystem* MountSystem = ch->GetMountSystem();

		if (0 == MountSystem)
			return 0;

		DWORD mobVnum = lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0;

		MountSystem->Unsummon(mobVnum);
		return 1;
	}
	
	int mount_count_summoned(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		CMountSystem* MountSystem = ch->GetMountSystem();

		lua_Number count = 0;

		if (0 != MountSystem)
			count = (lua_Number)MountSystem->CountSummoned();

		lua_pushnumber(L, count);

		return 1;
	}
	void RegisterMountFunctionTable()
	{
		luaL_reg mountF[] =
		{
			{ "summon",			mount_summon },
			{ "unsummon",			mount_unsummon },
			{ "count_summoned",	mount_count_summoned },
			{ NULL,				NULL }
		};

		CQuestManager::instance().AddLuaFunctionTable("Mount", mountF);
	}
#endif

}
