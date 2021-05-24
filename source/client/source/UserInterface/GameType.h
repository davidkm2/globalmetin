#pragma once
#include "../GameLib/ItemData.h"
#include "Locale_inc.h"

struct SAffects
{
	enum
	{
		AFFECT_MAX_NUM = 32,
	};

	SAffects() : dwAffects(0) {}
	SAffects(const DWORD & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}
	int operator = (const DWORD & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}

	BOOL IsAffect(BYTE byIndex)
	{
		return dwAffects & (1 << byIndex);
	}

	void __SetAffects(const DWORD & c_rAffects)
	{
		dwAffects = c_rAffects;
	}

	DWORD dwAffects;
};

enum
{
	POINT_MAX_NUM = 255,
	CHARACTER_NAME_MAX_LEN = 24,
	PLAYER_NAME_MAX_LEN = 12,
};

enum EShopPriceType
{
	SHOP_PRICE_GOLD,
#ifdef ENABLE_CHEQUE_SYSTEM
	SHOP_PRICE_CHEQUE,
#endif
#ifdef ENABLE_BATTLE_FIELD
	SHOP_PRICE_BATTLE_POINT,
#endif
	SHOP_PRICE_MAX_NUM
};

extern std::string g_strGuildSymbolPathName;

const DWORD c_Name_Max_Length = 64;
const DWORD c_FileName_Max_Length = 128;
const DWORD c_Short_Name_Max_Length = 32;

const DWORD c_Inventory_Page_Column = 5;
const DWORD c_Inventory_Page_Row = 9;
const DWORD c_Inventory_Page_Size = c_Inventory_Page_Column*c_Inventory_Page_Row; // x*y
const DWORD c_Inventory_Page_Count = 4;
const DWORD c_ItemSlot_Count = c_Inventory_Page_Size * c_Inventory_Page_Count;
const DWORD c_Equipment_Count = 12;
#ifdef ENABLE_EXTEND_INVEN_SYSTEM
const DWORD c_Ex_Inventory_Page_Count = 2;
const DWORD c_Ex_Inventory_Stage_Max = 18;
const DWORD c_Ex_Inventory_Page_Start = 3;
#endif

const DWORD c_Equipment_Start = c_ItemSlot_Count;

enum EEquipment
{
	EQUIPMENT_BODY = c_ItemSlot_Count,	// 180 + 0
	EQUIPMENT_HEAD,
	EQUIPMENT_SHOES,					// 180 + 2
	EQUIPMENT_WRIST,					// 180 + 3
	EQUIPMENT_WEAPON,					// 180 + 4
	EQUIPMENT_NECK,
	EQUIPMENT_EAR,
	EQUIPMENT_UNIQUE1,					// 180 + 7
	EQUIPMENT_UNIQUE2,					// 180 + 8
	EQUIPMENT_ARROW,					// 180 + 9
	EQUIPMENT_SHIELD,					// 180 + 10
	EQUIPMENT_ABILITY1, 				// 180 + 11
	EQUIPMENT_ABILITY2,  				// 180 + 12
	EQUIPMENT_ABILITY3,  				// 180 + 13
	EQUIPMENT_ABILITY4,  				// 180 + 14
	EQUIPMENT_ABILITY5,  				// 180 + 15
	EQUIPMENT_ABILITY6,  				// 180 + 16
	EQUIPMENT_ABILITY7,  				// 180 + 17
	EQUIPMENT_ABILITY8,  				// 180 + 18
	EQUIPMENT_RING1,  					// 180 + 19
	EQUIPMENT_RING2,  					// 180 + 20
	EQUIPMENT_BELT,  					// 180 + 21
#ifdef ENABLE_PENDANT
	EQUIPMENT_PENDANT,  				// 180 + 22
#endif
#ifdef ENABLE_COSTUME_PET
	EQUIPMENT_PET,
#endif
	EQUIPMENT_MAX,
};

