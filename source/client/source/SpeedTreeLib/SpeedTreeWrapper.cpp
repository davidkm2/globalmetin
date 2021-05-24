
#pragma warning(disable:4786)
#include "StdAfx.h"

#include <stdlib.h>
#include <stdio.h>
#include "../EterBase/Debug.h"
#include "../EterBase/Timer.h"
#include "../EterBase/Filename.h"
#include "../EterLib/ResourceManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/StateManager.h"

#include "SpeedTreeConfig.h"
#include "SpeedTreeForestDirectX9.h"
#include "SpeedTreeWrapper.h"
#include "VertexShaders.h"

using namespace std;

LPDIRECT3DVERTEXDECLARATION9 CSpeedTreeWrapper::ms_dwBranchVertexShader = 0;
LPDIRECT3DVERTEXDECLARATION9 CSpeedTreeWrapper::ms_dwLeafVertexShader = 0;
bool CSpeedTreeWrapper::ms_bSelfShadowOn = true;
CSpeedTreeWrapper::CSpeedTreeWrapper() :
m_pSpeedTree(new CSpeedTreeRT),
m_bIsInstance(false),
m_pInstanceOf(NULL),
m_pGeometryCache(NULL),
m_usNumLeafLods(0),
m_pBranchIndexCounts(NULL),
m_pBranchIndexBuffer(NULL),
m_pBranchVertexBuffer(NULL),
m_pFrondIndexCounts(NULL),
m_pFrondIndexBuffer(NULL),
m_pFrondVertexBuffer(NULL),
m_pLeafVertexBuffer(NULL),
m_pLeavesUpdatedByCpu(NULL),
m_unBranchVertexCount(0),
m_unFrondVertexCount(0),
m_pTextureInfo(NULL)
{

	m_afPos[0] = m_afPos[1] = m_afPos[2] = 0.0f;

	m_pSpeedTree->SetWindStrength(1.0f);
	m_pSpeedTree->SetLocalMatrices(0, 4);
}

void CSpeedTreeWrapper::SetVertexShaders(LPDIRECT3DVERTEXDECLARATION9 dwBranchVertexShader, LPDIRECT3DVERTEXDECLARATION9 dwLeafVertexShader)
{
	ms_dwBranchVertexShader = dwBranchVertexShader;
	ms_dwLeafVertexShader = dwLeafVertexShader;
}

void CSpeedTreeWrapper::OnRenderPCBlocker()
{
	if (ms_dwBranchVertexShader == 0)
	{
		ms_dwBranchVertexShader = LoadBranchShader(ms_lpd3dDevice);

	}

	if (ms_dwLeafVertexShader == 0)
	{
		ms_dwLeafVertexShader = LoadLeafShader(ms_lpd3dDevice);

	}

	CSpeedTreeForestDirectX9::Instance().UpdateSystem(ELTimer_GetMSec() / 1000.0f);
	m_pSpeedTree->SetLodLevel(1.0f);
	CSpeedTreeForestDirectX9::Instance().UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	DWORD dwLighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	DWORD dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	DWORD dwAlphaBlendEnable = STATEMANAGER.GetRenderState(D3DRS_ALPHABLENDENABLE);
 	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, TRUE);
    STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);
 	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
	STATEMANAGER.SetVertexDeclaration(ms_dwBranchVertexShader);
	{

#ifdef WRAPPER_USE_CPU_WIND
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

		if (m_pGeometryCache->m_sBranches.m_usNumStrips > 0)
		{

			SFVFBranchVertex* pVertexBuffer = NULL;
			m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
			for (UINT i = 0; i < m_unBranchVertexCount; ++i)
			{
				memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sBranches.m_pCoords[i * 3]), 3 * sizeof(float));
			}
			m_pBranchVertexBuffer->Unlock();
		}
#endif

		LPDIRECT3DTEXTURE9 lpd3dTexture;
		if ((lpd3dTexture = m_BranchImageInstance.GetTextureReference().GetD3DTexture()))
			STATEMANAGER.SetTexture(0, lpd3dTexture);

		if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
		{

			STATEMANAGER.SetStreamSource(0, m_pBranchVertexBuffer, sizeof(SFVFBranchVertex));

			STATEMANAGER.SetIndices(m_pBranchIndexBuffer, 0);
		}
	}

	RenderBranches();

	STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	{

#ifdef WRAPPER_USE_CPU_WIND
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);
		if (m_pGeometryCache->m_sFronds.m_usNumStrips > 0)
		{

			SFVFBranchVertex * pVertexBuffer = NULL;
			m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
			for (UINT i = 0; i < m_unFrondVertexCount; ++i)
			{
				memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sFronds.m_pCoords[i * 3]), 3 * sizeof(float));
			}
			m_pFrondVertexBuffer->Unlock();
		}
#endif

		if (!m_CompositeImageInstance.IsEmpty())
			STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

		if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0)
		{

			STATEMANAGER.SetStreamSource(0, m_pFrondVertexBuffer, sizeof(SFVFBranchVertex));

			STATEMANAGER.SetIndices(m_pFrondIndexBuffer, 0);
		}
	}
	RenderFronds();

	STATEMANAGER.SetVertexDeclaration(ms_dwLeafVertexShader);
	{

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
		UploadLeafTables(c_nVertexShader_LeafTables);
#endif

		if (!m_CompositeImageInstance.IsEmpty())
			STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
	}
	RenderLeaves();
	EndLeafForTreeType();

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, FALSE);
	RenderBillboards();

	STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlendEnable);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLighting);
 	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
}

