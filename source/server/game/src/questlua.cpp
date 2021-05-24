#include "stdafx.h"

#include <sstream>

#include "questmanager.h"
#include "questlua.h"
#include "config.h"
#include "desc.h"
#include "char.h"
#include "char_manager.h"
#include "buffer_manager.h"
#include "db.h"
#include "xmas_event.h"
#include "locale_service.h"
#include "regen.h"
#include "affect.h"
#include "guild.h"
#include "guild_manager.h"
#include "sectree_manager.h"

#undef sys_err
#ifndef _WIN32
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#define sys_err_direct(fmt, args...) _sys_err(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define sys_err_direct(fmt, ...) _sys_err(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	using namespace std;

	string ScriptToString(const string& str)
	{
		lua_State* L = CQuestManager::instance().GetLuaState();
		int x = lua_gettop(L);

		int errcode = lua_dobuffer(L, ("return "+str).c_str(), str.size()+7, "ScriptToString");
		string retstr;
		if (!errcode)
		{
			if (lua_isstring(L,-1))
				retstr = lua_tostring(L, -1);
		}
		else
		{
			sys_err("LUA ScriptRunError (code:%d src:[%s])", errcode, str.c_str());
		}
		lua_settop(L,x);
		return retstr;
	}

	void FSetWarpLocation::operator() (LPCHARACTER ch)
	{
		if (ch->IsPC())
		{
			ch->SetWarpLocation (map_index, x, y);
		}
	}

	void FSetQuestFlag::operator() (LPCHARACTER ch)
	{
		if (!ch->IsPC())
			return;

		PC * pPC = CQuestManager::instance().GetPCForce(ch->GetPlayerID());

		if (pPC)
			pPC->SetFlag(flagname, value);
	}

	bool FPartyCheckFlagLt::operator() (LPCHARACTER ch)
	{
		if (!ch->IsPC())
			return false;

		PC * pPC = CQuestManager::instance().GetPCForce(ch->GetPlayerID());
		bool returnBool = false;
		if (pPC)
		{
			int flagValue = pPC->GetFlag(flagname);
			if (value > flagValue)
				returnBool = true;
			else
				returnBool = false;
		}

		return returnBool;
	}

	FPartyChat::FPartyChat(int ChatType, const char* str) : iChatType(ChatType), str(str)
	{
	}

	void FPartyChat::operator() (LPCHARACTER ch)
	{
		ch->ChatPacket(iChatType, "%s", str);
	}

	void FPartyClearReady::operator() (LPCHARACTER ch)
	{
		ch->RemoveAffect(AFFECT_DUNGEON_READY);
	}

	void FSendPacket::operator() (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;

			if (ch->GetDesc())
			{
				ch->GetDesc()->Packet(buf.read_peek(), buf.size());
			}
		}
	}

	void FSendPacketToEmpire::operator() (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;

			if (ch->GetDesc())
			{
				if (ch->GetEmpire() == bEmpire)
					ch->GetDesc()->Packet(buf.read_peek(), buf.size());
			}
		}
	}

	void FWarpEmpire::operator() (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;

			if (ch->IsPC() && ch->GetEmpire() == m_bEmpire)
			{
				ch->WarpSet(m_x, m_y, m_lMapIndexTo);
			}
		}
	}

	FBuildLuaGuildWarList::FBuildLuaGuildWarList(lua_State * lua_state) : L(lua_state), m_count(1)
	{
		lua_newtable(lua_state);
	}

	void FBuildLuaGuildWarList::operator() (DWORD g1, DWORD g2)
	{
		CGuild* g = CGuildManager::instance().FindGuild(g1);

		if (!g)
			return;

		if (g->GetGuildWarType(g2) == GUILD_WAR_TYPE_FIELD)
			return;

		if (g->GetGuildWarState(g2) != GUILD_WAR_ON_WAR)
			return;

		lua_newtable(L);
		lua_pushnumber(L, g1);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, g2);
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, m_count++);
	}

	bool IsScriptTrue(const char* code, int size)
	{
		if (size==0)
			return true;

		lua_State* L = CQuestManager::instance().GetLuaState();
		int x = lua_gettop(L);
		int errcode = lua_dobuffer(L, code, size, "IsScriptTrue");
		int bStart = lua_toboolean(L, -1);
		if (errcode)
		{
			char buf[100];
			snprintf(buf, sizeof(buf), "LUA ScriptRunError (code:%%d src:[%%%ds])", size);
			sys_err(buf, errcode, code);
		}
		lua_settop(L,x);
		return bStart != 0;
	}

	void combine_lua_string(lua_State * L, ostringstream & s)
	{
		char buf[32];

		int n = lua_gettop(L);
		int i;

		for (i = 1; i <= n; ++i)
		{
			if (lua_isstring(L,i))
				//printf("%s\n",lua_tostring(L,i));
				s << lua_tostring(L, i);
			else if (lua_isnumber(L, i))
			{
				snprintf(buf, sizeof(buf), "%.14g\n", lua_tonumber(L,i));
				s << buf;
			}
		}
	}

	//
	// "member" Lua functions
	//
	int member_chat(lua_State* L)
	{
		ostringstream s;
		combine_lua_string(L, s);
		CQuestManager::Instance().GetCurrentPartyMember()->ChatPacket(CHAT_TYPE_TALKING, "%s", s.str().c_str());
		return 0;
	}

	int member_clear_ready(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentPartyMember();
		ch->RemoveAffect(AFFECT_DUNGEON_READY);
		return 0;
	}

	int member_set_ready(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentPartyMember();
		ch->AddAffect(AFFECT_DUNGEON_READY, POINT_NONE, 0, AFF_DUNGEON_READY, 65535, 0, true);
		return 0;
	}

	int mob_spawn(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		DWORD mob_vnum = (DWORD)lua_tonumber(L, 1);
		long local_x = (long) lua_tonumber(L, 2)*100;
		long local_y = (long) lua_tonumber(L, 3)*100;
		float radius = (float) lua_tonumber(L, 4)*100;
		bool bAggressive = lua_toboolean(L, 5);
		DWORD count = (lua_isnumber(L, 6))?(DWORD) lua_tonumber(L, 6):1;

		if (count == 0)
			count = 1;
		else if (count > 10)
		{
			sys_err("count bigger than 10");
			count = 10;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(ch->GetMapIndex());
		if (pMap == NULL) {
			return 0;
		}
		DWORD dwQuestIdx = CQuestManager::instance().GetCurrentPC()->GetCurrentQuestIndex();

		bool ret = false;
		LPCHARACTER mob = NULL;

		while (count--)
		{
			for (int loop = 0; loop < 8; ++loop)
			{
				float angle = number(0, 999) * M_PI * 2 / 1000;
				float r = number(0, 999) * radius / 1000;

				long x = local_x + pMap->m_setting.iBaseX + (long)(r * cos(angle));
				long y = local_y + pMap->m_setting.iBaseY + (long)(r * sin(angle));

				mob = CHARACTER_MANAGER::instance().SpawnMob(mob_vnum, ch->GetMapIndex(), x, y, 0);

				if (mob)
					break;
			}

			if (mob)
			{
				if (bAggressive)
					mob->SetAggressive();

				mob->SetQuestBy(dwQuestIdx);

#ifdef __12ZI__
				if (mob->IsRaceFlag(RACE_FLAG_ATT_CZ) && !mob->IsStone() && ch->GetDungeon())
				{
					bool boss = (mob->GetRaceNum() >= 2752 && mob->GetRaceNum() <= 2862)  ? 1 : 0;
					mob->SetLevel(GetRandomLevel(ch->GetDungeon()->GetFlag("zodiac_floor"), boss));
					int newHP = mob->GetMaxHP() + (mob->GetLevel() * 5000);
					mob->SetMaxHP(newHP);
					mob->SetHP(newHP);
					mob->UpdatePacket();
				}
#endif
				if (!ret)
				{
					ret = true;
					lua_pushnumber(L, (DWORD) mob->GetVID());
				}
			}
		}

		if (!ret)
			lua_pushnumber(L, 0);

		return 1;
	}

	int mob_spawn_group(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 6))
		{
			sys_err("invalid argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		DWORD group_vnum = (DWORD)lua_tonumber(L, 1);
		long local_x = (long) lua_tonumber(L, 2) * 100;
		long local_y = (long) lua_tonumber(L, 3) * 100;
		float radius = (float) lua_tonumber(L, 4) * 100;
		bool bAggressive = lua_toboolean(L, 5);
		DWORD count = (DWORD) lua_tonumber(L, 6);

		if (count == 0)
			count = 1;
		else if (count > 10)
		{
			sys_err("count bigger than 10");
			count = 10;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(ch->GetMapIndex());
		if (pMap == NULL) {
			lua_pushnumber(L, 0);
			return 1;
		}
		DWORD dwQuestIdx = CQuestManager::instance().GetCurrentPC()->GetCurrentQuestIndex();

		bool ret = false;
		LPCHARACTER mob = NULL;

		while (count--)
		{
			for (int loop = 0; loop < 8; ++loop)
			{
				float angle = number(0, 999) * M_PI * 2 / 1000;
				float r = number(0, 999)*radius/1000;

				long x = local_x + pMap->m_setting.iBaseX + (long)(r * cos(angle));
				long y = local_y + pMap->m_setting.iBaseY + (long)(r * sin(angle));

				mob = CHARACTER_MANAGER::instance().SpawnGroup(group_vnum, ch->GetMapIndex(), x, y, x, y, NULL, bAggressive);

				if (mob)
					break;
			}

			if (mob)
			{
				mob->SetQuestBy(dwQuestIdx);

				if (!ret)
				{
					ret = true;
					lua_pushnumber(L, (DWORD) mob->GetVID());
				}
			}
		}

		if (!ret)
			lua_pushnumber(L, 0);

		return 1;
	}

	//
	// global Lua functions
	//
	//
	// Registers Lua function table
	//
	void CQuestManager::AddLuaFunctionTable(const char * c_pszName, luaL_reg * preg, bool bCheckIfExists)
	{
#ifdef __NEWSTUFF__
		bool bIsExists = false;
		if (bCheckIfExists)
		{
			int x = lua_gettop(L);
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				lua_settop(L, x);
				bIsExists = true;
			}
		}
		if (!bIsExists)
			lua_newtable(L);
#else
		lua_newtable(L);
#endif

		while ((preg->name))
		{
			lua_pushstring(L, preg->name);
			lua_pushcfunction(L, preg->func);
			lua_rawset(L, -3);
			preg++;
		}

		lua_setglobal(L, c_pszName);
	}

	void CQuestManager::AddLuaFunctionSubTable(const char * c_pszName, const char * c_pszSubName, luaL_reg * preg)
	{
		// lua_State* L = CQuestManager::instance().GetLuaState();
		int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				sys_err("%s global index not found for %s", c_pszName, c_pszSubName);
				lua_settop(L, x);
				return;
			}
			lua_pushstring(L, c_pszSubName);
			{
				lua_newtable(L);
				while ((preg->name))
				{
					lua_pushstring(L, preg->name);
					lua_pushcfunction(L, preg->func);
					lua_rawset(L, -3);
					preg++;
				}
			}
			lua_rawset(L, -3);
			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}

