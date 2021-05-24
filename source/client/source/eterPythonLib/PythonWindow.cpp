#include "StdAfx.h"

#include "../EterBase/CRC32.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"
#include "PythonWindowManager.h"

#ifdef ENABLE_RENDER_TARGET
#include "..\UserInterface\StdAfx.h"
#include "..\UserInterface\InstanceBase.h"
#include "..\UserInterface\PythonApplication.h"
#include "..\EterLib\Camera.h"
#endif

BOOL g_bOutlineBoxEnable = FALSE;

namespace UI
{

	CWindow::CWindow(PyObject * ppyObject) :
		m_x(0),
		m_y(0),
		m_lWidth(0),
		m_lHeight(0),
		m_poHandler(ppyObject),
		m_bShow(false),
		m_pParent(NULL),
		m_dwFlag(0),
		m_isUpdatingChildren(FALSE)
	{
#ifdef __TEST__
		static DWORD DEBUG_dwGlobalCounter=0;
		DEBUG_dwCounter=DEBUG_dwGlobalCounter++;

		m_strName = "!!debug";
#endif

		m_HorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_VerticalAlign = VERTICAL_ALIGN_TOP;
		m_rect.bottom = m_rect.left = m_rect.right = m_rect.top = 0;
		m_limitBiasRect.bottom = m_limitBiasRect.left = m_limitBiasRect.right = m_limitBiasRect.top = 0;
	}

	CWindow::~CWindow()
	{
	}

	DWORD CWindow::Type()
	{
		static DWORD s_dwType = GetCRC32("CWindow", strlen("CWindow"));
		return (s_dwType);
	}

	BOOL CWindow::IsType(DWORD dwType)
	{
		return OnIsType(dwType);
	}

	BOOL CWindow::OnIsType(DWORD dwType)
	{
		if (CWindow::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	struct FClear
	{
		void operator () (CWindow * pWin)
		{
			pWin->Clear();
		}
	};

	void CWindow::Clear()
	{
		std::for_each(m_pChildList.begin(), m_pChildList.end(), FClear());
		m_pChildList.clear();

		m_pParent = NULL;
		DestroyHandle();
		Hide();
	}

	void CWindow::DestroyHandle()
	{
		m_poHandler = NULL;
	}

	void CWindow::Show()
	{
		m_bShow = true;
	}

	void CWindow::Hide()
	{
		m_bShow = false;
	}
	bool CWindow::IsRendering()
	{
		if (!IsShow())
			return false;

		if (!m_pParent)
			return true;

		return m_pParent->IsRendering();
	}

	void CWindow::__RemoveReserveChildren()
	{
		if (m_pReserveChildList.empty())
			return;

		TWindowContainer::iterator it;
		for(it = m_pReserveChildList.begin(); it != m_pReserveChildList.end(); ++it)
		{
			m_pChildList.remove(*it);
		}
		m_pReserveChildList.clear();
	}

	void CWindow::Update()
	{
		if (!IsShow())
			return;

		__RemoveReserveChildren();

		OnUpdate();

		m_isUpdatingChildren = TRUE;
		TWindowContainer::iterator it;
		for(it = m_pChildList.begin(); it != m_pChildList.end();)
		{
			TWindowContainer::iterator it_next = it;
			++it_next;
			(*it)->Update();
			it = it_next;
		}
		m_isUpdatingChildren = FALSE;
	}

	void CWindow::Render()
	{
		if (!IsShow())
			return;

		OnRender();

		if (g_bOutlineBoxEnable)
		{
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f);
			CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
		}

		std::for_each(m_pChildList.begin(), m_pChildList.end(), std::void_mem_fun(&CWindow::Render));
	}

	void CWindow::OnUpdate()
	{
		if (!m_poHandler)
			return;

		if (!IsShow())
			return;

		static PyObject* poFuncName_OnUpdate = PyString_InternFromString("OnUpdate");
		PyCallClassMemberFunc_ByPyString(m_poHandler, poFuncName_OnUpdate, BuildEmptyTuple());

	}

	void CWindow::OnRender()
	{
		if (!m_poHandler)
			return;

		if (!IsShow())
			return;
		PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
	}

	void CWindow::SetName(const char * c_szName)
	{
		m_strName = c_szName;
	}

	void CWindow::SetSize(long width, long height)
	{
		m_lWidth = width;
		m_lHeight = height;

		m_rect.right = m_rect.left + m_lWidth;
		m_rect.bottom = m_rect.top + m_lHeight;
	}

	void CWindow::SetHorizontalAlign(DWORD dwAlign)
	{
		m_HorizontalAlign = (EHorizontalAlign)dwAlign;
		UpdateRect();
	}

	void CWindow::SetVerticalAlign(DWORD dwAlign)
	{
		m_VerticalAlign = (EVerticalAlign)dwAlign;
		UpdateRect();
	}

	void CWindow::SetPosition(long x, long y)
	{
		m_x = x;
		m_y = y;

		UpdateRect();
	}

	void CWindow::GetPosition(long * plx, long * ply)
	{
		*plx = m_x;
		*ply = m_y;
	}

	long CWindow::UpdateRect()
	{
		m_rect.top		= m_y;
		if (m_pParent)
		{
			switch (m_VerticalAlign)
			{
				case VERTICAL_ALIGN_BOTTOM:
					m_rect.top = m_pParent->GetHeight() - m_rect.top;
					break;
				case VERTICAL_ALIGN_CENTER:
					m_rect.top = (m_pParent->GetHeight() - GetHeight()) / 2 + m_rect.top;
					break;
			}
			m_rect.top += m_pParent->m_rect.top;
		}
		m_rect.bottom	= m_rect.top + m_lHeight;

#if defined( _USE_CPP_RTL_FLIP )
		if( m_pParent == NULL ) {
			m_rect.left		= m_x;
			m_rect.right	= m_rect.left + m_lWidth;
		} else {
			if( m_pParent->IsFlag(UI::CWindow::FLAG_RTL) == true ) {
				m_rect.left = m_pParent->GetWidth() - m_lWidth - m_x;
				switch (m_HorizontalAlign)
				{
					case HORIZONTAL_ALIGN_RIGHT:
						m_rect.left = - m_x;
						break;
					case HORIZONTAL_ALIGN_CENTER:
						m_rect.left = m_pParent->GetWidth() / 2 - GetWidth() - m_x;
						break;
				}
				m_rect.left += m_pParent->m_rect.left;
				m_rect.right = m_rect.left + m_lWidth;
			} else {
				m_rect.left		= m_x;
				switch (m_HorizontalAlign)
				{
					case HORIZONTAL_ALIGN_RIGHT:
						m_rect.left = m_pParent->GetWidth() - m_rect.left;
						break;
					case HORIZONTAL_ALIGN_CENTER:
						m_rect.left = (m_pParent->GetWidth() - GetWidth()) / 2 + m_rect.left;
						break;
				}
				m_rect.left += m_pParent->m_rect.left;
				m_rect.right = m_rect.left + m_lWidth;
			}
		}
#else
		m_rect.left		= m_x;
		if (m_pParent)
		{
			switch (m_HorizontalAlign)
			{
				case HORIZONTAL_ALIGN_RIGHT:
					m_rect.left = ::abs(m_pParent->GetWidth()) - m_rect.left;
					break;
				case HORIZONTAL_ALIGN_CENTER:
					m_rect.left = m_pParent->GetWidth() / 2 - GetWidth() / 2 + m_rect.left;
					break;
			}
			m_rect.left += 0L < m_pParent->GetWidth() ? m_pParent->m_rect.left : m_pParent->m_rect.right + ::abs(m_pParent->GetWidth());
		}
		m_rect.right = m_rect.left + m_lWidth;
#endif
		std::for_each(m_pChildList.begin(), m_pChildList.end(), std::mem_fn(&CWindow::UpdateRect));

		OnChangePosition();

		return 1;
	}

	void CWindow::GetLocalPosition(long & rlx, long & rly)
	{
		rlx = rlx - m_rect.left;
		rly = rly - m_rect.top;
	}

	void CWindow::GetMouseLocalPosition(long & rlx, long & rly)
	{
		CWindowManager::Instance().GetMousePosition(rlx, rly);
		rlx = rlx - m_rect.left;
		rly = rly - m_rect.top;
	}

	void CWindow::AddChild(CWindow * pWin)
	{
		m_pChildList.push_back(pWin);
		pWin->m_pParent = this;
	}

	CWindow * CWindow::GetRoot()
	{
		if (m_pParent)
			if (m_pParent->IsWindow())
				return m_pParent->GetRoot();

		return this;
	}

	CWindow * CWindow::GetParent()
	{
		return m_pParent;
	}

	bool CWindow::IsChild(CWindow * pWin)
	{
		std::list<CWindow *>::iterator itor = m_pChildList.begin();

		while (itor != m_pChildList.end())
		{
			if (*itor == pWin)
				return true;

			++itor;
		}

		return false;
	}

	void CWindow::DeleteChild(CWindow * pWin)
	{
		if (m_isUpdatingChildren)
		{
			m_pReserveChildList.push_back(pWin);
		}
		else
		{
			m_pChildList.remove(pWin);
		}
	}

	void CWindow::SetTop(CWindow * pWin)
	{
		if (!pWin->IsFlag(CWindow::FLAG_FLOAT))
			return;

		TWindowContainer::iterator itor = std::find(m_pChildList.begin(), m_pChildList.end(), pWin);
		if (m_pChildList.end() != itor)
		{
			m_pChildList.push_back(*itor);
			m_pChildList.erase(itor);

			pWin->OnTop();
		}
		else
		{
			TraceError(" CWindow::SetTop - Failed to find child window\n");
		}
	}

	void CWindow::OnMouseDrag(long lx, long ly)
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseDrag", Py_BuildValue("(ii)", lx, ly));
	}

