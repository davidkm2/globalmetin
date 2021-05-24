#include "StdAfx.h"
#include "GrpImageInstance.h"
#include "StateManager.h"

#include "../EterBase/CRC32.h"
CDynamicPool<CGraphicImageInstance>		CGraphicImageInstance::ms_kPool;

void CGraphicImageInstance::CreateSystem(UINT uCapacity)
{
	ms_kPool.Create(uCapacity);
}

void CGraphicImageInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CGraphicImageInstance* CGraphicImageInstance::New()
{
	return ms_kPool.Alloc();
}

void CGraphicImageInstance::Delete(CGraphicImageInstance* pkImgInst)
{
	pkImgInst->Destroy();
	ms_kPool.Free(pkImgInst);
}

void CGraphicImageInstance::Render()
{
	if (IsEmpty())
		return;

	assert(!IsEmpty());

	OnRender();
}

void CGraphicImageInstance::OnRender()
{
	CGraphicImage * pImage = m_roImage.GetPointer();
	CGraphicTexture * pTexture = pImage->GetTexturePointer();

	float fimgWidth = pImage->GetWidth() * m_v2Scale.x;
	float fimgHeight = pImage->GetHeight() * m_v2Scale.y;

	const RECT& c_rRect = pImage->GetRectReference();
	float texReverseWidth = 1.0f / float(pTexture->GetWidth());
	float texReverseHeight = 1.0f / float(pTexture->GetHeight());
	float su = c_rRect.left * texReverseWidth;
	float sv = c_rRect.top * texReverseHeight;
	float eu = (c_rRect.left + (c_rRect.right-c_rRect.left)) * texReverseWidth;
	float ev = (c_rRect.top + (c_rRect.bottom-c_rRect.top)) * texReverseHeight;
	TPDTVertex vertices[4];
	vertices[0].position.x	= m_v2Position.x-0.5f;
	vertices[0].position.y	= m_v2Position.y-0.5f;
	vertices[0].position.z	= 0.0f;
	vertices[0].texCoord	= TTextureCoordinate(su, sv);
	vertices[0].diffuse		= m_DiffuseColor;

	vertices[1].position.x	= m_v2Position.x + fimgWidth-0.5f;
	vertices[1].position.y	= m_v2Position.y-0.5f;
	vertices[1].position.z	= 0.0f;
	vertices[1].texCoord	= TTextureCoordinate(eu, sv);
	vertices[1].diffuse		= m_DiffuseColor;

	vertices[2].position.x	= m_v2Position.x-0.5f;
	vertices[2].position.y	= m_v2Position.y + fimgHeight-0.5f;
	vertices[2].position.z	= 0.0f;
	vertices[2].texCoord	= TTextureCoordinate(su, ev);
	vertices[2].diffuse		= m_DiffuseColor;

	vertices[3].position.x	= m_v2Position.x + fimgWidth-0.5f;
	vertices[3].position.y	= m_v2Position.y + fimgHeight-0.5f;
	vertices[3].position.z	= 0.0f;
	vertices[3].texCoord	= TTextureCoordinate(eu, ev);
	vertices[3].diffuse		= m_DiffuseColor;

	if (m_bScalePivotCenter)
	{
		vertices[0].texCoord = TTextureCoordinate(eu, sv);
		vertices[1].texCoord = TTextureCoordinate(su, sv);
		vertices[2].texCoord = TTextureCoordinate(eu, ev);
		vertices[3].texCoord = TTextureCoordinate(su, ev);
	}
	if (CGraphicBase::SetPDTStream(vertices, 4))
	{
		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);

		STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
	}
}

void CGraphicImageInstance::RenderCoolTime(float fCoolTime)
{
	if (IsEmpty())
		return;

	assert(!IsEmpty());

	OnRenderCoolTime(fCoolTime);
}

