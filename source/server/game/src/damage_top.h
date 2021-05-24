#ifndef __DAMAGE_TOP_H__
#define __DAMAGE_TOP_H__

#include "packet.h"
enum EDamageTopTypes
{
	DAMAGE_TOP_TYPE_NONE,
	DAMAGE_TOP_TYPE_HIT_PVP,
	DAMAGE_TOP_TYPE_SKILL_PVP,
	DAMAGE_TOP_TYPE_HIT_PVM,
	DAMAGE_TOP_TYPE_SKILL_PVM,
	DAMAGE_TOP_TYPE_MAX,
};

class CDamageTop : public singleton<CDamageTop>
{
	public:
		CDamageTop();
		virtual ~CDamageTop();

		void		Initialize();
		
		TDamage 	GetDamage(LPCHARACTER pkChar, BYTE bType);
		void 		UpdateDamage(LPCHARACTER pkChar, LPCHARACTER pkVictim, BYTE bType, DWORD dam);
		
		void 		UpdateDamageExtern(LPDESC d, const char* c_pData);
		void 		SendGGUpdateDamage(TDamage damageT);
		
		void		RegisterHitDamage(LPCHARACTER pkVictim, LPCHARACTER pkAttacker, DWORD dam);
		void		RegisterSkillDamage(LPCHARACTER pkVictim, LPCHARACTER pkAttacker, DWORD dam);
		
		void 		GetSendVector(std::vector<TDamage>& retVector);
		void 		OpenDamageTop(LPCHARACTER pkChar);
	private:
		std::vector<TDamage> vecDamageTop;
};
#endif

