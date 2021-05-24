#include "stdafx.h"
#include "utils.h"
#include "desc.h"
#include "desc_client.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "packet.h"
#include "log.h"
#include "db.h"
#include "locale_service.h"
#include "../../common/length.h"
#include "exchange.h"
#include "dragon_soul.h"
#include "questmanager.h"
#include "messenger_manager.h"

void exchange_packet(LPCHARACTER ch, BYTE sub_header, bool is_me, DWORD arg1, TItemPos arg2, DWORD arg3, void * pvData)
{
	if (!ch->GetDesc())
		return;

	struct packet_exchange pack_exchg;

	pack_exchg.header 		= HEADER_GC_EXCHANGE;
	pack_exchg.sub_header 	= sub_header;
	pack_exchg.is_me		= is_me;
	pack_exchg.arg1		= arg1;
	pack_exchg.arg2		= arg2;
	pack_exchg.arg3		= arg3;

	if (sub_header == EXCHANGE_SUBHEADER_GC_ITEM_ADD && pvData)
	{
		memcpy(&pack_exchg.alSockets, ((LPITEM) pvData)->GetSockets(), sizeof(pack_exchg.alSockets));
		memcpy(&pack_exchg.aAttr, ((LPITEM) pvData)->GetAttributes(), sizeof(pack_exchg.aAttr));
#ifdef __CHANGELOOK__
		pack_exchg.dwTransmutation = ((LPITEM) pvData)->GetTransmutation();
#endif
	}
	else
	{
		memset(&pack_exchg.alSockets, 0, sizeof(pack_exchg.alSockets));
		memset(&pack_exchg.aAttr, 0, sizeof(pack_exchg.aAttr));
#ifdef __CHANGELOOK__
		pack_exchg.dwTransmutation = 0;
#endif
	}

	ch->GetDesc()->Packet(&pack_exchg, sizeof(pack_exchg));
}

// 교환을 시작
bool CHARACTER::ExchangeStart(LPCHARACTER victim)
{
	if (this == victim || !victim)
		return false;

	if (IsObserverMode())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("관전 상태에서는 교환을 할 수 없습니다."));
		return false;
	}

#ifdef __MESSENGER_BLOCK__
	if (MessengerManager::instance().CheckMessengerList(this, victim->GetName(), SYST_BLOCK))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s blokkk"), victim->GetName());
		return false;
	}
#endif

	if (victim->IsNPC())
		return false;

	if ( IsOpenSafebox() || GetShopOwner() || GetMyShop() || IsCubeOpen())
	{
		ChatPacket( CHAT_TYPE_INFO, LC_TEXT("다른 거래창이 열려있을경우 거래를 할수 없습니다." ) );
		return false;
	}

#ifdef __ATTR_TRANSFER__
	if (IsAttrTransferOpen())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래창이 열려있을경우 거래를 할수 없습니다."));
		return false;
	}
	
	if (victim->IsAttrTransferOpen())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래창이 열려있을경우 거래를 할수 없습니다."));
		return false;
	}
#endif

	if ( victim->IsOpenSafebox() || victim->GetShopOwner() || victim->GetMyShop() || victim->IsCubeOpen() )
	{
		ChatPacket( CHAT_TYPE_INFO, LC_TEXT("상대방이 다른 거래중이라 거래를 할수 없습니다." ) );
		return false;
	}
	//END_PREVENT_TRADE_WINDOW
#ifndef __REMOTE_EXCHANGE__
	int iDist = DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY());

	// 거리 체크
	if (iDist >= EXCHANGE_MAX_DISTANCE)
		return false;
#endif

	if (GetExchange())
		return false;

	if (victim->GetExchange())
	{
		exchange_packet(this, EXCHANGE_SUBHEADER_GC_ALREADY, 0, 0, NPOS, 0);
		return false;
	}

	if (victim->IsBlockMode(BLOCK_EXCHANGE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 교환 거부 상태입니다."));
		return false;
	}

	SetExchange(M2_NEW CExchange(this));
	victim->SetExchange(M2_NEW CExchange(victim));

	victim->GetExchange()->SetCompany(GetExchange());
	GetExchange()->SetCompany(victim->GetExchange());

	//
	SetExchangeTime();
	victim->SetExchangeTime();

	exchange_packet(victim, EXCHANGE_SUBHEADER_GC_START, 0, GetVID(), NPOS, 0);
	exchange_packet(this, EXCHANGE_SUBHEADER_GC_START, 0, victim->GetVID(), NPOS, 0);

	return true;
}

