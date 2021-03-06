#include "stdafx.h"

#include <stack>

#include "utils.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "item_manager.h"
#include "desc.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "packet.h"
#include "affect.h"
#include "skill.h"
#include "start_position.h"
#include "mob_manager.h"
#include "db.h"
#include "log.h"
#include "vector.h"
#include "buffer_manager.h"
#include "questmanager.h"
#include "fishing.h"
#include "party.h"
#include "dungeon.h"
#include "refine.h"
#include "unique_item.h"
#include "war_map.h"
#include "xmas_event.h"
#include "marriage.h"
#include "blend_item.h"
#include "arena.h"
#include "safebox.h"
#include "shop.h"
#include "pvp.h"
#include "meleylair.h"
#include "new_petsystem.h"
#include "mountsystem.h"
#include "battle_field.h"
#include "petsystem.h"
#include "../../common/VnumHelper.h"
#include "dragon_soul.h"
#include "buff_on_attributes.h"
#include "belt_inventory_helper.h"
#include "../../common/defines.h"

#define POLYMORPH_SKILL_ID	129

const int ITEM_BROKEN_METIN_VNUM = 28960;

// CHANGE_ITEM_ATTRIBUTES
// const DWORD CHARACTER::msc_dwDefaultChangeItemAttrCycle = 10;
const char CHARACTER::msc_szLastChangeItemAttrFlag[] = "Item.LastChangeItemAttr";
// const char CHARACTER::msc_szChangeItemAttrCycleFlag[] = "change_itemattr_cycle";
// END_OF_CHANGE_ITEM_ATTRIBUTES
const BYTE g_aBuffOnAttrPoints[] = { POINT_ENERGY, POINT_COSTUME_ATTR_BONUS };

struct FFindStone
{
	std::map<DWORD, LPCHARACTER> m_mapStone;

	void operator()(LPENTITY pEnt)
	{
		if (pEnt->IsType(ENTITY_CHARACTER) == true)
		{
			LPCHARACTER pChar = (LPCHARACTER)pEnt;

			if (pChar->IsStone() == true)
			{
				m_mapStone[(DWORD)pChar->GetVID()] = pChar;
			}
		}
	}
};


static bool IS_SUMMON_ITEM(int vnum)
{
	switch (vnum)
	{
		case 22000:
		case 22010:
		case 22011:
		case 22020:
		case ITEM_MARRIAGE_RING:
			return true;
	}

	return false;
}

static bool IS_MONKEY_DUNGEON(int map_index)
{
	switch (map_index)
	{
		case 5:
		case 25:
		case 45:
		case 108:
		case 109:
			return true;;
	}

	return false;
}

bool IS_SUMMONABLE_ZONE(int map_index)
{
	// ????????
	if (IS_MONKEY_DUNGEON(map_index))
		return false;

	switch (map_index)
	{
		case 66 : // ????????
		case 71 : // ???? ???? 2??
		case 72 : // ???? ????
		case 73 : // ???? ???? 2??
		case 193 : // ???? ???? 2-1??
#if 0
		case 184 : // ???? ????(????)
		case 185 : // ???? ???? 2??(????)
		case 186 : // ???? ????(????)
		case 187 : // ???? ???? 2??(????)
		case 188 : // ???? ????(????)
		case 189 : // ???? ???? 2??(????)
#endif
//		case 206 : // ????????
		case 216 : // ????????
		case 217 : // ???? ???? 3??
		case 208 : // ???? ???? (????)

		case 113 : // OX Event ??
			return false;
	}

	// ???? private ?????? ???? ??????
	if (map_index > 10000) return false;

	return true;
}

bool IsOnPolymorphMapBlacklist(DWORD mapIndex)
{
	if (mapIndex > 10000)
		mapIndex = mapIndex / 10000; // Also work with dungeons

//	if (mapIndex == (DWORD) NEMERE_MAP_INDEX) // variable
//		return true;

	switch (mapIndex)
	{
	case 26: // Dojang
	case 345: // Meley
	case 113:
#ifdef ENABLE_NEW_PVPARENA
	case PVP_ARENA_MAP_INDEX: // PvP Arena
#endif
		return true;
	}

	return false;
}

bool IS_BOTARYABLE_ZONE(int nMapIndex)
{
	if (!g_bEnableBootaryCheck) return true;

	switch (nMapIndex)
	{
		case 1 :
		case 3 :
		case 21 :
		case 23 :
		case 41 :
		case 43 :
			return true;
	}

	return false;
}

static bool FN_check_item_socket(LPITEM item)
{
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		if (item->GetSocket(i) != item->GetProto()->alSockets[i])
			return false;
	}

	return true;
}

static void FN_copy_item_socket(LPITEM dest, LPITEM src)
{
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		dest->SetSocket(i, src->GetSocket(i));
	}
}
static bool FN_check_item_sex(LPCHARACTER ch, LPITEM item)
{
	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_MALE))
	{
		if (SEX_MALE==GET_SEX(ch))
			return false;
	}
	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_FEMALE))
	{
		if (SEX_FEMALE==GET_SEX(ch))
			return false;
	}

	return true;
}

bool CHARACTER::CanHandleItem(bool bSkipCheckRefine, bool bSkipObserver)
{
	if (!bSkipObserver)
		if (m_bIsObserver)
			return false;

	if (GetMyShop())
		return false;

	if (!bSkipCheckRefine)
		if (m_bUnderRefine)
			return false;

	if (IsCubeOpen() || NULL != DragonSoul_RefineWindow_GetOpener())
		return false;

#ifdef __ATTR_TRANSFER__
	if (IsAttrTransferOpen())
		return false;
#endif

	if (IsWarping())
		return false;

#ifdef __CHANGELOOK__
	if (m_bChangeLook)
		return false;
#endif

#ifdef __COSTUME_ACCE__
	if ((m_bAcceCombination) || (m_bAcceAbsorption))
		return false;
#endif

	return true;
}

#ifdef __NEW_STORAGE__
LPITEM CHARACTER::GetStorageInventoryItem(BYTE bType, WORD wCell) const
{
	return GetItem(TItemPos(UPGRADE_INVENTORY + bType, wCell));
}
#endif

LPITEM CHARACTER::GetInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(INVENTORY, wCell));
}

LPITEM CHARACTER::GetDragonSoulInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(DRAGON_SOUL_INVENTORY, wCell));
}

LPITEM CHARACTER::GetItem(TItemPos Cell) const
{
	if (!IsValidItemPosition(Cell))
		return NULL;

	WORD wCell = Cell.cell;
	BYTE window_type = Cell.window_type;
	switch (window_type)
	{
	case INVENTORY:
	case EQUIPMENT:
		if (wCell >= INVENTORY_AND_EQUIP_SLOT_MAX)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid item cell %d", wCell);
			return NULL;
		}
		return m_pointsInstant.pItems[wCell];
	case DRAGON_SOUL_INVENTORY:
		if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid DS item cell %d", wCell);
			return NULL;
		}
		return m_pointsInstant.pDSItems[wCell];
#ifdef __NEW_STORAGE__
	case UPGRADE_INVENTORY:
	case BOOK_INVENTORY:
	case STONE_INVENTORY:
	case ATTR_INVENTORY:
	case GIFTBOX_INVENTORY:
		if (wCell >= STORAGE_INVENTORY_MAX_NUM)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid S item cell %d", wCell);
			return NULL;
		}
		return m_pointsInstant.pSItems[window_type - UPGRADE_INVENTORY][wCell];
#endif
	default:
		return NULL;
	}
	return NULL;
}

void CHARACTER::SetItem(TItemPos Cell, LPITEM pItem, bool isHighLight)
{
	WORD wCell = Cell.cell;
	BYTE window_type = Cell.window_type;
	if ((unsigned long)((CItem*)pItem) == 0xff || (unsigned long)((CItem*)pItem) == 0xffffffff)
	{
		sys_err("!!! FATAL ERROR !!! item == 0xff (char: %s cell: %u)", GetName(), wCell);
		core_dump();
		return;
	}

	if (pItem && pItem->GetOwner())
	{
		assert(!"GetOwner exist");
		return;
	}
	// ???? ????????
	switch(window_type)
	{
	case INVENTORY:
	case EQUIPMENT:
		{
			if (wCell >= INVENTORY_AND_EQUIP_SLOT_MAX)
			{
				sys_err("CHARACTER::SetItem: invalid item cell %d", wCell);
				return;
			}

			LPITEM pOld = m_pointsInstant.pItems[wCell];

			if (pOld)
			{
				if (wCell < INVENTORY_MAX_NUM)
				{
					for (int i = 0; i < pOld->GetSize(); ++i)
					{
						int p = wCell + (i * INVENTORY_PAGE_COLUMN);

						if (p >= INVENTORY_MAX_NUM)
							continue;

						if (m_pointsInstant.pItems[p] && m_pointsInstant.pItems[p] != pOld)
							continue;

						m_pointsInstant.bItemGrid[p] = 0;
					}
				}
				else
					m_pointsInstant.bItemGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell < INVENTORY_MAX_NUM)
				{
					for (int i = 0; i < pItem->GetSize(); ++i)
					{
						int p = wCell + (i * INVENTORY_PAGE_COLUMN);

						if (p >= INVENTORY_MAX_NUM)
							continue;

						// wCell + 1 ?? ???? ???? ?????? ?????? ?? ????
						// ???????? ???????????? ????
						m_pointsInstant.bItemGrid[p] = wCell + 1;
					}
				}
				else
					m_pointsInstant.bItemGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pItems[wCell] = pItem;
		}
		break;
	// ?????? ????????
	case DRAGON_SOUL_INVENTORY:
		{
			LPITEM pOld = m_pointsInstant.pDSItems[wCell];

			if (pOld)
			{
				if (wCell < DRAGON_SOUL_INVENTORY_MAX_NUM)
				{
					for (int i = 0; i < pOld->GetSize(); ++i)
					{
						int p = wCell + (i * DRAGON_SOUL_BOX_COLUMN_NUM);

						if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							continue;

						if (m_pointsInstant.pDSItems[p] && m_pointsInstant.pDSItems[p] != pOld)
							continue;

						m_pointsInstant.wDSItemGrid[p] = 0;
					}
				}
				else
					m_pointsInstant.wDSItemGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
				{
					sys_err("CHARACTER::SetItem: invalid DS item cell %d", wCell);
					return;
				}

				if (wCell < DRAGON_SOUL_INVENTORY_MAX_NUM)
				{
					for (int i = 0; i < pItem->GetSize(); ++i)
					{
						int p = wCell + (i * DRAGON_SOUL_BOX_COLUMN_NUM);

						if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							continue;

						// wCell + 1 ?? ???? ???? ?????? ?????? ?? ????
						// ???????? ???????????? ????
						m_pointsInstant.wDSItemGrid[p] = wCell + 1;
					}
				}
				else
					m_pointsInstant.wDSItemGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pDSItems[wCell] = pItem;
		}
		break;
#ifdef __NEW_STORAGE__	
	case UPGRADE_INVENTORY:
	case BOOK_INVENTORY:
	case STONE_INVENTORY:
	case ATTR_INVENTORY:
	case GIFTBOX_INVENTORY:
		{
			LPITEM pOld = m_pointsInstant.pSItems[window_type - UPGRADE_INVENTORY][wCell];

			if (pOld)
			{
				if (wCell < STORAGE_INVENTORY_MAX_NUM)
				{
					for (int i = 0; i < pOld->GetSize(); ++i)
					{
						int p = wCell + (i * STORAGE_INVENTORY_PAGE_COLUMN);

						if (p >= STORAGE_INVENTORY_MAX_NUM)
							continue;

						if (m_pointsInstant.pSItems[window_type - UPGRADE_INVENTORY][p] && m_pointsInstant.pSItems[window_type - UPGRADE_INVENTORY][p] != pOld)
							continue;

						m_pointsInstant.wSItemGrid[window_type - UPGRADE_INVENTORY][p] = 0;
					}
				}
				else
					m_pointsInstant.wSItemGrid[window_type - UPGRADE_INVENTORY][wCell] = 0;
			}

			if (pItem)
			{
				if (wCell >= STORAGE_INVENTORY_MAX_NUM)
				{
					sys_err("CHARACTER::SetItem: invalid SS item cell %d", wCell);
					return;
				}

				if (wCell < STORAGE_INVENTORY_MAX_NUM)
				{
					for (int i = 0; i < pItem->GetSize(); ++i)
					{
						int p = wCell + (i * STORAGE_INVENTORY_PAGE_COLUMN);

						if (p >= STORAGE_INVENTORY_MAX_NUM)
							continue;

						m_pointsInstant.wSItemGrid[window_type - UPGRADE_INVENTORY][p] = wCell + 1;
					}
				}
				else
					m_pointsInstant.wSItemGrid[window_type - UPGRADE_INVENTORY][wCell] = wCell + 1;
			}

			m_pointsInstant.pSItems[window_type - UPGRADE_INVENTORY][wCell] = pItem;
		}
		break;
#endif
	default:
		sys_err ("Invalid Inventory type %d", window_type);
		return;
	}

	if (GetDesc())
	{
		TPacketGCItemSet pack;
		pack.header = HEADER_GC_ITEM_SET;
		pack.Cell = Cell;

		if (pItem)
		{
			pack.count = pItem->GetCount();
			pack.vnum = pItem->GetVnum();
			pack.flags = pItem->GetFlag();
			pack.anti_flags	= pItem->GetAntiFlag();
			pack.highlight = isHighLight ? true : (Cell.window_type == DRAGON_SOUL_INVENTORY);

			memcpy(pack.alSockets, pItem->GetSockets(), sizeof(pack.alSockets));
			memcpy(pack.aAttr, pItem->GetAttributes(), sizeof(pack.aAttr));
#ifdef __SOULBIND__
			pack.sealbind = pItem->GetSealBindTime();
#endif
#ifdef __CHANGELOOK__
			pack.transmutation = pItem->GetTransmutation();
#endif
			pack.bIsBasic = pItem->IsBasicItem();
		}
		else
		{
			pack.count = 0;
			pack.vnum = 0;
			pack.flags = 0;
			pack.anti_flags	= 0;
			pack.highlight = 0;

			memset(pack.alSockets, 0, sizeof(pack.alSockets));
			memset(pack.aAttr, 0, sizeof(pack.aAttr));
		}

		GetDesc()->Packet(&pack, sizeof(TPacketGCItemSet));
	}

	if (pItem)
	{
		pItem->SetCell(this, wCell);
		switch (window_type)
		{
			case INVENTORY:
			case EQUIPMENT:
				if ((wCell < INVENTORY_MAX_NUM) || (BELT_INVENTORY_SLOT_START <= wCell && BELT_INVENTORY_SLOT_END > wCell))
					pItem->SetWindow(INVENTORY);
				else
					pItem->SetWindow(EQUIPMENT);
				break;
			case DRAGON_SOUL_INVENTORY:
				pItem->SetWindow(DRAGON_SOUL_INVENTORY);
				break;
#ifdef __NEW_STORAGE__
			case UPGRADE_INVENTORY:
			case BOOK_INVENTORY:
			case STONE_INVENTORY:
			case ATTR_INVENTORY:
			case GIFTBOX_INVENTORY:
				pItem->SetWindow(UPGRADE_INVENTORY + (window_type - UPGRADE_INVENTORY));
				break;
#endif
		}
	}
}

LPITEM CHARACTER::GetWear(BYTE bCell) const
{
	if (bCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
	{
		sys_err("CHARACTER::GetWear: invalid wear cell %d", bCell);
		return NULL;
	}

	return m_pointsInstant.pItems[INVENTORY_MAX_NUM + bCell];
}

void CHARACTER::SetWear(BYTE bCell, LPITEM item)
{
	if (bCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
	{
		sys_err("CHARACTER::SetItem: invalid item cell %d", bCell);
		return;
	}

	SetItem(TItemPos (INVENTORY, INVENTORY_MAX_NUM + bCell), item);

	if (!item && bCell == WEAR_WEAPON)
	{
		if (IsAffectFlag(AFF_GWIGUM))
			RemoveAffect(SKILL_GWIGEOM);

		if (IsAffectFlag(AFF_GEOMGYEONG))
			RemoveAffect(SKILL_GEOMKYUNG);
	}
}

void CHARACTER::ClearItem()
{
	int		i;
	LPITEM	item;

	for (i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; ++i)
	{
		if ((item = GetInventoryItem(i)))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);

			SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
		}
	}
	for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
#ifdef __NEW_STORAGE__
	for (int j = 0; j < STORAGE_INVENTORY_COUNT; ++j)
	{
		for (i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
		{
			if ((item = GetItem(TItemPos(UPGRADE_INVENTORY + j, i))))
			{
				item->SetSkipSave(true);
				ITEM_MANAGER::instance().FlushDelayedSave(item);

				item->RemoveFromCharacter();
				M2_DESTROY_ITEM(item);
			}
		}
	}
#endif
}

bool CHARACTER::IsEmptyItemGrid(TItemPos Cell, BYTE bSize, int iExceptionCell) const
{
	switch (Cell.window_type)
	{
		case INVENTORY:
			{
				WORD bCell = Cell.cell;

				++iExceptionCell;

				if (Cell.IsBeltInventoryPosition())
				{
					LPITEM beltItem = GetWear(WEAR_BELT);

					if (NULL == beltItem)
						return false;

					if (false == CBeltInventoryHelper::IsAvailableCell(bCell - BELT_INVENTORY_SLOT_START, beltItem->GetValue(0)))
						return false;

					if (m_pointsInstant.bItemGrid[bCell])
					{
						if (m_pointsInstant.bItemGrid[bCell] == iExceptionCell)
							return true;

						return false;
					}

					if (bSize == 1)
						return true;

				}
				else if (bCell >= GetInventoryMax())
					return false;

				if (m_pointsInstant.bItemGrid[bCell])
				{
					if (m_pointsInstant.bItemGrid[bCell] == iExceptionCell)
					{
						if (bSize == 1)
							return true;

						int j = 1;
						BYTE bPage = bCell / INVENTORY_PAGE_SIZE;

						do
						{
							BYTE p = bCell + (INVENTORY_PAGE_COLUMN * j);

							if (p >= GetInventoryMax())
								return false;

							if (p / INVENTORY_PAGE_SIZE != bPage)
								return false;

							if (m_pointsInstant.bItemGrid[p])
								if (m_pointsInstant.bItemGrid[p] != iExceptionCell)
									return false;
						}
						while (++j < bSize);

						return true;
					}
					else
						return false;
				}

				// ?????? 1???? ?????? ???????? ???????? ???? ????
				if (1 == bSize)
					return true;
				else
				{
					int j = 1;
					BYTE bPage = bCell / INVENTORY_PAGE_SIZE;

					do
					{
						BYTE p = bCell + (INVENTORY_PAGE_COLUMN * j);

						if (p >= GetInventoryMax())
							return false;

						if (p / INVENTORY_PAGE_SIZE != bPage)
							return false;

						if (m_pointsInstant.bItemGrid[p])
							if (m_pointsInstant.bItemGrid[p] != iExceptionCell)
								return false;
					}
					while (++j < bSize);

					return true;
				}
			}
			break;
		case DRAGON_SOUL_INVENTORY:
			{
				WORD wCell = Cell.cell;
				if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
					return false;

				// bItemCell?? 0?? false???? ???????? ???? + 1 ???? ????????.
				// ?????? iExceptionCell?? 1?? ???? ????????.
				iExceptionCell++;

				if (m_pointsInstant.wDSItemGrid[wCell])
				{
					if (m_pointsInstant.wDSItemGrid[wCell] == iExceptionCell)
					{
						if (bSize == 1)
							return true;

						int j = 1;

						do
						{
							int p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

							if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
								return false;

							if (m_pointsInstant.wDSItemGrid[p])
								if (m_pointsInstant.wDSItemGrid[p] != iExceptionCell)
									return false;
						}
						while (++j < bSize);

						return true;
					}
					else
						return false;
				}

				// ?????? 1???? ?????? ???????? ???????? ???? ????
				if (1 == bSize)
					return true;
				else
				{
					int j = 1;

					do
					{
						int p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

						if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							return false;

						if (m_pointsInstant.wDSItemGrid[p])
							if (m_pointsInstant.wDSItemGrid[p] != iExceptionCell)
								return false;
					}
					while (++j < bSize);

					return true;
				}
			}
			break;
#ifdef __NEW_STORAGE__
		case UPGRADE_INVENTORY:
		case BOOK_INVENTORY:
		case STONE_INVENTORY:
		case ATTR_INVENTORY:
		case GIFTBOX_INVENTORY:
			{
				WORD wCell = Cell.cell;
				if (wCell >= STORAGE_INVENTORY_MAX_NUM)
					return false;

				iExceptionCell++;

				if (m_pointsInstant.wSItemGrid[Cell.window_type - UPGRADE_INVENTORY][wCell])
				{
					if (m_pointsInstant.wSItemGrid[Cell.window_type - UPGRADE_INVENTORY][wCell] == iExceptionCell)
					{
						if (bSize == 1)
							return true;

						int j = 1;
						BYTE bPage = wCell / STORAGE_INVENTORY_PAGE_SIZE;

						do
						{
							int p = wCell + (STORAGE_INVENTORY_PAGE_COLUMN * j);

							if (p >= STORAGE_INVENTORY_MAX_NUM)
								return false;

							if (p / STORAGE_INVENTORY_PAGE_SIZE != bPage)
								return false;

							if (m_pointsInstant.wSItemGrid[Cell.window_type - UPGRADE_INVENTORY][p])
								if (m_pointsInstant.wSItemGrid[Cell.window_type - UPGRADE_INVENTORY][p] != iExceptionCell)
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
					BYTE bPage = wCell / STORAGE_INVENTORY_PAGE_SIZE;

					do
					{
						int p = wCell + (STORAGE_INVENTORY_PAGE_COLUMN * j);

						if (p >= STORAGE_INVENTORY_MAX_NUM)
							return false;

						if (p / STORAGE_INVENTORY_PAGE_SIZE != bPage)
							return false;

						if (m_pointsInstant.wSItemGrid[Cell.window_type - UPGRADE_INVENTORY][p]) // old bItemGrid
							if (m_pointsInstant.wSItemGrid[Cell.window_type - UPGRADE_INVENTORY][p] != iExceptionCell)
								return false;
					}
					while (++j < bSize);

					return true;
				}
			}
			break;
#endif
	}
	return false;
}

int CHARACTER::GetEmptyInventory(BYTE size) const
{
	for (int i = 0; i < GetInventoryMax(); ++i)
		if (IsEmptyItemGrid(TItemPos(INVENTORY, i), size))
			return i;

	return -1;
}

int CHARACTER::GetEmptyDragonSoulInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsDragonSoul())
		return -1;
	if (!DragonSoul_IsQualified())
	{
		return -1;
	}
	BYTE bSize = pItem->GetSize();
	WORD wBaseCell = DSManager::instance().GetBasePosition(pItem);

	if (WORD_MAX == wBaseCell)
		return -1;

	for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; ++i)
		if (IsEmptyItemGrid(TItemPos(DRAGON_SOUL_INVENTORY, i + wBaseCell), bSize))
			return i + wBaseCell;

	return -1;
}

void CHARACTER::CopyDragonSoulItemGrid(std::vector<WORD>& vDragonSoulItemGrid) const
{
	vDragonSoulItemGrid.resize(DRAGON_SOUL_INVENTORY_MAX_NUM);

	std::copy(m_pointsInstant.wDSItemGrid, m_pointsInstant.wDSItemGrid + DRAGON_SOUL_INVENTORY_MAX_NUM, vDragonSoulItemGrid.begin());
}

#ifdef __NEW_STORAGE__
int CHARACTER::GetEmptyStorageInventory(BYTE bType, LPITEM pItem) const
{
	if (!pItem)
		return -1;
	
	BYTE bSize = pItem->GetSize();
	
	for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
		if (IsEmptyItemGrid(TItemPos(UPGRADE_INVENTORY + bType, i), bSize))
			return i;
		
	return -1;
}
#endif

int CHARACTER::CountEmptyInventory() const
{
	int	count = 0;
	for (int i = 0; i < GetInventoryMax(); ++i)
		if (GetInventoryItem(i))
			count += GetInventoryItem(i)->GetSize();

	return (GetInventoryMax() - count);
}

void TransformRefineItem(LPITEM pkOldItem, LPITEM pkNewItem)
{
	if (pkOldItem->IsAccessoryForSocket())
	{
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			pkNewItem->SetSocket(i, pkOldItem->GetSocket(i));
		}
	}
	else
	{
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			if (!pkOldItem->GetSocket(i))
				break;
			else
				pkNewItem->SetSocket(i, 1);
		}

		int slot = 0;

		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			long socket = pkOldItem->GetSocket(i);

			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				pkNewItem->SetSocket(slot++, socket);
		}

	}

	pkOldItem->CopyAttributeTo(pkNewItem);
}

void NotifyRefineSuccess(LPCHARACTER ch, LPITEM item, const char* way)
{
	if (NULL != ch && item != NULL)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineSuceeded");

		LogManager::instance().RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(), item->GetRefineLevel(), 1, way);
	}
}

void NotifyRefineFail(LPCHARACTER ch, LPITEM item, const char* way, int success = 0)
{
	if (NULL != ch && NULL != item)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineFailed");

		LogManager::instance().RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(), item->GetRefineLevel(), success, way);
	}
}

void CHARACTER::SetRefineNPC(LPCHARACTER ch)
{
	if ( ch != NULL )
	{
		m_dwRefineNPCVID = ch->GetVID();
	}
	else
	{
		m_dwRefineNPCVID = 0;
	}
}

bool CHARACTER::DoRefine(LPITEM item, bool bMoneyOnly)
{
	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	if (quest::CQuestManager::instance().GetEventFlag("update_refine_time") != 0)
	{
		if (get_global_time() < quest::CQuestManager::instance().GetEventFlag("update_refine_time") + (60 * 5))
		{
			sys_log(0, "can't refine %d %s", GetPlayerID(), GetName());
			return false;
		}
	}

	const TRefineTable * prt = CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());

	if (!prt)
		return false;

	DWORD result_vnum = item->GetRefinedVnum();

	int cost = ComputeRefineFee(prt->cost);

	int RefineChance = GetQuestFlag("main_quest_lv7.refine_chance");

	if (RefineChance > 0)
	{
		if (!item->CheckItemUseLevel(20) || item->GetType() != ITEM_WEAPON)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???? ?????? 20 ?????? ?????? ??????????"));
			return false;
		}

		cost = 0;
		SetQuestFlag("main_quest_lv7.refine_chance", RefineChance - 1);
	}

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ?????? ?? ????????."));
		return false;
	}

	if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TUNING)
		return false;

	TItemTable * pProto = ITEM_MANAGER::instance().GetTable(item->GetRefinedVnum());

	if (!pProto)
	{
		sys_err("DoRefine NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?????? ?? ????????."));
		return false;
	}

	if (GetGold() < cost)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ???? ???? ??????????."));
		return false;
	}

	if (!bMoneyOnly && !RefineChance)
	{
		for (int i = 0; i < prt->material_count; ++i)
		{
#ifdef __PENDANT__
			if (CountSpecifyItemTheOutItem(prt->materials[i].vnum, item) < prt->materials[i].count)
#else
			if (CountSpecifyItem(prt->materials[i].vnum) < prt->materials[i].count)
#endif
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ???? ?????? ??????????."));
				return false;
			}
#ifdef __PENDANT__
			LPITEM src = FindSpecifyItemTheOutItem(prt->materials[i].vnum, item);
			if (!src)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ???? ?????? ??????????."));
				return false;
			}
#endif
		}

		for (int i = 0; i < prt->material_count; ++i)
#ifdef __PENDANT__
			RemoveSpecifyItemTheOutItem(prt->materials[i].vnum, prt->materials[i].count, item);
#else
			RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count);
#endif
	}

	int prob = number(1, 100);

	if (IsRefineThroughGuild())
		prob -= 10;	

	// END_OF_REFINE_COST

	if (prob <= prt->prob)	
	{
		// ????! ???? ???????? ????????, ???? ?????? ???? ?????? ????
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
			LogManager::instance().ItemLog(this, pkNewItem, "REFINE SUCCESS", pkNewItem->GetName());

			BYTE bCell = item->GetCell();

			// DETAIL_REFINE_LOG
			NotifyRefineSuccess(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
			LogManager::instance().MoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -cost);
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
			// END_OF_DETAIL_REFINE_LOG

			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, bCell), true);
			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);

			sys_log(0, "Refine Success %d", cost);
			pkNewItem->AttrLog();
			//PointChange(POINT_GOLD, -cost);
			sys_log(0, "PayPee %d", cost);
			PayRefineFee(cost);
			sys_log(0, "PayPee End %d", cost);
		}
		else
		{
			// DETAIL_REFINE_LOG
			// ?????? ?????? ???? -> ???? ?????? ????
			sys_err("cannot create item %u", result_vnum);
			NotifyRefineFail(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
			// END_OF_DETAIL_REFINE_LOG
		}
	}
	else
	{
		// ????! ???? ???????? ??????.
		LogManager::instance().MoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -cost);
		NotifyRefineFail(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
		item->AttrLog();
		ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE FAIL)");

		//PointChange(POINT_GOLD, -cost);
		PayRefineFee(cost);
	}

	return true;
}

enum enum_RefineScrolls
{
	CHUKBOK_SCROLL = 0,
	HYUNIRON_CHN   = 1,
	YONGSIN_SCROLL = 2,
	MUSIN_SCROLL   = 3,
	YAGONG_SCROLL  = 4,
	MEMO_SCROLL	   = 5,
	BDRAGON_SCROLL	= 6,
#ifdef __RITUEL_STONE__
	RITUALS_SCROLL = 7,
#endif
};

