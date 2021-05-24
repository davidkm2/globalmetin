#pragma once

#include "Locale.h"
#include "../GameLib/RaceData.h"
#include "../GameLib/ItemData.h"

#include "Locale_inc.h"
typedef BYTE TPacketHeader;

enum HeaderGC
{
	HEADER_GC_CHARACTER_ADD = 1,					// 1
	HEADER_GC_CHARACTER_DEL,						// 2
	HEADER_GC_CHARACTER_MOVE,						// 3
	HEADER_GC_CHAT,									// 4
	HEADER_GC_SYNC_POSITION,						// 5

	HEADER_GC_LOGIN_SUCCESS,						// 6
	HEADER_GC_LOGIN_FAILURE,						// 7

	HEADER_GC_PLAYER_CREATE_SUCCESS,				// 8
	HEADER_GC_PLAYER_CREATE_FAILURE,				// 9
	HEADER_GC_PLAYER_DELETE_SUCCESS,				// 10
	HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID,		// 11

	HEADER_GC_STUN,									// 12
	HEADER_GC_DEAD,									// 13

	HEADER_GC_MAIN_CHARACTER,						// 14
	HEADER_GC_PLAYER_POINTS,						// 15
	HEADER_GC_PLAYER_POINT_CHANGE,					// 16
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
	HEADER_GC_CHANGE_SKILL_GROUP,					// 73

	HEADER_GC_MAIN_CHARACTER2_EMPIRE,				// 74

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

	HEADER_GC_GUILD_SYMBOL_DATA,					// 90

	HEADER_GC_DIG_MOTION,							// 91

	HEADER_GC_DAMAGE_INFO,							// 92
	HEADER_GC_CHAR_ADDITIONAL_INFO,					// 93

	HEADER_GC_MAIN_CHARACTER3_BGM,					// 94
	HEADER_GC_MAIN_CHARACTER4_BGM_VOL,				// 95
	
	HEADER_GC_AUTH_SUCCESS,							// 96

	HEADER_GC_SPECIFIC_EFFECT,						// 97

	HEADER_GC_DRAGON_SOUL_REFINE,					// 98
	HEADER_GC_RESPOND_CHANNELSTATUS,				// 99

#ifdef ENABLE_GEM_SYSTEM
	HEADER_GC_GEM_SHOP,								// 100
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	HEADER_GC_EXTEND_INVEN,							// 101
#endif

#ifdef ENABLE_MINI_GAME_RUMI
	HEADER_GC_MINI_GAME_RUMI,						// 102
#endif

#ifdef ENABLE_SHOW_CHEST_DROP
	HEADER_GC_CHEST_DROP_INFO,						// 103
#endif

#ifdef ENABLE_ACCE_SYSTEM
	HEADER_GC_ACCE,									// 104
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	HEADER_GC_CL,									// 105
#endif

#ifdef ENABLE_SEND_TARGET_INFO
	HEADER_GC_TARGET_INFO,							// 106
#endif

#ifdef ENABLE_SUPPORT_SYSTEM
	HEADER_GC_SUPPORT_SKILL,						// 107
#endif

#ifdef ENABLE_DAMAGE_TOP
	HEADER_GC_DAMAGE_TOP,							// 109
#endif

#ifdef ENABLE_RANKING_SYSTEM
	HEADER_GC_RANKING,								// 110
#endif

#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
	HEADER_GC_ACCUMULATE,							// 111
#endif
	HEADER_GC_ATTENDANCE_REQUEST_DATA,				// 112
	HEADER_GC_ATTENDANCE_REQUEST_REWARD_LIST,		// 113
#endif

#ifdef ENABLE_FISH_EVENT
	HEADER_GC_FISH_EVENT,							// 114
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
	HEADER_GC_MINI_GAME_CATCH_KING,					// 115
#endif

#ifdef ENABLE_MAILBOX
	HEADER_GC_MAILBOX,								// 116
#endif

	HEADER_GC_ANTICHEAT_BLACKLIST,					// 117

#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
	HEADER_GC_OFFLINE_PRIVATE_SHOP,					// 118
#endif

	HEADER_GC_HANDSHAKE_OK						= 0xfc, // 252
	HEADER_GC_PHASE								= 0xfd,	// 253
    HEADER_GC_HANDSHAKE                         = 0xff, // 255
};

enum HeaderCG
{
	HEADER_CG_LOGIN = 1,							// 1
	HEADER_CG_ATTACK,								// 2
	HEADER_CG_CHAT,									// 3
	HEADER_CG_PLAYER_CREATE,						// 4
	HEADER_CG_PLAYER_DESTROY,						// 5
	HEADER_CG_PLAYER_SELECT,						// 6
	HEADER_CG_CHARACTER_MOVE,						// 7
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

	HEADER_CG_GIVE_ITEM,							// 50

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
	HEADER_CG_GUILD_SYMBOL_CRC,						// 63

	HEADER_CG_SCRIPT_SELECT_ITEM,					// 64

	HEADER_CG_DRAGON_SOUL_REFINE,					// 65
	HEADER_CG_STATE_CHECKER,						// 66

#ifdef ENABLE_GEM_SYSTEM
	HEADER_CG_GEM_SHOP,								// 67
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	HEADER_CG_EXTEND_INVEN,							// 68
#endif

#ifdef ENABLE_MINI_GAME_RUMI
	HEADER_CG_MINI_GAME_RUMI,						// 69
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	HEADER_CG_PET_SET_NAME,							// 70
#endif

	HEADER_CG_ITEM_DESTROY,							// 71
	HEADER_CG_ITEM_SELL,							// 72

#ifdef ENABLE_SHOW_CHEST_DROP
	HEADER_CG_CHEST_DROP_INFO,						// 73
#endif

#ifdef ENABLE_ACCE_SYSTEM
	HEADER_CG_ACCE,									// 74
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	HEADER_CG_CL,									// 75
#endif

#ifdef ENABLE_SEND_TARGET_INFO
	HEADER_CG_TARGET_INFO_LOAD,						// 76
#endif

#ifdef ENABLE_PARTY_MATCH
	HEADER_CG_PARTY_MATCH,							// 77
#endif

#ifdef ENABLE_SKILL_BOOK_COMBINATION
	HEADER_CG_SKILL_BOOK_COMBINATION,				// 79
#endif

#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
	HEADER_CG_ATTENDANCE_BUTTON_CLICK,				// 80
	HEADER_CG_ATTENDANCE_REQUEST_REWARD_LIST,		// 81
	HEADER_CG_ATTENDANCE_REQUEST_DATA,				// 82
#endif

#ifdef ENABLE_FISH_EVENT
	HEADER_CG_FISH_EVENT,							// 83
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
	HEADER_CG_MINI_GAME_CATCH_KING,					// 84
#endif

#ifdef ENABLE_MAILBOX
	HEADER_CG_MAILBOX,								// 85
#endif

#ifdef ENABLE_ATTR_6TH_7TH
	HEADER_CG_ATTR67_ADD,							// 86
#endif

	HEADER_CG_TIME_SYNC							= 0xfc,
	HEADER_CG_CLIENT_VERSION					= 0xfd,
	HEADER_CG_CLIENT_VERSION2					= 0xf1,
	HEADER_CG_PONG								= 0xfe,
	HEADER_CG_HANDSHAKE                         = 0xff,
};

enum
{
	ID_MAX_NUM = 30,
	PASS_MAX_NUM = 16,
	CHAT_MAX_NUM = 128,
	PATH_NODE_MAX_NUM = 64,
	SHOP_SIGN_MAX_LEN = 32,

	PLAYER_ITEM_SLOT_MAX_NUM = 20,

	QUICKSLOT_MAX_LINE = 4,
	QUICKSLOT_MAX_COUNT_PER_LINE = 8,
	QUICKSLOT_MAX_COUNT = QUICKSLOT_MAX_LINE * QUICKSLOT_MAX_COUNT_PER_LINE,

	QUICKSLOT_MAX_NUM = 36,

	SHOP_PAGE_COLUMN = 5,
	SHOP_PAGE_ROW = 9,
	SHOP_PAGE_SIZE = SHOP_PAGE_COLUMN * SHOP_PAGE_ROW,
	SHOP_PAGE_COUNT = 3,
	SHOP_MAX_NUM = SHOP_PAGE_SIZE * SHOP_PAGE_COUNT,

	EXCHANGE_PAGE_COLUMN = 4,
	EXCHANGE_PAGE_ROW = 3,
	EXCHANGE_PAGE_SIZE = EXCHANGE_PAGE_COLUMN * EXCHANGE_PAGE_ROW,
	EXCHANGE_PAGE_COUNT = 4,
	EXCHANGE_MAX_NUM = EXCHANGE_PAGE_SIZE * EXCHANGE_PAGE_COUNT,

	SAFEBOX_PAGE_COLUMN = 5,
	SAFEBOX_PAGE_ROW = 9,
	SAFEBOX_PAGE_SIZE = SAFEBOX_PAGE_COLUMN * SAFEBOX_PAGE_ROW,
	SAFEBOX_PAGE_COUNT = 5,
	SAFEBOX_MAX_NUM = SAFEBOX_PAGE_SIZE * SAFEBOX_PAGE_COUNT,

	METIN_SOCKET_COUNT = 6,

	PARTY_AFFECT_SLOT_MAX_NUM = 7,

