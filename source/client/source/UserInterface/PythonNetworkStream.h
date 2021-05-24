#pragma once

#include "../EterLib/FuncObject.h"
#include "../EterLib/NetStream.h"
#include "../EterLib/NetPacketHeaderMap.h"
#include "../AntiCheatLib/CheatQueueManager.h"

#include "InsultChecker.h"

#include "packet.h"

class CInstanceBase;
class CNetworkActorManager;
struct SNetworkActorData;
struct SNetworkUpdateActorData;

class CPythonNetworkStream : public CNetworkStream, public CSingleton<CPythonNetworkStream>
{
	public:
		enum
		{
			SERVER_COMMAND_LOG_OUT = 0,
			SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER = 1,
			SERVER_COMMAND_QUIT = 2,

			MAX_ACCOUNT_PLAYER
		};

		enum
		{
			ERROR_NONE,
			ERROR_UNKNOWN,
			ERROR_CONNECT_MARK_SERVER,
			ERROR_LOAD_MARK,
			ERROR_MARK_WIDTH,
			ERROR_MARK_HEIGHT,
			ERROR_MARK_UPLOAD_NEED_RECONNECT,
			ERROR_MARK_CHECK_NEED_RECONNECT,

		};

		enum
		{
			ACCOUNT_CHARACTER_SLOT_ID,
			ACCOUNT_CHARACTER_SLOT_NAME,
			ACCOUNT_CHARACTER_SLOT_RACE,
			ACCOUNT_CHARACTER_SLOT_LEVEL,
			ACCOUNT_CHARACTER_SLOT_STR,
			ACCOUNT_CHARACTER_SLOT_DEX,
			ACCOUNT_CHARACTER_SLOT_HTH,
			ACCOUNT_CHARACTER_SLOT_INT,
			ACCOUNT_CHARACTER_SLOT_PLAYTIME,
			ACCOUNT_CHARACTER_SLOT_FORM,
			ACCOUNT_CHARACTER_SLOT_ADDR,
			ACCOUNT_CHARACTER_SLOT_PORT,
			ACCOUNT_CHARACTER_SLOT_GUILD_ID,
			ACCOUNT_CHARACTER_SLOT_GUILD_NAME,
			ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG,
			ACCOUNT_CHARACTER_SLOT_HAIR,
#ifdef ENABLE_ACCE_SYSTEM
			ACCOUNT_CHARACTER_SLOT_ACCE,
#endif
		};

		enum
		{
			PHASE_WINDOW_LOGO,
			PHASE_WINDOW_LOGIN,
			PHASE_WINDOW_SELECT,
			PHASE_WINDOW_CREATE,
			PHASE_WINDOW_LOAD,
			PHASE_WINDOW_GAME,
			PHASE_WINDOW_EMPIRE,
			PHASE_WINDOW_NUM,
		};

	public:
		CPythonNetworkStream();
		virtual ~CPythonNetworkStream();

		bool SendSpecial(int nLen, void * pvBuf);

		void StartGame();
		void Warp(LONG lGlobalX, LONG lGlobalY);

		void NotifyHack(const char* c_szMsg);
		void SetWaitFlag();

		void SendEmoticon(UINT eEmoticon);

		void ExitApplication();
		void ExitGame();
		void LogOutGame();
		void AbsoluteExitGame();
		void AbsoluteExitApplication();

		void EnableChatInsultFilter(bool isEnable);
		bool IsChatInsultIn(const char* c_szMsg);
		bool IsInsultIn(const char* c_szMsg);

		DWORD GetGuildID();

		UINT UploadMark(const char* c_szImageFileName);
		UINT UploadSymbol(const char* c_szImageFileName);

		bool LoadInsultList(const char* c_szInsultListFileName);
		bool LoadConvertTable(DWORD dwEmpireID, const char* c_szFileName);

		UINT		GetAccountCharacterSlotDatau(UINT iSlot, UINT eType);
		const char* GetAccountCharacterSlotDataz(UINT iSlot, UINT eType);
		const char*		GetFieldMusicFileName();
		float			GetFieldMusicVolume();
		bool IsSelectedEmpire();

		void ToggleGameDebugInfo();

		void SetMarkServer(const char* c_szAddr, UINT uPort);
		void ConnectLoginServer(const char* c_szAddr, UINT uPort);
		void ConnectGameServer(UINT iChrSlot);

