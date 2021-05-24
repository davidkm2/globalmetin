#include "StdAfx.h"
#include "PythonItem.h"
#include "Packet.h"
#include "../GameLib/ItemManager.h"
#include "../GameLib/GameLibDefines.h"
#include "InstanceBase.h"
#include "AbstractApplication.h"

extern int TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE;

PyObject * itemSetUseSoundFileName(PyObject * poSelf, PyObject * poArgs)
{
	int iUseSound;
	if (!PyTuple_GetInteger(poArgs, 0, &iUseSound))
		return Py_BadArgument();

	char* szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BadArgument();

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.SetUseSoundFileName(iUseSound, szFileName);
	return Py_BuildNone();
}

PyObject * itemSetDropSoundFileName(PyObject * poSelf, PyObject * poArgs)
{
	int iDropSound;
	if (!PyTuple_GetInteger(poArgs, 0, &iDropSound))
		return Py_BadArgument();

	char* szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BadArgument();

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.SetDropSoundFileName(iDropSound, szFileName);
	return Py_BuildNone();
}

PyObject * itemSelectItem(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();

	if (!CItemManager::Instance().SelectItemData(iIndex))
	{
		TraceError("Cannot find item by %d", iIndex);
#ifdef ENABLE_NEW_USER_CARE
		return Py_BuildValue("b", false);
#else
		CItemManager::Instance().SelectItemData(60001);
#endif
	}

#ifdef ENABLE_NEW_USER_CARE
	return Py_BuildValue("b", true);
#else
	return Py_BuildNone();
#endif
}

PyObject * itemGetItemName(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("s", pItemData->GetName());
}

PyObject * itemGetItemDescription(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("s", pItemData->GetDescription());
}

PyObject * itemGetItemSummary(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("s", pItemData->GetSummary());
}

PyObject * itemGetIconImage(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");
	return Py_BuildValue("i", pItemData->GetIconImage());
}

PyObject * itemGetIconImageFileName(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	CGraphicSubImage * pImage = pItemData->GetIconImage();
	if (!pImage)
		return Py_BuildValue("s", "Noname");

	return Py_BuildValue("s", pImage->GetFileName());
}

PyObject * itemGetItemSize(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("(ii)", 1, pItemData->GetSize());
}

PyObject * itemGetItemType(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetType());
}

PyObject * itemGetItemSubType(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetSubType());
}

PyObject * itemGetIBuyItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();

	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetIBuyItemPrice());
}

PyObject * itemGetISellItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();

	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetISellItemPrice());
}
PyObject * itemIsAntiFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->IsAntiFlag(iFlag));
}

PyObject * itemIsFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->IsFlag(iFlag));
}

PyObject * itemIsWearableFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->IsWearableFlag(iFlag));
}

PyObject * itemIs1GoldItem(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->IsFlag(CItemData::ITEM_FLAG_COUNT_PER_1GOLD));
}

PyObject * itemGetLimit(PyObject * poSelf, PyObject * poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	CItemData::TItemLimit ItemLimit;
	if (!pItemData->GetLimit(iValueIndex, &ItemLimit))
		return Py_BuildException();

	return Py_BuildValue("ii", ItemLimit.bType, ItemLimit.lValue);
}

PyObject * itemGetAffect(PyObject * poSelf, PyObject * poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	CItemData::TItemApply ItemApply;
	if (!pItemData->GetApply(iValueIndex, &ItemApply))
		return Py_BuildException();

	if ((CItemData::APPLY_ATT_SPEED == ItemApply.bType) && (CItemData::ITEM_TYPE_WEAPON == pItemData->GetType()) && (CItemData::WEAPON_TWO_HANDED == pItemData->GetSubType()))
	{
		ItemApply.lValue -= TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE;
	}

	return Py_BuildValue("ii", ItemApply.bType, ItemApply.lValue);
}

PyObject * itemGetValue(PyObject * poSelf, PyObject * poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	return Py_BuildValue("i", pItemData->GetValue(iValueIndex));
}

PyObject * itemGetSocket(PyObject * poSelf, PyObject * poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	return Py_BuildValue("i", pItemData->GetSocket(iValueIndex));
}

PyObject * itemGetIconInstance(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	CGraphicSubImage * pImage = pItemData->GetIconImage();
	if (!pImage)
		return Py_BuildException("Cannot get icon image by %d", pItemData->GetIndex());

	CGraphicImageInstance * pImageInstance = CGraphicImageInstance::New();
	pImageInstance->SetImagePointer(pImage);

	return Py_BuildValue("i", pImageInstance);
}

PyObject * itemDeleteIconInstance(PyObject * poSelf, PyObject * poArgs)
{
	int iHandle;
	if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
		return Py_BadArgument();

	CGraphicImageInstance::Delete((CGraphicImageInstance *) iHandle);

	return Py_BuildNone();
}

PyObject * itemIsEquipmentVID(PyObject * poSelf, PyObject * poArgs)
{
	int iItemVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemVID);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	return Py_BuildValue("i", pItemData->IsEquipment());
}

PyObject* itemGetUseType(PyObject * poSelf, PyObject * poArgs)
{
	int iItemVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemVID);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	return Py_BuildValue("s", pItemData->GetUseTypeString());
}

PyObject * itemIsRefineScroll(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (pItemData->GetType() != CItemData::ITEM_TYPE_USE)
		return Py_BuildValue("i", FALSE);

	switch (pItemData->GetSubType())
	{
		case CItemData::USE_TUNING:
			return Py_BuildValue("i", TRUE);
			break;
	}

	return Py_BuildValue("i", FALSE);
}

