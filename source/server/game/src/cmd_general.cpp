#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "motion.h"
#include "packet.h"
#include "affect.h"
#include "pvp.h"
#include "start_position.h"
#include "party.h"
#include "guild_manager.h"
#include "p2p.h"
#include "dungeon.h"
#include "messenger_manager.h"
#include "war_map.h"
#include "questmanager.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "item.h"
#include "arena.h"
#include "buffer_manager.h"
#include "unique_item.h"
#include "log.h"
#include "shop.h"
#include "../../common/VnumHelper.h"
#include "mountsystem.h"
#include "localization.h"
#include "new_petsystem.h"
#include "supportsystem.h"
#include "battle_field.h"
#include "ranking_manager.h"
#include "safebox.h"
#include "shop_manager.h"
#include "offline_privateshop.h"
#include "damage_top.h"


#ifdef __OFFLINE_PRIVATE_SHOP__
ACMD(do_click_offline_private_shop)
{
	BYTE bCount = CShopManager::instance().GetOfflinePrivateShopCount(ch->GetPlayerID());

	if (!bCount)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
	else
	{
		TEMP_BUFFER buf;
		LPOPSHOP pkOfflinePrivateShops[255]; // FIXME: expression did not evaluate to a constant (bCount)
		CShopManager::instance().SetOfflinePrivateShops(pkOfflinePrivateShops, bCount, ch->GetPlayerID());
		TOfflinePrivateShopInfo pkTOfflinePrivateShopInfo[255]{ 0 }; // FIXME: expression did not evaluate to a constant (bCount)

		for (BYTE i = 0; i < bCount; ++i)
		{
			pkOfflinePrivateShops[i]->GetInfo(&pkTOfflinePrivateShopInfo[i]);
			buf.write(&pkTOfflinePrivateShopInfo[i], sizeof(TOfflinePrivateShopInfo));
		}

		TPacketGCOfflinePrivateShopInfo packet;
		packet.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
		packet.bSubHeader = OFFLINE_PRIVATE_SHOP_INFO;
		packet.wSize = sizeof(packet) + buf.size();

		ch->GetDesc()->BufferedPacket(&packet, sizeof(packet));
		ch->GetDesc()->Packet(buf.read_peek(), buf.size());
	}
}
#endif

#ifdef __DAMAGE_TOP__
ACMD(do_click_damage_top)
{
	CDamageTop::instance().OpenDamageTop(ch);
}
#endif

#ifdef __BATTLE_FIELD__
ACMD(do_open_battle_ui)
{
	if (!CBattleField::instance().IsBattleFieldMapIndex(ch->GetMapIndex()))
	{
		TPacketRanking packet;
		packet.bHeader = HEADER_GC_RANKING;
		TEMP_BUFFER buf;

		TInfoMap Map = CRankingManager::instance().GetRankingInfo();
		for (int i = 0; i < PARTY_RK_CATEGORY_MAX + SOLO_RK_CATEGORY_MAX; ++i)
			for (TRankingInfoMap::iterator it = Map[i].begin(); it != Map[i].end(); ++it)
				buf.write(&it->second, sizeof(it->second));

		packet.wSize = sizeof(packet) + buf.size();

		ch->GetDesc()->BufferedPacket(&packet, sizeof(packet));
		ch->GetDesc()->Packet(buf.read_peek(), buf.size());

		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenRankingBoard 0 0");
	}
	else
	{
		if (ch->IsDead())
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "ExitBattleFieldOnDead %d", ch->GetPoint(POINT_TEMP_BATTLE_POINT));
			
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "ExitBattleField %d", ch->GetPoint(POINT_TEMP_BATTLE_POINT));

		}
	}
}
ACMD(do_goto_battle)
{
	CBattleField::instance().RequestEnter(ch);
}

ACMD(do_exit_battle_field)
{
	CBattleField::instance().RequestExit(ch);
}

ACMD(do_exit_battle_field_on_dead)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	BOOL bIsAccept = 0;
	str_to_number(bIsAccept, arg1);

	if (bIsAccept)
		CBattleField::instance().RequestExit(ch);
	else
		return;
}
#endif

#ifdef __12ZI__
ACMD(do_cz_reward)
{
	char arg1[12];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	int type;
	str_to_number(type, arg1);

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT * FROM player.zodiac_reward WHERE name = '%s'", ch->GetName()));

	if (pMsg->Get()->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		switch (type)
		{
			case 1:
			{
				DBManager::instance().DirectQuery("UPDATE player.zodiac_reward SET yellowreward = (yellowreward + 1), yellowmark = 0 WHERE name = '%s'", ch->GetName());
				ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward %d %d %d %d %d", 0, atoi(row[2]), atoi(row[3]) + 1, atoi(row[4]), atoi(row[5]));
				ch->AutoGiveItem(33026);
			}
			break;
			case 2:
			{
				DBManager::instance().DirectQuery("UPDATE player.zodiac_reward SET greenreward = (greenreward + 1), greenmark = 0 WHERE name = '%s'", ch->GetName());
				ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward %d %d %d %d %d", atoi(row[1]), 0, atoi(row[3]), atoi(row[4]) + 1, atoi(row[5]));
				ch->AutoGiveItem(33027);
			}
			break;
			case 3:
			{
				DBManager::instance().DirectQuery("UPDATE player.zodiac_reward SET yellowreward = (yellowreward - 1), greenreward = (greenreward - 1) WHERE name = '%s'", ch->GetName());
				ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward %d %d %d %d %d", atoi(row[1]), atoi(row[2]), atoi(row[3]) - 1, atoi(row[4]) - 1, atoi(row[5]));
				ch->AutoGiveItem(33028);
			}
			break;
			default:
				return;
		}
	}
}

ACMD(do_cz_check_box)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	int type = 0;
	int value = 0;
	str_to_number(type, arg1);
	str_to_number(value, arg2);

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT * FROM player.zodiac_reward WHERE name = '%s'", ch->GetName()));

	if (pMsg->Get()->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		int yellowmark = 0;
		int greenmark = 0;
		str_to_number(yellowmark, row[1]);
		str_to_number(greenmark, row[2]);

		if (ch->CountSpecifyItem(iCzTable[type][value][0]) >= 50 && ch->CountSpecifyItem(iCzTable[type][value][1]) >= 50)
		{
			if (type == 0)
			{
				if (!IS_SET(yellowmark, 1 << value))
				{
					SET_BIT(yellowmark, 1 << value);
					ch->RemoveSpecifyItem(iCzTable[type][value][0], 50);
					ch->RemoveSpecifyItem(iCzTable[type][value][1], 50);
					DBManager::instance().DirectQuery("UPDATE player.zodiac_reward SET yellowmark = '%d' WHERE name = '%s'", yellowmark, ch->GetName());
					ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward %d %d %d %d %d", yellowmark, atoi(row[2]), atoi(row[3]), atoi(row[4]), atoi(row[5]));
				}
				else
					return;
			}
			else if (type == 1)
			{
				if (!IS_SET(greenmark, 1 << value))
				{
					SET_BIT(greenmark, 1 << value);
					ch->RemoveSpecifyItem(iCzTable[type][value][0], 50);
					ch->RemoveSpecifyItem(iCzTable[type][value][1], 50);
					DBManager::instance().DirectQuery("UPDATE player.zodiac_reward SET greenmark = '%d' WHERE name = '%s'", greenmark, ch->GetName());
					ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward %d %d %d %d %d", atoi(row[1]), greenmark, atoi(row[3]), atoi(row[4]), atoi(row[5]));
				}
				else
					return;
			}
		}
	}
}

ACMD(do_revive)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid;
	str_to_number(vid, arg1);

	LPCHARACTER tch = NULL;
	if (vid == ch->GetVID())
		tch = ch;
	else
		tch = CHARACTER_MANAGER::instance().Find(vid);

	if (!tch)
		return;

	int ZODIAC_REVIVE_ITEM = ch->CountSpecifyItem(33032);
	int REVIVE_PRICE = ch->GetQuestFlag("zodiac.PrismOfRevival") * 2;
	if (ZODIAC_REVIVE_ITEM < REVIVE_PRICE)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "You dont have enough Prism of Revival. You need %d.", REVIVE_PRICE);
		return;
	}

	tch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

	tch->GetDesc()->SetPhase(PHASE_GAME);
	tch->SetPosition(POS_STANDING);
	tch->StartRecoveryEvent();

	sys_log(0, "do_restart: restart here");
	tch->RestartAtSamePos();

	tch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
	tch->DeathPenalty(0);
	tch->ReviveInvisible(5);
	tch->CheckWears();
	if (vid != ch->GetVID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "{{212, %s}}", tch->GetName());
		tch->ChatPacket(CHAT_TYPE_INFO, "{{215}}");
		tch->ChatPacket(CHAT_TYPE_INFO, "{{213, %s}}", tch->GetName());
	}

	if (ch->GetQuestFlag("zodiac.PrismOfRevival") == 0)
	{
		ch->RemoveSpecifyItem(33032, 1);
		ch->SetQuestFlag("zodiac.PrismOfRevival", 1);
	}
	else
	{
		ch->RemoveSpecifyItem(33032, REVIVE_PRICE);
		ch->SetQuestFlag("zodiac.PrismOfRevival", REVIVE_PRICE);
	}
}

ACMD(do_revivedialog)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid;
	str_to_number(vid, arg1);

	int REVIVE_PRICE = ch->GetQuestFlag("zodiac.PrismOfRevival") * 2;
	ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenReviveDialog %d %d", vid, REVIVE_PRICE);
}
#endif

#ifdef __ANCIENT_ATTRIBUTE__
ACMD(do_add_new_attribute)
{
	std::vector<std::string> vecArgs;
	split_argument(argument, vecArgs);
	
	if (vecArgs.size() < 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_UNKNOWN_ARGUMENT"));
		return;
	}
	
	int sourceItemPos = 0;
	int targetItemPos = 0;
	int attrtype0 = 0;
	int attrtype1 = 0;
	int attrtype2 = 0;
	int attrtype3 = 0;
		
	str_to_number(sourceItemPos, vecArgs[1].c_str());
	str_to_number(targetItemPos, vecArgs[2].c_str());
	str_to_number(attrtype0, vecArgs[3].c_str());
	str_to_number(attrtype1, vecArgs[4].c_str());
	str_to_number(attrtype2, vecArgs[5].c_str());
	str_to_number(attrtype3, vecArgs[6].c_str());
	
	LPITEM item = ch->GetInventoryItem(targetItemPos);
	
	if (!item)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_CANNOT_FIND_ITEM"));
		return;
	}
	
	if (item->GetType() != ITEM_WEAPON && item->GetType() != ITEM_ARMOR)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_COSTUME"));
		return;
	}
	
	if (item->IsEquipped())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_EQUIPPED"));
		return;
	}
	
	if (item->IsExchanging())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_EXCHANGING"));
		return;
	}

	if (ch->CountSpecifyItem(71051) < 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_ENOUGH_NEW_ATTRIBUTE"));
		return;
	}
	
	TItemTable const * pProto = item->GetProto();
	
	bool addontype = false;
	if (pProto && pProto->sAddonType)
		addontype = true;
	
	int attrtype0prob = number(1,5), attrtype1prob = number(1,5), attrtype2prob = number(1,5), attrtype3prob = number(1,5);
	// int attrtype1prob = number(1,5);
	// int attrtype2prob = number(1,5);
	// int attrtype3prob = number(1,5);
	
	bool error = false;
	if (addontype == false)
	{
		if (attrtype0 == attrtype1 || attrtype0 == attrtype2 || attrtype0 == attrtype3 || attrtype0 == item->GetAttributeType(4))
			error = true;
		if (attrtype1 == attrtype0 || attrtype1 == attrtype2 || attrtype1 == attrtype3 || attrtype1 == item->GetAttributeType(4))
			error = true;
		if (attrtype2 == attrtype0 || attrtype2 == attrtype0 || attrtype2 == attrtype3 || attrtype2 == item->GetAttributeType(4))
			error = true;
		if (attrtype3 == attrtype0 || attrtype3 == attrtype1 || attrtype3 == attrtype2 || attrtype3 == item->GetAttributeType(4))
			error = true;
		if (item->GetAttributeType(4) == attrtype0 || item->GetAttributeType(4) == attrtype1 || item->GetAttributeType(4) == attrtype2 || item->GetAttributeType(4) == attrtype3)
			error = true;
	}
	else
	{
		if (attrtype0 == attrtype1 || attrtype0 == item->GetAttributeType(4))
			error = true;
		if (attrtype1 == attrtype0 || attrtype1 == item->GetAttributeType(4))
			error = true;
	}

	for (int i=0; i < 4; ++i)
	{
		if (addontype == false)
		{
			if (item->GetAttributeType(i) == attrtype0 && i != 0)
				error = true;
			if (item->GetAttributeType(i) == attrtype1 && i != 1)
				error = true;
			if (item->GetAttributeType(i) == attrtype2 && i != 2)
				error = true;
			if (item->GetAttributeType(i) == attrtype3 && i != 3)
				error = true;
		}
		else
		{
			if (item->GetAttributeType(i+2) == attrtype0 && i != 0)
				error = true;
			if (item->GetAttributeType(i+2) == attrtype1 && i != 1)
				error = true;			
		}
	}
	if (error)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_ERROR"));
		return;
	}
	
	if (addontype == false)
	{
		item->AddNewAttr(0, attrtype0, attrtype0prob);
		item->AddNewAttr(1, attrtype1, attrtype1prob);
		item->AddNewAttr(2, attrtype2, attrtype2prob);
		item->AddNewAttr(3, attrtype3, attrtype3prob);
	}
	else
	{	
		int oldattr5type = item->GetAttributeType(4), oldattr5value = item->GetAttributeValue(4);
		// int oldattr5value = item->GetAttributeValue(4);

		item->RemoveAttributeAt(0);
		item->RemoveAttributeAt(1);	
		item->ApplyAddon(100);
		item->ChangeAttribute();
		item->AddNewAttr(2, attrtype0, attrtype0prob);
		item->AddNewAttr(3, attrtype1, attrtype1prob);	
		item->AddNewAttr(4, oldattr5type, oldattr5value);

		if (item->GetAttributeType(1) != 71)
		{
			if (item->GetAttributeType(0) != 72)
			{
				item->SetForceAttribute(0, 72, 13);
				item->SetForceAttribute(1, 71, 1);
			}
			else
				item->SetForceAttribute(1, 71, -1);
		}				
	}
	
	ch->RemoveSpecifyItem(71051, 1);
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_SUCCESS_ATTR"));
}
#endif