		void SetLoginInfo(const char* c_szID, const char* c_szPassword);
		void SetLoginKey(DWORD dwLoginKey);
		void ClearLoginInfo( void );

		void SetHandler(PyObject* poHandler);
		void SetPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd);
		void ClearPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd);
		void SetServerCommandParserWindow(PyObject* poPhaseWnd);

		bool SendSyncPositionElementPacket(DWORD dwVictimVID, DWORD dwVictimX, DWORD dwVictimY);

		bool SendAttackPacketFromThread(UINT uMotAttack, DWORD dwVIDVictim, THREAD_ARGS);
		bool SendCharacterStatePacket(const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg);
		bool SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID=0);
		bool SendTargetPacket(DWORD dwVID);
		bool SendCharacterStartWalkingPacket(float fRotation, long lx, long ly);
		bool SendCharacterEndWalkingPacket(float fRotation, long lx, long ly);
		bool SendCharacterCheckWalkingPacket(float fRotation, long lx, long ly);

		bool SendCharacterPositionPacket(BYTE iPosition);

		bool SendItemUsePacket(TItemPos pos);
		bool SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos);
		bool SendItemDropPacket(TItemPos pos, DWORD elk);
		bool SendItemDropPacketNew(TItemPos pos, DWORD elk, DWORD count);
		bool SendItemMovePacket(TItemPos pos, TItemPos change_pos, BYTE num);
		bool SendItemPickUpPacket(DWORD vid);
		bool SendItemDestroyPacket(TItemPos pos);
		bool SendItemSellPacket(TItemPos pos);

		bool SendQuickSlotAddPacket(BYTE wpos, BYTE type, BYTE pos);
		bool SendQuickSlotDelPacket(BYTE wpos);
		bool SendQuickSlotMovePacket(BYTE wpos, BYTE change_pos);
		bool SendPointResetPacket();
		bool SendShopEndPacket();
		bool SendShopBuyPacket(BYTE byCount);
		bool SendShopSellPacket(BYTE bySlot);
#ifdef ENABLE_NEW_STORAGE_SYSTEM
		bool SendShopSellPacketNew(BYTE bySlot, BYTE byCount, BYTE byType);
#else
		bool SendShopSellPacketNew(BYTE bySlot, BYTE byCount);
#endif

		bool SendExchangeStartPacket(DWORD vid);
		bool SendExchangeItemAddPacket(TItemPos ItemPos, BYTE byDisplayPos);
		bool SendExchangeElkAddPacket(DWORD elk);
		bool SendExchangeItemDelPacket(BYTE pos);
		bool SendExchangeAcceptPacket();
		bool SendExchangeExitPacket();
#ifdef ENABLE_CHEQUE_SYSTEM
		bool SendExchangeChequeAddPacket(short cheque);
