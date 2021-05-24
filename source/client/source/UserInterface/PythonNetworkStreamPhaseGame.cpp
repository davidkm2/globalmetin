#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "Packet.h"

#include "PythonGuild.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"
#include "PythonBackground.h"
#include "PythonMiniMap.h"
#include "PythonTextTail.h"
#include "PythonItem.h"
#include "PythonChat.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonQuest.h"
#include "PythonEventManager.h"
#include "PythonMessenger.h"
#include "PythonApplication.h"

#include "../EterPack/EterPackManager.h"
#include "../GameLib/ItemManager.h"

#include "AbstractApplication.h"
#include "AbstractCharacterManager.h"
#include "InstanceBase.h"

#include "ProcessCRC.h"

#ifdef ENABLE_RANKING_SYSTEM
#include "PythonRanking.h"
#endif

#ifdef ENABLE_MAILBOX
#include "PythonMail.h"
#endif

#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
#include "PythonOfflinePrivateShop.h"
#endif

BOOL gs_bEmpireLanuageEnable = TRUE;

void CPythonNetworkStream::__RefreshAlignmentWindow()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshAlignment", Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshTargetBoardByVID(DWORD dwVID)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoardByVID", Py_BuildValue("(i)", dwVID));
}

void CPythonNetworkStream::__RefreshTargetBoardByName(const char * c_szName)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoardByName", Py_BuildValue("(s)", c_szName));
}

void CPythonNetworkStream::__RefreshTargetBoard()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoard", Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshGuildWindowGradePage()
{
	m_isRefreshGuildWndGradePage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowSkillPage()
{
	m_isRefreshGuildWndSkillPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPageGradeComboBox()
{
	m_isRefreshGuildWndMemberPageGradeComboBox=true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPage()
{
	m_isRefreshGuildWndMemberPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowBoardPage()
{
	m_isRefreshGuildWndBoardPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowInfoPage()
{
	m_isRefreshGuildWndInfoPage=true;
}

void CPythonNetworkStream::__RefreshMessengerWindow()
{
	m_isRefreshMessengerWnd=true;
}

void CPythonNetworkStream::__RefreshSafeboxWindow()
{
	m_isRefreshSafeboxWnd=true;
}

void CPythonNetworkStream::__RefreshMallWindow()
{
	m_isRefreshMallWnd=true;
}

void CPythonNetworkStream::__RefreshSkillWindow()
{
	m_isRefreshSkillWnd=true;
}

void CPythonNetworkStream::__RefreshExchangeWindow()
{
	m_isRefreshExchangeWnd=true;
}

void CPythonNetworkStream::__RefreshStatus()
{
	m_isRefreshStatus=true;
}

void CPythonNetworkStream::__RefreshCharacterWindow()
{
	m_isRefreshCharacterWnd=true;
}

void CPythonNetworkStream::__RefreshInventoryWindow()
{
	m_isRefreshInventoryWnd=true;
}

void CPythonNetworkStream::__RefreshEquipmentWindow()
{
	m_isRefreshEquipmentWnd=true;
}

void CPythonNetworkStream::__SetGuildID(DWORD id)
{
	if (m_dwGuildID != id)
	{
		m_dwGuildID = id;
		IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();

		for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
			if (!strncmp(m_akSimplePlayerInfo[i].szName, rkPlayer.GetName(), CHARACTER_NAME_MAX_LEN))
			{
				m_adwGuildID[i] = id;

				std::string  guildName;
				if (CPythonGuild::Instance().GetGuildName(id, &guildName))
				{
					m_astrGuildName[i] = guildName;
				}
				else
				{
					m_astrGuildName[i] = "";
				}
			}
	}
}

struct PERF_PacketInfo
{
	DWORD dwCount;
	DWORD dwTime;

	PERF_PacketInfo()
	{
		dwCount=0;
		dwTime=0;
	}
};

void CPythonNetworkStream::GamePhase()
{
	if (!m_kQue_stHack.empty())
	{
		__SendHack(m_kQue_stHack.front().c_str());
		m_kQue_stHack.pop_front();
	}

	TPacketHeader header = 0;
	bool ret = true;

	const DWORD MAX_RECV_COUNT = 4;
	const DWORD SAFE_RECV_BUFSIZE = 8192;
	DWORD dwRecvCount = 0;

    while (ret)
	{
		if(dwRecvCount++ >= MAX_RECV_COUNT-1 && GetRecvBufferSize() < SAFE_RECV_BUFSIZE
			&& m_strPhase == "Game") 
			break;

		if (!CheckPacket(&header))
			break;

		switch (header)
		{
			case HEADER_GC_WARP:
				ret = RecvWarpPacket();
				break;

			case HEADER_GC_PHASE:
				ret = RecvPhasePacket();
				return; 
				break;

			case HEADER_GC_PVP:
				ret = RecvPVPPacket();
				break;

			case HEADER_GC_DUEL_START:
				ret = RecvDuelStartPacket();
				break;

			case HEADER_GC_CHARACTER_ADD:
 				ret = RecvCharacterAppendPacket();
				break;

			case HEADER_GC_CHAR_ADDITIONAL_INFO:
				ret = RecvCharacterAdditionalInfo();
				break;

			case HEADER_GC_CHARACTER_UPDATE:
				ret = RecvCharacterUpdatePacket();
				break;

			case HEADER_GC_CHARACTER_DEL:
				ret = RecvCharacterDeletePacket();
				break;

			case HEADER_GC_CHAT:
				ret = RecvChatPacket();
				break;

			case HEADER_GC_SYNC_POSITION:
				ret = RecvSyncPositionPacket();
				break;

			case HEADER_GC_OWNERSHIP:
				ret = RecvOwnerShipPacket();
				break;

			case HEADER_GC_WHISPER:
				ret = RecvWhisperPacket();
				break;

			case HEADER_GC_CHARACTER_MOVE:
				ret = RecvCharacterMovePacket();
				break;

			case HEADER_GC_CHARACTER_POSITION:
				ret = RecvCharacterPositionPacket();
				break;

			case HEADER_GC_STUN:
				ret = RecvStunPacket();
				break;

			case HEADER_GC_DEAD:
				ret = RecvDeadPacket();
				break;

			case HEADER_GC_PLAYER_POINT_CHANGE:
				ret = RecvPointChange();
				break;

			case HEADER_GC_ITEM_SET:
				ret = RecvItemSetPacket();
				break;

			case HEADER_GC_ITEM_USE:
				ret = RecvItemUsePacket();
				break;

			case HEADER_GC_ITEM_UPDATE:
				ret = RecvItemUpdatePacket();
				break;

			case HEADER_GC_ITEM_GROUND_ADD:
				ret = RecvItemGroundAddPacket();
				break;

			case HEADER_GC_ITEM_GROUND_DEL:
				ret = RecvItemGroundDelPacket();
				break;

			case HEADER_GC_ITEM_OWNERSHIP:
				ret = RecvItemOwnership();
				break;

			case HEADER_GC_QUICKSLOT_ADD:
				ret = RecvQuickSlotAddPacket();
				break;

			case HEADER_GC_QUICKSLOT_DEL:
				ret = RecvQuickSlotDelPacket();
				break;

			case HEADER_GC_QUICKSLOT_SWAP:
				ret = RecvQuickSlotMovePacket();
				break;

			case HEADER_GC_MOTION:
				ret = RecvMotionPacket();
				break;

			case HEADER_GC_SHOP:
				ret = RecvShopPacket();
				break;

			case HEADER_GC_SHOP_SIGN:
				ret = RecvShopSignPacket();
				break;

			case HEADER_GC_EXCHANGE:
				ret = RecvExchangePacket();
				break;

			case HEADER_GC_QUEST_INFO:
				ret = RecvQuestInfoPacket();
				break;

			case HEADER_GC_REQUEST_MAKE_GUILD:
				ret = RecvRequestMakeGuild();
				break;

			case HEADER_GC_PING:
				ret = RecvPingPacket();
				break;

			case HEADER_GC_SCRIPT:
				ret = RecvScriptPacket();
				break;

			case HEADER_GC_QUEST_CONFIRM:
				ret = RecvQuestConfirmPacket();
				break;

			case HEADER_GC_TARGET:
				ret = RecvTargetPacket();
				break;

			case HEADER_GC_DAMAGE_INFO:
				ret = RecvDamageInfoPacket();
				break;

			case HEADER_GC_MOUNT:
				ret = RecvMountPacket();
				break;

			case HEADER_GC_CHANGE_SPEED:
				ret = RecvChangeSpeedPacket();
				break;

			case HEADER_GC_PLAYER_POINTS:
				ret = __RecvPlayerPoints();
				break;

			case HEADER_GC_CREATE_FLY:
				ret = RecvCreateFlyPacket();
				break;

			case HEADER_GC_FLY_TARGETING:
				ret = RecvFlyTargetingPacket();
				break;

			case HEADER_GC_ADD_FLY_TARGETING:
				ret = RecvAddFlyTargetingPacket();
				break;

			case HEADER_GC_SKILL_LEVEL:
				ret = RecvSkillLevel();
				break;

			case HEADER_GC_MESSENGER:
				ret = RecvMessenger();
				break;

			case HEADER_GC_GUILD:
				ret = RecvGuild();
				break;

			case HEADER_GC_PARTY_INVITE:
				ret = RecvPartyInvite();
				break;

			case HEADER_GC_PARTY_ADD:
				ret = RecvPartyAdd();
				break;

			case HEADER_GC_PARTY_UPDATE:
				ret = RecvPartyUpdate();
				break;

			case HEADER_GC_PARTY_REMOVE:
				ret = RecvPartyRemove();
				break;

			case HEADER_GC_PARTY_LINK:
				ret = RecvPartyLink();
				break;

			case HEADER_GC_PARTY_UNLINK:
				ret = RecvPartyUnlink();
				break;

			case HEADER_GC_PARTY_PARAMETER:
				ret = RecvPartyParameter();
				break;

			case HEADER_GC_SAFEBOX_SET:
				ret = RecvSafeBoxSetPacket();
				break;

			case HEADER_GC_SAFEBOX_DEL:
				ret = RecvSafeBoxDelPacket();
				break;

			case HEADER_GC_SAFEBOX_WRONG_PASSWORD:
				ret = RecvSafeBoxWrongPasswordPacket();
				break;

			case HEADER_GC_SAFEBOX_SIZE:
				ret = RecvSafeBoxSizePacket();
				break;

			case HEADER_GC_FISHING:
				ret = RecvFishing();
				break;

			case HEADER_GC_DUNGEON:
				ret = RecvDungeon();
				break;

			case HEADER_GC_TIME:
				ret = RecvTimePacket();
				break;

			case HEADER_GC_WALK_MODE:
				ret = RecvWalkModePacket();
				break;

			case HEADER_GC_CHANGE_SKILL_GROUP:
				ret = RecvChangeSkillGroupPacket();
				break;

			case HEADER_GC_REFINE_INFORMATION:
				ret = RecvRefineInformationPacket();
				break;

			case HEADER_GC_SEPCIAL_EFFECT:
				ret = RecvSpecialEffect();
				break;

			case HEADER_GC_NPC_POSITION:
				ret = RecvNPCList();
				break;

			case HEADER_GC_CHANNEL:
				ret = RecvChannelPacket();
				break;

			case HEADER_GC_VIEW_EQUIP:
				ret = RecvViewEquipPacket();
				break;

			case HEADER_GC_LAND_LIST:
				ret = RecvLandPacket();
				break;

			case HEADER_GC_TARGET_CREATE:
				ret = RecvTargetCreatePacket();
				break;

			case HEADER_GC_TARGET_UPDATE:
				ret = RecvTargetUpdatePacket();
				break;

			case HEADER_GC_TARGET_DELETE:
				ret = RecvTargetDeletePacket();
				break;

			case HEADER_GC_AFFECT_ADD:
				ret = RecvAffectAddPacket();
				break;

			case HEADER_GC_AFFECT_REMOVE:
				ret = RecvAffectRemovePacket();
				break;

			case HEADER_GC_MALL_OPEN:
				ret = RecvMallOpenPacket();
				break;

			case HEADER_GC_MALL_SET:
				ret = RecvMallItemSetPacket();
				break;

			case HEADER_GC_MALL_DEL:
				ret = RecvMallItemDelPacket();
				break;

			case HEADER_GC_LOVER_INFO:
				ret = RecvLoverInfoPacket();
				break;

			case HEADER_GC_LOVE_POINT_UPDATE:
				ret = RecvLovePointUpdatePacket();
				break;

			case HEADER_GC_DIG_MOTION:
				ret = RecvDigMotionPacket();
				break;

			case HEADER_GC_HANDSHAKE:
				RecvHandshakePacket();
				return;
				break;

			case HEADER_GC_HANDSHAKE_OK:
				RecvHandshakeOKPacket();
				return;
				break;

			case HEADER_GC_SPECIFIC_EFFECT:
				ret = RecvSpecificEffect();
				break;

			case HEADER_GC_DRAGON_SOUL_REFINE:
				ret = RecvDragonSoulRefine();
				break;

#ifdef ENABLE_GEM_SYSTEM
			case HEADER_GC_GEM_SHOP:
				ret = RecvGemShop();
				break;
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
			case HEADER_GC_EXTEND_INVEN:
				ret = RecvExtendInven();
				break;
#endif

#ifdef ENABLE_MINI_GAME_RUMI
			case HEADER_GC_MINI_GAME_RUMI:
				ret = RecvMiniGameRumi();
				break;
#endif

#ifdef ENABLE_ACCE_SYSTEM
			case HEADER_GC_ACCE:
				ret = RecvAccePacket();
				break;
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			case HEADER_GC_CL:
				ret = RecvChangeLookPacket();
				break;
#endif

#ifdef ENABLE_SHOW_CHEST_DROP
			case HEADER_GC_CHEST_DROP_INFO:
				ret = RecvChestDropInfo();
				break;
#endif

#ifdef ENABLE_SEND_TARGET_INFO
			case HEADER_GC_TARGET_INFO:
				ret = RecvTargetInfoPacket();
				break;
#endif

#ifdef ENABLE_SUPPORT_SYSTEM
			case HEADER_GC_SUPPORT_SKILL:
				ret = RecvSupportUseSkill();
				break;
#endif

#ifdef ENABLE_DAMAGE_TOP
			case HEADER_GC_DAMAGE_TOP:
				ret = RecvDamageTopInfo();
				break;
#endif

#ifdef ENABLE_RANKING_SYSTEM
			case HEADER_GC_RANKING:
				ret = RecvRankingInfo();
				break;
#endif

#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
			case HEADER_GC_ACCUMULATE:
				ret = RecvAccumulate();
				break;
#endif
				
			case HEADER_GC_ATTENDANCE_REQUEST_DATA:
				ret = RecvAttendanceRequestData();
				break;
				
			case HEADER_GC_ATTENDANCE_REQUEST_REWARD_LIST:
				ret = RecvAttendanceRequestRewardList();
				break;
#endif

#ifdef ENABLE_FISH_EVENT
			case HEADER_GC_FISH_EVENT:
				ret = RecvFishEvent();
				break;
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
			case HEADER_GC_MINI_GAME_CATCH_KING:
				ret = RecvMiniGameCatchKingPacket();
				break;
#endif

#ifdef ENABLE_MAILBOX
			case HEADER_GC_MAILBOX:
				ret = RecvMailBox();
				break;
#endif

			case HEADER_GC_ANTICHEAT_BLACKLIST:
				ret = RecvAntiCheatBlacklist();
				break;

#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
			case HEADER_GC_OFFLINE_PRIVATE_SHOP:
				ret = RecvOfflinePrivateShop();
				break;
#endif

			default:
				ret = RecvDefaultPacket(header);
				break;
		}
	}

	if (!ret)
		RecvErrorPacket(header);

	static DWORD s_nextRefreshTime = ELTimer_GetMSec();

	DWORD curTime = ELTimer_GetMSec();
	if (s_nextRefreshTime > curTime)
		return;

	if (m_isRefreshCharacterWnd)
	{
		m_isRefreshCharacterWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshEquipmentWnd)
	{
		m_isRefreshEquipmentWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshEquipment", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshInventoryWnd)
	{
		m_isRefreshInventoryWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshInventory", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshExchangeWnd)
	{
		m_isRefreshExchangeWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshExchange", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshSkillWnd)
	{
		m_isRefreshSkillWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSkill", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshSafeboxWnd)
	{
		m_isRefreshSafeboxWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafebox", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshMallWnd)
	{
		m_isRefreshMallWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMall", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshStatus)
	{
		m_isRefreshStatus=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshMessengerWnd)
	{
		m_isRefreshMessengerWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMessenger", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndInfoPage)
	{
		m_isRefreshGuildWndInfoPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildInfoPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndBoardPage)
	{
		m_isRefreshGuildWndBoardPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildBoardPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndMemberPage)
	{
		m_isRefreshGuildWndMemberPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMemberPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndMemberPageGradeComboBox)
	{
		m_isRefreshGuildWndMemberPageGradeComboBox=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMemberPageGradeComboBox", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndSkillPage)
	{
		m_isRefreshGuildWndSkillPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildSkillPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndGradePage)
	{
		m_isRefreshGuildWndGradePage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGradePage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}
}

void CPythonNetworkStream::__InitializeGamePhase()
{
	__ServerTimeSync_Initialize();

	m_isRefreshStatus=false;
	m_isRefreshCharacterWnd=false;
	m_isRefreshEquipmentWnd=false;
	m_isRefreshInventoryWnd=false;
	m_isRefreshExchangeWnd=false;
	m_isRefreshSkillWnd=false;
	m_isRefreshSafeboxWnd=false;
	m_isRefreshMallWnd=false;
	m_isRefreshMessengerWnd=false;
	m_isRefreshGuildWndInfoPage=false;
	m_isRefreshGuildWndBoardPage=false;
	m_isRefreshGuildWndMemberPage=false;
	m_isRefreshGuildWndMemberPageGradeComboBox=false;
	m_isRefreshGuildWndSkillPage=false;
	m_isRefreshGuildWndGradePage=false;

	m_EmoticonStringVector.clear();

	m_pInstTarget = NULL;
}

void CPythonNetworkStream::Warp(LONG lGlobalX, LONG lGlobalY)
{
	CPythonBackground& rkBgMgr=CPythonBackground::Instance();
	rkBgMgr.Destroy();
	rkBgMgr.Create();
	rkBgMgr.Warp(lGlobalX, lGlobalY);

	rkBgMgr.RefreshShadowLevel();

	LONG lLocalX = lGlobalX;
	LONG lLocalY = lGlobalY;
	__GlobalPositionToLocalPosition(lLocalX, lLocalY);
	float fHeight = CPythonBackground::Instance().GetHeight(float(lLocalX), float(lLocalY));

	IAbstractApplication& rkApp=IAbstractApplication::GetSingleton();
	rkApp.SetCenterPosition(float(lLocalX), float(lLocalY), fHeight);

	__ShowMapName(lLocalX, lLocalY);
}

void CPythonNetworkStream::__ShowMapName(LONG lLocalX, LONG lLocalY)
{
	const std::string & c_rstrMapFileName = CPythonBackground::Instance().GetWarpMapName();
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ShowMapName", Py_BuildValue("(sii)", c_rstrMapFileName.c_str(), lLocalX, lLocalY));
}

void CPythonNetworkStream::__LeaveGamePhase()
{
	CInstanceBase::ClearPVPKeySystem();

	__ClearNetworkActorManager();

	m_bComboSkillFlag = FALSE;

	IAbstractCharacterManager& rkChrMgr=IAbstractCharacterManager::GetSingleton();
	rkChrMgr.Destroy();

	CPythonItem& rkItemMgr=CPythonItem::Instance();
	rkItemMgr.Destroy();
}

void CPythonNetworkStream::SetGamePhase()
{
	if ("Game"!=m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Game Phase ##");
	Tracen("");

	m_strPhase = "Game";

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::GamePhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveGamePhase);

	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.SetMainCharacterIndex(GetMainActorVID());

	__RefreshStatus();
}

bool CPythonNetworkStream::RecvWarpPacket()
{
	TPacketGCWarp kWarpPacket;

	if (!Recv(sizeof(kWarpPacket), &kWarpPacket))
		return false;

	__DirectEnterMode_Set(m_dwSelectedCharacterIndex);

	CNetworkStream::Connect((DWORD)kWarpPacket.lAddr, kWarpPacket.wPort);

	return true;
}

bool CPythonNetworkStream::RecvDuelStartPacket()
{
	TPacketGCDuelStart kDuelStartPacket;
	if (!Recv(sizeof(kDuelStartPacket), &kDuelStartPacket))
		return false;

	DWORD count = (kDuelStartPacket.wSize - sizeof(kDuelStartPacket))/sizeof(DWORD);

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
	{
		TraceError("CPythonNetworkStream::RecvDuelStartPacket - MainCharacter is NULL");
		return false;
	}
	DWORD dwVIDSrc = pkInstMain->GetVirtualID();
	DWORD dwVIDDest;

	for ( DWORD i = 0; i < count; i++)
	{
		Recv(sizeof(dwVIDDest),&dwVIDDest);
		CInstanceBase::InsertDUELKey(dwVIDSrc,dwVIDDest);
	}

	if(count == 0)
		pkInstMain->SetDuelMode(CInstanceBase::DUEL_CANNOTATTACK);
	else
		pkInstMain->SetDuelMode(CInstanceBase::DUEL_START);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

	rkChrMgr.RefreshAllPCTextTail();

	return true;
}

bool CPythonNetworkStream::RecvPVPPacket()
{
	TPacketGCPVP kPVPPacket;
	if (!Recv(sizeof(kPVPPacket), &kPVPPacket))
		return false;

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();

	switch (kPVPPacket.bMode)
	{
		case PVP_MODE_AGREE:
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

			if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDDst))
				rkPlayer.RememberChallengeInstance(kPVPPacket.dwVIDSrc);

			if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDSrc))
				rkPlayer.RememberCantFightInstance(kPVPPacket.dwVIDDst);
			break;
		case PVP_MODE_REVENGE:
		{
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

			DWORD dwKiller = kPVPPacket.dwVIDSrc;
			DWORD dwVictim = kPVPPacket.dwVIDDst;

			if (rkPlayer.IsMainCharacterIndex(dwVictim))
				rkPlayer.RememberRevengeInstance(dwKiller);

			if (rkPlayer.IsMainCharacterIndex(dwKiller))
				rkPlayer.RememberCantFightInstance(dwVictim);
			break;
		}

		case PVP_MODE_FIGHT:
			rkChrMgr.InsertPVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
			break;
		case PVP_MODE_NONE:
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
			break;
	}

	__RefreshTargetBoardByVID(kPVPPacket.dwVIDSrc);
	__RefreshTargetBoardByVID(kPVPPacket.dwVIDDst);

	return true;
}

void CPythonNetworkStream::NotifyHack(const char* c_szMsg)
{
	if (!m_kQue_stHack.empty())
		if (c_szMsg==m_kQue_stHack.back())
			return;

	m_kQue_stHack.push_back(c_szMsg);	
}

bool CPythonNetworkStream::__SendHack(const char* c_szMsg)
{
	Tracen(c_szMsg);
	
	TPacketCGHack kPacketHack;
	kPacketHack.bHeader=HEADER_CG_HACK;
	strncpy(kPacketHack.szBuf, c_szMsg, sizeof(kPacketHack.szBuf)-1);

	if (!Send(sizeof(kPacketHack), &kPacketHack))
		return false;

	return true;
}

bool CPythonNetworkStream::SendMessengerAddByVIDPacket(DWORD vid)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_VID;
	if (!Send(sizeof(packet), &packet))
		return false;
	if (!Send(sizeof(vid), &vid))
		return false;
	return true;
}

bool CPythonNetworkStream::SendMessengerAddByNamePacket(const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_NAME;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szName[CHARACTER_NAME_MAX_LEN];
	strncpy(szName, c_szName, CHARACTER_NAME_MAX_LEN-1);
	szName[CHARACTER_NAME_MAX_LEN-1] = '\0'; 

	if (!Send(sizeof(szName), &szName))
		return false;
	Tracef(" SendMessengerAddByNamePacket : %s\n", c_szName);
	return true;
}

#ifdef ENABLE_MESSENGER_BLOCK
bool CPythonNetworkStream::SendMessengerBlockAddByVIDPacket(DWORD vid)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_VID;
	if (!Send(sizeof(packet), &packet))
		return false;
	if (!Send(sizeof(vid), &vid))
		return false;
	return true;
}

bool CPythonNetworkStream::SendMessengerBlockAddByNamePacket(const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_NAME;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szName[CHARACTER_NAME_MAX_LEN];
	strncpy(szName, c_szName, CHARACTER_NAME_MAX_LEN-1);
	szName[CHARACTER_NAME_MAX_LEN-1] = '\0'; 

	if (!Send(sizeof(szName), &szName))
		return false;
	Tracef("SendMessengerBlockAddByNamePacket : %s\n", c_szName);
	return true;
}

bool CPythonNetworkStream::SendMessengerBlockRemovePacket(const char * c_szKey, const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_REMOVE_BLOCK;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szKey[CHARACTER_NAME_MAX_LEN];
	strncpy(szKey, c_szKey, CHARACTER_NAME_MAX_LEN-1);
	if (!Send(sizeof(szKey), &szKey))
		return false;
	__RefreshTargetBoardByName(c_szName);
	return true;
}
#endif

bool CPythonNetworkStream::SendMessengerRemovePacket(const char * c_szKey, const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_REMOVE;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szKey[CHARACTER_NAME_MAX_LEN];
	strncpy(szKey, c_szKey, CHARACTER_NAME_MAX_LEN-1);
	if (!Send(sizeof(szKey), &szKey))
		return false;
	__RefreshTargetBoardByName(c_szName);
	return true;
}

bool CPythonNetworkStream::SendCharacterStatePacket(const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{
	if (!__CanActMainInstance())
		return true;

	if (fDstRot < 0.0f)
		fDstRot = 360 + fDstRot;
	else if (fDstRot > 360.0f)
		fDstRot = fmodf(fDstRot, 360.0f);

	TPacketCGMove kStatePacket;
	kStatePacket.bHeader = HEADER_CG_CHARACTER_MOVE;
	kStatePacket.bFunc = eFunc;
	kStatePacket.bArg = uArg;
	kStatePacket.bRot = fDstRot/5.0f;
	kStatePacket.lX = long(c_rkPPosDst.x);
	kStatePacket.lY = long(c_rkPPosDst.y);
	kStatePacket.dwTime = ELTimer_GetServerMSec();

	assert(kStatePacket.lX >= 0 && kStatePacket.lX < 204800);

	__LocalPositionToGlobalPosition(kStatePacket.lX, kStatePacket.lY);

	if (!Send(sizeof(kStatePacket), &kStatePacket))
	{
		Tracenf("CPythonNetworkStream::SendCharacterStatePacket(dwCmdTime=%u, fDstPos=(%f, %f), fDstRot=%f, eFunc=%d uArg=%d) - PACKET SEND ERROR",
			kStatePacket.dwTime,
			float(kStatePacket.lX),
			float(kStatePacket.lY),
			fDstRot,
			kStatePacket.bFunc,
			kStatePacket.bArg);
		return false;
	}
	return true;
}

bool CPythonNetworkStream::SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID)
{
	TPacketCGUseSkill UseSkillPacket;
	UseSkillPacket.bHeader = HEADER_CG_USE_SKILL;
	UseSkillPacket.dwVnum = dwSkillIndex;
	UseSkillPacket.dwTargetVID = dwTargetVID;
	if (!Send(sizeof(TPacketCGUseSkill), &UseSkillPacket))
	{
		Tracen("CPythonNetworkStream::SendUseSkillPacket - SEND PACKET ERROR");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendChatPacket(const char * c_szChat, BYTE byType)
{
	if (strlen(c_szChat) == 0)
		return true;

	if (strlen(c_szChat) >= 512)
		return true;

	if (c_szChat[0] == '/')
	{
		if (1 == strlen(c_szChat))
		{
			if (!m_strLastCommand.empty())
				c_szChat = m_strLastCommand.c_str();
		}
		else
		{
			m_strLastCommand = c_szChat;
		}
	}

	if (ClientCommand(c_szChat))
		return true;

	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGChat ChatPacket;
	ChatPacket.header = HEADER_CG_CHAT;
	ChatPacket.length = sizeof(ChatPacket) + iTextLen;
	ChatPacket.type = byType;

	if (!Send(sizeof(ChatPacket), &ChatPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return true;
}

void CPythonNetworkStream::RegisterEmoticonString(const char * pcEmoticonString)
{
	if (m_EmoticonStringVector.size() >= CInstanceBase::EMOTICON_NUM)
	{
		TraceError("Can't register emoticon string... vector is full (size:%d)", m_EmoticonStringVector.size() );
		return;
	}
	m_EmoticonStringVector.push_back(pcEmoticonString);
}

bool CPythonNetworkStream::ParseEmoticon(const char * pChatMsg, DWORD * pdwEmoticon)
{
	for (DWORD dwEmoticonIndex = 0; dwEmoticonIndex < m_EmoticonStringVector.size() ; ++dwEmoticonIndex)
	{
		if (strlen(pChatMsg) > m_EmoticonStringVector[dwEmoticonIndex].size())
			continue;

		const char * pcFind = strstr(pChatMsg, m_EmoticonStringVector[dwEmoticonIndex].c_str());

		if (pcFind != pChatMsg)
			continue;

		*pdwEmoticon = dwEmoticonIndex;

		return true;
	}

	return false;
}

void CPythonNetworkStream::__ConvertEmpireText(DWORD dwEmpireID, char* szText)
{
	if (dwEmpireID<1 || dwEmpireID>3)
		return;

	UINT uHanPos;

	STextConvertTable& rkTextConvTable=m_aTextConvTable[dwEmpireID-1];

	BYTE* pbText=(BYTE*)szText;
	while (*pbText)
	{
		if (*pbText & 0x80)
		{
			if (pbText[0]>=0xb0 && pbText[0]<=0xc8 && pbText[1]>=0xa1 && pbText[1]<=0xfe)
			{
				uHanPos=(pbText[0]-0xb0)*(0xfe - 0xa1+1)+(pbText[1]-0xa1);
				pbText[0]=rkTextConvTable.aacHan[uHanPos][0];
				pbText[1]=rkTextConvTable.aacHan[uHanPos][1];
			}
			pbText+=2;
		}
		else
		{
			if (*pbText>='a' && *pbText<='z')
			{
				*pbText=rkTextConvTable.acLower[*pbText-'a'];
			}
			else if (*pbText>='A' && *pbText<='Z')
			{
				*pbText=rkTextConvTable.acUpper[*pbText-'A'];
			}
			pbText++;
		}
	}
}

bool CPythonNetworkStream::RecvChatPacket()
{
	TPacketGCChat kChat;
    char buf[1024 + 1];
	char line[1024 + 1];

	if (!Recv(sizeof(kChat), &kChat))
		return false;

	UINT uChatSize=kChat.size - sizeof(kChat);

	if (!Recv(uChatSize, buf))
		return false;

	buf[uChatSize]='\0';

	if (LocaleService_IsEUROPE() && GetDefaultCodePage() == 1256)
	{
		char * p = strchr(buf, ':');
		if (p && p[1] == ' ')
			p[1] = 0x08;
	}

	if (kChat.type >= CHAT_TYPE_MAX_NUM)
		return true;

	IAbstractApplication & rkApp = IAbstractApplication::GetSingleton();

	string str(buf);

	if (str.find("|cffffc700|H", 0) != string::npos || str.find("|cfff1e6c0|H", 0) != string::npos)
	{
		size_t find, find1, find2, find3, find4;
		string s;

		find = str.find(':', 0);
		find1 = str.find(':', find + 1);
		find2 = str.find(':', find1 + 1);
		find3 = str.find('[');
		find4 = str.find(']');
		s += str.substr(0, find3 + 1);
		CItemData * pItemData;
		TraceError("Son %d %d: (%s)", find1, find2, str.substr(find1 + 1, find2 - 1 - find1).c_str());
		if (CItemManager::Instance().GetItemDataPointer(htoi(str.substr(find1 + 1, find2 - 1 - find1).c_str()), &pItemData))
			s += pItemData->GetName();

		s += str.substr(find4);

		str = s;
	}
	size_t counts = 0;

	if (str.find("{{{", 0) != string::npos)
	{
		size_t i, tfind;
		for (i = 0; (tfind = str.find("{{{", i)) != string::npos; i = tfind + 3)
			counts++;
	}

	if (counts)
	{
		size_t a = 0, b = 0, find, find2;
		string s;

		for (int i = 0; i < counts; ++i)
		{
			find = str.find("{{{", a);
			find2 = str.find("}}}", b);
			s += str.substr(b, find - b);
			CTokenVector kBuffTokenVector;
			rkApp.SplitLineByStr(str.substr(find + 3, find2 - 3 - find), ", ", &kBuffTokenVector);
			if (kBuffTokenVector.size() == 1)
			{
				if (atoi(kBuffTokenVector[0].c_str()) > 147000000)
				{
					CItemData * pItemData;
					if (CItemManager::Instance().GetItemDataPointer(atoi(kBuffTokenVector[0].c_str()) - 147000000, &pItemData))
						s += pItemData->GetName();
				}
				else
					s += CPythonNonPlayer::Instance().GetMonsterName(atoi(kBuffTokenVector[0].c_str()));
			}
			else if (kBuffTokenVector.size() == 2)
			{
				if (atoi(kBuffTokenVector[0].c_str()) > 147000000)
				{
					CPythonSkill::TSkillData * pSkillData;
					if (CPythonSkill::Instance().GetSkillData(atoi(kBuffTokenVector[0].c_str()) - 147000000, &pSkillData))
						s += pSkillData->strName;
				}
				else
					s += CPythonNonPlayer::Instance().GetMonsterName(atoi(kBuffTokenVector[0].c_str()));

				s += " ";

				CItemData * pItemData;
				if (CItemManager::Instance().GetItemDataPointer(atoi(kBuffTokenVector[1].c_str()) - 147000000, &pItemData))
					s += pItemData->GetName();
			}
			a = find + 3;
			b = find2 + 3;
			if (str.find("}}}", b) == string::npos)
				s += str.substr(find2 + 3);
		}

		str = s;
	}

#ifdef ENABLE_MULTI_LANGUAGE
	counts = 0;
	if (str.find("{{", 0) != string::npos)
	{
		size_t i, tfind;
		for (i = 0; (tfind = str.find("{{", i)) != string::npos; i = tfind + 2)
			counts++;
	}
	if (counts)
	{
		size_t a = 0, b = 0, find, find2;
		string s;

		for (int i = 0; i < counts; ++i)
		{
			find = str.find("{{", a);
			find2 = str.find("}}", b);
			s += str.substr(b, find - b);

			CTokenVector kBuffTokenVector;
			rkApp.SplitLineByStr(str.substr(find + 2, find2 - 2 - find), ", ", &kBuffTokenVector);
			if (kBuffTokenVector.size() == 1)
				s += rkApp.FindLocaleString(atoi(kBuffTokenVector[0].c_str()));
			else if (kBuffTokenVector.size() > 1)
			{
				string ss(rkApp.FindLocaleString(atoi(kBuffTokenVector[0].c_str())));

				string ddd = "";
				size_t c = 0, find3 = 0;
				for (int i = 1; i < kBuffTokenVector.size(); ++i)
				{
					if (kBuffTokenVector[i].find_first_not_of("0123456789", 0))
					{
						if (ss.find("%d", c) != string::npos)
						{
							find3 = ss.find("%d", c);
							ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), atoi(kBuffTokenVector[i].c_str()));
						}
						else if (ss.find("%s", c) != string::npos)
						{
							find3 = ss.find("%s", c);
							ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), kBuffTokenVector[i].c_str());
						}
					}
					else
					{
						if (ss.find("%s", c) != string::npos)
							find3 = ss.find("%s", c);

						ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), kBuffTokenVector[i].c_str());
					}

					c = find3 + 2;

					if (ss.find("%d", c) == string::npos && ss.find("%s", c) == string::npos)
						ddd += ss.substr(c);
				}
				s += ddd;
			}

			a = find + 2;
			b = find2 + 2;
			if (str.find("}}", b) == string::npos)
				s += str.substr(find2 + 2);

		}

		str = s;
	}

	counts = count(str.begin(), str.end(), '{');

	if (counts)
	{
		size_t a = 0, b = 0, find, find2;
		string s;

		for (int i = 0; i < counts; ++i)
		{
			find = str.find('{', a);
			find2 = str.find('}', b);
			s += str.substr(b, find - b);

			CTokenVector kBuffTokenVector;
			rkApp.SplitLineByStr(str.substr(find + 1, find2 - 1 - find), ", ", &kBuffTokenVector);
			if (kBuffTokenVector.size() == 1)
				s += rkApp.FindLocaleQuest(atoi(kBuffTokenVector[0].c_str()));
			else if (kBuffTokenVector.size() > 1)
			{
				string ss(rkApp.FindLocaleQuest(atoi(kBuffTokenVector[0].c_str())));
				string ddd = "";
				size_t c = 0, find3 = 0;
				for (int i = 1; i < kBuffTokenVector.size(); ++i)
				{
					if (kBuffTokenVector[i].find_first_not_of("0123456789", 0))
					{
						if (ss.find("%d", c) != string::npos)
						{
							find3 = ss.find("%d", c);
							ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), atoi(kBuffTokenVector[i].c_str()));
						}
						else if (ss.find("%s", c) != string::npos)
						{
							find3 = ss.find("%s", c);
							ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), kBuffTokenVector[i].c_str());
						}

					}
					else
					{
						if (ss.find("%s", c) != string::npos)
							find3 = ss.find("%s", c);

						ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), kBuffTokenVector[i].c_str());

					}

					c = find3 + 2;

					if (ss.find("%d", c) == string::npos && ss.find("%s", c) == string::npos)
						ddd += ss.substr(c);
				}
				s += ddd;
			}

			a = find + 1;
			b = find2 + 1;
			if (str.find('}', b) == string::npos)
				s += str.substr(find2 + 1);

		}

		str = s;
	}

	strcpy(buf, str.c_str());
