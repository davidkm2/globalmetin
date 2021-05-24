#include "stdafx.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "p2p.h"
#include "guild.h"
#include "guild_manager.h"
#include "party.h"
#include "messenger_manager.h"
#include "unique_item.h"
#include "xmas_event.h"
#include "affect.h"
#include "locale_service.h"
#include "questmanager.h"
#include "skill.h"
#ifdef __DAMAGE_TOP__
#include "damage_top.h"
#endif
#ifdef __REMOTE_EXCHANGE__
#include "exchange.h"
#endif

CInputP2P::CInputP2P()
{
	BindPacketInfo(&m_packetInfoGG);
}

void CInputP2P::Login(LPDESC d, const char * c_pData)
{
	P2P_MANAGER::instance().Login(d, (TPacketGGLogin *) c_pData);
}

void CInputP2P::Logout(LPDESC d, const char * c_pData)
{
	TPacketGGLogout * p = (TPacketGGLogout *) c_pData;
	P2P_MANAGER::instance().Logout(p->szName);
}

int CInputP2P::Relay(LPDESC d, const char * c_pData, size_t uiBytes)
{
	TPacketGGRelay * p = (TPacketGGRelay *) c_pData;

	if (uiBytes < sizeof(TPacketGGRelay) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	sys_log(0, "InputP2P::Relay : %s size %d", p->szName, p->lSize);

	LPCHARACTER pkChr = CHARACTER_MANAGER::instance().FindPC(p->szName);

	const BYTE* c_pbData = (const BYTE *) (c_pData + sizeof(TPacketGGRelay));

	if (!pkChr)
		return p->lSize;

	if (*c_pbData == HEADER_GC_WHISPER)
	{
		if (pkChr->IsBlockMode(BLOCK_WHISPER))
		{
			// 귓속말 거부 상태에서 귓속말 거부.
			return p->lSize;
		}

		char buf[1024];
		memcpy(buf, c_pbData, MIN(p->lSize, sizeof(buf)));

		TPacketGCWhisper* p2 = (TPacketGCWhisper*) buf;
		// bType 상위 4비트: Empire 번호
		// bType 하위 4비트: EWhisperType
		BYTE bToEmpire = (p2->bType >> 4);
		p2->bType = p2->bType & 0x0F;
		if(p2->bType == 0x0F) {
			// 시스템 메세지 귓속말은 bType의 상위비트까지 모두 사용함.
			p2->bType = WHISPER_TYPE_SYSTEM;
		}

		pkChr->GetDesc()->Packet(buf, p->lSize);
	}
#ifdef __MAILBOX__
	else if (*c_pbData == HEADER_GC_MAILBOX)
	{
		int space = pkChr->FindSpaceMail();
		if (space != -1)
		{
			char buf[1024];
			memcpy(buf, c_pbData, MIN(p->lSize, sizeof(buf)));

			TPacketGCMailBox* p2 = (TPacketGCMailBox*) buf;
			TMailData* Mail = (TMailData*) (buf + sizeof(p2));
			Mail->index = space;
			pkChr->AddMail(space, *Mail);
			pkChr->GetDesc()->Packet(buf, p->lSize);
		}
	}
#endif
	else
		pkChr->GetDesc()->Packet(c_pbData, p->lSize);

	return (p->lSize);
}

#ifdef __FULLNOTICE__
int CInputP2P::Notice(LPDESC d, const char * c_pData, size_t uiBytes, bool bBigFont)
#else
int CInputP2P::Notice(LPDESC d, const char * c_pData, size_t uiBytes)
#endif
{
	TPacketGGNotice * p = (TPacketGGNotice *) c_pData;

	if (uiBytes < sizeof(TPacketGGNotice) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char szBuf[256+1];
	strlcpy(szBuf, c_pData + sizeof(TPacketGGNotice), MIN(p->lSize + 1, sizeof(szBuf)));
#ifdef __FULLNOTICE__
	SendNotice(szBuf, bBigFont);
#else
	SendNotice(szBuf);
#endif
	return (p->lSize);
}

int CInputP2P::Guild(LPDESC d, const char* c_pData, size_t uiBytes)
{
	TPacketGGGuild * p = (TPacketGGGuild *) c_pData;
	uiBytes -= sizeof(TPacketGGGuild);
	c_pData += sizeof(TPacketGGGuild);

	CGuild * g = CGuildManager::instance().FindGuild(p->dwGuild);

	switch (p->bSubHeader)
	{
		case GUILD_SUBHEADER_GG_CHAT:
			{
				if (uiBytes < sizeof(TPacketGGGuildChat))
					return -1;

				TPacketGGGuildChat * p = (TPacketGGGuildChat *) c_pData;

				if (g)
					g->P2PChat(p->szText);

				return sizeof(TPacketGGGuildChat);
			}

		case GUILD_SUBHEADER_GG_SET_MEMBER_COUNT_BONUS:
			{
				if (uiBytes < sizeof(int))
					return -1;

				int iBonus = *((int *) c_pData);
				CGuild* pGuild = CGuildManager::instance().FindGuild(p->dwGuild);
				if (pGuild)
				{
					pGuild->SetMemberCountBonus(iBonus);
				}
				return sizeof(int);
			}
		default:
			sys_err ("UNKNOWN GUILD SUB PACKET");
			break;
	}
	return 0;
}


struct FuncShout
{
	const char * m_str;
	BYTE m_bEmpire;

	FuncShout(const char * str, BYTE bEmpire) : m_str(str), m_bEmpire(bEmpire)
	{
	}

	void operator () (LPDESC d)
	{
#ifdef __NEWSTUFF__
		if (!d->GetCharacter() || (!g_bGlobalShoutEnable && d->GetCharacter()->GetGMLevel() == GM_PLAYER && d->GetEmpire() != m_bEmpire))
			return;
#else
		if (!d->GetCharacter() || (d->GetCharacter()->GetGMLevel() == GM_PLAYER && d->GetEmpire() != m_bEmpire))
			return;
#endif
		d->GetCharacter()->ChatPacket(CHAT_TYPE_SHOUT, "%s", m_str);
	}
};

void SendShout(const char * szText, BYTE bEmpire)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), FuncShout(szText, bEmpire));
}

