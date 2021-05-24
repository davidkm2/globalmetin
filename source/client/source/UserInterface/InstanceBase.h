#pragma once
#include "Locale_inc.h"
#include "../GameLib/RaceData.h"
#include "../GameLib/ActorInstance.h"
#ifdef ENABLE_ACCE_SYSTEM
#include "../EterLib/GrpObjectInstance.h"
#endif
#include "AffectFlagContainer.h"

class CInstanceBase
{
	public:
		struct SCreateData
		{
			BYTE	m_bType;
			DWORD	m_dwStateFlags;
			DWORD	m_dwEmpireID;
			DWORD	m_dwGuildID;
			DWORD	m_dwLevel;
			DWORD	m_dwVID;
			DWORD	m_dwRace;
			DWORD	m_dwMovSpd;
			DWORD	m_dwAtkSpd;
			LONG	m_lPosX;
			LONG	m_lPosY;
			FLOAT	m_fRot;
			DWORD	m_dwArmor;
			DWORD	m_dwWeapon;
			DWORD	m_dwHair;
#ifdef ENABLE_ACCE_SYSTEM
			DWORD	m_dwAcce;
#endif
			DWORD	m_dwMountVnum;
#ifdef ENABLE_GUILD_GENERAL_AND_LEADER
			BYTE	m_dwNewIsGuildName;
#endif
			short	m_sAlignment;
			BYTE	m_byPKMode;
			CAffectFlagContainer	m_kAffectFlags;

			std::string m_stName;

			bool	m_isMain;
#ifdef ENABLE_QUIVER_SYSTEM
			DWORD	m_dwArrowType;
#endif
#ifdef ENABLE_COSTUME_EFFECT
			DWORD	m_dwBodyEffect;
			DWORD	m_dwWeaponEffectRight;
			DWORD	m_dwWeaponEffectLeft;
#endif
#ifdef ENABLE_BATTLE_FIELD
			BYTE	m_bBattleFieldRank;
#endif
#ifdef ENABLE_MULTI_LANGUAGE
			std::string m_stLanguage;
#endif
		};

	public:
		typedef DWORD TType;

		enum EDirection
		{
			DIR_NORTH,
			DIR_NORTHEAST,
			DIR_EAST,
			DIR_SOUTHEAST,
			DIR_SOUTH,
			DIR_SOUTHWEST,
			DIR_WEST,
			DIR_NORTHWEST,
			DIR_MAX_NUM,
		};

		enum EFunc
		{
			FUNC_WAIT,
			FUNC_MOVE,
			FUNC_ATTACK,
			FUNC_COMBO,
			FUNC_MOB_SKILL,
			FUNC_EMOTION,
			FUNC_SKILL = 0x80,
		};

		enum EAffectBits
		{
			// AFFECT_NONE,						// 0
			AFFECT_YMIR,						// 0
			AFFECT_INVISIBILITY,				// 1
			AFFECT_SPAWN,						// 2

			AFFECT_POISON,						// 3
			AFFECT_SLOW,						// 4
			AFFECT_STUN,						// 5

			AFFECT_DUNGEON_READY,				// 6
			AFFECT_SHOW_ALWAYS,					// 7

			AFFECT_BUILDING_CONSTRUCTION_SMALL,	// 8
			AFFECT_BUILDING_CONSTRUCTION_LARGE,	// 9
			AFFECT_BUILDING_UPGRADE,			// 10

			AFFECT_MOV_SPEED_POTION,			// 11
			AFFECT_ATT_SPEED_POTION,			// 12

			AFFECT_FISH_MIND,					// 13

			AFFECT_JEONGWI,						// 14
			AFFECT_GEOMGYEONG,					// 15
			AFFECT_CHEONGEUN,					// 16
			AFFECT_GYEONGGONG,					// 17
			AFFECT_EUNHYEONG,					// 18
			AFFECT_GWIGEOM,						// 19
			AFFECT_GONGPO,						// 20
			AFFECT_JUMAGAP,						// 21
			AFFECT_HOSIN,						// 22
			AFFECT_BOHO,						// 23
			AFFECT_KWAESOK,						// 24
		    AFFECT_HEUKSIN,						// 25
			AFFECT_MUYEONG,						// 26
			AFFECT_REVIVE_INVISIBILITY,			// 27
			AFFECT_FIRE,						// 28
			AFFECT_GICHEON,						// 29
			AFFECT_JEUNGRYEOK,					// 30
			AFFECT_DASH,						// 31
			AFFECT_PABEOP,						// 32
			AFFECT_FALLEN_CHEONGEUN,			// 33
			AFFECT_POLYMORPH,					// 34
			AFFECT_WAR_FLAG1,					// 35
			AFFECT_WAR_FLAG2,					// 36
			AFFECT_WAR_FLAG3,					// 37
			AFFECT_CHINA_FIREWORK,				// 38
			AFFECT_PREMIUM_SILVER,				// 39
			AFFECT_PREMIUM_GOLD,				// 40
			AFFECT_RAMADAN_RING,				// 41
#ifdef ENABLE_WOLFMAN_CHARACTER
			AFFECT_BLEEDING,					// 42
			AFFECT_RED_POSSESSION,				// 43
			AFFECT_BLUE_POSSESSION,				// 44
#endif
#ifdef ENABLE_12ZI
			AFFECT_ELECTRIC_SHOCK,				// 45
			AFFECT_CONFUSION,					// 46
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
			AFFECT_STATUE1,						// 47
			AFFECT_STATUE2,						// 48
			AFFECT_STATUE3,						// 49
			AFFECT_STATUE4,						// 50
#endif
			AFFECT_NUM = 64,
		};

		enum EAffectTypes
		{
			NEW_AFFECT_MOV_SPEED = 200,						// 200
			NEW_AFFECT_ATT_SPEED,							// 201
			NEW_AFFECT_ATT_GRADE,							// 202
			NEW_AFFECT_INVISIBILITY,						// 203
			NEW_AFFECT_STR,									// 204
			NEW_AFFECT_DEX,                 				// 205
			NEW_AFFECT_CON,									// 206
			NEW_AFFECT_INT,									// 207
			NEW_AFFECT_FISH_MIND_PILL,						// 208

			NEW_AFFECT_POISON,								// 209
			NEW_AFFECT_STUN,                				// 210
			NEW_AFFECT_SLOW,								// 211
			NEW_AFFECT_DUNGEON_READY,						// 212
			NEW_AFFECT_DUNGEON_UNIQUE,						// 213

