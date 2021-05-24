#pragma once

#include "GrpImage.h"
#include "GrpIndexBuffer.h"
#include "GrpVertexBufferDynamic.h"
#include "Pool.h"

class CGraphicImageInstance
{
	public:
		static DWORD Type();
		BOOL IsType(DWORD dwType);

	public:
		CGraphicImageInstance();
		virtual ~CGraphicImageInstance();

		void Destroy();

		void Render();

		void SetDiffuseColor(float fr, float fg, float fb, float fa);
		void SetPosition(float fx, float fy);
		void SetImagePointer(CGraphicImage* pImage);
		void ReloadImagePointer(CGraphicImage* pImage);
		bool IsEmpty() const;

		int GetWidth();
		int GetHeight();

		CGraphicTexture * GetTexturePointer();
		const CGraphicTexture &	GetTextureReference() const;
		CGraphicImage * GetGraphicImagePointer();

		bool operator == (const CGraphicImageInstance & rhs) const;

		void SetScale(float fx, float fy);
		void SetScale(D3DXVECTOR2 v2Scale);
		const D3DXVECTOR2 & GetScale() const;
		void SetScalePercent(BYTE byPercent);
		void SetScalePivotCenter(bool bScalePivotCenter);
		void RenderCoolTime(float fCoolTime);

	protected:
		void Initialize();

		virtual void OnRender();
		virtual void OnSetImagePointer();

		virtual BOOL OnIsType(DWORD dwType);
		virtual void OnRenderCoolTime(float fCoolTime);

	protected:
		D3DXCOLOR m_DiffuseColor;
		D3DXVECTOR2 m_v2Position;
		CGraphicImage::TRef m_roImage;
		D3DXVECTOR2 m_v2Scale;
		bool m_bScalePivotCenter;

	public:
		static void CreateSystem(UINT uCapacity);
		static void DestroySystem();

		static CGraphicImageInstance* New();
		static void Delete(CGraphicImageInstance* pkImgInst);

		static CDynamicPool<CGraphicImageInstance>		ms_kPool;
};