PyObject * itemIsDetachScroll(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	int iType = pItemData->GetType();
	int iSubType = pItemData->GetSubType();
	if (iType == CItemData::ITEM_TYPE_USE)
	if (iSubType == CItemData::USE_DETACHMENT)
	{
		return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}

PyObject * itemCanAddToQuickSlotItem(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (CItemData::ITEM_TYPE_USE == pItemData->GetType() || CItemData::ITEM_TYPE_QUEST == pItemData->GetType())
	{
		return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}

PyObject * itemIsKey(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (CItemData::ITEM_TYPE_TREASURE_KEY == pItemData->GetType())
	{
		return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}

PyObject * itemIsMetin(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (CItemData::ITEM_TYPE_METIN == pItemData->GetType())
	{
		return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}

PyObject * itemRender(PyObject * poSelf, PyObject * poArgs)
{
	CPythonItem::Instance().Render();
	return Py_BuildNone();
}

PyObject * itemUpdate(PyObject * poSelf, PyObject * poArgs)
{
	IAbstractApplication& rkApp=IAbstractApplication::GetSingleton();

	POINT ptMouse;
	rkApp.GetMousePosition(&ptMouse);

	CPythonItem::Instance().Update(ptMouse);
	return Py_BuildNone();
}

PyObject * itemDeleteItem(PyObject * poSelf, PyObject * poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BadArgument();

	CPythonItem::Instance().DeleteItem(iVirtualID);
	return Py_BuildNone();
}

PyObject * itemPick(PyObject * poSelf, PyObject * poArgs)
{
	DWORD dwItemID;
	if (CPythonItem::Instance().GetPickedItemID(&dwItemID))
		return Py_BuildValue("i", dwItemID);
	else
		return Py_BuildValue("i", -1);
}

PyObject* itemLoadItemTable(PyObject* poSelf, PyObject* poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BadArgument();

	CItemManager::Instance().LoadItemTable(szFileName);
	return Py_BuildNone();
}

#ifdef ENABLE_SOULBIND_SYSTEM
PyObject * itemIsSealScroll(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	if (iItemIndex == CItemData::SEAL_ITEM_BINDING_VNUM )
		return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}

PyObject * itemIsUnSealScroll(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	if (iItemIndex == CItemData::SEAL_ITEM_UNBINDING_VNUM)
		return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}
#endif

#ifdef ENABLE_DETAILS_UI
PyObject * itemGetApplyPoint(PyObject * poSelf, PyObject * poArgs)
{
	int iApply;
	if (!PyTuple_GetInteger(poArgs, 0, &iApply))
		return Py_BadArgument();

	int iRet;
	switch(iApply)
	{
		case CItemData::APPLY_ATTBONUS_HUMAN:
			iRet = POINT_ATTBONUS_HUMAN;
			break;
#ifdef ENABLE_PENDANT
		case CItemData::APPLY_RESIST_HUMAN:
			iRet = POINT_RESIST_HUMAN;
			break;
#endif
		case CItemData::APPLY_ATTBONUS_ORC:
			iRet = POINT_ATTBONUS_ORC;
			break;
		case CItemData::APPLY_ATTBONUS_UNDEAD:
			iRet = POINT_ATTBONUS_UNDEAD;
			break;
		case CItemData::APPLY_ATT_BONUS_TO_MONSTER:
			iRet = POINT_ATTBONUS_MONSTER;
			break;
#ifdef ENABLE_ELEMENT_ADD
		case CItemData::APPLY_ATTBONUS_CZ:
			iRet = POINT_ATTBONUS_CZ;
			break;
		case CItemData::APPLY_ATTBONUS_DESERT:
			iRet = POINT_ATTBONUS_DESERT;
			break;
		case CItemData::APPLY_ATTBONUS_INSECT:
			iRet = POINT_ATTBONUS_INSECT;
			break;
		case CItemData::APPLY_ENCHANT_ELECT:
			iRet = POINT_ENCHANT_ELECT;
			break;
		case CItemData::APPLY_ENCHANT_ICE:
			iRet = POINT_ENCHANT_ICE;
			break;
		case CItemData::APPLY_ENCHANT_DARK:
			iRet = POINT_ENCHANT_DARK;
			break;
		case CItemData::APPLY_ENCHANT_FIRE:
			iRet = POINT_ENCHANT_FIRE;
			break;
		case CItemData::APPLY_ENCHANT_WIND:
			iRet = POINT_ENCHANT_WIND;
			break;
		case CItemData::APPLY_ENCHANT_EARTH:
			iRet = POINT_ENCHANT_EARTH;
			break;
		case CItemData::APPLY_ATTBONUS_SWORD:
			iRet = POINT_ATTBONUS_SWORD;
			break;
		case CItemData::APPLY_ATTBONUS_TWOHAND:
			iRet = POINT_ATTBONUS_TWOHAND;
			break;
		case CItemData::APPLY_ATTBONUS_DAGGER:
			iRet = POINT_ATTBONUS_DAGGER;
			break;
		case CItemData::APPLY_ATTBONUS_BELL:
			iRet = POINT_ATTBONUS_BELL;
			break;
		case CItemData::APPLY_ATTBONUS_FAN:
			iRet = POINT_ATTBONUS_FAN;
			break;
		case CItemData::APPLY_ATTBONUS_BOW:
			iRet = POINT_ATTBONUS_BOW;
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER
		case CItemData::APPLY_ATTBONUS_CLAW:
			iRet = POINT_ATTBONUS_CLAW;
			break;
#endif
		case CItemData::APPLY_RESIST_MOUNT_FALL:
			iRet = POINT_RESIST_MOUNT_FALL;
			break;
#endif
		case CItemData::APPLY_MOUNT:
			iRet = POINT_MOUNT;
			break;

		case CItemData::APPLY_NONAME:
			iRet = POINT_NONAME;
			break;

		case CItemData::APPLY_ATTBONUS_ANIMAL:
			iRet = POINT_ATTBONUS_ANIMAL;
			break;
		case CItemData::APPLY_ATTBONUS_MILGYO:
			iRet = POINT_ATTBONUS_MILGYO;
			break;
		case CItemData::APPLY_ATTBONUS_DEVIL:
			iRet = POINT_ATTBONUS_DEVIL;
			break;
		case CItemData::APPLY_ATT_GRADE_BONUS:
			iRet = POINT_ATT_GRADE_BONUS;
			break;
		case CItemData::APPLY_DEF_GRADE_BONUS:
			iRet = POINT_DEF_GRADE_BONUS;
			break;
		case CItemData::APPLY_NORMAL_HIT_DAMAGE_BONUS:
			iRet = POINT_NORMAL_HIT_DAMAGE_BONUS;
			break;
		case CItemData::APPLY_NORMAL_HIT_DEFEND_BONUS:
			iRet = POINT_NORMAL_HIT_DEFEND_BONUS;
			break;
		case CItemData::APPLY_SKILL_DAMAGE_BONUS:
			iRet = POINT_SKILL_DAMAGE_BONUS;
			break;
		case CItemData::APPLY_SKILL_DEFEND_BONUS:
			iRet = POINT_SKILL_DEFEND_BONUS;
			break;
		case CItemData::APPLY_MELEE_MAGIC_ATTBONUS_PER:
			iRet = POINT_MELEE_MAGIC_ATT_BONUS_PER;
			break;
		case CItemData::APPLY_MAGIC_ATTBONUS_PER:
			iRet = POINT_MAGIC_ATT_BONUS_PER;
			break;
		case CItemData::APPLY_CRITICAL_PCT:
			iRet = POINT_CRITICAL_PCT;
			break;
		case CItemData::APPLY_PENETRATE_PCT:
			iRet = POINT_PENETRATE_PCT;
			break;
		case CItemData::APPLY_ANTI_CRITICAL_PCT:
			iRet = POINT_RESIST_CRITICAL;
			break;
		case CItemData::APPLY_ANTI_PENETRATE_PCT:
			iRet = POINT_RESIST_PENETRATE;
			break;
		case CItemData::APPLY_RESIST_MAGIC:
			iRet = POINT_RESIST_MAGIC;
			break;
		case CItemData::APPLY_RESIST_ELEC:
			iRet = POINT_RESIST_ELEC;
			break;
		case CItemData::APPLY_RESIST_ICE:
			iRet = POINT_RESIST_ICE;
			break;
		case CItemData::APPLY_RESIST_DARK:
			iRet = POINT_RESIST_DARK;
			break;
		case CItemData::APPLY_RESIST_FIRE:
			iRet = POINT_RESIST_FIRE;
			break;
		case CItemData::APPLY_RESIST_WIND:
			iRet = POINT_RESIST_WIND;
			break;
		case CItemData::APPLY_RESIST_EARTH:
			iRet = POINT_RESIST_EARTH;
			break;
		case CItemData::APPLY_RESIST_MAGIC_REDUCTION:
			iRet = POINT_RESIST_MAGIC_REDUCTION;
			break;
		case CItemData::APPLY_ATT_BONUS_TO_WARRIOR:
			iRet = POINT_ATTBONUS_WARRIOR;
			break;
		case CItemData::APPLY_ATT_BONUS_TO_ASSASSIN:
			iRet = POINT_ATTBONUS_ASSASSIN;
			break;
		case CItemData::APPLY_ATT_BONUS_TO_SURA:
			iRet = POINT_ATTBONUS_SURA;
			break;
		case CItemData::APPLY_ATT_BONUS_TO_SHAMAN:
			iRet = POINT_ATTBONUS_SHAMAN;
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER
		case CItemData::APPLY_ATT_BONUS_TO_WOLFMAN:
			iRet = POINT_ATTBONUS_WOLFMAN;
			break;
		case CItemData::APPLY_RESIST_WOLFMAN:
			iRet = POINT_RESIST_WOLFMAN;
			break;
		case CItemData::APPLY_RESIST_CLAW:
			iRet = POINT_RESIST_CLAW;
			break;
		case CItemData::APPLY_BLEEDING_PCT:
			iRet = POINT_BLEEDING_PCT;
			break;
		case CItemData::APPLY_BLEEDING_REDUCE:
			iRet = POINT_BLEEDING_REDUCE;
			break;
#endif
		case CItemData::APPLY_RESIST_WARRIOR:
			iRet = POINT_RESIST_WARRIOR;
			break;
		case CItemData::APPLY_RESIST_ASSASSIN:
			iRet = POINT_RESIST_ASSASSIN;
			break;
		case CItemData::APPLY_RESIST_SURA:
			iRet = POINT_RESIST_SURA;
			break;
		case CItemData::APPLY_RESIST_SHAMAN:
			iRet = POINT_RESIST_SHAMAN;
			break;
		case CItemData::APPLY_RESIST_SWORD:
			iRet = POINT_RESIST_SWORD;
			break;
		case CItemData::APPLY_RESIST_TWOHAND:
			iRet = POINT_RESIST_TWOHAND;
			break;
		case CItemData::APPLY_RESIST_DAGGER:
			iRet = POINT_RESIST_DAGGER;
			break;
		case CItemData::APPLY_RESIST_BELL:
			iRet = POINT_RESIST_BELL;
			break;
		case CItemData::APPLY_RESIST_FAN:
			iRet = POINT_RESIST_FAN;
			break;
		case CItemData::APPLY_RESIST_BOW:
			iRet = POINT_RESIST_BOW;
			break;
		case CItemData::APPLY_STUN_PCT:
			iRet = POINT_STUN_PCT;
			break;
		case CItemData::APPLY_SLOW_PCT:
			iRet = POINT_SLOW_PCT;
			break;
		case CItemData::APPLY_POISON_PCT:
			iRet = POINT_POISON_PCT;
			break;
		case CItemData::APPLY_POISON_REDUCE:
			iRet = POINT_POISON_REDUCE;
			break;
		case CItemData::APPLY_STEAL_HP:
			iRet = POINT_STEAL_HP;
			break;
		case CItemData::APPLY_STEAL_SP:
			iRet = POINT_STEAL_SP;
			break;
		case CItemData::APPLY_HP_REGEN:
			iRet = POINT_HP_REGEN;
			break;
		case CItemData::APPLY_SP_REGEN:
			iRet = POINT_SP_REGEN;
			break;
		case CItemData::APPLY_BLOCK:
			iRet = POINT_BLOCK;
			break;
		case CItemData::APPLY_DODGE:
			iRet = POINT_DODGE;
			break;
		case CItemData::APPLY_REFLECT_MELEE:
			iRet = POINT_REFLECT_MELEE;
			break;
		case CItemData::APPLY_KILL_HP_RECOVER:
			iRet = POINT_KILL_HP_RECOVER;
			break;
		case CItemData::APPLY_KILL_SP_RECOVER:
			iRet = POINT_KILL_SP_RECOVER;
			break;
		case CItemData::APPLY_EXP_DOUBLE_BONUS:
			iRet = POINT_EXP_DOUBLE_BONUS;
			break;
		case CItemData::APPLY_GOLD_DOUBLE_BONUS:
			iRet = POINT_GOLD_DOUBLE_BONUS;
			break;
		case CItemData::APPLY_ITEM_DROP_BONUS:
			iRet = POINT_ITEM_DROP_BONUS;
			break;
		case CItemData::APPLY_MALL_ATTBONUS:
			iRet = POINT_MALL_ATTBONUS;
			break;
		case CItemData::APPLY_MALL_DEFBONUS:
			iRet = POINT_MALL_DEFBONUS;
			break;
		case CItemData::APPLY_MALL_EXPBONUS:
			iRet = POINT_MALL_EXPBONUS;
			break;
		case CItemData::APPLY_MALL_ITEMBONUS:
			iRet = POINT_MALL_ITEMBONUS;
			break;
		case CItemData::APPLY_MALL_GOLDBONUS:
			iRet = POINT_MALL_GOLDBONUS;
			break;
		default:
			iRet = 0;
			break;
	}

	return Py_BuildValue("i", iRet);
}
#endif

#ifdef ENABLE_ATTR_6TH_7TH
PyObject * itemGetLevelLimit(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	for (int i = 0; i < CItemData::ITEM_LIMIT_MAX_NUM; ++i)
	{
		CItemData::TItemLimit ItemLimit;
		if (!pItemData->GetLimit(i, &ItemLimit))
			return Py_BuildException();

		if (ItemLimit.bType == CItemData::LIMIT_LEVEL)
			return Py_BuildValue("i", ItemLimit.lValue);
	}

	return Py_BuildNone();
}
#endif

void initItem()
{
	static PyMethodDef s_methods[] =
	{
		{ "SetUseSoundFileName",			itemSetUseSoundFileName,				METH_VARARGS },
		{ "SetDropSoundFileName",			itemSetDropSoundFileName,				METH_VARARGS },
		{ "SelectItem",						itemSelectItem,							METH_VARARGS },
		{ "GetItemName",					itemGetItemName,						METH_VARARGS },
		{ "GetItemDescription",				itemGetItemDescription,					METH_VARARGS },
		{ "GetItemSummary",					itemGetItemSummary,						METH_VARARGS },
		{ "GetIconImage",					itemGetIconImage,						METH_VARARGS },
		{ "GetIconImageFileName",			itemGetIconImageFileName,				METH_VARARGS },
		{ "GetItemSize",					itemGetItemSize,						METH_VARARGS },
		{ "GetItemType",					itemGetItemType,						METH_VARARGS },
		{ "GetItemSubType",					itemGetItemSubType,						METH_VARARGS },
		{ "GetIBuyItemPrice",				itemGetIBuyItemPrice,					METH_VARARGS },
		{ "GetISellItemPrice",				itemGetISellItemPrice,					METH_VARARGS },
		{ "IsAntiFlag",						itemIsAntiFlag,							METH_VARARGS },
		{ "IsFlag",							itemIsFlag,								METH_VARARGS },
		{ "IsWearableFlag",					itemIsWearableFlag,						METH_VARARGS },
		{ "Is1GoldItem",					itemIs1GoldItem,						METH_VARARGS },
		{ "GetLimit",						itemGetLimit,							METH_VARARGS },
		{ "GetAffect",						itemGetAffect,							METH_VARARGS },
		{ "GetValue",						itemGetValue,							METH_VARARGS },
		{ "GetSocket",						itemGetSocket,							METH_VARARGS },
		{ "GetIconInstance",				itemGetIconInstance,					METH_VARARGS },
		{ "GetUseType",						itemGetUseType,							METH_VARARGS },
		{ "DeleteIconInstance",				itemDeleteIconInstance,					METH_VARARGS },
		{ "IsEquipmentVID",					itemIsEquipmentVID,						METH_VARARGS },
		{ "IsRefineScroll",					itemIsRefineScroll,						METH_VARARGS },
		{ "IsDetachScroll",					itemIsDetachScroll,						METH_VARARGS },
		{ "IsKey",							itemIsKey,								METH_VARARGS },
		{ "IsMetin",						itemIsMetin,							METH_VARARGS },
		{ "CanAddToQuickSlotItem",			itemCanAddToQuickSlotItem,				METH_VARARGS },

		{ "Update",							itemUpdate,								METH_VARARGS },
		{ "Render",							itemRender,								METH_VARARGS },
		{ "DeleteItem",						itemDeleteItem,							METH_VARARGS },
		{ "Pick",							itemPick,								METH_VARARGS },

		{ "LoadItemTable",					itemLoadItemTable,						METH_VARARGS },
#ifdef ENABLE_SOULBIND_SYSTEM
		{ "IsSealScroll",					itemIsSealScroll,						METH_VARARGS },
		{ "IsUnSealScroll",					itemIsUnSealScroll,						METH_VARARGS },
#endif
#ifdef ENABLE_DETAILS_UI
		{ "GetApplyPoint",					itemGetApplyPoint,						METH_VARARGS },
#endif
#ifdef ENABLE_ATTR_6TH_7TH
		{ "GetLevelLimit",					itemGetLevelLimit,						METH_VARARGS },
#endif
		{ NULL,								NULL,									NULL		 },
	};

	PyObject * poModule = Py_InitModule("item", s_methods);

	PyModule_AddIntConstant(poModule, "USESOUND_ACCESSORY",			CPythonItem::USESOUND_ACCESSORY);
	PyModule_AddIntConstant(poModule, "USESOUND_ARMOR",				CPythonItem::USESOUND_ARMOR);
	PyModule_AddIntConstant(poModule, "USESOUND_BOW",				CPythonItem::USESOUND_BOW);
	PyModule_AddIntConstant(poModule, "USESOUND_DEFAULT",			CPythonItem::USESOUND_DEFAULT);
	PyModule_AddIntConstant(poModule, "USESOUND_WEAPON",			CPythonItem::USESOUND_WEAPON);
	PyModule_AddIntConstant(poModule, "USESOUND_POTION",			CPythonItem::USESOUND_POTION);
	PyModule_AddIntConstant(poModule, "USESOUND_PORTAL",			CPythonItem::USESOUND_PORTAL);

	PyModule_AddIntConstant(poModule, "DROPSOUND_ACCESSORY",		CPythonItem::DROPSOUND_ACCESSORY);
	PyModule_AddIntConstant(poModule, "DROPSOUND_ARMOR",			CPythonItem::DROPSOUND_ARMOR);
	PyModule_AddIntConstant(poModule, "DROPSOUND_BOW",				CPythonItem::DROPSOUND_BOW);
	PyModule_AddIntConstant(poModule, "DROPSOUND_DEFAULT",			CPythonItem::DROPSOUND_DEFAULT);
	PyModule_AddIntConstant(poModule, "DROPSOUND_WEAPON",			CPythonItem::DROPSOUND_WEAPON);

	PyModule_AddIntConstant(poModule, "EQUIPMENT_COUNT",			EQUIPMENT_MAX);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_HEAD",				EQUIPMENT_HEAD);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_BODY",				EQUIPMENT_BODY);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_WEAPON",			EQUIPMENT_WEAPON);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_WRIST",			EQUIPMENT_WRIST);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_SHOES",			EQUIPMENT_SHOES);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_NECK",				EQUIPMENT_NECK);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_EAR",				EQUIPMENT_EAR);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_UNIQUE1",			EQUIPMENT_UNIQUE1);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_UNIQUE2",			EQUIPMENT_UNIQUE2);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_ARROW",			EQUIPMENT_ARROW);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_SHIELD",			EQUIPMENT_SHIELD);

	PyModule_AddIntConstant(poModule, "EQUIPMENT_RING1",			EQUIPMENT_RING1);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_RING2",			EQUIPMENT_RING2);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_BELT",				EQUIPMENT_BELT);
