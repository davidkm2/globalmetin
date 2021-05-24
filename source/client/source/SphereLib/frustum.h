#pragma once
#include "vector.h"

enum ViewState
{
	VS_INSIDE,   
	VS_PARTIAL,  
	VS_OUTSIDE   
};

class Frustum
{
	public:
		void BuildViewFrustum(D3DXMATRIX & mat);
		void BuildViewFrustum2(D3DXMATRIX & mat, float fNear, float fFar, float fFov, float fAspect, const D3DXVECTOR3 & vCamera, const D3DXVECTOR3 & vLook);
		ViewState ViewVolumeTest(const Vector3d &c_v3Center,const float c_fRadius) const;

	private:
		bool m_bUsingSphere;
		D3DXVECTOR3 m_v3Center;
		float m_fRadius;
		D3DXPLANE m_plane[6];
};