#endif

#ifdef ENABLE_12ZI
	if (CHAT_TYPE_ZODIAC_NOTICE == kChat.type)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetMissionMessage", Py_BuildValue("(s)", buf));
		return true;
	}
#endif

	if (CHAT_TYPE_COMMAND == kChat.type)
	{
		ServerCommand(buf);
		return true;
	}

	if (kChat.dwVID != 0)
	{
		CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
		CInstanceBase * pkInstChatter = rkChrMgr.GetInstancePtr(kChat.dwVID);
		if (NULL == pkInstChatter)
			return true;

		switch (kChat.type)
		{
		case CHAT_TYPE_TALKING:
		case CHAT_TYPE_PARTY:
		case CHAT_TYPE_GUILD:
		case CHAT_TYPE_SHOUT:
		case CHAT_TYPE_WHISPER:
			{
				char * p = strchr(buf, ':');

				if (p)
					p += 2;
				else
					p = buf;

				DWORD dwEmoticon;

				if (ParseEmoticon(p, &dwEmoticon))
				{
					pkInstChatter->SetEmoticon(dwEmoticon);
					return true;
				}
				else
				{
					if (gs_bEmpireLanuageEnable)
					{
						CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
						if (pkInstMain)
							if (!pkInstMain->IsSameEmpire(*pkInstChatter))
								__ConvertEmpireText(pkInstChatter->GetEmpireID(), p);
					}

					if (m_isEnableChatInsultFilter)
					{
						if (false == pkInstChatter->IsNPC() && false == pkInstChatter->IsEnemy())
						{
							__FilterInsult(p, strlen(p));
						}
					}

					_snprintf(line, sizeof(line), "%s", p);
				}
			}
			break;
		case CHAT_TYPE_COMMAND:
		case CHAT_TYPE_INFO:
		case CHAT_TYPE_NOTICE:
		case CHAT_TYPE_BIG_NOTICE:
		case CHAT_TYPE_MAX_NUM:
		default:
			_snprintf(line, sizeof(line), "%s", buf);
			break;
		}

		if (CHAT_TYPE_SHOUT != kChat.type)
		{
			CPythonTextTail::Instance().RegisterChatTail(kChat.dwVID, line);
		}

		if (pkInstChatter->IsPC())
			CPythonChat::Instance().AppendChat(kChat.type, buf);
	}
	else
	{
		if (CHAT_TYPE_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetTipMessage", Py_BuildValue("(s)", buf));
		}
		else if (CHAT_TYPE_BIG_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetBigMessage", Py_BuildValue("(s)", buf));
		}
		else if (CHAT_TYPE_SHOUT == kChat.type)
		{
			char * p = strchr(buf, ':');

			if (p)
			{
				if (m_isEnableChatInsultFilter)
					__FilterInsult(p, strlen(p));
			}
		}

#ifdef ENABLE_MULTI_LANGUAGE
		if (str.find("[ENTER]", 0) != string::npos)
		{
			size_t i, tfind, a;
			for (i = 0, a = 0; (tfind = str.find("[ENTER]", i)) != string::npos; i = tfind + 7)
			{
				if (!a)
					CPythonChat::Instance().AppendChat(kChat.type, str.substr(i, tfind - i).c_str());
				else
					CPythonChat::Instance().AppendChat(kChat.type, ("\b" + str.substr(i, tfind - i)).c_str());
				a++;
			}

			CPythonChat::Instance().AppendChat(kChat.type, ("\b" + str.substr(i, str.size())).c_str());
		}
		else
			CPythonChat::Instance().AppendChat(kChat.type, buf);
#else
		CPythonChat::Instance().AppendChat(kChat.type, buf);
#endif
	}
	return true;
}

bool CPythonNetworkStream::RecvWhisperPacket()
{
	TPacketGCWhisper whisperPacket;
    char buf[512 + 1];

	if (!Recv(sizeof(whisperPacket), &whisperPacket))
		return false;

	assert(whisperPacket.wSize - sizeof(whisperPacket) < 512);

	if (!Recv(whisperPacket.wSize - sizeof(whisperPacket), &buf))
		return false;

	buf[whisperPacket.wSize - sizeof(whisperPacket)] = '\0';

	static char line[256];
	if (CPythonChat::WHISPER_TYPE_CHAT == whisperPacket.bType || CPythonChat::WHISPER_TYPE_GM == whisperPacket.bType)
	{
		_snprintf(line, sizeof(line), "%s : %s", whisperPacket.szNameFrom, buf);
#ifdef ENABLE_MULTI_LANGUAGE
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisper", Py_BuildValue("(issss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, line, whisperPacket.szEmpire, whisperPacket.szLanguage));
#else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisper", Py_BuildValue("(isss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, line, whisperPacket.szEmpire));
#endif
	}
	else if (CPythonChat::WHISPER_TYPE_SYSTEM == whisperPacket.bType || CPythonChat::WHISPER_TYPE_ERROR == whisperPacket.bType)
	{
#ifdef ENABLE_MULTI_LANGUAGE
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperSystemMessage", Py_BuildValue("(issss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, buf, whisperPacket.szEmpire, whisperPacket.szLanguage));
#else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperSystemMessage", Py_BuildValue("(isss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, buf, whisperPacket.szEmpire));
#endif
	}
	else
	{
#ifdef ENABLE_MULTI_LANGUAGE
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperError", Py_BuildValue("(issss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, buf, whisperPacket.szEmpire, whisperPacket.szLanguage));
#else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperError", Py_BuildValue("(isss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, buf, whisperPacket.szEmpire));
#endif
	}

	return true;
}