void CGraphicImageInstance::OnRenderCoolTime(float fCoolTime)
{
	if (fCoolTime >= 1.0f)
		fCoolTime = 1.0f;

	CGraphicImage * pImage = m_roImage.GetPointer();
	CGraphicTexture * pTexture = pImage->GetTexturePointer();

	float fimgWidth = pImage->GetWidth() * m_v2Scale.x;
	float fimgHeight = pImage->GetHeight() * m_v2Scale.y;
	float fimgWidthHalf = fimgWidth * 0.5f;
	float fimgHeightHalf = fimgHeight * 0.5f;

	const RECT& c_rRect = pImage->GetRectReference();
	float texReverseWidth = 1.0f / float(pTexture->GetWidth());
	float texReverseHeight = 1.0f / float(pTexture->GetHeight());
	float su = c_rRect.left * texReverseWidth;
	float sv = c_rRect.top * texReverseHeight;
	float eu = c_rRect.right * texReverseWidth;
	float ev = c_rRect.bottom * texReverseHeight;
	float euh = eu * 0.5f;
	float evh = ev * 0.5f;
	float fxCenter = m_v2Position.x + fimgWidthHalf - 0.5f;
	float fyCenter = m_v2Position.y + fimgHeightHalf - 0.5f;

	if (fCoolTime < 1.0f)
	{
		if (fCoolTime < 0.0)
			fCoolTime = 0.0;

		const int c_iTriangleCountPerBox = 8;
		static D3DXVECTOR2 s_v2BoxPos[c_iTriangleCountPerBox] =
		{
			D3DXVECTOR2(-1.0f, -1.0f),
			D3DXVECTOR2(-1.0f,  0.0f),
			D3DXVECTOR2(-1.0f, +1.0f),
			D3DXVECTOR2( 0.0f, +1.0f),
			D3DXVECTOR2(+1.0f, +1.0f),
			D3DXVECTOR2(+1.0f,  0.0f),
			D3DXVECTOR2(+1.0f, -1.0f),
			D3DXVECTOR2( 0.0f, -1.0f),
		};
		 
		D3DXVECTOR2 v2TexPos[c_iTriangleCountPerBox] =
		{
			D3DXVECTOR2( su,  sv),
			D3DXVECTOR2( su, evh),
			D3DXVECTOR2( su,  ev),
			D3DXVECTOR2(euh,  ev),
			D3DXVECTOR2( eu,  ev),
			D3DXVECTOR2( eu, evh),
			D3DXVECTOR2( eu,  sv),
			D3DXVECTOR2(euh,  sv),
		};

		int iTriCount = int(8.0f - 8.0f * fCoolTime);
		float fLastPercentage = (8.0f - 8.0f * fCoolTime) - iTriCount;

		std::vector<TPDTVertex> vertices;
		TPDTVertex vertex;
		vertex.position = TPosition(fxCenter, fyCenter, 0.0f);
		vertex.texCoord = TTextureCoordinate(euh, evh);
		vertex.diffuse = m_DiffuseColor;
		vertices.push_back(vertex);

		vertex.position = TPosition(fxCenter, fyCenter - fimgHeightHalf - 0.5f, 0.0f);
		vertex.texCoord = TTextureCoordinate(euh, sv);
		vertex.diffuse = m_DiffuseColor;
		vertices.push_back(vertex);

		if (iTriCount > 0)
		{
			for (int j = 0; j < iTriCount; ++j)
			{
				vertex.position = TPosition(fxCenter + (s_v2BoxPos[j].x * fimgWidthHalf) - 0.5f,
											fyCenter + (s_v2BoxPos[j].y * fimgHeightHalf) - 0.5f,
											0.0f);
				vertex.texCoord = TTextureCoordinate(v2TexPos[j & (c_iTriangleCountPerBox - 1)].x,
													 v2TexPos[j & (c_iTriangleCountPerBox - 1)].y);
				vertex.diffuse = m_DiffuseColor;
				vertices.push_back(vertex);
			}
		}

		if (fLastPercentage > 0.0f)
		{
			D3DXVECTOR2 * pv2Pos;
			D3DXVECTOR2 * pv2LastPos;
			assert((iTriCount+8)%8>=0&&(iTriCount+8)%8<8);
			assert((iTriCount+7)%8>=0&&(iTriCount+7)%8<8);
			pv2LastPos=&s_v2BoxPos[(iTriCount+8)%8];
			pv2Pos=&s_v2BoxPos[(iTriCount+7)%8];
			float fxShit = (pv2LastPos->x - pv2Pos->x) * fLastPercentage + pv2Pos->x;
			float fyShit = (pv2LastPos->y - pv2Pos->y) * fLastPercentage + pv2Pos->y;
			vertex.position = TPosition(fimgWidthHalf * fxShit + fxCenter - 0.5f,
										fimgHeightHalf * fyShit + fyCenter - 0.5f,
										0.0f);
			vertex.texCoord = TTextureCoordinate(euh * fxShit + euh,
												 evh * fyShit + evh);
			vertex.diffuse = m_DiffuseColor;
			vertices.push_back(vertex);
			++iTriCount;
		}

		if (vertices.empty())
			return;

		STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		if (CGraphicBase::SetPDTStream(&vertices[0], vertices.size()))
		{
			CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_TRI);
			STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
			STATEMANAGER.SetTexture(1, NULL);
			STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
			STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLEFAN, 0, iTriCount);
		}
		STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	}
}

