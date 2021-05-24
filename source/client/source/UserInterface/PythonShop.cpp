#include "stdafx.h"
#include "PythonShop.h"
#include "GameType.h"
#include "PythonNetworkStream.h"

void CPythonShop::SetItemData(DWORD dwIndex, const TShopItemData & c_rShopItemData)
{
	DWORD dwSlotPos = dwIndex % SHOP_MAX_NUM;
	items[dwSlotPos] = c_rShopItemData;
}

BOOL CPythonShop::GetItemData(DWORD dwIndex, const TShopItemData ** c_ppItemData)
{
	DWORD dwSlotPos = dwIndex % SHOP_MAX_NUM;
	*c_ppItemData = &items[dwSlotPos];

	return true;
}

#ifdef ENABLE_SHOP_SELL_INFO
void CPythonShop::SetInfo(DWORD dwIndex, const TShopSellInfo & c_rShopSellInfo)
{
	DWORD dwSlotPos = dwIndex % SHOP_MAX_NUM;
	infos[dwSlotPos] = c_rShopSellInfo;
}

BOOL CPythonShop::GetInfo(DWORD dwIndex, const TShopSellInfo ** c_ppInfo)
{
	DWORD dwSlotPos = dwIndex % SHOP_MAX_NUM;
	*c_ppInfo = &infos[dwSlotPos];

	return true;
}
#endif

void CPythonShop::ClearPrivateShopStock()
{
	m_PrivateShopItemStock.clear();
}

void CPythonShop::AddPrivateShopItemStock(TItemPos ItemPos, BYTE dwDisplayPos, DWORD dwPrices[])
{
	DelPrivateShopItemStock(ItemPos);

	TShopItemTable SellingItem;
	SellingItem.vnum = 0;
	SellingItem.count = 0;
	SellingItem.pos = ItemPos;
	memcpy(SellingItem.dwPrices, dwPrices, sizeof(SellingItem.dwPrices));

	SellingItem.display_pos = dwDisplayPos;
	m_PrivateShopItemStock.insert(make_pair(ItemPos, SellingItem));
}

void CPythonShop::DelPrivateShopItemStock(TItemPos ItemPos)
{
	if (m_PrivateShopItemStock.end() == m_PrivateShopItemStock.find(ItemPos))
		return;

	m_PrivateShopItemStock.erase(ItemPos);
}

int CPythonShop::GetPrivateShopItemPrice(TItemPos ItemPos, BYTE bPriceType)
{
	TPrivateShopItemStock::iterator itor = m_PrivateShopItemStock.find(ItemPos);

	if (m_PrivateShopItemStock.end() == itor)
		return 0;

	TShopItemTable & rShopItemTable = itor->second;
	return rShopItemTable.dwPrices[bPriceType];
}

struct ItemStockSortFunc
{
	bool operator() (TShopItemTable & rkLeft, TShopItemTable & rkRight)
	{
		return rkLeft.display_pos < rkRight.display_pos;
	}
};

void CPythonShop::BuildPrivateShop(const char * c_szName
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
, int iTime
#endif
)
{
	std::vector<TShopItemTable> ItemStock;
	ItemStock.reserve(m_PrivateShopItemStock.size());

	TPrivateShopItemStock::iterator itor = m_PrivateShopItemStock.begin();
	for (; itor != m_PrivateShopItemStock.end(); ++itor)
	{
		ItemStock.push_back(itor->second);
	}

	std::sort(ItemStock.begin(), ItemStock.end(), ItemStockSortFunc());

	CPythonNetworkStream::Instance().SendBuildPrivateShopPacket(c_szName, ItemStock
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
, iTime
#endif
);
}

void CPythonShop::Open(BOOL isPrivateShop, BOOL isMainPrivateShop)
{
	m_isShoping = TRUE;
	m_isPrivateShop = isPrivateShop;
	m_isMainPlayerPrivateShop = isMainPrivateShop;
}

void CPythonShop::Close()
{
	m_isShoping = FALSE;
	m_isPrivateShop = FALSE;
	m_isMainPlayerPrivateShop = FALSE;
}

BOOL CPythonShop::IsOpen()
{
	return m_isShoping;
}

BOOL CPythonShop::IsPrivateShop()
{
	return m_isPrivateShop;
}

BOOL CPythonShop::IsMainPlayerPrivateShop()
{
	return m_isMainPlayerPrivateShop;
}