bool CPythonNetworkStream::SendWhisperPacket(const char * name, const char * c_szChat)
{
	if (strlen(c_szChat) >= 255)
		return true;

	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGWhisper WhisperPacket;
	WhisperPacket.bHeader = HEADER_CG_WHISPER;
	WhisperPacket.wSize = sizeof(WhisperPacket) + iTextLen;

	strncpy(WhisperPacket.szNameTo, name, sizeof(WhisperPacket.szNameTo) - 1);

	if (!Send(sizeof(WhisperPacket), &WhisperPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvPointChange()
{
	TPacketGCPointChange PointChange;

	if (!Recv(sizeof(TPacketGCPointChange), &PointChange))
	{
		Tracen("Recv Point Change Packet Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.ShowPointEffect(PointChange.Type, PointChange.dwVID);

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pInstance)
	if (PointChange.dwVID == pInstance->GetVirtualID())
	{
		CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
		rkPlayer.SetStatus(PointChange.Type, PointChange.value);

		switch (PointChange.Type)
		{
			case POINT_STAT_RESET_COUNT:
				__RefreshStatus();
				break;
			case POINT_LEVEL:
			case POINT_ST:
			case POINT_DX:
			case POINT_HT:
			case POINT_IQ:
				__RefreshStatus();
				__RefreshSkillWindow();
				break;
			case POINT_SKILL:
			case POINT_SUB_SKILL:
			case POINT_HORSE_SKILL:
				__RefreshSkillWindow();
				break;
			case POINT_ENERGY:
				if (PointChange.value == 0)
					rkPlayer.SetStatus(POINT_ENERGY_END_TIME, 0);
				__RefreshStatus();
				break;
			default:
				__RefreshStatus();
				break;
		}

		if (POINT_GOLD == PointChange.Type)
			if (PointChange.amount > 0)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickMoney", Py_BuildValue("(i)", PointChange.amount));

#ifdef ENABLE_CHEQUE_SYSTEM
		if (POINT_CHEQUE == PointChange.Type)
			if (PointChange.amount > 0)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickCheque", Py_BuildValue("(b)", PointChange.amount));
#endif
#ifdef ENABLE_GEM_SYSTEM
		if (POINT_GEM == PointChange.Type)
			if (PointChange.amount > 0)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickGem", Py_BuildValue("(i)", PointChange.amount));
#endif
#ifdef ENABLE_BATTLE_FIELD
		if (POINT_BATTLE_POINT == PointChange.Type)
			if (PointChange.amount > 0)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickBattlePoint", Py_BuildValue("(i)", PointChange.amount));
#endif
	}

	return true;
}

bool CPythonNetworkStream::RecvStunPacket()
{
	TPacketGCStun StunPacket;

	if (!Recv(sizeof(StunPacket), &StunPacket))
	{
		Tracen("CPythonNetworkStream::RecvStunPacket Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase * pkInstSel = rkChrMgr.GetInstancePtr(StunPacket.vid);

	if (pkInstSel)
	{
		if (CPythonCharacterManager::Instance().GetMainInstancePtr()==pkInstSel)
			pkInstSel->Die();
		else
			pkInstSel->Stun();
	}

	return true;
}

bool CPythonNetworkStream::RecvDeadPacket()
{
	TPacketGCDead DeadPacket;
	if (!Recv(sizeof(DeadPacket), &DeadPacket))
	{
		Tracen("CPythonNetworkStream::RecvDeadPacket Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase * pkChrInstSel = rkChrMgr.GetInstancePtr(DeadPacket.vid);
	if (pkChrInstSel)
	{
		CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
		if (pkInstMain==pkChrInstSel)
		{
			Tracenf("On MainActor");
			if (false == pkInstMain->GetDuelMode())
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnGameOver", Py_BuildValue("()"));

			CPythonPlayer::Instance().NotifyDeadMainCharacter();
		}

		pkChrInstSel->Die();
	}

	return true;
}

bool CPythonNetworkStream::SendCharacterPositionPacket(BYTE iPosition)
{
	TPacketCGPosition PositionPacket;

	PositionPacket.header = HEADER_CG_CHARACTER_POSITION;
	PositionPacket.position = iPosition;

	if (!Send(sizeof(TPacketCGPosition), &PositionPacket))
	{
		Tracen("Send Character Position Packet Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendOnClickPacket(DWORD vid)
{
	TPacketCGOnClick OnClickPacket;
	OnClickPacket.header	= HEADER_CG_ON_CLICK;
	OnClickPacket.vid		= vid;

	if (!Send(sizeof(OnClickPacket), &OnClickPacket))
	{
		Tracen("Send On_Click Packet Error");
		return false;
	}

	Tracef("SendOnClickPacket\n");
	return true;
}

bool CPythonNetworkStream::RecvCharacterPositionPacket()
{
	TPacketGCPosition PositionPacket;

	if (!Recv(sizeof(TPacketGCPosition), &PositionPacket))
		return false;

	CInstanceBase * pChrInstance = CPythonCharacterManager::Instance().GetInstancePtr(PositionPacket.vid);

	if (!pChrInstance)
		return true;

	return true;
}

bool CPythonNetworkStream::RecvMotionPacket()
{
	TPacketGCMotion MotionPacket;

	if (!Recv(sizeof(TPacketGCMotion), &MotionPacket))
		return false;

	CInstanceBase * pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(MotionPacket.vid);
	CInstanceBase * pVictimInstance = NULL;

	if (0 != MotionPacket.victim_vid)
		pVictimInstance = CPythonCharacterManager::Instance().GetInstancePtr(MotionPacket.victim_vid);

	if (!pMainInstance)
		return false;

	return true;
}

bool CPythonNetworkStream::RecvShopPacket()
{
	std::vector<char> vecBuffer;
	vecBuffer.clear();

    TPacketGCShop  packet_shop;
	if (!Recv(sizeof(packet_shop), &packet_shop))
		return false;

	int iSize = packet_shop.size - sizeof(packet_shop);
	if (iSize > 0)
	{
		vecBuffer.resize(iSize);
		if (!Recv(iSize, &vecBuffer[0]))
			return false;
	}

	switch (packet_shop.subheader)
	{
		case SHOP_SUBHEADER_GC_START:
			{
				CPythonShop::Instance().Clear();

				TPacketGCShopStart * pShopStartPacket = (TPacketGCShopStart *)&vecBuffer[0];
				DWORD dwVID = pShopStartPacket->owner_vid;
#ifdef ENABLE_BATTLE_FIELD
				CPythonShop::Instance().SetUsablePoint(pShopStartPacket->usablePoints);
				CPythonShop::Instance().SetLimitMaxPoint(pShopStartPacket->limitMaxPoints);
#endif
				for (BYTE iItemIndex = 0; iItemIndex < SHOP_MAX_NUM; ++iItemIndex)
					CPythonShop::Instance().SetItemData(iItemIndex, pShopStartPacket->items[iItemIndex]);

#ifdef ENABLE_SHOP_SELL_INFO
				for (BYTE iItemIndex = 0; iItemIndex < SHOP_MAX_NUM; ++iItemIndex)
					CPythonShop::Instance().SetInfo(iItemIndex, pShopStartPacket->infos[iItemIndex]);
#endif

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(i)", dwVID));
				__RefreshInventoryWindow();
			}
			break;

		case SHOP_SUBHEADER_GC_END:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndShop", Py_BuildValue("()"));
			__RefreshInventoryWindow();
			break;

		case SHOP_SUBHEADER_GC_UPDATE_ITEM:
			{
				TPacketGCShopUpdateItem * pShopUpdateItemPacket = (TPacketGCShopUpdateItem *)&vecBuffer[0];
				CPythonShop::Instance().SetItemData(pShopUpdateItemPacket->pos, pShopUpdateItemPacket->item);
#ifdef ENABLE_SHOP_SELL_INFO
				CPythonShop::Instance().SetInfo(pShopUpdateItemPacket->pos, pShopUpdateItemPacket->info);
#endif
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShop", Py_BuildValue("()"));
			}
			break;

		case SHOP_SUBHEADER_GC_UPDATE_PRICE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetShopSellingPrice", Py_BuildValue("(i)", *(int *)&vecBuffer[0]));
			break;

		case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY"));
			break;

		case SHOP_SUBHEADER_GC_SOLDOUT:
		case SHOP_SUBHEADER_GC_SOLD_OUT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SOLDOUT"));
			break;

		case SHOP_SUBHEADER_GC_INVENTORY_FULL:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVENTORY_FULL"));
			break;

		case SHOP_SUBHEADER_GC_INVALID_POS:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVALID_POS"));
			break;
#ifdef ENABLE_BATTLE_FIELD
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_BATTLE_POINTS:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_BP"));
			break;
			
		case SHOP_SUBHEADER_GC_EXCEED_LIMIT_TODAY:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "EXCEED_LIMIT_TODAY"));
			break;
#endif
#ifdef ENABLE_12ZI
		case SHOP_SUBHEADER_GC_ZODIAC_SHOP:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "LIMITED_PURCHASE_OVER"));
			break;
#endif
		default:
			TraceError("CPythonNetworkStream::RecvShopPacket: Unknown subheader %d\n", packet_shop.subheader);
			break;
	}

	return true;
}

bool CPythonNetworkStream::RecvExchangePacket()
{
	TPacketGCExchange exchange_packet;

	if (!Recv(sizeof(exchange_packet), &exchange_packet))
		return false;

	switch (exchange_packet.subheader)
	{
		case EXCHANGE_SUBHEADER_GC_START:
			CPythonExchange::Instance().Clear();
			CPythonExchange::Instance().Start();
			CPythonExchange::Instance().SetSelfName(CPythonPlayer::Instance().GetName());
#ifdef ENABLE_LEVEL_IN_TRADE

			CPythonExchange::Instance().SetSelfLevel(CPythonPlayer::Instance().GetStatus(POINT_LEVEL));
#endif

			{
				CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(exchange_packet.arg1);

				if (pCharacterInstance)
				{
					CPythonExchange::Instance().SetTargetName(pCharacterInstance->GetNameString());
#ifdef ENABLE_LEVEL_IN_TRADE
					CPythonExchange::Instance().SetTargetLevel(pCharacterInstance->GetLevel());
#endif
				}
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartExchange", Py_BuildValue("()"));
			__RefreshInventoryWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_ITEM_ADD:
			if (exchange_packet.is_me)
			{
				int iSlotIndex = exchange_packet.arg2.cell;
				CPythonExchange::Instance().SetItemToSelf(exchange_packet.arg2, exchange_packet.arg1, (BYTE) exchange_packet.arg3);
				for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
					CPythonExchange::Instance().SetItemMetinSocketToSelf(iSlotIndex, i, exchange_packet.alValues[i]);
				for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
					CPythonExchange::Instance().SetItemAttributeToSelf(iSlotIndex, j, exchange_packet.aAttr[j].bType, exchange_packet.aAttr[j].sValue);
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
				CPythonExchange::Instance().SetItemTransmutation(iSlotIndex, exchange_packet.dwTransmutation, true);
#endif
			}
			else
			{
				int iSlotIndex = exchange_packet.arg2.cell;
				CPythonExchange::Instance().SetItemToTarget(exchange_packet.arg2, exchange_packet.arg1, (BYTE) exchange_packet.arg3);
				for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
					CPythonExchange::Instance().SetItemMetinSocketToTarget(iSlotIndex, i, exchange_packet.alValues[i]);
				for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
					CPythonExchange::Instance().SetItemAttributeToTarget(iSlotIndex, j, exchange_packet.aAttr[j].bType, exchange_packet.aAttr[j].sValue);
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
				CPythonExchange::Instance().SetItemTransmutation(iSlotIndex, exchange_packet.dwTransmutation, false);
#endif
			}

			__RefreshExchangeWindow();
			__RefreshInventoryWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_ITEM_DEL:
			if (exchange_packet.is_me)
			{
				CPythonExchange::Instance().DelItemOfSelf((BYTE) exchange_packet.arg1, exchange_packet.arg3);
			}
			else
			{
				CPythonExchange::Instance().DelItemOfTarget((BYTE) exchange_packet.arg1, exchange_packet.arg3);
			}
			__RefreshExchangeWindow();
			__RefreshInventoryWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_ELK_ADD:
			if (exchange_packet.is_me)
				CPythonExchange::Instance().SetElkToSelf(exchange_packet.arg1);
			else
				CPythonExchange::Instance().SetElkToTarget(exchange_packet.arg1);

			__RefreshExchangeWindow();
			break;

#ifdef ENABLE_CHEQUE_SYSTEM
		case EXCHANGE_SUBHEADER_GC_CHEQUE_ADD:
			if (exchange_packet.is_me)
				CPythonExchange::Instance().SetChequeToSelf(exchange_packet.arg1);
			else
				CPythonExchange::Instance().SetChequeToTarget(exchange_packet.arg1);

			__RefreshExchangeWindow();
			break;
#endif

		case EXCHANGE_SUBHEADER_GC_ACCEPT:
			if (exchange_packet.is_me)
			{
				CPythonExchange::Instance().SetAcceptToSelf((BYTE) exchange_packet.arg1);
			}
			else
			{
				CPythonExchange::Instance().SetAcceptToTarget((BYTE) exchange_packet.arg1);
			}
			__RefreshExchangeWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_END:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndExchange", Py_BuildValue("()"));
			__RefreshInventoryWindow();
			CPythonExchange::Instance().End();
			break;

		case EXCHANGE_SUBHEADER_GC_ALREADY:
			Tracef("trade_already");
			break;

		case EXCHANGE_SUBHEADER_GC_LESS_ELK:
			Tracef("trade_less_elk");
			break;
#ifdef ENABLE_CHEQUE_SYSTEM
		case EXCHANGE_SUBHEADER_GC_LESS_CHEQUE:
			Tracef("trade_less_cheque");
			break;
#endif
	};

	return true;
}

bool CPythonNetworkStream::RecvQuestInfoPacket()
{
	TPacketGCQuestInfo QuestInfo;

	if (!Peek(sizeof(TPacketGCQuestInfo), &QuestInfo))
	{
		Tracen("Recv Quest Info Packet Error #1");
		return false;
	}

	if (!Peek(QuestInfo.size))
	{
		Tracen("Recv Quest Info Packet Error #2");
		return false;
	}

	Recv(sizeof(TPacketGCQuestInfo));

	const BYTE & c_rFlag = QuestInfo.flag;

	enum
	{
		QUEST_PACKET_TYPE_NONE,
		QUEST_PACKET_TYPE_BEGIN,
		QUEST_PACKET_TYPE_UPDATE,
		QUEST_PACKET_TYPE_END,
	};

	BYTE byQuestPacketType = QUEST_PACKET_TYPE_NONE;

	if (0 != (c_rFlag & QUEST_SEND_IS_BEGIN))
	{
		BYTE isBegin;
		if (!Recv(sizeof(isBegin), &isBegin))
			return false;

		if (isBegin)
			byQuestPacketType = QUEST_PACKET_TYPE_BEGIN;
		else
			byQuestPacketType = QUEST_PACKET_TYPE_END;
	}
	else
	{
		byQuestPacketType = QUEST_PACKET_TYPE_UPDATE;
	}

	char szTitle[30 + 1] = "";
	char szClockName[16 + 1] = "";
	int iClockValue = 0;
	char szCounterName[16 + 1] = "";
	int iCounterValue = 0;
	char szIconFileName[24 + 1] = "";

	if (0 != (c_rFlag & QUEST_SEND_TITLE))
	{
		if (!Recv(sizeof(szTitle), &szTitle))
			return false;

		szTitle[30]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_CLOCK_NAME))
	{
		if (!Recv(sizeof(szClockName), &szClockName))
			return false;

		szClockName[16]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_CLOCK_VALUE))
	{
		if (!Recv(sizeof(iClockValue), &iClockValue))
			return false;
	}
	if (0 != (c_rFlag & QUEST_SEND_COUNTER_NAME))
	{
		if (!Recv(sizeof(szCounterName), &szCounterName))
			return false;

		szCounterName[16]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_COUNTER_VALUE))
	{
		if (!Recv(sizeof(iCounterValue), &iCounterValue))
			return false;
	}
	if (0 != (c_rFlag & QUEST_SEND_ICON_FILE))
	{
		if (!Recv(sizeof(szIconFileName), &szIconFileName))
			return false;

		szIconFileName[24]='\0';
	}

	CPythonQuest& rkQuest=CPythonQuest::Instance();

	if (QUEST_PACKET_TYPE_END == byQuestPacketType)
	{
		rkQuest.DeleteQuestInstance(QuestInfo.index);
	}
	else if (QUEST_PACKET_TYPE_UPDATE == byQuestPacketType)
	{
		if (!rkQuest.IsQuest(QuestInfo.index))
		{
			rkQuest.MakeQuest(QuestInfo.index);
		}

		if (strlen(szTitle) > 0)
			rkQuest.SetQuestTitle(QuestInfo.index, szTitle);
		if (strlen(szClockName) > 0)
			rkQuest.SetQuestClockName(QuestInfo.index, szClockName);
		if (strlen(szCounterName) > 0)
			rkQuest.SetQuestCounterName(QuestInfo.index, szCounterName);
		if (strlen(szIconFileName) > 0)
			rkQuest.SetQuestIconFileName(QuestInfo.index, szIconFileName);

		if (c_rFlag & QUEST_SEND_CLOCK_VALUE)
			rkQuest.SetQuestClockValue(QuestInfo.index, iClockValue);
		if (c_rFlag & QUEST_SEND_COUNTER_VALUE)
			rkQuest.SetQuestCounterValue(QuestInfo.index, iCounterValue);
	}
	else if (QUEST_PACKET_TYPE_BEGIN == byQuestPacketType)
	{
		CPythonQuest::SQuestInstance QuestInstance;
		QuestInstance.dwIndex = QuestInfo.index;
		QuestInstance.strTitle = szTitle;
		QuestInstance.strClockName = szClockName;
		QuestInstance.iClockValue = iClockValue;
		QuestInstance.strCounterName = szCounterName;
		QuestInstance.iCounterValue = iCounterValue;
		QuestInstance.strIconFileName = szIconFileName;
		CPythonQuest::Instance().RegisterQuestInstance(QuestInstance);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshQuest", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvQuestConfirmPacket()
{
	TPacketGCQuestConfirm kQuestConfirmPacket;
	if (!Recv(sizeof(kQuestConfirmPacket), &kQuestConfirmPacket))
	{
		Tracen("RecvQuestConfirmPacket Error");
		return false;
	}

	PyObject * poArg = Py_BuildValue("(sii)", kQuestConfirmPacket.msg, kQuestConfirmPacket.timeout, kQuestConfirmPacket.requestPID);
 	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OnQuestConfirm", poArg);
	return true;
}

bool CPythonNetworkStream::RecvRequestMakeGuild()
{
	TPacketGCBlank blank;
	if (!Recv(sizeof(blank), &blank))
	{
		Tracen("RecvRequestMakeGuild Packet Error");
		return false;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AskGuildName", Py_BuildValue("()"));

	return true;
}

void CPythonNetworkStream::ToggleGameDebugInfo()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ToggleDebugInfo", Py_BuildValue("()"));
}

bool CPythonNetworkStream::SendExchangeStartPacket(DWORD vid)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_START;
	packet.arg1			= vid;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_start_packet Error\n");
		return false;
	}

	Tracef("send_trade_start_packet   vid %d \n", vid);
	return true;
}

#ifdef ENABLE_CHEQUE_SYSTEM
bool CPythonNetworkStream::SendExchangeChequeAddPacket(short cheque)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header = HEADER_CG_EXCHANGE;
	packet.subheader = EXCHANGE_SUBHEADER_CG_CHEQUE_ADD;
	packet.arg1 = cheque;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_cheque_add_packet Error\n");
		return false;
	}

	return true;
}
#endif

bool CPythonNetworkStream::SendExchangeElkAddPacket(DWORD elk)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ELK_ADD;
	packet.arg1			= elk;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_elk_add_packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendExchangeItemAddPacket(TItemPos ItemPos, BYTE byDisplayPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ITEM_ADD;
	packet.Pos			= ItemPos;
	packet.arg2			= byDisplayPos;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_item_add_packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendExchangeItemDelPacket(BYTE pos)
{

	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ITEM_DEL;
	packet.arg1			= pos;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_item_del_packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendExchangeAcceptPacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ACCEPT;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_accept_packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendExchangeExitPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_CANCEL;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_exit_packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendPointResetPacket()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartPointReset", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::__IsPlayerAttacking()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return false;

	if (!pkInstMain->IsAttacking())
		return false;

	return true;
}

bool CPythonNetworkStream::RecvScriptPacket()
{
	TPacketGCScript ScriptPacket;

	if (!Recv(sizeof(TPacketGCScript), &ScriptPacket))
	{
		TraceError("RecvScriptPacket_RecvError");
		return false;
	}

	if (ScriptPacket.size < sizeof(TPacketGCScript))
	{
		TraceError("RecvScriptPacket_SizeError");
		return false;
	}

	ScriptPacket.size -= sizeof(TPacketGCScript);
	
	static string str;
	str = "";
	str.resize(ScriptPacket.size+1);

	if (!Recv(ScriptPacket.size, &str[0]))
		return false;

	str[str.size()-1] = '\0';

	IAbstractApplication & rkApp = IAbstractApplication::GetSingleton();

	size_t counts = 0;
	if (str.find("{{{", 0) != string::npos)
	{
		size_t i, tfind;
		for (i = 0; (tfind = str.find("{{{", i)) != string::npos; i = tfind + 3)
			counts++;
	}

	if (counts)
	{
		size_t a = 0, b = 0, find, find2;
		string s;

		for (int i = 0; i < counts; ++i)
		{
			find = str.find("{{{", a);
			find2 = str.find("}}}", b);
			s += str.substr(b, find - b);
			CTokenVector kBuffTokenVector;
			rkApp.SplitLineByStr(str.substr(find + 3, find2 - 3 - find), ", ", &kBuffTokenVector);
			if (kBuffTokenVector.size() == 1)
			{
				if (atoi(kBuffTokenVector[0].c_str()) > 147000000)
				{
					CItemData * pItemData;
					if (CItemManager::Instance().GetItemDataPointer(atoi(kBuffTokenVector[0].c_str()) - 147000000, &pItemData))
						s += pItemData->GetName();
				}
				else
					s += CPythonNonPlayer::Instance().GetMonsterName(atoi(kBuffTokenVector[0].c_str()));
			}
			else if (kBuffTokenVector.size() == 2)
			{
				if (atoi(kBuffTokenVector[0].c_str()) > 147000000)
				{
					CPythonSkill::TSkillData * pSkillData;
					if (CPythonSkill::Instance().GetSkillData(atoi(kBuffTokenVector[0].c_str()) - 147000000, &pSkillData))
						s += pSkillData->strName;
				}
				else
					s += CPythonNonPlayer::Instance().GetMonsterName(atoi(kBuffTokenVector[0].c_str()));

				s += " ";

				CItemData * pItemData;
				if (CItemManager::Instance().GetItemDataPointer(atoi(kBuffTokenVector[1].c_str()) - 147000000, &pItemData))
					s += pItemData->GetName();
			}
			a = find + 3;
			b = find2 + 3;
			if (str.find("}}}", b) == string::npos)
				s += str.substr(find2 + 3);
		}

		str = s;
	}

#ifdef ENABLE_MULTI_LANGUAGE
	counts = 0;
	if (str.find("{{", 0) != string::npos)
	{
		size_t i, tfind;
		for (i = 0; (tfind = str.find("{{", i)) != string::npos; i = tfind + 2)
			counts++;
	}

	if (counts)
	{
		size_t a = 0, b = 0, find, find2;
		string s;

		for (int i = 0; i < counts; ++i)
		{
			find = str.find("{{", a);
			find2 = str.find("}}", b);
			s += str.substr(b, find - b);

			CTokenVector kBuffTokenVector;
			rkApp.SplitLineByStr(str.substr(find + 2, find2 - 2 - find), ", ", &kBuffTokenVector);
			if (kBuffTokenVector.size() == 1)
				s += rkApp.FindLocaleString(atoi(kBuffTokenVector[0].c_str()));
			else if (kBuffTokenVector.size() > 1)
			{
				string ss(rkApp.FindLocaleString(atoi(kBuffTokenVector[0].c_str())));
				string ddd = "";
				size_t c = 0, find3 = 0;
				for (int i = 1; i < kBuffTokenVector.size(); ++i)
				{
					if (kBuffTokenVector[i].find_first_not_of("0123456789", 0))
					{
						if (ss.find("%d", c) != string::npos)
						{
							find3 = ss.find("%d", c);
							ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), atoi(kBuffTokenVector[i].c_str()));
						}
						else if (ss.find("%s", c) != string::npos)
						{
							find3 = ss.find("%s", c);
							ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), kBuffTokenVector[i].c_str());
						}
						TraceError("DDDI:%s %s %s", ddd.c_str(), ss.substr(c, find3 + 2 - c).c_str(), rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), atoi(kBuffTokenVector[i].c_str())).c_str());
					}
					else
					{
						if (ss.find("%s", c) != string::npos)
							find3 = ss.find("%s", c);

						ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), kBuffTokenVector[i].c_str());
						TraceError("DDDS:%s %s %s", ddd.c_str(), ss.substr(c, find3 + 2 - c).c_str(), rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), atoi(kBuffTokenVector[i].c_str())).c_str());
					}

					c = find3 + 2;

					if (ss.find("%d", c) == string::npos && ss.find("%s", c) == string::npos)
						ddd += ss.substr(c);
				}
				s += ddd;
			}

			a = find + 2;
			b = find2 + 2;
			if (str.find("}}", b) == string::npos)
				s += str.substr(find2 + 2);

		}

		str = s;
	}

	counts = count(str.begin(), str.end(), '{');

	if (counts)
	{
		size_t a = 0, b = 0, find, find2;
		string s;

		for (int i = 0; i < counts; ++i)
		{
			find = str.find('{', a);
			find2 = str.find('}', b);
			s += str.substr(b, find - b);

			CTokenVector kBuffTokenVector;
			rkApp.SplitLineByStr(str.substr(find + 1, find2 - 1 - find), ", ", &kBuffTokenVector);
			if (kBuffTokenVector.size() == 1)
				s += rkApp.FindLocaleQuest(atoi(kBuffTokenVector[0].c_str()));
			else if (kBuffTokenVector.size() > 1)
			{
				string ss(rkApp.FindLocaleQuest(atoi(kBuffTokenVector[0].c_str())));
				string ddd = "";
				size_t c = 0, find3 = 0;
				for (int i = 1; i < kBuffTokenVector.size(); ++i)
				{
					if (kBuffTokenVector[i].find_first_not_of("0123456789", 0))
					{
						if (ss.find("%d", c) != string::npos)
						{
							find3 = ss.find("%d", c);
							ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), atoi(kBuffTokenVector[i].c_str()));
						}
						else if (ss.find("%s", c) != string::npos)
						{
							find3 = ss.find("%s", c);
							ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), kBuffTokenVector[i].c_str());
						}
					}
					else
					{
						if (ss.find("%s", c) != string::npos)
							find3 = ss.find("%s", c);

						ddd += rkApp.Convert_arg(ss.substr(c, find3 + 2 - c).c_str(), kBuffTokenVector[i].c_str());
					}

					c = find3 + 2;

					if (ss.find("%d", c) == string::npos && ss.find("%s", c) == string::npos)
						ddd += ss.substr(c);
				}
				s += ddd;
			}

			a = find + 1;
			b = find2 + 1;
			if (str.find('}', b) == string::npos)
				s += str.substr(find2 + 1);

		}

		str = s;
	}

