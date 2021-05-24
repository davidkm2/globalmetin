#pragma once

#include "Packet.h"

class CPythonExchange : public CSingleton<CPythonExchange>
{
	public:
		typedef struct trade
		{
			char					name[CHARACTER_NAME_MAX_LEN + 1];
#ifdef ENABLE_LEVEL_IN_TRADE
			DWORD					level;
#endif

			DWORD					item_vnum[EXCHANGE_MAX_NUM];
			BYTE					item_count[EXCHANGE_MAX_NUM];
			DWORD					item_metin[EXCHANGE_MAX_NUM][ITEM_SOCKET_SLOT_MAX_NUM];
			TPlayerItemAttribute	item_attr[EXCHANGE_MAX_NUM][ITEM_ATTRIBUTE_SLOT_MAX_NUM];
			WORD					item_grid[EXCHANGE_MAX_NUM];

			BYTE					accept;
			DWORD					elk;
#ifdef ENABLE_CHEQUE_SYSTEM
			short					cheque;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			DWORD					transmutation[EXCHANGE_MAX_NUM];
#endif
		} TExchangeData;

	public:
		CPythonExchange();
		virtual ~CPythonExchange();

		void			Clear();

		void			Start();
		void			End();
		bool			isTrading();
		void			SetSelfName(const char *name);
		void			SetTargetName(const char *name);

		char			*GetNameFromSelf();
		char			*GetNameFromTarget();

#ifdef ENABLE_LEVEL_IN_TRADE
		void			SetSelfLevel(DWORD level);
		void			SetTargetLevel(DWORD level);

		DWORD			GetLevelFromSelf();
		DWORD			GetLevelFromTarget();
#endif

		void			SetElkToTarget(DWORD elk);
		void			SetElkToSelf(DWORD elk);

		DWORD			GetElkFromTarget();
		DWORD			GetElkFromSelf();

#ifdef ENABLE_CHEQUE_SYSTEM
		void			SetChequeToTarget(short cheque);
		void			SetChequeToSelf(short cheque);

		short			GetChequeFromTarget();
		short			GetChequeFromSelf();
#endif

		void			SetItemToTarget(TItemPos pos, DWORD vnum, BYTE count);
		void			SetItemToSelf(TItemPos pos, DWORD vnum, BYTE count);

		void			SetItemMetinSocketToTarget(int pos, int imetinpos, DWORD vnum);
		void			SetItemMetinSocketToSelf(int pos, int imetinpos, DWORD vnum);

		void			SetItemAttributeToTarget(int pos, int iattrpos, BYTE byType, int sValue);
		void			SetItemAttributeToSelf(int pos, int iattrpos, BYTE byType, int sValue);

		void			DelItemOfTarget(BYTE pos, BYTE size);
		void			DelItemOfSelf(BYTE pos, BYTE size);

		DWORD			GetItemVnumFromTarget(BYTE pos);
		DWORD			GetItemVnumFromSelf(BYTE pos);

		BYTE			GetItemCountFromTarget(BYTE pos);
		BYTE			GetItemCountFromSelf(BYTE pos);

		BYTE			GetItemGridFromTarget(BYTE pos);
		BYTE			GetItemGridFromSelf(BYTE pos);

		DWORD			GetItemMetinSocketFromTarget(BYTE pos, int iMetinSocketPos);
		DWORD			GetItemMetinSocketFromSelf(BYTE pos, int iMetinSocketPos);

		void			GetItemAttributeFromTarget(BYTE pos, int iAttrPos, BYTE * pbyType, int * psValue);
		void			GetItemAttributeFromSelf(BYTE pos, int iAttrPos, BYTE * pbyType, int * psValue);

		void			SetAcceptToTarget(BYTE Accept);
		void			SetAcceptToSelf(BYTE Accept);

		bool			GetAcceptFromTarget();
		bool			GetAcceptFromSelf();

		bool			GetElkMode();
		void			SetElkMode(bool value);

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		void			SetItemTransmutation(int iPos, DWORD dwTransmutation, bool bSelf);
		DWORD			GetItemTransmutation(int iPos, bool bSelf);
#endif

	protected:
		bool				m_isTrading;

		bool				m_elk_mode;   
		TExchangeData		m_self;
		TExchangeData		m_victim;
};