void CSpeedTreeWrapper::OnRender()
{
	if (ms_dwBranchVertexShader == 0)
	{
		ms_dwBranchVertexShader = LoadBranchShader(ms_lpd3dDevice);

	}

	if (ms_dwLeafVertexShader == 0)
	{
		ms_dwLeafVertexShader = LoadLeafShader(ms_lpd3dDevice);

	}

	CSpeedTreeForestDirectX9::Instance().UpdateSystem(ELTimer_GetMSec() / 1000.0f);
	m_pSpeedTree->SetLodLevel(1.0f);
	CSpeedTreeForestDirectX9::Instance().UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);
	STATEMANAGER.SetVertexDeclaration(ms_dwBranchVertexShader);

	SetupBranchForTreeType();
	RenderBranches();

	STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	SetupFrondForTreeType();
	RenderFronds();

	STATEMANAGER.SetVertexDeclaration(ms_dwLeafVertexShader);

	SetupLeafForTreeType();
	RenderLeaves();
	EndLeafForTreeType();

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, FALSE);
	RenderBillboards();

	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
	STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
}
CSpeedTreeWrapper::~CSpeedTreeWrapper()
{

	if (!m_bIsInstance)
	{
		if (m_unBranchVertexCount > 0)
		{
			SAFE_RELEASE(m_pBranchVertexBuffer);
			SAFE_RELEASE(m_pBranchIndexBuffer);
			SAFE_DELETE_ARRAY(m_pBranchIndexCounts);
		}

		if (m_unFrondVertexCount > 0)
		{
			SAFE_RELEASE(m_pFrondVertexBuffer);
			SAFE_RELEASE(m_pFrondIndexBuffer);
			SAFE_DELETE_ARRAY(m_pFrondIndexCounts);
		}

		for (short i = 0; i < m_usNumLeafLods; ++i)
		{
			m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, i);

			if (m_pGeometryCache->m_sLeaves0.m_usLeafCount > 0)
				SAFE_RELEASE(m_pLeafVertexBuffer[i]);
		}

		SAFE_DELETE_ARRAY(m_pLeavesUpdatedByCpu);
		SAFE_DELETE_ARRAY(m_pLeafVertexBuffer);

		SAFE_DELETE(m_pTextureInfo);

		SAFE_DELETE(m_pGeometryCache);
	}
	SAFE_DELETE(m_pSpeedTree);

	Clear();
}
bool CSpeedTreeWrapper::LoadTree(const char * pszSptFile, const BYTE * c_pbBlock, unsigned int uiBlockSize, UINT nSeed, float fSize, float fSizeVariance)
{
    bool bSuccess = false;
#ifdef WRAPPER_FLIP_T_TEXCOORD
	m_pSpeedTree->SetTextureFlip(true);
#endif
	if (!m_pSpeedTree->LoadTree(c_pbBlock, uiBlockSize))
	{
		if (!m_pSpeedTree->LoadTree(pszSptFile))
		{
			TraceError("SpeedTreeRT Error: %s", CSpeedTreeRT::GetCurrentError());
			return false;
		}
	}
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
	m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
	m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
	m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
#else
	m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
	m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
	m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
#endif
#ifdef WRAPPER_USE_GPU_WIND
	m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_GPU);
	m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_GPU);
	m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_GPU);
#endif
#ifdef WRAPPER_USE_CPU_WIND
	m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_CPU);
	m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_CPU);
	m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_CPU);
#endif
#ifdef WRAPPER_USE_NO_WIND
	m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_NONE);
	m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_NONE);
	m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_NONE);