#endif

	int iIndex = CPythonEventManager::Instance().RegisterEventSetFromString(str);

	if (-1 != iIndex)
	{
		CPythonEventManager::Instance().SetVisibleLineCount(iIndex, 30);
		CPythonNetworkStream::Instance().OnScriptEventStart(ScriptPacket.skin,iIndex);
	}

	return true;
}

bool CPythonNetworkStream::SendScriptAnswerPacket(int iAnswer)
{
	TPacketCGScriptAnswer ScriptAnswer;

	ScriptAnswer.header = HEADER_CG_SCRIPT_ANSWER;
	ScriptAnswer.answer = (BYTE) iAnswer;
	if (!Send(sizeof(TPacketCGScriptAnswer), &ScriptAnswer))
	{
		Tracen("Send Script Answer Packet Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendScriptButtonPacket(unsigned int iIndex)
{
	TPacketCGScriptButton ScriptButton;

	ScriptButton.header = HEADER_CG_SCRIPT_BUTTON;
	ScriptButton.idx = iIndex;
	if (!Send(sizeof(TPacketCGScriptButton), &ScriptButton))
	{
		Tracen("Send Script Button Packet Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendAnswerMakeGuildPacket(const char * c_szName)
{
	TPacketCGAnswerMakeGuild Packet;

	Packet.header = HEADER_CG_ANSWER_MAKE_GUILD;
	strncpy(Packet.guild_name, c_szName, GUILD_NAME_MAX_LEN);
	Packet.guild_name[GUILD_NAME_MAX_LEN] = '\0';

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendAnswerMakeGuild Packet Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendQuestInputStringPacket(const char * c_szString)
{
	TPacketCGQuestInputString Packet;
	Packet.bHeader = HEADER_CG_QUEST_INPUT_STRING;
	strncpy(Packet.szString, c_szString, QUEST_INPUT_STRING_MAX_NUM);

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendQuestInputStringPacket Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendQuestConfirmPacket(BYTE byAnswer, DWORD dwPID)
{
	TPacketCGQuestConfirm kPacket;
	kPacket.header = HEADER_CG_QUEST_CONFIRM;
	kPacket.answer = byAnswer;
	kPacket.requestPID = dwPID;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracen("SendQuestConfirmPacket Error");
		return false;
	}

	Tracenf(" SendQuestConfirmPacket : %d, %d", byAnswer, dwPID);
	return true;
}

bool CPythonNetworkStream::RecvSkillLevel()
{
	TPacketGCSkillLevel packet;

	if (!Recv(sizeof(TPacketGCSkillLevel), &packet))
	{
		Tracen("CPythonNetworkStream::RecvSkillLevel - RecvError");
		return false;
	}

	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();

#ifdef ENABLE_678TH_SKILL
	BYTE SKILL_ANTI_PALBANG = 221;
	BYTE SKILL_ANTI_BYEURAK = 228;
#ifdef ENABLE_WOLFMAN_CHARACTER
	BYTE SKILL_ANTI_SALPOONG = 229;
#endif
	BYTE SKILL_HELP_PALBANG = 236;
	BYTE SKILL_HELP_BYEURAK = 243;
#ifdef ENABLE_WOLFMAN_CHARACTER
	BYTE SKILL_HELP_SALPOONG = 244;
#endif
	
	rkPlayer.SetSkill(7, 0);
	rkPlayer.SetSkill(8, 0);
#endif

	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		TPlayerSkill & rPlayerSkill = packet.skills[i];
#ifdef ENABLE_678TH_SKILL
#ifdef ENABLE_WOLFMAN_CHARACTER
		if (i >= SKILL_ANTI_PALBANG && i <= SKILL_ANTI_SALPOONG)
#else
		if (i >= SKILL_ANTI_PALBANG && i <= SKILL_ANTI_BYEURAK)
#endif
		{
			if (rPlayerSkill.bLevel >= 1)
			{
				rkPlayer.SetSkill(7, i);
				rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
			}
		}
#ifdef ENABLE_WOLFMAN_CHARACTER
		else if (i >= SKILL_HELP_PALBANG && i <= SKILL_HELP_SALPOONG)
#else
		else if (i >= SKILL_HELP_PALBANG && i <= SKILL_HELP_BYEURAK)
#endif
		{
			if (rPlayerSkill.bLevel >= 1)
			{
				rkPlayer.SetSkill(8, i);
				rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
			}
		}
		else
			rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
#else
		if (i >= 112 && i <= 115 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(7, i);

		if (i >= 116 && i <= 119 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(8, i);
		rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
#endif
	}

	__RefreshSkillWindow();
	__RefreshStatus();

	return true;
}

bool CPythonNetworkStream::RecvDamageInfoPacket()
{
	TPacketGCDamageInfo DamageInfoPacket;

	if (!Recv(sizeof(TPacketGCDamageInfo), &DamageInfoPacket))
	{
		Tracen("Recv Target Packet Error");
		return false;
	}

	CInstanceBase * pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(DamageInfoPacket.dwVID);
	bool bSelf = (pInstTarget == CPythonCharacterManager::Instance().GetMainInstancePtr());
	bool bTarget = (pInstTarget==m_pInstTarget);
	if (pInstTarget)
	{
		if(DamageInfoPacket.damage >= 0)
			pInstTarget->AddDamageEffect(DamageInfoPacket.damage,DamageInfoPacket.flag,bSelf,bTarget);
		else
			TraceError("Damage is equal or below 0.");
	}

	return true;
}
bool CPythonNetworkStream::RecvTargetPacket()
{
	TPacketGCTarget TargetPacket;

	if (!Recv(sizeof(TPacketGCTarget), &TargetPacket))
	{
		Tracen("Recv Target Packet Error");
		return false;
	}

	CInstanceBase * pInstPlayer = CPythonCharacterManager::Instance().GetMainInstancePtr();
	CInstanceBase * pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(TargetPacket.dwVID);
	if (pInstPlayer && pInstTarget)
	{
		if (!pInstTarget->IsDead())
		{
#ifdef ENABLE_SUPPORT_SYSTEM
			if (pInstTarget->GetInstanceType() == CActorInstance::TYPE_NPC && pInstTarget->GetRace() == 3)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoardIfDifferent", Py_BuildValue("(i)", TargetPacket.dwVID));
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
						return false;
			}
#endif
			if (pInstTarget->IsPC() || pInstTarget->IsBuilding())
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoardIfDifferent", Py_BuildValue("(i)", TargetPacket.dwVID));
			else if (pInstPlayer->CanViewTargetHP(*pInstTarget))
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetHPTargetBoard", Py_BuildValue("(ii)", TargetPacket.dwVID, TargetPacket.bHPPercent));
#ifdef ENABLE_BATTLE_FIELD
			else if (pInstPlayer->IsBattleFieldMap() || pInstTarget->IsBattleFieldMap())
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));	
#endif
			else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

			m_pInstTarget = pInstTarget;
		}
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
	}

	return true;
}

bool CPythonNetworkStream::RecvMountPacket()
{
	TPacketGCMount MountPacket;

	if (!Recv(sizeof(TPacketGCMount), &MountPacket))
	{
		Tracen("Recv Mount Packet Error");
		return false;
	}

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(MountPacket.vid);

	if (pInstance)
	{

		if (0 != MountPacket.mount_vid)
		{

		}

		else
		{

		}
	}

	if (CPythonPlayer::Instance().IsMainCharacterIndex(MountPacket.vid))
	{

	}

	return true;
}

bool CPythonNetworkStream::RecvChangeSpeedPacket()
{
	TPacketGCChangeSpeed SpeedPacket;

	if (!Recv(sizeof(TPacketGCChangeSpeed), &SpeedPacket))
	{
		Tracen("Recv Speed Packet Error");
		return false;
	}

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(SpeedPacket.vid);

	if (!pInstance)
		return true;

	return true;
}

bool CPythonNetworkStream::SendAttackPacketFromThread(UINT uMotAttack, DWORD dwVIDVictim, THREAD_ARGS)
{
	if (!__CanActMainInstance())
		return true;

	TraceError("%s %d %s", file, line, func);
#ifdef ATTACK_TIME_LOG
	static DWORD prevTime = timeGetTime();
	DWORD curTime = timeGetTime();
	TraceError("TIME: %.4f(%.4f) ATTACK_PACKET: %d TARGET: %d", curTime/1000.0f, (curTime-prevTime)/1000.0f, uMotAttack, dwVIDVictim);
	prevTime = curTime;
#endif

	TPacketCGAttack kPacketAtk;

	kPacketAtk.header = HEADER_CG_ATTACK;
	kPacketAtk.bType = uMotAttack;
	kPacketAtk.dwVictimVID = dwVIDVictim;

	if (!SendSpecial(sizeof(kPacketAtk), &kPacketAtk))
	{
		Tracen("Send Battle Attack Packet Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendSpecial(int nLen, void * pvBuf)
{
	BYTE bHeader = *(BYTE *) pvBuf;

	switch (bHeader)
	{
		case HEADER_CG_ATTACK:
			{
				TPacketCGAttack * pkPacketAtk = (TPacketCGAttack *) pvBuf;
				pkPacketAtk->bCRCMagicCubeProcPiece = GetProcessCRCMagicCubePiece();
				pkPacketAtk->bCRCMagicCubeFilePiece = GetProcessCRCMagicCubePiece();
				return Send(nLen, pvBuf);
			}
			break;
	}

	return Send(nLen, pvBuf);
}

bool CPythonNetworkStream::RecvAddFlyTargetingPacket()
{
	TPacketGCFlyTargeting kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	__GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

	Tracef("VID [%d] Added to target settings\n",kPacket.dwShooterVID);

	CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	CInstanceBase * pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

	if (!pShooter)
	{
#ifndef __TEST__
		TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - dwShooterVID[%d] NOT EXIST", kPacket.dwShooterVID);
#endif
		return true;
	}

	CInstanceBase * pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

	if (kPacket.dwTargetVID && pTarget)
	{
		pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(pTarget->GetGraphicThingInstancePtr());
	}
	else
	{
		float h = CPythonBackground::Instance().GetHeight(kPacket.lX,kPacket.lY) + 60.0f; 
		pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(D3DXVECTOR3(kPacket.lX,kPacket.lY,h));

	}

	return true;
}

bool CPythonNetworkStream::RecvFlyTargetingPacket()
{
	TPacketGCFlyTargeting kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	__GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

	CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	CInstanceBase * pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

	if (!pShooter)
	{
#ifdef __TEST__
		TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - dwShooterVID[%d] NOT EXIST", kPacket.dwShooterVID);
#endif
		return true;
	}

	CInstanceBase * pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

	if (kPacket.dwTargetVID && pTarget)
	{
		pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(pTarget->GetGraphicThingInstancePtr());
	}
	else
	{
		float h = CPythonBackground::Instance().GetHeight(kPacket.lX, kPacket.lY) + 60.0f; 
		pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(D3DXVECTOR3(kPacket.lX,kPacket.lY,h));

	}

	return true;
}

bool CPythonNetworkStream::SendShootPacket(UINT uSkill)
{
	TPacketCGShoot kPacketShoot;
	kPacketShoot.bHeader=HEADER_CG_SHOOT;
	kPacketShoot.bType=uSkill;

	if (!Send(sizeof(kPacketShoot), &kPacketShoot))
	{
		Tracen("SendShootPacket Error");
		return false;
	}

	return true;
}

#ifdef ENABLE_PARTY_MATCH
bool CPythonNetworkStream::GroupMatchh(BYTE index, BYTE ayar)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGGrup packet;
	packet.header = HEADER_CG_PARTY_MATCH;
	packet.index = index;
	packet.ayar = ayar;
	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Error in CPythonNetworkStream::ENABLE_PARTY_MATCH");
		return false;
	}

	return true;
}
#endif

bool CPythonNetworkStream::SendAddFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition & kPPosTarget)
{
	TPacketCGFlyTargeting packet;

	packet.bHeader	= HEADER_CG_ADD_FLY_TARGETING;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = kPPosTarget.x;
	packet.lY = kPPosTarget.y;

	__LocalPositionToGlobalPosition(packet.lX, packet.lY);

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send FlyTargeting Packet Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition & kPPosTarget)
{
	TPacketCGFlyTargeting packet;

	packet.bHeader	= HEADER_CG_FLY_TARGETING;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = kPPosTarget.x;
	packet.lY = kPPosTarget.y;

	__LocalPositionToGlobalPosition(packet.lX, packet.lY);

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send FlyTargeting Packet Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvCreateFlyPacket()
{
	TPacketGCCreateFly kPacket;
	if (!Recv(sizeof(TPacketGCCreateFly), &kPacket))
		return false;

	CFlyingManager& rkFlyMgr = CFlyingManager::Instance();
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase * pkStartInst = rkChrMgr.GetInstancePtr(kPacket.dwStartVID);
	CInstanceBase * pkEndInst = rkChrMgr.GetInstancePtr(kPacket.dwEndVID);
	if (!pkStartInst || !pkEndInst)
		return true;

	rkFlyMgr.CreateIndexedFly(kPacket.bType, pkStartInst->GetGraphicThingInstancePtr(), pkEndInst->GetGraphicThingInstancePtr());

	return true;
}

bool CPythonNetworkStream::SendTargetPacket(DWORD dwVID)
{
	TPacketCGTarget packet;
	packet.header = HEADER_CG_TARGET;
	packet.dwVID = dwVID;

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send Target Packet Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendSyncPositionElementPacket(DWORD dwVictimVID, DWORD dwVictimX, DWORD dwVictimY)
{
	TPacketCGSyncPositionElement kSyncPos;
	kSyncPos.dwVID=dwVictimVID;
	kSyncPos.lX=dwVictimX;
	kSyncPos.lY=dwVictimY;

	__LocalPositionToGlobalPosition(kSyncPos.lX, kSyncPos.lY);

	if (!Send(sizeof(kSyncPos), &kSyncPos))
	{
		Tracen("CPythonNetworkStream::SendSyncPositionElementPacket - ERROR");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvMessenger()
{
    TPacketGCMessenger p;
	if (!Recv(sizeof(p), &p))
		return false;

	int iSize = p.size - sizeof(p);
	char char_name[24+1];

	switch (p.subheader)
	{
		case MESSENGER_SUBHEADER_GC_LIST:
		{
			TPacketGCMessengerListOnline on;
			while(iSize)
			{
				if (!Recv(sizeof(TPacketGCMessengerListOffline),&on))
					return false;

				if (!Recv(on.length, char_name))
					return false;

				char_name[on.length] = 0;

				if (on.connected & MESSENGER_CONNECTED_STATE_ONLINE)
					CPythonMessenger::Instance().OnFriendLogin(char_name);
				else
					CPythonMessenger::Instance().OnFriendLogout(char_name);

				iSize -= sizeof(TPacketGCMessengerListOffline);
				iSize -= on.length;
			}
			break;
		}

#ifdef ENABLE_MESSENGER_BLOCK
		case MESSENGER_SUBHEADER_GC_BLOCK_LIST:
		{
			TPacketGCMessengerBlockListOnline onn;
			while(iSize)
			{
				if (!Recv(sizeof(TPacketGCMessengerBlockListOffline),&onn))
					return false;

				if (!Recv(onn.length, char_name))
					return false;

				char_name[onn.length] = 0;

				if (onn.connected & MESSENGER_CONNECTED_STATE_ONLINE)
					CPythonMessenger::Instance().OnBlockLogin(char_name);
				else
					CPythonMessenger::Instance().OnBlockLogout(char_name);

				iSize -= sizeof(TPacketGCMessengerBlockListOffline);
				iSize -= onn.length;
			}
			break;
		}
		
		case MESSENGER_SUBHEADER_GC_BLOCK_LOGIN:
		{
			TPacketGCMessengerLogin pp;
			if (!Recv(sizeof(pp),&pp))
				return false;
			if (!Recv(pp.length, char_name))
				return false;
			char_name[pp.length] = 0;
			CPythonMessenger::Instance().OnBlockLogin(char_name);
			__RefreshTargetBoardByName(char_name);
			break;
		}

		case MESSENGER_SUBHEADER_GC_BLOCK_LOGOUT:
		{
			TPacketGCMessengerLogout logout2;
			if (!Recv(sizeof(logout2),&logout2))
				return false;
			if (!Recv(logout2.length, char_name))
				return false;
			char_name[logout2.length] = 0;
			CPythonMessenger::Instance().OnBlockLogout(char_name);
			break;
		}
#endif

		case MESSENGER_SUBHEADER_GC_LOGIN:
		{
			TPacketGCMessengerLogin p;
			if (!Recv(sizeof(p),&p))
				return false;
			if (!Recv(p.length, char_name))
				return false;
			char_name[p.length] = 0;
			CPythonMessenger::Instance().OnFriendLogin(char_name);
			__RefreshTargetBoardByName(char_name);
			break;
		}

		case MESSENGER_SUBHEADER_GC_LOGOUT:
		{
			TPacketGCMessengerLogout logout;
			if (!Recv(sizeof(logout),&logout))
				return false;
			if (!Recv(logout.length, char_name))
				return false;
			char_name[logout.length] = 0;
			CPythonMessenger::Instance().OnFriendLogout(char_name);
			break;
		}
	}
	return true;
}

bool CPythonNetworkStream::SendPartyInvitePacket(DWORD dwVID)
{
	TPacketCGPartyInvite kPartyInvitePacket;
	kPartyInvitePacket.header = HEADER_CG_PARTY_INVITE;
	kPartyInvitePacket.vid = dwVID;

	if (!Send(sizeof(kPartyInvitePacket), &kPartyInvitePacket))
	{
		Tracenf("CPythonNetworkStream::SendPartyInvitePacket [%ud] - PACKET SEND ERROR", dwVID);
		return false;
	}

	Tracef(" << SendPartyInvitePacket : %d\n", dwVID);
	return true;
}

bool CPythonNetworkStream::SendPartyInviteAnswerPacket(DWORD dwLeaderVID, BYTE byAnswer)
{
	TPacketCGPartyInviteAnswer kPartyInviteAnswerPacket;
	kPartyInviteAnswerPacket.header = HEADER_CG_PARTY_INVITE_ANSWER;
	kPartyInviteAnswerPacket.leader_pid = dwLeaderVID;
	kPartyInviteAnswerPacket.accept = byAnswer;

	if (!Send(sizeof(kPartyInviteAnswerPacket), &kPartyInviteAnswerPacket))
	{
		Tracenf("CPythonNetworkStream::SendPartyInviteAnswerPacket [%ud %ud] - PACKET SEND ERROR", dwLeaderVID, byAnswer);
		return false;
	}

	Tracef(" << SendPartyInviteAnswerPacket : %d, %d\n", dwLeaderVID, byAnswer);
	return true;
}

bool CPythonNetworkStream::SendPartyRemovePacket(DWORD dwPID)
{
	TPacketCGPartyRemove kPartyInviteRemove;
	kPartyInviteRemove.header = HEADER_CG_PARTY_REMOVE;
	kPartyInviteRemove.pid = dwPID;

	if (!Send(sizeof(kPartyInviteRemove), &kPartyInviteRemove))
	{
		Tracenf("CPythonNetworkStream::SendPartyRemovePacket [%ud] - PACKET SEND ERROR", dwPID);
		return false;
	}

	Tracef(" << SendPartyRemovePacket : %d\n", dwPID);
	return true;
}

bool CPythonNetworkStream::SendPartySetStatePacket(DWORD dwVID, BYTE byState, BYTE byFlag)
{
	TPacketCGPartySetState kPartySetState;
	kPartySetState.byHeader = HEADER_CG_PARTY_SET_STATE;
	kPartySetState.dwVID = dwVID;
	kPartySetState.byState = byState;
	kPartySetState.byFlag = byFlag;

	if (!Send(sizeof(kPartySetState), &kPartySetState))
	{
		Tracenf("CPythonNetworkStream::SendPartySetStatePacket(%ud, %ud) - PACKET SEND ERROR", dwVID, byState);
		return false;
	}

	Tracef(" << SendPartySetStatePacket : %d, %d, %d\n", dwVID, byState, byFlag);
	return true;
}

bool CPythonNetworkStream::SendPartyUseSkillPacket(BYTE bySkillIndex, DWORD dwVID)
{
	TPacketCGPartyUseSkill kPartyUseSkill;
	kPartyUseSkill.byHeader = HEADER_CG_PARTY_USE_SKILL;
	kPartyUseSkill.bySkillIndex = bySkillIndex;
	kPartyUseSkill.dwTargetVID = dwVID;

	if (!Send(sizeof(kPartyUseSkill), &kPartyUseSkill))
	{
		Tracenf("CPythonNetworkStream::SendPartyUseSkillPacket(%ud, %ud) - PACKET SEND ERROR", bySkillIndex, dwVID);
		return false;
	}

	Tracef(" << SendPartyUseSkillPacket : %d, %d\n", bySkillIndex, dwVID);
	return true;
}

bool CPythonNetworkStream::SendPartyParameterPacket(BYTE byDistributeMode)
{
	TPacketCGPartyParameter kPartyParameter;
	kPartyParameter.bHeader = HEADER_CG_PARTY_PARAMETER;
	kPartyParameter.bDistributeMode = byDistributeMode;

	if (!Send(sizeof(kPartyParameter), &kPartyParameter))
	{
		Tracenf("CPythonNetworkStream::SendPartyParameterPacket(%d) - PACKET SEND ERROR", byDistributeMode);
		return false;
	}

	Tracef(" << SendPartyParameterPacket : %d\n", byDistributeMode);
	return true;
}

bool CPythonNetworkStream::RecvPartyInvite()
{
	TPacketGCPartyInvite kPartyInvitePacket;
	if (!Recv(sizeof(kPartyInvitePacket), &kPartyInvitePacket))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kPartyInvitePacket.leader_pid);
	if (!pInstance)
	{
		TraceError(" CPythonNetworkStream::RecvPartyInvite - Failed to find leader instance [%d]\n", kPartyInvitePacket.leader_pid);
		return true;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RecvPartyInviteQuestion", Py_BuildValue("(is)", kPartyInvitePacket.leader_pid, pInstance->GetNameString()));
	Tracef(" >> RecvPartyInvite : %d, %s\n", kPartyInvitePacket.leader_pid, pInstance->GetNameString());

	return true;
}

bool CPythonNetworkStream::RecvPartyAdd()
{
	TPacketGCPartyAdd kPartyAddPacket;
	if (!Recv(sizeof(kPartyAddPacket), &kPartyAddPacket))
		return false;

	CPythonPlayer::Instance().AppendPartyMember(kPartyAddPacket.pid, kPartyAddPacket.name);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddPartyMember", Py_BuildValue("(is)", kPartyAddPacket.pid, kPartyAddPacket.name));
	Tracef(" >> RecvPartyAdd : %d, %s\n", kPartyAddPacket.pid, kPartyAddPacket.name);

	return true;
}

bool CPythonNetworkStream::RecvPartyUpdate()
{
	TPacketGCPartyUpdate kPartyUpdatePacket;
	if (!Recv(sizeof(kPartyUpdatePacket), &kPartyUpdatePacket))
		return false;

	CPythonPlayer::TPartyMemberInfo * pPartyMemberInfo;
	if (!CPythonPlayer::Instance().GetPartyMemberPtr(kPartyUpdatePacket.pid, &pPartyMemberInfo))
		return true;

	BYTE byOldState = pPartyMemberInfo->byState;

	CPythonPlayer::Instance().UpdatePartyMemberInfo(kPartyUpdatePacket.pid, kPartyUpdatePacket.state, kPartyUpdatePacket.percent_hp);
	for (int i = 0; i < PARTY_AFFECT_SLOT_MAX_NUM; ++i)
	{
		CPythonPlayer::Instance().UpdatePartyMemberAffect(kPartyUpdatePacket.pid, i, kPartyUpdatePacket.affects[i]);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UpdatePartyMemberInfo", Py_BuildValue("(i)", kPartyUpdatePacket.pid));

	DWORD dwVID;
	if (CPythonPlayer::Instance().PartyMemberPIDToVID(kPartyUpdatePacket.pid, &dwVID))
	if (byOldState != kPartyUpdatePacket.state)
	{
		__RefreshTargetBoardByVID(dwVID);
	}

	return true;
}

bool CPythonNetworkStream::RecvPartyRemove()
{
	TPacketGCPartyRemove kPartyRemovePacket;
	if (!Recv(sizeof(kPartyRemovePacket), &kPartyRemovePacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RemovePartyMember", Py_BuildValue("(i)", kPartyRemovePacket.pid));
	Tracef(" >> RecvPartyRemove : %d\n", kPartyRemovePacket.pid);

	return true;
}

bool CPythonNetworkStream::RecvPartyLink()
{
	TPacketGCPartyLink kPartyLinkPacket;
	if (!Recv(sizeof(kPartyLinkPacket), &kPartyLinkPacket))
		return false;

	CPythonPlayer::Instance().LinkPartyMember(kPartyLinkPacket.pid, kPartyLinkPacket.vid);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "LinkPartyMember", Py_BuildValue("(ii)", kPartyLinkPacket.pid, kPartyLinkPacket.vid));
	Tracef(" >> RecvPartyLink : %d, %d\n", kPartyLinkPacket.pid, kPartyLinkPacket.vid);

	return true;
}

bool CPythonNetworkStream::RecvPartyUnlink()
{
	TPacketGCPartyUnlink kPartyUnlinkPacket;
	if (!Recv(sizeof(kPartyUnlinkPacket), &kPartyUnlinkPacket))
		return false;

	CPythonPlayer::Instance().UnlinkPartyMember(kPartyUnlinkPacket.pid);

	if (CPythonPlayer::Instance().IsMainCharacterIndex(kPartyUnlinkPacket.vid))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkAllPartyMember", Py_BuildValue("()"));
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkPartyMember", Py_BuildValue("(i)", kPartyUnlinkPacket.pid));
	}

	Tracef(" >> RecvPartyUnlink : %d, %d\n", kPartyUnlinkPacket.pid, kPartyUnlinkPacket.vid);

	return true;
}

bool CPythonNetworkStream::RecvPartyParameter()
{
	TPacketGCPartyParameter kPartyParameterPacket;
	if (!Recv(sizeof(kPartyParameterPacket), &kPartyParameterPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ChangePartyParameter", Py_BuildValue("(i)", kPartyParameterPacket.bDistributeMode));
	Tracef(" >> RecvPartyParameter : %d\n", kPartyParameterPacket.bDistributeMode);

	return true;
}

bool CPythonNetworkStream::SendGuildAddMemberPacket(DWORD dwVID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_ADD_MEMBER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwVID), &dwVID))
		return false;

	Tracef(" SendGuildAddMemberPacket\n", dwVID);
	return true;
}

bool CPythonNetworkStream::SendGuildRemoveMemberPacket(DWORD dwPID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_REMOVE_MEMBER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwPID), &dwPID))
		return false;

	Tracef(" SendGuildRemoveMemberPacket %d\n", dwPID);
	return true;
}

bool CPythonNetworkStream::SendGuildChangeGradeNamePacket(BYTE byGradeNumber, const char * c_szName)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(byGradeNumber), &byGradeNumber))
		return false;

	char szName[GUILD_GRADE_NAME_MAX_LEN+1];
	strncpy(szName, c_szName, GUILD_GRADE_NAME_MAX_LEN);
	szName[GUILD_GRADE_NAME_MAX_LEN] = '\0';

	if (!Send(sizeof(szName), &szName))
		return false;

	Tracef(" SendGuildChangeGradeNamePacket %d, %s\n", byGradeNumber, c_szName);
	return true;
}

bool CPythonNetworkStream::SendGuildChangeGradeAuthorityPacket(BYTE byGradeNumber, BYTE byAuthority)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(byGradeNumber), &byGradeNumber))
		return false;
	if (!Send(sizeof(byAuthority), &byAuthority))
		return false;

	Tracef(" SendGuildChangeGradeAuthorityPacket %d, %d\n", byGradeNumber, byAuthority);
	return true;
}

