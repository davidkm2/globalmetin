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

		// �Խ�Ʈ �߰�/����
		virtual bool	AddGuest(LPCHARACTER ch,DWORD owner_vid, bool bOtherEmpire);
		void	RemoveGuest(LPCHARACTER ch);

		// ���� ����
		virtual int	Buy(LPCHARACTER ch, BYTE pos);

		// �Խ�Ʈ���� ��Ŷ�� ����
		void	BroadcastUpdateItem(BYTE pos);

		// �Ǹ����� �������� ������ �˷��ش�.
		int		GetNumberByVnum(DWORD dwVnum);

		// �������� ������ ��ϵǾ� �ִ��� �˷��ش�.
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
		std::vector<SHOP_ITEM>		m_itemVector;	// �� �������� ����ϴ� ���ǵ�

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