CExchange::CExchange(LPCHARACTER pOwner)
{
	m_pCompany = NULL;

	m_bAccept = false;

	memset(m_apItems, NULL, sizeof(m_apItems));
	memset(m_abItemDisplayPos, 0, sizeof(m_abItemDisplayPos));

	for (int i = 0; i < EXCHANGE_MAX_NUM; ++i)
		m_aItemPos[i] = NPOS;

	m_lGold = 0;
#ifdef __CHEQUE__
	m_sCheque = 0;
#endif
	m_pOwner = pOwner;
	pOwner->SetExchange(this);
}

CExchange::~CExchange()
{

}

#ifdef __CHEQUE__
bool CExchange::AddCheque(short sCheque)
{
	if (sCheque <= 0)
		return false;

	if (GetOwner()->GetCheque() < sCheque)
	{
		exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_LESS_CHEQUE, 0, 0, NPOS, 0);
		return false;
	}

	if (m_sCheque > 0)
		return false;

	Accept(false);
	GetCompany()->Accept(false);

	m_sCheque = sCheque;

	exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_CHEQUE_ADD, true, m_sCheque, NPOS, 0);
	exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_CHEQUE_ADD, false, m_sCheque, NPOS, 0);

	return true;
}
#endif

bool CExchange::AddItem(TItemPos item_pos, BYTE display_pos)
{
	assert(m_pOwner != NULL && GetCompany());

	if (!item_pos.IsValidItemPosition())
		return false;

	// 장비는 교환할 수 없음
	if (item_pos.IsEquipPosition())
		return false;

	LPITEM item;

	if (!(item = m_pOwner->GetItem(item_pos)))
		return false;

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE))
	{
		m_pOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아이템을 건네줄 수 없습니다."));
		return false;
	}

	if (item->isLocked())
		return false;

#ifdef __SOULBIND__ 
	if (item->IsSealed())
	{
		m_pOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't trade with sealed item."));
		return false;
	}
#endif

	if (item->IsBasicItem())
		return false;

	if (item->IsExchanging())
	{
		sys_log(0, "EXCHANGE under exchanging");
		return false;
	}

	if (display_pos == 0xff)
		display_pos = GetEmptyExchange(item->GetSize());

	if (!IsEmpty(display_pos, item->GetSize()))
	{
		sys_log(0, "EXCHANGE not empty item_pos %d %d", display_pos, item->GetSize());
		return false;
	}

	Accept(false);
	GetCompany()->Accept(false);

	for (int j = 0; j < item->GetSize(); ++j)
	{
		int p = display_pos + (j * EXCHANGE_PAGE_COLUMN);

		if (!IsValidPosition(p))
			continue;

		m_abItemDisplayPos[p] = display_pos + 1;
	}

	m_aItemPos[display_pos] = item_pos;
	m_apItems[display_pos] = item;
	item->SetExchanging(true);

	exchange_packet(m_pOwner,
			EXCHANGE_SUBHEADER_GC_ITEM_ADD,
			true,
			item->GetVnum(),
			TItemPos(item->GetSize(), display_pos),
			item->GetCount(),
			item);

	exchange_packet(GetCompany()->GetOwner(),
			EXCHANGE_SUBHEADER_GC_ITEM_ADD,
			false,
			item->GetVnum(),
			TItemPos(item->GetSize(), display_pos),
			item->GetCount(),
			item);

	sys_log(0, "EXCHANGE AddItem success %s pos(%d, %d) %d", item->GetName(), item_pos.window_type, item_pos.cell, display_pos);

	return true;
}

