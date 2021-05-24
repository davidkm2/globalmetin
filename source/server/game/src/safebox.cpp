#include "stdafx.h"
#include "constants.h"
#include "safebox.h"
#include "packet.h"
#include "char.h"
#include "desc_client.h"
#include "item.h"
#include "item_manager.h"
#include "config.h"

CSafebox::CSafebox(LPCHARACTER pkChrOwner, int iSize, DWORD dwGold) : m_pkChrOwner(pkChrOwner), m_lGold(dwGold)
{
	assert(m_pkChrOwner != NULL);
	memset(m_pkItems, 0, sizeof(m_pkItems));
	memset(&m_pkGrid, 0, sizeof(m_pkGrid));

	m_bWindowMode = SAFEBOX;
}

CSafebox::~CSafebox()
{
	__Destroy();
}

void CSafebox::SetWindowMode(BYTE bMode)
{
	m_bWindowMode = bMode;
}

void CSafebox::__Destroy()
{
	for (int i = 0; i < SAFEBOX_MAX_NUM; ++i)
	{
		if (m_pkItems[i])
		{
			m_pkItems[i]->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(m_pkItems[i]);

			M2_DESTROY_ITEM(m_pkItems[i]->RemoveFromCharacter());
			m_pkItems[i] = NULL;
		}
	}
}

bool CSafebox::Add(DWORD dwPos, LPITEM pkItem)
{
	if (!IsValidPosition(dwPos))
	{
		sys_err("SAFEBOX: item on wrong position at %d (size of grid = %d)", dwPos, SAFEBOX_MAX_NUM);
		return false;
	}

	pkItem->SetWindow(m_bWindowMode);
	pkItem->SetCell(m_pkChrOwner, dwPos);
	pkItem->Save();
	ITEM_MANAGER::instance().FlushDelayedSave(pkItem);

	for (int i = 0; i < pkItem->GetSize(); ++i)
	{
		int p = dwPos + (i * SAFEBOX_PAGE_COLUMN);

		if (!IsValidPosition(p))
			continue;

		m_pkGrid[p] = dwPos + 1;
	}
	m_pkItems[dwPos] = pkItem;

	TPacketGCItemSet pack;

	pack.header	= m_bWindowMode == SAFEBOX ? HEADER_GC_SAFEBOX_SET : HEADER_GC_MALL_SET;
	pack.Cell	= TItemPos(m_bWindowMode, dwPos);
	pack.vnum	= pkItem->GetVnum();
	pack.count	= pkItem->GetCount();
#ifdef __CHANGELOOK__
	pack.transmutation = pkItem->GetTransmutation();
#endif
	pack.flags	= pkItem->GetFlag();
	pack.anti_flags	= pkItem->GetAntiFlag();
	memcpy(pack.alSockets, pkItem->GetSockets(), sizeof(pack.alSockets));
	memcpy(pack.aAttr, pkItem->GetAttributes(), sizeof(pack.aAttr));

	m_pkChrOwner->GetDesc()->Packet(&pack, sizeof(pack));
	sys_log(1, "SAFEBOX: ADD %s %s count %d", m_pkChrOwner->GetName(), pkItem->GetName(), pkItem->GetCount());
	return true;
}

LPITEM CSafebox::Get(DWORD dwPos)
{
	if (!IsValidPosition(dwPos))
		return NULL;

	return m_pkItems[dwPos];
}

LPITEM CSafebox::Remove(DWORD dwPos)
{
	LPITEM pkItem = Get(dwPos);

	if (!pkItem)
		return NULL;

	for (int i = 0; i < pkItem->GetSize(); ++i)
	{
		int p = dwPos + (i * SAFEBOX_PAGE_COLUMN);

		if (!IsValidPosition(p))
			continue;

		m_pkGrid[p] = 0;
	}

	pkItem->RemoveFromCharacter();

	m_pkItems[dwPos] = NULL;

	TPacketGCItemDel pack;

	pack.header	= m_bWindowMode == SAFEBOX ? HEADER_GC_SAFEBOX_DEL : HEADER_GC_MALL_DEL;
	pack.pos	= dwPos;

	m_pkChrOwner->GetDesc()->Packet(&pack, sizeof(pack));
	sys_log(1, "SAFEBOX: REMOVE %s %s count %d", m_pkChrOwner->GetName(), pkItem->GetName(), pkItem->GetCount());
	return pkItem;
}

void CSafebox::Save()
{
	TSafeboxTable t;

	memset(&t, 0, sizeof(TSafeboxTable));

	t.dwID = m_pkChrOwner->GetDesc()->GetAccountTable().id;
	t.dwGold = m_lGold;

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_SAVE, 0, &t, sizeof(TSafeboxTable));
	sys_log(1, "SAFEBOX: SAVE %s", m_pkChrOwner->GetName());
}

int CSafebox::GetEmptySafeBox(DWORD dwSize)
{
	for (int i = 0; i < SAFEBOX_MAX_NUM; ++i)
		if (IsEmpty(i, dwSize))
			return i;

	return -1;
}

bool CSafebox::IsEmpty(WORD wCell, BYTE bSize, int iExceptionCell)
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
			BYTE bPage = wCell / SAFEBOX_PAGE_SIZE;

			do
			{
				int p = wCell + (SAFEBOX_PAGE_COLUMN * j);

				if (!IsValidPosition(p))
					return false;

				if (p / SAFEBOX_PAGE_SIZE != bPage)
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
		BYTE bPage = wCell / SAFEBOX_PAGE_SIZE;

		do
		{
			int p = wCell + (SAFEBOX_PAGE_COLUMN * j);

			if (!IsValidPosition(p))
				return false;

			if (p / SAFEBOX_PAGE_SIZE != bPage)
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

LPITEM CSafebox::GetItem(DWORD dwCell)
{
	if (!IsValidPosition(dwCell))
	{
		sys_err("CHARACTER::GetItem: invalid item cell %d", dwCell);
		return NULL;
	}

	return m_pkItems[dwCell];
}

bool CSafebox::MoveItem(DWORD dwCell, DWORD dwDestCell, DWORD count)
{
	LPITEM item;

	if (!IsValidPosition(dwCell) || !IsValidPosition(dwDestCell))
		return false;

	if (!(item = GetItem(dwCell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (item->GetCount() < count)
		return false;

	LPITEM item2;

	if ((item2 = GetItem(dwDestCell)) && item != item2 && item2->IsStackable() && item2->GetVnum() == item->GetVnum()) // 합칠 수 있는 아이템의 경우
	{
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
			if (item2->GetSocket(i) != item->GetSocket(i))
				return false;

		if (count == 0)
			count = item->GetCount();

		count = MIN(g_bItemCountLimit - item2->GetCount(), count);

		if (item->GetCount() >= count)
			Remove(dwCell);

		item->SetCount(item->GetCount() - count);
		item2->SetCount(item2->GetCount() + count);

		sys_log(1, "SAFEBOX: STACK %s %d -> %d %s count %d", m_pkChrOwner->GetName(), dwCell, dwDestCell, item2->GetName(), item2->GetCount());
		return true;
	}

	if (!IsEmpty(dwDestCell, item->GetSize()))
		return false;

	sys_log(1, "SAFEBOX: MOVE %s %d -> %d %s count %d", m_pkChrOwner->GetName(), dwCell, dwDestCell, item->GetName(), item->GetCount());

	Remove(dwCell);
	Add(dwDestCell, item);

	return true;
}

bool CSafebox::IsValidPosition(DWORD dwPos)
{
	if (dwPos >= SAFEBOX_MAX_NUM)
		return false;

	return true;
}