#ifdef __NEWSTUFF__
	void CQuestManager::AppendLuaFunctionTable(const char * c_pszName, luaL_reg * preg, bool bForceCreation)
	{
		int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				sys_err("%s global index not found (force=%d)", c_pszName, bForceCreation);
				lua_settop(L, x);
				if (bForceCreation)
					AddLuaFunctionTable(c_pszName, preg);
				return;
			}

			while ((preg->name))
			{
				lua_pushstring(L, preg->name);
				lua_pushcfunction(L, preg->func);
				lua_rawset(L, -3);
				preg++;
			}

			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}

	void CQuestManager::AddLuaConstantGlobal(const char * c_pszName, lua_Number lNumber, bool bOverwrite)
	{
		int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (lua_isnumber(L, -1))
			{
				if (!bOverwrite)
				{
					sys_err("%s global index already defined", c_pszName);
					lua_settop(L, x);
					return;
				}
			}
			lua_pushnumber(L, lNumber);
			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}

	void CQuestManager::AddLuaConstantInTable(const char * c_pszName, const char * c_pszSubName, lua_Number lNumber, bool bForceCreation)
	{
		int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				if (!bForceCreation)
				{
					sys_err("%s global index for %s already defined", c_pszName, c_pszSubName);
					lua_settop(L, x);
					return;
				}
				lua_newtable(L);
			}
			{
				lua_pushstring(L, c_pszSubName);
				lua_pushnumber(L, lNumber);
				lua_rawset(L, -3);
			}
			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}

	void CQuestManager::AddLuaConstantInTable(const char * c_pszName, const char * c_pszSubName, const char * szString, bool bForceCreation)
	{
		int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				if (!bForceCreation)
				{
					sys_err("%s global index for %s already defined", c_pszName, c_pszSubName);
					lua_settop(L, x);
					return;
				}
				lua_newtable(L);
			}
			{
				lua_pushstring(L, c_pszSubName);
				lua_pushstring(L, szString);
				lua_rawset(L, -3);
			}
			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}

	void CQuestManager::AddLuaConstantSubTable(const char * c_pszName, const char * c_pszSubName, luaC_tab * preg)
	{
		// lua_State* L = CQuestManager::instance().GetLuaState();
		int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				sys_err("%s global index not found for %s", c_pszName, c_pszSubName);
				lua_settop(L, x);
				return;
			}
			lua_pushstring(L, c_pszSubName);
			{
				lua_newtable(L);
				while ((preg->name))
				{
					lua_pushstring(L, preg->name);
					switch (preg->val.type)
					{
						case ETL_CFUN:
							lua_pushcfunction(L, preg->val.cfVal);
							break;
						case ETL_LNUM:
							lua_pushnumber(L, preg->val.lnVal);
							break;
						case ETL_LSTR:
							lua_pushstring(L, preg->val.lsVal);
							break;
						case ETL_NIL:
							lua_pushnil(L);
							break;
					}
					lua_rawset(L, -3);
					preg++;
				}
			}
			lua_rawset(L, -3);
			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}
