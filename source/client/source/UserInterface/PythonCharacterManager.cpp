#include "stdafx.h"
#include "pythoncharactermanager.h"
#include "PythonBackground.h"
#include "PythonNonPlayer.h"
#include "AbstractPlayer.h"
#include "packet.h"
#include "AbstractApplication.h"
#include "InstanceBase.h"
#include "../EterLib/Camera.h"
int CHAR_STAGE_VIEW_BOUND = 200*100;

struct FCharacterManagerCharacterInstanceUpdate
{
	inline void operator () (const pair<DWORD,CInstanceBase *>& cr_Pair)
	{
		cr_Pair.second->Update();
	}
};

void CPythonCharacterManager::AdjustCollisionWithOtherObjects(CActorInstance* pInst )
{
	if( !pInst->IsPC() )
		return;

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	for(CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd();++i)
	{
		CInstanceBase*  pkInstEach=*i;
		CActorInstance* rkActorEach=pkInstEach->GetGraphicThingInstancePtr();

		if (rkActorEach==pInst)
			continue;

		if( rkActorEach->IsPC() || rkActorEach->IsNPC() || rkActorEach->IsEnemy() || rkActorEach->IsPet()
#ifdef ENABLE_GROWTH_PET_SYSTEM
|| rkActorEach->IsNewPet()
#endif
)
			continue;

		if(pInst->TestPhysicsBlendingCollision(*rkActorEach) )
		{
			TPixelPosition curPos;
			pInst->GetPixelPosition(&curPos);
			pInst->SetBlendingPosition(curPos);

			break;
		}
	}
}
void CPythonCharacterManager::EnableSortRendering(bool isEnable)
{
}

void CPythonCharacterManager::InsertPVPKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	CInstanceBase::InsertPVPKey(dwVIDSrc, dwVIDDst);

	CInstanceBase* pkInstSrc=GetInstancePtr(dwVIDSrc);
	if (pkInstSrc)
		pkInstSrc->RefreshTextTail();

	CInstanceBase* pkInstDst=GetInstancePtr(dwVIDDst);
	if (pkInstDst)
		pkInstDst->RefreshTextTail();
}

void CPythonCharacterManager::RemovePVPKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	CInstanceBase::RemovePVPKey(dwVIDSrc, dwVIDDst);

	CInstanceBase* pkInstSrc=GetInstancePtr(dwVIDSrc);
	if (pkInstSrc)
		pkInstSrc->RefreshTextTail();

	CInstanceBase* pkInstDst=GetInstancePtr(dwVIDDst);
	if (pkInstDst)
		pkInstDst->RefreshTextTail();
}

void CPythonCharacterManager::ChangeGVG(DWORD dwSrcGuildID, DWORD dwDstGuildID)
{
	TCharacterInstanceMap::iterator itor;
	for (itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); itor++)
	{
		CInstanceBase * pInstance = itor->second;

		DWORD dwInstanceGuildID = pInstance->GetGuildID();
		if (dwSrcGuildID == dwInstanceGuildID || dwDstGuildID == dwInstanceGuildID)
		{
			pInstance->RefreshTextTail();
		}
	}
}

void CPythonCharacterManager::ClearMainInstance()
{
	m_pkInstMain=NULL;
}

bool CPythonCharacterManager::SetMainInstance(DWORD dwVID)
{
	m_pkInstMain=GetInstancePtr(dwVID);

	if (!m_pkInstMain)
		return false;

	return true;
}

CInstanceBase* CPythonCharacterManager::GetMainInstancePtr()
{
	return m_pkInstMain;
}

void CPythonCharacterManager::GetInfo(std::string* pstInfo)
{
	pstInfo->append("Actor: ");

	CInstanceBase::GetInfo(pstInfo);

	char szInfo[256];
	sprintf(szInfo, "Container - Live %d, Dead %d", m_kAliveInstMap.size(), m_kDeadInstList.size());
	pstInfo->append(szInfo);
}
bool CPythonCharacterManager::IsCacheMode()
{
	static bool s_isOldCacheMode=false;

	bool isCacheMode=s_isOldCacheMode;
	if (s_isOldCacheMode)
	{
		if (m_kAliveInstMap.size()<30)
			isCacheMode=false;
	}
	else
	{
		if (m_kAliveInstMap.size()>40)
			isCacheMode=true;
	}
	s_isOldCacheMode=isCacheMode;

	return isCacheMode;
}