#ifdef __REMOTE_EXCHANGE__
ACMD(do_remote_exchange)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(arg1);
		if (pkCCI)
			ch->RemoteExchangeStart(pkCCI);
		else
			return;
	}
	else
		ch->ExchangeStart(tch);
}
#endif

ACMD(do_hp)
{
	if (!ch || !ch->GetDungeon() || !ch->GetDungeon()->GetMast())
		return;

	LPCHARACTER mast = ch->GetDungeon()->GetMast();
	if (mast)
		ch->ChatPacket(0, "Direk: %d", mast->GetHP());
}

#ifdef __SUPPORT__
ACMD(do_support_system)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;
	
	DWORD items = 0;
	str_to_number(items, arg1);
	if (ch->GetSupportSystem()->IsActiveSupport())
	{
		for (int i = 0; i < ch->GetInventoryMax(); i++)
		{
			LPITEM item = ch->GetInventoryItem(i);
			LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(ch->GetSupportSystem()->GetActiveSupport()->GetSummonItemVID());
			if (item != NULL)
				if (item->GetVnum() == items)
				{
					if (item->GetType() == ITEM_COSTUME || item->GetType() == ITEM_ARMOR)
					{
						
						if (item->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> sadece ?aman zırhı giydirebilirsin..");
							return;
						}
						
						if (item->IsExchanging())
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> ticaret yaparken bu i?lemi yapamazsın");
							return;
						}
						if(item->GetSubType() == COSTUME_BODY)
						{
							item->ClearAttribute();
							pSummonItem->SetSocket(1,item->GetVnum());
							ch->ChatPacket(CHAT_TYPE_COMMAND, "SupportArmor %u", item->GetVnum());
							DBManager::instance().DirectQuery("UPDATE player.new_support SET armor = '%u' WHERE id = '%d'", item->GetVnum(), pSummonItem->GetID());
							ch->UpdatePacket();
							item->RemoveFromCharacter();
							break;
						}
						else
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi yapamazsın");
							return;
						}	
					}
					
				}
		}
		ch->GetSupportSystem()->GetActiveSupport()->RefreshCostume();
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi kullanabilmek icin yardımcı ?aman muhuru gerekiyor.");
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> e?er ?aman muhuru varsa lutfen ?amanı ca?ırın.");
	}
}
ACMD(do_support_system_w)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	DWORD items = 0;
	str_to_number(items, arg1);
	if (ch->GetSupportSystem()->IsActiveSupport())
	{
		for (int i = 0; i < ch->GetInventoryMax(); i++)
		{
			LPITEM item = ch->GetInventoryItem(i);
			LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(ch->GetSupportSystem()->GetActiveSupport()->GetSummonItemVID());
			if (item != NULL)
			if (item->GetVnum() == items)
			{
				if (item->GetType() == ITEM_WEAPON)
				{
					if (item->IsExchanging())
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> ticaret yaparken bu i?lemi yapamazsın");
						return;
					}
					if (item->GetSubType() == WEAPON_FAN || item->GetSubType() == WEAPON_BELL)
					{
						item->ClearAttribute();
						pSummonItem->SetForceAttribute(3, 3, item->GetVnum());
						ch->ChatPacket(CHAT_TYPE_COMMAND, "SupportWeapon %u", item->GetVnum());
						DBManager::instance().DirectQuery("UPDATE player.new_support SET weapon = '%u' WHERE id = '%d'", item->GetVnum(), pSummonItem->GetID());
						ch->UpdatePacket();
						item->RemoveFromCharacter();
						break;
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi yapamazsın");
						return;
					}
				}

			}
		}
		ch->GetSupportSystem()->GetActiveSupport()->RefreshCostume();
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi kullanabilmek icin yardımcı ?aman muhuru gerekiyor.");
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> e?er ?aman muhuru varsa lutfen ?amanı ca?ırın.");
	}
}
ACMD(do_support_system_d)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;
	
	DWORD items = 0;
	str_to_number(items, arg1);
	if (ch->GetSupportSystem()->IsActiveSupport())
	{
		for (int i = 0; i < ch->GetInventoryMax(); i++)
		{
			LPITEM item = ch->GetInventoryItem(i);
			LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(ch->GetSupportSystem()->GetActiveSupport()->GetSummonItemVID());
			if (item != NULL)
				if (item->GetVnum() == items)
				{
					if (item->GetType() == ITEM_COSTUME)
					{
						if (item->IsExchanging())
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> ticaret yaparken bu i?lemi yapamazsın");
							return;
						}
						if (item->GetSubType() == COSTUME_HAIR)
						{
							item->ClearAttribute();
							pSummonItem->SetSocket(2,item->GetValue(3));
							ch->ChatPacket(CHAT_TYPE_COMMAND, "SupportHair %u", item->GetVnum());
							DBManager::instance().DirectQuery("UPDATE player.new_support SET hair = '%u', hairvnum = '%u' WHERE id = '%d'", item->GetValue(3), item->GetVnum(), pSummonItem->GetID());
							ch->UpdatePacket();
							item->RemoveFromCharacter();
							break;
						}
						else
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi yapamazsın");
							return;
						}
					}
					
				}
		}
		ch->GetSupportSystem()->GetActiveSupport()->RefreshCostume();
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi kullanabilmek icin yardımcı ?aman muhuru gerekiyor.");
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> e?er ?aman muhuru varsa lutfen ?amanı ca?ırın.");
	}
}
#endif

#ifdef __SORT_INVENTORY__
ACMD (do_sort_inventory_items)
{
    if (ch->IsDead() || ch->GetExchange() || ch->IsOpenSafebox() || ch->IsCubeOpen())
    {
        ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't sort your inventory with those windows open."));
        return;
    }
   
    int lastSortInventoryPulse = ch->GetSortInventoryPulse();
    int currentPulse = thecore_pulse();
   
    if (lastSortInventoryPulse > currentPulse)
        return;
   
	for (int i = 0; i < ch->GetInventoryMax(); ++i)
	{
		LPITEM item = ch->GetInventoryItem(i);

		if (!item)
			continue;

		if (item->isLocked())
			continue;

		// if (item->GetCount() == g_bItemCountLimit)
			// continue;

		int pos = ch->GetEmptyInventory(item->GetSize());

		if (pos >= 0 && pos < item->GetCell())
		{
			item->RemoveFromCharacter();
			item->AddToCharacter(ch, TItemPos(INVENTORY, pos));
		}
       
        if (item->IsStackable() && item->GetCount() < g_bItemCountLimit)
        {
            for (int j = i; j < ch->GetInventoryMax(); ++j)
            {
				LPITEM item2 = ch->GetInventoryItem(j);
               
                if (!item2)
                    continue;
               
                if (item2->isLocked())
                    continue;
   
                if (item2->GetVnum() == item->GetVnum())
                {
                    bool bStopSockets = false;
                   
                    for (int k = 0; k < ITEM_SOCKET_MAX_NUM; ++k)
                    {
                        if (item2->GetSocket(k) != item->GetSocket(k))
                        {
                            bStopSockets = true;
                            break;
                        }
                    }
                   
                    if (bStopSockets)
                        continue;
   
                    BYTE bAddCount = MIN(g_bItemCountLimit - item->GetCount(), item2->GetCount());
   
                    item->SetCount(item->GetCount() + bAddCount);
                    item2->SetCount(item2->GetCount() - bAddCount);
                   
                    continue;
                }
            }
        }
    }
   
    ch->SetNextSortInventoryPulse(thecore_pulse() + PASSES_PER_SEC(60));
}
ACMD (do_sort_safebox_items)
{
    if (ch->IsDead() || ch->GetExchange() || ch->IsCubeOpen())
    {
        ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't sort your inventory with those windows open."));
        return;
    }
   
    // int lastSortInventoryPulse = ch->GetSortInventoryPulse();
    // int currentPulse = thecore_pulse();
   
    // if (lastSortInventoryPulse > currentPulse)
        // return;

	CSafebox * pkSafebox;

	pkSafebox = ch->GetSafebox();

	if (!pkSafebox)
		return;

	for (int i = 0; i < SAFEBOX_MAX_NUM; ++i)
	{
		LPITEM item = pkSafebox->Get(i);

		if (!item)
			continue;

		if (item->isLocked())
			continue;

		// if (item->GetCount() == g_bItemCountLimit)
			// continue;

		int pos = pkSafebox->GetEmptySafeBox(item->GetSize());

		if (pos >= 0 && pos < item->GetCell())
			pkSafebox->MoveItem(item->GetCell(), pos, item->GetCount());

		if (item->IsStackable() && item->GetCount() < g_bItemCountLimit)
		{
			for (int j = i; j < SAFEBOX_MAX_NUM; ++j)
			{
				LPITEM item2 = pkSafebox->Get(j);

				if (!item2)
					continue;

				if (item2->isLocked())
					continue;
   
				if (item2->GetVnum() == item->GetVnum())
				{
					bool bStopSockets = false;
                   
					for (int k = 0; k < ITEM_SOCKET_MAX_NUM; ++k)
					{
						if (item2->GetSocket(k) != item->GetSocket(k))
						{
							bStopSockets = true;
							break;
						}
					}
                   
					if (bStopSockets)
						continue;
   
					// BYTE bAddCount = MIN(g_bItemCountLimit - item->GetCount(), item2->GetCount());

					pkSafebox->MoveItem(item2->GetCell(), item->GetCell(), item2->GetCount());
					pkSafebox->Remove(item->GetCell());
					pkSafebox->Add(item->GetCell(), item);

					// if (item && item2->GetCount() < bAddCount)
						// pkSafebox->Add(item2->GetCell(), item2);

					continue;
				}
			}
		}
	}
   
    // ch->SetNextSortInventoryPulse(thecore_pulse() + PASSES_PER_SEC(60));
}
#ifdef __NEW_STORAGE__
ACMD (do_sort_special_storage_items)
{
    if (ch->IsDead() || ch->GetExchange() || ch->IsOpenSafebox() || ch->IsCubeOpen())
    {
        ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't sort your inventory with those windows open."));
        return;
    }
   
    int lastSortSpecialStoragePulse = ch->GetSortSpecialStoragePulse();
    int currentPulse = thecore_pulse();
   
    if (lastSortSpecialStoragePulse > currentPulse)
	{
        int deltaInSeconds = ((lastSortSpecialStoragePulse / PASSES_PER_SEC(1)) - (currentPulse / PASSES_PER_SEC(1)));
        int minutes = deltaInSeconds / 60;
        int seconds = (deltaInSeconds - (minutes * 60));
 
        ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can sort your inventory again in %02d seconds."), seconds);
        return;
    }
   
   
	for (int j = 0; j < STORAGE_INVENTORY_COUNT; ++j)
	{
        for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
        {
            LPITEM item = ch->GetStorageInventoryItem(j, i);
           
            if (!item)
                continue;
           
            if (item->isLocked())
                continue;

			int pos = ch->GetEmptyStorageInventory(j, item);

			if (pos >= 0 && pos < item->GetCell())
			{
				item->RemoveFromCharacter();
				item->AddToCharacter(ch, TItemPos(UPGRADE_INVENTORY + j, pos));
			}

            if (item->IsStackable() && item->GetCount() < g_bItemCountLimit)
            {
                for (int j = i; j < STORAGE_INVENTORY_MAX_NUM; ++j)
                {
                   
                    LPITEM item2 = ch->GetStorageInventoryItem(j, i);
           
                    if (!item2)
                        continue;
                   
                    if (item2->isLocked())
                        continue;
       
                    if (item2->GetVnum() == item->GetVnum())
                    {
                        bool bStopSockets = false;
                       
                        for (int k = 0; k < ITEM_SOCKET_MAX_NUM; ++k)
                        {
                            if (item2->GetSocket(k) != item->GetSocket(k))
                            {
                                bStopSockets = true;
                                break;
                            }
                        }
                       
                        if (bStopSockets)
                            continue;
       
                        BYTE bAddCount = MIN(g_bItemCountLimit - item->GetCount(), item2->GetCount());
       
                        item->SetCount(item->GetCount() + bAddCount);
                        item2->SetCount(item2->GetCount() - bAddCount);
                       
                        continue;
                    }
                }
            }
        }
    }
 
    ch->SetNextSortSpecialStoragePulse(thecore_pulse() + PASSES_PER_SEC(60));
}
#endif
#endif