#endif

	m_pSpeedTree->SetNumLeafRockingGroups(1);
	if (fSize >= 0.0f && fSizeVariance >= 0.0f)
		m_pSpeedTree->SetTreeSize(fSize, fSizeVariance);
	if (m_pSpeedTree->Compute(NULL, nSeed))
	{

		m_pSpeedTree->GetBoundingBox(m_afBoundingBox);
		m_pSpeedTree->SetLeafRockingState(true);
#ifdef WRAPPER_NO_BILLBOARD_MODE
		CSpeedTreeRT::SetDropToBillboard(false);
#else
		CSpeedTreeRT::SetDropToBillboard(true);
#endif
		m_cBranchMaterial.Set(m_pSpeedTree->GetBranchMaterial());
		m_cFrondMaterial.Set(m_pSpeedTree->GetFrondMaterial());
		m_cLeafMaterial.Set(m_pSpeedTree->GetLeafMaterial());
		float fHeight = m_afBoundingBox[5] - m_afBoundingBox[2];
		m_pSpeedTree->SetLodLimits(fHeight * c_fNearLodFactor, fHeight * c_fFarLodFactor);
		m_pTextureInfo = new CSpeedTreeRT::STextures;
		m_pSpeedTree->GetTextures(*m_pTextureInfo);
		LoadTexture((CFileNameHelper::GetPath(string(pszSptFile)) + CFileNameHelper::NoExtension(string(m_pTextureInfo->m_pBranchTextureFilename)) + ".dds").c_str(), m_BranchImageInstance);

#ifdef WRAPPER_RENDER_SELF_SHADOWS
		if (m_pTextureInfo->m_pSelfShadowFilename != NULL)
			LoadTexture((CFileNameHelper::GetPath(string(pszSptFile)) + CFileNameHelper::NoExtension(string(m_pTextureInfo->m_pSelfShadowFilename)) + ".dds").c_str(), m_ShadowImageInstance);
#endif
		if (m_pTextureInfo->m_pCompositeFilename)
			LoadTexture((CFileNameHelper::GetPath(string(pszSptFile)) + CFileNameHelper::NoExtension(string(m_pTextureInfo->m_pCompositeFilename)) + ".dds").c_str(), m_CompositeImageInstance);
		SetupBuffers();
		bSuccess = true;
	}
	else 
		fprintf(stderr, "\nFatal Error, cannot compute tree [%s]\n\n", CSpeedTreeRT::GetCurrentError());

    return bSuccess;
}
void CSpeedTreeWrapper::SetupBuffers(void)
{

	m_pSpeedTree->SetLodLevel(1.0f);

	if (m_pGeometryCache == NULL)
		m_pGeometryCache = new CSpeedTreeRT::SGeometry;

	m_pSpeedTree->GetGeometry(*m_pGeometryCache);
	SetupBranchBuffers();
	SetupFrondBuffers();
	SetupLeafBuffers();
}
void CSpeedTreeWrapper::SetupBranchBuffers(void)
{

	CSpeedTreeRT::SGeometry::SIndexed* pBranches = &(m_pGeometryCache->m_sBranches);
	m_unBranchVertexCount = pBranches->m_usVertexCount; 
	if (m_unBranchVertexCount > 1)
	{

		SFVFBranchVertex * pVertexBuffer = NULL;

#ifndef WRAPPER_USE_CPU_WIND
		ms_lpd3dDevice->CreateVertexBuffer(m_unBranchVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_MANAGED, &m_pBranchVertexBuffer, NULL);

		m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
#else
		ms_lpd3dDevice->CreateVertexBuffer(m_unBranchVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_DYNAMIC, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_SYSTEMMEM, &m_pBranchVertexBuffer, NULL);

		m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
#endif
		{
			for (UINT i = 0; i < m_unBranchVertexCount; ++i)
			{

				memcpy(&pVertexBuffer->m_vPosition, &(pBranches->m_pCoords[i * 3]), 3 * sizeof(float));
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
				memcpy(&pVertexBuffer->m_vNormal, &(pBranches->m_pNormals[i * 3]), 3 * sizeof(float));
#else
				pVertexBuffer->m_dwDiffuseColor = pBranches->m_pColors[i];
#endif
				pVertexBuffer->m_fTexCoords[0] = pBranches->m_pTexCoords0[i * 2];
				pVertexBuffer->m_fTexCoords[1] = pBranches->m_pTexCoords0[i * 2 + 1];
#ifdef WRAPPER_RENDER_SELF_SHADOWS
				pVertexBuffer->m_fShadowCoords[0] = pBranches->m_pTexCoords1[i * 2];
				pVertexBuffer->m_fShadowCoords[1] = pBranches->m_pTexCoords1[i * 2 + 1];
#endif
#ifdef WRAPPER_USE_GPU_WIND
				pVertexBuffer->m_fWindIndex = 4.0f * pBranches->m_pWindMatrixIndices[i];
				pVertexBuffer->m_fWindWeight = pBranches->m_pWindWeights[i];
#endif

				++pVertexBuffer;
			}
			m_pBranchVertexBuffer->Unlock();
		}
		UINT unNumLodLevels = m_pSpeedTree->GetNumBranchLodLevels();
		m_pBranchIndexCounts = new unsigned short[unNumLodLevels];
		for (UINT i = 0; i < unNumLodLevels; ++i)
		{

			m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, i);
			if (pBranches->m_usNumStrips > 0)
				m_pBranchIndexCounts[i] = pBranches->m_pStripLengths[0];
			else
				m_pBranchIndexCounts[i] = 0;
		}

		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, 0);
		ms_lpd3dDevice->CreateIndexBuffer(m_pBranchIndexCounts[0] * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pBranchIndexBuffer, NULL);
		unsigned short* pIndexBuffer = NULL;
		m_pBranchIndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pIndexBuffer), 0);
		memcpy(pIndexBuffer, pBranches->m_pStrips[0], pBranches->m_pStripLengths[0] * sizeof(unsigned short));
		m_pBranchIndexBuffer->Unlock();
	}
}
void CSpeedTreeWrapper::SetupFrondBuffers(void)
{

	CSpeedTreeRT::SGeometry::SIndexed* pFronds = &(m_pGeometryCache->m_sFronds);
	m_unFrondVertexCount = pFronds->m_usVertexCount; 
	if (m_unFrondVertexCount > 1)
	{

		SFVFBranchVertex * pVertexBuffer = NULL;
#ifndef WRAPPER_USE_CPU_WIND
		ms_lpd3dDevice->CreateVertexBuffer(m_unFrondVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_MANAGED, &m_pFrondVertexBuffer, NULL);

		m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
#else
		ms_lpd3dDevice->CreateVertexBuffer(m_unFrondVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_DYNAMIC, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_SYSTEMMEM, &m_pFrondVertexBuffer, NULL);

		m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
#endif
		for (UINT i = 0; i < m_unFrondVertexCount; ++i)
		{

			memcpy(&pVertexBuffer->m_vPosition, &(pFronds->m_pCoords[i * 3]), 3 * sizeof(float));
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
			memcpy(&pVertexBuffer->m_vNormal, &(pFronds->m_pNormals[i * 3]), 3 * sizeof(float));
#else
			pVertexBuffer->m_dwDiffuseColor = pFronds->m_pColors[i];
#endif
			pVertexBuffer->m_fTexCoords[0] = pFronds->m_pTexCoords0[i * 2];
			pVertexBuffer->m_fTexCoords[1] = pFronds->m_pTexCoords0[i * 2 + 1];
#ifdef WRAPPER_RENDER_SELF_SHADOWS
			pVertexBuffer->m_fShadowCoords[0] = pFronds->m_pTexCoords1[i * 2];
			pVertexBuffer->m_fShadowCoords[1] = pFronds->m_pTexCoords1[i * 2 + 1];
#endif
#ifdef WRAPPER_USE_GPU_WIND
			pVertexBuffer->m_fWindIndex = 4.0f * pFronds->m_pWindMatrixIndices[i];
			pVertexBuffer->m_fWindWeight = pFronds->m_pWindWeights[i];
#endif

			++pVertexBuffer;
		}
		m_pFrondVertexBuffer->Unlock();
		UINT unNumLodLevels = m_pSpeedTree->GetNumFrondLodLevels();
		m_pFrondIndexCounts = new unsigned short[unNumLodLevels];
		for (WORD j = 0; j < unNumLodLevels; ++j)
		{

			m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, j);
			if (pFronds->m_usNumStrips > 0)
				m_pFrondIndexCounts[j] = pFronds->m_pStripLengths[0];
			else
				m_pFrondIndexCounts[j] = 0;
		}

		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, 0);
		ms_lpd3dDevice->CreateIndexBuffer(m_pFrondIndexCounts[0] * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pFrondIndexBuffer, NULL);
		unsigned short * pIndexBuffer = NULL;
		m_pFrondIndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pIndexBuffer), 0);
		memcpy(pIndexBuffer, pFronds->m_pStrips[0], pFronds->m_pStripLengths[0] * sizeof(unsigned short));
		m_pFrondIndexBuffer->Unlock();
	}
}
void CSpeedTreeWrapper::SetupLeafBuffers(void)
{

	const short anVertexIndices[6] = { 0, 1, 2, 0, 2, 3 };
	m_usNumLeafLods = m_pSpeedTree->GetNumLeafLodLevels();
	m_pLeafVertexBuffer = new LPDIRECT3DVERTEXBUFFER9[m_usNumLeafLods];
	m_pLeavesUpdatedByCpu = new bool[m_usNumLeafLods];
	for (UINT unLod = 0; unLod < m_usNumLeafLods; ++unLod)
	{
		m_pLeavesUpdatedByCpu[unLod] = false;
		m_pLeafVertexBuffer[unLod] = NULL;
		unsigned short usLeafCount = m_pGeometryCache->m_sLeaves0.m_usLeafCount;

		if (usLeafCount < 1)
			continue;

		SFVFLeafVertex* pVertexBuffer = NULL;

#ifndef WRAPPER_USE_CPU_LEAF_PLACEMENT
		ms_lpd3dDevice->CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_LEAF_VERTEX, D3DPOOL_MANAGED, &m_pLeafVertexBuffer[unLod]);

		m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), 0);