bool CExchange::RemoveItem(BYTE pos)
{
	if (!IsValidPosition(pos))
		return false;

	if (!m_apItems[pos])
		return false;

	TItemPos PosOfInventory = m_aItemPos[pos];

	LPITEM item = m_apItems[pos];

	exchange_packet(GetOwner(),	EXCHANGE_SUBHEADER_GC_ITEM_DEL, true, pos, NPOS, item->GetSize());
	exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_ITEM_DEL, false, pos, PosOfInventory, item->GetSize());

	Accept(false);
	GetCompany()->Accept(false);

	for (int j = 0; j < item->GetSize(); ++j)
	{
		int p = pos + (j * EXCHANGE_PAGE_COLUMN);

		if (!IsValidPosition(p))
			continue;

		m_abItemDisplayPos[p] = 0;
	}

	m_apItems[pos]->SetExchanging(false);
	m_apItems[pos] = NULL;
	m_aItemPos[pos] = NPOS;

	return true;
}

bool CExchange::AddGold(long gold)
{
	if (gold <= 0)
		return false;

	if (GetOwner()->GetGold() < gold)
	{
		// 가지고 있는 돈이 부족.
		exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_LESS_GOLD, 0, 0, NPOS, 0);
		return false;
	}

	if (m_lGold > 0)
		return false;

	Accept(false);
	GetCompany()->Accept(false);

	m_lGold = gold;

	exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_GOLD_ADD, true, m_lGold, NPOS, 0);
	exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_GOLD_ADD, false, m_lGold, NPOS, 0);

	return true;
}

bool CExchange::Check(int * piItemCount)
{
	if (GetOwner()->GetGold() < m_lGold)
		return false;
#ifdef __CHEQUE__
	if (GetOwner()->GetCheque() < m_sCheque)
		return false;
#endif

	int item_count = 0;

	for (int i = 0; i < EXCHANGE_MAX_NUM; ++i)
	{
		if (!m_apItems[i])
			continue;

		if (!m_aItemPos[i].IsValidItemPosition())
			return false;

		if (m_apItems[i] != GetOwner()->GetItem(m_aItemPos[i]))
			return false;

		++item_count;
	}

	*piItemCount = item_count;
	return true;
}