#ifdef __ATTR_TRANSFER__
ACMD(do_attr_transfer)
{
	// if (!ch->CanDoAttrTransfer())
		// return;
	
	sys_log(1, "%s has used an Attr Transfer command: %s.", ch->GetName(), argument);
	
	int w_index = 0, i_index = 0;
	const char *line;
	char arg1[256], arg2[256], arg3[256];
	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));
	if (0 == arg1[0])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage:");
		ch->ChatPacket(CHAT_TYPE_INFO, "- attrtransfer open");
		ch->ChatPacket(CHAT_TYPE_INFO, "- attrtransfer close");
		ch->ChatPacket(CHAT_TYPE_INFO, "- attrtransfer add <attr_transfer_slot> <inventory_slot>");
		ch->ChatPacket(CHAT_TYPE_INFO, "- attrtransfer delete <attr_transfer_slot>");
		return;
	}
	
	const std::string& strArg1 = std::string(arg1);
	if (strArg1 == "open")
	{
		AttrTransfer_open(ch);
		return;
	}
	else if (strArg1 == "close")
	{
		AttrTransfer_close(ch);
		return;
	}
	else if (strArg1 == "make")
	{
		AttrTransfer_make(ch);
		return;
	}
	else if (strArg1 == "add")
	{
		if (0 == arg2[0] || !isdigit(*arg2) || 0 == arg3[0] || !isdigit(*arg3))
			return;
		
		str_to_number(w_index, arg2);
		str_to_number(i_index, arg3);
		AttrTransfer_add_item(ch, w_index, i_index);
		return;
	}
	else if (strArg1 == "delete")
	{
		if (0 == arg2[0] || !isdigit(*arg2))
			return;
		
		str_to_number(w_index, arg2);
		AttrTransfer_delete_item(ch, w_index);
		return;
	}
	
	switch (LOWER(arg1[0]))
	{
		case 'o':
			AttrTransfer_open(ch);
			break;
		case 'c':
			AttrTransfer_close(ch);
			break;
		case 'm':
			AttrTransfer_make(ch);
			break;
		case 'a':
			{
				if (0 == arg2[0] || !isdigit(*arg2) || 0 == arg3[0] || !isdigit(*arg3))
					return;
				
				str_to_number(w_index, arg2);
				str_to_number(i_index, arg3);
				AttrTransfer_add_item(ch, w_index, i_index);
			}
			break;
		case 'd':
			{
				if (0 == arg2[0] || !isdigit(*arg2))
					return;
				
				str_to_number(w_index, arg2);
				AttrTransfer_delete_item(ch, w_index);
			}
			break;
		default:
			return;
	}
}
#endif

#ifdef __POLY_SHOP__
ACMD(do_buy_marble)
{
	if (ch->IsObserverMode() || ch->GetExchange())
		return;
	
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: buy_marble_item <mobVnum> <itemCount>");
		return;
	}

	long mobVnum;
	int itemCount = 0;	
	int price;
	
	str_to_number(mobVnum, arg1);
	str_to_number(itemCount, arg2);
	if (!mobVnum || (mobVnum <= 0))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "mobVnum not exist or is wrong.");
		return;
	}
	
	if (!itemCount)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "itemCount not exist.");
		return;
	}

	if (itemCount <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "itemCount bellow 0.");
		return;
	}
	
	if (itemCount > 10)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "itemCount higher than 10.");
		return;
	}
	
	/*
	Configurable:
	add new case same like in client, to add new marble to buy
	example:
		case 501: - item vnum
			price = 250000; - item price
			break;
	*/
	switch(mobVnum)
	{
		case 501:
			price = 250000;
			break;
		case 502:
			price = 300000;
			break;
		case 551:
			price = 350000;
			break;
		case 552:
			price = 400000;
			break;
		case 554:
			price = 450000;
			break;
			
		default:
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Sorry but poly marble which you want tu buy not exist.");
			price = 0;
			break;
		}
	}
	
	if (price == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Price is wrong.");
		return;
	}
	
	if (ch->GetGold() < price * itemCount)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Not enough gold to buy these marbles.");
		return;
	}
	
	for (int createMarble = 0; createMarble < itemCount; createMarble++)
	{
		LPITEM item = ITEM_MANAGER::instance().CreateItem(70104, 1, 0, true);
		if (item)
		{
			item->SetSocket(0, mobVnum);
			int iEmptyPos = ch->GetEmptyInventory(item->GetSize());

			if (iEmptyPos != -1)
			{
				ch->PointChange(POINT_GOLD, -price);
				item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space for buy more marbles.");
				break;
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%d item not exist by that vnum.", 70104);
			break;
		}
	}
}
#endif

#ifndef __BK_RUH__
LPEVENT timer = NULL;

EVENTINFO(TMainEventInfo)
{
	LPCHARACTER	kim;	
	long skillindexx;	
	
	TMainEventInfo() 
	: kim( NULL )
	, skillindexx( 0 )
	{
	}

} ;

EVENTFUNC(bk_event)
{
	TMainEventInfo * info = dynamic_cast<TMainEventInfo *>(  event->info );

	if ( info == NULL )
	{
		sys_err( "bk_event> <Factor> Null pointer" );
		return 0;
	}
	
	LPCHARACTER	ch = info->kim;
	long skillindex = info->skillindexx;
	
	
	
	if (NULL == ch || skillindex == 0)
		return 0;
	
	if (!ch)
		return 0;
 
	if (!ch->IsPC())
		return 0;
 
    if (ch->IsDead() || ch->IsStun())
		return 0;
 
	if (ch->IsHack())
		return 0;
	
	if(ch->CountSpecifyItem(50300) < 1 )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bkyok"));
		return 0;
	}
	
	
	int skilllevel = ch->GetSkillLevel(skillindex);

	if (skilllevel >= 40)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bkskillson"));
		return 0;
	}
	int gerekenderece = (1000+500*(skilllevel-30));
	int derecem = (ch->GetRealAlignment()/10);
	int sonuc = (-19000+gerekenderece);
	if (derecem < 0)
	{
		gerekenderece = gerekenderece*2;
		sonuc = (-19000-gerekenderece);
	}
	if (derecem > sonuc)
	{
		// int gerekliknk = sonuc-derecem;
		int gerekliknk = gerekenderece;
		int kactane = gerekliknk/500;
		if (kactane < 0)
		{
			kactane = 0 - kactane;
		}
		
		if (derecem < 0)
		{
			if (ch->CountSpecifyItem(70102) < kactane)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bkzenbitti %d"),kactane);
				return 0;
			}
			
			int delta = MIN(-(ch->GetAlignment()), 500);
			ch->UpdateAlignment(delta*kactane);
			ch->RemoveSpecifyItem(70102,kactane);
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bkzenbastim"));
		}
	}
		
	if(ch->GetQuestFlag("bk.sure") > get_global_time())
	{
		if (ch->CountSpecifyItem(71001) < 1 )
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bksuredolmadi"));
			return 0;
		}
		else
		{
			ch->RemoveSpecifyItem(71001,1);
		}
			
	}
	
	if (ch->CountSpecifyItem(71094) >= 1)
	{
		ch->AddAffect(512, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
		ch->RemoveSpecifyItem(71094,1);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bkmunzevikullandim"));
	}
	
	if (gerekenderece < 0)
	{
		ch->UpdateAlignment(gerekenderece*10);
	}
	else
	{
		ch->UpdateAlignment(-gerekenderece*10);
	}
	ch->LearnSkillByBook(skillindex);
	ch->RemoveSpecifyItem(50300,1);
	ch->SetQuestFlag("bk.sure",get_global_time()+60*60*24);

	return 1;
}

ACMD(do_bkoku)
{
	int gelen;
	long skillindex;
	char arg1[256], arg2[256];
	
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	str_to_number(gelen, arg1);
	str_to_number(skillindex, arg2);
	
	if (!ch)
		return;
 
	if (!ch->IsPC())
		return;
 
    if (ch->IsDead() || ch->IsStun())
		return;
 
	if (ch->IsHack())
		return;
	
	
	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "BK okuyamazsin");
		return;
	}
	
	
	
	if(ch->CountSpecifyItem(50300) < 1 )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bkyok"));
		return;
	}
	
	if (gelen == 1) ///tek
	{
		int skilllevel = ch->GetSkillLevel(skillindex);
		int gerekenderece = (1000+500*(skilllevel-30));
		int derecem = (ch->GetRealAlignment()/10);
		int sonuc = (-19000+gerekenderece);
		if (derecem < 0)
		{
			gerekenderece = gerekenderece*2;
			sonuc = (-19000-gerekenderece);
		}
		if (derecem > sonuc)
		{
			
			// int gerekliknk = sonuc-derecem;
			int gerekliknk = gerekenderece;
			int kactane = gerekliknk/500;
			if (kactane < 0)
			{
				kactane = 0 - kactane;
			}
			
			if (derecem < 0)
			{
				if (ch->CountSpecifyItem(70102) < kactane)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bkzenbitti %d"),kactane);
					return;
				}
				
				int delta = MIN(-(ch->GetAlignment()), 500);
				ch->UpdateAlignment(delta*kactane);
				ch->RemoveSpecifyItem(70102,kactane);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bkzenbastim"));
			}
		}

		
		if(ch->GetQuestFlag("bk.sure") > get_global_time())
		{
			if (ch->CountSpecifyItem(71001) < 1 )
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bksuredolmadi"));
				return;
			}
			else
			{
				ch->RemoveSpecifyItem(71001,1);
			}
				
		}
		
		if (ch->CountSpecifyItem(71094) >= 1)
		{
			ch->AddAffect(512, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
			ch->RemoveSpecifyItem(71094,1);
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bkmunzevikullandim"));
		}
		
		if (gerekenderece < 0)
		{
			ch->UpdateAlignment(gerekenderece*10);
		}
		else
		{
			ch->UpdateAlignment(-gerekenderece*10);
		}
		
		ch->LearnSkillByBook(skillindex);
		ch->RemoveSpecifyItem(50300,1);
		ch->SetQuestFlag("bk.sure",get_global_time()+60*60*24);
		

	}
	else if(gelen == 0) ///hepsi
	{
		
		if (timer)
		{
			event_cancel(&timer);
		}
		
		TMainEventInfo* info = AllocEventInfo<TMainEventInfo>();
		
		
		info->kim = ch;
		info->skillindexx = skillindex;
		timer = event_create(bk_event, info, PASSES_PER_SEC(1));
	}
	return;
}

EVENTFUNC(ruh_event)
{
	TMainEventInfo * info = dynamic_cast<TMainEventInfo *>(  event->info );

	if ( info == NULL )
	{
		sys_err( "ruh_event> <Factor> Null pointer" );
		return 0;
	}
	
	LPCHARACTER	ch = info->kim;
	long skillindex = info->skillindexx;
	
	if (NULL == ch || skillindex == 0)
		return 0;
	
	if(ch->CountSpecifyItem(50513) < 1 )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhtasiyok"));
		return 0;
	}
	
	
	
	
	int skilllevel = ch->GetSkillLevel(skillindex);

	if (skilllevel >= 40)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhskillson"));
		return 0;
	}
	int gerekenderece = (1000+500*(skilllevel-30));
	int derecem = (ch->GetRealAlignment()/10);
	int sonuc = (-19000+gerekenderece);
	if (derecem < 0)
	{
		gerekenderece = gerekenderece*2;
		sonuc = (-19000-gerekenderece);
	}
	if (derecem > sonuc)
	{
		// int gerekliknk = sonuc-derecem;
		int gerekliknk = gerekenderece;
		int kactane = gerekliknk/500;
		if (kactane < 0)
		{
			kactane = 0 - kactane;
		}
		
		if (derecem < 0)
		{
			if (ch->CountSpecifyItem(70102) < kactane)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhzenbitti %d"),kactane);
				return 0;
			}
			
			int delta = MIN(-(ch->GetAlignment()), 500);
			ch->UpdateAlignment(delta*kactane);
			ch->RemoveSpecifyItem(70102,kactane);
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhzenbastim"));
		}
	}
		
	if(ch->GetQuestFlag("ruh.sure") > get_global_time())
	{
		if (ch->CountSpecifyItem(71001) < 1 )
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhsuredolmadi"));
			return 0;
		}
		else
		{
			ch->RemoveSpecifyItem(71001,1);
		}
			
	}
	
	if (ch->CountSpecifyItem(71094) >= 1)
	{
		ch->AddAffect(512, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
		ch->RemoveSpecifyItem(71094,1);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhmunzevikullandim"));
	}
	
	if (gerekenderece < 0)
	{
		ch->UpdateAlignment(gerekenderece*10);
	}
	else
	{
		ch->UpdateAlignment(-gerekenderece*10);
	}
	ch->LearnGrandMasterSkill(skillindex);
	ch->RemoveSpecifyItem(50513,1);
	ch->SetQuestFlag("ruh.sure",get_global_time()+60*60*24);

	return 1;
}