#ifdef ENABLE_PENDANT
	PyModule_AddIntConstant(poModule, "EQUIPMENT_PENDANT",			EQUIPMENT_PENDANT);
#endif
#ifdef ENABLE_COSTUME_PET
	PyModule_AddIntConstant(poModule, "EQUIPMENT_PET", 				EQUIPMENT_PET);
#endif

	PyModule_AddIntConstant(poModule, "ITEM_TYPE_NONE",				CItemData::ITEM_TYPE_NONE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_WEAPON",			CItemData::ITEM_TYPE_WEAPON);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_ARMOR",			CItemData::ITEM_TYPE_ARMOR);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_USE",				CItemData::ITEM_TYPE_USE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_AUTOUSE",			CItemData::ITEM_TYPE_AUTOUSE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_MATERIAL",			CItemData::ITEM_TYPE_MATERIAL);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SPECIAL",			CItemData::ITEM_TYPE_SPECIAL);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_TOOL",				CItemData::ITEM_TYPE_TOOL);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_LOTTERY",			CItemData::ITEM_TYPE_LOTTERY);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_ELK",				CItemData::ITEM_TYPE_ELK);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_METIN",			CItemData::ITEM_TYPE_METIN);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_CONTAINER",		CItemData::ITEM_TYPE_CONTAINER);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_FISH",				CItemData::ITEM_TYPE_FISH);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_ROD",				CItemData::ITEM_TYPE_ROD);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_RESOURCE",			CItemData::ITEM_TYPE_RESOURCE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_CAMPFIRE",			CItemData::ITEM_TYPE_CAMPFIRE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_UNIQUE",			CItemData::ITEM_TYPE_UNIQUE);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SKILLBOOK",		CItemData::ITEM_TYPE_SKILLBOOK);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_QUEST",			CItemData::ITEM_TYPE_QUEST);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_POLYMORPH",		CItemData::ITEM_TYPE_POLYMORPH);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_TREASURE_BOX",		CItemData::ITEM_TYPE_TREASURE_BOX);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_TREASURE_KEY",		CItemData::ITEM_TYPE_TREASURE_KEY);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_PICK",				CItemData::ITEM_TYPE_PICK);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_BLEND",			CItemData::ITEM_TYPE_BLEND);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_DS",				CItemData::ITEM_TYPE_DS);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SPECIAL_DS",		CItemData::ITEM_TYPE_SPECIAL_DS);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_RING",				CItemData::ITEM_TYPE_RING);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_BELT",				CItemData::ITEM_TYPE_BELT);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_COSTUME",			CItemData::ITEM_TYPE_COSTUME);
