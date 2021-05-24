
#pragma once
#define SPEEDTREE_DATA_FORMAT_DIRECTX

#include "SpeedTreeForest.h"
#include "SpeedTreeMaterial.h"
class CSpeedTreeForestDirectX9 : public CSpeedTreeForest, public CGraphicBase, public CSingleton<CSpeedTreeForestDirectX9>
{
	public:
		CSpeedTreeForestDirectX9();
		virtual ~CSpeedTreeForestDirectX9();

		void			UploadWindMatrix(unsigned int uiLocation, const float* pMatrix) const;
		void			UpdateCompundMatrix(const D3DXVECTOR3 & c_rEyeVec, const D3DXMATRIX & c_rmatView, const D3DXMATRIX & c_rmatProj);

		void			Render(unsigned long ulRenderBitVector = Forest_RenderAll);
		bool			SetRenderingDevice(LPDIRECT3DDEVICE9 pDevice);

	private:
		bool			InitVertexShaders();

	private:
		LPDIRECT3DDEVICE9		m_pDx;

		LPDIRECT3DVERTEXDECLARATION9 m_dwBranchVertexShader;
		LPDIRECT3DVERTEXDECLARATION9 m_dwLeafVertexShader;
};
