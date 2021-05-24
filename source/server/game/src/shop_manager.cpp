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
#include "desc_client.h"
#include "shop_manager.h"
#include "group_text_parse_tree.h"
#include "shop_manager.h"
#include <cctype>
#ifdef __OFFLINE_PRIVATE_SHOP__
#include "offline_privateshop.h"
#include "sectree_manager.h"
#endif

CShopManager::CShopManager() :
	m_pGemShopTable(nullptr)
{
}

CShopManager::~CShopManager()
{
	Destroy();
}

#ifdef __GEM__
bool CShopManager::InitializeGemShop(TGemShopTable * table, int size)
{
	std::map<int, TGemShopTable *> map_shop;
	
	if (m_pGemShopTable)
	{
		delete [] (m_pGemShopTable);
		m_pGemShopTable = NULL;
	}
	
	TGemShopTable * shop_table = m_pGemShopTable;
	
	for (int i = 0; i < size; ++i, ++table)
	{
		if (map_shop.end() == map_shop.find(i))
		{
			TItemTable * pProto = ITEM_MANAGER::instance().GetTable(table->dwVnum);
			
			if (!pProto)
			{
				sys_err("No item by vnum : %d", table->dwVnum);
				continue;
			}
			
			shop_table = new TGemShopTable;
			memset(shop_table, 0, sizeof(TGemShopTable));

			shop_table->dwVnum = table->dwVnum;
			shop_table->bCount = table->bCount;
			shop_table->dwPrice = table->dwPrice;
			shop_table->dwRow = table->dwRow;
			
			map_shop[i] = shop_table;
		}
		else
			shop_table = map_shop[i];
	}
	
	m_pGemShopTable = new TGemShopTable[map_shop.size()];
	m_dwGemShopTableSize = map_shop.size();

	std::map<int, TGemShopTable *>::iterator it = map_shop.begin();
	// decltype(map_shop.begin()) it = map_shop.begin();

	int i = 0;

	while (it != map_shop.end())
	{
		memcpy((m_pGemShopTable + i), (it++)->second, sizeof(TGemShopTable));
		++i;
	}

	return true;
}

DWORD CShopManager::GemShopGetRandomId(DWORD dwRow)
{
	std::vector<DWORD> dwItemId;
	
	for (DWORD id = 0; id < m_dwGemShopTableSize; id++)
		if (m_pGemShopTable[id].dwRow == dwRow)
			dwItemId.push_back(id);
	
	DWORD randomNumber = number(0, dwItemId.size() - 1);
	
	return dwItemId[randomNumber];
}

bool CShopManager::GemShopGetInfoById(DWORD itemId, DWORD &dwVnum, BYTE &bCount, DWORD &dwPrice)
{
	if (itemId < m_dwGemShopTableSize)
	{
		dwVnum = m_pGemShopTable[itemId].dwVnum;
		bCount = m_pGemShopTable[itemId].bCount;
		dwPrice = m_pGemShopTable[itemId].dwPrice;
		
		return true;
	}

	return false;
}
#endif

bool CShopManager::Initialize(TShopTable * table, int size)
{
	if (!m_map_pkShop.empty())
		return false;

	int i;

	for (i = 0; i < size; ++i, ++table)
	{
		LPSHOP shop = M2_NEW CShop;

		if (!shop->Create(table->dwVnum, table->dwNPCVnum, table->items))
		{
			M2_DELETE(shop);
			continue;
		}

		m_map_pkShop.insert(TShopMap::value_type(table->dwVnum, shop));
		m_map_pkShopByNPCVnum.insert(TShopMap::value_type(table->dwNPCVnum, shop));
	}

	return true;
}

void CShopManager::Destroy()
{
	TShopMap::iterator it = m_map_pkShop.begin();

	while (it != m_map_pkShop.end())
	{
		M2_DELETE(it->second);
		++it;
	}

	m_map_pkShop.clear();

#ifdef __OFFLINE_PRIVATE_SHOP__
	TOfflinePrivateShopMap::iterator it2;

	for (it2 = m_map_pkPrivateOfflineShop.begin(); it2 != m_map_pkPrivateOfflineShop.end(); ++it2)
	{
		it2->second->Destroy();
		M2_DELETE(it2->second);
	}

	m_map_pkPrivateOfflineShop.clear();
#endif
}