void CInputP2P::Shout(const char * c_pData)
{
	TPacketGGShout * p = (TPacketGGShout *) c_pData;
	SendShout(p->szText, p->bEmpire);
}

void CInputP2P::Disconnect(const char * c_pData)
{
	TPacketGGDisconnect * p = (TPacketGGDisconnect *) c_pData;

	LPDESC d = DESC_MANAGER::instance().FindByLoginName(p->szLogin);

	if (!d)
		return;

	if (!d->GetCharacter())
	{
		d->SetPhase(PHASE_CLOSE);
	}
	else
		d->DisconnectOfSameLogin();
}

void CInputP2P::Setup(LPDESC d, const char * c_pData)
{
	TPacketGGSetup * p = (TPacketGGSetup *) c_pData;
	sys_log(0, "P2P: Setup %s:%d", d->GetHostName(), p->wPort);
	d->SetP2P(d->GetHostName(), p->wPort, p->bChannel);
}

void CInputP2P::MessengerAdd(const char * c_pData)
{
	TPacketGGMessenger * p = (TPacketGGMessenger *) c_pData;
	sys_log(0, "P2P: Messenger Add %s %s", p->szAccount, p->szCompanion);
	MessengerManager::instance().__AddToList(p->szAccount, p->szCompanion);
}

void CInputP2P::MessengerRemove(const char * c_pData)
{
	TPacketGGMessenger * p = (TPacketGGMessenger *) c_pData;
	sys_log(0, "P2P: Messenger Remove %s %s", p->szAccount, p->szCompanion);
	MessengerManager::instance().__RemoveFromList(p->szAccount, p->szCompanion);
}

#ifdef __MESSENGER_BLOCK__
void CInputP2P::MessengerBlockAdd(const char * c_pData)
{
	TPacketGGMessenger * p = (TPacketGGMessenger *) c_pData;
	MessengerManager::instance().__AddToBlockList(p->szAccount, p->szCompanion);
}

void CInputP2P::MessengerBlockRemove(const char * c_pData)
{
	TPacketGGMessenger * p = (TPacketGGMessenger *) c_pData;
	MessengerManager::instance().__RemoveFromBlockList(p->szAccount, p->szCompanion);
}
#endif

void CInputP2P::FindPosition(LPDESC d, const char* c_pData)
{
	TPacketGGFindPosition* p = (TPacketGGFindPosition*) c_pData;
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(p->dwTargetPID);
#ifdef __CMD_WARP_IN_DUNGEON__
	if (ch)
#else
 	if (ch && ch->GetMapIndex() < 10000)
#endif
	{
		TPacketGGWarpCharacter pw;
		pw.header = HEADER_GG_WARP_CHARACTER;
		pw.pid = p->dwFromPID;
		pw.x = ch->GetX();
		pw.y = ch->GetY();
#ifdef __CMD_WARP_IN_DUNGEON__
		pw.mapIndex = (ch->GetMapIndex() < 10000) ? 0 : ch->GetMapIndex();
#endif
		d->Packet(&pw, sizeof(pw));
	}
}