#endif
		bool SendScriptAnswerPacket(int iAnswer);
		bool SendScriptButtonPacket(unsigned int iIndex);
		bool SendAnswerMakeGuildPacket(const char * c_szName);
		bool SendQuestInputStringPacket(const char * c_szString);
		bool SendQuestConfirmPacket(BYTE byAnswer, DWORD dwPID);
		bool SendOnClickPacket(DWORD vid);
		bool SendFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition& kPPosTarget);
		bool SendAddFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition& kPPosTarget);
		bool SendShootPacket(UINT uSkill);
		bool ClientCommand(const char * c_szCommand);
		void ServerCommand(char * c_szCommand);
		void RegisterEmoticonString(const char * pcEmoticonString);
		bool SendPartyInvitePacket(DWORD dwVID);
		bool SendPartyInviteAnswerPacket(DWORD dwLeaderVID, BYTE byAccept);
		bool SendPartyRemovePacket(DWORD dwPID);
		bool SendPartySetStatePacket(DWORD dwVID, BYTE byState, BYTE byFlag);
		bool SendPartyUseSkillPacket(BYTE bySkillIndex, DWORD dwVID);
		bool SendPartyParameterPacket(BYTE byDistributeMode);
		bool SendSafeBoxMoneyPacket(BYTE byState, DWORD dwMoney);
		bool SendSafeBoxCheckinPacket(TItemPos InventoryPos, DWORD dwSafeBoxPos);
		bool SendSafeBoxCheckoutPacket(DWORD dwSafeBoxPos, TItemPos InventoryPos);
		bool SendSafeBoxItemMovePacket(DWORD dwSourcePos, DWORD dwTargetPos, DWORD dwCount);
		bool SendMallCheckoutPacket(BYTE byMallPos, TItemPos InventoryPos);
		bool SendGuildAddMemberPacket(DWORD dwVID);
		bool SendGuildRemoveMemberPacket(DWORD dwPID);
		bool SendGuildChangeGradeNamePacket(BYTE byGradeNumber, const char * c_szName);
		bool SendGuildChangeGradeAuthorityPacket(BYTE byGradeNumber, BYTE byAuthority);
		bool SendGuildOfferPacket(DWORD dwExperience);
		bool SendGuildPostCommentPacket(const char * c_szMessage);
		bool SendGuildDeleteCommentPacket(DWORD dwIndex);
		bool SendGuildRefreshCommentsPacket(DWORD dwHighestIndex);
		bool SendGuildChangeMemberGradePacket(DWORD dwPID, BYTE byGrade);
		bool SendGuildUseSkillPacket(DWORD dwSkillID, DWORD dwTargetVID);
		bool SendGuildChangeMemberGeneralPacket(DWORD dwPID, BYTE byFlag);
		bool SendGuildInvitePacket(DWORD dwVID);
		bool SendGuildInviteAnswerPacket(DWORD dwGuildID, BYTE byAnswer);
		bool SendGuildChargeGSPPacket(DWORD dwMoney);
		bool SendGuildDepositMoneyPacket(DWORD dwMoney);
		bool SendGuildWithdrawMoneyPacket(DWORD dwMoney);
		bool RecvMallOpenPacket();
		bool RecvMallItemSetPacket();
		bool RecvMallItemDelPacket();
		bool RecvLoverInfoPacket();
		bool RecvLovePointUpdatePacket();
		bool RecvDigMotionPacket();
		bool SendFishingPacket(int iRotation);
		bool SendGiveItemPacket(DWORD dwTargetVID, TItemPos ItemPos, int iItemCount);
		bool SendBuildPrivateShopPacket(const char * c_szName, const std::vector<TShopItemTable> & c_rSellingItemStock
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
, int iTime
#endif
);
		bool SendRefinePacket(BYTE byPos, BYTE byType, BYTE byWin);	
		bool SendSelectItemPacket(DWORD dwItemPos);
		bool SendClientVersionPacket();
		bool __SendCRCReportPacket();
		bool SendDragonSoulRefinePacket(BYTE bRefineType, TItemPos* pos);
		bool RecvHandshakePacket();
		bool RecvHandshakeOKPacket();
		DWORD GetMainActorVID();
		DWORD GetMainActorRace();
		DWORD GetMainActorEmpire();
		DWORD GetMainActorSkillGroup();
		void SetEmpireID(DWORD dwEmpireID);
		DWORD GetEmpireID();
		void __TEST_SetSkillGroupFake(int iIndex);

#ifdef ENABLE_GEM_SYSTEM
		bool SendGemShopBuy(BYTE bPos);
		bool SendSlotAdd();
		bool SendRequestRefresh();
		bool SendGemShopClose();
		bool RecvGemShop();
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
		bool SendExtendInvenButtonClick(BYTE bIndex);
		bool SendExtendInvenUpgrade();
		bool RecvExtendInven();
#endif

#ifdef ENABLE_MINI_GAME_RUMI
		bool SendMiniGameRumiStart();
		bool SendMiniGameRumiExit();
		bool SendMiniGameRumiHandCardClick(BOOL bAdd, BYTE bIndex);
		bool SendMiniGameRumiDeckCardClick();
		bool SendMiniGameRumiFieldCardClick(BYTE bIndex);
		bool RecvMiniGameRumi();
#endif

#ifdef ENABLE_ACCE_SYSTEM
		bool	SendAcceClosePacket();
		bool	SendAcceAddPacket(TItemPos tPos, BYTE bPos);
		bool	SendAcceRemovePacket(BYTE bPos);
		bool	SendAcceRefinePacket();
		bool	RecvAccePacket(bool bReturn = false);
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		bool	SendClClosePacket();
		bool	SendClAddPacket(TItemPos tPos, BYTE bPos);
		bool	SendClRemovePacket(BYTE bPos);
		bool	SendClRefinePacket();
		bool	RecvChangeLookPacket();
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
		bool PetSetNamePacket(const char * petname);
