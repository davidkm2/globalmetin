#pragma once
#include "../EterLib/GrpImage.h"
#include "../EterLib/GrpCollisionObject.h"

#include "Model.h"
#include "Motion.h"

class CGrannyModelInstance : public CGraphicCollisionObject
{
	public:
		enum
		{
			ANIFPS_MIN = 30,
			ANIFPS_MAX = 120,
		};
	public:
		static void DestroySystem();

		static CGrannyModelInstance* New();
		static void Delete(CGrannyModelInstance* pkInst);

		static CDynamicPool<CGrannyModelInstance>		ms_kPool;

	public:
		struct FCreateDeviceObjects
		{
			void operator() (CGrannyModelInstance * pModelInstance)
			{pModelInstance->CreateDeviceObjects();}
		};

		struct FDestroyDeviceObjects
		{
			void operator() (CGrannyModelInstance * pModelInstance)
			{pModelInstance->DestroyDeviceObjects();}
		};

	public:
		CGrannyModelInstance();
		virtual ~CGrannyModelInstance();

		bool	IsEmpty();
		void	Clear();

		bool	CreateDeviceObjects();
		void	DestroyDeviceObjects();
		void	Update(DWORD dwAniFPS);
		void	UpdateLocalTime(float fElapsedTime);
		void	UpdateTransform(D3DXMATRIX * pMatrix, float fSecondsElapsed);

		void	UpdateSkeleton(const D3DXMATRIX * c_pWorldMatrix, float fLocalTime);
		void	DeformNoSkin(const D3DXMATRIX * c_pWorldMatrix);
		void	Deform(const D3DXMATRIX * c_pWorldMatrix);
		void	RenderWithOneTexture();
		void	RenderWithTwoTexture();
		void	BlendRenderWithOneTexture();
		void	BlendRenderWithTwoTexture();
		void	RenderWithoutTexture();
		CGrannyModel* GetModel();
		void	SetMaterialImagePointer(const char* c_szImageName, CGraphicImage* pImage);
		void	SetMaterialData(const char* c_szImageName, const SMaterialData& c_rkMaterialData);
		void	SetSpecularInfo(const char* c_szMtrlName, BOOL bEnable, float fPower);

		void	SetMainModelPointer(CGrannyModel* pkModel, CGraphicVertexBuffer* pkSharedDefromableVertexBuffer);
		void	SetLinkedModelPointer(CGrannyModel* pkModel, CGraphicVertexBuffer* pkSharedDefromableVertexBuffer, CGrannyModelInstance** ppkSkeletonInst);
		void	SetMotionPointer(const CGrannyMotion* pMotion, float blendTime=0.0f, int loopCount=0, float speedRatio=1.0f);
		void	ChangeMotionPointer(const CGrannyMotion* pMotion, int loopCount=0, float speedRatio=1.0f);
		void	SetMotionAtEnd();
		bool	IsMotionPlaying();

		void	CopyMotion(CGrannyModelInstance * pModelInstance, bool bIsFreeSourceControl=false);
		void	SetLocalTime(float fLocalTime);
		int		ResetLocalTime();
		float	GetLocalTime();
		float	GetNextTime();
		DWORD	GetDeformableVertexCount();
		DWORD	GetVertexCount();
		const float *	GetBoneMatrixPointer(int iBone) const;
		const float *	GetCompositeBoneMatrixPointer(int iBone) const;
		bool			GetMeshMatrixPointer(int iMesh, const D3DXMATRIX ** c_ppMatrix) const;
		bool			GetBoneIndexByName(const char * c_szBoneName, int * pBoneIndex) const;
		void			SetParentModelInstance(const CGrannyModelInstance* c_pParentModelInstance, const char * c_szBoneName);
		void			SetParentModelInstance(const CGrannyModelInstance* c_pParentModelInstance, int iBone);
		bool	Intersect(const D3DXMATRIX * c_pMatrix, float * pu, float * pv, float * pt);
		void	MakeBoundBox(TBoundBox* pBoundBox, const float* mat, const float* OBBMin, const float* OBBMax, D3DXVECTOR3* vtMin, D3DXVECTOR3* vtMax);
		void	GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3* vtMax);
		void	ReloadTexture();
	protected:
		void	__Initialize();

		void	__DestroyModelInstance();
		void	__DestroyMeshMatrices();
		void	__DestroyDynamicVertexBuffer();
		void	__CreateModelInstance();
		void	__CreateMeshMatrices();
		void	__CreateDynamicVertexBuffer();
		void	__DestroyWorldPose();
		void	__CreateWorldPose(CGrannyModelInstance* pkSrcModelInst);

		bool	__CreateMeshBindingVector(CGrannyModelInstance* pkDstModelInst);
		void	__DestroyMeshBindingVector();

		const granny_int32x*	__GetMeshBoneIndices(unsigned int iMeshBinding) const;

		bool	__IsDeformableVertexBuffer();
		void	__SetSharedDeformableVertexBuffer(CGraphicVertexBuffer* pkSharedDeformableVertexBuffer);

		IDirect3DVertexBuffer9* __GetDeformableD3DVertexBufferPtr();
		CGraphicVertexBuffer&	__GetDeformableVertexBufferRef();

		granny_world_pose* __GetWorldPosePtr() const;
		void	UpdateWorldPose();
		void	UpdateWorldMatrices(const D3DXMATRIX * c_pWorldMatrix);
		void	DeformPNTVertices(void * pvDest);

		void	RenderMeshNodeListWithOneTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType);
		void	RenderMeshNodeListWithTwoTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType);
		void	RenderMeshNodeListWithoutTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType);

	protected:
		CGrannyModel *					m_pModel;
		granny_model_instance *			m_pgrnModelInstance;
		granny_control *				m_pgrnCtrl;
		granny_animation *				m_pgrnAni;
		D3DXMATRIX *					m_meshMatrices;
		const CGrannyModelInstance *	mc_pParentInstance;
		int								m_iParentBoneIndex;
		float							m_fLocalTime;
		float							m_fSecondsElapsed;

		DWORD							m_dwOldUpdateFrame;

		CGrannyMaterialPalette			m_kMtrlPal;
		granny_world_pose*					m_pgrnWorldPoseReal;
		std::vector<granny_mesh_binding*>	m_vct_pgrnMeshBinding;
		CGraphicVertexBuffer*			m_pkSharedDeformableVertexBuffer;
		CGraphicVertexBuffer			m_kLocalDeformableVertexBuffer;
		bool							m_isDeformableVertexBuffer;
		CGrannyModelInstance** m_ppkSkeletonInst;

#ifdef _TEST
		D3DXMATRIX TEST_matWorld;
#endif
	public:
		bool							HaveBlendThing() { return m_pModel->HaveBlendThing(); }
};
