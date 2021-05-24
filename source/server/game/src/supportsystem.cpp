#include "stdafx.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "supportsystem.h"
#include "../../common/VnumHelper.h"
#include "packet.h"
#include "item_manager.h"
#include "item.h"
#include "affect.h"
#include "skill.h"
#include "db.h"

extern int passes_per_sec;
EVENTINFO(supportSystem_event_info)
{
	CSupportSystem* psupportSystem;
};

EVENTFUNC(supportSystem_update_event)
{
	supportSystem_event_info* info = dynamic_cast<supportSystem_event_info*>( event->info );
	if ( info == NULL )
	{
		sys_err( "check_speedhack_event> <Factor> Null pointer" );
		return 0;
	}

	CSupportSystem*	psupportSystem = info->psupportSystem;

	if (NULL == psupportSystem)
		return 0;

	
	psupportSystem->Update(0);
	return PASSES_PER_SEC(1) / 4;
}
const float SUPPORT_COUNT_LIMIT = 3;

///////////////////////////////////////////////////////////////////////////////////////
//  CSupportActor
///////////////////////////////////////////////////////////////////////////////////////

CSupportActor::CSupportActor(LPCHARACTER owner, DWORD vnum, DWORD options)
{
	m_dwVnum = vnum;
	m_dwVID = 0;
	m_dwOptions = options;
	m_dwLastActionTime = 0;
	
	m_dwlevel = 1;
	m_dwExp = 0;
	m_dwNextExp = 0;
	m_dwIntSkill = 3;

	m_dwArmor = 0;
	m_dwHair = 0;
	m_dwHairVnum = 0;
	m_dwWeapon = 0;
	
	m_pkChar = 0;
	m_pkOwner = owner;

	m_originalMoveSpeed = 0;
	
	m_dwSummonItemVID = 0;
	m_dwSummonItemVnum = 0;
}

CSupportActor::~CSupportActor()
{
	this->Unsummon();

	m_pkOwner = 0;
}

void CSupportActor::SetName()
{
	std::string supportName = m_pkChar->GetOwner()->GetName();

	std::string supportnames = " - Destek";

	if (true == IsSummoned())
		m_pkChar->SetName(supportName);
	
	first_name = m_pkChar->GetOwner()->GetName();
	second_name = supportnames;
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportName %s%s", first_name.c_str(), second_name.c_str());
}

void CSupportActor::SetNextExp(int nextExp)
{	
	m_dwNextExp = nextExp;
}

void CSupportActor::SetIntSkill()
{	
	if (GetLevel() * 1 >= 90)
		m_dwIntSkill = 90;
	else
		m_dwIntSkill = GetLevel()*3;
	
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != NULL)
		pSummonItem->SetForceAttribute(1, 1, m_dwIntSkill);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupporInt %d", m_dwIntSkill);
}

int CSupportActor::GetIntSkill()
{
	int k=0;
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if(pSummonItem != NULL)
		k = pSummonItem->GetAttributeValue(1) + pSummonItem->GetAttributeValue(2);	
	
	return k;	
}
void CSupportActor::SetLevel(DWORD level)
{
	m_pkChar->SetLevel(static_cast<char>(level));
	m_dwlevel = level;
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportLv %d", m_dwlevel);	
	SetNextExp(m_pkChar->SupportGetNextExp());
	SetIntSkill();
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != NULL)
		pSummonItem->SetForceAttribute(0, 1, m_dwlevel);
}
void CSupportActor::SetCostume()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != NULL)
	{
		char OGArmorQuery[1024];
		snprintf(OGArmorQuery, sizeof(OGArmorQuery), "SELECT armor FROM player.new_support WHERE id = %u ", pSummonItem->GetID());
		std::unique_ptr<SQLMsg> armor2(DBManager::instance().DirectQuery(OGArmorQuery));
		if (armor2->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(armor2->Get()->pSQLResult);
			m_pkChar->SetSupportArmor(atoi(row[0]));
		}
		else
			m_pkChar->SetSupportArmor(11809);
	}
}

void CSupportActor::SetHair()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != NULL)
	{
		char OGHairQuery[1024];
		snprintf(OGHairQuery, sizeof(OGHairQuery), "SELECT hair FROM player.new_support WHERE id = %u ", pSummonItem->GetID());
		std::unique_ptr<SQLMsg> hair2(DBManager::instance().DirectQuery(OGHairQuery));
		if (hair2->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(hair2->Get()->pSQLResult);
			m_pkChar->SetSupportHair(atoi(row[0]));
		}
		else
			m_pkChar->SetSupportHair(4004);
	}
}

