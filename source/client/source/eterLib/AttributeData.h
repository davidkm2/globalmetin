#pragma once

#include "Resource.h"
#include "Ref.h"
#include "CollisionData.h"

typedef struct SHeightData
{
	char szName[32+1];
	std::vector<D3DXVECTOR3> v3VertexVector;
} THeightData;

typedef std::vector<THeightData> THeightDataVector;

class CAttributeData : public CResource
{
	public:
		typedef CRef<CAttributeData> TRef;

	public:
		static TType Type();

	public:
		CAttributeData(const char * c_szFileName);
		virtual ~CAttributeData();
		const CStaticCollisionDataVector & GetCollisionDataVector() const;
		const THeightDataVector & GetHeightDataVector() const;

		size_t AddCollisionData(const CStaticCollisionData& collisionData);

		DWORD GetHeightDataCount() const;
		BOOL GetHeightDataPointer(DWORD dwIndex, const THeightData ** c_ppHeightData) const;

		float GetMaximizeRadius();

	protected:
		bool OnLoad(int iSize, const void * c_pvBuf);
		void OnClear();
		bool OnIsEmpty() const;
		bool OnIsType(TType type);
		void OnSelfDestruct();

	protected:
		float m_fMaximizeRadius;
		CStaticCollisionDataVector m_StaticCollisionDataVector;
		THeightDataVector m_HeightDataVector;
};