			NEW_AFFECT_BUILDING,							// 214
			NEW_AFFECT_REVIVE_INVISIBLE,    				// 215
			NEW_AFFECT_FIRE,								// 216
			NEW_AFFECT_CAST_SPEED,							// 217
			NEW_AFFECT_HP_RECOVER_CONTINUE,					// 218
			NEW_AFFECT_SP_RECOVER_CONTINUE,					// 219

			NEW_AFFECT_POLYMORPH,           				// 220
			NEW_AFFECT_MOUNT,								// 221

			NEW_AFFECT_WAR_FLAG,            				// 222

			NEW_AFFECT_BLOCK_CHAT,          				// 223
			NEW_AFFECT_CHINA_FIREWORK,						// 224

			NEW_AFFECT_BOW_DISTANCE,        				// 225
			NEW_AFFECT_DEF_GRADE,							// 226
#ifdef ENABLE_WOLFMAN_CHARACTER
			NEW_AFFECT_BLEEDING,							// 227
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
			NEW_AFFECT_NEW_PET,								// 228
#endif
#ifdef ENABLE_BATTLE_FIELD
			NEW_AFFECT_BATTLE_POTION,						// 229
#endif
#ifdef ENABLE_ELEMENT_ADD
			NEW_AFFECT_MOUNT_FALL,							// 230
			NEW_AFFECT_NO_RECOVERY,							// 231
			NEW_AFFECT_REDUCE_CAST_SPEED,					// 232
#endif
#ifdef ENABLE_12ZI
			NEW_AFFECT_ATT_GRADE_DOWN,						// 233
			NEW_AFFECT_DEF_GRADE_DOWN,						// 234
			NEW_AFFECT_CRITICAL_PCT_DOWN,					// 235
			NEW_AFFECT_CZ_UNLIMIT_ENTER,					// 236
#endif

			NEW_AFFECT_RAMADAN_ABILITY = 300,				// 300
			NEW_AFFECT_RAMADAN_RING = 301,					// 301

			NEW_AFFECT_NOG_POCKET_ABILITY = 302,			// 302
			NEW_AFFECT_HOLLY_STONE_POWER = 303,				// 303

			NEW_AFFECT_EXP_BONUS = 500,						// 500
			NEW_AFFECT_ITEM_BONUS = 501,					// 501
			NEW_AFFECT_SAFEBOX = 502,						// 502
			NEW_AFFECT_AUTOLOOT = 503,						// 503
			NEW_AFFECT_FISH_MIND = 504,						// 504
			NEW_AFFECT_MARRIAGE_FAST = 505, 				// 505
			NEW_AFFECT_GOLD_BONUS = 506,					// 506

		    NEW_AFFECT_MALL = 510, 							// 510
			NEW_AFFECT_NO_DEATH_PENALTY = 511,				// 511
			NEW_AFFECT_SKILL_BOOK_BONUS = 512,				// 512
			NEW_AFFECT_SKILL_BOOK_NO_DELAY = 513,			// 513

			NEW_AFFECT_HAIR	= 514,							// 514
			NEW_AFFECT_COLLECT = 515,						// 515

			NEW_AFFECT_EXP_BONUS_EURO_FREE = 516, 			// 516
			NEW_AFFECT_EXP_BONUS_EURO_FREE_UNDER_15 = 517,	// 517

			NEW_AFFECT_UNIQUE_ABILITY = 518,				// 518

			NEW_AFFECT_CUBE_1,								// 519
			NEW_AFFECT_CUBE_2,								// 520
			NEW_AFFECT_CUBE_3,								// 521
			NEW_AFFECT_CUBE_4,								// 522
			NEW_AFFECT_CUBE_5,								// 523
			NEW_AFFECT_CUBE_6,								// 524
			NEW_AFFECT_CUBE_7,								// 525
			NEW_AFFECT_CUBE_8,								// 526
			NEW_AFFECT_CUBE_9,								// 527
			NEW_AFFECT_CUBE_10,								// 528
			NEW_AFFECT_CUBE_11,								// 529
			NEW_AFFECT_CUBE_12,								// 530

			NEW_AFFECT_BLEND,								// 531

			NEW_AFFECT_HORSE_NAME,							// 532
			NEW_AFFECT_MOUNT_BONUS,							// 533

			NEW_AFFECT_AUTO_HP_RECOVERY = 534,				// 534
			NEW_AFFECT_AUTO_SP_RECOVERY = 535,				// 535

			NEW_AFFECT_DRAGON_SOUL_QUALIFIED = 540,			// 540
			NEW_AFFECT_DRAGON_SOUL_DECK1 = 541,				// 541
			NEW_AFFECT_DRAGON_SOUL_DECK2 = 542,				// 542

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
			NEW_AFFECT_STATUE,								// 543
#endif

#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
			NEW_AFFECT_EXP_BONUS_EVENT,						// 544
			NEW_AFFECT_ATT_SPEED_SLOW,						// 545
#endif

#ifdef ENABLE_EXPRESSING_EMOTION
			NEW_AFFECT_EMOTION,								// 546
#endif

			NEW_AFFECT_QUEST_START_IDX = 1000,				// 1000
		};

		enum EStone
		{
			STONE_SMOKE1 = 0,	// 99%
			STONE_SMOKE2 = 1,	// 85%
			STONE_SMOKE3 = 2,	// 80%
			STONE_SMOKE4 = 3,	// 60%
			STONE_SMOKE5 = 4,	// 45%
			STONE_SMOKE6 = 5,	// 40%
			STONE_SMOKE7 = 6,	// 20%
			STONE_SMOKE8 = 7,	// 10%
			STONE_SMOKE_NUM = 4,
		};

		enum EBuildingAffect
		{
			BUILDING_CONSTRUCTION_SMALL = 0,
			BUILDING_CONSTRUCTION_LARGE = 1,
			BUILDING_UPGRADE = 2,
		};

		enum EWeapon
		{
			WEAPON_DUALHAND,
			WEAPON_ONEHAND,
			WEAPON_TWOHAND,
			WEAPON_NUM,
		};

		enum EEmpire
		{
			EMPIRE_NONE,
			EMPIRE_A,
			EMPIRE_B,
			EMPIRE_C,
			EMPIRE_NUM,
		};

