#include "StdAfx.h"
#include "../EterLib/NetPacketHeaderMap.h"

#include "PythonNetworkStream.h"
#include "Packet.h"
#include "NetworkActorManager.h"

#include "GuildMarkDownloader.h"
#include "GuildMarkUploader.h"
#include "MarkManager.h"

#include "ProcessCRC.h"
#include "../eterPack/EterPackManager.h"
#include "../AntiCheatLib/CheatQueueManager.h"

static DWORD gs_nextDownloadMarkTime = 0;
class CMainPacketHeaderMap : public CNetworkPacketHeaderMap
{
	public:
		enum
		{
			STATIC_SIZE_PACKET = false,
			DYNAMIC_SIZE_PACKET = true,
		};

	public:
		CMainPacketHeaderMap()
		{
			Set(HEADER_GC_EMPIRE,				CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCEmpire), STATIC_SIZE_PACKET));
			Set(HEADER_GC_WARP,					CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCWarp), STATIC_SIZE_PACKET));
			Set(HEADER_GC_QUEST_INFO,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuestInfo), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_REQUEST_MAKE_GUILD,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PVP,					CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPVP), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DUEL_START,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDuelStart), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_CHARACTER_ADD,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHAR_ADDITIONAL_INFO,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterAdditionalInfo), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHARACTER_UPDATE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterUpdate), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHARACTER_DEL,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterDelete), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHARACTER_MOVE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMove), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHAT,					CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChat), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_SYNC_POSITION,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCC2C), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_LOGIN_SUCCESS,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoginSuccess), STATIC_SIZE_PACKET));
			Set(HEADER_GC_LOGIN_FAILURE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoginFailure), STATIC_SIZE_PACKET));

			Set(HEADER_GC_PLAYER_CREATE_SUCCESS,		 CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPlayerCreateSuccess), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_CREATE_FAILURE,		 CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCreateFailure), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_DELETE_SUCCESS,		 CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));

			Set(HEADER_GC_STUN,					CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCStun), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DEAD,					CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDead), STATIC_SIZE_PACKET));

			Set(HEADER_GC_MAIN_CHARACTER,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMainCharacter), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MAIN_CHARACTER2_EMPIRE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMainCharacter2_EMPIRE), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MAIN_CHARACTER3_BGM,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMainCharacter3_BGM), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MAIN_CHARACTER4_BGM_VOL,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMainCharacter4_BGM_VOL), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_POINTS,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPoints), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_POINT_CHANGE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPointChange), STATIC_SIZE_PACKET));

			Set(HEADER_GC_ITEM_SET,				CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSet), STATIC_SIZE_PACKET));

			Set(HEADER_GC_ITEM_USE,				CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemUse), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ITEM_UPDATE,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemUpdate), STATIC_SIZE_PACKET));

			Set(HEADER_GC_ITEM_GROUND_ADD,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemGroundAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ITEM_GROUND_DEL,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemGroundDel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ITEM_OWNERSHIP,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemOwnership), STATIC_SIZE_PACKET));

			Set(HEADER_GC_QUICKSLOT_ADD,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuickSlotAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_QUICKSLOT_DEL,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuickSlotDel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_QUICKSLOT_SWAP,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuickSlotSwap), STATIC_SIZE_PACKET));

			Set(HEADER_GC_WHISPER,				CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCWhisper), STATIC_SIZE_PACKET));

			Set(HEADER_GC_CHARACTER_POSITION,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPosition), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MOTION,				CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMotion), STATIC_SIZE_PACKET));

			Set(HEADER_GC_SHOP,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShop), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_SHOP_SIGN,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShopSign), STATIC_SIZE_PACKET));
			Set(HEADER_GC_EXCHANGE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExchange), STATIC_SIZE_PACKET));

			Set(HEADER_GC_PING,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPing), STATIC_SIZE_PACKET));

#ifdef ENABLE_MULTI_LANGUAGE
			Set(HEADER_GC_SCRIPT,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCScript), STATIC_SIZE_PACKET));
#else
			Set(HEADER_GC_SCRIPT,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCScript), DYNAMIC_SIZE_PACKET));
