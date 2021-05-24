#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "shop.h"
#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "log.h"
#include "db.h"
#include "questmanager.h"
#include "mob_manager.h"
#include "locale_service.h"
#ifdef __OFFLINE_PRIVATE_SHOP__
#include "shop_manager.h"
#endif

/* ------------------------------------------------------------------------------------ */
CShop::CShop()
	: m_dwVnum(0), m_dwNPCVnum(0), m_pkPC(NULL)
#ifdef __OFFLINE_PRIVATE_SHOP__
, m_bIsOfflinePrivateShop(false)
#endif
{
	memset(&m_pkGrid, 0, sizeof(m_pkGrid));
	m_itemVector.resize(SHOP_MAX_NUM);
	memset(&m_itemVector[0], 0, sizeof(SHOP_ITEM) * m_itemVector.size());
}

CShop::~CShop()
{
	TPacketGCShop pack;

	pack.header		= HEADER_GC_SHOP;
	pack.subheader	= SHOP_SUBHEADER_GC_END;
	pack.size		= sizeof(TPacketGCShop);

	Broadcast(&pack, sizeof(pack));

	GuestMapType::iterator it;

	it = m_map_guest.begin();

	while (it != m_map_guest.end())
	{
		LPCHARACTER ch = it->first;
		ch->SetShop(NULL);
		++it;
	}
}

void CShop::SetPCShop(LPCHARACTER ch)
{
	m_pkPC = ch;
}

bool CShop::Create(DWORD dwVnum, DWORD dwNPCVnum, TShopItemTable * pTable)
{
	/*
	   if (NULL == CMobManager::instance().Get(dwNPCVnum))
	   {
	   sys_err("No such a npc by vnum %d", dwNPCVnum);
	   return false;
	   }
	 */
	sys_log(0, "SHOP #%d (Shopkeeper %d)", dwVnum, dwNPCVnum);

	m_dwVnum = dwVnum;
	m_dwNPCVnum = dwNPCVnum;

	BYTE bItemCount;

	for (bItemCount = 0; bItemCount < SHOP_MAX_NUM; ++bItemCount)
		if (0 == (pTable + bItemCount)->vnum)
			break;

	SetShopItems(pTable, bItemCount);
	return true;
}