void CPythonCharacterManager::Update()
{
	CInstanceBase::ResetPerformanceCounter();

	CInstanceBase* pkInstMain=GetMainInstancePtr();

	DWORD dwDeadInstCount=0;
	DWORD dwForceVisibleInstCount=0;

	TCharacterInstanceMap::iterator i=m_kAliveInstMap.begin();
	while (m_kAliveInstMap.end()!=i)
	{
		TCharacterInstanceMap::iterator c=i++;

		CInstanceBase* pkInstEach=c->second;
		pkInstEach->Update();

		if (pkInstMain)
		{
			if (pkInstEach->IsForceVisible())
			{
				dwForceVisibleInstCount++;

			}

			int nDistance = int(pkInstEach->NEW_GetDistanceFromDestInstance(*pkInstMain));
			if (nDistance > CHAR_STAGE_VIEW_BOUND + 10)
			{
				__DeleteBlendOutInstance(pkInstEach);
				m_kAliveInstMap.erase(c);
				dwDeadInstCount++;
			}
		}
	}
	UpdateTransform();

	UpdateDeleting();
	__NEW_Pick();
}

void CPythonCharacterManager::ShowPointEffect(DWORD ePoint, DWORD dwVID)
{
	CInstanceBase * pkInstSel = (dwVID == 0xffffffff) ? GetMainInstancePtr() : GetInstancePtr(dwVID);

	if (!pkInstSel)
		return;

	switch (ePoint)
	{
		case POINT_LEVEL:
			pkInstSel->LevelUp();
			break;
		case POINT_LEVEL_STEP:
			pkInstSel->SkillUp();
			break;
	}
}

bool CPythonCharacterManager::RegisterPointEffect(DWORD ePoint, const char* c_szFileName)
{
	if (ePoint>=POINT_MAX_NUM)
		return false;

	CEffectManager& rkEftMgr=CEffectManager::Instance();
	rkEftMgr.RegisterEffect2(c_szFileName, &m_adwPointEffect[ePoint]);

	return true;
}

void CPythonCharacterManager::UpdateTransform()
{
	CInstanceBase * pMainInstance = GetMainInstancePtr();
	if (pMainInstance)
	{
		CPythonBackground& rkBG=CPythonBackground::Instance();
		for (TCharacterInstanceMap::iterator i = m_kAliveInstMap.begin(); i != m_kAliveInstMap.end(); ++i)
		{
			CInstanceBase * pSrcInstance = i->second;

			pSrcInstance->CheckAdvancing();
			if (pSrcInstance->IsPushing())
				rkBG.CheckAdvancing(pSrcInstance);
		}

#ifdef __MOVIE_MODE__
		if (!m_pkInstMain->IsMovieMode())
		{
			rkBG.CheckAdvancing(m_pkInstMain);
		}
#else
		rkBG.CheckAdvancing(m_pkInstMain);
#endif
	}

	{
		for (TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); ++itor)
		{
			CInstanceBase * pInstance = itor->second;
			pInstance->Transform();
		}
	}
}
void CPythonCharacterManager::UpdateDeleting()
{
	TCharacterInstanceList::iterator itor = m_kDeadInstList.begin();
	for (; itor != m_kDeadInstList.end();)
	{
		CInstanceBase * pInstance = *itor;

		if (pInstance->UpdateDeleting())
		{
			++itor;
		}
		else
		{
			CInstanceBase::Delete(pInstance);
			itor = m_kDeadInstList.erase(itor);
		}
	}
}

struct FCharacterManagerCharacterInstanceDeform
{
	inline void operator () (const pair<DWORD,CInstanceBase *>& cr_Pair)
	{
		cr_Pair.second->Deform();

	}
};
struct FCharacterManagerCharacterInstanceListDeform
{
	inline void operator () (CInstanceBase * pInstance)
	{
		pInstance->Deform();
	}
};

