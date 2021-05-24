#include "stdafx.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "mountsystem.h"
#include "../../common/VnumHelper.h"
#include "packet.h"
#include "item_manager.h"
#include "item.h"
#include "db.h"
#include "fstream"
extern int passes_per_sec;
EVENTINFO(mount_event_info)
{
	CMountSystem* pMountSystem;
};


EVENTFUNC(mount_update_event)
{
	mount_event_info* info = dynamic_cast<mount_event_info*>( event->info );
	if ( info == NULL )
	{
		sys_err( "check_speedhack_event> <Factor> Null pointer" );
		return 0;
	}

	CMountSystem* pMountSystem = info->pMountSystem;

	if (NULL == pMountSystem)
		return 0;

	
	pMountSystem->Update(0);
	return PASSES_PER_SEC(1) / 4;
}


CMountActor::CMountActor(LPCHARACTER owner, DWORD vnum)
{
	m_dwVnum = vnum;
	m_dwVID = 0;
	m_pkCharacter = 0;
	m_pkOwner = owner;

	m_OMoveSpeed = 0;
	m_dwMounted = false;
}

CMountActor::~CMountActor()
{
	this->Unsummon();

	m_pkOwner = 0;
}

void CMountActor::SetName(const char* name)
{
	std::string mName = "";

	mName += name;

	if (true == IsSummoned())
		m_pkCharacter->SetName(mName);

	m_Name = mName;
}

DWORD CMountActor::Mount()
{
	if (!m_pkOwner)
		return 0;

	long x = m_pkOwner->GetX(), y = m_pkOwner->GetY();

	x += number(-100, 100);
	y += number(-100, 100);
	
	if (IsMounting() == true)
		return 0;

	if (NULL != m_pkCharacter)
		M2_DESTROY_CHARACTER(m_pkCharacter);
	
	m_pkCharacter = 0;

	m_pkOwner->MountVnum(m_dwVnum);
	
	m_pkOwner->SetMountingM(true);

	m_dwMounted = true;
	
	return m_dwVnum;;
}

DWORD CMountActor::Unmount()
{
	if (!m_pkOwner || IsMounting() == false)
		return 0;

	m_pkOwner->SetMountingM(false);
	
	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();

	m_pkOwner->MountVnum(0);	
	m_dwMounted = false;
	
	this->Summon(m_pkOwner->GetName(), m_dwVnum);

	return 1;
}

void CMountActor::Unsummon()
{
	if (true == this->IsSummoned())
	{
		if (m_pkOwner)
			m_pkOwner->ComputePoints();

		if(IsMounting() == true)
			this->Unmount();

		if (NULL != m_pkCharacter)
			M2_DESTROY_CHARACTER(m_pkCharacter);
		
		m_pkCharacter = 0;
		m_dwVID = 0;
		m_dwMounted = false;

		if (m_pkOwner)
			m_pkOwner->SetMountVnumM(0);
	}
}

void CMountActor::Summon(const char* mName, DWORD mobVnum)
{
	if (!m_pkOwner)
		return;

	long x = m_pkOwner->GetX(), y = m_pkOwner->GetY(), z = m_pkOwner->GetZ();

	m_pkOwner->SetMountingM(false);
	
	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();

	m_pkOwner->MountVnum(0);	
	m_dwMounted = false;

	
	x += number(-100, 100);
	y += number(-100, 100);

	if(this->IsMounting() == true && this->IsSummoned() == false)
	{
		m_pkOwner->SetMountingM(false);
	
		if (m_pkOwner->IsHorseRiding())
			m_pkOwner->StopRiding();

		m_pkOwner->MountVnum(0);	
		m_dwMounted = false;
		m_pkCharacter = 0;
		m_dwVID = 0;

	}
	if (this->IsSummoned() == true)
		this->Unsummon();

	if (0 != m_pkCharacter)
	{
		m_pkCharacter->Show (m_pkOwner->GetMapIndex(), x, y);
		m_dwVID = m_pkCharacter->GetVID();

		return;
	}
	
	m_pkCharacter = CHARACTER_MANAGER::instance().SpawnMob(mobVnum, m_pkOwner->GetMapIndex(), x, y, z, false, (int)(m_pkOwner->GetRotation()+180), false);

	if (0 == m_pkCharacter)
	{
		sys_err("[CMountActor::Summon] Cannot summon mount - ERROR CODE [1]");
		return;
	}


	m_pkCharacter->SetEmpire(m_pkOwner->GetEmpire());
	m_pkCharacter->SetMount();
	
	
	m_dwVID = m_pkCharacter->GetVID();
	
	this->SetName(m_pkOwner->GetName());	
	m_pkOwner->SetMountVnumM(m_dwVnum);
	m_pkOwner->ComputePoints();
	m_pkCharacter->Show(m_pkOwner->GetMapIndex(), x, y, z);
	
	return;
}

