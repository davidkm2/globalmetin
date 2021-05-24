#pragma once
#include "Packet.h"

class CPythonShop : public CSingleton<CPythonShop>
{
	public:
		CPythonShop(void);
		virtual ~CPythonShop(void);

		void Clear();

		void SetItemData(DWORD dwIndex, const TShopItemData & c_rShopItemData);
		BOOL GetItemData(DWORD dwIndex, const TShopItemData ** c_ppItemData);
#ifdef ENABLE_SHOP_SELL_INFO
		void SetInfo(DWORD dwIndex, const TShopSellInfo & c_rShopSellInfo);
		BOOL GetInfo(DWORD dwIndex, const TShopSellInfo ** c_ppInfo);
#endif

		void Open(BOOL isPrivateShop, BOOL isMainPrivateShop);
		void Close();
		BOOL IsOpen();
		BOOL IsPrivateShop();
		BOOL IsMainPlayerPrivateShop();

		void ClearPrivateShopStock();
		void AddPrivateShopItemStock(TItemPos ItemPos, BYTE byDisplayPos, DWORD dwPrices[]);

		void DelPrivateShopItemStock(TItemPos ItemPos);
		int GetPrivateShopItemPrice(TItemPos ItemPos, BYTE bPriceType);
		void BuildPrivateShop(const char * c_szName
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
, int iTime
#endif
);
#ifdef ENABLE_BATTLE_FIELD
		void SetLimitMaxPoint(int iMaxPoints) { limitMaxPoints = iMaxPoints; }
		void SetUsablePoint(int iUsablePoints) { usablePoints = iUsablePoints; }
		int GetLimitMaxPoint() { return limitMaxPoints; }
		int GetUsablePoint() { return usablePoints; }
#endif
	protected:
		BOOL	CheckSlotIndex(DWORD dwIndex);

	protected:
		BOOL				m_isShoping;
		BOOL				m_isPrivateShop;
		BOOL				m_isMainPlayerPrivateShop;
#ifdef ENABLE_BATTLE_FIELD
		int 				usablePoints;
		int 				limitMaxPoints;
#endif

		TShopItemData		items[SHOP_MAX_NUM];
#ifdef ENABLE_SHOP_SELL_INFO
		TShopSellInfo		infos[SHOP_MAX_NUM];
#endif

		typedef std::map<TItemPos, TShopItemTable> TPrivateShopItemStock;
		TPrivateShopItemStock	m_PrivateShopItemStock;
};

