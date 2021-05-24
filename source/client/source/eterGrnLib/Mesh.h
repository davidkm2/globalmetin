#pragma once

#include "Material.h"

extern granny_data_type_definition GrannyPNT3322VertexType[5];

struct granny_pnt3322_vertex
{
    granny_real32 Position[3];
    granny_real32 Normal[3];
    granny_real32 UV0[2];
    granny_real32 UV1[2];
};

class CGrannyMesh
{
	public:
		enum EType
		{
			TYPE_RIGID,
			TYPE_DEFORM,
			TYPE_MAX_NUM
		};

		typedef struct STriGroupNode
		{
			STriGroupNode *		pNextTriGroupNode;
			int					idxPos;
			int					triCount;
			DWORD				mtrlIndex;
		} TTriGroupNode;

	public:
		CGrannyMesh();
		virtual ~CGrannyMesh();

		bool					IsEmpty() const;
		bool					CreateFromGrannyMeshPointer(granny_skeleton* pgrnSkeleton, granny_mesh* pgrnMesh, int vtxBasePos, int idxBasePos, CGrannyMaterialPalette& rkMtrlPal);
		void					LoadIndices(void* dstBaseIndices);
 		void					LoadPNTVertices(void* dstBaseVertices);
		void					NEW_LoadVertices(void* dstBaseVertices);
		void					Destroy();

		void					SetPNT2Mesh();

		void					DeformPNTVertices(void* dstBaseVertices, D3DXMATRIX* boneMatrices, granny_mesh_binding* pgrnMeshBinding) const;
		bool					CanDeformPNTVertices() const;
		bool					IsTwoSide() const;

		int						GetVertexCount() const;
		const granny_int32x *					GetDefaultBoneIndices() const;
		int						GetVertexBasePosition() const;
		int						GetIndexBasePosition() const;

		const granny_mesh *					GetGrannyMeshPointer() const;
		const CGrannyMesh::TTriGroupNode *	GetTriGroupNodeList(CGrannyMaterial::EType eMtrlType) const;

		void					RebuildTriGroupNodeList();
		void					ReloadMaterials();

	protected:
		void					Initialize();

		bool					LoadMaterials(CGrannyMaterialPalette& rkMtrlPal);
		bool					LoadTriGroupNodeList(CGrannyMaterialPalette& rkMtrlPal);

	protected:
		granny_data_type_definition *	m_pgrnMeshType;
		granny_mesh *			m_pgrnMesh;
		granny_mesh_binding *	m_pgrnMeshBindingTemp;
		granny_mesh_deformer *	m_pgrnMeshDeformer;
		std::vector<DWORD>		m_mtrlIndexVector;
		TTriGroupNode *			m_triGroupNodes;
		TTriGroupNode *			m_triGroupNodeLists[CGrannyMaterial::TYPE_MAX_NUM];

		int						m_vtxBasePos;
		int						m_idxBasePos;

		bool					m_canDeformPNTVertex;
		bool					m_isTwoSide;
	private:
		bool						m_bHaveBlendThing;
	public:
		bool						HaveBlendThing() { return m_bHaveBlendThing; }
};