		enum ENameColor
		{
			NAMECOLOR_MOB,
			NAMECOLOR_NPC,
			NAMECOLOR_PC,
			NAMECOLOR_PC_END = NAMECOLOR_PC + EMPIRE_NUM,
			NAMECOLOR_NORMAL_MOB,
			NAMECOLOR_NORMAL_NPC,
			NAMECOLOR_NORMAL_PC,
			NAMECOLOR_NORMAL_PC_END = NAMECOLOR_NORMAL_PC + EMPIRE_NUM,
			NAMECOLOR_EMPIRE_MOB,
			NAMECOLOR_EMPIRE_NPC,
			NAMECOLOR_EMPIRE_PC,
			NAMECOLOR_EMPIRE_PC_END = NAMECOLOR_EMPIRE_PC + EMPIRE_NUM,
			NAMECOLOR_FUNC,
			NAMECOLOR_PK,
			NAMECOLOR_PVP,
			NAMECOLOR_PARTY,
			NAMECOLOR_WARP,
			NAMECOLOR_WAYPOINT,
			NAMECOLOR_METIN,
			NAMECOLOR_SHOP,
			NAMECOLOR_EXTRA = NAMECOLOR_FUNC + 10,
			NAMECOLOR_NUM = NAMECOLOR_EXTRA + 10,
		};

		enum EAlignmentType
		{
			ALIGNMENT_TYPE_WHITE,
			ALIGNMENT_TYPE_NORMAL,
			ALIGNMENT_TYPE_DARK,
		};

		enum EEmoticon
		{
			EMOTICON_EXCLAMATION	= 1,
			EMOTICON_FISH			= 11,
			EMOTICON_NUM			= 128,
			TITLE_NUM				= 9,
			TITLE_NONE				= 4,
		};

		enum ERefinedEffect
		{
			EFFECT_REFINED_NONE,

			EFFECT_SWORD_REFINED7,
			EFFECT_SWORD_REFINED8,
			EFFECT_SWORD_REFINED9,

			EFFECT_BOW_REFINED7,
			EFFECT_BOW_REFINED8,
			EFFECT_BOW_REFINED9,

			EFFECT_FANBELL_REFINED7,
			EFFECT_FANBELL_REFINED8,
			EFFECT_FANBELL_REFINED9,

			EFFECT_SMALLSWORD_REFINED7,
			EFFECT_SMALLSWORD_REFINED8,
			EFFECT_SMALLSWORD_REFINED9,

			EFFECT_SMALLSWORD_REFINED7_LEFT,
			EFFECT_SMALLSWORD_REFINED8_LEFT,
			EFFECT_SMALLSWORD_REFINED9_LEFT,

			EFFECT_BODYARMOR_REFINED7,
			EFFECT_BODYARMOR_REFINED8,
			EFFECT_BODYARMOR_REFINED9,

			EFFECT_BODYARMOR_SPECIAL,	// 4-2-1
			EFFECT_BODYARMOR_SPECIAL2,	// 4-2-2

#ifdef ENABLE_LVL115_ARMOR_EFFECT
			EFFECT_BODYARMOR_SPECIAL3,	// 5-1
#endif

#ifdef ENABLE_ACCE_SYSTEM
			EFFECT_ACCE,
#endif

			EFFECT_REFINED_NUM,
		};

		enum DamageFlag
		{
			DAMAGE_NORMAL	= (1 << 0),
			DAMAGE_POISON	= (1 << 1),
			DAMAGE_DODGE	= (1 << 2),
			DAMAGE_BLOCK	= (1 << 3),
			DAMAGE_PENETRATE= (1 << 4),
			DAMAGE_CRITICAL = (1 << 5),
		};

		enum EEffect
		{
			EFFECT_DUST,
			EFFECT_STUN,
			EFFECT_HIT,
			EFFECT_FLAME_ATTACK,
			EFFECT_FLAME_HIT,
			EFFECT_FLAME_ATTACH,
			EFFECT_ELECTRIC_ATTACK,
			EFFECT_ELECTRIC_HIT,
			EFFECT_ELECTRIC_ATTACH,
			EFFECT_SPAWN_APPEAR,
			EFFECT_SPAWN_DISAPPEAR,
			EFFECT_LEVELUP,
			EFFECT_SKILLUP,
			EFFECT_HPUP_RED,
			EFFECT_SPUP_BLUE,
			EFFECT_SPEEDUP_GREEN,
			EFFECT_DXUP_PURPLE,
			EFFECT_CRITICAL,
			EFFECT_PENETRATE,
			EFFECT_BLOCK,
			EFFECT_DODGE,
			EFFECT_FIRECRACKER,
			EFFECT_SPIN_TOP,
			EFFECT_WEAPON,
			EFFECT_WEAPON_END = EFFECT_WEAPON + WEAPON_NUM,
			EFFECT_AFFECT,
			EFFECT_AFFECT_GYEONGGONG = EFFECT_AFFECT + AFFECT_GYEONGGONG,
			EFFECT_AFFECT_KWAESOK = EFFECT_AFFECT + AFFECT_KWAESOK,
			EFFECT_AFFECT_END = EFFECT_AFFECT + AFFECT_NUM,
			EFFECT_EMOTICON,
			EFFECT_EMOTICON_END = EFFECT_EMOTICON + EMOTICON_NUM,
			EFFECT_SELECT,
			EFFECT_TARGET,
			EFFECT_EMPIRE,
			EFFECT_EMPIRE_END = EFFECT_EMPIRE + EMPIRE_NUM,
			EFFECT_HORSE_DUST,
			EFFECT_REFINED,
			EFFECT_REFINED_END = EFFECT_REFINED + EFFECT_REFINED_NUM,
			EFFECT_DAMAGE_TARGET,
			EFFECT_DAMAGE_NOT_TARGET,
			EFFECT_DAMAGE_SELFDAMAGE,
			EFFECT_DAMAGE_SELFDAMAGE2,
			EFFECT_DAMAGE_POISON,
			EFFECT_DAMAGE_MISS,
			EFFECT_DAMAGE_TARGETMISS,
			EFFECT_DAMAGE_CRITICAL,
			EFFECT_SUCCESS,
			EFFECT_FAIL,
			EFFECT_FR_SUCCESS,
			EFFECT_LEVELUP_ON_14_FOR_GERMANY,	//레벨업 14일때 ( 독일전용 )
			EFFECT_LEVELUP_UNDER_15_FOR_GERMANY,//레벨업 15일때 ( 독일전용 )
			EFFECT_PERCENT_DAMAGE1,
			EFFECT_PERCENT_DAMAGE2,
			EFFECT_PERCENT_DAMAGE3,
			EFFECT_AUTO_HPUP,
			EFFECT_AUTO_SPUP,
			EFFECT_RAMADAN_RING_EQUIP,			// 초승달 반지 착용 순간에 발동하는 이펙트
			EFFECT_HALLOWEEN_CANDY_EQUIP,		// 할로윈 사탕 착용 순간에 발동하는 이펙트
			EFFECT_HAPPINESS_RING_EQUIP,				// 행복의 반지 착용 순간에 발동하는 이펙트
			EFFECT_LOVE_PENDANT_EQUIP,				// 행복의 반지 착용 순간에 발동하는 이펙트
			EFFECT_TEMP,
#ifdef ENABLE_ACCE_SYSTEM
			EFFECT_ACCE_SUCCEDED,
			EFFECT_ACCE_EQUIP,
#endif
#ifdef ENABLE_TEMPLE_OCHAO
			EFFECT_HEALER,
#endif
#ifdef ENABLE_BOSS_EFFECT
			EFEKT_BOSS,
#endif
#ifdef ENABLE_GEM_SYSTEM
			EFFECT_GEM_PENDANT,
#endif
#ifdef ENABLE_12ZI
			EFFECT_SKILL_DAMAGE_ZONE,
			EFFECT_SKILL_SAFE_ZONE,
			EFFECT_METEOR,
			EFFECT_BEAD_RAIN,
			EFFECT_FALL_ROCK,
			EFFECT_ARROW_RAIN,
			EFFECT_HORSE_DROP,
			EFFECT_EGG_DROP,
			EFFECT_DEAPO_BOOM,
#endif
#ifdef ENABLE_BATTLE_FIELD
			EFFECT_BATTLE_POTION,
#endif
			EFFECT_NUM,
		};