#endif

#ifdef ENABLE_SHOW_CHEST_DROP
		bool 	SendChestDropInfo(TItemPos Cell);
		bool 	RecvChestDropInfo();
#endif

#ifdef ENABLE_SEND_TARGET_INFO
		bool SendTargetInfoLoadPacket(DWORD dwVID);
		bool RecvTargetInfoPacket();
#endif

#ifdef ENABLE_MESSENGER_BLOCK
		bool SendMessengerBlockRemovePacket(const char * c_szKey, const char * c_szName);
		bool SendMessengerBlockAddByNamePacket(const char * c_szName);
		bool SendMessengerBlockAddByVIDPacket(DWORD vid);
#endif

#ifdef ENABLE_PARTY_MATCH
		bool GroupMatchh(BYTE index, BYTE ayar);
#endif

#ifdef ENABLE_SUPPORT_SYSTEM
		bool RecvSupportUseSkill();
#endif

#ifdef ENABLE_SKILL_BOOK_COMBINATION
		bool SendSkillBookCombinationPacket(int List[10], int mod);
#endif

#ifdef ENABLE_DAMAGE_TOP
		bool RecvDamageTopInfo();
#endif

#ifdef ENABLE_RANKING_SYSTEM
		bool RecvRankingInfo();
#endif

#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
		bool SendMiniGameAttendanceButtonClick(int iNumber);
		bool SendMiniGameAttendanceRequestRewardList();
		bool SendMiniGameAttendanceRequestData(BYTE bType);
		bool RecvAccumulate();
		bool RecvAttendanceRequestData();
		bool RecvAttendanceRequestRewardList();
#endif

#ifdef ENABLE_FISH_EVENT
		bool SendFishBoxUse(BYTE bWindow, WORD wCell);
		bool SendFishShapeAdd(BYTE bPos);
		bool SendRequestFishEventBlock();
		bool RecvFishEvent();
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
		bool SendMiniGameCatchKing(BYTE bSubHeader, BYTE bSubArgument);
		bool RecvMiniGameCatchKingPacket();
#endif

#ifdef ENABLE_MAILBOX
		bool SendMailBoxClose();
		bool SendPostWriteConfirm(const char * szToName);
		bool SendPostWrite(const char * szToName, const char * szTitle, const char * szMessage, BYTE bWindowType, DWORD dwItemPos, DWORD dwMoney, DWORD dwCheque);
		bool SendPostGetItems(BYTE bDataIndex);
		bool RequestPostAddData(BYTE bButtonIndex, BYTE bDataIndex);
		bool SendPostDelete(BYTE bDataIndex);
		bool SendPostAllDelete();
		bool SendPostAllGetItems();
		bool RecvMailBox();
#endif

		bool SendHackNotification(const char* c_szMsg, const char* c_szInfo, const char* c_szHwid);
		bool RecvAntiCheatBlacklist();

#ifdef ENABLE_ATTR_6TH_7TH
		bool SendAttr67Add(BYTE bWindowType, WORD wSlotIndex, BYTE bMaterialCount, BYTE bSupportCount, BYTE bSupportWindowType, WORD wSupportSlotIndex);
#endif

#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
		bool RecvOfflinePrivateShop();
