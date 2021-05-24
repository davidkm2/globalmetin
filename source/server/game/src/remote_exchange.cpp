#include "stdafx.h"
#include "remote_exchange.h"
#include "char.h"
#include "messenger_manager.h"
#include "exchange.h"
#include "p2p.h"

#if 0
void exchange_packet(void * pvData, BYTE sub_header, bool is_me, DWORD arg1, TItemPos arg2, DWORD arg3, void * pvData2)
{
	LPCHARACTER ch = (LPCHARACTER) pvData;
	LPDESC pkDesc = NULL;

	if (!ch || !ch->GetDesc())
	{
		const char* szName = (const char*)pvData;
		CCI * pkCCI = P2P_MANAGER::instance().Find(szName);
		if (pkCCI)
		{
			pkDesc = pkCCI->pkDesc;
			pkDesc->SetRelay(szName);
		}
		else
			return;
	}
	else
		pkDesc = ch->GetDesc();

	struct packet_exchange pack_exchg;

	pack_exchg.header 		= HEADER_GC_EXCHANGE;
	pack_exchg.sub_header 	= sub_header;
	pack_exchg.is_me		= is_me;
	pack_exchg.arg1		= arg1;
	pack_exchg.arg2		= arg2;
	pack_exchg.arg3		= arg3;

	if (sub_header == EXCHANGE_SUBHEADER_GC_ITEM_ADD && pvData)
	{
		memcpy(&pack_exchg.alSockets, ((LPITEM) pvData)->GetSockets(), sizeof(pack_exchg.alSockets));
		memcpy(&pack_exchg.aAttr, ((LPITEM) pvData)->GetAttributes(), sizeof(pack_exchg.aAttr));
#ifdef __CHANGELOOK__
		pack_exchg.dwTransmutation = ((LPITEM) pvData)->GetTransmutation();
#endif
	}
	else
	{
		memset(&pack_exchg.alSockets, 0, sizeof(pack_exchg.alSockets));
		memset(&pack_exchg.aAttr, 0, sizeof(pack_exchg.aAttr));
#ifdef __CHANGELOOK__
		pack_exchg.dwTransmutation = 0;
#endif
	}

	pkDesc->Packet(&pack_exchg, sizeof(pack_exchg));
	pkDesc->SetRelay("");
}

CRemoteExchange::CRemoteExchange(LPCHARACTER pOwner) : CExchange(pOwner)
{
	
}

CRemoteExchange::~CRemoteExchange()
{

}

bool CHARACTER::RemoteExchangeStart(CCI * pkCCI)
{
	if (IsObserverMode())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("관전 상태에서는 교환을 할 수 없습니다."));
		return false;
	}

#ifdef __MESSENGER_BLOCK__
	if (MessengerManager::instance().CheckMessengerList(this, pkCCI->szName, SYST_BLOCK))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s blokkk"), pkCCI->szName);
		return false;
	}
#endif

	if (IsOpenSafebox() || GetShopOwner() || GetMyShop() || IsCubeOpen())
	{
		ChatPacket( CHAT_TYPE_INFO, LC_TEXT("다른 거래창이 열려있을경우 거래를 할수 없습니다." ) );
		return false;
	}

#ifdef __ATTR_TRANSFER__
	if (IsAttrTransferOpen())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래창이 열려있을경우 거래를 할수 없습니다."));
		return false;
	}
#endif

	if (GetExchange())
		return false;

	exchange_packet(pkCCI->szName, EXCHANGE_SUBHEADER_GC_START, 0, GetVID(), NPOS, 0);
	exchange_packet(this, EXCHANGE_SUBHEADER_GC_START, 0, pkCCI->dwVID, NPOS, 0);

	return true;
}
#endif