ACMD(do_ruhoku)
{
	int gelen;
	long skillindex;
	char arg1[256], arg2[256];
	
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	str_to_number(gelen, arg1);
	str_to_number(skillindex, arg2);
	
	if (!ch)
		return;
 
	if (!ch->IsPC())
		return;
 
    if (ch->IsDead() || ch->IsStun())
		return;
 
	if (ch->IsHack())
		return;
	
	
	if(ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중(창고,교환,상점)에는 개인상점을 사용할 수 없습니다."));
		return;
	}
	
	
	
	if(ch->CountSpecifyItem(50513) < 1 )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhtasiyok"));
		return;
	}
	
	LPITEM slot1 = ch->GetWear(WEAR_UNIQUE1);
	LPITEM slot2 = ch->GetWear(WEAR_UNIQUE2);
	
	if (NULL != slot1)
	{
		if (slot1->GetVnum() == 70048)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("pelerin_cikar"));
			return;
		}
	}
	if (NULL != slot2)
	{
		if (slot2->GetVnum() == 70048)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("pelerin_cikar"));
			return;
		}
	}
	
	int skillgrup = ch->GetSkillGroup();
	if (skillgrup == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhokuyamazsin"));
		return;
	}
	
	
	
	if (gelen == 1) ///tek
	{
		int skilllevel = ch->GetSkillLevel(skillindex);
		int gerekenderece = (1000+500*(skilllevel-30));
		int derecem = (ch->GetRealAlignment()/10);
		int sonuc = (-19000+gerekenderece);
		
		if (ch->GetQuestFlag("ruh.yenisure") > get_global_time())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruh1sn"));
			return;
		}
		
		
		if (derecem < 0)
		{
			gerekenderece = gerekenderece*2;
			sonuc = (-19000-gerekenderece);
		}
		if (derecem > sonuc)
		{
			
			// int gerekliknk = sonuc-derecem;
			int gerekliknk = gerekenderece;
			int kactane = gerekliknk/500;
			if (kactane < 0)
			{
				kactane = 0 - kactane;
			}
			
			if (derecem < 0)
			{
				if (ch->CountSpecifyItem(70102) < kactane)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhzenbitti %d"),kactane);
					return;
				}
				
				int delta = MIN(-(ch->GetAlignment()), 500);
				ch->UpdateAlignment(delta*kactane);
				ch->RemoveSpecifyItem(70102,kactane);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhzenbastim"));
			}
		}

		
		if(ch->GetQuestFlag("ruh.sure") > get_global_time())
		{
			if (ch->CountSpecifyItem(71001) < 1 )
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhsuredolmadi"));
				return;
			}
			else
			{
				ch->RemoveSpecifyItem(71001,1);
			}
				
		}
		
		if (ch->CountSpecifyItem(71094) >= 1)
		{
			ch->AddAffect(512, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
			ch->RemoveSpecifyItem(71094,1);
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhmunzevikullandim"));
		}
		
		if (gerekenderece < 0)
		{
			ch->UpdateAlignment(gerekenderece*10);
		}
		else
		{
			ch->UpdateAlignment(-gerekenderece*10);
		}
		
		ch->LearnGrandMasterSkill(skillindex);
		ch->RemoveSpecifyItem(50513,1);
		ch->SetQuestFlag("ruh.sure",get_global_time()+60*60*24);
		ch->SetQuestFlag("ruh.yenisure",get_global_time()+1);
		

	}
	else if(gelen == 0) ///hepsi
	{
		
		if (ruhtimer)
		{
			event_cancel(&ruhtimer);
		}
		
		TMainEventInfo* info = AllocEventInfo<TMainEventInfo>();
		
		
		info->kim = ch;
		info->skillindexx = skillindex;
		ruhtimer = event_create(ruh_event, info, PASSES_PER_SEC(1));
	}
	return;
}
#endif

ACMD(do_anti_exp)
{
	if (!ch)
		return;

	char arg1[12];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	int anti;
	str_to_number(anti, arg1);

	ch->SetAntiExp(anti);
}

ACMD(do_user_horse_ride)
{
	if (ch->IsObserverMode())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHorseRiding() == false)
	{
		// 말이 아닌 다른탈것을 타고있다.
		if (ch->GetMountVnum())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이미 탈것을 이용중입니다."));
			return;
		}

		if (ch->GetHorse() == NULL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 먼저 소환해주세요."));
			return;
		}

		ch->StartRiding();
	}
	else
	{
		ch->StopRiding();
	}
}

ACMD(do_user_horse_back)
{
	if (ch->GetHorse() != NULL)
	{
		ch->HorseSummon(false);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 돌려보냈습니다."));
	}
	else if (ch->IsHorseRiding() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말에서 먼저 내려야 합니다."));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 먼저 소환해주세요."));
	}
}

ACMD(do_user_horse_feed)
{
	// 개인상점을 연 상태에서는 말 먹이를 줄 수 없다.
	if (ch->GetMyShop())
		return;

	if (ch->GetHorse() == NULL)
	{
		if (ch->IsHorseRiding() == false)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 먼저 소환해주세요."));
		else
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 탄 상태에서는 먹이를 줄 수 없습니다."));
		return;
	}

	DWORD dwFood = ch->GetHorseGrade() + 50054 - 1;

	if (ch->CountSpecifyItem(dwFood) > 0)
	{
		ch->RemoveSpecifyItem(dwFood, 1);
		ch->FeedHorse();
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말에게 %s%s 주었습니다."),
				ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName,
				"");
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s 아이템이 필요합니다"), ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName);
	}
}

#define MAX_REASON_LEN		128

EVENTINFO(TimedEventInfo)
{
	DynamicCharacterPtr ch;
	int		subcmd;
	int         	left_second;
	char		szReason[MAX_REASON_LEN];

	TimedEventInfo()
	: ch()
	, subcmd( 0 )
	, left_second( 0 )
	{
		::memset( szReason, 0, MAX_REASON_LEN );
	}
};

#ifdef __MOVE_CHANNEL__
int new_ch;

EVENTFUNC(channel_event)
{
	TimedEventInfo * info = dynamic_cast<TimedEventInfo *>( event->info );
	
	if ( info == NULL )
	{
		sys_err( "timed_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == NULL) { // <Factor>
		return 0;
	}

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = NULL;

		TPacketNeedLoginLogInfo acc_info;
		acc_info.dwPlayerID = ch->GetDesc()->GetAccountTable().id;

		db_clientdesc->DBPacket( HEADER_GD_VALID_LOGOUT, 0, &acc_info, sizeof(acc_info) );

		LogManager::instance().DetailLoginLog( false, ch );

		ch->ChannelSwitch(new_ch);

		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d초 남았습니다."), info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

ACMD(do_move_channel)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	str_to_number(new_ch, arg1);

	if( new_ch < 1 || new_ch > 4)
		return;
	
	if (new_ch == g_bChannel)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Channel_Move_ch"));
			return;
		}

	if (!ch->IsPC())
		return;

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Channel_Move"));

	int nExitLimitTime = 10;

	if (ch->IsHack(false, true, nExitLimitTime) && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
		return;

	TimedEventInfo* info = AllocEventInfo<TimedEventInfo>();

	{
		if (ch->IsPosition(POS_FIGHTING))
			info->left_second = 10;
		else
			info->left_second = 3;
	}

	info->ch = ch;
	info->subcmd = subcmd;
	strlcpy(info->szReason, argument, sizeof(info->szReason));

	ch->m_pkTimedEvent	= event_create(channel_event, info, 1);
} 
#endif

struct SendDisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetCharacter())
		{
			if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
				d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit Shutdown(SendDisconnectFunc)");
		}
	}
};

struct DisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetType() == DESC_TYPE_CONNECTOR)
			return;

		if (d->IsPhase(PHASE_P2P))
			return;

		if (d->GetCharacter())
			d->GetCharacter()->Disconnect("Shutdown(DisconnectFunc)");

		d->SetPhase(PHASE_CLOSE);
	}
};

EVENTINFO(shutdown_event_data)
{
	int seconds;

	shutdown_event_data()
	: seconds( 0 )
	{
	}
};

EVENTFUNC(shutdown_event)
{
	shutdown_event_data* info = dynamic_cast<shutdown_event_data*>( event->info );

	if ( info == NULL )
	{
		sys_err( "shutdown_event> <Factor> Null pointer" );
		return 0;
	}

	int * pSec = & (info->seconds);

	if (*pSec < 0)
	{
		sys_log(0, "shutdown_event sec %d", *pSec);

		if (--*pSec == -10)
		{
			const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), DisconnectFunc());
			return passes_per_sec;
		}
		else if (*pSec < -10)
			return 0;

		return passes_per_sec;
	}
	else if (*pSec == 0)
	{
		const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
		std::for_each(c_set_desc.begin(), c_set_desc.end(), SendDisconnectFunc());
		g_bNoMoreClient = true;
		--*pSec;
		return passes_per_sec;
	}
	else
	{
		char buf[64];
		snprintf(buf, sizeof(buf), LC_TEXT("셧다운이 %d초 남았습니다."), *pSec);
		SendNotice(buf);

		--*pSec;
		return passes_per_sec;
	}
}

void Shutdown(int iSec)
{
	if (g_bNoMoreClient)
	{
		thecore_shutdown();
		return;
	}

	CWarMapManager::instance().OnShutdown();

	char buf[64];
	snprintf(buf, sizeof(buf), LC_TEXT("%d초 후 게임이 셧다운 됩니다."), iSec);

	SendNotice(buf);

	shutdown_event_data* info = AllocEventInfo<shutdown_event_data>();
	info->seconds = iSec;

	event_create(shutdown_event, info, 1);
}

ACMD(do_shutdown)
{
	if (NULL == ch)
	{
		sys_err("Accept shutdown command from %s.", ch->GetName());
	}
	TPacketGGShutdown p;
	p.bHeader = HEADER_GG_SHUTDOWN;
	P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGShutdown));

	Shutdown(10);
}

EVENTFUNC(timed_event)
{
	TimedEventInfo * info = dynamic_cast<TimedEventInfo *>( event->info );

	if ( info == NULL )
	{
		sys_err( "timed_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == NULL) { // <Factor>
		return 0;
	}
	LPDESC d = ch->GetDesc();

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = NULL;

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
			case SCMD_QUIT:
			case SCMD_PHASE_SELECT:
				{
					TPacketNeedLoginLogInfo acc_info;
					acc_info.dwPlayerID = ch->GetDesc()->GetAccountTable().id;

					db_clientdesc->DBPacket( HEADER_GD_VALID_LOGOUT, 0, &acc_info, sizeof(acc_info) );

					LogManager::instance().DetailLoginLog( false, ch );
				}
				break;
		}

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
				if (d)
					d->SetPhase(PHASE_CLOSE);
				break;

			case SCMD_QUIT:
				ch->ChatPacket(CHAT_TYPE_COMMAND, "quit");
				break;

			case SCMD_PHASE_SELECT:
				{
					ch->Disconnect("timed_event - SCMD_PHASE_SELECT");

					if (d)
					{
						d->SetPhase(PHASE_SELECT);
					}
				}
				break;
		}

		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d초 남았습니다."), info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

ACMD(do_cmd)
{
	/* RECALL_DELAY
	   if (ch->m_pkRecallEvent != NULL)
	   {
	   ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("취소 되었습니다."));
	   event_cancel(&ch->m_pkRecallEvent);
	   return;
	   }
	// END_OF_RECALL_DELAY */

	if (ch->m_pkTimedEvent)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("취소 되었습니다."));
		event_cancel(&ch->m_pkTimedEvent);
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("로그인 화면으로 돌아 갑니다. 잠시만 기다리세요."));
			break;

		case SCMD_QUIT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("게임을 종료 합니다. 잠시만 기다리세요."));
			break;

		case SCMD_PHASE_SELECT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("캐릭터를 전환 합니다. 잠시만 기다리세요."));
			break;
	}

	int nExitLimitTime = 10;

	if (ch->IsHack(false, true, nExitLimitTime) &&
	   	(!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
		case SCMD_QUIT:
		case SCMD_PHASE_SELECT:
			{
				TimedEventInfo* info = AllocEventInfo<TimedEventInfo>();

				{
					if (ch->IsPosition(POS_FIGHTING))
						info->left_second = 10;
					else
						info->left_second = 3;
				}

				info->ch		= ch;
				info->subcmd		= subcmd;
				strlcpy(info->szReason, argument, sizeof(info->szReason));

				ch->m_pkTimedEvent	= event_create(timed_event, info, 1);
			}
			break;
	}
}