bool CExchange::CheckSpace()
{
	LPCHARACTER	victim = GetCompany()->GetOwner();
	LPITEM item = NULL;
	int INVEN_SIZE = victim->GetInventoryMax(); 

	auto pkInvenGrid = std::vector<BYTE>(INVEN_SIZE);
	for (int i = 0; i < INVEN_SIZE; ++i)
	{
		if (!(item = victim->GetInventoryItem(i)))
			continue;

		for (int j = 0; j < item->GetSize(); ++j)
		{
			int p = i + (j * INVENTORY_PAGE_COLUMN);

			if (p >= INVEN_SIZE)
				continue;

			pkInvenGrid[p] = i + 1;
		}
	}

#ifdef __NEW_STORAGE__
	WORD pkStorageGrid[STORAGE_INVENTORY_COUNT][STORAGE_INVENTORY_MAX_NUM];
	memset(pkStorageGrid, 0, sizeof(pkStorageGrid));
	bool bStorageInitialized = false;
#endif

	static std::vector <WORD> s_vDSGrid(DRAGON_SOUL_INVENTORY_MAX_NUM);
	bool bDSInitialized = false;

	for (int x = 0; x < EXCHANGE_MAX_NUM; ++x)
	{
		if (!(item = m_apItems[x]))
			continue;

		if (item->IsDragonSoul())
		{
			if (!victim->DragonSoul_IsQualified())
			{
				return false;
			}

			if (!bDSInitialized)
			{
				bDSInitialized = true;
				victim->CopyDragonSoulItemGrid(s_vDSGrid);
			}

			bool bExistEmptySpace = false;
			WORD wBasePos = DSManager::instance().GetBasePosition(item);
			if (wBasePos >= DRAGON_SOUL_INVENTORY_MAX_NUM)
				return false;

			for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; i++)
			{
				WORD wPos = wBasePos + i;
				if (0 == s_vDSGrid[wPos]) // @fixme159 (wBasePos to wPos)
				{
					bool bEmpty = true;
					for (int j = 1; j < item->GetSize(); j++)
					{
						if (s_vDSGrid[wPos + j * DRAGON_SOUL_BOX_COLUMN_NUM])
						{
							bEmpty = false;
							break;
						}
					}
					if (bEmpty)
					{
						for (int j = 0; j < item->GetSize(); j++)
						{
							s_vDSGrid[wPos + j * DRAGON_SOUL_BOX_COLUMN_NUM] =  wPos + 1;
						}
						bExistEmptySpace = true;
						break;
					}
				}
				if (bExistEmptySpace)
					break;
			}
			if (!bExistEmptySpace)
				return false;
		}
#ifdef __NEW_STORAGE__
		else if (item->IsStorageItem())
		{
			if (!bStorageInitialized)
			{
				bStorageInitialized = true;
				victim->CopyStorageItemGrid(pkStorageGrid);
			}

			BYTE bWin = item->GetStorageType() - UPGRADE_INVENTORY;
			bool bExistEmptySpace = false;
			for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
			{
				if (pkStorageGrid[bWin][i] == 0)
				{
					bool bEmpty = true;
					BYTE bPage = i / STORAGE_INVENTORY_PAGE_SIZE;
					for (int j = 1; j < item->GetSize(); j++)
					{
						int p = i + (STORAGE_INVENTORY_PAGE_COLUMN * j);
						if (pkStorageGrid[bWin][p] || p / STORAGE_INVENTORY_PAGE_SIZE != bPage)
						{
							bEmpty = false;
							break;
						}
					}
					if (bEmpty)
					{
						for (int j = 0; j < item->GetSize(); j++)
							pkStorageGrid[bWin][i + j * STORAGE_INVENTORY_PAGE_COLUMN] =  i + 1;

						bExistEmptySpace = true;
						break;
					}
				}
				if (bExistEmptySpace)
					break;
			}
			if (!bExistEmptySpace)
				return false;
		}
#endif
		else
		{
			bool bExistEmptySpace = false;
			for (int i = 0; i < INVEN_SIZE; ++i)
			{
				if (pkInvenGrid[i] == 0)
				{
					bool bEmpty = true;
					BYTE bPage = i / INVENTORY_PAGE_SIZE;
					for (int j = 1; j < item->GetSize(); j++)
					{
						int p = i + (INVENTORY_PAGE_COLUMN * j);
						if (pkInvenGrid[p] || p / INVENTORY_PAGE_SIZE != bPage)
						{
							bEmpty = false;
							break;
						}
					}
					if (bEmpty)
					{
						for (int j = 0; j < item->GetSize(); j++)
							pkInvenGrid[i + j * INVENTORY_PAGE_COLUMN] =  i + 1;

						bExistEmptySpace = true;
						break;
					}
				}
				if (bExistEmptySpace)
					break;
			}
			if (!bExistEmptySpace)
				return false;
		}
	}

	return true;
}

