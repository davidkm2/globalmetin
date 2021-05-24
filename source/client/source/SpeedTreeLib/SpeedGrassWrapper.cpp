
#include "StdAfx.h"

#include <stdlib.h>
#include <stdio.h>
#include <vector>
using namespace std;

namespace { char dummy; }; 

#ifdef USE_SPEEDGRASS
CSpeedGrassWrapper::CSpeedGrassWrapper() : m_pMapOutdoor(NULL), m_lpD3DTexure8(NULL)
{
}
CSpeedGrassWrapper::~CSpeedGrassWrapper( )
{
}
int CSpeedGrassWrapper::Draw(float fDensity)
{
	int nTriangleCount = 0;
	return nTriangleCount;
}
bool CSpeedGrassWrapper::InitFromBsfFile(const char* pFilename,
										 unsigned int nNumBlades,
										 unsigned int uiRows,
										 unsigned int uiCols,
										 float fCollisionDistance)
{
	bool bSuccess = false;

	if (pFilename)
	{

		if (ParseBsfFile(pFilename, nNumBlades, uiRows, uiCols, fCollisionDistance))
			bSuccess = true;
	}
	InitGraphics( );

	return bSuccess;
}
float CSpeedGrassWrapper::Color(float fX, float fY, const float* pNormal, float* pTopColor, float* pBottomColor) const
{
	const float c_fColorAdjust = 0.3f;
	const float c_fColorThrow = 1.0f;
	const float c_fColorRandomness = 0.01f;
	const float c_TopLight = 0.75f;

	float afLowColor[4] = { 0.0f }, afHighColor[4] = { 0.0f };
	if (m_pMapOutdoor->GetBrushColor(fX, fY, afLowColor, afHighColor))
	{
		pBottomColor[0] = afLowColor[2];
		pBottomColor[1] = afLowColor[1];
		pBottomColor[2] = afLowColor[0];

		float fColorThrow = GetRandom(0.0f, c_fColorThrow);
		pTopColor[0] = VecInterpolate(pBottomColor[0], afHighColor[2], fColorThrow) + GetRandom(-c_fColorRandomness, c_fColorRandomness);
		pTopColor[1] = VecInterpolate(pBottomColor[1], afHighColor[1], fColorThrow) + GetRandom(-c_fColorRandomness, c_fColorRandomness);
		pTopColor[2] = VecInterpolate(pBottomColor[2], afHighColor[0], fColorThrow) + GetRandom(-c_fColorRandomness, c_fColorRandomness);

		float fLargest = pTopColor[0];
		if (pTopColor[1] > fLargest)
			fLargest = pTopColor[1];
		if (pTopColor[2] > fLargest)
			fLargest = pTopColor[2];
		if (fLargest > 1.0f)
		{
			pTopColor[0] /= fLargest;
			pTopColor[1] /= fLargest;
			pTopColor[2] /= fLargest;
		}
		pTopColor[0] = max(0.0f, pTopColor[0]);
		pTopColor[1] = max(0.0f, pTopColor[1]);
		pTopColor[2] = max(0.0f, pTopColor[2]);
	}

	return afLowColor[3];
}
float CSpeedGrassWrapper::Height(float fX, float fY, float* pNormal) const
{
	float fHeight = 0.0f;
	float afPos[3] = { fX, fY, 0.0f };
	fHeight = m_pMapOutdoor->GetHeight(afPos);

	pNormal[0] = 0.0f;
	pNormal[1] = 0.0f;
	pNormal[2] = 1.0f;

	return fHeight;
}
void CSpeedGrassWrapper::InitGraphics(void)
{
	CGraphicImage * pImage = (CGraphicImage *) CResourceManager::Instance().GetResourcePointer("D:/ymir work/special/brush_2.dds");
	m_GrassImageInstance.SetImagePointer(pImage);
	m_lpD3DTexure8 = m_GrassImageInstance.GetTexturePointer()->GetD3DTexture();
	for (int i = 0; i < m_nNumRegions; ++i)
	{
		SRegion* pRegion = m_pRegions + i;
		const int c_nNumCorners = 4;
		unsigned int uiNumBlades = pRegion->m_vBlades.size( );
		unsigned int uiBufferSize = uiNumBlades * c_nNumCorners * c_nGrassVertexTotalSize;
		unsigned char* pBuffer = new unsigned char[uiBufferSize];
		float* pTexCoords0 = reinterpret_cast<float*>(pBuffer + 0);
		float* pTexCoords1 = reinterpret_cast<float*>(pTexCoords0 + c_nGrassVertexTexture0Size * uiNumBlades * c_nNumCorners / sizeof(float));
		unsigned char* pColors = (unsigned char*) pTexCoords1 + c_nGrassVertexTexture1Size * uiNumBlades * c_nNumCorners;
		float* pPositions = reinterpret_cast<float*>(pColors + c_nGrassVertexColorSize * uiNumBlades * c_nNumCorners);

		for (vector<SBlade>::const_iterator iBlade = pRegion->m_vBlades.begin( ); iBlade != pRegion->m_vBlades.end( ); ++iBlade)
		{
			float fS1 = float(iBlade->m_ucWhichTexture) / c_nNumBladeMaps;
			float fS2 = float(iBlade->m_ucWhichTexture + 1) / c_nNumBladeMaps;

			for (int nCorner = 0; nCorner < c_nNumCorners; ++nCorner)
			{

				switch (nCorner)
				{
				case 0:
					pTexCoords0[0] = fS2;
					pTexCoords0[1] = 1.0f;
					break;
				case 1:
					pTexCoords0[0] = fS1;
					pTexCoords0[1] = 1.0f;
					break;
				case 2:
					pTexCoords0[0] = fS1;
					pTexCoords0[1] = 0.0f;
					break;
				case 3:
					pTexCoords0[0] = fS2;
					pTexCoords0[1] = 0.0f;
					break;
				default:
					assert(false);
				}
				pTexCoords0 += c_nGrassVertexTexture0Size / sizeof(float);
				switch (nCorner)
				{
				case 0:
					pTexCoords1[0] = c_nShaderGrassBillboard;
					pTexCoords1[2] = iBlade->m_fThrow;
					break;
				case 1:
					pTexCoords1[0] = c_nShaderGrassBillboard + 1;
					pTexCoords1[2] = iBlade->m_fThrow;
					break;
				case 2:
					pTexCoords1[0] = c_nShaderGrassBillboard + 2;
					pTexCoords1[2] = 0.0f;
					break;
				case 3:
					pTexCoords1[0] = c_nShaderGrassBillboard + 3;
					pTexCoords1[2] = 0.0f;
					break;
				default:
					assert(false);
				}

				pTexCoords1[1] = iBlade->m_fSize;
				pTexCoords1[3] = iBlade->m_fNoise;
				pTexCoords1 += c_nGrassVertexTexture1Size / sizeof(float);
				unsigned long ulColor = 0;
				if (nCorner == 0 || nCorner == 1)
					ulColor = (int(iBlade->m_afTopColor[0] * 255.0f) << 0) +
						      (int(iBlade->m_afTopColor[1] * 255.0f) << 8) +
						      (int(iBlade->m_afTopColor[2] * 255.0f) << 16) +
						      0xff000000;
				else
					ulColor = (int(iBlade->m_afBottomColor[0] * 255.0f) << 0) +
						      (int(iBlade->m_afBottomColor[1] * 255.0f) << 8) +
						      (int(iBlade->m_afBottomColor[2] * 255.0f) << 16) +
						      0xff000000;
				memcpy(pColors, &ulColor, c_nGrassVertexColorSize);
				pColors += c_nGrassVertexColorSize;
				memcpy(pPositions, iBlade->m_afPos, c_nGrassVertexPositionSize);
				pPositions += c_nGrassVertexPositionSize / sizeof(float);
			}
		}
 		DWORD dwFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
		delete[] pBuffer;
	}
}

#endif 