void CSupportActor::SetWeapon()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != NULL)
	{
		char OGWeaponQuery[1024];
		snprintf(OGWeaponQuery, sizeof(OGWeaponQuery), "SELECT weapon FROM player.new_support WHERE id = %u ", pSummonItem->GetID());
		std::unique_ptr<SQLMsg> weapon2(DBManager::instance().DirectQuery(OGWeaponQuery));
		if (weapon2->Get()->uiNumRows > 0) {
			MYSQL_ROW row = mysql_fetch_row(weapon2->Get()->pSQLResult);
			m_pkChar->SetSupportWeapon(atoi(row[0]));
		}
		else
		{
			m_pkChar->SetSupportWeapon(7009);
		}
	}
}

void CSupportActor::SetExp(DWORD exp)
{
	if (exp < 0)
		exp = MAX(m_dwExp - exp, 0);

	if (GetExp() + exp >= GetNextExp())
	{
		SetLevel(GetLevel() + 1);
		m_pkChar->UpdatePacket();
		m_dwExp = 0;
		m_pkChar->SetExp(0);
		return;

	}
	m_dwExp += exp;
	m_pkChar->SetExp(m_dwExp);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportExp %d %d", m_dwExp, m_pkChar->SupportGetNextExp());
}

bool CSupportActor::Mount()
{
	if (0 == m_pkOwner)
		return false;

	if (true == HasOption(ESupportOption_Mountable))
		m_pkOwner->MountVnum(m_dwVnum);

	return m_pkOwner->GetMountVnum() == m_dwVnum;;
}

void CSupportActor::Unmount()
{
	if (0 == m_pkOwner)
		return;

	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();
}

void CSupportActor::Unsummon()
{
	if (true == this->IsSummoned())
	{
		LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
		if (pSummonItem != NULL){
			DBManager::instance().DirectQuery("UPDATE player.new_support SET level = %u, exp=%u, bonus=%u WHERE id = %u ", this->GetLevel(), this->m_dwExp, this->m_dwIntSkill, ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())->GetID());
			this->ClearBuff();
			pSummonItem->SetForceAttribute(0, 1, m_dwlevel);
			pSummonItem->SetSocket(3, m_dwExp);
			pSummonItem->SetForceAttribute(1, 1, m_dwIntSkill);
			pSummonItem->Lock(false);
		}
		this->SetSummonItem(NULL);
		if (NULL != m_pkOwner)
			m_pkOwner->ComputePoints();

		if (NULL != m_pkChar)
			M2_DESTROY_CHARACTER(m_pkChar);

		m_pkChar = 0;
		m_dwVID = 0;
		m_dwlevel = 1;
		m_dwExp = 0;
		m_dwNextExp = 0;
		m_dwIntSkill = 0;

		this->ClearBuff();
		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportLeave");
	}
}


