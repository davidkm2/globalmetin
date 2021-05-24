
#pragma once

#include "GrpImageInstance.h"
#include "GrpScreen.h"

#include <float.h>
#include <string>
#include <vector>
const   float	c_fHalfMaxBright = 0.45f;
const   float	c_fDistanceScale = 1.0f;
const   int     c_nDepthTestDimension = 15;
class CFlare
{
public:
	void Draw(float fBrightScale, int nWidth, int nHeight, int nX, int nY);
	void Init(std::string strPath);

	CFlare();
	virtual ~CFlare();

private:
	struct SFlarePiece
	{
		SFlarePiece() :
	m_fPosition(0.0f),
		m_fWidth(0.0f),
		m_pColor(NULL)
	{
	}
	CGraphicImageInstance m_imageInstance;
	float	m_fPosition;
	float	m_fWidth;
	float *	m_pColor;
	};

	std::vector<SFlarePiece *> m_vFlares;
};
class CLensFlare : public CScreen
{
public:
	CLensFlare();
	virtual ~CLensFlare();

	void			Compute(const D3DXVECTOR3 & c_rv3LightDirection); 

	void			DrawBeforeFlare();
	void			DrawAfterFlare();
	void			DrawFlare();

	void			SetMainFlare(std::string strSunFile, float fSunSize);
	void			Initialize(std::string strPath);

	void            SetFlareLocation(double dX, double dY);

	void            SetVisible(bool bState)		{ m_bFlareVisible = bState; }
	bool			IsVisible()					{ return m_bFlareVisible; }

	void            SetBrightnesses(float fBeforeBright, float fAfterBright);

	void            ReadControlPixels();
	void            AdjustBrightness();

	void			CharacterizeFlare(bool bEnabled, bool bShowMainFlare, float fMaxBrightness, const D3DXCOLOR & c_rColor);

protected:
	float			Interpolate(float fStart, float fEnd, float fPercent);

private:
	float	        m_afFlarePos[2], m_afFlareWinPos[2];
	float	        m_fBeforeBright, m_fAfterBright;
	bool        	m_bFlareVisible, m_bDrawFlare, m_bDrawBrightScreen;
	float			m_fSunSize;
	CFlare			m_cFlare;
	float *			m_pControlPixels;
	float *			m_pTestPixels;
	bool			m_bEnabled;
	bool			m_bShowMainFlare;
	float			m_fMaxBrightness;
	float			m_afColor[4];

	CGraphicImageInstance m_SunFlareImageInstance;

	void            ReadDepthPixels(float * pPixels);
	void			ClampBrightness();
};