#ifdef ENABLE_ATTR_TRANSFER_SYSTEM
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_MEDIUM", 			CItemData::ITEM_TYPE_MEDIUM);
#endif
#ifdef ENABLE_SPECIAL_GACHA
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_GACHA",			CItemData::ITEM_TYPE_GACHA);
#endif
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_GIFTBOX",			CItemData::ITEM_TYPE_GIFTBOX);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_PET",				CItemData::ITEM_TYPE_PET);
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SOUL",				CItemData::ITEM_TYPE_SOUL);

	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_BODY",			CItemData::COSTUME_BODY);
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_HAIR",			CItemData::COSTUME_HAIR);
#ifdef ENABLE_COSTUME_MOUNT
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_MOUNT",			CItemData::COSTUME_MOUNT);
#endif
#ifdef ENABLE_ACCE_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_ACCE", 			CItemData::COSTUME_ACCE);
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_WEAPON", 		CItemData::COSTUME_WEAPON);
#endif
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_START",			EQUIPMENT_MAX);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_COUNT",			COSTUME_MAX - EQUIPMENT_MAX);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_BODY",			COSTUME_BODY);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_HAIR",			COSTUME_HAIR);
#ifdef ENABLE_COSTUME_MOUNT
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_MOUNT",			COSTUME_MOUNT);
#endif
#ifdef ENABLE_ACCE_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_ACCE", 			COSTUME_ACCE);
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_WEAPON", 		COSTUME_WEAPON);
#endif
#ifdef ENABLE_COSTUME_EFFECT
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_EFFECT_BODY",		COSTUME_EFFECT_BODY);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_EFFECT_WEAPON",		COSTUME_EFFECT_WEAPON);
#endif

	PyModule_AddIntConstant(poModule, "BELT_INVENTORY_SLOT_START",			c_Belt_Inventory_Slot_Start);
	PyModule_AddIntConstant(poModule, "BELT_INVENTORY_SLOT_COUNT",			c_Belt_Inventory_Slot_Count);
	PyModule_AddIntConstant(poModule, "BELT_INVENTORY_SLOT_END",			c_Belt_Inventory_Slot_End);

	PyModule_AddIntConstant(poModule, "WEAPON_SWORD",				CItemData::WEAPON_SWORD);
	PyModule_AddIntConstant(poModule, "WEAPON_DAGGER",				CItemData::WEAPON_DAGGER);
	PyModule_AddIntConstant(poModule, "WEAPON_BOW",					CItemData::WEAPON_BOW);
	PyModule_AddIntConstant(poModule, "WEAPON_TWO_HANDED",			CItemData::WEAPON_TWO_HANDED);
	PyModule_AddIntConstant(poModule, "WEAPON_BELL",				CItemData::WEAPON_BELL);
	PyModule_AddIntConstant(poModule, "WEAPON_FAN",					CItemData::WEAPON_FAN);
	PyModule_AddIntConstant(poModule, "WEAPON_ARROW",				CItemData::WEAPON_ARROW);
	PyModule_AddIntConstant(poModule, "WEAPON_MOUNT_SPEAR",			CItemData::WEAPON_MOUNT_SPEAR);