#else
		ms_lpd3dDevice->CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex), D3DUSAGE_DYNAMIC, D3DFVF_SPEEDTREE_LEAF_VERTEX, D3DPOOL_SYSTEMMEM, &m_pLeafVertexBuffer[unLod], NULL);

		m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
#endif
		SFVFLeafVertex* pVertex = pVertexBuffer;
		for (UINT unLeaf = 0; unLeaf < usLeafCount; ++unLeaf)
		{
			const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = &(m_pGeometryCache->m_sLeaves0);
			for (UINT unVert = 0; unVert < 6; ++unVert)  
			{

				memcpy(pVertex->m_vPosition, &(pLeaf->m_pCenterCoords[unLeaf * 3]), 3 * sizeof(float));

#ifdef WRAPPER_USE_DYNAMIC_LIGHTING

				memcpy(&pVertex->m_vNormal, &(pLeaf->m_pNormals[unLeaf * 3]), 3 * sizeof(float));
#else

				pVertex->m_dwDiffuseColor = pLeaf->m_pColors[unLeaf];
#endif
				memcpy(pVertex->m_fTexCoords, &(pLeaf->m_pLeafMapTexCoords[unLeaf][anVertexIndices[unVert] * 2]), 2 * sizeof(float));
#ifdef WRAPPER_USE_GPU_WIND
				pVertex->m_fWindIndex = 4.0f * pLeaf->m_pWindMatrixIndices[unLeaf];
				pVertex->m_fWindWeight = pLeaf->m_pWindWeights[unLeaf];
#endif
#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
				pVertex->m_fLeafPlacementIndex = c_nVertexShader_LeafTables + pLeaf->m_pLeafClusterIndices[unLeaf] * 4.0f + anVertexIndices[unVert];
				pVertex->m_fLeafScalarValue = m_pSpeedTree->GetLeafLodSizeAdjustments()[unLod];
#endif

				++pVertex;
			}
		}
		m_pLeafVertexBuffer[unLod]->Unlock();
	}
}
void CSpeedTreeWrapper::Advance(void)
{

	m_pSpeedTree->ComputeLodLevel();
	m_pSpeedTree->SetLodLevel(1.0f);
#ifdef WRAPPER_USE_CPU_WIND
	m_pSpeedTree->ComputeWindEffects(true, true, true);
#endif
}
CSpeedTreeWrapper * CSpeedTreeWrapper::MakeInstance()
{
	CSpeedTreeWrapper * pInstance = new CSpeedTreeWrapper;
	pInstance->m_bIsInstance = true;
	pInstance->m_pSpeedTree = m_pSpeedTree->MakeInstance();

	if (pInstance->m_pSpeedTree)
    {

		pInstance->m_cBranchMaterial = m_cBranchMaterial;
		pInstance->m_cLeafMaterial = m_cLeafMaterial;
		pInstance->m_cFrondMaterial = m_cFrondMaterial;
		pInstance->m_CompositeImageInstance.SetImagePointer(m_CompositeImageInstance.GetGraphicImagePointer());
		pInstance->m_BranchImageInstance.SetImagePointer(m_BranchImageInstance.GetGraphicImagePointer());

		if (!m_ShadowImageInstance.IsEmpty())
			pInstance->m_ShadowImageInstance.SetImagePointer(m_ShadowImageInstance.GetGraphicImagePointer());

		pInstance->m_pTextureInfo = m_pTextureInfo;
		pInstance->m_pGeometryCache = m_pGeometryCache;
		pInstance->m_pBranchIndexBuffer = m_pBranchIndexBuffer;
		pInstance->m_pBranchIndexCounts = m_pBranchIndexCounts;
		pInstance->m_pBranchVertexBuffer = m_pBranchVertexBuffer;
		pInstance->m_unBranchVertexCount = m_unBranchVertexCount;

		pInstance->m_pFrondIndexBuffer = m_pFrondIndexBuffer;
		pInstance->m_pFrondIndexCounts = m_pFrondIndexCounts;
		pInstance->m_pFrondVertexBuffer = m_pFrondVertexBuffer;
		pInstance->m_unFrondVertexCount = m_unFrondVertexCount;

		pInstance->m_pLeafVertexBuffer = m_pLeafVertexBuffer;
		pInstance->m_usNumLeafLods = m_usNumLeafLods;
		pInstance->m_pLeavesUpdatedByCpu = m_pLeavesUpdatedByCpu;
		memcpy(pInstance->m_afPos, m_afPos, 3 * sizeof(float));
		memcpy(pInstance->m_afBoundingBox, m_afBoundingBox, 6 * sizeof(float));
		pInstance->m_pInstanceOf = this;
		m_vInstances.push_back(pInstance);
    }
    else
	{
		fprintf(stderr, "SpeedTreeRT Error: %s\n", m_pSpeedTree->GetCurrentError());
        delete pInstance;
        pInstance = NULL;
	}

	return pInstance;
}
CSpeedTreeWrapper ** CSpeedTreeWrapper::GetInstances(UINT& nCount)
{
	nCount = m_vInstances.size();
	if (nCount)
		return &(m_vInstances[0]);
	else
		return NULL;
}