void CInputP2P::WarpCharacter(const char* c_pData)
{
	TPacketGGWarpCharacter* p = (TPacketGGWarpCharacter*) c_pData;
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(p->pid);
#ifdef __CMD_WARP_IN_DUNGEON__
	if (ch)
		ch->WarpSet(p->x, p->y, p->mapIndex);
#else
 	if (ch)
 		ch->WarpSet(p->x, p->y);
#endif
}

void CInputP2P::GuildWarZoneMapIndex(const char* c_pData)
{
	TPacketGGGuildWarMapIndex * p = (TPacketGGGuildWarMapIndex*) c_pData;
	CGuildManager & gm = CGuildManager::instance();

	sys_log(0, "P2P: GuildWarZoneMapIndex g1(%u) vs g2(%u), mapIndex(%d)", p->dwGuildID1, p->dwGuildID2, p->lMapIndex);

	CGuild * g1 = gm.FindGuild(p->dwGuildID1);
	CGuild * g2 = gm.FindGuild(p->dwGuildID2);

	if (g1 && g2)
	{
		g1->SetGuildWarMapIndex(p->dwGuildID2, p->lMapIndex);
		g2->SetGuildWarMapIndex(p->dwGuildID1, p->lMapIndex);
	}
}

void CInputP2P::Transfer(const char * c_pData)
{
	TPacketGGTransfer * p = (TPacketGGTransfer *) c_pData;

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(p->szName);

	if (ch)
		ch->WarpSet(p->lX, p->lY);
}