const CGraphicTexture & CGraphicImageInstance::GetTextureReference() const
{
	return m_roImage->GetTextureReference();
}

CGraphicTexture * CGraphicImageInstance::GetTexturePointer()
{
	CGraphicImage* pkImage = m_roImage.GetPointer();
	return pkImage ? pkImage->GetTexturePointer() : NULL;
}

CGraphicImage * CGraphicImageInstance::GetGraphicImagePointer()
{
	return m_roImage.GetPointer();
}

int CGraphicImageInstance::GetWidth()
{
	if (IsEmpty())
		return 0;

	return m_roImage->GetWidth();
}

int CGraphicImageInstance::GetHeight()
{
	if (IsEmpty())
		return 0;

	return m_roImage->GetHeight();
}

void CGraphicImageInstance::SetDiffuseColor(float fr, float fg, float fb, float fa)
{
	m_DiffuseColor.r = fr;
	m_DiffuseColor.g = fg;
	m_DiffuseColor.b = fb;
	m_DiffuseColor.a = fa;
}
void CGraphicImageInstance::SetPosition(float fx, float fy)
{
	m_v2Position.x = fx;
	m_v2Position.y = fy;
}

void CGraphicImageInstance::SetImagePointer(CGraphicImage * pImage)
{
	m_roImage.SetPointer(pImage);

	OnSetImagePointer();
}

void CGraphicImageInstance::ReloadImagePointer(CGraphicImage * pImage)
{
	if (m_roImage.IsNull())
	{
		SetImagePointer(pImage);
		return;
	}

	CGraphicImage * pkImage = m_roImage.GetPointer();

	if (pkImage)
		pkImage->Reload();
}

bool CGraphicImageInstance::IsEmpty() const
{
	if (!m_roImage.IsNull() && !m_roImage->IsEmpty())
		return false;

	return true;
}

bool CGraphicImageInstance::operator == (const CGraphicImageInstance & rhs) const
{
	return (m_roImage.GetPointer() == rhs.m_roImage.GetPointer());
}

DWORD CGraphicImageInstance::Type()
{
	static DWORD s_dwType = GetCRC32("CGraphicImageInstance", strlen("CGraphicImageInstance"));
	return (s_dwType);
}

BOOL CGraphicImageInstance::IsType(DWORD dwType)
{
	return OnIsType(dwType);
}

BOOL CGraphicImageInstance::OnIsType(DWORD dwType)
{
	if (CGraphicImageInstance::Type() == dwType)
		return TRUE;

	return FALSE;
}

void CGraphicImageInstance::SetScale(float fx, float fy)
{
	m_v2Scale.x = fx;
	m_v2Scale.y = fy;
}

void CGraphicImageInstance::SetScale(D3DXVECTOR2 v2Scale)
{
	m_v2Scale = v2Scale;
}

void CGraphicImageInstance::SetScalePercent(BYTE byPercent)
{
	m_v2Scale.x *= byPercent;
	m_v2Scale.y *= byPercent;
}

const D3DXVECTOR2 & CGraphicImageInstance::GetScale() const
{
	return m_v2Scale;
}

void CGraphicImageInstance::SetScalePivotCenter(bool bScalePivotCenter)
{
	m_bScalePivotCenter = bScalePivotCenter;
}

void CGraphicImageInstance::OnSetImagePointer()
{
}

void CGraphicImageInstance::Initialize()
{
	m_DiffuseColor.r = m_DiffuseColor.g = m_DiffuseColor.b = m_DiffuseColor.a = 1.0f;
	m_v2Position.x = m_v2Position.y = 0.0f;
	m_v2Scale.x = m_v2Scale.y = 1.0f;
	m_bScalePivotCenter = false;
}

void CGraphicImageInstance::Destroy()
{
	m_roImage.SetPointer(NULL); 
	Initialize();
}

CGraphicImageInstance::CGraphicImageInstance()
{
	Initialize();
}

CGraphicImageInstance::~CGraphicImageInstance()
{
	Destroy();
}
