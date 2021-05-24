#ifndef __INC_METIN_II_GAME_SHOP_H__
#define __INC_METIN_II_GAME_SHOP_H__
#include "../../common/defines.h"
#include "packet.h"

enum
{
	SHOP_MAX_DISTANCE = 1000
};

class CShop
{
	public:
		typedef struct shop_item
		{
			TShopItemData item;
#ifdef __SHOP_SELL_INFO__
			TShopSellInfo info;
#endif
			int		itemid;
			LPITEM	pkItem;
			shop_item()
			{
				memset(&item, 0, sizeof(item));
#ifdef __SHOP_SELL_INFO__
				memset(&info, 0, sizeof(info));
#endif
				itemid = 0;
				pkItem = NULL;
			}
		} SHOP_ITEM;

		CShop();
		virtual ~CShop(); // @fixme139 (+virtual)

		bool	Create(DWORD dwVnum, DWORD dwNPCVnum, TShopItemTable * pItemTable);
		void	SetShopItems(TShopItemTable * pItemTable, BYTE bItemCount
#ifdef __OFFLINE_PRIVATE_SHOP__
, DWORD dwShopVid = 0
#endif
);

		virtual void	SetPCShop(LPCHARACTER ch);
		virtual bool	IsPCShop()	{ return m_pkPC ? true : false; }

		// 게스트 추가/삭제
		virtual bool	AddGuest(LPCHARACTER ch,DWORD owner_vid, bool bOtherEmpire);
		void	RemoveGuest(LPCHARACTER ch);

		// 물건 구입
		virtual int	Buy(LPCHARACTER ch, BYTE pos);

		// 게스트에게 패킷을 보냄
		void	BroadcastUpdateItem(BYTE pos);

		// 판매중인 아이템의 갯수를 알려준다.
		int		GetNumberByVnum(DWORD dwVnum);

		// 아이템이 상점에 등록되어 있는지 알려준다.
		virtual bool	IsSellingItem(DWORD itemID);

		DWORD	GetVnum() { return m_dwVnum; }
		DWORD	GetNPCVnum() { return m_dwNPCVnum; }

		bool	IsEmpty(WORD wCell, BYTE bSize, int iExceptionCell = -1);
		bool	IsValidPosition(DWORD dwPos);
		int		GetEmptyShop(DWORD dwSize);
		
	protected:
		void	Broadcast(const void * data, int bytes);

	protected:
		DWORD				m_dwVnum;
		DWORD				m_dwNPCVnum;

		WORD				m_pkGrid[SHOP_MAX_NUM];

		typedef std::unordered_map<LPCHARACTER, bool> GuestMapType;
		GuestMapType m_map_guest;
		std::vector<SHOP_ITEM>		m_itemVector;	// 이 상점에서 취급하는 물건들

		LPCHARACTER			m_pkPC;

#ifdef __OFFLINE_PRIVATE_SHOP__
	public:
		void SetOfflinePrivateShop(bool bIsOfflinePrivateShop) { m_bIsOfflinePrivateShop = bIsOfflinePrivateShop; }
		void SetOfflinePrivateShopItem(LPITEM pkItem);
		void SetOfflinePrivateShopInfo(TOfflinePrivateShopInfo * pkTOfflinePrivateShopInfo);

	protected:
		bool m_bIsOfflinePrivateShop;
#endif
};

#endif