	void CWindow::OnMoveWindow(long lx, long ly)
	{
		PyCallClassMemberFunc(m_poHandler, "OnMoveWindow", Py_BuildValue("(ii)", lx, ly));
	}

	void CWindow::OnSetFocus()
	{

		PyCallClassMemberFunc(m_poHandler, "OnSetFocus", BuildEmptyTuple());
	}

	void CWindow::OnKillFocus()
	{
		PyCallClassMemberFunc(m_poHandler, "OnKillFocus", BuildEmptyTuple());
	}

	void CWindow::OnMouseOverIn()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
	}

	void CWindow::OnMouseOverOut()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
	}

	void CWindow::OnMouseOver()
	{
	}

	void CWindow::OnDrop()
	{
		PyCallClassMemberFunc(m_poHandler, "OnDrop", BuildEmptyTuple());
	}

	void CWindow::OnTop()
	{
		PyCallClassMemberFunc(m_poHandler, "OnTop", BuildEmptyTuple());
	}

	void CWindow::OnIMEUpdate()
	{
		PyCallClassMemberFunc(m_poHandler, "OnIMEUpdate", BuildEmptyTuple());
	}

	BOOL CWindow::RunIMETabEvent()
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMETabEvent())
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunIMETabEvent())
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunIMEReturnEvent()
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMEReturnEvent())
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunIMEReturnEvent())
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunIMEKeyDownEvent(int ikey)
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMEKeyDownEvent(ikey))
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunIMEKeyDownEvent(ikey))
				return TRUE;
		}

		return FALSE;
	}

	CWindow * CWindow::RunKeyDownEvent(int ikey)
	{
		if (OnKeyDown(ikey))
			return this;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->IsShow())
			{
				CWindow * pProcessedWindow = pWindow->RunKeyDownEvent(ikey);
				if (NULL != pProcessedWindow)
				{
					return pProcessedWindow;
				}
			}
		}

		return NULL;
	}

	BOOL CWindow::RunKeyUpEvent(int ikey)
	{
		if (OnKeyUp(ikey))
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->IsShow())
			if (pWindow->RunKeyUpEvent(ikey))
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunPressEscapeKeyEvent()
	{
		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->IsShow())
			if (pWindow->RunPressEscapeKeyEvent())
				return TRUE;
		}

		if (OnPressEscapeKey())
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::RunPressExitKeyEvent()
	{
		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunPressExitKeyEvent())
				return TRUE;

			if (pWindow->IsShow())
			if (pWindow->OnPressExitKey())
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::OnMouseLeftButtonDown()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseLeftButtonUp()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonUp", BuildEmptyTuple());
		return TRUE; 
	}

	BOOL CWindow::OnMouseLeftButtonDoubleClick()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonDoubleClick", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonDown()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonUp()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonUp", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonDoubleClick()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonDoubleClick", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseMiddleButtonDown()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseMiddleButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseMiddleButtonUp()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseMiddleButtonUp", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMETabEvent()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMETab", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEReturnEvent()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEReturn", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEKeyDownEvent(int ikey)
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEKeyDown", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEChangeCodePage()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEChangeCodePage", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEOpenCandidateListEvent()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEOpenCandidateList", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMECloseCandidateListEvent()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMECloseCandidateList", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEOpenReadingWndEvent()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEOpenReadingWnd", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMECloseReadingWndEvent()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMECloseReadingWnd", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnKeyDown(int ikey)
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnKeyDown", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnKeyUp(int ikey)
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnKeyUp", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnPressEscapeKey()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnPressEscapeKey", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnPressExitKey()
	{
		int32_t lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnPressExitKey", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}
	bool CWindow::IsIn(long x, long y)
	{
		if (x >= m_rect.left && x <= m_rect.right)
			if (y >= m_rect.top && y <= m_rect.bottom)
				return true;

		return false;
	}

	bool CWindow::IsIn()
	{
		long lx, ly;
		UI::CWindowManager::Instance().GetMousePosition(lx, ly);

		return IsIn(lx, ly);
	}

	CWindow * CWindow::PickWindow(long x, long y)
	{
		std::list<CWindow *>::reverse_iterator ritor = m_pChildList.rbegin();
		for (; ritor != m_pChildList.rend(); ++ritor)
		{
			CWindow * pWin = *ritor;
			if (pWin->IsShow())
			{
				if (!pWin->IsFlag(CWindow::FLAG_IGNORE_SIZE))
				{
					if (!pWin->IsIn(x, y)) {
						if (0L <= pWin->GetWidth()) {
							continue;
						}
					}
				}

				CWindow * pResult = pWin->PickWindow(x, y);
				if (pResult)
					return pResult;
			}
		}

		if (IsFlag(CWindow::FLAG_NOT_PICK))
			return NULL;

		return (this);
	}

	CWindow * CWindow::PickTopWindow(long x, long y)
	{
		std::list<CWindow *>::reverse_iterator ritor = m_pChildList.rbegin();
		for (; ritor != m_pChildList.rend(); ++ritor)
		{
			CWindow * pWin = *ritor;
			if (pWin->IsShow())
				if (pWin->IsIn(x, y))
					if (!pWin->IsFlag(CWindow::FLAG_NOT_PICK))
						return pWin;
		}

		return NULL;
	}
	CBox::CBox(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CBox::~CBox()
	{
	}

	void CBox::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CBox::OnRender()
	{
		CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
		CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	}
	CBar::CBar(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CBar::~CBar()
	{
	}

	void CBar::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CBar::OnRender()
	{
		CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
		CPythonGraphic::Instance().RenderBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	}
	CLine::CLine(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CLine::~CLine()
	{
	}

	void CLine::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CLine::OnRender()
	{
		CPythonGraphic & rkpyGraphic = CPythonGraphic::Instance();
		rkpyGraphic.SetDiffuseColor(m_dwColor);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	}
	DWORD CBar3D::Type()
	{
		static DWORD s_dwType = GetCRC32("CBar3D", strlen("CBar3D"));
		return (s_dwType);
	}

	CBar3D::CBar3D(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_dwLeftColor = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
		m_dwRightColor = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);
		m_dwCenterColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	}
	CBar3D::~CBar3D()
	{
	}

	void CBar3D::SetColor(DWORD dwLeft, DWORD dwRight, DWORD dwCenter)
	{
		m_dwLeftColor = dwLeft;
		m_dwRightColor = dwRight;
		m_dwCenterColor = dwCenter;
	}

	void CBar3D::OnRender()
	{
		CPythonGraphic & rkpyGraphic = CPythonGraphic::Instance();

		rkpyGraphic.SetDiffuseColor(m_dwCenterColor);
		rkpyGraphic.RenderBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);

		rkpyGraphic.SetDiffuseColor(m_dwLeftColor);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.right, m_rect.top);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.left, m_rect.bottom);

		rkpyGraphic.SetDiffuseColor(m_dwRightColor);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.bottom, m_rect.right, m_rect.bottom);
		rkpyGraphic.RenderLine2d(m_rect.right, m_rect.top, m_rect.right, m_rect.bottom);
	}
	CTextLine::CTextLine(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_TextInstance.SetColor(0.78f, 0.78f, 0.78f);
		m_TextInstance.SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
		m_TextInstance.SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_TOP);
	}
	CTextLine::~CTextLine()
	{
		m_TextInstance.Destroy();
	}

	void CTextLine::SetMax(int iMax)
	{
		m_TextInstance.SetMax(iMax);
	}
	void CTextLine::SetHorizontalAlign(int iType)
	{
		m_TextInstance.SetHorizonalAlign(iType);
	}
	void CTextLine::SetVerticalAlign(int iType)
	{
		m_TextInstance.SetVerticalAlign(iType);
	}
	void CTextLine::SetSecret(BOOL bFlag)
	{
		m_TextInstance.SetSecret(bFlag ? true : false);
	}
	void CTextLine::SetOutline(BOOL bFlag)
	{
		m_TextInstance.SetOutline(bFlag ? true : false);
	}
	void CTextLine::SetFeather(BOOL bFlag)
	{
		m_TextInstance.SetFeather(bFlag ? true : false);
	}
	void CTextLine::SetMultiLine(BOOL bFlag)
	{
		m_TextInstance.SetMultiLine(bFlag ? true : false);
	}
	void CTextLine::SetFontName(const char * c_szFontName)
	{
		std::string stFontName = c_szFontName;
		stFontName += ".fnt";

		CResourceManager& rkResMgr=CResourceManager::Instance();
		CResource* pkRes = rkResMgr.GetTypeResourcePointer(stFontName.c_str());
		CGraphicText* pkResFont=static_cast<CGraphicText*>(pkRes);
		m_TextInstance.SetTextPointer(pkResFont);
	}
	void CTextLine::SetFontColor(DWORD dwColor)
	{
		m_TextInstance.SetColor(dwColor);
	}
	void CTextLine::SetLimitWidth(float fWidth)
	{
		m_TextInstance.SetLimitWidth(fWidth);
	}
	void CTextLine::SetText(const char * c_szText)
	{
		OnSetText(c_szText);
	}
	void CTextLine::GetTextSize(int* pnWidth, int* pnHeight)
	{
		m_TextInstance.GetTextSize(pnWidth, pnHeight);
	}
	const char * CTextLine::GetText()
	{
		return m_TextInstance.GetValueStringReference().c_str();
	}
	void CTextLine::ShowCursor()
	{
		m_TextInstance.ShowCursor();
	}
	void CTextLine::HideCursor()
	{
		m_TextInstance.HideCursor();
	}
	int CTextLine::GetCursorPosition()
	{
		long lx, ly;
		CWindow::GetMouseLocalPosition(lx, ly);
		return m_TextInstance.PixelPositionToCharacterPosition(lx);
	}

	void CTextLine::OnSetText(const char * c_szText)
	{
		m_TextInstance.SetValue(c_szText);
		m_TextInstance.Update();
	}

	void CTextLine::OnUpdate()
	{
		if (IsShow())
			m_TextInstance.Update();
	}
	void CTextLine::OnRender()
	{
		if (IsShow())
			m_TextInstance.Render();
	}

	void CTextLine::OnChangePosition()
	{
		if( GetDefaultCodePage() == CP_ARABIC )
		{
			m_TextInstance.SetPosition(m_rect.right, m_rect.top);
		}
		else
		{
			m_TextInstance.SetPosition(m_rect.left, m_rect.top);
		}
	}

	WORD CTextLine::GetTextLineCount()
	{
		return m_TextInstance.GetTextLineCount();
	}
	WORD CTextLine::GetLineHeight()
	{
		return m_TextInstance.GetLineHeight();
	}
	CNumberLine::CNumberLine(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_strPath = "d:/ymir work/ui/game/taskbar/";
		m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_dwWidthSummary = 0;
	}
	CNumberLine::CNumberLine(CWindow * pParent) : CWindow(NULL)
	{
		m_strPath = "d:/ymir work/ui/game/taskbar/";
		m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_dwWidthSummary = 0;

		m_pParent = pParent;
	}
	CNumberLine::~CNumberLine()
	{
		ClearNumber();
	}

	void CNumberLine::SetPath(const char * c_szPath)
	{
		m_strPath = c_szPath;
	}
	void CNumberLine::SetHorizontalAlign(int iType)
	{
		m_iHorizontalAlign = iType;
	}
	void CNumberLine::SetNumber(const char * c_szNumber)
	{
		if (0 == m_strNumber.compare(c_szNumber))
			return;

		ClearNumber();

		m_strNumber = c_szNumber;

		for (DWORD i = 0; i < m_strNumber.size(); ++i)
		{
			char cChar = m_strNumber[i];
			std::string strImageFileName;

			if (':' == cChar)
			{
				strImageFileName = m_strPath + "colon.sub";
			}
			else if ('?' == cChar)
			{
				strImageFileName = m_strPath + "questionmark.sub";
			}
			else if ('/' == cChar)
			{
				strImageFileName = m_strPath + "slash.sub";
			}
			else if ('%' == cChar)
			{
				strImageFileName = m_strPath + "percent.sub";
			}
			else if ('+' == cChar)
			{
				strImageFileName = m_strPath + "plus.sub";
			}
			else if ('m' == cChar)
			{
				strImageFileName = m_strPath + "m.sub";
			}
			else if ('g' == cChar)
			{
				strImageFileName = m_strPath + "g.sub";
			}
			else if ('p' == cChar)
			{
				strImageFileName = m_strPath + "p.sub";
			}
			else if (cChar >= '0' && cChar <= '9')
			{
				strImageFileName = m_strPath;
				strImageFileName += cChar;
				strImageFileName += ".sub";
			}
			else
				continue;

#ifdef ENABLE_FOX_FS
			if (!CResourceManager::Instance().IsFileExist(strImageFileName.c_str(), __FUNCTION__))
#else
			if (!CResourceManager::Instance().IsFileExist(strImageFileName.c_str()))
#endif
				continue;

			CGraphicImage * pImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(strImageFileName.c_str());

			CGraphicImageInstance * pInstance = CGraphicImageInstance::New();
			pInstance->SetImagePointer(pImage);
			m_ImageInstanceVector.push_back(pInstance);

			m_dwWidthSummary += pInstance->GetWidth();
		}
	}

	void CNumberLine::ClearNumber()
	{
		m_ImageInstanceVector.clear();
		m_dwWidthSummary = 0;
		m_strNumber = "";
	}

	void CNumberLine::OnRender()
	{
		for (DWORD i = 0; i < m_ImageInstanceVector.size(); ++i)
		{
			m_ImageInstanceVector[i]->Render();
		}
	}

	void CNumberLine::OnChangePosition()
	{
		int ix = m_x;
		int iy = m_y;

		if (m_pParent)
		{
			ix = m_rect.left;
			iy = m_rect.top;
		}

		switch (m_iHorizontalAlign)
		{
			case HORIZONTAL_ALIGN_LEFT:
				break;
			case HORIZONTAL_ALIGN_CENTER:
				ix -= int(m_dwWidthSummary) / 2;
				break;
			case HORIZONTAL_ALIGN_RIGHT:
				ix -= int(m_dwWidthSummary);
				break;
		}

		for (DWORD i = 0; i < m_ImageInstanceVector.size(); ++i)
		{
			m_ImageInstanceVector[i]->SetPosition(ix, iy);
			ix += m_ImageInstanceVector[i]->GetWidth();
		}
	}
	CImageBox::CImageBox(PyObject * ppyObject) : CWindow(ppyObject), m_fCoolTime(0.0f), m_fCoolTimeStart(0.0f)
	{
		m_pImageInstance = NULL;
	}

	CImageBox::~CImageBox()
	{
		OnDestroyInstance();
	}

	void CImageBox::OnCreateInstance()
	{
		OnDestroyInstance();

		m_pImageInstance = CGraphicImageInstance::New();
	}
	void CImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicImageInstance::Delete(m_pImageInstance);
			m_pImageInstance=NULL;
		}
	}

	BOOL CImageBox::LoadImage(const char * c_szFileName)
	{
		if (!c_szFileName[0])
			return FALSE;

		OnCreateInstance();

		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource)
			return FALSE;
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_pImageInstance->IsEmpty())
			return FALSE;

		SetSize(m_pImageInstance->GetWidth(), m_pImageInstance->GetHeight());
		UpdateRect();

		return TRUE;
	}

	void CImageBox::SetScale(float sx, float sy)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicImageInstance*)m_pImageInstance)->SetScale(sx, sy);
		CWindow::SetSize(long(float(GetWidth())*sx), long(float(GetHeight())*sy));
	}

	void CImageBox::SetDiffuseColor(float fr, float fg, float fb, float fa)
	{
		if (!m_pImageInstance)
			return;

		m_pImageInstance->SetDiffuseColor(fr, fg, fb, fa);
	}

	int CImageBox::GetWidth()
	{
		if (!m_pImageInstance)
			return 0;

		return m_pImageInstance->GetWidth();
	}

	int CImageBox::GetHeight()
	{
		if (!m_pImageInstance)
			return 0;

		return m_pImageInstance->GetHeight();
	}

	void CImageBox::OnUpdate()
	{
	}
	void CImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow())
		{
			if (m_fCoolTime == 0.0f)
				m_pImageInstance->Render();
			else
				m_pImageInstance->RenderCoolTime((CTimer::Instance().GetCurrentSecond() - m_fCoolTimeStart) / m_fCoolTime);
		}
	}
	void CImageBox::OnChangePosition()
	{
		if (!m_pImageInstance)
			return;

		m_pImageInstance->SetPosition(m_rect.left, m_rect.top);
	}

	void CImageBox::SetCoolTime(float fCoolTime)
	{
		m_fCoolTime = fCoolTime;
	}

	void CImageBox::SetCoolTimeStart(float fCoolTimeStart)
	{
		m_fCoolTimeStart = fCoolTimeStart;
	}

	void CImageBox::LeftRightReverse()
	{
	}
	CMarkBox::CMarkBox(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_pMarkInstance = NULL;
	}

	CMarkBox::~CMarkBox()
	{
		OnDestroyInstance();
	}

	void CMarkBox::OnCreateInstance()
	{
		OnDestroyInstance();
		m_pMarkInstance = CGraphicMarkInstance::New();
	}

	void CMarkBox::OnDestroyInstance()
	{
		if (m_pMarkInstance)
		{
			CGraphicMarkInstance::Delete(m_pMarkInstance);
			m_pMarkInstance=NULL;
		}
	}

	void CMarkBox::LoadImage(const char * c_szFilename)
	{
		OnCreateInstance();

		m_pMarkInstance->SetImageFileName(c_szFilename);
		m_pMarkInstance->Load();
		SetSize(m_pMarkInstance->GetWidth(), m_pMarkInstance->GetHeight());

		UpdateRect();
	}

	void CMarkBox::SetScale(FLOAT fScale)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetScale(fScale);
	}

	void CMarkBox::SetIndex(UINT uIndex)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetIndex(uIndex);
	}

	void CMarkBox::SetDiffuseColor(float fr, float fg, float fb, float fa)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetDiffuseColor(fr, fg, fb, fa);
	}

	void CMarkBox::OnUpdate()
	{
	}

	void CMarkBox::OnRender()
	{
		if (!m_pMarkInstance)
			return;

		if (IsShow())
			m_pMarkInstance->Render();
	}

	void CMarkBox::OnChangePosition()
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetPosition(m_rect.left, m_rect.top);
	}
	DWORD CExpandedImageBox::Type()
	{
		static DWORD s_dwType = GetCRC32("CExpandedImageBox", strlen("CExpandedImageBox"));
		return (s_dwType);
	}

	BOOL CExpandedImageBox::OnIsType(DWORD dwType)
	{
		if (CExpandedImageBox::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	CExpandedImageBox::CExpandedImageBox(PyObject * ppyObject) : CImageBox(ppyObject)
	{
	}
	CExpandedImageBox::~CExpandedImageBox()
	{
		OnDestroyInstance();
	}

	void CExpandedImageBox::OnCreateInstance()
	{
		OnDestroyInstance();

		m_pImageInstance = CGraphicExpandedImageInstance::New();
	}
	void CExpandedImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicExpandedImageInstance::Delete((CGraphicExpandedImageInstance*)m_pImageInstance);
			m_pImageInstance=NULL;
		}
	}

	void CExpandedImageBox::SetScale(float fx, float fy)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetScale(fx, fy);
		CWindow::SetSize(long(float(GetWidth())*fx), long(float(GetHeight())*fy));
	}
	void CExpandedImageBox::SetOrigin(float fx, float fy)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetOrigin(fx, fy);
	}
	void CExpandedImageBox::SetRotation(float fRotation)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRotation(fRotation);
	}
	void CExpandedImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingRect(fLeft, fTop, fRight, fBottom);
	}

	void CExpandedImageBox::SetRenderingMode(int iMode)
	{
		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingMode(iMode);
	}

	void CExpandedImageBox::OnUpdate()
	{
	}
	void CExpandedImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow())
		{
			if (m_fCoolTime == 0.0f)
				m_pImageInstance->Render();
			else
				((CGraphicExpandedImageInstance*)m_pImageInstance)->RenderCoolTime((CTimer::Instance().GetCurrentSecond() - m_fCoolTimeStart) / m_fCoolTime);
		}
	}
	DWORD CAniImageBox::Type()
	{
		static DWORD s_dwType = GetCRC32("CAniImageBox", strlen("CAniImageBox"));
		return (s_dwType);
	}

	BOOL CAniImageBox::OnIsType(DWORD dwType)
	{
		if (CAniImageBox::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	CAniImageBox::CAniImageBox(PyObject * ppyObject)
		:	CWindow(ppyObject),
			m_bycurDelay(0),
			m_byDelay(4),
			m_bycurIndex(0)
	{
		m_ImageVector.clear();
	}
	CAniImageBox::~CAniImageBox()
	{
		for_each(m_ImageVector.begin(), m_ImageVector.end(), CGraphicExpandedImageInstance::DeleteExpandedImageInstance);
	}

	void CAniImageBox::SetDelay(int iDelay)
	{
		m_byDelay = iDelay;
	}
	void CAniImageBox::AppendImage(const char * c_szFileName, float r, float g, float b, float a)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return;
		
		CGraphicExpandedImageInstance * pImageInstance = CGraphicExpandedImageInstance::New();
		pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));
		pImageInstance->SetDiffuseColor(r, g, b, a);
		
		if (pImageInstance->IsEmpty())
		{
			CGraphicExpandedImageInstance::Delete(pImageInstance);
			return;
		}
		
		m_ImageVector.push_back(pImageInstance);
		m_bycurIndex = (BYTE)(rand() % m_ImageVector.size());
	}

	void CAniImageBox::AppendImageScale(const char * c_szFileName, float scale_x, float scale_y)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return;

		CGraphicExpandedImageInstance * pImageInstance = CGraphicExpandedImageInstance::New();

		pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));
		pImageInstance->SetScale(scale_x, scale_y);
		if (pImageInstance->IsEmpty())
		{
			CGraphicExpandedImageInstance::Delete(pImageInstance);
			return;
		}

		m_ImageVector.push_back(pImageInstance);

		m_bycurIndex = (BYTE)(rand() % m_ImageVector.size());
	}

	struct FSetRenderingRect
	{
		float fLeft, fTop, fRight, fBottom;
		void operator () (CGraphicExpandedImageInstance * pInstance)
		{
			pInstance->SetRenderingRect(fLeft, fTop, fRight, fBottom);
		}
	};
	void CAniImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		FSetRenderingRect setRenderingRect;
		setRenderingRect.fLeft = fLeft;
		setRenderingRect.fTop = fTop;
		setRenderingRect.fRight = fRight;
		setRenderingRect.fBottom = fBottom;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingRect);
	}

	struct FSetRenderingMode
	{
		int iMode;
		void operator () (CGraphicExpandedImageInstance * pInstance)
		{
			pInstance->SetRenderingMode(iMode);
		}
	};
	void CAniImageBox::SetRenderingMode(int iMode)
	{
		FSetRenderingMode setRenderingMode;
		setRenderingMode.iMode = iMode;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingMode);
	}

	void CAniImageBox::ResetFrame()
	{
		m_bycurIndex = 0;
	}

	void CAniImageBox::OnUpdate()
	{
		++m_bycurDelay;
		if (m_bycurDelay < m_byDelay)
			return;

		m_bycurDelay = 0;

		++m_bycurIndex;
		PyCallClassMemberFunc(m_poHandler, "OnKeyFrame", Py_BuildValue("(i)", m_bycurIndex));
		if (m_bycurIndex >= m_ImageVector.size())
		{
			m_bycurIndex = 0;

			OnEndFrame();
		}
	}
	void CAniImageBox::OnRender()
	{
		if (m_bycurIndex < m_ImageVector.size())
		{
			CGraphicExpandedImageInstance * pImage = m_ImageVector[m_bycurIndex];
			pImage->Render();
		}
	}

	struct FChangePosition
	{
		float fx, fy;
		void operator () (CGraphicExpandedImageInstance * pInstance)
		{
			pInstance->SetPosition(fx, fy);
		}
	};

	void CAniImageBox::OnChangePosition()
	{
		FChangePosition changePosition;
		changePosition.fx = m_rect.left;
		changePosition.fy = m_rect.top;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), changePosition);
	}

	void CAniImageBox::OnEndFrame()
	{
		PyCallClassMemberFunc(m_poHandler, "OnEndFrame", BuildEmptyTuple());
	}

	struct FSetScale
	{
		float fx, fy;
		void operator () (CGraphicExpandedImageInstance * pInstance)
		{
			pInstance->SetScale(fx, fy);
		}
	};

	void CAniImageBox::SetScale(float fx, float fy)
	{
		FSetScale setScale;
		setScale.fx = fx;
		setScale.fy = fy;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), setScale);

	}
	CMoveTextLine::CMoveTextLine(PyObject * ppyObject) :
		CTextLine(ppyObject),
		m_v2SrcPos(0.0f, 0.0f),
		m_v2DstPos(0.0f, 0.0f),
		m_v2NextPos(0.0f, 0.0f),
		m_v2Direction(0.0f, 0.0f),
		m_v2NextDistance(0.0f, 0.0f),
		m_fDistance(0.0f),
		m_fMoveSpeed(10.0f),
		m_bIsMove(false)
	{
	}

	CMoveTextLine::~CMoveTextLine()
	{
		m_TextInstance.Destroy();
	}

	DWORD CMoveTextLine::Type()
	{
		static DWORD s_dwType = GetCRC32("CMoveTextLine", strlen("CMoveTextLine"));
		return (s_dwType);
	}

	BOOL CMoveTextLine::OnIsType(DWORD dwType)
	{
		if (CMoveTextLine::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	void CMoveTextLine::SetMoveSpeed(float fSpeed)
	{
		m_fMoveSpeed = fSpeed;
	}

	bool CMoveTextLine::GetMove()
	{
		return m_bIsMove;
	}

	void CMoveTextLine::MoveStart()
	{
		m_bIsMove = true;
		m_v2NextPos = m_v2SrcPos;
	}

	void CMoveTextLine::MoveStop()
	{
		m_bIsMove = false;
	}

	void CMoveTextLine::OnEndMove()
	{
		PyCallClassMemberFunc(m_poHandler, "OnEndMove", BuildEmptyTuple());
	}

	void CMoveTextLine::OnChangePosition()
	{
		m_TextInstance.SetPosition((GetDefaultCodePage() == CP_1256) ? m_rect.right : m_rect.left, m_rect.top);
	}

	void CMoveTextLine::SetMovePosition(float fDstX, float fDstY)
	{
		if (fDstX != m_v2DstPos.x || fDstY != m_v2DstPos.y || m_rect.left != m_v2SrcPos.x || m_rect.top != m_v2SrcPos.y)
		{
			m_v2SrcPos.x = m_rect.left;
			m_v2SrcPos.y = m_rect.top;

			m_v2DstPos.x = fDstX;
			m_v2DstPos.y = fDstY;

			D3DXVec2Subtract(&m_v2Direction, &m_v2DstPos, &m_v2SrcPos);

			m_fDistance = (m_v2Direction.y*m_v2Direction.y + m_v2Direction.x*m_v2Direction.x);
			D3DXVec2Normalize(&m_v2Direction, &m_v2Direction);

			if (m_v2SrcPos != m_v2NextPos)
			{
				float fDist = sqrtf(m_v2NextDistance.x*m_v2NextDistance.x + m_v2NextDistance.y*m_v2NextDistance.y);
				m_v2NextPos = m_v2Direction * fDist;
				m_TextInstance.SetPosition(m_v2NextPos.x, m_v2NextPos.y);
			}
		}
	}

	void CMoveTextLine::OnUpdate()
	{
		if (IsShow() && GetMove())
		{
			D3DXVec2Add(&m_v2NextPos, &m_v2NextPos, &(m_v2Direction * m_fMoveSpeed));
			D3DXVec2Subtract(&m_v2NextDistance, &m_v2NextPos, &m_v2SrcPos);

			float fNextDistance = m_v2NextDistance.y * m_v2NextDistance.y + m_v2NextDistance.x * m_v2NextDistance.x;
			if (fNextDistance >= m_fDistance)
			{
				m_v2NextPos = m_v2DstPos;
				MoveStop();
				OnEndMove();
			}

			m_TextInstance.SetPosition(m_v2NextPos.x, m_v2NextPos.y);
			m_TextInstance.Update();
		}
	}

	void CMoveTextLine::OnRender()
	{
		if (IsShow())
			m_TextInstance.Render();
	}
	CMoveImageBox::CMoveImageBox(PyObject * ppyObject) :
		CImageBox(ppyObject),
		m_v2SrcPos(0.0f, 0.0f),
		m_v2DstPos(0.0f, 0.0f),
		m_v2NextPos(0.0f, 0.0f),
		m_v2Direction(0.0f, 0.0f),
		m_v2NextDistance(0.0f, 0.0f),
		m_fDistance(0.0f),
		m_fMoveSpeed(10.0f),
		m_bIsMove(false)
	{
	}

	CMoveImageBox::~CMoveImageBox()
	{
		OnDestroyInstance();
	}

	void CMoveImageBox::OnCreateInstance()
	{
		OnDestroyInstance();

		m_pImageInstance = CGraphicImageInstance::New();
	}

	void CMoveImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicImageInstance::Delete(m_pImageInstance);
			m_pImageInstance = NULL;
		}
	}

	DWORD CMoveImageBox::Type()
	{
		static DWORD s_dwType = GetCRC32("CMoveImageBox", strlen("CMoveImageBox"));
		return (s_dwType);
	}

	BOOL CMoveImageBox::OnIsType(DWORD dwType)
	{
		if (CMoveImageBox::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	void CMoveImageBox::OnEndMove()
	{
		PyCallClassMemberFunc(m_poHandler, "OnEndMove", BuildEmptyTuple());
	}

	void CMoveImageBox::SetMovePosition(float fDstX, float fDstY)
	{
		if (fDstX != m_v2DstPos.x || fDstY != m_v2DstPos.y || m_rect.left != m_v2SrcPos.x || m_rect.top != m_v2SrcPos.y)
		{
			m_v2SrcPos.x = m_rect.left;
			m_v2SrcPos.y = m_rect.top;

			m_v2DstPos.x = fDstX;
			m_v2DstPos.y = fDstY;

			D3DXVec2Subtract(&m_v2Direction, &m_v2DstPos, &m_v2SrcPos);

			m_fDistance = (m_v2Direction.x*m_v2Direction.x + m_v2Direction.y*m_v2Direction.y);

			D3DXVec2Normalize(&m_v2Direction, &m_v2Direction);

			if (m_pImageInstance && m_v2SrcPos != m_v2NextPos)
			{
				float fDist = sqrtf(m_v2NextDistance.x*m_v2NextDistance.x + m_v2NextDistance.y*m_v2NextDistance.y);

				m_v2NextPos = m_v2Direction * fDist;
				m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
			}
		}
	}

	void CMoveImageBox::SetMoveSpeed(float fSpeed)
	{
		m_fMoveSpeed = fSpeed;
	}

	void CMoveImageBox::MoveStart()
	{
		m_bIsMove = true;
		m_v2NextPos = m_v2SrcPos;
	}

	void CMoveImageBox::MoveStop()
	{
		m_bIsMove = false;
	}

	bool CMoveImageBox::GetMove()
	{
		return m_bIsMove;
	}

	void CMoveImageBox::OnUpdate()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow() && GetMove())
		{
			D3DXVec2Add(&m_v2NextPos, &m_v2NextPos, &(m_v2Direction * m_fMoveSpeed));
			D3DXVec2Subtract(&m_v2NextDistance, &m_v2NextPos, &m_v2SrcPos);

			float fNextDistance = (m_v2NextDistance.x*m_v2NextDistance.x + m_v2NextDistance.y*m_v2NextDistance.y);
			if (fNextDistance >= m_fDistance)
			{
				m_v2NextPos = m_v2DstPos;
				MoveStop();
				OnEndMove();
			}

			m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
		}
	}

	void CMoveImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow())
			m_pImageInstance->Render();
	}
	CMoveScaleImageBox::CMoveScaleImageBox(PyObject * ppyObject) :
		CMoveImageBox(ppyObject),
		m_fMaxScale(1.0f),
		m_fMaxScaleRate(1.0f),
		m_fScaleDistance(0.0f),
		m_fAdditionalScale(0.0f),
		m_v2CurScale(1.0f, 1.0f)
	{
	}

	CMoveScaleImageBox::~CMoveScaleImageBox()
	{
		OnDestroyInstance();
	}

	void CMoveScaleImageBox::OnCreateInstance()
	{
		OnDestroyInstance();

		m_pImageInstance = CGraphicImageInstance::New();
	}

	void CMoveScaleImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicImageInstance::Delete(m_pImageInstance);
			m_pImageInstance = NULL;
		}
	}

	DWORD CMoveScaleImageBox::Type()
	{
		static DWORD s_dwType = GetCRC32("CMoveScaleImageBox", strlen("CMoveScaleImageBox"));
		return (s_dwType);
	}

	BOOL CMoveScaleImageBox::OnIsType(DWORD dwType)
	{
		if (CMoveScaleImageBox::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	void CMoveScaleImageBox::SetMaxScale(float fMaxScale)
	{
		m_fMaxScale = fMaxScale;
	}

	void CMoveScaleImageBox::SetMaxScaleRate(float fMaxScaleRate)
	{
		m_fMaxScaleRate = fMaxScaleRate;
		float fDistanceRate = m_fDistance * fMaxScaleRate;
		m_fScaleDistance = fDistanceRate;
		m_v2CurScale = m_pImageInstance->GetScale();
		float fDiffScale = m_fMaxScale - m_v2CurScale.x;
		m_fAdditionalScale = fDiffScale / (sqrtf(fDistanceRate) / m_fMoveSpeed);
	}

	void CMoveScaleImageBox::SetScalePivotCenter(bool bScalePivotCenter)
	{
		if (m_pImageInstance)
			m_pImageInstance->SetScalePivotCenter(bScalePivotCenter);
	}

	void CMoveScaleImageBox::OnUpdate()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow() && GetMove())
		{
			D3DXVec2Add(&m_v2NextPos, &m_v2NextPos, &(m_v2Direction * m_fMoveSpeed));
			D3DXVec2Subtract(&m_v2NextDistance, &m_v2NextPos, &m_v2SrcPos);

			float fNextDistance = (m_v2NextDistance.x*m_v2NextDistance.x + m_v2NextDistance.y*m_v2NextDistance.y);
			if (m_fScaleDistance < fNextDistance)
				m_fAdditionalScale *= -1.0f;
			
			D3DXVECTOR2 v2NewScale;
			D3DXVec2Add(&v2NewScale, &m_pImageInstance->GetScale(), &D3DXVECTOR2(m_fAdditionalScale, m_fAdditionalScale));
			if (m_fMaxScale < v2NewScale.x)
				v2NewScale = D3DXVECTOR2(m_fMaxScale, m_fMaxScale);

			if (m_v2CurScale.x > v2NewScale.x)
				v2NewScale = m_v2CurScale;

			m_pImageInstance->SetScale(v2NewScale);

			if (fNextDistance >= m_fDistance)
			{
				m_pImageInstance->SetScale(m_v2CurScale);
				m_v2NextPos = m_v2DstPos;
				MoveStop();
				OnEndMove();
			}

			m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
		}
	}

	CButton::CButton(PyObject * ppyObject)
		:	CWindow(ppyObject),
			m_pcurVisual(NULL),
			m_bEnable(TRUE),
			m_isPressed(FALSE),
			m_isFlash(FALSE),
			m_isFlashEnable(FALSE)
	{
		CWindow::AddFlag(CWindow::FLAG_NOT_CAPTURE);
	}
	CButton::~CButton()
	{
	}

	BOOL CButton::SetUpVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_upVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_upVisual.IsEmpty())
			return FALSE;

		SetSize(m_upVisual.GetWidth(), m_upVisual.GetHeight());

		SetCurrentVisual(&m_upVisual);
		return TRUE;
	}
	BOOL CButton::SetOverVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_overVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_overVisual.IsEmpty())
			return FALSE;

		SetSize(m_overVisual.GetWidth(), m_overVisual.GetHeight());

		return TRUE;
	}
	BOOL CButton::SetDownVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_downVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_downVisual.IsEmpty())
			return FALSE;

		SetSize(m_downVisual.GetWidth(), m_downVisual.GetHeight());

		return TRUE;
	}
	BOOL CButton::SetDisableVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_disableVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_downVisual.IsEmpty())
			return FALSE;

		SetSize(m_disableVisual.GetWidth(), m_disableVisual.GetHeight());

		return TRUE;
	}

	const char * CButton::GetUpVisualFileName()
	{
		return m_upVisual.GetGraphicImagePointer()->GetFileName();
	}
	const char * CButton::GetOverVisualFileName()
	{
		return m_overVisual.GetGraphicImagePointer()->GetFileName();
	}
	const char * CButton::GetDownVisualFileName()
	{
		return m_downVisual.GetGraphicImagePointer()->GetFileName();
	}

	void CButton::Flash()
	{
		m_isFlash = TRUE;
	}

	void CButton::EnableFlash()
	{
		m_isFlashEnable = TRUE;
	}

	void CButton::DisableFlash()
	{
		m_isFlashEnable = FALSE;
	}

	void CButton::Enable()
	{
		SetUp();
		m_bEnable = TRUE;
	}

	void CButton::Disable()
	{
		m_bEnable = FALSE;
		if (!m_disableVisual.IsEmpty())
			SetCurrentVisual(&m_disableVisual);
	}

	BOOL CButton::IsDisable()
	{
		return m_bEnable;
	}

	void CButton::SetUp()
	{
		SetCurrentVisual(&m_upVisual);
		m_isPressed = FALSE;
	}
	void CButton::Up()
	{
		if (IsIn())
			SetCurrentVisual(&m_overVisual);
		else
			SetCurrentVisual(&m_upVisual);

		PyCallClassMemberFunc(m_poHandler, "CallEvent", BuildEmptyTuple());
	}
	void CButton::Over()
	{
		SetCurrentVisual(&m_overVisual);
	}
	void CButton::Down()
	{
		m_isPressed = TRUE;
		SetCurrentVisual(&m_downVisual);
		PyCallClassMemberFunc(m_poHandler, "DownEvent", BuildEmptyTuple());
	}

	void CButton::OnUpdate()
	{
	}
	void CButton::OnRender()
	{
		if (!IsShow())
			return;

		if (m_pcurVisual)
		{
			if (m_isFlashEnable && !IsIn())
			{
				if (int(timeGetTime() / 500) % 2)
					SetCurrentVisual(&m_upVisual);
				else
					SetCurrentVisual(&m_overVisual);
			}

			if (m_isFlash)
			if (!IsIn())
			if (int(timeGetTime() / 500) % 2)
				return;

			m_pcurVisual->Render();
		}

		PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
	}
	void CButton::OnChangePosition()
	{
		if (m_pcurVisual)
			m_pcurVisual->SetPosition(m_rect.left, m_rect.top);
	}

	BOOL CButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		m_isPressed = TRUE;
		Down();

		return TRUE;
	}
	BOOL CButton::OnMouseLeftButtonDoubleClick()
	{
		if (!IsEnable())
			return TRUE;

		OnMouseLeftButtonDown();

		return TRUE;
	}
	BOOL CButton::OnMouseLeftButtonUp()
	{
		if (!IsEnable())
			return TRUE;
		if (!IsPressed())
			return TRUE;

		m_isPressed = FALSE;
		Up();

		return TRUE;
	}
	void CButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		Over();
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
	}
	void CButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		SetUp();
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
	}

	void CButton::SetCurrentVisual(CGraphicImageInstance * pVisual)
	{
		m_pcurVisual = pVisual;
		m_pcurVisual->SetPosition(m_rect.left, m_rect.top);
	}

	BOOL CButton::IsEnable()
	{
		return m_bEnable;
	}

	BOOL CButton::IsPressed()
	{
		return m_isPressed;
	}

	void CButton::LeftRightReverse()
	{
	}
	CRadioButton::CRadioButton(PyObject * ppyObject) : CButton(ppyObject)
	{
	}
	CRadioButton::~CRadioButton()
	{
	}

	BOOL CRadioButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		if (!m_isPressed)
		{
			Down();
			PyCallClassMemberFunc(m_poHandler, "CallEvent", BuildEmptyTuple());
		}

		return TRUE;
	}
	BOOL CRadioButton::OnMouseLeftButtonUp()
	{
		return TRUE;
	}
	void CRadioButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_overVisual);
		}
	}
	void CRadioButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_upVisual);
		}

	}
	CToggleButton::CToggleButton(PyObject * ppyObject) : CButton(ppyObject)
	{
	}
	CToggleButton::~CToggleButton()
	{
	}

	BOOL CToggleButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		if (m_isPressed)
		{
			SetUp();
			if (IsIn())
				SetCurrentVisual(&m_overVisual);
			else
				SetCurrentVisual(&m_upVisual);
			PyCallClassMemberFunc(m_poHandler, "OnToggleUp", BuildEmptyTuple());
		}
		else
		{
			Down();
			PyCallClassMemberFunc(m_poHandler, "OnToggleDown", BuildEmptyTuple());
		}

		return TRUE;
	}
	BOOL CToggleButton::OnMouseLeftButtonUp()
	{
		return TRUE;
	}

	void CToggleButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_overVisual);
		}
	}
	void CToggleButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_upVisual);
		}
	}
	CDragButton::CDragButton(PyObject * ppyObject) : CButton(ppyObject)
	{
		CWindow::RemoveFlag(CWindow::FLAG_NOT_CAPTURE);
		m_restrictArea.left = 0;
		m_restrictArea.top = 0;
		m_restrictArea.right = CWindowManager::Instance().GetScreenWidth();
		m_restrictArea.bottom = CWindowManager::Instance().GetScreenHeight();
	}
	CDragButton::~CDragButton()
	{
	}

	void CDragButton::SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight)
	{
		m_restrictArea.left = ix;
		m_restrictArea.top = iy;
		m_restrictArea.right = ix + iwidth;
		m_restrictArea.bottom = iy + iheight;
	}

	void CDragButton::OnChangePosition()
	{
		m_x = std::max<int32_t>(m_x, m_restrictArea.left);
		m_y = std::max<int32_t>(m_y, m_restrictArea.top);
		m_x = std::min<int32_t>(m_x, std::max(0l, m_restrictArea.right - m_lWidth));
		m_y = std::min<int32_t>(m_y, std::max(0l, m_restrictArea.bottom - m_lHeight));

		m_rect.left = m_x;
		m_rect.top = m_y;

		if (m_pParent)
		{
			const RECT & c_rRect = m_pParent->GetRect();
			m_rect.left += c_rRect.left;
			m_rect.top += c_rRect.top;
		}

		m_rect.right = m_rect.left + m_lWidth;
		m_rect.bottom = m_rect.top + m_lHeight;

		std::for_each(m_pChildList.begin(), m_pChildList.end(), std::mem_fn(&CWindow::UpdateRect));

		if (m_pcurVisual)
			m_pcurVisual->SetPosition(m_rect.left, m_rect.top);

		if (IsPressed())
			PyCallClassMemberFunc(m_poHandler, "OnMove", BuildEmptyTuple());
	}

	void CDragButton::OnMouseOverIn()
	{
		if (!IsEnable())

			return;

		CButton::OnMouseOverIn();
	}

	void CDragButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		CButton::OnMouseOverIn();
	}
	
