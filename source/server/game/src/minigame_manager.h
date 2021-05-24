#pragma once
#include "../../common/length.h"
#include "../../common/tables.h"
#include "../../common/defines.h"
#include "packet.h"

class CMiniGameManager : public singleton<CMiniGameManager>
{
	public:
		CMiniGameManager();
		virtual ~CMiniGameManager();

		void Initialize();
		void Destroy();

		void SpawnNPC(DWORD dwVnum, BYTE bType);
		void SpawnRegen(bool bIsEnable, BYTE bType);
		void SetEvent(bool bIsEnable, BYTE bType);
		void SetCharecterEvents(LPCHARACTER ch);

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
		bool ReadRewardItemFile(const char * c_pszFileName);
		void RequestRewardList(LPCHARACTER pChar);
		void AttendanceButtonClick(LPCHARACTER pChar, const char * data);
		void RequestData(LPCHARACTER pChar, const char * data);
#endif

	protected:
		BYTE bEventState[MINIGAME_TYPE_MAX];

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
		std::vector<TRewardItem>	attendanceRewardVec;
#endif
};

