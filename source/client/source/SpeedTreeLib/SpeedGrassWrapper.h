
#pragma once

#ifdef USE_SPEEDGRASS
class CMapOutdoor;
class CSpeedGrassWrapper : public CSpeedGrassRT
{
public:
		CSpeedGrassWrapper( );
		virtual ~CSpeedGrassWrapper( );

		void							SetMapOutdoor(CMapOutdoor* pMapOutdoor)	{ m_pMapOutdoor = pMapOutdoor; }
		int								Draw(float fDensity);
		bool							InitFromBsfFile(const char* pFilename,
														unsigned int nNumBlades,
														unsigned int uiRows,
														unsigned int uiCols,
														float fCollisionDistance);

private:
virtual float							Color(float fX, float fY, const float* pNormal, float* pTopColor, float* pBottomColor) const;
virtual	float							Height(float fX, float fY, float* pNormal) const;
		void							InitGraphics(void);

		CMapOutdoor *					m_pMapOutdoor;

		LPDIRECT3DTEXTURE9				m_lpD3DTexure8;

		CGraphicImageInstance			m_GrassImageInstance;
};

#endif 