#ifdef ENABLE_WOLFMAN_CHARACTER
	PyModule_AddIntConstant(poModule, "WEAPON_CLAW",				CItemData::WEAPON_CLAW);
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	PyModule_AddIntConstant(poModule, "WEAPON_QUIVER", CItemData::WEAPON_QUIVER);
#endif
	PyModule_AddIntConstant(poModule, "WEAPON_NUM_TYPES",			CItemData::WEAPON_NUM_TYPES);

	PyModule_AddIntConstant(poModule, "USE_POTION",					CItemData::USE_POTION);
	PyModule_AddIntConstant(poModule, "USE_TALISMAN",				CItemData::USE_TALISMAN);
	PyModule_AddIntConstant(poModule, "USE_TUNING",					CItemData::USE_TUNING);
	PyModule_AddIntConstant(poModule, "USE_MOVE",					CItemData::USE_MOVE);
	PyModule_AddIntConstant(poModule, "USE_TREASURE_BOX",			CItemData::USE_TREASURE_BOX);
	PyModule_AddIntConstant(poModule, "USE_MONEYBAG",				CItemData::USE_MONEYBAG);
	PyModule_AddIntConstant(poModule, "USE_BAIT",					CItemData::USE_BAIT);
	PyModule_AddIntConstant(poModule, "USE_ABILITY_UP",				CItemData::USE_ABILITY_UP);
	PyModule_AddIntConstant(poModule, "USE_AFFECT",					CItemData::USE_AFFECT);
	PyModule_AddIntConstant(poModule, "USE_CREATE_STONE",			CItemData::USE_CREATE_STONE);
	PyModule_AddIntConstant(poModule, "USE_SPECIAL",				CItemData::USE_SPECIAL);
	PyModule_AddIntConstant(poModule, "USE_POTION_NODELAY",			CItemData::USE_POTION_NODELAY);
	PyModule_AddIntConstant(poModule, "USE_CLEAR",					CItemData::USE_CLEAR);
	PyModule_AddIntConstant(poModule, "USE_INVISIBILITY",			CItemData::USE_INVISIBILITY);
	PyModule_AddIntConstant(poModule, "USE_DETACHMENT",				CItemData::USE_DETACHMENT);
	PyModule_AddIntConstant(poModule, "USE_TIME_CHARGE_PER",		CItemData::USE_TIME_CHARGE_PER);
	PyModule_AddIntConstant(poModule, "USE_TIME_CHARGE_FIX",		CItemData::USE_TIME_CHARGE_FIX);

	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_NORMAL",	CItemData::MATERIAL_DS_REFINE_NORMAL);
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_BLESSED",	CItemData::MATERIAL_DS_REFINE_BLESSED);
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_HOLLY",	CItemData::MATERIAL_DS_REFINE_HOLLY);
	PyModule_AddIntConstant(poModule, "MATERIAL_LEATHER",			CItemData::MATERIAL_LEATHER);

	PyModule_AddIntConstant(poModule, "METIN_NORMAL",				CItemData::METIN_NORMAL);
	PyModule_AddIntConstant(poModule, "METIN_GOLD",					CItemData::METIN_GOLD);

	PyModule_AddIntConstant(poModule, "LIMIT_NONE",					CItemData::LIMIT_NONE);
	PyModule_AddIntConstant(poModule, "LIMIT_LEVEL",				CItemData::LIMIT_LEVEL);
	PyModule_AddIntConstant(poModule, "LIMIT_STR",					CItemData::LIMIT_STR);
	PyModule_AddIntConstant(poModule, "LIMIT_DEX",					CItemData::LIMIT_DEX);
	PyModule_AddIntConstant(poModule, "LIMIT_INT",					CItemData::LIMIT_INT);
	PyModule_AddIntConstant(poModule, "LIMIT_CON",					CItemData::LIMIT_CON);
	PyModule_AddIntConstant(poModule, "LIMIT_PCBANG",				CItemData::LIMIT_PCBANG);
	PyModule_AddIntConstant(poModule, "LIMIT_REAL_TIME",			CItemData::LIMIT_REAL_TIME);
	PyModule_AddIntConstant(poModule, "LIMIT_REAL_TIME_START_FIRST_USE",	CItemData::LIMIT_REAL_TIME_START_FIRST_USE);
	PyModule_AddIntConstant(poModule, "LIMIT_TIMER_BASED_ON_WEAR",	CItemData::LIMIT_TIMER_BASED_ON_WEAR);
	PyModule_AddIntConstant(poModule, "LIMIT_TYPE_MAX_NUM",			CItemData::LIMIT_MAX_NUM);
	PyModule_AddIntConstant(poModule, "LIMIT_MAX_NUM",				CItemData::ITEM_LIMIT_MAX_NUM);

	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_FEMALE",		CItemData::ITEM_ANTIFLAG_FEMALE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_MALE",			CItemData::ITEM_ANTIFLAG_MALE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_WARRIOR",		CItemData::ITEM_ANTIFLAG_WARRIOR);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_ASSASSIN",		CItemData::ITEM_ANTIFLAG_ASSASSIN);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SURA",			CItemData::ITEM_ANTIFLAG_SURA);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SHAMAN",		CItemData::ITEM_ANTIFLAG_SHAMAN);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SAFEBOX",		CItemData::ITEM_ANTIFLAG_SAFEBOX);