void CPythonShop::Clear()
{
	m_isShoping = FALSE;
	m_isPrivateShop = FALSE;
	m_isMainPlayerPrivateShop = FALSE;
	ClearPrivateShopStock();
}

CPythonShop::CPythonShop(void)
{
	Clear();
}

CPythonShop::~CPythonShop(void)
{
}

PyObject * shopOpen(PyObject * poSelf, PyObject * poArgs)
{
	int isPrivateShop = false;
	PyTuple_GetInteger(poArgs, 0, &isPrivateShop);
	int isMainPrivateShop = false;
	PyTuple_GetInteger(poArgs, 1, &isMainPrivateShop);

	CPythonShop& rkShop=CPythonShop::Instance();
	rkShop.Open(isPrivateShop, isMainPrivateShop);
	return Py_BuildNone();
}

PyObject * shopClose(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	rkShop.Close();
	return Py_BuildNone();
}

PyObject * shopIsOpen(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsOpen());
}

PyObject * shopIsPrviateShop(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsPrivateShop());
}

PyObject * shopIsMainPlayerPrivateShop(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsMainPlayerPrivateShop());
}

PyObject * shopGetItemID(PyObject * poSelf, PyObject * poArgs)
{
	int nPos;
	if (!PyTuple_GetInteger(poArgs, 0, &nPos))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(nPos, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->vnum);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemCount(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->count);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	int bPriceType;
	if (!PyTuple_GetInteger(poArgs, 1, &bPriceType))
		return Py_BuildException();

	if (bPriceType >= SHOP_PRICE_MAX_NUM)
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->dwPrices[bPriceType]);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemMetinSocket(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	int iMetinSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->alSockets[iMetinSocketIndex]);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemAttribute(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		const TShopItemData * c_pItemData;
		if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
			return Py_BuildValue("ii", c_pItemData->aAttr[iAttrSlotIndex].bType, c_pItemData->aAttr[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject * shopClearPrivateShopStock(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop::Instance().ClearPrivateShopStock();
	return Py_BuildNone();
}

PyObject * shopAddPrivateShopItemStock(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	WORD wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();
	int iDisplaySlotIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iDisplaySlotIndex))
		return Py_BuildException();
	DWORD dwPrices[SHOP_PRICE_MAX_NUM];
	memset(dwPrices, 0, sizeof(dwPrices));
	int a, b;
	if (!PyTuple_GetInteger(poArgs, 3, &a))
		return Py_BuildException();
	dwPrices[SHOP_PRICE_GOLD] = a;
#ifdef ENABLE_CHEQUE_SYSTEM
	if (!PyTuple_GetInteger(poArgs, 4, &b))
		return Py_BuildException();
	dwPrices[SHOP_PRICE_CHEQUE] = b;
#endif

	CPythonShop::Instance().AddPrivateShopItemStock(TItemPos(bItemWindowType, wItemSlotIndex), iDisplaySlotIndex, dwPrices);
	return Py_BuildNone();
}

PyObject * shopDelPrivateShopItemStock(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	WORD wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();

	CPythonShop::Instance().DelPrivateShopItemStock(TItemPos(bItemWindowType, wItemSlotIndex));
	return Py_BuildNone();
}

PyObject * shopGetPrivateShopItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	WORD wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();
	int bPriceType;
	if (!PyTuple_GetInteger(poArgs, 2, &bPriceType))
		return Py_BuildException();

	if (bPriceType >= SHOP_PRICE_MAX_NUM)
		return Py_BuildException();

	int iValue = CPythonShop::Instance().GetPrivateShopItemPrice(TItemPos(bItemWindowType, wItemSlotIndex), bPriceType);
	return Py_BuildValue("i", iValue);
}

PyObject * shopBuildPrivateShop(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
	int iTime;
	if (!PyTuple_GetInteger(poArgs, 1, &iTime))
		return Py_BuildException();

	CPythonShop::Instance().BuildPrivateShop(szName, iTime);
#else
	CPythonShop::Instance().BuildPrivateShop(szName);
#endif

	return Py_BuildNone();
}

#ifdef ENABLE_BATTLE_FIELD
PyObject * shopGetUsablePoint(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonShop::Instance().GetUsablePoint());
}
PyObject * shopGetLimitMaxPoint(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonShop::Instance().GetLimitMaxPoint());
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject * shopGetItemTransmutation(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	
	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->transmutation);
	
	return Py_BuildValue("i", 0);
}
#endif