void CPythonCharacterManager::Deform()
{
	std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), FCharacterManagerCharacterInstanceDeform());
	std::for_each(m_kDeadInstList.begin(), m_kDeadInstList.end(), FCharacterManagerCharacterInstanceListDeform());
}
bool CPythonCharacterManager::OLD_GetPickedInstanceVID(DWORD* pdwPickedActorID)
{
	if (!m_pkInstPick)
		return false;

	*pdwPickedActorID=m_pkInstPick->GetVirtualID();
	return true;
}

CInstanceBase * CPythonCharacterManager::OLD_GetPickedInstancePtr()
{
	return m_pkInstPick;
}

D3DXVECTOR2 & CPythonCharacterManager::OLD_GetPickedInstPosReference()
{
	return m_v2PickedInstProjPos;
}

bool CPythonCharacterManager::IsRegisteredVID(DWORD dwVID)
{
	if (m_kAliveInstMap.end()==m_kAliveInstMap.find(dwVID))
		return false;

	return true;
}

bool CPythonCharacterManager::IsAliveVID(DWORD dwVID)
{
	return m_kAliveInstMap.find(dwVID)!=m_kAliveInstMap.end();
}

bool CPythonCharacterManager::IsDeadVID(DWORD dwVID)
{
	for (TCharacterInstanceList::iterator f=m_kDeadInstList.begin(); f!=m_kDeadInstList.end(); ++f)
	{
		if ((*f)->GetVirtualID()==dwVID)
			return true;
	}

	return false;
}

struct LessCharacterInstancePtrRenderOrder
{
	bool operator() (CInstanceBase* pkLeft, CInstanceBase* pkRight)
	{
		return pkLeft->LessRenderOrder(pkRight);
	}
};

struct FCharacterManagerCharacterInstanceRender
{
	inline void operator () (const pair<DWORD,CInstanceBase *>& cr_Pair)
	{
		cr_Pair.second->Render();
		cr_Pair.second->RenderTrace();
	}
};
struct FCharacterInstanceRender
{
	inline void operator () (CInstanceBase * pInstance)
	{
		pInstance->Render();
	}
};
struct FCharacterInstanceRenderTrace
{
	inline void operator () (CInstanceBase * pInstance)
	{
		pInstance->RenderTrace();
	}
};
void CPythonCharacterManager::__RenderSortedAliveActorList()
{
	static std::vector<CInstanceBase*> s_kVct_pkInstAliveSort;
	s_kVct_pkInstAliveSort.clear();

	TCharacterInstanceMap& rkMap_pkInstAlive=m_kAliveInstMap;
	TCharacterInstanceMap::iterator i;
	for (i=rkMap_pkInstAlive.begin(); i!=rkMap_pkInstAlive.end(); ++i)
		s_kVct_pkInstAliveSort.push_back(i->second);

	std::sort(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(), LessCharacterInstancePtrRenderOrder());
	std::for_each(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(), FCharacterInstanceRender());
	std::for_each(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(), FCharacterInstanceRenderTrace());
}

void CPythonCharacterManager::__RenderSortedDeadActorList()
{
	static std::vector<CInstanceBase*> s_kVct_pkInstDeadSort;
	s_kVct_pkInstDeadSort.clear();

	TCharacterInstanceList& rkLst_pkInstDead=m_kDeadInstList;
	TCharacterInstanceList::iterator i;
	for (i=rkLst_pkInstDead.begin(); i!=rkLst_pkInstDead.end(); ++i)
		s_kVct_pkInstDeadSort.push_back(*i);

	std::sort(s_kVct_pkInstDeadSort.begin(), s_kVct_pkInstDeadSort.end(), LessCharacterInstancePtrRenderOrder());
	std::for_each(s_kVct_pkInstDeadSort.begin(), s_kVct_pkInstDeadSort.end(), FCharacterInstanceRender());

}