LPSHOP CShopManager::Get(DWORD dwVnum)
{
	TShopMap::const_iterator it = m_map_pkShop.find(dwVnum);

	if (it == m_map_pkShop.end())
		return NULL;

	return (it->second);
}

LPSHOP CShopManager::GetByNPCVnum(DWORD dwVnum)
{
	TShopMap::const_iterator it = m_map_pkShopByNPCVnum.find(dwVnum);

	if (it == m_map_pkShopByNPCVnum.end())
		return NULL;

	return (it->second);
}

/*
 * 인터페이스 함수들
 */

// 상점 거래를 시작
bool CShopManager::StartShopping(LPCHARACTER pkChr, LPCHARACTER pkChrShopKeeper, int iShopVnum)
{
	if (pkChr->GetShopOwner() == pkChrShopKeeper)
		return false;
	// this method is only for NPC
	if (pkChrShopKeeper->IsPC())
		return false;

	//PREVENT_TRADE_WINDOW
	if (pkChr->IsOpenSafebox() || pkChr->GetExchange() || pkChr->GetMyShop() || pkChr->IsCubeOpen())
	{
		pkChr->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래창이 열린상태에서는 상점거래를 할수 가 없습니다."));
		return false;
	}
	//END_PREVENT_TRADE_WINDOW

	long distance = DISTANCE_APPROX(pkChr->GetX() - pkChrShopKeeper->GetX(), pkChr->GetY() - pkChrShopKeeper->GetY());

	if (distance >= SHOP_MAX_DISTANCE)
	{
		sys_log(1, "SHOP: TOO_FAR: %s distance %d", pkChr->GetName(), distance);
		return false;
	}

	LPSHOP pkShop;

	if (iShopVnum)
		pkShop = Get(iShopVnum);
	else
		pkShop = GetByNPCVnum(pkChrShopKeeper->GetRaceNum());

	if (!pkShop)
	{
		sys_log(1, "SHOP: NO SHOP");
		return false;
	}

	bool bOtherEmpire = false;

	if (pkChr->GetEmpire() != pkChrShopKeeper->GetEmpire())
		bOtherEmpire = true;

	pkShop->AddGuest(pkChr, pkChrShopKeeper->GetVID(), bOtherEmpire);
	pkChr->SetShopOwner(pkChrShopKeeper);
	sys_log(0, "SHOP: START: %s", pkChr->GetName());
	return true;
}

LPSHOP CShopManager::FindPCShop(DWORD dwVID)
{
	TShopMap::iterator it = m_map_pkShopByPC.find(dwVID);

	if (it == m_map_pkShopByPC.end())
		return NULL;

	return it->second;
}

LPSHOP CShopManager::CreatePCShop(LPCHARACTER ch, TShopItemTable * pTable, BYTE bItemCount)
{
	if (FindPCShop(ch->GetVID()))
		return NULL;

	LPSHOP pkShop = M2_NEW CShop;
	pkShop->SetPCShop(ch);
	pkShop->SetShopItems(pTable, bItemCount);

	m_map_pkShopByPC.insert(TShopMap::value_type(ch->GetVID(), pkShop));
	return pkShop;
}

void CShopManager::DestroyPCShop(LPCHARACTER ch)
{
	LPSHOP pkShop = FindPCShop(ch->GetVID());

	if (!pkShop)
		return;

	//PREVENT_ITEM_COPY;
	ch->SetMyShopTime();
	//END_PREVENT_ITEM_COPY

	m_map_pkShopByPC.erase(ch->GetVID());
	M2_DELETE(pkShop);
}