		enum EDuel
		{
			DUEL_NONE,
			DUEL_CANNOTATTACK,
			DUEL_START,
		};

	public:
		static void DestroySystem();
		static void CreateSystem(UINT uCapacity);
		static bool RegisterEffect(UINT eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache);
		static void RegisterTitleName(int iIndex, const char * c_szTitleName);
		static bool RegisterNameColor(UINT uIndex, UINT r, UINT g, UINT b);
		static bool RegisterTitleColor(UINT uIndex, UINT r, UINT g, UINT b);
		static bool ChangeEffectTexture(UINT eEftType, const char* c_szSrcFileName, const char* c_szDstFileName);

		static void SetDustGap(float fDustGap);
		static void SetHorseDustGap(float fDustGap);

		static void SetEmpireNameMode(bool isEnable);
		static const D3DXCOLOR& GetIndexedNameColor(UINT eNameColor);

	public:
		void SetMainInstance();

		void OnSelected();
		void OnUnselected();
		void OnTargeted();
		void OnUntargeted();

	protected:
		bool __IsExistMainInstance();
		bool __IsMainInstance();
		bool __MainCanSeeHiddenThing();
		float __GetBowRange();

	protected:
		DWORD	__AttachEffect(UINT eEftType);
		DWORD	__AttachEffect(char filename[128]);
		void	__DetachEffect(DWORD dwEID);

	public:
		void CreateSpecialEffect(DWORD iEffectIndex);
		void AttachSpecialEffect(DWORD effect);

	protected:
		static std::string ms_astAffectEffectAttachBone[EFFECT_NUM];
		static DWORD ms_adwCRCAffectEffect[EFFECT_NUM];
		static float ms_fDustGap;
		static float ms_fHorseDustGap;

	public:
		CInstanceBase();
		virtual ~CInstanceBase();

		bool LessRenderOrder(CInstanceBase* pkInst);

		void MountHorse(UINT eRace);
		void DismountHorse();

		// 스크립트용 테스트 함수. 나중에 없에자
		void SCRIPT_SetAffect(UINT eAffect, bool isVisible);

		float CalculateDistanceSq3d(const TPixelPosition& c_rkPPosDst);

		// Instance Data
		bool IsFlyTargetObject();
		void ClearFlyTargetInstance();
		void SetFlyTargetInstance(CInstanceBase& rkInstDst);
		void AddFlyTargetInstance(CInstanceBase& rkInstDst);
		void AddFlyTargetPosition(const TPixelPosition& c_rkPPosDst);

		float GetFlyTargetDistance();

		void SetAlpha(float fAlpha);

		void DeleteBlendOut();

		void					AttachTextTail();
		void					DetachTextTail();
		void					UpdateTextTailLevel(DWORD level);

		void					RefreshTextTail();
		void					RefreshTextTailTitle();

		bool					Create(const SCreateData& c_rkCreateData);

		bool					CreateDeviceObjects();
		void					DestroyDeviceObjects();

		void					Destroy();

		void					Update();
		bool					UpdateDeleting();

		void					Transform();
		void					Deform();
		void					Render();
		void					RenderTrace();
		void					RenderToShadowMap();
		void					RenderCollision();
		void					RegisterBoundingSphere();

		// Temporary
		void					GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax);

		void					SetNameString(const char* c_szName, int len);
		bool					SetRace(DWORD dwRaceIndex);
		void					SetVirtualID(DWORD wVirtualNumber);
		void					SetVirtualNumber(DWORD dwVirtualNumber);
		void					SetInstanceType(int iInstanceType);
		void					SetAlignment(short sAlignment);
		void					SetLevelText(int mLevel);
		void					SetPKMode(BYTE byPKMode);
		void					SetKiller(bool bFlag);
		void					SetPartyMemberFlag(bool bFlag);
		void					SetStateFlags(DWORD dwStateFlags);

		void					SetArmor(DWORD dwArmor);
		void					SetShape(DWORD eShape, float fSpecular=0.0f);
		void					SetHair(DWORD eHair);
#ifdef ENABLE_QUIVER_SYSTEM
		bool					SetWeapon(DWORD eWeapon, DWORD eArrow = 0);
#else
		bool					SetWeapon(DWORD eWeapon);
#endif
		bool					ChangeArmor(DWORD dwArmor);
