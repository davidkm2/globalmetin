#include "StdAfx.h"
#include "PythonOfflinePrivateShop.h"

CPythonOffflinePrivateShop::CPythonOffflinePrivateShop(void)
{
	Clear();
}

CPythonOffflinePrivateShop::~CPythonOffflinePrivateShop(void)
{
	Clear();
}

void CPythonOffflinePrivateShop::Clear(void)
{
	m_OffflinePrivateShopData.clear();
}

const TShopItemData * CPythonOffflinePrivateShop::GetOffflinePrivateShopItemData(int id, DWORD dwSlotId) const
{
	return &(m_OffflinePrivateShopData[id].items[dwSlotId]);
}

DWORD CPythonOffflinePrivateShop::GetItemMetinSocket(int id, DWORD dwSlotId, int dwMetinSocketIndex)
{
	if (dwMetinSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return 0;

	return GetOffflinePrivateShopItemData(id, dwSlotId)->alSockets[dwMetinSocketIndex];
}

void CPythonOffflinePrivateShop::GetItemAttribute(int id, DWORD dwSlotId, int dwAttrSlotIndex, BYTE * pbyType, int * psValue)
{
	*pbyType = 0;
	*psValue = 0;

	if (dwAttrSlotIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM)
		return;

	*pbyType = GetOffflinePrivateShopItemData(id, dwSlotId)->aAttr[dwAttrSlotIndex].bType;
	*psValue = GetOffflinePrivateShopItemData(id, dwSlotId)->aAttr[dwAttrSlotIndex].sValue;
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
DWORD CPythonOffflinePrivateShop::GetItemChangeLookVnum(int id, DWORD dwSlotId)
{
	return GetOffflinePrivateShopItemData(id, dwSlotId)->transmutation;
}
#endif

void CPythonOffflinePrivateShop::AppendInfo(TOfflinePrivateShopInfo Info)
{
	m_OffflinePrivateShopData.push_back(Info);
}

const TOfflinePrivateShopInfo * CPythonOffflinePrivateShop::GetInfo(int id) const
{
	return &m_OffflinePrivateShopData[id];
}

#ifdef ENABLE_SHOP_SELL_INFO
const TShopSellInfo * CPythonOffflinePrivateShop::GetOffflinePrivateShopSellData(int id, DWORD dwSlotId) const
{
	return &(m_OffflinePrivateShopData[id].infos[dwSlotId]);
}
#endif

PyObject * opsGetItemAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid, iAttributeSlotIndex, sValue;
	BYTE byType;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &slotid))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &iAttributeSlotIndex))
		return Py_BuildException();

	CPythonOffflinePrivateShop::Instance().GetItemAttribute(id, slotid, iAttributeSlotIndex, &byType, &sValue);

	return Py_BuildValue("ii", byType, sValue);
}

PyObject * opsGetItemMetinSocket(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid, iMetinSocketIndex;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &slotid))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &iMetinSocketIndex))
		return Py_BuildException();

	int nMetinSocketValue = CPythonOffflinePrivateShop::Instance().GetItemMetinSocket(id, slotid, iMetinSocketIndex);
	return Py_BuildValue("i", nMetinSocketValue);
}

PyObject * opsGetItemVnum(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &slotid))
		return Py_BuildException();

	DWORD dwVnum = CPythonOffflinePrivateShop::Instance().GetOffflinePrivateShopItemData(id, slotid)->vnum;
	return Py_BuildValue("i", dwVnum);
}

PyObject * opsGetItemCount(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &slotid))
		return Py_BuildException();

	DWORD dwCount = CPythonOffflinePrivateShop::Instance().GetOffflinePrivateShopItemData(id, slotid)->count;
	return Py_BuildValue("i", dwCount);
}

PyObject * opsGetItemPrice(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid, price;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &slotid))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &price))
		return Py_BuildException();

	if (price >= SHOP_PRICE_MAX_NUM)
		return Py_BuildValue("i", 0);

	DWORD dwPrice = CPythonOffflinePrivateShop::Instance().GetOffflinePrivateShopItemData(id, slotid)->dwPrices[price];
	return Py_BuildValue("i", dwPrice);
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject * opsGetItemChangeLookVnum(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &slotid))
		return Py_BuildException();

	int iChangeLook = CPythonOffflinePrivateShop::Instance().GetItemChangeLookVnum(id, slotid);
	return Py_BuildValue("i", iChangeLook);
}
#endif