#ifdef ENABLE_RENDER_TARGET
	CRenderTarget::CRenderTarget(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_dwNumber = -1;

		m_BackGroundImageInstance = CGraphicImageInstance::New();
		m_BackGroundImageInstance->SetImagePointer((CGraphicImage *)CResourceManager::Instance().GetResourcePointer("d:/ymir work/ui/game/monster_card/model_view_bg.sub"));

		m_InstanceModel = NULL;

		m_Zoom = m_xRotation = m_yRotation = 0.0f;
	}
	
	CRenderTarget::~CRenderTarget()
	{
		m_dwNumber = -1;
	}

	DWORD CRenderTarget::Type()
	{
		static DWORD s_dwType = GetCRC32("CRenderTarget", strlen("CRenderTarget"));
		return (s_dwType);
	}

	BOOL CRenderTarget::OnIsType(DWORD dwType)
	{
		if (CRenderTarget::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	void CRenderTarget::SetRenderTarget(DWORD dwNumber)
	{
		m_dwNumber = dwNumber;

		m_Rect = m_rect;

		bool success = SUCCEEDED(ms_lpd3dDevice->CreateTexture(GetWidth(), GetHeight(), 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_RenderTexture, NULL));
		if (!success)
		{
			TraceError("CRenderTarget: Creating Texture For CreateTexture failed!");
			ReleaseResources();
			return;
		}
		success = SUCCEEDED(m_RenderTexture->GetSurfaceLevel(0, &m_RenderTargetSurface));
		if (!success)
		{
			TraceError("CRenderTarget: GetSurfaceLevel failed!");
			ReleaseResources();
			return;
		}
		success = SUCCEEDED(ms_lpd3dDevice->CreateDepthStencilSurface(GetWidth(), GetHeight(), D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &m_DepthSurface, NULL));
		if (!success)
		{
			TraceError("CRenderTarget: CreateDepthStencilSurface failed!");
			ReleaseResources();
			return;
		}

		SetRenderTargetData(0, 0, 0, 0, 0, CRaceMotionData::MODE_GENERAL);
		CWindow::Show();
	}

	void CRenderTarget::ReleaseResources()
	{
		safe_release(m_RenderTexture);
		safe_release(m_RenderTargetSurface);
		safe_release(m_DepthSurface);
		safe_release(m_PreviousRenderTarget);
		safe_release(m_PreviousDepthBuffer);
	}

	void CRenderTarget::SetRenderTargetData(int iRace, int iArmor, int iWeapon, int iHair, int iAcce, int iMotionMode)
	{
		m_iRace = iRace;
		m_iArmor = iArmor;
		m_iWeapon = iWeapon;
		m_iHair = iHair;
		m_iAcce = iAcce;
		m_iMotionMode = iMotionMode;

		if (m_InstanceModel)
		{
			m_InstanceModel->SetRace(iRace);
			m_InstanceModel->SetArmor(iArmor);
			m_InstanceModel->SetWeapon(iWeapon);
			m_InstanceModel->SetHair(iHair);
			m_InstanceModel->SetAcce(iAcce);
			m_InstanceModel->SetMotionMode(iMotionMode);
			m_InstanceModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);

		}
		else
		{
			m_InstanceModel = CInstanceBase::New();
			m_InstanceModel->DisableTextTail();
			CInstanceBase::SCreateData data;
			std::memset(&data, 0, sizeof(CInstanceBase::SCreateData));
			data.m_dwRace = iRace;
			data.m_dwArmor = iArmor;
			data.m_dwWeapon = iWeapon;
			data.m_dwHair = iHair;
			data.m_dwAcce = iAcce;
			if (iRace >= 0 && iRace <= 8)
				data.m_bType = CActorInstance::TYPE_PC;
			else
				data.m_bType = CActorInstance::TYPE_NPC;

			m_InstanceModel->Create(data);

			m_InstanceModel->Refresh(CRaceMotionData::NAME_WAIT, true);
			m_InstanceModel->SetMotionMode(iMotionMode);
			m_InstanceModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);

			m_InstanceModel->Show();
			m_InstanceModel->EnableAlwaysRender();
			m_InstanceModel->NEW_SetPixelPosition(TPixelPosition(0, 0, 0));
			RestoreStartPosition();
		}
	}

	void CRenderTarget::ShowInstance()
	{
		if (!m_InstanceModel)
			SetRenderTargetData(m_iRace, m_iArmor, m_iWeapon, m_iHair, m_iAcce, m_iMotionMode);

		m_InstanceModel->Show();
		m_InstanceModel->EnableAlwaysRender();
	}

	void CRenderTarget::HideInstance()
	{
		if (!m_InstanceModel)
			SetRenderTargetData(m_iRace, m_iArmor, m_iWeapon, m_iHair, m_iAcce, m_iMotionMode);

		m_InstanceModel->Hide();
		m_InstanceModel->DisableAlwaysRender();
	}

	void CRenderTarget::RotateLeft(float fValue)
	{
		if (!m_InstanceModel)
			SetRenderTargetData(m_iRace, m_iArmor, m_iWeapon, m_iHair, m_iAcce, m_iMotionMode);

		m_xRotation += fValue;
		m_InstanceModel->SetRotation(m_xRotation);
		m_InstanceModel->GetGraphicThingInstanceRef().RotationProcess();
	}

	void CRenderTarget::RotateRight(float fValue)
	{
		if (!m_InstanceModel)
			SetRenderTargetData(m_iRace, m_iArmor, m_iWeapon, m_iHair, m_iAcce, m_iMotionMode);

		m_xRotation -= fValue;
		m_InstanceModel->SetRotation(m_xRotation);
		m_InstanceModel->GetGraphicThingInstanceRef().RotationProcess();
	}

	void CRenderTarget::RotateUp(float fValue)
	{
		if (!m_InstanceModel)
			SetRenderTargetData(m_iRace, m_iArmor, m_iWeapon, m_iHair, m_iAcce, m_iMotionMode);

		m_yRotation -= fValue;
		m_InstanceModel->GetGraphicThingInstanceRef().SetXYRotation(m_InstanceModel->GetRotation(), m_yRotation);
		m_InstanceModel->GetGraphicThingInstanceRef().RotationProcess();
	}

	void CRenderTarget::RotateDown(float fValue)
	{
		if (!m_InstanceModel)
			SetRenderTargetData(m_iRace, m_iArmor, m_iWeapon, m_iHair, m_iAcce, m_iMotionMode);

		m_yRotation += fValue;
		m_InstanceModel->GetGraphicThingInstanceRef().SetXYRotation(m_InstanceModel->GetRotation(), m_yRotation);
		m_InstanceModel->GetGraphicThingInstanceRef().RotationProcess();
	}

	void CRenderTarget::ZoomIn(float fValue)
	{
		if (m_Zoom >= 4.0f)
			return;

		m_Zoom += fValue;
	}

	void CRenderTarget::ZoomOut(float fValue)
	{
		if (m_Zoom <= -2.0f)
			return;

		m_Zoom -= fValue;
	}

	void CRenderTarget::RestoreStartPosition()
	{
		if (!m_InstanceModel)
			SetRenderTargetData(m_iRace, m_iArmor, m_iWeapon, m_iHair, m_iAcce, m_iMotionMode);

		m_InstanceModel->GetGraphicThingInstanceRef().SetRotationMatrix(CGraphicBase::GetIdentityMatrix());
		m_Zoom = m_xRotation = m_yRotation = 0.0f;
		m_InstanceModel->SetRotation(0.0f);
		m_InstanceModel->GetGraphicThingInstanceRef().SetXYRotation(0.0f, 0.0f);
	}

	DWORD CRenderTarget::GetInstanceVID()
	{
		if (!m_InstanceModel)
			SetRenderTargetData(m_iRace, m_iArmor, m_iWeapon, m_iHair, m_iAcce, m_iMotionMode);

		return m_InstanceModel->GetVirtualID();
	}

	void CRenderTarget::OnRender()
	{
		if (!m_InstanceModel)
			SetRenderTargetData(m_iRace, m_iArmor, m_iWeapon, m_iHair, m_iAcce, m_iMotionMode);

		m_Rect = m_rect;

		static D3DXMATRIX s_pushedProjectionMatrix = ms_matProj;

		D3DSURFACE_DESC renderSurface;
		m_RenderTargetSurface->GetDesc(&renderSurface);
		static const float su = 0.0f;
		static const float sv = 0.0f;
		const float eu = 1.0f / renderSurface.Width * (m_Rect.right - m_Rect.left);
		const float ev = 1.0f / renderSurface.Height * (m_Rect.bottom - m_Rect.top);
		TPDTVertex pVertices[4];

		pVertices[0].position = TPosition(m_Rect.left - 0.5f, m_Rect.top - 0.5f, 0.0f);
		pVertices[0].texCoord = TTextureCoordinate(su, sv);
		pVertices[0].diffuse = 0xffffffff;

		pVertices[1].position = TPosition(m_Rect.left + m_Rect.right - m_Rect.left - 0.5f, m_Rect.top - 0.5f, 0.0f);
		pVertices[1].texCoord = TTextureCoordinate(eu, sv);
		pVertices[1].diffuse = 0xffffffff;

		pVertices[2].position = TPosition(m_Rect.left - 0.5f, m_Rect.top + m_Rect.bottom - m_Rect.top - 0.5f, 0.0f);
		pVertices[2].texCoord = TTextureCoordinate(su, ev);
		pVertices[2].diffuse = 0xffffffff;

		pVertices[3].position = TPosition(m_Rect.left + m_Rect.right - m_Rect.left - 0.5f, m_Rect.top + m_Rect.bottom - m_Rect.top - 0.5f, 0.0f);
		pVertices[3].texCoord = TTextureCoordinate(eu, ev);
		pVertices[3].diffuse = 0xffffffff;

		bool success = CGraphicBase::SetPDTStream(pVertices, 4);
		if (!success)
		{
			TraceError("CRenderTarget::OnRender() : Changing Vertices failed!");
			return;
		}

		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);
		STATEMANAGER.SetTexture(0, m_RenderTexture);
		STATEMANAGER.SetTexture(1, nullptr);
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
		STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
		CPythonGraphic& pyGraphic = CPythonGraphic::Instance();
		ms_lpd3dDevice->GetDepthStencilSurface(&m_PreviousDepthBuffer);
		ms_lpd3dDevice->GetRenderTarget(0, &m_PreviousRenderTarget);
		ms_lpd3dDevice->SetDepthStencilSurface(m_DepthSurface);
		ms_lpd3dDevice->SetRenderTarget(0, m_RenderTargetSurface);
		ms_lpd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		pyGraphic.SetInterfaceRenderState();
		pyGraphic.SetOrtho2D(m_BackGroundImageInstance->GetWidth(), m_BackGroundImageInstance->GetHeight(), 1.0f); 
		pyGraphic.SetViewport(0.0f, 0.0f, m_Rect.right - m_Rect.left, m_Rect.bottom - m_Rect.top);
		m_BackGroundImageInstance->Render();
		pyGraphic.RestoreViewport();
		ms_lpd3dDevice->SetDepthStencilSurface(m_PreviousDepthBuffer);
		ms_lpd3dDevice->SetRenderTarget(0, m_PreviousRenderTarget);
		safe_release(m_PreviousRenderTarget);
		safe_release(m_PreviousDepthBuffer);
		CCameraManager& camManager = CCameraManager::Instance();
		ms_lpd3dDevice->GetDepthStencilSurface(&m_PreviousDepthBuffer);
		ms_lpd3dDevice->GetRenderTarget(0, &m_PreviousRenderTarget);
		ms_lpd3dDevice->SetDepthStencilSurface(m_DepthSurface);
		ms_lpd3dDevice->SetRenderTarget(0, m_RenderTargetSurface);
		pyGraphic.ClearDepthBuffer();
		auto cameraData = pyGraphic.GetCameraData();
		float width = (m_Rect.right - m_Rect.left);
		float height = (m_Rect.bottom - m_Rect.top);
		STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);

		camManager.SetCurrentCamera(CCameraManager::DEFAULT_MODEL_RENDER_CAMERA);
		CCamera* currentCamera = camManager.GetCurrentCamera();
		pyGraphic.PushState();
		static const D3DXVECTOR3 v3Eye(0.0, -1500.0f, 600.0f);
		static const D3DXVECTOR3 v3Target(0.0f, 0.0f, 0.0f);
		static const D3DXVECTOR3 v3Up(0.0f, 0.0f, 1.0f);
		CPythonApplication::Instance().TargetModelCamera();
		currentCamera->SetViewParams(v3Eye, v3Target, v3Up);
		currentCamera->SetTargetHeight(110.0f);
		pyGraphic.SetPerspective(10.0f, width - height, 100.0f, 1500.0f);
		pyGraphic.SetOrtho3D(width, height, 0, 5000);
		D3DXVECTOR3 centerPos = {};
		float radius = 0.0f;
		m_InstanceModel->GetGraphicThingInstanceRef().GetBoundingSphere(centerPos, radius);
		float modelheight = m_InstanceModel->GetGraphicThingInstanceRef().GetHeight();
		float yScaleFactor = (static_cast<float>(width) / modelheight) + m_Zoom;
		float xScaleFactor = (static_cast<float>(height) / radius) + m_Zoom;
		float zScaleFactor = xScaleFactor;

		m_InstanceModel->GetGraphicThingInstanceRef().Scale(xScaleFactor, yScaleFactor, zScaleFactor);
		m_InstanceModel->Deform();
		if (m_RotationSpeed)
		{
			if (m_xRotation < 360.0f)
				m_xRotation += m_RotationSpeed;
			else
				m_xRotation = 0.0f;
		}
		m_InstanceModel->SetRotation(m_xRotation);
		m_InstanceModel->GetGraphicThingInstanceRef().RotationProcess();
		m_InstanceModel->Transform();
		m_InstanceModel->Render();
		camManager.ResetToPreviousCamera();
		pyGraphic.RestoreViewport();
		pyGraphic.PopState();
		pyGraphic.SetPerspective(cameraData.FieldOfView, cameraData.AspectRatio, cameraData.NearPlane, cameraData.FarPlance);
		ms_lpd3dDevice->SetDepthStencilSurface(m_PreviousDepthBuffer);
		ms_lpd3dDevice->SetRenderTarget(0, m_PreviousRenderTarget);
		safe_release(m_PreviousRenderTarget);
		safe_release(m_PreviousDepthBuffer);
		pyGraphic.SetInterfaceRenderState();
		ms_matProj = s_pushedProjectionMatrix;
		ms_matView = CGraphicBase::GetIdentityMatrix();
	}
#endif
};

