#include "../../common/defines.h"

#ifdef __GUILD_DRAGONLAIR__
#include "../../common/length.h"
#include "../../common/item_length.h"
#include "../../common/tables.h"
#include "guild.h"
#include "char_manager.h"
#include "sectree_manager.h"

namespace MeleyLair
{
	enum eConfig
	{
		MAP_INDEX = 356,
		SUBMAP_INDEX = 62,
		MIN_LVL = 3,
		PARTECIPANTS_LIMIT = 16,
		LADDER_POINTS_COST = 600,
		LADDER_POINTS_RETURN = 300,
		COOLDOWN_DUNGEON = 10800,
		NPC_VNUM = 20419,
		GATE_VNUM = 20388,
		BOSS_VNUM = 6193,
		STATUE_VNUM = 6118,
		CHEST_VNUM = 20420,
		REWARD_ITEMCHEST_VNUM_1 = 50270,
		REWARD_ITEMCHEST_VNUM_2 = 50271,
		TIME_LIMIT_DUNGEON = 3600,
		SEAL_VNUM_KILL_STATUE = 30341,
		TIME_LIMIT_TO_KILL_STATUE = 10,
		TIME_RESPAWN_COMMON_STEP1 = 5,
		MOBCOUNT_RESPAWN_COMMON_STEP = 12,
		MOBVNUM_RESPAWN_COMMON_STEP1 = 6112,
		TIME_RESPAWN_COMMON_STEP2 = 30,
		MOBVNUM_RESPAWN_COMMON_STEP2 = 6113,
		MOBVNUM_RESPAWN_STONE_STEP2 = 20422,
		MOBCOUNT_RESPAWN_STONE_STEP2 = 4,
		TIME_RESPAWN_COMMON_STEP3 = 30,
		MOBVNUM_RESPAWN_COMMON_STEP3 = 6115,
		MOBVNUM_RESPAWN_BOSS_STEP3 = 6116,
		MOBVNUM_RESPAWN_SUBBOSS_STEP3 = 6117,
		MOBCOUNT_RESPAWN_BOSS_STEP3 = 4,
	};
	
	extern int stoneSpawnPos[MOBCOUNT_RESPAWN_STONE_STEP2][2];
	extern int monsterSpawnPos[MOBCOUNT_RESPAWN_COMMON_STEP][2];
	
	class CMgrMap
	{
		public:
			CMgrMap(long lMapIndex, DWORD dwGuildID);
			~CMgrMap();
			void	Destroy();
			long	GetMapIndex() const	{return map_index;}
			DWORD	GetGuildID() const	{return guild_id;}
			BYTE	GetDungeonStep() const	{return dungeon_step;}
			void	SetDungeonStep(BYTE bStep);
			void	StartDungeonStep(BYTE bStep);
			DWORD	GetDungeonTimeStart() const	{return time_start;}
			void	SetDungeonTimeStart(DWORD dwTime)	{time_start = dwTime;}
			DWORD	GetLastStoneKilledTime() const	{return last_stoneKilled;}
			void	SetLastStoneKilledTime(DWORD dwTime)	{last_stoneKilled = dwTime;}
			DWORD	GetKillCountStones() const	{return kill_stonesCount;}
			void	SetKillCountStones(DWORD dwCount)	{kill_stonesCount = dwCount;}
			DWORD	GetKillCountBosses() const	{return kill_bossesCount;}
			void	SetKillCountBosses(DWORD dwCount)	{kill_bossesCount = dwCount;}
			DWORD	GetRewardTime() const	{return reward;}
			void	SetRewardTime(DWORD dwTime)	{reward = dwTime;}
			DWORD	GetPartecipantsCount() const;
			void	Partecipant(bool bInsert, DWORD dwPlayerID);
			bool	IsPartecipant(DWORD dwPlayerID);
			LPCHARACTER	Spawn(DWORD dwVnum, int iX, int iY, int iDir = 0, bool bSpawnMotion = false);
			void	Start();
			void	StartDungeon(LPCHARACTER pkChar);
			void	EndDungeon(bool bSuccess, bool bGiveBack);
			void	EndDungeonWarp();
			bool	Damage(LPCHARACTER pkStatue);
			void	OnKill(DWORD dwVnum);
			void	OnKillStatue(LPITEM pkItem, LPCHARACTER pkChar, LPCHARACTER pkStatue);
			void	DungeonResult();
			bool	CheckRewarder(DWORD dwPlayerID);
			void	GiveReward(LPCHARACTER pkChar, BYTE bReward);
			LPSECTREE_MAP &	GetMapSectree()	{return pkSectreeMap;}
			LPCHARACTER &	GetBossChar()	{return pkBoss;}
			LPCHARACTER &	GetStatue1Char()	{return pkStatue1;}
			LPCHARACTER &	GetStatue2Char()	{return pkStatue2;}
			LPCHARACTER &	GetStatue3Char()	{return pkStatue3;}
			LPCHARACTER &	GetStatue4Char()	{return pkStatue4;}
		