void CSpeedTreeWrapper::DeleteInstance(CSpeedTreeWrapper * pInstance)
{
	std::vector<CSpeedTreeWrapper *>::iterator itor = m_vInstances.begin();

	while (itor != m_vInstances.end())
	{
		if (*itor == pInstance)
		{
			itor = m_vInstances.erase(itor);
		}
		else
			++itor;
	}
	delete pInstance;
}
void CSpeedTreeWrapper::SetupBranchForTreeType(void) const
{
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING

	STATEMANAGER.SetMaterial(m_cBranchMaterial.Get());
	SetShaderConstants(m_pSpeedTree->GetBranchMaterial());
#endif
#ifdef WRAPPER_USE_CPU_WIND
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

	if (m_pGeometryCache->m_sBranches.m_usNumStrips > 0)
	{

		SFVFBranchVertex* pVertexBuffer = NULL;
		m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
		for (UINT i = 0; i < m_unBranchVertexCount; ++i)
		{
			memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sBranches.m_pCoords[i * 3]), 3 * sizeof(float));
		}
		m_pBranchVertexBuffer->Unlock();
	}
#endif

	LPDIRECT3DTEXTURE9 lpd3dTexture;

    
    if ((lpd3dTexture = m_BranchImageInstance.GetTextureReference().GetD3DTexture()))
        STATEMANAGER.SetTexture(0, lpd3dTexture);
#ifdef WRAPPER_RENDER_SELF_SHADOWS
	if (ms_bSelfShadowOn && (lpd3dTexture = m_ShadowImageInstance.GetTextureReference().GetD3DTexture()))
		STATEMANAGER.SetTexture(1, lpd3dTexture);
	else
		STATEMANAGER.SetTexture(1, NULL);
#endif

	if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
	{

		STATEMANAGER.SetStreamSource(0, m_pBranchVertexBuffer, sizeof(SFVFBranchVertex));

		STATEMANAGER.SetIndices(m_pBranchIndexBuffer, 0);
	}
}
void CSpeedTreeWrapper::RenderBranches(void) const
{
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

	if (m_pGeometryCache->m_fBranchAlphaTestValue)
	{
		PositionTree();
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fBranchAlphaTestValue));
		if (m_pBranchIndexCounts &&
			m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] > 0)
		{
			ms_faceCount += m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2;
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sBranches.m_usVertexCount, 0, m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2);
		}
	}
}
void CSpeedTreeWrapper::SetupFrondForTreeType(void) const
{
#ifdef SPEEDTREE_LIGHTING_DYNAMIC

	STATEMANAGER.SetMaterial(m_cFrondMaterial.Get());
	SetShaderConstants(m_pSpeedTree->GetFrondMaterial());
#endif
#ifdef WRAPPER_USE_CPU_WIND
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);
	if (m_pGeometryCache->m_sFronds.m_usNumStrips > 0)
	{

		SFVFBranchVertex * pVertexBuffer = NULL;
		m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
		for (UINT i = 0; i < m_unFrondVertexCount; ++i)
		{
			memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sFronds.m_pCoords[i * 3]), 3 * sizeof(float));
		}
		m_pFrondVertexBuffer->Unlock();
	}
#endif

	if (!m_CompositeImageInstance.IsEmpty())
		STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
#ifdef WRAPPER_RENDER_SELF_SHADOWS
	LPDIRECT3DTEXTURE9 lpd3dTexture;

	if ((lpd3dTexture = m_ShadowImageInstance.GetTextureReference().GetD3DTexture()))
		STATEMANAGER.SetTexture(1, lpd3dTexture);