bool CPythonNetworkStream::SendGuildOfferPacket(DWORD dwExperience)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_OFFER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwExperience), &dwExperience))
		return false;

	Tracef(" SendGuildOfferPacket %d\n", dwExperience);
	return true;
}

bool CPythonNetworkStream::SendGuildPostCommentPacket(const char * c_szMessage)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_POST_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	BYTE bySize = BYTE(strlen(c_szMessage)) + 1;
	if (!Send(sizeof(bySize), &bySize))
		return false;
	if (!Send(bySize, c_szMessage))
		return false;

	Tracef(" SendGuildPostCommentPacket %d, %s\n", bySize, c_szMessage);
	return true;
}

bool CPythonNetworkStream::SendGuildDeleteCommentPacket(DWORD dwIndex)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_DELETE_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwIndex), &dwIndex))
		return false;

	Tracef(" SendGuildDeleteCommentPacket %d\n", dwIndex);
	return true;
}

bool CPythonNetworkStream::SendGuildRefreshCommentsPacket(DWORD dwHighestIndex)
{
	static DWORD s_LastTime = timeGetTime() - 1001;

	if (timeGetTime() - s_LastTime < 1000)
		return true;
	s_LastTime = timeGetTime();

	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_REFRESH_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	Tracef(" SendGuildRefreshCommentPacket %d\n", dwHighestIndex);
	return true;
}

bool CPythonNetworkStream::SendGuildChangeMemberGradePacket(DWORD dwPID, BYTE byGrade)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwPID), &dwPID))
		return false;
	if (!Send(sizeof(byGrade), &byGrade))
		return false;

	Tracef(" SendGuildChangeMemberGradePacket %d, %d\n", dwPID, byGrade);
	return true;
}

bool CPythonNetworkStream::SendGuildUseSkillPacket(DWORD dwSkillID, DWORD dwTargetVID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_USE_SKILL;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwSkillID), &dwSkillID))
		return false;
	if (!Send(sizeof(dwTargetVID), &dwTargetVID))
		return false;

	Tracef(" SendGuildUseSkillPacket %d, %d\n", dwSkillID, dwTargetVID);
	return true;
}

bool CPythonNetworkStream::SendGuildChangeMemberGeneralPacket(DWORD dwPID, BYTE byFlag)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwPID), &dwPID))
		return false;
	if (!Send(sizeof(byFlag), &byFlag))
		return false;

	Tracef(" SendGuildChangeMemberGeneralFlagPacket %d, %d\n", dwPID, byFlag);
	return true;
}

bool CPythonNetworkStream::SendGuildInviteAnswerPacket(DWORD dwGuildID, BYTE byAnswer)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwGuildID), &dwGuildID))
		return false;
	if (!Send(sizeof(byAnswer), &byAnswer))
		return false;

	Tracef(" SendGuildInviteAnswerPacket %d, %d\n", dwGuildID, byAnswer);
	return true;
}

bool CPythonNetworkStream::SendGuildChargeGSPPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHARGE_GSP;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildChargeGSPPacket %d\n", dwMoney);
	return true;
}

bool CPythonNetworkStream::SendGuildDepositMoneyPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_DEPOSIT_MONEY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildDepositMoneyPacket %d\n", dwMoney);
	return true;
}

bool CPythonNetworkStream::SendGuildWithdrawMoneyPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_WITHDRAW_MONEY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildWithdrawMoneyPacket %d\n", dwMoney);
	return true;
}