enum ECostume
{
	COSTUME_BODY = EQUIPMENT_MAX,		// 180 + 24
	COSTUME_HAIR,
#ifdef ENABLE_COSTUME_MOUNT
	COSTUME_MOUNT, 
#endif
#ifdef ENABLE_ACCE_SYSTEM
	COSTUME_ACCE,  
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	COSTUME_WEAPON,  					// 180 + 28
#endif
#ifdef ENABLE_COSTUME_EFFECT
	COSTUME_EFFECT_BODY,				// 180 + 29
	COSTUME_EFFECT_WEAPON,				// 180 + 30
#endif
	COSTUME_MAX,
};
#ifdef ENABLE_NEW_STORAGE_SYSTEM
const DWORD c_Storage_Page_Column = 5;
const DWORD c_Storage_Page_Row = 9;
const DWORD c_Storage_Page_Size = c_Storage_Page_Column * c_Storage_Page_Row; // x*y
const DWORD c_Storage_Page_Count = 4;
const DWORD c_Storage_Count = c_Storage_Page_Size * c_Storage_Page_Count;
#endif

enum EDragonSoulDeckType
{
	DS_DECK_1,
	DS_DECK_2,
	DS_DECK_MAX_NUM = 2,
};

enum EDragonSoulGradeTypes
{
	DRAGON_SOUL_GRADE_NORMAL,
	DRAGON_SOUL_GRADE_BRILLIANT,
	DRAGON_SOUL_GRADE_RARE,
	DRAGON_SOUL_GRADE_ANCIENT,
	DRAGON_SOUL_GRADE_LEGENDARY,
	DRAGON_SOUL_GRADE_MAX,

};

enum EDragonSoulStepTypes
{
	DRAGON_SOUL_STEP_LOWEST,
	DRAGON_SOUL_STEP_LOW,
	DRAGON_SOUL_STEP_MID,
	DRAGON_SOUL_STEP_HIGH,
	DRAGON_SOUL_STEP_HIGHEST,
	DRAGON_SOUL_STEP_MAX,
};

const DWORD c_Wear_Max = CItemData::WEAR_MAX_NUM;
const DWORD c_DragonSoul_Equip_Start = c_ItemSlot_Count + c_Wear_Max;
const DWORD c_DragonSoul_Equip_Slot_Max = 6;
const DWORD c_DragonSoul_Equip_End = c_DragonSoul_Equip_Start + c_DragonSoul_Equip_Slot_Max * DS_DECK_MAX_NUM;

const DWORD c_DragonSoul_Equip_Reserved_Count = c_DragonSoul_Equip_Slot_Max * 3;

const DWORD c_Belt_Inventory_Slot_Start = c_DragonSoul_Equip_End + c_DragonSoul_Equip_Reserved_Count;
const DWORD c_Belt_Inventory_Width = 4;
const DWORD c_Belt_Inventory_Height= 4;
const DWORD c_Belt_Inventory_Slot_Count = c_Belt_Inventory_Width * c_Belt_Inventory_Height;
const DWORD c_Belt_Inventory_Slot_End = c_Belt_Inventory_Slot_Start + c_Belt_Inventory_Slot_Count;

const DWORD c_Inventory_Count	= c_Belt_Inventory_Slot_End;

const DWORD c_DragonSoul_Inventory_Start = 0;
const DWORD c_DragonSoul_Inventory_Box_Size = 32;
const DWORD c_DragonSoul_Inventory_Count = CItemData::DS_SLOT_NUM_TYPES * DRAGON_SOUL_GRADE_MAX * c_DragonSoul_Inventory_Box_Size;
const DWORD c_DragonSoul_Inventory_End = c_DragonSoul_Inventory_Start + c_DragonSoul_Inventory_Count;

