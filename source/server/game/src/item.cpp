#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "desc.h"
#include "sectree_manager.h"
#include "packet.h"
#include "protocol.h"
#include "log.h"
#include "skill.h"
#include "unique_item.h"
#include "marriage.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"
#include "affect.h"
#include "dragon_soul.h"
#include "buff_on_attributes.h"
#include "belt_inventory_helper.h"
#include "../../common/VnumHelper.h"
#include "../../common/defines.h"
#include "petsystem.h"
#include "mob_manager.h"

CItem::CItem(DWORD dwVnum)
	: m_dwVnum(dwVnum), m_bWindow(0), m_dwID(0), m_bEquipped(false), m_dwVID(0), m_wCell(0), m_dwCount(0),
#ifdef __CHANGELOOK__
	m_dwTransmutation(0), 
#endif
	m_lFlag(0), m_dwLastOwnerPID(0),
	m_bExchanging(false), m_pkDestroyEvent(NULL), m_pkExpireEvent(NULL), m_pkUniqueExpireEvent(NULL),
	m_pkTimerBasedOnWearExpireEvent(NULL), m_pkRealTimeExpireEvent(NULL),
   	m_pkAccessorySocketExpireEvent(NULL), m_pkOwnershipEvent(NULL), m_dwOwnershipPID(0), m_bSkipSave(false), m_isLocked(false),
	m_dwMaskVnum(0), m_dwSIGVnum (0)
#ifdef __SOULBIND__
	, m_lSealbind(0), m_pkUnSealTimerExpireEvent(NULL)
#endif
	, bIsBasic(false)
#ifdef __OFFLINE_PRIVATE_SHOP__
	, m_dwShopId(0)
#endif
{
	memset( &m_alSockets, 0, sizeof(m_alSockets) );
	memset( &m_aAttr, 0, sizeof(m_aAttr) );
#ifdef __OFFLINE_PRIVATE_SHOP__
	memset( &m_dwPrices, 0, sizeof(m_dwPrices) );
#endif
}

CItem::~CItem()
{
	Destroy();
}

void CItem::Initialize()
{
	CEntity::Initialize(ENTITY_ITEM);

	m_bWindow = RESERVED_WINDOW;
	m_pOwner = NULL;
	m_dwID = 0;
	m_bEquipped = false;
	m_dwVID = m_wCell = m_dwCount = m_lFlag = 0;
	m_pProto = NULL;
	m_bExchanging = false;
	memset(&m_alSockets, 0, sizeof(m_alSockets));
	memset(&m_aAttr, 0, sizeof(m_aAttr));
#ifdef __SOULBIND__
	m_lSealbind = 0;
	m_pkUnSealTimerExpireEvent = NULL;
#endif
#ifdef __OFFLINE_PRIVATE_SHOP__
	m_dwShopId = 0;
#endif
#ifdef __CHANGELOOK__
	m_dwTransmutation = 0;
#endif
	m_pkDestroyEvent = NULL;
	m_pkOwnershipEvent = NULL;
	m_dwOwnershipPID = 0;
	m_pkUniqueExpireEvent = NULL;
	m_pkTimerBasedOnWearExpireEvent = NULL;
	m_pkRealTimeExpireEvent = NULL;

	m_pkAccessorySocketExpireEvent = NULL;

	m_bSkipSave = false;
	m_dwLastOwnerPID = 0;

	bIsBasic = false;
}

void CItem::Destroy()
{
	event_cancel(&m_pkDestroyEvent);
	event_cancel(&m_pkOwnershipEvent);
	event_cancel(&m_pkUniqueExpireEvent);
	event_cancel(&m_pkTimerBasedOnWearExpireEvent);
	event_cancel(&m_pkRealTimeExpireEvent);
	event_cancel(&m_pkAccessorySocketExpireEvent);
#ifdef __SOULBIND__
	event_cancel(&m_pkUnSealTimerExpireEvent);
#endif
	CEntity::Destroy();

	if (GetSectree())
		GetSectree()->RemoveEntity(this);
}

EVENTFUNC(item_destroy_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>( event->info );

	if ( info == NULL )
	{
		sys_err( "item_destroy_event> <Factor> Null pointer" );
		return 0;
	}

	LPITEM pkItem = info->item;

	if (pkItem->GetOwner())
		sys_err("item_destroy_event: Owner exist. (item %s owner %s)", pkItem->GetName(), pkItem->GetOwner()->GetName());

	pkItem->SetDestroyEvent(NULL);
	M2_DESTROY_ITEM(pkItem);
	return 0;
}

void CItem::SetDestroyEvent(LPEVENT pkEvent)
{
	m_pkDestroyEvent = pkEvent;
}