void CShopManager::StopShopping(LPCHARACTER ch)
{
	LPSHOP shop;

#ifdef __OFFLINE_PRIVATE_SHOP__
	LPOPSHOP pkOfflinePrivateShop = FindOfflinePrivateShop(ch->GetOfflinePrivateShopVid());

	if (!(shop = ch->GetShop()) || !pkOfflinePrivateShop || !(shop = pkOfflinePrivateShop->GetShop()))
		return;

	ch->SetOfflinePrivateShopVid(0);
#else
	if (!(shop = ch->GetShop()))
		return;
#endif

	//PREVENT_ITEM_COPY;
	ch->SetMyShopTime();
	//END_PREVENT_ITEM_COPY

	shop->RemoveGuest(ch);
	sys_log(0, "SHOP: END: %s", ch->GetName());
}

// 아이템 구입
void CShopManager::Buy(LPCHARACTER ch, BYTE pos)
{
#ifdef __NEWSTUFF__
	if (0 != g_BuySellTimeLimitValue)
	{
		if (get_dword_time() < ch->GetLastBuySellTime()+g_BuySellTimeLimitValue)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 골드를 버릴 수 없습니다."));
			return;
		}
	}

	ch->SetLastBuySellTime(get_dword_time());
#endif

	if (!ch->GetShop())
		return;

	if (!ch->GetShopOwner()
#ifdef __OFFLINE_PRIVATE_SHOP__
 && !ch->GetOfflinePrivateShopVid()
#endif
)
		return;

#ifdef __OFFLINE_PRIVATE_SHOP__
	if (!ch->GetOfflinePrivateShopVid())
#endif
	if (DISTANCE_APPROX(ch->GetX() - ch->GetShopOwner()->GetX(), ch->GetY() - ch->GetShopOwner()->GetY()) > 2000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상점과의 거리가 너무 멀어 물건을 살 수 없습니다."));
		return;
	}

	CShop* pkShop = ch->GetShop();

	//PREVENT_ITEM_COPY
	ch->SetMyShopTime();
	//END_PREVENT_ITEM_COPY

	int ret = pkShop->Buy(ch, pos);

	if (SHOP_SUBHEADER_GC_OK != ret) // 문제가 있었으면 보낸다.
	{
		TPacketGCShop pack;

		pack.header	= HEADER_GC_SHOP;
		pack.subheader = ret;
		pack.size = sizeof(TPacketGCShop);

		ch->GetDesc()->Packet(&pack, sizeof(pack));
	}
}

#ifdef __NEW_STORAGE__
void CShopManager::Sell(LPCHARACTER ch, BYTE bCell, BYTE bCount, BYTE bType)
#else
void CShopManager::Sell(LPCHARACTER ch, BYTE bCell, BYTE bCount)
#endif
{
#ifdef __NEWSTUFF__
	if (0 != g_BuySellTimeLimitValue)
	{
		if (get_dword_time() < ch->GetLastBuySellTime()+g_BuySellTimeLimitValue)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 골드를 버릴 수 없습니다."));
			return;
		}
	}

	ch->SetLastBuySellTime(get_dword_time());
#endif
	if (!ch->GetShop())
		return;

	if (!ch->GetShopOwner())
		return;

	if (!ch->CanHandleItem())
		return;

	if (ch->GetShop()->IsPCShop())
		return;

	if (DISTANCE_APPROX(ch->GetX()-ch->GetShopOwner()->GetX(), ch->GetY()-ch->GetShopOwner()->GetY())>2000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상점과의 거리가 너무 멀어 물건을 팔 수 없습니다."));
		return;
	}

#ifdef __NEW_STORAGE__
	LPITEM item = ch->GetItem(TItemPos(bType, bCell));
#else
	LPITEM item = ch->GetInventoryItem(bCell);
#endif

	if (!item)
		return;

	if (item->IsEquipped() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("착용 중인 아이템은 판매할 수 없습니다."));
		return;
	}

	if (true == item->isLocked())
	{
		return;
	}

#ifdef __SOULBIND__
	if (item->IsSealed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't sell sealed item."));
		return;
	}