#endif
			Set(HEADER_GC_QUEST_CONFIRM,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuestConfirm), STATIC_SIZE_PACKET));

			Set(HEADER_GC_TARGET,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTarget), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MOUNT,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMount), STATIC_SIZE_PACKET));

			Set(HEADER_GC_CHANGE_SPEED,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChangeSpeed), STATIC_SIZE_PACKET));

			Set(HEADER_GC_HANDSHAKE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCHandshake), STATIC_SIZE_PACKET));
			Set(HEADER_GC_HANDSHAKE_OK,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));
			Set(HEADER_GC_OWNERSHIP,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCOwnership), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CREATE_FLY,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCreateFly), STATIC_SIZE_PACKET));

			Set(HEADER_GC_ADD_FLY_TARGETING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFlyTargeting), STATIC_SIZE_PACKET));
			Set(HEADER_GC_FLY_TARGETING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFlyTargeting), STATIC_SIZE_PACKET));

			Set(HEADER_GC_PHASE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPhase), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SKILL_LEVEL,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSkillLevel), STATIC_SIZE_PACKET));

			Set(HEADER_GC_MESSENGER,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMessenger), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_GUILD,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGuild), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_PARTY_INVITE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyInvite), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_ADD,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_UPDATE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyUpdate), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_REMOVE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyRemove), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_LINK,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyLink), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_UNLINK,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyUnlink), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_PARAMETER,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyParameter), STATIC_SIZE_PACKET));

			Set(HEADER_GC_SAFEBOX_SET,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSet), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SAFEBOX_DEL,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemDel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SAFEBOX_WRONG_PASSWORD,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSafeboxWrongPassword), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SAFEBOX_SIZE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSafeboxSize), STATIC_SIZE_PACKET));

			Set(HEADER_GC_FISHING,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFishing), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DUNGEON, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDungeon), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_TIME, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTime), STATIC_SIZE_PACKET));
			Set(HEADER_GC_WALK_MODE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCWalkMode), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHANGE_SKILL_GROUP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChangeSkillGroup), STATIC_SIZE_PACKET));
			Set(HEADER_GC_REFINE_INFORMATION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCRefineInformation), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SEPCIAL_EFFECT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSpecialEffect), STATIC_SIZE_PACKET));
			Set(HEADER_GC_NPC_POSITION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCNPCPosition), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_CHANGE_NAME, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChangeName), STATIC_SIZE_PACKET));

			Set(HEADER_GC_LOGIN_KEY, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoginKey), STATIC_SIZE_PACKET));

			Set(HEADER_GC_AUTH_SUCCESS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAuthSuccess), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHANNEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChannel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_VIEW_EQUIP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCViewEquip), STATIC_SIZE_PACKET));
			Set(HEADER_GC_LAND_LIST, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLandList), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_TARGET_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetUpdate), STATIC_SIZE_PACKET));
			Set(HEADER_GC_TARGET_DELETE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetDelete), STATIC_SIZE_PACKET));
			Set(HEADER_GC_TARGET_CREATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetCreate), STATIC_SIZE_PACKET));

			Set(HEADER_GC_AFFECT_ADD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAffectAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_AFFECT_REMOVE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAffectRemove), STATIC_SIZE_PACKET));

			Set(HEADER_GC_MALL_OPEN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMallOpen), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MALL_SET, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSet), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MALL_DEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemDel), STATIC_SIZE_PACKET));

			Set(HEADER_GC_LOVER_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoverInfo), STATIC_SIZE_PACKET));
			Set(HEADER_GC_LOVE_POINT_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLovePointUpdate), STATIC_SIZE_PACKET));

			Set(HEADER_GC_DIG_MOTION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDigMotion), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DAMAGE_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDamageInfo), STATIC_SIZE_PACKET));

			Set(HEADER_GC_SPECIFIC_EFFECT,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSpecificEffect), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DRAGON_SOUL_REFINE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDragonSoulRefine), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MARK_BLOCK,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMarkBlock), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MARK_IDXLIST,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMarkIDXList), STATIC_SIZE_PACKET));
			Set(HEADER_GC_GUILD_SYMBOL_DATA,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGuildSymbolData), STATIC_SIZE_PACKET));