ACMD(do_mount)
{
	/*
	   char			arg1[256];
	   struct action_mount_param	param;

	// 이미 타고 있으면
	if (ch->GetMountingChr())
	{
	char arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	return;

	param.x		= atoi(arg1);
	param.y		= atoi(arg2);
	param.vid	= ch->GetMountingChr()->GetVID();
	param.is_unmount = true;

	float distance = DISTANCE_SQRT(param.x - (DWORD) ch->GetX(), param.y - (DWORD) ch->GetY());

	if (distance > 600.0f)
	{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("좀 더 가까이 가서 내리세요."));
	return;
	}

	action_enqueue(ch, ACTION_TYPE_MOUNT, &param, 0.0f, true);
	return;
	}

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	return;

	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(atoi(arg1));

	if (!tch->IsNPC() || !tch->IsMountable())
	{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("거기에는 탈 수 없어요."));
	return;
	}

	float distance = DISTANCE_SQRT(tch->GetX() - ch->GetX(), tch->GetY() - ch->GetY());

	if (distance > 600.0f)
	{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("좀 더 가까이 가서 타세요."));
	return;
	}

	param.vid		= tch->GetVID();
	param.is_unmount	= false;

	action_enqueue(ch, ACTION_TYPE_MOUNT, &param, 0.0f, true);
	 */
}

#ifdef __LOCALIZATION_SYSTEM__
ACMD(do_localization)
{
	if (!ch)
		return;

	if (!ch->IsPC() || NULL == ch)
		return;

	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
		return;

	if (!strcmp(arg1, "open"))
	{
		LocalizationManager::instance().Send_Localization_Open(ch, false);
	}
	
	if (!strcmp(arg1, "refresh"))
	{
		LocalizationManager::instance().Send_Localization_Open(ch, true);
	}

	if (!strcmp(arg1, "teleport")) 
	{
		int idxWarp = 0;
		str_to_number(idxWarp, arg2);
		LocalizationManager::instance().Send_Localization_Teleport(ch, idxWarp);
	}

	if (!strcmp(arg1, "save"))
	{
		int idxSave = 0;
		str_to_number(idxSave, arg2);
		LocalizationManager::instance().Send_Localization_Save(ch, idxSave);
	}
}; 
#endif

ACMD(do_fishing)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	ch->SetRotation(atof(arg1));
	ch->fishing();
}

ACMD(do_console)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");
}

ACMD(do_restart)
{
	if (false == ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
		ch->StartRecoveryEvent();
		return;
	}

	if (NULL == ch->m_pkDeadEvent)
		return;

	int iTimeToDead = (event_time(ch->m_pkDeadEvent) / passes_per_sec);

#ifdef __BATTLE_FIELD__
	bool isBattleFieldMap = CBattleField::instance().IsBattleFieldMapIndex(ch->GetMapIndex());
	int toWaitSeconds = isBattleFieldMap ? MINMAX(g_nPortalLimitTime, 5 + (ch->GetBattleFieldDeadCount() * 5), 30) : g_nPortalLimitTime;
#endif

#ifdef __BATTLE_FIELD__
	if (subcmd != SCMD_RESTART_TOWN && subcmd != SCMD_RESTART_BATTLE && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
#else
	if (subcmd != SCMD_RESTART_TOWN && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
#endif
	{
#define eFRS_HERESEC	170
#ifdef __BATTLE_FIELD__
			if (iTimeToDead > (180 - toWaitSeconds))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 재시작 할 수 없습니다. (%d초 남음)"), iTimeToDead - (180 - toWaitSeconds));
				return;
			}
#else
			if (iTimeToDead > eFRS_HERESEC)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 재시작 할 수 없습니다. (%d초 남음)"), iTimeToDead - eFRS_HERESEC);
				return;
			}
#endif
	}

#ifdef __BATTLE_FIELD__
	if (subcmd == SCMD_RESTART_BATTLE)
	{	
		if (!isBattleFieldMap)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can use instant restart just in battle map."));
			return;
		}
		
		if (ch->CountSpecifyItem(50289) < 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You need revive item for instant restart."));
			return;
		}
	}
#endif

	if (subcmd == SCMD_RESTART_TOWN)
	{
		if (ch->IsHack())
		{
			if ((!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
			{
#ifdef __BATTLE_FIELD__
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 재시작 할 수 없습니다. (%d초 남음)"), iTimeToDead - (180 - toWaitSeconds));
#else
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 재시작 할 수 없습니다. (%d초 남음)"), iTimeToDead - (180 - g_nPortalLimitTime));
#endif
				return;
			}
		}

#define eFRS_TOWNSEC	173
#ifdef __BATTLE_FIELD__
		if (iTimeToDead > (180 - toWaitSeconds))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 마을에서 재시작 할 수 없습니다. (%d 초 남음)"), iTimeToDead - (180 - toWaitSeconds));
			return;
		}
#else
		if (iTimeToDead > eFRS_TOWNSEC)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 마을에서 재시작 할 수 없습니다. (%d 초 남음)"), iTimeToDead - eFRS_TOWNSEC);
			return;
		}
#endif
	}
	//END_PREVENT_HACK

#ifndef __12ZI__
	ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

	ch->GetDesc()->SetPhase(PHASE_GAME);
	ch->SetPosition(POS_STANDING);
	ch->StartRecoveryEvent();
#endif
	if (ch->GetDungeon())
		ch->GetDungeon()->UseRevive(ch);

	if (ch->GetWarMap() && !ch->IsObserverMode())
	{
		CWarMap * pMap = ch->GetWarMap();
		DWORD dwGuildOpponent = pMap ? pMap->GetGuildOpponent(ch) : 0;

		if (dwGuildOpponent)
		{
			switch (subcmd)
			{
				case SCMD_RESTART_TOWN:
					sys_log(0, "do_restart: restart town");
					PIXEL_POSITION pos;

					if (CWarMapManager::instance().GetStartPosition(ch->GetMapIndex(), ch->GetGuild()->GetID() < dwGuildOpponent ? 0 : 1, pos))
						ch->Show(ch->GetMapIndex(), pos.x, pos.y);
					else
						ch->ExitToSavedLocation();

					ch->PointChange(POINT_HP, ch->GetMaxHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP());
					ch->ReviveInvisible(5);
					break;

				case SCMD_RESTART_HERE:
					sys_log(0, "do_restart: restart here");
					ch->RestartAtSamePos();
					//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
					ch->PointChange(POINT_HP, ch->GetMaxHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP());
					ch->ReviveInvisible(5);
					break;
#ifdef __BATTLE_FIELD__
				case SCMD_RESTART_BATTLE:
					sys_log(0, "do_restart: restart battle");
					ch->RemoveSpecifyItem(50289, 1);
					ch->RestartAtSamePos();
					ch->PointChange(POINT_HP, ch->GetMaxHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP());
					ch->DeathPenalty(1);
					ch->ReviveInvisible(5);
					break;
#endif
			}

			ch->CheckWears();
			return;
		}
	}
	switch (subcmd)
	{
		case SCMD_RESTART_TOWN:
		{
			sys_log(0, "do_restart: restart town");

#ifdef __BATTLE_FIELD__
			if (isBattleFieldMap)
			{
				CBattleField::instance().RestartAtRandomPos(ch);
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
			}
			else
			{
				PIXEL_POSITION pos;
	
				if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
					ch->WarpSet(pos.x, pos.y);
				else
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			
				ch->PointChange(POINT_HP, 50 - ch->GetHP());
			}
#else
			PIXEL_POSITION pos;

			if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
				ch->WarpSet(pos.x, pos.y);
			else
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			
			ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
#endif
			ch->DeathPenalty(1);
		}
		break;
		case SCMD_RESTART_HERE:
		{
#ifdef __12ZI__
			if (ch->GetMapIndex() >= 3550000 && ch->GetMapIndex() < 3560000)
			{
				int REVIVE_PRICE = ch->GetQuestFlag("zodiac.PrismOfRevival") * 2;
				int vid = (int)ch->GetVID();
				ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenReviveDialog %d %d", vid, REVIVE_PRICE);
				return;
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

				ch->GetDesc()->SetPhase(PHASE_GAME);
				ch->SetPosition(POS_STANDING);
				ch->StartRecoveryEvent();

				sys_log(0, "do_restart: restart here");
				ch->RestartAtSamePos();

#ifdef __BATTLE_FIELD__
				if (isBattleFieldMap)
				{
					ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP());
					ch->DeathPenalty(1);
				}
				else
				{
					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->DeathPenalty(0);
				}
#else
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->DeathPenalty(0);
#endif
				ch->ReviveInvisible(5);
				ch->CheckWears();
			}
#else
			sys_log(0, "do_restart: restart here");
			ch->RestartAtSamePos();
			
#ifdef __BATTLE_FIELD__
			if (isBattleFieldMap)
			{
				ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP());
				ch->DeathPenalty(1);
			}
			else
			{
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->DeathPenalty(0);
			}
#else
			ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
			ch->DeathPenalty(0);
#endif
			ch->ReviveInvisible(5);
			ch->CheckWears();
#endif
		}
		break;
	}
}

#define MAX_STAT g_iStatusPointSetMaxValue

ACMD(do_stat_reset)
{
	ch->PointChange(POINT_STAT_RESET_COUNT, 12 - ch->GetPoint(POINT_STAT_RESET_COUNT));
}

ACMD(do_stat_minus)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("둔갑 중에는 능력을 올릴 수 없습니다."));
		return;
	}

	if (ch->GetPoint(POINT_STAT_RESET_COUNT) <= 0)
		return;

	if (!strcmp(arg1, "st"))
	{
		if (ch->GetRealPoint(POINT_ST) <= JobInitialPoints[ch->GetJob()].st)
			return;

		ch->SetRealPoint(POINT_ST, ch->GetRealPoint(POINT_ST) - 1);
		ch->SetPoint(POINT_ST, ch->GetPoint(POINT_ST) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_ST, 0);
	}
	else if (!strcmp(arg1, "dx"))
	{
		if (ch->GetRealPoint(POINT_DX) <= JobInitialPoints[ch->GetJob()].dx)
			return;

		ch->SetRealPoint(POINT_DX, ch->GetRealPoint(POINT_DX) - 1);
		ch->SetPoint(POINT_DX, ch->GetPoint(POINT_DX) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_DX, 0);
	}
	else if (!strcmp(arg1, "ht"))
	{
		if (ch->GetRealPoint(POINT_HT) <= JobInitialPoints[ch->GetJob()].ht)
			return;

		ch->SetRealPoint(POINT_HT, ch->GetRealPoint(POINT_HT) - 1);
		ch->SetPoint(POINT_HT, ch->GetPoint(POINT_HT) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (!strcmp(arg1, "iq"))
	{
		if (ch->GetRealPoint(POINT_IQ) <= JobInitialPoints[ch->GetJob()].iq)
			return;

		ch->SetRealPoint(POINT_IQ, ch->GetRealPoint(POINT_IQ) - 1);
		ch->SetPoint(POINT_IQ, ch->GetPoint(POINT_IQ) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_MAX_SP, 0);
	}
	else
		return;

	ch->PointChange(POINT_STAT, +1);
	ch->PointChange(POINT_STAT_RESET_COUNT, -1);
	ch->ComputePoints();
}

ACMD(do_stat)
{
	char arg1[256];
	char arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("둔갑 중에는 능력을 올릴 수 없습니다."));
		return;
	}

	if (ch->GetPoint(POINT_STAT) <= 0)
		return;

	BYTE idx = 0;
	BYTE count = 0;
	str_to_number(count, arg2);
	
	if (!strcmp(arg1, "st"))
		idx = POINT_ST;
	else if (!strcmp(arg1, "dx"))
		idx = POINT_DX;
	else if (!strcmp(arg1, "ht"))
		idx = POINT_HT;
	else if (!strcmp(arg1, "iq"))
		idx = POINT_IQ;
	else
		return;

	if (ch->GetRealPoint(idx) + count > MAX_STAT)
		return;

	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + count);
	ch->SetPoint(idx, ch->GetPoint(idx) + count);
	ch->ComputePoints();
	ch->PointChange(idx, 0);

	if (idx == POINT_IQ)
	{
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (idx == POINT_HT)
	{
		ch->PointChange(POINT_MAX_SP, 0);
	}

	ch->PointChange(POINT_STAT, -count);
	ch->ComputePoints();
}