bool CMountActor::_UpdatAloneActionAI(float fMinDist, float fMaxDist)
{
	float fDist = number(fMinDist, fMaxDist), r = (float)number (0, 359), dest_x = GetOwner()->GetX() + fDist * cos(r), dest_y = GetOwner()->GetY() + fDist * sin(r);
	
	m_pkCharacter->SetNowWalking(true);

	if (!m_pkCharacter->IsStateMove() && m_pkCharacter->Goto(dest_x, dest_y))
		m_pkCharacter->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	m_dwLastActionTime = get_dword_time();
	return true;
}

bool CMountActor::_UpdateFollowAI()
{
	if (0 == m_pkCharacter->m_pkMobData)
		return false;

	if (!m_pkOwner)
		return false;

	if (0 == m_OMoveSpeed)
	{
		const CMob* mobData = CMobManager::Instance().Get(m_dwVnum);

		if (0 != mobData)
			m_OMoveSpeed = mobData->m_table.sMovingSpeed;
	}
	float	START_FOLLOW_DISTANCE = 300.0f, START_RUN_DISTANCE = 900.0f, RESPAWN_DISTANCE = 4500.f;
	int		APPROACH = 290;						
	
	bool bRun = false;
	
	DWORD currentTime = get_dword_time();

	long ownerX = m_pkOwner->GetX(), ownerY = m_pkOwner->GetY(), charX = m_pkCharacter->GetX(), charY = m_pkCharacter->GetY();

	float fDist = DISTANCE_APPROX(charX - ownerX, charY - ownerY);

	if (fDist >= RESPAWN_DISTANCE)
	{
		float fOwnerRot = m_pkOwner->GetRotation() * 3.141592f / 180.f, fx = -APPROACH * cos(fOwnerRot), fy = -APPROACH * sin(fOwnerRot);
		if (m_pkCharacter->Show(m_pkOwner->GetMapIndex(), ownerX + fx, ownerY + fy))
			return true;
	}
	
	
	if (fDist >= START_FOLLOW_DISTANCE)
	{
		if( fDist >= START_RUN_DISTANCE)
		{
			bRun = true;
		}
		m_pkCharacter->SetNowWalking(!bRun);
		Follow(APPROACH);
		m_pkCharacter->SetLastAttacked(currentTime);
		m_dwLastActionTime = currentTime;
	}
	else 
		m_pkCharacter->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	
	return true;
}


bool CMountActor::Update(DWORD deltaTime)
{
	bool bResult = true;
	if (!m_pkOwner)
		return false;

	if (m_pkOwner->IsDead() || (IsSummoned() && m_pkCharacter->IsDead()) || (IsSummoned() && (m_pkOwner->GetExchange() || m_pkOwner->GetMyShop() || m_pkOwner->GetShopOwner() || m_pkOwner->IsOpenSafebox() || m_pkOwner->IsCubeOpen())))	
	{
		this->Unsummon();
		return true;
	}

	if (this->IsSummoned())
		bResult = bResult && this->_UpdateFollowAI();

	return bResult;
}

bool CMountActor::Follow(float fMinDistance)
{
	if( !m_pkOwner || !m_pkCharacter) 
		return false;

	float fOwnerX = m_pkOwner->GetX(), fOwnerY = m_pkOwner->GetY(), fMountX = m_pkCharacter->GetX(), fMountY = m_pkCharacter->GetY(), fDist = DISTANCE_SQRT(fOwnerX - fMountX, fOwnerY - fMountY);
	
	if (fDist <= fMinDistance)
		return false;

	m_pkCharacter->SetRotationToXY(fOwnerX, fOwnerY);

	float fx, fy, fDistToGo = fDist - fMinDistance;
	GetDeltaByDegree(m_pkCharacter->GetRotation(), fDistToGo, &fx, &fy);
	
	if (!m_pkCharacter->Goto((int)(fMountX+fx+0.5f), (int)(fMountY+fy+0.5f)) )
		return false;

	m_pkCharacter->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);

	return true;
}


CMountSystem::CMountSystem(LPCHARACTER owner)
{
	m_pkOwner = owner;
	m_dwUpdatePeriod = 400;
	m_dwLastUpdateTime = 0;
}

CMountSystem::~CMountSystem()
{
	Destroy();
}


void CMountSystem::Destroy()
{
	for (TMountActorMap::iterator iter = m_MountActorMap.begin(); iter != m_MountActorMap.end(); ++iter)
	{
		CMountActor* MountActor = iter->second;

		if (0 != MountActor)
			delete MountActor;
	}
	event_cancel(&m_pkMountSystemUpdateEvent);
	m_MountActorMap.clear();
}

bool CMountSystem::Update(DWORD deltaTime)
{
	bool bResult = true;

	DWORD currentTime = get_dword_time();
	
	if (m_dwUpdatePeriod > currentTime - m_dwLastUpdateTime)
		return true;
	
	std::vector <CMountActor*> v_garbageActor;

	for (TMountActorMap::iterator iter = m_MountActorMap.begin(); iter != m_MountActorMap.end(); ++iter)
	{
		CMountActor* MountActor = iter->second;

		if (0 != MountActor && MountActor->IsSummoned())
		{
			LPCHARACTER pMount = MountActor->GetCharacter();
			
			if (NULL == CHARACTER_MANAGER::instance().Find(pMount->GetVID()))
				v_garbageActor.push_back(MountActor);
			else
				bResult = bResult && MountActor->Update(deltaTime);
		}
	}
	for (std::vector<CMountActor*>::iterator it = v_garbageActor.begin(); it != v_garbageActor.end(); it++)
		DeleteMount(*it);

	m_dwLastUpdateTime = currentTime;

	return bResult;
}