bool CPythonNetworkStream::RecvGuild()
{
    TPacketGCGuild GuildPacket;
	if (!Recv(sizeof(GuildPacket), &GuildPacket))
		return false;

	switch(GuildPacket.subheader)
	{
		case GUILD_SUBHEADER_GC_LOGIN:
		{
			DWORD dwPID;
			if (!Recv(sizeof(DWORD), &dwPID))
				return false;

			CPythonGuild::TGuildMemberData * pGuildMemberData;
			if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
				if (0 != pGuildMemberData->strName.compare(CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().LoginGuildMember(pGuildMemberData->strName.c_str());

			break;
		}
		case GUILD_SUBHEADER_GC_LOGOUT:
		{
			DWORD dwPID;
			if (!Recv(sizeof(DWORD), &dwPID))
				return false;

			CPythonGuild::TGuildMemberData * pGuildMemberData;
			if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
				if (0 != pGuildMemberData->strName.compare(CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().LogoutGuildMember(pGuildMemberData->strName.c_str());

			break;
		}
		case GUILD_SUBHEADER_GC_REMOVE:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;

			if (CPythonGuild::Instance().IsMainPlayer(dwPID))
			{
				CPythonGuild::Instance().Destroy();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "DeleteGuild", Py_BuildValue("()"));
				CPythonMessenger::Instance().RemoveAllGuildMember();
				__SetGuildID(0);
				__RefreshMessengerWindow();
				__RefreshTargetBoard();
				__RefreshCharacterWindow();
			}
			else
			{

				std::string strMemberName = "";
				CPythonGuild::TGuildMemberData * pData;
				if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pData))
				{
					strMemberName = pData->strName;
					CPythonMessenger::Instance().RemoveGuildMember(pData->strName.c_str());
				}

				CPythonGuild::Instance().RemoveMember(dwPID);

				__RefreshTargetBoardByName(strMemberName.c_str());
				__RefreshGuildWindowMemberPage();
			}

			Tracef(" <Remove> %d\n", dwPID);
			break;
		}
		case GUILD_SUBHEADER_GC_LIST:
		{
			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);

			for (; iPacketSize > 0;)
			{
				TPacketGCGuildSubMember memberPacket;
				if (!Recv(sizeof(memberPacket), &memberPacket))
					return false;

				char szName[CHARACTER_NAME_MAX_LEN+1] = "";
				if (memberPacket.byNameFlag)
				{
					if (!Recv(sizeof(szName), &szName))
						return false;

					iPacketSize -= CHARACTER_NAME_MAX_LEN+1;
				}
				else
				{
					CPythonGuild::TGuildMemberData * pMemberData;
					if (CPythonGuild::Instance().GetMemberDataPtrByPID(memberPacket.pid, &pMemberData))
					{
						strncpy(szName, pMemberData->strName.c_str(), CHARACTER_NAME_MAX_LEN);
					}
				}

				CPythonGuild::SGuildMemberData GuildMemberData;
				GuildMemberData.dwPID = memberPacket.pid;
				GuildMemberData.byGrade = memberPacket.byGrade;
				GuildMemberData.strName = szName;
				GuildMemberData.byJob = memberPacket.byJob;
				GuildMemberData.byLevel = memberPacket.byLevel;
				GuildMemberData.dwOffer = memberPacket.dwOffer;
				GuildMemberData.byGeneralFlag = memberPacket.byIsGeneral;
				CPythonGuild::Instance().RegisterMember(GuildMemberData);

				if (strcmp(szName, CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().AppendGuildMember(szName);

				__RefreshTargetBoardByName(szName);

				iPacketSize -= sizeof(memberPacket);
			}

			__RefreshGuildWindowInfoPage();
			__RefreshGuildWindowMemberPage();
			__RefreshMessengerWindow();
			__RefreshCharacterWindow();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE:
		{
			BYTE byCount;
			if (!Recv(sizeof(byCount), &byCount))
				return false;

			for (BYTE i = 0; i < byCount; ++ i)
			{
				BYTE byIndex;
				if (!Recv(sizeof(byCount), &byIndex))
					return false;
				TPacketGCGuildSubGrade GradePacket;
				if (!Recv(sizeof(GradePacket), &GradePacket))
					return false;

				CPythonGuild::Instance().SetGradeData(byIndex, CPythonGuild::SGuildGradeData(GradePacket.auth_flag, GradePacket.grade_name));

			}
			__RefreshGuildWindowGradePage();
			__RefreshGuildWindowMemberPageGradeComboBox();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE_NAME:
		{
			BYTE byGradeNumber;
			if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
				return false;

			char szGradeName[GUILD_GRADE_NAME_MAX_LEN+1] = "";
			if (!Recv(sizeof(szGradeName), &szGradeName))
				return false;

			CPythonGuild::Instance().SetGradeName(byGradeNumber, szGradeName);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade", Py_BuildValue("()"));

			Tracef(" <Change Grade Name> %d, %s\n", byGradeNumber, szGradeName);
			__RefreshGuildWindowGradePage();
			__RefreshGuildWindowMemberPageGradeComboBox();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE_AUTH:
		{
			BYTE byGradeNumber;
			if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
				return false;
			BYTE byAuthorityFlag;
			if (!Recv(sizeof(byAuthorityFlag), &byAuthorityFlag))
				return false;

			CPythonGuild::Instance().SetGradeAuthority(byGradeNumber, byAuthorityFlag);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade", Py_BuildValue("()"));

			Tracef(" <Change Grade Authority> %d, %d\n", byGradeNumber, byAuthorityFlag);
			__RefreshGuildWindowGradePage();
			break;
		}
		case GUILD_SUBHEADER_GC_INFO:
		{
			TPacketGCGuildInfo GuildInfo;
			if (!Recv(sizeof(GuildInfo), &GuildInfo))
				return false;

			CPythonGuild::Instance().EnableGuild();
			CPythonGuild::TGuildInfo & rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
			strncpy(rGuildInfo.szGuildName, GuildInfo.name, GUILD_NAME_MAX_LEN);
			rGuildInfo.szGuildName[GUILD_NAME_MAX_LEN] = '\0';

			rGuildInfo.dwGuildID = GuildInfo.guild_id;
			rGuildInfo.dwMasterPID = GuildInfo.master_pid;
			rGuildInfo.dwGuildLevel = GuildInfo.level;
			rGuildInfo.dwCurrentExperience = GuildInfo.exp;
			rGuildInfo.dwCurrentMemberCount = GuildInfo.member_count;
			rGuildInfo.dwMaxMemberCount = GuildInfo.max_member_count;
			rGuildInfo.dwGuildMoney = GuildInfo.gold;
			rGuildInfo.bHasLand = GuildInfo.hasLand;

			__RefreshGuildWindowInfoPage();
			break;
		}
		case GUILD_SUBHEADER_GC_COMMENTS:
		{
			BYTE byCount;
			if (!Recv(sizeof(byCount), &byCount))
				return false;

			CPythonGuild::Instance().ClearComment();

			for (BYTE i = 0; i < byCount; ++i)
			{
				DWORD dwCommentID;
				if (!Recv(sizeof(dwCommentID), &dwCommentID))
					return false;

				char szName[CHARACTER_NAME_MAX_LEN+1] = "";
				if (!Recv(sizeof(szName), &szName))
					return false;

				char szComment[GULID_COMMENT_MAX_LEN+1] = "";
				if (!Recv(sizeof(szComment), &szComment))
					return false;

				CPythonGuild::Instance().RegisterComment(dwCommentID, szName, szComment);
			}

			__RefreshGuildWindowBoardPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_EXP:
		{
			BYTE byLevel;
			if (!Recv(sizeof(byLevel), &byLevel))
				return false;
			DWORD dwEXP;
			if (!Recv(sizeof(dwEXP), &dwEXP))
				return false;
			CPythonGuild::Instance().SetGuildEXP(byLevel, dwEXP);
			Tracef(" <ChangeEXP> %d, %d\n", byLevel, dwEXP);
			__RefreshGuildWindowInfoPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;
			BYTE byGrade;
			if (!Recv(sizeof(byGrade), &byGrade))
				return false;
			CPythonGuild::Instance().ChangeGuildMemberGrade(dwPID, byGrade);
			Tracef(" <ChangeMemberGrade> %d, %d\n", dwPID, byGrade);
			__RefreshGuildWindowMemberPage();
			break;
		}
		case GUILD_SUBHEADER_GC_SKILL_INFO:
		{
			CPythonGuild::TGuildSkillData & rSkillData = CPythonGuild::Instance().GetGuildSkillDataRef();
			if (!Recv(sizeof(rSkillData.bySkillPoint), &rSkillData.bySkillPoint))
				return false;
			if (!Recv(sizeof(rSkillData.bySkillLevel), rSkillData.bySkillLevel))
				return false;
			if (!Recv(sizeof(rSkillData.wGuildPoint), &rSkillData.wGuildPoint))
				return false;
			if (!Recv(sizeof(rSkillData.wMaxGuildPoint), &rSkillData.wMaxGuildPoint))
				return false;

			Tracef(" <SkillInfo> %d / %d, %d\n", rSkillData.bySkillPoint, rSkillData.wGuildPoint, rSkillData.wMaxGuildPoint);
			__RefreshGuildWindowSkillPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;
			BYTE byFlag;
			if (!Recv(sizeof(byFlag), &byFlag))
				return false;

			CPythonGuild::Instance().ChangeGuildMemberGeneralFlag(dwPID, byFlag);
			Tracef(" <ChangeMemberGeneralFlag> %d, %d\n", dwPID, byFlag);
			__RefreshGuildWindowMemberPage();
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_INVITE:
		{
			DWORD dwGuildID;
			if (!Recv(sizeof(dwGuildID), &dwGuildID))
				return false;
			char szGuildName[GUILD_NAME_MAX_LEN+1];
			if (!Recv(GUILD_NAME_MAX_LEN, &szGuildName))
				return false;

			szGuildName[GUILD_NAME_MAX_LEN] = 0;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RecvGuildInviteQuestion", Py_BuildValue("(is)", dwGuildID, szGuildName));
			Tracef(" <Guild Invite> %d, %s\n", dwGuildID, szGuildName);
			break;
		}
		case GUILD_SUBHEADER_GC_WAR:
		{
			TPacketGCGuildWar kGuildWar;
			if (!Recv(sizeof(kGuildWar), &kGuildWar))
				return false;

			switch (kGuildWar.bWarState)
			{
				case GUILD_WAR_SEND_DECLARE:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_SEND_DECLARE\n");
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"BINARY_GuildWar_OnSendDeclare",
						Py_BuildValue("(i)", kGuildWar.dwGuildOpp)
					);
					break;
				case GUILD_WAR_RECV_DECLARE:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_RECV_DECLARE\n");
					PyCallClassMemberFunc( m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_GuildWar_OnRecvDeclare", Py_BuildValue("(ii)", kGuildWar.dwGuildOpp, kGuildWar.bType) );
					break;
				case GUILD_WAR_ON_WAR:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_ON_WAR : %d, %d\n", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp);
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"BINARY_GuildWar_OnStart",
						Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp)
					);
					CPythonGuild::Instance().StartGuildWar(kGuildWar.dwGuildOpp);
					break;
				case GUILD_WAR_END:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_END\n");
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"BINARY_GuildWar_OnEnd",
						Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp)
					);
					CPythonGuild::Instance().EndGuildWar(kGuildWar.dwGuildOpp);
					break;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_NAME:
		{
			DWORD dwID;
			char szGuildName[GUILD_NAME_MAX_LEN+1];

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);

			int nItemSize = sizeof(dwID) + GUILD_NAME_MAX_LEN;

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_NAME");

			for (; iPacketSize > 0;)
			{
				if (!Recv(sizeof(dwID), &dwID))
					return false;

				if (!Recv(GUILD_NAME_MAX_LEN, &szGuildName))
					return false;

				szGuildName[GUILD_NAME_MAX_LEN] = 0;

				CPythonGuild::Instance().RegisterGuildName(dwID, szGuildName);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_WAR_LIST:
		{
			DWORD dwSrcGuildID;
			DWORD dwDstGuildID;

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);
			int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_WAR_LIST");

			for (; iPacketSize > 0;)
			{
				if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
					return false;

				if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
					return false;

				Tracef(" >> GulidWarList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
				CInstanceBase::InsertGVGKey(dwSrcGuildID, dwDstGuildID);
				CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST:
		{
			DWORD dwSrcGuildID;
			DWORD dwDstGuildID;

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);
			int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST");

			for (; iPacketSize > 0;)
			{

				if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
					return false;

				if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
					return false;

				Tracef(" >> GulidWarEndList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
				CInstanceBase::RemoveGVGKey(dwSrcGuildID, dwDstGuildID);
				CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_WAR_POINT:
		{
			TPacketGuildWarPoint GuildWarPoint;
			if (!Recv(sizeof(GuildWarPoint), &GuildWarPoint))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
				"BINARY_GuildWar_OnRecvPoint",
				Py_BuildValue("(iii)", GuildWarPoint.dwGainGuildID, GuildWarPoint.dwOpponentGuildID, GuildWarPoint.lPoint)
			);
			break;
		}
		case GUILD_SUBHEADER_GC_MONEY_CHANGE:
		{
			DWORD dwMoney;
			if (!Recv(sizeof(dwMoney), &dwMoney))
				return false;

			CPythonGuild::Instance().SetGuildMoney(dwMoney);

			__RefreshGuildWindowInfoPage();
			Tracef(" >> Guild Money Change : %d\n", dwMoney);
			break;
		}
	}

	return true;
}

bool CPythonNetworkStream::SendFishingPacket(int iRotation)
{
	BYTE byHeader = HEADER_CG_FISHING;
	if (!Send(sizeof(byHeader), &byHeader))
		return false;
	BYTE byPacketRotation = iRotation / 5;
	if (!Send(sizeof(BYTE), &byPacketRotation))
		return false;

	return true;
}

bool CPythonNetworkStream::SendGiveItemPacket(DWORD dwTargetVID, TItemPos ItemPos, int iItemCount)
{
	TPacketCGGiveItem GiveItemPacket;
	GiveItemPacket.byHeader = HEADER_CG_GIVE_ITEM;
	GiveItemPacket.dwTargetVID = dwTargetVID;
	GiveItemPacket.ItemPos = ItemPos;
	GiveItemPacket.byItemCount = iItemCount;

	if (!Send(sizeof(GiveItemPacket), &GiveItemPacket))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvFishing()
{
	TPacketGCFishing FishingPacket;
	if (!Recv(sizeof(FishingPacket), &FishingPacket))
		return false;

	CInstanceBase * pFishingInstance = NULL;
	if (FISHING_SUBHEADER_GC_FISH != FishingPacket.subheader)
	{
		pFishingInstance = CPythonCharacterManager::Instance().GetInstancePtr(FishingPacket.info);
		if (!pFishingInstance)
			return true;
	}

	switch (FishingPacket.subheader)
	{
		case FISHING_SUBHEADER_GC_START:
			pFishingInstance->StartFishing(float(FishingPacket.dir) * 5.0f);
			break;
		case FISHING_SUBHEADER_GC_STOP:
			if (pFishingInstance->IsFishing())
				pFishingInstance->StopFishing();
			break;
		case FISHING_SUBHEADER_GC_REACT:
			if (pFishingInstance->IsFishing())
			{
				pFishingInstance->SetFishEmoticon(); 
				pFishingInstance->ReactFishing();
			}
			break;
		case FISHING_SUBHEADER_GC_SUCCESS:
			pFishingInstance->CatchSuccess();
			break;
		case FISHING_SUBHEADER_GC_FAIL:
			pFishingInstance->CatchFail();
			if (pFishingInstance == CPythonCharacterManager::Instance().GetMainInstancePtr())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingFailure", Py_BuildValue("()"));
			}
			break;
		case FISHING_SUBHEADER_GC_FISH:
		{
			DWORD dwFishID = FishingPacket.info;

			if (0 == FishingPacket.info)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingNotifyUnknown", Py_BuildValue("()"));
				return true;
			}

			CItemData * pItemData;
			if (!CItemManager::Instance().GetItemDataPointer(dwFishID, &pItemData))
				return true;

			CInstanceBase * pMainInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
			if (!pMainInstance)
				return true;

			if (pMainInstance->IsFishing())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingNotify", Py_BuildValue("(is)", CItemData::ITEM_TYPE_FISH == pItemData->GetType(), pItemData->GetName()));
			}
			else
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingSuccess", Py_BuildValue("(is)", CItemData::ITEM_TYPE_FISH == pItemData->GetType(), pItemData->GetName()));
			}
			break;
		}
	}

	return true;
}

bool CPythonNetworkStream::RecvDungeon()
{
	TPacketGCDungeon DungeonPacket;
	if (!Recv(sizeof(DungeonPacket), &DungeonPacket))
		return false;

	switch (DungeonPacket.subheader)
	{
		case DUNGEON_SUBHEADER_GC_TIME_ATTACK_START:
		{
			break;
		}
		case DUNGEON_SUBHEADER_GC_DESTINATION_POSITION:
		{
			unsigned long ulx, uly;
			if (!Recv(sizeof(ulx), &ulx))
				return false;
			if (!Recv(sizeof(uly), &uly))
				return false;

			CPythonPlayer::Instance().SetDungeonDestinationPosition(ulx, uly);
			break;
		}
	}

	return true;
}

bool CPythonNetworkStream::SendBuildPrivateShopPacket(const char * c_szName, const std::vector<TShopItemTable> & c_rSellingItemStock
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
, int iTime
#endif
)
{
	TPacketCGMyShop packet;
	packet.bHeader = HEADER_CG_MYSHOP;
	strncpy(packet.szSign, c_szName, SHOP_SIGN_MAX_LEN);
	packet.bCount = (BYTE)c_rSellingItemStock.size();
#ifdef ENABLE_MYSHOP_DECO
	IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
	packet.Deco.bDeco = rPlayer.MyShopDecoGetDeco();
	packet.Deco.bVnum = rPlayer.MyShopDecoGetVnum();
#endif
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
	packet.iTime = iTime;
#endif
	if (!Send(sizeof(packet), &packet))
		return false;

	for (std::vector<TShopItemTable>::const_iterator itor = c_rSellingItemStock.begin(); itor < c_rSellingItemStock.end(); ++itor)
	{
		const TShopItemTable & c_rItem = *itor;
		if (!Send(sizeof(c_rItem), &c_rItem))
			return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvShopSignPacket()
{
	TraceError("RecvShopSignPacket -1");
	TPacketGCShopSign p;
	if (!Recv(sizeof(TPacketGCShopSign), &p))
		return false;

	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
	
	TraceError("RecvShopSignPacket 0");
	if (strlen(p.szSign) == 0)
	{
		TraceError("RecvShopSignPacket 1");
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_PrivateShop_Disappear", Py_BuildValue("(i)", p.dwVID));
		if (rkPlayer.IsMainCharacterIndex(p.dwVID))
			rkPlayer.ClosePrivateShop();
	}
	else
	{
		TraceError("RecvShopSignPacket 2");
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(p.dwVID);
		if (pInstance && (pInstance->IsPC() || pInstance->IsPrivateShop()))
		{
#ifdef ENABLE_MYSHOP_DECO
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_PrivateShop_Appear", Py_BuildValue("(isi)", p.dwVID, p.szSign, p.Deco.bDeco));
#else
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_PrivateShop_Appear", Py_BuildValue("(is)", p.dwVID, p.szSign));
#endif
			if (rkPlayer.IsMainCharacterIndex(p.dwVID))
				rkPlayer.OpenPrivateShop();
		}
		else
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_PrivateShop_Disappear", Py_BuildValue("(i)", p.dwVID));
	}

	return true;
}

bool CPythonNetworkStream::RecvTimePacket()
{
	TPacketGCTime TimePacket;
	if (!Recv(sizeof(TimePacket), &TimePacket))
		return false;

	IAbstractApplication& rkApp=IAbstractApplication::GetSingleton();
	rkApp.SetServerTime(TimePacket.time);

	return true;
}

bool CPythonNetworkStream::RecvWalkModePacket()
{
	TPacketGCWalkMode WalkModePacket;
	if (!Recv(sizeof(WalkModePacket), &WalkModePacket))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(WalkModePacket.vid);
	if (pInstance)
	{
		if (WALKMODE_RUN == WalkModePacket.mode)
		{
			pInstance->SetRunMode();
		}
		else
		{
			pInstance->SetWalkMode();
		}
	}

	return true;
}

bool CPythonNetworkStream::RecvChangeSkillGroupPacket()
{
	TPacketGCChangeSkillGroup ChangeSkillGroup;
	if (!Recv(sizeof(ChangeSkillGroup), &ChangeSkillGroup))
		return false;

	m_dwMainActorSkillGroup = ChangeSkillGroup.skill_group;

	CPythonPlayer::Instance().NEW_ClearSkillData();
	__RefreshCharacterWindow();
	return true;
}

void CPythonNetworkStream::__TEST_SetSkillGroupFake(int iIndex)
{
	m_dwMainActorSkillGroup = DWORD(iIndex);

	CPythonPlayer::Instance().NEW_ClearSkillData();
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter", Py_BuildValue("()"));
}

bool CPythonNetworkStream::SendRefinePacket(BYTE byPos, BYTE byType, BYTE byWin)
{
	TPacketCGRefine kRefinePacket;
	kRefinePacket.header = HEADER_CG_REFINE;
	kRefinePacket.pos = byPos;
	kRefinePacket.type = byType;
	kRefinePacket.win = byWin;

	if (!Send(sizeof(kRefinePacket), &kRefinePacket))
		return false;

	return true;
}

bool CPythonNetworkStream::SendSelectItemPacket(DWORD dwItemPos)
{
	TPacketCGScriptSelectItem kScriptSelectItem;
	kScriptSelectItem.header = HEADER_CG_SCRIPT_SELECT_ITEM;
	kScriptSelectItem.selection = dwItemPos;

	if (!Send(sizeof(kScriptSelectItem), &kScriptSelectItem))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvRefineInformationPacket()
{
	TPacketGCRefineInformation kRefineInfoPacket;
	if (!Recv(sizeof(kRefineInfoPacket), &kRefineInfoPacket))
		return false;

	TRefineTable & rkRefineTable = kRefineInfoPacket.refine_table;
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
		"OpenRefineDialog",
		Py_BuildValue("(iiiii)",
			kRefineInfoPacket.pos,
			kRefineInfoPacket.refine_table.result_vnum,
			rkRefineTable.cost,
			rkRefineTable.prob,
			kRefineInfoPacket.type)
		);

	for (int i = 0; i < rkRefineTable.material_count; ++i)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AppendMaterialToRefineDialog", Py_BuildValue("(ii)", rkRefineTable.materials[i].vnum, rkRefineTable.materials[i].count));
	}

#ifdef __TEST__
	Tracef(" >> RecvRefineInformationPacket(pos=%d, result_vnum=%d, cost=%d, prob=%d, type=%d)\n",
														kRefineInfoPacket.pos,
														kRefineInfoPacket.refine_table.result_vnum,
														rkRefineTable.cost,
														rkRefineTable.prob,
														kRefineInfoPacket.type);
#endif

	return true;
}

bool CPythonNetworkStream::RecvNPCList()
{
	TPacketGCNPCPosition kNPCPosition;
	if (!Recv(sizeof(kNPCPosition), &kNPCPosition))
		return false;

	assert(int(kNPCPosition.size)-sizeof(kNPCPosition) == kNPCPosition.count*sizeof(TNPCPosition) && "HEADER_GC_NPC_POSITION");

	CPythonMiniMap::Instance().ClearAtlasMarkInfo();

	for (int i = 0; i < kNPCPosition.count; ++i)
	{
		TNPCPosition NPCPosition;
		if (!Recv(sizeof(TNPCPosition), &NPCPosition))
			return false;

		CPythonMiniMap::Instance().RegisterAtlasMark(NPCPosition.bType, NPCPosition.name, NPCPosition.x, NPCPosition.y);
	}

	return true;
}

bool CPythonNetworkStream::__SendCRCReportPacket()
{
	return true;
}

bool CPythonNetworkStream::SendClientVersionPacket()
{
	std::string filename;

	GetExcutedFileName(filename);

	filename = CFileNameHelper::NoPath(filename);
	CFileNameHelper::ChangeDosPath(filename);

	if (LocaleService_IsEUROPE() && false == LocaleService_IsYMIR())
	{
		TPacketCGClientVersion2 kVersionPacket;
		kVersionPacket.header = HEADER_CG_CLIENT_VERSION2;
		strncpy(kVersionPacket.filename, filename.c_str(), sizeof(kVersionPacket.filename)-1);
		strncpy(kVersionPacket.timestamp, "1215955205", sizeof(kVersionPacket.timestamp)-1); 

		if (!Send(sizeof(kVersionPacket), &kVersionPacket))
			Tracef("SendClientReportPacket Error");
	}
	else
	{
		TPacketCGClientVersion kVersionPacket;
		kVersionPacket.header = HEADER_CG_CLIENT_VERSION;
		strncpy(kVersionPacket.filename, filename.c_str(), sizeof(kVersionPacket.filename)-1);
		strncpy(kVersionPacket.timestamp, __TIMESTAMP__, sizeof(kVersionPacket.timestamp)-1);

		if (!Send(sizeof(kVersionPacket), &kVersionPacket))
			Tracef("SendClientReportPacket Error");
	}
	return true;
}

bool CPythonNetworkStream::RecvAffectAddPacket()
{
	TPacketGCAffectAdd kAffectAdd;
	if (!Recv(sizeof(kAffectAdd), &kAffectAdd))
		return false;

	TPacketAffectElement & rkElement = kAffectAdd.elem;
	if (rkElement.bPointIdxApplyOn == POINT_ENERGY)
	{
		CPythonPlayer::Instance().SetStatus (POINT_ENERGY_END_TIME, CPythonApplication::Instance().GetServerTimeStamp() + rkElement.lDuration);
		__RefreshStatus();
	}
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_AddAffect", Py_BuildValue("(iiii)", rkElement.dwType, rkElement.bPointIdxApplyOn, rkElement.lApplyValue, rkElement.lDuration));

	return true;
}

bool CPythonNetworkStream::RecvAffectRemovePacket()
{
	TPacketGCAffectRemove kAffectRemove;
	if (!Recv(sizeof(kAffectRemove), &kAffectRemove))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_RemoveAffect", Py_BuildValue("(ii)", kAffectRemove.dwType, kAffectRemove.bApplyOn));

	return true;
}

bool CPythonNetworkStream::RecvChannelPacket()
{
	TPacketGCChannel kChannelPacket;
	if (!Recv(sizeof(kChannelPacket), &kChannelPacket))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvViewEquipPacket()
{
	TPacketGCViewEquip kViewEquipPacket;
	if (!Recv(sizeof(kViewEquipPacket), &kViewEquipPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenEquipmentDialog", Py_BuildValue("(i)", kViewEquipPacket.dwVID));

	for (int i = 0; i < WEAR_MAX_NUM; ++i)
	{
		TEquipmentItemSet & rItemSet = kViewEquipPacket.equips[i];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogItem", Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i, rItemSet.vnum, rItemSet.count));

		for (int j = 0; j < ITEM_SOCKET_SLOT_MAX_NUM; ++j)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogSocket", Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i, j, rItemSet.alSockets[j]));

		for (int k = 0; k < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++k)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogAttr", Py_BuildValue("(iiiii)", kViewEquipPacket.dwVID, i, k, rItemSet.aAttr[k].bType, rItemSet.aAttr[k].sValue));
	}

	return true;
}

bool CPythonNetworkStream::RecvLandPacket()
{
	TPacketGCLandList kLandList;
	if (!Recv(sizeof(kLandList), &kLandList))
		return false;

	std::vector<DWORD> kVec_dwGuildID;

	CPythonMiniMap & rkMiniMap = CPythonMiniMap::Instance();
	CPythonBackground & rkBG = CPythonBackground::Instance();
	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();

	rkMiniMap.ClearGuildArea();
	rkBG.ClearGuildArea();

	int iPacketSize = (kLandList.size - sizeof(TPacketGCLandList));
	for (; iPacketSize > 0; iPacketSize-=sizeof(TLandPacketElement))
	{
		TLandPacketElement kElement;
		if (!Recv(sizeof(TLandPacketElement), &kElement))
			return false;

		rkMiniMap.RegisterGuildArea(kElement.dwID,
									kElement.dwGuildID,
									kElement.x,
									kElement.y,
									kElement.width,
									kElement.height);

		if (pMainInstance)
		if (kElement.dwGuildID == pMainInstance->GetGuildID())
		{
			rkBG.RegisterGuildArea(kElement.x,
								   kElement.y,
								   kElement.x+kElement.width,
								   kElement.y+kElement.height);
		}

		if (0 != kElement.dwGuildID)
			kVec_dwGuildID.push_back(kElement.dwGuildID);
	}

	if (kVec_dwGuildID.size()>0)
		__DownloadSymbol(kVec_dwGuildID);

	return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacket()
{
	TPacketGCTargetCreate kTargetCreate;
	if (!Recv(sizeof(kTargetCreate), &kTargetCreate))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	if (CREATE_TARGET_TYPE_LOCATION == kTargetCreate.byType)
	{
		rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName);
	}
	else
	{
		rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName, kTargetCreate.dwVID);
		rkpyBG.CreateTargetEffect(kTargetCreate.lID, kTargetCreate.dwVID);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenAtlasWindow", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvTargetUpdatePacket()
{
	TPacketGCTargetUpdate kTargetUpdate;
	if (!Recv(sizeof(kTargetUpdate), &kTargetUpdate))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.UpdateTarget(kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);

	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	rkpyBG.CreateTargetEffect(kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);

	return true;
}

bool CPythonNetworkStream::RecvTargetDeletePacket()
{
	TPacketGCTargetDelete kTargetDelete;
	if (!Recv(sizeof(kTargetDelete), &kTargetDelete))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.DeleteTarget(kTargetDelete.lID);

	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	rkpyBG.DeleteTargetEffect(kTargetDelete.lID);

	return true;
}

bool CPythonNetworkStream::RecvLoverInfoPacket()
{
	TPacketGCLoverInfo kLoverInfo;
	if (!Recv(sizeof(kLoverInfo), &kLoverInfo))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LoverInfo", Py_BuildValue("(si)", kLoverInfo.szName, kLoverInfo.byLovePoint));
#ifdef __TEST__
	Tracef("RECV LOVER INFO : %s, %d\n", kLoverInfo.szName, kLoverInfo.byLovePoint);
#endif
	return true;
}

bool CPythonNetworkStream::RecvLovePointUpdatePacket()
{
	TPacketGCLovePointUpdate kLovePointUpdate;
	if (!Recv(sizeof(kLovePointUpdate), &kLovePointUpdate))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_UpdateLovePoint", Py_BuildValue("(i)", kLovePointUpdate.byLovePoint));
#ifdef __TEST__
	Tracef("RECV LOVE POINT UPDATE : %d\n", kLovePointUpdate.byLovePoint);
#endif
	return true;
}

bool CPythonNetworkStream::RecvDigMotionPacket()
{
	TPacketGCDigMotion kDigMotion;
	if (!Recv(sizeof(kDigMotion), &kDigMotion))
		return false;

#ifdef __TEST__
	Tracef(" Dig Motion [%d/%d]\n", kDigMotion.vid, kDigMotion.count);
#endif

	IAbstractCharacterManager& rkChrMgr=IAbstractCharacterManager::GetSingleton();
	CInstanceBase * pkInstMain = rkChrMgr.GetInstancePtr(kDigMotion.vid);
	CInstanceBase * pkInstTarget = rkChrMgr.GetInstancePtr(kDigMotion.target_vid);
	if (NULL == pkInstMain)
		return true;

	if (pkInstTarget)
		pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);

	for (int i = 0; i < kDigMotion.count; ++i)
		pkInstMain->PushOnceMotion(CRaceMotionData::NAME_DIG);

	return true;
}

bool CPythonNetworkStream::SendDragonSoulRefinePacket(BYTE bRefineType, TItemPos* pos)
{
	TPacketCGDragonSoulRefine pk;
	pk.header = HEADER_CG_DRAGON_SOUL_REFINE;
	pk.bSubType = bRefineType;
	memcpy (pk.ItemGrid, pos, sizeof (TItemPos) * DS_REFINE_WINDOW_MAX_NUM);
	if (!Send(sizeof (pk), &pk))
	{
		return false;
	}
	return true;
}

#ifdef ENABLE_GEM_SYSTEM
bool CPythonNetworkStream::SendGemShopBuy(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGGemShop PacketGemShop;
	PacketGemShop.header = HEADER_CG_GEM_SHOP;
	PacketGemShop.subheader = GEM_SHOP_SUBHEADER_CG_BUY;
	
	if (!Send(sizeof(PacketGemShop), &PacketGemShop))
	{
		Tracef("SendGemShopBuyPacket Error\n");
		return false;
	}

	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendGemShopBuyPacket Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendSlotAdd()
{
	if (!__CanActMainInstance())
		return true;

	BYTE bPos = CPythonPlayer::Instance().GetGemShopOpenSlotCount();
	
	TPacketCGGemShop PacketGemShop;
	PacketGemShop.header = HEADER_CG_GEM_SHOP;
	PacketGemShop.subheader = GEM_SHOP_SUBHEADER_CG_ADD;
	
	if (!Send(sizeof(PacketGemShop), &PacketGemShop))
	{
		Tracef("SendGemShopAddPacket Error\n");
		return false;
	}

	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendGemShopAddPacket Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendRequestRefresh()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGGemShop PacketGemShop;
	PacketGemShop.header = HEADER_CG_GEM_SHOP;
	PacketGemShop.subheader = GEM_SHOP_SUBHEADER_CG_REFRESH;
	
	if (!Send(sizeof(PacketGemShop), &PacketGemShop))
	{
		Tracef("SendGemShopAddPacket Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendGemShopClose()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGGemShop PacketGemShop;
	PacketGemShop.header = HEADER_CG_GEM_SHOP;
	PacketGemShop.subheader = GEM_SHOP_SUBHEADER_CG_CLOSE;
	
	if (!Send(sizeof(PacketGemShop), &PacketGemShop))
	{
		Tracef("SendGemShopAddPacket Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvGemShop()
{
	TPacketGCGemShop kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	switch(kPacket.subheader)
	{
		case GEM_SHOP_SUBHEADER_GC_BUY:
			{
				BYTE bPos;
				if (!Recv(sizeof(bPos), &bPos))
					return false;

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "GemShopSlotBuy", Py_BuildValue("(ii)", bPos, kPacket.shopItems[bPos].bEnable));
				return true;
			}
		case GEM_SHOP_SUBHEADER_GC_ADD:
			{
				BYTE bPos;
				if (!Recv(sizeof(bPos), &bPos))
					return false;

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "GemShopSlotAdd", Py_BuildValue("(ii)", bPos, kPacket.shopItems[bPos].bEnable));
				return true;
			}
		case GEM_SHOP_SUBHEADER_GC_OPEN:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenGemShop", Py_BuildValue("()"));
			return true;
		case GEM_SHOP_SUBHEADER_GC_CLOSE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseGemShop", Py_BuildValue("()"));
			return true;
		case GEM_SHOP_SUBHEADER_GC_REFRESH:
			{
				CPythonPlayer::Instance().ClearGemShopItemVector();
				BYTE bSlotCount = 0;
				if (!Recv(sizeof(bSlotCount), &bSlotCount))
					return false;

				for (int i = 0; i < 9; ++i)
				{
					TGemShopItem GemItem;
					GemItem.bEnable = kPacket.shopItems[i].bEnable;
				
					GemItem.dwVnum = kPacket.shopItems[i].dwVnum;
					GemItem.bCount = kPacket.shopItems[i].bCount;
					GemItem.dwPrice = kPacket.shopItems[i].dwPrice;

					CPythonPlayer::Instance().SetGemShopItemData(i, GemItem);
				}
			
				CPythonPlayer::Instance().SetGemShopRefreshTime(kPacket.RefreshTime);
				CPythonPlayer::Instance().SetGemShopOpenSlotCount(bSlotCount);

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGemShopWIndow", Py_BuildValue("()"));
				return true;
			}
		default:
			return false;
	}
}
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
bool CPythonNetworkStream::SendExtendInvenButtonClick(BYTE bIndex)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExtendInven kPacket;
	kPacket.header = HEADER_CG_EXTEND_INVEN;
	kPacket.subheader = EXTEND_INVEN_SUBHEADER_GC_CLICK;
	kPacket.bIndex = bIndex;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracef("SendExtendInvenButtonClick Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendExtendInvenUpgrade()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExtendInven kPacket;
	kPacket.header = HEADER_CG_EXTEND_INVEN;
	kPacket.subheader = EXTEND_INVEN_SUBHEADER_GC_UPGRADE;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracef("SendExtendInvenUpgrade Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvExtendInven()
{
	TPacketGCExtendInven kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ExInvenItemUseMsg", Py_BuildValue("(ii)", kPacket.bMsg, kPacket.bNeed));
	
	return true;
}
#endif

#ifdef ENABLE_MINI_GAME_RUMI
bool CPythonNetworkStream::SendMiniGameRumiStart()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMiniGameRumi kPacket;
	kPacket.header = HEADER_CG_MINI_GAME_RUMI;
	kPacket.subheader = MINI_GAME_RUMI_SUBHEADER_CG_START;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracef("SendMiniGameRumiStart Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendMiniGameRumiExit()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMiniGameRumi kPacket;
	kPacket.header = HEADER_CG_MINI_GAME_RUMI;
	kPacket.subheader = MINI_GAME_RUMI_SUBHEADER_CG_EXIT;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracef("SendMiniGameRumiExit Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendMiniGameRumiHandCardClick(BOOL bAdd, BYTE bIndex)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMiniGameRumi kPacket;
	kPacket.header = HEADER_CG_MINI_GAME_RUMI;
	kPacket.subheader = MINI_GAME_RUMI_SUBHEADER_CG_HAND_CARD_CLICK;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracef("SendMiniGameRumiHandCardClick Error\n");
		return false;
	}

	if (!Send(sizeof(bAdd), &bAdd))
	{
		Tracef("SendMiniGameRumiHandCardClick Error\n");
		return false;
	}

	if (!Send(sizeof(bIndex), &bIndex))
	{
		Tracef("SendMiniGameRumiHandCardClick Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendMiniGameRumiDeckCardClick()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMiniGameRumi kPacket;
	kPacket.header = HEADER_CG_MINI_GAME_RUMI;
	kPacket.subheader = MINI_GAME_RUMI_SUBHEADER_CG_DECK_CARD_CLICK;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracef("SendMiniGameRumiDeckCardClick Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendMiniGameRumiFieldCardClick(BYTE bIndex)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMiniGameRumi kPacket;
	kPacket.header = HEADER_CG_MINI_GAME_RUMI;
	kPacket.subheader = MINI_GAME_RUMI_SUBHEADER_CG_FIELD_CARD_CLICK;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracef("SendMiniGameRumiFieldCardClick Error\n");
		return false;
	}

	if (!Send(sizeof(bIndex), &bIndex))
	{
		Tracef("SendMiniGameRumiFieldCardClick Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvMiniGameRumi()
{
	TPacketGCMiniGameRumi kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	switch(kPacket.subheader)
	{
		case MINI_GAME_RUMI_SUBHEADER_GC_START:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameRumiStart", Py_BuildValue("()"));
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameRumiSetDeckCount", Py_BuildValue("(i)", 24));
			return true;

		case MINI_GAME_RUMI_SUBHEADER_GC_EXIT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameRumiEnd", Py_BuildValue("()"));
			return true;

		case MINI_GAME_RUMI_SUBHEADER_GC_HAND_CARD_CLICK:
		case MINI_GAME_RUMI_SUBHEADER_GC_DECK_CARD_CLICK:
		case MINI_GAME_RUMI_SUBHEADER_GC_FIELD_CARD_CLICK:
			TMiniGameRumiMove table;
			if (!Recv(sizeof(table), &table))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameRumiMoveCard", Py_BuildValue("(iiiiiiii)", table.src.bPos, table.src.bIndex, table.src.bColor, table.src.bNumber, table.dst.bPos, table.dst.bIndex, table.dst.bColor, table.dst.bNumber));
			return true;
		case MINI_GAME_RUMI_SUBHEADER_GC_SCORE_INCREASE:
			TMiniGameRumiScore tables;
			if (!Recv(sizeof(tables), &tables))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameRumiIncreaseScore", Py_BuildValue("(ii)", tables.bScore, tables.sTotalScore));
			return true;
		default:
			Tracef("RecvMiniGameRumi Unknown subheader:%d\n", kPacket.subheader);
	}
	
	return false;
}
#endif

#ifdef ENABLE_ACCE_SYSTEM
bool CPythonNetworkStream::RecvAccePacket(bool bReturn)
{
	TPacketAcce sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return bReturn;
	
	bReturn = true;
	switch (sPacket.subheader)
	{
		case ACCE_SUBHEADER_GC_OPEN:
			{
				CPythonAcce::Instance().Clear();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 1, sPacket.bWindow));
			}
			break;
		case ACCE_SUBHEADER_GC_CLOSE:
			{
				CPythonAcce::Instance().Clear();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 2, sPacket.bWindow));
			}
			break;
		case ACCE_SUBHEADER_GC_ADDED:
			{
				CPythonAcce::Instance().AddMaterial(sPacket.dwPrice, sPacket.bPos, sPacket.tPos);
				if (sPacket.bPos == 1)
				{
					CPythonAcce::Instance().AddResult(sPacket.dwItemVnum, sPacket.dwMinAbs, sPacket.dwMaxAbs);
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AlertAcce", Py_BuildValue("(b)", sPacket.bWindow));
				}
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 3, sPacket.bWindow));
			}
			break;
		case ACCE_SUBHEADER_GC_REMOVED:
			{
				CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, sPacket.bPos);
				if (sPacket.bPos == 0)
					CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 1);
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 4, sPacket.bWindow));
			}
			break;
		case ACCE_SUBHEADER_CG_REFINED:
			{
				if (sPacket.dwMaxAbs == 0)
					CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 1);
				else
				{
					CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 0);
					CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 1);
				}
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 4, sPacket.bWindow));
			}
			break;
		default:
			TraceError("CPythonNetworkStream::RecvAccePacket: unknown subheader %d\n.", sPacket.subheader);
			break;
	}
	
	return bReturn;
}

bool CPythonNetworkStream::SendAcceClosePacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketAcce sPacket;
	sPacket.header = HEADER_CG_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_CG_CLOSE;
	sPacket.dwPrice = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return true;
}

bool CPythonNetworkStream::SendAcceAddPacket(TItemPos tPos, BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketAcce sPacket;
	sPacket.header = HEADER_CG_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_CG_ADD;
	sPacket.dwPrice = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return true;
}

bool CPythonNetworkStream::SendAcceRemovePacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketAcce sPacket;
	sPacket.header = HEADER_CG_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_CG_REMOVE;
	sPacket.dwPrice = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return true;
}

bool CPythonNetworkStream::SendAcceRefinePacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketAcce sPacket;
	sPacket.header = HEADER_CG_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_CG_REFINE;
	sPacket.dwPrice = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return true;
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
bool CPythonNetworkStream::RecvChangeLookPacket()
{
	TPacketChangeLook sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return false;
	
	switch (sPacket.subheader)
	{
		case CL_SUBHEADER_OPEN:
			{
				CPythonChangeLook::Instance().Clear();
				CPythonChangeLook::Instance().SetCost(sPacket.dwCost);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 1));
			}
			break;
		case CL_SUBHEADER_CLOSE:
			{
				CPythonChangeLook::Instance().Clear();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 2));
			}
			break;
		case CL_SUBHEADER_ADD:
			{
				CPythonChangeLook::Instance().AddMaterial(sPacket.bPos, sPacket.tPos);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 3));
				if (sPacket.bPos == 1)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AlertChangeLook", Py_BuildValue("()"));
			}
			break;
		case CL_SUBHEADER_REMOVE:
			{
				if (sPacket.bPos == 1)
					CPythonChangeLook::Instance().RemoveMaterial(sPacket.bPos);
				else
					CPythonChangeLook::Instance().RemoveAllMaterials();
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 4));
			}
			break;
		case CL_SUBHEADER_REFINE:
			{
				CPythonChangeLook::Instance().RemoveAllMaterials();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 4));
			}
			break;
		default:
			TraceError("CPythonNetworkStream::RecvChangeLookPacket: unknown subheader %d\n.", sPacket.subheader);
			break;
	}
	
	return true;
}