void CItem::StartDestroyEvent(int iSec)
{
	if (m_pkDestroyEvent)
		return;

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetDestroyEvent(event_create(item_destroy_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::EncodeInsertPacket(LPENTITY ent)
{
	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	const PIXEL_POSITION & c_pos = GetXYZ();

	struct packet_item_ground_add pack;

	pack.bHeader	= HEADER_GC_ITEM_GROUND_ADD;
	pack.x		= c_pos.x;
	pack.y		= c_pos.y;
	pack.z		= c_pos.z;
	pack.dwVnum		= GetVnum();
	pack.dwVID		= m_dwVID;

	for (size_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		pack.alSockets[i] = GetSocket(i);

	memcpy(pack.aAttrs, GetAttributes(), sizeof(pack.aAttrs));

	//pack.count	= m_dwCount;

	d->Packet(&pack, sizeof(pack));

	if (m_pkOwnershipEvent != NULL)
	{
		item_event_info * info = dynamic_cast<item_event_info *>(m_pkOwnershipEvent->info);

		if ( info == NULL )
		{
			sys_err( "CItem::EncodeInsertPacket> <Factor> Null pointer" );
			return;
		}

		TPacketGCItemOwnership p;

		p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
		p.dwVID = m_dwVID;
		strlcpy(p.szName, info->szOwnerName, sizeof(p.szName));

		d->Packet(&p, sizeof(TPacketGCItemOwnership));
	}
}

void CItem::EncodeRemovePacket(LPENTITY ent)
{
	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	struct packet_item_ground_del pack;

	pack.bHeader	= HEADER_GC_ITEM_GROUND_DEL;
	pack.dwVID		= m_dwVID;

	d->Packet(&pack, sizeof(pack));
	sys_log(2, "Item::EncodeRemovePacket %s to %s", GetName(), ((LPCHARACTER) ent)->GetName());
}

const char * CItem::GetName()
{
	static char szItemName[128];
	memset(szItemName, 0, sizeof(szItemName));
	if (GetProto())
	{
		int len = 0;
		switch (GetType())
		{
			case ITEM_POLYMORPH:
			{
				const DWORD dwMobVnum = GetSocket(0);
				const CMob* pMob = CMobManager::instance().Get(dwMobVnum);
				if (pMob)
					len = snprintf(szItemName, sizeof(szItemName), "%s", pMob->m_table.szLocaleName);

				break;
			}
			case ITEM_SKILLBOOK:
			case ITEM_SKILLFORGET:
			{
				const DWORD dwSkillVnum = (GetVnum() == ITEM_SKILLBOOK_VNUM || GetVnum() == ITEM_SKILLFORGET_VNUM) ? GetSocket(0) : 0;
				const CSkillProto* pSkill = (dwSkillVnum != 0) ? CSkillManager::instance().Get(dwSkillVnum) : NULL;
				if (pSkill)
					len = snprintf(szItemName, sizeof(szItemName), "%s", pSkill->szName);

				break;
			}
		}
		len += snprintf(szItemName + len, sizeof(szItemName) - len, (len > 0) ? " %s" : "%s", GetProto()->szLocaleName);
	}

	return szItemName;
}

const char * CItem::GetMultiName()
{
	static char szItemName[128];
	memset(szItemName, 0, sizeof(szItemName));
	int len = snprintf(szItemName, sizeof(szItemName), "{{{");
	switch (GetType())
	{
		case ITEM_POLYMORPH:
			len += snprintf(szItemName + len, sizeof(szItemName) - len, "%ld, ", GetSocket(0));
			break;

		case ITEM_SKILLBOOK:
		case ITEM_SKILLFORGET:
			len += snprintf(szItemName + len, sizeof(szItemName) - len, "%ld, ", GetSocket(0) + 147000000);
			break;
	}
	len += snprintf(szItemName + len, sizeof(szItemName) - len, "%d}}}", GetVnum() + 147000000);

	return szItemName;
}

void CItem::SetProto(const TItemTable * table)
{
	assert(table != NULL);
	m_pProto = table;
	SetFlag(m_pProto->dwFlags);
}

void CItem::UsePacketEncode(LPCHARACTER ch, LPCHARACTER victim, struct packet_item_use *packet)
{
	if (!GetVnum())
		return;

	packet->header 	= HEADER_GC_ITEM_USE;
	packet->ch_vid 	= ch->GetVID();
	packet->victim_vid 	= victim->GetVID();
	packet->Cell = TItemPos(GetWindow(), m_wCell);
	packet->vnum	= GetVnum();
}

void CItem::RemoveFlag(long bit)
{
	REMOVE_BIT(m_lFlag, bit);
}

void CItem::AddFlag(long bit)
{
	SET_BIT(m_lFlag, bit);
}

void CItem::UpdatePacket()
{
	if (!m_pOwner || !m_pOwner->GetDesc())
		return;

	TPacketGCItemUpdate pack;

	pack.header = HEADER_GC_ITEM_UPDATE;
	pack.Cell = TItemPos(GetWindow(), m_wCell);
	pack.count	= m_dwCount;
	pack.bIsBasic = bIsBasic;
#ifdef __CHANGELOOK__
	pack.transmutation = m_dwTransmutation;
#endif

	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		pack.alSockets[i] = m_alSockets[i];

	memcpy(pack.aAttr, GetAttributes(), sizeof(pack.aAttr));
#ifdef __SOULBIND__
	pack.sealbind = m_lSealbind;
#endif

	sys_log(2, "UpdatePacket %s -> %s", GetName(), m_pOwner->GetName());
	m_pOwner->GetDesc()->Packet(&pack, sizeof(pack));
}

DWORD CItem::GetCount()
{
	if (GetType() == ITEM_ELK) return MIN(m_dwCount, INT_MAX);
	else
	{
		return MIN(m_dwCount, g_bItemCountLimit);
	}
}

bool CItem::SetCount(DWORD count)
{
	if (GetType() == ITEM_ELK)
	{
		m_dwCount = MIN(count, INT_MAX);
	}
	else
	{
		m_dwCount = MIN(count, g_bItemCountLimit);
	}

	if (count == 0 && m_pOwner)
	{
		if (GetSubType() == USE_ABILITY_UP || GetSubType() == USE_POTION || GetVnum() == 70020)
		{
			LPCHARACTER pOwner = GetOwner();
			WORD wCell = GetCell();

			RemoveFromCharacter();

			if (!IsDragonSoul())
			{
				LPITEM pItem = pOwner->FindSpecifyItem(GetVnum());

				if (NULL != pItem)
				{
					pOwner->ChainQuickslotItem(pItem, QUICKSLOT_TYPE_ITEM, wCell);
				}
				else
				{
					pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, wCell, 255);
				}
			}

			M2_DESTROY_ITEM(this);
		}
		else
		{
			if (!IsDragonSoul())
			{
				m_pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, m_wCell, 255);
			}
			M2_DESTROY_ITEM(RemoveFromCharacter());
		}

		return false;
	}

	UpdatePacket();

	Save();
	return true;
}

#ifdef __CHANGELOOK__
void CItem::SetTransmutation(DWORD dwVnum, bool bLog)
{
	m_dwTransmutation = dwVnum;
	UpdatePacket();
	Save();
}
#endif

LPITEM CItem::RemoveFromCharacter()
{
	if (!m_pOwner)
	{
		sys_err("Item::RemoveFromCharacter owner null");
		return (this);
	}

	LPCHARACTER pOwner = m_pOwner;

	if (m_bEquipped)	// 장착되었는가?
	{
		Unequip();
		//pOwner->UpdatePacket();

		SetWindow(RESERVED_WINDOW);
		Save();
		return (this);
	}
	else
	{
		if (GetWindow() != SAFEBOX && GetWindow() != MALL)
		{
			if (IsDragonSoul())
			{
				if (m_wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
					sys_err("CItem::RemoveFromCharacter: pos >= DRAGON_SOUL_INVENTORY_MAX_NUM");
				else
					pOwner->SetItem(TItemPos(m_bWindow, m_wCell), NULL);
			}
#ifdef __NEW_STORAGE__
			else if (IsStorageItem())
			{
				if (m_wCell >= STORAGE_INVENTORY_MAX_NUM)
					sys_err("CItem::RemoveFromCharacter: pos >= STORAGE_INVENTORY_MAX_NUM");
				else
					pOwner->SetItem(TItemPos(m_bWindow, m_wCell), NULL);
			}
#endif
			else
			{
				TItemPos cell(INVENTORY, m_wCell);

				if (false == cell.IsDefaultInventoryPosition() && false == cell.IsBeltInventoryPosition()) // 아니면 소지품에?
					sys_err("CItem::RemoveFromCharacter: Invalid Item Position");
				else
					pOwner->SetItem(cell, NULL);
			}
		}

		m_pOwner = NULL;
		m_wCell = 0;

		SetWindow(RESERVED_WINDOW);
		Save();
		return (this);
	}
}

#ifdef __OFFLINE_PRIVATE_SHOP__
void CItem::AddOfflinePrivateShop(int iPos, DWORD dwShopId)
{
	if (!m_pOwner)
	{
		sys_err("Item::AddOfflinePrivateShop owner null");
		return;
	}

	if (GetWindow() != SAFEBOX && GetWindow() != MALL)
	{
		if (IsDragonSoul())
		{
			if (m_wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
				sys_err("CItem::AddOfflinePrivateShop: pos >= DRAGON_SOUL_INVENTORY_MAX_NUM");
			else
				m_pOwner->SetItem(TItemPos(m_bWindow, m_wCell), NULL);
		}
#ifdef __NEW_STORAGE__
		else if (IsStorageItem())
		{
			if (m_wCell >= STORAGE_INVENTORY_MAX_NUM)
				sys_err("CItem::AddOfflinePrivateShop: pos >= STORAGE_INVENTORY_MAX_NUM");
			else
				m_pOwner->SetItem(TItemPos(m_bWindow, m_wCell), NULL);
		}
#endif
		else
		{
			TItemPos cell(INVENTORY, m_wCell);

			if (false == cell.IsDefaultInventoryPosition() && false == cell.IsBeltInventoryPosition()) // 아니면 소지품에?
				sys_err("CItem::AddOfflinePrivateShop: Invalid Item Position");
			else
				m_pOwner->SetItem(cell, NULL);
		}
	}

	m_dwShopId = dwShopId;
	m_wCell = iPos;
	SetWindow(RESERVED_WINDOW);
	Save();
}
#endif

bool CItem::AddToCharacter(LPCHARACTER ch, TItemPos Cell, bool isHighLight)
{
	assert(GetSectree() == NULL);
	assert(m_pOwner == NULL);
	WORD pos = Cell.cell;
	BYTE window_type = Cell.window_type;

	if (INVENTORY == window_type)
	{
		if (m_wCell >= ch->GetInventoryMax() && BELT_INVENTORY_SLOT_START > m_wCell)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
	else if (DRAGON_SOUL_INVENTORY == window_type)
	{
		if (m_wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
#ifdef __NEW_STORAGE__
	else if (UPGRADE_INVENTORY == window_type || BOOK_INVENTORY == window_type || STONE_INVENTORY == window_type || ATTR_INVENTORY == window_type || GIFTBOX_INVENTORY == window_type)
	{
		if (m_wCell >= STORAGE_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
#endif

	if (ch->GetDesc())
		m_dwLastOwnerPID = ch->GetPlayerID();

#ifdef __COSTUME_ACCE__
	if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE) && (GetSocket(ACCE_ABSORPTION_SOCKET) == 0))
	{
		long lVal = FindApplyValue(APPLY_ACCEDRAIN_RATE);
		
		SetSocket(ACCE_ABSORPTION_SOCKET, lVal);
	}
#endif

	event_cancel(&m_pkDestroyEvent);

	ch->SetItem(TItemPos(window_type, pos), this, isHighLight);
	m_pOwner = ch;

	Save();
	return true;
}

#ifdef __COSTUME_ACCE__
BYTE CItem::GetAcceValue()
{
	long lVal = FindApplyValue(APPLY_ACCEDRAIN_RATE);

	if (lVal == 1)
		return 1;
	else if (lVal == 5)
		return 2;
	else if (lVal == 10)
		return 3;
	else if (lVal == 20)
		return 4;

	return 1;
}
#endif

LPITEM CItem::RemoveFromGround()
{
	if (GetSectree())
	{
		SetOwnership(NULL);

		GetSectree()->RemoveEntity(this);

		ViewCleanup();

		Save();
	}

	return (this);
}

bool CItem::AddToGround(long lMapIndex, const PIXEL_POSITION & pos, bool skipOwnerCheck)
{
	if (0 == lMapIndex)
	{
		sys_err("wrong map index argument: %d", lMapIndex);
		return false;
	}

	if (GetSectree())
	{
		sys_err("sectree already assigned");
		return false;
	}

	if (!skipOwnerCheck && m_pOwner)
	{
		sys_err("owner pointer not null");
		return false;
	}

	LPSECTREE tree = SECTREE_MANAGER::instance().Get(lMapIndex, pos.x, pos.y);

	if (!tree)
	{
		sys_err("cannot find sectree by %dx%d", pos.x, pos.y);
		return false;
	}

	//tree->Touch();

	SetWindow(GROUND);
	SetXYZ(pos.x, pos.y, pos.z);
	tree->InsertEntity(this);
	UpdateSectree();
	Save();
	return true;
}

bool CItem::DistanceValid(LPCHARACTER ch)
{
	if (!GetSectree())
		return false;

	int iDist = DISTANCE_APPROX(GetX() - ch->GetX(), GetY() - ch->GetY());

	if (iDist > 300)
		return false;

	return true;
}

bool CItem::CanUsedBy(LPCHARACTER ch)
{
	// Anti flag check
	switch (ch->GetJob())
	{
		case JOB_WARRIOR:
			if (GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
				return false;
			break;

		case JOB_ASSASSIN:
			if (GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
				return false;
			break;

		case JOB_SHAMAN:
			if (GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
				return false;
			break;

		case JOB_SURA:
			if (GetAntiFlag() & ITEM_ANTIFLAG_SURA)
				return false;
			break;
#ifdef __WOLFMAN__
		case JOB_WOLFMAN:
			if (GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
				return false;
			break;
#endif
	}

	return true;
}

int CItem::FindEquipCell(LPCHARACTER ch, int iCandidateCell)
{
	if ((0 == GetWearFlag() || ITEM_TOTEM == GetType()) && ITEM_COSTUME != GetType() && ITEM_DS != GetType() && ITEM_SPECIAL_DS != GetType() && ITEM_RING != GetType() && ITEM_BELT != GetType()
#ifdef __COSTUME_PET__
&& (GetType() != ITEM_PET && GetSubType() != PET_PAY)
#endif
)
		return -1;

	if (GetType() == ITEM_DS || GetType() == ITEM_SPECIAL_DS)
	{
		if (iCandidateCell < 0)
			return WEAR_MAX_NUM + GetSubType();
		else
		{
			for (int i = 0; i < DRAGON_SOUL_DECK_MAX_NUM; i++)
				if (WEAR_MAX_NUM + i * DS_SLOT_MAX + GetSubType() == iCandidateCell)
					return iCandidateCell;

			return -1;
		}
	}
	else if (GetType() == ITEM_COSTUME)
	{
		if (GetSubType() == COSTUME_BODY)
			return WEAR_COSTUME_BODY;
		else if (GetSubType() == COSTUME_HAIR)
			return WEAR_COSTUME_HAIR;
#ifdef __COSTUME_MOUNT__
		else if (GetSubType() == COSTUME_MOUNT)
			return WEAR_COSTUME_MOUNT;
#endif
#ifdef __COSTUME_ACCE__
		else if (GetSubType() == COSTUME_ACCE)
			return WEAR_COSTUME_ACCE;
#endif
#ifdef __COSTUME_WEAPON__
		else if (GetSubType() == COSTUME_WEAPON)
			return WEAR_COSTUME_WEAPON;
#endif
#ifdef __COSTUME_EFFECT__
		else if (GetSubType() == COSTUME_EFFECT)
		{
			if (GetWearFlag() & WEARABLE_COSTUME_EFFECT_WEAPON)
				return WEAR_COSTUME_EFFECT_WEAPON;
			else
				return WEAR_COSTUME_EFFECT_BODY;
		}
#endif
	}

	else if (GetType() == ITEM_RING)
	{
		if (ch->GetWear(WEAR_RING1))
			return WEAR_RING2;
		else
			return WEAR_RING1;
	}
	else if (GetType() == ITEM_BELT)
		return WEAR_BELT;
	else if (GetWearFlag() & WEARABLE_BODY)
		return WEAR_BODY;
	else if (GetWearFlag() & WEARABLE_HEAD)
		return WEAR_HEAD;
	else if (GetWearFlag() & WEARABLE_FOOTS)
		return WEAR_FOOTS;
	else if (GetWearFlag() & WEARABLE_WRIST)
		return WEAR_WRIST;
	else if (GetWearFlag() & WEARABLE_WEAPON)
		return WEAR_WEAPON;
	else if (GetWearFlag() & WEARABLE_SHIELD)
		return WEAR_SHIELD;
	else if (GetWearFlag() & WEARABLE_NECK)
		return WEAR_NECK;
	else if (GetWearFlag() & WEARABLE_EAR)
		return WEAR_EAR;
	else if (GetWearFlag() & WEARABLE_ARROW)
		return WEAR_ARROW;
	else if (GetWearFlag() & WEARABLE_UNIQUE)
	{
		if (ch->GetWear(WEAR_UNIQUE1))
			return WEAR_UNIQUE2;
		else
			return WEAR_UNIQUE1;
	}
	else if (GetWearFlag() & WEARABLE_ABILITY)
	{
		if (!ch->GetWear(WEAR_ABILITY1))
		{
			return WEAR_ABILITY1;
		}
		else if (!ch->GetWear(WEAR_ABILITY2))
		{
			return WEAR_ABILITY2;
		}
		else if (!ch->GetWear(WEAR_ABILITY3))
		{
			return WEAR_ABILITY3;
		}
		else if (!ch->GetWear(WEAR_ABILITY4))
		{
			return WEAR_ABILITY4;
		}
		else if (!ch->GetWear(WEAR_ABILITY5))
		{
			return WEAR_ABILITY5;
		}
		else if (!ch->GetWear(WEAR_ABILITY6))
		{
			return WEAR_ABILITY6;
		}
		else if (!ch->GetWear(WEAR_ABILITY7))
		{
			return WEAR_ABILITY7;
		}
		else if (!ch->GetWear(WEAR_ABILITY8))
		{
			return WEAR_ABILITY8;
		}
		else
		{
			return -1;
		}
	}
#ifdef __PENDANT__
	else if (GetWearFlag() & WEARABLE_PENDANT)
		return WEAR_PENDANT;
#endif
#ifdef __COSTUME_PET__
	else if (GetType() == ITEM_PET && GetSubType() == PET_PAY)
		return WEAR_PET;
#endif
	return -1;
}

bool CItem::IsEdit()
{
    bool bRet = false;
    if (GetAttributeCount())
	{
        for (int i = 0; i < ITEM_ATTRIBUTE_NORM_NUM; ++i)
		{
            const TPlayerItemAttribute& ia = GetAttribute(i);
            if (ia.bType > 0 && ia.bType != APPLY_SKILL_DAMAGE_BONUS && ia.bType != APPLY_NORMAL_HIT_DAMAGE_BONUS)
			{
                const TItemAttrTable & t_ia_list = g_map_itemAttr[ia.bType];
                if (ia.sValue > t_ia_list.lValues[4])
                    bRet = true;
            }
        }
    }

    if (GetRareAttrCount())
	{
        for (int i = 0; i < ITEM_ATTRIBUTE_RARE_NUM; ++i)
		{
            const TPlayerItemAttribute& ia = GetAttribute(i);
            if (ia.bType > 0 && ia.bType != APPLY_SKILL_DAMAGE_BONUS && ia.bType != APPLY_NORMAL_HIT_DAMAGE_BONUS)
			{
                const TItemAttrTable & t_ia_list = g_map_itemRare[ia.bType];
                if (ia.sValue > t_ia_list.lValues[4])
                    bRet = true;
            }
        }
    }

    return bRet;
}

void CItem::ModifyPoints(bool bAdd)
{
	int accessoryGrade;

	if (false == IsAccessoryForSocket())
	{
		if (m_pProto->bType == ITEM_WEAPON || m_pProto->bType == ITEM_ARMOR)
		{
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
			{
				DWORD dwVnum;

				if ((dwVnum = GetSocket(i)) <= 2)
					continue;

				TItemTable * p = ITEM_MANAGER::instance().GetTable(dwVnum);

				if (!p)
				{
#ifdef __QUIVER__
					if (m_pProto->bSubType != WEAPON_QUIVER)
#endif
						sys_err("cannot find table by vnum %u", dwVnum);
					continue;
				}

				if (ITEM_METIN == p->bType)
				{
					for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
					{
						if (p->aApplies[i].bType == APPLY_NONE)
							continue;

						if (p->aApplies[i].bType == APPLY_SKILL)
							m_pOwner->ApplyPoint(p->aApplies[i].bType, bAdd ? p->aApplies[i].lValue : p->aApplies[i].lValue ^ 0x00800000);
						else
							m_pOwner->ApplyPoint(p->aApplies[i].bType, bAdd ? p->aApplies[i].lValue : -p->aApplies[i].lValue);
					}
				}
			}
		}

		accessoryGrade = 0;
	}
	else
	{
		accessoryGrade = MIN(GetAccessorySocketGrade(), ITEM_ACCESSORY_SOCKET_MAX_NUM);
	}

#ifdef __COSTUME_ACCE__
	if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE) && (GetSocket(ACCE_ABSORBED_SOCKET)))
	{
		TItemTable * pkItemAbsorbed = ITEM_MANAGER::instance().GetTable(GetSocket(ACCE_ABSORBED_SOCKET));
		if (pkItemAbsorbed)
		{
			if ((pkItemAbsorbed->bType == ITEM_ARMOR) && (pkItemAbsorbed->bSubType == ARMOR_BODY))
			{
				long lDefGrade = pkItemAbsorbed->alValues[1] + long(pkItemAbsorbed->alValues[5] * 2);
				double dValue = lDefGrade * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				lDefGrade = (long) dValue;
				if ((pkItemAbsorbed->alValues[1] > 0) && (lDefGrade <= 0) || (pkItemAbsorbed->alValues[5] > 0) && (lDefGrade < 1))
					lDefGrade += 1;
				else if ((pkItemAbsorbed->alValues[1] > 0) || (pkItemAbsorbed->alValues[5] > 0))
					lDefGrade += 1;
				
				m_pOwner->ApplyPoint(APPLY_DEF_GRADE_BONUS, bAdd ? lDefGrade : -lDefGrade);
				
				long lDefMagicBonus = pkItemAbsorbed->alValues[0];
				dValue = lDefMagicBonus * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				lDefMagicBonus = (long) dValue;
				if ((pkItemAbsorbed->alValues[0] > 0) && (lDefMagicBonus < 1))
					lDefMagicBonus += 1;
				else if (pkItemAbsorbed->alValues[0] > 0)
					lDefMagicBonus += 1;
				
				m_pOwner->ApplyPoint(APPLY_MAGIC_DEF_GRADE, bAdd ? lDefMagicBonus : -lDefMagicBonus);
			}
			else if (pkItemAbsorbed->bType == ITEM_WEAPON)
			{
				long lAttGrade = pkItemAbsorbed->alValues[4] + pkItemAbsorbed->alValues[5];
				if (pkItemAbsorbed->alValues[3] > pkItemAbsorbed->alValues[4])
					lAttGrade = pkItemAbsorbed->alValues[3] + pkItemAbsorbed->alValues[5];
				
				double dValue = lAttGrade * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				lAttGrade = (long) dValue;
				if (((pkItemAbsorbed->alValues[3] > 0) && (lAttGrade < 1)) || ((pkItemAbsorbed->alValues[4] > 0) && (lAttGrade < 1)))
					lAttGrade += 1;
				else if ((pkItemAbsorbed->alValues[3] > 0) || (pkItemAbsorbed->alValues[4] > 0))
					lAttGrade += 1;
				
				m_pOwner->ApplyPoint(APPLY_ATT_GRADE_BONUS, bAdd ? lAttGrade : -lAttGrade);
				
				long lAttMagicGrade = pkItemAbsorbed->alValues[2] + pkItemAbsorbed->alValues[5];
				if (pkItemAbsorbed->alValues[1] > pkItemAbsorbed->alValues[2])
					lAttMagicGrade = pkItemAbsorbed->alValues[1] + pkItemAbsorbed->alValues[5];
				
				dValue = lAttMagicGrade * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				lAttMagicGrade = (long) dValue;
				if (((pkItemAbsorbed->alValues[1] > 0) && (lAttMagicGrade < 1)) || ((pkItemAbsorbed->alValues[2] > 0) && (lAttMagicGrade < 1)))
					lAttMagicGrade += 1;
				else if ((pkItemAbsorbed->alValues[1] > 0) || (pkItemAbsorbed->alValues[2] > 0))
					lAttMagicGrade += 1;
				
				m_pOwner->ApplyPoint(APPLY_MAGIC_ATT_GRADE, bAdd ? lAttMagicGrade : -lAttMagicGrade);
			}
		}
	}
#endif

	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
#ifdef __COSTUME_ACCE__
		if ((m_pProto->aApplies[i].bType == APPLY_NONE) && (GetType() != ITEM_COSTUME) && (GetSubType() != COSTUME_ACCE) && !IsBasicItem())
#else
		if (m_pProto->aApplies[i].bType == APPLY_NONE && !IsBasicItem())
#endif
			continue;
		
		BYTE bType = m_pProto->aApplies[i].bType;
		long value = m_pProto->aApplies[i].lValue;
#ifdef __COSTUME_ACCE__
		if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE))
		{
			TItemTable * pkItemAbsorbed = ITEM_MANAGER::instance().GetTable(GetSocket(ACCE_ABSORBED_SOCKET));
			if (pkItemAbsorbed)
			{
				if (pkItemAbsorbed->aApplies[i].bType == APPLY_NONE)
					continue;
				
				bType = pkItemAbsorbed->aApplies[i].bType;
				value = pkItemAbsorbed->aApplies[i].lValue;
				if (value < 0)
					continue;
				
				double dValue = value * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				value = (long) dValue;
				if ((pkItemAbsorbed->aApplies[i].lValue > 0) && (value <= 0))
					value += 1;
			}
			else
				continue;
		}
#endif

		if (IsBasicItem())
		{
			switch(i)
			{
				case 0:
				{
					switch(GetVnum())
					{
						case 49000:
							bType = 1;
							value = 1000;
						break;

						case 49001:
						case 40129:
						case 40130:
						case 40134:
						case 40135:
						case 40107:
						case 40101:
							bType = 53;
							value = 75;
						break;

						case 53271:
						case 41682:
						case 41680:
							bType = 1;
							value = 1000;
						break;

						case 45356:
						case 45358:
							bType = 53;
							value = 50;
						break;
						default:
							break;
					}
				}
				break;

				case 1:
				{
					switch(GetVnum())
					{
						case 49000:
						case 49001:
							bType = 63;
							value = 15;
						break;

						case 71236:
							bType = 53;
							value = 75;
						break;

						case 53271:
						case 41682:
						case 41680:
						case 45356:
						case 45358:
						case 40129:
						case 40130:
						case 40134:
						case 40135:
						case 40107:
						case 40101:
							bType = 63;
							value = 10;
						break;
						default:
							break;
					}
				}
				break;

				case 2:
				{
					switch(GetVnum())
					{
						case 53271:
							bType = 15;
							value = 10;
						break;

						case 71236:
							bType = 63;
							value = 20;
						break;
						default:
							break;
					}
				}
				break;

				case 3:
				{
					switch(GetVnum())
					{
						case 71236:
							bType = 8;
							value = 20;
						break;
						default:
							break;
					}
				}
				break;

				default:
					break;
			}
		}

		if (bType != APPLY_SKILL)
		{
			if (accessoryGrade != 0)
				value += MAX(accessoryGrade, value * aiAccessorySocketEffectivePct[accessoryGrade] / 100);
			
			m_pOwner->ApplyPoint(bType, bAdd ? value : -value);
		}
		else
			m_pOwner->ApplyPoint(bType, bAdd ? value : value ^ 0x00800000);
	}

	if (true == CItemVnumHelper::IsRamadanMoonRing(GetVnum()) || true == CItemVnumHelper::IsHalloweenCandy(GetVnum())
		|| true == CItemVnumHelper::IsHappinessRing(GetVnum()) || true == CItemVnumHelper::IsLovePendant(GetVnum()))
	{
		// Do not anything.
	}
	else
	{
		for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		{
			if (GetAttributeType(i))
			{
				const TPlayerItemAttribute& ia = GetAttribute(i);
				int sValue = ia.sValue;
#ifdef __COSTUME_ACCE__
				if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE))
				{
					double dValue = sValue * GetSocket(ACCE_ABSORPTION_SOCKET);
					dValue = (double)dValue / 100;
					dValue = (double)dValue + .5;
					sValue = (long) dValue;
					if ((ia.sValue > 0) && (sValue <= 0))
						sValue += 1;
				}
#endif
				
				if (ia.bType == APPLY_SKILL)
					m_pOwner->ApplyPoint(ia.bType, bAdd ? sValue : sValue ^ 0x00800000);
				else
					m_pOwner->ApplyPoint(ia.bType, bAdd ? sValue : -sValue);
			}
		}
	}

	switch (m_pProto->bType)
	{
		case ITEM_PICK:
		case ITEM_ROD:
			{
				if (bAdd)
				{
					if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
						m_pOwner->SetPart(PART_WEAPON, GetVnum());
				}
				else
				{
					if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
#ifdef __COSTUME_WEAPON__
						m_pOwner->SetPart(PART_WEAPON, 0);
#else
						m_pOwner->SetPart(PART_WEAPON, m_pOwner->GetOriginalPart(PART_WEAPON));
#endif
				}
			}
			break;

		case ITEM_WEAPON:
			{
#ifdef __QUIVER__
				if (m_pProto->bSubType == WEAPON_ARROW || m_pProto->bSubType == WEAPON_QUIVER)
				{
					if (bAdd)
					{
						if (m_wCell == INVENTORY_MAX_NUM + WEAR_ARROW)
						{
							m_pOwner->SetPart(PART_ARROW_TYPE, m_pProto->bSubType);
							const CItem* pWeapon = m_pOwner->GetWear(WEAR_WEAPON);
							if (pWeapon != NULL && pWeapon->GetSubType() == WEAPON_BOW)
							{
#ifdef __COSTUME_WEAPON__
								const CItem* pCostumeWeapon = m_pOwner->GetWear(WEAR_COSTUME_WEAPON);
								if (pCostumeWeapon != NULL)
									m_pOwner->SetPart(PART_WEAPON, pCostumeWeapon->GetVnum());
								else
									m_pOwner->SetPart(PART_WEAPON, pWeapon->GetVnum());
#else
								m_pOwner->SetPart(PART_WEAPON, pWeapon->GetVnum());
#endif
							}
						}
					}
					else
					{
						if (m_wCell == INVENTORY_MAX_NUM + WEAR_ARROW)
							m_pOwner->SetPart(PART_ARROW_TYPE, m_pOwner->GetOriginalPart(PART_ARROW_TYPE));
					}
					
					break;
				}
#endif
#ifdef __QUIVER__
				const CItem* pArrow = m_pOwner->GetWear(WEAR_ARROW);
#endif

#ifdef __COSTUME_WEAPON__
				if (m_pOwner->GetWear(WEAR_COSTUME_WEAPON) != 0)
					break;
#endif

				if (bAdd)
				{
#ifdef __QUIVER__
					if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
						if (pArrow != NULL)
							m_pOwner->SetPart(PART_ARROW_TYPE, pArrow->GetSubType());
#endif
					if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
#ifdef __CHANGELOOK__
					{
						DWORD dwRes = GetTransmutation() != 0 ? GetTransmutation() : GetVnum();
						m_pOwner->SetPart(PART_WEAPON, dwRes);
					}
#else
						m_pOwner->SetPart(PART_WEAPON, GetVnum());
#endif
				}
				else
				{
					if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
#ifdef __COSTUME_WEAPON__
						m_pOwner->SetPart(PART_WEAPON, 0);
#else
						m_pOwner->SetPart(PART_WEAPON, m_pOwner->GetOriginalPart(PART_WEAPON));
#endif
				}
			}
			break;

		case ITEM_ARMOR:
			{
				// 코스츔 body를 입고있다면 armor는 벗던 입던 상관 없이 비주얼에 영향을 주면 안 됨.
				if (0 != m_pOwner->GetWear(WEAR_COSTUME_BODY))
					break;

				if (GetSubType() == ARMOR_BODY || GetSubType() == ARMOR_HEAD || GetSubType() == ARMOR_FOOTS || GetSubType() == ARMOR_SHIELD)
				{
					if (bAdd)
					{
						if (GetProto()->bSubType == ARMOR_BODY)
#ifdef __CHANGELOOK__
						{
							DWORD dwRes = GetTransmutation() != 0 ? GetTransmutation() : GetVnum();
							m_pOwner->SetPart(PART_MAIN, dwRes);
						}
#else
							m_pOwner->SetPart(PART_MAIN, GetVnum());
#endif
					}
					else
					{
						if (GetProto()->bSubType == ARMOR_BODY)
							m_pOwner->SetPart(PART_MAIN, m_pOwner->GetOriginalPart(PART_MAIN));
					}
				}
			}
			break;

		// 코스츔 아이템 입었을 때 캐릭터 parts 정보 세팅. 기존 스타일대로 추가함..
		case ITEM_COSTUME:
			{
				DWORD toSetValue = this->GetVnum();
				EParts toSetPart = PART_MAX_NUM;

				// 갑옷 코스츔
				if (GetSubType() == COSTUME_BODY)
				{
					toSetPart = PART_MAIN;

					if (false == bAdd)
					{
						// 코스츔 갑옷을 벗었을 때 원래 갑옷을 입고 있었다면 그 갑옷으로 look 세팅, 입지 않았다면 default look
						const CItem* pArmor = m_pOwner->GetWear(WEAR_BODY);
						toSetValue = (NULL != pArmor) ? pArmor->GetVnum() : m_pOwner->GetOriginalPart(PART_MAIN);
#ifdef __CHANGELOOK__
						if (pArmor)
							toSetValue = pArmor->GetTransmutation() != 0 ? pArmor->GetTransmutation() : pArmor->GetVnum();
#endif
					}
#ifdef __CHANGELOOK__
					else
						toSetValue = GetTransmutation() != 0 ? GetTransmutation() : GetVnum();
#endif

				}
				else if (GetSubType() == COSTUME_HAIR)
				{
					toSetPart = PART_HAIR;
					if (!bAdd)
						toSetValue = 0;
					else
					{
#ifdef __CHANGELOOK__
						DWORD dwTransmutation = GetTransmutation();
						if (dwTransmutation != 0)
						{
							TItemTable* pItemTable = ITEM_MANAGER::instance().GetTable(dwTransmutation);
							toSetValue = (pItemTable != NULL) ? pItemTable->alValues[3] : GetValue(3);
						}
						else
							toSetValue = GetValue(3);
#else
						toSetValue = GetValue(3);
#endif
					}
				}
#ifdef __COSTUME_ACCE__
				else if (GetSubType() == COSTUME_ACCE)
				{
					toSetValue -= 85000;
					if (GetSocket(ACCE_ABSORPTION_SOCKET) >= ACCE_EFFECT_FROM_ABS)
						toSetValue += 10000;
					
					toSetValue = bAdd ? toSetValue : 0;
					toSetPart = PART_ACCE;
				}
#endif
#ifdef __COSTUME_WEAPON__
				else if (GetSubType() == COSTUME_WEAPON)
				{
					toSetPart = PART_WEAPON;
					if (!bAdd)
					{
						const CItem* pWeapon = m_pOwner->GetWear(WEAR_WEAPON);
						toSetValue = (NULL != pWeapon) ? pWeapon->GetVnum() : m_pOwner->GetPart(PART_WEAPON);						
					}
				}
#endif
#ifdef __COSTUME_EFFECT__
				else if (GetSubType() == COSTUME_EFFECT)
				{
					if (GetWearFlag() & WEARABLE_COSTUME_EFFECT_ARMOR)
					{
						if (!bAdd)
							m_pOwner->SetPart((BYTE)PART_BODY_EFFECT, 0);
						else
							m_pOwner->SetPart((BYTE)PART_BODY_EFFECT, GetValue(0));
					}
					else
					{
						if (!bAdd)
						{
							m_pOwner->SetPart((BYTE)PART_WEAPON_RIGHT_EFFECT, 0);
							m_pOwner->SetPart((BYTE)PART_WEAPON_LEFT_EFFECT, 0);
						}
						else
						{
							const CItem* pWeaponEffectCheck = m_pOwner->GetWear(WEAR_WEAPON);
							if (pWeaponEffectCheck != NULL)
							{
								DWORD dwEffectRight = 0;
								DWORD dwEffectLeft = 0;
								if (pWeaponEffectCheck->GetSubType() == WEAPON_DAGGER)
								{
									dwEffectRight = GetValue(1);
									dwEffectLeft = GetValue(2);
								}
								else if (pWeaponEffectCheck->GetSubType() == WEAPON_BOW)
								{
									dwEffectRight = GetValue(3);
								}
								else if (pWeaponEffectCheck->GetSubType() == WEAPON_BELL)
								{
									dwEffectRight = GetValue(1);
								}
								else if (pWeaponEffectCheck->GetSubType() == WEAPON_FAN)
								{
									dwEffectRight = GetValue(4);
								}
#ifdef __WOLFMAN__
								else if (pWeaponEffectCheck->GetSubType() == WEAPON_CLAW)
								{
									dwEffectRight = GetValue(1);
									dwEffectLeft = GetValue(2);
								}
#endif
								else
								{
									dwEffectRight = GetValue(0);
								}
								
								m_pOwner->SetPart((BYTE)PART_WEAPON_RIGHT_EFFECT, dwEffectRight);
								m_pOwner->SetPart((BYTE)PART_WEAPON_LEFT_EFFECT, dwEffectLeft);
							}
						}
					}
					
					m_pOwner->UpdatePacket();
				}
#endif
				if (PART_MAX_NUM != toSetPart)
				{
					m_pOwner->SetPart((BYTE)toSetPart, toSetValue);
					m_pOwner->UpdatePacket();
				}
			}
			break;
		case ITEM_UNIQUE:
			{
				if (0 != GetSIGVnum())
				{
					const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(GetSIGVnum());
					if (NULL == pItemGroup)
						break;
					DWORD dwAttrVnum = pItemGroup->GetAttrVnum(GetVnum());
					const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::instance().GetSpecialAttrGroup(dwAttrVnum);
					if (NULL == pAttrGroup)
						break;
					for (itertype (pAttrGroup->m_vecAttrs) it = pAttrGroup->m_vecAttrs.begin(); it != pAttrGroup->m_vecAttrs.end(); it++)
					{
						m_pOwner->ApplyPoint(it->apply_type, bAdd ? it->apply_value : -it->apply_value);
					}
				}
			}
			break;
	}
}

bool CItem::IsEquipable() const
{
	switch (this->GetType())
	{
	case ITEM_COSTUME:
	case ITEM_ARMOR:
	case ITEM_WEAPON:
	case ITEM_ROD:
	case ITEM_PICK:
	case ITEM_UNIQUE:
	case ITEM_DS:
	case ITEM_SPECIAL_DS:
	case ITEM_RING:
	case ITEM_BELT:
#ifdef __COSTUME_PET__
	case ITEM_PET:
#endif
		return true;
	}

	return false;
}

// return false on error state
bool CItem::EquipTo(LPCHARACTER ch, BYTE bWearCell)
{
	if (!ch)
	{
		sys_err("EquipTo: nil character");
		return false;
	}

	// 용혼석 슬롯 index는 WEAR_MAX_NUM 보다 큼.
	if (IsDragonSoul())
	{
		if (bWearCell < WEAR_MAX_NUM || bWearCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
		{
			sys_err("EquipTo: invalid dragon soul cell (this: #%d %s wearflag: %d cell: %d)", GetOriginalVnum(), GetName(), GetSubType(), bWearCell - WEAR_MAX_NUM);
			return false;
		}
	}
	else
	{
		if (bWearCell >= WEAR_MAX_NUM)
		{
			sys_err("EquipTo: invalid wear cell (this: #%d %s wearflag: %d cell: %d)", GetOriginalVnum(), GetName(), GetWearFlag(), bWearCell);
			return false;
		}
	}

	if (ch->GetWear(bWearCell))
	{
		sys_err("EquipTo: item already exist (this: #%d %s cell: %d %s)", GetOriginalVnum(), GetName(), bWearCell, ch->GetWear(bWearCell)->GetName());
		return false;
	}

	if (GetOwner())
		RemoveFromCharacter();

	ch->SetWear(bWearCell, this);

	m_pOwner = ch;
	m_bEquipped = true;
	m_wCell	= INVENTORY_MAX_NUM + bWearCell;

#ifdef __COSTUME_PET__
	if (GetType() == ITEM_PET && GetSubType() == PET_PAY)
	{
		CPetSystem* petSystem = ch->GetPetSystem();
		if (petSystem)
			petSystem->Summon(GetValue(0), this, GetName(), false);
	}
#endif

#ifndef __IMMUNE_FIX__
	DWORD dwImmuneFlag = 0;

	for (int i = 0; i < WEAR_MAX_NUM; ++i)
		if (m_pOwner->GetWear(i))
			SET_BIT(dwImmuneFlag, m_pOwner->GetWear(i)->m_pProto->dwImmuneFlag);

	m_pOwner->SetImmuneFlag(dwImmuneFlag);
#endif

	if (IsDragonSoul())
		DSManager::instance().ActivateDragonSoul(this);
	else
	{
#ifdef __COSTUME_PET__
		if (!(GetType() == ITEM_PET && GetSubType() == PET_PAY)
#ifdef __COSTUME_MOUNT__
&& !(ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType())
#endif
)
			ModifyPoints(true);
#else
#ifdef __COSTUME_MOUNT__
		if (!(ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType()))
			ModifyPoints(true);
#else
		ModifyPoints(true);
#endif
#endif
		StartUniqueExpireEvent();
		if (-1 != GetProto()->cLimitTimerBasedOnWearIndex)
			StartTimerBasedOnWearExpireEvent();

		StartAccessorySocketExpireEvent();
	}

	ch->BuffOnAttr_AddBuffsFromItem(this);

	m_pOwner->ComputeBattlePoints();

	m_pOwner->UpdatePacket();

	Save();

	return (true);
}

bool CItem::Unequip()
{
	if (!m_pOwner || GetCell() < INVENTORY_MAX_NUM)
	{
		sys_err("%s %u m_pOwner %p, GetCell %d",
				GetName(), GetID(), get_pointer(m_pOwner), GetCell());
		return false;
	}

	if (this != m_pOwner->GetWear(GetCell() - INVENTORY_MAX_NUM))
	{
		sys_err("m_pOwner->GetWear() != this");
		return false;
	}

#ifdef __COSTUME_PET__
	if (GetType() == ITEM_PET && GetSubType() == PET_PAY)
	{
		CPetSystem* petSystem = m_pOwner->GetPetSystem();
		if (petSystem)
			petSystem->Unsummon(GetValue(0));
	}
#endif

	//신규 말 아이템 제거시 처리
	if (IsRideItem())
		ClearMountAttributeAndAffect();

	if (IsDragonSoul())
	{
		DSManager::instance().DeactivateDragonSoul(this);
	}
	else
	{
		ModifyPoints(false);
	}

	StopUniqueExpireEvent();

	if (-1 != GetProto()->cLimitTimerBasedOnWearIndex)
		StopTimerBasedOnWearExpireEvent();

	StopAccessorySocketExpireEvent();

	m_pOwner->BuffOnAttr_RemoveBuffsFromItem(this);

	m_pOwner->SetWear(GetCell() - INVENTORY_MAX_NUM, NULL);

#ifndef __IMMUNE_FIX__
	DWORD dwImmuneFlag = 0;

	for (int i = 0; i < WEAR_MAX_NUM; ++i)
		if (m_pOwner->GetWear(i))
			SET_BIT(dwImmuneFlag, m_pOwner->GetWear(i)->m_pProto->dwImmuneFlag);

	m_pOwner->SetImmuneFlag(dwImmuneFlag);
#endif

	m_pOwner->ComputeBattlePoints();

	m_pOwner->UpdatePacket();

	m_pOwner = NULL;
	m_wCell = 0;
	m_bEquipped	= false;

	return true;
}

long CItem::GetValue(DWORD idx)
{
	assert(idx < ITEM_VALUES_MAX_NUM);
	return GetProto()->alValues[idx];
}

void CItem::SetExchanging(bool bOn)
{
	m_bExchanging = bOn;
}

void CItem::Save()
{
	if (m_bSkipSave)
		return;

	ITEM_MANAGER::instance().DelayedSave(this);
}

bool CItem::CreateSocket(BYTE bSlot, BYTE bGold)
{
	assert(bSlot < ITEM_SOCKET_MAX_NUM);

	if (m_alSockets[bSlot] != 0)
	{
		sys_err("Item::CreateSocket : socket already exist %s %d", GetName(), bSlot);
		return false;
	}

	if (bGold)
		m_alSockets[bSlot] = 2;
	else
		m_alSockets[bSlot] = 1;

	UpdatePacket();

	Save();
	return true;
}

void CItem::SetSockets(const long * c_al)
{
	memcpy(m_alSockets, c_al, sizeof(m_alSockets));
	Save();
}

void CItem::SetSocket(int i, long v, bool bLog)
{
	assert(i < ITEM_SOCKET_MAX_NUM);
	m_alSockets[i] = v;
	UpdatePacket();
	Save();
	if (bLog)
	{
#ifdef __NEWSTUFF__
		if (g_iDbLogLevel >= LOG_LEVEL_MAX)
#endif
		LogManager::instance().ItemLog(i, v, 0, GetID(), "SET_SOCKET", "", "", GetOriginalVnum());
	}
}

int CItem::GetGold()
{
	if (IS_SET(GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
		if (GetProto()->dwGold == 0)
			return GetCount();
		else
			return GetCount() / GetProto()->dwGold;
	}
	else
		return GetProto()->dwGold;
}

int CItem::GetShopBuyPrice()
{
	return GetProto()->dwShopBuyPrice;
}

bool CItem::IsOwnership(LPCHARACTER ch)
{
	if (!m_pkOwnershipEvent)
		return true;

	return m_dwOwnershipPID == ch->GetPlayerID() ? true : false;
}

EVENTFUNC(ownership_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>( event->info );

	if ( info == NULL )
	{
		sys_err( "ownership_event> <Factor> Null pointer" );
		return 0;
	}

	LPITEM pkItem = info->item;

	pkItem->SetOwnershipEvent(NULL);

	TPacketGCItemOwnership p;

	p.bHeader	= HEADER_GC_ITEM_OWNERSHIP;
	p.dwVID	= pkItem->GetVID();
	p.szName[0]	= '\0';

	pkItem->PacketAround(&p, sizeof(p));
	return 0;
}

void CItem::SetOwnershipEvent(LPEVENT pkEvent)
{
	m_pkOwnershipEvent = pkEvent;
}

void CItem::SetOwnership(LPCHARACTER ch, int iSec)
{
	if (!ch)
	{
		if (m_pkOwnershipEvent)
		{
			event_cancel(&m_pkOwnershipEvent);
			m_dwOwnershipPID = 0;

			TPacketGCItemOwnership p;

			p.bHeader	= HEADER_GC_ITEM_OWNERSHIP;
			p.dwVID	= m_dwVID;
			p.szName[0]	= '\0';

			PacketAround(&p, sizeof(p));
		}
		return;
	}

	if (m_pkOwnershipEvent)
		return;

	if (iSec <= 10)
		iSec = 30;

	m_dwOwnershipPID = ch->GetPlayerID();

	item_event_info* info = AllocEventInfo<item_event_info>();
	strlcpy(info->szOwnerName, ch->GetName(), sizeof(info->szOwnerName));
	info->item = this;

	SetOwnershipEvent(event_create(ownership_event, info, PASSES_PER_SEC(iSec)));

	TPacketGCItemOwnership p;

	p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
	p.dwVID = m_dwVID;
	strlcpy(p.szName, ch->GetName(), sizeof(p.szName));

	PacketAround(&p, sizeof(p));
}

int CItem::GetSocketCount()
{
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
	{
		if (GetSocket(i) == 0)
			return i;
	}
	return ITEM_SOCKET_MAX_NUM;
}

bool CItem::AddSocket()
{
	int count = GetSocketCount();
	if (count == ITEM_SOCKET_MAX_NUM)
		return false;
	m_alSockets[count] = 1;
	return true;
}

void CItem::AlterToSocketItem(int iSocketCount)
{
	if (iSocketCount >= ITEM_SOCKET_MAX_NUM)
	{
		sys_log(0, "Invalid Socket Count %d, set to maximum", ITEM_SOCKET_MAX_NUM);
		iSocketCount = ITEM_SOCKET_MAX_NUM;
	}

	for (int i = 0; i < iSocketCount; ++i)
		SetSocket(i, 1);
}

void CItem::AlterToMagicItem()
{
	int idx = GetAttributeSetIndex();

	if (idx < 0)
		return;

	//      Appeariance Second Third
	// Weapon 50        20     5
	// Armor  30        10     2
	// Acc    20        10     1

	int iSecondPct;
	int iThirdPct;

	switch (GetType())
	{
		case ITEM_WEAPON:
			iSecondPct = 20;
			iThirdPct = 5;
			break;

		case ITEM_ARMOR:
		case ITEM_COSTUME:
			if (GetSubType() == ARMOR_BODY)
			{
				iSecondPct = 10;
				iThirdPct = 2;
			}
			else
			{
				iSecondPct = 10;
				iThirdPct = 1;
			}
			break;

		default:
			return;
	}

	// 100% 확률로 좋은 속성 하나
	PutAttribute(aiItemMagicAttributePercentHigh);

	if (number(1, 100) <= iSecondPct)
		PutAttribute(aiItemMagicAttributePercentLow);

	if (number(1, 100) <= iThirdPct)
		PutAttribute(aiItemMagicAttributePercentLow);
}

DWORD CItem::GetRefineFromVnum()
{
	return ITEM_MANAGER::instance().GetRefineFromVnum(GetVnum());
}

int CItem::GetRefineLevel()
{
	const char* name = GetBaseName();
	char* p = const_cast<char*>(strrchr(name, '+'));

	if (!p)
		return 0;

	int	rtn = 0;
	str_to_number(rtn, p+1);

	const char* locale_name = GetName();
	p = const_cast<char*>(strrchr(locale_name, '+'));

	if (p)
	{
		int	locale_rtn = 0;
		str_to_number(locale_rtn, p+1);
		if (locale_rtn != rtn)
		{
			sys_err("refine_level_based_on_NAME(%d) is not equal to refine_level_based_on_LOCALE_NAME(%d).", rtn, locale_rtn);
		}
	}

	return rtn;
}

bool CItem::IsPolymorphItem()
{
	return GetType() == ITEM_POLYMORPH;
}

EVENTFUNC(unique_expire_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>( event->info );

	if ( info == NULL )
	{
		sys_err( "unique_expire_event> <Factor> Null pointer" );
		return 0;
	}

	LPITEM pkItem = info->item;

	if (pkItem->GetValue(2) == 0)
	{
		if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) <= 1)
		{
			sys_log(0, "UNIQUE_ITEM: expire %s %u", pkItem->GetName(), pkItem->GetID());
			pkItem->SetUniqueExpireEvent(NULL);
			ITEM_MANAGER::instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
			return 0;
		}
		else
		{
			pkItem->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - 1);
			return PASSES_PER_SEC(60);
		}
	}
	else
	{
		time_t cur = get_global_time();

		if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) <= cur)
		{
			pkItem->SetUniqueExpireEvent(NULL);
			ITEM_MANAGER::instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
			return 0;
		}
		else
		{
			// 게임 내에 시간제 아이템들이 빠릿빠릿하게 사라지지 않는 버그가 있어
			// 수정
			// by rtsummit
			if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - cur < 600)
				return PASSES_PER_SEC(pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - cur);
			else
				return PASSES_PER_SEC(600);
		}
	}
}