#ifdef ENABLE_QUIVER_SYSTEM
		void					ChangeWeapon(DWORD eWeapon, DWORD eArrow = 0);
#else
		void					ChangeWeapon(DWORD eWeapon);
#endif
		void					ChangeHair(DWORD eHair);
		void					ChangeGuild(DWORD dwGuildID
#ifdef ENABLE_GUILD_GENERAL_AND_LEADER
, DWORD dwNewIsGuildName
#endif
);

		DWORD					GetWeaponType();

#ifdef ENABLE_ACCE_SYSTEM
		void					SetAcce(DWORD dwAcce);
		void					ChangeAcce(DWORD dwAcce);
#endif

#ifdef ENABLE_COSTUME_EFFECT
		void					SetSpecialEffect(DWORD dwArmorEffect = 0, DWORD dwWeaponEffectRight = 0, DWORD dwWeaponEffectLeft = 0);
		void					ClearSpecialEffect(bool isWeapon = false);
		void					SpecialEffectCheck(bool isWeapon = false);
#endif

#ifdef ENABLE_ELEMENT_ADD
		int						GetElement() { return m_bElement; }
		void					SetElement(BYTE bElement) { m_bElement = bElement; }
#endif

#ifdef WJ_SHOW_MOB_INFO
		DWORD					GetAIFlag();
#endif

		void					SetComboType(UINT uComboType);
		void					SetAttackSpeed(UINT uAtkSpd);
		void					SetMoveSpeed(UINT uMovSpd);
		void					SetRotationSpeed(float fRotSpd);

		const char *			GetNameString();
// #ifdef ENABLE_LEVEL_IN_TRADE
		DWORD					GetLevel();
// #endif
		int						GetInstanceType();
		DWORD					GetPart(CRaceData::EParts part);
		DWORD					GetShape();
		DWORD					GetRace();
		DWORD					GetVirtualID();
		DWORD					GetVirtualNumber();
		DWORD					GetEmpireID();
		DWORD					GetGuildID();
#ifdef ENABLE_GUILD_GENERAL_AND_LEADER
		BYTE					GetNewIsGuildName();
#endif
		int						GetAlignment();
		UINT					GetAlignmentGrade();
		int						GetAlignmentType();
		BYTE					GetPKMode();
		bool					IsKiller();
		bool					IsPartyMember();
#ifdef ENABLE_DEFENSE_WAVE
		bool					IsDefenseWaveMap();
#endif
#ifdef ENABLE_BATTLE_FIELD
		bool 					IsBattleFieldMap();
		void					SetBattleFieldRank(BYTE bRank) { m_bBattleFieldRank = bRank; }
		BYTE					GetBattleFieldRank() const{ return m_bBattleFieldRank; }
#endif
		void					ActDualEmotion(CInstanceBase & rkDstInst, WORD dwMotionNumber1, WORD dwMotionNumber2);
		void					ActEmotion(DWORD dwMotionNumber);
		void					LevelUp();
		void					SkillUp();
		void					UseSpinTop();
		void					Revive();
		void					Stun();
		void					Die();
		void					Hide();
		void					Show();

		bool					CanAct();
		bool					CanMove();
		bool					CanAttack();
		bool					CanUseSkill();
		bool					CanFishing();
		bool					IsConflictAlignmentInstance(CInstanceBase& rkInstVictim);
		bool					IsAttackableInstance(CInstanceBase& rkInstVictim);
		bool					IsTargetableInstance(CInstanceBase& rkInstVictim);
		bool					IsPVPInstance(CInstanceBase& rkInstVictim);
		bool					CanChangeTarget();
		bool					CanPickInstance();
		bool					CanViewTargetHP(CInstanceBase& rkInstVictim);
		// Movement
		BOOL					IsGoing();
		bool					NEW_Goto(const TPixelPosition& c_rkPPosDst, float fDstRot);
		void					EndGoing();

		void					SetRunMode();
		void					SetWalkMode();

		bool					IsAffect(UINT uAffect);
		BOOL					IsInvisibility();
		BOOL					IsParalysis();
		BOOL					IsGameMaster();
		BOOL					IsSameEmpire(CInstanceBase& rkInstDst);
		BOOL					IsBowMode();
		BOOL					IsHandMode();
		BOOL					IsFishingMode();
		BOOL					IsFishing();

		BOOL					IsWearingDress();
		BOOL					IsHoldingPickAxe();
		BOOL					IsMountingHorse();
		BOOL					IsNewMount();
		BOOL					IsForceVisible();
		BOOL					IsInSafe();
#ifdef ENABLE_GROWTH_PET_SYSTEM
		BOOL					IsNewPet();
#endif
#ifdef ENABLE_SUPPORT_SYSTEM
		BOOL					IsSupport();
#endif
		BOOL					IsPrivateShop();
		BOOL					IsPet();
		BOOL					IsEnemy();
		BOOL					IsStone();
		BOOL					IsResource();
		BOOL					IsNPC();
		BOOL					IsPC();
		BOOL					IsPoly();
		BOOL					IsWarp();
		BOOL					IsGoto();
		BOOL					IsObject();
		BOOL					IsDoor();
		BOOL					IsBuilding();
		BOOL					IsWoodenDoor();
		BOOL					IsStoneDoor();
		BOOL					IsFlag();
		BOOL					IsGuildWall();

		BOOL					IsDead();
		BOOL					IsStun();
		BOOL					IsSleep();
		BOOL					__IsSyncing();
		BOOL					IsWaiting();
		BOOL					IsWalking();
		BOOL					IsPushing();
		BOOL					IsAttacking();
		BOOL					IsActingEmotion();
		BOOL					IsAttacked();
		BOOL					IsKnockDown();
		BOOL					IsUsingSkill();
		BOOL					IsUsingMovingSkill();
		BOOL					CanCancelSkill();
		BOOL					CanAttackHorseLevel();
#ifdef __MOVIE_MODE__
		BOOL					IsMovieMode(); // 운영자용 완전히 안보이는거
