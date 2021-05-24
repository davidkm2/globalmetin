#ifndef __INC_METIN_II_BATTLE_FIELD_H__
#define __INC_METIN_II_BATTLE_FIELD_H__

#include <unordered_map>

#include "../../common/stl.h"
#include "../../common/length.h"
#include "../../common/tables.h"
#include "packet.h"
#include <lua.h>

enum
{
	BATTLE_FIELD_MAP_INDEX = 357,	// Map index
	MIN_LEVEL_FOR_ENTER = 50,
	TIME_BETWEEN_KILLS = 300, 		// In seconds
};

enum
{
	MONDAY = 1,
	TUESDAY = 2,
	WEDNESDAY = 3,
	THURSDAY = 4,
	FRIDAY = 5,
	SATURDAY = 6,
	SUNDAY = 0,
};

class CBattleField : public singleton<CBattleField>
{
/*
* Standard functions
*/
	public:
		CBattleField ();
		virtual ~CBattleField ();

		bool Initialize();
		void Destroy();
		
		bool IsBattleFieldMapIndex(int iMapIndex) const { return iMapIndex == BATTLE_FIELD_MAP_INDEX; };
		
		void PlayerKill(LPCHARACTER pChar, LPCHARACTER pKiller);
		void RewardKiller(LPCHARACTER pKiller, LPCHARACTER pChar);
		void RewardVictim(LPCHARACTER pChar);
		
		void Connect(LPCHARACTER pChar);
		void RestartAtRandomPos(LPCHARACTER pChar);
		PIXEL_POSITION GetRandomPosition();
		
		void RemovePlayerFromVector(DWORD dwPlayerId);
		void TargetOnLeave(LPCHARACTER pChar, bool isDelete);
		bool IsInLeaveProcess(LPCHARACTER pkKiller, LPCHARACTER pChar);
		
	private:
		std::vector<DWORD> m_vec_char;

		typedef std::map<DWORD, TBattleFieldKillVictim> TKillMap;
		TKillMap m_playersKills;
/*
* Open time functions
*/		
	public:
		int GetTimeDiff(bool isClose);
		void GetOpenTimeList(lua_State* L);
		
		void Update();
		
/*
* Enter functions
*/
	public:
		void SetEnterStatus(bool bFlag) { bEnterStatus = bFlag; };
		bool GetEnterStatus() const { return bEnterStatus; };
		
		void SetEventStatus(bool bFlag) { bEventStatus = bFlag; };
		bool GetEventStatus() const { return bEventStatus; };
		
		void SetEventInfo(BYTE bMonth, BYTE bDay) { bEventMonth = bMonth; bEventDay = bDay; };
		bool GetEventMonth() const { return bEventMonth; };
		bool GetEventDay() const { return bEventDay; };
		
		void OpenEnter(bool isEvent = false);
		void CloseEnter(bool isGm = false);
		
		void RequestEnter(LPCHARACTER pChar);
		void RequestExit(LPCHARACTER pChar);
		
	private:
		bool bEnterStatus, bEventStatus;
		BYTE bEventMonth, bEventDay;
};

#endif