ACMD(do_pvp)
{
	if (ch->GetArena() != NULL || CArenaManager::instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(vid);

	if (!pkVictim)
		return;

	if (pkVictim->IsNPC())
		return;

#ifdef __BATTLE_FIELD__
	if (CBattleField::instance().IsBattleFieldMapIndex(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_FIELD_CANT_FIGHT"));
		return;
	}
#endif

#ifdef __MESSENGER_BLOCK__
	if (MessengerManager::instance().CheckMessengerList(ch, pkVictim->GetName(), SYST_BLOCK))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s blokkk"), pkVictim->GetName());
		return;
	}
#endif

	if (pkVictim->GetArena() != NULL)
	{
		pkVictim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 대련중입니다."));
		return;
	}

	CPVPManager::instance().Insert(ch, pkVictim);
}

ACMD(do_guildskillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch->GetGuild())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드에 속해있지 않습니다."));
		return;
	}

	CGuild* g = ch->GetGuild();
	TGuildMember* gm = g->GetMember(ch->GetPlayerID());
	if (gm->grade == GUILD_LEADER_GRADE)
	{
		DWORD vnum = 0;
		str_to_number(vnum, arg1);
		g->SkillLevelUp(vnum);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드 스킬 레벨을 변경할 권한이 없습니다."));
	}
}

ACMD(do_skillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vnum = 0;
	str_to_number(vnum, arg1);

	if (true == ch->CanUseSkill(vnum))
	{
		ch->SkillLevelUp(vnum);
	}
	else
	{
		switch(vnum)
		{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:

			case SKILL_7_A_ANTI_TANHWAN:
			case SKILL_7_B_ANTI_AMSEOP:
			case SKILL_7_C_ANTI_SWAERYUNG:
			case SKILL_7_D_ANTI_YONGBI:

			case SKILL_8_A_ANTI_GIGONGCHAM:
			case SKILL_8_B_ANTI_YEONSA:
			case SKILL_8_C_ANTI_MAHWAN:
			case SKILL_8_D_ANTI_BYEURAK:

			case SKILL_ADD_HP:
			case SKILL_RESIST_PENETRATE:
#ifdef __678TH_SKILL__
			case SKILL_ANTI_PALBANG:
			case SKILL_ANTI_AMSEOP:
			case SKILL_ANTI_SWAERYUNG:
			case SKILL_ANTI_YONGBI:
			case SKILL_ANTI_GIGONGCHAM:
			case SKILL_ANTI_HWAJO:
			case SKILL_ANTI_MARYUNG:
			case SKILL_ANTI_BYEURAK:
#ifdef __WOLFMAN__
			case SKILL_ANTI_SALPOONG:
#endif
			case SKILL_HELP_PALBANG:
			case SKILL_HELP_AMSEOP:
			case SKILL_HELP_SWAERYUNG:
			case SKILL_HELP_YONGBI:
			case SKILL_HELP_GIGONGCHAM:
			case SKILL_HELP_HWAJO:
			case SKILL_HELP_MARYUNG:
			case SKILL_HELP_BYEURAK:
#ifdef __WOLFMAN__
			case SKILL_HELP_SALPOONG:
#endif
#endif
				ch->SkillLevelUp(vnum);
				break;
		}
	}
}

//
// @version	05/06/20 Bang2ni - 커맨드 처리 Delegate to CHARACTER class
//
ACMD(do_safebox_close)
{
	ch->CloseSafebox();
}

//
// @version	05/06/20 Bang2ni - 커맨드 처리 Delegate to CHARACTER class
//
ACMD(do_safebox_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	ch->ReqSafeboxLoad(arg1);
}

ACMD(do_safebox_change_password)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || strlen(arg1)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 잘못된 암호를 입력하셨습니다."));
		return;
	}

	if (!*arg2 || strlen(arg2)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 잘못된 암호를 입력하셨습니다."));
		return;
	}

	TSafeboxChangePasswordPacket p;

	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szOldPassword, arg1, sizeof(p.szOldPassword));
	strlcpy(p.szNewPassword, arg2, sizeof(p.szNewPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_PASSWORD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 잘못된 암호를 입력하셨습니다."));
		return;
	}

	int iPulse = thecore_pulse();

	if (ch->GetMall())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 창고가 이미 열려있습니다."));
		return;
	}

	if (iPulse - ch->GetMallLoadTime() < passes_per_sec * 10) // 10초에 한번만 요청 가능
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 창고를 닫은지 10초 안에는 열 수 없습니다."));
		return;
	}

	ch->SetMallLoadTime(iPulse);

	TSafeboxLoadPacket p;
	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, ch->GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, arg1, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_MALL_LOAD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_close)
{
	if (ch->GetMall())
	{
		ch->SetMallLoadTime(thecore_pulse());
		ch->CloseMall();
		ch->Save();
	}
}

ACMD(do_ungroup)
{
	if (!ch->GetParty())
		return;

	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던전 안에서는 파티에서 나갈 수 없습니다."));
		return;
	}

	LPPARTY pParty = ch->GetParty();

	if (pParty->GetMemberCount() == 2)
	{
		// party disband
		CPartyManager::instance().DeleteParty(pParty);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티에서 나가셨습니다."));
		//pParty->SendPartyRemoveOneToAll(ch);
		pParty->Quit(ch->GetPlayerID());
		//pParty->SendPartyRemoveAllToOne(ch);
	}
}

ACMD(do_close_shop)
{
	if (ch->IsObserverMode())
		return;
	if (ch->GetMyShop())
	{
		ch->CloseMyShop();
		return;
	}
#ifdef __OFFLINE_PRIVATE_SHOP__
	else
	{
		char arg1[256];
		one_argument(argument, arg1, sizeof(arg1));

		if (!*arg1)
			return;

		DWORD dwVid = 0;
		str_to_number(dwVid, arg1);

		CShopManager::instance().DestroyOfflinePrivateShop(dwVid);
	}
#endif
}

#ifdef __GROWTH_PET__
ACMD(do_CubePetAdd)
{

	int pos = 0;
	int invpos = 0;

	const char *line;
	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
		return;
	switch (LOWER(arg1[0]))
	{
	case 'a':	// add cue_index inven_index
	{
		if (0 == arg2[0] || !isdigit(*arg2) ||
			0 == arg3[0] || !isdigit(*arg3))
			return;

		str_to_number(pos, arg2);
		str_to_number(invpos, arg3);

	}
	break;

	default:
		return;
	}

	if (ch->GetNewPetSystem()->IsActivePet())
		ch->GetNewPetSystem()->SetItemCube(pos, invpos);
	else
		return;

}

ACMD(do_PetSkill) {
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	DWORD skillslot = 0;
	str_to_number(skillslot, arg1);
	if (skillslot > 2 || skillslot < 0)
		return;

	if (ch->GetNewPetSystem()->IsActivePet())
		ch->GetNewPetSystem()->DoPetSkill(skillslot);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Devam etmek icin Pet'i cagirmis olmaniz gerekir.");
}

ACMD(do_FeedCubePet) {
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	DWORD feedtype = 0;
	str_to_number(feedtype, arg1);
	if (ch->GetNewPetSystem()->IsActivePet())
		ch->GetNewPetSystem()->ItemCubeFeed(feedtype);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Lutfen once Petinizi cagiriniz!");
}

ACMD(do_PetEvo) {

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen()) {
		ch->ChatPacket(CHAT_TYPE_INFO, "[Pet Gelistirme] Evcil hayvaniniz daha fazla gelistirilemez.");
		return;
	}
	if (ch->GetNewPetSystem()->IsActivePet()) {

		int it[3][7] = { 
						{ 55003, 30058, 30073, 30041, 30017, 30074, 30088 }, //Here Modify Items to request for 1 evo
						{ 27993 }, //Here Modify Items to request for 2 evo
						{ 27994 }  //Here Modify Items to request for 3 evo
		};
		int ic[3][7] = {
						{ 10, 10, 10, 10, 10, 5, 5},
						{ 2 },
						{ 3 }
		};
		int tmpevo = ch->GetNewPetSystem()->GetEvolution();

		if (tmpevo == 2 && ch->GetNewPetSystem()->GetAge() < 30 && ch->GetNewPetSystem()->GetLevel() == 81)
			return;

		if (ch->GetNewPetSystem()->GetLevel() == 40 && tmpevo == 0 ||
			ch->GetNewPetSystem()->GetLevel() == 81 && tmpevo == 1 ||
			ch->GetNewPetSystem()->GetLevel() == 81 && tmpevo == 2) {
			for (int b = 0; b < 7; b++) {
				if (ch->CountSpecifyItem(it[tmpevo][b]) < ic[tmpevo][b]) {
					ch->ChatPacket(CHAT_TYPE_INFO, "[Pet Gelistirme] Gereken itemler:");
					for (int c = 0; c < 7; c++)
					{
						DWORD vnum = it[tmpevo][c];
						ch->ChatPacket(CHAT_TYPE_INFO, "%s X%d", ITEM_MANAGER::instance().GetTable(vnum)->szLocaleName , ic[tmpevo][c]);
					}
					return;
				}
			}
			for (int c = 0; c < 1; c++) {
				ch->RemoveSpecifyItem(it[tmpevo][c], ic[tmpevo][c]);
			}
			ch->GetNewPetSystem()->IncreasePetEvolution();

		}
		else {
			ch->ChatPacket(CHAT_TYPE_INFO, "Senin Pet Suan icin Gelistirilemez!");
			return;
		}

	}else
		ch->ChatPacket(CHAT_TYPE_INFO, "Once Pet'i Cagir!");

}

#endif

ACMD(do_set_walk_mode)
{
	ch->SetNowWalking(true);
	ch->SetWalking(true);
}

ACMD(do_set_run_mode)
{
	ch->SetNowWalking(false);
	ch->SetWalking(false);
}

ACMD(do_war)
{
	//내 길드 정보를 얻어오고
	CGuild * g = ch->GetGuild();

	if (!g)
		return;

	//전쟁중인지 체크한번!
	if (g->UnderAnyWar())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 이미 다른 전쟁에 참전 중 입니다."));
		return;
	}

	//파라메터를 두배로 나누고
	DWORD type = GUILD_WAR_TYPE_FIELD; //fixme102 base int modded uint
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
		return;

	if (*arg2)
	{
		str_to_number(type, arg2);

		if (type >= GUILD_WAR_TYPE_MAX_NUM)
			type = GUILD_WAR_TYPE_FIELD;
	}

	//길드의 마스터 아이디를 얻어온뒤
	DWORD gm_pid = g->GetMasterPID();

	//마스터인지 체크(길전은 길드장만이 가능)
	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전에 대한 권한이 없습니다."));
		return;
	}

	//상대 길드를 얻어오고
	CGuild * opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 그런 길드가 없습니다."));
		return;
	}

	//상대길드와의 상태 체크
	switch (g->GetGuildWarState(opp_g->GetID()))
	{
		case GUILD_WAR_NONE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드가 이미 전쟁 중 입니다."));
					return;
				}

				int iWarPrice = aGuildWarInfo[type].iWarPrice;

				if (g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 전비가 부족하여 길드전을 할 수 없습니다."));
					return;
				}

				if (opp_g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드의 전비가 부족하여 길드전을 할 수 없습니다."));
					return;
				}
			}
			break;

		case GUILD_WAR_SEND_DECLARE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이미 선전포고 중인 길드입니다."));
				return;
			}
			break;

		case GUILD_WAR_RECV_DECLARE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드가 이미 전쟁 중 입니다."));
					g->RequestRefuseWar(opp_g->GetID());
					return;
				}
			}
			break;

		case GUILD_WAR_RESERVE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 이미 전쟁이 예약된 길드 입니다."));
				return;
			}
			break;

		case GUILD_WAR_END:
			return;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 이미 전쟁 중인 길드입니다."));
			g->RequestRefuseWar(opp_g->GetID());
			return;
	}

	if (!g->CanStartWar(type))
	{
		// 길드전을 할 수 있는 조건을 만족하지않는다.
		if (g->GetLadderPoint() == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 레더 점수가 모자라서 길드전을 할 수 없습니다."));
			sys_log(0, "GuildWar.StartError.NEED_LADDER_POINT");
		}
		else if (g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전을 하기 위해선 최소한 %d명이 있어야 합니다."), GUILD_WAR_MIN_MEMBER_COUNT);
			sys_log(0, "GuildWar.StartError.NEED_MINIMUM_MEMBER[%d]", GUILD_WAR_MIN_MEMBER_COUNT);
		}
		else
		{
			sys_log(0, "GuildWar.StartError.UNKNOWN_ERROR");
		}
		return;
	}

	// 필드전 체크만 하고 세세한 체크는 상대방이 승낙할때 한다.
	if (!opp_g->CanStartWar(GUILD_WAR_TYPE_FIELD))
	{
		if (opp_g->GetLadderPoint() == 0)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드의 레더 점수가 모자라서 길드전을 할 수 없습니다."));
		else if (opp_g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드의 길드원 수가 부족하여 길드전을 할 수 없습니다."));
		return;
	}

	do
	{
		if (g->GetMasterCharacter() != NULL)
			break;

		CCI *pCCI = P2P_MANAGER::instance().FindByPID(g->GetMasterPID());

		if (pCCI != NULL)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드의 길드장이 접속중이 아닙니다."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	do
	{
		if (opp_g->GetMasterCharacter() != NULL)
			break;

		CCI *pCCI = P2P_MANAGER::instance().FindByPID(opp_g->GetMasterPID());

		if (pCCI != NULL)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드의 길드장이 접속중이 아닙니다."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	g->RequestDeclareWar(opp_g->GetID(), type);
}

ACMD(do_nowar)
{
	CGuild* g = ch->GetGuild();
	if (!g)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전에 대한 권한이 없습니다."));
		return;
	}

	CGuild* opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 그런 길드가 없습니다."));
		return;
	}

	g->RequestRefuseWar(opp_g->GetID());
}