bool CHARACTER::DoRefineWithScroll(LPITEM item)
{
	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	ClearRefineMode();

	if (quest::CQuestManager::instance().GetEventFlag("update_refine_time") != 0)
	{
		if (get_global_time() < quest::CQuestManager::instance().GetEventFlag("update_refine_time") + (60 * 5))
		{
			sys_log(0, "can't refine %d %s", GetPlayerID(), GetName());
			return false;
		}
	}

	const TRefineTable * prt = CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());

	if (!prt)
		return false;

	LPITEM pkItemScroll;

	if (m_iRefineAdditionalCell < 0)
		return false;

	pkItemScroll = GetInventoryItem(m_iRefineAdditionalCell);

	if (!pkItemScroll)
		return false;

	if (!(pkItemScroll->GetType() == ITEM_USE && pkItemScroll->GetSubType() == USE_TUNING))
		return false;

	if (pkItemScroll->GetVnum() == item->GetVnum())
		return false;

	DWORD result_vnum = item->GetRefinedVnum();
	DWORD result_fail_vnum = item->GetRefineFromVnum();

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ?????? ?? ????????."));
		return false;
	}

	if (pkItemScroll->GetValue(0) == MUSIN_SCROLL)
	{
		if (item->GetRefineLevel() >= 4)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?? ???? ?????? ?? ????????."));
			return false;
		}
	}
	else if (pkItemScroll->GetValue(0) == MEMO_SCROLL)
	{
		if (item->GetRefineLevel() != pkItemScroll->GetValue(1))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?????? ?? ????????."));
			return false;
		}
	}
	else if (pkItemScroll->GetValue(0) == BDRAGON_SCROLL)
	{
		if (item->GetType() != ITEM_METIN || item->GetRefineLevel() != 4)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ?????????? ?????? ?? ????????."));
			return false;
		}
	}
#ifdef __RITUEL_STONE__
	else if (pkItemScroll->GetValue(0) == RITUALS_SCROLL)
	{
		if (item->GetLevelLimit() < pkItemScroll->GetValue(1))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("you can only upgrade items above level"));
			return false;
		}
	}
#endif

	TItemTable * pProto = ITEM_MANAGER::instance().GetTable(item->GetRefinedVnum());

	if (!pProto)
	{
		sys_err("DoRefineWithScroll NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?????? ?? ????????."));
		return false;
	}

	if (GetGold() < prt->cost)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ???? ???? ??????????."));
		return false;
	}

	for (int i = 0; i < prt->material_count; ++i)
	{
		if (CountSpecifyItem(prt->materials[i].vnum) < prt->materials[i].count)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ???? ?????? ??????????."));
			return false;
		}
	}

	for (int i = 0; i < prt->material_count; ++i)
		RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count);

	int prob = number(1, 100);
	int success_prob = prt->prob;
	bool bDestroyWhenFail = false;

	const char* szRefineType = "SCROLL";

	if (pkItemScroll->GetValue(0) == HYUNIRON_CHN ||
		pkItemScroll->GetValue(0) == YONGSIN_SCROLL ||
		pkItemScroll->GetValue(0) == YAGONG_SCROLL
#ifdef __RITUEL_STONE__
|| pkItemScroll->GetValue(0) == RITUALS_SCROLL
#endif
)
	{
		const char hyuniron_prob[9] = { 100, 75, 65, 55, 45, 40, 35, 25, 20 };
		const char yagong_prob[9] = { 100, 100, 90, 80, 70, 60, 50, 30, 20 };

		if (pkItemScroll->GetValue(0) == YONGSIN_SCROLL)
		{
			success_prob = hyuniron_prob[MINMAX(0, item->GetRefineLevel(), 8)];
		}
		else if (pkItemScroll->GetValue(0) == YAGONG_SCROLL)
		{
			success_prob = yagong_prob[MINMAX(0, item->GetRefineLevel(), 8)];
		}
		else if (pkItemScroll->GetValue(0) == HYUNIRON_CHN
#ifdef __RITUEL_STONE__
|| pkItemScroll->GetValue(0) == RITUALS_SCROLL
#endif
)
		{
			success_prob = yagong_prob[MINMAX(0, item->GetRefineLevel(), 8)];
		} // @fixme121
		else
		{
			sys_err("REFINE : Unknown refine scroll item. Value0: %d", pkItemScroll->GetValue(0));
		}

		if (pkItemScroll->GetValue(0) == HYUNIRON_CHN
#ifdef __RITUEL_STONE__
 || pkItemScroll->GetValue(0) == RITUALS_SCROLL
#endif
)
			bDestroyWhenFail = true;

		// DETAIL_REFINE_LOG
		if (pkItemScroll->GetValue(0) == HYUNIRON_CHN)
		{
			szRefineType = "HYUNIRON";
		}
		else if (pkItemScroll->GetValue(0) == YONGSIN_SCROLL)
		{
			szRefineType = "GOD_SCROLL";
		}
		else if (pkItemScroll->GetValue(0) == YAGONG_SCROLL)
		{
			szRefineType = "YAGONG_SCROLL";
		}
#ifdef __RITUEL_STONE__
		else if (pkItemScroll->GetValue(0) == RITUALS_SCROLL)
		{
			szRefineType = "RITUALS_SCROLL";
		}
#endif
		// END_OF_DETAIL_REFINE_LOG
	}

	// DETAIL_REFINE_LOG
	if (pkItemScroll->GetValue(0) == MUSIN_SCROLL) // ?????? ???????? 100% ???? (+4??????)
	{
		success_prob = 100;

		szRefineType = "MUSIN_SCROLL";
	}
	// END_OF_DETAIL_REFINE_LOG
	else if (pkItemScroll->GetValue(0) == MEMO_SCROLL)
	{
		success_prob = 100;
		szRefineType = "MEMO_SCROLL";
	}
	else if (pkItemScroll->GetValue(0) == BDRAGON_SCROLL)
	{
		success_prob = 80;
		szRefineType = "BDRAGON_SCROLL";
	}

	pkItemScroll->SetCount(pkItemScroll->GetCount() - 1);

	if (prob <= success_prob)
	{
		// ????! ???? ???????? ????????, ???? ?????? ???? ?????? ????
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
			LogManager::instance().ItemLog(this, pkNewItem, "REFINE SUCCESS", pkNewItem->GetName());

			BYTE bCell = item->GetCell();

			NotifyRefineSuccess(this, item, szRefineType);
			LogManager::instance().MoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -prt->cost);
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");

			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, bCell), true);
			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);
			pkNewItem->AttrLog();
			//PointChange(POINT_GOLD, -prt->cost);
			PayRefineFee(prt->cost);
		}
		else
		{
			// ?????? ?????? ???? -> ???? ?????? ????
			sys_err("cannot create item %u", result_vnum);
			NotifyRefineFail(this, item, szRefineType);
		}
	}
	else if (!bDestroyWhenFail && result_fail_vnum)
	{
		// ????! ???? ???????? ????????, ???? ?????? ???? ?????? ?????? ????
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_fail_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
			LogManager::instance().ItemLog(this, pkNewItem, "REFINE FAIL", pkNewItem->GetName());

			BYTE bCell = item->GetCell();

			LogManager::instance().MoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -prt->cost);
			NotifyRefineFail(this, item, szRefineType, -1);
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE FAIL)");

			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, bCell), true);
			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);

			pkNewItem->AttrLog();

			//PointChange(POINT_GOLD, -prt->cost);
			PayRefineFee(prt->cost);
		}
		else
		{
			// ?????? ?????? ???? -> ???? ?????? ????
			sys_err("cannot create item %u", result_fail_vnum);
			NotifyRefineFail(this, item, szRefineType);
		}
	}
	else
	{
		NotifyRefineFail(this, item, szRefineType); // ?????? ?????? ???????? ????

		PayRefineFee(prt->cost);
	}

	return true;
}

bool CHARACTER::RefineInformation(BYTE bCell, BYTE bType, int iAdditionalCell)
{
	if (bCell > GetInventoryMax())
		return false;

	LPITEM item = GetInventoryItem(bCell);

	if (!item)
		return false;

	if (bType == REFINE_TYPE_MONEY_ONLY && !GetQuestFlag("deviltower_zone.can_refine"))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???? ???? ?????? ???????? ??????????????."));
		return false;
	}

	TPacketGCRefineInformation p;

	p.header = HEADER_GC_REFINE_INFORMATION;
	p.pos = bCell;
	p.src_vnum = item->GetVnum();
	p.result_vnum = item->GetRefinedVnum();
	p.type = bType;

	if (p.result_vnum == 0)
	{
		sys_err("RefineInformation p.result_vnum == 0");
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?????? ?? ????????."));
		return false;
	}

	if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TUNING)
	{
		if (bType == 0)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?? ?????????? ?????? ?? ????????."));
			return false;
		}
		else
		{
			LPITEM itemScroll = GetInventoryItem(iAdditionalCell);
			if (!itemScroll || item->GetVnum() == itemScroll->GetVnum())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? ???? ???? ????????."));
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ?????? ???? ?? ????????."));
				return false;
			}
		}
	}

	CRefineManager & rm = CRefineManager::instance();

	const TRefineTable* prt = rm.GetRefineRecipe(item->GetRefineSet());

	if (!prt)
	{
		sys_err("RefineInformation NOT GET REFINE SET %d", item->GetRefineSet());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?????? ?? ????????."));
		return false;
	}

	// REFINE_COST

	//MAIN_QUEST_LV7
	if (GetQuestFlag("main_quest_lv7.refine_chance") > 0)
	{
		// ?????? ????
		if (!item->CheckItemUseLevel(20) || item->GetType() != ITEM_WEAPON)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???? ?????? 20 ?????? ?????? ??????????"));
			return false;
		}
		p.cost = 0;
	}
	else
		p.cost = ComputeRefineFee(prt->cost);

	//END_MAIN_QUEST_LV7
	p.prob = prt->prob;
	if (bType == REFINE_TYPE_MONEY_ONLY)
	{
		p.material_count = 0;
		memset(p.materials, 0, sizeof(p.materials));
	}
	else
	{
		p.material_count = prt->material_count;
		memcpy(&p.materials, prt->materials, sizeof(prt->materials));
	}
	// END_OF_REFINE_COST

	GetDesc()->Packet(&p, sizeof(TPacketGCRefineInformation));

	SetRefineMode(iAdditionalCell);
	return true;
}

bool CHARACTER::RefineItem(LPITEM pkItem, LPITEM pkTarget)
{
	if (!CanHandleItem())
		return false;

	if (pkItem->GetSubType() == USE_TUNING)
	{
		if (pkItem->GetValue(0) == MUSIN_SCROLL)
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_MUSIN, pkItem->GetCell());
		else if (pkItem->GetValue(0) == HYUNIRON_CHN)
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_HYUNIRON, pkItem->GetCell());
		else if (pkItem->GetValue(0) == BDRAGON_SCROLL)
		{
			if (pkTarget->GetRefineSet() != 702)
				return false;
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_BDRAGON, pkItem->GetCell());
		}
#ifdef __RITUEL_STONE__
		else if (pkItem->GetValue(0) == RITUALS_SCROLL)
		{
			if (pkTarget->GetLevelLimit() <= 80)
				return false;

			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_RITUALS_SCROLL, pkItem->GetCell());
		}
#endif
		else
		{
			if (pkTarget->GetRefineSet() == 501) return false;
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_SCROLL, pkItem->GetCell());
		}
	}
	else if (pkItem->GetSubType() == USE_DETACHMENT && IS_SET(pkTarget->GetFlag(), ITEM_FLAG_REFINEABLE))
	{
		LogManager::instance().ItemLog(this, pkTarget, "USE_DETACHMENT", pkTarget->GetName());

		bool bHasMetinStone = false;

		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
		{
			long socket = pkTarget->GetSocket(i);
			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
			{
				bHasMetinStone = true;
				break;
			}
		}

		if (bHasMetinStone)
		{
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
			{
				long socket = pkTarget->GetSocket(i);
				if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				{
					AutoGiveItem(socket);
					//TItemTable* pTable = ITEM_MANAGER::instance().GetTable(pkTarget->GetSocket(i));
					//pkTarget->SetSocket(i, pTable->alValues[2]);
					// ???????? ??????????
					pkTarget->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
				}
			}
			pkItem->SetCount(pkItem->GetCount() - 1);
			return true;
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?? ???? ???????? ????????."));
			return false;
		}
	}

	return false;
}

EVENTFUNC(kill_campfire_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );

	if ( info == NULL )
	{
		sys_err( "kill_campfire_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;

	if (ch == NULL) { // <Factor>
		return 0;
	}
	ch->m_pkMiningEvent = NULL;
	M2_DESTROY_CHARACTER(ch);
	return 0;
}

bool CHARACTER::GiveRecallItem(LPITEM item)
{
	int idx = GetMapIndex();
	int iEmpireByMapIndex = -1;

	if (idx < 20)
		iEmpireByMapIndex = 1;
	else if (idx < 40)
		iEmpireByMapIndex = 2;
	else if (idx < 60)
		iEmpireByMapIndex = 3;
	else if (idx < 10000)
		iEmpireByMapIndex = 0;

	switch (idx)
	{
		case 66:
		case 216:
			iEmpireByMapIndex = -1;
			break;
	}

	if (iEmpireByMapIndex && GetEmpire() != iEmpireByMapIndex)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?? ?? ???? ???? ??????."));
		return false;
	}

	int pos;

	if (item->GetCount() == 1)	// ???????? ???????? ???? ????.
	{
		item->SetSocket(0, GetX());
		item->SetSocket(1, GetY());
	}
	else if ((pos = GetEmptyInventory(item->GetSize())) != -1) // ?????? ?????? ???? ???????? ?????? ??????.
	{
		LPITEM item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), 1);

		if (NULL != item2)
		{
			item2->SetSocket(0, GetX());
			item2->SetSocket(1, GetY());
			item2->AddToCharacter(this, TItemPos(INVENTORY, pos), true);

			item->SetCount(item->GetCount() - 1);
		}
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ?? ?????? ????????."));
		return false;
	}

	return true;
}

void CHARACTER::ProcessRecallItem(LPITEM item)
{
	int idx;

	if ((idx = SECTREE_MANAGER::instance().GetMapIndex(item->GetSocket(0), item->GetSocket(1))) == 0)
		return;

	int iEmpireByMapIndex = -1;

	if (idx < 20)
		iEmpireByMapIndex = 1;
	else if (idx < 40)
		iEmpireByMapIndex = 2;
	else if (idx < 60)
		iEmpireByMapIndex = 3;
	else if (idx < 10000)
		iEmpireByMapIndex = 0;

	switch (idx)
	{
		case 66:
		case 216:
			iEmpireByMapIndex = -1;
			break;
		// ???????? ????
		case 301:
		case 302:
		case 303:
		case 304:
			if( GetLevel() < 90 )
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???????? ?????? ????????."));
				return;
			}
			else
				break;
	}

	if (iEmpireByMapIndex && GetEmpire() != iEmpireByMapIndex)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ???????? ???? ?????? ?????? ?? ????????."));
		item->SetSocket(0, 0);
		item->SetSocket(1, 0);
	}
	else
	{
		sys_log(1, "Recall: %s %d %d -> %d %d", GetName(), GetX(), GetY(), item->GetSocket(0), item->GetSocket(1));
		WarpSet(item->GetSocket(0), item->GetSocket(1));
		item->SetCount(item->GetCount() - 1);
	}
}

void CHARACTER::__OpenPrivateShop()
{
#ifdef __OPEN_SHOP_WITH_ARMOR__
	ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
#else
	unsigned bodyPart = GetPart(PART_MAIN);
	switch (bodyPart)
	{
		case 0:
		case 1:
		case 2:
			ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
			break;
		default:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ???? ?????? ?? ?? ????????."));
			break;
	}
#endif
}

int CalculateConsume(LPCHARACTER ch)
{
	static const int WARP_NEED_LIFE_PERCENT	= 30;
	static const int WARP_MIN_LIFE_PERCENT	= 10;
	int consumeLife = 0;
	{
		const int curLife		= ch->GetHP();
		const int needPercent	= WARP_NEED_LIFE_PERCENT;
		const int needLife = ch->GetMaxHP() * needPercent / 100;
		if (curLife < needLife)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ?????? ?????? ?? ????????."));
			return -1;
		}

		consumeLife = needLife;

		const int minPercent	= WARP_MIN_LIFE_PERCENT;
		const int minLife	= ch->GetMaxHP() * minPercent / 100;
		if (curLife - needLife < minLife)
			consumeLife = curLife - minLife;

		if (consumeLife < 0)
			consumeLife = 0;
	}
	return consumeLife;
}

int CalculateConsumeSP(LPCHARACTER lpChar)
{
	static const int NEED_WARP_SP_PERCENT = 30;

	const int curSP = lpChar->GetSP();
	const int needSP = lpChar->GetMaxSP() * NEED_WARP_SP_PERCENT / 100;

	if (curSP < needSP)
	{
		lpChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ?????? ?????? ?? ????????."));
		return -1;
	}

	return needSP;
}

bool CHARACTER::UseItemEx(LPITEM item, TItemPos DestCell)
{
	int iLimitRealtimeStartFirstUseFlagIndex = -1;
	//int iLimitTimerBasedOnWearFlagIndex = -1;

	WORD wDestCell = DestCell.cell;
	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		long limitValue = item->GetProto()->aLimits[i].lValue;

		switch (item->GetProto()->aLimits[i].bType)
		{
			case LIMIT_LEVEL:
				if (GetLevel() < limitValue)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???????? ?????? ????????."));
					return false;
				}
				break;

			case LIMIT_REAL_TIME_START_FIRST_USE:
				iLimitRealtimeStartFirstUseFlagIndex = i;
				break;

			case LIMIT_TIMER_BASED_ON_WEAR:
				//iLimitTimerBasedOnWearFlagIndex = i;
				break;
		}
	}

	if ( CArenaManager::instance().IsLimitedItem( GetMapIndex(), item->GetVnum() ) == true )
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
		return false;
	}
#ifdef __NEWSTUFF__
	else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && IsLimitedPotionOnPVP(item->GetVnum()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
		return false;
	}
#endif

	// @fixme402 (IsLoadedAffect to block affect hacking)
	if (!IsLoadedAffect())
	{
		ChatPacket(CHAT_TYPE_INFO, "Affects are not loaded yet!");
		return false;
	}

	// @fixme141 BEGIN
	if (TItemPos(item->GetWindow(), item->GetCell()).IsBeltInventoryPosition())
	{
		LPITEM beltItem = GetWear(WEAR_BELT);

		if (NULL == beltItem)
		{
			ChatPacket(CHAT_TYPE_INFO, "<Belt> You can't use this item if you have no equipped belt.");
			return false;
		}

		if (false == CBeltInventoryHelper::IsAvailableCell(item->GetCell() - BELT_INVENTORY_SLOT_START, beltItem->GetValue(0)))
		{
			ChatPacket(CHAT_TYPE_INFO, "<Belt> You can't use this item if you don't upgrade your belt.");
			return false;
		}
	}
	// @fixme141 END

	// ?????? ???? ???? ?????????? ???????? ?????? ?????? ???????? ???? ????.
	if (-1 != iLimitRealtimeStartFirstUseFlagIndex)
	{
		// ?? ???????? ?????? ?????????? ?????? Socket1?? ???? ????????. (Socket1?? ???????? ????)
		if (0 == item->GetSocket(1))
		{
			// ?????????????? Default ?????? Limit Value ???? ????????, Socket0?? ???? ?????? ?? ???? ?????????? ????. (?????? ??)
			long duration = (0 != item->GetSocket(0)) ? item->GetSocket(0) : item->GetProto()->aLimits[iLimitRealtimeStartFirstUseFlagIndex].lValue;

			if (0 == duration)
				duration = 60 * 60 * 24 * 7;

			item->SetSocket(0, time(0) + duration);
			item->StartRealTimeExpireEvent();
		}

		if (false == item->IsEquipped())
			item->SetSocket(1, item->GetSocket(1) + 1);
	}

#ifdef __GROWTH_PET__
	if (item->GetVnum() == 55009 || item->GetVnum() == 76044)
	{
		int n = number(55010, 55027);
		AutoGiveItem(n);
		item->SetCount(item->GetCount() - 1);
	}
#endif

	switch (item->GetType())
	{
#ifdef __GROWTH_PET__
		case ITEM_PET:
		{
			switch (item->GetSubType())
			{
				case PET_EGG:
					SetEggVid(item->GetVnum());
					ChatPacket(CHAT_TYPE_COMMAND, "OpenPetIncubator %d", item->GetVnum() - 55401);
					break;

				case PET_UPBRINGING:
				{
					if (LPITEM item2 = GetItem(DestCell))
					{
						if (item2->GetSubType() == PET_BAG)
						{
							if (item2->GetAttributeValue(0) > 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_LCTEXT4"));
							}
							else
							{
								if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
									return false;

								if (item2->IsExchanging())
									return false;

								if (GetNewPetSystem()->IsActivePet())
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_LCTEXT5"));
									return false;
								}
								
								for (int b = 0; b < 3; b++)
									item2->SetForceAttribute(b, 1, item->GetAttributeValue(b));

								item2->SetForceAttribute(3, 1, item->GetAttributeValue(3));
								item2->SetForceAttribute(4, 1, item->GetSocket(0) - time(0));
								item2->SetSocket(0, time(0) + item2->GetLimitValue(0));
								item2->StartRealTimeExpireEvent();
								item2->SetSocket(1, item->GetSocket(1));
								ChatPacket(1, "%d %d %d %d %d", item->GetSocket(1), item2->GetSocket(1), time(0), item->GetSocket(0), item->GetSocket(0) - time(0));
								item2->SetSocket(2, item->GetVnum() - 55700);
								item2->SetSocket(3, item->GetSocket(3));
								DBManager::instance().DirectQuery("UPDATE new_petsystem SET id = %d WHERE id = %d", item2->GetID(), item->GetID());
								ITEM_MANAGER::instance().RemoveItem(item);
								return true;
							}
						}
					}
					else
					{
						CNewPetSystem* petSystem = GetNewPetSystem();
						if (petSystem)
						{
							CNewPetActor* pet = petSystem->GetByVnum(item->GetValue(0));
							if (pet && pet->IsSummoned())
							{
								petSystem->Unsummon(item->GetValue(0));
								return true;
							}
							else
							{
								size_t count = petSystem->CountSummoned();
								if (count > 0)
								{
									ChatPacket(CHAT_TYPE_INFO, "Zaten Petin Var");
									return true;
								}
								else
								{
									const CMob* pMob = CMobManager::instance().Get(item->GetValue(0));
									// std::string nm = GetName();
									std::string nm = "";
									nm += GetName();
									nm += "'s ";
									nm += pMob->m_table.szLocaleName;
									const char* mobname = nm.c_str();
									petSystem->Summon(item->GetValue(0), item, mobname, false);
									return true;
								}
							}
						}
						else
							return false;
					}
				}
				break;

				case PET_FEEDSTUFF:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging())
						return false;

					if (item2->GetSubType() != PET_UPBRINGING)
						return false;
					
					char szQuery1[1024];
					snprintf(szQuery1, sizeof(szQuery1), "SELECT duration, tduration FROM new_petsystem WHERE id = %d LIMIT 1", item2->GetID());
					std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
					if (pmsg2->Get()->uiNumRows > 0)
					{
						MYSQL_ROW row = mysql_fetch_row(pmsg2->Get()->pSQLResult);
						if (atoi(row[0]) > 0)
						{
							if (GetNewPetSystem()->IsActivePet())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_LCTEXT1"));
								return false;
							}

							DBManager::instance().DirectQuery("UPDATE new_petsystem SET duration = (tduration) WHERE id = %d", item2->GetID());
							item2->SetSocket(0, time(0) + atoi(row[1]));
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_LCTEXT2"));
						}
						else
						{
							DBManager::instance().DirectQuery("UPDATE new_petsystem SET duration = (tduration / 2) WHERE id = %d", item2->GetID());
							item2->SetSocket(0, time(0) + (atoi(row[1]) / 2));
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_LCTEXT3"));
						}
						item->SetCount(item->GetCount() - 1);
						return true;
					}
					else
						return false;
				}
				break;

				case PET_BAG:
				{
					if (item->GetAttributeValue(0) > 0)
					{
						int pos = GetEmptyInventory(item->GetSize());
						if(pos == -1)
						{
							ChatPacket(CHAT_TYPE_INFO, "Yeterli Alan Yok !");
							return false;
						}

						if (item->IsExchanging())
							return false;
						DWORD vnum2 = 55700 + item->GetSocket(2);
						LPITEM item2 = AutoGiveItem(vnum2, 1);
						for (int b = 0; b < 3; b++)
							item2->SetForceAttribute(b, 1, item->GetAttributeValue(b));

						item2->SetForceAttribute(3, 1, item->GetAttributeValue(3));
						item2->SetSocket(0, item->GetAttributeValue(4) + time(0));
						ChatPacket(1, "%d %d", item->GetSocket(1), item2->GetSocket(1));
						item2->SetSocket(1, item->GetSocket(1));
						ChatPacket(1, "%d %d %d %d %d", item->GetSocket(1), item2->GetSocket(1), time(0), item->GetAttributeValue(4), item->GetAttributeValue(4) + time(0));
						item2->SetSocket(3, item->GetSocket(3));
						DBManager::instance().DirectQuery("UPDATE new_petsystem SET id = %d WHERE id = %d", item2->GetID(), item->GetID());
						ITEM_MANAGER::instance().RemoveItem(item);
						return true;
					}
				}
				break;

				case PET_SKILL:
				{
					CNewPetSystem* petSystem = GetNewPetSystem();
					if (petSystem)
					{
						if (petSystem->GetLevel() >= 81 && petSystem->GetEvolution() >= 3)
						{
							petSystem->IncreasePetSkill(item->GetVnum() - 55009);
							item->SetCount(item->GetCount() - 1);
						}
					}
				}
				break;

				case PET_NAME_CHANGE:
				{
					
				}
				break;

				case PET_SKILL_ALL_DEL_BOOK:
				{
					
				}
				break;

				case PET_SKILL_DEL_BOOK:
				{
					
				}
				break;

				case PET_EXPFOOD_PER:
				{
					
				}
				break;

				case PET_ATTR_CHANGE:
					break;

				case PET_ATTR_CHANGE_ITEMVNUM:
					break;

				case PET_PAY:
				{
#ifdef __COSTUME_PET__
					if (!item->IsEquipped())
					{
						if (GetPetSystem()->CountSummoned() > 0)
							return false;

						EquipItem(item);
					}
					else
						UnequipItem(item);
#endif
				}
				break;
			}
		}
		break;
#endif
		case ITEM_HAIR:
			return ItemProcess_Hair(item, wDestCell);

		case ITEM_POLYMORPH:
			return ItemProcess_Polymorph(item);

		case ITEM_QUEST:
			if (GetArena() != NULL || IsObserverMode() == true)
			{
				if (item->GetVnum() == 50051 || item->GetVnum() == 50052 || item->GetVnum() == 50053)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
					return false;
				}
			}

			if (!IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE))
			{
				if (item->GetSIGVnum() == 0)
				{
					quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);
				}
				else
				{
					quest::CQuestManager::instance().SIGUse(GetPlayerID(), item->GetSIGVnum(), item, false);
				}
			}
			break;

		case ITEM_CAMPFIRE:
			{
				float fx, fy;
				GetDeltaByDegree(GetRotation(), 100.0f, &fx, &fy);

				LPSECTREE tree = SECTREE_MANAGER::instance().Get(GetMapIndex(), (long)(GetX()+fx), (long)(GetY()+fy));

				if (!tree)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ?? ???? ??????????."));
					return false;
				}

				if (tree->IsAttr((long)(GetX()+fx), (long)(GetY()+fy), ATTR_WATER))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ???????? ???? ?? ????????."));
					return false;
				}

				LPCHARACTER campfire = CHARACTER_MANAGER::instance().SpawnMob(fishing::CAMPFIRE_MOB, GetMapIndex(), (long)(GetX()+fx), (long)(GetY()+fy), 0, false, number(0, 359));

				char_event_info* info = AllocEventInfo<char_event_info>();

				info->ch = campfire;

				campfire->m_pkMiningEvent = event_create(kill_campfire_event, info, PASSES_PER_SEC(40));

				item->SetCount(item->GetCount() - 1);
			}
			break;

		case ITEM_UNIQUE:
			{
				switch (item->GetSubType())
				{
					case USE_ABILITY_UP:
						{
							switch (item->GetValue(0))
							{
								case APPLY_MOV_SPEED:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_MOV_SPEED, item->GetValue(2), AFF_MOV_SPEED_POTION, item->GetValue(1), 0, true, true);
									break;

								case APPLY_ATT_SPEED:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_SPEED, item->GetValue(2), AFF_ATT_SPEED_POTION, item->GetValue(1), 0, true, true);
									break;

								case APPLY_STR:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ST, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_DEX:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_DX, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_CON:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_HT, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_INT:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_IQ, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_CAST_SPEED:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_CASTING_SPEED, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_RESIST_MAGIC:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_RESIST_MAGIC, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_ATT_GRADE_BONUS:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_GRADE_BONUS,
											item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_DEF_GRADE_BONUS:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_DEF_GRADE_BONUS,
											item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;
							}
						}

						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						item->SetCount(item->GetCount() - 1);
						break;
#ifdef __MYSHOP_DECO__
					case USE_SPECIAL:
						switch (item->GetVnum())
						{
							case 71221:
								ChatPacket(CHAT_TYPE_COMMAND, "OpenMyShopDecoWnd");
								break;
							default:
								break;
						}
						break;
