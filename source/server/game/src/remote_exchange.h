#pragma once

#if 0
class CRemoteExchange : public CExchange
{
	public:
		CRemoteExchange(LPCHARACTER pOwner);
		virtual ~CRemoteExchange();

	protected:
		char szVictimName;
};

void exchange_packet(void * pvData, BYTE sub_header, bool is_me, DWORD arg1, TItemPos arg2, DWORD arg3, void * pvData2 = NULL);
#endif