// 시간 후불제
// timer를 시작할 때에 시간 차감하는 것이 아니라,
// timer가 발화할 때에 timer가 동작한 시간 만큼 시간 차감을 한다.
EVENTFUNC(timer_based_on_wear_expire_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>( event->info );

	if ( info == NULL )
	{
		sys_err( "expire_event <Factor> Null pointer" );
		return 0;
	}

	LPITEM pkItem = info->item;
	int remain_time = pkItem->GetSocket(ITEM_SOCKET_REMAIN_SEC) - processing_time/passes_per_sec;
	if (remain_time <= 0)
	{
		sys_log(0, "ITEM EXPIRED : expired %s %u", pkItem->GetName(), pkItem->GetID());
		pkItem->SetTimerBasedOnWearExpireEvent(NULL);
		pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, 0);

		// 일단 timer based on wear 용혼석은 시간 다 되었다고 없애지 않는다.
		if (pkItem->IsDragonSoul())
		{
			DSManager::instance().DeactivateDragonSoul(pkItem);
		}
		else
		{
			ITEM_MANAGER::instance().RemoveItem(pkItem, "TIMER_BASED_ON_WEAR_EXPIRE");
		}
		return 0;
	}
	pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, remain_time);
	return PASSES_PER_SEC (MIN (60, remain_time));
}