#endif
					default:
						{
							if (item->GetSubType() == USE_SPECIAL)
							{
								sys_log(0, "ITEM_UNIQUE: USE_SPECIAL %u", item->GetVnum());

								switch (item->GetVnum())
								{
									case 71049: // ??????????
										if (g_bEnableBootaryCheck)
										{
											if (IS_BOTARYABLE_ZONE(GetMapIndex()) == true)
											{
												__OpenPrivateShop();
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?? ?? ???? ??????????"));
											}
										}
										else
										{
											__OpenPrivateShop();
										}
										break;
								}
							}
							else
							{
								if (!item->IsEquipped())
									EquipItem(item);
								else
									UnequipItem(item);
							}
						}
						break;
				}
			}
			break;

		case ITEM_COSTUME:
		case ITEM_WEAPON:
		case ITEM_ARMOR:
		case ITEM_ROD:
		case ITEM_RING:		// ???? ???? ??????
		case ITEM_BELT:		// ???? ???? ??????
			// MINING
		case ITEM_PICK:
			// END_OF_MINING
			if (!item->IsEquipped())
				EquipItem(item);
			else
				UnequipItem(item);
			break;
			// ???????? ???? ???????? ?????? ?? ????.
			// ???????? ????????, ???????? ?????? item use ?????? ???? ?? ????.
			// ?????? ?????? item move ???????? ????.
			// ?????? ???????? ????????.
		case ITEM_DS:
			{
				if (!item->IsEquipped())
					return false;
				return DSManager::instance().PullOut(this, NPOS, item);
			break;
			}
		case ITEM_SPECIAL_DS:
			if (!item->IsEquipped())
				EquipItem(item);
			else
				UnequipItem(item);
			break;

		case ITEM_FISH:
			{
				if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
					return false;
				}
#ifdef __NEWSTUFF__
				else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
					return false;
				}
#endif

				if (item->GetSubType() == FISH_ALIVE)
					fishing::UseFish(this, item);
			}
			break;

		case ITEM_TREASURE_BOX:
			{
				return false;
				//ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("?????? ???? ?????? ?????? ?????? ????. ?????? ????????."));
			}
			break;

		case ITEM_TREASURE_KEY:
			{
				LPITEM item2;

				if (!GetItem(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
					return false;

				if (item2->GetType() != ITEM_TREASURE_BOX)
				{
					ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("?????? ???? ?????? ?????? ????."));
					return false;
				}

				if (item->GetValue(0) == item2->GetValue(0))
				{
					//ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("?????? ?????? ?????? ???? ???? ?????? ????????????."));
					DWORD dwBoxVnum = item2->GetVnum();
					std::vector <DWORD> dwVnums;
					std::vector <DWORD> dwCounts;
					std::vector <LPITEM> item_gets(0);
					int count = 0;

					if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
					{
						ITEM_MANAGER::instance().RemoveItem(item);
						ITEM_MANAGER::instance().RemoveItem(item2);

						for (int i = 0; i < count; i++){
							switch (dwVnums[i])
							{
								case CSpecialItemGroup::GOLD:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? %d ???? ????????????."), dwCounts[i]);
									break;
								case CSpecialItemGroup::EXP:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ?????? ???? ????????."));
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d?? ???????? ????????????."), dwCounts[i]);
									break;
								case CSpecialItemGroup::MOB:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ????????????!"));
									break;
								case CSpecialItemGroup::SLOW:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???????? ?????? ????????????!"));
									break;
								case CSpecialItemGroup::DRAIN_HP:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ??????????????! ???????? ????????????."));
									break;
								case CSpecialItemGroup::POISON:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???? ???????? ????????!"));
									break;
#ifdef __WOLFMAN__
								case CSpecialItemGroup::BLEEDING:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???? ???????? ????????!"));
									break;
#endif
								case CSpecialItemGroup::MOB_GROUP:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ????????????!"));
									break;
								default:
									if (item_gets[i])
									{
										if (dwCounts[i] > 1)
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? %s ?? %d ?? ??????????."), item_gets[i]->GetMultiName(), dwCounts[i]);
										else
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? %s ?? ??????????."), item_gets[i]->GetMultiName());

									}
							}
						}
					}
					else
					{
						ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("?????? ???? ???? ?? ????."));
						return false;
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("?????? ???? ???? ?? ????."));
					return false;
				}
			}
			break;

		case ITEM_GIFTBOX:
			{
#ifdef __NEWSTUFF__
				if (0 != g_BoxUseTimeLimitValue)
				{
					if (get_dword_time() < m_dwLastBoxUseTime+g_BoxUseTimeLimitValue)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ?? ????????."));
						return false;
					}
				}

				m_dwLastBoxUseTime = get_dword_time();
#endif
				DWORD dwBoxVnum = item->GetVnum();
				std::vector <DWORD> dwVnums;
				std::vector <DWORD> dwCounts;
				std::vector <LPITEM> item_gets(0);
				int count = 0;

				if( dwBoxVnum > 51500 && dwBoxVnum < 52000 )	// ??????????
				{
					if( !(this->DragonSoul_IsQualified()) )
					{
						ChatPacket(CHAT_TYPE_INFO,LC_TEXT("???? ?????? ???????? ?????????? ??????."));
						return false;
					}
				}

				if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
				{
					item->SetCount(item->GetCount()-1);

					for (int i = 0; i < count; i++){
						switch (dwVnums[i])
						{
						case CSpecialItemGroup::GOLD:
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? %d ???? ????????????."), dwCounts[i]);
							break;
						case CSpecialItemGroup::EXP:
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ?????? ???? ????????."));
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d?? ???????? ????????????."), dwCounts[i]);
							break;
						case CSpecialItemGroup::MOB:
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ????????????!"));
							break;
						case CSpecialItemGroup::SLOW:
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???????? ?????? ????????????!"));
							break;
						case CSpecialItemGroup::DRAIN_HP:
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ??????????????! ???????? ????????????."));
							break;
						case CSpecialItemGroup::POISON:
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???? ???????? ????????!"));
							break;
#ifdef __WOLFMAN__
						case CSpecialItemGroup::BLEEDING:
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???? ???????? ????????!"));
							break;
#endif
						case CSpecialItemGroup::MOB_GROUP:
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ????????????!"));
							break;
						default:
							if (item_gets[i])
							{
								if (dwCounts[i] > 1)
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? %s ?? %d ?? ??????????."), item_gets[i]->GetMultiName(), dwCounts[i]);
								else
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? %s ?? ??????????."), item_gets[i]->GetMultiName());
							}
						}
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("???????? ???? ?? ??????????."));
					return false;
				}
			}
			break;

		case ITEM_SKILLFORGET:
			{
				if (!item->GetSocket(0))
				{
					ITEM_MANAGER::instance().RemoveItem(item);
					return false;
				}

				DWORD dwVnum = item->GetSocket(0);

				if (SkillLevelDown(dwVnum))
				{
					ITEM_MANAGER::instance().RemoveItem(item);
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???????? ??????????????."));
				}
				else
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ?? ????????."));
			}
			break;

		case ITEM_SKILLBOOK:
			{
				if (IsPolymorphed())
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???? ?????? ????????."));
					return false;
				}

				DWORD dwVnum = 0;

				if (item->GetVnum() == 50300)
				{
					dwVnum = item->GetSocket(0);
				}
				else
				{
					// ?????? ???????? value 0 ?? ???? ?????? ???????? ?????? ????.
					dwVnum = item->GetValue(0);
				}

				if (0 == dwVnum)
				{
					ITEM_MANAGER::instance().RemoveItem(item);

					return false;
				}

				if (true == LearnSkillByBook(dwVnum))
				{
#ifdef __BOOKS_STACKFIX_
					item->SetCount(item->GetCount() - 1);
#else
					ITEM_MANAGER::instance().RemoveItem(item);
#endif

					int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

					SetSkillNextReadTime(dwVnum, get_global_time() + iReadDelay);
				}
			}
			break;

		case ITEM_USE:
			{
#ifdef __EXPRESSING_EMOTION__
				if (item->GetVnum() == 38060)
				{
					if (GetEmotionCount() < SPECIAL_EMOTION_MAX)
					{
num:
						BYTE bNumber = number(61, 78);
						if (!FindEmotion(AFFECT_EMOTION, APPLY_NONE, bNumber))
						{
							AddAffect(AFFECT_EMOTION, 0, bNumber, AFF_NONE, 60*60*12, 0, false);
							item->SetCount(item->GetCount() - 1);
						}
						else
							goto num;

						return true;
					}
					else
					{
						return false;
					}

				}
#endif
				if (item->GetVnum() > 50800 && item->GetVnum() <= 50820)
				{
					int affect_type = AFFECT_EXP_BONUS_EURO_FREE;
					int apply_type = aApplyInfo[item->GetValue(0)].bPointType;
					int apply_value = item->GetValue(2);
					int apply_duration = item->GetValue(1);

					switch (item->GetSubType())
					{
						case USE_ABILITY_UP:
							if (FindAffect(affect_type, apply_type))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ????????."));
								return false;
							}

							{
								switch (item->GetValue(0))
								{
									case APPLY_MOV_SPEED:
										AddAffect(affect_type, apply_type, apply_value, AFF_MOV_SPEED_POTION, apply_duration, 0, true, true);
										break;

									case APPLY_ATT_SPEED:
										AddAffect(affect_type, apply_type, apply_value, AFF_ATT_SPEED_POTION, apply_duration, 0, true, true);
										break;

									case APPLY_STR:
									case APPLY_DEX:
									case APPLY_CON:
									case APPLY_INT:
									case APPLY_CAST_SPEED:
									case APPLY_RESIST_MAGIC:
									case APPLY_ATT_GRADE_BONUS:
									case APPLY_DEF_GRADE_BONUS:
										AddAffect(affect_type, apply_type, apply_value, 0, apply_duration, 0, true, true);
										break;
								}
							}

							if (GetDungeon())
								GetDungeon()->UsePotion(this);

							if (GetWarMap())
								GetWarMap()->UsePotion(this, item);

							item->SetCount(item->GetCount() - 1);
							break;

					case USE_AFFECT :
						{
							if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE, aApplyInfo[item->GetValue(1)].bPointType))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ????????."));
							}
							else
							{
								AddAffect(AFFECT_EXP_BONUS_EURO_FREE, aApplyInfo[item->GetValue(1)].bPointType, item->GetValue(2), 0, item->GetValue(3), 0, false, true);
								item->SetCount(item->GetCount() - 1);
							}
						}
						break;

					case USE_POTION_NODELAY:
						{
							if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
							{
								if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit") > 0)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???????? ?? ????????."));
									return false;
								}

								switch (item->GetVnum())
								{
									case 70020 :
									case 71018 :
									case 71019 :
									case 71020 :
										if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count") < 10000)
										{
											if (m_nPotionLimit <= 0)
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? ??????????????."));
												return false;
											}
										}
										break;

									default :
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???????? ?? ????????."));
										return false;
										break;
								}
							}
#ifdef __NEWSTUFF__
							else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
								return false;
							}
#endif

							bool used = false;

							if (item->GetValue(0) != 0) // HP ?????? ????
							{
								if (GetHP() < GetMaxHP())
								{
									PointChange(POINT_HP, item->GetValue(0) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
									EffectPacket(SE_HPUP_RED);
									used = TRUE;
								}
							}

							if (item->GetValue(1) != 0)	// SP ?????? ????
							{
								if (GetSP() < GetMaxSP())
								{
									PointChange(POINT_SP, item->GetValue(1) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
									EffectPacket(SE_SPUP_BLUE);
									used = TRUE;
								}
							}

							if (item->GetValue(3) != 0) // HP % ????
							{
								if (GetHP() < GetMaxHP())
								{
									PointChange(POINT_HP, item->GetValue(3) * GetMaxHP() / 100);
									EffectPacket(SE_HPUP_RED);
									used = TRUE;
								}
							}

							if (item->GetValue(4) != 0) // SP % ????
							{
								if (GetSP() < GetMaxSP())
								{
									PointChange(POINT_SP, item->GetValue(4) * GetMaxSP() / 100);
									EffectPacket(SE_SPUP_BLUE);
									used = TRUE;
								}
							}

							if (used)
							{
								if (item->GetVnum() == 50085 || item->GetVnum() == 50086)
								{
									SetUseSeedOrMoonBottleTime();
								}
								if (GetDungeon())
									GetDungeon()->UsePotion(this);

								if (GetWarMap())
									GetWarMap()->UsePotion(this, item);

								m_nPotionLimit--;

								//RESTRICT_USE_SEED_OR_MOONBOTTLE
								item->SetCount(item->GetCount() - 1);
								//END_RESTRICT_USE_SEED_OR_MOONBOTTLE
							}
						}
						break;
					}

					return true;
				}


				if (item->GetVnum() >= 27863 && item->GetVnum() <= 27883)
				{
					if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
						return false;
					}
#ifdef __NEWSTUFF__
					else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
						return false;
					}
#endif
				}

				switch (item->GetSubType())
				{
					case USE_TIME_CHARGE_PER:
						{
							LPITEM pDestItem = GetItem(DestCell);
							if (NULL == pDestItem)
							{
								return false;
							}
							// ???? ???????? ???????? ?????? ????.
							if (pDestItem->IsDragonSoul())
							{
								int ret;
								char buf[128];
								if (item->GetVnum() == DRAGON_HEART_VNUM)
								{
									ret = pDestItem->GiveMoreTime_Per((float)item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
								}
								else
								{
									ret = pDestItem->GiveMoreTime_Per((float)item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
								}
								if (ret > 0)
								{
									if (item->GetVnum() == DRAGON_HEART_VNUM)
									{
										sprintf(buf, "Inc %ds by item{VN:%d SOC%d:%ld}", ret, item->GetVnum(), ITEM_SOCKET_CHARGING_AMOUNT_IDX, item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
									}
									else
									{
										sprintf(buf, "Inc %ds by item{VN:%d VAL%d:%ld}", ret, item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
									}

									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d?? ???? ??????????????."), ret);
									item->SetCount(item->GetCount() - 1);
									LogManager::instance().ItemLog(this, item, "DS_CHARGING_SUCCESS", buf);
									return true;
								}
								else
								{
									if (item->GetVnum() == DRAGON_HEART_VNUM)
									{
										sprintf(buf, "No change by item{VN:%d SOC%d:%ld}", item->GetVnum(), ITEM_SOCKET_CHARGING_AMOUNT_IDX, item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
									}
									else
									{
										sprintf(buf, "No change by item{VN:%d VAL%d:%ld}", item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
									}

									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?? ????????."));
									LogManager::instance().ItemLog(this, item, "DS_CHARGING_FAILED", buf);
									return false;
								}
							}
							else
								return false;
						}
						break;
					case USE_TIME_CHARGE_FIX:
						{
							LPITEM pDestItem = GetItem(DestCell);
							if (NULL == pDestItem)
							{
								return false;
							}
							// ???? ???????? ???????? ?????? ????.
							if (pDestItem->IsDragonSoul())
							{
								int ret = pDestItem->GiveMoreTime_Fix(item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
								char buf[128];
								if (ret)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d?? ???? ??????????????."), ret);
									sprintf(buf, "Increase %ds by item{VN:%d VAL%d:%ld}", ret, item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
									LogManager::instance().ItemLog(this, item, "DS_CHARGING_SUCCESS", buf);
									item->SetCount(item->GetCount() - 1);
									return true;
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?? ????????."));
									sprintf(buf, "No change by item{VN:%d VAL%d:%ld}", item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
									LogManager::instance().ItemLog(this, item, "DS_CHARGING_FAILED", buf);
									return false;
								}
							}
							else
								return false;
						}
						break;
					case USE_SPECIAL:

						switch (item->GetVnum())
						{
#ifdef __12ZI__
							case 72329:
							case 72327:
							{
								if (GetBead() + 30 >= BEAD_MAX)
								{
									ChatPacket(CHAT_TYPE_INFO, "Gecemez la 99 u");
									return false;
								}
								
								SetBead(GetBead() + 30, true);
								item->SetCount(item->GetCount() - 1);
							}
							break;
							case 72328:
							{
								if (!FindAffect(AFFECT_CZ_UNLIMIT_ENTER))
								{
									AddAffect(AFFECT_CZ_UNLIMIT_ENTER, 0, 0, AFF_NONE, 60*60*3, 0, false);
									item->SetCount(item->GetCount() - 1);
								}
								else
									ChatPacket(CHAT_TYPE_INFO, "Zaten Aktif");
							}
							break;
#endif
#ifdef __SOULBIND__
							case 50263:
								{
									LPITEM item2;
									if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
										return false;
									
									if (item2->IsEquipped() || item2->IsExchanging())
										return false;
									
									if (item2->IsSealed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Item already sealed."));
										return false;
									}

									if (item2->IsBasicItem())
										return false;

									if (item2->GetType() != ITEM_WEAPON && item2->GetType() != ITEM_ARMOR && item2->GetType() != ITEM_COSTUME)
										return false;

									item2->SetSealBind();
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Item sealbind success."));
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());
										LogManager::instance().ItemLog(this, item, "SET_SEALBIND_SUCCESS", buf);
									}
									item->SetCount(item->GetCount() - 1);
								}
								break;
							
							case 50264:
								{
									LPITEM item2;
									if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
										return false;
									
									if (item2->isLocked() || item2->IsEquipped() || item2->GetSealBindTime() >= 0)
										return false;

									if (item2->IsBasicItem())
										return false;

									if (item2->GetType() != ITEM_WEAPON && item2->GetType() != ITEM_ARMOR && item2->GetType() != ITEM_COSTUME && item2->GetType() != ITEM_DS)
										return false;

									long duration = 72 * 60 * 60;
									item2->SetSealBind(time(0) + duration);
									item2->StartUnSealBindTimerExpireEvent();
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Item unsealbind success."));
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());
										LogManager::instance().ItemLog(this, item, "REMOVE_SEALBIND_TIME_BEGIN", buf);
									}
									item->SetCount(item->GetCount() - 1);
								}
								break;
#endif
							//?????????? ????
							case ITEM_NOG_POCKET:
							{
								/*
								?????????? : item_proto value ????
									????????  value 1
									??????	  value 2
									??????    value 3
									????????  value 0 (???? ??)

								*/
								if (FindAffect(AFFECT_NOG_ABILITY))
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ????????."));
									return false;
								}
								long time = item->GetValue(0);
								long moveSpeedPer	= item->GetValue(1);
								long attPer	= item->GetValue(2);
								long expPer			= item->GetValue(3);
								AddAffect(AFFECT_NOG_ABILITY, POINT_MOV_SPEED, moveSpeedPer, AFF_MOV_SPEED_POTION, time, 0, true, true);
								AddAffect(AFFECT_NOG_ABILITY, POINT_MALL_ATTBONUS, attPer, AFF_NONE, time, 0, true, true);
								AddAffect(AFFECT_NOG_ABILITY, POINT_MALL_EXPBONUS, expPer, AFF_NONE, time, 0, true, true);
								item->SetCount(item->GetCount() - 1);
							}
							break;

							//???????? ????
							case ITEM_RAMADAN_CANDY:
							{
								/*
								?????????? : item_proto value ????
									????????  value 1
									??????	  value 2
									??????    value 3
									????????  value 0 (???? ??)

								*/
								// @fixme147 BEGIN
								if (FindAffect(AFFECT_RAMADAN_ABILITY))
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ????????."));
									return false;
								}
								// @fixme147 END
								long time = item->GetValue(0);
								long moveSpeedPer	= item->GetValue(1);
								long attPer	= item->GetValue(2);
								long expPer			= item->GetValue(3);
								AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MOV_SPEED, moveSpeedPer, AFF_MOV_SPEED_POTION, time, 0, true, true);
								AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MALL_ATTBONUS, attPer, AFF_NONE, time, 0, true, true);
								AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MALL_EXPBONUS, expPer, AFF_NONE, time, 0, true, true);
								item->SetCount(item->GetCount() - 1);
							}
							break;
							case ITEM_MARRIAGE_RING:
							{
								marriage::TMarriage* pMarriage = marriage::CManager::instance().Get(GetPlayerID());
								if (pMarriage)
								{
									if (pMarriage->ch1 != NULL)
									{
										if (CArenaManager::instance().IsArenaMap(pMarriage->ch1->GetMapIndex()) == true)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
											break;
										}
									}

									if (pMarriage->ch2 != NULL)
									{
										if (CArenaManager::instance().IsArenaMap(pMarriage->ch2->GetMapIndex()) == true)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
											break;
										}
									}

									int consumeSP = CalculateConsumeSP(this);

									if (consumeSP < 0)
										return false;

									PointChange(POINT_SP, -consumeSP, false);

									WarpToPID(pMarriage->GetOther(GetPlayerID()));
								}
								else
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?????????? ?????? ?? ????????."));
							}
							break;

								//???? ?????? ????
							case UNIQUE_ITEM_CAPE_OF_COURAGE:
								//?????? ?????? ?????? ????
							case 70057:
							case REWARD_BOX_UNIQUE_ITEM_CAPE_OF_COURAGE:
								AggregateMonster();
								// item->SetCount(item->GetCount()-1);
								break;

							case UNIQUE_ITEM_WHITE_FLAG:
								ForgetMyAttacker();
								item->SetCount(item->GetCount()-1);
								break;

							case UNIQUE_ITEM_TREASURE_BOX:
								break;

							case 30093:
							case 30094:
							case 30095:
							case 30096:
							{
								const int MAX_BAG_INFO = 26;
								static struct LuckyBagInfo
								{
									DWORD count;
									int prob;
									DWORD vnum;
								} b1[MAX_BAG_INFO] =
								{
									{ 1000,	302,	1 },
									{ 10,	150,	27002 },
									{ 10,	75,	27003 },
									{ 10,	100,	27005 },
									{ 10,	50,	27006 },
									{ 10,	80,	27001 },
									{ 10,	50,	27002 },
									{ 10,	80,	27004 },
									{ 10,	50,	27005 },
									{ 1,	10,	50300 },
									{ 1,	6,	92 },
									{ 1,	2,	132 },
									{ 1,	6,	1052 },
									{ 1,	2,	1092 },
									{ 1,	6,	2082 },
									{ 1,	2,	2122 },
									{ 1,	6,	3082 },
									{ 1,	2,	3122 },
									{ 1,	6,	5052 },
									{ 1,	2,	5082 },
									{ 1,	6,	7082 },
									{ 1,	2,	7122 },
									{ 1,	1,	11282 },
									{ 1,	1,	11482 },
									{ 1,	1,	11682 },
									{ 1,	1,	11882 },
								};

								LuckyBagInfo * bi = NULL;
								bi = b1;

								int pct = number(1, 1000);

								int i;
								for (i=0;i<MAX_BAG_INFO;i++)
								{
									if (pct <= bi[i].prob)
										break;
									pct -= bi[i].prob;
								}
								if (i>=MAX_BAG_INFO)
									return false;

								if (bi[i].vnum == 50300)
								{
									// ???????????? ???????? ????.
									GiveRandomSkillBook();
								}
								else if (bi[i].vnum == 1)
								{
									PointChange(POINT_GOLD, 1000, true);
								}
								else
								{
									AutoGiveItem(bi[i].vnum, bi[i].count);
								}
								ITEM_MANAGER::instance().RemoveItem(item);
							}
							break;

							case 50004: // ???????? ??????
							{
								if (item->GetSocket(0))
								{
									item->SetSocket(0, item->GetSocket(0) + 1);
								}
								else
								{
									// ???? ??????
									int iMapIndex = GetMapIndex();

									PIXEL_POSITION pos;

									if (SECTREE_MANAGER::instance().GetRandomLocation(iMapIndex, pos, 700))
									{
										item->SetSocket(0, 1);
										item->SetSocket(1, pos.x);
										item->SetSocket(2, pos.y);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ?????? ???????? ???????? ???????? ?????? ????????."));
										return false;
									}
								}

								int dist = 0;
								float distance = (DISTANCE_SQRT(GetX()-item->GetSocket(1), GetY()-item->GetSocket(2)));

								if (distance < 1000.0f)
								{
									// ????!
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ???????? ???? ???? ??????????."));

									// ?????????? ???? ???? ???????? ?????? ????.
									struct TEventStoneInfo
									{
										DWORD dwVnum;
										int count;
										int prob;
									};
									const int EVENT_STONE_MAX_INFO = 15;
									TEventStoneInfo info_10[EVENT_STONE_MAX_INFO] =
									{
										{ 27001, 10,  8 },
										{ 27004, 10,  6 },
										{ 27002, 10, 12 },
										{ 27005, 10, 12 },
										{ 27100,  1,  9 },
										{ 27103,  1,  9 },
										{ 27101,  1, 10 },
										{ 27104,  1, 10 },
										{ 27999,  1, 12 },

										{ 25040,  1,  4 },

										{ 27410,  1,  0 },
										{ 27600,  1,  0 },
										{ 25100,  1,  0 },

										{ 50001,  1,  0 },
										{ 50003,  1,  1 },
									};
									TEventStoneInfo info_7[EVENT_STONE_MAX_INFO] =
									{
										{ 27001, 10,  1 },
										{ 27004, 10,  1 },
										{ 27004, 10,  9 },
										{ 27005, 10,  9 },
										{ 27100,  1,  5 },
										{ 27103,  1,  5 },
										{ 27101,  1, 10 },
										{ 27104,  1, 10 },
										{ 27999,  1, 14 },

										{ 25040,  1,  5 },

										{ 27410,  1,  5 },
										{ 27600,  1,  5 },
										{ 25100,  1,  5 },

										{ 50001,  1,  0 },
										{ 50003,  1,  5 },

									};
									TEventStoneInfo info_4[EVENT_STONE_MAX_INFO] =
									{
										{ 27001, 10,  0 },
										{ 27004, 10,  0 },
										{ 27002, 10,  0 },
										{ 27005, 10,  0 },
										{ 27100,  1,  0 },
										{ 27103,  1,  0 },
										{ 27101,  1,  0 },
										{ 27104,  1,  0 },
										{ 27999,  1, 25 },

										{ 25040,  1,  0 },

										{ 27410,  1,  0 },
										{ 27600,  1,  0 },
										{ 25100,  1, 15 },

										{ 50001,  1, 10 },
										{ 50003,  1, 50 },

									};

									{
										TEventStoneInfo* info;
										if (item->GetSocket(0) <= 4)
											info = info_4;
										else if (item->GetSocket(0) <= 7)
											info = info_7;
										else
											info = info_10;

										int prob = number(1, 100);

										for (int i = 0; i < EVENT_STONE_MAX_INFO; ++i)
										{
											if (!info[i].prob)
												continue;

											if (prob <= info[i].prob)
											{
												AutoGiveItem(info[i].dwVnum, info[i].count);
												break;
											}
											prob -= info[i].prob;
										}
									}

									char chatbuf[CHAT_MAX_LEN + 1];
									int len = snprintf(chatbuf, sizeof(chatbuf), "StoneDetect %u 0 0", (DWORD)GetVID());

									if (len < 0 || len >= (int) sizeof(chatbuf))
										len = sizeof(chatbuf) - 1;

									++len;  // \0 ???????? ??????

									TPacketGCChat pack_chat;
									pack_chat.header	= HEADER_GC_CHAT;
									pack_chat.size		= sizeof(TPacketGCChat) + len;
									pack_chat.type		= CHAT_TYPE_COMMAND;
									pack_chat.id		= 0;
									pack_chat.bEmpire	= GetDesc()->GetEmpire();
									//pack_chat.id	= vid;

									TEMP_BUFFER buf;
									buf.write(&pack_chat, sizeof(TPacketGCChat));
									buf.write(chatbuf, len);

									PacketAround(buf.read_peek(), buf.size());

									ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (DETECT_EVENT_STONE) 1");
									return true;
								}
								else if (distance < 20000)
									dist = 1;
								else if (distance < 70000)
									dist = 2;
								else
									dist = 3;

								// ???? ?????????? ????????.
								const int STONE_DETECT_MAX_TRY = 10;
								if (item->GetSocket(0) >= STONE_DETECT_MAX_TRY)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ?????? ???? ??????????."));
									ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (DETECT_EVENT_STONE) 0");
									AutoGiveItem(27002);
									return true;
								}

								if (dist)
								{
									char chatbuf[CHAT_MAX_LEN + 1];
									int len = snprintf(chatbuf, sizeof(chatbuf),
											"StoneDetect %u %d %d",
											(DWORD)GetVID(), dist, (int)GetDegreeFromPositionXY(GetX(), item->GetSocket(2), item->GetSocket(1), GetY()));

									if (len < 0 || len >= (int) sizeof(chatbuf))
										len = sizeof(chatbuf) - 1;

									++len;  // \0 ???????? ??????

									TPacketGCChat pack_chat;
									pack_chat.header	= HEADER_GC_CHAT;
									pack_chat.size		= sizeof(TPacketGCChat) + len;
									pack_chat.type		= CHAT_TYPE_COMMAND;
									pack_chat.id		= 0;
									pack_chat.bEmpire	= GetDesc()->GetEmpire();
									//pack_chat.id		= vid;

									TEMP_BUFFER buf;
									buf.write(&pack_chat, sizeof(TPacketGCChat));
									buf.write(chatbuf, len);

									PacketAround(buf.read_peek(), buf.size());
								}

							}
							break;

							case 27989: // ??????????
							case 76006: // ?????? ??????????
							{
								LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());

								if (pMap != NULL)
								{
									// item->SetSocket(0, item->GetSocket(0) + 1);

									FFindStone f;

									// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
									pMap->for_each(f);

									if (f.m_mapStone.size() > 0)
									{
										std::map<DWORD, LPCHARACTER>::iterator stone = f.m_mapStone.begin();

										DWORD max = UINT_MAX;
										LPCHARACTER pTarget = stone->second;

										while (stone != f.m_mapStone.end())
										{
											DWORD dist = (DWORD)DISTANCE_SQRT(GetX()-stone->second->GetX(), GetY()-stone->second->GetY());

											if (dist != 0 && max > dist)
											{
												max = dist;
												pTarget = stone->second;
											}
											stone++;
										}

										if (pTarget != NULL)
										{
											int val = 3;

											if (max < 10000) val = 2;
											else if (max < 70000) val = 1;

											ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u %d %d", (DWORD)GetVID(), val,
													(int)GetDegreeFromPositionXY(GetX(), pTarget->GetY(), pTarget->GetX(), GetY()));
										}
										else
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????????? ???????? ?????? ????????."));
										}
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????????? ???????? ?????? ????????."));
									}

									// if (item->GetSocket(0) >= 6)
									// {
										// ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u 0 0", (DWORD)GetVID());
										// ITEM_MANAGER::instance().RemoveItem(item);
									// }
								}
								break;
							}
							break;

							case 27996: // ????
								item->SetCount(item->GetCount() - 1);
								AttackedByPoison(NULL); // @warme008
								break;

							case 27987: // ????
								// 50  ?????? 47990
								// 30  ??
								// 10  ?????? 47992
								// 7   ?????? 47993
								// 3   ?????? 47994
							{
								item->SetCount(item->GetCount() - 1);

								int r = number(1, 100);

								if (r <= 50)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ??????????."));
									AutoGiveItem(27990);
								}
								else
								{
									const int prob_table_gb2312[] =
									{
										95, 97, 99
									};

									const int * prob_table = prob_table_gb2312;

									if (r <= prob_table[0])
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ???? ??????????."));
									}
									else if (r <= prob_table[1])
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ??????????."));
										AutoGiveItem(27992);
									}
									else if (r <= prob_table[2])
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ??????????."));
										AutoGiveItem(27993);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ??????????."));
										AutoGiveItem(27994);
									}
								}
							}
							break;