#ifdef ENABLE_GEM_SYSTEM
			Set(HEADER_GC_GEM_SHOP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGemShop), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_EXTEND_INVEN_SYSTEM
			Set(HEADER_GC_EXTEND_INVEN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExtendInven), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MINI_GAME_RUMI
			Set(HEADER_GC_MINI_GAME_RUMI, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMiniGameRumi), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SHOW_CHEST_DROP
			Set(HEADER_GC_CHEST_DROP_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChestDropInfo), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_ACCE_SYSTEM
			Set(HEADER_GC_ACCE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketAcce), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			Set(HEADER_GC_CL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketChangeLook), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SEND_TARGET_INFO
			Set(HEADER_GC_TARGET_INFO,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetInfo), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SUPPORT_SYSTEM
			Set(HEADER_GC_SUPPORT_SKILL,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSupportUseSkill), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_DAMAGE_TOP
			Set(HEADER_GC_DAMAGE_TOP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDamageTop), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_RANKING_SYSTEM
			Set(HEADER_GC_RANKING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketRanking), DYNAMIC_SIZE_PACKET));
#endif
#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
			Set(HEADER_GC_ACCUMULATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAccumulate), STATIC_SIZE_PACKET));
#endif
			Set(HEADER_GC_ATTENDANCE_REQUEST_DATA, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAttendanceRequestData), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ATTENDANCE_REQUEST_REWARD_LIST, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAttendanceRequestRewardList), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_FISH_EVENT
			Set(HEADER_GC_FISH_EVENT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFishEvent), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
			Set(HEADER_GC_MINI_GAME_CATCH_KING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMiniGameCatchKing), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MAILBOX
			Set(HEADER_GC_MAILBOX, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMailBox), DYNAMIC_SIZE_PACKET));
#endif
			Set(HEADER_GC_ANTICHEAT_BLACKLIST, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAntiCheatBlacklist), DYNAMIC_SIZE_PACKET));
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
			Set(HEADER_GC_OFFLINE_PRIVATE_SHOP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCOfflinePrivateShopInfo), DYNAMIC_SIZE_PACKET));
#endif
		}
};

int g_iLastPacket[2] = { 0, 0 };

void CPythonNetworkStream::ExitApplication()
{
	if (__IsNotPing())
	{
		AbsoluteExitApplication();
	}
	else
	{
		SendChatPacket("/quit");
	}
}

void CPythonNetworkStream::ExitGame()
{
	if (__IsNotPing())
	{
		LogOutGame();
	}
	else
	{
		SendChatPacket("/phase_select");
	}
}
void CPythonNetworkStream::LogOutGame()
{
	if (__IsNotPing())
	{
		AbsoluteExitGame();
	}
	else
	{
		SendChatPacket("/logout");
	}
}

void CPythonNetworkStream::AbsoluteExitGame()
{
	if (!IsOnline())
		return;

	OnRemoteDisconnect();
	Disconnect();
}

void CPythonNetworkStream::AbsoluteExitApplication()
{
	PostQuitMessage(0);
}

bool CPythonNetworkStream::__IsNotPing()
{
	return false;
}

DWORD CPythonNetworkStream::GetGuildID()
{
	return m_dwGuildID;
}

UINT CPythonNetworkStream::UploadMark(const char * c_szImageFileName)
{

	if (0 == m_dwGuildID)
		return ERROR_MARK_UPLOAD_NEED_RECONNECT;

	gs_nextDownloadMarkTime = 0;
	UINT uError=ERROR_UNKNOWN;
	CGuildMarkUploader& rkGuildMarkUploader=CGuildMarkUploader::Instance();
	if (!rkGuildMarkUploader.Connect(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, m_dwGuildID, c_szImageFileName, &uError))
	{
		switch (uError)
		{
			case CGuildMarkUploader::ERROR_CONNECT:
				return ERROR_CONNECT_MARK_SERVER;
				break;
			case CGuildMarkUploader::ERROR_LOAD:
				return ERROR_LOAD_MARK;
				break;
			case CGuildMarkUploader::ERROR_WIDTH:
				return ERROR_MARK_WIDTH;
				break;
			case CGuildMarkUploader::ERROR_HEIGHT:
				return ERROR_MARK_HEIGHT;
				break;
			default:
				return ERROR_UNKNOWN;
		}
	}
	__DownloadMark();
	if (CGuildMarkManager::INVALID_MARK_ID == CGuildMarkManager::Instance().GetMarkID(m_dwGuildID))
		return ERROR_MARK_CHECK_NEED_RECONNECT;

	return ERROR_NONE;
}