void CShop::SetShopItems(TShopItemTable * pTable, BYTE bItemCount
#ifdef __OFFLINE_PRIVATE_SHOP__
, DWORD dwShopVid
#endif
)
{
	if (bItemCount > SHOP_MAX_NUM)
		return;

	memset(&m_pkGrid, 0, sizeof(m_pkGrid));

	m_itemVector.resize(SHOP_MAX_NUM);
	memset(&m_itemVector[0], 0, sizeof(SHOP_ITEM) * m_itemVector.size());

	for (int i = 0; i < bItemCount; ++i)
	{
		LPITEM pkItem = NULL;
		const TItemTable * item_table;

		if (m_pkPC)
		{
			pkItem = m_pkPC->GetItem(pTable->pos);

			if (!pkItem)
			{
				sys_err("cannot find item on pos (%d, %d) (name: %s)", pTable->pos.window_type, pTable->pos.cell, m_pkPC->GetName());
				continue;
			}

			item_table = pkItem->GetProto();
		}
		else
		{
			if (!pTable->vnum)
				continue;

			item_table = ITEM_MANAGER::instance().GetTable(pTable->vnum);
		}

		if (!item_table)
		{
			sys_err("Shop: no item table by item vnum #%d", pTable->vnum);
			continue;
		}

		int iPos;

		if (IsPCShop())
		{
			sys_log(0, "MyShop: use position %d", pTable->display_pos);
			iPos = pTable->display_pos;
		}
		else
			iPos = GetEmptyShop(item_table->bSize);

		if (iPos < 0)
		{
			sys_err("not enough shop window");
			continue;
		}

		if (!IsEmpty(iPos, item_table->bSize))
		{
			if (IsPCShop())
			{
				sys_err("not empty position for pc shop %s[%d]", m_pkPC->GetName(), m_pkPC->GetPlayerID());
			}
			else
			{
				sys_err("not empty position for npc shop");
			}
			continue;
		}

		for (int i = 0; i < item_table->bSize; ++i)
		{
			int p = iPos + (i * SHOP_PAGE_COLUMN);

			if (!IsValidPosition(p))
				continue;

			m_pkGrid[p] = iPos + 1;
		}

		SHOP_ITEM & item = m_itemVector[iPos];

		item.pkItem = pkItem;
		item.itemid = 0;

		if (item.pkItem)
		{
			item.item.vnum = pkItem->GetVnum();
			item.item.count = pkItem->GetCount(); // PC 샵의 경우 아이템 개수는 진짜 아이템의 개수여야 한다.
			memcpy(item.item.dwPrices, pTable->dwPrices, sizeof(item.item.dwPrices));
			item.itemid	= pkItem->GetID();
#ifdef __CHANGELOOK__
			item.item.transmutation = pkItem->GetTransmutation();
#endif
			memcpy(item.item.alSockets, pkItem->GetSockets(), sizeof(item.item.alSockets));
			memcpy(item.item.aAttr, pkItem->GetAttributes(), sizeof(item.item.aAttr));
		}
		else
		{
			item.item.vnum = pTable->vnum;
			item.item.count = pTable->count;
			memcpy(item.item.dwPrices, pTable->dwPrices, sizeof(item.item.dwPrices));
		}

#ifdef __OFFLINE_PRIVATE_SHOP__
		if (m_bIsOfflinePrivateShop && pkItem)
		{
			pkItem->AddOfflinePrivateShop(iPos, dwShopVid);
			pkItem->SetOfflinePrivateShopPrice(pTable->dwPrices);
		}
#endif

		char name[36];
		snprintf(name, sizeof(name), "%-20s(#%-5d) (x %d)", item_table->szName, (int) item.item.vnum, item.item.count);

		sys_log(0, "SHOP_ITEM: %-36s PRICE %-5d", name, item.item.dwPrices[SHOP_PRICE_GOLD]);
		++pTable;
	}
}

#ifdef __OFFLINE_PRIVATE_SHOP__
void CShop::SetOfflinePrivateShopItem(LPITEM pkItem)
{
	if (!pkItem)
		sys_err("SetOfflinePrivateShopItem Not Found Item !");

	int iPos = GetEmptyShop(pkItem->GetSize());

	if (iPos < 0)
	{
		sys_err("not enough shop window");
		return;
	}

	if (!IsEmpty(iPos, pkItem->GetSize()))
	{
		sys_err("not empty position for npc shop");
		return;
	}

	for (int i = 0; i < pkItem->GetSize(); ++i)
	{
		int p = iPos + (i * SHOP_PAGE_COLUMN);

		if (!IsValidPosition(p))
			continue;

		m_pkGrid[p] = iPos + 1;
	}

	SHOP_ITEM & item = m_itemVector[iPos];

	item.pkItem = pkItem;

	item.item.vnum = pkItem->GetVnum();
	item.item.count = pkItem->GetCount();
	memcpy(item.item.dwPrices, pkItem->GetOfflinePrivateShopPrices(), sizeof(item.item.dwPrices));
	item.itemid	= pkItem->GetID();
#ifdef __CHANGELOOK__
	item.item.transmutation = pkItem->GetTransmutation();
#endif
	memcpy(item.item.alSockets, pkItem->GetSockets(), sizeof(item.item.alSockets));
	memcpy(item.item.aAttr, pkItem->GetAttributes(), sizeof(item.item.aAttr));
}

void CShop::SetOfflinePrivateShopInfo(TOfflinePrivateShopInfo * pkTOfflinePrivateShopInfo)
{
	for (DWORD i = 0; i < m_itemVector.size() && i < SHOP_MAX_NUM; ++i)
	{
		const SHOP_ITEM & item = m_itemVector[i];

		pkTOfflinePrivateShopInfo->items[i] = item.item;
#ifdef __SHOP_SELL_INFO__
		pkTOfflinePrivateShopInfo->infos[i] = item.info;
#endif
	}
}
#endif