#ifdef __BK_RUH__
							case 50513:
							case 50300:
								ChatPacket(CHAT_TYPE_COMMAND, "openOku");
								break;
#endif

							case 71013: // ??????????
								CreateFly(number(FLY_FIREWORK1, FLY_FIREWORK6), this);
								item->SetCount(item->GetCount() - 1);
								break;

							case 50100: // ????
							case 50101:
							case 50102:
							case 50103:
							case 50104:
							case 50105:
							case 50106:
								CreateFly(item->GetVnum() - 50100 + FLY_FIREWORK1, this);
								item->SetCount(item->GetCount() - 1);
								break;

							case 50200: // ??????
								if (g_bEnableBootaryCheck)
								{
									if (IS_BOTARYABLE_ZONE(GetMapIndex()) == true)
									{
										__OpenPrivateShop();
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?? ?? ???? ??????????"));
									}
								}
								else
								{
									__OpenPrivateShop();
								}
								break;

							case fishing::FISH_MIND_PILL_VNUM:
								AddAffect(AFFECT_FISH_MIND_PILL, POINT_NONE, 0, AFF_FISH_MIND, 20*60, 0, true);
								item->SetCount(item->GetCount() - 1);
								break;

							case 50301: // ?????? ??????
							case 50302:
							case 50303:
								{
									if (IsPolymorphed() == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ???? ?? ????????."));
										return false;
									}

									int lv = GetSkillLevel(SKILL_LEADERSHIP);

									if (lv < item->GetValue(0))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ???? ?????? ?????????? ????????."));
										return false;
									}

									if (lv >= item->GetValue(1))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ?????? ???? ?????? ?? ?? ???? ????????."));
										return false;
									}

									if (LearnSkillByBook(SKILL_LEADERSHIP))
									{
#ifdef __BOOKS_STACKFIX_
										item->SetCount(item->GetCount() - 1);
#else
										ITEM_MANAGER::instance().RemoveItem(item);
#endif

										int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

										SetSkillNextReadTime(SKILL_LEADERSHIP, get_global_time() + iReadDelay);
									}
								}
								break;

							case 50304: // ?????? ??????
							case 50305:
							case 50306:
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???? ?????? ????????."));
										return false;

									}
									if (GetSkillLevel(SKILL_COMBO) == 0 && GetLevel() < 30)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? 30?? ???? ?????? ?????? ?? ???? ?? ???? ????????."));
										return false;
									}

									if (GetSkillLevel(SKILL_COMBO) == 1 && GetLevel() < 50)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? 50?? ???? ?????? ?????? ?? ???? ?? ???? ????????."));
										return false;
									}

									if (GetSkillLevel(SKILL_COMBO) >= 2)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ?????? ?????? ?? ????????."));
										return false;
									}

									int iPct = item->GetValue(0);

									if (LearnSkillByBook(SKILL_COMBO, iPct))
									{
#ifdef __BOOKS_STACKFIX_
										item->SetCount(item->GetCount() - 1);
#else
										ITEM_MANAGER::instance().RemoveItem(item);
#endif

										int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

										SetSkillNextReadTime(SKILL_COMBO, get_global_time() + iReadDelay);
									}
								}
								break;
							case 50311: // ???? ??????
							case 50312:
							case 50313:
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???? ?????? ????????."));
										return false;

									}
									DWORD dwSkillVnum = item->GetValue(0);
									int iPct = MINMAX(0, item->GetValue(1), 100);
									if (GetSkillLevel(dwSkillVnum)>=20 || dwSkillVnum-SKILL_LANGUAGE1+1 == GetEmpire())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? ???????? ?? ???? ????????."));
										return false;
									}

									if (LearnSkillByBook(dwSkillVnum, iPct))
									{
#ifdef __BOOKS_STACKFIX_
										item->SetCount(item->GetCount() - 1);
#else
										ITEM_MANAGER::instance().RemoveItem(item);
#endif

										int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

										SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
									}
								}
								break;

							case 50061 : // ???? ?? ???? ???? ??????
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???? ?????? ????????."));
										return false;

									}
									DWORD dwSkillVnum = item->GetValue(0);
									int iPct = MINMAX(0, item->GetValue(1), 100);

									if (GetSkillLevel(dwSkillVnum) >= 10)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ?????? ?? ????????."));
										return false;
									}

									if (LearnSkillByBook(dwSkillVnum, iPct))
									{
#ifdef __BOOKS_STACKFIX_
										item->SetCount(item->GetCount() - 1);
#else
										ITEM_MANAGER::instance().RemoveItem(item);
#endif

										int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

										SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
									}
								}
								break;

							case 50314: case 50315: case 50316: // ???? ??????
							case 50323: case 50324: // ???? ??????
							case 50325: case 50326: // ???? ??????
								{
									if (IsPolymorphed() == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ???? ?? ????????."));
										return false;
									}

									int iSkillLevelLowLimit = item->GetValue(0);
									int iSkillLevelHighLimit = item->GetValue(1);
									int iPct = MINMAX(0, item->GetValue(2), 100);
									int iLevelLimit = item->GetValue(3);
									DWORD dwSkillVnum = 0;

									switch (item->GetVnum())
									{
										case 50314: case 50315: case 50316:
											dwSkillVnum = SKILL_POLYMORPH;
											break;

										case 50323: case 50324:
											dwSkillVnum = SKILL_ADD_HP;
											break;

										case 50325: case 50326:
											dwSkillVnum = SKILL_RESIST_PENETRATE;
											break;

										default:
											return false;
									}

									if (0 == dwSkillVnum)
										return false;

									if (GetLevel() < iLevelLimit)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ???????? ?????? ?? ?????? ??????."));
										return false;
									}

									if (GetSkillLevel(dwSkillVnum) >= 40)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ?????? ?? ????????."));
										return false;
									}

									if (GetSkillLevel(dwSkillVnum) < iSkillLevelLowLimit)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ???? ?????? ?????????? ????????."));
										return false;
									}

									if (GetSkillLevel(dwSkillVnum) >= iSkillLevelHighLimit)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?? ???? ?????? ?? ????????."));
										return false;
									}

									if (LearnSkillByBook(dwSkillVnum, iPct))
									{
#ifdef __BOOKS_STACKFIX_
										item->SetCount(item->GetCount() - 1);
#else
										ITEM_MANAGER::instance().RemoveItem(item);
#endif

										int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

										SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
									}
								}
								break;

							case 50902:
							case 50903:
							case 50904:
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???? ?????? ????????."));
										return false;

									}
									DWORD dwSkillVnum = SKILL_CREATE;
									int iPct = MINMAX(0, item->GetValue(1), 100);

									if (GetSkillLevel(dwSkillVnum)>=40)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ?????? ?? ????????."));
										return false;
									}

									if (LearnSkillByBook(dwSkillVnum, iPct))
									{
#ifdef __BOOKS_STACKFIX_
										item->SetCount(item->GetCount() - 1);
#else
										ITEM_MANAGER::instance().RemoveItem(item);
#endif

										int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

										SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
									}
								}
								break;

								// MINING
							case ITEM_MINING_SKILL_TRAIN_BOOK:
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???? ?????? ????????."));
										return false;

									}
									DWORD dwSkillVnum = SKILL_MINING;
									int iPct = MINMAX(0, item->GetValue(1), 100);

									if (GetSkillLevel(dwSkillVnum)>=40)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ?????? ?? ????????."));
										return false;
									}

									if (LearnSkillByBook(dwSkillVnum, iPct))
									{
#ifdef __BOOKS_STACKFIX_
										item->SetCount(item->GetCount() - 1);
#else
										ITEM_MANAGER::instance().RemoveItem(item);
#endif

										int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

										SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
									}
								}
								break;
								// END_OF_MINING

							case ITEM_HORSE_SKILL_TRAIN_BOOK:
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???? ?????? ????????."));
										return false;

									}
									DWORD dwSkillVnum = SKILL_HORSE;
									int iPct = MINMAX(0, item->GetValue(1), 100);

									if (GetLevel() < 50)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???? ?????? ?????? ?? ???? ?????? ????????."));
										return false;
									}

									if (get_global_time() < GetSkillNextReadTime(dwSkillVnum))
									{
										if (FindAffect(AFFECT_SKILL_NO_BOOK_DELAY))
										{
											// ???????? ?????????? ???? ???? ????
											RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???? ???????????? ??????????????."));
										}
										else
										{
											SkillLearnWaitMoreTimeMessage(GetSkillNextReadTime(dwSkillVnum) - get_global_time());
											return false;
										}
									}

									if (GetPoint(POINT_HORSE_SKILL) >= 20 ||
											GetSkillLevel(SKILL_HORSE_WILDATTACK) + GetSkillLevel(SKILL_HORSE_CHARGE) + GetSkillLevel(SKILL_HORSE_ESCAPE) >= 60 ||
											GetSkillLevel(SKILL_HORSE_WILDATTACK_RANGE) + GetSkillLevel(SKILL_HORSE_CHARGE) + GetSkillLevel(SKILL_HORSE_ESCAPE) >= 60)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ???? ???????? ???? ?? ????????."));
										return false;
									}

									if (number(1, 100) <= iPct)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? ???? ???? ???? ???????? ??????????."));
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????????? ???? ?????? ?????? ???? ?? ????????."));
										PointChange(POINT_HORSE_SKILL, 1);

										int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

										SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ??????????????."));
									}
#ifdef __BOOKS_STACKFIX_
									item->SetCount(item->GetCount() - 1);
#else
									ITEM_MANAGER::instance().RemoveItem(item);
#endif
								}
								break;

							case 70102: // ????
							case 70103: // ????
								{
									if (GetAlignment() >= 0)
										return false;

									int delta = MIN(-GetAlignment(), item->GetValue(0));

									sys_log(0, "%s ALIGNMENT ITEM %d", GetName(), delta);

									UpdateAlignment(delta);
									item->SetCount(item->GetCount() - 1);

									if (delta / 10 > 0)
									{
										ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("?????? ??????????. ?????? ???????? ???????? ?? ???????? ????????."));
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? %d ??????????????."), delta/10);
									}
								}
								break;

							case 71107: // ??????????
								{
									int val = item->GetValue(0);
									int interval = item->GetValue(1);
									quest::PC* pPC = quest::CQuestManager::instance().GetPC(GetPlayerID());
									int last_use_time = pPC->GetFlag("mythical_peach.last_use_time");

									if (get_global_time() - last_use_time < interval * 60 * 60)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?? ????????."));
										return false;
									}

									if (GetAlignment() == 200000)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ?? ???? ???? ?? ????????."));
										return false;
									}

									if (200000 - GetAlignment() < val * 10)
									{
										val = (200000 - GetAlignment()) / 10;
									}

									int old_alignment = GetAlignment() / 10;

									UpdateAlignment(val*10);

									item->SetCount(item->GetCount()-1);
									pPC->SetFlag("mythical_peach.last_use_time", get_global_time());

									ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("?????? ??????????. ?????? ???????? ???????? ?? ???????? ????????."));
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? %d ??????????????."), val);

									char buf[256 + 1];
									snprintf(buf, sizeof(buf), "%d %d", old_alignment, GetAlignment() / 10);
									LogManager::instance().CharLog(this, val, "MYTHICAL_PEACH", buf);
								}
								break;

							case 71109: // ??????
							case 72719:
								{
									LPITEM item2;

									if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
										return false;

									if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
										return false;

									if (item2->GetSocketCount() == 0)
										return false;

									switch( item2->GetType() )
									{
										case ITEM_WEAPON:
											break;
										case ITEM_ARMOR:
											switch (item2->GetSubType())
											{
											case ARMOR_EAR:
											case ARMOR_WRIST:
											case ARMOR_NECK:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ????????"));
												return false;
											}
											break;

										default:
											return false;
									}

									std::stack<long> socket;

									for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
										socket.push(item2->GetSocket(i));

									int idx = ITEM_SOCKET_MAX_NUM - 1;

									while (socket.size() > 0)
									{
										if (socket.top() > 2 && socket.top() != ITEM_BROKEN_METIN_VNUM)
											break;

										idx--;
										socket.pop();
									}

									if (socket.size() == 0)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ????????"));
										return false;
									}

									LPITEM pItemReward = AutoGiveItem(socket.top());

									if (pItemReward != NULL)
									{
										item2->SetSocket(idx, 1);

										char buf[256+1];
										snprintf(buf, sizeof(buf), "%s(%u) %s(%u)",
												item2->GetName(), item2->GetID(), pItemReward->GetName(), pItemReward->GetID());
										LogManager::instance().ItemLog(this, item, "USE_DETACHMENT_ONE", buf);

										item->SetCount(item->GetCount() - 1);
									}
								}
								break;

							case 70201:   // ??????
							case 70202:   // ??????(????)
							case 70203:   // ??????(????)
							case 70204:   // ??????(??????)
							case 70205:   // ??????(????)
							case 70206:   // ??????(??????)
								{
									// NEW_HAIR_STYLE_ADD
									if (GetPart(PART_HAIR) >= 1001)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????????????? ?????? ?????? ????????????."));
									}
									// END_NEW_HAIR_STYLE_ADD
									else
									{
										quest::CQuestManager& q = quest::CQuestManager::instance();
										quest::PC* pPC = q.GetPC(GetPlayerID());

										if (pPC)
										{
											int last_dye_level = pPC->GetFlag("dyeing_hair.last_dye_level");

											if (last_dye_level == 0 ||
													last_dye_level+3 <= GetLevel() ||
													item->GetVnum() == 70201)
											{
												SetPart(PART_HAIR, item->GetVnum() - 70201);

												if (item->GetVnum() == 70201)
													pPC->SetFlag("dyeing_hair.last_dye_level", 0);
												else
													pPC->SetFlag("dyeing_hair.last_dye_level", GetLevel());

												item->SetCount(item->GetCount() - 1);
												UpdatePacket();
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ?????? ?????? ???? ???????? ?? ????????."), last_dye_level+3);
											}
										}
									}
								}
								break;

							case ITEM_NEW_YEAR_GREETING_VNUM:
								{
									DWORD dwBoxVnum = ITEM_NEW_YEAR_GREETING_VNUM;
									std::vector <DWORD> dwVnums;
									std::vector <DWORD> dwCounts;
									std::vector <LPITEM> item_gets;
									int count = 0;

									if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
									{
										for (int i = 0; i < count; i++)
										{
											if (dwVnums[i] == CSpecialItemGroup::GOLD)
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? %d ???? ????????????."), dwCounts[i]);
										}

										item->SetCount(item->GetCount() - 1);
									}
								}
								break;

							case ITEM_VALENTINE_ROSE:
							case ITEM_VALENTINE_CHOCOLATE:
								{
									DWORD dwBoxVnum = item->GetVnum();
									std::vector <DWORD> dwVnums;
									std::vector <DWORD> dwCounts;
									std::vector <LPITEM> item_gets(0);
									int count = 0;


									if (((item->GetVnum() == ITEM_VALENTINE_ROSE) && (SEX_MALE==GET_SEX(this))) ||
										((item->GetVnum() == ITEM_VALENTINE_CHOCOLATE) && (SEX_FEMALE==GET_SEX(this))))
									{
										// ?????? ???????? ?? ?? ????.
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???????? ?? ???????? ?? ?? ????????."));
										return false;
									}


									if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
										item->SetCount(item->GetCount()-1);
								}
								break;

							case ITEM_WHITEDAY_CANDY:
							case ITEM_WHITEDAY_ROSE:
								{
									DWORD dwBoxVnum = item->GetVnum();
									std::vector <DWORD> dwVnums;
									std::vector <DWORD> dwCounts;
									std::vector <LPITEM> item_gets(0);
									int count = 0;


									if (((item->GetVnum() == ITEM_WHITEDAY_CANDY) && (SEX_MALE==GET_SEX(this))) ||
										((item->GetVnum() == ITEM_WHITEDAY_ROSE) && (SEX_FEMALE==GET_SEX(this))))
									{
										// ?????? ???????? ?? ?? ????.
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???????? ?? ???????? ?? ?? ????????."));
										return false;
									}


									if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
										item->SetCount(item->GetCount()-1);
								}
								break;

							case 50011: // ????????
								{
									DWORD dwBoxVnum = 50011;
									std::vector <DWORD> dwVnums;
									std::vector <DWORD> dwCounts;
									std::vector <LPITEM> item_gets(0);
									int count = 0;

									if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
									{
										for (int i = 0; i < count; i++)
										{
											char buf[50 + 1];
											snprintf(buf, sizeof(buf), "%u %u", dwVnums[i], dwCounts[i]);
											LogManager::instance().ItemLog(this, item, "MOONLIGHT_GET", buf);

											//ITEM_MANAGER::instance().RemoveItem(item);
											item->SetCount(item->GetCount() - 1);

											switch (dwVnums[i])
											{
											case CSpecialItemGroup::GOLD:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? %d ???? ????????????."), dwCounts[i]);
												break;

											case CSpecialItemGroup::EXP:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ?????? ???? ????????."));
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d?? ???????? ????????????."), dwCounts[i]);
												break;

											case CSpecialItemGroup::MOB:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ????????????!"));
												break;

											case CSpecialItemGroup::SLOW:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???????? ?????? ????????????!"));
												break;

											case CSpecialItemGroup::DRAIN_HP:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ??????????????! ???????? ????????????."));
												break;

											case CSpecialItemGroup::POISON:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???? ???????? ????????!"));
												break;
#ifdef __WOLFMAN__
											case CSpecialItemGroup::BLEEDING:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???? ???????? ????????!"));
												break;
#endif
											case CSpecialItemGroup::MOB_GROUP:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ????????????!"));
												break;

											default:
												if (item_gets[i])
												{
													if (dwCounts[i] > 1)
														ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? %s ?? %d ?? ??????????."), item_gets[i]->GetMultiName(), dwCounts[i]);
													else
														ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? %s ?? ??????????."), item_gets[i]->GetMultiName());
												}
												break;
											}
										}
									}
									else
									{
										ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("???????? ???? ?? ??????????."));
										return false;
									}
								}
								break;

							case ITEM_GIVE_STAT_RESET_COUNT_VNUM:
								{
									//PointChange(POINT_GOLD, -iCost);
									PointChange(POINT_STAT_RESET_COUNT, 1);
									item->SetCount(item->GetCount()-1);
								}
								break;

							case 50107:
								{
									if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
										return false;
									}
#ifdef __NEWSTUFF__
									else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
										return false;
									}
#endif

									EffectPacket(SE_CHINA_FIREWORK);
#ifdef __FIREWORK_STUN__
									// ???? ?????? ????????
									AddAffect(AFFECT_CHINA_FIREWORK, POINT_STUN_PCT, 30, AFF_CHINA_FIREWORK, 5*60, 0, true);
#endif
									item->SetCount(item->GetCount()-1);
								}
								break;

							case 50108:
								{
									if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
										return false;
									}
#ifdef __NEWSTUFF__
									else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
										return false;
									}
#endif

									EffectPacket(SE_SPIN_TOP);
#ifdef __FIREWORK_STUN__
									// ???? ?????? ????????
									AddAffect(AFFECT_CHINA_FIREWORK, POINT_STUN_PCT, 30, AFF_CHINA_FIREWORK, 5*60, 0, true);
#endif
									item->SetCount(item->GetCount()-1);
								}
								break;

							case ITEM_WONSO_BEAN_VNUM:
								PointChange(POINT_HP, GetMaxHP() - GetHP());
								item->SetCount(item->GetCount()-1);
								break;

							case ITEM_WONSO_SUGAR_VNUM:
								PointChange(POINT_SP, GetMaxSP() - GetSP());
								item->SetCount(item->GetCount()-1);
								break;

							case ITEM_WONSO_FRUIT_VNUM:
								PointChange(POINT_STAMINA, GetMaxStamina()-GetStamina());
								item->SetCount(item->GetCount()-1);
								break;

							case ITEM_ELK_VNUM: // ????????
								{
									int iGold = item->GetSocket(0);
									ITEM_MANAGER::instance().RemoveItem(item);
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? %d ???? ????????????."), iGold);
									PointChange(POINT_GOLD, iGold);
								}
								break;

							case 27995:
								{
								}
								break;

							case 71092 : // ???? ?????? ????
								{
									if (m_pkChrTarget != NULL)
									{
										if (m_pkChrTarget->IsPolymorphed())
										{
											m_pkChrTarget->SetPolymorph(0);
											m_pkChrTarget->RemoveAffect(AFFECT_POLYMORPH);
										}
									}
									else
									{
										if (IsPolymorphed())
										{
											SetPolymorph(0);
											RemoveAffect(AFFECT_POLYMORPH);
										}
									}
								}
								break;

#ifdef __ANCIENT_ATTRIBUTE__
							case 71051 : // ??????
								{
									// ????, ??????, ?????? ?????? ????????
									LPITEM item2;

									if (!IsValidItemPosition(DestCell) || !(item2 = GetInventoryItem(wDestCell)))
										return false;

									if (ITEM_COSTUME == item2->GetType()) // @fixme124
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ?? ???? ????????????."));
										return false;
									}

									if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
										return false;

									if (item2->GetAttributeSetIndex() == -1)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ?? ???? ????????????."));
										return false;
									}

									if (item2->AddRareAttribute() == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ?????? ???? ??????????"));

										int iAddedIdx = item2->GetRareAttrCount() + 4;
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());

										LogManager::instance().ItemLog(
												GetPlayerID(),
												item2->GetAttributeType(iAddedIdx),
												item2->GetAttributeValue(iAddedIdx),
												item->GetID(),
												"ADD_RARE_ATTR",
												buf,
												GetDesc()->GetHostName(),
												item->GetOriginalVnum());

										item->SetCount(item->GetCount() - 1);
									}
									else
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ?? ?????????? ?????? ?????? ?? ????????"));
								}
								break;

							case 71052 : // ??????
								{
									// ????, ??????, ?????? ?????? ????????
									LPITEM item2;

									if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
										return false;

									if (ITEM_COSTUME == item2->GetType()) // @fixme124
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ?? ???? ????????????."));
										return false;
									}

									if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
										return false;

									if (item2->GetAttributeSetIndex() == -1)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ?? ???? ????????????."));
										return false;
									}

									if (item2->GetAttributeCount() == 5)
									{
										item2->RemoveAttributeAt(4);
										item2->AddAttribute();
										item->SetCount(item->GetCount() - 1);
									}
									else if (item2->GetAttributeCount() == 4)
									{
										item2->AddAttribute();
										item->SetCount(item->GetCount()-1);
									}
									else
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???? ?????? ????????"));
								}
								break;
#endif

							case ITEM_AUTO_HP_RECOVERY_S:
							case ITEM_AUTO_HP_RECOVERY_M:
							case ITEM_AUTO_HP_RECOVERY_L:
							case ITEM_AUTO_HP_RECOVERY_X:
							case ITEM_AUTO_SP_RECOVERY_S:
							case ITEM_AUTO_SP_RECOVERY_M:
							case ITEM_AUTO_SP_RECOVERY_L:
							case ITEM_AUTO_SP_RECOVERY_X:
							// ?????????????? ?????? ???? ?? ???????? ??????...
							// ?????? ???? ???? ????. ???? ?????? ???????? ????????.
							case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
							case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
							case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
							case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
							case FUCKING_BRAZIL_ITEM_AUTO_SP_RECOVERY_S:
							case FUCKING_BRAZIL_ITEM_AUTO_HP_RECOVERY_S:
								{
									if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???????? ?? ????????."));
										return false;
									}
#ifdef __NEWSTUFF__
									else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
										return false;
									}
#endif

									EAffectTypes type = AFFECT_NONE;
									bool isSpecialPotion = false;

									switch (item->GetVnum())
									{
										case ITEM_AUTO_HP_RECOVERY_X:
											isSpecialPotion = true;

										case ITEM_AUTO_HP_RECOVERY_S:
										case ITEM_AUTO_HP_RECOVERY_M:
										case ITEM_AUTO_HP_RECOVERY_L:
										case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
										case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
										case FUCKING_BRAZIL_ITEM_AUTO_HP_RECOVERY_S:
											type = AFFECT_AUTO_HP_RECOVERY;
											break;

										case ITEM_AUTO_SP_RECOVERY_X:
											isSpecialPotion = true;

										case ITEM_AUTO_SP_RECOVERY_S:
										case ITEM_AUTO_SP_RECOVERY_M:
										case ITEM_AUTO_SP_RECOVERY_L:
										case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
										case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
										case FUCKING_BRAZIL_ITEM_AUTO_SP_RECOVERY_S:
											type = AFFECT_AUTO_SP_RECOVERY;
											break;
									}

									if (AFFECT_NONE == type)
										break;

									if (item->GetCount() > 1)
									{
										int pos = GetEmptyInventory(item->GetSize());

										if (-1 == pos)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ?? ?????? ????????."));
											break;
										}

										item->SetCount( item->GetCount() - 1 );

										LPITEM item2 = ITEM_MANAGER::instance().CreateItem( item->GetVnum(), 1 );
										item2->AddToCharacter(this, TItemPos(INVENTORY, pos), true);

										if (item->GetSocket(1) != 0)
										{
											item2->SetSocket(1, item->GetSocket(1));
										}

										item = item2;
									}

									CAffect* pAffect = FindAffect( type );

									if (NULL == pAffect)
									{
										EPointTypes bonus = POINT_NONE;

										if (true == isSpecialPotion)
										{
											if (type == AFFECT_AUTO_HP_RECOVERY)
											{
												bonus = POINT_MAX_HP_PCT;
											}
											else if (type == AFFECT_AUTO_SP_RECOVERY)
											{
												bonus = POINT_MAX_SP_PCT;
											}
										}

										AddAffect( type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);

										item->Lock(true);
										item->SetSocket(0, true);

										AutoRecoveryItemProcess( type );
									}
									else
									{
										if (item->GetID() == pAffect->dwFlag)
										{
											RemoveAffect( pAffect );

											item->Lock(false);
											item->SetSocket(0, false);
										}
										else
										{
											LPITEM old = FindItemByID( pAffect->dwFlag );

											if (NULL != old)
											{
												old->Lock(false);
												old->SetSocket(0, false);
											}

											RemoveAffect( pAffect );

											EPointTypes bonus = POINT_NONE;

											if (true == isSpecialPotion)
											{
												if (type == AFFECT_AUTO_HP_RECOVERY)
												{
													bonus = POINT_MAX_HP_PCT;
												}
												else if (type == AFFECT_AUTO_SP_RECOVERY)
												{
													bonus = POINT_MAX_SP_PCT;
												}
											}

											AddAffect( type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);

											item->Lock(true);
											item->SetSocket(0, true);

											AutoRecoveryItemProcess( type );
										}
									}
								}
								break;
						}
						break;

					case USE_CLEAR:
						{
							switch (item->GetVnum())
							{
#ifdef __WOLFMAN__
								case 27124: // Bandage
									RemoveBleeding();
									break;
#endif
								case 27874: // Grilled Perch
								default:
									RemoveBadAffect();
									break;
							}
							item->SetCount(item->GetCount() - 1);
						}
						break;

					case USE_INVISIBILITY:
						{
							if (item->GetVnum() == 70026)
							{
								quest::CQuestManager& q = quest::CQuestManager::instance();
								quest::PC* pPC = q.GetPC(GetPlayerID());

								if (pPC != NULL)
								{
									int last_use_time = pPC->GetFlag("mirror_of_disapper.last_use_time");

									if (get_global_time() - last_use_time < 10*60)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?? ????????."));
										return false;
									}

									pPC->SetFlag("mirror_of_disapper.last_use_time", get_global_time());
								}
							}

							AddAffect(AFFECT_INVISIBILITY, POINT_NONE, 0, AFF_INVISIBILITY, 300, 0, true);
							item->SetCount(item->GetCount() - 1);
						}
						break;

					case USE_POTION_NODELAY:
						{
							if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
							{
								if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit") > 0)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???????? ?? ????????."));
									return false;
								}

								switch (item->GetVnum())
								{
									case 70020 :
									case 71018 :
									case 71019 :
									case 71020 :
										if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count") < 10000)
										{
											if (m_nPotionLimit <= 0)
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? ??????????????."));
												return false;
											}
										}
										break;

									default :
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???????? ?? ????????."));
										return false;
								}
							}