void CMountSystem::DeleteMount(DWORD mobVnum)
{
	TMountActorMap::iterator iter = m_MountActorMap.find(mobVnum);

	if (m_MountActorMap.end() == iter)
	{
		sys_err("CMountSystem::DeleteMount ERROR CODE [1]");
		return;
	}
	CMountActor* MountActor = iter->second;

	if (0 == MountActor)
		sys_err("CMountSystem::DeleteMount ERROR CODE [2]");
	else
		delete MountActor;

	m_MountActorMap.erase(iter);	
}

void CMountSystem::DeleteMount(CMountActor* MountActor)
{
	for (TMountActorMap::iterator iter = m_MountActorMap.begin(); iter != m_MountActorMap.end(); ++iter)
	{
		if (iter->second == MountActor)
		{
			delete MountActor;
			m_MountActorMap.erase(iter);

			return;
		}
	}

	sys_err("CMountSystem::DeleteMount ERROR CODE [2]");
}

void CMountSystem::Unsummon(DWORD vnum, bool bDeleteFromList)
{
	CMountActor* actor = this->GetByVnum(vnum);
	if (0 == actor)
	{
		sys_err("[CMountSystem::GetByVnum(%d)] Null Pointer (MountActor)", vnum);
		return;
	}
	actor->Unsummon();

	if (true == bDeleteFromList)
		this->DeleteMount(actor);

	bool bActive = false;
	for (TMountActorMap::iterator it = m_MountActorMap.begin(); it != m_MountActorMap.end(); it++)
		bActive |= it->second->IsSummoned();
	
	if (false == bActive)
	{
		event_cancel(&m_pkMountSystemUpdateEvent);
		m_pkMountSystemUpdateEvent = NULL;
	}
}

bool CMountSystem::IsActiveMount()
{
	bool state = false;
	for (TMountActorMap::iterator iter = m_MountActorMap.begin(); iter != m_MountActorMap.end(); ++iter)
	{
		CMountActor* MountActor = iter->second;
		if (MountActor != 0)
		{
			if (MountActor->IsSummoned()) 
			{
				state = true;
				break;
			}			
		}
	}
	return state;
}

bool CMountSystem::IsMounting(DWORD mobVnum)
{
	CMountActor* MountActor = this->GetByVnum(mobVnum);
	if (MountActor)
		return MountActor->IsMounting();
	else
		return false;
}

CMountActor* CMountSystem::Summon(DWORD mobVnum, const char* mName)
{
	CMountActor* MountActor = this->GetByVnum(mobVnum);
	if (0 == MountActor)
	{
		MountActor = M2_NEW CMountActor(m_pkOwner, mobVnum);
		m_MountActorMap.insert(std::make_pair(mobVnum, MountActor));
	}

	MountActor->Summon(mName, mobVnum);

	if (NULL == m_pkMountSystemUpdateEvent)
	{
		mount_event_info* info = AllocEventInfo<mount_event_info>();

		info->pMountSystem = this;

		m_pkMountSystemUpdateEvent = event_create(mount_update_event, info, PASSES_PER_SEC(1) / 4);	
	}
	return MountActor;
}


CMountActor* CMountSystem::Mount(DWORD mobVnum)
{
	CMountActor* MountActor = this->GetByVnum(mobVnum);
	if (MountActor)
		MountActor->Mount();

	return MountActor;
}

CMountActor* CMountSystem::Unmount(DWORD mobVnum)
{
	CMountActor* MountActor = this->GetByVnum(mobVnum);
	if (MountActor)
		MountActor->Unmount();

	return MountActor;
}

CMountActor* CMountSystem::GetByVID(DWORD vid) const
{
	CMountActor* MountActor = 0;
	bool bFound = false;
	
	for (TMountActorMap::const_iterator iter = m_MountActorMap.begin(); iter != m_MountActorMap.end(); ++iter)
	{
		MountActor = iter->second;

		if (0 == MountActor)
		{
			sys_err("CMountSystem:: CMountSystem ERROR[1]", vid);
			continue;
		}

		bFound = MountActor->GetVID() == vid;

		if (true == bFound)
			break;
	}
	return bFound ? MountActor : 0;
}

CMountActor* CMountSystem::GetByVnum(DWORD vnum) const
{
	CMountActor* MountActor = 0;

	TMountActorMap::const_iterator iter = m_MountActorMap.find(vnum);

	if (m_MountActorMap.end() != iter)
		MountActor = iter->second;

	return MountActor;
}

size_t CMountSystem::CountSummoned() const
{
	size_t count = 0;

	for (TMountActorMap::const_iterator iter = m_MountActorMap.begin(); iter != m_MountActorMap.end(); ++iter)
	{
		CMountActor* MountActor = iter->second;

		if (0 != MountActor)
			if (MountActor->IsSummoned())
				++count;
	}
	return count;
}