int CShop::Buy(LPCHARACTER ch, BYTE pos)
{
	if (pos >= m_itemVector.size())
	{
		sys_log(0, "Shop::Buy : invalid position %d : %s", pos, ch->GetName());
		return SHOP_SUBHEADER_GC_INVALID_POS;
	}

	sys_log(0, "Shop::Buy : name %s pos %d", ch->GetName(), pos);

	GuestMapType::iterator it = m_map_guest.find(ch);

	if (it == m_map_guest.end())
		return SHOP_SUBHEADER_GC_END;

	SHOP_ITEM& r_item = m_itemVector[pos];

#ifdef __SHOP_SELL_INFO__
	if (r_item.info.bIsSell)
		return SHOP_SUBHEADER_GC_SOLD_OUT;
#endif

	if (r_item.item.dwPrices[SHOP_PRICE_GOLD] < 0
#ifdef __CHEQUE__
 && r_item.item.dwPrices[SHOP_PRICE_CHEQUE] < 0
#endif
)
	{
		LogManager::instance().HackLog("SHOP_BUY_GOLD_OVERFLOW", ch);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}

	LPITEM pkSelectedItem = ITEM_MANAGER::instance().Find(r_item.itemid);

	if (IsPCShop())
	{
		if (!pkSelectedItem)
		{
			sys_log(0, "Shop::Buy : Critical: This user seems to be a hacker : invalid pcshop item : BuyerPID:%d SellerPID:%d",
					ch->GetPlayerID(),
					m_pkPC->GetPlayerID());

			return SHOP_SUBHEADER_GC_SOLD_OUT; // @fixme132 false to SHOP_SUBHEADER_GC_SOLD_OUT
		}

		if ((pkSelectedItem->GetOwner() != m_pkPC))
		{
			sys_log(0, "Shop::Buy : Critical: This user seems to be a hacker : invalid pcshop item : BuyerPID:%d SellerPID:%d",
					ch->GetPlayerID(),
					m_pkPC->GetPlayerID());

			return SHOP_SUBHEADER_GC_SOLD_OUT; // @fixme132 false to SHOP_SUBHEADER_GC_SOLD_OUT
		}
	}

	DWORD dwPrice = r_item.item.dwPrices[SHOP_PRICE_GOLD];
#ifdef __CHEQUE__
	short sChequePrice = r_item.item.dwPrices[SHOP_PRICE_CHEQUE];
#endif
	if (ch->GetGold() < (int) dwPrice)
	{
		sys_log(1, "Shop::Buy : Not enough money : %s has %d, price %d", ch->GetName(), ch->GetGold(), dwPrice);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}
#ifdef __CHEQUE__
	if (ch->GetCheque() < sChequePrice)
	{
		sys_log(1, "Shop::Buy : Not enough cheque : %s has %d, price %d", ch->GetName(), ch->GetCheque(), sChequePrice);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}
#endif
	LPITEM item;

	if (m_pkPC
#ifdef __OFFLINE_PRIVATE_SHOP__
 || m_bIsOfflinePrivateShop
#endif
) // 피씨가 운영하는 샵은 피씨가 실제 아이템을 가지고있어야 한다.
		item = r_item.pkItem;
	else
		item = ITEM_MANAGER::instance().CreateItem(r_item.item.vnum, r_item.item.count);

#ifdef __12ZI__
	if ((ch->GetQuestFlag("zodiac.Insignia1") == 1 && item->GetVnum() == 33001) ||
		(ch->GetQuestFlag("zodiac.Insignia2") == 1 && item->GetVnum() == 33002) ||
		(ch->GetQuestFlag("zodiac.Insignia3") == 1 && item->GetVnum() == 33003) ||
		(ch->GetQuestFlag("zodiac.Insignia4") == 1 && item->GetVnum() == 33004) ||
		(ch->GetQuestFlag("zodiac.Insignia5") == 1 && item->GetVnum() == 33005) ||
		(ch->GetQuestFlag("zodiac.Insignia6") == 1 && item->GetVnum() == 33006) ||
		(ch->GetQuestFlag("zodiac.Insignia7") == 1 && item->GetVnum() == 33007) ||
		(ch->GetQuestFlag("zodiac.Insignia8") == 1 && item->GetVnum() == 33008) ||
		(ch->GetQuestFlag("zodiac.Insignia9") == 1 && item->GetVnum() == 33009) ||
		(ch->GetQuestFlag("zodiac.Insignia10") == 1 && item->GetVnum() == 33010) ||
		(ch->GetQuestFlag("zodiac.Insignia11") == 1 && item->GetVnum() == 33011) ||
		(ch->GetQuestFlag("zodiac.Insignia12") == 1 && item->GetVnum() == 33012) ||
		(ch->GetQuestFlag("zodiac.Insignia13") == 1 && item->GetVnum() == 33013) ||
		(ch->GetQuestFlag("zodiac.Insignia14") == 1 && item->GetVnum() == 33014) ||
		(ch->GetQuestFlag("zodiac.Insignia15") == 1 && item->GetVnum() == 33015) ||
		(ch->GetQuestFlag("zodiac.Insignia16") == 1 && item->GetVnum() == 33016) ||
		(ch->GetQuestFlag("zodiac.Insignia17") == 1 && item->GetVnum() == 33017) ||
		(ch->GetQuestFlag("zodiac.Insignia18") == 1 && item->GetVnum() == 33018) ||
		(ch->GetQuestFlag("zodiac.Insignia19") == 1 && item->GetVnum() == 33019) ||
		(ch->GetQuestFlag("zodiac.Insignia20") == 1 && item->GetVnum() == 33020) ||
		(ch->GetQuestFlag("zodiac.Insignia21") == 1 && item->GetVnum() == 33021) ||
		(ch->GetQuestFlag("zodiac.Insignia22") == 1 && item->GetVnum() == 33022) ||
		(ch->GetQuestFlag("zodiac.Insignia23") == 1 && item->GetVnum() == 33032) ||
		(ch->GetQuestFlag("zodiac.Insignia24") == 1 && item->GetVnum() == 72329))
	{
		return SHOP_SUBHEADER_GC_ZODIAC_SHOP;
	}

	if (item->GetVnum() == 33001)
		ch->SetQuestFlag("zodiac.Insignia1", 1);

	if (item->GetVnum() == 33002)
		ch->SetQuestFlag("zodiac.Insignia2", 1);

	if (item->GetVnum() == 33003)
		ch->SetQuestFlag("zodiac.Insignia3", 1);

	if (item->GetVnum() == 33004)
		ch->SetQuestFlag("zodiac.Insignia4", 1);

	if (item->GetVnum() == 33005)
		ch->SetQuestFlag("zodiac.Insignia5", 1);

	if (item->GetVnum() == 33006)
		ch->SetQuestFlag("zodiac.Insignia6", 1);

	if (item->GetVnum() == 33007)
		ch->SetQuestFlag("zodiac.Insignia7", 1);

	if (item->GetVnum() == 33008)
		ch->SetQuestFlag("zodiac.Insignia8", 1);

	if (item->GetVnum() == 33009)
		ch->SetQuestFlag("zodiac.Insignia9", 1);

	if (item->GetVnum() == 33010)
		ch->SetQuestFlag("zodiac.Insignia10", 1);

	if (item->GetVnum() == 33011)
		ch->SetQuestFlag("zodiac.Insignia11", 1);

	if (item->GetVnum() == 33012)
		ch->SetQuestFlag("zodiac.Insignia12", 1);

	if (item->GetVnum() == 33013)
		ch->SetQuestFlag("zodiac.Insignia13", 1);

	if (item->GetVnum() == 33014)
		ch->SetQuestFlag("zodiac.Insignia14", 1);

	if (item->GetVnum() == 33015)
		ch->SetQuestFlag("zodiac.Insignia15", 1);

	if (item->GetVnum() == 33016)
		ch->SetQuestFlag("zodiac.Insignia16", 1);

	if (item->GetVnum() == 33017)
		ch->SetQuestFlag("zodiac.Insignia17", 1);

	if (item->GetVnum() == 33018)
		ch->SetQuestFlag("zodiac.Insignia18", 1);

	if (item->GetVnum() == 33019)
		ch->SetQuestFlag("zodiac.Insignia19", 1);

	if (item->GetVnum() == 33020)
		ch->SetQuestFlag("zodiac.Insignia20", 1);

	if (item->GetVnum() == 33021)
		ch->SetQuestFlag("zodiac.Insignia21", 1);

	if (item->GetVnum() == 33022)
		ch->SetQuestFlag("zodiac.Insignia22", 1);

	if (item->GetVnum() == 33032)
		ch->SetQuestFlag("zodiac.Insignia23", 1);

	if (item->GetVnum() == 72329)
		ch->SetQuestFlag("zodiac.Insignia24", 1);

	if (item->GetVnum() == 72329 || item->GetVnum() == 33032 || (item->GetVnum() >= 33001 && item->GetVnum() <= 33022))
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefreshShopItemToolTip %d", item->GetVnum());
#endif

	if (!item)
		return SHOP_SUBHEADER_GC_SOLD_OUT;

	int iEmptyPos;
	if (item->IsDragonSoul())
		iEmptyPos = ch->GetEmptyDragonSoulInventory(item);
#ifdef __NEW_STORAGE__
	else if (item->IsStorageItem())
		iEmptyPos = ch->GetEmptyStorageInventory(item->GetStorageType() - UPGRADE_INVENTORY, item);
#endif
	else
		iEmptyPos = ch->GetEmptyInventory(item->GetSize());

	if (iEmptyPos < 0)
	{
		if (m_pkPC)
		{
			sys_log(1, "Shop::Buy at PC Shop : Inventory full : %s size %d", ch->GetName(), item->GetSize());
			return SHOP_SUBHEADER_GC_INVENTORY_FULL;
		}
		else
		{
			sys_log(1, "Shop::Buy : Inventory full : %s size %d", ch->GetName(), item->GetSize());
			M2_DESTROY_ITEM(item);
			return SHOP_SUBHEADER_GC_INVENTORY_FULL;
		}
	}

	if (dwPrice)
		ch->PointChange(POINT_GOLD, -dwPrice, false);
#ifdef __CHEQUE__
	if (sChequePrice)
		ch->PointChange(POINT_CHEQUE, -sChequePrice, false);
#endif

	//세금 계산
	DWORD dwTax = 0;
	int iVal = 0;

	{
		iVal = quest::CQuestManager::instance().GetEventFlag("personal_shop");

		if (0 < iVal)
		{
			if (iVal > 100)
				iVal = 100;

			dwTax = dwPrice * iVal / 100;
			dwPrice = dwPrice - dwTax;
		}
		else
		{
			iVal = 0;
			dwTax = 0;
		}
	}

#ifdef __OFFLINE_PRIVATE_SHOP__
	if (m_bIsOfflinePrivateShop)
	{
		if (item->IsDragonSoul())
			item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
#ifdef __NEW_STORAGE__
		else if (item->IsStorageItem())
			item->AddToCharacter(ch, TItemPos(UPGRADE_INVENTORY + (item->GetStorageType() - UPGRADE_INVENTORY), iEmptyPos));
#endif
		else
			item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
		item->SetOfflinePrivateShopVid(0);
		ITEM_MANAGER::instance().FlushDelayedSave(item);

#ifdef __SHOP_SELL_INFO__
		memcpy(&r_item.info.szName, ch->GetName(), sizeof(r_item.info.szName));
		r_item.info.bIsSell = true;
		r_item.info.lSellTime = time(NULL);
#endif
		r_item.pkItem = NULL;
		BroadcastUpdateItem(pos);

		CShopManager::instance().AddBankOfflinePrivateShop(this, r_item.item.dwPrices);
	}
	else if (m_pkPC)
#else
	if (m_pkPC)
#endif
	{
		m_pkPC->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 255);

		{
			char buf[512];
			if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
			{
				snprintf(buf, sizeof(buf), "%s FROM: %u TO: %u PRICE: %u", item->GetName(), ch->GetPlayerID(), m_pkPC->GetPlayerID(), dwPrice);
				LogManager::instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), SHOP_BUY, buf);
				LogManager::instance().GoldBarLog(m_pkPC->GetPlayerID(), item->GetID(), SHOP_SELL, buf);
			}
			item->RemoveFromCharacter();
			if (item->IsDragonSoul())
				item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