void CItem::SetUniqueExpireEvent(LPEVENT pkEvent)
{
	m_pkUniqueExpireEvent = pkEvent;
}

void CItem::SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent)
{
	m_pkTimerBasedOnWearExpireEvent = pkEvent;
}

EVENTFUNC(real_time_expire_event)
{
	const item_vid_event_info* info = reinterpret_cast<const item_vid_event_info*>(event->info);

	if (NULL == info)
		return 0;

	const LPITEM item = ITEM_MANAGER::instance().FindByVID( info->item_vid );

	if (NULL == item)
		return 0;

	const time_t current = get_global_time();

	if (current >= item->GetSocket(0))
	{
#ifdef __GROWTH_PET__
		if (!(item->GetType() == ITEM_PET && item->GetSubType() == PET_UPBRINGING))
		{
			switch (item->GetVnum())
			{
				if (item->IsNewMountItem())
				{
					if (item->GetSocket(2) != 0)
						item->ClearMountAttributeAndAffect();
				}
				break;
			}

			ITEM_MANAGER::instance().RemoveItem(item, "REAL_TIME_EXPIRE");
		}
#else
		switch (item->GetVnum())
		{
			if (item->IsNewMountItem())
			{
				if (item->GetSocket(2) != 0)
					item->ClearMountAttributeAndAffect();
			}
			break;
		}

		ITEM_MANAGER::instance().RemoveItem(item, "REAL_TIME_EXPIRE");
#endif

		return 0;
	}

	return PASSES_PER_SEC(1);
}