#endif

	if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0)
	{

		STATEMANAGER.SetStreamSource(0, m_pFrondVertexBuffer, sizeof(SFVFBranchVertex));

		STATEMANAGER.SetIndices(m_pFrondIndexBuffer, 0);
	}
}
void CSpeedTreeWrapper::RenderFronds(void) const
{
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);

	if (m_pGeometryCache->m_fFrondAlphaTestValue > 0.0f)
	{
		PositionTree();
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fFrondAlphaTestValue));
		if (m_pFrondIndexCounts &&
			m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] > 0)
		{
			ms_faceCount += m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2;
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sFronds.m_usVertexCount, 0, m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2);
		}
	}
}
void CSpeedTreeWrapper::SetupLeafForTreeType(void) const
{
#ifdef SPEEDTREE_LIGHTING_DYNAMIC

	STATEMANAGER.SetMaterial(m_cLeafMaterial.Get());
	SetShaderConstants(m_pSpeedTree->GetLeafMaterial());
#endif
#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
	UploadLeafTables(c_nVertexShader_LeafTables);
#endif

	if (!m_CompositeImageInstance.IsEmpty())
		STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
#ifdef WRAPPER_RENDER_SELF_SHADOWS
	STATEMANAGER.SetTexture(1, NULL);
#endif
}
#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
void CSpeedTreeWrapper::UploadLeafTables(UINT uiLocation) const
{

	UINT uiEntryCount = 0;
	const float * pTable = m_pSpeedTree->GetLeafBillboardTable(uiEntryCount);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_LeafTables, pTable, uiEntryCount / 4);
}
#endif
void CSpeedTreeWrapper::RenderLeaves(void) const
{

	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry);
#if defined(WRAPPER_USE_GPU_LEAF_PLACEMENT) && defined(WRAPPER_USE_GPU_WIND)

#else
#if !defined WRAPPER_USE_NO_WIND || defined WRAPPER_USE_CPU_LEAF_PLACEMENT

	for (UINT i = 0; i < 2; ++i)
	{

		const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = (i == 0) ? &m_pGeometryCache->m_sLeaves0 : &m_pGeometryCache->m_sLeaves1;
		int unLod = pLeaf->m_nDiscreteLodLevel;

#if defined WRAPPER_USE_GPU_LEAF_PLACEMENT
		if (pLeaf->m_bIsActive && !m_pLeavesUpdatedByCpu[unLod])
		{

			SFVFLeafVertex* pVertex = NULL;
			m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertex), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
			for (UINT unLeaf = 0; unLeaf < pLeaf->m_usLeafCount; ++unLeaf)
			{
				D3DXVECTOR3 vecCenter(&(pLeaf->m_pCenterCoords[unLeaf * 3]));
				(pVertex++)->m_vPosition = vecCenter;
				(pVertex++)->m_vPosition = vecCenter;
				(pVertex++)->m_vPosition = vecCenter;
				(pVertex++)->m_vPosition = vecCenter;
				(pVertex++)->m_vPosition = vecCenter;
				(pVertex++)->m_vPosition = vecCenter;
			}
			m_pLeafVertexBuffer[unLod]->Unlock();
			m_pLeavesUpdatedByCpu[unLod] = true;
		}
#else
		if (pLeaf->m_bIsActive && m_pLeafVertexBuffer[unLod])
		{

			SFVFLeafVertex * pVertex = NULL;

			const UINT VERTEX_NUM = 8192;
			if (pLeaf->m_usLeafCount*3>=VERTEX_NUM)
				return;

			D3DXVECTOR3 akPosition[VERTEX_NUM];
			D3DXVECTOR3*pkPosition=akPosition;
			const float* center=pLeaf->m_pCenterCoords;
			for (UINT unLeaf = 0; unLeaf < pLeaf->m_usLeafCount; ++unLeaf)
			{
				pkPosition[0].x=pLeaf->m_pLeafMapCoords[unLeaf][0]+center[0];
				pkPosition[0].y=pLeaf->m_pLeafMapCoords[unLeaf][1]+center[1];
				pkPosition[0].z=pLeaf->m_pLeafMapCoords[unLeaf][2]+center[2];
				pkPosition[1].x=pLeaf->m_pLeafMapCoords[unLeaf][4]+center[0];
				pkPosition[1].y=pLeaf->m_pLeafMapCoords[unLeaf][5]+center[1];
				pkPosition[1].z=pLeaf->m_pLeafMapCoords[unLeaf][6]+center[2];
				pkPosition[2].x=pLeaf->m_pLeafMapCoords[unLeaf][8]+center[0];
				pkPosition[2].y=pLeaf->m_pLeafMapCoords[unLeaf][9]+center[1];
				pkPosition[2].z=pLeaf->m_pLeafMapCoords[unLeaf][10]+center[2];
				pkPosition[3]=pkPosition[0];
				pkPosition[4]=pkPosition[2];
				pkPosition[5].x=pLeaf->m_pLeafMapCoords[unLeaf][12]+center[0];
				pkPosition[5].y=pLeaf->m_pLeafMapCoords[unLeaf][13]+center[1];
				pkPosition[5].z=pLeaf->m_pLeafMapCoords[unLeaf][14]+center[2];
				pkPosition+=6;
				center+=3;
			}

			if (SUCCEEDED(
				m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<void**>(&pVertex), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK)
			))
			{
				UINT uVtxCount=pLeaf->m_usLeafCount*6;
				for (UINT i = 0; i<uVtxCount; ++i)
					pVertex[i].m_vPosition=akPosition[i];

				m_pLeafVertexBuffer[unLod]->Unlock();
			}
		}