#ifdef __NEW_STORAGE__
			else if (item->IsStorageItem())
				item->AddToCharacter(ch, TItemPos(UPGRADE_INVENTORY + (item->GetStorageType() - UPGRADE_INVENTORY), iEmptyPos));
#endif
			else
				item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
			ITEM_MANAGER::instance().FlushDelayedSave(item);

			snprintf(buf, sizeof(buf), "%s %u(%s) %u "
#ifdef __CHEQUE__
			"%d "
#endif
			"%u", item->GetName(), m_pkPC->GetPlayerID(), m_pkPC->GetName(), dwPrice,
#ifdef __CHEQUE__
sChequePrice,
#endif
			item->GetCount());
			LogManager::instance().ItemLog(ch, item, "SHOP_BUY", buf);

			snprintf(buf, sizeof(buf), "%s %u(%s) %u "
#ifdef __CHEQUE__
			"%d "
#endif
			"%u", item->GetName(), ch->GetPlayerID(), ch->GetName(), dwPrice,
#ifdef __CHEQUE__
sChequePrice,
#endif
			item->GetCount());
			LogManager::instance().ItemLog(m_pkPC, item, "SHOP_SELL", buf);
		}

#ifdef __SHOP_SELL_INFO__
		memcpy(&r_item.info.szName, ch->GetName(), sizeof(r_item.info.szName));
		r_item.info.bIsSell = true;
		r_item.info.lSellTime = time(NULL);
