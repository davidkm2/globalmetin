
#pragma once
#ifdef CMAKE_BUILD_SYSTEM
#include <SpeedTreeRT_s.h>
#else
#include <SpeedTreeRT.h>
#endif
#include "SpeedTreeWrapper.h"

#include <vector>
#include <map>
#define Forest_RenderBranches		(1 << 0)
#define Forest_RenderLeaves			(1 << 1)
#define Forest_RenderFronds			(1 << 2)
#define Forest_RenderBillboards		(1 << 3)
#define Forest_RenderAll			((1 << 4) - 1)
#define Forest_RenderToShadow		(1 << 5)
#define Forest_RenderToMiniMap		(1 << 6)
class CSpeedTreeForest
{
	public:
		typedef std::map<DWORD, CSpeedTreeWrapper *> TTreeMap;

	public:
		CSpeedTreeForest();
		virtual ~CSpeedTreeForest();

		void						ClearMainTree();
		BOOL						GetMainTree(DWORD dwCRC, CSpeedTreeWrapper ** ppMainTree, const char * c_pszFileName);
		CSpeedTreeWrapper *			GetMainTree(DWORD dwCRC);
		void						DeleteMainTree(DWORD dwCRC);

		CSpeedTreeWrapper *			CreateInstance(float x, float y, float z, DWORD dwTreeCRC, const char * c_pszTreeName);
		void						DeleteInstance(CSpeedTreeWrapper * pTree);
		void						UpdateSystem(float fCurrentTime);

		void						Clear();

		void						SetLight(const float * afDirection, const float * afAmbient, const float * afDiffuse);
		void						SetFog(float fFogNear, float fFogFar);
		const float *				GetExtents(void) const						{ return m_afForestExtents; }
		float						GetWindStrength(void) const					{ return m_fWindStrength; }
		void						SetWindStrength(float fStrength);
		void						SetupWindMatrices(float fTimeInSecs);
		virtual	void				UploadWindMatrix(unsigned int uiLocation, const float* pMatrix) const = 0;
		virtual void				Render(unsigned long ulRenderBitVector) = 0;

	protected:
		TTreeMap					m_pMainTreeMap;

		float						m_afLighting[12];
		float						m_afFog[4];

	private:
		void						AdjustExtents(float x, float y, float z);

		float						m_afForestExtents[6];
		float						m_fWindStrength;

		float						m_fAccumTime;
};
