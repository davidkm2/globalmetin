#ifndef __INC_PACKET_H__
#define __INC_PACKET_H__
#include "stdafx.h"
#include "party.h"

enum HeaderGC
{
	HEADER_GC_CHARACTER_ADD = 1,					// 1
	HEADER_GC_CHARACTER_DEL,						// 2
	HEADER_GC_MOVE,									// 3
	HEADER_GC_CHAT,									// 4
	HEADER_GC_SYNC_POSITION,						// 5

	HEADER_GC_LOGIN_SUCCESS,						// 6
	HEADER_GC_LOGIN_FAILURE,						// 7

	HEADER_GC_CHARACTER_CREATE_SUCCESS,				// 8
	HEADER_GC_CHARACTER_CREATE_FAILURE,				// 9
	HEADER_GC_CHARACTER_DELETE_SUCCESS,				// 10
	HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID,		// 11

	HEADER_GC_STUN,									// 12
	HEADER_GC_DEAD,									// 13

	HEADER_GC_MAIN_CHARACTER_OLD,					// 14
	HEADER_GC_CHARACTER_POINTS,						// 15
	HEADER_GC_CHARACTER_POINT_CHANGE,				// 16
	HEADER_GC_CHANGE_SPEED,							// 17
	HEADER_GC_CHARACTER_UPDATE,						// 18

	HEADER_GC_ITEM_SET,								// 19
	HEADER_GC_ITEM_USE,								// 20
	HEADER_GC_ITEM_DROP,							// 21
	HEADER_GC_ITEM_UPDATE,							// 22

	HEADER_GC_ITEM_GROUND_ADD,						// 23
	HEADER_GC_ITEM_GROUND_DEL,						// 24

	HEADER_GC_QUICKSLOT_ADD,						// 25
	HEADER_GC_QUICKSLOT_DEL,						// 26
	HEADER_GC_QUICKSLOT_SWAP,						// 27

	HEADER_GC_ITEM_OWNERSHIP,						// 28

	HEADER_GC_WHISPER,								// 29
	HEADER_GC_MOTION,								// 30

	HEADER_GC_SHOP,									// 31
	HEADER_GC_SHOP_SIGN,							// 32

	HEADER_GC_DUEL_START,							// 33
	HEADER_GC_PVP,									// 34
	HEADER_GC_EXCHANGE,								// 35
	HEADER_GC_CHARACTER_POSITION,					// 36

	HEADER_GC_PING,									// 37

	HEADER_GC_SCRIPT,								// 38
	HEADER_GC_QUEST_CONFIRM,						// 39

	HEADER_GC_MOUNT,								// 40
	HEADER_GC_OWNERSHIP,							// 41
	HEADER_GC_TARGET,								// 42

	HEADER_GC_WARP,									// 43

	HEADER_GC_ADD_FLY_TARGETING,					// 44
	HEADER_GC_CREATE_FLY,							// 45
	HEADER_GC_FLY_TARGETING,						// 46
	HEADER_GC_SKILL_LEVEL,							// 47

	HEADER_GC_MESSENGER,							// 48
	HEADER_GC_GUILD,								// 49

	HEADER_GC_PARTY_INVITE,							// 50
	HEADER_GC_PARTY_ADD,							// 51
	HEADER_GC_PARTY_UPDATE,							// 52
	HEADER_GC_PARTY_REMOVE,							// 53

	HEADER_GC_QUEST_INFO,							// 54
	HEADER_GC_REQUEST_MAKE_GUILD,					// 55
	HEADER_GC_PARTY_PARAMETER,						// 56

	HEADER_GC_SAFEBOX_SET,							// 57
	HEADER_GC_SAFEBOX_DEL,							// 58
	HEADER_GC_SAFEBOX_WRONG_PASSWORD,				// 59
	HEADER_GC_SAFEBOX_SIZE,							// 60

	HEADER_GC_FISHING,								// 61

	HEADER_GC_EMPIRE,								// 62

	HEADER_GC_PARTY_LINK,							// 63
	HEADER_GC_PARTY_UNLINK,							// 64

	HEADER_GC_REFINE_INFORMATION,					// 65

	HEADER_GC_VIEW_EQUIP,							// 66

	HEADER_GC_MARK_BLOCK,							// 67
	HEADER_GC_MARK_IDXLIST,							// 68

	HEADER_GC_TIME,									// 69
	HEADER_GC_CHANGE_NAME,							// 70

	HEADER_GC_DUNGEON,								// 71

	HEADER_GC_WALK_MODE,							// 72
	HEADER_GC_SKILL_GROUP,							// 73

	HEADER_GC_MAIN_CHARACTER,						// 74

	HEADER_GC_SEPCIAL_EFFECT,						// 75

	HEADER_GC_NPC_POSITION,							// 76

	HEADER_GC_LOGIN_KEY,							// 77

	HEADER_GC_CHANNEL,								// 78

	HEADER_GC_MALL_OPEN,							// 79
	HEADER_GC_TARGET_UPDATE,						// 80
	HEADER_GC_TARGET_DELETE,						// 81
	HEADER_GC_TARGET_CREATE,						// 82

	HEADER_GC_AFFECT_ADD,							// 83
	HEADER_GC_AFFECT_REMOVE,						// 84

	HEADER_GC_MALL_SET,								// 85
	HEADER_GC_MALL_DEL,								// 86

	HEADER_GC_LAND_LIST,							// 87
	HEADER_GC_LOVER_INFO,							// 88
	HEADER_GC_LOVE_POINT_UPDATE,					// 89

	HEADER_GC_SYMBOL_DATA,							// 90

	HEADER_GC_DIG_MOTION,							// 91

	HEADER_GC_DAMAGE_INFO,							// 92
	HEADER_GC_CHAR_ADDITIONAL_INFO,					// 93

	HEADER_GC_MAIN_CHARACTER3_BGM,					// 94
	HEADER_GC_MAIN_CHARACTER4_BGM_VOL,				// 95
	
	HEADER_GC_AUTH_SUCCESS,							// 96

	HEADER_GC_SPECIFIC_EFFECT,						// 97

	HEADER_GC_DRAGON_SOUL_REFINE,					// 98
	HEADER_GC_RESPOND_CHANNELSTATUS,				// 99

#ifdef __GEM__
	HEADER_GC_GEM_SHOP,								// 100
#endif

#ifdef __EXTEND_INVEN__
	HEADER_GC_EXTEND_INVEN,							// 101
#endif

#ifdef __MINI_GAME_RUMI__
	HEADER_GC_MINI_GAME_RUMI,						// 102
#endif

#ifdef __SHOW_CHEST_DROP__
	HEADER_GC_CHEST_DROP_INFO,						// 103
#endif

#ifdef __COSTUME_ACCE__
	HEADER_GC_ACCE,									// 104
#endif

#ifdef __CHANGELOOK__
	HEADER_GC_CL,									// 105
#endif

#ifdef __SEND_TARGET_INFO__
	HEADER_GC_TARGET_INFO,							// 106
#endif

#ifdef __SUPPORT__
	HEADER_GC_SUPPORT_SKILL,						// 107
#endif

#ifdef __DAMAGE_TOP__
	HEADER_GC_DAMAGE_TOP,							// 109
#endif

#ifdef __RANKING__
	HEADER_GC_RANKING,								// 110
#endif

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
#ifdef __ACCUMULATE_DAMAGE_DISPLAY__
	HEADER_GC_ACCUMULATE,							// 111
#endif
	HEADER_GC_ATTENDANCE_REQUEST_DATA,				// 112
	HEADER_GC_ATTENDANCE_REQUEST_REWARD_LIST,		// 113
#endif

#ifdef __MINI_GAME_FISH__
	HEADER_GC_FISH_EVENT,							// 114
#endif

#ifdef __MINI_GAME_CATCH_KING__
	HEADER_GC_MINI_GAME_CATCH_KING,					// 115
#endif

#ifdef __MAILBOX__
	HEADER_GC_MAILBOX,								// 116
#endif

#ifdef __ANTICHEAT__
	HEADER_GC_ANTICHEAT_BLACKLIST,					// 117
#endif

#ifdef __OFFLINE_PRIVATE_SHOP__
	HEADER_GC_OFFLINE_PRIVATE_SHOP,					// 118
#endif

	HEADER_GC_TIME_SYNC				= 0xfc,
	HEADER_GC_PHASE					= 0xfd,
	HEADER_GC_HANDSHAKE				= 0xff,
};

enum HeaderCG
{
	HEADER_CG_LOGIN = 1,							// 1
	HEADER_CG_ATTACK,								// 2
	HEADER_CG_CHAT,									// 3
	HEADER_CG_CHARACTER_CREATE,						// 4
	HEADER_CG_CHARACTER_DELETE,						// 5
	HEADER_CG_CHARACTER_SELECT,						// 6
	HEADER_CG_MOVE,									// 7
	HEADER_CG_SYNC_POSITION,						// 8
	HEADER_CG_ENTERGAME,							// 9

	HEADER_CG_ITEM_USE,								// 10
	HEADER_CG_ITEM_DROP,							// 11
	HEADER_CG_ITEM_MOVE,							// 12
	HEADER_CG_ITEM_PICKUP,							// 13

	HEADER_CG_QUICKSLOT_ADD,						// 14
	HEADER_CG_QUICKSLOT_DEL,						// 15
	HEADER_CG_QUICKSLOT_SWAP,						// 16
	HEADER_CG_WHISPER,								// 17
	HEADER_CG_ITEM_DROP2,							// 18

	HEADER_CG_ON_CLICK,								// 19
	HEADER_CG_EXCHANGE,								// 20
	HEADER_CG_CHARACTER_POSITION,					// 21
	HEADER_CG_SCRIPT_ANSWER,						// 22
	HEADER_CG_QUEST_INPUT_STRING,					// 23
	HEADER_CG_QUEST_CONFIRM,						// 24

	HEADER_CG_SHOP,									// 25
	HEADER_CG_FLY_TARGETING,						// 26
	HEADER_CG_USE_SKILL,							// 27
	HEADER_CG_ADD_FLY_TARGETING,					// 28
	HEADER_CG_SHOOT,								// 29
	HEADER_CG_MYSHOP,								// 30

	HEADER_CG_ITEM_USE_TO_ITEM,						// 31
	HEADER_CG_TARGET,								// 32

	HEADER_CG_WARP,									// 33
	HEADER_CG_SCRIPT_BUTTON,						// 34
	HEADER_CG_MESSENGER,							// 35