void CPythonCharacterManager::Render()
{
	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG1);

	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,	D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);
	__RenderSortedAliveActorList();
	__RenderSortedDeadActorList();

	CInstanceBase * pkPickedInst = OLD_GetPickedInstancePtr();
	if (pkPickedInst)
	{
		const D3DXVECTOR3 & c_rv3Position = pkPickedInst->GetGraphicThingInstanceRef().GetPosition();
		CPythonGraphic::Instance().ProjectPosition(c_rv3Position.x, c_rv3Position.y, c_rv3Position.z, &m_v2PickedInstProjPos.x, &m_v2PickedInstProjPos.y);
	}
}

void CPythonCharacterManager::RenderShadowMainInstance()
{
	CInstanceBase* pkInstMain=GetMainInstancePtr();
	if (pkInstMain)
		pkInstMain->RenderToShadowMap();
}

struct FCharacterManagerCharacterInstanceRenderToShadowMap
{
	inline void operator () (const pair<DWORD,CInstanceBase *>& cr_Pair)
	{
		cr_Pair.second->RenderToShadowMap();
	}
};

void CPythonCharacterManager::RenderShadowAllInstances()
{
	std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), FCharacterManagerCharacterInstanceRenderToShadowMap());
}

struct FCharacterManagerCharacterInstanceRenderCollision
{
	inline void operator () (const pair<DWORD,CInstanceBase *>& cr_Pair)
	{
		cr_Pair.second->RenderCollision();
	}
};

void CPythonCharacterManager::RenderCollision()
{
 	std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), FCharacterManagerCharacterInstanceRenderCollision());
}
CInstanceBase * CPythonCharacterManager::CreateInstance(const CInstanceBase::SCreateData& c_rkCreateData)
{
	CInstanceBase * pCharacterInstance = RegisterInstance(c_rkCreateData.m_dwVID);
	if (!pCharacterInstance)
	{
		TraceError("CPythonCharacterManager::CreateInstance: VID[%d] - ALREADY EXIST\n", c_rkCreateData.m_dwVID); 
		return NULL;
	}

	if (!pCharacterInstance->Create(c_rkCreateData))
	{
		TraceError("CPythonCharacterManager::CreateInstance VID[%d] Race[%d]", c_rkCreateData.m_dwVID, c_rkCreateData.m_dwRace);
		DeleteInstance(c_rkCreateData.m_dwVID);
		return NULL;
	}

	if (c_rkCreateData.m_isMain)
		SelectInstance(c_rkCreateData.m_dwVID);

	return (pCharacterInstance);
}

CInstanceBase * CPythonCharacterManager::RegisterInstance(DWORD VirtualID)
{
	TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(VirtualID);

	if (m_kAliveInstMap.end() != itor)
	{
		return NULL;
	}

	CInstanceBase * pCharacterInstance = CInstanceBase::New();
	m_kAliveInstMap.insert(TCharacterInstanceMap::value_type(VirtualID, pCharacterInstance));
#ifdef ENABLE_AUTO_SYSTEM
	if (VirtualID > m_VIDMax)
		m_VIDMax = VirtualID;
#endif
	return (pCharacterInstance);
}

void CPythonCharacterManager::DeleteInstance(DWORD dwDelVID)
{
	TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(dwDelVID);

	if (m_kAliveInstMap.end() == itor)
	{
		Tracef("DeleteCharacterInstance: no vid by %d\n", dwDelVID);
		return;
	}

	CInstanceBase * pkInstDel = itor->second;

	if (pkInstDel == m_pkInstBind)
		m_pkInstBind = NULL;

	if (pkInstDel == m_pkInstMain)
		m_pkInstMain = NULL;

	if (pkInstDel == m_pkInstPick)
		m_pkInstPick = NULL;

	CInstanceBase::Delete(pkInstDel);

	m_kAliveInstMap.erase(itor);
}