	GUILD_GRADE_NAME_MAX_LEN = 8,
	GUILD_NAME_MAX_LEN = 12,
	GUILD_GRADE_COUNT = 15,
	GULID_COMMENT_MAX_LEN = 50,

	MARK_CRC_NUM = 8*8,
	MARK_DATA_SIZE = 16*12,
	SYMBOL_DATA_SIZE = 128*256,
	QUEST_INPUT_STRING_MAX_NUM = 64,

	PRIVATE_CODE_LENGTH = 8,

	REFINE_MATERIAL_MAX_NUM = 5,

	WEAR_MAX_NUM = CItemData::WEAR_MAX_NUM,

#ifdef ENABLE_PLAYER_PER_ACCOUNT5
	PLAYER_PER_ACCOUNT = 5,
#else
	PLAYER_PER_ACCOUNT = 4,
#endif

#ifdef ENABLE_GEM_SYSTEM
	GEM_SLOTS_MAX_NUM = 9,
#endif

#ifdef ENABLE_MAILBOX
	MAIL_SLOT_MAX_NUM = 18,
	POST_TIME_OUT_DELETE = 2,
	MAIL_MESSAGE_MAX_LEN = 100,
	MAIL_TITLE_MAX_LEN = 25,
#endif

	PARTY_MAX_MEMBER = 8,

#ifdef ENABLE_EXPRESSING_EMOTION
	SPECIAL_EMOTION_MAX = 12,
#endif
};

#pragma pack(push)
#pragma pack(1)

///-------------------------------------- CG PACKETS --------------------------------------
#ifdef ENABLE_MINI_GAME_RUMI
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

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
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

#ifdef ENABLE_GEM_SYSTEM
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

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
enum EChangeLookInfo
{
#ifdef ENABLE_CHANGE_LOOK_ITEM_SYSTEM
	CL_WINDOW_MAX_MATERIALS = 3,
#else
	CL_WINDOW_MAX_MATERIALS = 2,
#endif
	CL_CLEAN_ATTR_VALUE0 = 8,
};

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

typedef struct SChangeLookMaterial
{
	BYTE	bHere;
	WORD	wCell;
} TChangeLookMaterial;
#endif

#ifdef ENABLE_ACCE_SYSTEM
enum EAcceInfo
{
	ACCE_ABSORPTION_SOCKET = 0,
	ACCE_ABSORBED_SOCKET = 1,
	ACCE_CLEAN_ATTR_VALUE0 = 7,
	ACCE_WINDOW_MAX_MATERIALS = 2,
};

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

typedef struct SAcceMaterial
{
	BYTE	bHere;
	WORD	wCell;
} TAcceMaterial;

typedef struct SAcceResult
{
	DWORD	dwItemVnum;
	DWORD	dwMinAbs;
	DWORD	dwMaxAbs;
} TAcceResult;
#endif

#ifdef ENABLE_SHOW_CHEST_DROP
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

#ifdef ENABLE_SEND_TARGET_INFO
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

#ifdef ENABLE_PARTY_MATCH
typedef struct grup_paketi
{
	BYTE		header;
	BYTE		index;
	BYTE		ayar;
} TPacketCGGrup;
#endif

#ifdef ENABLE_SUPPORT_SYSTEM
typedef struct support_use_skill
{
	BYTE	bHeader;
	DWORD	dwVnum;
	DWORD	dwVid;
	DWORD	dwLevel;
}TPacketGCSupportUseSkill;
#endif

#ifdef ENABLE_DAMAGE_TOP
typedef struct SPacketGCDamageTop
{
	BYTE	bHeader;
	WORD	wSize;
} TPacketGCDamageTop;

typedef struct SDamage
{
	char szAttackerName[CHARACTER_NAME_MAX_LEN + 1];
	BYTE bAttackerLevel;
	BYTE bAttackerEmpire;
	char szVictimName[CHARACTER_NAME_MAX_LEN + 1];
	DWORD dwDamageValue;
	BYTE bDamageType;
} TDamage;
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
typedef struct packet_RequestPetName
{
	BYTE byHeader;
	char petname[13];
} TPacketCGRequestPetName;
#endif

#ifdef ENABLE_SKILL_BOOK_COMBINATION
typedef struct SPacketCGSkillBookCombination
{
	BYTE header;
	int List[10];
	int	mod;
} TPacketCGSkillBookCombination;
#endif