	HEADER_CG_MALL_CHECKOUT,						// 36
	HEADER_CG_SAFEBOX_CHECKIN,						// 37
	HEADER_CG_SAFEBOX_CHECKOUT,						// 38

	HEADER_CG_PARTY_INVITE,							// 39
	HEADER_CG_PARTY_INVITE_ANSWER,					// 40
	HEADER_CG_PARTY_REMOVE,							// 41
	HEADER_CG_PARTY_SET_STATE,						// 42
	HEADER_CG_PARTY_USE_SKILL,						// 43
	HEADER_CG_SAFEBOX_ITEM_MOVE,					// 45
	HEADER_CG_PARTY_PARAMETER,						// 46

	HEADER_CG_GUILD,								// 47
	HEADER_CG_ANSWER_MAKE_GUILD,					// 48

	HEADER_CG_FISHING,								// 49

	HEADER_CG_ITEM_GIVE,							// 50

	HEADER_CG_EMPIRE,								// 51

	HEADER_CG_REFINE,								// 52

	HEADER_CG_MARK_LOGIN,							// 53
	HEADER_CG_MARK_CRCLIST,							// 54
	HEADER_CG_MARK_UPLOAD,							// 55
	HEADER_CG_MARK_IDXLIST,							// 56

	HEADER_CG_HACK,									// 57
	HEADER_CG_CHANGE_NAME,							// 58
	HEADER_CG_LOGIN2,								// 59
	HEADER_CG_DUNGEON,								// 60
	HEADER_CG_LOGIN3,								// 61

	HEADER_CG_GUILD_SYMBOL_UPLOAD,					// 62
	HEADER_CG_SYMBOL_CRC,							// 63

	HEADER_CG_SCRIPT_SELECT_ITEM,					// 64

	HEADER_CG_DRAGON_SOUL_REFINE,					// 65
	HEADER_CG_STATE_CHECKER,						// 66

#ifdef __GEM__
	HEADER_CG_GEM_SHOP,								// 67
#endif

#ifdef __EXTEND_INVEN__
	HEADER_CG_EXTEND_INVEN,							// 68
#endif

#ifdef __MINI_GAME_RUMI__
	HEADER_CG_MINI_GAME_RUMI,						// 69
#endif

#ifdef __GROWTH_PET__
	HEADER_CG_PET_SET_NAME,							// 70
#endif

	HEADER_CG_ITEM_DESTROY,							// 71
	HEADER_CG_ITEM_SELL,							// 72

#ifdef __SHOW_CHEST_DROP__
	HEADER_CG_CHEST_DROP_INFO,						// 73
#endif

#ifdef __COSTUME_ACCE__
	HEADER_CG_ACCE,									// 74
#endif

#ifdef __CHANGELOOK__
	HEADER_CG_CL,									// 75
#endif

#ifdef __SEND_TARGET_INFO__
	HEADER_CG_TARGET_INFO_LOAD,						// 76
#endif

#ifdef __PARTY_MATCH__
	HEADER_CG_PARTY_MATCH,							// 77
#endif

#ifdef __SKILL_BOOK_COMBINATION__
	HEADER_CG_SKILL_BOOK_COMBINATION,				// 79
#endif

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
	HEADER_CG_ATTENDANCE_BUTTON_CLICK,				// 80
	HEADER_CG_ATTENDANCE_REQUEST_REWARD_LIST,		// 81
	HEADER_CG_ATTENDANCE_REQUEST_DATA,				// 82
#endif

#ifdef __MINI_GAME_FISH__
	HEADER_CG_FISH_EVENT,							// 83
#endif

#ifdef __MINI_GAME_CATCH_KING__
	HEADER_CG_MINI_GAME_CATCH_KING,					// 84
#endif

#ifdef __MAILBOX__
	HEADER_CG_MAILBOX,								// 85
#endif

#ifdef __ATTR_6TH_7TH__
	HEADER_CG_ATTR67_ADD,							// 86
#endif

	HEADER_CG_TIME_SYNC					= 0xfc,
	HEADER_CG_CLIENT_VERSION			= 0xfd,
	HEADER_CG_CLIENT_VERSION2			= 0xf1,
	HEADER_CG_PONG						= 0xfe,
	HEADER_CG_HANDSHAKE					= 0xff,
};

enum HeaderGG
{
	HEADER_GG_LOGIN = 1,							// 1
	HEADER_GG_LOGOUT,								// 2
	HEADER_GG_RELAY,								// 3
	HEADER_GG_NOTICE,								// 4
	HEADER_GG_SHUTDOWN,								// 5
	HEADER_GG_GUILD,								// 6
	HEADER_GG_DISCONNECT,							// 7
	HEADER_GG_SHOUT,								// 8
	HEADER_GG_SETUP,								// 9
	HEADER_GG_MESSENGER_ADD,						// 10
	HEADER_GG_MESSENGER_REMOVE,						// 11
	HEADER_GG_FIND_POSITION,						// 12
	HEADER_GG_WARP_CHARACTER,						// 13
	HEADER_GG_GUILD_WAR_ZONE_MAP_INDEX, 			// 14 
	HEADER_GG_TRANSFER,								// 15
	HEADER_GG_XMAS_WARP_SANTA,						// 16
	HEADER_GG_XMAS_WARP_SANTA_REPLY,				// 17
	HEADER_GG_RELOAD_CRC_LIST,						// 19
	HEADER_GG_LOGIN_PING,							// 20
	HEADER_GG_CHECK_CLIENT_VERSION,					// 21
	HEADER_GG_BLOCK_CHAT,							// 22

	HEADER_GG_CHECK_AWAKENESS,						// 23
#ifdef __FULLNOTICE__
	HEADER_GG_BIG_NOTICE,							// 24
#endif
#ifdef __MESSENGER_BLOCK__
	HEADER_GG_MESSENGER_BLOCK_ADD,					// 25
	HEADER_GG_MESSENGER_BLOCK_REMOVE,				// 26
#endif
#ifdef __REMOTE_EXCHANGE__
	HEADER_GG_REMOTE_EXCHANGE,						// 29
#endif
#ifdef __DAMAGE_TOP__
	HEADER_GG_REGISTER_DAMAGE,						// 30
#endif
};

#pragma pack(1)
///-------------------------------------- GG PACKETS --------------------------------------

typedef struct SPacketGGSetup
{
	BYTE	bHeader;
	WORD	wPort;
	BYTE	bChannel;
} TPacketGGSetup;

typedef struct SPacketGGLogin
{
	BYTE	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	DWORD	dwPID;
#ifdef __REMOTE_EXCHANGE__
	DWORD	dwVID;
#endif
	BYTE	bEmpire;
	long	lMapIndex;
	BYTE	bChannel;
} TPacketGGLogin;

