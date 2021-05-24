#include "stdafx.h"
#include "minigame_manager.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "sectree_manager.h"
#include "constants.h"
#include "questmanager.h"
#include "desc.h"
#include "item_manager.h"
#include "regen.h"

CMiniGameManager::CMiniGameManager()
{
	Destroy();
	Initialize();
}

CMiniGameManager::~CMiniGameManager()
{
	Destroy();
}

void CMiniGameManager::Initialize()
{
	memset(bEventState, 0, sizeof(bEventState));
}

void CMiniGameManager::Destroy()
{
	memset(bEventState, 0, sizeof(bEventState));
}

void CMiniGameManager::SpawnNPC(DWORD dwVnum, BYTE bType)
{
	CharacterVectorInteractor i;
	bool isNpcA1 = false;
	bool isNpcB1 = false;
	bool isNpcC1 = false;
	
	if (CHARACTER_MANAGER::instance().GetCharactersByRaceNum(dwVnum, i))
	{
		CharacterVectorInteractor::iterator it = i.begin();

		while (it != i.end())
		{
			LPCHARACTER tch = *(it++);

			if (tch->GetMapIndex() == 1)
				isNpcA1 = true;
			else if (tch->GetMapIndex() == 21)
				isNpcB1 = true;
			else if (tch->GetMapIndex() == 41)
				isNpcC1 = true;
		}
	}
	
	if (map_allow_find(1) && !isNpcA1)
		if (LPSECTREE_MAP pkMap = SECTREE_MANAGER::instance().GetMap(1))
			CHARACTER_MANAGER::instance().SpawnMob(dwVnum, 1, pkMap->m_setting.iBaseX + dwMiniGameNpcPos[bType][0] * 100, pkMap->m_setting.iBaseY + dwMiniGameNpcPos[bType][1] * 100, 0, false, 90, true);

	if (map_allow_find(21) && !isNpcB1)
		if (LPSECTREE_MAP pkMap = SECTREE_MANAGER::instance().GetMap(21))
			CHARACTER_MANAGER::instance().SpawnMob(dwVnum, 21, pkMap->m_setting.iBaseX + dwMiniGameNpcPos[bType][2] * 100, pkMap->m_setting.iBaseY + dwMiniGameNpcPos[bType][3] * 100, 0, false, 110, true);

	if (map_allow_find(41) && !isNpcC1)
		if (LPSECTREE_MAP pkMap = SECTREE_MANAGER::instance().GetMap(41))
			CHARACTER_MANAGER::instance().SpawnMob(dwVnum, 41, pkMap->m_setting.iBaseX + dwMiniGameNpcPos[bType][4] * 100, pkMap->m_setting.iBaseY + dwMiniGameNpcPos[bType][5] * 100, 0, false, 140, true);
}

void CMiniGameManager::SetEvent(bool bIsEnable, BYTE bType)
{
	bEventState[bType] = bIsEnable;
#ifdef __MINI_GAME_FISH__
	if (bType == MINIGAME_FISH)
		return;
#endif
	if (bIsEnable)
	{
		SpawnNPC(dwMiniGameNpcVnums[bType], bType);
		SpawnRegen(bIsEnable, bType);
	}
	else
	{
		CharacterVectorInteractor i;
		CHARACTER_MANAGER::instance().GetCharactersByRaceNum(dwMiniGameNpcVnums[bType], i);

		for (CharacterVectorInteractor::iterator it = i.begin(); it != i.end(); it++)
			M2_DESTROY_CHARACTER(*it);
	}
}