void CItem::StartRealTimeExpireEvent()
{
	if (m_pkRealTimeExpireEvent)
		return;

	for (int i = 0 ; i < ITEM_LIMIT_MAX_NUM ; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType || LIMIT_REAL_TIME_START_FIRST_USE == GetProto()->aLimits[i].bType)
		{
			item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
			info->item_vid = GetVID();

			m_pkRealTimeExpireEvent = event_create( real_time_expire_event, info, PASSES_PER_SEC(1));

			sys_log(0, "REAL_TIME_EXPIRE: StartRealTimeExpireEvent %d", GetVnum());

			return;
		}
	}
}

bool CItem::IsRealTimeItem()
{
	if(!GetProto())
		return false;
	for (int i=0 ; i < ITEM_LIMIT_MAX_NUM ; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType)
			return true;
	}
	return false;
}

void CItem::StartUniqueExpireEvent()
{
	if (GetType() != ITEM_UNIQUE)
		return;

	if (m_pkUniqueExpireEvent)
		return;

	//기간제 아이템일 경우 시간제 아이템은 동작하지 않는다
	if (IsRealTimeItem())
		return;

	// HARD CODING
	if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		m_pOwner->ShowAlignment(false);

	int iSec = GetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME);

	if (iSec == 0)
		iSec = 60;
	else
		iSec = MIN(iSec, 60);

	SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME, 0);

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetUniqueExpireEvent(event_create(unique_expire_event, info, PASSES_PER_SEC(iSec)));
}