enum ESlotType
{
	SLOT_TYPE_NONE,
	SLOT_TYPE_INVENTORY,
	SLOT_TYPE_SKILL,
	SLOT_TYPE_EMOTION,
	SLOT_TYPE_SHOP,
	SLOT_TYPE_EXCHANGE_OWNER,
	SLOT_TYPE_EXCHANGE_TARGET,
	SLOT_TYPE_QUICK_SLOT,
	SLOT_TYPE_SAFEBOX,
	SLOT_TYPE_PRIVATE_SHOP,
	SLOT_TYPE_MALL,
	SLOT_TYPE_DRAGON_SOUL_INVENTORY,
#ifdef ENABLE_NEW_STORAGE_SYSTEM
	SLOT_TYPE_UPGRADE_INVENTORY,
	SLOT_TYPE_BOOK_INVENTORY,
	SLOT_TYPE_STONE_INVENTORY,
	SLOT_TYPE_ATTR_INVENTORY,
	SLOT_TYPE_GIFTBOX_INVENTORY,
#endif
#ifdef ENABLE_AUTO_SYSTEM
	SLOT_TYPE_AUTO,
#endif
#ifdef ENABLE_FISH_EVENT
	SLOT_TYPE_FISH_EVENT,
#endif
	SLOT_TYPE_MAX,
};

enum EWindows
{
	RESERVED_WINDOW,
	INVENTORY,				// 기본 인벤토리. (45칸 짜리가 2페이지 존재 = 90칸)
	EQUIPMENT,
	SAFEBOX,
	MALL,
	DRAGON_SOUL_INVENTORY,
	BELT_INVENTORY,			// NOTE: W2.1 버전에 새로 추가되는 벨트 슬롯 아이템이 제공하는 벨트 인벤토리
	GROUND,					// NOTE: 2013년 2월5일 현재까지 unused.. 왜 있는거지???
#ifdef ENABLE_NEW_STORAGE_SYSTEM
	UPGRADE_INVENTORY,
	BOOK_INVENTORY,
	STONE_INVENTORY,
	ATTR_INVENTORY,
	GIFTBOX_INVENTORY,
#endif
	WINDOW_TYPE_MAX,
};

#ifdef ENABLE_AUTO_SYSTEM
enum EAutoSlots
{
	AUTO_SKILL_SLOT_MAX = 8,
	AUTO_POSITINO_SLOT_MAX = 16,
};
#endif

#ifdef ENABLE_MINI_GAME
enum EMiniGameTypes
{
#ifdef ENABLE_MINI_GAME_RUMI
	MINIGAME_TYPE_RUMI,
#endif
#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
	MINIGAME_ATTENDANCE,
	MINIGAME_MONSTERBACK,
#endif
#ifdef ENABLE_FISH_EVENT
	MINIGAME_FISH,
#endif
	MINIGAME_YUTNORI,
	MINIGAME_CATCHKING,
	MINIGAME_TYPE_MAX,
};
#endif

#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
enum EAttenDanceDataTypes
{
	ATTENDANCE_DATA_TYPE_DAY,
	ATTENDANCE_DATA_TYPE_MISSION_CLEAR,
	ATTENDANCE_DATA_TYPE_GET_REWARD,
	ATTENDANCE_DATA_TYPE_SHOW_MAX,
};
#endif

#ifdef ENABLE_FISH_EVENT
enum EFishEventInfo
{
	FISH_EVENT_SHAPE_NONE,
	FISH_EVENT_SHAPE_1,
	FISH_EVENT_SHAPE_2,			
	FISH_EVENT_SHAPE_3,			
	FISH_EVENT_SHAPE_4,			
	FISH_EVENT_SHAPE_5,			
	FISH_EVENT_SHAPE_6,			
	FISH_EVENT_SHAPE_7,
	FISH_EVENT_SHAPE_MAX_NUM,
};
#endif

enum EDSInventoryMaxNum
{
	DS_INVENTORY_MAX_NUM = c_DragonSoul_Inventory_Count,
	DS_REFINE_WINDOW_MAX_NUM = 15,
};

#pragma pack (push, 1)
#define WORD_MAX 0xffff