DWORD CSupportActor::Summon(const char* supportName, LPITEM pSummonItem, bool bSpawnFar)
{
	long x = m_pkOwner->GetX();
	long y = m_pkOwner->GetY();
	long z = m_pkOwner->GetZ();

	if (true == bSpawnFar)
	{
		x += (number(0, 1) * 2 - 1) * number(2000, 2500);
		y += (number(0, 1) * 2 - 1) * number(2000, 2500);
	}
	else
	{
		x += number(-100, 100);
		y += number(-100, 100);
	}

	if (0 != m_pkChar)
	{
		m_pkChar->Show (m_pkOwner->GetMapIndex(), x, y);
		m_dwVID = m_pkChar->GetVID();

		return m_dwVID;
	}
	
	m_pkChar = CHARACTER_MANAGER::instance().SpawnMob(
				m_dwVnum, 
				m_pkOwner->GetMapIndex(), 
				x, y, z,
				false, (int)(m_pkOwner->GetRotation()+180), false);

	if (0 == m_pkChar)
	{
		sys_err("[CSupportSystem::Summon] Failed to summon the shaman. (vnum: %d)", m_dwVnum);
		return 0;
	}

	m_pkChar->SetSupport();
	
	m_pkChar->SetOwner(m_pkOwner);
	m_pkChar->SetEmpire(m_pkOwner->GetEmpire());

	m_dwVID = m_pkChar->GetVID();

	std::unique_ptr<SQLMsg> pMsg(DBManager::Instance().DirectQuery("SELECT id FROM player.new_support WHERE id='%u'", pSummonItem->GetID()));
	if (pMsg->Get()->uiNumRows == 0)
		DBManager::Instance().DirectQuery("INSERT INTO player.new_support (id, level, exp, bonus, armor, weapon, hair, hairvnum) VALUES(%u, 1, 0, 0, 11809, 7009, 1, 4004)", pSummonItem->GetID());

	char szQuery1[1024];
	snprintf(szQuery1, sizeof(szQuery1), "SELECT level,exp,bonus,armor,weapon,hair,hairvnum FROM player.new_support WHERE id = %u ", pSummonItem->GetID());
	std::unique_ptr<SQLMsg> pmsg2(DBManager::instance().DirectQuery(szQuery1));
	if (pmsg2->Get()->uiNumRows > 0) {
		MYSQL_ROW row = mysql_fetch_row(pmsg2->Get()->pSQLResult);
		this->m_dwlevel = atoi(row[0]);
//		this->m_dwExp = atoi(row[1]);

		this->SetLevel(atoi(row[0]));
//		this->SetExp(atoi(row[1]));
		this->m_dwIntSkill = atoi(row[2]);
		this->m_dwArmor = atoi(row[3]);
		this->m_dwWeapon = atoi(row[4]);
		this->m_dwHair = atoi(row[5]);
		this->m_dwHairVnum = atoi(row[6]);
	}

	this->SetSummonItem(pSummonItem);

	this->SetCostume();
	this->SetHair();
	this->SetWeapon();
	m_dwExp = pSummonItem->GetSocket(3);
	if (m_dwlevel == 0)
	{
		m_dwlevel = 1;
		m_dwExp = 0;
		this->SetLevel(1);
	}
	else
		this->SetLevel(m_dwlevel);
	m_dwNextExp = m_pkChar->SupportGetNextExp();
	
	this->SetName();
	
	std::string m_speciality;
	if (m_dwSummonItemVnum == 8383 || m_dwSummonItemVnum ==8384)
	{
		m_speciality = "E.Gücü";
	}
	else
	{
		m_speciality = "/";
	}
	m_pkOwner->ComputePoints();
	m_pkChar->Show(m_pkOwner->GetMapIndex(), x, y, z);
	m_pkChar->UpdatePacket();
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportLv %d", m_dwlevel);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportArmor %d", m_dwArmor);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportWeapon %u", m_dwWeapon);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportHair %u", m_dwHairVnum);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupporInt %d", m_dwIntSkill);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportExp %d %d", m_dwExp, m_pkChar->SupportGetNextExp());
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportIcon %d", m_dwSummonItemVnum);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportSpeciality %s", m_speciality.c_str());
	pSummonItem->Lock(true);
	return m_dwVID;
}
void CSupportActor::RefreshCostume()
{
	SetCostume();
	SetHair();
	SetWeapon();
	m_pkChar->UpdatePacket();
}
void CSupportActor::UpdatePacketsupportActor()
{
	m_pkChar->UpdatePacket();
}
bool CSupportActor::_UpdatAloneActionAI(float fMinDist, float fMaxDist)
{
	float fDist = number(fMinDist, fMaxDist);
	float r = (float)number (0, 359);
	float dest_x = GetOwner()->GetX() + fDist * cos(r);
	float dest_y = GetOwner()->GetY() + fDist * sin(r);

	m_pkChar->SetNowWalking(true);

	if (!m_pkChar->IsStateMove() && m_pkChar->Goto(dest_x, dest_y))
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	m_dwLastActionTime = get_dword_time();

	return true;
}

bool CSupportActor::_UpdateFollowAI()
{
	if (0 == m_pkChar->m_pkMobData)
	{
		return false;
	}
	
	if (0 == m_originalMoveSpeed)
	{
		const CMob* mobData = CMobManager::Instance().Get(m_dwVnum);

		if (0 != mobData)
			m_originalMoveSpeed = mobData->m_table.sMovingSpeed;
	}
	float	START_FOLLOW_DISTANCE = 300.0f;		
	float	START_RUN_DISTANCE = 900.0f;		

	float	RESPAWN_DISTANCE = 4500.f;			
	int		APPROACH = 200;						

	bool bRun = false;							

	DWORD currentTime = get_dword_time();

	long ownerX = m_pkOwner->GetX();		long ownerY = m_pkOwner->GetY();
	long charX = m_pkChar->GetX();			long charY = m_pkChar->GetY();

	float fDist = DISTANCE_APPROX(charX - ownerX, charY - ownerY);

	if (fDist >= RESPAWN_DISTANCE)
	{
		float fOwnerRot = m_pkOwner->GetRotation() * 3.141592f / 180.f;
		float fx = -APPROACH * cos(fOwnerRot);
		float fy = -APPROACH * sin(fOwnerRot);
		if (m_pkChar->Show(m_pkOwner->GetMapIndex(), ownerX + fx, ownerY + fy))
		{
			return true;
		}
	}
	
	
	if (fDist >= START_FOLLOW_DISTANCE)
	{
		if( fDist >= START_RUN_DISTANCE)
		{
			bRun = true;
		}

		m_pkChar->SetNowWalking(!bRun);
		
		Follow(APPROACH);

		m_pkChar->SetLastAttacked(currentTime);
		m_dwLastActionTime = currentTime;
	}
	else 
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	return true;
}

