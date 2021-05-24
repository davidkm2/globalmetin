
#pragma once
#include "SpeedTreeMaterial.h"
#ifdef CMAKE_BUILD_SYSTEM
#include <SpeedTreeRT_s.h>
#else
#include <SpeedTreeRT.h>
#endif

#include <d3d9/d3d9.h>
#include <d3d9/d3d9types.h>
#include <d3d9/d3dx9.h>
#include <vector>

#include "../EterLib/GrpObjectInstance.h"
#include "../EterLib/GrpImageInstance.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p) = NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p) = NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = NULL; } }
#endif
#pragma warning(push)
#pragma warning(disable:4100)

class CSpeedTreeWrapper : public CGraphicObjectInstance
{
	enum
	{
		ID = TREE_OBJECT
	};
	int GetType() const { return ID; }
protected:
	virtual void OnUpdateCollisionData(const CStaticCollisionDataVector * pscdVector);
	virtual void OnUpdateHeighInstance(CAttributeInstance * pAttributeInstance) {}
	virtual bool OnGetObjectHeight(float fX, float fY, float * pfHeight) { return false; }

public:
	virtual bool GetBoundingSphere(D3DXVECTOR3 & v3Center, float & fRadius);

public:
	static bool					ms_bSelfShadowOn;

public:
	virtual void				SetPosition(float x, float y, float z);
	virtual void				CalculateBBox();

	virtual void				OnRender(); 

	virtual void				OnBlendRender() {}
	virtual void				OnRenderToShadowMap() {}
	virtual void				OnRenderShadow() {}
	virtual void				OnRenderPCBlocker();

public:
	CSpeedTreeWrapper();
	virtual	~CSpeedTreeWrapper();

	const float *				GetPosition();
	static void					SetVertexShaders(LPDIRECT3DVERTEXDECLARATION9 dwBranchVertexShader, LPDIRECT3DVERTEXDECLARATION9 dwLeafVertexShader);
	bool                        LoadTree(const char * pszSptFile, const BYTE * c_pbBlock = NULL, unsigned int uiBlockSize = 0, unsigned int nSeed = 1, float fSize = -1.0f, float fSizeVariance = -1.0f);
	const float *				GetBoundingBox(void) const						{ return m_afBoundingBox; }
	void						GetTreeSize(float & r_fSize, float & r_fVariance);
	UINT						GetCollisionObjectCount();
	void						GetCollisionObject(unsigned int nIndex, CSpeedTreeRT::ECollisionObjectType& eType, float* pPosition, float* pDimensions);
	void						SetupBranchForTreeType(void) const;
	void						SetupFrondForTreeType(void) const;
	void						SetupLeafForTreeType(void) const;
	void						EndLeafForTreeType(void);

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
	void						UploadLeafTables(unsigned int uiLocation) const;
#endif

	void						RenderBranches(void) const;
	void                        RenderFronds(void) const;
	void						RenderLeaves(void) const;
	void						RenderBillboards(void) const;
	CSpeedTreeWrapper **		GetInstances(unsigned int& nCount);
	CSpeedTreeWrapper *			InstanceOf(void) const							{ return m_pInstanceOf; }
	CSpeedTreeWrapper * 		MakeInstance();
	void						DeleteInstance(CSpeedTreeWrapper * pInstance);
	CSpeedTreeRT *				GetSpeedTree(void) const						{ return m_pSpeedTree; }
	const CSpeedTreeMaterial &	GetBranchMaterial(void) const					{ return m_cBranchMaterial; }
	const CSpeedTreeMaterial &	GetFrondMaterial(void) const					{ return m_cFrondMaterial; }
	const CSpeedTreeMaterial &	GetLeafMaterial(void) const						{ return m_cLeafMaterial; }
	float                       GetLeafLightingAdjustment(void) const			{ return m_pSpeedTree->GetLeafLightingAdjustment( ); }
	void						SetWindStrength(float fStrength)				{ m_pSpeedTree->SetWindStrength(fStrength); }
	void						Advance(void);
	LPDIRECT3DTEXTURE9			GetBranchTexture(void) const;
	void						CleanUpMemory(void);

private:
	void						SetupBuffers(void);
	void						SetupBranchBuffers(void);
	void						SetupFrondBuffers(void);
	void						SetupLeafBuffers(void);
	void						PositionTree(void) const;
	static bool					LoadTexture(const char* pFilename, CGraphicImageInstance & rImage);
	void						SetShaderConstants(const float* pMaterial) const;
private:
	CSpeedTreeRT*					m_pSpeedTree;
	CSpeedTreeRT::STextures*		m_pTextureInfo;
	bool							m_bIsInstance;
	std::vector<CSpeedTreeWrapper*>	m_vInstances;
	CSpeedTreeWrapper*				m_pInstanceOf;
	CSpeedTreeRT::SGeometry*		m_pGeometryCache;
	LPDIRECT3DVERTEXBUFFER9			m_pBranchVertexBuffer;
	unsigned int					m_unBranchVertexCount;
	LPDIRECT3DINDEXBUFFER9			m_pBranchIndexBuffer;
	unsigned short*					m_pBranchIndexCounts;
	LPDIRECT3DVERTEXBUFFER9			m_pFrondVertexBuffer;
	unsigned int					m_unFrondVertexCount;
	LPDIRECT3DINDEXBUFFER9			m_pFrondIndexBuffer;
	unsigned short*					m_pFrondIndexCounts;
	unsigned short					m_usNumLeafLods;
	LPDIRECT3DVERTEXBUFFER9*		m_pLeafVertexBuffer;
	bool*							m_pLeavesUpdatedByCpu;
	float							m_afPos[3];
	float							m_afBoundingBox[6];
	CSpeedTreeMaterial				m_cBranchMaterial;
	CSpeedTreeMaterial				m_cLeafMaterial;
	CSpeedTreeMaterial				m_cFrondMaterial;
	CGraphicImageInstance			m_BranchImageInstance;
	CGraphicImageInstance			m_ShadowImageInstance;
	CGraphicImageInstance			m_CompositeImageInstance;

	static LPDIRECT3DVERTEXDECLARATION9 ms_dwBranchVertexShader;
	static LPDIRECT3DVERTEXDECLARATION9 ms_dwLeafVertexShader;
};

#pragma warning(pop)