#ifdef __NEWSTUFF__
							else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
								return false;
							}
#endif

							bool used = false;

							if (item->GetValue(0) != 0) // HP ?????? ????
							{
								if (GetHP() < GetMaxHP())
								{
									PointChange(POINT_HP, item->GetValue(0) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
									EffectPacket(SE_HPUP_RED);
									used = TRUE;
								}
							}

							if (item->GetValue(1) != 0)	// SP ?????? ????
							{
								if (GetSP() < GetMaxSP())
								{
									PointChange(POINT_SP, item->GetValue(1) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
									EffectPacket(SE_SPUP_BLUE);
									used = TRUE;
								}
							}

							if (item->GetValue(3) != 0) // HP % ????
							{
								if (GetHP() < GetMaxHP())
								{
									PointChange(POINT_HP, item->GetValue(3) * GetMaxHP() / 100);
									EffectPacket(SE_HPUP_RED);
									used = TRUE;
								}
							}

							if (item->GetValue(4) != 0) // SP % ????
							{
								if (GetSP() < GetMaxSP())
								{
									PointChange(POINT_SP, item->GetValue(4) * GetMaxSP() / 100);
									EffectPacket(SE_SPUP_BLUE);
									used = TRUE;
								}
							}

							if (used)
							{
								if (item->GetVnum() == 50085 || item->GetVnum() == 50086)
								{
									SetUseSeedOrMoonBottleTime();
								}
								if (GetDungeon())
									GetDungeon()->UsePotion(this);

								if (GetWarMap())
									GetWarMap()->UsePotion(this, item);

								m_nPotionLimit--;

								//RESTRICT_USE_SEED_OR_MOONBOTTLE
								item->SetCount(item->GetCount() - 1);
								//END_RESTRICT_USE_SEED_OR_MOONBOTTLE
							}
						}
						break;

					case USE_POTION:
						if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
						{
							if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit") > 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???????? ?? ????????."));
								return false;
							}

							switch (item->GetVnum())
							{
								case 27001 :
								case 27002 :
								case 27003 :
								case 27004 :
								case 27005 :
								case 27006 :
									if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count") < 10000)
									{
										if (m_nPotionLimit <= 0)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? ??????????????."));
											return false;
										}
									}
									break;

								default :
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????????? ???????? ?? ????????."));
									return false;
							}
						}
#ifdef __NEWSTUFF__
						else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
							return false;
						}
#endif

						if (item->GetValue(1) != 0)
						{
							if (GetPoint(POINT_SP_RECOVERY) + GetSP() >= GetMaxSP())
							{
								return false;
							}

							PointChange(POINT_SP_RECOVERY, item->GetValue(1) * MIN(200, (100 + GetPoint(POINT_POTION_BONUS))) / 100);
							StartAffectEvent();
							EffectPacket(SE_SPUP_BLUE);
						}

						if (item->GetValue(0) != 0)
						{
							if (GetPoint(POINT_HP_RECOVERY) + GetHP() >= GetMaxHP())
							{
								return false;
							}

							PointChange(POINT_HP_RECOVERY, item->GetValue(0) * MIN(200, (100 + GetPoint(POINT_POTION_BONUS))) / 100);
							StartAffectEvent();
							EffectPacket(SE_HPUP_RED);
						}

						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						item->SetCount(item->GetCount() - 1);
						m_nPotionLimit--;
						break;

					case USE_POTION_CONTINUE:
						{
							if (item->GetValue(0) != 0)
							{
								AddAffect(AFFECT_HP_RECOVER_CONTINUE, POINT_HP_RECOVER_CONTINUE, item->GetValue(0), 0, item->GetValue(2), 0, true);
							}
							else if (item->GetValue(1) != 0)
							{
								AddAffect(AFFECT_SP_RECOVER_CONTINUE, POINT_SP_RECOVER_CONTINUE, item->GetValue(1), 0, item->GetValue(2), 0, true);
							}
							else
								return false;
						}

						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						item->SetCount(item->GetCount() - 1);
						break;

					case USE_ABILITY_UP:
						{
							switch (item->GetValue(0))
							{
								case APPLY_MOV_SPEED:
									AddAffect(AFFECT_MOV_SPEED, POINT_MOV_SPEED, item->GetValue(2), AFF_MOV_SPEED_POTION, item->GetValue(1), 0, true);
#ifdef __EFFECT_EXTRAPOT__
									EffectPacket(SE_DXUP_PURPLE);
#endif
									break;

								case APPLY_ATT_SPEED:
									AddAffect(AFFECT_ATT_SPEED, POINT_ATT_SPEED, item->GetValue(2), AFF_ATT_SPEED_POTION, item->GetValue(1), 0, true);
#ifdef __EFFECT_EXTRAPOT__
									EffectPacket(SE_SPEEDUP_GREEN);
#endif
									break;

								case APPLY_STR:
									AddAffect(AFFECT_STR, POINT_ST, item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_DEX:
									AddAffect(AFFECT_DEX, POINT_DX, item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_CON:
									AddAffect(AFFECT_CON, POINT_HT, item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_INT:
									AddAffect(AFFECT_INT, POINT_IQ, item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_CAST_SPEED:
									AddAffect(AFFECT_CAST_SPEED, POINT_CASTING_SPEED, item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_ATT_GRADE_BONUS:
									AddAffect(AFFECT_ATT_GRADE, POINT_ATT_GRADE_BONUS,
											item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_DEF_GRADE_BONUS:
									AddAffect(AFFECT_DEF_GRADE, POINT_DEF_GRADE_BONUS,
											item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;
							}
						}

#ifdef __BATTLE_FIELD__
						if (item->GetVnum() == 27125)
						{
							if (FindAffect(AFFECT_BATTLE_POTION))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You already used that potion."));
								return false;
							}
					
							if (CBattleField::instance().IsBattleFieldMapIndex(GetMapIndex()))
							{
								int iMultiply = 0;
								if (GetLevel() <= 64)
									iMultiply = 4;
								else if (GetLevel() > 64 && GetLevel() < 90)
									iMultiply = 3;
								else if (GetLevel() >= 90)
									iMultiply = 2;

								int iAttackDefenceValue = (gPlayerMaxLevel - (GetLevel() - 50)) * iMultiply;
								
								AddAffect(AFFECT_BATTLE_POTION, POINT_MAX_HP, 50000, 0, INFINITE_AFFECT_DURATION, 0, false);
								AddAffect(AFFECT_BATTLE_POTION, POINT_ATT_GRADE_BONUS, iAttackDefenceValue, 0, INFINITE_AFFECT_DURATION, 0, false);
								AddAffect(AFFECT_BATTLE_POTION, POINT_DEF_GRADE_BONUS, iAttackDefenceValue, 0, INFINITE_AFFECT_DURATION, 0, false);
								
								EffectPacket(SE_EFFECT_BATTLE_POTION);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can use this item just in battle zone map."));
								return false;
							}
						}
#endif

						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						item->SetCount(item->GetCount() - 1);
						break;

					case USE_TALISMAN:
						{
							const int TOWN_PORTAL	= 1;
							const int MEMORY_PORTAL = 2;


							// gm_guild_build, oxevent ?????? ?????? ?????????? ?? ?????????? ????
							if (GetMapIndex() == 200 || GetMapIndex() == 113)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? ?????? ?? ????????."));
								return false;
							}

							if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
								return false;
							}
#ifdef __NEWSTUFF__
							else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?? ???? ??????????."));
								return false;
							}
#endif

							if (m_pkWarpEvent)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ???????????? ???????? ???????? ????????"));
								return false;
							}

							// CONSUME_LIFE_WHEN_USE_WARP_ITEM
							int consumeLife = CalculateConsume(this);

							if (consumeLife < 0)
								return false;
							// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

							if (item->GetValue(0) == TOWN_PORTAL) // ??????
							{
								if (item->GetSocket(0) == 0)
								{
									if (!GetDungeon())
										if (!GiveRecallItem(item))
											return false;

									PIXEL_POSITION posWarp;

									if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(GetMapIndex(), GetEmpire(), posWarp))
									{
										// CONSUME_LIFE_WHEN_USE_WARP_ITEM
										PointChange(POINT_HP, -consumeLife, false);
										// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

										WarpSet(posWarp.x, posWarp.y);
									}
									else
									{
										sys_err("CHARACTER::UseItem : cannot find spawn position (name %s, %d x %d)", GetName(), GetX(), GetY());
									}
								}
								else
								{
									ProcessRecallItem(item);
								}
							}
							else if (item->GetValue(0) == MEMORY_PORTAL) // ??????????
							{
								if (item->GetSocket(0) == 0)
								{
									if (GetDungeon())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? %s%s ?????? ?? ????????."),
												item->GetMultiName(),
												"");
										return false;
									}

									if (!GiveRecallItem(item))
										return false;
								}
								else
								{
									// CONSUME_LIFE_WHEN_USE_WARP_ITEM
									PointChange(POINT_HP, -consumeLife, false);
									// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

									ProcessRecallItem(item);
								}
							}
						}
						break;

					case USE_TUNING:
					case USE_DETACHMENT:
						{
							LPITEM item2;

							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
								return false;

// #ifdef __COSTUME_ACCE__
							// if (item->GetValue(0) == ACCE_CLEAN_ATTR_VALUE0)
							// {
								// if (!CleanAcceAttr(item, item2))
									// return false;
								
								// return true;
							// }
// #endif
// #ifdef __CHANGELOOK__
							// if (item->GetValue(0) == CL_CLEAN_ATTR_VALUE0)
							// {
								// if (!CleanTransmutation(item, item2))
									// return false;
								
								// return true;
							// }
// #endif
							if (item2->GetVnum() >= 28330 && item2->GetVnum() <= 28343) // ????+3
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("+3 ?????? ?? ?????????? ?????? ?? ????????"));
								return false;
							}

							if (item2->GetVnum() >= 28430 && item2->GetVnum() <= 28443)  // ????+4
							{
								if (item->GetVnum() == 71056) // ??????????
								{
									RefineItem(item, item2);
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?? ?????????? ?????? ?? ????????"));
								}
							}
#ifdef __RITUEL_STONE__
							if (item->GetValue(0) == RITUALS_SCROLL)
							{
								if (item2->GetLevelLimit() <= 80)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("you can only upgrade items above level"));
									return false;
								}
								else
									RefineItem(item, item2);
							}
#endif
							else
							{
								RefineItem(item, item2);
							}
						}
						break;

						//  ACCESSORY_REFINE & ADD/CHANGE_ATTRIBUTES
					case USE_PUT_INTO_BELT_SOCKET:
					case USE_PUT_INTO_RING_SOCKET:
					case USE_PUT_INTO_ACCESSORY_SOCKET:
					case USE_ADD_ACCESSORY_SOCKET:
					case USE_CLEAN_SOCKET:
					case USE_CHANGE_ATTRIBUTE:
					case USE_CHANGE_ATTRIBUTE2:
					case USE_ADD_ATTRIBUTE:
					case USE_ADD_ATTRIBUTE2:
						{
							LPITEM item2;
							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item2->IsEquipped())
							{
								BuffOnAttr_RemoveBuffsFromItem(item2);
							}

							// [NOTE] ?????? ?????????? ?????? ???? ?????? ???? ?????? ????????, ???????? ?????? ?????????? ?????? ??????.
							// ???? ANTI_CHANGE_ATTRIBUTE ???? ?????? Flag?? ???????? ???? ???????? ???????? ?????? ?? ?? ?????? ?? ????????????
							// ?????? ?????????? ?????? ???? ???????? ???? ?????? ????... -_-
							if (ITEM_COSTUME == item2->GetType())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ?? ???? ????????????."));
								return false;
							}

							if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
								return false;

							switch (item->GetSubType())
							{
								case USE_CLEAN_SOCKET:
									{
										int i;
										for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
										{
											if (item2->GetSocket(i) == ITEM_BROKEN_METIN_VNUM)
												break;
										}

										if (i == ITEM_SOCKET_MAX_NUM)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ???????? ????????."));
											return false;
										}

										int j = 0;

										for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
										{
											if (item2->GetSocket(i) != ITEM_BROKEN_METIN_VNUM && item2->GetSocket(i) != 0)
												item2->SetSocket(j++, item2->GetSocket(i));
										}

										for (; j < ITEM_SOCKET_MAX_NUM; ++j)
										{
											if (item2->GetSocket(j) > 0)
												item2->SetSocket(j, 1);
										}

										{
											char buf[21];
											snprintf(buf, sizeof(buf), "%u", item2->GetID());
											LogManager::instance().ItemLog(this, item, "CLEAN_SOCKET", buf);
										}

										item->SetCount(item->GetCount() - 1);

									}
									break;

								case USE_CHANGE_ATTRIBUTE :
								case USE_CHANGE_ATTRIBUTE2 : // @fixme123
									if (item2->GetAttributeSetIndex() == -1)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ?? ???? ????????????."));
										return false;
									}

									if (item2->GetAttributeCount() == 0)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ????????."));
										return false;
									}

#ifdef __SOULBIND__
									if (item2->IsSealed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't change attr an sealbind item."));
										return false;
									}
#endif

									if ((GM_PLAYER == GetGMLevel()) && (g_dwItemBonusChangeTime > 0))
									{
										//
										// Event Flag ?? ???? ?????? ?????? ???? ?????? ?? ???????? ???? ?????? ?????? ???????? ????????
										// ?????? ?????? ???????? ???? ?????????? ???? ?????? ?????? ????.
										//

										// DWORD dwChangeItemAttrCycle = quest::CQuestManager::instance().GetEventFlag(msc_szChangeItemAttrCycleFlag);
										// if (dwChangeItemAttrCycle < msc_dwDefaultChangeItemAttrCycle)
											// dwChangeItemAttrCycle = msc_dwDefaultChangeItemAttrCycle;
										DWORD dwChangeItemAttrCycle = g_dwItemBonusChangeTime;

										quest::PC* pPC = quest::CQuestManager::instance().GetPC(GetPlayerID());

										if (pPC)
										{
											DWORD dwNowSec = get_global_time();

											DWORD dwLastChangeItemAttrSec = pPC->GetFlag(msc_szLastChangeItemAttrFlag);

											if (dwLastChangeItemAttrSec + dwChangeItemAttrCycle > dwNowSec)
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? %d?? ???????? ???? ?????? ?? ????????.(%d ?? ????)"),
														dwChangeItemAttrCycle, dwChangeItemAttrCycle - (dwNowSec - dwLastChangeItemAttrSec));
												return false;
											}

											pPC->SetFlag(msc_szLastChangeItemAttrFlag, dwNowSec);
										}
									}

									if (item->GetSubType() == USE_CHANGE_ATTRIBUTE2)
									{
										int aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] =
										{
											0, 0, 30, 40, 3
										};

										item2->ChangeRareAttribute();
									}
									else if (item->GetVnum() == 76014)
									{
										int aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] =
										{
											0, 10, 50, 39, 1
										};

										item2->ChangeAttribute(aiChangeProb);
									}

									else
									{
										// ?????? ????????
										// ?????? ?????? ???? ???????? ???? ???? ??????.
										if (item->GetVnum() == 71151 || item->GetVnum() == 76023)
										{
											if ((item2->GetType() == ITEM_WEAPON)
												|| (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_BODY))
											{
												bool bCanUse = true;
												for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
												{
													if (item2->GetLimitType(i) == LIMIT_LEVEL && item2->GetLimitValue(i) > 40)
													{
														bCanUse = false;
														break;
													}
												}
												if (false == bCanUse)
												{
													ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? ???? ?????? ????????????."));
													break;
												}
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???????? ???? ??????????."));
												break;
											}
										}
										item2->ChangeAttribute();
									}

									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ??????????????."));
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());
										LogManager::instance().ItemLog(this, item, "CHANGE_ATTRIBUTE", buf);
									}

									item->SetCount(item->GetCount() - 1);
									break;

								case USE_ADD_ATTRIBUTE :
									if (item2->GetAttributeSetIndex() == -1)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ?? ???? ????????????."));
										return false;
									}
#ifdef __SOULBIND__
									if (item2->IsSealed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't change attr an sealbind item."));
										return false;
									}
#endif
									if (item2->GetAttributeCount() < 4)
									{
										// ?????? ????????
										// ?????? ?????? ???? ???????? ???? ???? ??????.
										if (item->GetVnum() == 71152 || item->GetVnum() == 76024)
										{
											if ((item2->GetType() == ITEM_WEAPON)
												|| (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_BODY))
											{
												bool bCanUse = true;
												for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
												{
													if (item2->GetLimitType(i) == LIMIT_LEVEL && item2->GetLimitValue(i) > 40)
													{
														bCanUse = false;
														break;
													}
												}
												if (false == bCanUse)
												{
													ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? ???? ?????? ????????????."));
													break;
												}
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???????? ???? ??????????."));
												break;
											}
										}
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());

										if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
										{
											item2->AddAttribute();
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ??????????????."));

											int iAddedIdx = item2->GetAttributeCount() - 1;
											LogManager::instance().ItemLog(
													GetPlayerID(),
													item2->GetAttributeType(iAddedIdx),
													item2->GetAttributeValue(iAddedIdx),
													item->GetID(),
													"ADD_ATTRIBUTE_SUCCESS",
													buf,
													GetDesc()->GetHostName(),
													item->GetOriginalVnum());
										}
										else
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ??????????????."));
											LogManager::instance().ItemLog(this, item, "ADD_ATTRIBUTE_FAIL", buf);
										}

										item->SetCount(item->GetCount() - 1);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?? ???????? ???????? ?????? ?????? ?? ????????."));
									}
									break;

								case USE_ADD_ATTRIBUTE2 :
									// ?????? ????
									// ?????????? ???? ?????? 4?? ???? ???? ???????? ?????? ?????? ?????? ?? ????????.
									if (item2->GetAttributeSetIndex() == -1)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ?? ???? ????????????."));
										return false;
									}
#ifdef __SOULBIND__
									if (item2->IsSealed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't change attr an sealbind item."));
										return false;
									}
#endif
									// ?????? ???? 4?? ???? ?????? ???? ?????? ???? ????????.
									if (item2->GetAttributeCount() == 4)
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());

										if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
										{
											item2->AddAttribute();
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ??????????????."));

											int iAddedIdx = item2->GetAttributeCount() - 1;
											LogManager::instance().ItemLog(
													GetPlayerID(),
													item2->GetAttributeType(iAddedIdx),
													item2->GetAttributeValue(iAddedIdx),
													item->GetID(),
													"ADD_ATTRIBUTE2_SUCCESS",
													buf,
													GetDesc()->GetHostName(),
													item->GetOriginalVnum());
										}
										else
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ??????????????."));
											LogManager::instance().ItemLog(this, item, "ADD_ATTRIBUTE2_FAIL", buf);
										}

										item->SetCount(item->GetCount() - 1);
									}
									else if (item2->GetAttributeCount() == 5)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ?? ???????? ???????? ?????? ?????? ?? ????????."));
									}
									else if (item2->GetAttributeCount() < 4)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????????? ???????? ?????? ???????? ??????."));
									}
									else
									{
										// wtf ?!
										sys_err("ADD_ATTRIBUTE2 : Item has wrong AttributeCount(%d)", item2->GetAttributeCount());
									}
									break;

								case USE_ADD_ACCESSORY_SOCKET:
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());

										if (item2->IsAccessoryForSocket())
										{
											if (item2->GetAccessorySocketMaxGrade() < ITEM_ACCESSORY_SOCKET_MAX_NUM)
											{
#ifdef __ADDSTONE_FAILURE__
												if (number(1, 100) <= 50)
#else
												if (1)
#endif
												{
													item2->SetAccessorySocketMaxGrade(item2->GetAccessorySocketMaxGrade() + 1);
													ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????????? ??????????????."));
													LogManager::instance().ItemLog(this, item, "ADD_SOCKET_SUCCESS", buf);
												}
												else
												{
													ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ??????????????."));
													LogManager::instance().ItemLog(this, item, "ADD_SOCKET_FAIL", buf);
												}

												item->SetCount(item->GetCount() - 1);
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????????? ?????? ?????? ?????? ?????? ????????."));
											}
										}
										else
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ?????????? ?????? ?????? ?? ???? ????????????."));
										}
									}
									break;

								case USE_PUT_INTO_BELT_SOCKET:
								case USE_PUT_INTO_ACCESSORY_SOCKET:
									if (item2->IsAccessoryForSocket() && item->CanPutInto(item2))
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());

										if (item2->GetAccessorySocketGrade() < item2->GetAccessorySocketMaxGrade())
										{
											if (number(1, 100) <= aiAccessorySocketPutPct[item2->GetAccessorySocketGrade()])
											{
												item2->SetAccessorySocketGrade(item2->GetAccessorySocketGrade() + 1);
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ??????????????."));
												LogManager::instance().ItemLog(this, item, "PUT_SOCKET_SUCCESS", buf);
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ??????????????."));
												LogManager::instance().ItemLog(this, item, "PUT_SOCKET_FAIL", buf);
											}

											item->SetCount(item->GetCount() - 1);
										}
										else
										{
											if (item2->GetAccessorySocketMaxGrade() == 0)
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????????? ?????????? ?????? ??????????????."));
											else if (item2->GetAccessorySocketMaxGrade() < ITEM_ACCESSORY_SOCKET_MAX_NUM)
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????????? ?????? ?????? ?????? ????????."));
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????????? ?????? ??????????????."));
											}
											else
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????????? ?????? ?????? ?????? ?? ????????."));
										}
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?????? ?? ????????."));
									}
									break;
							}
							if (item2->IsEquipped())
							{
								BuffOnAttr_AddBuffsFromItem(item2);
							}
						}
						break;
						//  END_OF_ACCESSORY_REFINE & END_OF_ADD_ATTRIBUTES & END_OF_CHANGE_ATTRIBUTES

#ifdef __MOVE_COSTUME_ATTR__
					case USE_ADD_COSTUME_ATTRIBUTE:
					case USE_RESET_COSTUME_ATTR:
						{
							LPITEM item2;
							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;
							
							if (item2->IsEquipped())
								BuffOnAttr_RemoveBuffsFromItem(item2);
							
							if (item2->IsExchanging() || item2->IsEquipped())
								return false;

#ifdef __SOULBIND__
							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't change attr an sealbind item."));
								return false;
							}
#endif

							if (item2->IsBasicItem())
								return false;

							if (item2->GetType() != ITEM_COSTUME)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can use this just on costumes."));
								return false;
							}
							
							if (item2->GetAttributeCount() == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This costume doesn't have any bonus."));
								return false;
							}
							
							switch (item->GetSubType())
							{
								case USE_ADD_COSTUME_ATTRIBUTE:
									{
										int aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] = {100, 100, 100, 100, 100};
										item2->ChangeAttribute(aiChangeProb);
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You enchant succesfully the costume."));
										{
											char buf[21];
											snprintf(buf, sizeof(buf), "%u", item2->GetID());
											LogManager::instance().ItemLog(this, item, "USE_ADD_COSTUME_ATTRIBUTE", buf);
										}
										
										item->SetCount(item->GetCount() - 1);
									}
									break;
								case USE_RESET_COSTUME_ATTR:
									{
										item2->ClearAttribute();
										int iCostumePercent = number(1, 3);
										for (int i = 0; i < iCostumePercent; ++i)
										{
											item2->AddAttribute();
										}
										
										int aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] = {100, 100, 100, 100, 100};
										item2->ChangeAttribute(aiChangeProb);
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You transform succesfully the costume."));
										{
											char buf[21];
											snprintf(buf, sizeof(buf), "%u", item2->GetID());
											LogManager::instance().ItemLog(this, item, "USE_RESET_COSTUME_ATTR", buf);
										}
										
										item->SetCount(item->GetCount() - 1);
									}
									break;
							}
						}
						break;
#endif

					case USE_BAIT:
						{

							if (m_pkFishingEvent)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???? ?????? ???????? ?? ????????."));
								return false;
							}

							LPITEM weapon = GetWear(WEAR_WEAPON);

							if (!weapon || weapon->GetType() != ITEM_ROD)
								return false;

							if (weapon->GetSocket(2))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? ?????? ???? %s?? ????????."), item->GetMultiName());
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? %s?? ?????? ????????."), item->GetMultiName());
							}

							weapon->SetSocket(2, item->GetValue(0));
							item->SetCount(item->GetCount() - 1);
						}
						break;

					case USE_MOVE:
					case USE_TREASURE_BOX:
					case USE_MONEYBAG:
						break;

					case USE_AFFECT :
						{
							if (FindAffect(item->GetValue(0), aApplyInfo[item->GetValue(1)].bPointType))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ????????."));
							}
							else
							{
								AddAffect(item->GetValue(0), aApplyInfo[item->GetValue(1)].bPointType, item->GetValue(2), 0, item->GetValue(3), 0, false);
								item->SetCount(item->GetCount() - 1);
							}
						}
						break;

					case USE_CREATE_STONE:
						AutoGiveItem(number(28000, 28013));
						item->SetCount(item->GetCount() - 1);
						break;

					// ???? ???? ?????? ?????? ????
					case USE_RECIPE :
						{
							LPITEM pSource1 = FindSpecifyItem(item->GetValue(1));
							DWORD dwSourceCount1 = item->GetValue(2);

							LPITEM pSource2 = FindSpecifyItem(item->GetValue(3));
							DWORD dwSourceCount2 = item->GetValue(4);

							if (dwSourceCount1 != 0)
							{
								if (pSource1 == NULL)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ?????? ??????????."));
									return false;
								}
							}

							if (dwSourceCount2 != 0)
							{
								if (pSource2 == NULL)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ?????? ??????????."));
									return false;
								}
							}

							if (pSource1 != NULL)
							{
								if (pSource1->GetCount() < dwSourceCount1)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("????(%s)?? ??????????."), pSource1->GetMultiName());
									return false;
								}

								pSource1->SetCount(pSource1->GetCount() - dwSourceCount1);
							}

							if (pSource2 != NULL)
							{
								if (pSource2->GetCount() < dwSourceCount2)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("????(%s)?? ??????????."), pSource2->GetMultiName());
									return false;
								}

								pSource2->SetCount(pSource2->GetCount() - dwSourceCount2);
							}

							LPITEM pBottle = FindSpecifyItem(50901);

							if (!pBottle || pBottle->GetCount() < 1)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???? ??????????."));
								return false;
							}

							pBottle->SetCount(pBottle->GetCount() - 1);

							if (number(1, 100) > item->GetValue(5))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ????????????."));
								return false;
							}

							AutoGiveItem(item->GetValue(0));
						}
						break;
				}
			}
			break;

		case ITEM_METIN:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
					return false;

				if (item2->GetType() == ITEM_PICK)
					return false;

				if (item2->GetType() == ITEM_ROD)
					return false;

				int i;

				for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				{
					DWORD dwVnum;

					if ((dwVnum = item2->GetSocket(i)) <= 2)
						continue;

					TItemTable * p = ITEM_MANAGER::instance().GetTable(dwVnum);

					if (!p)
						continue;

					if (item->GetValue(5) == p->alValues[5])
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???????? ?????? ?????? ?? ????????."));
						return false;
					}
				}

				if (item2->GetType() == ITEM_ARMOR)
				{
					if (!IS_SET(item->GetWearFlag(), WEARABLE_BODY) || !IS_SET(item2->GetWearFlag(), WEARABLE_BODY))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?????? ?????? ?? ????????."));
						return false;
					}
				}
				else if (item2->GetType() == ITEM_WEAPON)
				{
					if (!IS_SET(item->GetWearFlag(), WEARABLE_WEAPON))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?????? ?????? ?? ????????."));
						return false;
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?? ???? ?????? ????????."));
					return false;
				}

				for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
					if (item2->GetSocket(i) >= 1 && item2->GetSocket(i) <= 2 && item2->GetSocket(i) >= item->GetValue(2))
					{
						// ?? ????
#ifdef __ADDSTONE_FAILURE__
						if (number(1, 100) <= 30)
#else
						if (1)
#endif
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ??????????????."));
							item2->SetSocket(i, item->GetVnum());
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ??????????????."));
							item2->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
						}

						LogManager::instance().ItemLog(this, item2, "SOCKET", item->GetName());
						item->SetCount(item->GetCount() - 1);
						break;
					}

				if (i == ITEM_SOCKET_MAX_NUM)
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?? ???? ?????? ????????."));
			}
			break;

		case ITEM_AUTOUSE:
		case ITEM_MATERIAL:
			break;
		case ITEM_SPECIAL:
			break;
		case ITEM_TOOL:
		case ITEM_LOTTERY:
			break;

		case ITEM_TOTEM:
			{
				if (!item->IsEquipped())
					EquipItem(item);
			}
			break;

		case ITEM_BLEND:
			// ?????? ??????
			sys_log(0,"ITEM_BLEND!!");
			if (Blend_Item_find(item->GetVnum()))
			{
				int		affect_type		= AFFECT_BLEND;
				int		apply_type		= aApplyInfo[item->GetSocket(0)].bPointType;
				int		apply_value		= item->GetSocket(1);
				int		apply_duration	= item->GetSocket(2);

				if (FindAffect(affect_type, apply_type))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ????????."));
				}
				else
				{
					if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE, POINT_RESIST_MAGIC))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ????????."));
					}
					else
					{
						AddAffect(affect_type, apply_type, apply_value, 0, apply_duration, 0, false);
						item->SetCount(item->GetCount() - 1);
					}
				}
			}
			break;
		case ITEM_EXTRACT:
			{
				LPITEM pDestItem = GetItem(DestCell);
				if (NULL == pDestItem)
				{
					return false;
				}
				switch (item->GetSubType())
				{
				case EXTRACT_DRAGON_SOUL:
					if (pDestItem->IsDragonSoul())
					{
						return DSManager::instance().PullOut(this, NPOS, pDestItem, item);
					}
					return false;
				case EXTRACT_DRAGON_HEART:
					if (item->GetVnum() == 80039)
					{
						if (pDestItem->IsDragonSoul())
						{
							if (DSManager::instance().IsActiveDragonSoul(pDestItem) == true)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYA_GIYILIYOR"));
								return false;
							}

							pDestItem->SetForceAttribute(0, 0, 0);
							pDestItem->SetForceAttribute(1, 0, 0);
							pDestItem->SetForceAttribute(2, 0, 0);
							pDestItem->SetForceAttribute(3, 0, 0);
							pDestItem->SetForceAttribute(4, 0, 0);
							pDestItem->SetForceAttribute(5, 0, 0);
							pDestItem->SetForceAttribute(6, 0, 0);
							
							bool ret = DSManager::instance().PutAttributes(pDestItem);
							if (ret == true)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYA_DEGISTIRME_BASARILI!"));
								item->SetCount(item->GetCount()-1);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYA_DEGISTIRME_BASARISIZ!"));
							}
						}
					}
					else
					{
						if (pDestItem->IsDragonSoul())
						{
							return DSManager::instance().ExtractDragonHeart(this, pDestItem, item);
						}
					}
					return false;
				default:
					return false;
				}
			}
			break;