#endif

	if (item->IsBasicItem())
		return;

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_SELL))
		return;

	int iPrice = 0;

	if (bCount == 0 || bCount > item->GetCount())
		bCount = item->GetCount();

	if (iPrice > item->GetGold())
		iPrice = item->GetGold();

	if (IS_SET(item->GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
		if (iPrice == 0)
			iPrice = bCount;
		else
			iPrice = bCount / iPrice;
	}
	else
		iPrice *= bCount;

	iPrice /= 5;

	//세금 계산
	DWORD dwTax = 0;
	int iVal = 3;

	{
		dwTax = iPrice * iVal/100;
		iPrice -= dwTax;
	}
	const int64_t nTotalMoney = static_cast<int64_t>(ch->GetGold()) + static_cast<int64_t>(iPrice);

	if (GOLD_MAX <= nTotalMoney)
	{
		sys_err("[OVERFLOW_GOLD] id %u name %s gold %u", ch->GetPlayerID(), ch->GetName(), ch->GetGold());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20억냥이 초과하여 물품을 팔수 없습니다."));
		return;
	}

	// 20050802.myevan.상점 판매 로그에 아이템 ID 추가
	sys_log(0, "SHOP: SELL: %s item name: %s(x%d):%u price: %u", ch->GetName(), item->GetName(), bCount, item->GetID(), iPrice);

	if (iVal > 0)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("판매금액의 %d %% 가 세금으로 나가게됩니다"), iVal);

	LogManager::instance().MoneyLog(MONEY_LOG_SHOP, item->GetVnum(), iPrice);

	if (bCount == item->GetCount())
		ITEM_MANAGER::instance().RemoveItem(item, "SELL");
	else
		item->SetCount(item->GetCount() - bCount);

	ch->PointChange(POINT_GOLD, iPrice, false);
}

bool CompareShopItemName(const SShopItemTable& lhs, const SShopItemTable& rhs)
{
	TItemTable* lItem = ITEM_MANAGER::instance().GetTable(lhs.vnum);
	TItemTable* rItem = ITEM_MANAGER::instance().GetTable(rhs.vnum);
	if (lItem && rItem)
		return strcmp(lItem->szLocaleName, rItem->szLocaleName) < 0;
	else
		return true;
}

#ifdef __OFFLINE_PRIVATE_SHOP__
void CShopManager::CreateOfflinePrivateShop(LPCHARACTER ch, const char* szSign, DWORD dwRace, DWORD dwTime, TShopItemTable * pTable, BYTE bItemCount
#ifdef __MYSHOP_DECO__
, TMyshopDeco * Deco
#endif
)
{
	LPSECTREE sectree = SECTREE_MANAGER::instance().Get(ch->GetMapIndex(), ch->GetX(), ch->GetY());

	if (!sectree)
	{
		sys_log(0, "cannot find sectree by %dx%d mapindex %d", ch->GetX(), ch->GetY(), ch->GetMapIndex());
		return;
	}

	LPSHOP pkShop = M2_NEW CShop;
	pkShop->SetOfflinePrivateShop(true);
	pkShop->SetPCShop(ch);

	DWORD dwLocalTime = dwTime + time(NULL);
	LPOPSHOP pkOfflinePrivateShop = M2_NEW COfflinePrivateShop(dwRace, dwLocalTime, pkShop);
	pkOfflinePrivateShop->SetMapIndex(ch->GetMapIndex());
	pkOfflinePrivateShop->SetXYZ(ch->GetXYZ());
	pkOfflinePrivateShop->SetOwnerName(ch->GetName());
	pkOfflinePrivateShop->SetSign(szSign);
	pkOfflinePrivateShop->SetPlayerID(ch->GetPlayerID());
#ifdef __MYSHOP_DECO__
	pkOfflinePrivateShop->SetDeco(Deco->bDeco);
#endif
	sectree->InsertEntity(pkOfflinePrivateShop);
	pkOfflinePrivateShop->UpdateSectree();

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("INSERT INTO player.player_shop SET player_id = %d, race = %d, deco = %d, item_count = %d, name = '%s', player_name = '%s', map_index = %d, x = %d, y = %d, z = %d, date = NOW(), date_close = FROM_UNIXTIME(%d), ip = '%s', channel = %d, status = 1",
		ch->GetPlayerID(), dwRace,
#ifdef __MYSHOP_DECO__
		Deco->bDeco,
#else
		0,
#endif
		bItemCount, szSign, ch->GetName(), ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ(), dwLocalTime, inet_ntoa(ch->GetDesc()->GetAddr().sin_addr), g_bChannel));
	SQLResult * pRes = pMsg->Get();
	DWORD dwShopVid = pRes->uiInsertID;
	pkOfflinePrivateShop->SetShopVid(dwShopVid);
	pkShop->SetShopItems(pTable, bItemCount, dwShopVid);
	m_map_pkPrivateOfflineShop.insert(TOfflinePrivateShopMap::value_type(pkOfflinePrivateShop->GetVid(), pkOfflinePrivateShop));
}