#ifdef ENABLE_SHOP_SELL_INFO
PyObject * shopGetSellTime(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopSellInfo * c_pInfo;
	if (CPythonShop::Instance().GetInfo(iIndex, &c_pInfo))
		return Py_BuildValue("i", c_pInfo->lSellTime);
	
	return Py_BuildValue("i", 0);
}

PyObject * shopGetSellName(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopSellInfo * c_pInfo;
	if (CPythonShop::Instance().GetInfo(iIndex, &c_pInfo))
		return Py_BuildValue("s", c_pInfo->szName);
	
	return Py_BuildValue("i", 0);
}

PyObject * shopIsSellItem(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopSellInfo * c_pInfo;
	if (CPythonShop::Instance().GetInfo(iIndex, &c_pInfo))
		return Py_BuildValue("i", c_pInfo->bIsSell);
	
	return Py_BuildValue("i", 0);
}
#endif

void initshop()
{
	static PyMethodDef s_methods[] =
	{

		{ "Open",						shopOpen,						METH_VARARGS },
		{ "Close",						shopClose,						METH_VARARGS },
		{ "IsOpen",						shopIsOpen,						METH_VARARGS },
		{ "IsPrivateShop",				shopIsPrviateShop,				METH_VARARGS },
		{ "IsMainPlayerPrivateShop",	shopIsMainPlayerPrivateShop,	METH_VARARGS },
		{ "GetItemID",					shopGetItemID,					METH_VARARGS },
		{ "GetItemCount",				shopGetItemCount,				METH_VARARGS },
		{ "GetItemPrice",				shopGetItemPrice,				METH_VARARGS },
		{ "GetItemMetinSocket",			shopGetItemMetinSocket,			METH_VARARGS },
		{ "GetItemAttribute",			shopGetItemAttribute,			METH_VARARGS },
		{ "ClearPrivateShopStock",		shopClearPrivateShopStock,		METH_VARARGS },
		{ "AddPrivateShopItemStock",	shopAddPrivateShopItemStock,	METH_VARARGS },
		{ "DelPrivateShopItemStock",	shopDelPrivateShopItemStock,	METH_VARARGS },
		{ "GetPrivateShopItemPrice",	shopGetPrivateShopItemPrice,	METH_VARARGS },
		{ "BuildPrivateShop",			shopBuildPrivateShop,			METH_VARARGS },
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		{ "GetItemTransmutation", 		shopGetItemTransmutation, 		METH_VARARGS },
#endif
#ifdef ENABLE_BATTLE_FIELD		
		{ "GetUsablePoint",				shopGetUsablePoint,				METH_VARARGS },
		{ "GetLimitMaxPoint",			shopGetLimitMaxPoint,			METH_VARARGS },
#endif

#ifdef ENABLE_SHOP_SELL_INFO
		{ "GetSellTime",				shopGetSellTime,				METH_VARARGS },
		{ "GetSellName",				shopGetSellName,				METH_VARARGS },
		{ "IsSellItem",					shopIsSellItem,					METH_VARARGS },
#endif
		{ NULL,							NULL,							NULL },
	};
	PyObject * poModule = Py_InitModule("shop", s_methods);

	PyModule_AddIntConstant(poModule, "SHOP_PAGE_COLUMN", SHOP_PAGE_COLUMN);
	PyModule_AddIntConstant(poModule, "SHOP_PAGE_ROW", SHOP_PAGE_ROW);
	PyModule_AddIntConstant(poModule, "SHOP_PAGE_SIZE", SHOP_PAGE_SIZE);
	PyModule_AddIntConstant(poModule, "SHOP_PAGE_COUNT", SHOP_PAGE_COUNT);
	PyModule_AddIntConstant(poModule, "SHOP_MAX_NUM", SHOP_MAX_NUM);
	PyModule_AddIntConstant(poModule, "SHOP_PRICE_GOLD", SHOP_PRICE_GOLD);
#ifdef ENABLE_CHEQUE_SYSTEM
	PyModule_AddIntConstant(poModule, "SHOP_PRICE_CHEQUE", SHOP_PRICE_CHEQUE);
#endif
#ifdef ENABLE_BATTLE_FIELD
	PyModule_AddIntConstant(poModule, "SHOP_PRICE_BATTLE_POINT", SHOP_PRICE_BATTLE_POINT);
#endif
	PyModule_AddIntConstant(poModule, "SHOP_PRICE_MAX_NUM", SHOP_PRICE_MAX_NUM);
}