bool CSupportActor::Update(DWORD deltaTime)
{
	bool bResult = true;

	if (m_pkOwner->IsDead() || (IsSummoned() && m_pkChar->IsDead()) 
		|| NULL == ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())
		|| ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())->GetOwner() != this->GetOwner()
		)
	{
		this->Unsummon();
		return true;
	}
	if(get_dword_time() - this->GetLastExpUpdate() >= 3000)
	{
		this->UpdateItemExp();
		m_pkChar->SetSupportInt(this->GetIntSkill());
		this->SetLastExpUpdate(get_dword_time());
	}
	
	this->UseSkill();
	this->RefreshCostume();
	
	if (this->IsSummoned() && HasOption(ESupportOption_Followable))
		bResult = bResult && this->_UpdateFollowAI();

	return bResult;
}
void CSupportActor::UseSkill()
{
	if (m_pkChar->GetOwner()->IsAffectFlag(AFF_HOSIN) == false && get_dword_time() - this->GetLastSkillTime() >= 3000)
	{
		if (m_pkChar->GetLevel() >= 1 && m_pkChar->GetLevel() <= 3)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 1);
		if (m_pkChar->GetLevel() >= 4 && m_pkChar->GetLevel() <= 6)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 2);
		if (m_pkChar->GetLevel() >= 7 && m_pkChar->GetLevel() <= 9)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 3);
		if (m_pkChar->GetLevel() >= 10 && m_pkChar->GetLevel() <= 12)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 4);
		if (m_pkChar->GetLevel() >= 13 && m_pkChar->GetLevel() <= 15)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 5);
		if (m_pkChar->GetLevel() >= 16 && m_pkChar->GetLevel() <= 18)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 6);
		if (m_pkChar->GetLevel() >= 19 && m_pkChar->GetLevel() <= 21)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 7);
		if (m_pkChar->GetLevel() >= 22 && m_pkChar->GetLevel() <= 24)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 8);
		if (m_pkChar->GetLevel() >= 25 && m_pkChar->GetLevel() <= 27)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 9);
		if (m_pkChar->GetLevel() >= 28 && m_pkChar->GetLevel() <= 30)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 10);
		if (m_pkChar->GetLevel() >= 31 && m_pkChar->GetLevel() <= 33)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 11);
		if (m_pkChar->GetLevel() >= 34 && m_pkChar->GetLevel() <= 36)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 12);
		if (m_pkChar->GetLevel() >= 37 && m_pkChar->GetLevel() <= 39)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 13);
		if (m_pkChar->GetLevel() >= 40 && m_pkChar->GetLevel() <= 42)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 14);
		if (m_pkChar->GetLevel() >= 43 && m_pkChar->GetLevel() <= 45)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 15);
		if (m_pkChar->GetLevel() >= 46 && m_pkChar->GetLevel() <= 48)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 16);
		if (m_pkChar->GetLevel() >= 49 && m_pkChar->GetLevel() <= 51)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 17);
		if (m_pkChar->GetLevel() >= 52 && m_pkChar->GetLevel() <= 54)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 18);
		if (m_pkChar->GetLevel() >= 55 && m_pkChar->GetLevel() <= 57)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 19);
		if (m_pkChar->GetLevel() >= 58 && m_pkChar->GetLevel() <= 60)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 20);
		if (m_pkChar->GetLevel() >= 61 && m_pkChar->GetLevel() <= 63)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 21);
		if (m_pkChar->GetLevel() >= 64 && m_pkChar->GetLevel() <= 66)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 22);
		if (m_pkChar->GetLevel() >= 67 && m_pkChar->GetLevel() <= 69)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 23);
		if (m_pkChar->GetLevel() >= 70 && m_pkChar->GetLevel() <= 72)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 24);
		if (m_pkChar->GetLevel() >= 73 && m_pkChar->GetLevel() <= 75)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 25);
		if (m_pkChar->GetLevel() >= 76 && m_pkChar->GetLevel() <= 78)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 26);
		if (m_pkChar->GetLevel() >= 79 && m_pkChar->GetLevel() <= 81)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 27);
		if (m_pkChar->GetLevel() >= 82 && m_pkChar->GetLevel() <= 84)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 28);
		if (m_pkChar->GetLevel() >= 85 && m_pkChar->GetLevel() <= 87)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 29);
		if (m_pkChar->GetLevel() >= 88 && m_pkChar->GetLevel() <= 90)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 30);
		if (m_pkChar->GetLevel() >= 91 && m_pkChar->GetLevel() <= 93)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 31);
		if (m_pkChar->GetLevel() >= 94 && m_pkChar->GetLevel() <= 96)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 32);
		if (m_pkChar->GetLevel() >= 97 && m_pkChar->GetLevel() <= 99)
			m_pkChar->ComputeSkill(94, m_pkChar->GetOwner(), 41);
		this->SetLastSkillTime(get_dword_time());
		m_pkChar->SendSupportSkillPacket(SKILL_HOSIN);
	}
	else if (m_pkChar->GetOwner()->IsAffectFlag(AFF_BOHO) == false && get_dword_time() - this->GetLastSkillTime() >= 3000)
	{
		if (m_pkChar->GetLevel() >= 1 && m_pkChar->GetLevel() <= 3)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 1);
		if (m_pkChar->GetLevel() >= 4 && m_pkChar->GetLevel() <= 6)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 2);
		if (m_pkChar->GetLevel() >= 7 && m_pkChar->GetLevel() <= 9)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 3);
		if (m_pkChar->GetLevel() >= 10 && m_pkChar->GetLevel() <= 12)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 4);
		if (m_pkChar->GetLevel() >= 13 && m_pkChar->GetLevel() <= 15)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 5);
		if (m_pkChar->GetLevel() >= 16 && m_pkChar->GetLevel() <= 18)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 6);
		if (m_pkChar->GetLevel() >= 19 && m_pkChar->GetLevel() <= 21)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 7);
		if (m_pkChar->GetLevel() >= 22 && m_pkChar->GetLevel() <= 24)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 8);
		if (m_pkChar->GetLevel() >= 25 && m_pkChar->GetLevel() <= 27)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 9);
		if (m_pkChar->GetLevel() >= 28 && m_pkChar->GetLevel() <= 30)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 10);
		if (m_pkChar->GetLevel() >= 31 && m_pkChar->GetLevel() <= 33)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 11);
		if (m_pkChar->GetLevel() >= 34 && m_pkChar->GetLevel() <= 36)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 12);
		if (m_pkChar->GetLevel() >= 37 && m_pkChar->GetLevel() <= 39)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 13);
		if (m_pkChar->GetLevel() >= 40 && m_pkChar->GetLevel() <= 42)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 14);
		if (m_pkChar->GetLevel() >= 43 && m_pkChar->GetLevel() <= 45)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 15);
		if (m_pkChar->GetLevel() >= 46 && m_pkChar->GetLevel() <= 48)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 16);
		if (m_pkChar->GetLevel() >= 49 && m_pkChar->GetLevel() <= 51)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 17);
		if (m_pkChar->GetLevel() >= 52 && m_pkChar->GetLevel() <= 54)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 18);
		if (m_pkChar->GetLevel() >= 55 && m_pkChar->GetLevel() <= 57)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 19);
		if (m_pkChar->GetLevel() >= 58 && m_pkChar->GetLevel() <= 60)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 20);
		if (m_pkChar->GetLevel() >= 61 && m_pkChar->GetLevel() <= 63)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 21);
		if (m_pkChar->GetLevel() >= 64 && m_pkChar->GetLevel() <= 66)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 22);
		if (m_pkChar->GetLevel() >= 67 && m_pkChar->GetLevel() <= 69)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 23);
		if (m_pkChar->GetLevel() >= 70 && m_pkChar->GetLevel() <= 72)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 24);
		if (m_pkChar->GetLevel() >= 73 && m_pkChar->GetLevel() <= 75)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 25);
		if (m_pkChar->GetLevel() >= 76 && m_pkChar->GetLevel() <= 78)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 26);
		if (m_pkChar->GetLevel() >= 79 && m_pkChar->GetLevel() <= 81)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 27);
		if (m_pkChar->GetLevel() >= 82 && m_pkChar->GetLevel() <= 84)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 28);
		if (m_pkChar->GetLevel() >= 85 && m_pkChar->GetLevel() <= 87)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 29);
		if (m_pkChar->GetLevel() >= 88 && m_pkChar->GetLevel() <= 90)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 30);
		if (m_pkChar->GetLevel() >= 91 && m_pkChar->GetLevel() <= 93)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 31);
		if (m_pkChar->GetLevel() >= 94 && m_pkChar->GetLevel() <= 96)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 32);
		if (m_pkChar->GetLevel() >= 97 && m_pkChar->GetLevel() <= 99)
			m_pkChar->ComputeSkill(95, m_pkChar->GetOwner(), 41);
		this->SetLastSkillTime(get_dword_time());
		m_pkChar->SendSupportSkillPacket(SKILL_REFLECT);
	}
	else if (m_pkChar->GetOwner()->IsAffectFlag(AFF_GICHEON) == false && get_dword_time() - this->GetLastSkillTime() >= 3000)
	{
		if (m_pkChar->GetLevel() >= 1 && m_pkChar->GetLevel() <= 3)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 1);
		if (m_pkChar->GetLevel() >= 4 && m_pkChar->GetLevel() <= 6)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 2);
		if (m_pkChar->GetLevel() >= 7 && m_pkChar->GetLevel() <= 9)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 3);
		if (m_pkChar->GetLevel() >= 10 && m_pkChar->GetLevel() <= 12)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 4);
		if (m_pkChar->GetLevel() >= 13 && m_pkChar->GetLevel() <= 15)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 5);
		if (m_pkChar->GetLevel() >= 16 && m_pkChar->GetLevel() <= 18)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 6);
		if (m_pkChar->GetLevel() >= 19 && m_pkChar->GetLevel() <= 21)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 7);
		if (m_pkChar->GetLevel() >= 22 && m_pkChar->GetLevel() <= 24)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 8);
		if (m_pkChar->GetLevel() >= 25 && m_pkChar->GetLevel() <= 27)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 9);
		if (m_pkChar->GetLevel() >= 28 && m_pkChar->GetLevel() <= 30)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 10);
		if (m_pkChar->GetLevel() >= 31 && m_pkChar->GetLevel() <= 33)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 11);
		if (m_pkChar->GetLevel() >= 34 && m_pkChar->GetLevel() <= 36)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 12);
		if (m_pkChar->GetLevel() >= 37 && m_pkChar->GetLevel() <= 39)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 13);
		if (m_pkChar->GetLevel() >= 40 && m_pkChar->GetLevel() <= 42)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 14);
		if (m_pkChar->GetLevel() >= 43 && m_pkChar->GetLevel() <= 45)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 15);
		if (m_pkChar->GetLevel() >= 46 && m_pkChar->GetLevel() <= 48)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 16);
		if (m_pkChar->GetLevel() >= 49 && m_pkChar->GetLevel() <= 51)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 17);
		if (m_pkChar->GetLevel() >= 52 && m_pkChar->GetLevel() <= 54)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 18);
		if (m_pkChar->GetLevel() >= 55 && m_pkChar->GetLevel() <= 57)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 19);
		if (m_pkChar->GetLevel() >= 58 && m_pkChar->GetLevel() <= 60)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 20);
		if (m_pkChar->GetLevel() >= 61 && m_pkChar->GetLevel() <= 63)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 21);
		if (m_pkChar->GetLevel() >= 64 && m_pkChar->GetLevel() <= 66)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 22);
		if (m_pkChar->GetLevel() >= 67 && m_pkChar->GetLevel() <= 69)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 23);
		if (m_pkChar->GetLevel() >= 70 && m_pkChar->GetLevel() <= 72)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 24);
		if (m_pkChar->GetLevel() >= 73 && m_pkChar->GetLevel() <= 75)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 25);
		if (m_pkChar->GetLevel() >= 76 && m_pkChar->GetLevel() <= 78)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 26);
		if (m_pkChar->GetLevel() >= 79 && m_pkChar->GetLevel() <= 81)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 27);
		if (m_pkChar->GetLevel() >= 82 && m_pkChar->GetLevel() <= 84)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 28);
		if (m_pkChar->GetLevel() >= 85 && m_pkChar->GetLevel() <= 87)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 29);
		if (m_pkChar->GetLevel() >= 88 && m_pkChar->GetLevel() <= 90)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 30);
		if (m_pkChar->GetLevel() >= 91 && m_pkChar->GetLevel() <= 93)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 31);
		if (m_pkChar->GetLevel() >= 94 && m_pkChar->GetLevel() <= 96)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 32);
		if (m_pkChar->GetLevel() >= 97 && m_pkChar->GetLevel() <= 99)
			m_pkChar->ComputeSkill(96, m_pkChar->GetOwner(), 41);
		this->SetLastSkillTime(get_dword_time());
		m_pkChar->SendSupportSkillPacket(SKILL_GICHEON);
	}
}
void CSupportActor::UpdateItemExp()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != NULL)
	{
		pSummonItem->SetSocket(3, m_dwExp);
		pSummonItem->SetForceAttribute(0, 1, m_dwlevel);
		pSummonItem->SetForceAttribute(1, 1, m_dwIntSkill);
	}
}