bool CPythonNetworkStream::SendClClosePacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_CLOSE;
	sPacket.dwCost = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return true;
}

bool CPythonNetworkStream::SendClAddPacket(TItemPos tPos, BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_ADD;
	sPacket.dwCost = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return true;
}

bool CPythonNetworkStream::SendClRemovePacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_REMOVE;
	sPacket.dwCost = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return true;
}

bool CPythonNetworkStream::SendClRefinePacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_REFINE;
	sPacket.dwCost = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return true;
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
bool CPythonNetworkStream::PetSetNamePacket(const char * petname)
{
	TPacketCGRequestPetName PetSetName;
	PetSetName.byHeader = HEADER_CG_PET_SET_NAME;
	strncpy(PetSetName.petname, petname, 12);
	PetSetName.petname[12] = '\0';

	if (!Send(sizeof(PetSetName), &PetSetName))
		return false;

	return true;
}
#endif

#ifdef ENABLE_SHOW_CHEST_DROP
bool CPythonNetworkStream::SendChestDropInfo(TItemPos Cell)
{
	TPacketCGChestDropInfo packet;
	packet.header = HEADER_CG_CHEST_DROP_INFO;
	packet.tPos = Cell;
	
	if (!Send(sizeof(packet), &packet))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvChestDropInfo()
{
	TPacketGCChestDropInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);
	while (packet.wSize > 0)
	{
		TChestDropInfoTable kTab;
		if (!Recv(sizeof(kTab), &kTab))
			return false;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddChestDropInfo", Py_BuildValue("(iiiiii)", packet.tPos.window_type, packet.tPos.cell, kTab.bPageIndex, kTab.bSlotIndex, kTab.dwItemVnum, kTab.bItemCount));

		packet.wSize -= sizeof(kTab);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RefreshChestDropInfo", Py_BuildValue("(ii)", packet.tPos.window_type, packet.tPos.cell));

	return true;
}
#endif

#ifdef ENABLE_SEND_TARGET_INFO
bool CPythonNetworkStream::RecvTargetInfoPacket()
{
	TPacketGCTargetInfo pInfoTargetPacket;

	if (!Recv(sizeof(TPacketGCTargetInfo), &pInfoTargetPacket))
	{
		Tracen("Recv Info Target Packet Error");
		return false;
	}

	CInstanceBase * pInstPlayer = CPythonCharacterManager::Instance().GetMainInstancePtr();
	CInstanceBase * pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(pInfoTargetPacket.dwVID);
	if (pInstPlayer && pInstTarget)
	{
		if (!pInstTarget->IsDead())
		{
			if (pInstTarget->IsEnemy() || pInstTarget->IsStone())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddTargetMonsterDropInfo", 
				Py_BuildValue("(iii)", pInfoTargetPacket.race, pInfoTargetPacket.dwVnum, pInfoTargetPacket.count));
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RefreshTargetMonsterDropInfo", Py_BuildValue("(i)", pInfoTargetPacket.race));
			}
			else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

		}
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
	}

	return true;
}

bool CPythonNetworkStream::SendTargetInfoLoadPacket(DWORD dwVID)
{
	TPacketCGTargetInfoLoad TargetInfoLoadPacket;
	TargetInfoLoadPacket.header = HEADER_CG_TARGET_INFO_LOAD;
	TargetInfoLoadPacket.dwVID = dwVID;

	if (!Send(sizeof(TargetInfoLoadPacket), &TargetInfoLoadPacket))
		return false;

	return true;
}
#endif

#ifdef ENABLE_SUPPORT_SYSTEM
bool CPythonNetworkStream::RecvSupportUseSkill()
{
	TPacketGCSupportUseSkill packet;

	if (!Recv(sizeof(TPacketGCSupportUseSkill), &packet))
	{
		Tracen("CPythonNetworkStream::RecvCharacterMovePacket - PACKET READ ERROR");
		return false;
	}
	CInstanceBase* pkInstClone = CPythonCharacterManager::Instance().GetInstancePtr(packet.dwVid);
	DWORD dwSkillIndex = packet.dwVnum;
	CPythonSkill::TSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(dwSkillIndex, &pSkillData))
		return false;

	DWORD value = 0;
	if (packet.dwLevel <= 17)
		value = 0;
	else if (packet.dwLevel <= 30 && packet.dwLevel >17)
		value = 1;
	else if (packet.dwLevel > 30 && packet.dwLevel <= 39)
		value = 2;
	else if (packet.dwLevel >= 40)
		value = 3;

	DWORD dwMotionIndex = pSkillData->GetSkillMotionIndex(value);
	if (!pkInstClone->NEW_UseSkill(dwSkillIndex, dwMotionIndex, 1, false))
	{
		Tracenf("CPythonPlayer::UseGuildSkill(%d) - pkInstMain->NEW_UseSkill - ERROR", dwSkillIndex);
	}

	return true;
}
#endif

#ifdef ENABLE_SKILL_BOOK_COMBINATION
bool CPythonNetworkStream::SendSkillBookCombinationPacket(int List[10], int mod)
{
	TPacketCGSkillBookCombination SkillBookCombinationPacket;

	SkillBookCombinationPacket.header = HEADER_CG_SKILL_BOOK_COMBINATION;
	SkillBookCombinationPacket.mod = mod;
	memcpy(SkillBookCombinationPacket.List, List, sizeof(SkillBookCombinationPacket.List));

	if (!Send(sizeof(SkillBookCombinationPacket), &SkillBookCombinationPacket))
		return false;

	return true;
}
#endif

#ifdef ENABLE_DAMAGE_TOP
bool CPythonNetworkStream::RecvDamageTopInfo()
{
	TPacketGCDamageTop packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);
	
	int iPos = 1;
	int iDmgType = 0;
	
	while (packet.wSize > 0)
	{
		TDamage damage;
		if (!Recv(sizeof(damage), &damage))
			return false;
		
		if (iDmgType != damage.bDamageType)
			iPos = 1;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DamageTopAddMember", Py_BuildValue("(iisiisi)", 
			damage.bDamageType, iPos, damage.szAttackerName, damage.bAttackerLevel, damage.bAttackerEmpire, damage.szVictimName, damage.dwDamageValue));
			
		iPos++;
		iDmgType = damage.bDamageType;

		packet.wSize -= sizeof(damage);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DamageTopRefresh", Py_BuildValue("()"));
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DamageTopOpen", Py_BuildValue("()"));

	return true;
} 
#endif

