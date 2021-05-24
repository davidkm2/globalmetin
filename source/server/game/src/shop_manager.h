#ifndef __INC_METIN_II_GAME_SHOP_MANAGER_H__
#define __INC_METIN_II_GAME_SHOP_MANAGER_H__
#include "../../common/defines.h"

class CShopManager : public singleton<CShopManager>
{
	public:
		typedef std::map<DWORD, CShop *> TShopMap;
#ifdef __OFFLINE_PRIVATE_SHOP__
		typedef std::map<DWORD, COfflinePrivateShop*> TOfflinePrivateShopMap;
#endif

	public:
		CShopManager();
		virtual ~CShopManager();

		bool	Initialize(TShopTable * table, int size);
		void	Destroy();

		LPSHOP	Get(DWORD dwVnum);
		LPSHOP	GetByNPCVnum(DWORD dwVnum);

		bool	StartShopping(LPCHARACTER pkChr, LPCHARACTER pkShopKeeper, int iShopVnum = 0);
		void	StopShopping(LPCHARACTER ch);

		void	Buy(LPCHARACTER ch, BYTE pos);
#ifdef __NEW_STORAGE__
		void	Sell(LPCHARACTER ch, BYTE bCell, BYTE bCount = 0, BYTE bType = 0);
#else
		void	Sell(LPCHARACTER ch, BYTE bCell, BYTE bCount = 0);
#endif

		LPSHOP	CreatePCShop(LPCHARACTER ch, TShopItemTable * pTable, BYTE bItemCount);
		LPSHOP	FindPCShop(DWORD dwVID);
		void	DestroyPCShop(LPCHARACTER ch);

	private:
		TShopMap	m_map_pkShop;
		TShopMap	m_map_pkShopByNPCVnum;
		TShopMap	m_map_pkShopByPC;

		bool	ReadShopTableEx(const char* stFileName);

#ifdef __GEM__
	public:
		bool	InitializeGemShop(TGemShopTable * table, int size);
		DWORD 	GemShopGetRandomId(DWORD dwRow);
		bool 	GemShopGetInfoById(DWORD itemId, DWORD &dwVnum, BYTE &bCount, DWORD &dwPrice);

	private:
		DWORD						m_dwGemShopTableSize;
		TGemShopTable *				m_pGemShopTable;
#endif

#ifdef __OFFLINE_PRIVATE_SHOP__
	public:
		void CreateOfflinePrivateShop(LPCHARACTER ch, const char* szSign, DWORD dwRace, DWORD dwTime, TShopItemTable * pTable, BYTE bItemCount
#ifdef __MYSHOP_DECO__
, TMyshopDeco * Deco
#endif
);
		LPOPSHOP FindOfflinePrivateShop(DWORD dwVid);
		void DestroyOfflinePrivateShop(DWORD dwVid);
		void ProcessOfflinePrivateShop();
		void BuildOfflinePrivateShop();
		void StartOfflinePrivateShop(LPCHARACTER ch, DWORD dwVid);
		void AddBankOfflinePrivateShop(LPSHOP pkShop, DWORD dwPrices[]);
		BYTE GetOfflinePrivateShopCount(DWORD dwVid);
		void SetOfflinePrivateShops(LPOPSHOP pkOfflinePrivateShops[], BYTE bCount, DWORD dwVid);

	private:
		TOfflinePrivateShopMap m_map_pkPrivateOfflineShop;
#endif
};

#endif