#endif
	}
#endif
#endif

	PositionTree();
	for (UINT unLeafLevel = 0; unLeafLevel < 2; ++unLeafLevel)
	{
		const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = (unLeafLevel == 0) ?
			&m_pGeometryCache->m_sLeaves0 : &m_pGeometryCache->m_sLeaves1;

		int unLod = pLeaf->m_nDiscreteLodLevel;

		if (unLod > -1 && pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
		{
			STATEMANAGER.SetStreamSource(0, m_pLeafVertexBuffer[unLod], sizeof(SFVFLeafVertex));
			STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(pLeaf->m_fAlphaTestValue));

			ms_faceCount += pLeaf->m_usLeafCount * 2;
			STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLELIST, 0, pLeaf->m_usLeafCount * 2);
		}
	}
}
void CSpeedTreeWrapper::EndLeafForTreeType(void)
{

	for (UINT i = 0; i < m_usNumLeafLods; ++i)
		m_pLeavesUpdatedByCpu[i] = false;
}
void CSpeedTreeWrapper::RenderBillboards(void) const
{

#ifdef WRAPPER_BILLBOARD_MODE
	if (!m_CompositeImageInstance.IsEmpty())
		STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

	PositionTree();

	struct SBillboardVertex
	{
		float fX, fY, fZ;
		float fU, fV;
	};

	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BillboardGeometry);

	if (m_pGeometryCache->m_sBillboard0.m_bIsActive)
	{
		const float* pCoords = m_pGeometryCache->m_sBillboard0.m_pCoords;
		const float* pTexCoords = m_pGeometryCache->m_sBillboard0.m_pTexCoords;
		SBillboardVertex sVertex[4] =
		{
			{ pCoords[0], pCoords[1], pCoords[2], pTexCoords[0], pTexCoords[1] },
			{ pCoords[3], pCoords[4], pCoords[5], pTexCoords[2], pTexCoords[3] },
			{ pCoords[6], pCoords[7], pCoords[8], pTexCoords[4], pTexCoords[5] },
			{ pCoords[9], pCoords[10], pCoords[11], pTexCoords[6], pTexCoords[7] },
		};

		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard0.m_fAlphaTestValue));

		ms_faceCount += 2;
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
	}
	if (m_pGeometryCache->m_sBillboard1.m_bIsActive)
	{
		const float* pCoords = m_pGeometryCache->m_sBillboard1.m_pCoords;
		const float* pTexCoords = m_pGeometryCache->m_sBillboard1.m_pTexCoords;
		SBillboardVertex sVertex[4] =
		{
			{ pCoords[0], pCoords[1], pCoords[2], pTexCoords[0], pTexCoords[1] },
			{ pCoords[3], pCoords[4], pCoords[5], pTexCoords[2], pTexCoords[3] },
			{ pCoords[6], pCoords[7], pCoords[8], pTexCoords[4], pTexCoords[5] },
			{ pCoords[9], pCoords[10], pCoords[11], pTexCoords[6], pTexCoords[7] },
		};
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard1.m_fAlphaTestValue));

		ms_faceCount += 2;
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
	}

#ifdef WRAPPER_RENDER_HORIZONTAL_BILLBOARD

	if (m_pGeometryCache->m_sHorizontalBillboard.m_bIsActive)
	{
		const float* pCoords = m_pGeometryCache->m_sHorizontalBillboard.m_pCoords;
		const float* pTexCoords = m_pGeometryCache->m_sHorizontalBillboard.m_pTexCoords;
		SBillboardVertex sVertex[4] =
		{
			{ pCoords[0], pCoords[1], pCoords[2], pTexCoords[0], pTexCoords[1] },
			{ pCoords[3], pCoords[4], pCoords[5], pTexCoords[2], pTexCoords[3] },
			{ pCoords[6], pCoords[7], pCoords[8], pTexCoords[4], pTexCoords[5] },
			{ pCoords[9], pCoords[10], pCoords[11], pTexCoords[6], pTexCoords[7] },
		};
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sHorizontalBillboard.m_fAlphaTestValue));

		ms_faceCount += 2;
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
	}

#endif
#endif
}
void CSpeedTreeWrapper::CleanUpMemory(void)
{
	if (!m_bIsInstance)
		m_pSpeedTree->DeleteTransientData();
}
void CSpeedTreeWrapper::PositionTree(void) const
{
	D3DXVECTOR3 vecPosition = m_pSpeedTree->GetTreePosition();
	D3DXMATRIX matTranslation;
	D3DXMatrixIdentity(&matTranslation);
	D3DXMatrixTranslation(&matTranslation, vecPosition.x, vecPosition.y, vecPosition.z);
	STATEMANAGER.SetTransform(D3DTS_WORLD, &matTranslation);

    
	D3DXVECTOR4 vecConstant(vecPosition[0], vecPosition[1], vecPosition[2], 0.0f);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_TreePos, (float*)&vecConstant, 1);
}
bool CSpeedTreeWrapper::LoadTexture(const char * pFilename, CGraphicImageInstance & rImage)
{
	CResource * pResource = CResourceManager::Instance().GetResourcePointer(pFilename);
	rImage.SetImagePointer(static_cast<CGraphicImage *>(pResource));

	if (rImage.IsEmpty())
		return false;
	return true;
}
void CSpeedTreeWrapper::SetShaderConstants(const float* pMaterial) const
{
	const float afUsefulConstants[] =
	{
		m_pSpeedTree->GetLeafLightingAdjustment(), 0.0f, 0.0f, 0.0f,
	};

	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_LeafLightingAdjustment, afUsefulConstants, 1);

	const float afMaterial[] =
	{
		pMaterial[0], pMaterial[1], pMaterial[2], 1.0f,
			pMaterial[3], pMaterial[4], pMaterial[5], 1.0f
	};

	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Material, afMaterial, 2);
}