ACMD(do_detaillog)
{
	ch->DetailLog();
}

ACMD(do_monsterlog)
{
	ch->ToggleMonsterLog();
}

ACMD(do_pkmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	BYTE mode = 0;
	str_to_number(mode, arg1);

	if (mode == PK_MODE_PROTECT)
		return;

#ifdef __BATTLE_FIELD__
	if (CBattleField::instance().IsBattleFieldMapIndex(ch->GetMapIndex()))
		return;
#endif

	if (ch->GetLevel() < PK_PROTECT_LEVEL && mode != 0)
		return;

	ch->SetPKMode(mode);
}

ACMD(do_messenger_auth)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	char answer = LOWER(*arg1);
	// @fixme130 AuthToAdd void -> bool
	bool bIsDenied = answer != 'y';
	bool bIsAdded = MessengerManager::instance().AuthToAdd(ch->GetName(), arg2, bIsDenied); // DENY
	if (bIsAdded && bIsDenied)
	{
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg2);

		if (tch)
			tch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s 님으로 부터 친구 등록을 거부 당했습니다."), ch->GetName());
	}

}

ACMD(do_setblockmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		BYTE flag = 0;
		str_to_number(flag, arg1);
		ch->SetBlockMode(flag);
	}
}

ACMD(do_unmount)
{
#ifdef __COSTUME_MOUNT__
#ifndef __MOUNT__
	LPITEM item = ch->GetWear(WEAR_COSTUME_MOUNT);
	if (item && item->IsRideItem())
		ch->UnequipItem(item);
#endif
#else
	LPITEM item = ch->GetWear(WEAR_UNIQUE1);
	LPITEM item2 = ch->GetWear(WEAR_UNIQUE2);

	if (item && item->IsRideItem())
		ch->UnequipItem(item);
  
	if (item2 && item2->IsRideItem())
		ch->UnequipItem(item2);
#endif
	if (true == ch->UnEquipSpecialRideUniqueItem())
	{
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);

		if (ch->IsHorseRiding())
			ch->StopRiding();
	}
	else
		ch->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("인벤토리가 꽉 차서 내릴 수 없습니다."));
}

ACMD(do_observer_exit)
{
	if (ch->IsObserverMode())
	{
		if (ch->GetWarMap())
			ch->SetWarMap(NULL);

		if (ch->GetArena() != NULL || ch->GetArenaObserverMode() == true)
		{
			ch->SetArenaObserverMode(false);

			if (ch->GetArena() != NULL)
				ch->GetArena()->RemoveObserver(ch->GetPlayerID());

			ch->SetArena(NULL);
			ch->WarpSet(ARENA_RETURN_POINT_X(ch->GetEmpire()), ARENA_RETURN_POINT_Y(ch->GetEmpire()));
		}
		else {
			ch->ExitToSavedLocation();
		}
		ch->SetObserverMode(false);
	}
}

ACMD(do_view_equip)
{
	if (ch->GetGMLevel() <= GM_PLAYER)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		DWORD vid = 0;
		str_to_number(vid, arg1);
		LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

		if (!tch)
			return;

		if (!tch->IsPC())
			return;
		/*
		   int iSPCost = ch->GetMaxSP() / 3;

		   if (ch->GetSP() < iSPCost)
		   {
		   ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("정신력이 부족하여 다른 사람의 장비를 볼 수 없습니다."));
		   return;
		   }
		   ch->PointChange(POINT_SP, -iSPCost);
		 */
		tch->SendEquipment(ch);
	}
}

ACMD(do_party_request)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	if (ch->GetParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이미 파티에 속해 있으므로 가입신청을 할 수 없습니다."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		if (!ch->RequestToParty(tch))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

ACMD(do_party_request_accept)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->AcceptToParty(tch);
}

ACMD(do_party_request_deny)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->DenyToParty(tch);
}

// LUA_ADD_GOTO_INFO
struct GotoInfo
{
	std::string 	st_name;

	BYTE 	empire;
	int 	mapIndex;
	DWORD 	x, y;

	GotoInfo()
	{
		st_name 	= "";
		empire 		= 0;
		mapIndex 	= 0;

		x = 0;
		y = 0;
	}

	GotoInfo(const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void operator = (const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void __copy__(const GotoInfo& c_src)
	{
		st_name 	= c_src.st_name;
		empire 		= c_src.empire;
		mapIndex 	= c_src.mapIndex;

		x = c_src.x;
		y = c_src.y;
	}
};

static const char* FN_point_string(int apply_number)
{
	switch (apply_number)
	{
		case POINT_MAX_HP:	return LC_TEXT("최대 생명력 +%d");
		case POINT_MAX_SP:	return LC_TEXT("최대 정신력 +%d");
		case POINT_HT:		return LC_TEXT("체력 +%d");
		case POINT_IQ:		return LC_TEXT("지능 +%d");
		case POINT_ST:		return LC_TEXT("근력 +%d");
		case POINT_DX:		return LC_TEXT("민첩 +%d");
		case POINT_ATT_SPEED:	return LC_TEXT("공격속도 +%d");
		case POINT_MOV_SPEED:	return LC_TEXT("이동속도 %d");
		case POINT_CASTING_SPEED:	return LC_TEXT("쿨타임 -%d");
		case POINT_HP_REGEN:	return LC_TEXT("생명력 회복 +%d");
		case POINT_SP_REGEN:	return LC_TEXT("정신력 회복 +%d");
		case POINT_POISON_PCT:	return LC_TEXT("독공격 %d");
#ifdef __WOLFMAN__
		case POINT_BLEEDING_PCT:	return LC_TEXT("독공격 %d");
#endif
		case POINT_STUN_PCT:	return LC_TEXT("스턴 +%d");
		case POINT_SLOW_PCT:	return LC_TEXT("슬로우 +%d");
		case POINT_CRITICAL_PCT:	return LC_TEXT("%d%% 확률로 치명타 공격");
		case POINT_RESIST_CRITICAL:	return LC_TEXT("상대의 치명타 확률 %d%% 감소");
		case POINT_PENETRATE_PCT:	return LC_TEXT("%d%% 확률로 관통 공격");
		case POINT_RESIST_PENETRATE: return LC_TEXT("상대의 관통 공격 확률 %d%% 감소");
		case POINT_ATTBONUS_HUMAN:	return LC_TEXT("인간류 몬스터 타격치 +%d%%");
		case POINT_ATTBONUS_ANIMAL:	return LC_TEXT("동물류 몬스터 타격치 +%d%%");
		case POINT_ATTBONUS_ORC:	return LC_TEXT("웅귀족 타격치 +%d%%");
		case POINT_ATTBONUS_MILGYO:	return LC_TEXT("밀교류 타격치 +%d%%");
		case POINT_ATTBONUS_UNDEAD:	return LC_TEXT("시체류 타격치 +%d%%");
		case POINT_ATTBONUS_DEVIL:	return LC_TEXT("악마류 타격치 +%d%%");
		case POINT_STEAL_HP:		return LC_TEXT("타격치 %d%% 를 생명력으로 흡수");
		case POINT_STEAL_SP:		return LC_TEXT("타력치 %d%% 를 정신력으로 흡수");
		case POINT_MANA_BURN_PCT:	return LC_TEXT("%d%% 확률로 타격시 상대 전신력 소모");
		case POINT_DAMAGE_SP_RECOVER:	return LC_TEXT("%d%% 확률로 피해시 정신력 회복");
		case POINT_BLOCK:			return LC_TEXT("물리타격시 블럭 확률 %d%%");
		case POINT_DODGE:			return LC_TEXT("활 공격 회피 확률 %d%%");
		case POINT_RESIST_SWORD:	return LC_TEXT("한손검 방어 %d%%");
		case POINT_RESIST_TWOHAND:	return LC_TEXT("양손검 방어 %d%%");
		case POINT_RESIST_DAGGER:	return LC_TEXT("두손검 방어 %d%%");
		case POINT_RESIST_BELL:		return LC_TEXT("방울 방어 %d%%");
		case POINT_RESIST_FAN:		return LC_TEXT("부채 방어 %d%%");
		case POINT_RESIST_BOW:		return LC_TEXT("활공격 저항 %d%%");
#ifdef __WOLFMAN__
		case POINT_RESIST_CLAW:		return LC_TEXT("두손검 방어 %d%%");
#endif
		case POINT_RESIST_FIRE:		return LC_TEXT("화염 저항 %d%%");
		case POINT_RESIST_ELEC:		return LC_TEXT("전기 저항 %d%%");
		case POINT_RESIST_MAGIC:	return LC_TEXT("마법 저항 %d%%");
		case POINT_RESIST_WIND:		return LC_TEXT("바람 저항 %d%%");
		case POINT_RESIST_ICE:		return LC_TEXT("냉기 저항 %d%%");
		case POINT_RESIST_EARTH:	return LC_TEXT("대지 저항 %d%%");
		case POINT_RESIST_DARK:		return LC_TEXT("어둠 저항 %d%%");
		case POINT_REFLECT_MELEE:	return LC_TEXT("직접 타격치 반사 확률 : %d%%");
		case POINT_REFLECT_CURSE:	return LC_TEXT("저주 되돌리기 확률 %d%%");
		case POINT_POISON_REDUCE:	return LC_TEXT("독 저항 %d%%");
#ifdef __WOLFMAN__
		case POINT_BLEEDING_REDUCE:	return LC_TEXT("독 저항 %d%%");
#endif
		case POINT_KILL_SP_RECOVER:	return LC_TEXT("%d%% 확률로 적퇴치시 정신력 회복");
		case POINT_EXP_DOUBLE_BONUS:	return LC_TEXT("%d%% 확률로 적퇴치시 경험치 추가 상승");
		case POINT_GOLD_DOUBLE_BONUS:	return LC_TEXT("%d%% 확률로 적퇴치시 돈 2배 드롭");
		case POINT_ITEM_DROP_BONUS:	return LC_TEXT("%d%% 확률로 적퇴치시 아이템 2배 드롭");
		case POINT_POTION_BONUS:	return LC_TEXT("물약 사용시 %d%% 성능 증가");
		case POINT_KILL_HP_RECOVERY:	return LC_TEXT("%d%% 확률로 적퇴치시 생명력 회복");
//		case POINT_IMMUNE_STUN:	return LC_TEXT("기절하지 않음 %d%%");
//		case POINT_IMMUNE_SLOW:	return LC_TEXT("느려지지 않음 %d%%");
//		case POINT_IMMUNE_FALL:	return LC_TEXT("넘어지지 않음 %d%%");
//		case POINT_SKILL:	return LC_TEXT("");
//		case POINT_BOW_DISTANCE:	return LC_TEXT("");
		case POINT_ATT_GRADE_BONUS:	return LC_TEXT("공격력 +%d");
		case POINT_DEF_GRADE_BONUS:	return LC_TEXT("방어력 +%d");
		case POINT_MAGIC_ATT_GRADE:	return LC_TEXT("마법 공격력 +%d");
		case POINT_MAGIC_DEF_GRADE:	return LC_TEXT("마법 방어력 +%d");
//		case POINT_CURSE_PCT:	return LC_TEXT("");
		case POINT_MAX_STAMINA:	return LC_TEXT("최대 지구력 +%d");
		case POINT_ATTBONUS_WARRIOR:	return LC_TEXT("무사에게 강함 +%d%%");
		case POINT_ATTBONUS_ASSASSIN:	return LC_TEXT("자객에게 강함 +%d%%");
		case POINT_ATTBONUS_SURA:		return LC_TEXT("수라에게 강함 +%d%%");
		case POINT_ATTBONUS_SHAMAN:		return LC_TEXT("무당에게 강함 +%d%%");
#ifdef __WOLFMAN__
		case POINT_ATTBONUS_WOLFMAN:	return LC_TEXT("무당에게 강함 +%d%%");
#endif
		case POINT_ATTBONUS_MONSTER:	return LC_TEXT("몬스터에게 강함 +%d%%");
		case POINT_MALL_ATTBONUS:		return LC_TEXT("공격력 +%d%%");
		case POINT_MALL_DEFBONUS:		return LC_TEXT("방어력 +%d%%");
		case POINT_MALL_EXPBONUS:		return LC_TEXT("경험치 %d%%");
		case POINT_MALL_ITEMBONUS:		return LC_TEXT("아이템 드롭율 %.1f배");
		case POINT_MALL_GOLDBONUS:		return LC_TEXT("돈 드롭율 %.1f배");
		case POINT_MAX_HP_PCT:			return LC_TEXT("최대 생명력 +%d%%");
		case POINT_MAX_SP_PCT:			return LC_TEXT("최대 정신력 +%d%%");
		case POINT_SKILL_DAMAGE_BONUS:	return LC_TEXT("스킬 데미지 %d%%");
		case POINT_NORMAL_HIT_DAMAGE_BONUS:	return LC_TEXT("평타 데미지 %d%%");
		case POINT_SKILL_DEFEND_BONUS:		return LC_TEXT("스킬 데미지 저항 %d%%");
		case POINT_NORMAL_HIT_DEFEND_BONUS:	return LC_TEXT("평타 데미지 저항 %d%%");
//		case POINT_PC_BANG_EXP_BONUS:	return LC_TEXT("");
//		case POINT_PC_BANG_DROP_BONUS:	return LC_TEXT("");
//		case POINT_EXTRACT_HP_PCT:	return LC_TEXT("");
		case POINT_RESIST_WARRIOR:	return LC_TEXT("무사공격에 %d%% 저항");
		case POINT_RESIST_ASSASSIN:	return LC_TEXT("자객공격에 %d%% 저항");
		case POINT_RESIST_SURA:		return LC_TEXT("수라공격에 %d%% 저항");
		case POINT_RESIST_SHAMAN:	return LC_TEXT("무당공격에 %d%% 저항");
#ifdef __WOLFMAN__
		case POINT_RESIST_WOLFMAN:	return LC_TEXT("무당공격에 %d%% 저항");
#endif
		default:					return NULL;
	}
}

static bool FN_hair_affect_string(LPCHARACTER ch, char *buf, size_t bufsiz)
{
	if (NULL == ch || NULL == buf)
		return false;

	CAffect* aff = NULL;
	time_t expire = 0;
	struct tm ltm;
	int	year, mon, day;
	int	offset = 0;

	aff = ch->FindAffect(AFFECT_HAIR);

	if (NULL == aff)
		return false;

	expire = ch->GetQuestFlag("hair.limit_time");

	if (expire < get_global_time())
		return false;

	// set apply string
	offset = snprintf(buf, bufsiz, FN_point_string(aff->bApplyOn), aff->lApplyValue);

	if (offset < 0 || offset >= (int) bufsiz)
		offset = bufsiz - 1;

	localtime_r(&expire, &ltm);

	year	= ltm.tm_year + 1900;
	mon		= ltm.tm_mon + 1;
	day		= ltm.tm_mday;

	snprintf(buf + offset, bufsiz - offset, LC_TEXT(" (만료일 : %d년 %d월 %d일)"), year, mon, day);

	return true;
}

ACMD(do_costume)
{
#ifdef __COSTUME_ACCE__
	char buf[768];
#else
	char buf[512];
#endif
	const size_t bufferSize = sizeof(buf);

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CItem* pBody = ch->GetWear(WEAR_COSTUME_BODY);
	CItem* pHair = ch->GetWear(WEAR_COSTUME_HAIR);
#ifdef __COSTUME_ACCE__
	CItem * pAcce = ch->GetWear(WEAR_COSTUME_ACCE);
#endif
	ch->ChatPacket(CHAT_TYPE_INFO, "COSTUME status:");

	if (pHair)
	{
		const char* itemName = pHair->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  HAIR : %s", itemName);

		for (int i = 0; i < pHair->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pHair->GetAttribute(i);
			if (0 < attr.bType)
			{
				snprintf(buf, bufferSize, FN_point_string(attr.bType), attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "     %s", buf);
			}
		}

		if (pHair->IsEquipped() && arg1[0] == 'h')
			ch->UnequipItem(pHair);
	}

	if (pBody)
	{
		const char* itemName = pBody->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  BODY : %s", itemName);

		if (pBody->IsEquipped() && arg1[0] == 'b')
			ch->UnequipItem(pBody);
	}

#ifdef __COSTUME_ACCE__
	if (pAcce)
	{
		const char * itemName = pAcce->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  ACCE: %s", itemName);
		for (int i = 0; i < pAcce->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pAcce->GetAttribute(i);
			if (attr.bType > 0)
			{
				const char * pAttrName = FN_point_string(attr.bType);
				if (pAttrName == NULL)
					continue;
				
				snprintf(buf, sizeof(buf), FN_point_string(attr.bType), attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "     %s", buf);
			}
		}

		if (pAcce->IsEquipped() && arg1[0] == 's')
			ch->UnequipItem(pAcce);
	}
#endif
}

ACMD(do_hair)
{
	char buf[256];

	if (false == FN_hair_affect_string(ch, buf, sizeof(buf)))
		return;

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

ACMD(do_inventory)
{
	int	index = 0;
	int	count		= 1;

	char arg1[256];
	char arg2[256];

	LPITEM	item;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: inventory <start_index> <count>");
		return;
	}

	if (!*arg2)
	{
		index = 0;
		str_to_number(count, arg1);
	}
	else
	{
		str_to_number(index, arg1); index = MIN(index, ch->GetInventoryMax());
		str_to_number(count, arg2); count = MIN(count, ch->GetInventoryMax());
	}

	for (int i = 0; i < count; ++i)
	{
		if (index >= ch->GetInventoryMax())
			break;

		item = ch->GetInventoryItem(index);

		ch->ChatPacket(CHAT_TYPE_INFO, "inventory [%d] = %s",
						index, item ? item->GetName() : "<NONE>");
		++index;
	}
}

//gift notify quest command
ACMD(do_gift)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "gift");
}