#endif
		bool					NEW_CanMoveToDestPixelPosition(const TPixelPosition& c_rkPPosDst);

		void					NEW_SetAdvancingRotationFromPixelPosition(const TPixelPosition& c_rkPPosSrc, const TPixelPosition& c_rkPPosDst);
		void					NEW_SetAdvancingRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		bool					NEW_SetAdvancingRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		void					SetAdvancingRotation(float fRotation);

		void					EndWalking(float fBlendingTime=0.15f);
		void					EndWalkingWithoutBlending();

		// Battle
		void					SetEventHandler(CActorInstance::IEventHandler* pkEventHandler);

		void					PushUDPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg);
		void					PushTCPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg);
		void					PushTCPStateExpanded(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg, UINT uTargetVID);

		void					NEW_Stop();

		bool					NEW_UseSkill(UINT uSkill, UINT uMot, UINT uMotLoopCount, bool isMovingSkill);
		void					NEW_Attack();
		void					NEW_Attack(float fDirRot);
		void					NEW_AttackToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
		bool					NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst, IFlyEventHandler* pkFlyHandler);
		bool					NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst);

		bool					NEW_MoveToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
		void					NEW_MoveToDestInstanceDirection(CInstanceBase& rkInstDst);
		void					NEW_MoveToDirection(float fDirRot);

		float					NEW_GetDistanceFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		float					NEW_GetDistanceFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		float					NEW_GetDistanceFromDestInstance(CInstanceBase& rkInstDst);

		float					NEW_GetRotation();
		float					NEW_GetRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		float					NEW_GetRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		float					NEW_GetRotationFromDestInstance(CInstanceBase& rkInstDst);

		float					NEW_GetAdvancingRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		float					NEW_GetAdvancingRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		float					NEW_GetAdvancingRotationFromPixelPosition(const TPixelPosition& c_rkPPosSrc, const TPixelPosition& c_rkPPosDst);

		BOOL					NEW_IsClickableDistanceDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		BOOL					NEW_IsClickableDistanceDestInstance(CInstanceBase& rkInstDst);

		bool					NEW_GetFrontInstance(CInstanceBase ** ppoutTargetInstance, float fDistance);
		void					NEW_GetRandomPositionInFanRange(CInstanceBase& rkInstTarget, TPixelPosition* pkPPosDst);
		bool					NEW_GetInstanceVectorInFanRange(float fSkillDistance, CInstanceBase& rkInstTarget, std::vector<CInstanceBase*>* pkVct_pkInst);
		bool					NEW_GetInstanceVectorInCircleRange(float fSkillDistance, std::vector<CInstanceBase*>* pkVct_pkInst);

		void					NEW_SetOwner(DWORD dwOwnerVID);
		void					NEW_SyncPixelPosition(long & nPPosX, long & nPPosY);
		void					NEW_SyncCurrentPixelPosition();

		void					NEW_SetPixelPosition(const TPixelPosition& c_rkPPosDst);

		bool					NEW_IsLastPixelPosition();
		const TPixelPosition&	NEW_GetLastPixelPositionRef();
		// Battle
		BOOL					isNormalAttacking();
		BOOL					isComboAttacking();
		MOTION_KEY				GetNormalAttackIndex();
		DWORD					GetComboIndex();
		float					GetAttackingElapsedTime();
		void					InputNormalAttack(float fAtkDirRot);
		void					InputComboAttack(float fAtkDirRot);

		void					RunNormalAttack(float fAtkDirRot);
		void					RunComboAttack(float fAtkDirRot, DWORD wMotionIndex);

		CInstanceBase*			FindNearestVictim();
		BOOL					CheckAdvancing();
		bool					AvoidObject(const CGraphicObjectInstance& c_rkBGObj);
		bool					IsBlockObject(const CGraphicObjectInstance& c_rkBGObj);
		void					BlockMovement();

	public:
		BOOL					CheckAttacking(CInstanceBase& rkInstVictim);
		void					ProcessHitting(DWORD dwMotionKey, CInstanceBase * pVictimInstance);
		void					ProcessHitting(DWORD dwMotionKey, BYTE byEventIndex, CInstanceBase * pVictimInstance);
		void					GetBlendingPosition(TPixelPosition * pPixelPosition);
		void					SetBlendingPosition(const TPixelPosition & c_rPixelPosition);

		// Fishing
		void					StartFishing(float frot);
		void					StopFishing();
		void					ReactFishing();
		void					CatchSuccess();
		void					CatchFail();
		BOOL					GetFishingRot(int * pirot);

		// Render Mode
		void					RestoreRenderMode();
		void					SetAddRenderMode();
		void					SetModulateRenderMode();
		void					SetRenderMode(int iRenderMode);
		void					SetAddColor(const D3DXCOLOR & c_rColor);

		// Position
		void					SCRIPT_SetPixelPosition(float fx, float fy);
		void					NEW_GetPixelPosition(TPixelPosition * pPixelPosition);

		// Rotation
		void					NEW_LookAtFlyTarget();
		void					NEW_LookAtDestInstance(CInstanceBase& rkInstDst);
		void					NEW_LookAtDestPixelPosition(const TPixelPosition& c_rkPPosDst);

		float					GetRotation();
		float					GetAdvancingRotation();
		void					SetRotation(float fRotation);
		void					BlendRotation(float fRotation, float fBlendTime = 0.1f);

		void					SetDirection(int dir);
		void					BlendDirection(int dir, float blendTime);
		float					GetDegreeFromDirection(int dir);

		// Motion
		//	Motion Deque
		BOOL					isLock();

		void					SetMotionMode(int iMotionMode);
		int						GetMotionMode(DWORD dwMotionIndex);

		// Motion
		//	Pushing Motion
		void					ResetLocalTime();
		void					SetLoopMotion(WORD wMotion, float fBlendTime=0.1f, float fSpeedRatio=1.0f);
		void					PushOnceMotion(WORD wMotion, float fBlendTime=0.1f, float fSpeedRatio=1.0f);
		void					PushLoopMotion(WORD wMotion, float fBlendTime=0.1f, float fSpeedRatio=1.0f);
		void					SetEndStopMotion();

		// Intersect
		bool					IntersectDefendingSphere();
		bool					IntersectBoundingBox();

		// Part
		//void					SetParts(const WORD * c_pParts);
		void					Refresh(DWORD dwMotIndex, bool isLoop);

		//void					AttachEffectByID(DWORD dwParentPartIndex, const char * c_pszBoneName, DWORD dwEffectID, int dwLife = CActorInstance::EFFECT_LIFE_INFINITE ); // 수명은 ms단위입니다.
		//void					AttachEffectByName(DWORD dwParentPartIndex, const char * c_pszBoneName, const char * c_pszEffectName, int dwLife = CActorInstance::EFFECT_LIFE_INFINITE ); // 수명은 ms단위입니다.

		float					GetDistance(CInstanceBase * pkTargetInst);
		float					GetDistance(const TPixelPosition & c_rPixelPosition);

		// ETC
		CActorInstance&			GetGraphicThingInstanceRef();
		CActorInstance*			GetGraphicThingInstancePtr();

		bool __Background_IsWaterPixelPosition(const TPixelPosition& c_rkPPos);
		bool __Background_GetWaterHeight(const TPixelPosition& c_rkPPos, float* pfHeight);

		// 2004.07.25.myevan.이펙트 안나오는 문제
		/////////////////////////////////////////////////////////////
		void __ClearAffectFlagContainer();
		void __ClearAffects();
		/////////////////////////////////////////////////////////////

		void __SetAffect(UINT eAffect, bool isVisible);

		void SetAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);

		void __SetNormalAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);
		void __SetStoneSmokeFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);

		void SetEmoticon(UINT eEmoticon);
		void SetFishEmoticon();
		bool IsPossibleEmoticon();

	protected:
		UINT					__LessRenderOrder_GetLODLevel();
		void					__Initialize();
		void					__InitializeRotationSpeed();

		void					__Create_SetName(const SCreateData& c_rkCreateData);
		void					__Create_SetWarpName(const SCreateData& c_rkCreateData);

		CInstanceBase*			__GetMainInstancePtr();
		CInstanceBase*			__FindInstancePtr(DWORD dwVID);

		bool  __FindRaceType(DWORD dwRace, BYTE* pbType);
		DWORD __GetRaceType();

		bool __IsShapeAnimalWear();
		BOOL __IsChangableWeapon(int iWeaponID);

		void __EnableSkipCollision();
		void __DisableSkipCollision();

		void __ClearMainInstance();

		void __Shaman_SetParalysis(bool isParalysis);
		void __Warrior_SetGeomgyeongAffect(bool isVisible);
		void __Assassin_SetEunhyeongAffect(bool isVisible);
		void __SetReviveInvisibilityAffect(bool isVisible);

		BOOL __CanProcessNetworkStatePacket();

		bool __IsInDustRange();

		// Emotion
		void __ProcessFunctionEmotion(DWORD dwMotionNumber, DWORD dwTargetVID, const TPixelPosition & c_rkPosDst);
		void __EnableChangingTCPState();
		void __DisableChangingTCPState();
		BOOL __IsEnableTCPProcess(UINT eCurFunc);

		// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
		bool __CanRender();
		bool __IsInViewFrustum();

		// HORSE
		void __AttachHorseSaddle();
		void __DetachHorseSaddle();

		struct SHORSE
		{
			bool m_isMounting;
			CActorInstance* m_pkActor;

			SHORSE();
			~SHORSE();

			void Destroy();
			void Create(const TPixelPosition& c_rkPPos, UINT eRace, UINT eHitEffect);

			void SetAttackSpeed(UINT uAtkSpd);
			void SetMoveSpeed(UINT uMovSpd);
			void Deform();
			void Render();
			CActorInstance& GetActorRef();
			CActorInstance* GetActorPtr();

			bool IsMounting();
			bool CanAttack();
			bool CanUseSkill();

			UINT GetLevel();
			bool IsNewMount();

			void __Initialize();
		} m_kHorse;
	protected:
		// Blend Mode
		void					__SetBlendRenderingMode();
		void					__SetAlphaValue(float fAlpha);
		float					__GetAlphaValue();

		void					__ComboProcess();
		void					MovementProcess();
		void					TodoProcess();
		void					StateProcess();
		void					AttackProcess();

		void					StartWalking();
		float					GetLocalTime();

		void					RefreshState(DWORD dwMotIndex, bool isLoop);
		void					RefreshActorInstance();

	protected:
		void					OnSyncing();
		void					OnWaiting();
		void					OnMoving();

		void					NEW_SetCurPixelPosition(const TPixelPosition& c_rkPPosDst);
		void					NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosDst);
		void					NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst);
		void					NEW_SetDstPixelPositionZ(FLOAT z);

		const TPixelPosition&	NEW_GetCurPixelPositionRef();
		const TPixelPosition&	NEW_GetSrcPixelPositionRef();

	public:
		const TPixelPosition&	NEW_GetDstPixelPositionRef();

	protected:
		BOOL m_isTextTail;

		// Instance Data
		std::string				m_stName;

		DWORD					m_awPart[CRaceData::PART_MAX_NUM];

		DWORD					m_dwLevel;
		DWORD					m_dwEmpireID;
		DWORD					m_dwGuildID;