#ifdef ENABLE_RANKING_SYSTEM
bool CPythonNetworkStream::RecvRankingInfo()
{
	TPacketRanking packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);
		
	while (packet.wSize > 0)
	{
		TRankingInfo Info;
		if (!Recv(sizeof(Info), &Info))
			return false;

		CPythonRanking::Instance().AddRankingInfo(Info.bCategory, Info.iRankingIdx, Info);

		packet.wSize -= sizeof(Info);
	}

	return true;
}
#endif

#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
bool CPythonNetworkStream::RecvAccumulate()
{
	TPacketGCAccumulate AccumulatePacket;
	if (!Recv(sizeof(AccumulatePacket), &AccumulatePacket))
		return false;

	CPythonPlayer::Instance().SetAccumulateDamage(AccumulatePacket.dwVid, AccumulatePacket.dwCount);
	
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshAccumulateCount", Py_BuildValue("(i)", AccumulatePacket.dwVid));

	return true;
}
#endif

bool CPythonNetworkStream::SendMiniGameAttendanceButtonClick(int iNumber)
{
	TPacketCGAttendanceButtonClick AttendanceButtonClickPacket;
	AttendanceButtonClickPacket.bHeader = HEADER_CG_ATTENDANCE_BUTTON_CLICK;
	AttendanceButtonClickPacket.iNumber = iNumber;
	if (!Send(sizeof(AttendanceButtonClickPacket), &AttendanceButtonClickPacket))
		return false;

	return true;
}

bool CPythonNetworkStream::SendMiniGameAttendanceRequestRewardList()
{
	BYTE bHeader = HEADER_CG_ATTENDANCE_REQUEST_REWARD_LIST;
	if (!Send(sizeof(bHeader), &bHeader))
		return false;

	return true;
}

bool CPythonNetworkStream::SendMiniGameAttendanceRequestData(BYTE bType)
{
	TPacketCGAttendanceRequestData AttendanceRequestDataPacket;
	AttendanceRequestDataPacket.bHeader = HEADER_CG_ATTENDANCE_REQUEST_DATA;
	AttendanceRequestDataPacket.bType = bType;
	if (!Send(sizeof(TPacketCGAttendanceRequestData), &AttendanceRequestDataPacket))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvAttendanceRequestData()
{
	TPacketGCAttendanceRequestData packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameAttendanceSetData", Py_BuildValue("(ii)", packet.bType, packet.bValue));

	return true;
}

bool CPythonNetworkStream::RecvAttendanceRequestRewardList()
{
	TPacketGCAttendanceRequestRewardList packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);
	while (packet.wSize > 0)
	{
		TRewardItem kReward;
		if (!Recv(sizeof(kReward), &kReward))
			return false;

		CPythonPlayer::Instance().SetRewardItem(kReward.bDay, kReward.dwVnum, kReward.dwCount);

		packet.wSize -= sizeof(kReward);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameAttendanceRequestRewardList", Py_BuildValue("()"));

	return true;
}
#endif

#ifdef ENABLE_FISH_EVENT
bool CPythonNetworkStream::SendFishBoxUse(BYTE bWindow, WORD wCell)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGFishEvent packetFishEvent;
	packetFishEvent.bHeader = HEADER_CG_FISH_EVENT;
	packetFishEvent.bSubheader = FISH_EVENT_SUBHEADER_CG_BOX_USE;

	if (!Send(sizeof(TPacketCGFishEvent), &packetFishEvent))
	{
		Tracef("SendFishBoxUse Error\n");
		return false;
	}
	
	if (!Send(sizeof(bWindow), &bWindow))
	{
		Tracef("SendFishBoxUse Error\n");
		return false;
	}
	
	if (!Send(sizeof(wCell), &wCell))
	{
		Tracef("SendFishBoxUse Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendFishShapeAdd(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGFishEvent packetFishEvent;
	packetFishEvent.bHeader = HEADER_CG_FISH_EVENT;
	packetFishEvent.bSubheader = FISH_EVENT_SUBHEADER_CG_SHAPE_ADD;

	if (!Send(sizeof(TPacketCGFishEvent), &packetFishEvent))
	{
		Tracef("SendFishShapeAdd Error\n");
		return false;
	}
	
	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendFishShapeAdd Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendRequestFishEventBlock()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGFishEvent packetFishEvent;
	packetFishEvent.bHeader = HEADER_CG_FISH_EVENT;
	packetFishEvent.bSubheader = FISH_EVENT_SUBHEADER_CG_REQUEST_BLOCK;

	if (!Send(sizeof(TPacketCGFishEvent), &packetFishEvent))
	{
		Tracef("SendRequestFishEventBlock Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvFishEvent()
{
	TPacketGCFishEvent fishEventPacket;

	if (!Recv(sizeof(fishEventPacket), &fishEventPacket))
		return false;

	switch (fishEventPacket.bSubheader)
	{
		case FISH_EVENT_SUBHEADER_GC_BOX_USE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishUse", Py_BuildValue("(ii)", INVENTORY, fishEventPacket.dwFirstArg));
			break;
			
		case FISH_EVENT_SUBHEADER_GC_SHAPE_ADD:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishAdd", Py_BuildValue("(ii)", fishEventPacket.dwFirstArg, fishEventPacket.dwSecondArg));
			break;
			
		case FISH_EVENT_SUBHEADER_GC_REWARD:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishReward", Py_BuildValue("(i)", fishEventPacket.dwFirstArg));
			break;
			
		case FISH_EVENT_SUBHEADER_GC_COUNT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishCount", Py_BuildValue("(i)", fishEventPacket.dwFirstArg));
			break;
	};

	return true;
}
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
bool CPythonNetworkStream::SendMiniGameCatchKing(BYTE bSubHeader, BYTE bSubArgument)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketCGMiniGameCatchKing packet;
	packet.bHeader = HEADER_CG_MINI_GAME_CATCH_KING;
	packet.bSubheader = bSubHeader;
	packet.bSubArgument = bSubArgument;

	if (!Send(sizeof(TPacketCGMiniGameCatchKing), &packet))
	{
		Tracef("SendMiniGameCatchKing Send Packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvMiniGameCatchKingPacket()
{
	std::vector<char> vecBuffer;
	vecBuffer.clear();

	TPacketGCMiniGameCatchKing packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	int iSize = packet.wSize - sizeof(packet);
	if (iSize > 0)
	{
		vecBuffer.resize(iSize);
		if (!Recv(iSize, &vecBuffer[0]))
			return false;
	}

	switch (packet.bSubheader)
	{
		case SUBHEADER_GC_CATCH_KING_START:
			{
				DWORD dwBigScore = *(DWORD *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingEventStart", Py_BuildValue("(i)", dwBigScore));
			}
			break;
			
		case SUBHEADER_GC_CATCH_KING_SET_CARD:
			{
				BYTE bCardNumber = *(BYTE *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingSetHandCard", Py_BuildValue("(i)", bCardNumber));
			}
			break;
			
		case SUBHEADER_GC_CATCH_KING_RESULT_FIELD:
			{
				TPacketGCMiniGameCatchKingResult * packSecond = (TPacketGCMiniGameCatchKingResult *)&vecBuffer[0];
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingResultField", Py_BuildValue("(iiiibbbb)", 
					packSecond->dwPoints, packSecond->bRowType, packSecond->bCardPos, packSecond->bCardValue, 
					packSecond->bKeepFieldCard, packSecond->bDestroyHandCard, packSecond->bGetReward, packSecond->bIsFiveNearBy));
			}
			break;
			
		case SUBHEADER_GC_CATCH_KING_SET_END_CARD:
			{
				TPacketGCMiniGameCatchKingSetEndCard * packSecond = (TPacketGCMiniGameCatchKingSetEndCard *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingSetEndCard", Py_BuildValue("(ii)", packSecond->bCardPos, packSecond->bCardValue));
			}
			break;
			
		case SUBHEADER_GC_CATCH_KING_REWARD:
			{
				BYTE bReturnCode = *(BYTE *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingReward", Py_BuildValue("(i)", bReturnCode));
			}
			break;
			
		default:
			TraceError("CPythonNetworkStream::RecvMiniGameCatchKingPacket: Unknown subheader\n");
			break;
	}

	return true;
}
#endif

#ifdef ENABLE_MAILBOX
bool CPythonNetworkStream::SendMailBoxClose()
{
	TPacketCGMailBox packet;
	packet.bHeader = HEADER_CG_MAILBOX;
	packet.bSubHeader = MAILBOX_CG_CLOSE;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("SendMailBoxClose Send Packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendPostWriteConfirm(const char * szToName)
{
	TPacketCGMailBox packet;
	packet.bHeader = HEADER_CG_MAILBOX;
	packet.bSubHeader = MAILBOX_CG_WRITE_CONFIRM;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("SendPostWriteConfirm Send Packet Error\n");
		return false;
	}

	char c_szToName[CHARACTER_NAME_MAX_LEN];
	strncpy(c_szToName, szToName, CHARACTER_NAME_MAX_LEN - 1);
	if (!Send(sizeof(c_szToName), &c_szToName))
		return false;

	return true;
}

bool CPythonNetworkStream::SendPostWrite(const char * szToName, const char * szTitle, const char * szMessage, BYTE bWindowType, DWORD dwItemPos, DWORD dwMoney, DWORD dwCheque)
{
	TPacketCGMailBox packet;
	packet.bHeader = HEADER_CG_MAILBOX;
	packet.bSubHeader = MAILBOX_CG_POST_WRITE;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("SendPostWrite Send Packet Error\n");
		return false;
	}

	char c_szToName[CHARACTER_NAME_MAX_LEN];
	strncpy(c_szToName, szToName, CHARACTER_NAME_MAX_LEN - 1);
	if (!Send(sizeof(c_szToName), &c_szToName))
		return false;

	char c_szTitle[MAIL_TITLE_MAX_LEN];
	strncpy(c_szTitle, szTitle, MAIL_TITLE_MAX_LEN - 1);
	if (!Send(sizeof(c_szTitle), &c_szTitle))
		return false;

	char c_szMessage[MAIL_MESSAGE_MAX_LEN];
	strncpy(c_szMessage, szMessage, MAIL_MESSAGE_MAX_LEN - 1);
	if (!Send(sizeof(c_szMessage), &c_szMessage))
		return false;

	if (!Send(sizeof(bWindowType), &bWindowType))
		return false;

	if (!Send(sizeof(dwItemPos), &dwItemPos))
		return false;

	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	if (!Send(sizeof(dwCheque), &dwCheque))
		return false;

	return true;
}

bool CPythonNetworkStream::SendPostGetItems(BYTE bDataIndex)
{
	TPacketCGMailBox packet;
	packet.bHeader = HEADER_CG_MAILBOX;
	packet.bSubHeader = MAILBOX_CG_POST_GET_ITEMS;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("SendPostGetItems Send Packet Error\n");
		return false;
	}

	if (!Send(sizeof(bDataIndex), &bDataIndex))
		return false;

	return true;
}

bool CPythonNetworkStream::RequestPostAddData(BYTE bButtonIndex, BYTE bDataIndex)
{
	TPacketCGMailBox packet;
	packet.bHeader = HEADER_CG_MAILBOX;
	packet.bSubHeader = MAILBOX_CG_REQUEST_POST_ADD_DATA;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("RequestPostAddData Send Packet Error\n");
		return false;
	}

	if (!Send(sizeof(bButtonIndex), &bButtonIndex))
		return false;

	if (!Send(sizeof(bDataIndex), &bDataIndex))
		return false;

	return true;
}

bool CPythonNetworkStream::SendPostDelete(BYTE bDataIndex)
{
	TPacketCGMailBox packet;
	packet.bHeader = HEADER_CG_MAILBOX;
	packet.bSubHeader = MAILBOX_CG_POST_DELETE;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("SendPostDelete Send Packet Error\n");
		return false;
	}

	if (!Send(sizeof(bDataIndex), &bDataIndex))
		return false;

	return true;
}

bool CPythonNetworkStream::SendPostAllDelete()
{
	TPacketCGMailBox packet;
	packet.bHeader = MAILBOX_CG_ALL_POST_DELETE;
	packet.bSubHeader = MAILBOX_CG_CLOSE;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("SendPostAllDelete Send Packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendPostAllGetItems()
{
	TPacketCGMailBox packet;
	packet.bHeader = HEADER_CG_MAILBOX;
	packet.bSubHeader = MAILBOX_CG_ALL_GET_ITEMS;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("SendPostAllGetItems Send Packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvMailBox()
{
	std::vector<char> vecBuffer;
	vecBuffer.clear();

	TPacketGCMailBox packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	int iSize = packet.wSize - sizeof(packet);
	if (iSize)
	{
		vecBuffer.resize(iSize);
		if (!Recv(iSize, &vecBuffer[0]))
			return false;
	}

	switch(packet.bSubHeader)
	{
		case MAILBOX_GC_CLOSE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(ii)", packet.bSubHeader, nullptr));
			break;

		case MAILBOX_GC_POST_WRITE_CONFIRM:
		case MAILBOX_GC_POST_WRITE:
		case MAILBOX_GC_POST_ALL_DELETE:
		case MAILBOX_GC_OPEN:
		{
			BYTE bData = *(BYTE *)&vecBuffer[0];
			if (packet.bSubHeader == MAILBOX_GC_POST_ALL_DELETE && POST_ALL_DELETE_OK)
				CPythonMail::Instance().AllDeleteMail();

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(ii)", packet.bSubHeader, bData));
		}
		break;

		case MAILBOX_GC_UNREAD_DATA:
		{
			TMailData Mail = *(TMailData *)&vecBuffer[0];
			CPythonMail::Instance().AppendMailData(Mail.index, Mail);
			PyObject* Data = PyTuple_New(5);
			PyTuple_SetItem(Data, 0, Py_True);
			PyTuple_SetItem(Data, 1, PyInt_FromLong(CPythonMail::Instance().GetNotConfirmCount()));
			PyTuple_SetItem(Data, 2, PyInt_FromLong(CPythonMail::Instance().GetItemCount()));
			PyTuple_SetItem(Data, 3, PyInt_FromLong(CPythonMail::Instance().GetNotConfirmCount() - CPythonMail::Instance().GetItemCount()));
			PyTuple_SetItem(Data, 4, PyBool_FromLong(Mail.is_gm_post));
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(iO)", packet.bSubHeader, Data));
		}
		break;

		case MAILBOX_GC_POST_GET_ITEMS:
		case MAILBOX_GC_POST_DELETE:
		{
			BYTE bData = *(BYTE *)&vecBuffer[0];
			BYTE bDataIndex = *(BYTE *)&vecBuffer[sizeof(bData)];

			if (packet.bSubHeader == MAILBOX_GC_POST_DELETE && bData == POST_DELETE_OK)
				CPythonMail::Instance().DeleteMail(bDataIndex);
			else if (packet.bSubHeader == MAILBOX_GC_POST_GET_ITEMS && bData == POST_GET_ITEMS_OK)
				CPythonMail::Instance().GetItems(bDataIndex);

			PyObject* Data = PyTuple_New(2);
			PyTuple_SetItem(Data, 0, PyInt_FromLong(bDataIndex));
			PyTuple_SetItem(Data, 1, PyInt_FromLong(bData));
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(iO)", packet.bSubHeader, Data));
		}
		break;

		case MAILBOX_GC_ADD_DATA:
		{
			BYTE bButtonIndex = *(BYTE *)&vecBuffer[0];
			TMailData Mail = *(TMailData *)&vecBuffer[sizeof(bButtonIndex)];
			CPythonMail::Instance().DeleteMail(Mail.index);
			CPythonMail::Instance().AppendMailData(Mail.index, Mail);

			PyObject* Data = PyTuple_New(2);
			PyTuple_SetItem(Data, 0, PyInt_FromLong(bButtonIndex));
			PyTuple_SetItem(Data, 1, PyInt_FromLong(Mail.index));
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(iO)", packet.bSubHeader, Data));

			if (CPythonMail::Instance().GetNotConfirmCount())
			{
				PyObject* Data2 = PyTuple_New(5);
				PyTuple_SetItem(Data2, 0, Py_False);
				PyTuple_SetItem(Data2, 1, PyInt_FromLong(CPythonMail::Instance().GetNotConfirmCount()));
				PyTuple_SetItem(Data2, 2, PyInt_FromLong(CPythonMail::Instance().GetItemCount()));
				PyTuple_SetItem(Data2, 3, PyInt_FromLong(CPythonMail::Instance().GetNotConfirmCount() - CPythonMail::Instance().GetItemCount()));
				PyTuple_SetItem(Data2, 4, PyBool_FromLong(Mail.is_gm_post));
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(iO)", MAILBOX_GC_UNREAD_DATA, Data2));
			}
			else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(ii)", MAILBOX_GC_SYSTEM_CLOSE, nullptr));
		}
		break;

		case MAILBOX_GC_POST_ALL_GET_ITEMS:
		{
			BYTE bSuccesList[MAIL_SLOT_MAX_NUM];
			if (!Recv(sizeof(bSuccesList), &bSuccesList))
				return false;

			BYTE bData = *(BYTE *)&vecBuffer[0];

			PyObject* List = PyList_New(0);
			for (int i = 0; i < MAIL_SLOT_MAX_NUM; ++i)
			{
				if (bSuccesList[i])
				{
					CPythonMail::Instance().GetItems(i);
					PyList_Append(List, PyInt_FromLong(i));
				}
			}

			PyObject* Data = PyTuple_New(2);
			PyTuple_SetItem(Data, 0, PyInt_FromLong(bData));
			PyTuple_SetItem(Data, 1, List);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(iO)", packet.bSubHeader, Data));
		}
		break;

		case MAILBOX_GC_SET:
		{
			CPythonMail::Instance().Clear();
			int read_point = 0;
			bool bIsGmPost = false;
			if (iSize)
				while (iSize != read_point)
				{
					TMailData Mail = *(TMailData *)&vecBuffer[read_point];
					CPythonMail::Instance().AppendMailData(Mail.index, Mail);
					read_point += sizeof(Mail);
					if (Mail.is_gm_post)
						bIsGmPost = true;
				}

			if (CPythonMail::Instance().GetNotConfirmCount())
			{
				PyObject* Data = PyTuple_New(5);
				PyTuple_SetItem(Data, 0, Py_False);
				PyTuple_SetItem(Data, 1, PyInt_FromLong(CPythonMail::Instance().GetNotConfirmCount()));
				PyTuple_SetItem(Data, 2, PyInt_FromLong(CPythonMail::Instance().GetItemCount()));
				PyTuple_SetItem(Data, 3, PyInt_FromLong(CPythonMail::Instance().GetNotConfirmCount() - CPythonMail::Instance().GetItemCount()));
				PyTuple_SetItem(Data, 4, PyBool_FromLong(bIsGmPost));
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(iO)", MAILBOX_GC_UNREAD_DATA, Data));
			}
			else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(ii)", MAILBOX_GC_SYSTEM_CLOSE, nullptr));
		}
		break;
	}

	return true;
}
#endif

bool CPythonNetworkStream::RecvAntiCheatBlacklist()
{
	TPacketGCAntiCheatBlacklist kAntiBlacklistPacket;

	if (!Recv(sizeof(kAntiBlacklistPacket), &kAntiBlacklistPacket))
		return false;

	CAnticheatManager::Instance().ParseCheatBlacklist(kAntiBlacklistPacket.content);

	return true;
}

bool CPythonNetworkStream::SendHackNotification(const char* c_szMsg, const char* c_szInfo, const char* c_szHwid)
{
	TPacketCGHack kPacketHack;
	kPacketHack.bHeader = HEADER_CG_HACK;
	strncpy_s(kPacketHack.szBuf, c_szMsg, sizeof(kPacketHack.szBuf) - 1);
	strncpy_s(kPacketHack.szInfo, c_szInfo, sizeof(kPacketHack.szInfo) - 1);

	if (!Send(sizeof(kPacketHack), &kPacketHack))
		return false;

	return true;
}

#ifdef ENABLE_ATTR_6TH_7TH
bool CPythonNetworkStream::SendAttr67Add(BYTE bWindowType, WORD wSlotIndex, BYTE bMaterialCount, BYTE bSupportCount, BYTE bSupportWindowType, WORD wSupportSlotIndex)
{
	TPacketCGAttr67Add packet;
	packet.bHeader = HEADER_CG_ATTR67_ADD;
	packet.bWindowType = bWindowType;
	packet.wSlotIndex = wSlotIndex;
	packet.bMaterialCount = bMaterialCount;
	packet.bSupportCount = bSupportCount;
	packet.bSupportWindowType = bSupportWindowType;
	packet.wSupportSlotIndex = wSupportSlotIndex;

	if (!Send(sizeof(packet), &packet))
		return false;

	return true;
}
#endif

#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
bool CPythonNetworkStream::RecvOfflinePrivateShop()
{
	TPacketGCOfflinePrivateShopInfo packet;

	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);

	switch(packet.bSubHeader)
	{
		case OFFLINE_PRIVATE_SHOP_INFO:
		{
			CPythonOffflinePrivateShop::Instance().Clear();
			BYTE bCount = 0;
			while (packet.wSize)
			{
				TraceError("girdi lo:%d:%d", packet.wSize, sizeof(TOfflinePrivateShopInfo));
				TOfflinePrivateShopInfo Info;
				if (!Recv(sizeof(Info), &Info))
					return false;

				CPythonOffflinePrivateShop::Instance().AppendInfo(Info);

				packet.wSize -= sizeof(Info);
				++bCount;
			}
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OfflinePrivateShopEditOpen", Py_BuildValue("()"));
		}
		break;

		default:
			break;
	}

	return true;
}
#endif