typedef struct SItemPos
{
	BYTE window_type;
	WORD cell;
    SItemPos ()
    {
		window_type = INVENTORY;
		cell = WORD_MAX;
    }
	SItemPos (BYTE _window_type, WORD _cell)
    {
        window_type = _window_type;
        cell = _cell;
    }

	bool IsValidCell()
	{
		switch (window_type)
		{
		case INVENTORY:
			return cell < c_Inventory_Count;
			break;
		case EQUIPMENT:
			return cell < c_DragonSoul_Equip_End;
			break;
		case DRAGON_SOUL_INVENTORY:
			return cell < (DS_INVENTORY_MAX_NUM);
			break;
#ifdef ENABLE_NEW_STORAGE_SYSTEM
		case UPGRADE_INVENTORY:
			return cell < c_Storage_Count;
			break;
		case BOOK_INVENTORY:
			return cell < c_Storage_Count;
			break;
		case STONE_INVENTORY:
			return cell < c_Storage_Count;
			break;
		case ATTR_INVENTORY:
			return cell < c_Storage_Count;
			break;
		case GIFTBOX_INVENTORY:
			return cell < c_Storage_Count;
			break;
#endif
		default:
			return false;
		}
	}
	bool IsEquipCell()
	{
		switch (window_type)
		{
		case INVENTORY:
		case EQUIPMENT:
			return (c_ItemSlot_Count + c_Wear_Max > cell) && (c_ItemSlot_Count <= cell);
			break;

		case BELT_INVENTORY:
		case DRAGON_SOUL_INVENTORY:
#ifdef ENABLE_NEW_STORAGE_SYSTEM
		case UPGRADE_INVENTORY:
		case BOOK_INVENTORY:
		case STONE_INVENTORY:
		case ATTR_INVENTORY:
		case GIFTBOX_INVENTORY:
#endif
			return false;
			break;

		default:
			return false;
		}
	}

	bool IsBeltInventoryCell()
	{
		bool bResult = c_Belt_Inventory_Slot_Start <= cell && c_Belt_Inventory_Slot_End > cell;
		return bResult;
	}

#ifdef ENABLE_NEW_STORAGE_SYSTEM
	bool IsUpgradeInventoryCell()
	{
		bool bResult = UPGRADE_INVENTORY == window_type && 0 <= cell && c_Storage_Count > cell;
		return bResult;
	}

	bool IsBookInventoryCell()
	{
		bool bResult = BOOK_INVENTORY == window_type && 0 <= cell && c_Storage_Count > cell;
		return bResult;
	}

	bool IsStoneInventoryCell()
	{
		bool bResult = STONE_INVENTORY == window_type && 0 <= cell && c_Storage_Count > cell;
		return bResult;
	}

	bool IsAttrInventoryCell()
	{
		bool bResult = ATTR_INVENTORY == window_type && 0 <= cell && c_Storage_Count > cell;
		return bResult;
	}

	bool IsGiftBoxInventoryCell()
	{
		bool bResult = GIFTBOX_INVENTORY == window_type && 0 <= cell && c_Storage_Count > cell;
		return bResult;
	}
#endif

	bool operator==(const struct SItemPos& rhs) const
	{
		return (window_type == rhs.window_type) && (cell == rhs.cell);
	}

	bool operator<(const struct SItemPos& rhs) const
	{
		return (window_type < rhs.window_type) || ((window_type == rhs.window_type) && (cell < rhs.cell));
	}
} TItemPos;

#pragma pack(pop)

const DWORD c_QuickBar_Line_Count = 3;
const DWORD c_QuickBar_Slot_Count = 12;

const float c_Idle_WaitTime = 5.0f;

const int c_Monster_Race_Start_Number = 6;
const int c_Monster_Model_Start_Number = 20001;

const float c_fAttack_Delay_Time = 0.2f;
const float c_fHit_Delay_Time = 0.1f;
const float c_fCrash_Wave_Time = 0.2f;
const float c_fCrash_Wave_Distance = 3.0f;

const float c_fHeight_Step_Distance = 50.0f;

enum
{
	DISTANCE_TYPE_FOUR_WAY,
	DISTANCE_TYPE_EIGHT_WAY,
	DISTANCE_TYPE_ONE_WAY,
	DISTANCE_TYPE_MAX_NUM,
};

const float c_fMagic_Script_Version = 1.0f;
const float c_fSkill_Script_Version = 1.0f;
const float c_fMagicSoundInformation_Version = 1.0f;
const float c_fBattleCommand_Script_Version = 1.0f;
const float c_fEmotionCommand_Script_Version = 1.0f;
const float c_fActive_Script_Version = 1.0f;
const float c_fPassive_Script_Version = 1.0f;