		private:
			long	map_index;
			DWORD	guild_id, time_start, last_stoneKilled, kill_stonesCount, kill_bossesCount, reward;
			BYTE	dungeon_step;
			std::vector<DWORD>	v_Partecipants, v_Already, v_Rewards;
			LPSECTREE_MAP	pkSectreeMap;
			LPCHARACTER	pkMainNPC, pkGate, pkBoss, pkStatue1, pkStatue2, pkStatue3, pkStatue4;
		
		protected:
			LPEVENT	e_pEndEvent, e_pWarpEvent, e_SpawnEvent, e_SEffectEvent, e_DestroyStatues;
	};
	
	class CMgr : public singleton<CMgr>
	{
		public:
			void	Initialize();
			void	Destroy();
			void	Register(LPCHARACTER pkChar, int & iRes1, int & iRes2);
			bool	isRegistered(CGuild* pkGuild, int & iCH);
			bool	Enter(CGuild* pkGuild, LPCHARACTER pkChar, int & iLimit);
			void	Leave(CGuild* pkGuild, LPCHARACTER pkChar, bool bSendOut);
			void	LeaveRequest(CGuild* pkGuild, LPCHARACTER pkChar);
			bool	IsMeleyMap(long lMapIndex);
			void	Check(CGuild* pkGuild, LPCHARACTER pkChar);
			void	WarpOut(LPCHARACTER pkChar);
			void	SetXYZ(long lX, long lY, long lZ)	{lMapCenterPos.x = lX, lMapCenterPos.y = lY, lMapCenterPos.z = lZ;}
			const PIXEL_POSITION &	GetXYZ() const	{return lMapCenterPos;}
			void	SetSubXYZ(long lX, long lY, long lZ)	{lSubMapPos.x = lX, lSubMapPos.y = lY, lSubMapPos.z = lZ;}
			const PIXEL_POSITION &	GetSubXYZ() const	{return lSubMapPos;}
			void	Start(LPCHARACTER pkChar, CGuild* pkGuild);
			bool	Damage(LPCHARACTER pkStatue, CGuild* pkGuild);
			void	Remove(DWORD dwGuildID);
			void	OnKill(DWORD dwVnum, CGuild* pkGuild);
			void	OnKillStatue(LPITEM pkItem, LPCHARACTER pkChar, LPCHARACTER pkStatue, CGuild* pkGuild);
			void	OnKillCommon(LPCHARACTER pkMonster, LPCHARACTER pkChar, CGuild* pkGuild);
			bool	CanGetReward(LPCHARACTER pkChar, CGuild* pkGuild);
			void	Reward(LPCHARACTER pkChar, CGuild* pkGuild, BYTE bReward);
			void	OpenRanking(LPCHARACTER pkChar);
			void	MemberRemoved(LPCHARACTER pkChar, CGuild* pkGuild);
			void	GuildRemoved(CGuild* pkGuild);
		
		private:
			PIXEL_POSITION	lMapCenterPos, lSubMapPos;
			std::map<DWORD, CMgrMap*> m_RegGuilds;
	};
};
#endif