#ifdef ENABLE_GRAPHIC_ON_OFF
void CPythonCharacterManager::SetPrivateShopLevel(unsigned int level)
{
	CInstanceBase * pMainInstance = GetMainInstancePtr();
	for (auto it : m_kAliveInstMap)
	{
		CInstanceBase * pInstance = it.second;
		if (pInstance && pMainInstance != pInstance)
		{
			if (pInstance->IsPrivateShop())
			{
				if (level)
					pInstance->Hide();
				else
					pInstance->Show();

				SCRIPT_SetAffect(pInstance->GetVirtualID(), CInstanceBase::AFFECT_INVISIBILITY, level);
			}
		}
	}
}

void CPythonCharacterManager::SetEffectLevel(unsigned int level)
{
	CInstanceBase * pMainInstance = GetMainInstancePtr();
	for (auto it : m_kAliveInstMap)
	{
		CInstanceBase * pInstance = it.second;
		if (pInstance && pMainInstance)
		{
			if (level != 4)
			{
				if (level == 0)
					pInstance->GetGraphicThingInstancePtr()->ShowAllAttachingEffect();

				if (level == 1 && (pMainInstance != pInstance || !pInstance->IsEnemy()))
					pInstance->GetGraphicThingInstancePtr()->HideAllAttachingEffect();
				else
					pInstance->GetGraphicThingInstancePtr()->ShowAllAttachingEffect();

				if (level == 3 && pMainInstance != pInstance)
					pInstance->GetGraphicThingInstancePtr()->HideAllAttachingEffect();
				else
					pInstance->GetGraphicThingInstancePtr()->ShowAllAttachingEffect();

				if (level == 2 && (pMainInstance != pInstance || pInstance->IsEnemy()))
					pInstance->GetGraphicThingInstancePtr()->HideAllAttachingEffect();
				else
					pInstance->GetGraphicThingInstancePtr()->ShowAllAttachingEffect();
			}
			else
				pInstance->GetGraphicThingInstancePtr()->HideAllAttachingEffect();
		}
	}
}

void CPythonCharacterManager::SetPetStatus(bool bStatus)
{
	CInstanceBase * pMainInstance = GetMainInstancePtr();

	for (auto it : m_kAliveInstMap)
	{
		CInstanceBase * pInstance = it.second;
		if (pInstance && pMainInstance != pInstance && (pInstance->IsPet()
#ifdef ENABLE_GROWTH_PET_SYSTEM
|| pInstance->IsNewPet()
#endif
))
		{
			if (bStatus)
				pInstance->Hide();
			else
				pInstance->Show();

			SCRIPT_SetAffect(pInstance->GetVirtualID(), CInstanceBase::AFFECT_INVISIBILITY, bStatus);
		}
	}
}

void CPythonCharacterManager::SetNpcNameStatus(bool bStatus)
{
	CInstanceBase * pMainInstance = GetMainInstancePtr();

	for (auto it : m_kAliveInstMap)
	{
		CInstanceBase * pInstance = it.second;
		if (pInstance && pMainInstance != pInstance && pInstance->IsNPC())
		{
			if (bStatus)
				pInstance->DetachTextTail();
			else
			{
				pInstance->AttachTextTail();
				pInstance->RefreshTextTail();
			}
		}
	}
}
#endif

void CPythonCharacterManager::__DeleteBlendOutInstance(CInstanceBase* pkInstDel)
{
	pkInstDel->DeleteBlendOut();
	m_kDeadInstList.push_back(pkInstDel);

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.NotifyCharacterDead(pkInstDel->GetVirtualID());
}

void CPythonCharacterManager::DeleteInstanceByFade(DWORD dwVID)
{
	TCharacterInstanceMap::iterator f = m_kAliveInstMap.find(dwVID);
	if (m_kAliveInstMap.end() == f)
		return;

	__DeleteBlendOutInstance(f->second);
	m_kAliveInstMap.erase(f);
}