bool CExchange::Done()
{
	int		empty_pos, i;
	LPITEM	item;
	LPCHARACTER	victim = GetCompany()->GetOwner();

	for (i = 0; i < EXCHANGE_MAX_NUM; ++i)
	{
		if (!(item = m_apItems[i]))
			continue;

		if (item->IsDragonSoul())
			empty_pos = victim->GetEmptyDragonSoulInventory(item);
#ifdef __NEW_STORAGE__
		else if (item->IsStorageItem())
			empty_pos = victim->GetEmptyStorageInventory(item->GetStorageType() - UPGRADE_INVENTORY, item);
#endif
		else
			empty_pos = victim->GetEmptyInventory(item->GetSize());

		if (empty_pos < 0)
		{
			sys_err("Exchange::Done : Cannot find blank position in inventory %s <-> %s item %s",
					m_pOwner->GetName(), victim->GetName(), item->GetName());
			continue;
		}

		assert(empty_pos >= 0);

		m_pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 255);

		item->RemoveFromCharacter();

		if (item->IsDragonSoul())
			item->AddToCharacter(victim, TItemPos(DRAGON_SOUL_INVENTORY, empty_pos));
#ifdef __NEW_STORAGE__
		else if (item->IsStorageItem())
			item->AddToCharacter(victim, TItemPos(UPGRADE_INVENTORY + (item->GetStorageType() - UPGRADE_INVENTORY), empty_pos));
#endif
		else
			item->AddToCharacter(victim, TItemPos(INVENTORY, empty_pos));
		ITEM_MANAGER::instance().FlushDelayedSave(item);

		item->SetExchanging(false);
		{
			char exchange_buf[51];

			snprintf(exchange_buf, sizeof(exchange_buf), "%s %u %u", item->GetName(), GetOwner()->GetPlayerID(), item->GetCount());
			LogManager::instance().ItemLog(victim, item, "EXCHANGE_TAKE", exchange_buf);

			snprintf(exchange_buf, sizeof(exchange_buf), "%s %u %u", item->GetName(), victim->GetPlayerID(), item->GetCount());
			LogManager::instance().ItemLog(GetOwner(), item, "EXCHANGE_GIVE", exchange_buf);

			if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
			{
				LogManager::instance().GoldBarLog(victim->GetPlayerID(), item->GetID(), EXCHANGE_TAKE, "");
				LogManager::instance().GoldBarLog(GetOwner()->GetPlayerID(), item->GetID(), EXCHANGE_GIVE, "");
			}
		}

		m_apItems[i] = NULL;
	}

	if (m_lGold)
	{
		GetOwner()->PointChange(POINT_GOLD, -m_lGold, true);
		victim->PointChange(POINT_GOLD, m_lGold, true);

		if (m_lGold > 1000)
		{
			char exchange_buf[51];
			snprintf(exchange_buf, sizeof(exchange_buf), "%u %s", GetOwner()->GetPlayerID(), GetOwner()->GetName());
			LogManager::instance().CharLog(victim, m_lGold, "EXCHANGE_GOLD_TAKE", exchange_buf);

			snprintf(exchange_buf, sizeof(exchange_buf), "%u %s", victim->GetPlayerID(), victim->GetName());
			LogManager::instance().CharLog(GetOwner(), m_lGold, "EXCHANGE_GOLD_GIVE", exchange_buf);
		}
	}
#ifdef __CHEQUE__
	if (m_sCheque)
	{
		GetOwner()->PointChange(POINT_CHEQUE, -m_sCheque, true);
		victim->PointChange(POINT_CHEQUE, m_sCheque, true);
	}
#endif

	return true;
}

// 교환을 동의
bool CExchange::Accept(bool bAccept)
{
	if (m_bAccept == bAccept)
		return true;

	m_bAccept = bAccept;

	// 둘 다 동의 했으므로 교환 성립
	if (m_bAccept && GetCompany()->m_bAccept)
	{
		int	iItemCount;

		LPCHARACTER victim = GetCompany()->GetOwner();

		//PREVENT_PORTAL_AFTER_EXCHANGE
		GetOwner()->SetExchangeTime();
		victim->SetExchangeTime();
		//END_PREVENT_PORTAL_AFTER_EXCHANGE

		// @fixme150 BEGIN
		if (quest::CQuestManager::instance().GetPCForce(GetOwner()->GetPlayerID())->IsRunning() == true)
		{
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if you're using quests"));
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if the other part using quests"));
			goto EXCHANGE_END;
		}
		else if (quest::CQuestManager::instance().GetPCForce(victim->GetPlayerID())->IsRunning() == true)
		{
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if you're using quests"));
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if the other part using quests"));
			goto EXCHANGE_END;
		}
		// @fixme150 END

		// exchange_check 에서는 교환할 아이템들이 제자리에 있나 확인하고,
		// 엘크도 충분히 있나 확인한다, 두번째 인자로 교환할 아이템 개수
		// 를 리턴한다.
		if (!Check(&iItemCount))
		{
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("돈이 부족하거나 아이템이 제자리에 없습니다."));
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방의 돈이 부족하거나 아이템이 제자리에 없습니다."));
			goto EXCHANGE_END;
		}

		// 리턴 받은 아이템 개수로 상대방의 소지품에 남은 자리가 있나 확인한다.
		if (!CheckSpace())
		{
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방의 소지품에 빈 공간이 없습니다."));
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("소지품에 빈 공간이 없습니다."));
			goto EXCHANGE_END;
		}

		// 상대방도 마찬가지로..
		if (!GetCompany()->Check(&iItemCount))
		{
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("돈이 부족하거나 아이템이 제자리에 없습니다."));
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방의 돈이 부족하거나 아이템이 제자리에 없습니다."));
			goto EXCHANGE_END;
		}

		if (!GetCompany()->CheckSpace())
		{
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방의 소지품에 빈 공간이 없습니다."));
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("소지품에 빈 공간이 없습니다."));
			goto EXCHANGE_END;
		}

		if (db_clientdesc->GetSocket() == INVALID_SOCKET)
		{
			victim->ChatPacket(CHAT_TYPE_INFO, "Unknown error");
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, "Unknown error");
			goto EXCHANGE_END;
		}

		if (Done())
		{

			if (m_lGold
#ifdef __CHEQUE__
 || m_sCheque
#endif
)
				GetOwner()->Save();
			if (GetCompany()->Done())
			{
				if (GetCompany()->m_lGold
#ifdef __CHEQUE__
|| GetCompany()->m_sCheque
#endif
)
					victim->Save();

				// INTERNATIONAL_VERSION
				GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s 님과의 교환이 성사 되었습니다."), victim->GetName());
				victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s 님과의 교환이 성사 되었습니다."), GetOwner()->GetName());
				// END_OF_INTERNATIONAL_VERSION
			}
		}