#ifdef __REMOTE_EXCHANGE__
void CInputP2P::RemoteExchange(const char * c_pData)
{
	// TPacketGGRemoteExchange * p = (TPacketGGRemoteExchange *) c_pData;

	// LPCHARACTER ch, tch;
	// ch = CHARACTER_MANAGER::instance().FindByPID(p->arg1);
	// tch = p->pkChr;
	// if (tch && ch)
		// ch->ChatPacket(0, "%s", tch->GetName());

	// int iPulse = thecore_pulse();

	// switch(p->bSubHeader)
	// {
		// case REMOTE_EXCHANGE_SUBHEADER_GG_START:
		// {
			// ch = CHARACTER_MANAGER::instance().FindByPID(p->arg1);

			// if (!ch || !ch->CanHandleItem())
				// return;

			// if (!ch->GetExchange())
			// {
				// if (iPulse - ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
				// {
					// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("창고를 연후 %d초 이내에는 거래를 할수 없습니다."), g_nPortalLimitTime);
					// return;
				// }

				// if (ch->GetGold() >= GOLD_MAX)
				// {
					// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("액수가 20억 냥을 초과하여 거래를 할수가 없습니다.."));
					// sys_err("[OVERFLOG_GOLD] START (%u) id %u name %s ", ch->GetGold(), ch->GetPlayerID(), ch->GetName());
					// return;
				// }

// #ifdef __ATTR_TRANSFER__
				// if (ch->IsAttrTransferOpen())
				// {
					// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중일경우 개인상점을 열수가 없습니다."));
					// return;
				// }
// #endif

				// if (ch->GetMyShop() || ch->IsOpenSafebox() || ch->GetShopOwner() || ch->IsCubeOpen())
				// {
					// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중일경우 개인상점을 열수가 없습니다."));
					// return;
				// }

// #ifdef __BATTLE_FIELD__
				// if (CBattleField::instance().IsBattleFieldMapIndex(ch->GetMapIndex()))
				// {
					// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_FIELD_CANT_EXCHANGE"));
					// return;
				// }
// #endif

				// if (ch->IsObserverMode())
				// {
					// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("관전 상태에서는 교환을 할 수 없습니다."));
					// return false;
				// }

				// if (ch->IsOpenSafebox() || ch->GetShopOwner() || ch->GetMyShop() || ch->IsCubeOpen())
				// {
					// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래창이 열려있을경우 거래를 할수 없습니다."));
					// return false;
				// }

				// ch->SetExchange(M2_NEW CExchange(ch));
				// ch->SetExchangeTime();
				// exchange_packet(ch, EXCHANGE_SUBHEADER_GC_START, 0, p->arg2, NPOS, 0);
			// }
		// }
		// break;
		// case EXCHANGE_SUBHEADER_CG_ITEM_ADD:	// arg1 == position of item, arg2 == position in exchange window
			// if (ch->GetExchange())
				// ch->GetExchange()->AddItem(pinfo->Pos, pinfo->arg2);
			// break;

		// case EXCHANGE_SUBHEADER_CG_ITEM_DEL:	// arg1 == position of item
			// if (ch->GetExchange())
				// ch->GetExchange()->RemoveItem(pinfo->arg1);
			// break;

		// case EXCHANGE_SUBHEADER_CG_ELK_ADD:	// arg1 == amount of gold
			// if (ch->GetExchange())
			// {
				// const int64_t nTotalGold = static_cast<int64_t>(ch->GetExchange()->GetCompany()->GetOwner()->GetGold()) + static_cast<int64_t>(pinfo->arg1);

				// if (GOLD_MAX <= nTotalGold)
				// {
					// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방의 총금액이 20억 냥을 초과하여 거래를 할수가 없습니다.."));

					// sys_err("[OVERFLOW_GOLD] ELK_ADD (%u) id %u name %s ",
							// ch->GetExchange()->GetCompany()->GetOwner()->GetGold(),
							// ch->GetExchange()->GetCompany()->GetOwner()->GetPlayerID(),
						   	// ch->GetExchange()->GetCompany()->GetOwner()->GetName());

					// return;
				// }

				// if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
					// ch->GetExchange()->AddGold(pinfo->arg1);
			// }
			// break;
// #ifdef __CHEQUE__
		// case REMOTE_EXCHANGE_SUBHEADER_GG_CHEQUE_ADD:	// arg1 == amount of cheque
			// if (ch->GetExchange())
			// {
				// const int16_t nTotalCheque = static_cast<int16_t>(ch->GetExchange()->GetCompany()->GetOwner()->GetCheque()) + static_cast<int16_t>(pinfo->arg1);

				// if (CHEQUE_MAX <= nTotalCheque)
				// {
					// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The other player has cheque over the limite."));

					// sys_err("[OVERFLOW_CHEQUE] CHEQUE_ADD (%u) id %u name %s ",
							// ch->GetExchange()->GetCompany()->GetOwner()->GetCheque(),
							// ch->GetExchange()->GetCompany()->GetOwner()->GetPlayerID(),
						   	// ch->GetExchange()->GetCompany()->GetOwner()->GetName());

					// return;
				// }

				// if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
					// ch->GetExchange()->AddCheque(pinfo->arg1);
			// }
			// break;
// #endif
		// case REMOTE_EXCHANGE_SUBHEADER_GG_ACCEPT:	// arg1 == not used
			// if (ch->GetExchange())
				// ch->GetExchange()->Accept(true);
			// break;

		// case REMOTE_EXCHANGE_SUBHEADER_GG_CANCEL:	// arg1 == not used
			// if (ch->GetExchange())
				// ch->GetExchange()->Cancel();
			// break;

		// default:
			// break;
	// }

}
#endif

void CInputP2P::XmasWarpSanta(const char * c_pData)
{
	TPacketGGXmasWarpSanta * p =(TPacketGGXmasWarpSanta *) c_pData;

	if (p->bChannel == g_bChannel && map_allow_find(p->lMapIndex))
	{
		int	iNextSpawnDelay = 50 * 60;

		xmas::SpawnSanta(p->lMapIndex, iNextSpawnDelay); // 50분있다가 새로운 산타가 나타남 (한국은 20분)

		TPacketGGXmasWarpSantaReply pack_reply;
		pack_reply.bHeader = HEADER_GG_XMAS_WARP_SANTA_REPLY;
		pack_reply.bChannel = g_bChannel;
		P2P_MANAGER::instance().Send(&pack_reply, sizeof(pack_reply));
	}
}

void CInputP2P::XmasWarpSantaReply(const char* c_pData)
{
	TPacketGGXmasWarpSantaReply* p = (TPacketGGXmasWarpSantaReply*) c_pData;

	if (p->bChannel == g_bChannel)
	{
		CharacterVectorInteractor i;

		if (CHARACTER_MANAGER::instance().GetCharactersByRaceNum(xmas::MOB_SANTA_VNUM, i))
		{
			CharacterVectorInteractor::iterator it = i.begin();

			while (it != i.end()) {
				M2_DESTROY_CHARACTER(*it++);
			}
		}
	}
}

