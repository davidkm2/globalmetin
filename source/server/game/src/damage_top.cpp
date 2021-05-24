#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "log.h"
#include "char.h"
#include "db.h"
#include "lzo_manager.h"
#include "desc_client.h"
#include "buffer_manager.h"
#include "char_manager.h"
#include "packet.h"
#include "war_map.h"
#include "questmanager.h"
#include "locale_service.h"
#include "damage_top.h"
#include "p2p.h"

CDamageTop::CDamageTop()
{
}

CDamageTop::~CDamageTop()
{
	vecDamageTop.clear();
}

void CDamageTop::Initialize()
{
	if (g_bAuthServer)
		return;

	std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery("SELECT name, level, empire, victim_name, damage_value, damage_type+0 FROM damage_top;"));

	vecDamageTop.reserve(pmsg->Get()->uiNumRows);

	for (uint i = 0; i < pmsg->Get()->uiNumRows; i++)
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
		int col = 0;
		
		TDamage damage;
		strlcpy(damage.szAttackerName, row[col++], sizeof(damage.szAttackerName));
		str_to_number(damage.bAttackerLevel, row[col++]);
		str_to_number(damage.bAttackerEmpire, row[col++]);
		strlcpy(damage.szVictimName, row[col++], sizeof(damage.szVictimName));
		str_to_number(damage.dwDamageValue, row[col++]);
		str_to_number(damage.bDamageType, row[col++]);

		vecDamageTop.push_back(damage);
	}
}

TDamage CDamageTop::GetDamage(LPCHARACTER pkChar, BYTE bType)
{
	TDamage returnDamage;
	memset(&returnDamage, 0, sizeof(TDamage));
	
	if (!pkChar)
		return returnDamage;
	
	for (size_t i = 0; i < vecDamageTop.size(); i++)
	{
		if ((strcmp(pkChar->GetName(), vecDamageTop[i].szAttackerName) == 0) && bType == vecDamageTop[i].bDamageType)
		{
			returnDamage = vecDamageTop[i];
			return returnDamage;
		}
	}
	
	return returnDamage;
}

void CDamageTop::UpdateDamage(LPCHARACTER pkChar, LPCHARACTER pkVictim, BYTE bType, DWORD dam)
{
	if (!pkChar || !pkVictim)
		return;
	
	for (size_t i = 0; i < vecDamageTop.size(); i++)
	{
		if ((strcmp(pkChar->GetName(), vecDamageTop[i].szAttackerName) == 0) && bType == vecDamageTop[i].bDamageType)
		{
			vecDamageTop[i].dwDamageValue = dam;
			vecDamageTop[i].bAttackerLevel = pkChar->GetLevel();
			vecDamageTop[i].bAttackerEmpire = pkChar->GetEmpire();
			strlcpy(vecDamageTop[i].szVictimName, pkVictim->GetName(), sizeof(vecDamageTop[i].szVictimName));
			
			DBManager::instance().DirectQuery("UPDATE damage_top SET damage_value = %d, level = %d, empire = %d, victim_name = '%s' "
											  "WHERE name = '%s' AND damage_type = %d;", 
											  dam, pkChar->GetLevel(), pkChar->GetEmpire(), pkVictim->GetName(), pkChar->GetName(), bType);
											  
			SendGGUpdateDamage(vecDamageTop[i]);
											  
			break;
		}
	}
}

void CDamageTop::UpdateDamageExtern(LPDESC d, const char* c_pData)
{
	TPacketGGRegisterDamage* p = (TPacketGGRegisterDamage*) c_pData;

	bool bContain = false;
	
	if (!p->damageT.dwDamageValue)
		return;

	if (!vecDamageTop.empty())
	{
		for (size_t i = 0; i < vecDamageTop.size(); i++)
		{
			if (!vecDamageTop[i].dwDamageValue)
				continue;
		
			if ((strcmp(p->damageT.szAttackerName, vecDamageTop[i].szAttackerName) == 0) && p->damageT.bDamageType == vecDamageTop[i].bDamageType)
			{
				bContain = true;
				memcpy(&vecDamageTop[i], &p->damageT, sizeof(vecDamageTop[i]));
				break;
			}
		}
	}
	
	if (!bContain)
		vecDamageTop.push_back(p->damageT);
}

void CDamageTop::SendGGUpdateDamage(TDamage damageT)
{
	TPacketGGRegisterDamage p2pPacket;
	p2pPacket.bHeader = HEADER_GG_REGISTER_DAMAGE;
	memcpy(&p2pPacket.damageT, &damageT, sizeof(p2pPacket.damageT));
	P2P_MANAGER::instance().Send(&p2pPacket, sizeof(TPacketGGRegisterDamage));
}

