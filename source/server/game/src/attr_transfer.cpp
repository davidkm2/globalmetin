#define _attr_transfer_cpp_
#include "stdafx.h"
#include "config.h"
#include "constants.h"
#include "utils.h"
#include "log.h"
#include "char.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"
#include <stdlib.h>
#include <sstream>
#define RETURN_IF_ATTR_TRANSFER_IS_NOT_OPENED(ch) if (!(ch)->IsAttrTransferOpen()) return

void AttrTransfer_open(LPCHARACTER ch)
{
	LPCHARACTER	npc;
	if (ch == NULL)
		return;
	
	npc = ch->GetQuestNPC();
	if (npc == NULL)
	{
		sys_log(0, "%s has try to open the Attr Transfer window without talk to the NPC.", ch->GetName());
		return;
	}
	
	if (ch->IsAttrTransferOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Attr Transfer> The Attr Transfer window is already opened."));
		return;
	}
	
	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen() || ch->IsAttrTransferOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Attr Transfer> You have any other window opened, please close and try again."));
		return;
	}
	
	long distance = DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY());
	if (distance >= ATTR_TRANSFER_MAX_DISTANCE)
	{
		sys_log(1, "%s is too far for can open the Attr Transfer Window. (character distance: %d, distance allowed: %d)", ch->GetName(), distance, ATTR_TRANSFER_MAX_DISTANCE);
		return;
	}
	
	AttrTransfer_clean_item(ch);
	ch->SetAttrTransferNpc(npc);
	ch->ChatPacket(CHAT_TYPE_COMMAND, "AttrTransfer open");
}

void AttrTransfer_close(LPCHARACTER ch)
{
	RETURN_IF_ATTR_TRANSFER_IS_NOT_OPENED(ch);
	AttrTransfer_clean_item(ch);
	ch->SetAttrTransferNpc(NULL);
	ch->ChatPacket(CHAT_TYPE_COMMAND, "AttrTransfer close");
}

void AttrTransfer_clean_item(LPCHARACTER ch)
{
	LPITEM	*attr_transfer_item;
	attr_transfer_item = ch->GetAttrTransferItem();
	for (int i = 0; i < MAX_ATTR_TRANSFER_SLOT; ++i)
	{
		if (attr_transfer_item[i] == NULL)
			continue;
		
		attr_transfer_item[i] = NULL;
	}
}

bool AttrTransfer_make(LPCHARACTER ch)
{
	LPCHARACTER	npc;
	LPITEM	*items;
	int	has_attr = 0;
	
	if (ch == NULL)
		return false;
	
	if (!(ch)->IsAttrTransferOpen())
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Attr Transfer> The window is not open."));
		return false;
	}
	
	npc = ch->GetQuestNPC();
	if (npc == NULL)
	{
		sys_log(0, "%s has try to open the transfer the bonuses between costumes without talk to the NPC.", ch->GetName());
		return false;
	}
	
	items = ch->GetAttrTransferItem();
	if (items[0] == NULL || items[1] == NULL || items[2] == NULL)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Drag the items into the window."));
		return false;
	}
	
	if (items[0]->GetType() != ITEM_MEDIUM || items[1]->GetType() != ITEM_COSTUME || items[2]->GetType() != ITEM_COSTUME)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Drag the items into the window."));
		return false;
	}
	
	for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
	{
		if (has_attr != 1 && items[2]->GetAttributeType(i) > 0 && items[2]->GetAttributeValue(i) > 0);
		{
			has_attr = 1;
		}
	}
	
	if (has_attr != 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The costume 2 must have at least 1 bonus."));
		return false;
	}
	
	for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		items[1]->SetForceAttribute(i, items[2]->GetAttributeType(i), items[2]->GetAttributeValue(i));
	
	items[0]->SetCount(items[0]->GetCount() - 1);
	items[0] = NULL;
	items[2]->SetCount(items[2]->GetCount() - 1);
	items[2] = NULL;
	
	ch->ChatPacket(CHAT_TYPE_COMMAND, "AttrTransfer success");
	LogManager::instance().AttrTransferLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), items[1]->GetVnum());
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The bonuses has transfered to the Costume 1 succesfully!"));
	return true;
}

void AttrTransfer_add_item(LPCHARACTER ch, int w_index, int i_index)
{
	LPITEM	item;
	LPITEM	*attr_transfer_item;
	RETURN_IF_ATTR_TRANSFER_IS_NOT_OPENED(ch);
	
	if (i_index < 0 || INVENTORY_MAX_NUM <= i_index || w_index < 0 || MAX_ATTR_TRANSFER_SLOT <= w_index)
		return;
	
	item = ch->GetInventoryItem(i_index);
	if (item == NULL)
		return;
	
	if (w_index == 0 && item->GetType() != ITEM_MEDIUM)
		return;
	
	if (w_index == 1 && item->GetType() != ITEM_COSTUME || w_index == 2 && item->GetType() != ITEM_COSTUME)
		return;
	
	if (gAttrTransferLimit != 1 && item->GetSubType() != COSTUME_BODY)
		return;
	
	attr_transfer_item = ch->GetAttrTransferItem();
	for (int i = 0; i < MAX_ATTR_TRANSFER_SLOT; ++i)
	{
		if (item == attr_transfer_item[i])
		{
			attr_transfer_item[i] = NULL;
			break;
		}
	}
	
	if (w_index != 0 && attr_transfer_item[0] == NULL)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Attr Transfer> For do this, you must add the Transfer Scroll into their slot."));
		return;
	}
	else if (w_index == 2 && item->GetSubType() != attr_transfer_item[1]->GetSubType())
	{
		return;
	}
	
	if (w_index == 2)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "AttrTransferMessage");
	}
	
	attr_transfer_item[w_index] = item;
	
	return;
}

void AttrTransfer_delete_item(LPCHARACTER ch, int w_index)
{
	LPITEM	item;
	LPITEM	*attr_transfer_item;
	RETURN_IF_ATTR_TRANSFER_IS_NOT_OPENED(ch);
	
	if (w_index < 0 || MAX_ATTR_TRANSFER_SLOT <= w_index)
		return;
	
	attr_transfer_item = ch->GetAttrTransferItem();
	if (attr_transfer_item[w_index] == NULL)
		return;
	
	item = attr_transfer_item[w_index];
	attr_transfer_item[w_index] = NULL;
	
	return;
}