void CInputP2P::LoginPing(LPDESC d, const char * c_pData)
{
	TPacketGGLoginPing * p = (TPacketGGLoginPing *) c_pData;

	if (!g_pkAuthMasterDesc) // If I am master, I have to broadcast
		P2P_MANAGER::instance().Send(p, sizeof(TPacketGGLoginPing), d);
}

// BLOCK_CHAT
void CInputP2P::BlockChat(const char * c_pData)
{
	TPacketGGBlockChat * p = (TPacketGGBlockChat *) c_pData;

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(p->szName);

	if (ch)
	{
		sys_log(0, "BLOCK CHAT apply name %s dur %d", p->szName, p->lBlockDuration);
		ch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, AFF_NONE, p->lBlockDuration, 0, true);
	}
	else
	{
		sys_log(0, "BLOCK CHAT fail name %s dur %d", p->szName, p->lBlockDuration);
	}
}
// END_OF_BLOCK_CHAT
//

void CInputP2P::IamAwake(LPDESC d, const char * c_pData)
{
	std::string hostNames;
	P2P_MANAGER::instance().GetP2PHostNames(hostNames);
	sys_log(0, "P2P Awakeness check from %s. My P2P connection number is %d. and details...\n%s", d->GetHostName(), P2P_MANAGER::instance().GetDescCount(), hostNames.c_str());
}

int CInputP2P::Analyze(LPDESC d, BYTE bHeader, const char * c_pData)
{
	int iExtraLen = 0;

	switch (bHeader)
	{
		case HEADER_GG_SETUP:
			Setup(d, c_pData);
			break;

		case HEADER_GG_LOGIN:
			Login(d, c_pData);
			break;

		case HEADER_GG_LOGOUT:
			Logout(d, c_pData);
			break;

		case HEADER_GG_RELAY:
			if ((iExtraLen = Relay(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#ifdef __FULLNOTICE__
		case HEADER_GG_BIG_NOTICE:
			if ((iExtraLen = Notice(d, c_pData, m_iBufferLeft, true)) < 0)
				return -1;
			break;
#endif
		case HEADER_GG_NOTICE:
			if ((iExtraLen = Notice(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_GG_SHUTDOWN:
			sys_err("Accept shutdown p2p command from %s.", d->GetHostName());
			Shutdown(10);
			break;

		case HEADER_GG_GUILD:
			if ((iExtraLen = Guild(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_GG_SHOUT:
			Shout(c_pData);
			break;

		case HEADER_GG_DISCONNECT:
			Disconnect(c_pData);
			break;

		case HEADER_GG_MESSENGER_ADD:
			MessengerAdd(c_pData);
			break;

		case HEADER_GG_MESSENGER_REMOVE:
			MessengerRemove(c_pData);
			break;

#ifdef __MESSENGER_BLOCK__
		case HEADER_GG_MESSENGER_BLOCK_ADD:
			MessengerBlockAdd(c_pData);
			break;

		case HEADER_GG_MESSENGER_BLOCK_REMOVE:
			MessengerBlockRemove(c_pData);
			break;
#endif
		case HEADER_GG_FIND_POSITION:
			FindPosition(d, c_pData);
			break;

		case HEADER_GG_WARP_CHARACTER:
			WarpCharacter(c_pData);
			break;

		case HEADER_GG_GUILD_WAR_ZONE_MAP_INDEX:
			GuildWarZoneMapIndex(c_pData);
			break;

		case HEADER_GG_TRANSFER:
			Transfer(c_pData);
			break;

		case HEADER_GG_XMAS_WARP_SANTA:
			XmasWarpSanta(c_pData);
			break;

		case HEADER_GG_XMAS_WARP_SANTA_REPLY:
			XmasWarpSantaReply(c_pData);
			break;

		case HEADER_GG_RELOAD_CRC_LIST:
			LoadValidCRCList();
			break;

		case HEADER_GG_CHECK_CLIENT_VERSION:
			CheckClientVersion();
			break;

		case HEADER_GG_LOGIN_PING:
			LoginPing(d, c_pData);
			break;

		case HEADER_GG_BLOCK_CHAT:
			BlockChat(c_pData);
			break;

		case HEADER_GG_CHECK_AWAKENESS:
			IamAwake(d, c_pData);
			break;
#ifdef __REMOTE_EXCHANGE__
		case HEADER_GG_REMOTE_EXCHANGE:
			RemoteExchange(c_pData);
			break;
#endif
#ifdef __DAMAGE_TOP__
		case HEADER_GG_REGISTER_DAMAGE:
			CDamageTop::instance().UpdateDamageExtern(d, c_pData);
			break;
#endif
	}

	return (iExtraLen);
}