#ifdef ENABLE_GUILD_GENERAL_AND_LEADER
		BYTE					m_dwNewIsGuildName;
#endif
#ifdef ENABLE_BATTLE_FIELD
		BYTE					m_bBattleFieldRank;
#endif
#ifdef ENABLE_MULTI_LANGUAGE
	public:
		const char*				GetLanguage() { return m_stLanguage.c_str(); }
		void					SetLanguage(const char* szLanguage) { m_stLanguage = szLanguage; }

	protected:
		std::string				m_stLanguage;
#endif
	protected:
		CAffectFlagContainer	m_kAffectFlagContainer;
		DWORD					m_adwCRCAffectEffect[AFFECT_NUM];

		UINT	__GetRefinedEffect(CItemData* pItem);
		void	__ClearWeaponRefineEffect();
		void	__ClearArmorRefineEffect();

#ifdef ENABLE_ACCE_SYSTEM
		void	ClearAcceEffect();
#endif

	protected:
		void __AttachSelectEffect();
		void __DetachSelectEffect();

		void __AttachTargetEffect();
		void __DetachTargetEffect();

		void __AttachEmpireEffect(DWORD eEmpire);
#ifdef ENABLE_BOSS_EFFECT
		void __AttachEfektBoss();
#endif
	protected:
		struct SEffectContainer
		{
			typedef std::map<DWORD, DWORD> Dict;
			Dict m_kDct_dwEftID;
		} m_kEffectContainer;

		void __EffectContainer_Initialize();
		void __EffectContainer_Destroy();

		DWORD __EffectContainer_AttachEffect(DWORD eEffect);
		void __EffectContainer_DetachEffect(DWORD eEffect);

		SEffectContainer::Dict& __EffectContainer_GetDict();

	protected:
		struct SStoneSmoke
		{
			DWORD m_dwEftID;
		} m_kStoneSmoke;

		void __StoneSmoke_Inialize();
		void __StoneSmoke_Destroy();
		void __StoneSmoke_Create(DWORD eSmoke);
	protected:
		// Emoticon
		//DWORD					m_adwCRCEmoticonEffect[EMOTICON_NUM];

		BYTE					m_eType;
		BYTE					m_eRaceType;
		DWORD					m_eShape;
		DWORD					m_dwRace;
		DWORD					m_dwVirtualNumber;
		short					m_sAlignment;
		BYTE					m_byPKMode;
		bool					m_isKiller;
		bool					m_isPartyMember;
		// Movement
		int						m_iRotatingDirection;

		DWORD					m_dwAdvActorVID;
		DWORD					m_dwLastDmgActorVID;

		LONG					m_nAverageNetworkGap;
		DWORD					m_dwNextUpdateHeightTime;

		bool					m_isGoing;

		TPixelPosition			m_kPPosDust;

		DWORD					m_dwLastComboIndex;

		DWORD					m_swordRefineEffectRight;
		DWORD					m_swordRefineEffectLeft;
		DWORD					m_armorRefineEffect;