#ifdef ENABLE_TAB_NEXT_TARGET
CInstanceBase * CPythonCharacterManager::GetTabNextTargetPointer(CInstanceBase * pkInstMain)
{
	if (!pkInstMain)
	{
		ResetTabNextTargetVectorIndex();
		return NULL;
	}
	
	struct FCharacterManagerInstanceTarget
	{
		CInstanceBase * pkInstMain;
		FCharacterManagerInstanceTarget(CInstanceBase * pInstance) : pkInstMain(pInstance) {}

		inline void operator () (const std::pair<DWORD, CInstanceBase *>& itor)
		{
			CInstanceBase * pkInstTarget = itor.second;
			if (!pkInstTarget || pkInstTarget == pkInstMain || !pkInstTarget->IsEnemy())
				return;

			const float fRadiusDistance = pkInstMain->GetDistance(pkInstTarget);
			if (fRadiusDistance < 1500.0f)
				m_vecTargetInstance.push_back(pkInstTarget);
		}

		std::vector<CInstanceBase *> m_vecTargetInstance;
	};

	FCharacterManagerInstanceTarget f(pkInstMain);
	f = std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), f);

	const size_t kTargetCount = f.m_vecTargetInstance.size();
	if (kTargetCount == 0)
	{
		ResetTabNextTargetVectorIndex();
		return NULL;
	}

	if (GetTabNextTargetVectorIndex() >= kTargetCount - 1)
		ResetTabNextTargetVectorIndex();

	return f.m_vecTargetInstance.at(++m_adwVectorIndexTabNextTarget);
}
#endif

#ifdef ENABLE_AUTO_SYSTEM
void CPythonCharacterManager::SetDistanceVector()
{
	m_DistanceMap.clear();

	for (int i = 0; i <= m_VIDMax; ++i)
	{
		float fDistance = 0.0f;
		CInstanceBase * pMainInstance = GetMainInstancePtr();
		CInstanceBase * pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(i);
		if (!pMainInstance || !pTargetInstance)
			fDistance = 0.0f;
		else
		{
			fDistance = pMainInstance->GetDistance(pTargetInstance);

			if (pTargetInstance->IsEnemy() && !pTargetInstance->IsDead())
				m_DistanceMap.insert(make_pair(i, fDistance));
		}
	}
}

DWORD CPythonCharacterManager::GetDistanceVID()
{
	float distance = 2000.0f;
	DWORD vid = 0;

	for (int i = 0; i <= m_VIDMax; ++i)
		if (m_DistanceMap.count(i) == 1)
			if (m_DistanceMap.at(i) < distance && m_DistanceMap.at(i) > 0)
			{
				distance = m_DistanceMap.at(i);
				vid = i;
			}

	return vid;
}
#endif

void CPythonCharacterManager::SelectInstance(DWORD VirtualID)
{
	TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(VirtualID);

	if (m_kAliveInstMap.end() == itor)
	{
		Tracef("SelectCharacterInstance: no vid by %d\n", VirtualID);
		return;
	}

	m_pkInstBind = itor->second;
}

CInstanceBase * CPythonCharacterManager::GetInstancePtr(DWORD VirtualID)
{
	TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(VirtualID);

	if (m_kAliveInstMap.end() == itor)
		return NULL;

	return itor->second;
}

CInstanceBase * CPythonCharacterManager::GetInstancePtrByName(const char *name)
{
	TCharacterInstanceMap::iterator itor;

	for (itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); itor++)
	{
		CInstanceBase * pInstance = itor->second;

		if (!strcmp(pInstance->GetNameString(), name))
			return pInstance;
	}

	return NULL;
}

CInstanceBase * CPythonCharacterManager::GetSelectedInstancePtr()
{
	return m_pkInstBind;
}

CInstanceBase* CPythonCharacterManager::FindClickableInstancePtr()
{
	return NULL;
}

void CPythonCharacterManager::__UpdateSortPickedActorList()
{
	__UpdatePickedActorList();
	__SortPickedActorList();
}

void CPythonCharacterManager::__UpdatePickedActorList()
{
	m_kVct_pkInstPicked.clear();

	TCharacterInstanceMap::iterator i;
	for (i=m_kAliveInstMap.begin(); i!=m_kAliveInstMap.end(); ++i)
	{
		CInstanceBase* pkInstEach=i->second;

		if (pkInstEach->CanPickInstance())
		{
			if (pkInstEach->IsDead())
			{
				if (pkInstEach->IntersectBoundingBox())
					m_kVct_pkInstPicked.push_back(pkInstEach);
			}
			else
			{
				if (pkInstEach->IntersectDefendingSphere())
					m_kVct_pkInstPicked.push_back(pkInstEach);
			}
		}
	}
}