// 시간 후불제
// timer_based_on_wear_expire_event 설명 참조
void CItem::StartTimerBasedOnWearExpireEvent()
{
	if (m_pkTimerBasedOnWearExpireEvent)
		return;

	//기간제 아이템일 경우 시간제 아이템은 동작하지 않는다
	if (IsRealTimeItem())
		return;

	if (-1 == GetProto()->cLimitTimerBasedOnWearIndex)
		return;

	int iSec = GetSocket(0);

	// 남은 시간을 분단위로 끊기 위해...
	if (0 != iSec)
	{
		iSec %= 60;
		if (0 == iSec)
			iSec = 60;
	}

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetTimerBasedOnWearExpireEvent(event_create(timer_based_on_wear_expire_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::StopUniqueExpireEvent()
{
	if (!m_pkUniqueExpireEvent)
		return;

	if (GetValue(2) != 0) // 게임시간제 이외의 아이템은 UniqueExpireEvent를 중단할 수 없다.
		return;

	// HARD CODING
	if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		m_pOwner->ShowAlignment(true);

	SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME, event_time(m_pkUniqueExpireEvent) / passes_per_sec);
	event_cancel(&m_pkUniqueExpireEvent);

	ITEM_MANAGER::instance().SaveSingleItem(this);
}

void CItem::StopTimerBasedOnWearExpireEvent()
{
	if (!m_pkTimerBasedOnWearExpireEvent)
		return;

	int remain_time = GetSocket(ITEM_SOCKET_REMAIN_SEC) - event_processing_time(m_pkTimerBasedOnWearExpireEvent) / passes_per_sec;

	SetSocket(ITEM_SOCKET_REMAIN_SEC, remain_time);
	event_cancel(&m_pkTimerBasedOnWearExpireEvent);

	ITEM_MANAGER::instance().SaveSingleItem(this);
}

void CItem::ApplyAddon(int iAddonType)
{
	// Re-apply damage/hit bonuses
	int iSkillBonus = MINMAX(-30, (int)(gauss_random(0, 5) + 0.5), 30);
	int iNormalHitBonus = 0;
	if (abs(iSkillBonus) <= 20)
		iNormalHitBonus = -2 * iSkillBonus + abs(number(-8, 8) + number(-8, 8)) + number(1, 4);
	else
		iNormalHitBonus = -2 * iSkillBonus + number(1, 5);

	RemoveAttributeType(APPLY_SKILL_DAMAGE_BONUS);
	RemoveAttributeType(APPLY_NORMAL_HIT_DAMAGE_BONUS);
	AddAttribute(APPLY_NORMAL_HIT_DAMAGE_BONUS, iNormalHitBonus);
	AddAttribute(APPLY_SKILL_DAMAGE_BONUS, iSkillBonus);
}

int CItem::GetSpecialGroup() const
{
	return ITEM_MANAGER::instance().GetSpecialGroupFromItem(GetVnum());
}

//
// 악세서리 소켓 처리.
//
bool CItem::IsAccessoryForSocket()
{
	return (m_pProto->bType == ITEM_ARMOR && (m_pProto->bSubType == ARMOR_WRIST || m_pProto->bSubType == ARMOR_NECK || m_pProto->bSubType == ARMOR_EAR)) ||
		(m_pProto->bType == ITEM_BELT);				// 2013년 2월 새로 추가된 '벨트' 아이템의 경우 기획팀에서 악세서리 소켓 시스템을 그대로 이용하자고 함.
}

void CItem::SetAccessorySocketGrade(int iGrade)
{
	SetSocket(0, MINMAX(0, iGrade, GetAccessorySocketMaxGrade()));

	int iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];

	SetAccessorySocketDownGradeTime(iDownTime);
}

void CItem::SetAccessorySocketMaxGrade(int iMaxGrade)
{
	SetSocket(1, MINMAX(0, iMaxGrade, ITEM_ACCESSORY_SOCKET_MAX_NUM));
}

void CItem::SetAccessorySocketDownGradeTime(DWORD time)
{
	SetSocket(2, time);
}

EVENTFUNC(accessory_socket_expire_event)
{
	item_vid_event_info* info = dynamic_cast<item_vid_event_info*>( event->info );

	if ( info == NULL )
	{
		sys_err( "accessory_socket_expire_event> <Factor> Null pointer" );
		return 0;
	}

	LPITEM item = ITEM_MANAGER::instance().FindByVID(info->item_vid);

	if (item->GetAccessorySocketDownGradeTime() <= 1)
	{
degrade:
		item->SetAccessorySocketExpireEvent(NULL);
		item->AccessorySocketDegrade();
		return 0;
	}
	else
	{
		int iTime = item->GetAccessorySocketDownGradeTime() - 60;

		if (iTime <= 1)
			goto degrade;

		item->SetAccessorySocketDownGradeTime(iTime);

		if (iTime > 60)
			return PASSES_PER_SEC(60);
		else
			return PASSES_PER_SEC(iTime);
	}
}

void CItem::StartAccessorySocketExpireEvent()
{
	if (!IsAccessoryForSocket())
		return;

	if (m_pkAccessorySocketExpireEvent)
		return;

	if (GetAccessorySocketMaxGrade() == 0)
		return;

	if (GetAccessorySocketGrade() == 0)
		return;

	int iSec = GetAccessorySocketDownGradeTime();
	SetAccessorySocketExpireEvent(NULL);

	if (iSec <= 1)
		iSec = 5;
	else
		iSec = MIN(iSec, 60);

	item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
	info->item_vid = GetVID();

	SetAccessorySocketExpireEvent(event_create(accessory_socket_expire_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::StopAccessorySocketExpireEvent()
{
	if (!m_pkAccessorySocketExpireEvent)
		return;

	if (!IsAccessoryForSocket())
		return;

	int new_time = GetAccessorySocketDownGradeTime() - (60 - event_time(m_pkAccessorySocketExpireEvent) / passes_per_sec);

	event_cancel(&m_pkAccessorySocketExpireEvent);

	if (new_time <= 1)
	{
		AccessorySocketDegrade();
	}
	else
	{
		SetAccessorySocketDownGradeTime(new_time);
	}
}

bool CItem::IsRideItem()
{
	if (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_RIDE == GetSubType())
		return true;
	if (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_MOUNT_RIDE == GetSubType())
		return true;
#ifdef __COSTUME_MOUNT__
	if (ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType())
		return true;
#endif
	return false;
}

bool CItem::IsRamadanRing()
{
	if (GetVnum() == UNIQUE_ITEM_RAMADAN_RING)
		return true;
	return false;
}

void CItem::ClearMountAttributeAndAffect()
{
	LPCHARACTER ch = GetOwner();

	ch->RemoveAffect(AFFECT_MOUNT);
	ch->RemoveAffect(AFFECT_MOUNT_BONUS);

	ch->MountVnum(0);

	ch->PointChange(POINT_ST, 0);
	ch->PointChange(POINT_DX, 0);
	ch->PointChange(POINT_HT, 0);
	ch->PointChange(POINT_IQ, 0);
}

// fixme
// 이거 지금은 안쓴데... 근데 혹시나 싶어서 남겨둠.
// by rtsummit
bool CItem::IsNewMountItem()
{
	switch(GetVnum())
	{
		case 76000: case 76001: case 76002: case 76003:
		case 76004: case 76005: case 76006: case 76007:
		case 76008: case 76009: case 76010: case 76011:
		case 76012: case 76013: case 76014:
			return true;
	}
	return false;
}

void CItem::SetAccessorySocketExpireEvent(LPEVENT pkEvent)
{
	m_pkAccessorySocketExpireEvent = pkEvent;
}

void CItem::AccessorySocketDegrade()
{
	if (GetAccessorySocketGrade() > 0)
	{
		LPCHARACTER ch = GetOwner();

		if (ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s에 박혀있던 보석이 사라집니다."), GetName());
		}

		ModifyPoints(false);
		SetAccessorySocketGrade(GetAccessorySocketGrade()-1);
		ModifyPoints(true);

		int iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];

		SetAccessorySocketDownGradeTime(iDownTime);

		if (iDownTime)
			StartAccessorySocketExpireEvent();
	}
}

// ring에 item을 박을 수 있는지 여부를 체크해서 리턴
static const bool CanPutIntoRing(LPITEM ring, LPITEM item)
{
	//const DWORD vnum = item->GetVnum();
	return false;
}

bool CItem::CanPutInto(LPITEM item)
{
	if (item->GetType() == ITEM_BELT)
		return this->GetSubType() == USE_PUT_INTO_BELT_SOCKET;

	else if(item->GetType() == ITEM_RING)
		return CanPutIntoRing(item, this);

	else if (item->GetType() != ITEM_ARMOR)
		return false;

	DWORD vnum = item->GetVnum();

	struct JewelAccessoryInfo
	{
		DWORD jewel;
		DWORD wrist;
		DWORD neck;
		DWORD ear;
	};
	const static JewelAccessoryInfo infos[] = {
		{ 50634, 14420, 16220, 17220 },
		{ 50635, 14500, 16500, 17500 },
		{ 50636, 14520, 16520, 17520 },
		{ 50637, 14540, 16540, 17540 },
		{ 50638, 14560, 16560, 17560 },
		{ 50639, 14570, 16570, 17570 },
	};

	DWORD item_type = (item->GetVnum() / 10) * 10;
	for (size_t i = 0; i < sizeof(infos) / sizeof(infos[0]); i++)
	{
		const JewelAccessoryInfo& info = infos[i];
		switch(item->GetSubType())
		{
		case ARMOR_WRIST:
			if (info.wrist == item_type)
			{
				if (info.jewel == GetVnum())
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			break;
		case ARMOR_NECK:
			if (info.neck == item_type)
			{
				if (info.jewel == GetVnum())
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			break;
		case ARMOR_EAR:
			if (info.ear == item_type)
			{
				if (info.jewel == GetVnum())
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			break;
		}
	}
	if (item->GetSubType() == ARMOR_WRIST)
		vnum -= 14000;
	else if (item->GetSubType() == ARMOR_NECK)
		vnum -= 16000;
	else if (item->GetSubType() == ARMOR_EAR)
		vnum -= 17000;
	else
		return false;

	DWORD type = vnum / 20;

	if (type < 0 || type > 11)
	{
		type = (vnum - 170) / 20;

		if (50623 + type != GetVnum())
			return false;
		else
			return true;
	}
	else if (item->GetVnum() >= 16210 && item->GetVnum() <= 16219)
	{
		if (50625 != GetVnum())
			return false;
		else
			return true;
	}
	else if (item->GetVnum() >= 16230 && item->GetVnum() <= 16239)
	{
		if (50626 != GetVnum())
			return false;
		else
			return true;
	}

	return 50623 + type == GetVnum();
}

bool CItem::CheckItemUseLevel(int nLevel)
{
	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		if (this->m_pProto->aLimits[i].bType == LIMIT_LEVEL)
		{
			if (this->m_pProto->aLimits[i].lValue > nLevel) return false;
			else return true;
		}
	}
	return true;
}

long CItem::FindApplyValue(BYTE bApplyType)
{
	if (m_pProto == NULL)
		return 0;

	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		if (m_pProto->aApplies[i].bType == bApplyType)
			return m_pProto->aApplies[i].lValue;
	}

	return 0;
}

void CItem::CopySocketTo(LPITEM pItem)
{
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		pItem->m_alSockets[i] = m_alSockets[i];
	}
}

int CItem::GetAccessorySocketGrade()
{
   	return MINMAX(0, GetSocket(0), GetAccessorySocketMaxGrade());
}

int CItem::GetAccessorySocketMaxGrade()
{
   	return MINMAX(0, GetSocket(1), ITEM_ACCESSORY_SOCKET_MAX_NUM);
}

int CItem::GetAccessorySocketDownGradeTime()
{
	return MINMAX(0, GetSocket(2), aiAccessorySocketDegradeTime[GetAccessorySocketGrade()]);
}

void CItem::AttrLog()
{
	const char * pszIP = NULL;

	if (GetOwner() && GetOwner()->GetDesc())
		pszIP = GetOwner()->GetDesc()->GetHostName();

	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		if (m_alSockets[i])
		{
#ifdef __NEWSTUFF__
			if (g_iDbLogLevel>=LOG_LEVEL_MAX)
#endif
			LogManager::instance().ItemLog(i, m_alSockets[i], 0, GetID(), "INFO_SOCKET", "", pszIP ? pszIP : "", GetOriginalVnum());
		}
	}

	for (int i = 0; i<ITEM_ATTRIBUTE_MAX_NUM; ++i)
	{
		int	type	= m_aAttr[i].bType;
		int value	= m_aAttr[i].sValue;

		if (type)
		{
#ifdef __NEWSTUFF__
			if (g_iDbLogLevel>=LOG_LEVEL_MAX)
#endif
			LogManager::instance().ItemLog(i, type, value, GetID(), "INFO_ATTR", "", pszIP ? pszIP : "", GetOriginalVnum());
		}
	}
}

int CItem::GetLevelLimit()
{
	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		if (this->m_pProto->aLimits[i].bType == LIMIT_LEVEL)
		{
			return this->m_pProto->aLimits[i].lValue;
		}
	}
	return 0;
}

bool CItem::OnAfterCreatedItem()
{
	// 아이템을 한 번이라도 사용했다면, 그 이후엔 사용 중이지 않아도 시간이 차감되는 방식
	if (-1 != this->GetProto()->cLimitRealTimeFirstUseIndex)
	{
		// Socket1에 아이템의 사용 횟수가 기록되어 있으니, 한 번이라도 사용한 아이템은 타이머를 시작한다.
		if (0 != GetSocket(1))
		{
			StartRealTimeExpireEvent();
		}
	}

#ifdef __SOULBIND__
	if (GetSealBindTime() > 0)
		StartUnSealBindTimerExpireEvent();
#endif

	return true;
}

bool CItem::IsDragonSoul()
{
	return GetType() == ITEM_DS;
}

#ifdef __NEW_STORAGE__
bool CItem::IsUpgradeItem()
{
	return (GetType() == ITEM_MATERIAL && GetSubType() == MATERIAL_LEATHER);
}
bool CItem::IsBook()
{
	return GetType() == ITEM_SKILLBOOK;
}
bool CItem::IsStone()
{
	return (GetType() == ITEM_METIN && GetSubType() == METIN_NORMAL);
}
bool CItem::IsAttr()
{
	return (ITEM_MEDIUM == GetType() || (GetType() == ITEM_USE && (GetSubType() == USE_ADD_ATTRIBUTE || GetSubType() == USE_ADD_ATTRIBUTE2 || GetSubType() == USE_CHANGE_ATTRIBUTE || GetSubType() == USE_CHANGE_ATTRIBUTE2 || GetSubType() == USE_RESET_COSTUME_ATTR || GetSubType() == USE_ADD_COSTUME_ATTRIBUTE || GetSubType() == USE_CHANGE_ATTRIBUTE_PLUS)));
}
bool CItem::IsGiftBox()
{
	return GetType() == ITEM_GIFTBOX || GetType() == ITEM_GACHA;
}

bool CItem::IsStorageItem()
{
	return IsUpgradeItem() || IsBook() || IsStone() || IsAttr() || IsGiftBox();
}

BYTE CItem::GetStorageType()
{
	if (IsUpgradeItem())
		return UPGRADE_INVENTORY;
	else if (IsBook())
		return BOOK_INVENTORY;
	else if (IsStone())
		return STONE_INVENTORY;
	else if (IsAttr())
		return ATTR_INVENTORY;
	else if (IsGiftBox())
		return GIFTBOX_INVENTORY;

	return UPGRADE_INVENTORY;
}
#endif

int CItem::GiveMoreTime_Per(float fPercent)
{
	if (IsDragonSoul())
	{
		DWORD duration = DSManager::instance().GetDuration(this);
		DWORD remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);
		DWORD given_time = fPercent * duration / 100u;
		if (remain_sec == duration)
			return false;
		if ((given_time + remain_sec) >= duration)
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
			return duration - remain_sec;
		}
		else
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, given_time + remain_sec);
			return given_time;
		}
	}
	// 우선 용혼석에 관해서만 하도록 한다.
	else
		return 0;
}