LPOPSHOP CShopManager::FindOfflinePrivateShop(DWORD dwVid)
{
	TOfflinePrivateShopMap::iterator it;

	for (it = m_map_pkPrivateOfflineShop.begin(); it != m_map_pkPrivateOfflineShop.end(); ++it)
		if (it->second->GetVid() == dwVid)
			return it->second;

	return NULL;
}

void CShopManager::DestroyOfflinePrivateShop(DWORD dwVid)
{
	LPOPSHOP pkOfflinePrivateShop = FindOfflinePrivateShop(dwVid);

	if (!pkOfflinePrivateShop)
		return;

	DBManager::instance().DirectQuery("UPDATE player.player_shop SET status = 0 WHERE id = %d and player_id = %d", pkOfflinePrivateShop->GetShopVid(), pkOfflinePrivateShop->GetPlayerID());

	TOfflinePrivateShopMap::iterator it;

	for (it = m_map_pkPrivateOfflineShop.begin(); it != m_map_pkPrivateOfflineShop.end(); ++it)
		if (it->second->GetVid() == dwVid)
			m_map_pkPrivateOfflineShop.erase(it->first);

	pkOfflinePrivateShop->Destroy();
	M2_DELETE(pkOfflinePrivateShop);
}

void CShopManager::ProcessOfflinePrivateShop()
{
	TOfflinePrivateShopMap::iterator it;

	for (it = m_map_pkPrivateOfflineShop.begin(); it != m_map_pkPrivateOfflineShop.end(); ++it)
		if ((DWORD)time(NULL) > it->second->GetTime())
			DestroyOfflinePrivateShop(it->second->GetVid());
}

bool CreateItemTableFromRes(MYSQL_RES * res, std::vector<TPlayerItem> * pVec)
{
	if (!res)
	{
		pVec->clear();
		return true;
	}

	int rows;

	if ((rows = mysql_num_rows(res)) <= 0)
	{
		pVec->clear();
		return true;
	}

	pVec->resize(rows);

	for (int i = 0; i < rows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		TPlayerItem & item = pVec->at(i);

		int cur = 0;

		str_to_number(item.id, row[cur++]);
		str_to_number(item.owner, row[cur++]);
		str_to_number(item.window, row[cur++]);
		str_to_number(item.pos, row[cur++]);
		str_to_number(item.count, row[cur++]);
		str_to_number(item.vnum, row[cur++]);
#ifdef __OFFLINE_PRIVATE_SHOP__
		str_to_number(item.shop_id, row[cur++]);
		if (row[cur])
			memcpy(item.prices, row[cur], sizeof(item.prices));
		else
			memset(&item.prices, 0, sizeof(item.prices));
		cur++;
#endif
#ifdef __CHANGELOOK__
		str_to_number(item.transmutation, row[cur++]);
#endif
		str_to_number(item.alSockets[0], row[cur++]);
		str_to_number(item.alSockets[1], row[cur++]);
		str_to_number(item.alSockets[2], row[cur++]);
		str_to_number(item.alSockets[3], row[cur++]);

		for (int j = 0; j < ITEM_ATTRIBUTE_MAX_NUM; j++)
		{
			str_to_number(item.aAttr[j].bType, row[cur++]);
			str_to_number(item.aAttr[j].sValue, row[cur++]);
		}

#ifdef __SOULBIND__
		str_to_number(item.sealbind, row[cur++]);
#endif
		str_to_number(item.bIsBasic, row[cur++]);
	}

	return true;
}