#ifdef __SPECIAL_GACHA__
		case ITEM_GACHA:
			{
				DWORD dwBoxVnum = item->GetVnum();
				std::vector <DWORD> dwVnums;
				std::vector <DWORD> dwCounts;
				std::vector <LPITEM> item_gets(0);
				int count = 0;

				if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
				{		
					for (int i = 0; i < count; i++)
					{
						switch (dwVnums[i])
						{
							case CSpecialItemGroup::GOLD:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? %d ???? ????????????."), dwCounts[i]);
								break;
							case CSpecialItemGroup::EXP:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ?????? ???? ????????."));
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d?? ???????? ????????????."), dwCounts[i]);
								break;
							case CSpecialItemGroup::MOB:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ????????????!"));
								break;
							case CSpecialItemGroup::SLOW:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???????? ?????? ????????????!"));
								break;
							case CSpecialItemGroup::DRAIN_HP:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ??????????????! ???????? ????????????."));
								break;
							case CSpecialItemGroup::POISON:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???? ???????? ????????!"));
								break;
#ifdef __WOLFMAN__
							case CSpecialItemGroup::BLEEDING:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???? ?????? ?????????? ???? ???????? ????????!"));
								break;
#endif
							case CSpecialItemGroup::MOB_GROUP:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???????? ????????????!"));
								break;
							default:
								if (item_gets[i])
								{
									if (dwCounts[i] > 1)
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? %s ?? %d ?? ??????????."), item_gets[i]->GetMultiName(), dwCounts[i]);
									else
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? %s ?? ??????????."), item_gets[i]->GetMultiName());
								}
						}
					}
					
					if (item->GetSocket(0) > 1)
						item->SetSocket(0, item->GetSocket(0) - 1);
					else
						ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (ITEM_GACHA)");
				}
				else
				{
					ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("???????? ???? ?? ??????????."));
					return false;
				}
			}
			break;
#endif

		case ITEM_NONE:
			sys_err("Item type NONE %s", item->GetName());
			break;

		default:
			sys_log(0, "UseItemEx: Unknown type %s %d", item->GetName(), item->GetType());
			return false;
	}

	return true;
}

int g_nPortalLimitTime = 10;

bool CHARACTER::UseItem(TItemPos Cell, TItemPos DestCell)
{
	WORD wCell = Cell.cell;
	BYTE window_type = Cell.window_type;
	//WORD wDestCell = DestCell.cell;
	//BYTE bDestInven = DestCell.window_type;
	LPITEM item;

	if (!CanHandleItem())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	sys_log(0, "%s: USE_ITEM %s (inven %d, cell: %d)", GetName(), item->GetName(), window_type, wCell);

	if (item->IsExchanging())
		return false;

	if (!item->CanUsedBy(this))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???????? ?? ???????? ?????? ?? ????????."));
		return false;
	}

	if (IsStun())
		return false;

	if (false == FN_check_item_sex(this, item))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???????? ?? ???????? ?????? ?? ????????."));
		return false;
	}

	//PREVENT_TRADE_WINDOW
	if (IS_SUMMON_ITEM(item->GetVnum()))
	{
		if (false == IS_SUMMONABLE_ZONE(GetMapIndex()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ????????."));
			return false;
		}

		// ???????? ?????? ???????? SUMMONABLE_ZONE?? ???????? WarpToPC()???? ????

		int iPulse = thecore_pulse();

		//???? ???? ????
		if (iPulse - GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? %d?? ???????? ??????,???????????? ?????? ?? ????????."), g_nPortalLimitTime);

			return false;
		}

		//???????? ?? ????
		if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("??????,???? ???? ?? ?????????? ??????,?????????? ?? ???????? ????????."));
			return false;
		}

#ifdef __ATTR_TRANSFER__
		if (IsAttrTransferOpen())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("??????,???? ???? ?? ?????????? ??????,?????????? ?? ???????? ????????."));
			return false;
		}
#endif

		//PREVENT_REFINE_HACK
		//?????? ????????
		{
			if (iPulse - GetRefineTime() < PASSES_PER_SEC(g_nPortalLimitTime))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? %d?? ???????? ??????,???????????? ?????? ?? ????????."), g_nPortalLimitTime);
				return false;
			}
		}
		//END_PREVENT_REFINE_HACK


		//PREVENT_ITEM_COPY
		{
			if (iPulse - GetMyShopTime() < PASSES_PER_SEC(g_nPortalLimitTime))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ?????? %d?? ???????? ??????,???????????? ?????? ?? ????????."), g_nPortalLimitTime);
				return false;
			}

		}
		//END_PREVENT_ITEM_COPY


		//?????? ????????
		if (item->GetVnum() != 70302)
		{
			PIXEL_POSITION posWarp;

			int x = 0;
			int y = 0;

			double nDist = 0;
			const double nDistant = 5000.0;
			//??????????
			if (item->GetVnum() == 22010)
			{
				x = item->GetSocket(0) - GetX();
				y = item->GetSocket(1) - GetY();
			}
			//??????
			else if (item->GetVnum() == 22000)
			{
				SECTREE_MANAGER::instance().GetRecallPositionByEmpire(GetMapIndex(), GetEmpire(), posWarp);

				if (item->GetSocket(0) == 0)
				{
					x = posWarp.x - GetX();
					y = posWarp.y - GetY();
				}
				else
				{
					x = item->GetSocket(0) - GetX();
					y = item->GetSocket(1) - GetY();
				}
			}

			nDist = sqrt(pow((float)x,2) + pow((float)y,2));

			if (nDistant > nDist)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ???? ?????? ???????? ???????? ????????."));
				return false;
			}
		}

		//PREVENT_PORTAL_AFTER_EXCHANGE
		//???? ?? ????????
		if (iPulse - GetExchangeTime()  < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?? %d?? ???????? ??????,?????????????? ?????? ?? ????????."), g_nPortalLimitTime);
			return false;
		}
		//END_PREVENT_PORTAL_AFTER_EXCHANGE

	}

	//?????? ???? ?????? ?????? ???? ????
	if ((item->GetVnum() == 50200) || (item->GetVnum() == 71049))
	{
#ifdef __ATTR_TRANSFER__
		if (IsAttrTransferOpen())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("??????,???? ???? ?? ?????????? ??????,???????????? ???????? ????????."));
			return false;
		}
#endif
		if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("??????,???? ???? ?? ?????????? ??????,???????????? ???????? ????????."));
			return false;
		}

	}
	//END_PREVENT_TRADE_WINDOW

	// @fixme150 BEGIN
	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot use this item if you're using quests"));
		return false;
	}
	// @fixme150 END

	if (IS_SET(item->GetFlag(), ITEM_FLAG_LOG)) // ???? ?????? ?????? ?????? ????
	{
		DWORD vid = item->GetVID();
		DWORD oldCount = item->GetCount();
		DWORD vnum = item->GetVnum();

		char hint[ITEM_NAME_MAX_LEN + 32 + 1];
		int len = snprintf(hint, sizeof(hint) - 32, "%s", item->GetName());

		if (len < 0 || len >= (int) sizeof(hint) - 32)
			len = (sizeof(hint) - 32) - 1;

		bool ret = UseItemEx(item, DestCell);

		if (NULL == ITEM_MANAGER::instance().FindByVID(vid)) // UseItemEx???? ???????? ???? ??????. ???? ?????? ????
		{
			LogManager::instance().ItemLog(this, vid, vnum, "REMOVE", hint);
		}
		else if (oldCount != item->GetCount())
		{
			snprintf(hint + len, sizeof(hint) - len, " %u", oldCount - 1);
			LogManager::instance().ItemLog(this, vid, vnum, "USE_ITEM", hint);
		}
		return (ret);
	}
	else
		return UseItemEx(item, DestCell);
}

bool CHARACTER::DropItem(TItemPos Cell, BYTE bCount)
{
	LPITEM item = NULL;

	if (!CanHandleItem())
	{
		if (NULL != DragonSoul_RefineWindow_GetOpener())
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ?? ?????????? ???????? ???? ?? ????????."));
		return false;
	}
#ifdef __NEWSTUFF__
	if (0 != g_ItemDropTimeLimitValue)
	{
		if (get_dword_time() < m_dwLastItemDropTime+g_ItemDropTimeLimitValue)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ?? ????????."));
			return false;
		}
	}

	m_dwLastItemDropTime = get_dword_time();
#endif
	if (IsDead())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (true == item->isLocked())
		return false;

#ifdef __SOULBIND__ 
	if (item->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't drop sealbind item."));
		return false;
	}
#endif

	if (item->IsBasicItem())
		return false;

	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		return false;

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP | ITEM_ANTIFLAG_GIVE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?? ???? ????????????."));
		return false;
	}

	if (bCount == 0 || bCount > item->GetCount())
		bCount = item->GetCount();

	SyncQuickslot(QUICKSLOT_TYPE_ITEM, Cell.cell, 255);	// Quickslot ???? ????

	LPITEM pkItemToDrop;

	if (bCount == item->GetCount())
	{
		item->RemoveFromCharacter();
		pkItemToDrop = item;
	}
	else
	{
		if (bCount == 0)
		{
			return false;
		}

		item->SetCount(item->GetCount() - bCount);
		ITEM_MANAGER::instance().FlushDelayedSave(item);

		pkItemToDrop = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), bCount);

		// copy item socket -- by mhh
		FN_copy_item_socket(pkItemToDrop, item);

		char szBuf[51 + 1];
		snprintf(szBuf, sizeof(szBuf), "%u %u", pkItemToDrop->GetID(), pkItemToDrop->GetCount());
		LogManager::instance().ItemLog(this, item, "ITEM_SPLIT", szBuf);
	}

	PIXEL_POSITION pxPos = GetXYZ();

	if (pkItemToDrop->AddToGround(GetMapIndex(), pxPos))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???????? 3?? ?? ??????????."));
#ifdef __NEWSTUFF__
		pkItemToDrop->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPITEM]);
#else
		pkItemToDrop->StartDestroyEvent();
#endif

		ITEM_MANAGER::instance().FlushDelayedSave(pkItemToDrop);

		char szHint[32 + 1];
		snprintf(szHint, sizeof(szHint), "%s %u %u", pkItemToDrop->GetName(), pkItemToDrop->GetCount(), pkItemToDrop->GetOriginalVnum());
		LogManager::instance().ItemLog(this, pkItemToDrop, "DROP", szHint);
		//Motion(MOTION_PICKUP);
	}

	return true;
}

bool CHARACTER::DestroyItem(TItemPos Cell)
{
    LPITEM item = NULL;
    if (IsDead())
        return false;
    if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
        return false;
    if (item->IsExchanging())
        return false;
    if (true == item->isLocked())
        return false;
    if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
        return false;
    if (item->GetCount() <= 0)
        return false;
    SyncQuickslot(QUICKSLOT_TYPE_ITEM, Cell.cell, 255);
    ITEM_MANAGER::instance().RemoveItem(item);
    return true;
}

bool CHARACTER::SellItem(TItemPos Cell)
{
	LPITEM item = NULL;
    if (IsDead())
        return false;
    if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
        return false;
    if (item->IsExchanging())
        return false;
    if (true == item->isLocked())
        return false;
    if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
        return false;
    if (item->GetCount() <= 0)
        return false;
	if (item->IsBasicItem())
		return false;

	DWORD dwPrice = item->GetGold();
	dwPrice *= item->GetCount();
	
	LogManager::instance().MoneyLog(MONEY_LOG_SHOP, item->GetVnum(), dwPrice);
	item->SetCount(item->GetCount() - item->GetCount());
	PointChange(POINT_GOLD, dwPrice, false);
	return true;
}

bool CHARACTER::DropGold(int gold)
{
	if (gold <= 0 || gold > GetGold())
		return false;

	if (!CanHandleItem())
		return false;

	if (0 != g_GoldDropTimeLimitValue)
	{
		if (get_dword_time() < m_dwLastGoldDropTime+g_GoldDropTimeLimitValue)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???? ?? ????????."));
			return false;
		}
	}

	m_dwLastGoldDropTime = get_dword_time();

	LPITEM item = ITEM_MANAGER::instance().CreateItem(1, gold);

	if (item)
	{
		PIXEL_POSITION pos = GetXYZ();

		if (item->AddToGround(GetMapIndex(), pos))
		{
			//Motion(MOTION_PICKUP);
			PointChange(POINT_GOLD, -gold, true);

			if (gold > 1000) // ???? ?????? ????????.
				LogManager::instance().CharLog(this, gold, "DROP_GOLD", "");

#ifdef __NEWSTUFF__
			item->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPGOLD]);
#else
			item->StartDestroyEvent();
#endif
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???????? %d?? ?? ??????????."), 150/60);
		}

		Save();
		return true;
	}

	return false;
}

bool CHARACTER::MoveItem(TItemPos Cell, TItemPos DestCell, BYTE count)
{
	LPITEM item = NULL;

	if (!IsValidItemPosition(Cell))
		return false;

	if (!(item = GetItem(Cell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (item->GetCount() < count)
		return false;

	if (INVENTORY == Cell.window_type && Cell.cell >= GetInventoryMax() && IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		return false;

	if (true == item->isLocked())
		return false;

	if (!IsValidItemPosition(DestCell))
		return false;

	if (!CanHandleItem())
	{
		if (NULL != DragonSoul_RefineWindow_GetOpener())
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ?? ?????????? ???????? ???? ?? ????????."));
		return false;
	}

	// ???????? ???????? ???? ???????????? ???? ?????? ???????? ???? ?? ????.
	if (DestCell.IsBeltInventoryPosition() && false == CBeltInventoryHelper::CanMoveIntoBeltInventory(item))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ???? ?????????? ???? ?? ????????."));
		return false;
	}

	// ???? ???????? ???????? ???? ?????? ?????? ????, '???? ????' ?????? ?? ???????? ????
	if (Cell.IsEquipPosition() && !CanUnequipNow(item))
		return false;

	if (DestCell.IsEquipPosition())
	{
		if (GetItem(DestCell))	// ?????? ???? ?? ???? ???????? ????.
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???????? ????????."));

			return false;
		}

		EquipItem(item, DestCell.cell - INVENTORY_MAX_NUM);
	}
	else
	{
		if (item->IsDragonSoul())
		{
			if (item->IsEquipped())
			{
				return DSManager::instance().PullOut(this, DestCell, item);
			}
			else
			{
				if (DestCell.window_type != DRAGON_SOUL_INVENTORY)
				{
					return false;
				}

				if (!DSManager::instance().IsValidCellForThisItem(item, DestCell))
					return false;
			}
		}
		// ???????? ???? ???????? ?????? ?????? ?????? ?? ????.
		else if (DRAGON_SOUL_INVENTORY == DestCell.window_type)
			return false;

#ifdef __NEW_STORAGE__
		if ((item->IsUpgradeItem() && (DestCell.window_type != UPGRADE_INVENTORY)) || (item->IsBook() && DestCell.window_type != BOOK_INVENTORY) || (item->IsStone() && DestCell.window_type != STONE_INVENTORY) || (item->IsAttr() && DestCell.window_type != ATTR_INVENTORY) || (item->IsGiftBox() && DestCell.window_type != GIFTBOX_INVENTORY))
			return false;
#endif

		LPITEM item2;

		if ((item2 = GetItem(DestCell)) && item != item2 && item2->IsStackable() && item2->GetVnum() == item->GetVnum()) // ???? ?? ???? ???????? ????
		{
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				if (item2->GetSocket(i) != item->GetSocket(i))
					return false;

			if (count == 0)
				count = item->GetCount();

			sys_log(0, "%s: ITEM_STACK %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			count = MIN(g_bItemCountLimit - item2->GetCount(), count);

			item->SetCount(item->GetCount() - count);
			item2->SetCount(item2->GetCount() + count);
			return true;
		}

		if (!IsEmptyItemGrid(DestCell, item->GetSize(), Cell.cell))
			return false;

		if (count == 0 || count >= item->GetCount() || !item->IsStackable())
		{
			sys_log(0, "%s: ITEM_MOVE %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			item->RemoveFromCharacter();
			SetItem(DestCell, item);

			if (INVENTORY == Cell.window_type && INVENTORY == DestCell.window_type)
			{
				SyncQuickslot(QUICKSLOT_TYPE_ITEM, Cell.cell, DestCell.cell);
			}
		}
		else if (count < item->GetCount())
		{

			sys_log(0, "%s: ITEM_SPLIT %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			item->SetCount(item->GetCount() - count);
			LPITEM item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), count);

			// copy socket -- by mhh
			FN_copy_item_socket(item2, item);

			item2->AddToCharacter(this, DestCell);

			char szBuf[51+1];
			snprintf(szBuf, sizeof(szBuf), "%u %u %u %u ", item2->GetID(), item2->GetCount(), item->GetCount(), item->GetCount() + item2->GetCount());
			LogManager::instance().ItemLog(this, item, "ITEM_SPLIT", szBuf);
		}
	}

	return true;
}

namespace NPartyPickupDistribute
{
	struct FFindOwnership
	{
		LPITEM item;
		LPCHARACTER owner;

		FFindOwnership(LPITEM item)
			: item(item), owner(NULL)
		{
		}

		void operator () (LPCHARACTER ch)
		{
			if (item->IsOwnership(ch))
				owner = ch;
		}
	};

	struct FCountNearMember
	{
		int		total;
		int		x, y;

		FCountNearMember(LPCHARACTER center )
			: total(0), x(center->GetX()), y(center->GetY())
		{
		}

		void operator () (LPCHARACTER ch)
		{
			if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				total += 1;
		}
	};

	struct FMoneyDistributor
	{
		int		total;
		LPCHARACTER	c;
		int		x, y;
		int		iMoney;

		FMoneyDistributor(LPCHARACTER center, int iMoney)
			: total(0), c(center), x(center->GetX()), y(center->GetY()), iMoney(iMoney)
		{
		}

		void operator ()(LPCHARACTER ch)
		{
			if (ch!=c)
				if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				{
					ch->PointChange(POINT_GOLD, iMoney, true);

					if (iMoney > 1000) // ???? ?????? ????????.
					{
						LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::instance().CharLog(ch, iMoney, "GET_GOLD", ""));
					}
				}
		}
	};
}

void CHARACTER::GiveGold(int iAmount)
{
	if (iAmount <= 0)
		return;

	sys_log(0, "GIVE_GOLD: %s %d", GetName(), iAmount);

	if (GetParty())
	{
		LPPARTY pParty = GetParty();

		// ?????? ???? ???? ?????? ??????.
		DWORD dwTotal = iAmount;
		DWORD dwMyAmount = dwTotal;

		NPartyPickupDistribute::FCountNearMember funcCountNearMember(this);
		pParty->ForEachOnlineMember(funcCountNearMember);

		if (funcCountNearMember.total > 1)
		{
			DWORD dwShare = dwTotal / funcCountNearMember.total;
			dwMyAmount -= dwShare * (funcCountNearMember.total - 1);

			NPartyPickupDistribute::FMoneyDistributor funcMoneyDist(this, dwShare);

			pParty->ForEachOnlineMember(funcMoneyDist);
		}

		PointChange(POINT_GOLD, dwMyAmount, true);

		if (dwMyAmount > 1000) // ???? ?????? ????????.
		{
			LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::instance().CharLog(this, dwMyAmount, "GET_GOLD", ""));
		}
	}
	else
	{
		PointChange(POINT_GOLD, iAmount, true);

		if (iAmount > 1000) // ???? ?????? ????????.
		{
			LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::instance().CharLog(this, iAmount, "GET_GOLD", ""));
		}
	}
}

bool CHARACTER::PickupItem(DWORD dwVID)
{
	LPITEM item = ITEM_MANAGER::instance().FindByVID(dwVID);

	if (IsObserverMode())
		return false;

	if (!item || !item->GetSectree())
		return false;

	if (item->DistanceValid(this))
	{
		// @fixme150 BEGIN
		if (item->GetType() == ITEM_QUEST)
		{
			if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot pickup this item if you're using quests"));
				return false;
			}
		}
		// @fixme150 END

		if (item->IsOwnership(this))
		{
			// ???? ?????? ???? ???????? ????????
			if (item->GetType() == ITEM_ELK)
			{
				GiveGold(item->GetCount());
				item->RemoveFromGround();

				M2_DESTROY_ITEM(item);

				Save();
			}
			// ?????? ????????????
			else
			{
#ifdef __NEW_STORAGE__
				if ((item->IsStorageItem()) && item->IsStackable())
				{
					BYTE bCount = item->GetCount();

					for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = GetStorageInventoryItem(item->GetStorageType() - UPGRADE_INVENTORY, i);

						if (!item2)
							continue;

						if ((item2->GetVnum() == item->GetVnum() && !item->IsBook()) || (item->IsBook() && item2->GetVnum() == item->GetVnum() && item2->GetSocket(0) == item->GetSocket(0)))
						{

							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
							bCount -= bCount2;

							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s"), item2->GetMultiName());
								M2_DESTROY_ITEM(item);
								return true;
							}
						}
					}

					item->SetCount(bCount);
				}
#endif
				if (item->IsStackable())
				{
					BYTE bCount = item->GetCount();

					for (int i = 0; i < GetInventoryMax(); ++i)
					{
						LPITEM item2 = GetInventoryItem(i);

						if (!item2)
							continue;

						if (item2->GetVnum() == item->GetVnum())
						{
							int j;

							for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
								if (item2->GetSocket(j) != item->GetSocket(j))
									break;

							if (j != ITEM_SOCKET_MAX_NUM)
								continue;

							BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
							bCount -= bCount2;

							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s"), item2->GetMultiName());
								M2_DESTROY_ITEM(item);
								if (item2->GetType() == ITEM_QUEST)
									quest::CQuestManager::instance().PickupItem (GetPlayerID(), item2);
								return true;
							}
						}
					}

					item->SetCount(bCount);
				}
				int iEmptyCell;
				if (item->IsDragonSoul())
				{
					if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
					{
						sys_log(0, "No empty ds inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???????? ???? ????????."));
						return false;
					}
				}
#ifdef __NEW_STORAGE__
				else if (item->IsStorageItem())
				{
					if ((iEmptyCell = GetEmptyStorageInventory(item->GetStorageType() - UPGRADE_INVENTORY, item)) == -1)
					{
						sys_log(0, "No empty ss inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???????? ???? ????????."));
						return false;
					}
				}
#endif
				else
				{
					if ((iEmptyCell = GetEmptyInventory(item->GetSize())) == -1)
					{
						sys_log(0, "No empty inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???????? ???? ????????."));
						return false;
					}
				}

				item->RemoveFromGround();

				if (item->IsDragonSoul())
					item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell), true);
#ifdef __NEW_STORAGE__
				else if (item->IsStorageItem())
					item->AddToCharacter(this, TItemPos(UPGRADE_INVENTORY + (item->GetStorageType() - UPGRADE_INVENTORY), iEmptyCell), true);
#endif
				else
					item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyCell), true);

				char szHint[32+1];
				snprintf(szHint, sizeof(szHint), "%s %u %u", item->GetName(), item->GetCount(), item->GetOriginalVnum());
				LogManager::instance().ItemLog(this, item, "GET", szHint);
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s"), item->GetMultiName());

				if (item->GetType() == ITEM_QUEST)
					quest::CQuestManager::instance().PickupItem (GetPlayerID(), item);
			}

			//Motion(MOTION_PICKUP);
			return true;
		}
		else if (!IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_DROP) && GetParty())
		{
			// ???? ?????? ?????? ???????? ???????? ??????
			NPartyPickupDistribute::FFindOwnership funcFindOwnership(item);

			GetParty()->ForEachOnlineMember(funcFindOwnership);

			LPCHARACTER owner = funcFindOwnership.owner;
#ifdef __ANTI_DROPHACK__
			if (!owner)
			{
				LogManager::instance().HackLog("DropHack", this);
				return false;
			}
#endif

			int iEmptyCell;

			if (item->IsDragonSoul())
			{
				if (!(owner && (iEmptyCell = owner->GetEmptyDragonSoulInventory(item)) != -1))
				{
					owner = this;

					if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
					{
						owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???????? ???? ????????."));
						return false;
					}
				}
			}
#ifdef __NEW_STORAGE__
			else if (item->IsStorageItem())
			{
				if (!(owner && (iEmptyCell = owner->GetEmptyStorageInventory(item->GetStorageType() - UPGRADE_INVENTORY, item)) != -1))
				{
					owner = this;

					if ((iEmptyCell = GetEmptyStorageInventory(item->GetStorageType() - UPGRADE_INVENTORY, item)) == -1)
					{
						owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???????? ???? ????????."));
						return false;
					}
				}
			}
#endif
			else
			{
				if (!(owner && (iEmptyCell = owner->GetEmptyInventory(item->GetSize())) != -1))
				{
					owner = this;

					if ((iEmptyCell = GetEmptyInventory(item->GetSize())) == -1)
					{
						owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???????? ???? ????????."));
						return false;
					}
				}
			}

			item->RemoveFromGround();

			if (item->IsDragonSoul())
				item->AddToCharacter(owner, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell), true);
#ifdef __NEW_STORAGE__
			else if (item->IsStorageItem())
				item->AddToCharacter(owner, TItemPos(UPGRADE_INVENTORY + (item->GetStorageType() - UPGRADE_INVENTORY), iEmptyCell), true);
#endif
			else
				item->AddToCharacter(owner, TItemPos(INVENTORY, iEmptyCell), true);

			char szHint[32+1];
			snprintf(szHint, sizeof(szHint), "%s %u %u", item->GetName(), item->GetCount(), item->GetOriginalVnum());
			LogManager::instance().ItemLog(owner, item, "GET", szHint);

			if (owner == this)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s"), item->GetMultiName());
			else
			{
				owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s ?????????? %s"), GetName(), item->GetMultiName());
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s ?????? %s"), owner->GetName(), item->GetMultiName());
			}

			if (item->GetType() == ITEM_QUEST)
				quest::CQuestManager::instance().PickupItem (owner->GetPlayerID(), item);

			return true;
		}
	}

	return false;
}

bool CHARACTER::SwapItem(BYTE bCell, BYTE bDestCell)
{
	if (!CanHandleItem())
		return false;

	TItemPos srcCell(INVENTORY, bCell), destCell(INVENTORY, bDestCell);

	// ?????? Cell ???? ????
	// ???????? Swap?? ?? ????????, ?????? ????.
	//if (bCell >= INVENTORY_MAX_NUM + WEAR_MAX_NUM || bDestCell >= INVENTORY_MAX_NUM + WEAR_MAX_NUM)
	if (srcCell.IsDragonSoulEquipPosition() || destCell.IsDragonSoulEquipPosition())
		return false;

	// ???? CELL ???? ????
	if (bCell == bDestCell)
		return false;

	// ?? ?? ?????? ?????? Swap ?? ?? ????.
	if (srcCell.IsEquipPosition() && destCell.IsEquipPosition())
		return false;

	LPITEM item1, item2;

	// item2?? ???????? ???? ???? ??????.
	if (srcCell.IsEquipPosition())
	{
		item1 = GetInventoryItem(bDestCell);
		item2 = GetInventoryItem(bCell);
	}
	else
	{
		item1 = GetInventoryItem(bCell);
		item2 = GetInventoryItem(bDestCell);
	}

	if (!item1 || !item2)
		return false;

	if (item1 == item2)
	{
	    sys_log(0, "[WARNING][WARNING][HACK USER!] : %s %d %d", m_stName.c_str(), bCell, bDestCell);
	    return false;
	}

	// item2?? bCell?????? ?????? ?? ?????? ????????.
	if (!IsEmptyItemGrid(TItemPos (INVENTORY, item1->GetCell()), item2->GetSize(), item1->GetCell()))
		return false;

	// ???? ???????? ???????? ??????
	if (TItemPos(EQUIPMENT, item2->GetCell()).IsEquipPosition())
	{
		BYTE bEquipCell = item2->GetCell() - INVENTORY_MAX_NUM;
		BYTE bInvenCell = item1->GetCell();

		// ???????? ???????? ???? ?? ????, ???? ???? ???????? ???? ?????? ?????????? ????
		if (item2->IsDragonSoul() || item2->GetType() == ITEM_BELT) // @fixme117
		{
			if (false == CanUnequipNow(item2) || false == CanEquipNow(item1))
				return false;
		}

		if (bEquipCell != item1->FindEquipCell(this)) // ???? ?????????? ????
			return false;

		item2->RemoveFromCharacter();

		if (item1->EquipTo(this, bEquipCell)) 
		{
			item2->AddToCharacter(this, TItemPos(INVENTORY, bInvenCell));
			// item2->ModifyPoints(false);
			// ComputePoints();
		} 
		else
			sys_err("SwapItem cannot equip %s! item1 %s", item2->GetName(), item1->GetName());
	}
	else
	{
		BYTE bCell1 = item1->GetCell();
		BYTE bCell2 = item2->GetCell();

		item1->RemoveFromCharacter();
		item2->RemoveFromCharacter();

		item1->AddToCharacter(this, TItemPos(INVENTORY, bCell2));
		item2->AddToCharacter(this, TItemPos(INVENTORY, bCell1));
	}

	return true;
}