void CSpeedTreeWrapper::SetPosition(float x, float y, float z)
{
	m_afPos[0] = x;
	m_afPos[1] = y;
	m_afPos[2] = z;
	m_pSpeedTree->SetTreePosition(x, y, z);
	CGraphicObjectInstance::SetPosition(x, y, z);
}

bool CSpeedTreeWrapper::GetBoundingSphere(D3DXVECTOR3 & v3Center, float & fRadius)
{
	float fX, fY, fZ;

	fX = m_afBoundingBox[3] - m_afBoundingBox[0];
	fY = m_afBoundingBox[4] - m_afBoundingBox[1];
	fZ = m_afBoundingBox[5] - m_afBoundingBox[2];

	v3Center.x = 0.0f;
	v3Center.y = 0.0f;
	v3Center.z = fZ * 0.5f;

	fRadius = sqrtf(fX * fX + fY * fY + fZ * fZ) * 0.5f * 0.9f; 

	D3DXVECTOR3 vec = m_pSpeedTree->GetTreePosition();

	v3Center+=vec;

	return true;
}

void CSpeedTreeWrapper::CalculateBBox()
{
	float fX, fY, fZ;

	fX = m_afBoundingBox[3] - m_afBoundingBox[0];
	fY = m_afBoundingBox[4] - m_afBoundingBox[1];
	fZ = m_afBoundingBox[5] - m_afBoundingBox[2];

	m_v3BBoxMin.x = -fX / 2.0f;
	m_v3BBoxMin.y = -fY / 2.0f;
	m_v3BBoxMin.z = 0.0f;
	m_v3BBoxMax.x = fX / 2.0f;
	m_v3BBoxMax.y = fY / 2.0f;
	m_v3BBoxMax.z = fZ;

	m_v4TBBox[0] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[1] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[2] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[3] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[4] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[5] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[6] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[7] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);

	const D3DXMATRIX & c_rmatTransform = GetTransform();

	for (DWORD i = 0; i < 8; ++i)
	{
		D3DXVec4Transform(&m_v4TBBox[i], &m_v4TBBox[i], &c_rmatTransform);
		if (0 == i)
		{
			m_v3TBBoxMin.x = m_v4TBBox[i].x;
			m_v3TBBoxMin.y = m_v4TBBox[i].y;
			m_v3TBBoxMin.z = m_v4TBBox[i].z;
			m_v3TBBoxMax.x = m_v4TBBox[i].x;
			m_v3TBBoxMax.y = m_v4TBBox[i].y;
			m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
		else
		{
			if (m_v3TBBoxMin.x > m_v4TBBox[i].x)
				m_v3TBBoxMin.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMax.x < m_v4TBBox[i].x)
				m_v3TBBoxMax.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMin.y > m_v4TBBox[i].y)
				m_v3TBBoxMin.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMax.y < m_v4TBBox[i].y)
				m_v3TBBoxMax.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMin.z > m_v4TBBox[i].z)
				m_v3TBBoxMin.z = m_v4TBBox[i].z;
			if (m_v3TBBoxMax.z < m_v4TBBox[i].z)
				m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
	}
}
UINT CSpeedTreeWrapper::GetCollisionObjectCount()
{
	assert(m_pSpeedTree);
	return m_pSpeedTree->GetCollisionObjectCount();
}

void CSpeedTreeWrapper::GetCollisionObject(UINT nIndex, CSpeedTreeRT::ECollisionObjectType& eType, float* pPosition, float* pDimensions)
{
	assert(m_pSpeedTree);
	m_pSpeedTree->GetCollisionObject(nIndex, eType, pPosition, pDimensions);
}
const float * CSpeedTreeWrapper::GetPosition()
{
	return m_afPos;
}

void CSpeedTreeWrapper::GetTreeSize(float & r_fSize, float & r_fVariance)
{
	m_pSpeedTree->GetTreeSize(r_fSize, r_fVariance);
}
void CSpeedTreeWrapper::OnUpdateCollisionData(const CStaticCollisionDataVector * /*pscdVector*/)
{
	D3DXMATRIX mat;
	D3DXMatrixTranslation(&mat, m_afPos[0], m_afPos[1], m_afPos[2]);
	for (UINT i = 0; i < GetCollisionObjectCount(); ++i)
	{
		CSpeedTreeRT::ECollisionObjectType ObjectType;
		CStaticCollisionData CollisionData;

		GetCollisionObject(i, ObjectType, (float * )&CollisionData.v3Position, CollisionData.fDimensions);

		if (ObjectType == CSpeedTreeRT::CO_BOX)
			continue;

		switch(ObjectType)
		{
		case CSpeedTreeRT::CO_SPHERE:
			CollisionData.dwType = COLLISION_TYPE_SPHERE;
			CollisionData.fDimensions[0] = CollisionData.fDimensions[0];

			break;

		case CSpeedTreeRT::CO_CYLINDER:
			CollisionData.dwType = COLLISION_TYPE_CYLINDER;
			CollisionData.fDimensions[0] = CollisionData.fDimensions[0];
			CollisionData.fDimensions[1] = CollisionData.fDimensions[1];

			break;

		}
		AddCollision(&CollisionData, &mat);
	}
}