void CShopManager::BuildOfflinePrivateShop()
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT id, player_id, race, deco, item_count, name, player_name, map_index, x, y, z, UNIX_TIMESTAMP(date_close), prices FROM player.player_shop WHERE status = 1 and channel = %d", g_bChannel));
	SQLResult * pRes = pMsg->Get();
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pRes->pSQLResult)))
	{
		DWORD dwPrices[SHOP_PRICE_MAX_NUM];
		if (row[12])
			memcpy(dwPrices, row[12], sizeof(dwPrices));
		else
			memset(dwPrices, 0, sizeof(dwPrices));

		DWORD dwShopVid, dwPlayerId, dwTime, dwRace;
		dwShopVid = dwPlayerId = dwTime = dwRace = 0;
		str_to_number(dwShopVid, row[0]);
		str_to_number(dwPlayerId, row[1]);
		str_to_number(dwTime, row[11]);
		str_to_number(dwRace, row[2]);
		long map_index, x, y, z;
		map_index = x = y = z = 0;
		str_to_number(map_index, row[7]);
		str_to_number(x, row[8]);
		str_to_number(y, row[9]);
		str_to_number(z, row[10]);
		BYTE bDeco, bItemCount;
		bDeco = bItemCount = 0;
		str_to_number(bDeco, row[3]);
		str_to_number(bItemCount, row[4]);
		LPSECTREE sectree = SECTREE_MANAGER::instance().Get(map_index, x, y);

		if (!sectree)
		{
			sys_log(0, "cannot find sectree by %dx%d mapindex %d", x, y, map_index);
			return;
		}

		LPSHOP pkShop = M2_NEW CShop;
		pkShop->SetOfflinePrivateShop(true);
		char queryStr[1024];
		snprintf(queryStr, sizeof(queryStr),
			"SELECT id,owner_id,window+0,pos,count,vnum,"
#ifdef __OFFLINE_PRIVATE_SHOP__
			"shop_id,prices,"
#endif
#ifdef __CHANGELOOK__
			"transmutation,"
#endif
			"socket0,socket1,socket2,socket3,attrtype0,attrvalue0,attrtype1,attrvalue1,attrtype2,attrvalue2,attrtype3,attrvalue3,attrtype4,attrvalue4,attrtype5,attrvalue5,attrtype6,attrvalue6 "
#ifdef __SOULBIND__
			",sealbind "
#endif
			",basic "
			"FROM player.item WHERE shop_id=%d", dwShopVid);

		std::unique_ptr<SQLMsg> pMsg2(DBManager::instance().DirectQuery(queryStr));
		SQLResult * pRes2 = pMsg2->Get();
		static std::vector<TPlayerItem> s_items;
		CreateItemTableFromRes(pRes2->pSQLResult, &s_items);
		for (size_t i = 0; i < s_items.size(); ++i)
		{
			TPlayerItem & p = s_items.at(i);

			LPITEM item = ITEM_MANAGER::instance().CreateItem(p.vnum, p.count, p.id);
			if (!item)
			{
				sys_err("cannot create item by vnum %u id %u", p.vnum, p.id);
				continue;
			}

			item->SetSkipSave(true);
			item->SetSockets(p.alSockets);
			item->SetAttributes(p.aAttr);
#ifdef __SOULBIND__
			item->SetSealBind(p.sealbind);
#endif
#ifdef __CHANGELOOK__
			item->SetTransmutation(p.transmutation);
#endif
			item->SetBasic(p.bIsBasic);
#ifdef __OFFLINE_PRIVATE_SHOP__
			item->SetOfflinePrivateShopVid(p.shop_id);
			item->SetOfflinePrivateShopPrice(p.prices);
#endif
			item->SetSkipSave(false);
			pkShop->SetOfflinePrivateShopItem(item);
		}

		LPOPSHOP pkOfflinePrivateShop = M2_NEW COfflinePrivateShop(dwRace, dwTime, pkShop);
		pkOfflinePrivateShop->SetMapIndex(map_index);
		pkOfflinePrivateShop->SetXYZ(x, y, z);
		pkOfflinePrivateShop->SetOwnerName(row[6]);
		pkOfflinePrivateShop->SetSign(row[5]);
		pkOfflinePrivateShop->SetPlayerID(dwPlayerId);
#ifdef __MYSHOP_DECO__
		pkOfflinePrivateShop->SetDeco(bDeco);
#endif
		pkOfflinePrivateShop->SetPrices(dwPrices);
		sectree->InsertEntity(pkOfflinePrivateShop);
		pkOfflinePrivateShop->UpdateSectree();
		pkOfflinePrivateShop->SetShopVid(dwShopVid);
		m_map_pkPrivateOfflineShop.insert(TOfflinePrivateShopMap::value_type(pkOfflinePrivateShop->GetVid(), pkOfflinePrivateShop));
	}
}