#ifdef ENABLE_WOLFMAN_CHARACTER
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_WOLFMAN",		CItemData::ITEM_ANTIFLAG_WOLFMAN);
#endif
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_GET",			CItemData::ITEM_ANTIFLAG_GET);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_DROP",			CItemData::ITEM_ANTIFLAG_DROP);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SELL",			CItemData::ITEM_ANTIFLAG_SELL);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_A",		CItemData::ITEM_ANTIFLAG_EMPIRE_A);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_B",		CItemData::ITEM_ANTIFLAG_EMPIRE_B);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_R",		CItemData::ITEM_ANTIFLAG_EMPIRE_R);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SAVE",			CItemData::ITEM_ANTIFLAG_SAVE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_GIVE",			CItemData::ITEM_ANTIFLAG_GIVE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_PKDROP",		CItemData::ITEM_ANTIFLAG_PKDROP);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_STACK",		CItemData::ITEM_ANTIFLAG_STACK);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_MYSHOP",		CItemData::ITEM_ANTIFLAG_MYSHOP);

	PyModule_AddIntConstant(poModule, "ITEM_FLAG_RARE",				CItemData::ITEM_FLAG_RARE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_UNIQUE",			CItemData::ITEM_FLAG_UNIQUE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_CONFIRM_WHEN_USE",	CItemData::ITEM_FLAG_CONFIRM_WHEN_USE);

	PyModule_AddIntConstant(poModule, "ANTIFLAG_FEMALE",			CItemData::ITEM_ANTIFLAG_FEMALE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_MALE",				CItemData::ITEM_ANTIFLAG_MALE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_WARRIOR",			CItemData::ITEM_ANTIFLAG_WARRIOR);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_ASSASSIN",			CItemData::ITEM_ANTIFLAG_ASSASSIN);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SURA",				CItemData::ITEM_ANTIFLAG_SURA);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SHAMAN",			CItemData::ITEM_ANTIFLAG_SHAMAN);
#ifdef ENABLE_WOLFMAN_CHARACTER
	PyModule_AddIntConstant(poModule, "ANTIFLAG_WOLFMAN",			CItemData::ITEM_ANTIFLAG_WOLFMAN);
#endif
	PyModule_AddIntConstant(poModule, "ANTIFLAG_GET",				CItemData::ITEM_ANTIFLAG_GET);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_DROP",				CItemData::ITEM_ANTIFLAG_DROP);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SELL",				CItemData::ITEM_ANTIFLAG_SELL);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_A",			CItemData::ITEM_ANTIFLAG_EMPIRE_A);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_B",			CItemData::ITEM_ANTIFLAG_EMPIRE_B);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_R",			CItemData::ITEM_ANTIFLAG_EMPIRE_R);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SAVE",				CItemData::ITEM_ANTIFLAG_SAVE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_GIVE",				CItemData::ITEM_ANTIFLAG_GIVE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_PKDROP",			CItemData::ITEM_ANTIFLAG_PKDROP);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_STACK",				CItemData::ITEM_ANTIFLAG_STACK);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_MYSHOP",			CItemData::ITEM_ANTIFLAG_MYSHOP);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SAFEBOX",			CItemData::ITEM_ANTIFLAG_SAFEBOX);

	PyModule_AddIntConstant(poModule, "WEARABLE_BODY",				CItemData::WEARABLE_BODY);
	PyModule_AddIntConstant(poModule, "WEARABLE_HEAD",				CItemData::WEARABLE_HEAD);
	PyModule_AddIntConstant(poModule, "WEARABLE_FOOTS",				CItemData::WEARABLE_FOOTS);
	PyModule_AddIntConstant(poModule, "WEARABLE_WRIST",				CItemData::WEARABLE_WRIST);
	PyModule_AddIntConstant(poModule, "WEARABLE_WEAPON",			CItemData::WEARABLE_WEAPON);
	PyModule_AddIntConstant(poModule, "WEARABLE_NECK",				CItemData::WEARABLE_NECK);
	PyModule_AddIntConstant(poModule, "WEARABLE_EAR",				CItemData::WEARABLE_EAR);
	PyModule_AddIntConstant(poModule, "WEARABLE_UNIQUE",			CItemData::WEARABLE_UNIQUE);
	PyModule_AddIntConstant(poModule, "WEARABLE_SHIELD",			CItemData::WEARABLE_SHIELD);
	PyModule_AddIntConstant(poModule, "WEARABLE_ARROW",				CItemData::WEARABLE_ARROW);
#ifdef ENABLE_PENDANT
	PyModule_AddIntConstant(poModule, "WEARABLE_PENDANT",			CItemData::WEARABLE_PENDANT);
#endif
#ifdef ENABLE_COSTUME_EFFECT
	PyModule_AddIntConstant(poModule, "WEARABLE_COSTUME_EFFECT_ARMOR",	CItemData::WEARABLE_COSTUME_EFFECT_ARMOR);
	PyModule_AddIntConstant(poModule, "WEARABLE_COSTUME_EFFECT_WEAPON",	CItemData::WEARABLE_COSTUME_EFFECT_WEAPON);
#endif

	PyModule_AddIntConstant(poModule, "ARMOR_BODY",					CItemData::ARMOR_BODY);
	PyModule_AddIntConstant(poModule, "ARMOR_HEAD",					CItemData::ARMOR_HEAD);
	PyModule_AddIntConstant(poModule, "ARMOR_SHIELD",				CItemData::ARMOR_SHIELD);
	PyModule_AddIntConstant(poModule, "ARMOR_WRIST",				CItemData::ARMOR_WRIST);
	PyModule_AddIntConstant(poModule, "ARMOR_FOOTS",				CItemData::ARMOR_FOOTS);
	PyModule_AddIntConstant(poModule, "ARMOR_NECK",					CItemData::ARMOR_NECK);
	PyModule_AddIntConstant(poModule, "ARMOR_EAR",					CItemData::ARMOR_EAR);
#ifdef ENABLE_PENDANT
	PyModule_AddIntConstant(poModule, "ARMOR_PENDANT",				CItemData::ARMOR_PENDANT);