UINT CPythonNetworkStream::UploadSymbol(const char* c_szImageFileName)
{
	UINT uError=ERROR_UNKNOWN;
	CGuildMarkUploader& rkGuildMarkUploader=CGuildMarkUploader::Instance();
	if (!rkGuildMarkUploader.ConnectToSendSymbol(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, m_dwGuildID, c_szImageFileName, &uError))
	{
		switch (uError)
		{
			case CGuildMarkUploader::ERROR_CONNECT:
				return ERROR_CONNECT_MARK_SERVER;
				break;
			case CGuildMarkUploader::ERROR_LOAD:
				return ERROR_LOAD_MARK;
				break;
			case CGuildMarkUploader::ERROR_WIDTH:
				return ERROR_MARK_WIDTH;
				break;
			case CGuildMarkUploader::ERROR_HEIGHT:
				return ERROR_MARK_HEIGHT;
				break;
			default:
				return ERROR_UNKNOWN;
		}
	}

	return ERROR_NONE;
}

void CPythonNetworkStream::__DownloadMark()
{
	DWORD curTime = ELTimer_GetMSec();

	if (curTime < gs_nextDownloadMarkTime)
		return;

	gs_nextDownloadMarkTime = curTime + 60000 * 3;

	CGuildMarkDownloader& rkGuildMarkDownloader = CGuildMarkDownloader::Instance();
	rkGuildMarkDownloader.Connect(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey);
}

void CPythonNetworkStream::__DownloadSymbol(const std::vector<DWORD> & c_rkVec_dwGuildID)
{
	CGuildMarkDownloader& rkGuildMarkDownloader=CGuildMarkDownloader::Instance();
	rkGuildMarkDownloader.ConnectToRecvSymbol(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, c_rkVec_dwGuildID);
}

void CPythonNetworkStream::SetPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd)
{
	if (ePhaseWnd>=PHASE_WINDOW_NUM)
		return;

	m_apoPhaseWnd[ePhaseWnd]=poPhaseWnd;
}

void CPythonNetworkStream::ClearPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd)
{
	if (ePhaseWnd>=PHASE_WINDOW_NUM)
		return;

	if (poPhaseWnd != m_apoPhaseWnd[ePhaseWnd])
		return;

	m_apoPhaseWnd[ePhaseWnd]=0;
}

void CPythonNetworkStream::SetServerCommandParserWindow(PyObject* poWnd)
{
	m_poSerCommandParserWnd = poWnd;
}

bool CPythonNetworkStream::IsSelectedEmpire()
{
	if (m_dwEmpireID)
		return true;

	return false;
}

UINT CPythonNetworkStream::GetAccountCharacterSlotDatau(UINT iSlot, UINT eType)
{
	if (iSlot >= PLAYER_PER_ACCOUNT)
		return 0;

	TSimplePlayerInformation&	rkSimplePlayerInfo=m_akSimplePlayerInfo[iSlot];

	switch (eType)
	{
		case ACCOUNT_CHARACTER_SLOT_ID:
			return rkSimplePlayerInfo.dwID;
		case ACCOUNT_CHARACTER_SLOT_RACE:
			return rkSimplePlayerInfo.byJob;
		case ACCOUNT_CHARACTER_SLOT_LEVEL:
			return rkSimplePlayerInfo.byLevel;
		case ACCOUNT_CHARACTER_SLOT_STR:
			return rkSimplePlayerInfo.byST;
		case ACCOUNT_CHARACTER_SLOT_DEX:
			return rkSimplePlayerInfo.byDX;
		case ACCOUNT_CHARACTER_SLOT_HTH:
			return rkSimplePlayerInfo.byHT;
		case ACCOUNT_CHARACTER_SLOT_INT:
			return rkSimplePlayerInfo.byIQ;
		case ACCOUNT_CHARACTER_SLOT_PLAYTIME:
			return rkSimplePlayerInfo.dwPlayMinutes;
		case ACCOUNT_CHARACTER_SLOT_FORM:

			return rkSimplePlayerInfo.wMainPart;
		case ACCOUNT_CHARACTER_SLOT_PORT:
			return rkSimplePlayerInfo.wPort;
		case ACCOUNT_CHARACTER_SLOT_GUILD_ID:
			return m_adwGuildID[iSlot];
			break;
		case ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG:
			return rkSimplePlayerInfo.bChangeName;
			break;
		case ACCOUNT_CHARACTER_SLOT_HAIR:
			return rkSimplePlayerInfo.wHairPart;
			break;
#ifdef ENABLE_ACCE_SYSTEM
		case ACCOUNT_CHARACTER_SLOT_ACCE:
			return rkSimplePlayerInfo.wAccePart;
			break;
#endif
	}
	return 0;
}

