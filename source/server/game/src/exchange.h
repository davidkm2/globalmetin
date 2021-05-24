#ifndef __INC_METIN_II_GAME_EXCHANGE_H__
#define __INC_METIN_II_GAME_EXCHANGE_H__

enum EExchangeValues
{
	EXCHANGE_MAX_DISTANCE	= 1000
};

class CExchange
{
	public:
		CExchange(LPCHARACTER pOwner);
		virtual ~CExchange();

		bool		Accept(bool bIsAccept = true);
		void		Cancel();
#ifdef __CHEQUE__
		bool		AddCheque(short sCheque);
#endif
		bool		AddGold(long lGold);
		bool		AddItem(TItemPos item_pos, BYTE display_pos);
		bool		RemoveItem(BYTE pos);

		LPCHARACTER	GetOwner()	{ return m_pOwner;	}
		CExchange *	GetCompany()	{ return m_pCompany;	}

		bool		GetAcceptStatus() { return m_bAccept; }

		void		SetCompany(CExchange * pExchange)	{ m_pCompany = pExchange; }

		bool		IsEmpty(WORD wCell, BYTE bSize, int iExceptionCell = -1);
		bool		IsValidPosition(DWORD dwPos);

		int			GetEmptyExchange(DWORD dwSize);

	private:
		bool		Done();
		bool		Check(int * piItemCount);
		bool		CheckSpace();

	private:
		CExchange *	m_pCompany;	// 상대방의 CExchange 포인터

		LPCHARACTER	m_pOwner;

		TItemPos	m_aItemPos[EXCHANGE_MAX_NUM];
		LPITEM		m_apItems[EXCHANGE_MAX_NUM];
		BYTE		m_abItemDisplayPos[EXCHANGE_MAX_NUM];

		bool 		m_bAccept;
		long		m_lGold;
#ifdef __CHEQUE__
		short		m_sCheque;
#endif
};

void exchange_packet(LPCHARACTER ch, BYTE sub_header, bool is_me, DWORD arg1, TItemPos arg2, DWORD arg3, void * pvData = NULL);
#endif