bool CHARACTER::UnequipItem(LPITEM item)
{
	int pos;

	if (false == CanUnequipNow(item))
		return false;

	if (item->IsDragonSoul())
		pos = GetEmptyDragonSoulInventory(item);
#ifdef __NEW_STORAGE__
	else if (item->IsStorageItem())
		pos = GetEmptyStorageInventory(item->GetStorageType() - UPGRADE_INVENTORY, item);
#endif
	else
		pos = GetEmptyInventory(item->GetSize());

	// HARD CODING
	if (item->GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		ShowAlignment(true);

#ifdef __COSTUME_MOUNT__
	if ((item->GetType() == ITEM_COSTUME) && (COSTUME_MOUNT == item->GetSubType()))
#ifdef __MOUNT__
	{
		CMountSystem* MountSystem = GetMountSystem();
		if (MountSystem)
			MountSystem->Unsummon(item->FindApplyValue(APPLY_MOUNT));
	}
#else
		quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);
#endif
#endif

	item->RemoveFromCharacter();
	if (item->IsDragonSoul())
		item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, pos));
#ifdef __NEW_STORAGE__
	else if (item->IsStorageItem())
		item->AddToCharacter(this, TItemPos(UPGRADE_INVENTORY + (item->GetStorageType() - UPGRADE_INVENTORY), pos));
#endif
	else
		item->AddToCharacter(this, TItemPos(INVENTORY, pos));

	CheckMaximumPoints();

	return true;
}


bool CHARACTER::EquipItem(LPITEM item, int iCandidateCell)
{
	if (item->IsExchanging())
		return false;

	if (false == item->IsEquipable())
		return false;

	if (false == CanEquipNow(item))
		return false;

	int iWearCell = item->FindEquipCell(this, iCandidateCell);

	if (iWearCell < 0)
		return false;

	if ((iWearCell == WEAR_BODY || iWearCell == WEAR_COSTUME_BODY) && IsRiding() && get_global_time() < GetQuestFlag("zirh.engel"))
	{
		ChatPacket(CHAT_TYPE_INFO, "Bu islemi bu kadar hizli yapamassin!");
		return false;
	}

	if (item->GetType() == ITEM_RING && ((GetWear(WEAR_RING1) && GetWear(WEAR_RING1)->GetVnum() == item->GetVnum()) || (GetWear(WEAR_RING2) && GetWear(WEAR_RING2)->GetVnum() == item->GetVnum())))
		return false;

	if (item->IsEdit())
	{
		sys_err("Edit DETECTED ! in '[%lu]%s' Item: [%lu]%s Vnum: %lu", GetAID(), GetName(), item->GetID(), item->GetName(), item->GetVnum());
		ITEM_MANAGER::instance().RemoveItem(item);
		return false;
	}

	// ???????? ?? ???????? ?????? ???? ????
	if (iWearCell == WEAR_BODY && IsRiding() && (item->GetVnum() >= 11901 && item->GetVnum() <= 11904))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?? ???????? ?????? ???? ?? ????????."));
		return false;
	}

	if (iWearCell != WEAR_ARROW && IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ???????? ?????? ?????? ?? ????????."));
		return false;
	}

	if (FN_check_item_sex(this, item) == false)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???????? ?? ???????? ?????? ?? ????????."));
		return false;
	}

	//???? ???? ?????? ???? ?? ???????? ????
	if(item->IsRideItem() && IsRiding())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ????????????."));
		return false;
	}

#if defined(__COSTUME_MOUNT__) && defined(__MOUNT__)
	if (item->GetType() == ITEM_COSTUME && COSTUME_MOUNT == item->GetSubType())
	{
		CMountSystem* MountSystem = GetMountSystem();
		if (MountSystem)
			if (MountSystem->CountSummoned())
				return false;
	}
#endif

	// ???? ???????? ?????? ???? ???? ???? ???? ???? 1.5 ???? ???? ?????? ????
	// DWORD dwCurTime = get_dword_time();

	// if (iWearCell != WEAR_ARROW
		// && (dwCurTime - GetLastAttackTime() <= 1500 || dwCurTime - m_dwLastSkillTime <= 1500))
	// {
		// ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ???? ?????? ?? ????????."));
		// return false;
	// }

	// ?????? ???? ????
	if (item->IsDragonSoul())
	{
		// ???? ?????? ???????? ???? ?????? ?????? ?????? ?? ????.
		// ???????? swap?? ???????? ????.
		if (GetInventoryItem(INVENTORY_MAX_NUM + iWearCell))
		{
			ChatPacket(CHAT_TYPE_INFO, "???? ???? ?????? ???????? ???????? ????????.");
			return false;
		}

		if (!item->EquipTo(this, iWearCell))
		{
			return false;
		}
	}
	// ???????? ????.
	else
	{
		// ?????? ???? ???????? ??????,
		if (GetWear(iWearCell) && !IS_SET(GetWear(iWearCell)->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		{
			// ?? ???????? ???? ?????? ???? ????. swap ???? ???? ????
			if (item->GetWearFlag() == WEARABLE_ABILITY)
				return false;

			if (false == SwapItem(item->GetCell(), INVENTORY_MAX_NUM + iWearCell))
			{
				return false;
			}
		}
		else
		{
			BYTE bOldCell = item->GetCell();

			if (item->EquipTo(this, iWearCell))
			{
				SyncQuickslot(QUICKSLOT_TYPE_ITEM, bOldCell, iWearCell);
			}
		}
	}

	if (true == item->IsEquipped())
	{
		// ?????? ???? ???? ?????????? ???????? ?????? ?????? ???????? ???? ????.
		if (-1 != item->GetProto()->cLimitRealTimeFirstUseIndex)
		{
			// ?? ???????? ?????? ?????????? ?????? Socket1?? ???? ????????. (Socket1?? ???????? ????)
			if (0 == item->GetSocket(1))
			{
				// ?????????????? Default ?????? Limit Value ???? ????????, Socket0?? ???? ?????? ?? ???? ?????????? ????. (?????? ??)
				long duration = (0 != item->GetSocket(0)) ? item->GetSocket(0) : item->GetProto()->aLimits[(unsigned char)(item->GetProto()->cLimitRealTimeFirstUseIndex)].lValue;

				if (0 == duration)
					duration = 60 * 60 * 24 * 7;

				item->SetSocket(0, time(0) + duration);
				item->StartRealTimeExpireEvent();
			}

			item->SetSocket(1, item->GetSocket(1) + 1);
		}

		if (item->GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
			ShowAlignment(false);

		const DWORD& dwVnum = item->GetVnum();

		// ?????? ?????? ???????? ????(71135) ?????? ?????? ????
		if (true == CItemVnumHelper::IsRamadanMoonRing(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_RAMADAN_RING);
		}
		// ?????? ????(71136) ?????? ?????? ????
		else if (true == CItemVnumHelper::IsHalloweenCandy(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_HALLOWEEN_CANDY);
		}
		// ?????? ????(71143) ?????? ?????? ????
		else if (true == CItemVnumHelper::IsHappinessRing(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_HAPPINESS_RING);
		}
		// ?????? ??????(71145) ?????? ?????? ????
		else if (true == CItemVnumHelper::IsLovePendant(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_LOVE_PENDANT);
		}
		// ITEM_UNIQUE?? ????, SpecialItemGroup?? ???????? ????, (item->GetSIGVnum() != NULL)
		//
		else if (ITEM_UNIQUE == item->GetType() && 0 != item->GetSIGVnum())
		{
			const CSpecialItemGroup* pGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(item->GetSIGVnum());
			if (NULL != pGroup)
			{
				const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::instance().GetSpecialAttrGroup(pGroup->GetAttrVnum(item->GetVnum()));
				if (NULL != pAttrGroup)
				{
					const std::string& std = pAttrGroup->m_stEffectFileName;
					SpecificEffectPacket(std.c_str());
				}
			}
		}
#ifdef __COSTUME_ACCE__
		else if ((item->GetType() == ITEM_COSTUME) && (item->GetSubType() == COSTUME_ACCE))
			this->EffectPacket(SE_EFFECT_ACCE_EQUIP);
#endif

		if (UNIQUE_SPECIAL_RIDE == item->GetSubType() && IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE))
			quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);

		if ((iWearCell == WEAR_BODY || iWearCell == WEAR_COSTUME_BODY) && IsRiding())
			SetQuestFlag("zirh.engel", get_global_time() + 4);	

#ifdef __COSTUME_MOUNT__
		if ((item->GetType() == ITEM_COSTUME) && (COSTUME_MOUNT == item->GetSubType()))
#ifdef __MOUNT__
		{
			CMountSystem* MountSystem = GetMountSystem();
			if (MountSystem)
				MountSystem->Summon(item->FindApplyValue(APPLY_MOUNT), GetName());
		}
#else
			quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);
#endif
#endif
	}

	return true;
}

void CHARACTER::BuffOnAttr_AddBuffsFromItem(LPITEM pItem)
{
	for (size_t i = 0; i < sizeof(g_aBuffOnAttrPoints)/sizeof(g_aBuffOnAttrPoints[0]); i++)
	{
		TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
		if (it != m_map_buff_on_attrs.end())
		{
			it->second->AddBuffFromItem(pItem);
		}
	}
}

void CHARACTER::BuffOnAttr_RemoveBuffsFromItem(LPITEM pItem)
{
	for (size_t i = 0; i < sizeof(g_aBuffOnAttrPoints)/sizeof(g_aBuffOnAttrPoints[0]); i++)
	{
		TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
		if (it != m_map_buff_on_attrs.end())
		{
			it->second->RemoveBuffFromItem(pItem);
		}
	}
}

void CHARACTER::BuffOnAttr_ClearAll()
{
	for (TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.begin(); it != m_map_buff_on_attrs.end(); it++)
	{
		CBuffOnAttributes* pBuff = it->second;
		if (pBuff)
		{
			pBuff->Initialize();
		}
	}
}

void CHARACTER::BuffOnAttr_ValueChange(BYTE bType, BYTE bOldValue, BYTE bNewValue)
{
	TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(bType);

	if (0 == bNewValue)
	{
		if (m_map_buff_on_attrs.end() == it)
			return;
		else
			it->second->Off();
	}
	else if(0 == bOldValue)
	{
		CBuffOnAttributes* pBuff = NULL;
		if (m_map_buff_on_attrs.end() == it)
		{
			switch (bType)
			{
			case POINT_ENERGY:
				{
					static BYTE abSlot[] = { WEAR_BODY, WEAR_HEAD, WEAR_FOOTS, WEAR_WRIST, WEAR_WEAPON, WEAR_NECK, WEAR_EAR, WEAR_SHIELD };
					static std::vector <BYTE> vec_slots (abSlot, abSlot + _countof(abSlot));
					pBuff = M2_NEW CBuffOnAttributes(this, bType, &vec_slots);
				}
				break;
			case POINT_COSTUME_ATTR_BONUS:
				{
					static BYTE abSlot[] = {WEAR_COSTUME_BODY, WEAR_COSTUME_HAIR,
#ifdef __COSTUME_WEAPON__
WEAR_COSTUME_WEAPON,
#endif
};
					static std::vector <BYTE> vec_slots (abSlot, abSlot + _countof(abSlot));
					pBuff = M2_NEW CBuffOnAttributes(this, bType, &vec_slots);
				}
				break;
			default:
				break;
			}
			m_map_buff_on_attrs.insert(TMapBuffOnAttrs::value_type(bType, pBuff));

		}
		else
			pBuff = it->second;
		if (pBuff != NULL)
			pBuff->On(bNewValue);
	}
	else
	{
		assert (m_map_buff_on_attrs.end() != it);
		it->second->ChangeBuffValue(bNewValue);
	}
}

LPITEM CHARACTER::FindSpecifyItem(DWORD vnum) const
{
	for (int i = 0; i < GetInventoryMax(); ++i)
		if (GetInventoryItem(i) && GetInventoryItem(i)->GetVnum() == vnum)
			return GetInventoryItem(i);

#ifdef __NEW_STORAGE__
	for (int j = 0; j < STORAGE_INVENTORY_COUNT; ++j)
		for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
			if (GetStorageInventoryItem(j, i) && GetStorageInventoryItem(j, i)->GetVnum() == vnum)
				return GetStorageInventoryItem(j, i);
#endif

	return NULL;
}

#ifdef __PENDANT__
LPITEM CHARACTER::FindSpecifyItemTheOutItem(DWORD vnum, LPITEM item) const
{
	for (int i = 0; i < GetInventoryMax(); ++i)
		if (GetInventoryItem(i) && GetInventoryItem(i)->GetVnum() == vnum && GetInventoryItem(i)->GetVID() != item->GetVID())
			return GetInventoryItem(i);

#ifdef __NEW_STORAGE__
	for (int j = 0; j < STORAGE_INVENTORY_COUNT; ++j)
		for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
			if (GetStorageInventoryItem(j, i) && GetStorageInventoryItem(j, i)->GetVnum() == vnum && GetStorageInventoryItem(j, i)->GetVID() != item->GetVID())
				return GetStorageInventoryItem(j, i);
#endif

	return NULL;
}
#endif

LPITEM CHARACTER::FindItemByID(DWORD id) const
{
	for (int i=0 ; i < GetInventoryMax() ; ++i)
		if (NULL != GetInventoryItem(i) && GetInventoryItem(i)->GetID() == id)
			return GetInventoryItem(i);

	for (int i=BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END ; ++i)
		if (NULL != GetInventoryItem(i) && GetInventoryItem(i)->GetID() == id)
			return GetInventoryItem(i);

#ifdef __NEW_STORAGE__
	for (int j = 0; j < STORAGE_INVENTORY_COUNT; ++j)
		for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
			if (NULL != GetStorageInventoryItem(j, i) && GetStorageInventoryItem(j, i)->GetID() == id)
				return GetStorageInventoryItem(j, i);
#endif

	return NULL;
}

int CHARACTER::CountSpecifyItem(DWORD vnum) const
{
	int	count = 0;
	LPITEM item;

	for (int i = 0; i < GetInventoryMax(); ++i)
	{
		item = GetInventoryItem(i);
		if (NULL != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
				continue;
			else
				count += item->GetCount();
		}
	}

#ifdef __NEW_STORAGE__
	for (int j = 0; j < STORAGE_INVENTORY_COUNT; ++j)
	{
		for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
		{
			item = GetStorageInventoryItem(j, i);
			if (NULL != item && item->GetVnum() == vnum)
			{
				if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
					continue;
				else
					count += item->GetCount();
			}
		}
	}
#endif

	return count;
}

#ifdef __PENDANT__
int CHARACTER::CountSpecifyItemTheOutItem(DWORD vnum, LPITEM item2) const
{
	int	count = 0;
	LPITEM item;

	for (int i = 0; i < GetInventoryMax(); ++i)
	{
		item = GetInventoryItem(i);
		if (NULL != item && item->GetVnum() == vnum && item->GetVID() != item2->GetVID())
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
				continue;
			else
				count += item->GetCount();
		}
	}

#ifdef __NEW_STORAGE__
	for (int j = 0; j < STORAGE_INVENTORY_COUNT; ++j)
	{
		for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
		{
			item = GetStorageInventoryItem(j, i);
			if (NULL != item && item->GetVnum() == vnum && item->GetVID() != item2->GetVID())
			{
				if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
					continue;
				else
					count += item->GetCount();
			}
		}
	}
#endif

	return count;
}
#endif

void CHARACTER::RemoveSpecifyItem(DWORD vnum, DWORD count)
{
	if (0 == count)
		return;

	for (int i = 0; i < GetInventoryMax(); ++i)
	{
		if (NULL == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetVnum() != vnum)
			continue;

		//???? ?????? ?????? ???????? ????????. (???? ???????? ???????? ?? ???????? ?????? ???? ????!)
		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (vnum >= 80003 && vnum <= 80007)
			LogManager::instance().GoldBarLog(GetPlayerID(), GetInventoryItem(i)->GetID(), QUEST, "RemoveSpecifyItem");

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}

#ifdef __NEW_STORAGE__
	for (int j = 0; j < STORAGE_INVENTORY_COUNT; ++j)
	{
		for (UINT i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
		{
			LPITEM item = GetStorageInventoryItem(j, i);
			if (!item || item->GetVnum() != vnum)
				continue;

			if (m_pkMyShop)
			{
				bool isItemSelling = m_pkMyShop->IsSellingItem(item->GetID());
				if (isItemSelling)
					continue;
			}

			if (count >= item->GetCount())
			{
				count -= item->GetCount();
				item->SetCount(0);

				if (0 == count)
					return;
			}
			else
			{
				item->SetCount(item->GetCount() - count);
				return;
			}
		}
	}
#endif

	// ?????????? ??????.
	if (count)
		sys_log(0, "CHARACTER::RemoveSpecifyItem cannot remove enough item vnum %u, still remain %d", vnum, count);
}

#ifdef __PENDANT__
void CHARACTER::RemoveSpecifyItemTheOutItem(DWORD vnum, DWORD count, LPITEM item)
{
	if (0 == count)
		return;

	for (int i = 0; i < GetInventoryMax(); ++i)
	{
		if (NULL == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetVnum() != vnum)
			continue;

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (GetInventoryItem(i)->GetVID() == item->GetVID())
			continue;

		if (vnum >= 80003 && vnum <= 80007)
			LogManager::instance().GoldBarLog(GetPlayerID(), GetInventoryItem(i)->GetID(), QUEST, "RemoveSpecifyItem");

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}

#ifdef __NEW_STORAGE__
	for (int j = 0; j < STORAGE_INVENTORY_COUNT; ++j)
	{
		for (UINT i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
		{
			LPITEM item2 = GetStorageInventoryItem(j, i);
			if (!item2 || item2->GetVnum() != vnum)
				continue;

			if (m_pkMyShop)
			{
				bool isItemSelling = m_pkMyShop->IsSellingItem(item2->GetID());
				if (isItemSelling)
					continue;
			}

			if (item2->GetVID() == item->GetVID())
				continue;

			if (count >= item2->GetCount())
			{
				count -= item2->GetCount();
				item2->SetCount(0);

				if (0 == count)
					return;
			}
			else
			{
				item2->SetCount(item2->GetCount() - count);
				return;
			}
		}
	}
#endif

	if (count)
		sys_log(0, "CHARACTER::RemoveSpecifyItem cannot remove enough item vnum %u, still remain %d", vnum, count);
}
#endif

int CHARACTER::CountSpecifyTypeItem(BYTE type) const
{
	int	count = 0;

	for (int i = 0; i < GetInventoryMax(); ++i)
	{
		LPITEM pItem = GetInventoryItem(i);
		if (pItem != NULL && pItem->GetType() == type)
			count += pItem->GetCount();
	}

#ifdef __NEW_STORAGE__
	for (int j = 0; j < STORAGE_INVENTORY_COUNT; ++j)
	{
		for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
		{
			LPITEM pItem = GetStorageInventoryItem(j, i);
			if (pItem != NULL && pItem->GetType() == type)
				count += pItem->GetCount();
		}
	}
#endif

	return count;
}

void CHARACTER::RemoveSpecifyTypeItem(BYTE type, DWORD count)
{
	if (0 == count)
		return;

	for (int i = 0; i < GetInventoryMax(); ++i)
	{
		if (NULL == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetType() != type)
			continue;

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}

#ifdef __NEW_STORAGE__
	for (int j = 0; j < STORAGE_INVENTORY_COUNT; ++j)
	{
		for (UINT i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
		{
			LPITEM item = GetStorageInventoryItem(j, i);
			if (!item || item->GetType() != type)
				continue;

			if (m_pkMyShop)
			{
				bool isItemSelling = m_pkMyShop->IsSellingItem(item->GetID());
				if (isItemSelling)
					continue;
			}

			if (count >= item->GetCount())
			{
				count -= item->GetCount();
				item->SetCount(0);

				if (0 == count)
					return;
			}
			else
			{
				item->SetCount(item->GetCount() - count);
				return;
			}
		}
	}
#endif
}

void CHARACTER::AutoGiveItem(LPITEM item, bool longOwnerShip)
{
	if (NULL == item)
	{
		sys_err ("NULL point.");
		return;
	}
	if (item->GetOwner())
	{
		sys_err ("item %d 's owner exists!",item->GetID());
		return;
	}

#ifdef __NEW_STORAGE__
	if ((item->IsStorageItem()) && item->IsStackable())
	{
		BYTE bCount = item->GetCount();

		for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
		{
			LPITEM item2 = GetStorageInventoryItem(item->GetStorageType() - UPGRADE_INVENTORY, i);

			if (!item2)
				continue;

			if ((item2->GetVnum() == item->GetVnum() && !item->IsBook()) || (item->IsBook() && item2->GetVnum() == item->GetVnum() && item2->GetSocket(0) == item->GetSocket(0)))
			{

				BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
				bCount -= bCount2;

				item2->SetCount(item2->GetCount() + bCount2);

				if (bCount == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s"), item2->GetMultiName());
					M2_DESTROY_ITEM(item);
					return;
				}
			}
		}

		item->SetCount(bCount);
	}
#endif

	if (item->IsStackable())
	{
		BYTE bCount = item->GetCount();

		for (int i = 0; i < GetInventoryMax(); ++i)
		{
			LPITEM item2 = GetInventoryItem(i);

			if (!item2)
				continue;

			if (item2->GetVnum() == item->GetVnum())
			{
				int j;

				for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
					if (item2->GetSocket(j) != item->GetSocket(j))
						break;

				if (j != ITEM_SOCKET_MAX_NUM)
					continue;

				BYTE bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
				bCount -= bCount2;

				item2->SetCount(item2->GetCount() + bCount2);

				if (bCount == 0)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s"), item2->GetMultiName());
					M2_DESTROY_ITEM(item);
					if (item2->GetType() == ITEM_QUEST)
						quest::CQuestManager::instance().PickupItem (GetPlayerID(), item2);
					return;
				}
			}
		}

		item->SetCount(bCount);
	}

	int cell;
	if (item->IsDragonSoul())
		cell = GetEmptyDragonSoulInventory(item);
#ifdef __NEW_STORAGE__
	else if (item->IsStorageItem())
		cell = GetEmptyStorageInventory(item->GetStorageType() - UPGRADE_INVENTORY, item);
#endif
	else
		cell = GetEmptyInventory(item->GetSize());

	if (cell != -1)
	{
		if (item->IsDragonSoul())
			item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, cell), true);
#ifdef __NEW_STORAGE__
		else if (item->IsStorageItem())
			item->AddToCharacter(this, TItemPos(UPGRADE_INVENTORY + (item->GetStorageType() - UPGRADE_INVENTORY), cell), true);
#endif
		else
			item->AddToCharacter(this, TItemPos(INVENTORY, cell), true);

		LogManager::instance().ItemLog(this, item, "SYSTEM", item->GetName());

		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_POTION)
		{
			TQuickslot * pSlot;

			if (GetQuickslot(0, &pSlot) && pSlot->type == QUICKSLOT_TYPE_NONE)
			{
				TQuickslot slot;
				slot.type = QUICKSLOT_TYPE_ITEM;
				slot.pos = cell;
				SetQuickslot(0, slot);
			}
		}
	}
	else
	{
		item->AddToGround(GetMapIndex(), GetXYZ());
#ifdef __NEWSTUFF__
		item->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);
#else
		item->StartDestroyEvent();
#endif

		if (longOwnerShip)
			item->SetOwnership (this, 300);
		else
			item->SetOwnership (this, 60);
		LogManager::instance().ItemLog(this, item, "SYSTEM_DROP", item->GetName());
	}
}

LPITEM CHARACTER::AutoGiveItem(DWORD dwItemVnum, BYTE bCount, int iRarePct, bool bMsg, BYTE inven)
{
	TItemTable * p = ITEM_MANAGER::instance().GetTable(dwItemVnum);

	if (!p)
		return NULL;

	LogManager::instance().MoneyLog(MONEY_LOG_DROP, dwItemVnum, bCount);

	if (p->dwFlags & ITEM_FLAG_STACKABLE && p->bType != ITEM_BLEND)
	{
		switch(inven)
		{
			case DRAGON_SOUL_INVENTORY:
				for (int i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item = GetDragonSoulInventoryItem(i);

					if (!item)
						continue;

					if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
					{
						if (IS_SET(p->dwFlags, ITEM_FLAG_MAKECOUNT))
						{
							if (bCount < p->alValues[1])
								bCount = p->alValues[1];
						}

						BYTE bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
						bCount -= bCount2;

						item->SetCount(item->GetCount() + bCount2);

						if (bCount == 0)
						{
							if (bMsg)
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s"), item->GetMultiName());

							return item;
						}
					}
				}
				break;
#ifdef __NEW_STORAGE__
			case UPGRADE_INVENTORY:
			case BOOK_INVENTORY:
			case STONE_INVENTORY:
			case ATTR_INVENTORY:
			case GIFTBOX_INVENTORY:
				for (int i = 0; i < STORAGE_INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item = GetStorageInventoryItem(inven - UPGRADE_INVENTORY, i);

					if (!item)
						continue;

					if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
					{
						if (IS_SET(p->dwFlags, ITEM_FLAG_MAKECOUNT))
						{
							if (bCount < p->alValues[1])
								bCount = p->alValues[1];
						}

						BYTE bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
						bCount -= bCount2;

						item->SetCount(item->GetCount() + bCount2);

						if (bCount == 0)
						{
							if (bMsg)
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s"), item->GetMultiName());

							return item;
						}
					}
				}
				break;
#endif
			default:
				for (int i = 0; i < GetInventoryMax(); ++i)
				{
					LPITEM item = GetInventoryItem(i);

					if (!item)
						continue;

					if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
					{
						if (IS_SET(p->dwFlags, ITEM_FLAG_MAKECOUNT))
						{
							if (bCount < p->alValues[1])
								bCount = p->alValues[1];
						}

						BYTE bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
						bCount -= bCount2;

						item->SetCount(item->GetCount() + bCount2);

						if (bCount == 0)
						{
							if (bMsg)
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s"), item->GetMultiName());

							return item;
						}
					}
				}
				break;
		}
	}

	LPITEM item = ITEM_MANAGER::instance().CreateItem(dwItemVnum, bCount, 0, true);

	if (!item)
	{
		sys_err("cannot create item by vnum %u (name: %s)", dwItemVnum, GetName());
		return NULL;
	}

	if (item->GetType() == ITEM_BLEND)
	{
		for (int i=0; i < GetInventoryMax(); i++)
		{
			LPITEM inv_item = GetInventoryItem(i);

			if (inv_item == NULL) continue;

			if (inv_item->GetType() == ITEM_BLEND)
			{
				if (inv_item->GetVnum() == item->GetVnum())
				{
					if (inv_item->GetSocket(0) == item->GetSocket(0) &&
							inv_item->GetSocket(1) == item->GetSocket(1) &&
							inv_item->GetSocket(2) == item->GetSocket(2) &&
							inv_item->GetCount() < g_bItemCountLimit)
					{
						inv_item->SetCount(inv_item->GetCount() + item->GetCount());
						return inv_item;
					}
				}
			}
		}
	}

	int iEmptyCell;
	if (item->IsDragonSoul())
	{
		iEmptyCell = GetEmptyDragonSoulInventory(item);
	}
#ifdef __NEW_STORAGE__
	else if (item->IsStorageItem())
		iEmptyCell = GetEmptyStorageInventory(item->GetStorageType() - UPGRADE_INVENTORY, item);
#endif
	else
		iEmptyCell = GetEmptyInventory(item->GetSize());

	if (iEmptyCell != -1)
	{
		if (bMsg)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ????: %s"), item->GetMultiName());

		if (item->IsDragonSoul())
			item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell), true);
#ifdef __NEW_STORAGE__
		else if (item->IsStorageItem())
			item->AddToCharacter(this, TItemPos(UPGRADE_INVENTORY + (item->GetStorageType() - UPGRADE_INVENTORY), iEmptyCell), true);
#endif
		else
			item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyCell), true);
		LogManager::instance().ItemLog(this, item, "SYSTEM", item->GetName());

		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_POTION)
		{
			TQuickslot * pSlot;

			if (GetQuickslot(0, &pSlot) && pSlot->type == QUICKSLOT_TYPE_NONE)
			{
				TQuickslot slot;
				slot.type = QUICKSLOT_TYPE_ITEM;
				slot.pos = iEmptyCell;
				SetQuickslot(0, slot);
			}
		}
	}
	else
	{
		item->AddToGround(GetMapIndex(), GetXYZ());
#ifdef __NEWSTUFF__
		item->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);