#endif
		r_item.pkItem = NULL;
		BroadcastUpdateItem(pos);

		if (dwPrice)
			m_pkPC->PointChange(POINT_GOLD, dwPrice, false);
#ifdef __CHEQUE__
		if (sChequePrice)
			m_pkPC->PointChange(POINT_CHEQUE, sChequePrice, false);
#endif
		if (iVal > 0)
			m_pkPC->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("판매금액의 %d %% 가 세금으로 나가게됩니다"), iVal);
	}
	else
	{
		if (item->IsDragonSoul())
			item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
#ifdef __NEW_STORAGE__
		else if (item->IsStorageItem())
			item->AddToCharacter(ch, TItemPos(UPGRADE_INVENTORY + (item->GetStorageType() - UPGRADE_INVENTORY), iEmptyPos));
#endif
		else
			item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
		ITEM_MANAGER::instance().FlushDelayedSave(item);
		LogManager::instance().ItemLog(ch, item, "BUY", item->GetName());

		if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
		{
			LogManager::instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), PERSONAL_SHOP_BUY, "");
		}

		LogManager::instance().MoneyLog(MONEY_LOG_SHOP, item->GetVnum(), -dwPrice);
	}

	if (item)
		sys_log(0, "SHOP: BUY: name %s %s(x %d):%u price %u", ch->GetName(), item->GetName(), item->GetCount(), item->GetID(), dwPrice);

    ch->Save();

    return (SHOP_SUBHEADER_GC_OK);
}

