#pragma once
#include "Packet.h"

class CPythonOffflinePrivateShop : public CSingleton<CPythonOffflinePrivateShop>
{
	public:
		CPythonOffflinePrivateShop(void);
		virtual ~CPythonOffflinePrivateShop(void);

		void Clear(void);
		typedef std::vector<TOfflinePrivateShopInfo> TOffflinePrivateShop;

		void AppendInfo(TOfflinePrivateShopInfo Info);
		const TOfflinePrivateShopInfo * GetInfo(int id) const;
		const TShopItemData * GetOffflinePrivateShopItemData(int id, DWORD dwSlotId) const;
		DWORD GetItemMetinSocket(int id, DWORD dwSlotId, int dwMetinSocketIndex);
		void GetItemAttribute(int id, DWORD dwSlotId, int dwAttrSlotIndex, BYTE * pbyType, int * psValue);
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		DWORD GetItemChangeLookVnum(int id, DWORD dwSlotId);
#endif
#ifdef ENABLE_SHOP_SELL_INFO
		const TShopSellInfo * GetOffflinePrivateShopSellData(int id, DWORD dwSlotId) const;
#endif
		size_t GetCount() { return m_OffflinePrivateShopData.size(); }

	protected:
		TOffflinePrivateShop m_OffflinePrivateShopData;
};

