#include "stdafx.h"
#include "locale_service.h"
#include "constants.h"
#include "banword.h"
#include "utils.h"
#include "mob_manager.h"
#include "config.h"
#include "../../common/defines.h"

using namespace std;

extern string		g_stQuestDir;
extern set<string> 	g_setQuestObjectDir;

string g_stServiceName;
string g_stServiceBasePath = ".";
string g_stServiceMapPath = "data/map";

string g_stLocale = "euckr";
string g_stLocaleFilename;

BYTE PK_PROTECT_LEVEL = 30;

string 			g_stLocal = "";

int (*check_name) (const char * str) = NULL;
int (*is_twobyte) (const char * str) = NULL;

int is_twobyte_euckr(const char * str)
{
	return ishan(*str);
}

int check_name_independent(const char * str)
{
	if (CBanwordManager::instance().CheckString(str, strlen(str)))
		return 0;

	// 몬스터 이름으로는 만들 수 없다.
	char szTmp[256];
	str_lower(str, szTmp, sizeof(szTmp));

	if (CMobManager::instance().Get(szTmp, false))
		return 0;

	return 1;
}

int check_name_alphabet(const char * str)
{
	const char*	tmp;

	if (!str || !*str)
		return 0;

	if (strlen(str) < 2)
		return 0;

	for (tmp = str; *tmp; ++tmp)
	{
		// 알파벳과 수자만 허용
		if (isdigit(*tmp) || isalpha(*tmp))
			continue;
		else
			return 0;
	}

	return check_name_independent(str);
}

void LocaleService_LoadLocaleStringFile()
{
	if (g_stLocaleFilename.empty())
		return;

	if (g_bAuthServer)
		return;

	fprintf(stderr, "LocaleService %s\n", g_stLocaleFilename.c_str());

	locale_init(g_stLocaleFilename.c_str());
}

static void __LocaleService_Init_Turkey()
{
	g_stLocale="latin5";
	g_stServiceBasePath = "locale/turkey";
	g_stQuestDir = "locale/turkey/quest";
	g_stServiceMapPath = "locale/turkey/map";

	g_setQuestObjectDir.clear();
	g_setQuestObjectDir.insert("locale/turkey/quest/object");
	g_stLocaleFilename = "locale/turkey/locale_string.txt";

	check_name = check_name_alphabet;

	PK_PROTECT_LEVEL = 15;
}

static void __CheckPlayerSlot(const std::string& service_name)
{
#ifndef __PLAYER_PER_ACCOUNT5__
	if (PLAYER_PER_ACCOUNT != 4)
	{
		printf("<ERROR> PLAYER_PER_ACCOUNT = %d\n", PLAYER_PER_ACCOUNT);
		exit(0);
	}
#endif
}

bool LocaleService_Init(const std::string& c_rstServiceName)
{
	if (!g_stServiceName.empty())
	{
		sys_err("ALREADY exist service");
		return false;
	}

	g_stServiceName = c_rstServiceName;

	__LocaleService_Init_Turkey();

	fprintf(stdout, "Setting Locale \"%s\" (Path: %s)\n", g_stServiceName.c_str(), g_stServiceBasePath.c_str());

	__CheckPlayerSlot(g_stServiceName);

	return true;
}

void LocaleService_TransferDefaultSetting()
{
	if (!is_twobyte)
		is_twobyte = is_twobyte_euckr;

	if (!exp_table)
		exp_table = exp_table_common;

#ifdef __GROWTH_PET__
	if (!exppet_table)
		exppet_table = exppet_table_common;
#endif

	if (!aiPercentByDeltaLevForBoss)
		aiPercentByDeltaLevForBoss = aiPercentByDeltaLevForBoss_euckr;

	if (!aiPercentByDeltaLev)
		aiPercentByDeltaLev = aiPercentByDeltaLev_euckr;

	if (!aiChainLightningCountBySkillLevel)
		aiChainLightningCountBySkillLevel = aiChainLightningCountBySkillLevel_euckr;
}

const std::string& LocaleService_GetBasePath()
{
	return g_stServiceBasePath;
}

const std::string& LocaleService_GetMapPath()
{
	return g_stServiceMapPath;
}

const std::string& LocaleService_GetQuestPath()
{
	return g_stQuestDir;
}