int CItem::GiveMoreTime_Fix(DWORD dwTime)
{
	if (IsDragonSoul())
	{
		DWORD duration = DSManager::instance().GetDuration(this);
		DWORD remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);
		if (remain_sec == duration)
			return false;
		if ((dwTime + remain_sec) >= duration)
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
			return duration - remain_sec;
		}
		else
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, dwTime + remain_sec);
			return dwTime;
		}
	}
	// 우선 용혼석에 관해서만 하도록 한다.
	else
		return 0;
}


int	CItem::GetDuration()
{
	if(!GetProto())
		return -1;

	for (int i=0 ; i < ITEM_LIMIT_MAX_NUM ; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType)
			return GetProto()->aLimits[i].lValue;
	}

	if (GetProto()->cLimitTimerBasedOnWearIndex >= 0)
	{
		BYTE cLTBOWI = GetProto()->cLimitTimerBasedOnWearIndex;
		return GetProto()->aLimits[cLTBOWI].lValue;
	}

	return -1;
}

bool CItem::IsSameSpecialGroup(const LPITEM item) const
{
	// 서로 VNUM이 같다면 같은 그룹인 것으로 간주
	if (this->GetVnum() == item->GetVnum())
		return true;

	if (GetSpecialGroup() && (item->GetSpecialGroup() == GetSpecialGroup()))
		return true;

	return false;
}

