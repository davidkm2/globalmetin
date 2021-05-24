
#pragma once
#ifdef USE_SPEEDGRASS
class CIdvTerrain;
class CSpeedGrassRT
{
public:
		CSpeedGrassRT( );
		virtual ~CSpeedGrassRT( );
        
        

		struct SBlade
		{
			SBlade( );
			float				m_afPos[3];
			float				m_afNormal[3];
			float				m_fSize;
			unsigned char		m_ucWhichTexture;
			float				m_fNoise;
			float				m_fThrow;
			float				m_afBottomColor[3];
			float				m_afTopColor[3];
		};
        
        

		struct SRegion
		{
			SRegion( );
			float				m_afCenter[3];
			float				m_afMin[3];
			float				m_afMax[3];
			bool				m_bCulled;
			float				m_fCullingRadius;
			std::vector<SBlade>	m_vBlades;

			CGraphicVertexBuffer	m_VertexBuffer;
		};

		void				DeleteRegions(void);
		const SRegion*		GetRegions(unsigned int& uiNumRegions);
		bool				ParseBsfFile(const char* pFilename, unsigned int nNumBlades, unsigned int uiRows, unsigned int uiCols, float fCollisionDistance = 0.0f);
		bool				CustomPlacement(unsigned int uiRows, unsigned int uiCols);
static	void				RotateAxisFromIdentity(D3DXMATRIX * pMat, const float & c_fAngle, const D3DXVECTOR3 & c_rv3Axis);

        
        

static	const float*		GetUnitBillboard(void)											{ return m_afUnitBillboard; }
        
        

static  void		        GetLodParams(float& fFarDistance, float& fTransitionLength);
static  void		        SetLodParams(float fFarDistance, float fTransitionLength);
        
        

		bool				AllRegionsAreCulled(void) const									{ return m_bAllRegionsCulled; }
		void				Cull(void);
        
        

static	void				SetWindDirection(const float* pWindDir);
static	const float*		GetWindDirection(void);
        
        

static  const float*		GetCameraPos(void);
static  void                SetCamera(const float* pPosition, const double* pModelviewMatrix);
static	void				SetPerspective(float fAspectRatio, float fFieldOfView);
        
        

virtual float				Color(float fX, float fY, const float* pNormal, float* pTopColor, float* pBottomColor) const { return 0.0f; }
virtual float				Height(float fX, float fY, float* pNormal) const { return 0.0f; }

protected:
		void				CreateRegions(const std::vector<SBlade>& vSceneBlades, float fCollisionDistance = 0.0f);
static	void				ComputeFrustum(void);
static	void				ComputeUnitBillboard(void);
		void				ConvertCoordsToCell(const float* pCoords, int* pGridCoords) const;
		unsigned int		GetRegionIndex(unsigned int uiRow, unsigned int uiCol) const	{ return uiRow * m_nNumRegionCols + uiCol; }
static	bool				OutsideFrustum(SRegion* pRegion);
static	float				m_fLodFarDistance;
static	float				m_fLodTransitionLength;
static	float				m_afUnitBillboard[12];
static	float				m_afWindDir[4];
		int					m_nNumRegions;
		int					m_nNumRegionCols;
		int					m_nNumRegionRows;
		SRegion*			m_pRegions;
static	float				m_afCameraOut[3];
static	float				m_afCameraRight[3];
static	float				m_afCameraUp[3];
static	float				m_afCameraPos[3];
static	float				m_fFieldOfView;
static	float				m_fAspectRatio;
static	float				m_afFrustumBox[6];
static	float				m_afFrustumMin[2];
static	float				m_afFrustumMax[2];
static	float				m_afFrustumPlanes[5][4];
		float				m_afBoundingBox[6];
		bool				m_bAllRegionsCulled;
};

extern float VecInterpolate(float fStart, float fEnd, float fPercent);
#endif 