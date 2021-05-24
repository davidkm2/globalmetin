#include "stdafx.h"
#include "desc.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "start_position.h"
#include "p2p.h"
#include "dungeon.h"
#include <string>
#include <sstream>
#include "localization.h"

using namespace std;

LocalizationManager::LocalizationManager()
{
}
LocalizationManager::~LocalizationManager()
{
}

#define myGlobalPos_x "localization_save.x_"
#define myGlobalPos_y "localization_save.y_"
#define myLocalPos_x "localization_local.x_"
#define myLocalPos_y "localization_local.y_"

bool LocalizationManager::Send_Localization_Open(LPCHARACTER ch, bool refresh)
{
	int register_callback = 0;
	
	if (refresh == true)
	{
		register_callback = 1;
	}
	
	string strLine_x1, strLine_x2, strLine_x3, strLine_x4, strLine_x5, strLine_x6, strLine_x7, strLine_y1, strLine_y2, strLine_y3, strLine_y4, strLine_y5, strLine_y6, strLine_y7;	
	ostringstream mex1, mex2, mex3, mex4, mex5, mex6, mex7, ret1, ret2, ret3, ret4, ret5, ret6, ret7;
	
	{mex1 << myLocalPos_x << 0; strLine_x1 = mex1.str();} {ret1 << myLocalPos_y << 0; strLine_y1 = ret1.str();}
	{mex2 << myLocalPos_x << 1; strLine_x2 = mex2.str();} {ret2 << myLocalPos_y << 1; strLine_y2 = ret2.str();}
	{mex3 << myLocalPos_x << 2; strLine_x3 = mex3.str();} {ret3 << myLocalPos_y << 2; strLine_y3 = ret3.str();}
	{mex4 << myLocalPos_x << 3; strLine_x4 = mex4.str();} {ret4 << myLocalPos_y << 3; strLine_y4 = ret4.str();}
	{mex5 << myLocalPos_x << 4; strLine_x5 = mex5.str();} {ret5 << myLocalPos_y << 4; strLine_y5 = ret5.str();}
	{mex6 << myLocalPos_x << 5; strLine_x6 = mex6.str();} {ret6 << myLocalPos_y << 5; strLine_y6 = ret6.str();}
	{mex7 << myLocalPos_x << 6; strLine_x7 = mex7.str();} {ret7 << myLocalPos_y << 6; strLine_y7 = ret7.str();}

	int registerPos[2][7] =
	{
		{(ch->GetQuestFlag(strLine_x1)), (ch->GetQuestFlag(strLine_x2)), (ch->GetQuestFlag(strLine_x3)), (ch->GetQuestFlag(strLine_x4)), (ch->GetQuestFlag(strLine_x5)), (ch->GetQuestFlag(strLine_x6)), (ch->GetQuestFlag(strLine_x7))},
		{(ch->GetQuestFlag(strLine_y1)), (ch->GetQuestFlag(strLine_y2)), (ch->GetQuestFlag(strLine_y3)), (ch->GetQuestFlag(strLine_y4)), (ch->GetQuestFlag(strLine_y5)), (ch->GetQuestFlag(strLine_y6)), (ch->GetQuestFlag(strLine_y7))}
	};

	char buf[512+1];
	snprintf(buf, sizeof(buf), "BINARY_Localization_Append %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", register_callback, registerPos[0][0], registerPos[1][0], registerPos[0][1], registerPos[1][1], registerPos[0][2], registerPos[1][2], registerPos[0][3], registerPos[1][3], registerPos[0][4], registerPos[1][4], registerPos[0][5], registerPos[1][5], registerPos[0][6], registerPos[1][6]);
	ch->ChatPacket(CHAT_TYPE_COMMAND, buf);

	return true;
}