#ifdef __SOULBIND__
void CItem::SetSealBind(long sp)
{
	m_lSealbind = sp;
	UpdatePacket();
	Save();
}

EVENTFUNC(unbind_timer_expire_event)
{
	const item_vid_event_info* info = reinterpret_cast<const item_vid_event_info*>(event->info);
	if (!info)
		return 0;

	const LPITEM item = ITEM_MANAGER::instance().FindByVID( info->item_vid );
	if (!item)
		return 0;
	
	const time_t current = get_global_time();
	if (current > item->GetSealBindTime())
	{
		item->SetSealBind(0);
		return 0;
	}
	
	return PASSES_PER_SEC (1);
}

void CItem::StartUnSealBindTimerExpireEvent()
{
	if (m_pkUnSealTimerExpireEvent)
		return;

	item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
	info->item_vid = GetVID();

	m_pkUnSealTimerExpireEvent = event_create(unbind_timer_expire_event, info, PASSES_PER_SEC(1));
	sys_log(0, "UNSEAL_TIME_EXPIRE: StartUnSealBindTimerExpireEvent");
}

#endif

void CItem::SetBasic(bool IsBasic)
{
	bIsBasic = IsBasic;
	UpdatePacket();
}

bool CItem::IsStackable()
{
	switch(GetType())
	{
		case ITEM_NONE:
		case ITEM_WEAPON:
		case ITEM_ARMOR:
		case ITEM_HAIR:
		case ITEM_COSTUME:
		case ITEM_UNIQUE:
			return false;
    // ITEM_NONE,              //0
    // ITEM_WEAPON,            //1
    // ITEM_ARMOR,             //2
    // ITEM_USE,               //3
    // ITEM_AUTOUSE,           //4
    // ITEM_MATERIAL,          //5
    // ITEM_SPECIAL,           //6
    // ITEM_TOOL,              //7
    // ITEM_LOTTERY,           //8
    // ITEM_ELK,               //9
    // ITEM_METIN,             //10
    // ITEM_CONTAINER,         //11
    // ITEM_FISH,              //12
    // ITEM_ROD,               //13
    // ITEM_RESOURCE,          //14
    // ITEM_CAMPFIRE,          //15
    // ITEM_UNIQUE,            //16
    // ITEM_SKILLBOOK,         //17
    // ITEM_QUEST,             //18
    // ITEM_POLYMORPH,         //19
    // ITEM_TREASURE_BOX,      //20
    // ITEM_TREASURE_KEY,      //21/
    // ITEM_SKILLFORGET,       //22
    // ITEM_GIFTBOX,           //23
    // ITEM_PICK,              //24
    // ITEM_HAIR,              //25
    // ITEM_TOTEM,             //26
	// ITEM_BLEND,				//27
	// ITEM_COSTUME,			//28
	// ITEM_DS,				//29
	// ITEM_SPECIAL_DS,		//30
	// ITEM_EXTRACT,			//31
	// ITEM_SECONDARY_COIN,	//32
	// ITEM_RING,				//33
	// ITEM_BELT,				//34
	// ITEM_PET,				//35
// #ifdef __ATTR_TRANSFER__
	// ITEM_MEDIUM,			//36
// #endif
// #ifdef __SPECIAL_GACHA__
	// ITEM_GACHA,				//37
// #endif
	// ITEM_SOUL,				//38
		default:
			break;
	}

	return (GetFlag() & ITEM_FLAG_STACKABLE || (!(GetFlag() & ITEM_FLAG_STACKABLE) && !IS_SET(GetAntiFlag(), ITEM_ANTIFLAG_STACK)));
}