bool CSupportActor::Follow(float fMinDistance)
{
	if( !m_pkOwner || !m_pkChar) 
		return false;

	float fOwnerX = m_pkOwner->GetX();
	float fOwnerY = m_pkOwner->GetY();

	float fSupportX = m_pkChar->GetX();
	float fSupportY = m_pkChar->GetY();

	float fDist = DISTANCE_SQRT(fOwnerX - fSupportX, fOwnerY - fSupportY);
	if (fDist <= fMinDistance)
		return false;

	m_pkChar->SetRotationToXY(fOwnerX, fOwnerY);

	float fx, fy;

	float fDistToGo = fDist - fMinDistance;
	GetDeltaByDegree(m_pkChar->GetRotation(), fDistToGo, &fx, &fy);
	
	if (!m_pkChar->Goto((int)(fSupportX+fx+0.5f), (int)(fSupportY+fy+0.5f)) )
		return false;

	m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);

	return true;
}

void CSupportActor::SetSummonItem (LPITEM pItem)
{
	if (NULL == pItem)
	{
		m_dwSummonItemVID = 0;
		m_dwSummonItemVnum = 0;
		return;
	}

	m_dwSummonItemVID = pItem->GetVID();
	m_dwSummonItemVnum = pItem->GetVnum();
}