bool CShop::AddGuest(LPCHARACTER ch, DWORD owner_vid, bool bOtherEmpire)
{
	if (!ch)
		return false;

	if (ch->GetExchange())
		return false;

	if (ch->GetShop())
		return false;

	ch->SetShop(this);

	m_map_guest.insert(GuestMapType::value_type(ch, bOtherEmpire));

	TPacketGCShop pack;

	pack.header		= HEADER_GC_SHOP;
	pack.subheader	= SHOP_SUBHEADER_GC_START;

	TPacketGCShopStart pack2;

	memset(&pack2, 0, sizeof(pack2));
	pack2.owner_vid = owner_vid;

	for (DWORD i = 0; i < m_itemVector.size() && i < SHOP_MAX_NUM; ++i)
	{
		const SHOP_ITEM & item = m_itemVector[i];

		pack2.items[i] = item.item;
#ifdef __SHOP_SELL_INFO__
		pack2.infos[i] = item.info;
#endif
	}

	pack.size = sizeof(pack) + sizeof(pack2);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
	ch->GetDesc()->Packet(&pack2, sizeof(pack2));
	return true;
}

void CShop::RemoveGuest(LPCHARACTER ch)
{
	if (ch->GetShop() != this)
		return;

	m_map_guest.erase(ch);
	ch->SetShop(NULL);

	TPacketGCShop pack;

	pack.header		= HEADER_GC_SHOP;
	pack.subheader	= SHOP_SUBHEADER_GC_END;
	pack.size		= sizeof(TPacketGCShop);

	ch->GetDesc()->Packet(&pack, sizeof(pack));
}