EXCHANGE_END:
		Cancel();
		return false;
	}
	else
	{
		// 아니면 accept에 대한 패킷을 보내자.
		exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_ACCEPT, true, m_bAccept, NPOS, 0);
		exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_ACCEPT, false, m_bAccept, NPOS, 0);
		return true;
	}
}

// 교환 취소
void CExchange::Cancel()
{
	exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_END, 0, 0, NPOS, 0);
	GetOwner()->SetExchange(NULL);

	for (int i = 0; i < EXCHANGE_MAX_NUM; ++i)
	{
		if (m_apItems[i])
			m_apItems[i]->SetExchanging(false);
	}

	if (GetCompany())
	{
		GetCompany()->SetCompany(NULL);
		GetCompany()->Cancel();
	}

	M2_DELETE(this);
}

int CExchange::GetEmptyExchange(DWORD dwSize)
{
	for (int i = 0; i < EXCHANGE_MAX_NUM; ++i)
		if (IsEmpty(i, dwSize))
			return i;

	return -1;
}

bool CExchange::IsEmpty(WORD wCell, BYTE bSize, int iExceptionCell)
{
	if (!IsValidPosition(wCell))
		return false;

	iExceptionCell++;

	if (m_abItemDisplayPos[wCell])
	{
		if (m_abItemDisplayPos[wCell] == iExceptionCell)
		{
			if (bSize == 1)
				return true;

			int j = 1;
			BYTE bPage = wCell / EXCHANGE_PAGE_SIZE;

			do
			{
				int p = wCell + (EXCHANGE_PAGE_COLUMN * j);

				if (!IsValidPosition(p))
					return false;

				if (p / EXCHANGE_PAGE_SIZE != bPage)
					return false;

				if (m_abItemDisplayPos[p])
					if (m_abItemDisplayPos[p] != iExceptionCell)
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
		BYTE bPage = wCell / EXCHANGE_PAGE_SIZE;

		do
		{
			int p = wCell + (EXCHANGE_PAGE_COLUMN * j);

			if (!IsValidPosition(p))
				return false;

			if (p / EXCHANGE_PAGE_SIZE != bPage)
				return false;

			if (m_abItemDisplayPos[p])
				if (m_abItemDisplayPos[p] != iExceptionCell)
					return false;
		}
		while (++j < bSize);

		return true;
	}

	return false;
}

bool CExchange::IsValidPosition(DWORD dwPos)
{
	if (dwPos >= EXCHANGE_MAX_NUM)
		return false;

	return true;
}

