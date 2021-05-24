
#pragma once
#include <d3d9/d3d9.h>
#include <d3d9/d3d9types.h>
#include <d3d9/d3dx9.h>
class CSpeedTreeMaterial
{
	public:
		CSpeedTreeMaterial()
		{
			m_cMaterial.Ambient.r = m_cMaterial.Diffuse.r = m_cMaterial.Specular.r = m_cMaterial.Emissive.r = 1.0f;
			m_cMaterial.Ambient.g = m_cMaterial.Diffuse.g = m_cMaterial.Specular.g = m_cMaterial.Emissive.g = 1.0f;
			m_cMaterial.Ambient.b = m_cMaterial.Diffuse.b = m_cMaterial.Specular.b = m_cMaterial.Emissive.b = 1.0f;
			m_cMaterial.Ambient.a = m_cMaterial.Diffuse.a = m_cMaterial.Specular.a = m_cMaterial.Emissive.a = 1.0f;
			m_cMaterial.Power = 5.0f;
		}

		void Set(const float * pMaterialArray)
		{
			memcpy(&m_cMaterial.Diffuse, pMaterialArray, 3 * sizeof(float));
			m_cMaterial.Diffuse.a = 1.0f;

			memcpy(&m_cMaterial.Ambient, pMaterialArray + 3, 3 * sizeof(float));
			m_cMaterial.Ambient.a = 1.0f;

			memcpy(&m_cMaterial.Specular, pMaterialArray + 6, 3 * sizeof(float));
			m_cMaterial.Specular.a = 1.0f;

			memcpy(&m_cMaterial.Emissive, pMaterialArray + 9, 3 * sizeof(float));
			m_cMaterial.Emissive.a = 1.0f;

			m_cMaterial.Power = pMaterialArray[12];
		}
		
		D3DMATERIAL9 * Get()
		{
			return &m_cMaterial;
		}

	private:
		D3DMATERIAL9 m_cMaterial;
};
