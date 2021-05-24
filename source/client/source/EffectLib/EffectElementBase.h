#pragma once

#include "Type.h"

class CEffectElementBase
{
	public:
		CEffectElementBase();
		virtual ~CEffectElementBase();

		void Clear();
		bool isData();

		BOOL LoadScript(CTextFileLoader & rTextFileLoader);

		void GetPosition(float fTime, D3DXVECTOR3 & rPosition);
		float GetStartTime();

	protected:
		virtual void OnClear() = 0;
		virtual bool OnIsData() = 0;
		virtual BOOL OnLoadScript(CTextFileLoader & rTextFileLoader) = 0;

	protected:
		float						m_fStartTime;
		TTimeEventTablePosition		m_TimeEventTablePosition;
};