void CSupportActor::GiveBuff()
{
	if (m_dwVnum == 34077)
	{
		if (NULL == m_pkOwner->GetDungeon())
		{
			return;
		}
	}
	LPITEM item = ITEM_MANAGER::instance().FindByVID(m_dwSummonItemVID);
	if (NULL != item)
		item->ModifyPoints(true);
	return ;
}

void CSupportActor::ClearBuff()
{
	if (NULL == m_pkOwner)
		return ;
	TItemTable* item_proto = ITEM_MANAGER::instance().GetTable(m_dwSummonItemVnum);
	if (NULL == item_proto)
		return;
	for (int i = 0; i < ITEM_APPLY_MAX_NUM; i++)
	{
		if (item_proto->aApplies[i].bType == APPLY_NONE)
			continue;
		m_pkOwner->ApplyPoint(item_proto->aApplies[i].bType, -item_proto->aApplies[i].lValue);
	}

	return ;
}

///////////////////////////////////////////////////////////////////////////////////////
//  CSupportSystem
///////////////////////////////////////////////////////////////////////////////////////

CSupportSystem::CSupportSystem(LPCHARACTER owner)
{

	m_pkOwner = owner;
	m_dwUpdatePeriod = 400;

	m_dwLastUpdateTime = 0;
}