#endif

	void CQuestManager::BuildStateIndexToName(const char* questName)
	{
		int x = lua_gettop(L);
		lua_getglobal(L, questName);

		if (lua_isnil(L,-1))
		{
			sys_err("QUEST wrong quest state file for quest %s",questName);
			lua_settop(L,x);
			return;
		}

		for (lua_pushnil(L); lua_next(L, -2);)
		{
			if (lua_isstring(L, -2) && lua_isnumber(L, -1))
			{
				lua_pushvalue(L, -2);
				lua_rawset(L, -4);
			}
			else
			{
				lua_pop(L, 1);
			}
		}

		lua_settop(L, x);
	}

	/**
	 * @version 05/06/08	Bang2ni - __get_guildid_byname 스크립트 함수 등록
	 */
	bool CQuestManager::InitializeLua()
	{
#if LUA_V == 503
		L = lua_open();

		luaopen_base(L);
		luaopen_table(L);
		luaopen_string(L);
		luaopen_math(L);
		//TEMP
		luaopen_io(L);
		luaopen_debug(L);
#elif LUA_V == 523
		L = luaL_newstate();

		luaL_openlibs(L);
		//luaopen_debug(L);
#else
	#error "lua version not found"
#endif

		RegisterAffectFunctionTable();
		RegisterBuildingFunctionTable();
		RegisterDungeonFunctionTable();
		RegisterGameFunctionTable();
		RegisterGuildFunctionTable();
		RegisterHorseFunctionTable();
#ifdef __PET__
		RegisterPetFunctionTable();
#endif
		RegisterITEMFunctionTable();
		RegisterMarriageFunctionTable();
		RegisterNPCFunctionTable();
		RegisterPartyFunctionTable();
		RegisterPCFunctionTable();
		RegisterQuestFunctionTable();
		RegisterTargetFunctionTable();
		RegisterArenaFunctionTable();
		RegisterOXEventFunctionTable();
		RegisterDanceEventFunctionTable();
		RegisterDragonLairFunctionTable();
		RegisterDragonSoulFunctionTable();
#ifdef __MOUNT__
		RegisterMountFunctionTable();
#endif 
#ifdef __GROWTH_PET__
		RegisterNewPetFunctionTable();
#endif
#ifdef __GUILD_DRAGONLAIR__
		RegisterMeleyLairFunctionTable();
#endif
#ifdef __TEMPLE_OCHAO__
		RegisterTempleOchaoFunctionTable();
#endif
#ifdef __PARTY_MATCH__
		RegisterGroupMatchFunctionTable();
#endif
#ifdef __SUPPORT__
		RegisterSupportFunctionTable();
#endif
		{
			luaL_reg member_functions[] =
			{
				{ "chat",			member_chat		},
				{ "set_ready",			member_set_ready	},
				{ "clear_ready",		member_clear_ready	},
				{ NULL,				NULL			}
			};

			AddLuaFunctionTable("member", member_functions);
		}

		{
			luaL_reg mob_functions[] =
			{
				{ "spawn",			mob_spawn		},
				{ "spawn_group",		mob_spawn_group		},
				{ NULL,				NULL			}
			};

			AddLuaFunctionTable("mob", mob_functions);
		}

		//
		// global namespace functions
		//
		RegisterGlobalFunctionTable(L);

		// LUA_INIT_ERROR_MESSAGE
		{
			char settingsFileName[256];
			snprintf(settingsFileName, sizeof(settingsFileName), "%s/settings.lua", LocaleService_GetBasePath().c_str());

			int settingsLoadingResult = lua_dofile(L, settingsFileName);
			sys_log(0, "LoadSettings(%s), returns %d", settingsFileName, settingsLoadingResult);
			if (settingsLoadingResult != 0)
			{
				sys_err("LOAD_SETTINS_FAILURE(%s)", settingsFileName);
				return false;
			}
		}

		{
			char questlibFileName[256];
			snprintf(questlibFileName, sizeof(questlibFileName), "%s/questlib.lua", LocaleService_GetQuestPath().c_str());

			int questlibLoadingResult = lua_dofile(L, questlibFileName);
			sys_log(0, "LoadQuestlib(%s), returns %d", questlibFileName, questlibLoadingResult);
			if (questlibLoadingResult != 0)
			{
				sys_err("LOAD_QUESTLIB_FAILURE(%s)", questlibFileName);
				return false;
			}
		}

#ifdef __TRANSLATE_LUA__
		{
			char translateFileName[256];
			snprintf(translateFileName, sizeof(translateFileName), "%s/translate.lua", LocaleService_GetBasePath().c_str());

			int translateLoadingResult = lua_dofile(L, translateFileName);
			sys_log(0, "LoadTranslate(%s), returns %d", translateFileName, translateLoadingResult);
			if (translateLoadingResult != 0)
			{
				sys_err("LOAD_TRANSLATE_ERROR(%s)", translateFileName);
				return false;
			}
		}
#endif

		{
			char questLocaleFileName[256];
			snprintf(questLocaleFileName, sizeof(questLocaleFileName), "%s/locale.lua", g_stQuestDir.c_str());

			int questLocaleLoadingResult = lua_dofile(L, questLocaleFileName);
			sys_log(0, "LoadQuestLocale(%s), returns %d", questLocaleFileName, questLocaleLoadingResult);
			if (questLocaleLoadingResult != 0)
			{
				sys_err("LoadQuestLocale(%s) FAILURE", questLocaleFileName);
				return false;
			}
		}
		// END_OF_LUA_INIT_ERROR_MESSAGE

		for (itertype(g_setQuestObjectDir) it = g_setQuestObjectDir.begin(); it != g_setQuestObjectDir.end(); ++it)
		{
			const string& stQuestObjectDir = *it;
			char buf[PATH_MAX];
			snprintf(buf, sizeof(buf), "%s/state/", stQuestObjectDir.c_str());
			DIR * pdir = opendir(buf);
			int iQuestIdx = 0;

			if (pdir)
			{
				dirent * pde;

				while ((pde = readdir(pdir)))
				{
					if (pde->d_name[0] == '.')
						continue;

					snprintf(buf + 11, sizeof(buf) - 11, "%s", pde->d_name);

					RegisterQuest(pde->d_name, ++iQuestIdx);
					int ret = lua_dofile(L, (stQuestObjectDir + "/state/" + pde->d_name).c_str());
					sys_log(0, "QUEST: loading %s, returns %d", (stQuestObjectDir + "/state/" + pde->d_name).c_str(), ret);

					BuildStateIndexToName(pde->d_name);
				}

				closedir(pdir);
			}
		}

#if LUA_V == 503
		lua_setgcthreshold(L, 0);
#endif
		lua_newtable(L);
		lua_setglobal(L, "__codecache");
		return true;
	}

	void CQuestManager::GotoSelectState(QuestState& qs)
	{
		lua_checkstack(qs.co, 1);

		//int n = lua_gettop(L);
		int n = luaL_getn(qs.co, -1);
		qs.args = n;
		//cout << "select here (1-" << qs.args << ")" << endl;
		//

		ostringstream os;
		os << "[QUESTION ";

		for (int i = 1; i <= n; i++)
		{
			lua_rawgeti(qs.co, -1, i);
#ifdef __MULTI_LANGUAGE__
			if (lua_isnumber(qs.co, -1))
			{
				if (i != 1)
					os << "|";

				os << i << ";" << "{" << lua_tonumber(qs.co, -1) << "}";
			}
			else if (lua_isstring(qs.co, -1))
#else
			if (lua_isstring(qs.co, -1))
#endif
			{
				if (i != 1)
					os << "|";

				os << i << ";" << lua_tostring(qs.co, -1);
			}
			else
			{
				sys_err("SELECT wrong data %s", lua_typename(qs.co, -1));
				sys_err("here");
			}
			lua_pop(qs.co,1);
		}
		os << "]";


		AddScript(os.str());
		qs.suspend_state = SUSPEND_STATE_SELECT;
		SendScript();
	}

	EVENTINFO(confirm_timeout_event_info)
	{
		DWORD dwWaitPID;
		DWORD dwReplyPID;

		confirm_timeout_event_info()
		: dwWaitPID( 0 )
		, dwReplyPID( 0 )
		{
		}
	};

	EVENTFUNC(confirm_timeout_event)
	{
		confirm_timeout_event_info * info = dynamic_cast<confirm_timeout_event_info *>(event->info);

		if ( info == NULL )
		{
			sys_err( "confirm_timeout_event> <Factor> Null pointer" );
			return 0;
		}

		LPCHARACTER chWait = CHARACTER_MANAGER::instance().FindByPID(info->dwWaitPID);
		LPCHARACTER chReply = NULL; //CHARACTER_MANAGER::info().FindByPID(info->dwReplyPID);

		if (chReply)
		{
			// 시간 지나면 알아서 닫힘
		}

		if (chWait)
		{
			CQuestManager::instance().Confirm(info->dwWaitPID, CONFIRM_TIMEOUT);
		}

		return 0;
	}

	void CQuestManager::GotoConfirmState(QuestState & qs)
	{
		qs.suspend_state = SUSPEND_STATE_CONFIRM;
		DWORD dwVID = (DWORD) lua_tonumber(qs.co, -3);
		const char* szMsg = lua_tostring(qs.co, -2);
		int iTimeout = (int) lua_tonumber(qs.co, -1);

		sys_log(0, "GotoConfirmState vid %u msg '%s', timeout %d", dwVID, szMsg, iTimeout);

		// 1. 상대방에게 확인창 띄움
		// 2. 나에게 확인 기다린다고 표시하는 창 띄움
		// 3. 타임아웃 설정 (타임아웃 되면 상대방 창 닫고 나에게도 창 닫으라고 보냄)

		// 1
		// 상대방이 없는 경우는 그냥 상대방에게 보내지 않는다. 타임아웃에 의해서 넘어가게됨
		LPCHARACTER ch = CHARACTER_MANAGER::instance().Find(dwVID);
		if (ch && ch->IsPC())
		{
			ch->ConfirmWithMsg(szMsg, iTimeout, GetCurrentCharacterPtr()->GetPlayerID());
		}

		// 2
		GetCurrentPC()->SetConfirmWait((ch && ch->IsPC())?ch->GetPlayerID():0);
		ostringstream os;
		os << "[CONFIRM_WAIT timeout;" << iTimeout << "]";
		AddScript(os.str());
		SendScript();

		// 3
		confirm_timeout_event_info* info = AllocEventInfo<confirm_timeout_event_info>();

		info->dwWaitPID = GetCurrentCharacterPtr()->GetPlayerID();
		info->dwReplyPID = (ch && ch->IsPC()) ? ch->GetPlayerID() : 0;

		event_create(confirm_timeout_event, info, PASSES_PER_SEC(iTimeout));
	}

	void CQuestManager::GotoSelectItemState(QuestState& qs)
	{
		qs.suspend_state = SUSPEND_STATE_SELECT_ITEM;
		AddScript("[SELECT_ITEM]");
		SendScript();
	}

