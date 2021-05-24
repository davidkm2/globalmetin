#ifndef __INC_AFFECT_H
#define __INC_AFFECT_H

class CAffect
{
	public:
		DWORD	dwType;
		BYTE    bApplyOn;
		long    lApplyValue;
		DWORD   dwFlag;
		long	lDuration;
		long	lSPCost;

		static CAffect* Acquire();
		static void Release(CAffect* p);
};

enum EAffectTypes
{
	AFFECT_NONE,

	AFFECT_MOV_SPEED = 200,						// 200
	AFFECT_ATT_SPEED,							// 201
	AFFECT_ATT_GRADE,							// 202
	AFFECT_INVISIBILITY,						// 203
	AFFECT_STR,									// 204
	AFFECT_DEX,									// 205
	AFFECT_CON,									// 206
	AFFECT_INT,									// 207
	AFFECT_FISH_MIND_PILL,						// 208

	AFFECT_POISON,								// 209 
	AFFECT_STUN,								// 210
	AFFECT_SLOW,								// 211
	AFFECT_DUNGEON_READY,						// 212
	AFFECT_DUNGEON_UNIQUE,						// 213

	AFFECT_BUILDING,							// 214
	AFFECT_REVIVE_INVISIBLE,					// 215
	AFFECT_FIRE,								// 216
	AFFECT_CAST_SPEED,							// 217
	AFFECT_HP_RECOVER_CONTINUE,					// 218
	AFFECT_SP_RECOVER_CONTINUE,					// 219

	AFFECT_POLYMORPH,							// 220
	AFFECT_MOUNT,								// 221

	AFFECT_WAR_FLAG,							// 222

	AFFECT_BLOCK_CHAT,							// 223
	AFFECT_CHINA_FIREWORK,

	AFFECT_BOW_DISTANCE,						// 225
	AFFECT_DEF_GRADE,							// 226
#ifdef __WOLFMAN__
	AFFECT_BLEEDING,							// 227
#endif
#ifdef __GROWTH_PET__
	AFFECT_NEW_PET,								// 228
#endif
#ifdef __BATTLE_FIELD__
	AFFECT_BATTLE_POTION,						// 229
#endif
#ifdef __ELEMENT_ADD__
	AFFECT_MOUNT_FALL,							// 230
	AFFECT_NO_RECOVERY,							// 231
	AFFECT_REDUCE_CAST_SPEED,					// 232
#endif
#ifdef __12ZI__
	AFFECT_ATT_GRADE_DOWN,						// 233
	AFFECT_DEF_GRADE_DOWN,						// 234
	AFFECT_CRITICAL_PCT_DOWN,					// 235
	AFFECT_CZ_UNLIMIT_ENTER,					// 236
#endif

	AFFECT_RAMADAN_ABILITY = 300,				// 300
	AFFECT_RAMADAN_RING = 301,					// 301

	AFFECT_NOG_ABILITY = 302,					// 302
	AFFECT_HOLLY_STONE_POWER = 303,				// 303

	AFFECT_PREMIUM_START = 500,					// 500
	AFFECT_EXP_BONUS = 500,						// 500
	AFFECT_ITEM_BONUS = 501,					// 501
	AFFECT_SAFEBOX = 502,						// 502
	AFFECT_AUTOLOOT = 503,						// 503
	AFFECT_FISH_MIND = 504,						// 504
	AFFECT_MARRIAGE_FAST = 505,					// 505
	AFFECT_GOLD_BONUS = 506,					// 506
	AFFECT_PREMIUM_END = 509,					// 509

	AFFECT_MALL = 510,							// 510
	AFFECT_NO_DEATH_PENALTY	= 511,				// 511
	AFFECT_SKILL_BOOK_BONUS	= 512,				// 512
	AFFECT_SKILL_NO_BOOK_DELAY	= 513,			// 513

	AFFECT_HAIR	= 514,							// 514
	AFFECT_COLLECT = 515,						// 515
	AFFECT_EXP_BONUS_EURO_FREE = 516, 			// 516
	AFFECT_EXP_BONUS_EURO_FREE_UNDER_15 = 517,	// 517
	AFFECT_UNIQUE_ABILITY = 518,				// 518