void CShop::Broadcast(const void * data, int bytes)
{
	sys_log(1, "Shop::Broadcast %p %d", data, bytes);

	GuestMapType::iterator it;

	it = m_map_guest.begin();

	while (it != m_map_guest.end())
	{
		LPCHARACTER ch = it->first;

		if (ch->GetDesc())
			ch->GetDesc()->Packet(data, bytes);

		++it;
	}
}

void CShop::BroadcastUpdateItem(BYTE pos)
{
	TPacketGCShop pack;
	TPacketGCShopUpdateItem pack2;

	TEMP_BUFFER	buf;

	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_UPDATE_ITEM;
	pack.size = sizeof(pack) + sizeof(pack2);

	pack2.pos = pos;

	pack2.item = m_itemVector[pos].item;
#ifdef __SHOP_SELL_INFO__
	pack2.info = m_itemVector[pos].info;
#endif

	buf.write(&pack, sizeof(pack));
	buf.write(&pack2, sizeof(pack2));

	Broadcast(buf.read_peek(), buf.size());
}

int CShop::GetNumberByVnum(DWORD dwVnum)
{
	int itemNumber = 0;

	for (DWORD i = 0; i < m_itemVector.size() && i < SHOP_MAX_NUM; ++i)
	{
		const SHOP_ITEM & item = m_itemVector[i];

		if (item.item.vnum == dwVnum)
		{
			itemNumber += item.item.count;
		}
	}

	return itemNumber;
}

bool CShop::IsSellingItem(DWORD itemID)
{
	bool isSelling = false;

	for (DWORD i = 0; i < m_itemVector.size() && i < SHOP_MAX_NUM; ++i)
	{
		if ((unsigned int)(m_itemVector[i].itemid) == itemID)
		{
			isSelling = true;
			break;
		}
	}

	return isSelling;

}

bool CShop::IsValidPosition(DWORD dwPos)
{
	if (dwPos >= SHOP_MAX_NUM)
		return false;

	return true;
}

int CShop::GetEmptyShop(DWORD dwSize)
{
	for (int i = 0; i < SHOP_MAX_NUM; ++i)
		if (IsEmpty(i, dwSize))
			return i;

	return -1;
}

bool CShop::IsEmpty(WORD wCell, BYTE bSize, int iExceptionCell)
{
	if (!IsValidPosition(wCell))
		return false;

	iExceptionCell++;

	if (m_pkGrid[wCell])
	{
		if (m_pkGrid[wCell] == iExceptionCell)
		{
			if (bSize == 1)
				return true;

			int j = 1;
			BYTE bPage = wCell / SHOP_PAGE_SIZE;

			do
			{
				int p = wCell + (SHOP_PAGE_COLUMN * j);

				if (!IsValidPosition(p))
					return false;

				if (p / SHOP_PAGE_SIZE != bPage)
					return false;

				if (m_pkGrid[p])
					if (m_pkGrid[p] != iExceptionCell)
						return false;
			}
			while (++j < bSize);

			return true;
		}
		else
			return false;
	}

	if (1 == bSize)
		return true;
	else
	{
		int j = 1;
		BYTE bPage = wCell / SHOP_PAGE_SIZE;

		do
		{
			int p = wCell + (SHOP_PAGE_COLUMN * j);

			if (!IsValidPosition(p))
				return false;

			if (p / SHOP_PAGE_SIZE != bPage)
				return false;

			if (m_pkGrid[p])
				if (m_pkGrid[p] != iExceptionCell)
					return false;
		}
		while (++j < bSize);

		return true;
	}

	return false;
}