#ifdef ENABLE_RANKING_SYSTEM
enum ERKType
{
	TYPE_RK_SOLO,
#ifdef ENABLE_RANKING_SYSTEM_PARTY
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

#ifdef ENABLE_RANKING_SYSTEM_PARTY
enum ERKPartyCategory
{
	PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_ALL,
	PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_NOW_WEEK,
	PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_PAST_WEEK,
#ifdef ENABLE_12ZI
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
#ifdef ENABLE_DEFENSE_WAVE
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

#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
typedef struct SAccumulate
{
	SAccumulate()
	{
		dwVid = dwCount = 0;
	}

	SAccumulate(DWORD vid, DWORD count)
	{
		dwVid = vid;
		dwCount = count;
	}

	DWORD	dwVid;
	DWORD	dwCount;
} TAccumulate;

typedef struct SPacketGCAccumulate
{
	BYTE	bHeader;
	DWORD	dwVid;
	DWORD	dwCount;
} TPacketGCAccumulate;
#endif

typedef struct SRewardItem
{
	BYTE bDay;
	DWORD dwVnum;
	DWORD dwCount;
} TRewardItem;

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

#ifdef ENABLE_FISH_EVENT
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

#ifdef ENABLE_MINI_GAME_CATCH_KING
enum ECatchKingSubHeaders
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

typedef struct SPacketGCCatchKingEventInfo
{
	BYTE	bHeader;
	bool	bIsEnable;
} TPacketGCCatchKingEventInfo;

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

#ifdef ENABLE_MAILBOX
typedef struct SMailData
{
	int index;
	char from_name[CHARACTER_NAME_MAX_LEN + 1];
	char message[MAIL_MESSAGE_MAX_LEN + 1];
	char title[MAIL_TITLE_MAX_LEN + 1];
	int yang;
	BYTE cheque;
	int send_time;
	int delete_time;
	bool is_gm_post;
	bool is_item_exist;
	bool is_confirm;
	TItemData item;
} TMailData;

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

typedef struct packet_anticheat_blacklist
{
	BYTE header;
	char content[1024];
} TPacketGCAntiCheatBlacklist;

#ifdef ENABLE_ATTR_6TH_7TH
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

#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
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

typedef struct SOfflinePrivateShopInfo
{
	DWORD dwVid;
	char szSign[CHARACTER_NAME_MAX_LEN + 1];
	DWORD dwStartTime;
	DWORD dwEndTime;
	DWORD dwPrices[SHOP_PRICE_MAX_NUM];
	DWORD dwX;
	DWORD dwY;
	DWORD dwMapIndex;
	TShopItemData items[SHOP_MAX_NUM];
#ifdef ENABLE_SHOP_SELL_INFO
	TShopSellInfo infos[SHOP_MAX_NUM];
#endif
} TOfflinePrivateShopInfo;
#endif

typedef struct command_mark_login
{
	BYTE header;
	DWORD handle;
	DWORD random_key;
} TPacketCGMarkLogin;

typedef struct command_mark_upload
{
	BYTE header;
	DWORD gid;
	BYTE image[16 * 12 * 4];
} TPacketCGMarkUpload;

typedef struct command_mark_idxlist
{
	BYTE header;
} TPacketCGMarkIDXList;

typedef struct command_mark_crclist
{
	BYTE header;
	BYTE imgIdx;
	DWORD crclist[80];
} TPacketCGMarkCRCList;

typedef struct command_symbol_upload
{
	BYTE header;
	WORD size;
	DWORD handle;
} TPacketCGSymbolUpload;

typedef struct command_symbol_crc
{
	BYTE header;
	DWORD dwGuildID;
	DWORD dwCRC;
	DWORD dwSize;
} TPacketCGSymbolCRC;

typedef struct command_checkin
{
	BYTE header;
	char name[ID_MAX_NUM+1];
	char pwd[PASS_MAX_NUM+1];
} TPacketCGCheckin;

typedef struct command_login
{
	BYTE header;
	char name[ID_MAX_NUM + 1];
	char pwd[PASS_MAX_NUM + 1];
} TPacketCGLogin;

typedef struct command_login2
{
	BYTE header;
	char name[ID_MAX_NUM + 1];
	DWORD login_key;
	DWORD adwClientKey[4];
} TPacketCGLogin2;

typedef struct command_login3
{
	BYTE header;
	char name[ID_MAX_NUM + 1];
	char pwd[PASS_MAX_NUM + 1];
	DWORD adwClientKey[4];
} TPacketCGLogin3;

typedef struct command_player_select
{
	BYTE header;
	BYTE player_index;
#ifdef ENABLE_MULTI_LANGUAGE
	char szLanguage[2 + 1];
#endif
} TPacketCGSelectCharacter;

typedef struct command_attack
{
	BYTE	header;
	BYTE	bType;
	DWORD	dwVictimVID;
	BYTE	bCRCMagicCubeProcPiece;
	BYTE	bCRCMagicCubeFilePiece;
} TPacketCGAttack;

typedef struct command_chat
{
	BYTE	header;
	WORD	length;
	BYTE	type;
} TPacketCGChat;

typedef struct command_whisper
{
	BYTE        bHeader;
	WORD        wSize;
	char        szNameTo[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGWhisper;

enum EBattleMode
{
	BATTLEMODE_ATTACK = 0,
	BATTLEMODE_DEFENSE = 1,
};

typedef struct command_EnterFrontGame
{
	BYTE header;
} TPacketCGEnterFrontGame;

typedef struct command_item_use
{
	BYTE header;
	TItemPos pos;
} TPacketCGItemUse;

typedef struct command_item_use_to_item
{
	BYTE header;
	TItemPos source_pos;
	TItemPos target_pos;
} TPacketCGItemUseToItem;

typedef struct command_item_drop
{
	BYTE  header;
	TItemPos pos;
	DWORD elk;
} TPacketCGItemDrop;

typedef struct command_item_drop2
{
    BYTE        header;
    TItemPos pos;
    DWORD       gold;
    BYTE        count;
} TPacketCGItemDrop2;

typedef struct command_item_move
{
	BYTE header;
	TItemPos pos;
	TItemPos change_pos;
	BYTE num;
} TPacketCGItemMove;

typedef struct command_item_destroy
{
    BYTE        header;
    TItemPos    pos;
} TPacketCGItemDestroy;

typedef struct command_item_sell
{
	BYTE		header;
	TItemPos	pos;
} TPacketCGItemSell;

typedef struct command_item_pickup
{
	BYTE header;
	DWORD vid;
} TPacketCGItemPickUp;

typedef struct command_quickslot_add
{
	BYTE        header;
	BYTE        pos;
	TQuickSlot	slot;
}TPacketCGQuickSlotAdd;

typedef struct command_quickslot_del
{
	BYTE        header;
	BYTE        pos;
} TPacketCGQuickSlotDel;

typedef struct command_quickslot_swap
{
	BYTE        header;
	BYTE        pos;
	BYTE        change_pos;
} TPacketCGQuickSlotSwap;

typedef struct command_on_click
{
	BYTE header;
	DWORD vid;
} TPacketCGOnClick;

enum EShopSubheaderCG
{
    SHOP_SUBHEADER_CG_END,
	SHOP_SUBHEADER_CG_BUY,
	SHOP_SUBHEADER_CG_SELL,
	SHOP_SUBHEADER_CG_SELL2,
};

typedef struct command_shop
{
	BYTE header;
	BYTE subheader;
} TPacketCGShop;

enum
{
	EXCHANGE_SUBHEADER_CG_START,			// arg1 == vid of target character
	EXCHANGE_SUBHEADER_CG_ITEM_ADD,		// arg1 == position of item
	EXCHANGE_SUBHEADER_CG_ITEM_DEL,		// arg1 == position of item
	EXCHANGE_SUBHEADER_CG_ELK_ADD,			// arg1 == amount of elk
#ifdef ENABLE_CHEQUE_SYSTEM
	EXCHANGE_SUBHEADER_CG_CHEQUE_ADD,	/* arg1 == amount of cheque */
#endif
	EXCHANGE_SUBHEADER_CG_ACCEPT,			// arg1 == not used
	EXCHANGE_SUBHEADER_CG_CANCEL,			// arg1 == not used
};

typedef struct command_exchange
{
	BYTE		header;
	BYTE		subheader;
	DWORD		arg1;
	BYTE		arg2;
	TItemPos	Pos;
} TPacketCGExchange;

typedef struct command_position
{
	BYTE        header;
	BYTE        position;
} TPacketCGPosition;

typedef struct command_script_answer
{
	BYTE        header;
	BYTE		answer;
} TPacketCGScriptAnswer;

typedef struct command_script_button
{
	BYTE header;
	unsigned int idx;
} TPacketCGScriptButton;

typedef struct command_target
{
	BYTE        header;
	DWORD       dwVID;
} TPacketCGTarget;

typedef struct command_move
{
	BYTE		bHeader;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
} TPacketCGMove;

typedef struct command_sync_position_element
{
	DWORD       dwVID;
	long        lX;
	long        lY;
} TPacketCGSyncPositionElement;

typedef struct command_sync_position
{
	BYTE        bHeader;
	WORD		wSize;
} TPacketCGSyncPosition;

typedef struct command_fly_targeting
{
	BYTE		bHeader;
	DWORD		dwTargetVID;
	long		lX;
	long		lY;
} TPacketCGFlyTargeting;

typedef struct packet_fly_targeting
{
	BYTE        bHeader;
	DWORD		dwShooterVID;
	DWORD		dwTargetVID;
	long		lX;
	long		lY;
} TPacketGCFlyTargeting;

typedef struct packet_shoot
{
	BYTE		bHeader;
	BYTE		bType;
} TPacketCGShoot;

typedef struct command_warp
{
	BYTE			bHeader;
} TPacketCGWarp;

enum
{
#ifdef ENABLE_MESSENGER_BLOCK
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

#ifdef ENABLE_MESSENGER_BLOCK
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

typedef struct packet_messenger_list_offline
{
	BYTE connected;
	BYTE length;
} TPacketGCMessengerListOffline;

enum EMessengerState
{
	MESSENGER_CONNECTED_STATE_OFFLINE,
	MESSENGER_CONNECTED_STATE_ONLINE,
};

typedef struct packet_messenger_list_online
{
	BYTE connected;
	BYTE length;
} TPacketGCMessengerListOnline;

typedef struct packet_messenger_login
{
	BYTE length;
} TPacketGCMessengerLogin;

typedef struct packet_messenger_logout
{
	BYTE length;
} TPacketGCMessengerLogout;

enum
{
#ifdef ENABLE_MESSENGER_BLOCK
	MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_VID,
    MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_NAME,
    MESSENGER_SUBHEADER_CG_REMOVE_BLOCK,
#endif
    MESSENGER_SUBHEADER_CG_ADD_BY_VID,
    MESSENGER_SUBHEADER_CG_ADD_BY_NAME,
    MESSENGER_SUBHEADER_CG_REMOVE,
};

typedef struct command_messenger
{
	BYTE header;
	BYTE subheader;
} TPacketCGMessenger;

typedef struct command_messenger_remove
{
	BYTE length;
} TPacketCGMessengerRemove;

enum
{
	SAFEBOX_MONEY_STATE_SAVE,
	SAFEBOX_MONEY_STATE_WITHDRAW,
};

typedef struct command_safebox_money
{
    BYTE        bHeader;
    BYTE        bState;
    DWORD       dwMoney;
} TPacketCGSafeboxMoney;

typedef struct command_safebox_checkout
{
	BYTE        bHeader;
	DWORD		dwSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckout;

typedef struct command_safebox_checkin
{
	BYTE        bHeader;
	DWORD		dwSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckin;

typedef struct command_mall_checkout
{
	BYTE bHeader;
	BYTE bMallPos;
	TItemPos ItemPos;
} TPacketCGMallCheckout;

typedef struct command_use_skill
{
	BYTE bHeader;
	DWORD dwVnum;
	DWORD dwTargetVID;
} TPacketCGUseSkill;

typedef struct command_party_invite
{
	BYTE header;
	DWORD vid;
} TPacketCGPartyInvite;

typedef struct command_party_invite_answer
{
	BYTE header;
	DWORD leader_pid;
	BYTE accept;
} TPacketCGPartyInviteAnswer;

typedef struct command_party_remove
{
	BYTE header;
	DWORD pid;
} TPacketCGPartyRemove;

typedef struct command_party_set_state
{
	BYTE byHeader;
	DWORD dwVID;
	BYTE byState;
	BYTE byFlag;
} TPacketCGPartySetState;

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

typedef struct command_party_use_skill
{
	BYTE byHeader;
	BYTE bySkillIndex;
	DWORD dwTargetVID;
} TPacketCGPartyUseSkill;

enum
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

typedef struct command_guild
{
	BYTE byHeader;
	BYTE bySubHeader;
} TPacketCGGuild;

typedef struct command_guild_answer_make_guild
{
	BYTE header;
	char guild_name[GUILD_NAME_MAX_LEN + 1];
} TPacketCGAnswerMakeGuild;

typedef struct command_give_item
{
	BYTE byHeader;
	DWORD dwTargetVID;
	TItemPos ItemPos;
	BYTE byItemCount;
} TPacketCGGiveItem;

typedef struct SPacketCGHack
{
	BYTE        bHeader;
	char        szBuf[255 + 1];
#ifdef ENABLE_ANTICHEAT
	char		szInfo[255 + 1];
#endif
} TPacketCGHack;

typedef struct command_dungeon
{
	BYTE		bHeader;
	WORD		size;
} TPacketCGDungeon;

// Private Shop
typedef struct SShopItemTable
{
	DWORD		vnum;
	BYTE		count;

	TItemPos	pos;
	DWORD		dwPrices[SHOP_PRICE_MAX_NUM];
	BYTE		display_pos;
} TShopItemTable;

typedef struct SPacketCGMyShop
{
	BYTE        bHeader;
	char        szSign[SHOP_SIGN_MAX_LEN + 1];
	BYTE        bCount;
#ifdef ENABLE_MYSHOP_DECO
	TMyshopDeco Deco;
#endif
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
	int			iTime;
#endif
} TPacketCGMyShop;

typedef struct SPacketCGRefine
{
	BYTE		header;
	BYTE		pos;
	BYTE		type;
	BYTE		win;
} TPacketCGRefine;

typedef struct SPacketCGChangeName
{
	BYTE header;
	BYTE index;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGChangeName;

typedef struct command_client_version
{
	BYTE header;
	char filename[32 + 1];
	char timestamp[32 + 1];
} TPacketCGClientVersion;

typedef struct command_client_version2
{
	BYTE header;
	char filename[32+1];
	char timestamp[32+1];
} TPacketCGClientVersion2;

typedef struct command_crc_report
{
	BYTE header;
	BYTE byPackMode;
	DWORD dwBinaryCRC32;
	DWORD dwProcessCRC32;
	DWORD dwRootPackCRC32;
} TPacketCGCRCReport;

enum EPartyExpDistributionType
{
	PARTY_EXP_DISTRIBUTION_NON_PARITY,
	PARTY_EXP_DISTRIBUTION_PARITY,
};

typedef struct command_party_parameter
{
	BYTE        bHeader;
	BYTE        bDistributeMode;
} TPacketCGPartyParameter;

typedef struct command_quest_input_string
{
	BYTE        bHeader;
	char		szString[QUEST_INPUT_STRING_MAX_NUM + 1];
} TPacketCGQuestInputString;

typedef struct command_quest_confirm
{
	BYTE header;
	BYTE answer;
	DWORD requestPID;
} TPacketCGQuestConfirm;

typedef struct command_script_select_item
{
	BYTE header;
	DWORD selection;
} TPacketCGScriptSelectItem;

enum EPhase
{
    PHASE_CLOSE,
    PHASE_HANDSHAKE,
    PHASE_LOGIN,
    PHASE_SELECT,
    PHASE_LOADING,
    PHASE_GAME,
    PHASE_DEAD,

	PHASE_DBCLIENT_CONNECTING,
    PHASE_DBCLIENT,
    PHASE_P2P,
    PHASE_AUTH,
};

typedef struct packet_phase
{
	BYTE        header;
	BYTE        phase;
} TPacketGCPhase;

typedef struct packet_blank
{
	BYTE header;
} TPacketGCBlank;

typedef struct packet_blank_dynamic
{
	BYTE		header;
	WORD		size;
} TPacketGCBlankDynamic;

typedef struct packet_header_handshake
{
	BYTE		header;
	DWORD		dwHandshake;
	DWORD		dwTime;
	LONG		lDelta;
} TPacketGCHandshake;

typedef struct packet_header_dynamic_size
{
	BYTE header;
	WORD size;
} TDynamicSizePacketHeader;

typedef struct SSimplePlayerInformation
{
	DWORD dwID;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	BYTE byJob;
	BYTE byLevel;
	DWORD dwPlayMinutes;
	BYTE byST, byHT, byDX, byIQ;
	WORD wMainPart;
	BYTE bChangeName;
	WORD wHairPart;
#ifdef ENABLE_ACCE_SYSTEM
	WORD wAccePart;
#endif
	BYTE bDummy[4];
	long x, y;
	LONG lAddr;
	WORD wPort;
	BYTE bySkillGroup;
} TSimplePlayerInformation;

typedef struct packet_login_success
{
	BYTE						header;
	TSimplePlayerInformation	akSimplePlayerInformation[PLAYER_PER_ACCOUNT];
    DWORD						guild_id[PLAYER_PER_ACCOUNT];
    char						guild_name[PLAYER_PER_ACCOUNT][GUILD_NAME_MAX_LEN+1];
	DWORD handle;
	DWORD random_key;
} TPacketGCLoginSuccess;

enum { LOGIN_STATUS_MAX_LEN = 8 };
typedef struct packet_login_failure
{
	BYTE	header;
	char	szStatus[LOGIN_STATUS_MAX_LEN + 1];
} TPacketGCLoginFailure;

typedef struct command_player_create
{
	BYTE        header;
	BYTE        index;
	char        name[CHARACTER_NAME_MAX_LEN + 1];
	WORD        job;
	BYTE		shape;
	BYTE		CON;
	BYTE		INT;
	BYTE		STR;
	BYTE		DEX;
} TPacketCGCreateCharacter;

typedef struct command_player_create_success
{
	BYTE header;
	BYTE bAccountCharacterSlot;
	TSimplePlayerInformation kSimplePlayerInfomation;
} TPacketGCPlayerCreateSuccess;

typedef struct command_create_failure
{
	BYTE	header;
	BYTE	bType;
} TPacketGCCreateFailure;

typedef struct command_player_delete
{
	BYTE        header;
	BYTE        index;
	char		szPrivateCode[PRIVATE_CODE_LENGTH];
} TPacketCGDestroyCharacter;

typedef struct packet_player_delete_success
{
	BYTE        header;
	BYTE        account_index;
} TPacketGCDestroyCharacterSuccess;

enum
{
	ADD_CHARACTER_STATE_DEAD   = (1 << 0),
	ADD_CHARACTER_STATE_SPAWN  = (1 << 1),
	ADD_CHARACTER_STATE_GUNGON = (1 << 2),
	ADD_CHARACTER_STATE_KILLER = (1 << 3),
	ADD_CHARACTER_STATE_PARTY  = (1 << 4),
};

enum EPKModes
{
	PK_MODE_PEACE,
	PK_MODE_REVENGE,
	PK_MODE_FREE,
	PK_MODE_PROTECT,
	PK_MODE_GUILD,
#ifdef ENABLE_BATTLE_FIELD
	PK_MODE_BATTLE_FIELD,
#endif
	PK_MODE_MAX_NUM,
};

enum ECharacterEquipmentPart
{
	CHR_EQUIPPART_ARMOR,
	CHR_EQUIPPART_WEAPON,
	CHR_EQUIPPART_HEAD,
	CHR_EQUIPPART_HAIR,
#ifdef ENABLE_ACCE_SYSTEM
	CHR_EQUIPPART_ACCE,
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	CHR_EQUIPPART_ARROW_TYPE,
#endif
#ifdef ENABLE_COSTUME_EFFECT
	CHR_EQUIPPART_BODY_EFFECT,
	CHR_EQUIPPART_WEAPON_RIGHT_EFFECT,
	CHR_EQUIPPART_WEAPON_LEFT_EFFECT,
#endif
	CHR_EQUIPPART_NUM,
};

typedef struct packet_char_additional_info
{
	BYTE header;
	DWORD dwVID;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	WORD awPart[CHR_EQUIPPART_NUM];
	BYTE bEmpire;
	DWORD dwGuildID;
	DWORD dwLevel;
	short sAlignment;
	BYTE bPKMode;
	DWORD dwMountVnum;
#ifdef ENABLE_GUILD_GENERAL_AND_LEADER
	BYTE dwNewIsGuildName;
#endif
#ifdef ENABLE_BATTLE_FIELD
	BYTE bBattleFieldRank;
#endif
#ifdef ENABLE_ELEMENT_ADD
	BYTE bElement;
#endif
#ifdef ENABLE_MULTI_LANGUAGE
	char szLanguage[2 + 1];
#endif
} TPacketGCCharacterAdditionalInfo;

typedef struct packet_add_char
{
	BYTE header;

	DWORD dwVID;
	float angle;
	long x;
	long y;
	long z;

	BYTE bType;
	WORD wRaceNum;
	BYTE bMovingSpeed;
	BYTE bAttackSpeed;

	BYTE bStateFlag;
	DWORD dwAffectFlag[2];
	DWORD dwLevel;
} TPacketGCCharacterAdd;

typedef struct packet_update_char
{
    BYTE        header;
    DWORD       dwVID;

    WORD        awPart[CHR_EQUIPPART_NUM];
    BYTE        bMovingSpeed;
	BYTE		bAttackSpeed;

    BYTE        bStateFlag;
    DWORD       dwAffectFlag[2];

	DWORD		dwGuildID;
    short       sAlignment;
	BYTE		bPKMode;
	DWORD		dwMountVnum;
#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	BYTE		bStage;
#endif
	DWORD		dwLevel;
#ifdef ENABLE_GUILD_GENERAL_AND_LEADER
	BYTE		dwNewIsGuildName;
#endif
#ifdef ENABLE_BATTLE_FIELD
	BYTE		bBattleFieldRank;
#endif
#ifdef ENABLE_ELEMENT_ADD
	BYTE		bElement;
#endif
} TPacketGCCharacterUpdate;

typedef struct packet_del_char
{
	BYTE	header;
    DWORD	dwVID;
} TPacketGCCharacterDelete;

typedef struct packet_GlobalTime
{
	BYTE	header;
	float	GlobalTime;
} TPacketGCGlobalTime;

enum EChatType
{
	CHAT_TYPE_TALKING,
	CHAT_TYPE_INFO,
	CHAT_TYPE_NOTICE,
	CHAT_TYPE_PARTY,
	CHAT_TYPE_GUILD,
	CHAT_TYPE_COMMAND,
	CHAT_TYPE_SHOUT,
	CHAT_TYPE_WHISPER,
	CHAT_TYPE_BIG_NOTICE,
#ifdef ENABLE_DICE_SYSTEM
	CHAT_TYPE_DICE_INFO,
#endif
#ifdef ENABLE_12ZI
	CHAT_TYPE_ZODIAC_NOTICE,
#endif
	CHAT_TYPE_MAX_NUM,
};

typedef struct packet_chatting
{
	BYTE header;
	WORD size;
	BYTE type;
	DWORD dwVID;
	BYTE bEmpire;
} TPacketGCChat;

typedef struct packet_whisper
{
	BYTE        bHeader;
	WORD        wSize;
	BYTE        bType;
	char        szNameFrom[CHARACTER_NAME_MAX_LEN + 1];
    char		szEmpire[1 + 1];
#ifdef ENABLE_MULTI_LANGUAGE
    char		szLanguage[2 + 1];
#endif
} TPacketGCWhisper;

typedef struct packet_stun
{
	BYTE		header;
	DWORD		vid;
} TPacketGCStun;

typedef struct packet_dead
{
	BYTE		header;
	DWORD		vid;
} TPacketGCDead;

typedef struct packet_main_character
{
	BYTE        header;
	DWORD       dwVID;
	WORD		wRaceNum;
	char        szName[CHARACTER_NAME_MAX_LEN + 1];
	long        lX, lY, lZ;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter;

typedef struct packet_main_character2_empire
{
	BYTE        header;
	DWORD       dwVID;
	WORD		wRaceNum;
	char        szName[CHARACTER_NAME_MAX_LEN + 1];
	long        lX, lY, lZ;
	BYTE		byEmpire;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter2_EMPIRE;

typedef struct packet_main_character3_bgm
{
	enum
	{
		MUSIC_NAME_MAX_LEN = 24,
	};
    BYTE        header;
    DWORD       dwVID;
	WORD		wRaceNum;
    char        szUserName[CHARACTER_NAME_MAX_LEN + 1];
	char        szBGMName[MUSIC_NAME_MAX_LEN + 1];
    long        lX, lY, lZ;
	BYTE		byEmpire;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter3_BGM;

typedef struct packet_main_character4_bgm_vol
{
	enum
	{
		MUSIC_NAME_MAX_LEN = 24,
	};
    BYTE        header;
    DWORD       dwVID;
	WORD		wRaceNum;
    char        szUserName[CHARACTER_NAME_MAX_LEN + 1];
	char        szBGMName[MUSIC_NAME_MAX_LEN + 1];
	float		fBGMVol;
    long        lX, lY, lZ;
	BYTE		byEmpire;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter4_BGM_VOL;

enum EPointTypes
{
    POINT_NONE,                 // 0
    POINT_LEVEL,                // 1
    POINT_VOICE,                // 2
    POINT_EXP,                  // 3
    POINT_NEXT_EXP,             // 4
    POINT_HP,                   // 5
    POINT_MAX_HP,               // 6
    POINT_SP,                   // 7
    POINT_MAX_SP,               // 8
    POINT_STAMINA,              // 9  스테미너
    POINT_MAX_STAMINA,          // 10 최대 스테미너

    POINT_GOLD,                 // 11
    POINT_ST,                   // 12 근력
    POINT_HT,                   // 13 체력
    POINT_DX,                   // 14 민첩성
    POINT_IQ,                   // 15 정신력
    POINT_ATT_POWER,            // 16 공격력
    POINT_ATT_SPEED,            // 17 공격속도
    POINT_EVADE_RATE,           // 18 회피율
    POINT_MOV_SPEED,            // 19 이동속도
    POINT_DEF_GRADE,            // 20 방어등급
	POINT_CASTING_SPEED,        // 21 주문속도 (쿨다운타임*100) / (100 + 이값) = 최종 쿨다운 타임
	POINT_MAGIC_ATT_GRADE,      // 22 마법공격력
    POINT_MAGIC_DEF_GRADE,      // 23 마법방어력
    POINT_EMPIRE_POINT,         // 24 제국점수
    POINT_LEVEL_STEP,           // 25 한 레벨에서의 단계.. (1 2 3 될 때 보상, 4 되면 레벨 업)
    POINT_STAT,                 // 26 능력치 올릴 수 있는 개수
	POINT_SUB_SKILL,            // 27 보조 스킬 포인트
	POINT_SKILL,                // 28 액티브 스킬 포인트
	POINT_MIN_ATK,				// 29 최소 파괴력
	POINT_MAX_ATK,				// 30 최대 파괴력
    POINT_PLAYTIME,             // 31 플레이시간
    POINT_HP_REGEN,             // 32 HP 회복률
    POINT_SP_REGEN,             // 33 SP 회복률

    POINT_BOW_DISTANCE,         // 34 활 사정거리 증가치 (meter)

    POINT_HP_RECOVERY,          // 35 체력 회복 증가량
    POINT_SP_RECOVERY,          // 36 정신력 회복 증가량

    POINT_POISON_PCT,           // 37 독 확률
    POINT_STUN_PCT,             // 38 기절 확률
    POINT_SLOW_PCT,             // 39 슬로우 확률
    POINT_CRITICAL_PCT,         // 40 크리티컬 확률
    POINT_PENETRATE_PCT,        // 41 관통타격 확률
    POINT_CURSE_PCT,            // 42 저주 확률

    POINT_ATTBONUS_HUMAN,       // 43 인간에게 강함
    POINT_ATTBONUS_ANIMAL,      // 44 동물에게 데미지 % 증가
    POINT_ATTBONUS_ORC,         // 45 웅귀에게 데미지 % 증가
    POINT_ATTBONUS_MILGYO,      // 46 밀교에게 데미지 % 증가
    POINT_ATTBONUS_UNDEAD,      // 47 시체에게 데미지 % 증가
    POINT_ATTBONUS_DEVIL,       // 48 마귀(악마)에게 데미지 % 증가
    POINT_ATTBONUS_INSECT,      // 49 벌레족
    POINT_ATTBONUS_FIRE,        // 50 화염족
    POINT_ATTBONUS_ICE,         // 51 빙설족
	POINT_ATTBONUS_DESERT,      // 52 사막족
	POINT_ATTBONUS_MONSTER,     // 53 모든 몬스터에게 강함
	POINT_ATTBONUS_WARRIOR,     // 54 무사에게 강함
	POINT_ATTBONUS_ASSASSIN,	// 55 자객에게 강함
	POINT_ATTBONUS_SURA,		// 56 수라에게 강함
	POINT_ATTBONUS_SHAMAN,		// 57 무당에게 강함
	POINT_ATTBONUS_TREE,     	// 58 나무에게 강함 20050729.myevan UNUSED5

	POINT_RESIST_WARRIOR,		// 59 무사에게 저항
	POINT_RESIST_ASSASSIN,		// 60 자객에게 저항
	POINT_RESIST_SURA,			// 61 수라에게 저항
	POINT_RESIST_SHAMAN,		// 62 무당에게 저항

    POINT_STEAL_HP,             // 63 생명력 흡수
    POINT_STEAL_SP,             // 64 정신력 흡수

    POINT_MANA_BURN_PCT,        // 65 마나 번

    POINT_DAMAGE_SP_RECOVER,    // 66 공격당할 시 정신력 회복 확률

    POINT_BLOCK,                // 67 블럭율
    POINT_DODGE,                // 68 회피율

    POINT_RESIST_SWORD,         // 69
    POINT_RESIST_TWOHAND,       // 70
    POINT_RESIST_DAGGER,        // 71
    POINT_RESIST_BELL,          // 72
    POINT_RESIST_FAN,           // 73
    POINT_RESIST_BOW,           // 74  화살   저항   : 대미지 감소
    POINT_RESIST_FIRE,          // 75  화염   저항   : 화염공격에 대한 대미지 감소
    POINT_RESIST_ELEC,          // 76  전기   저항   : 전기공격에 대한 대미지 감소
    POINT_RESIST_MAGIC,         // 77  술법   저항   : 모든술법에 대한 대미지 감소
    POINT_RESIST_WIND,          // 78  바람   저항   : 바람공격에 대한 대미지 감소

    POINT_REFLECT_MELEE,        // 79 공격 반사

    POINT_REFLECT_CURSE,        // 80 저주 반사
    POINT_POISON_REDUCE,        // 81 독데미지 감소

    POINT_KILL_SP_RECOVER,      // 82 적 소멸시 MP 회복
    POINT_EXP_DOUBLE_BONUS,     // 83
    POINT_GOLD_DOUBLE_BONUS,    // 84
    POINT_ITEM_DROP_BONUS,      // 85

    POINT_POTION_BONUS,         // 86
    POINT_KILL_HP_RECOVER,      // 87

    POINT_IMMUNE_STUN,          // 88
    POINT_IMMUNE_SLOW,          // 89
    POINT_IMMUNE_FALL,          // 90

    POINT_PARTY_ATT_GRADE,      // 91
    POINT_PARTY_DEF_GRADE,      // 92

    POINT_ATT_BONUS,            // 93
    POINT_DEF_BONUS,            // 94

    POINT_ATT_GRADE_BONUS,			// 95
    POINT_DEF_GRADE_BONUS,			// 96
    POINT_MAGIC_ATT_GRADE_BONUS,	// 97
    POINT_MAGIC_DEF_GRADE_BONUS,	// 98

    POINT_RESIST_NORMAL_DAMAGE,		// 99

	POINT_HIT_HP_RECOVERY,		// 100
	POINT_HIT_SP_RECOVERY, 		// 101
	POINT_MANASHIELD,			// 102 흑신수호 스킬에 의한 마나쉴드 효과 정도

	POINT_PARTY_BUFFER_BONUS,		// 103
	POINT_PARTY_SKILL_MASTER_BONUS,	// 104

	POINT_HP_RECOVER_CONTINUE,		// 105
	POINT_SP_RECOVER_CONTINUE,		// 106

	POINT_STEAL_GOLD,			// 107
	POINT_POLYMORPH,			// 108 변신한 몬스터 번호
	POINT_MOUNT,			// 109 타고있는 몬스터 번호

	POINT_PARTY_HASTE_BONUS,		// 110
	POINT_PARTY_DEFENDER_BONUS,		// 111
	POINT_STAT_RESET_COUNT,		// 112 피의 단약 사용을 통한 스텟 리셋 포인트 (1당 1포인트 리셋가능)

	POINT_HORSE_SKILL,			// 113

	POINT_MALL_ATTBONUS,		// 114 공격력 +x%
	POINT_MALL_DEFBONUS,		// 115 방어력 +x%
	POINT_MALL_EXPBONUS,		// 116 경험치 +x%
	POINT_MALL_ITEMBONUS,		// 117 아이템 드롭율 x/10배
	POINT_MALL_GOLDBONUS,		// 118 돈 드롭율 x/10배

    POINT_MAX_HP_PCT,			// 119 최대생명력 +x%
    POINT_MAX_SP_PCT,			// 120 최대정신력 +x%

	POINT_SKILL_DAMAGE_BONUS,       // 121 스킬 데미지 *(100+x)%
	POINT_NORMAL_HIT_DAMAGE_BONUS,  // 122 평타 데미지 *(100+x)%

    POINT_SKILL_DEFEND_BONUS,       // 123 스킬 방어 데미지
    POINT_NORMAL_HIT_DEFEND_BONUS,  // 124 평타 방어 데미지
 
	POINT_PC_BANG_EXP_BONUS,        // 125
	POINT_PC_BANG_DROP_BONUS,       // 126 PC방 전용 드롭률 보너스

	POINT_RAMADAN_CANDY_BONUS_EXP,			// 127 라마단 사탕 경험치 증가용

	POINT_ENERGY,					// 128 기력

	POINT_ENERGY_END_TIME,					// 129 기력 종료 시간

	POINT_COSTUME_ATTR_BONUS,			// 130
	POINT_MAGIC_ATT_BONUS_PER,			// 131
	POINT_MELEE_MAGIC_ATT_BONUS_PER,	// 132

	POINT_RESIST_ICE,          // 133   냉기 저항   : 얼음공격에 대한 대미지 감소
	POINT_RESIST_EARTH,        // 134  대지 저항   : 얼음공격에 대한 대미지 감소
	POINT_RESIST_DARK,         // 135  어둠 저항   : 얼음공격에 대한 대미지 감소

	POINT_RESIST_CRITICAL,		// 136 크리티컬 저항	: 상대의 크리티컬 확률을 감소
	POINT_RESIST_PENETRATE,		// 137 관통타격 저항	: 상대의 관통타격 확률을 감소

#ifdef ENABLE_WOLFMAN_CHARACTER
	POINT_BLEEDING_REDUCE,		// 138
	POINT_BLEEDING_PCT,			// 139

	POINT_ATTBONUS_WOLFMAN,				// 140 수인족에게 강함
	POINT_RESIST_WOLFMAN,				// 141 수인족에게 저항
	POINT_RESIST_CLAW,					// 142 CLAW에 저항
#endif

	POINT_ACCEDRAIN_RATE,		// 143
	POINT_RESIST_MAGIC_REDUCTION,			// 144

#ifdef ENABLE_ELEMENT_ADD
	POINT_ENCHANT_ELECT,			// 145
	POINT_ENCHANT_FIRE,				// 146
	POINT_ENCHANT_ICE,				// 147
	POINT_ENCHANT_WIND,				// 148
	POINT_ENCHANT_EARTH,			// 149
	POINT_ENCHANT_DARK,				// 150
	POINT_ATTBONUS_CZ,				// 151
#endif

#ifdef ENABLE_PENDANT
	POINT_ATTBONUS_SWORD,			// 152
	POINT_ATTBONUS_TWOHAND,			// 153
	POINT_ATTBONUS_DAGGER,			// 154
	POINT_ATTBONUS_BELL,			// 155
	POINT_ATTBONUS_FAN,				// 156
	POINT_ATTBONUS_BOW,				// 157
#ifdef ENABLE_WOLFMAN_CHARACTER
	POINT_ATTBONUS_CLAW,			// 158
#endif
	POINT_RESIST_HUMAN,				// 159
	POINT_RESIST_MOUNT_FALL,		// 160
#endif

#ifdef ENABLE_BATTLE_FIELD
	POINT_BATTLE_POINT,				// 161
	POINT_TEMP_BATTLE_POINT,		// 162
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
	POINT_CHEQUE,					// 163
#endif

#ifdef ENABLE_GEM_SYSTEM
	POINT_GEM,						// 164
#endif

	POINT_NONAME,					// 165

	POINT_MIN_WEP = 200,
	POINT_MAX_WEP,
	POINT_MIN_MAGIC_WEP,
	POINT_MAX_MAGIC_WEP,
	POINT_HIT_RATE,
};

typedef struct packet_points
{
    BYTE        header;
    long        points[POINT_MAX_NUM];
} TPacketGCPoints;

typedef struct packet_point_change
{
    int         header;

	DWORD		dwVID;
	BYTE		Type;

	long        amount; // 바뀐 값
    long        value;  // 현재 값
} TPacketGCPointChange;

typedef struct packet_motion
{
	BYTE		header;
	DWORD		vid;
	DWORD		victim_vid;
	WORD		motion;
} TPacketGCMotion;

typedef struct packet_set_item
{
	BYTE		header;
	TItemPos	Cell;
	DWORD		vnum;
	BYTE		count;
	DWORD		flags;
	DWORD		anti_flags;
	bool		highlight;
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_SOULBIND_SYSTEM
	long		sealbind;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	DWORD		transmutation;
#endif
	bool		bIsBasic;
} TPacketGCItemSet;

typedef struct packet_item_del
{
    BYTE        header;
    BYTE        pos;
} TPacketGCItemDel;

typedef struct packet_use_item
{
	BYTE		header;
	TItemPos	Cell;
	DWORD		ch_vid;
	DWORD		victim_vid;

	DWORD		vnum;
} TPacketGCItemUse;

typedef struct packet_update_item
{
	BYTE		header;
	TItemPos	Cell;
	BYTE		count;
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_SOULBIND_SYSTEM
	long		sealbind;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	DWORD		transmutation;
#endif
	bool		bIsBasic;
} TPacketGCItemUpdate;

typedef struct packet_ground_add_item
{
    BYTE        bHeader;
    long        lX;
	long		lY;
	long		lZ;

    DWORD       dwVID;
    DWORD       dwVnum;

	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttrs[ITEM_ATTRIBUTE_SLOT_MAX_NUM];

} TPacketGCItemGroundAdd;

typedef struct packet_ground_del_item
{
	BYTE		header;
	DWORD		vid;
} TPacketGCItemGroundDel;

typedef struct packet_item_ownership
{
    BYTE        bHeader;
    DWORD       dwVID;
    char        szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCItemOwnership;

typedef struct packet_quickslot_add
{
    BYTE        header;
    BYTE        pos;
	TQuickSlot	slot;
} TPacketGCQuickSlotAdd;

typedef struct packet_quickslot_del
{
    BYTE        header;
    BYTE        pos;
} TPacketGCQuickSlotDel;

typedef struct packet_quickslot_swap
{
    BYTE        header;
    BYTE        pos;
    BYTE        change_pos;
} TPacketGCQuickSlotSwap;

typedef struct packet_shop_start
{
	DWORD owner_vid;
	packet_shop_item items[SHOP_MAX_NUM];
#ifdef ENABLE_SHOP_SELL_INFO
	TShopSellInfo infos[SHOP_MAX_NUM];
#endif
#ifdef ENABLE_BATTLE_FIELD
	int	usablePoints;
	int	limitMaxPoints;
#endif
} TPacketGCShopStart;

typedef struct packet_shop_update_item
{
	BYTE						pos;
	struct packet_shop_item		item;
#ifdef ENABLE_SHOP_SELL_INFO
	TShopSellInfo info;
#endif
} TPacketGCShopUpdateItem;

typedef struct packet_shop_update_price
{
	int iElkAmount;
} TPacketGCShopUpdatePrice;

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
#ifdef ENABLE_BATTLE_FIELD
	SHOP_SUBHEADER_GC_NOT_ENOUGH_BATTLE_POINTS,
	SHOP_SUBHEADER_GC_EXCEED_LIMIT_TODAY,
#endif
#ifdef ENABLE_12ZI
	SHOP_SUBHEADER_GC_ZODIAC_SHOP,
#endif
};

typedef struct packet_shop
{
	BYTE        header;
	WORD		size;
	BYTE        subheader;
} TPacketGCShop;

typedef struct packet_exchange
{
    BYTE        header;
    BYTE        subheader;
    BYTE        is_me;
    DWORD       arg1;
    TItemPos       arg2;
    DWORD       arg3;
	long		alValues[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	DWORD	dwTransmutation;
#endif
} TPacketGCExchange;

enum
{
    EXCHANGE_SUBHEADER_GC_START,			// arg1 == vid
    EXCHANGE_SUBHEADER_GC_ITEM_ADD,		// arg1 == vnum  arg2 == pos  arg3 == count
	EXCHANGE_SUBHEADER_GC_ITEM_DEL,		// arg1 == pos
    EXCHANGE_SUBHEADER_GC_ELK_ADD,			// arg1 == elk
#ifdef ENABLE_CHEQUE_SYSTEM
	EXCHANGE_SUBHEADER_GC_CHEQUE_ADD,	/* arg1 == cheque */
#endif
    EXCHANGE_SUBHEADER_GC_ACCEPT,			// arg1 == accept
    EXCHANGE_SUBHEADER_GC_END,				// arg1 == not used
    EXCHANGE_SUBHEADER_GC_ALREADY,			// arg1 == not used
    EXCHANGE_SUBHEADER_GC_LESS_ELK,		// arg1 == not used
#ifdef ENABLE_CHEQUE_SYSTEM
	EXCHANGE_SUBHEADER_GC_LESS_CHEQUE,	/* arg1 == not used */
#endif
};

typedef struct packet_position
{
    BYTE        header;
	DWORD		vid;
    BYTE        position;
} TPacketGCPosition;

typedef struct packet_ping
{
	BYTE		header;
} TPacketGCPing;

typedef struct packet_pong
{
	BYTE		bHeader;
} TPacketCGPong;

typedef struct packet_script
{
    BYTE		header;
	BYTE		skin;
    WORD        size;
    WORD        src_size;
} TPacketGCScript;

typedef struct packet_target
{
    BYTE        header;
    DWORD       dwVID;
    BYTE        bHPPercent;
} TPacketGCTarget;

typedef struct packet_damage_info
{
	BYTE header;
	DWORD dwVID;
	BYTE flag;
	int  damage;
} TPacketGCDamageInfo;

typedef struct packet_mount
{
    BYTE        header;
    DWORD       vid;
    DWORD       mount_vid;
    BYTE        pos;
	DWORD		_x, _y;
} TPacketGCMount;

typedef struct packet_change_speed
{
	BYTE		header;
	DWORD		vid;
	WORD		moving_speed;
} TPacketGCChangeSpeed;

typedef struct packet_move
{
	BYTE		bHeader;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	DWORD		dwVID;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
	DWORD		dwDuration;
} TPacketGCMove;

enum
{
	QUEST_SEND_IS_BEGIN         = 1 << 0,
    QUEST_SEND_TITLE            = 1 << 1,  // 28자 까지
    QUEST_SEND_CLOCK_NAME       = 1 << 2,  // 16자 까지
    QUEST_SEND_CLOCK_VALUE      = 1 << 3,
    QUEST_SEND_COUNTER_NAME     = 1 << 4,  // 16자 까지
    QUEST_SEND_COUNTER_VALUE    = 1 << 5,
	QUEST_SEND_ICON_FILE		= 1 << 6,  // 24자 까지
};

typedef struct packet_quest_info
{
	BYTE header;
	WORD size;
	WORD index;
	BYTE flag;
} TPacketGCQuestInfo;

typedef struct packet_quest_confirm
{
    BYTE header;
    char msg[64+1];
    long timeout;
    DWORD requestPID;
} TPacketGCQuestConfirm;

typedef struct packet_attack
{
    BYTE        header;
    DWORD       dwVID;
    DWORD       dwVictimVID;    // 적 VID
    BYTE        bType;          // 공격 유형
} TPacketGCAttack;

typedef struct packet_c2c
{
	BYTE		header;
	WORD		wSize;
} TPacketGCC2C;

typedef struct packetd_sync_position_element
{
    DWORD       dwVID;
    long        lX;
    long        lY;
} TPacketGCSyncPositionElement;

typedef struct packetd_sync_position
{
    BYTE        bHeader;
    WORD		wSize;
} TPacketGCSyncPosition;

typedef struct packet_ownership
{
    BYTE                bHeader;
    DWORD               dwOwnerVID;
    DWORD               dwVictimVID;
} TPacketGCOwnership;

#define	SKILL_MAX_NUM 255

typedef struct SPlayerSkill
{
	BYTE bMasterType;
	BYTE bLevel;
	time_t tNextRead;
} TPlayerSkill;

typedef struct packet_skill_level
{
	BYTE bHeader;
	TPlayerSkill skills[SKILL_MAX_NUM];
} TPacketGCSkillLevel;

// fly
typedef struct packet_fly
{
    BYTE        bHeader;
    BYTE        bType;
    DWORD       dwStartVID;
    DWORD       dwEndVID;
} TPacketGCCreateFly;

enum EPVPModes
{
	PVP_MODE_NONE,
    PVP_MODE_AGREE,
    PVP_MODE_FIGHT,
    PVP_MODE_REVENGE,
};

typedef struct packet_duel_start
{
    BYTE	header ;
    WORD	wSize ;	// DWORD가 몇개? 개수 = (wSize - sizeof(TPacketGCPVPList)) / 4
} TPacketGCDuelStart ;

typedef struct packet_pvp
{
	BYTE		header;
	DWORD		dwVIDSrc;
	DWORD		dwVIDDst;
	BYTE		bMode;
} TPacketGCPVP;

typedef struct packet_warp
{
	BYTE			bHeader;
	LONG			lX;
	LONG			lY;
	LONG			lAddr;
	WORD			wPort;
} TPacketGCWarp;

typedef struct packet_party_invite
{
    BYTE header;
    DWORD leader_pid;
} TPacketGCPartyInvite;

typedef struct packet_party_add
{
    BYTE header;
    DWORD pid;
    char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketGCPartyAdd;

typedef struct packet_party_update
{
    BYTE header;
    DWORD pid;
    BYTE state;
    BYTE percent_hp;
    short affects[PARTY_AFFECT_SLOT_MAX_NUM];
} TPacketGCPartyUpdate;

typedef struct packet_party_remove
{
    BYTE header;
    DWORD pid;
} TPacketGCPartyRemove;

typedef TPacketCGSafeboxCheckout TPacketGCSafeboxCheckout;
typedef TPacketCGSafeboxCheckin TPacketGCSafeboxCheckin;

typedef struct packet_safebox_wrong_password
{
    BYTE        bHeader;
} TPacketGCSafeboxWrongPassword;

typedef struct packet_safebox_size
{
	BYTE bHeader;
	BYTE bSize;
} TPacketGCSafeboxSize;

typedef struct command_empire
{
    BYTE        bHeader;
    BYTE        bEmpire;
} TPacketCGEmpire;

typedef struct packet_empire
{
    BYTE        bHeader;
    BYTE        bEmpire;
} TPacketGCEmpire;

enum
{
	FISHING_SUBHEADER_GC_START,
	FISHING_SUBHEADER_GC_STOP,
	FISHING_SUBHEADER_GC_REACT,
	FISHING_SUBHEADER_GC_SUCCESS,
	FISHING_SUBHEADER_GC_FAIL,
    FISHING_SUBHEADER_GC_FISH,
};

typedef struct packet_fishing
{
    BYTE header;
    BYTE subheader;
    DWORD info;
    BYTE dir;
} TPacketGCFishing;

typedef struct paryt_parameter
{
    BYTE        bHeader;
    BYTE        bDistributeMode;
} TPacketGCPartyParameter;

//////////////////////////////////////////////////////////////////////////
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
    GUILD_SUBHEADER_GC_WAR_POINT,
	GUILD_SUBHEADER_GC_MONEY_CHANGE,
};

typedef struct packet_guild
{
    BYTE header;
    WORD size;
    BYTE subheader;
} TPacketGCGuild;

// SubHeader - Grade
enum
{
    GUILD_AUTH_ADD_MEMBER       = (1 << 0),
    GUILD_AUTH_REMOVE_MEMBER    = (1 << 1),
    GUILD_AUTH_NOTICE           = (1 << 2),
    GUILD_AUTH_SKILL            = (1 << 3),
};

typedef struct packet_guild_sub_grade
{
	char grade_name[GUILD_GRADE_NAME_MAX_LEN+1]; // 8+1 길드장, 길드원 등의 이름
	BYTE auth_flag;
} TPacketGCGuildSubGrade;

typedef struct packet_guild_sub_member
{
	DWORD pid;
	BYTE byGrade;
	BYTE byIsGeneral;
	BYTE byJob;
	BYTE byLevel;
	DWORD dwOffer;
	BYTE byNameFlag;
// if NameFlag is TRUE, name is sent from server.
//	char szName[CHARACTER_ME_MAX_LEN+1];
} TPacketGCGuildSubMember;

typedef struct packet_guild_sub_info
{
    WORD member_count;
    WORD max_member_count;
	DWORD guild_id;
    DWORD master_pid;
    DWORD exp;
    BYTE level;
    char name[GUILD_NAME_MAX_LEN+1];
	DWORD gold;
	BYTE hasLand;
} TPacketGCGuildInfo;

enum EGuildWarState
{
    GUILD_WAR_NONE,
    GUILD_WAR_SEND_DECLARE,
    GUILD_WAR_REFUSE,
    GUILD_WAR_RECV_DECLARE,
    GUILD_WAR_WAIT_START,
    GUILD_WAR_CANCEL,
    GUILD_WAR_ON_WAR,
    GUILD_WAR_END,

    GUILD_WAR_DURATION = 2*60*60, // 2시간
};

typedef struct packet_guild_war
{
    DWORD       dwGuildSelf;
    DWORD       dwGuildOpp;
    BYTE        bType;
    BYTE        bWarState;
} TPacketGCGuildWar;

typedef struct SPacketGuildWarPoint
{
    DWORD dwGainGuildID;
    DWORD dwOpponentGuildID;
    long lPoint;
} TPacketGuildWarPoint;

// SubHeader - Dungeon
enum
{
	DUNGEON_SUBHEADER_GC_TIME_ATTACK_START = 0,
	DUNGEON_SUBHEADER_GC_DESTINATION_POSITION = 1,
};

typedef struct packet_dungeon
{
	BYTE		bHeader;
    WORD		size;
    BYTE		subheader;
} TPacketGCDungeon;

// Private Shop
typedef struct SPacketGCShopSign
{
    BYTE        bHeader;
    DWORD       dwVID;
    char        szSign[SHOP_SIGN_MAX_LEN + 1];
#ifdef ENABLE_MYSHOP_DECO
	TMyshopDeco Deco;
#endif
} TPacketGCShopSign;

typedef struct SPacketGCTime
{
    BYTE        bHeader;
    time_t      time;
} TPacketGCTime;

enum
{
    WALKMODE_RUN,
    WALKMODE_WALK,
};

typedef struct SPacketGCWalkMode
{
    BYTE        header;
    DWORD       vid;
    BYTE        mode;
} TPacketGCWalkMode;

typedef struct SPacketGCChangeSkillGroup
{
    BYTE        header;
    BYTE        skill_group;
} TPacketGCChangeSkillGroup;

struct TMaterial
{
    DWORD vnum;
    DWORD count;
};

typedef struct SRefineTable
{
    DWORD src_vnum;
    DWORD result_vnum;
    BYTE material_count;
    int cost; // 소요 비용
    int prob; // 확률
    TMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TRefineTable;

typedef struct SPacketGCRefineInformation
{
	BYTE			header;
	BYTE			type;
	BYTE			pos;
	TRefineTable	refine_table;
} TPacketGCRefineInformation;

enum SPECIAL_EFFECT
{
	SE_NONE,
	SE_HPUP_RED,
	SE_SPUP_BLUE,
	SE_SPEEDUP_GREEN,
	SE_DXUP_PURPLE,
	SE_CRITICAL,
	SE_PENETRATE,
	SE_BLOCK,
	SE_DODGE,
	SE_CHINA_FIREWORK,
	SE_SPIN_TOP,
	SE_SUCCESS,
	SE_FAIL,
	SE_FR_SUCCESS,
    SE_LEVELUP_ON_14_FOR_GERMANY,	//레벨업 14일때 ( 독일전용 )
    SE_LEVELUP_UNDER_15_FOR_GERMANY,//레벨업 15일때 ( 독일전용 )
    SE_PERCENT_DAMAGE1,
    SE_PERCENT_DAMAGE2,
    SE_PERCENT_DAMAGE3,
	SE_AUTO_HPUP,
	SE_AUTO_SPUP,
	SE_EQUIP_RAMADAN_RING,			// 초승달의 반지를 착용하는 순간에 발동하는 이펙트
	SE_EQUIP_HALLOWEEN_CANDY,		// 할로윈 사탕을 착용(-_-;)한 순간에 발동하는 이펙트
	SE_EQUIP_HAPPINESS_RING,		// 크리스마스 행복의 반지를 착용하는 순간에 발동하는 이펙트
	SE_EQUIP_LOVE_PENDANT,		// 발렌타인 사랑의 팬던트(71145) 착용할 때 이펙트 (발동이펙트임, 지속이펙트 아님)
#ifdef ENABLE_ACCE_SYSTEM
	SE_EFFECT_ACCE_SUCCEDED,
	SE_EFFECT_ACCE_EQUIP,
#endif
#ifdef ENABLE_TEMPLE_OCHAO
	SE_EFFECT_HEALER,
#endif
#ifdef ENABLE_BATTLE_FIELD
	SE_EFFECT_BATTLE_POTION,
#endif
#ifdef ENABLE_12ZI
	SE_EFFECT_SKILL,
#endif
};

typedef struct SPacketGCSpecialEffect
{
    BYTE header;
    BYTE type;
    DWORD vid;
} TPacketGCSpecialEffect;

typedef struct SPacketGCNPCPosition
{
    BYTE header;
	WORD size;
    WORD count;
} TPacketGCNPCPosition;

struct TNPCPosition
{
    BYTE bType;
    char name[CHARACTER_NAME_MAX_LEN+1];
    long x;
    long y;
};

typedef struct SPacketGCChangeName
{
    BYTE header;
    DWORD pid;
    char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketGCChangeName;

enum EBlockAction
{
    BLOCK_EXCHANGE              = (1 << 0),
    BLOCK_PARTY_INVITE          = (1 << 1),
    BLOCK_GUILD_INVITE          = (1 << 2),
    BLOCK_WHISPER               = (1 << 3),
    BLOCK_MESSENGER_INVITE      = (1 << 4),
    BLOCK_PARTY_REQUEST         = (1 << 5),
};

typedef struct packet_login_key
{
	BYTE	bHeader;
	DWORD	dwLoginKey;
} TPacketGCLoginKey;

typedef struct packet_auth_success
{
    BYTE        bHeader;
    DWORD       dwLoginKey;
    BYTE        bResult;
} TPacketGCAuthSuccess;

typedef struct packet_channel
{
    BYTE header;
    BYTE channel;
} TPacketGCChannel;

typedef struct SEquipmentItemSet
{
	DWORD   vnum;
	BYTE    count;
	long    alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TEquipmentItemSet;

typedef struct pakcet_view_equip
{
    BYTE header;
	DWORD dwVID;
	TEquipmentItemSet equips[WEAR_MAX_NUM];
} TPacketGCViewEquip;

typedef struct
{
    DWORD       dwID;
    long        x, y;
    long        width, height;
    DWORD       dwGuildID;
} TLandPacketElement;

typedef struct packet_land_list
{
    BYTE        header;
    WORD        size;
} TPacketGCLandList;

enum
{
	CREATE_TARGET_TYPE_NONE,
	CREATE_TARGET_TYPE_LOCATION,
	CREATE_TARGET_TYPE_CHARACTER,
};

typedef struct
{
	BYTE		bHeader;
	long		lID;
	char		szTargetName[32+1];
	DWORD		dwVID;
	BYTE		byType;
} TPacketGCTargetCreate;

typedef struct
{
    BYTE        bHeader;
    long        lID;
    long        lX, lY;
} TPacketGCTargetUpdate;

typedef struct
{
    BYTE        bHeader;
    long        lID;
} TPacketGCTargetDelete;

typedef struct
{
    DWORD       dwType;
    BYTE        bPointIdxApplyOn;
    long        lApplyValue;
    DWORD       dwFlag;
    long        lDuration;
    long        lSPCost;
} TPacketAffectElement;

typedef struct
{
    BYTE bHeader;
    TPacketAffectElement elem;
} TPacketGCAffectAdd;

typedef struct
{
    BYTE bHeader;
    DWORD dwType;
    BYTE bApplyOn;
} TPacketGCAffectRemove;

typedef struct packet_mall_open
{
	BYTE bHeader;
	BYTE bSize;
} TPacketGCMallOpen;

typedef struct packet_lover_info
{
	BYTE bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	BYTE byLovePoint;
} TPacketGCLoverInfo;

typedef struct packet_love_point_update
{
	BYTE bHeader;
	BYTE byLovePoint;
} TPacketGCLovePointUpdate;

typedef struct packet_dig_motion
{
    BYTE header;
    DWORD vid;
    DWORD target_vid;
	BYTE count;
} TPacketGCDigMotion;

typedef struct SPacketGCOnTime
{
    BYTE header;
    int ontime;     // sec
} TPacketGCOnTime;

typedef struct SPacketGCResetOnTime
{
    BYTE header;
} TPacketGCResetOnTime;

typedef struct packet_state
{
	BYTE			bHeader;
	BYTE			bFunc;
	BYTE			bArg;
	BYTE			bRot;
	DWORD			dwVID;
	DWORD			dwTime;
	TPixelPosition	kPPos;
} TPacketCCState;

typedef struct SPacketGCSpecificEffect
{
	BYTE header;
	DWORD vid;
	char effect_file[128];
} TPacketGCSpecificEffect;

// 용혼석
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
	DS_SUB_HEADER_DO_UPGRADE,
	DS_SUB_HEADER_DO_IMPROVEMENT,
	DS_SUB_HEADER_DO_REFINE,
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
	TItemPos ItemGrid[DS_REFINE_WINDOW_MAX_NUM];
} TPacketCGDragonSoulRefine;

typedef struct SPacketGCDragonSoulRefine
{
	SPacketGCDragonSoulRefine() : header(HEADER_GC_DRAGON_SOUL_REFINE)
	{}
	BYTE header;
	BYTE bSubType;
	TItemPos Pos;
} TPacketGCDragonSoulRefine;

typedef struct SChannelStatus
{
	short nPort;
	BYTE bStatus;
} TChannelStatus;
typedef struct packet_mark_idxlist
{
	BYTE header;
	DWORD bufSize;
	WORD count;
} TPacketGCMarkIDXList;

typedef struct packet_mark_block
{
	BYTE header;
	DWORD bufSize;
	BYTE imgIdx;
	DWORD count;
} TPacketGCMarkBlock;

typedef struct packet_symbol_data
{
	BYTE header;
	WORD size;
	DWORD guild_id;
} TPacketGCGuildSymbolData;

typedef struct packet_observer_add
{
	BYTE header;
	DWORD vid;
	WORD x;
	WORD y;
} TPacketGCObserverAdd;

typedef struct packet_observer_move
{
	BYTE	header;
	DWORD	vid;
	WORD	x;
	WORD	y;
} TPacketGCObserverMove;
typedef struct packet_observer_remove
{
	BYTE	header;
	DWORD	vid;
} TPacketGCObserverRemove;
#pragma pack(pop)
