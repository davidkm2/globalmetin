#pragma once
#include "vid.h"
#include "entity.h"
#include "sectree_manager.h"
#include "packet.h"
#include "../../common/tables.h"

class COfflinePrivateShop : public CEntity
{
	public:
		COfflinePrivateShop(DWORD dwRaceVnum, DWORD dwTime, LPSHOP pkShop);
		virtual ~COfflinePrivateShop(void);

		void Initialize(void);
		void Destroy(void);

		DWORD GetRaceVnum() { return m_dwRaceVnum; }
		DWORD GetVid() { return m_dwVid; }
		DWORD GetTime() { return m_dwEndTime; }
		DWORD GetShopVid() { return m_dwShopVid; }
		DWORD GetPlayerID() { return m_dwPlayerID; }
		LPSHOP GetShop() { return m_pkShop; }
#ifdef __MYSHOP_DECO__
		BYTE GetDeco() { return m_bDeco; }
		void SetDeco(BYTE bDeco) { m_bDeco = bDeco; }
#endif
		void SetShopVid(DWORD dwShopVid) { m_dwShopVid = dwShopVid; }
		void SetSign(const std::string& strSign) { m_strSign = strSign; }
		void SetOwnerName(const std::string& strOwnerName) { m_strOwnerName = strOwnerName; }
		void SetPlayerID(DWORD dwPlayerID) { m_dwPlayerID = dwPlayerID; }

		void SetPrice(DWORD dwPrice, BYTE bIndex) { m_dwPrices[bIndex] = dwPrice; }
		void SetPrices(DWORD dwPrice[]) { memcpy(m_dwPrices, dwPrice, sizeof(m_dwPrices)); }
		const DWORD* GetPrices() { return &m_dwPrices[0]; }
		const DWORD* AddPrices(DWORD dwPrice[]);
		void GetInfo(TOfflinePrivateShopInfo * pkTOfflinePrivateShopInfo);

	protected:
		virtual void EncodeInsertPacket(LPENTITY ent);
		virtual void EncodeRemovePacket(LPENTITY ent);

		DWORD m_dwRaceVnum;
		DWORD m_dwVid;
		DWORD m_dwEndTime;
		DWORD m_dwStartTime;
		DWORD m_dwShopVid;
		DWORD m_dwPlayerID;
		LPSHOP m_pkShop;
		std::string m_strSign;
		std::string m_strOwnerName;
#ifdef __MYSHOP_DECO__
		BYTE m_bDeco;
#endif
		DWORD m_dwPrices[SHOP_PRICE_MAX_NUM];
};