#endif
	public:
		void SetOffLinePhase();
		void SetHandShakePhase();
		void SetLoginPhase();
		void SetSelectPhase();
		void SetLoadingPhase();
		void SetGamePhase();
		void ClosePhase();
		bool SendLoginPacket(const char * c_szName, const char * c_szPassword);
		bool SendLoginPacketNew(const char * c_szName, const char * c_szPassword);

		bool SendEnterGame();
		bool SendSelectEmpirePacket(DWORD dwEmpireID);
		bool SendSelectCharacterPacket(BYTE account_Index);
		bool SendChangeNamePacket(BYTE index, const char *name);
		bool SendCreateCharacterPacket(BYTE index, const char *name, BYTE job, BYTE shape, BYTE byStat1, BYTE byStat2, BYTE byStat3, BYTE byStat4);
		bool SendDestroyCharacterPacket(BYTE index, const char * szPrivateCode);
		bool SendC2CPacket(DWORD dwSize, void * pData);
		bool SendChatPacket(const char * c_szChat, BYTE byType = CHAT_TYPE_TALKING);
		bool SendWhisperPacket(const char * name, const char * c_szChat);
		bool SendMessengerAddByVIDPacket(DWORD vid);
		bool SendMessengerAddByNamePacket(const char * c_szName);
		bool SendMessengerRemovePacket(const char * c_szKey, const char * c_szName);

	protected:
		bool OnProcess();
		void OffLinePhase();
		void HandShakePhase();
		void LoginPhase();
		void SelectPhase();
		void LoadingPhase();
		void GamePhase();

		bool __IsNotPing();

		void __DownloadMark();
		void __DownloadSymbol(const std::vector<DWORD> & c_rkVec_dwGuildID);

		void __PlayInventoryItemUseSound(TItemPos uSlotPos);
		void __PlayInventoryItemDropSound(TItemPos uSlotPos);

		void __PlaySafeBoxItemDropSound(UINT uSlotPos);
		void __PlayMallItemDropSound(UINT uSlotPos);

		bool __CanActMainInstance();

		enum REFRESH_WINDOW_TYPE
		{
			RefreshStatus = (1 << 0),
			RefreshAlignmentWindow = (1 << 1),
			RefreshCharacterWindow = (1 << 2),
			RefreshEquipmentWindow = (1 << 3),
			RefreshInventoryWindow = (1 << 4),
			RefreshExchangeWindow = (1 << 5),
			RefreshSkillWindow = (1 << 6),
			RefreshSafeboxWindow  = (1 << 7),
			RefreshMessengerWindow = (1 << 8),
			RefreshGuildWindowInfoPage = (1 << 9),
			RefreshGuildWindowBoardPage = (1 << 10),
			RefreshGuildWindowMemberPage = (1 << 11),
			RefreshGuildWindowMemberPageGradeComboBox = (1 << 12),
			RefreshGuildWindowSkillPage = (1 << 13),
			RefreshGuildWindowGradePage = (1 << 14),
			RefreshTargetBoard = (1 << 15),
			RefreshMallWindow = (1 << 16),
		};

		void __RefreshStatus();
		void __RefreshAlignmentWindow();
		void __RefreshCharacterWindow();
		void __RefreshEquipmentWindow();
		void __RefreshInventoryWindow();
		void __RefreshExchangeWindow();
		void __RefreshSkillWindow();
		void __RefreshSafeboxWindow();
		void __RefreshMessengerWindow();
		void __RefreshGuildWindowInfoPage();
		void __RefreshGuildWindowBoardPage();
		void __RefreshGuildWindowMemberPage();
		void __RefreshGuildWindowMemberPageGradeComboBox();
		void __RefreshGuildWindowSkillPage();
		void __RefreshGuildWindowGradePage();
		void __RefreshTargetBoardByVID(DWORD dwVID);
		void __RefreshTargetBoardByName(const char * c_szName);
		void __RefreshTargetBoard();
		void __RefreshMallWindow();

		bool __SendHack(const char* c_szMsg);
		bool RecvErrorPacket(int header);
		bool RecvPingPacket();
		bool RecvDefaultPacket(int header);
		bool RecvPhasePacket();
		bool __RecvLoginSuccessPacket();
		bool __RecvLoginFailurePacket();
		bool __RecvEmpirePacket();
		bool __RecvLoginKeyPacket();
		bool __RecvPlayerCreateSuccessPacket();
		bool __RecvPlayerCreateFailurePacket();
		bool __RecvPlayerDestroySuccessPacket();
		bool __RecvPlayerDestroyFailurePacket();
		bool __RecvPreserveItemPacket();
		bool __RecvPlayerPoints();
		bool __RecvChangeName();
		bool RecvMainCharacter();
		bool RecvMainCharacter2_EMPIRE();
		bool RecvMainCharacter3_BGM();
		bool RecvMainCharacter4_BGM_VOL();

		void __SetFieldMusicFileName(const char* musicName);
		void __SetFieldMusicFileInfo(const char* musicName, float vol);
		bool RecvWarpPacket();
		bool RecvPVPPacket();
		bool RecvDuelStartPacket();
        bool RecvGlobalTimePacket();
		bool RecvCharacterAppendPacket();
		bool RecvCharacterAdditionalInfo();
		bool RecvCharacterUpdatePacket();
		bool RecvCharacterDeletePacket();
		bool RecvChatPacket();
		bool RecvOwnerShipPacket();
		bool RecvSyncPositionPacket();
		bool RecvWhisperPacket();
		bool RecvPointChange();
		bool RecvChangeSpeedPacket();

		bool RecvStunPacket();
		bool RecvDeadPacket();
		bool RecvCharacterMovePacket();

		bool RecvItemSetPacket();
		bool RecvItemUsePacket();
		bool RecvItemUpdatePacket();
		bool RecvItemGroundAddPacket();
		bool RecvItemGroundDelPacket();
		bool RecvItemOwnership();

		bool RecvQuickSlotAddPacket();
		bool RecvQuickSlotDelPacket();
		bool RecvQuickSlotMovePacket();

		bool RecvCharacterPositionPacket();
		bool RecvMotionPacket();

		bool RecvShopPacket();
		bool RecvShopSignPacket();
		bool RecvExchangePacket();
		bool RecvScriptPacket();
		bool RecvQuestInfoPacket();
		bool RecvQuestConfirmPacket();
		bool RecvRequestMakeGuild();
		bool RecvSkillLevel();
		bool RecvTargetPacket();
		bool RecvViewEquipPacket();
		bool RecvDamageInfoPacket();
		bool RecvMountPacket();
		bool RecvCreateFlyPacket();
		bool RecvFlyTargetingPacket();
		bool RecvAddFlyTargetingPacket();
		bool RecvMessenger();
		bool RecvGuild();
		bool RecvPartyInvite();
		bool RecvPartyAdd();
		bool RecvPartyUpdate();
		bool RecvPartyRemove();
		bool RecvPartyLink();
		bool RecvPartyUnlink();
		bool RecvPartyParameter();
		bool RecvSafeBoxSetPacket();
		bool RecvSafeBoxDelPacket();
		bool RecvSafeBoxWrongPasswordPacket();
		bool RecvSafeBoxSizePacket();
		bool RecvFishing();
		bool RecvDungeon();
		bool RecvTimePacket();
		bool RecvWalkModePacket();
		bool RecvChangeSkillGroupPacket();
		bool RecvRefineInformationPacket();
		bool RecvSpecialEffect();
		bool RecvSpecificEffect();
		bool RecvDragonSoulRefine();
		bool RecvNPCList();
		bool RecvLandPacket();
		bool RecvTargetCreatePacket();
		bool RecvTargetUpdatePacket();
		bool RecvTargetDeletePacket();
		bool RecvAffectAddPacket();
		bool RecvAffectRemovePacket();
		bool RecvChannelPacket();

	protected:
		bool ParseEmoticon(const char * pChatMsg, DWORD * pdwEmoticon);
		void OnConnectFailure();
		void OnScriptEventStart(int iSkin, int iIndex);

		void OnRemoteDisconnect();
		void OnDisconnect();

		void SetGameOnline();
		void SetGameOffline();
		BOOL IsGameOnline();

	protected:
		bool CheckPacket(TPacketHeader * pRetHeader);

		void __InitializeGamePhase();
		void __InitializeMarkAuth();
		void __GlobalPositionToLocalPosition(LONG& rGlobalX, LONG& rGlobalY);
		void __LocalPositionToGlobalPosition(LONG& rLocalX, LONG& rLocalY);

		bool __IsPlayerAttacking();
		bool __IsEquipItemInSlot(TItemPos Cell);

		void __ShowMapName(LONG lLocalX, LONG lLocalY);

		void __LeaveOfflinePhase() {}
		void __LeaveHandshakePhase() {}
		void __LeaveLoginPhase() {}
		void __LeaveSelectPhase() {}
		void __LeaveLoadingPhase() {}
		void __LeaveGamePhase();

		void __ClearNetworkActorManager();

		void __ClearSelectCharacterData();
		void __ConvertEmpireText(DWORD dwEmpireID, char* szText);

		void __RecvCharacterAppendPacket(SNetworkActorData * pkNetActorData);
		void __RecvCharacterUpdatePacket(SNetworkUpdateActorData * pkNetUpdateActorData);

		void __FilterInsult(char* szLine, UINT uLineLen);

		void __SetGuildID(DWORD id);

	protected:
		TPacketGCHandshake m_HandshakeData;
		DWORD m_dwChangingPhaseTime;
		DWORD m_dwBindupRetryCount;
		DWORD m_dwMainActorVID;
		DWORD m_dwMainActorRace;
		DWORD m_dwMainActorEmpire;
		DWORD m_dwMainActorSkillGroup;
		BOOL m_isGameOnline;
		BOOL m_isStartGame;

		DWORD m_dwGuildID;
		DWORD m_dwEmpireID;

		struct SServerTimeSync
		{
			DWORD m_dwChangeServerTime;
			DWORD m_dwChangeClientTime;
		} m_kServerTimeSync;

		void __ServerTimeSync_Initialize();
		DWORD m_dwLastGamePingTime;

		std::string	m_stID;
		std::string	m_stPassword;
		std::string	m_strLastCommand;
		std::string	m_strPhase;
		DWORD m_dwLoginKey;
		BOOL m_isWaitLoginKey;

		std::string m_stMarkIP;

		CFuncObject<CPythonNetworkStream>	m_phaseProcessFunc;
		CFuncObject<CPythonNetworkStream>	m_phaseLeaveFunc;

		PyObject*							m_poHandler;
		PyObject*							m_apoPhaseWnd[PHASE_WINDOW_NUM];
		PyObject*							m_poSerCommandParserWnd;

		CCheatDetectQueueMgr		m_kCheatQueueManager;

		TSimplePlayerInformation			m_akSimplePlayerInfo[PLAYER_PER_ACCOUNT];
		DWORD								m_adwGuildID[PLAYER_PER_ACCOUNT];
		std::string							m_astrGuildName[PLAYER_PER_ACCOUNT];
		bool m_bSimplePlayerInfo;

		CRef<CNetworkActorManager>			m_rokNetActorMgr;

		bool m_isRefreshStatus;
		bool m_isRefreshCharacterWnd;
		bool m_isRefreshEquipmentWnd;
		bool m_isRefreshInventoryWnd;
		bool m_isRefreshExchangeWnd;
		bool m_isRefreshSkillWnd;
		bool m_isRefreshSafeboxWnd;
		bool m_isRefreshMallWnd;
		bool m_isRefreshMessengerWnd;
		bool m_isRefreshGuildWndInfoPage;
		bool m_isRefreshGuildWndBoardPage;
		bool m_isRefreshGuildWndMemberPage;
		bool m_isRefreshGuildWndMemberPageGradeComboBox;
		bool m_isRefreshGuildWndSkillPage;
		bool m_isRefreshGuildWndGradePage;
		std::vector<std::string> m_EmoticonStringVector;

		struct STextConvertTable
		{
			char acUpper[26];
			char acLower[26];
			BYTE aacHan[5000][2];
		} m_aTextConvTable[3];
		struct SMarkAuth
		{
			CNetworkAddress m_kNetAddr;
			DWORD m_dwHandle;
			DWORD m_dwRandomKey;
		} m_kMarkAuth;
		DWORD m_dwSelectedCharacterIndex;

		CInsultChecker m_kInsultChecker;

		bool m_isEnableChatInsultFilter;
		bool m_bComboSkillFlag;

		std::deque<std::string> m_kQue_stHack;

	private:
		struct SDirectEnterMode
		{
			bool m_isSet;
			DWORD m_dwChrSlotIndex;
		} m_kDirectEnterMode;

		void __DirectEnterMode_Initialize();
		void __DirectEnterMode_Set(UINT uChrSlotIndex);
		bool __DirectEnterMode_IsSet();

	public:
		DWORD EXPORT_GetBettingGuildWarValue(const char* c_szValueName);

	private:
		struct SBettingGuildWar
		{
			DWORD m_dwBettingMoney;
			DWORD m_dwObserverCount;
		} m_kBettingGuildWar;

		CInstanceBase * m_pInstTarget;

		void __BettingGuildWar_Initialize();
		void __BettingGuildWar_SetObserverCount(UINT uObserverCount);
		void __BettingGuildWar_SetBettingMoney(UINT uBettingMoney);
};
