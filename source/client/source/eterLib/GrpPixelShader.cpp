#include "StdAfx.h"
#include "GrpPixelShader.h"
#include "GrpD3DXBuffer.h"
#include "StateManager.h"

CPixelShader::CPixelShader()
{
	Initialize();
}

CPixelShader::~CPixelShader()
{
	Destroy();
}

void CPixelShader::Initialize()
{
	m_handle=0;
}

void CPixelShader::Destroy()
{
	if (m_handle)
	{

		m_handle=0;
	}
}

bool CPixelShader::CreateFromDiskFile(const char* c_szFileName)
{
	Destroy();
	return false;
}

void CPixelShader::Set()
{

}