// Used by PushMove
const float c_fWalkDistance = 175.0f;
const float c_fRunDistance = 310.0f;

#define FILE_MAX_LEN 128

enum
{
	ITEM_SOCKET_SLOT_MAX_NUM = 4,
	// refactored attribute slot begin
	ITEM_ATTRIBUTE_SLOT_NORM_NUM	= 5,
	ITEM_ATTRIBUTE_SLOT_RARE_NUM	= 2,

	ITEM_ATTRIBUTE_SLOT_NORM_START	= 0,
	ITEM_ATTRIBUTE_SLOT_NORM_END	= ITEM_ATTRIBUTE_SLOT_NORM_START + ITEM_ATTRIBUTE_SLOT_NORM_NUM,

	ITEM_ATTRIBUTE_SLOT_RARE_START	= ITEM_ATTRIBUTE_SLOT_NORM_END,
	ITEM_ATTRIBUTE_SLOT_RARE_END	= ITEM_ATTRIBUTE_SLOT_RARE_START + ITEM_ATTRIBUTE_SLOT_RARE_NUM,

	ITEM_ATTRIBUTE_SLOT_MAX_NUM		= ITEM_ATTRIBUTE_SLOT_RARE_END, // 7
	// refactored attribute slot end
};

#pragma pack(push)
#pragma pack(1)

typedef struct SQuickSlot
{
	BYTE Type;
	BYTE Position;
} TQuickSlot;

typedef struct TPlayerItemAttribute
{
    BYTE        bType;
    int			sValue;
} TPlayerItemAttribute;

typedef struct packet_item
{
    DWORD       vnum;
    BYTE        count;
	DWORD		flags;
	DWORD		anti_flags;
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_SOULBIND_SYSTEM
	long		sealbind;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	DWORD		transmutation;
#endif
	bool		bIsBasic;
} TItemData;

typedef struct packet_shop_item
{
    DWORD       vnum;
	DWORD		dwPrices[SHOP_PRICE_MAX_NUM];
    BYTE        count;
	BYTE		display_pos;
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	DWORD	transmutation;
#endif
} TShopItemData;

#ifdef ENABLE_SHOP_SELL_INFO
typedef struct SShopSellInfo
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	bool bIsSell;
	long lSellTime;
} TShopSellInfo;
#endif

#ifdef ENABLE_GEM_SYSTEM
typedef struct SGemShopItem
{
	DWORD	dwVnum;
	DWORD	dwPrice;
	BYTE	bEnable;
	BYTE	bCount;
} TGemShopItem;
#endif

#ifdef ENABLE_MINI_GAME_RUMI
typedef struct SMiniGameRumiMoveCard
{
	BYTE bPos;
	BYTE bIndex;
	BYTE bColor;
	BYTE bNumber;
} TMiniGameRumiMoveCard;

typedef struct SMiniGameRumiMove
{
	TMiniGameRumiMoveCard src;
	TMiniGameRumiMoveCard dst;
} TMiniGameRumiMove;

typedef struct SMiniGameRumiScore
{
	BYTE bScore;
	short sTotalScore;
} TMiniGameRumiScore;
#endif

#ifdef ENABLE_MYSHOP_DECO
typedef struct SMyshopDeco
{
	BYTE bDeco;
	BYTE bVnum;
} TMyshopDeco;
#endif

#pragma pack(pop)

inline float GetSqrtDistance(int ix1, int iy1, int ix2, int iy2) // By sqrt
{
	float dx, dy;

	dx = float(ix1 - ix2);
	dy = float(iy1 - iy2);

	return sqrtf(dx*dx + dy*dy);
}

// DEFAULT_FONT
void DefaultFont_Startup();
void DefaultFont_Cleanup();
void DefaultFont_SetName(const char * c_szFontName);
CResource* DefaultFont_GetResource();
CResource* DefaultItalicFont_GetResource();
// END_OF_DEFAULT_FONT

void SetGuildSymbolPath(const char * c_szPathName);
const char * GetGuildSymbolFileName(DWORD dwGuildID);
BYTE SlotTypeToInvenType(BYTE bSlotType);