#ifdef __GEM__
	void CQuestManager::GotoSelectItemExState(QuestState& qs)
	{
		qs.suspend_state = SUSPEND_STATE_SELECT_ITEM_EX;
		AddScript("[SELECT_ITEM_EX]");
		SendScript();
	}
#endif

	void CQuestManager::GotoInputState(QuestState & qs)
	{
		qs.suspend_state = SUSPEND_STATE_INPUT;
		AddScript("[INPUT]");
		SendScript();

		// 시간 제한을 검
		//event_create(input_timeout_event, dwEI, PASSES_PER_SEC(iTimeout));
	}

	void CQuestManager::GotoPauseState(QuestState & qs)
	{
		qs.suspend_state = SUSPEND_STATE_PAUSE;
		AddScript("[NEXT]");
		SendScript();
	}

	void CQuestManager::GotoEndState(QuestState & qs)
	{
		AddScript("[DONE]");
		SendScript();
	}

	//
	// * OpenState
	//
	// The beginning of script
	//

	QuestState CQuestManager::OpenState(const string& quest_name, int state_index)
	{
		QuestState qs;
		qs.args=0;
		qs.st = state_index;
		qs.co = lua_newthread(L);
		qs.ico = lua_ref(L, 1/*qs.co*/);
		return qs;
	}

	//
	// * RunState
	//
	// decides script to wait for user input, or finish
	//
	bool CQuestManager::RunState(QuestState & qs)
	{
		ClearError();

		m_CurrentRunningState = &qs;
		int ret = lua_resume(qs.co, qs.args);

		if (ret == 0)
		{
			if (lua_gettop(qs.co) == 0)
			{
				// end of quest
				GotoEndState(qs);
				return false;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "select"))
			{
				GotoSelectState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "wait"))
			{
				GotoPauseState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "input"))
			{
				GotoInputState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "confirm"))
			{
				GotoConfirmState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "select_item"))
			{
				GotoSelectItemState(qs);
				return true;
			}
#ifdef __GEM__
			if (!strcmp(lua_tostring(qs.co, 1), "select_item_ex"))
			{
				GotoSelectItemExState(qs);
				return true;
			}
#endif
		}
		else
		{
			sys_err("LUA_ERROR: %s", lua_tostring(qs.co, 1));
		}

		WriteRunningStateToSyserr();
		SetError();

		GotoEndState(qs);
		return false;
	}

	//
	// * CloseState
	//
	// makes script end
	//
	void CQuestManager::CloseState(QuestState& qs)
	{
		if (qs.co)
		{
			//cerr << "ICO "<<qs.ico <<endl;
			lua_unref(L, qs.ico);
			qs.co = 0;
		}
	}
}