CSupportSystem::~CSupportSystem()
{
	Destroy();
}

void CSupportSystem::Destroy()
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;

		if (0 != supportActor)
		{
			delete supportActor;
		}
	}
	event_cancel(&m_pksupportSystemUpdateEvent);
	m_supportActorMap.clear();
}

/// Æê ½Ã½ºÅÛ ¾÷µ¥ÀÌÆ®. µî·ÏµÈ ÆêµéÀÇ AI Ã³¸® µîÀ» ÇÔ.
bool CSupportSystem::Update(DWORD deltaTime)
{
	bool bResult = true;

	DWORD currentTime = get_dword_time();	
	
	if (m_dwUpdatePeriod > currentTime - m_dwLastUpdateTime)
		return true;
	
	std::vector <CSupportActor*> v_garbageActor;

	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;

		if (0 != supportActor && supportActor->IsSummoned())
		{
			LPCHARACTER pSupport = supportActor->GetCharacter();
			
			if (NULL == CHARACTER_MANAGER::instance().Find(pSupport->GetVID()))
			{
				v_garbageActor.push_back(supportActor);
			}
			else
			{
				bResult = bResult && supportActor->Update(deltaTime);
			}
		}
	}
	for (std::vector<CSupportActor*>::iterator it = v_garbageActor.begin(); it != v_garbageActor.end(); it++)
		DeleteSupport(*it);

	m_dwLastUpdateTime = currentTime;

	return bResult;
}

void CSupportSystem::DeleteSupport(DWORD mobVnum)
{
	TsupportActorMap::iterator iter = m_supportActorMap.find(mobVnum);

	if (m_supportActorMap.end() == iter)
	{
		sys_err("[CSupportSystem::DeleteSupport] Can't find shaman on my list (VNUM: %d)", mobVnum);
		return;
	}

	CSupportActor* supportActor = iter->second;

	if (0 == supportActor)
		sys_err("[CSupportSystem::DeleteSupport] Null Pointer (supportActor)");
	else
		delete supportActor;

	m_supportActorMap.erase(iter);	
}