#endif

	PyModule_AddIntConstant(poModule, "ITEM_APPLY_MAX_NUM",			CItemData::ITEM_APPLY_MAX_NUM);
	PyModule_AddIntConstant(poModule, "ITEM_SOCKET_MAX_NUM",		CItemData::ITEM_SOCKET_MAX_NUM);

	PyModule_AddIntConstant(poModule, "APPLY_NONE",					CItemData::APPLY_NONE);
	PyModule_AddIntConstant(poModule, "APPLY_STR",					CItemData::APPLY_STR);
	PyModule_AddIntConstant(poModule, "APPLY_DEX",					CItemData::APPLY_DEX);
	PyModule_AddIntConstant(poModule, "APPLY_CON",					CItemData::APPLY_CON);
	PyModule_AddIntConstant(poModule, "APPLY_INT",					CItemData::APPLY_INT);
	PyModule_AddIntConstant(poModule, "APPLY_MAX_HP",				CItemData::APPLY_MAX_HP);
	PyModule_AddIntConstant(poModule, "APPLY_MAX_SP",				CItemData::APPLY_MAX_SP);
	PyModule_AddIntConstant(poModule, "APPLY_HP_REGEN",				CItemData::APPLY_HP_REGEN);
	PyModule_AddIntConstant(poModule, "APPLY_SP_REGEN",				CItemData::APPLY_SP_REGEN);
	PyModule_AddIntConstant(poModule, "APPLY_DEF_GRADE_BONUS",		CItemData::APPLY_DEF_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_ATT_GRADE_BONUS",		CItemData::APPLY_ATT_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_ATT_SPEED",			CItemData::APPLY_ATT_SPEED);
	PyModule_AddIntConstant(poModule, "APPLY_MOV_SPEED",			CItemData::APPLY_MOV_SPEED);
	PyModule_AddIntConstant(poModule, "APPLY_CAST_SPEED",			CItemData::APPLY_CAST_SPEED);
	PyModule_AddIntConstant(poModule, "APPLY_MAGIC_ATT_GRADE",		CItemData::APPLY_MAGIC_ATT_GRADE);
	PyModule_AddIntConstant(poModule, "APPLY_MAGIC_DEF_GRADE",		CItemData::APPLY_MAGIC_DEF_GRADE);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL",				CItemData::APPLY_SKILL);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ANIMAL",		CItemData::APPLY_ATTBONUS_ANIMAL);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_UNDEAD",		CItemData::APPLY_ATTBONUS_UNDEAD);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DEVIL", 		CItemData::APPLY_ATTBONUS_DEVIL);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_HUMAN",		CItemData::APPLY_ATTBONUS_HUMAN);
    PyModule_AddIntConstant(poModule, "APPLY_BOW_DISTANCE", 		CItemData::APPLY_BOW_DISTANCE);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_BOW", 			CItemData::APPLY_RESIST_BOW);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_FIRE", 			CItemData::APPLY_RESIST_FIRE);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_ELEC", 			CItemData::APPLY_RESIST_ELEC);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_MAGIC", 		CItemData::APPLY_RESIST_MAGIC);
    PyModule_AddIntConstant(poModule, "APPLY_POISON_PCT",			CItemData::APPLY_POISON_PCT);
    PyModule_AddIntConstant(poModule, "APPLY_SLOW_PCT", 			CItemData::APPLY_SLOW_PCT);
    PyModule_AddIntConstant(poModule, "APPLY_STUN_PCT", 			CItemData::APPLY_STUN_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_CRITICAL_PCT",			CItemData::APPLY_CRITICAL_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_PENETRATE_PCT",		CItemData::APPLY_PENETRATE_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ORC",			CItemData::APPLY_ATTBONUS_ORC);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_MILGYO",		CItemData::APPLY_ATTBONUS_MILGYO);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_UNDEAD",		CItemData::APPLY_ATTBONUS_UNDEAD);
	PyModule_AddIntConstant(poModule, "APPLY_STEAL_HP",				CItemData::APPLY_STEAL_HP);
	PyModule_AddIntConstant(poModule, "APPLY_STEAL_SP",				CItemData::APPLY_STEAL_SP);
	PyModule_AddIntConstant(poModule, "APPLY_MANA_BURN_PCT",		CItemData::APPLY_MANA_BURN_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_DAMAGE_SP_RECOVER",	CItemData::APPLY_DAMAGE_SP_RECOVER);
	PyModule_AddIntConstant(poModule, "APPLY_BLOCK",				CItemData::APPLY_BLOCK);
	PyModule_AddIntConstant(poModule, "APPLY_DODGE",				CItemData::APPLY_DODGE);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_SWORD",			CItemData::APPLY_RESIST_SWORD);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_TWOHAND",		CItemData::APPLY_RESIST_TWOHAND);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_DAGGER",		CItemData::APPLY_RESIST_DAGGER);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_BELL",			CItemData::APPLY_RESIST_BELL);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_FAN",			CItemData::APPLY_RESIST_FAN);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_WIND",			CItemData::APPLY_RESIST_WIND);
	PyModule_AddIntConstant(poModule, "APPLY_REFLECT_MELEE",		CItemData::APPLY_REFLECT_MELEE);
	PyModule_AddIntConstant(poModule, "APPLY_REFLECT_CURSE",		CItemData::APPLY_REFLECT_CURSE);
	PyModule_AddIntConstant(poModule, "APPLY_POISON_REDUCE",		CItemData::APPLY_POISON_REDUCE);
	PyModule_AddIntConstant(poModule, "APPLY_KILL_SP_RECOVER",		CItemData::APPLY_KILL_SP_RECOVER);
	PyModule_AddIntConstant(poModule, "APPLY_EXP_DOUBLE_BONUS",		CItemData::APPLY_EXP_DOUBLE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_GOLD_DOUBLE_BONUS",	CItemData::APPLY_GOLD_DOUBLE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_ITEM_DROP_BONUS",		CItemData::APPLY_ITEM_DROP_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_POTION_BONUS",			CItemData::APPLY_POTION_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_KILL_HP_RECOVER",		CItemData::APPLY_KILL_HP_RECOVER);
	PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_STUN",			CItemData::APPLY_IMMUNE_STUN);
	PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_SLOW",			CItemData::APPLY_IMMUNE_SLOW);
	PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_FALL",			CItemData::APPLY_IMMUNE_FALL);
	PyModule_AddIntConstant(poModule, "APPLY_MAX_STAMINA",			CItemData::APPLY_MAX_STAMINA);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_WARRIOR",		CItemData::APPLY_ATT_BONUS_TO_WARRIOR);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ASSASSIN",	CItemData::APPLY_ATT_BONUS_TO_ASSASSIN);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_SURA",		CItemData::APPLY_ATT_BONUS_TO_SURA);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_SHAMAN",		CItemData::APPLY_ATT_BONUS_TO_SHAMAN);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_MONSTER",		CItemData::APPLY_ATT_BONUS_TO_MONSTER);
	PyModule_AddIntConstant(poModule, "APPLY_MALL_ATTBONUS",		CItemData::APPLY_MALL_ATTBONUS);
	PyModule_AddIntConstant(poModule, "APPLY_MALL_DEFBONUS",		CItemData::APPLY_MALL_DEFBONUS);
	PyModule_AddIntConstant(poModule, "APPLY_MALL_EXPBONUS",		CItemData::APPLY_MALL_EXPBONUS);
	PyModule_AddIntConstant(poModule, "APPLY_MALL_ITEMBONUS",		CItemData::APPLY_MALL_ITEMBONUS);
	PyModule_AddIntConstant(poModule, "APPLY_MALL_GOLDBONUS",		CItemData::APPLY_MALL_GOLDBONUS);
	PyModule_AddIntConstant(poModule, "APPLY_MAX_HP_PCT",			CItemData::APPLY_MAX_HP_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_MAX_SP_PCT",			CItemData::APPLY_MAX_SP_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_BONUS",		CItemData::APPLY_SKILL_DAMAGE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_NORMAL_HIT_DAMAGE_BONUS",	CItemData::APPLY_NORMAL_HIT_DAMAGE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DEFEND_BONUS",		CItemData::APPLY_SKILL_DEFEND_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_NORMAL_HIT_DEFEND_BONUS",	CItemData::APPLY_NORMAL_HIT_DEFEND_BONUS);

	PyModule_AddIntConstant(poModule, "APPLY_PC_BANG_EXP_BONUS",	CItemData::APPLY_PC_BANG_EXP_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_PC_BANG_DROP_BONUS",	CItemData::APPLY_PC_BANG_DROP_BONUS);

	PyModule_AddIntConstant(poModule, "APPLY_RESIST_WARRIOR",	CItemData::APPLY_RESIST_WARRIOR );
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_ASSASSIN",	CItemData::APPLY_RESIST_ASSASSIN );
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_SURA",		CItemData::APPLY_RESIST_SURA );
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_SHAMAN",	CItemData::APPLY_RESIST_SHAMAN );
	PyModule_AddIntConstant(poModule, "APPLY_ENERGY",	CItemData::APPLY_ENERGY );
	PyModule_AddIntConstant(poModule, "APPLY_COSTUME_ATTR_BONUS",	CItemData::APPLY_COSTUME_ATTR_BONUS );

	PyModule_AddIntConstant(poModule, "APPLY_MAGIC_ATTBONUS_PER",	CItemData::APPLY_MAGIC_ATTBONUS_PER );
	PyModule_AddIntConstant(poModule, "APPLY_MELEE_MAGIC_ATTBONUS_PER",	CItemData::APPLY_MELEE_MAGIC_ATTBONUS_PER );
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_ICE",	CItemData::APPLY_RESIST_ICE );
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_EARTH",	CItemData::APPLY_RESIST_EARTH );
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_DARK",	CItemData::APPLY_RESIST_DARK );
	PyModule_AddIntConstant(poModule, "APPLY_ANTI_CRITICAL_PCT",	CItemData::APPLY_ANTI_CRITICAL_PCT );
	PyModule_AddIntConstant(poModule, "APPLY_ANTI_PENETRATE_PCT",	CItemData::APPLY_ANTI_PENETRATE_PCT );
#ifdef ENABLE_WOLFMAN_CHARACTER
	PyModule_AddIntConstant(poModule, "APPLY_BLEEDING_PCT",	CItemData::APPLY_BLEEDING_PCT );
	PyModule_AddIntConstant(poModule, "APPLY_BLEEDING_REDUCE",	CItemData::APPLY_BLEEDING_REDUCE );
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_WOLFMAN",	CItemData::APPLY_ATT_BONUS_TO_WOLFMAN );
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_WOLFMAN",	CItemData::APPLY_RESIST_WOLFMAN );
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_CLAW",	CItemData::APPLY_RESIST_CLAW );
#endif

	PyModule_AddIntConstant(poModule, "APPLY_ACCEDRAIN_RATE",	CItemData::APPLY_ACCEDRAIN_RATE );
#ifdef ENABLE_ANTI_RESIST_MAGIC_BONUS_SYSTEM
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_MAGIC_REDUCTION",	CItemData::APPLY_RESIST_MAGIC_REDUCTION );
#endif

#ifdef ENABLE_ELEMENT_ADD
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_ELECT",	CItemData::APPLY_ENCHANT_ELECT );
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_FIRE",	CItemData::APPLY_ENCHANT_FIRE );
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_ICE",	CItemData::APPLY_ENCHANT_ICE );
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_WIND",	CItemData::APPLY_ENCHANT_WIND );
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_EARTH",	CItemData::APPLY_ENCHANT_EARTH );
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_DARK",	CItemData::APPLY_ENCHANT_DARK );
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_CZ",	CItemData::APPLY_ATTBONUS_CZ );
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_INSECT",	CItemData::APPLY_ATTBONUS_INSECT );
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DESERT",	CItemData::APPLY_ATTBONUS_DESERT );
#endif

#ifdef ENABLE_PENDANT
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_SWORD",	CItemData::APPLY_ATTBONUS_SWORD );
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_TWOHAND",	CItemData::APPLY_ATTBONUS_TWOHAND );
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DAGGER",	CItemData::APPLY_ATTBONUS_DAGGER );
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_BELL",	CItemData::APPLY_ATTBONUS_BELL );
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_FAN",	CItemData::APPLY_ATTBONUS_FAN );
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_BOW",	CItemData::APPLY_ATTBONUS_BOW );
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_CLAW",	CItemData::APPLY_ATTBONUS_CLAW );
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_HUMAN",	CItemData::APPLY_RESIST_HUMAN );
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_MOUNT_FALL",	CItemData::APPLY_RESIST_MOUNT_FALL );
#endif

	PyModule_AddIntConstant(poModule, "APPLY_MOUNT",	CItemData::APPLY_MOUNT );
	PyModule_AddIntConstant(poModule, "APPLY_NONAME",	CItemData::APPLY_NONAME );
#ifdef ENABLE_SOULBIND_SYSTEM
	PyModule_AddIntConstant(poModule, "E_SEAL_DATE_DEFAULT_TIMESTAMP",		CItemData::SEAL_DATE_DEFAULT_TIMESTAMP);
	PyModule_AddIntConstant(poModule, "E_SEAL_DATE_UNLIMITED_TIMESTAMP",	CItemData::SEAL_DATE_UNLIMITED_TIMESTAMP);
#endif

	PyModule_AddIntConstant(poModule, "PET_EGG",	CItemData::PET_EGG );
	PyModule_AddIntConstant(poModule, "PET_UPBRINGING",	CItemData::PET_UPBRINGING );
	PyModule_AddIntConstant(poModule, "PET_BAG",	CItemData::PET_BAG );
	PyModule_AddIntConstant(poModule, "PET_FEEDSTUFF",	CItemData::PET_FEEDSTUFF );
	PyModule_AddIntConstant(poModule, "PET_SKILL",	CItemData::PET_SKILL );
	PyModule_AddIntConstant(poModule, "PET_SKILL_DEL_BOOK",	CItemData::PET_SKILL_DEL_BOOK );
	PyModule_AddIntConstant(poModule, "PET_NAME_CHANGE",	CItemData::PET_NAME_CHANGE );
	PyModule_AddIntConstant(poModule, "PET_EXPFOOD",	CItemData::PET_EXPFOOD );
	PyModule_AddIntConstant(poModule, "PET_SKILL_ALL_DEL_BOOK",	CItemData::PET_SKILL_ALL_DEL_BOOK );
	PyModule_AddIntConstant(poModule, "PET_EXPFOOD_PER",	CItemData::PET_EXPFOOD_PER );
	PyModule_AddIntConstant(poModule, "PET_ATTR_CHANGE",	CItemData::PET_ATTR_CHANGE );
	PyModule_AddIntConstant(poModule, "PET_ATTR_CHANGE_ITEMVNUM",	CItemData::PET_ATTR_CHANGE_ITEMVNUM );
	PyModule_AddIntConstant(poModule, "PET_PAY",	CItemData::PET_PAY );
}