void CMiniGameManager::SpawnRegen(bool bIsEnable, BYTE bType)
{
	if (bIsEnable)
	{
		switch(bType)
		{
#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
			case MINIGAME_ATTENDANCE:
			{
				const char * c_apszRegenNames[4] = {
					"data/event/attendance/nemere_regen.txt",
					"data/event/attendance/razador_regen.txt",
					"data/event/attendance/red_dragon_regen.txt",
					"data/event/attendance/beran_setaou_regen.txt"
				};
				
				for (size_t i = 0; i < sizeof(c_apszRegenNames) / sizeof(c_apszRegenNames[0]); i++)
				{
					if (LPSECTREE_MAP pkMap = SECTREE_MANAGER::instance().GetMap(61 + i))
					{
						char szFileName[256 + 1];
						snprintf(szFileName, sizeof(szFileName), "%s", c_apszRegenNames[i]);
						regen_load(szFileName, 61 + i, pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					}
				}
			}
			break;

			case MINIGAME_MONSTERBACK:
			{
				DWORD stoneMaps[19] = {
					1, 3, 21, 23, 41, 43, 61, 62, 63, 64, 65, 67, 68, 69, 70, 
					218, 219, 220, 301 // indexurile mapelor beta 
				};
				
				char szFileName10[256 + 1];
				char szFileName20[256 + 1];
				char szFileName30[256 + 1];
				char szFileName40[256 + 1];
				char szFileName50[256 + 1];
				char szFileName60[256 + 1];
				char szFileName70[256 + 1];
				char szFileName80[256 + 1];
				char szFileName90[256 + 1];
				char szFileName95[256 + 1];

				snprintf(szFileName10, sizeof(szFileName10), "data/event/attendance/easter/stone_level_10.txt");
				snprintf(szFileName20, sizeof(szFileName20), "data/event/attendance/easter/stone_level_20.txt");
				snprintf(szFileName30, sizeof(szFileName30), "data/event/attendance/easter/stone_level_30.txt");
				snprintf(szFileName40, sizeof(szFileName40), "data/event/attendance/easter/stone_level_40.txt");
				snprintf(szFileName50, sizeof(szFileName50), "data/event/attendance/easter/stone_level_50.txt");
				snprintf(szFileName60, sizeof(szFileName60), "data/event/attendance/easter/stone_level_60.txt");
				snprintf(szFileName70, sizeof(szFileName70), "data/event/attendance/easter/stone_level_70.txt");
				snprintf(szFileName80, sizeof(szFileName80), "data/event/attendance/easter/stone_level_80.txt");
				snprintf(szFileName90, sizeof(szFileName90), "data/event/attendance/easter/stone_level_90.txt");
				snprintf(szFileName95, sizeof(szFileName95), "data/event/attendance/easter/stone_level_95.txt");
				
				for (size_t i = 0; i < sizeof(stoneMaps) / sizeof(stoneMaps[0]); i++)
				{
					if (map_allow_find(stoneMaps[i]))
					{
						if (LPSECTREE_MAP pkMap = SECTREE_MANAGER::instance().GetMap(stoneMaps[i]))
						{
							if(stoneMaps[i] == 1 || stoneMaps[i] == 21 || stoneMaps[i] == 41)
							{
								regen_load(szFileName10, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
								regen_load(szFileName20, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
							}
							else if(stoneMaps[i] == 3 || stoneMaps[i] == 23 || stoneMaps[i] == 43)
							{
								regen_load(szFileName20, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
								regen_load(szFileName30, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
							}
							else if(stoneMaps[i] == 64)
							{
								regen_load(szFileName30, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
								regen_load(szFileName40, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
								regen_load(szFileName50, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
							}
							else if(stoneMaps[i] == 63 || stoneMaps[i] == 65)
							{
								regen_load(szFileName40, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
								regen_load(szFileName50, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
							}
							else if(stoneMaps[i] == 61)
							{
								regen_load(szFileName50, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
								regen_load(szFileName60, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
							}
							else if(stoneMaps[i] == 67 || stoneMaps[i] == 62 || stoneMaps[i] == 69)
							{
								regen_load(szFileName60, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
								regen_load(szFileName70, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
							}
							else if(stoneMaps[i] == 68 || stoneMaps[i] == 70)
							{
								regen_load(szFileName70, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
								regen_load(szFileName80, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
							}
							// Acest else if de la sfarsit e pentru mapele beta
							else if(stoneMaps[i] == 218 || stoneMaps[i] == 219 || stoneMaps[i] == 220 || stoneMaps[i] == 301)
							{
								regen_load(szFileName90, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
								regen_load(szFileName95, stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
							}
						}
					}
				}
			}
			break;
#endif
			default:
				break;
		}
	}
}

void CMiniGameManager::SetCharecterEvents(LPCHARACTER ch)
{
	for (int i = 0; i < MINIGAME_TYPE_MAX; ++i)
		if (bEventState[i])
			ch->ChatPacket(CHAT_TYPE_COMMAND, "SetMiniGameEvent %d %d", i, bEventState[i]);
}

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
bool CMiniGameManager::ReadRewardItemFile(const char * c_pszFileName)
{
	FILE *fp = fopen(c_pszFileName, "r");
	if (!fp)
		return false;

	int iDay, iVnum, iCount;
	
	while (fscanf(fp, "%u	%u	%u", &iDay, &iVnum, &iCount) != EOF)
	{
		if (!ITEM_MANAGER::instance().GetTable(iVnum))
		{
			sys_err("No such item (Vnum: %lu)", iVnum);
			return false;
		}
		
		TRewardItem tempItem;
		tempItem.bDay = iDay;
		tempItem.dwVnum = iVnum;
		tempItem.dwCount = iCount;

		attendanceRewardVec.push_back(tempItem);
	}
	
	fclose(fp);
	return true;
}

void CMiniGameManager::RequestRewardList(LPCHARACTER pChar)
{
	if (!pChar)
		return;
	
	if (!pChar->GetDesc())
		return;
	
	TPacketGCAttendanceRequestRewardList packRequestRewardList;
	packRequestRewardList.bHeader = HEADER_GC_ATTENDANCE_REQUEST_REWARD_LIST;
	packRequestRewardList.wSize = sizeof(packRequestRewardList) + (sizeof(TRewardItem) * attendanceRewardVec.size());

	pChar->GetDesc()->BufferedPacket(&packRequestRewardList, sizeof(packRequestRewardList));
	pChar->GetDesc()->Packet(&attendanceRewardVec[0], sizeof(TRewardItem) * attendanceRewardVec.size());
}

void CMiniGameManager::AttendanceButtonClick(LPCHARACTER pChar, const char * data)
{
	if (!pChar)
		return;
	
	if (!pChar->GetDesc())
		return;

	// TPacketCGAttendanceButtonClick* Packet = (TPacketCGAttendanceButtonClick *) data;

	time_t iTime;
	time(&iTime);
	tm* pTimeInfo = localtime(&iTime);

	char szClearName[32 + 1], szRewardName[32 + 1];
	snprintf(szClearName, sizeof(szClearName), "attendance.clear_day_%d", pTimeInfo->tm_yday);
	snprintf(szRewardName, sizeof(szRewardName), "attendance.reward_day_%d", pTimeInfo->tm_yday);
	
	if (!quest::CQuestManager::instance().GetEventFlag("mini_game_attendance"))
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is now enabled."));
		return;
	}
	
	if (!pChar->GetQuestFlag(szClearName))
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You need to earn this reward before you get it."));
		return;
	}
	
	if (pChar->GetQuestFlag(szRewardName))
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You already get your reward for today."));
		return;
	}
	
	if (pChar->GetLevel() < 30)
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You need at least level 30 for this event."));
		return;
	}

	DWORD rewardVnum = 0, rewardCount = 0;
	if (!attendanceRewardVec.empty())
	{
		for (DWORD i = 0; i < attendanceRewardVec.size(); i++)
		{
			if (attendanceRewardVec[i].bDay == (pTimeInfo->tm_yday - quest::CQuestManager::instance().GetEventFlag("attendance_start_day")) + 1)
			{
				rewardVnum = attendanceRewardVec[i].dwVnum;
				rewardCount = attendanceRewardVec[i].dwCount;
				break;
			}
		}
	}
	
	pChar->SetQuestFlag(szRewardName, 1);
	
	if (rewardVnum && rewardCount)
	{
		pChar->AutoGiveItem(rewardVnum, rewardCount);
		
		TPacketGCAttendanceRequestData pack;
		pack.bHeader = HEADER_GC_ATTENDANCE_REQUEST_DATA;
		pack.bType = ATTENDANCE_DATA_TYPE_GET_REWARD;
		pack.bValue = 1;
		pChar->GetDesc()->Packet(&pack, sizeof(TPacketGCAttendanceRequestData));
	}
}

void CMiniGameManager::RequestData(LPCHARACTER pChar, const char * data)
{
	if (!pChar)
		return;

	if (!pChar->GetDesc())
		return;

	TPacketCGAttendanceRequestData* packet = (TPacketCGAttendanceRequestData *) data;

	TPacketGCAttendanceRequestData pack;
	pack.bHeader = HEADER_GC_ATTENDANCE_REQUEST_DATA;
	pack.bType = packet->bType;
	time_t iTime;
	time(&iTime);
	tm* pTimeInfo = localtime(&iTime);
	switch(packet->bType)
	{
		case ATTENDANCE_DATA_TYPE_DAY:
			pack.bValue = pTimeInfo->tm_yday - quest::CQuestManager::instance().GetEventFlag("attendance_start_day");
		break;

		case ATTENDANCE_DATA_TYPE_MISSION_CLEAR:
		{
			char szClearName[32 + 1];
			snprintf(szClearName, sizeof(szClearName), "attendance.clear_day_%d", pTimeInfo->tm_yday);
			pack.bValue = pChar->GetQuestFlag(szClearName);
		}
		break;

		case ATTENDANCE_DATA_TYPE_GET_REWARD:
		{
			char szRewardName[32 + 1];
			snprintf(szRewardName, sizeof(szRewardName), "attendance.reward_day_%d", pTimeInfo->tm_yday);
			pack.bValue = pChar->GetQuestFlag(szRewardName);
		}
		break;

		case ATTENDANCE_DATA_TYPE_SHOW_MAX:
			pack.bValue = quest::CQuestManager::instance().GetEventFlag("mini_game_attendance");
			break;

		default:
			pack.bValue = 1;
	}

	pChar->GetDesc()->Packet(&pack, sizeof(TPacketGCAttendanceRequestData));
}
#endif