#ifdef ENABLE_SHOP_SELL_INFO
PyObject * opsIsSellItem(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &slotid))
		return Py_BuildException();

	const TShopSellInfo * c_pInfo = CPythonOffflinePrivateShop::Instance().GetOffflinePrivateShopSellData(id, slotid);
	return Py_BuildValue("i", c_pInfo->bIsSell);
}

PyObject * opsGetSellTime(PyObject * poSelf, PyObject * poArgs)
{
	int id, slotid;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &slotid))
		return Py_BuildException();

	const TShopSellInfo * c_pInfo = CPythonOffflinePrivateShop::Instance().GetOffflinePrivateShopSellData(id, slotid);
	return Py_BuildValue("i", c_pInfo->lSellTime);
}

PyObject * opsGetSellName(PyObject * poSelf, PyObject * poArgs)
{
	int id, slotid;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &slotid))
		return Py_BuildException();

	const TShopSellInfo * c_pInfo = CPythonOffflinePrivateShop::Instance().GetOffflinePrivateShopSellData(id, slotid);
	return Py_BuildValue("s", c_pInfo->szName);
}
#endif

PyObject * opsGetPosition(PyObject* poSelf, PyObject* poArgs)
{
	int id;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	const TOfflinePrivateShopInfo * Info = CPythonOffflinePrivateShop::Instance().GetInfo(id);
	return Py_BuildValue("iii", Info->dwX, Info->dwY, Info->dwMapIndex);
}

PyObject * opsGetSign(PyObject* poSelf, PyObject* poArgs)
{
	int id;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	const TOfflinePrivateShopInfo * Info = CPythonOffflinePrivateShop::Instance().GetInfo(id);
	return Py_BuildValue("s", Info->szSign);
}

PyObject * opsGetTime(PyObject* poSelf, PyObject* poArgs)
{
	int id;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	const TOfflinePrivateShopInfo * Info = CPythonOffflinePrivateShop::Instance().GetInfo(id);
	return Py_BuildValue("ii", Info->dwStartTime, Info->dwEndTime);
}

PyObject * opsGetVid(PyObject* poSelf, PyObject* poArgs)
{
	int id;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	const TOfflinePrivateShopInfo * Info = CPythonOffflinePrivateShop::Instance().GetInfo(id);
	return Py_BuildValue("i", Info->dwVid);
}

PyObject * opsGetPrice(PyObject* poSelf, PyObject* poArgs)
{
	int id, price;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &price))
		return Py_BuildException();

	const TOfflinePrivateShopInfo * Info = CPythonOffflinePrivateShop::Instance().GetInfo(id);
	return Py_BuildValue("i", Info->dwPrices[price]);
}

PyObject * opsGetCount(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonOffflinePrivateShop::Instance().GetCount());
}

void initOfflinePrivateShop()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetItemAttribute",		opsGetItemAttribute,		METH_VARARGS },
		{ "GetItemMetinSocket",		opsGetItemMetinSocket,		METH_VARARGS },
		{ "GetItemVnum",			opsGetItemVnum,				METH_VARARGS },
		{ "GetItemCount",			opsGetItemCount,			METH_VARARGS },
		{ "GetItemPrice",			opsGetItemPrice,			METH_VARARGS },
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		{ "GetItemChangeLookVnum",	opsGetItemChangeLookVnum,	METH_VARARGS },
#endif
#ifdef ENABLE_SHOP_SELL_INFO
		{ "GetSellTime",			opsGetSellTime,				METH_VARARGS },
		{ "GetSellName",			opsGetSellName,				METH_VARARGS },
		{ "IsSellItem",				opsIsSellItem,				METH_VARARGS },
#endif
		{ "GetPosition",			opsGetPosition,				METH_VARARGS },
		{ "GetSign",				opsGetSign,					METH_VARARGS },
		{ "GetTime",				opsGetTime,					METH_VARARGS },
		{ "GetVid",					opsGetVid,					METH_VARARGS },
		{ "GetPrice",				opsGetPrice,				METH_VARARGS },
		{ "GetCount",				opsGetCount,				METH_VARARGS },
		{ NULL,						NULL,						NULL },
	};

	PyObject * poModule = Py_InitModule("ops", s_methods);
}