#ifdef ENABLE_ACCE_SYSTEM
		DWORD					m_dwAcceEffect;
#endif
#ifdef ENABLE_COSTUME_EFFECT
		DWORD					m_costumeWeaponEffectRight;
		DWORD					m_costumeWeaponEffectLeft;
		DWORD					m_costumeBodyEffect;
#endif
#ifdef ENABLE_ELEMENT_ADD
		BYTE					m_bElement;
#endif
		struct SMoveAfterFunc
		{
			UINT eFunc;
			UINT uArg;

			// For Emotion Function
			UINT uArgExpanded;
			TPixelPosition kPosDst;
		};

		SMoveAfterFunc m_kMovAfterFunc;

		float m_fDstRot;
		float m_fAtkPosTime;
		float m_fRotSpd;
		float m_fMaxRotSpd;

		BOOL m_bEnableTCPState;

		// Graphic Instance
		CActorInstance m_GraphicThingInstance;
	protected:
		struct SCommand
		{
			DWORD	m_dwChkTime;
			DWORD	m_dwCmdTime;
			float	m_fDstRot;
			UINT 	m_eFunc;
			UINT 	m_uArg;
			UINT	m_uTargetVID;
			TPixelPosition m_kPPosDst;
		};

		typedef std::list<SCommand> CommandQueue;

		DWORD		m_dwBaseChkTime;
		DWORD		m_dwBaseCmdTime;

		DWORD		m_dwSkipTime;

		CommandQueue m_kQue_kCmdNew;

		BOOL		m_bDamageEffectType;

		struct SEffectDamage
		{
			DWORD damage;
			BYTE flag;
			BOOL bSelf;
			BOOL bTarget;
		};

		typedef std::list<SEffectDamage> CommandDamageQueue;
		CommandDamageQueue m_DamageQueue;

		void ProcessDamage();

	public:
		void AddDamageEffect(DWORD damage,BYTE flag,BOOL bSelf,BOOL bTarget);

	protected:
		struct SWarrior
		{
			DWORD m_dwGeomgyeongEffect;
		};

		SWarrior m_kWarrior;

		void __Warrior_Initialize();

	public:
		static void ClearPVPKeySystem();

		static void InsertPVPKey(DWORD dwSrcVID, DWORD dwDstVID);
		static void InsertPVPReadyKey(DWORD dwSrcVID, DWORD dwDstVID);
		static void RemovePVPKey(DWORD dwSrcVID, DWORD dwDstVID);

		static void InsertGVGKey(DWORD dwSrcGuildVID, DWORD dwDstGuildVID);
		static void RemoveGVGKey(DWORD dwSrcGuildVID, DWORD dwDstGuildVID);

		static void InsertDUELKey(DWORD dwSrcVID, DWORD dwDstVID);

		UINT GetNameColorIndex();

		const D3DXCOLOR& GetNameColor();
		const D3DXCOLOR& GetTitleColor();

	protected:
		static DWORD __GetPVPKey(DWORD dwSrcVID, DWORD dwDstVID);
		static bool __FindPVPKey(DWORD dwSrcVID, DWORD dwDstVID);
		static bool __FindPVPReadyKey(DWORD dwSrcVID, DWORD dwDstVID);
		static bool __FindGVGKey(DWORD dwSrcGuildID, DWORD dwDstGuildID);
		static bool __FindDUELKey(DWORD dwSrcGuildID, DWORD dwDstGuildID);

	protected:
		CActorInstance::IEventHandler* GetEventHandlerPtr();
		CActorInstance::IEventHandler& GetEventHandlerRef();

	protected:
		static float __GetBackgroundHeight(float x, float y);
		static DWORD __GetShadowMapColor(float x, float y);

	public:
		static void ResetPerformanceCounter();
		static void GetInfo(std::string* pstInfo);

	public:
		static CInstanceBase* New();
		static void Delete(CInstanceBase* pkInst);

		static CDynamicPool<CInstanceBase>	ms_kPool;

	protected:
		static DWORD ms_dwUpdateCounter;
		static DWORD ms_dwRenderCounter;
		static DWORD ms_dwDeformCounter;

	public:
		DWORD					GetDuelMode();
		void					SetDuelMode(DWORD type);
	protected:
		DWORD					m_dwDuelMode;
		DWORD					m_dwEmoticonTime;

#ifdef ENABLE_RENDER_TARGET
	public:
		inline void DisableTextTail() { m_textTailDisable = true; }
		void EnableAlwaysRender();
		void DisableAlwaysRender();

	protected:
		bool m_textTailDisable = false;
		bool m_isAlwaysRender;
#endif
};

inline int RaceToJob(int race)
{
#ifdef ENABLE_WOLFMAN_CHARACTER
	if (race==8)
		return 4;
#endif
	const int JOB_NUM = 4;
	return race % JOB_NUM;
}

inline int RaceToSex(int race)
{
	switch (race)
	{
		case 0:
		case 2:
		case 5:
		case 7:
#ifdef ENABLE_WOLFMAN_CHARACTER
		case 8:
#endif
			return 1;
		case 1:
		case 3:
		case 4:
		case 6:
			return 0;

	}
	return 0;
}