ACMD(do_cube)
{
	if (!ch->CanDoCube())
		return;

	int cube_index = 0, inven_index = 0;
#ifdef __NEW_STORAGE__
	int inven_type = 0;
	
	char arg1[256], arg2[256], arg3[256], arg4[256];
	two_arguments (two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3), arg4, sizeof(arg4));
#else
	const char *line;
	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));
#endif

	if (0 == arg1[0])
	{
		// print usage
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: cube open");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube close");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube add <inveltory_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube delete <cube_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube list");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube cancel");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube make [all]");
		return;
	}

	const std::string& strArg1 = std::string(arg1);

	// r_info (request information)
	// /cube r_info     ==> (Client -> Server) 현재 NPC가 만들 수 있는 레시피 요청
	//					    (Server -> Client) /cube r_list npcVNUM resultCOUNT 123,1/125,1/128,1/130,5
	//
	// /cube r_info 3   ==> (Client -> Server) 현재 NPC가 만들수 있는 레시피 중 3번째 아이템을 만드는 데 필요한 정보를 요청
	// /cube r_info 3 5 ==> (Client -> Server) 현재 NPC가 만들수 있는 레시피 중 3번째 아이템부터 이후 5개의 아이템을 만드는 데 필요한 재료 정보를 요청
	//					   (Server -> Client) /cube m_info startIndex count 125,1|126,2|127,2|123,5&555,5&555,4/120000@125,1|126,2|127,2|123,5&555,5&555,4/120000
	//
	if (strArg1 == "r_info")
	{
		if (0 == arg2[0])
			Cube_request_result_list(ch);
		else
		{
			if (isdigit(*arg2))
			{
				int listIndex = 0, requestCount = 1;
				str_to_number(listIndex, arg2);

				if (0 != arg3[0] && isdigit(*arg3))
					str_to_number(requestCount, arg3);

				Cube_request_material_info(ch, listIndex, requestCount);
			}
		}

		return;
	}

	switch (LOWER(arg1[0]))
	{
		case 'o':	// open
			Cube_open(ch);
			break;

		case 'c':	// close
			Cube_close(ch);
			break;

		case 'l':	// list
			Cube_show_list(ch);
			break;

		case 'a':	// add cue_index inven_index
			{
#ifdef __NEW_STORAGE__
				if (0 == arg2[0] || !isdigit(*arg2) || 0 == arg3[0] || !isdigit(*arg3) || 0 == arg4[0] || !isdigit(*arg4))
#else
				if (0 == arg2[0] || !isdigit(*arg2) || 0 == arg3[0] || !isdigit(*arg3))
#endif
					return;

				str_to_number(cube_index, arg2);
				str_to_number(inven_index, arg3);
#ifdef __NEW_STORAGE__
				str_to_number(inven_type, arg4);
				Cube_add_item (ch, cube_index, inven_index, inven_type);
#else
				Cube_add_item (ch, cube_index, inven_index);
#endif
			}
			break;

		case 'd':	// delete
			{
				if (0 == arg2[0] || !isdigit(*arg2))
					return;

				str_to_number(cube_index, arg2);
				Cube_delete_item (ch, cube_index);
			}
			break;

		case 'm':	// make
			if (0 != arg2[0])
				while (true == Cube_make(ch))
					continue;
			else
				Cube_make(ch);
			break;

		default:
			return;
	}
}

ACMD(do_in_game_mall)
{
	return;
}

// 주사위
ACMD(do_dice)
{
	char arg1[256], arg2[256];
	int start = 1, end = 100;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		start = atoi(arg1);
		end = atoi(arg2);
	}
	else if (*arg1 && !*arg2)
	{
		start = 1;
		end = atoi(arg1);
	}

	end = MAX(start, end);
	start = MIN(start, end);

	int n = number(start, end);

	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_INFO, LC_TEXT("%s님이 주사위를 굴려 %d가 나왔습니다. (%d-%d)"), ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("당신이 주사위를 굴려 %d가 나왔습니다. (%d-%d)"), n, start, end);
}

#ifdef __NEWSTUFF__
ACMD(do_click_safebox)
{
	if (ch->GetDungeon() || ch->GetWarMap())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
		return;
	}

	ch->SetSafeboxOpenPosition();
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
}
ACMD(do_force_logout)
{
	LPDESC pDesc=DESC_MANAGER::instance().FindByCharacterName(ch->GetName());
	if (!pDesc)
		return;
	pDesc->DelayedDisconnect(0);
}
#endif

ACMD(do_click_mall)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
}

ACMD(do_ride)
{
    if (ch->IsDead() || ch->IsStun())
		return;

#ifdef __MOUNT__
	if (ch->GetMountingVnumM() && ch->GetWear(WEAR_COSTUME_MOUNT))
	{
		CMountSystem* MountSystem = ch->GetMountSystem();
		if (MountSystem)
		{
			bool MState = MountSystem->IsMounting(ch->GetMountingVnumM());
			if (MState)
			{
				MountSystem->Unmount(ch->GetMountingVnumM());
				return;
			}
			else
			{
				do_unmount(ch, NULL, 0, 0);
				if (ch->IsHorseRiding())
				    ch->StopRiding(); 
				MountSystem->Mount(ch->GetMountingVnumM());
				return;
			}
		} 
	}
#endif

    {
	if (ch->IsHorseRiding())
	{
	    ch->StopRiding();
	    return;
	}

	if (ch->GetMountVnum())
	{
	    do_unmount(ch, NULL, 0, 0);
	    return;
	}
    }

    // 타기
    {
	if (ch->GetHorse() != NULL)
	{
	    ch->StartRiding();
	    return;
	}

	for (BYTE i = 0; i < ch->GetInventoryMax(); ++i)
	{
	    LPITEM item = ch->GetInventoryItem(i);
	    if (NULL == item)
		continue;

	    // 유니크 탈것 아이템
		if (item->IsRideItem())
		{
			if (NULL == ch->GetWear(WEAR_UNIQUE1) || NULL == ch->GetWear(WEAR_UNIQUE2)
#if defined(__COSTUME_MOUNT__) && !defined(__MOUNT__)
 || NULL == ch->GetWear(WEAR_COSTUME_MOUNT)
#endif
)
			{
				//ch->EquipItem(item);
				ch->UseItem(TItemPos (INVENTORY, i));
				return;
			}
		}

	    // 일반 탈것 아이템
	    // TODO : 탈것용 SubType 추가
	    switch (item->GetVnum())
	    {
		case 71114:	// 저신이용권
		case 71116:	// 산견신이용권
		case 71118:	// 투지범이용권
		case 71120:	// 사자왕이용권
		    ch->UseItem(TItemPos (INVENTORY, i));
		    return;
	    }

		// GF mantis #113524, 52001~52090 번 탈것
		if( (item->GetVnum() > 52000) && (item->GetVnum() < 52091) )	{
			ch->UseItem(TItemPos (INVENTORY, i));
		    return;
		}
	}
    }


    // 타거나 내릴 수 없을때
    ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 먼저 소환해주세요."));
}