void CDamageTop::RegisterHitDamage(LPCHARACTER pkVictim, LPCHARACTER pkAttacker, DWORD dam)
{
	if (!pkVictim || !pkAttacker)
		return;
	
	if (!pkAttacker->IsPC())
		return;
	
	if (dam < 3000)
		return;
	
	BYTE damageType = pkVictim->IsPC() ? DAMAGE_TOP_TYPE_HIT_PVP : DAMAGE_TOP_TYPE_HIT_PVM;
	
	TDamage damageInfo = GetDamage(pkAttacker, damageType);
	if (damageInfo.dwDamageValue != 0 && damageInfo.dwDamageValue < dam)
		UpdateDamage(pkAttacker, pkVictim, damageType, dam);
	else
	{
		DBManager::instance().DirectQuery("REPLACE INTO damage_top (name, level, empire, victim_name, damage_value, damage_type) VALUES ('%s', %d, %d, '%s', %d, %d);", 
				pkAttacker->GetName(), pkAttacker->GetLevel(), pkAttacker->GetEmpire(), pkVictim->GetName(), dam, damageType);
		
		TDamage newDamage;
		strlcpy(newDamage.szAttackerName, pkAttacker->GetName(), sizeof(newDamage.szAttackerName));
		newDamage.bAttackerLevel = pkAttacker->GetLevel();
		newDamage.bAttackerEmpire = pkAttacker->GetEmpire();
		strlcpy(newDamage.szVictimName, pkVictim->GetName(), sizeof(newDamage.szVictimName));
		newDamage.dwDamageValue = dam;
		newDamage.bDamageType = damageType;
		
		SendGGUpdateDamage(newDamage);
		
		vecDamageTop.push_back(newDamage);
	}
}

void CDamageTop::RegisterSkillDamage(LPCHARACTER pkVictim, LPCHARACTER pkAttacker, DWORD dam)
{
	if (!pkVictim || !pkAttacker)
		return;
	
	if (!pkAttacker->IsPC())
		return;
	
	if (dam < 3000)
		return;
	
	BYTE damageType = pkVictim->IsPC() ? DAMAGE_TOP_TYPE_SKILL_PVP : DAMAGE_TOP_TYPE_SKILL_PVM;
	
	TDamage damageInfo = GetDamage(pkAttacker, damageType);
	if (damageInfo.dwDamageValue != 0 && damageInfo.dwDamageValue < dam)
		UpdateDamage(pkAttacker, pkVictim, damageType, dam);
	else
	{
		DBManager::instance().DirectQuery("REPLACE INTO damage_top (name, level, empire, victim_name, damage_value, damage_type) VALUES ('%s', %d, %d, '%s', %d, %d);", 
				pkAttacker->GetName(), pkAttacker->GetLevel(), pkAttacker->GetEmpire(), pkVictim->GetName(), dam, damageType);
		
		TDamage newDamage;
		strlcpy(newDamage.szAttackerName, pkAttacker->GetName(), sizeof(newDamage.szAttackerName));
		newDamage.bAttackerLevel = pkAttacker->GetLevel();
		newDamage.bAttackerEmpire = pkAttacker->GetEmpire();
		strlcpy(newDamage.szVictimName, pkVictim->GetName(), sizeof(newDamage.szVictimName));
		newDamage.dwDamageValue = dam;
		newDamage.bDamageType = damageType;
		
		SendGGUpdateDamage(newDamage);
		
		vecDamageTop.push_back(newDamage);
	}
}

struct FDamageCompare : public std::function<bool(TDamage, TDamage)>
{
	bool operator () (TDamage d1, TDamage d2) const
	{
		if (d1.dwDamageValue > d2.dwDamageValue)
			return true;
		if (d1.dwDamageValue < d2.dwDamageValue)
			return false;
		//if (d1.bAttackerLevel < d2.bAttackerLevel)
		//	return true;
		//if (d1.bAttackerLevel > d2.bAttackerLevel)
		//	return false;
 
		return false;
	}
};

void CDamageTop::GetSendVector(std::vector<TDamage>& retVector)
{
	std::vector<TDamage> tempVec[DAMAGE_TOP_TYPE_MAX - 1];
	
	for (size_t i = 0; i < vecDamageTop.size(); i++)
		tempVec[vecDamageTop[i].bDamageType - 1].push_back(vecDamageTop[i]);
	
	for (int j = 0; j < DAMAGE_TOP_TYPE_MAX - 1; j++)
	{
		std::sort(tempVec[j].begin(), tempVec[j].end(), FDamageCompare());
		
		if(tempVec[j].size() > 10)
			tempVec[j].resize(10);
		
		retVector.insert(retVector.end(), tempVec[j].begin(), tempVec[j].end());
	}
}

void CDamageTop::OpenDamageTop(LPCHARACTER pkChar)
{
	if (pkChar == NULL)
		return;
	
	if (!pkChar->GetDesc())
		return;

	std::vector<TDamage> topVector;
	GetSendVector(topVector);
	
	TPacketGCDamageTop packet;
	packet.bHeader = HEADER_GC_DAMAGE_TOP;
	packet.wSize = sizeof(packet) + (sizeof(TDamage) * topVector.size()); 

	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(packet));
	pkChar->GetDesc()->Packet(&topVector[0], sizeof(TDamage) * topVector.size());
}