typedef struct SPacketGGLogout
{
	BYTE	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGGLogout;

typedef struct SPacketGGRelay
{
	BYTE	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	long	lSize;
} TPacketGGRelay;

typedef struct SPacketGGNotice
{
	BYTE	bHeader;
	long	lSize;
} TPacketGGNotice;

typedef struct SPacketGGShutdown
{
	BYTE	bHeader;
} TPacketGGShutdown;

typedef struct SPacketGGGuild
{
	BYTE	bHeader;
	BYTE	bSubHeader;
	DWORD	dwGuild;
} TPacketGGGuild;

enum
{
	GUILD_SUBHEADER_GG_CHAT,
	GUILD_SUBHEADER_GG_SET_MEMBER_COUNT_BONUS,
};

typedef struct SPacketGGGuildChat
{
	BYTE	bHeader;
	BYTE	bSubHeader;
	DWORD	dwGuild;
	char	szText[CHAT_MAX_LEN + 1];
} TPacketGGGuildChat;

typedef struct SPacketGGParty
{
	BYTE	header;
	BYTE	subheader;
	DWORD	pid;
	DWORD	leaderpid;
} TPacketGGParty;

enum
{
	PARTY_SUBHEADER_GG_CREATE,
	PARTY_SUBHEADER_GG_DESTROY,
	PARTY_SUBHEADER_GG_JOIN,
	PARTY_SUBHEADER_GG_QUIT,
};

typedef struct SPacketGGDisconnect
{
	BYTE	bHeader;
	char	szLogin[LOGIN_MAX_LEN + 1];
} TPacketGGDisconnect;

typedef struct SPacketGGShout
{
	BYTE	bHeader;
	BYTE	bEmpire;
	char	szText[CHAT_MAX_LEN + 1];
} TPacketGGShout;

typedef struct SPacketGGXmasWarpSanta
{
	BYTE	bHeader;
	BYTE	bChannel;
	long	lMapIndex;
} TPacketGGXmasWarpSanta;

typedef struct SPacketGGXmasWarpSantaReply
{
	BYTE	bHeader;
	BYTE	bChannel;
} TPacketGGXmasWarpSantaReply;

typedef struct SPacketGGMessenger
{
	BYTE        bHeader;
	char        szAccount[CHARACTER_NAME_MAX_LEN + 1];
	char        szCompanion[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGGMessenger;

typedef struct SPacketGGFindPosition
{
	BYTE header;
	DWORD dwFromPID; // �� ��ġ�� �����Ϸ��� ���
	DWORD dwTargetPID; // ã�� ���
} TPacketGGFindPosition;

typedef struct SPacketGGWarpCharacter
{
	BYTE header;
	DWORD pid;
	long x;
	long y;
#ifdef __CMD_WARP_IN_DUNGEON__
	int mapIndex;
#endif
} TPacketGGWarpCharacter;

typedef struct SPacketGGGuildWarMapIndex
{
	BYTE bHeader;
	DWORD dwGuildID1;
	DWORD dwGuildID2;
	long lMapIndex;
} TPacketGGGuildWarMapIndex;

typedef struct SPacketGGTransfer
{
	BYTE	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	long	lX, lY;
} TPacketGGTransfer;

typedef struct SPacketGGLoginPing
{
	BYTE	bHeader;
	char	szLogin[LOGIN_MAX_LEN + 1];
} TPacketGGLoginPing;

typedef struct SPacketGGBlockChat
{
	BYTE	bHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	long	lBlockDuration;
} TPacketGGBlockChat;

#ifdef __REMOTE_EXCHANGE__
typedef struct SPacketGGRemoteExchange
{
	BYTE		bHeader;
	// BYTE		bSubHeader;
	// char		szCharName[CHARACTER_NAME_MAX_LEN + 1];
	// DWORD		arg1;
	// BYTE		arg2;
	// TItemPos	Pos;
	LPCHARACTER pkChar;
} TPacketGGRemoteExchange;

enum
{
	REMOTE_EXCHANGE_SUBHEADER_GG_START,	/* arg1 == vid of target character */
	REMOTE_EXCHANGE_SUBHEADER_GG_ITEM_ADD,	/* arg1 == position of item */
	REMOTE_EXCHANGE_SUBHEADER_GG_ITEM_DEL,	/* arg1 == position of item */
	REMOTE_EXCHANGE_SUBHEADER_GG_ELK_ADD,	/* arg1 == amount of gold */
#ifdef __CHEQUE__
	REMOTE_EXCHANGE_SUBHEADER_GG_CHEQUE_ADD,	/* arg1 == amount of cheque */
#endif
	REMOTE_EXCHANGE_SUBHEADER_GG_ACCEPT,	/* arg1 == not used */
	REMOTE_EXCHANGE_SUBHEADER_GG_CANCEL,	/* arg1 == not used */
};
#endif

///-------------------------------------- CG PACKETS --------------------------------------
#ifdef __MINI_GAME_RUMI__
typedef struct command_minigame_rumi
{
	BYTE        header;
	BYTE		subheader;
} TPacketCGMiniGameRumi;

typedef struct command_minigame_rumis
{
	BYTE        header;
	BYTE		subheader;
	WORD		size;
} TPacketGCMiniGameRumi;

enum
{
	MINI_GAME_RUMI_SUBHEADER_CG_START,
	MINI_GAME_RUMI_SUBHEADER_CG_EXIT,
	MINI_GAME_RUMI_SUBHEADER_CG_HAND_CARD_CLICK,
	MINI_GAME_RUMI_SUBHEADER_CG_DECK_CARD_CLICK,
	MINI_GAME_RUMI_SUBHEADER_CG_FIELD_CARD_CLICK,
};

enum
{
	MINI_GAME_RUMI_SUBHEADER_GC_START,
	MINI_GAME_RUMI_SUBHEADER_GC_EXIT,
	MINI_GAME_RUMI_SUBHEADER_GC_HAND_CARD_CLICK,
	MINI_GAME_RUMI_SUBHEADER_GC_DECK_CARD_CLICK,
	MINI_GAME_RUMI_SUBHEADER_GC_FIELD_CARD_CLICK,
	MINI_GAME_RUMI_SUBHEADER_GC_SCORE_INCREASE,
};
#endif

#ifdef __EXTEND_INVEN__
typedef struct command_extend_inven
{
	BYTE        header;
	BYTE		subheader;
	BYTE		bIndex;
} TPacketCGExtendInven;

typedef struct command_extend_invens
{
	BYTE        header;
	BYTE		bMsg;
	BYTE		bNeed;
} TPacketGCExtendInven;

enum
{
	EXTEND_INVEN_SUBHEADER_GC_CLICK,
	EXTEND_INVEN_SUBHEADER_GC_UPGRADE,
};

enum
{
	EX_INVEN_FAIL_FALL_SHORT,
	EX_INVEN_SUCCESS,
	EX_INVEN_FAIL_FOURTH_PAGE_STAGE_MAX,
};
#endif

#ifdef __GEM__
typedef struct command_gem_shop
{
	BYTE        header;
	BYTE		subheader;
} TPacketCGGemShop;

typedef struct command_gem_shops
{
	BYTE	header;
	BYTE	subheader;
	int     RefreshTime;
	TGemShopItem shopItems[GEM_SLOTS_MAX_NUM];
} TPacketGCGemShop;

enum
{
	GEM_SHOP_SUBHEADER_GC_BUY,
	GEM_SHOP_SUBHEADER_GC_ADD,
	GEM_SHOP_SUBHEADER_GC_REFRESH,
	GEM_SHOP_SUBHEADER_GC_OPEN,
	GEM_SHOP_SUBHEADER_GC_CLOSE,
};

enum
{
	GEM_SHOP_SUBHEADER_CG_BUY,
	GEM_SHOP_SUBHEADER_CG_ADD,
	GEM_SHOP_SUBHEADER_CG_REFRESH,
	GEM_SHOP_SUBHEADER_CG_CLOSE,
};
#endif

#ifdef __GROWTH_PET__
typedef struct packet_RequestPetName
{
	BYTE byHeader;
	char petname[13];

}TPacketCGRequestPetName;
#endif

#ifdef __SKILL_BOOK_COMBINATION__
typedef struct SPacketCGSkillBookCombination
{
	BYTE header;
	int List[10];
	int	mod;
} TPacketCGSkillBookCombination;
#endif

#ifdef __RANKING__
enum ERKType
{
	TYPE_RK_SOLO,
#ifdef __RANKING_PARTY__
	TYPE_RK_PARTY,
#endif
	TYPE_RK_MAX
};

enum ERKSoloCategory
{
	SOLO_RK_CATEGORY_BF_WEAK,
	SOLO_RK_CATEGORY_BF_TOTAL,
	SOLO_RK_CATEGORY_MAX
};

#ifdef __RANKING_PARTY__
enum ERKPartyCategory
{
	PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_ALL,
	PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_NOW_WEEK,
	PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_PAST_WEEK,
#ifdef __12ZI__
	PARTY_RK_CATEGORY_CZ_MOUSE,
	PARTY_RK_CATEGORY_CZ_COW,
	PARTY_RK_CATEGORY_CZ_TIGER,
	PARTY_RK_CATEGORY_CZ_RABBIT,
	PARTY_RK_CATEGORY_CZ_DRAGON,
	PARTY_RK_CATEGORY_CZ_SNAKE,
	PARTY_RK_CATEGORY_CZ_HORSE,
	PARTY_RK_CATEGORY_CZ_SHEEP,
	PARTY_RK_CATEGORY_CZ_MONKEY,
	PARTY_RK_CATEGORY_CZ_CHICKEN,
	PARTY_RK_CATEGORY_CZ_DOG,
	PARTY_RK_CATEGORY_CZ_PIG,
#endif
#ifdef __DEFENSE_WAVE__
	PARTY_RK_CATEGORY_DEFENSE_WAVE,
#endif
	PARTY_RK_CATEGORY_MAX
};
#endif

typedef struct SRankingInfo
{
	BYTE	bCategory;
	int		iRankingIdx;
	char	szCharName[CHARACTER_NAME_MAX_LEN + 4];
	char	szMembersName[CHARACTER_NAME_MAX_LEN * PARTY_MAX_MEMBER + 14 + 1];
	int		iRecord0;
	int		iRecord1;
	int		iTime;
	BYTE	bEmpire;
	SRankingInfo()
	{
		bCategory = 0;
		iRankingIdx = 0;
		iRecord0 = 0;
		iRecord1 = 0;
		iTime = 0;
		bEmpire = 0;
	}
} TRankingInfo;

typedef std::map<int, TRankingInfo> TRankingInfoMap;
typedef std::map<int, TRankingInfoMap> TInfoMap;

typedef struct SPacketRanking
{
	BYTE	bHeader;
	WORD	wSize;
} TPacketRanking;
#endif

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
typedef struct SRewardItem
{
	BYTE bDay;
	DWORD dwVnum;
	DWORD dwCount;
} TRewardItem;

#ifdef __ACCUMULATE_DAMAGE_DISPLAY__
typedef struct SPacketGCAccumulate
{
	BYTE	bHeader;
	DWORD	dwVid;
	DWORD	dwCount;
} TPacketGCAccumulate;
#endif

typedef struct SPacketGCAttendanceRequestRewardList
{
	BYTE	bHeader;
	WORD	wSize;
} TPacketGCAttendanceRequestRewardList;

typedef struct SPacketGCAttendanceRequestData
{
	BYTE	bHeader;
	BYTE	bType;
	BYTE	bValue;
} TPacketGCAttendanceRequestData;

typedef struct SPacketCGAttendanceButtonClick
{
	BYTE	bHeader;
	int		iNumber;
} TPacketCGAttendanceButtonClick;

typedef struct SPacketCGAttendanceRequestData
{
	BYTE	bHeader;
	BYTE	bType;
} TPacketCGAttendanceRequestData;
#endif

#ifdef __MINI_GAME_FISH__
enum EFishCGSubHeaders
{
	FISH_EVENT_SUBHEADER_CG_BOX_USE,
	FISH_EVENT_SUBHEADER_CG_SHAPE_ADD,
	FISH_EVENT_SUBHEADER_CG_REQUEST_BLOCK,
};

enum EFishGCSubHeaders
{
	FISH_EVENT_SUBHEADER_GC_BOX_USE,
	FISH_EVENT_SUBHEADER_GC_SHAPE_ADD,
	FISH_EVENT_SUBHEADER_GC_REWARD,
	FISH_EVENT_SUBHEADER_GC_COUNT,
};

typedef struct SPacketCGFishEvent
{
	BYTE	bHeader;
	BYTE	bSubheader;
} TPacketCGFishEvent;

typedef struct SPacketGCFishEvent
{
	BYTE	bHeader;
	BYTE	bSubheader;
	DWORD	dwFirstArg;
	DWORD	dwSecondArg;
} TPacketGCFishEvent;
#endif

#ifdef __MINI_GAME_CATCH_KING__
enum
{
	SUBHEADER_GC_CATCH_KING_START,
	SUBHEADER_GC_CATCH_KING_SET_CARD,
	SUBHEADER_GC_CATCH_KING_RESULT_FIELD,
	SUBHEADER_GC_CATCH_KING_SET_END_CARD,
	SUBHEADER_GC_CATCH_KING_REWARD
};

typedef struct SPacketCGMiniGameCatchKing
{
	BYTE	bHeader;
	BYTE	bSubheader;
	BYTE	bSubArgument;
} TPacketCGMiniGameCatchKing;

typedef struct SPacketGCMiniGameCatchKing
{
	BYTE	bHeader;
	WORD	wSize;
	BYTE	bSubheader;
} TPacketGCMiniGameCatchKing;

typedef struct SPacketGCMiniGameCatchKingResult
{
	DWORD	dwPoints;
	BYTE	bRowType;
	BYTE	bCardPos;
	BYTE	bCardValue;
	bool	bKeepFieldCard;
	bool	bDestroyHandCard;
	bool	bGetReward;
	bool 	bIsFiveNearBy;
} TPacketGCMiniGameCatchKingResult;

typedef struct SPacketGCMiniGameCatchKingSetEndCard
{
	BYTE	bCardPos;
	BYTE	bCardValue;
} TPacketGCMiniGameCatchKingSetEndCard;
#endif

#ifdef __MAILBOX__
typedef struct SPacketCGMailBox
{
	BYTE bHeader;
	BYTE bSubHeader;
} TPacketCGMailBox;

typedef struct SPacketGCMailBox
{
	BYTE bHeader;
	WORD wSize;
	BYTE bSubHeader;
} TPacketGCMailBox;

enum EMailCGSubHeaders
{
	MAILBOX_CG_CLOSE,
	MAILBOX_CG_WRITE_CONFIRM,
	MAILBOX_CG_POST_WRITE,
	MAILBOX_CG_POST_GET_ITEMS,
	MAILBOX_CG_REQUEST_POST_ADD_DATA,
	MAILBOX_CG_POST_DELETE,
	MAILBOX_CG_ALL_POST_DELETE,
	MAILBOX_CG_ALL_GET_ITEMS,
};

enum EMailGCSubHeaders
{
	MAILBOX_GC_OPEN,
	MAILBOX_GC_POST_WRITE_CONFIRM,
	MAILBOX_GC_POST_WRITE,
	MAILBOX_GC_SET,
	MAILBOX_GC_ADD_DATA,
	MAILBOX_GC_POST_GET_ITEMS,
	MAILBOX_GC_POST_DELETE,
	MAILBOX_GC_POST_ALL_DELETE,
	MAILBOX_GC_POST_ALL_GET_ITEMS,
	MAILBOX_GC_UNREAD_DATA,
	MAILBOX_GC_ITEM_EXPIRE,
	MAILBOX_GC_CLOSE,
	MAILBOX_GC_SYSTEM_CLOSE,
};

enum EMailWriteHeaders
{
	POST_WRITE_FAIL,
	POST_WRITE_OK,
	POST_WRITE_INVALID_NAME,
	POST_WRITE_TARGET_BLOCKED,
	POST_WRITE_BLOCKED_ME,
	POST_WRITE_FULL_MAILBOX,
	POST_WRITE_WRONG_TITLE,
	POST_WRITE_YANG_NOT_ENOUGHT,
	POST_WRITE_WON_NOT_ENOUGHT,
	POST_WRITE_WRONG_MESSAGE,
	POST_WRITE_WRONG_ITEM,
	POST_WRITE_LEVEL_NOT_ENOUGHT,
};

enum EMailGetItemsHeaders
{
	POST_GET_ITEMS_FAIL,
	POST_GET_ITEMS_OK,
	POST_GET_ITEMS_NONE,
	POST_GET_ITEMS_NOT_ENOUGHT_INVENTORY,
	POST_GET_ITEMS_YANG_OVERFLOW,
	POST_GET_ITEMS_WON_OVERFLOW,
	POST_GET_ITEMS_FAIL_BLOCK_CHAR,
	POST_GET_ITEMS_RESULT_MAX,
};

enum EMailDeleteHeaders
{
	POST_DELETE_FAIL,
	POST_DELETE_OK,
	POST_DELETE_FAIL_EXIST_ITEMS,
};

enum EMailAllDeleteHeaders
{
	POST_ALL_DELETE_FAIL,
	POST_ALL_DELETE_OK,
	POST_ALL_DELETE_FAIL_EMPTY,
	POST_ALL_DELETE_FAIL_DONT_EXIST,
};

enum EMailAllGetItemsHeaders
{
	POST_ALL_GET_ITEMS_FAIL,
	POST_ALL_GET_ITEMS_OK,
	POST_ALL_GET_ITEMS_EMPTY,
	POST_ALL_GET_ITEMS_FAIL_DONT_EXIST,
	POST_ALL_GET_ITEMS_FAIL_CANT_GET,
};
#endif

#ifdef __ANTICHEAT__
typedef struct packet_anticheat_blacklist
{
	BYTE header;
	char content[1024];
} TPacketGCAntiCheatBlacklist;
#endif

#ifdef __ATTR_6TH_7TH__
typedef struct packet_attr67_add
{
	BYTE bHeader;
	BYTE bWindowType;
	WORD wSlotIndex;
	BYTE bMaterialCount;
	BYTE bSupportCount;
	BYTE bSupportWindowType;
	WORD wSupportSlotIndex;
} TPacketCGAttr67Add;
#endif

#ifdef __OFFLINE_PRIVATE_SHOP__
typedef struct packet_offline_private_shop_info
{
	BYTE bHeader;
	WORD wSize;
	BYTE bSubHeader;
} TPacketGCOfflinePrivateShopInfo;

enum EOfflinePrivateShopSubHeaders
{
	OFFLINE_PRIVATE_SHOP_INFO,
};
#endif

typedef struct command_handshake
{
	BYTE	bHeader;
	DWORD	dwHandshake;
	DWORD	dwTime;
	long	lDelta;
} TPacketCGHandshake;

typedef struct command_login
{
	BYTE	header;
	char	login[LOGIN_MAX_LEN + 1];
	char	passwd[PASSWD_MAX_LEN + 1];
} TPacketCGLogin;

typedef struct command_login2
{
	BYTE	header;
	char	login[LOGIN_MAX_LEN + 1];
	DWORD	dwLoginKey;
	DWORD	adwClientKey[4];
} TPacketCGLogin2;

typedef struct command_login3
{
	BYTE	header;
	char	login[LOGIN_MAX_LEN + 1];
	char	passwd[PASSWD_MAX_LEN + 1];
	DWORD	adwClientKey[4];
} TPacketCGLogin3;

typedef struct packet_login_key
{
	BYTE	bHeader;
	DWORD	dwLoginKey;
} TPacketGCLoginKey;

typedef struct command_player_select
{
	BYTE	header;
	BYTE	index;
#ifdef __MULTI_LANGUAGE__
	char	szLanguage[2 + 1];
#endif
} TPacketCGPlayerSelect;

typedef struct command_player_delete
{
	BYTE	header;
	BYTE	index;
	char	private_code[8];
} TPacketCGPlayerDelete;

typedef struct command_player_create
{
	BYTE        header;
	BYTE        index;
	char        name[CHARACTER_NAME_MAX_LEN + 1];
	WORD        job;
	BYTE	shape;
	BYTE	Con;
	BYTE	Int;
	BYTE	Str;
	BYTE	Dex;
} TPacketCGPlayerCreate;

typedef struct command_player_create_success
{
	BYTE		header;
	BYTE		bAccountCharacterIndex;
	TSimplePlayer	player;
} TPacketGCPlayerCreateSuccess;

// ����
typedef struct command_attack
{
	BYTE	bHeader;
	BYTE	bType;
	DWORD	dwVID;
	BYTE	bCRCMagicCubeProcPiece;
	BYTE	bCRCMagicCubeFilePiece;
} TPacketCGAttack;

enum EMoveFuncType
{
	FUNC_WAIT,
	FUNC_MOVE,
	FUNC_ATTACK,
	FUNC_COMBO,
	FUNC_MOB_SKILL,
	_FUNC_SKILL,
	FUNC_MAX_NUM,
	FUNC_SKILL = 0x80,
};

// �̵�
typedef struct command_move
{
	BYTE	bHeader;
	BYTE	bFunc;
	BYTE	bArg;
	BYTE	bRot;
	long	lX;
	long	lY;
	DWORD	dwTime;
} TPacketCGMove;

typedef struct command_sync_position_element
{
	DWORD	dwVID;
	long	lX;
	long	lY;
} TPacketCGSyncPositionElement;

// ��ġ ����ȭ
typedef struct command_sync_position	// ���� ��Ŷ
{
	BYTE	bHeader;
	WORD	wSize;
} TPacketCGSyncPosition;

/* ä�� (3) */
typedef struct command_chat	// ���� ��Ŷ
{
	BYTE	header;
	WORD	size;
	BYTE	type;
} TPacketCGChat;

/* �ӼӸ� */
typedef struct command_whisper
{
	BYTE	bHeader;
	WORD	wSize;
	char 	szNameTo[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGWhisper;

typedef struct command_entergame
{
	BYTE	header;
} TPacketCGEnterGame;

typedef struct command_item_use
{
	BYTE 	header;
	TItemPos 	Cell;
} TPacketCGItemUse;

typedef struct command_item_use_to_item
{
	BYTE	header;
	TItemPos	Cell;
	TItemPos	TargetCell;
} TPacketCGItemUseToItem;

typedef struct command_item_drop
{
	BYTE 	header;
	TItemPos 	Cell;
	DWORD	gold;
} TPacketCGItemDrop;

typedef struct command_item_drop2
{
	BYTE 	header;
	TItemPos 	Cell;
	DWORD	gold;
	BYTE	count;
} TPacketCGItemDrop2;

typedef struct command_item_move
{
	BYTE 	header;
	TItemPos	Cell;
	TItemPos	CellTo;
	BYTE	count;
} TPacketCGItemMove;

#ifdef __PARTY_MATCH__
typedef struct grup_paketi
{
	BYTE		header;
	BYTE		index;
	BYTE		ayar;
} TPacketCGGrup;
#endif

typedef struct command_item_destroy
{
    BYTE        header;
    TItemPos    Cell;
} TPacketCGItemDestroy;

typedef struct command_item_sell
{
	BYTE		header;
	TItemPos	Cell;
} TPacketCGItemSell;

typedef struct command_item_pickup
{
	BYTE 	header;
	DWORD	vid;
} TPacketCGItemPickup;

typedef struct command_quickslot_add
{
	BYTE	header;
	BYTE	pos;
	TQuickslot	slot;
} TPacketCGQuickslotAdd;

typedef struct command_quickslot_del
{
	BYTE	header;
	BYTE	pos;
} TPacketCGQuickslotDel;

typedef struct command_quickslot_swap
{
	BYTE	header;
	BYTE	pos;
	BYTE	change_pos;
} TPacketCGQuickslotSwap;

enum
{
	SHOP_SUBHEADER_CG_END,
	SHOP_SUBHEADER_CG_BUY,
	SHOP_SUBHEADER_CG_SELL,
	SHOP_SUBHEADER_CG_SELL2
};

typedef struct command_shop
{
	BYTE	header;
	BYTE	subheader;
} TPacketCGShop;

typedef struct command_on_click
{
	BYTE	header;
	DWORD	vid;
} TPacketCGOnClick;

enum
{
	EXCHANGE_SUBHEADER_CG_START,	/* arg1 == vid of target character */
	EXCHANGE_SUBHEADER_CG_ITEM_ADD,	/* arg1 == position of item */
	EXCHANGE_SUBHEADER_CG_ITEM_DEL,	/* arg1 == position of item */
	EXCHANGE_SUBHEADER_CG_ELK_ADD,	/* arg1 == amount of gold */
#ifdef __CHEQUE__
	EXCHANGE_SUBHEADER_CG_CHEQUE_ADD,	/* arg1 == amount of cheque */
#endif
	EXCHANGE_SUBHEADER_CG_ACCEPT,	/* arg1 == not used */
	EXCHANGE_SUBHEADER_CG_CANCEL,	/* arg1 == not used */
};

typedef struct command_exchange
{
	BYTE	header;
	BYTE	sub_header;
	DWORD	arg1;
	BYTE	arg2;
	TItemPos	Pos;
} TPacketCGExchange;

typedef struct command_position
{
	BYTE	header;
	BYTE	position;
} TPacketCGPosition;

typedef struct command_script_answer
{
	BYTE	header;
	BYTE	answer;
} TPacketCGScriptAnswer;

typedef struct command_script_button
{
	BYTE        header;
	unsigned int	idx;
} TPacketCGScriptButton;

typedef struct command_quest_input_string
{
	BYTE header;
	char msg[64+1];
} TPacketCGQuestInputString;

typedef struct command_quest_confirm
{
	BYTE header;
	BYTE answer;
	DWORD requestPID;
} TPacketCGQuestConfirm;

///-------------------------------------- GC PACKETS --------------------------------------

typedef struct packet_quest_confirm
{
	BYTE header;
	char msg[64+1];
	long timeout;
	DWORD requestPID;
} TPacketGCQuestConfirm;

typedef struct packet_handshake
{
	BYTE	bHeader;
	DWORD	dwHandshake;
	DWORD	dwTime;
	long	lDelta;
} TPacketGCHandshake;

enum EPhase
{
	PHASE_CLOSE,
	PHASE_HANDSHAKE,
	PHASE_LOGIN,
	PHASE_SELECT,
	PHASE_LOADING,
	PHASE_GAME,
	PHASE_DEAD,

	PHASE_CLIENT_CONNECTING,
	PHASE_DBCLIENT,
	PHASE_P2P,
	PHASE_AUTH,
};

typedef struct packet_phase
{
	BYTE	header;
	BYTE	phase;
} TPacketGCPhase;

enum
{
	LOGIN_FAILURE_ALREADY	= 1,
	LOGIN_FAILURE_ID_NOT_EXIST	= 2,
	LOGIN_FAILURE_WRONG_PASS	= 3,
	LOGIN_FAILURE_FALSE		= 4,
	LOGIN_FAILURE_NOT_TESTOR	= 5,
	LOGIN_FAILURE_NOT_TEST_TIME	= 6,
	LOGIN_FAILURE_FULL		= 7
};

typedef struct packet_login_success
{
	BYTE		bHeader;
	TSimplePlayer	players[PLAYER_PER_ACCOUNT];
	DWORD		guild_id[PLAYER_PER_ACCOUNT];
	char		guild_name[PLAYER_PER_ACCOUNT][GUILD_NAME_MAX_LEN+1];

	DWORD		handle;
	DWORD		random_key;
} TPacketGCLoginSuccess;

typedef struct packet_auth_success
{
	BYTE	bHeader;
	DWORD	dwLoginKey;
	BYTE	bResult;
} TPacketGCAuthSuccess;

typedef struct packet_login_failure
{
	BYTE	header;
	char	szStatus[ACCOUNT_STATUS_MAX_LEN + 1];
} TPacketGCLoginFailure;

typedef struct packet_create_failure
{
	BYTE	header;
	BYTE	bType;
} TPacketGCCreateFailure;

enum
{
	ADD_CHARACTER_STATE_DEAD		= (1 << 0),
	ADD_CHARACTER_STATE_SPAWN		= (1 << 1),
	ADD_CHARACTER_STATE_GUNGON		= (1 << 2),
	ADD_CHARACTER_STATE_KILLER		= (1 << 3),
	ADD_CHARACTER_STATE_PARTY		= (1 << 4),
};

enum ECharacterEquipmentPart
{
	CHR_EQUIPPART_ARMOR,
	CHR_EQUIPPART_WEAPON,
	CHR_EQUIPPART_HEAD,
	CHR_EQUIPPART_HAIR,
#ifdef __COSTUME_ACCE__
	CHR_EQUIPPART_ACCE,
#endif
#ifdef __QUIVER__
	CHR_EQUIPPART_ARROW_TYPE,
#endif
#ifdef __COSTUME_EFFECT__
	CHR_EQUIPPART_BODY_EFFECT,
	CHR_EQUIPPART_WEAPON_RIGHT_EFFECT,
	CHR_EQUIPPART_WEAPON_LEFT_EFFECT,
#endif
	CHR_EQUIPPART_NUM,
};

typedef struct packet_add_char
{
	BYTE	header;
	DWORD	dwVID;

	float	angle;
	long	x;
	long	y;
	long	z;

	BYTE	bType;
	WORD	wRaceNum;
	BYTE	bMovingSpeed;
	BYTE	bAttackSpeed;

	BYTE	bStateFlag;
	DWORD	dwAffectFlag[2];
	DWORD	dwLevel;
} TPacketGCCharacterAdd;

typedef struct packet_char_additional_info
{
	BYTE    header;
	DWORD   dwVID;
	char    name[CHARACTER_NAME_MAX_LEN + 1];
	WORD    awPart[CHR_EQUIPPART_NUM];
	BYTE	bEmpire;
	DWORD   dwGuildID;
	DWORD   dwLevel;
	short	sAlignment;
	BYTE	bPKMode;
	DWORD	dwMountVnum;
#ifdef __GUILD_GENERAL_AND_LEADER__
	BYTE	dwNewIsGuildName;
#endif
#ifdef __BATTLE_FIELD__
	BYTE	bBattleFieldRank;
#endif
#ifdef __ELEMENT_ADD__
	BYTE	bElement;
#endif
#ifdef __MULTI_LANGUAGE__
	char	szLanguage[2 + 1];
#endif
} TPacketGCCharacterAdditionalInfo;

typedef struct packet_update_char
{
	BYTE	header;
	DWORD	dwVID;

	WORD	awPart[CHR_EQUIPPART_NUM];
	BYTE	bMovingSpeed;
	BYTE	bAttackSpeed;

	BYTE	bStateFlag;
	DWORD	dwAffectFlag[2];

	DWORD	dwGuildID;
	short	sAlignment;
	BYTE	bPKMode;
	DWORD	dwMountVnum;
#ifdef __EXTEND_INVEN__
	BYTE	bStage;
#endif
	DWORD   dwLevel;
#ifdef __GUILD_GENERAL_AND_LEADER__
	BYTE	dwNewIsGuildName;
#endif
#ifdef __BATTLE_FIELD__
	BYTE	bBattleFieldRank;
#endif
#ifdef __ELEMENT_ADD__
	BYTE	bElement;
#endif
} TPacketGCCharacterUpdate;

typedef struct packet_del_char
{
	BYTE	header;
	DWORD	id;
} TPacketGCCharacterDelete;

typedef struct packet_chat	// ���� ��Ŷ
{
	BYTE	header;
	WORD	size;
	BYTE	type;
	DWORD	id;
	BYTE	bEmpire;
} TPacketGCChat;

typedef struct packet_whisper	// ���� ��Ŷ
{
	BYTE	bHeader;
	WORD	wSize;
	BYTE	bType;
	char	szNameFrom[CHARACTER_NAME_MAX_LEN + 1];
    char	szEmpire[1 + 1];
#ifdef __MULTI_LANGUAGE__
    char	szLanguage[2 + 1];
#endif
} TPacketGCWhisper;

typedef struct packet_main_character
{
	BYTE        header;
	DWORD	dwVID;
	WORD	wRaceNum;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	long	lx, ly, lz;
	BYTE	empire;
	BYTE	skill_group;
} TPacketGCMainCharacter;

typedef struct packet_main_character3_bgm
{
	enum
	{
		MUSIC_NAME_LEN = 24,
	};

	BYTE    header;
	DWORD	dwVID;
	WORD	wRaceNum;
	char	szChrName[CHARACTER_NAME_MAX_LEN + 1];
	char	szBGMName[MUSIC_NAME_LEN + 1];
	long	lx, ly, lz;
	BYTE	empire;
	BYTE	skill_group;
} TPacketGCMainCharacter3_BGM;

typedef struct packet_main_character4_bgm_vol
{
	enum
	{
		MUSIC_NAME_LEN = 24,
	};

	BYTE    header;
	DWORD	dwVID;
	WORD	wRaceNum;
	char	szChrName[CHARACTER_NAME_MAX_LEN + 1];
	char	szBGMName[MUSIC_NAME_LEN + 1];
	float	fBGMVol;
	long	lx, ly, lz;
	BYTE	empire;
	BYTE	skill_group;
} TPacketGCMainCharacter4_BGM_VOL;

typedef struct packet_points
{
	BYTE	header;
	INT		points[POINT_MAX_NUM];
} TPacketGCPoints;

typedef struct packet_skill_level
{
	BYTE		bHeader;
	TPlayerSkill	skills[SKILL_MAX_NUM];
} TPacketGCSkillLevel;

typedef struct packet_point_change
{
	int		header;
	DWORD	dwVID;
	BYTE	type;
	long	amount;
	long	value;
} TPacketGCPointChange;

typedef struct packet_stun
{
	BYTE	header;
	DWORD	vid;
} TPacketGCStun;

typedef struct packet_dead
{
	BYTE	header;
	DWORD	vid;
} TPacketGCDead;

typedef struct packet_item_set
{
	BYTE	header;
	TItemPos Cell;
	DWORD	vnum;
	BYTE	count;
	DWORD	flags;
	DWORD	anti_flags;
	bool	highlight;
	long	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef __SOULBIND__
	long	sealbind;
#endif
#ifdef __CHANGELOOK__
	DWORD	transmutation;
#endif
	bool	bIsBasic;
} TPacketGCItemSet;

typedef struct packet_item_del
{
	BYTE	header;
	BYTE	pos;
} TPacketGCItemDel;

struct packet_item_use
{
	BYTE	header;
	TItemPos Cell;
	DWORD	ch_vid;
	DWORD	victim_vid;
	DWORD	vnum;
};

typedef struct packet_item_update
{
	BYTE	header;
	TItemPos Cell;
	BYTE	count;
	long	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef __SOULBIND__
	long	sealbind;
#endif
#ifdef __CHANGELOOK__
	DWORD	transmutation;
#endif
	bool	bIsBasic;
} TPacketGCItemUpdate;

typedef struct packet_item_ground_add
{
	BYTE	bHeader;
	long 	x, y, z;
	DWORD	dwVID;
	DWORD	dwVnum;
	long	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttrs[ITEM_ATTRIBUTE_MAX_NUM];
} TPacketGCItemGroundAdd;

typedef struct packet_item_ownership
{
	BYTE	bHeader;
	DWORD	dwVID;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCItemOwnership;

typedef struct packet_item_ground_del
{
	BYTE	bHeader;
	DWORD	dwVID;
} TPacketGCItemGroundDel;

struct packet_quickslot_add
{
	BYTE	header;
	BYTE	pos;
	TQuickslot	slot;
};

struct packet_quickslot_del
{
	BYTE	header;
	BYTE	pos;
};

struct packet_quickslot_swap
{
	BYTE	header;
	BYTE	pos;
	BYTE	pos_to;
};

struct packet_motion
{
	BYTE	header;
	DWORD	vid;
	DWORD	victim_vid;
	WORD	motion;
};

enum EPacketShopSubHeaders
{
	SHOP_SUBHEADER_GC_START,
	SHOP_SUBHEADER_GC_END,
	SHOP_SUBHEADER_GC_UPDATE_ITEM,
	SHOP_SUBHEADER_GC_UPDATE_PRICE,
	SHOP_SUBHEADER_GC_OK,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY,
	SHOP_SUBHEADER_GC_SOLDOUT,
	SHOP_SUBHEADER_GC_INVENTORY_FULL,
	SHOP_SUBHEADER_GC_INVALID_POS,
	SHOP_SUBHEADER_GC_SOLD_OUT,
#ifdef __BATTLE_FIELD__
	SHOP_SUBHEADER_GC_NOT_ENOUGH_BATTLE_POINTS,
	SHOP_SUBHEADER_GC_EXCEED_LIMIT_TODAY,
#endif
#ifdef __12ZI__
	SHOP_SUBHEADER_GC_ZODIAC_SHOP,
#endif
};

typedef struct packet_shop_start
{
	DWORD owner_vid;
	TShopItemData items[SHOP_MAX_NUM];
#ifdef __SHOP_SELL_INFO__
	TShopSellInfo infos[SHOP_MAX_NUM];
#endif
#ifdef __BATTLE_FIELD__
	int	usablePoints;
	int	limitMaxPoints;
#endif
} TPacketGCShopStart;

typedef struct packet_shop_update_item
{
	BYTE			pos;
	TShopItemData item;
#ifdef __SHOP_SELL_INFO__
	TShopSellInfo info;
#endif
} TPacketGCShopUpdateItem;

typedef struct packet_shop_update_price
{
	int				iPrice;
} TPacketGCShopUpdatePrice;

typedef struct packet_shop	// ���� ��Ŷ
{
	BYTE        header;
	WORD	size;
	BYTE        subheader;
} TPacketGCShop;

struct packet_exchange
{
	BYTE	header;
	BYTE	sub_header;
	BYTE	is_me;
	DWORD	arg1;	// vnum
	TItemPos	arg2;	// cell
	DWORD	arg3;	// count
	long	alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef __CHANGELOOK__
	DWORD	dwTransmutation;
#endif
};

enum EPacketTradeSubHeaders
{
	EXCHANGE_SUBHEADER_GC_START,	/* arg1 == vid */
	EXCHANGE_SUBHEADER_GC_ITEM_ADD,	/* arg1 == vnum  arg2 == pos  arg3 == count */
	EXCHANGE_SUBHEADER_GC_ITEM_DEL,
	EXCHANGE_SUBHEADER_GC_GOLD_ADD,	/* arg1 == gold */
#ifdef __CHEQUE__
	EXCHANGE_SUBHEADER_GC_CHEQUE_ADD,	/* arg1 == cheque */
#endif
	EXCHANGE_SUBHEADER_GC_ACCEPT,	/* arg1 == accept */
	EXCHANGE_SUBHEADER_GC_END,		/* arg1 == not used */
	EXCHANGE_SUBHEADER_GC_ALREADY,	/* arg1 == not used */
	EXCHANGE_SUBHEADER_GC_LESS_GOLD,	/* arg1 == not used */
#ifdef __CHEQUE__
	EXCHANGE_SUBHEADER_GC_LESS_CHEQUE,	/* arg1 == not used */
#endif
};

struct packet_position
{
	BYTE	header;
	DWORD	vid;
	BYTE	position;
};

typedef struct packet_ping
{
	BYTE	header;
} TPacketGCPing;

struct packet_script
{
	BYTE	header;
	BYTE	skin;
	WORD	size;
	WORD	src_size;
};

typedef struct packet_change_speed
{
	BYTE		header;
	DWORD		vid;
	WORD		moving_speed;
} TPacketGCChangeSpeed;

struct packet_mount
{
	BYTE	header;
	DWORD	vid;
	DWORD	mount_vid;
	BYTE	pos;
	DWORD	x, y;
};

typedef struct packet_move
{
	BYTE		bHeader;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	DWORD		dwVID;
	long		lX;
	long		lY;
	DWORD		dwTime;
	DWORD		dwDuration;
} TPacketGCMove;

// ������
typedef struct packet_ownership
{
	BYTE		bHeader;
	DWORD		dwOwnerVID;
	DWORD		dwVictimVID;
} TPacketGCOwnership;

// ��ġ ����ȭ ��Ŷ�� bCount ��ŭ �ٴ� ����
typedef struct packet_sync_position_element
{
	DWORD	dwVID;
	long	lX;
	long	lY;
} TPacketGCSyncPositionElement;

// ��ġ ����ȭ
typedef struct packet_sync_position	// ���� ��Ŷ
{
	BYTE	bHeader;
	WORD	wSize;	// ���� = (wSize - sizeof(TPacketGCSyncPosition)) / sizeof(TPacketGCSyncPositionElement)
} TPacketGCSyncPosition;

typedef struct packet_fly
{
	BYTE	bHeader;
	BYTE	bType;
	DWORD	dwStartVID;
	DWORD	dwEndVID;
} TPacketGCCreateFly;

typedef struct command_fly_targeting
{
	BYTE		bHeader;
	DWORD		dwTargetVID;
	long		x, y;
} TPacketCGFlyTargeting;

typedef struct packet_fly_targeting
{
	BYTE		bHeader;
	DWORD		dwShooterVID;
	DWORD		dwTargetVID;
	long		x, y;
} TPacketGCFlyTargeting;

typedef struct packet_shoot
{
	BYTE		bHeader;
	BYTE		bType;
} TPacketCGShoot;

typedef struct packet_duel_start
{
	BYTE	header;
	WORD	wSize;	// DWORD�� �? ���� = (wSize - sizeof(TPacketGCPVPList)) / 4
} TPacketGCDuelStart;

enum EPVPModes
{
	PVP_MODE_NONE,
	PVP_MODE_AGREE,
	PVP_MODE_FIGHT,
	PVP_MODE_REVENGE
};

typedef struct packet_pvp
{
	BYTE        bHeader;
	DWORD       dwVIDSrc;
	DWORD       dwVIDDst;
	BYTE        bMode;	// 0 �̸� ��, 1�̸� ��
} TPacketGCPVP;

typedef struct command_use_skill
{
	BYTE	bHeader;
	DWORD	dwVnum;
	DWORD	dwVID;
} TPacketCGUseSkill;

typedef struct command_target
{
	BYTE	header;
	DWORD	dwVID;
} TPacketCGTarget;

typedef struct packet_target
{
	BYTE	header;
	DWORD	dwVID;
	BYTE	bHPPercent;
} TPacketGCTarget;

typedef struct packet_warp
{
	BYTE	bHeader;
	long	lX;
	long	lY;
	long	lAddr;
	WORD	wPort;
} TPacketGCWarp;

typedef struct command_warp
{
	BYTE	bHeader;
} TPacketCGWarp;

struct packet_quest_info
{
	BYTE header;
	WORD size;
	WORD index;
	BYTE flag;
};

enum
{
#ifdef __MESSENGER_BLOCK__
	MESSENGER_SUBHEADER_GC_BLOCK_LIST,
	MESSENGER_SUBHEADER_GC_BLOCK_LOGIN,
	MESSENGER_SUBHEADER_GC_BLOCK_LOGOUT,
	MESSENGER_SUBHEADER_GC_BLOCK_INVITE,//not used
#endif
	MESSENGER_SUBHEADER_GC_LIST,
	MESSENGER_SUBHEADER_GC_LOGIN,
	MESSENGER_SUBHEADER_GC_LOGOUT,
	MESSENGER_SUBHEADER_GC_INVITE,
};

typedef struct packet_messenger
{
	BYTE header;
	WORD size;
	BYTE subheader;
} TPacketGCMessenger;

typedef struct packet_messenger_guild_list
{
	BYTE connected;
	BYTE length;
	//char login[LOGIN_MAX_LEN+1];
} TPacketGCMessengerGuildList;

typedef struct packet_messenger_guild_login
{
	BYTE length;
	//char login[LOGIN_MAX_LEN+1];
} TPacketGCMessengerGuildLogin;

typedef struct packet_messenger_guild_logout
{
	BYTE length;

	//char login[LOGIN_MAX_LEN+1];
} TPacketGCMessengerGuildLogout;

typedef struct packet_messenger_list_offline
{
	BYTE connected; // always 0
	BYTE length;
} TPacketGCMessengerListOffline;

typedef struct packet_messenger_list_online
{
	BYTE connected; // always 1
	BYTE length;
} TPacketGCMessengerListOnline;

#ifdef __MESSENGER_BLOCK__
typedef struct packet_messenger_block_list_offline
{
	BYTE connected; // always 0
	BYTE length;
} TPacketGCMessengerBlockListOffline;

typedef struct packet_messenger_block_list_online
{
	BYTE connected; // always 1
	BYTE length;
} TPacketGCMessengerBlockListOnline;
#endif

enum
{
#ifdef __MESSENGER_BLOCK__
	MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_VID,
    MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_NAME,
    MESSENGER_SUBHEADER_CG_REMOVE_BLOCK,
#endif
	MESSENGER_SUBHEADER_CG_ADD_BY_VID,
	MESSENGER_SUBHEADER_CG_ADD_BY_NAME,
	MESSENGER_SUBHEADER_CG_REMOVE,
	MESSENGER_SUBHEADER_CG_INVITE_ANSWER,
};

typedef struct command_messenger
{
	BYTE header;
	BYTE subheader;
} TPacketCGMessenger;

#ifdef __MESSENGER_BLOCK__
typedef struct command_messenger_add_block_by_vid
{
	DWORD vid;
} TPacketCGMessengerAddBlockByVID;

typedef struct command_messenger_add_block_by_name
{
	BYTE length;
} TPacketCGMessengerAddBlockByName;

typedef struct command_messenger_remove_block
{
	char login[LOGIN_MAX_LEN+1];
} TPacketCGMessengerRemoveBlock;
#endif

typedef struct command_messenger_add_by_vid
{
	DWORD vid;
} TPacketCGMessengerAddByVID;

typedef struct command_messenger_add_by_name
{
	BYTE length;
	//char login[LOGIN_MAX_LEN+1];
} TPacketCGMessengerAddByName;

typedef struct command_messenger_remove
{
	char login[LOGIN_MAX_LEN+1];
	//DWORD account;
} TPacketCGMessengerRemove;

typedef struct command_safebox_checkout
{
	BYTE	bHeader;
	DWORD	dwSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckout;

typedef struct command_safebox_checkin
{
	BYTE	bHeader;
	DWORD	dwSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckin;

///////////////////////////////////////////////////////////////////////////////////
// Party

typedef struct command_party_parameter
{
	BYTE	bHeader;
	BYTE	bDistributeMode;
} TPacketCGPartyParameter;

typedef struct paryt_parameter
{
	BYTE	bHeader;
	BYTE	bDistributeMode;
} TPacketGCPartyParameter;

typedef struct packet_party_add
{
	BYTE	header;
	DWORD	pid;
	char	name[CHARACTER_NAME_MAX_LEN+1];
} TPacketGCPartyAdd;

typedef struct command_party_invite
{
	BYTE	header;
	DWORD	vid;
} TPacketCGPartyInvite;

typedef struct packet_party_invite
{
	BYTE	header;
	DWORD	leader_vid;
} TPacketGCPartyInvite;

typedef struct command_party_invite_answer
{
	BYTE	header;
	DWORD	leader_vid;
	BYTE	accept;
} TPacketCGPartyInviteAnswer;

typedef struct packet_party_update
{
	BYTE	header;
	DWORD	pid;
	BYTE	role;
	BYTE	percent_hp;
	short	affects[7];
} TPacketGCPartyUpdate;

typedef struct packet_party_remove
{
	BYTE header;
	DWORD pid;
} TPacketGCPartyRemove;

typedef struct packet_party_link
{
	BYTE header;
	DWORD pid;
	DWORD vid;
} TPacketGCPartyLink;

typedef struct packet_party_unlink
{
	BYTE header;
	DWORD pid;
	DWORD vid;
} TPacketGCPartyUnlink;

typedef struct command_party_remove
{
	BYTE header;
	DWORD pid;
} TPacketCGPartyRemove;

typedef struct command_party_set_state
{
	BYTE header;
	DWORD pid;
	BYTE byRole;
	BYTE flag;
} TPacketCGPartySetState;

enum
{
	PARTY_SKILL_HEAL = 1,
	PARTY_SKILL_WARP = 2
};

typedef struct command_party_use_skill
{
	BYTE header;
	BYTE bySkillIndex;
	DWORD vid;
} TPacketCGPartyUseSkill;

typedef struct packet_safebox_size
{
	BYTE bHeader;
	BYTE bSize;
} TPacketCGSafeboxSize;

typedef struct packet_safebox_wrong_password
{
	BYTE	bHeader;
} TPacketCGSafeboxWrongPassword;

typedef struct command_empire
{
	BYTE	bHeader;
	BYTE	bEmpire;
} TPacketCGEmpire;

typedef struct packet_empire
{
	BYTE	bHeader;
	BYTE	bEmpire;
} TPacketGCEmpire;

enum
{
	SAFEBOX_MONEY_STATE_SAVE,
	SAFEBOX_MONEY_STATE_WITHDRAW,
};

typedef struct command_safebox_money
{
	BYTE        bHeader;
	BYTE        bState;
	long	lMoney;
} TPacketCGSafeboxMoney;

// Guild

enum
{
	GUILD_SUBHEADER_GC_LOGIN,
	GUILD_SUBHEADER_GC_LOGOUT,
	GUILD_SUBHEADER_GC_LIST,
	GUILD_SUBHEADER_GC_GRADE,
	GUILD_SUBHEADER_GC_ADD,
	GUILD_SUBHEADER_GC_REMOVE,
	GUILD_SUBHEADER_GC_GRADE_NAME,
	GUILD_SUBHEADER_GC_GRADE_AUTH,
	GUILD_SUBHEADER_GC_INFO,
	GUILD_SUBHEADER_GC_COMMENTS,
	GUILD_SUBHEADER_GC_CHANGE_EXP,
	GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_GC_SKILL_INFO,
	GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_GC_GUILD_INVITE,
	GUILD_SUBHEADER_GC_WAR,
	GUILD_SUBHEADER_GC_GUILD_NAME,
	GUILD_SUBHEADER_GC_GUILD_WAR_LIST,
	GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST,
	GUILD_SUBHEADER_GC_WAR_SCORE,
	GUILD_SUBHEADER_GC_MONEY_CHANGE,
};

enum GUILD_SUBHEADER_CG
{
	GUILD_SUBHEADER_CG_ADD_MEMBER,
	GUILD_SUBHEADER_CG_REMOVE_MEMBER,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY,
	GUILD_SUBHEADER_CG_OFFER,
	GUILD_SUBHEADER_CG_POST_COMMENT,
	GUILD_SUBHEADER_CG_DELETE_COMMENT,
	GUILD_SUBHEADER_CG_REFRESH_COMMENT,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_CG_USE_SKILL,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER,
	GUILD_SUBHEADER_CG_CHARGE_GSP,
	GUILD_SUBHEADER_CG_DEPOSIT_MONEY,
	GUILD_SUBHEADER_CG_WITHDRAW_MONEY,
};

typedef struct packet_guild
{
	BYTE header;
	WORD size;
	BYTE subheader;
} TPacketGCGuild;

typedef struct packet_guild_name_t
{
	BYTE header;
	WORD size;
	BYTE subheader;
	DWORD	guildID;
	char	guildName[GUILD_NAME_MAX_LEN];
} TPacketGCGuildName;

typedef struct packet_guild_war
{
	DWORD	dwGuildSelf;
	DWORD	dwGuildOpp;
	BYTE	bType;
	BYTE 	bWarState;
} TPacketGCGuildWar;

typedef struct command_guild
{
	BYTE header;
	BYTE subheader;
} TPacketCGGuild;

typedef struct command_guild_answer_make_guild
{
	BYTE header;
	char guild_name[GUILD_NAME_MAX_LEN+1];
} TPacketCGAnswerMakeGuild;

typedef struct command_guild_use_skill
{
	DWORD	dwVnum;
	DWORD	dwPID;
} TPacketCGGuildUseSkill;

// Guild Mark
typedef struct command_mark_login
{
	BYTE    header;
	DWORD   handle;
	DWORD   random_key;
} TPacketCGMarkLogin;

typedef struct command_mark_upload
{
	BYTE	header;
	DWORD	gid;
	BYTE	image[16*12*4];
} TPacketCGMarkUpload;

typedef struct command_mark_idxlist
{
	BYTE	header;
} TPacketCGMarkIDXList;

typedef struct command_mark_crclist
{
	BYTE	header;
	BYTE	imgIdx;
	DWORD	crclist[80];
} TPacketCGMarkCRCList;

typedef struct packet_mark_idxlist
{
	BYTE    header;
	DWORD	bufSize;
	WORD	count;
	//�ڿ� size * (WORD + WORD)��ŭ ������ ����
} TPacketGCMarkIDXList;

typedef struct packet_mark_block
{
	BYTE	header;
	DWORD	bufSize;
	BYTE	imgIdx;
	DWORD	count;
	// �ڿ� 64 x 48 x �ȼ�ũ��(4����Ʈ) = 12288��ŭ ������ ����
} TPacketGCMarkBlock;

typedef struct command_symbol_upload
{
	BYTE	header;
	WORD	size;
	DWORD	guild_id;
} TPacketCGGuildSymbolUpload;

typedef struct command_symbol_crc
{
	BYTE header;
	DWORD guild_id;
	DWORD crc;
	DWORD size;
} TPacketCGSymbolCRC;

typedef struct packet_symbol_data
{
	BYTE header;
	WORD size;
	DWORD guild_id;
} TPacketGCGuildSymbolData;

// Fishing

typedef struct command_fishing
{
	BYTE header;
	BYTE dir;
} TPacketCGFishing;

typedef struct packet_fishing
{
	BYTE header;
	BYTE subheader;
	DWORD info;
	BYTE dir;
} TPacketGCFishing;

enum
{
	FISHING_SUBHEADER_GC_START,
	FISHING_SUBHEADER_GC_STOP,
	FISHING_SUBHEADER_GC_REACT,
	FISHING_SUBHEADER_GC_SUCCESS,
	FISHING_SUBHEADER_GC_FAIL,
	FISHING_SUBHEADER_GC_FISH,
};

typedef struct command_give_item
{
	BYTE byHeader;
	DWORD dwTargetVID;
	TItemPos ItemPos;
	BYTE byItemCount;
} TPacketCGGiveItem;

typedef struct SPacketCGHack
{
	BYTE	bHeader;
	char	szBuf[255 + 1];
#ifdef __ANTICHEAT__
	char	szInfo[255 + 1];
#endif
} TPacketCGHack;

// SubHeader - Dungeon
enum
{
	DUNGEON_SUBHEADER_GC_TIME_ATTACK_START = 0,
	DUNGEON_SUBHEADER_GC_DESTINATION_POSITION = 1,
};

typedef struct packet_dungeon
{
	BYTE bHeader;
	WORD size;
	BYTE subheader;
} TPacketGCDungeon;

typedef struct packet_dungeon_dest_position
{
	long x;
	long y;
} TPacketGCDungeonDestPosition;

typedef struct SPacketGCShopSign
{
	BYTE	bHeader;
	DWORD	dwVID;
	char	szSign[SHOP_SIGN_MAX_LEN + 1];
#ifdef __MYSHOP_DECO__
	TMyshopDeco	Deco;
#endif
} TPacketGCShopSign;

typedef struct SPacketCGMyShop
{
	BYTE	bHeader;
	char	szSign[SHOP_SIGN_MAX_LEN + 1];
	BYTE	bCount;
#ifdef __MYSHOP_DECO__
	TMyshopDeco Deco;
#endif
#ifdef __OFFLINE_PRIVATE_SHOP__
	int		iTime;
#endif
} TPacketCGMyShop;

typedef struct SPacketGCTime
{
	BYTE	bHeader;
	time_t	time;
} TPacketGCTime;

enum
{
	WALKMODE_RUN,
	WALKMODE_WALK,
};

typedef struct SPacketGCWalkMode
{
	BYTE	header;
	DWORD	vid;
	BYTE	mode;
} TPacketGCWalkMode;

typedef struct SPacketGCChangeSkillGroup
{
	BYTE        header;
	BYTE        skill_group;
} TPacketGCChangeSkillGroup;

typedef struct SPacketCGRefine
{
	BYTE	header;
	BYTE	pos;
	BYTE	type;
	BYTE	win;
} TPacketCGRefine;

typedef struct SPacketCGRequestRefineInfo
{
	BYTE	header;
	BYTE	pos;
} TPacketCGRequestRefineInfo;

typedef struct SPacketGCRefineInformaion
{
	BYTE	header;
	BYTE	type;
	BYTE	pos;
	DWORD	src_vnum;
	DWORD	result_vnum;
	BYTE	material_count;
	int		cost; // �ҿ� ���
	int		prob; // Ȯ��
	TRefineMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TPacketGCRefineInformation;

struct TNPCPosition
{
	BYTE bType;
	char name[CHARACTER_NAME_MAX_LEN+1];
	long x;
	long y;
};

typedef struct SPacketGCNPCPosition
{
	BYTE header;
	WORD size;
	WORD count;

	// array of TNPCPosition
} TPacketGCNPCPosition;

typedef struct SPacketGCSpecialEffect
{
	BYTE header;
	BYTE type;
	DWORD vid;
} TPacketGCSpecialEffect;

typedef struct SPacketCGChangeName
{
	BYTE header;
	BYTE index;
	char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketCGChangeName;

typedef struct SPacketGCChangeName
{
	BYTE header;
	DWORD pid;
	char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketGCChangeName;


typedef struct command_client_version
{
	BYTE header;
	char filename[32+1];
	char timestamp[32+1];
} TPacketCGClientVersion;

typedef struct command_client_version2
{
	BYTE header;
	char filename[32+1];
	char timestamp[32+1];
} TPacketCGClientVersion2;

typedef struct packet_channel
{
	BYTE header;
	BYTE channel;
} TPacketGCChannel;

typedef struct SEquipmentItemSet
{
	DWORD   vnum;
	BYTE    count;
	long    alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
} TEquipmentItemSet;

typedef struct pakcet_view_equip
{
	BYTE  header;
	DWORD vid;
	TEquipmentItemSet equips[WEAR_MAX_NUM];
} TPacketViewEquip;

typedef struct
{
	DWORD	dwID;
	long	x, y;
	long	width, height;
	DWORD	dwGuildID;
} TLandPacketElement;

typedef struct packet_land_list
{
	BYTE	header;
	WORD	size;
} TPacketGCLandList;

typedef struct
{
	BYTE	bHeader;
	long	lID;
	char	szName[32+1];
	DWORD	dwVID;
	BYTE	bType;
} TPacketGCTargetCreate;

typedef struct
{
	BYTE	bHeader;
	long	lID;
	long	lX, lY;
} TPacketGCTargetUpdate;

typedef struct
{
	BYTE	bHeader;
	long	lID;
} TPacketGCTargetDelete;

typedef struct
{
	BYTE		bHeader;
	TPacketAffectElement elem;
} TPacketGCAffectAdd;

typedef struct
{
	BYTE	bHeader;
	DWORD	dwType;
	BYTE	bApplyOn;
} TPacketGCAffectRemove;

typedef struct packet_lover_info
{
	BYTE header;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	BYTE love_point;
} TPacketGCLoverInfo;

typedef struct packet_love_point_update
{
	BYTE header;
	BYTE love_point;
} TPacketGCLovePointUpdate;

// MINING
typedef struct packet_dig_motion
{
	BYTE header;
	DWORD vid;
	DWORD target_vid;
	BYTE count;
} TPacketGCDigMotion;
// END_OF_MINING

// SCRIPT_SELECT_ITEM
typedef struct command_script_select_item
{
	BYTE header;
	DWORD selection;
} TPacketCGScriptSelectItem;
// END_OF_SCRIPT_SELECT_ITEM

typedef struct packet_damage_info
{
	BYTE header;
	DWORD dwVID;
	BYTE flag;
	int damage;
} TPacketGCDamageInfo;

typedef struct SPacketGGCheckAwakeness
{
	BYTE bHeader;
} TPacketGGCheckAwakeness;

#define MAX_EFFECT_FILE_NAME 128
typedef struct SPacketGCSpecificEffect
{
	BYTE header;
	DWORD vid;
	char effect_file[MAX_EFFECT_FILE_NAME];
} TPacketGCSpecificEffect;

// ��ȥ��
enum EDragonSoulRefineWindowRefineType
{
	DragonSoulRefineWindow_UPGRADE,
	DragonSoulRefineWindow_IMPROVEMENT,
	DragonSoulRefineWindow_REFINE,
};

enum EPacketCGDragonSoulSubHeaderType
{
	DS_SUB_HEADER_OPEN,
	DS_SUB_HEADER_CLOSE,
	DS_SUB_HEADER_DO_REFINE_GRADE,
	DS_SUB_HEADER_DO_REFINE_STEP,
	DS_SUB_HEADER_DO_REFINE_STRENGTH,
	DS_SUB_HEADER_REFINE_FAIL,
	DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE,
	DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL,
	DS_SUB_HEADER_REFINE_SUCCEED,
};
typedef struct SPacketCGDragonSoulRefine
{
	SPacketCGDragonSoulRefine() : header (HEADER_CG_DRAGON_SOUL_REFINE)
	{}
	BYTE header;
	BYTE bSubType;
	TItemPos ItemGrid[DRAGON_SOUL_REFINE_GRID_SIZE];
} TPacketCGDragonSoulRefine;

typedef struct SPacketGCDragonSoulRefine
{
	SPacketGCDragonSoulRefine() : header(HEADER_GC_DRAGON_SOUL_REFINE)
	{}
	BYTE header;
	BYTE bSubType;
	TItemPos Pos;
} TPacketGCDragonSoulRefine;

typedef struct SPacketCGStateCheck
{
	BYTE header;
	unsigned long key;
	unsigned long index;
} TPacketCGStateCheck;

typedef struct SPacketGCStateCheck
{
	BYTE header;
	unsigned long key;
	unsigned long index;
	unsigned char state;
} TPacketGCStateCheck;

#ifdef __COSTUME_ACCE__
enum
{
	ACCE_SUBHEADER_GC_OPEN = 0,
	ACCE_SUBHEADER_GC_CLOSE,
	ACCE_SUBHEADER_GC_ADDED,
	ACCE_SUBHEADER_GC_REMOVED,
	ACCE_SUBHEADER_CG_REFINED,
	ACCE_SUBHEADER_CG_CLOSE = 0,
	ACCE_SUBHEADER_CG_ADD,
	ACCE_SUBHEADER_CG_REMOVE,
	ACCE_SUBHEADER_CG_REFINE,
};

typedef struct SPacketAcce
{
	BYTE	header;
	BYTE	subheader;
	bool	bWindow;
	DWORD	dwPrice;
	BYTE	bPos;
	TItemPos	tPos;
	DWORD	dwItemVnum;
	DWORD	dwMinAbs;
	DWORD	dwMaxAbs;
} TPacketAcce;
#endif

#ifdef __CHANGELOOK__
enum
{
	CL_SUBHEADER_OPEN = 0,
	CL_SUBHEADER_CLOSE,
	CL_SUBHEADER_ADD,
	CL_SUBHEADER_REMOVE,
	CL_SUBHEADER_REFINE,
};

typedef struct SPacketChangeLook
{
	BYTE	header;
	BYTE	subheader;
	DWORD	dwCost;
	BYTE	bPos;
	TItemPos	tPos;
} TPacketChangeLook;
#endif

#ifdef __SHOW_CHEST_DROP__
typedef struct SPacketCGChestDropInfo
{
	BYTE	header;
	TItemPos	tPos;
} TPacketCGChestDropInfo;

typedef struct SChestDropInfoTable
{
	BYTE	bPageIndex;
	BYTE	bSlotIndex;
	DWORD	dwItemVnum;
	BYTE	bItemCount;
} TChestDropInfoTable;

typedef struct SPacketGCChestDropInfo
{
	BYTE	bHeader;
	WORD	wSize;
	TItemPos	tPos;
} TPacketGCChestDropInfo;
#endif

#ifdef __SEND_TARGET_INFO__
typedef struct packet_target_info
{
	BYTE	header;
	DWORD	dwVID;
	DWORD	race;
	DWORD	dwVnum;
	BYTE	count;
} TPacketGCTargetInfo;

typedef struct packet_target_info_load
{
	BYTE header;
	DWORD dwVID;
} TPacketCGTargetInfoLoad;
#endif

#ifdef __SUPPORT__
typedef struct support_use_skill
{
	BYTE	bHeader;
	DWORD	dwVnum;
	DWORD	dwVid;
	DWORD	dwLevel;
}TPacketGCSupportUseSkill;
#endif

#ifdef __DAMAGE_TOP__
typedef struct SPacketGCDamageTop
{
	BYTE	bHeader;
	WORD	wSize;
} TPacketGCDamageTop;

typedef struct SPacketGGRegisterDamage
{
	BYTE		bHeader;
	TDamage		damageT;
} TPacketGGRegisterDamage;
#endif

#pragma pack()
#endif