bool LocalizationManager::Send_Localization_Teleport(LPCHARACTER ch, int tmpIndex)
{
	string strLine_x1, strLine_x2, strLine_x3, strLine_x4, strLine_x5, strLine_x6, strLine_x7, strLine_y1, strLine_y2, strLine_y3, strLine_y4, strLine_y5, strLine_y6, strLine_y7;	
	ostringstream mex1, mex2, mex3, mex4, mex5, mex6, mex7, ret1, ret2, ret3, ret4, ret5, ret6, ret7;

	{mex1 << myGlobalPos_x << 0; strLine_x1 = mex1.str();} {ret1 << myGlobalPos_y << 0; strLine_y1 = ret1.str();}
	{mex2 << myGlobalPos_x << 1; strLine_x2 = mex2.str();} {ret2 << myGlobalPos_y << 1; strLine_y2 = ret2.str();}
	{mex3 << myGlobalPos_x << 2; strLine_x3 = mex3.str();} {ret3 << myGlobalPos_y << 2; strLine_y3 = ret3.str();}
	{mex4 << myGlobalPos_x << 3; strLine_x4 = mex4.str();} {ret4 << myGlobalPos_y << 3; strLine_y4 = ret4.str();}
	{mex5 << myGlobalPos_x << 4; strLine_x5 = mex5.str();} {ret5 << myGlobalPos_y << 4; strLine_y5 = ret5.str();}
	{mex6 << myGlobalPos_x << 5; strLine_x6 = mex6.str();} {ret6 << myGlobalPos_y << 5; strLine_y6 = ret6.str();}
	{mex7 << myGlobalPos_x << 6; strLine_x7 = mex7.str();} {ret7 << myGlobalPos_y << 6; strLine_y7 = ret7.str();}

	int registerPos[2][7] =
	{
		{(ch->GetQuestFlag(strLine_x1)), (ch->GetQuestFlag(strLine_x2)), (ch->GetQuestFlag(strLine_x3)), (ch->GetQuestFlag(strLine_x4)), (ch->GetQuestFlag(strLine_x5)), (ch->GetQuestFlag(strLine_x6)), (ch->GetQuestFlag(strLine_x7))},
		{(ch->GetQuestFlag(strLine_y1)), (ch->GetQuestFlag(strLine_y2)), (ch->GetQuestFlag(strLine_y3)), (ch->GetQuestFlag(strLine_y4)), (ch->GetQuestFlag(strLine_y5)), (ch->GetQuestFlag(strLine_y6)), (ch->GetQuestFlag(strLine_y7))}
	};

	for (int savePos = 0; savePos <= 6; savePos++)
	{
		if (savePos == tmpIndex)
		{
			if (registerPos[0][tmpIndex] && registerPos[1][tmpIndex] > 0)
			{
				if (!ch->CanWarp())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("localization_blocked_lastTimeOpenShopOrOther"));
					return false; 
				}

				ch->WarpSet(registerPos[0][tmpIndex], registerPos[1][tmpIndex]);
			}
			else 
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("localization_not_exist_saved_positions"), registerPos[0][tmpIndex], registerPos[1][tmpIndex]);
				return false; 
			}
		}
	}

	return true;
}

bool LocalizationManager::Send_Localization_Save(LPCHARACTER ch, int tmpIndex)
{
	if (ch->IsObserverMode())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("localization_blocked_observerMode"));
		return false;
	}
	
	if (g_bChannel == 99 || ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("localization_blocked_ch99_and_dungeon"));
		return false; 
	}
	
	int arrayMapIndexBlocked[] = {
		66, 70, 71, 72, 73, 79, 80, 81, 83, 85, 112, 113, 189, 110, 111, 193, 195 /* Settings mapIndex what you want to blocked on save locations, you can put what u want. */
	};

	for (size_t idxMapIndex = 0; idxMapIndex < _countof(arrayMapIndexBlocked); idxMapIndex++) 
	{
		if (ch->GetMapIndex() == arrayMapIndexBlocked[idxMapIndex])
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("localization_blocked_maps"));
			return false;
		}
	}
	
	for (int tmp = 0; tmp <= 6; tmp++)
	{
		if (tmp == tmpIndex)
		{
			PIXEL_POSITION pos;
			pos = ch->GetXYZ();

			string strGlobalPos_x, strGlobalPos_y, strLocalPos_x, strLocalPos_y;
			ostringstream mex1, mex2, mex3, mex4;
				
			mex1 << myGlobalPos_x << tmpIndex;	strGlobalPos_x = mex1.str();
			mex2 << myGlobalPos_y << tmpIndex;	strGlobalPos_y = mex2.str();
			mex3 << myLocalPos_x << tmpIndex; strLocalPos_x = mex3.str();
			mex4 << myLocalPos_y << tmpIndex; strLocalPos_y = mex4.str();

			LPSECTREE_MAP strLocalizationMap = SECTREE_MANAGER::instance().GetMap(ch->GetMapIndex());

			ch->SetQuestFlag(strLocalPos_x, (ch->GetX() - strLocalizationMap->m_setting.iBaseX) / 100);
			ch->SetQuestFlag(strLocalPos_y, (ch->GetY() - strLocalizationMap->m_setting.iBaseY) / 100);

			ch->SetQuestFlag(strGlobalPos_x, pos.x);
			ch->SetQuestFlag(strGlobalPos_y, pos.y);
		}
	}

	return true;
}