/// °ü¸® ¸ñ·Ï¿¡¼­ ÆêÀ» Áö¿ò
void CSupportSystem::DeleteSupport(CSupportActor* supportActor)
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		if (iter->second == supportActor)
		{
			delete supportActor;
			m_supportActorMap.erase(iter);

			return;
		}
	}

	sys_err("[CSupportSystem::DeleteSupport] Can't find supportActor(0x%x) on my list(size: %d) ", supportActor, m_supportActorMap.size());
}

void CSupportSystem::Unsummon(DWORD vnum, bool bDeleteFromList)
{
	CSupportActor* actor = this->GetByVnum(vnum);

	if (0 == actor)
	{
		sys_err("[CSupportSystem::GetByVnum(%d)] Null Pointer (supportActor)", vnum);
		return;
	}
	actor->Unsummon();

	if (true == bDeleteFromList)
		this->DeleteSupport(actor);

	bool bActive = false;
	for (TsupportActorMap::iterator it = m_supportActorMap.begin(); it != m_supportActorMap.end(); it++)
	{
		bActive |= it->second->IsSummoned();
	}
	if (false == bActive)
	{
		event_cancel(&m_pksupportSystemUpdateEvent);
		m_pksupportSystemUpdateEvent = NULL;
	}
}

void CSupportSystem::SetExp(int iExp)
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;
		if (supportActor != 0)
		{
			if (supportActor->IsSummoned()) {
				supportActor->SetExp(iExp);
				break;
			}
		}
	}
}

int CSupportSystem::GetLevel()
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;
		if (supportActor != 0)
		{
			if (supportActor->IsSummoned()) {
				return supportActor->GetLevel();
			}
		}
	}
	return -1;
}

DWORD CSupportSystem::GetExp()
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;
		if (supportActor != 0)
		{
			if (supportActor->IsSummoned()) {
				return supportActor->GetExp();
			}
		}
	}
	return 0;
}
bool CSupportSystem::IsActiveSupport()
{
	bool state = false;
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;
		if (supportActor != 0)
		{
			if (supportActor->IsSummoned()) {
				state = true;
				break;
			}			
		}
	}
	return state;

}

CSupportActor* CSupportSystem::GetActiveSupport()
{
	for (TsupportActorMap::iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;
		if (supportActor != 0 && supportActor->IsSummoned())
			return supportActor;
	}

	return NULL;
}

CSupportActor* CSupportSystem::Summon(DWORD mobVnum, LPITEM pSummonItem, const char* supportName, bool bSpawnFar, DWORD options)
{
	CSupportActor* supportActor = this->GetByVnum(mobVnum);

	if (0 == supportActor)
	{
		supportActor = M2_NEW CSupportActor(m_pkOwner, mobVnum, options);
		m_supportActorMap.insert(std::make_pair(mobVnum, supportActor));
	}

	supportActor->Summon(supportName, pSummonItem, bSpawnFar);

	if (NULL == m_pksupportSystemUpdateEvent)
	{
		supportSystem_event_info* info = AllocEventInfo<supportSystem_event_info>();

		info->psupportSystem = this;

		m_pksupportSystemUpdateEvent = event_create(supportSystem_update_event, info, PASSES_PER_SEC(1) / 4);	// 0.25ÃÊ	
	}

	return supportActor;
}

CSupportActor* CSupportSystem::GetByVID(DWORD vid) const
{
	CSupportActor* supportActor = 0;

	bool bFound = false;

	for (TsupportActorMap::const_iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		supportActor = iter->second;

		if (0 == supportActor)
		{
			sys_err("[CSupportSystem::GetByVID(%d)] Null Pointer (supportActor)", vid);
			continue;
		}

		bFound = supportActor->GetVID() == vid;

		if (true == bFound)
			break;
	}

	return bFound ? supportActor : 0;
}

CSupportActor* CSupportSystem::GetByVnum(DWORD vnum) const
{
	CSupportActor* supportActor = 0;

	TsupportActorMap::const_iterator iter = m_supportActorMap.find(vnum);

	if (m_supportActorMap.end() != iter)
		supportActor = iter->second;

	return supportActor;
}

size_t CSupportSystem::CountSummoned() const
{
	size_t count = 0;

	for (TsupportActorMap::const_iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;

		if (0 != supportActor)
		{
			if (supportActor->IsSummoned())
				++count;
		}
	}

	return count;
}

void CSupportSystem::RefreshBuff()
{
	for (TsupportActorMap::const_iterator iter = m_supportActorMap.begin(); iter != m_supportActorMap.end(); ++iter)
	{
		CSupportActor* supportActor = iter->second;

		if (0 != supportActor)
		{
			if (supportActor->IsSummoned())
			{
				supportActor->GiveBuff();
			}
		}
	}
}