#else
		item->StartDestroyEvent();
#endif
		if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP))
			item->SetOwnership(this, 300);
		else
			item->SetOwnership(this, 60);
		LogManager::instance().ItemLog(this, item, "SYSTEM_DROP", item->GetName());
	}

	return item;
}

bool CHARACTER::GiveItem(LPCHARACTER victim, TItemPos Cell)
{
	if (!CanHandleItem())
		return false;

	// @fixme150 BEGIN
	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot take this item if you're using quests"));
		return false;
	}
	// @fixme150 END

	LPITEM item = GetItem(Cell);

	if (item && !item->IsExchanging())
	{
		if (victim->CanReceiveItem(this, item))
		{
			victim->ReceiveItem(this, item);
			return true;
		}
	}

	return false;
}

bool CHARACTER::CanReceiveItem(LPCHARACTER from, LPITEM item) const
{
	if (IsPC())
		return false;

	// TOO_LONG_DISTANCE_EXCHANGE_BUG_FIX
	if (DISTANCE_APPROX(GetX() - from->GetX(), GetY() - from->GetY()) > 2000)
		return false;
	// END_OF_TOO_LONG_DISTANCE_EXCHANGE_BUG_FIX

#ifdef __SOULBIND__
	if (item->IsSealed())
	{
		from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't upgrade sealbind item."));
		return false;
	}
#endif

	if (item->IsBasicItem())
		return false;

	switch (GetRaceNum())
	{
		case fishing::CAMPFIRE_MOB:
			if (item->GetType() == ITEM_FISH &&
					(item->GetSubType() == FISH_ALIVE || item->GetSubType() == FISH_DEAD))
				return true;
			break;

		case fishing::FISHER_MOB:
			if (item->GetType() == ITEM_ROD)
				return true;
			break;

			// BUILDING_NPC
		case BLACKSMITH_WEAPON_MOB:
		case DEVILTOWER_BLACKSMITH_WEAPON_MOB:
			if (item->GetType() == ITEM_WEAPON &&
					item->GetRefinedVnum())
				return true;
			else
				return false;
			break;

		case BLACKSMITH_ARMOR_MOB:
		case DEVILTOWER_BLACKSMITH_ARMOR_MOB:
			if (item->GetType() == ITEM_ARMOR &&
					(item->GetSubType() == ARMOR_BODY || item->GetSubType() == ARMOR_SHIELD || item->GetSubType() == ARMOR_HEAD) &&
					item->GetRefinedVnum())
				return true;
			else
				return false;
			break;

		case BLACKSMITH_ACCESSORY_MOB:
		case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB:
			if (item->GetType() == ITEM_ARMOR &&
					!(item->GetSubType() == ARMOR_BODY || item->GetSubType() == ARMOR_SHIELD || item->GetSubType() == ARMOR_HEAD) &&
					item->GetRefinedVnum())
				return true;
			else
				return false;
			break;
			// END_OF_BUILDING_NPC

		case BLACKSMITH_MOB:
			if (item->GetRefinedVnum() && item->GetRefineSet() < 500)
			{
				return true;
			}
			else
			{
				return false;
			}

		case BLACKSMITH2_MOB:
			if (item->GetRefineSet() >= 500)
			{
				return true;
			}
			else
			{
				return false;
			}

		case ALCHEMIST_MOB:
			if (item->GetRefinedVnum())
				return true;
			break;

		case 20101:
		case 20102:
		case 20103:
			// ???? ??
			if (item->GetVnum() == ITEM_REVIVE_HORSE_1)
			{
				if (!IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???? ?????? ?????? ???? ?? ????????."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1)
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ???? ?? ????????."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_2 || item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				return false;
			}
			break;
		case 20104:
		case 20105:
		case 20106:
			// ???? ??
			if (item->GetVnum() == ITEM_REVIVE_HORSE_2)
			{
				if (!IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???? ?????? ?????? ???? ?? ????????."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_2)
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ???? ?? ????????."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 || item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				return false;
			}
			break;
		case 20107:
		case 20108:
		case 20109:
			// ???? ??
			if (item->GetVnum() == ITEM_REVIVE_HORSE_3)
			{
				if (!IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???? ?????? ?????? ???? ?? ????????."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ???? ?? ????????."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 || item->GetVnum() == ITEM_HORSE_FOOD_2)
			{
				return false;
			}
			break;
	}

	//if (IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_GIVE))
	{
		return true;
	}

	return false;
}

void CHARACTER::ReceiveItem(LPCHARACTER from, LPITEM item)
{
	if (IsPC())
		return;

	switch (GetRaceNum())
	{
#ifdef __GUILD_DRAGONLAIR__
		case MeleyLair::STATUE_VNUM:
			{
				if (MeleyLair::CMgr::instance().IsMeleyMap(from->GetMapIndex()))
					MeleyLair::CMgr::instance().OnKillStatue(item, from, this, from->GetGuild());
			}
			break;
#endif
		case fishing::CAMPFIRE_MOB:
			if (item->GetType() == ITEM_FISH && (item->GetSubType() == FISH_ALIVE || item->GetSubType() == FISH_DEAD))
				fishing::Grill(from, item);
			else
			{
				// TAKE_ITEM_BUG_FIX
				from->SetQuestNPCID(GetVID());
				// END_OF_TAKE_ITEM_BUG_FIX
				quest::CQuestManager::instance().TakeItem(from->GetPlayerID(), GetRaceNum(), item);
			}
			break;

			// DEVILTOWER_NPC
		case DEVILTOWER_BLACKSMITH_WEAPON_MOB:
		case DEVILTOWER_BLACKSMITH_ARMOR_MOB:
		case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB:
			if (item->GetRefinedVnum() != 0 && item->GetRefineSet() != 0 && item->GetRefineSet() < 500)
			{
				from->SetRefineNPC(this);
				from->RefineInformation(item->GetCell(), REFINE_TYPE_MONEY_ONLY);
			}
			else
			{
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?????? ?? ????????."));
			}
			break;
			// END_OF_DEVILTOWER_NPC

		case BLACKSMITH_MOB:
		case BLACKSMITH2_MOB:
		case BLACKSMITH_WEAPON_MOB:
		case BLACKSMITH_ARMOR_MOB:
		case BLACKSMITH_ACCESSORY_MOB:
			if (item->GetRefinedVnum())
			{
				from->SetRefineNPC(this);
				from->RefineInformation(item->GetCell(), REFINE_TYPE_NORMAL);
			}
			else
			{
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?? ???????? ?????? ?? ????????."));
			}
			break;

		case 20101:
		case 20102:
		case 20103:
		case 20104:
		case 20105:
		case 20106:
		case 20107:
		case 20108:
		case 20109:
			if (item->GetVnum() == ITEM_REVIVE_HORSE_1 ||
					item->GetVnum() == ITEM_REVIVE_HORSE_2 ||
					item->GetVnum() == ITEM_REVIVE_HORSE_3)
			{
				from->ReviveHorse();
				item->SetCount(item->GetCount()-1);
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ??????????."));
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 ||
					item->GetVnum() == ITEM_HORSE_FOOD_2 ||
					item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				from->FeedHorse();
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?????? ??????????."));
				item->SetCount(item->GetCount()-1);
				EffectPacket(SE_HPUP_RED);
			}
			break;

		default:
			sys_log(0, "TakeItem %s %d %s", from->GetName(), GetRaceNum(), item->GetName());
			from->SetQuestNPCID(GetVID());
			quest::CQuestManager::instance().TakeItem(from->GetPlayerID(), GetRaceNum(), item);
			break;
	}
}

bool CHARACTER::IsEquipUniqueItem(DWORD dwItemVnum) const
{
	{
		LPITEM u = GetWear(WEAR_UNIQUE1);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}

	{
		LPITEM u = GetWear(WEAR_UNIQUE2);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}

#ifdef __COSTUME_MOUNT__
	{
		LPITEM u = GetWear(WEAR_COSTUME_MOUNT);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}
#endif

	return false;
}

// CHECK_UNIQUE_GROUP
bool CHARACTER::IsEquipUniqueGroup(DWORD dwGroupVnum) const
{
	{
		LPITEM u = GetWear(WEAR_UNIQUE1);

		if (u && u->GetSpecialGroup() == (int) dwGroupVnum)
			return true;
	}

	{
		LPITEM u = GetWear(WEAR_UNIQUE2);

		if (u && u->GetSpecialGroup() == (int) dwGroupVnum)
			return true;
	}

#ifdef __COSTUME_MOUNT__
	{
		LPITEM u = GetWear(WEAR_COSTUME_MOUNT);

		if (u && u->GetSpecialGroup() == (int)dwGroupVnum)
			return true;
	}
#endif
	return false;
}
// END_OF_CHECK_UNIQUE_GROUP

void CHARACTER::SetRefineMode(int iAdditionalCell)
{
	m_iRefineAdditionalCell = iAdditionalCell;
	m_bUnderRefine = true;
}

void CHARACTER::ClearRefineMode()
{
	m_bUnderRefine = false;
	SetRefineNPC( NULL );
}

bool CHARACTER::GiveItemFromSpecialItemGroup(DWORD dwGroupNum, std::vector<DWORD> &dwItemVnums, std::vector<DWORD> &dwItemCounts, std::vector <LPITEM> &item_gets, int &count)
{
	const CSpecialItemGroup* pGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(dwGroupNum);

	if (!pGroup)
	{
		sys_err("cannot find special item group %d", dwGroupNum);
		return false;
	}

	std::vector <int> idxes;
	int n = pGroup->GetMultiIndex(idxes);

	bool bSuccess;

	for (int i = 0; i < n; i++)
	{
		bSuccess = false;
		int idx = idxes[i];
		DWORD dwVnum = pGroup->GetVnum(idx);
		DWORD dwCount = pGroup->GetCount(idx);
		int	iRarePct = pGroup->GetRarePct(idx);
		LPITEM item_get = NULL;
		switch (dwVnum)
		{
			case CSpecialItemGroup::GOLD:
				PointChange(POINT_GOLD, dwCount);
				LogManager::instance().CharLog(this, dwCount, "TREASURE_GOLD", "");

				bSuccess = true;
				break;
			case CSpecialItemGroup::EXP:
				{
					PointChange(POINT_EXP, dwCount);
					LogManager::instance().CharLog(this, dwCount, "TREASURE_EXP", "");

					bSuccess = true;
				}
				break;

			case CSpecialItemGroup::MOB:
				{
					sys_log(0, "CSpecialItemGroup::MOB %d", dwCount);
					int x = GetX() + number(-500, 500);
					int y = GetY() + number(-500, 500);

					LPCHARACTER ch = CHARACTER_MANAGER::instance().SpawnMob(dwCount, GetMapIndex(), x, y, 0, true, -1);
					if (ch)
						ch->SetAggressive();
					bSuccess = true;
				}
				break;
			case CSpecialItemGroup::SLOW:
				{
					sys_log(0, "CSpecialItemGroup::SLOW %d", -(int)dwCount);
					AddAffect(AFFECT_SLOW, POINT_MOV_SPEED, -(int)dwCount, AFF_SLOW, 300, 0, true);
					bSuccess = true;
				}
				break;
			case CSpecialItemGroup::DRAIN_HP:
				{
					int iDropHP = GetMaxHP()*dwCount/100;
					sys_log(0, "CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
					iDropHP = MIN(iDropHP, GetHP()-1);
					sys_log(0, "CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
					PointChange(POINT_HP, -iDropHP);
					bSuccess = true;
				}
				break;
			case CSpecialItemGroup::POISON:
				{
					AttackedByPoison(NULL);
					bSuccess = true;
				}
				break;
#ifdef __WOLFMAN__
			case CSpecialItemGroup::BLEEDING:
				{
					AttackedByBleeding(NULL);
					bSuccess = true;
				}
				break;
#endif
			case CSpecialItemGroup::MOB_GROUP:
				{
					int sx = GetX() - number(300, 500);
					int sy = GetY() - number(300, 500);
					int ex = GetX() + number(300, 500);
					int ey = GetY() + number(300, 500);
					CHARACTER_MANAGER::instance().SpawnGroup(dwCount, GetMapIndex(), sx, sy, ex, ey, NULL, true);

					bSuccess = true;
				}
				break;
			default:
				{
					BYTE bInven = INVENTORY;
					const TItemTable* table = ITEM_MANAGER::Instance().GetTable(dwVnum);
					if (table->bType == ITEM_DS)
						bInven = DRAGON_SOUL_INVENTORY;
#ifdef __NEW_STORAGE__
					else if (table->bType == ITEM_MATERIAL && table->bSubType == MATERIAL_LEATHER)
						bInven = UPGRADE_INVENTORY;
					else if (table->bType == ITEM_SKILLBOOK)
						bInven = BOOK_INVENTORY;
					else if (table->bType == ITEM_METIN && table->bSubType == METIN_NORMAL)
						bInven = STONE_INVENTORY;
					else if (ITEM_MEDIUM == table->bType || (table->bType == ITEM_USE && (table->bSubType == USE_ADD_ATTRIBUTE || table->bSubType == USE_ADD_ATTRIBUTE2 || table->bSubType == USE_CHANGE_ATTRIBUTE || table->bSubType == USE_CHANGE_ATTRIBUTE2 || table->bSubType == USE_RESET_COSTUME_ATTR || table->bSubType == USE_ADD_COSTUME_ATTRIBUTE || table->bSubType == USE_CHANGE_ATTRIBUTE_PLUS)))
						bInven = ATTR_INVENTORY;
					else if (table->bType == ITEM_GIFTBOX)
						bInven = GIFTBOX_INVENTORY;
#endif
					
					item_get = AutoGiveItem(dwVnum, dwCount, iRarePct, bInven);

					if (item_get)
					{
						bSuccess = true;
					}
				}
				break;
		}

		if (bSuccess)
		{
			dwItemVnums.push_back(dwVnum);
			dwItemCounts.push_back(dwCount);
			item_gets.push_back(item_get);
			count++;

		}
		else
		{
			return false;
		}
	}
	return bSuccess;
}

// NEW_HAIR_STYLE_ADD
bool CHARACTER::ItemProcess_Hair(LPITEM item, int iDestCell)
{
	if (item->CheckItemUseLevel(GetLevel()) == false)
	{
		// ???? ?????? ????
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?? ?????? ?????? ?? ???? ??????????."));
		return false;
	}

	DWORD hair = item->GetVnum();

	switch (GetJob())
	{
		case JOB_WARRIOR :
			hair -= 72000; // 73001 - 72000 = 1001 ???? ???? ???? ????
			break;

		case JOB_ASSASSIN :
			hair -= 71250;
			break;

		case JOB_SURA :
			hair -= 70500;
			break;

		case JOB_SHAMAN :
			hair -= 69750;
			break;
#ifdef __WOLFMAN__
		case JOB_WOLFMAN:
			break; // NOTE: ?? ?????????? ?? ???????? ????. (???? ???????????? ???? ?????????? ???? ?? ??????)
#endif
		default :
			return false;
			break;
	}

	if (hair == GetPart(PART_HAIR))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ?????????? ?????? ?? ????????."));
		return true;
	}

	item->SetCount(item->GetCount() - 1);

	SetPart(PART_HAIR, hair);
	UpdatePacket();

	return true;
}
// END_NEW_HAIR_STYLE_ADD

bool CHARACTER::ItemProcess_Polymorph(LPITEM item)
{
	if (IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ???????? ??????????."));
		return false;
	}

	if (IsRiding())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ?? ???? ??????????."));
		return false;
	}

	if (IsOnPolymorphMapBlacklist(GetMapIndex()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("????????? ?????? ???????????????"));
		return false;
	}

	DWORD dwVnum = item->GetSocket(0);

	if (dwVnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ????????????."));
		item->SetCount(item->GetCount()-1);
		return false;
	}

	const CMob* pMob = CMobManager::instance().Get(dwVnum);

	if (pMob == NULL)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ????????????."));
		item->SetCount(item->GetCount()-1);
		return false;
	}

	switch (item->GetVnum())
	{
		case 70104:
		case 70105:
		case 70106:
		case 70107:
		case 71093:
			{
				// ?????? ????
				sys_log(0, "USE_POLYMORPH_BALL PID(%d) vnum(%d)", GetPlayerID(), dwVnum);

				// ???? ???? ????
				int iPolymorphLevelLimit = MAX(0, 20 - GetLevel() * 3 / 10);
				if (pMob->m_table.bLevel >= GetLevel() + iPolymorphLevelLimit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ???? ?????? ?????????? ???? ?? ?? ????????."));
					return false;
				}

				int iDuration = GetSkillLevel(POLYMORPH_SKILL_ID) == 0 ? 5 : (5 + (5 + GetSkillLevel(POLYMORPH_SKILL_ID)/40 * 25));
				iDuration *= 60;

				DWORD dwBonus = 0;

				dwBonus = (2 + GetSkillLevel(POLYMORPH_SKILL_ID)/40) * 100;

				AddAffect(AFFECT_POLYMORPH, POINT_POLYMORPH, dwVnum, AFF_POLYMORPH, iDuration, 0, true);
				AddAffect(AFFECT_POLYMORPH, POINT_ATT_BONUS, dwBonus, AFF_POLYMORPH, iDuration, 0, false);

				// Run this so that skills are taken
				// into account after the poly changes
				ComputePoints();
				
				item->SetCount(item->GetCount()-1);
			}
			break;

		default :
			sys_err("POLYMORPH invalid item passed PID(%d) vnum(%d)", GetPlayerID(), item->GetOriginalVnum());
			return false;
	}

	return true;
}

bool CHARACTER::CanDoCube() const
{
	if (m_bIsObserver)	return false;
	if (GetShop())		return false;
	if (GetMyShop())	return false;
	if (m_bUnderRefine)	return false;
	if (IsWarping())	return false;

	return true;
}

bool CHARACTER::UnEquipSpecialRideUniqueItem()
{
	LPITEM Unique1 = GetWear(WEAR_UNIQUE1);
	LPITEM Unique2 = GetWear(WEAR_UNIQUE2);
#if defined(__COSTUME_MOUNT__) && !defined(__MOUNT__)
	LPITEM Unique3 = GetWear(WEAR_COSTUME_MOUNT);
#endif

	if( NULL != Unique1 )
	{
		if( UNIQUE_GROUP_SPECIAL_RIDE == Unique1->GetSpecialGroup() )
		{
			return UnequipItem(Unique1);
		}
	}

	if( NULL != Unique2 )
	{
		if( UNIQUE_GROUP_SPECIAL_RIDE == Unique2->GetSpecialGroup() )
		{
			return UnequipItem(Unique2);
		}
	}

#if defined(__COSTUME_MOUNT__) && !defined(__MOUNT__)
	if (NULL != Unique3)
		if (UNIQUE_GROUP_SPECIAL_RIDE == Unique3->GetSpecialGroup())
			return UnequipItem(Unique3);
#endif

	return true;
}

void CHARACTER::AutoRecoveryItemProcess(const EAffectTypes type)
{
	if (true == IsDead() || true == IsStun())
		return;

	if (false == IsPC())
		return;

	if (AFFECT_AUTO_HP_RECOVERY != type && AFFECT_AUTO_SP_RECOVERY != type)
		return;

	if (NULL != FindAffect(AFFECT_STUN))
		return;

	{
		const DWORD stunSkills[] = { SKILL_TANHWAN, SKILL_GEOMPUNG, SKILL_BYEURAK, SKILL_GIGUNG };

		for (size_t i=0 ; i < sizeof(stunSkills)/sizeof(DWORD) ; ++i)
		{
			const CAffect* p = FindAffect(stunSkills[i]);

			if (NULL != p && AFF_STUN == p->dwFlag)
				return;
		}
	}

	const CAffect* pAffect = FindAffect(type);
	const size_t idx_of_amount_of_used = 1;
	const size_t idx_of_amount_of_full = 2;

	if (NULL != pAffect)
	{
		LPITEM pItem = FindItemByID(pAffect->dwFlag);

		if (NULL != pItem && true == pItem->GetSocket(0))
		{
			if (!CArenaManager::instance().IsArenaMap(GetMapIndex())
#ifdef __NEWSTUFF__
				&& !(g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(pItem->GetVnum()))
#endif
			)
			{
				const long amount_of_used = pItem->GetSocket(idx_of_amount_of_used);
				const long amount_of_full = pItem->GetSocket(idx_of_amount_of_full);

				const int32_t avail = amount_of_full - amount_of_used;

				int32_t amount = 0;

				if (AFFECT_AUTO_HP_RECOVERY == type)
				{
					amount = GetMaxHP() - (GetHP() + GetPoint(POINT_HP_RECOVERY));
				}
				else if (AFFECT_AUTO_SP_RECOVERY == type)
				{
					amount = GetMaxSP() - (GetSP() + GetPoint(POINT_SP_RECOVERY));
				}

				if (amount > 0)
				{
					if (avail > amount)
					{
						const int pct_of_used = amount_of_used * 100 / amount_of_full;
						const int pct_of_will_used = (amount_of_used + amount) * 100 / amount_of_full;

						bool bLog = false;
						// ???????? 10% ?????? ?????? ????
						// (???????? %????, ???? ?????? ???? ?????? ?????? ????.)
						if ((pct_of_will_used / 10) - (pct_of_used / 10) >= 1)
							bLog = true;
						pItem->SetSocket(idx_of_amount_of_used, amount_of_used + amount, bLog);
					}
					else
					{
						amount = avail;

						ITEM_MANAGER::instance().RemoveItem( pItem );
					}

					if (AFFECT_AUTO_HP_RECOVERY == type)
					{
						PointChange( POINT_HP_RECOVERY, amount );
						EffectPacket( SE_AUTO_HPUP );
					}
					else if (AFFECT_AUTO_SP_RECOVERY == type)
					{
						PointChange( POINT_SP_RECOVERY, amount );
						EffectPacket( SE_AUTO_SPUP );
					}
				}
			}
			else
			{
				pItem->Lock(false);
				pItem->SetSocket(0, false);
				RemoveAffect( const_cast<CAffect*>(pAffect) );
			}
		}
		else
		{
			RemoveAffect( const_cast<CAffect*>(pAffect) );
		}
	}
}

bool CHARACTER::IsValidItemPosition(TItemPos Pos) const
{
	BYTE window_type = Pos.window_type;
	WORD cell = Pos.cell;

	switch (window_type)
	{
	case RESERVED_WINDOW:
		return false;

	case INVENTORY:
	case EQUIPMENT:
		return cell < (INVENTORY_AND_EQUIP_SLOT_MAX);

	case DRAGON_SOUL_INVENTORY:
		return cell < (DRAGON_SOUL_INVENTORY_MAX_NUM);

#ifdef __NEW_STORAGE__
	case UPGRADE_INVENTORY:
	case BOOK_INVENTORY:
	case STONE_INVENTORY:
	case ATTR_INVENTORY:
	case GIFTBOX_INVENTORY:
		return cell < (STORAGE_INVENTORY_MAX_NUM);
#endif

	case SAFEBOX:
		if (NULL != m_pkSafebox)
			return m_pkSafebox->IsValidPosition(cell);
		else
			return false;

	case MALL:
		if (NULL != m_pkMall)
			return m_pkMall->IsValidPosition(cell);
		else
			return false;
	default:
		return false;
	}
}


#define VERIFY_MSG(exp, msg)  \
	if (true == (exp)) { \
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT(msg)); \
			return false; \
	}

bool CHARACTER::CanEquipNow(const LPITEM item, const TItemPos& srcCell, const TItemPos& destCell) /*const*/
{
	const TItemTable* itemTable = item->GetProto();
	//BYTE itemType = item->GetType();
	//BYTE itemSubType = item->GetSubType();

	switch (GetJob())
	{
		case JOB_WARRIOR:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
				return false;
			break;

		case JOB_ASSASSIN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
				return false;
			break;

		case JOB_SHAMAN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
				return false;
			break;

		case JOB_SURA:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_SURA)
				return false;
			break;
#ifdef __WOLFMAN__
		case JOB_WOLFMAN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
				return false;
			break; // TODO: ?????? ?????? ???????????? ????
#endif
	}

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		long limit = itemTable->aLimits[i].lValue;
		switch (itemTable->aLimits[i].bType)
		{
			case LIMIT_LEVEL:
				if (GetLevel() < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ?????? ?? ????????."));
					return false;
				}
				break;

			case LIMIT_STR:
				if (GetPoint(POINT_ST) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ?????? ?? ????????."));
					return false;
				}
				break;

			case LIMIT_INT:
				if (GetPoint(POINT_IQ) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ?????? ?? ????????."));
					return false;
				}
				break;

			case LIMIT_DEX:
				if (GetPoint(POINT_DX) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ?????? ?? ????????."));
					return false;
				}
				break;

			case LIMIT_CON:
				if (GetPoint(POINT_HT) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("?????? ???? ?????? ?? ????????."));
					return false;
				}
				break;
		}
	}

	if (item->GetWearFlag() & WEARABLE_UNIQUE)
	{
		if ((GetWear(WEAR_UNIQUE1) && GetWear(WEAR_UNIQUE1)->IsSameSpecialGroup(item)) ||
			(GetWear(WEAR_UNIQUE2) && GetWear(WEAR_UNIQUE2)->IsSameSpecialGroup(item))
#ifdef __COSTUME_MOUNT__
 || (GetWear(WEAR_COSTUME_MOUNT) && GetWear(WEAR_COSTUME_MOUNT)->IsSameSpecialGroup(item))
#endif
)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???? ?????? ?????? ?????? ?? ???? ?????? ?????? ?? ????????."));
			return false;
		}

		if (marriage::CManager::instance().IsMarriageUniqueItem(item->GetVnum()) &&
			!marriage::CManager::instance().IsMarried(GetPlayerID()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???????? ???? ???????? ?????? ?????? ?? ????????."));
			return false;
		}

	}

#ifdef __COSTUME_WEAPON__
	if (item->GetType() == ITEM_WEAPON && item->GetSubType() != WEAPON_ARROW)
	{
		LPITEM pkItem = GetWear(WEAR_COSTUME_WEAPON);
		if (pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("For can do this unwear the costume weapon."));
			return false;
		}
	}
	else if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_WEAPON)
	{
		LPITEM pkItem = GetWear(WEAR_WEAPON);
		if (!pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't wear a costume weapon without have a weapon weared."));
			return false;
		}
		else if (item->GetValue(3) != pkItem->GetSubType())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't wear a costume weapon who has different type of your weapon."));
			return false;
		}
        else if (pkItem->GetType() == ITEM_ROD || pkItem->GetType() == ITEM_PICK)
            return false;
	}
	
	if (item->GetType() == ITEM_ROD || item->GetType() == ITEM_PICK)
	{
		LPITEM pkItem = GetWear(WEAR_COSTUME_WEAPON);
		if (pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("For can do this unwear the costume weapon."));
			return false;
		}
	}
#endif

#ifdef __COSTUME_EFFECT__
#ifdef __QUIVER__
	if (item->GetType() == ITEM_WEAPON && item->GetSubType() != WEAPON_ARROW && item->GetSubType() != WEAPON_QUIVER)
#else
	if (item->GetType() == ITEM_WEAPON && item->GetSubType() != WEAPON_ARROW)
#endif
	{
		LPITEM pkItem = GetWear(WEAR_COSTUME_EFFECT_WEAPON);
		if (pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You must unwear the weapon effect before."));
			return false;
		}
	}
	else if (item->GetType() == ITEM_ARMOR && item->GetSubType() == ARMOR_BODY)
	{
		LPITEM pkItem = GetWear(WEAR_COSTUME_EFFECT_BODY);
		if (pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You must unwear the armor effect before."));
			return false;
		}
	}
	else if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_EFFECT)
	{
		if (item->GetWearFlag() & WEARABLE_COSTUME_EFFECT_ARMOR)
		{
			LPITEM pkItem = GetWear(WEAR_BODY);
			if (!pkItem)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("To can wear a armor effect you must have a armor weared."));
				return false;
			}
		}
		else
		{
			LPITEM pkItem = GetWear(WEAR_WEAPON);
			if (!pkItem)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("To can wear a weapon effect you must have a weapon weared."));
				return false;
			}
		}
	}
#endif

	return true;
}

bool CHARACTER::CanUnequipNow(const LPITEM item, const TItemPos& srcCell, const TItemPos& destCell) /*const*/
{

	if (ITEM_BELT == item->GetType())
		VERIFY_MSG(CBeltInventoryHelper::IsExistItemInBeltInventory(this), "???? ?????????? ???????? ???????? ?????? ?? ????????.");

	// ?????? ?????? ?? ???? ??????
	if (IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		return false;

	// ?????? unequip?? ?????????? ???? ?? ?? ?????? ???? ?? ????
	{
		int pos = -1;

		if (item->IsDragonSoul())
			pos = GetEmptyDragonSoulInventory(item);
#ifdef __NEW_STORAGE__
		else if (item->IsStorageItem())
			pos = GetEmptyStorageInventory(item->GetStorageType() - UPGRADE_INVENTORY, item);
#endif
		else
			pos = GetEmptyInventory(item->GetSize());

		VERIFY_MSG( -1 == pos, "???????? ?? ?????? ????????." );
	}

#ifdef __COSTUME_WEAPON__
	if (item->GetType() == ITEM_WEAPON && item->GetSubType() != WEAPON_ARROW)
	{
		LPITEM pkItem = GetWear(WEAR_COSTUME_WEAPON);
		if (pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("For can do this unwear the costume weapon."));
			return false;
		}
	}
#endif

	return true;
}