struct CInstanceBase_SLessCameraDistance
{
	TPixelPosition m_kPPosEye;

	bool operator() (CInstanceBase* pkInstLeft, CInstanceBase* pkInstRight)
	{
		int nLeftDeadPoint=pkInstLeft->IsDead();
		int nRightDeadPoint=pkInstRight->IsDead();

		if (nLeftDeadPoint<nRightDeadPoint)
			return true;

		if (pkInstLeft->CalculateDistanceSq3d(m_kPPosEye)<pkInstRight->CalculateDistanceSq3d(m_kPPosEye))
			return true;

		return false;
	}
};

void CPythonCharacterManager::__SortPickedActorList()
{
	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	const D3DXVECTOR3& c_rv3EyePos=pCamera->GetEye();

	CInstanceBase_SLessCameraDistance kLess;
	kLess.m_kPPosEye=TPixelPosition(+c_rv3EyePos.x, -c_rv3EyePos.y, +c_rv3EyePos.z);

	std::sort(m_kVct_pkInstPicked.begin(), m_kVct_pkInstPicked.end(), kLess);
}

void CPythonCharacterManager::__NEW_Pick()
{
	__UpdateSortPickedActorList();

	CInstanceBase* pkInstMain=GetMainInstancePtr();

#ifdef __MOVIE_MODE
	if (pkInstMain)
		if (pkInstMain->IsMovieMode())
		{
			if (m_pkInstPick)
				m_pkInstPick->OnUnselected();
			return;
		}
#endif
	{
		std::vector<CInstanceBase*>::iterator f;
		for (f=m_kVct_pkInstPicked.begin(); f!=m_kVct_pkInstPicked.end(); ++f)
		{
			CInstanceBase* pkInstEach=*f;
			if (pkInstEach!=pkInstMain && pkInstEach->IntersectBoundingBox())
			{
				if (m_pkInstPick)
					if (m_pkInstPick!=pkInstEach)
						m_pkInstPick->OnUnselected();

				if (pkInstEach->CanPickInstance())
				{
					m_pkInstPick = pkInstEach;
					m_pkInstPick->OnSelected();
					return;
				}
			}
		}
	}
	{
		std::vector<CInstanceBase*>::iterator f;
		for (f=m_kVct_pkInstPicked.begin(); f!=m_kVct_pkInstPicked.end(); ++f)
		{
			CInstanceBase* pkInstEach=*f;
			if (pkInstEach!=pkInstMain)
			{
				if (m_pkInstPick)
					if (m_pkInstPick!=pkInstEach)
						m_pkInstPick->OnUnselected();

				if (pkInstEach->CanPickInstance())
				{
					m_pkInstPick = pkInstEach;
					m_pkInstPick->OnSelected();
					return;
				}
			}
		}
	}

	if (pkInstMain)
	if (pkInstMain->CanPickInstance())
	if (m_kVct_pkInstPicked.end() != std::find(m_kVct_pkInstPicked.begin(), m_kVct_pkInstPicked.end(), pkInstMain))
	{
		if (m_pkInstPick)
			if (m_pkInstPick!=pkInstMain)
				m_pkInstPick->OnUnselected();

		m_pkInstPick = pkInstMain;
		m_pkInstPick->OnSelected();
		return;
	}

	if (m_pkInstPick)
	{
		m_pkInstPick->OnUnselected();
		m_pkInstPick=NULL;
	}
}

void CPythonCharacterManager::__OLD_Pick()
{
	for (TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); ++itor)
	{
		CInstanceBase * pkInstEach = itor->second;

		if (pkInstEach == m_pkInstMain)
			continue;

		if (pkInstEach->IntersectDefendingSphere())
		{
			if (m_pkInstPick)
				if (m_pkInstPick!=pkInstEach)
					m_pkInstPick->OnUnselected();

			m_pkInstPick = pkInstEach;
			m_pkInstPick->OnSelected();

			return;
		}
	}

	if (m_pkInstPick)
	{
		m_pkInstPick->OnUnselected();
		m_pkInstPick=NULL;
	}
}