	AFFECT_CUBE_1,								// 519
	AFFECT_CUBE_2,								// 520
	AFFECT_CUBE_3,								// 521
	AFFECT_CUBE_4,								// 522
	AFFECT_CUBE_5,								// 523
	AFFECT_CUBE_6,								// 524
	AFFECT_CUBE_7,								// 525
	AFFECT_CUBE_8,								// 526
	AFFECT_CUBE_9,								// 527
	AFFECT_CUBE_10,								// 528
	AFFECT_CUBE_11,								// 529
	AFFECT_CUBE_12,								// 530

	AFFECT_BLEND,								// 531

	AFFECT_HORSE_NAME,							// 532
	AFFECT_MOUNT_BONUS,							// 533

	AFFECT_AUTO_HP_RECOVERY = 534,				// 534
	AFFECT_AUTO_SP_RECOVERY = 535,				// 535

	AFFECT_DRAGON_SOUL_QUALIFIED = 540,			// 540
	AFFECT_DRAGON_SOUL_DECK_0 = 541,			// 541
	AFFECT_DRAGON_SOUL_DECK_1 = 542,			// 542

#ifdef __GUILD_DRAGONLAIR__
	AFFECT_STATUE,								// 543
#endif

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
	AFFECT_EXP_BONUS_EVENT,						// 544
	AFFECT_ATT_SPEED_SLOW,						// 545
#endif

#ifdef __EXPRESSING_EMOTION__
	AFFECT_EMOTION,								// 546
#endif

	AFFECT_QUEST_START_IDX = 1000				// 1000
};

enum EAffectBits
{
	AFF_NONE,							// 0

	AFF_YMIR,							// 1
	AFF_INVISIBILITY,					// 2
	AFF_SPAWN,							// 3

	AFF_POISON,							// 4
	AFF_SLOW,							// 5
	AFF_STUN,							// 6

	AFF_DUNGEON_READY,					// 7
	AFF_DUNGEON_UNIQUE,					// 8

	AFF_BUILDING_CONSTRUCTION_SMALL,	// 9
	AFF_BUILDING_CONSTRUCTION_LARGE,	// 10
	AFF_BUILDING_UPGRADE,				// 11

	AFF_MOV_SPEED_POTION,				// 12
	AFF_ATT_SPEED_POTION,				// 13

	AFF_FISH_MIND,						// 14

	AFF_JEONGWIHON,						// 15
	AFF_GEOMGYEONG,						// 16
	AFF_CHEONGEUN,						// 17
	AFF_GYEONGGONG,						// 18
	AFF_EUNHYUNG,						// 19
	AFF_GWIGUM,							// 20
	AFF_TERROR,							// 21
	AFF_JUMAGAP,						// 22
	AFF_HOSIN,							// 23
	AFF_BOHO,							// 24
	AFF_KWAESOK,						// 25
	AFF_MANASHIELD,						// 26
	AFF_MUYEONG,						// 27
	AFF_REVIVE_INVISIBLE,				// 28
	AFF_FIRE,							// 29
	AFF_GICHEON,						// 30
	AFF_JEUNGRYEOK,						// 31
	AFF_TANHWAN_DASH,					// 32
	AFF_PABEOP,							// 33
	AFF_CHEONGEUN_WITH_FALL,			// 34
	AFF_POLYMORPH,						// 35
	AFF_WAR_FLAG1,						// 36
	AFF_WAR_FLAG2,						// 37
	AFF_WAR_FLAG3,						// 38

	AFF_CHINA_FIREWORK,					// 39
	AFF_HAIR,							// 40
	AFF_GERMANY,						// 41
	AFF_RAMADAN_RING,					// 42

#ifdef __WOLFMAN__
	AFF_BLEEDING,						// 43
	AFF_RED_POSSESSION,					// 44
	AFF_BLUE_POSSESSION,				// 45
#endif

#ifdef __12ZI__
	AFF_ELECTRIC_SHOCK,					// 46
	AFF_CONFUSION,						// 47
#endif

#ifdef __GUILD_DRAGONLAIR__
	AFF_STATUE1,						// 48
	AFF_STATUE2,						// 49
	AFF_STATUE3,						// 50
	AFF_STATUE4,						// 51
#endif

	AFF_BITS_MAX,
};

extern void SendAffectAddPacket(LPDESC d, CAffect * pkAff);

// AFFECT_DURATION_BUG_FIX
enum AffectVariable
{
	INFINITE_AFFECT_DURATION = 60 * 365 * 24 * 60 * 60
};
// END_AFFECT_DURATION_BUG_FIX

#endif