void CShopManager::StartOfflinePrivateShop(LPCHARACTER ch, DWORD dwVid)
{
	if (!ch || !ch->IsPC())
		return;

	if (ch->IsOpenSafebox() || ch->GetExchange() || ch->GetMyShop() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래창이 열린상태에서는 상점거래를 할수 가 없습니다."));
		return;
	}

	LPOPSHOP pkOfflinePrivateShop = FindOfflinePrivateShop(dwVid);

	if (!pkOfflinePrivateShop)
		return;

	long distance = DISTANCE_APPROX(pkOfflinePrivateShop->GetX() - ch->GetX(), pkOfflinePrivateShop->GetY() - ch->GetY());

	if (distance >= SHOP_MAX_DISTANCE)
	{
		sys_log(1, "SHOP: TOO_FAR: %s distance %d", ch->GetName(), distance);
		return;
	}

	LPSHOP pkShop = pkOfflinePrivateShop->GetShop();

	if (!pkShop)
		return;

	pkShop->AddGuest(ch, pkOfflinePrivateShop->GetVid(), false);
	ch->SetOfflinePrivateShopVid(pkOfflinePrivateShop->GetVid());
}
#endif

void CShopManager::AddBankOfflinePrivateShop(LPSHOP pkShop, DWORD dwPrices[])
{
	TOfflinePrivateShopMap::iterator it;

	for (it = m_map_pkPrivateOfflineShop.begin(); it != m_map_pkPrivateOfflineShop.end(); ++it)
	{
		if (it->second->GetShop() == pkShop)
		{
			const DWORD * dwTotalPrices = it->second->AddPrices(dwPrices);
			static char szPrices[QUERY_MAX_LEN + 1];
			DBManager::instance().EscapeString((char *)szPrices, sizeof(szPrices), (const char *)dwTotalPrices, sizeof(dwTotalPrices));
			DBManager::instance().DirectQuery("UPDATE player.player_shop SET item_count = item_count - 1, prices = '%s' WHERE id = %d and player_id = %d", szPrices, it->second->GetShopVid(), it->second->GetPlayerID());
		}
	}
}

BYTE CShopManager::GetOfflinePrivateShopCount(DWORD dwVid)
{
	BYTE bCount = 0;
	TOfflinePrivateShopMap::iterator it;

	for (it = m_map_pkPrivateOfflineShop.begin(); it != m_map_pkPrivateOfflineShop.end(); ++it)
		if (it->second->GetPlayerID() == dwVid)
			++bCount;

	return bCount;
}

void CShopManager::SetOfflinePrivateShops(LPOPSHOP pkOfflinePrivateShops[], BYTE bCount, DWORD dwVid)
{
	BYTE bTotalCount = 0;
	TOfflinePrivateShopMap::iterator it;
	for (it = m_map_pkPrivateOfflineShop.begin(); it != m_map_pkPrivateOfflineShop.end(); ++it)
	{
		if (it->second->GetPlayerID() == dwVid && bTotalCount < bCount)
		{
			pkOfflinePrivateShops[bTotalCount] = it->second;
			++bTotalCount;
		}
	}
}