const char* CPythonNetworkStream::GetAccountCharacterSlotDataz(UINT iSlot, UINT eType)
{
	static const char* sc_szEmpty="";

	if (iSlot >= PLAYER_PER_ACCOUNT)
		return sc_szEmpty;

	TSimplePlayerInformation&	rkSimplePlayerInfo=m_akSimplePlayerInfo[iSlot];

	switch (eType)
	{
		case ACCOUNT_CHARACTER_SLOT_ADDR:
			{
				BYTE ip[4];

				const int LEN = 4;
				for (int i = 0; i < LEN; i++)
				{
					ip[i] = BYTE(rkSimplePlayerInfo.lAddr&0xff);
					rkSimplePlayerInfo.lAddr>>=8;
				}
				static char s_szAddr[256];
				sprintf(s_szAddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
				return s_szAddr;
			}
			break;
		case ACCOUNT_CHARACTER_SLOT_NAME:
			return rkSimplePlayerInfo.szName;
			break;
		case ACCOUNT_CHARACTER_SLOT_GUILD_NAME:
			return m_astrGuildName[iSlot].c_str();
			break;
	}
	return sc_szEmpty;
}

void CPythonNetworkStream::ConnectLoginServer(const char* c_szAddr, UINT uPort)
{
	CNetworkStream::Connect(c_szAddr, uPort);
}

void CPythonNetworkStream::SetMarkServer(const char* c_szAddr, UINT uPort)
{
	m_kMarkAuth.m_kNetAddr.Set(c_szAddr, uPort);
}

void CPythonNetworkStream::ConnectGameServer(UINT iChrSlot)
{
	if (iChrSlot >= PLAYER_PER_ACCOUNT)
		return;

	m_dwSelectedCharacterIndex = iChrSlot;

	__DirectEnterMode_Set(iChrSlot);

	TSimplePlayerInformation&	rkSimplePlayerInfo=m_akSimplePlayerInfo[iChrSlot];
	CNetworkStream::Connect((DWORD)rkSimplePlayerInfo.lAddr, rkSimplePlayerInfo.wPort);
}

void CPythonNetworkStream::SetLoginInfo(const char* c_szID, const char* c_szPassword)
{
	m_stID=c_szID;
	m_stPassword=c_szPassword;
}

void CPythonNetworkStream::ClearLoginInfo( void )
{
	m_stPassword = "";
}

void CPythonNetworkStream::SetLoginKey(DWORD dwLoginKey)
{
	m_dwLoginKey = dwLoginKey;
}

bool CPythonNetworkStream::CheckPacket(TPacketHeader * pRetHeader)
{
	*pRetHeader = 0;

	static CMainPacketHeaderMap s_packetHeaderMap;

	TPacketHeader header;

	if (!Peek(sizeof(TPacketHeader), &header))
		return false;

	if (0 == header)
	{
		if (!Recv(sizeof(TPacketHeader), &header))
			return false;

		while (Peek(sizeof(TPacketHeader), &header))
		{
			if (0 == header)
			{
				if (!Recv(sizeof(TPacketHeader), &header))
					return false;
			}
			else
			{
				break;
			}
		}

		if (0 == header)
			return false;
	}

	CNetworkPacketHeaderMap::TPacketType PacketType;

	if (!s_packetHeaderMap.Get(header, &PacketType))
	{
		TraceError("Unknown packet header: %d, last: %d %d", header, g_iLastPacket[0], g_iLastPacket[1]);
		return false;
	}
	if (PacketType.isDynamicSizePacket)
	{
		TDynamicSizePacketHeader DynamicSizePacketHeader;

		if (!Peek(sizeof(TDynamicSizePacketHeader), &DynamicSizePacketHeader))
			return false;

		if (!Peek(DynamicSizePacketHeader.size))
		{
			Tracef("CPythonNetworkStream::CheckPacket - Not enough dynamic packet size: header %d packet size: %d\n",
				DynamicSizePacketHeader.header,
				DynamicSizePacketHeader.size);
			return false;
		}
	}
	else
	{
		if (!Peek(PacketType.iPacketSize))
		{
			return false;
		}
	}

	if (!header)
		return false;

	*pRetHeader = header;

	g_iLastPacket[0] = g_iLastPacket[1];
	g_iLastPacket[1] = header;
	return true;
}

bool CPythonNetworkStream::RecvErrorPacket(int header)
{
	TraceError("Phase %s does not handle this header (header: %d, last: %d, %d)",
		m_strPhase.c_str(), header, g_iLastPacket[0], g_iLastPacket[1]);

	ClearRecvBuffer();
	return true;
}

bool CPythonNetworkStream::RecvPhasePacket()
{
	TPacketGCPhase packet_phase;

	if (!Recv(sizeof(TPacketGCPhase), &packet_phase))
		return false;

	switch (packet_phase.phase)
	{
		case PHASE_CLOSE:
			ClosePhase();
			break;

		case PHASE_HANDSHAKE:
			SetHandShakePhase();
			break;

		case PHASE_LOGIN:
			SetLoginPhase();
			break;

		case PHASE_SELECT:
			SetSelectPhase();

			BuildProcessCRC();
			__DownloadMark();

			break;

		case PHASE_LOADING:
			SetLoadingPhase();
			break;

		case PHASE_GAME:
			SetGamePhase();
			break;

		case PHASE_DEAD:
			break;
	}

	return true;
}

bool CPythonNetworkStream::RecvPingPacket()
{
	Tracef("recv ping packet. (securitymode %u)\n", IsSecurityMode());

	TPacketGCPing kPacketPing;

	if (!Recv(sizeof(TPacketGCPing), &kPacketPing))
		return false;

	m_dwLastGamePingTime = ELTimer_GetMSec();

	TPacketCGPong kPacketPong;
	kPacketPong.bHeader = HEADER_CG_PONG;

	if (!Send(sizeof(TPacketCGPong), &kPacketPong))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvDefaultPacket(int header)
{
	if (!header)
		return true;

	TraceError("Unprocessed packet header %d, state %s\n", header, m_strPhase.c_str());
	ClearRecvBuffer();
	return true;
}

bool CPythonNetworkStream::OnProcess()
{
	if (m_isStartGame)
	{
		m_isStartGame = FALSE;

		PyCallClassMemberFunc(m_poHandler, "SetGamePhase", Py_BuildValue("()"));

	}

	m_rokNetActorMgr->Update();

	if (m_phaseProcessFunc.IsEmpty())
		return true;
	{
		m_phaseProcessFunc.Run();
	}

	return true;
}
void CPythonNetworkStream::SetOffLinePhase()
{
	if ("OffLine" != m_strPhase)
		m_phaseLeaveFunc.Run();

	m_strPhase = "OffLine";

	Tracen("");
	Tracen("## Network - OffLine Phase ##");
	Tracen("");

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::OffLinePhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveOfflinePhase);

	SetGameOffline();

	m_dwSelectedCharacterIndex = 0;

	__DirectEnterMode_Initialize();
	__BettingGuildWar_Initialize();
}
void CPythonNetworkStream::ClosePhase()
{
	PyCallClassMemberFunc(m_poHandler, "SetLoginPhase", Py_BuildValue("()"));
}
void CPythonNetworkStream::SetGameOnline()
{
	m_isGameOnline = TRUE;
}

void CPythonNetworkStream::SetGameOffline()
{
	m_isGameOnline = FALSE;
}

BOOL CPythonNetworkStream::IsGameOnline()
{
	return m_isGameOnline;
}
void CPythonNetworkStream::SetHandler(PyObject* poHandler)
{
	m_poHandler = poHandler;
}
DWORD CPythonNetworkStream::GetMainActorVID()
{
	return m_dwMainActorVID;
}

DWORD CPythonNetworkStream::GetMainActorRace()
{
	return m_dwMainActorRace;
}

DWORD CPythonNetworkStream::GetMainActorEmpire()
{
	return m_dwMainActorEmpire;
}

DWORD CPythonNetworkStream::GetMainActorSkillGroup()
{
	return m_dwMainActorSkillGroup;
}

void CPythonNetworkStream::SetEmpireID(DWORD dwEmpireID)
{
	m_dwEmpireID = dwEmpireID;
}

DWORD CPythonNetworkStream::GetEmpireID()
{
	return m_dwEmpireID;
}

void CPythonNetworkStream::__ClearSelectCharacterData()
{
	memset(&m_akSimplePlayerInfo, 0, sizeof(m_akSimplePlayerInfo));

	for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
	{
		m_adwGuildID[i] = 0;
		m_astrGuildName[i] = "";
	}
}

void CPythonNetworkStream::__DirectEnterMode_Initialize()
{
	m_kDirectEnterMode.m_isSet=false;
	m_kDirectEnterMode.m_dwChrSlotIndex=0;
}

void CPythonNetworkStream::__DirectEnterMode_Set(UINT uChrSlotIndex)
{
	m_kDirectEnterMode.m_isSet=true;
	m_kDirectEnterMode.m_dwChrSlotIndex=uChrSlotIndex;
}

bool CPythonNetworkStream::__DirectEnterMode_IsSet()
{
	return m_kDirectEnterMode.m_isSet;
}

void CPythonNetworkStream::__InitializeMarkAuth()
{
	m_kMarkAuth.m_dwHandle=0;
	m_kMarkAuth.m_dwRandomKey=0;
}

void CPythonNetworkStream::__BettingGuildWar_Initialize()
{
	m_kBettingGuildWar.m_dwBettingMoney=0;
	m_kBettingGuildWar.m_dwObserverCount=0;
}

void CPythonNetworkStream::__BettingGuildWar_SetObserverCount(UINT uObserverCount)
{
	m_kBettingGuildWar.m_dwObserverCount=uObserverCount;
}

void CPythonNetworkStream::__BettingGuildWar_SetBettingMoney(UINT uBettingMoney)
{
	m_kBettingGuildWar.m_dwBettingMoney=uBettingMoney;
}

DWORD CPythonNetworkStream::EXPORT_GetBettingGuildWarValue(const char* c_szValueName)
{
	if (stricmp(c_szValueName, "OBSERVER_COUNT") == 0)
		return m_kBettingGuildWar.m_dwObserverCount;

	if (stricmp(c_szValueName, "BETTING_MONEY") == 0)
		return m_kBettingGuildWar.m_dwBettingMoney;

	return 0;
}

void CPythonNetworkStream::__ServerTimeSync_Initialize()
{
	m_kServerTimeSync.m_dwChangeClientTime=0;
	m_kServerTimeSync.m_dwChangeServerTime=0;
}

void CPythonNetworkStream::SetWaitFlag()
{
	m_isWaitLoginKey = TRUE;
}

void CPythonNetworkStream::SendEmoticon(UINT eEmoticon)
{
	if(eEmoticon < m_EmoticonStringVector.size())
		SendChatPacket(m_EmoticonStringVector[eEmoticon].c_str());
	else
		assert(false && "SendEmoticon Error");
}

CPythonNetworkStream::CPythonNetworkStream()
{
	m_rokNetActorMgr=new CNetworkActorManager;

	memset(m_akSimplePlayerInfo, 0, sizeof(m_akSimplePlayerInfo));

	m_phaseProcessFunc.Clear();

	m_dwEmpireID = 0;
	m_dwGuildID = 0;

	m_dwMainActorVID = 0;
	m_dwMainActorRace = 0;
	m_dwMainActorEmpire = 0;
	m_dwMainActorSkillGroup = 0;
	m_poHandler = NULL;

	m_dwLastGamePingTime = 0;

	m_dwLoginKey = 0;
	m_isWaitLoginKey = FALSE;
	m_isStartGame = FALSE;
	m_isEnableChatInsultFilter = FALSE;
	m_bComboSkillFlag = FALSE;
	m_strPhase = "OffLine";

	__InitializeGamePhase();
	__InitializeMarkAuth();

	__DirectEnterMode_Initialize();
	__BettingGuildWar_Initialize();

	std::fill(m_apoPhaseWnd, m_apoPhaseWnd+PHASE_WINDOW_NUM, (PyObject*)NULL);
	m_poSerCommandParserWnd = NULL;

	SetOffLinePhase();

	CCheatDetectQueueMgr::Instance().RegisterReportFunction(
		std::bind(&CPythonNetworkStream::SendHackNotification, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
	);
}

CPythonNetworkStream::~CPythonNetworkStream()
{
	Tracen("PythonNetworkMainStream Clear");
}