int CPythonCharacterManager::PickAll()
{
	for (TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); ++itor)
	{
		CInstanceBase * pInstance = itor->second;

		if (pInstance->IntersectDefendingSphere())
			return pInstance->GetVirtualID();
	}

	return -1;
}

CInstanceBase * CPythonCharacterManager::GetCloseInstance(CInstanceBase * pInstance)
{
	float fMinDistance = 10000.0f;
	CInstanceBase * pCloseInstance = NULL;

	TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin();
	for (; itor != m_kAliveInstMap.end(); ++itor)
	{
		CInstanceBase * pTargetInstance = itor->second;

		if (pTargetInstance == pInstance)
			continue;

		DWORD dwVirtualNumber = pTargetInstance->GetVirtualNumber();
		if (CPythonNonPlayer::ON_CLICK_EVENT_BATTLE != CPythonNonPlayer::Instance().GetEventType(dwVirtualNumber))
			continue;

		float fDistance = pInstance->GetDistance(pTargetInstance);
		if (fDistance < fMinDistance)
		{
			fMinDistance = fDistance;
			pCloseInstance = pTargetInstance;
		}
	}

	return pCloseInstance;
}

void CPythonCharacterManager::RefreshAllPCTextTail()
{
	CPythonCharacterManager::CharacterIterator itor = CharacterInstanceBegin();
	CPythonCharacterManager::CharacterIterator itorEnd = CharacterInstanceEnd();
	for (; itor != itorEnd; ++itor)
	{
		CInstanceBase * pInstance = *itor;
		if (!pInstance->IsPC())
			continue;

		pInstance->RefreshTextTail();
	}
}

void CPythonCharacterManager::RefreshAllGuildMark()
{
	CPythonCharacterManager::CharacterIterator itor = CharacterInstanceBegin();
	CPythonCharacterManager::CharacterIterator itorEnd = CharacterInstanceEnd();
	for (; itor != itorEnd; ++itor)
	{
		CInstanceBase * pInstance = *itor;
		if (!pInstance->IsPC())
			continue;

		pInstance->ChangeGuild(pInstance->GetGuildID()
#ifdef ENABLE_GUILD_GENERAL_AND_LEADER
, pInstance->GetNewIsGuildName()
#endif
);
		pInstance->RefreshTextTail();
	}
}

void CPythonCharacterManager::DeleteAllInstances()
{
	DestroyAliveInstanceMap();
	DestroyDeadInstanceList();
}
void CPythonCharacterManager::DestroyAliveInstanceMap()
{
	for (TCharacterInstanceMap::iterator i = m_kAliveInstMap.begin(); i != m_kAliveInstMap.end(); ++i)
		CInstanceBase::Delete(i->second);

	m_kAliveInstMap.clear();
}

void CPythonCharacterManager::DestroyDeadInstanceList()
{
	std::for_each(m_kDeadInstList.begin(), m_kDeadInstList.end(), CInstanceBase::Delete);
	m_kDeadInstList.clear();
}

void CPythonCharacterManager::Destroy()
{
	DeleteAllInstances();

	CInstanceBase::DestroySystem();

	__Initialize();
}

void CPythonCharacterManager::__Initialize()
{
	memset(m_adwPointEffect, 0, sizeof(m_adwPointEffect));
	m_pkInstMain = NULL;
	m_pkInstBind = NULL;
	m_pkInstPick = NULL;
	m_v2PickedInstProjPos = D3DXVECTOR2(0.0f, 0.0f);
#if defined(ENABLE_TAB_NEXT_TARGET)
	ResetTabNextTargetVectorIndex();
#endif
#ifdef ENABLE_AUTO_SYSTEM
	m_VIDMax = 0;
#endif
}
CPythonCharacterManager::CPythonCharacterManager()
{
	__Initialize();
}

CPythonCharacterManager::~CPythonCharacterManager()
{
	Destroy();
}
