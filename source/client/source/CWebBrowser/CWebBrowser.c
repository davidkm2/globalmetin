#include <windows.h>
#include <exdisp.h>
#include <mshtml.h>
#include <mshtmhst.h>
#define _ASSERT(x)

unsigned char WindowCount = 0;
static const TCHAR ClassName[] = "Browser Example";
static const SAFEARRAYBOUND ArrayBound = {1, 0};

HRESULT STDMETHODCALLTYPE Frame_QueryInterface(IOleInPlaceFrame FAR* This, REFIID riid, LPVOID FAR* ppvObj);
HRESULT STDMETHODCALLTYPE Frame_AddRef(IOleInPlaceFrame FAR* This);
HRESULT STDMETHODCALLTYPE Frame_Release(IOleInPlaceFrame FAR* This);
HRESULT STDMETHODCALLTYPE Frame_GetWindow(IOleInPlaceFrame FAR* This, HWND FAR* lphwnd);
HRESULT STDMETHODCALLTYPE Frame_ContextSensitiveHelp(IOleInPlaceFrame FAR* This, BOOL fEnterMode);
HRESULT STDMETHODCALLTYPE Frame_GetBorder(IOleInPlaceFrame FAR* This, LPRECT lprectBorder);
HRESULT STDMETHODCALLTYPE Frame_RequestBorderSpace(IOleInPlaceFrame FAR* This, LPCBORDERWIDTHS pborderwidths);
HRESULT STDMETHODCALLTYPE Frame_SetBorderSpace(IOleInPlaceFrame FAR* This, LPCBORDERWIDTHS pborderwidths);
HRESULT STDMETHODCALLTYPE Frame_SetActiveObject(IOleInPlaceFrame FAR* This, IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName);
HRESULT STDMETHODCALLTYPE Frame_InsertMenus(IOleInPlaceFrame FAR* This, HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
HRESULT STDMETHODCALLTYPE Frame_SetMenu(IOleInPlaceFrame FAR* This, HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
HRESULT STDMETHODCALLTYPE Frame_RemoveMenus(IOleInPlaceFrame FAR* This, HMENU hmenuShared);
HRESULT STDMETHODCALLTYPE Frame_SetStatusText(IOleInPlaceFrame FAR* This, LPCOLESTR pszStatusText);
HRESULT STDMETHODCALLTYPE Frame_EnableModeless(IOleInPlaceFrame FAR* This, BOOL fEnable);
HRESULT STDMETHODCALLTYPE Frame_TranslateAccelerator(IOleInPlaceFrame FAR* This, LPMSG lpmsg, WORD wID);

IOleInPlaceFrameVtbl MyIOleInPlaceFrameTable = {Frame_QueryInterface,
Frame_AddRef,
Frame_Release,
Frame_GetWindow,
Frame_ContextSensitiveHelp,
Frame_GetBorder,
Frame_RequestBorderSpace,
Frame_SetBorderSpace,
Frame_SetActiveObject,
Frame_InsertMenus,
Frame_SetMenu,
Frame_RemoveMenus,
Frame_SetStatusText,
Frame_EnableModeless,
Frame_TranslateAccelerator};

typedef struct {
	IOleInPlaceFrame	frame;
	HWND				window;
} _IOleInPlaceFrameEx;

HRESULT STDMETHODCALLTYPE Site_QueryInterface(IOleClientSite FAR* This, REFIID riid, void ** ppvObject);
HRESULT STDMETHODCALLTYPE Site_AddRef(IOleClientSite FAR* This);
HRESULT STDMETHODCALLTYPE Site_Release(IOleClientSite FAR* This);
HRESULT STDMETHODCALLTYPE Site_SaveObject(IOleClientSite FAR* This);
HRESULT STDMETHODCALLTYPE Site_GetMoniker(IOleClientSite FAR* This, DWORD dwAssign, DWORD dwWhichMoniker, IMoniker ** ppmk);
HRESULT STDMETHODCALLTYPE Site_GetContainer(IOleClientSite FAR* This, LPOLECONTAINER FAR* ppContainer);
HRESULT STDMETHODCALLTYPE Site_ShowObject(IOleClientSite FAR* This);
HRESULT STDMETHODCALLTYPE Site_OnShowWindow(IOleClientSite FAR* This, BOOL fShow);
HRESULT STDMETHODCALLTYPE Site_RequestNewObjectLayout(IOleClientSite FAR* This);

IOleClientSiteVtbl MyIOleClientSiteTable = {Site_QueryInterface,
Site_AddRef,
Site_Release,
Site_SaveObject,
Site_GetMoniker,
Site_GetContainer,
Site_ShowObject,
Site_OnShowWindow,
Site_RequestNewObjectLayout};

HRESULT STDMETHODCALLTYPE UI_QueryInterface(IDocHostUIHandler FAR* This, REFIID riid, void ** ppvObject);
HRESULT STDMETHODCALLTYPE UI_AddRef(IDocHostUIHandler FAR* This);
HRESULT STDMETHODCALLTYPE UI_Release(IDocHostUIHandler FAR* This);
HRESULT STDMETHODCALLTYPE UI_ShowContextMenu(IDocHostUIHandler FAR* This, DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved);
HRESULT STDMETHODCALLTYPE UI_GetHostInfo(IDocHostUIHandler FAR* This, DOCHOSTUIINFO __RPC_FAR *pInfo);
HRESULT STDMETHODCALLTYPE UI_ShowUI(IDocHostUIHandler FAR* This, DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc);
HRESULT STDMETHODCALLTYPE UI_HideUI(IDocHostUIHandler FAR* This);
HRESULT STDMETHODCALLTYPE UI_UpdateUI(IDocHostUIHandler FAR* This);
HRESULT STDMETHODCALLTYPE UI_EnableModeless(IDocHostUIHandler FAR* This, BOOL fEnable);
HRESULT STDMETHODCALLTYPE UI_OnDocWindowActivate(IDocHostUIHandler FAR* This, BOOL fActivate);
HRESULT STDMETHODCALLTYPE UI_OnFrameWindowActivate(IDocHostUIHandler FAR* This, BOOL fActivate);
HRESULT STDMETHODCALLTYPE UI_ResizeBorder(IDocHostUIHandler FAR* This, LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow);
HRESULT STDMETHODCALLTYPE UI_TranslateAccelerator(IDocHostUIHandler FAR* This, LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID);
HRESULT STDMETHODCALLTYPE UI_GetOptionKeyPath(IDocHostUIHandler FAR* This, LPOLESTR __RPC_FAR *pchKey, DWORD dw);
HRESULT STDMETHODCALLTYPE UI_GetDropTarget(IDocHostUIHandler FAR* This, IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget);
HRESULT STDMETHODCALLTYPE UI_GetExternal(IDocHostUIHandler FAR* This, IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
HRESULT STDMETHODCALLTYPE UI_TranslateUrl(IDocHostUIHandler FAR* This, DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut);
HRESULT STDMETHODCALLTYPE UI_FilterDataObject(IDocHostUIHandler FAR* This, IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet);

IDocHostUIHandlerVtbl MyIDocHostUIHandlerTable =  {UI_QueryInterface,
UI_AddRef,
UI_Release,
UI_ShowContextMenu,
UI_GetHostInfo,
UI_ShowUI,
UI_HideUI,
UI_UpdateUI,
UI_EnableModeless,
UI_OnDocWindowActivate,
UI_OnFrameWindowActivate,
UI_ResizeBorder,
UI_TranslateAccelerator,
UI_GetOptionKeyPath,
UI_GetDropTarget,
UI_GetExternal,
UI_TranslateUrl,
UI_FilterDataObject};

HRESULT STDMETHODCALLTYPE InPlace_QueryInterface(IOleInPlaceSite FAR* This, REFIID riid, void ** ppvObject);
HRESULT STDMETHODCALLTYPE InPlace_AddRef(IOleInPlaceSite FAR* This);
HRESULT STDMETHODCALLTYPE InPlace_Release(IOleInPlaceSite FAR* This);
HRESULT STDMETHODCALLTYPE InPlace_GetWindow(IOleInPlaceSite FAR* This, HWND FAR* lphwnd);
HRESULT STDMETHODCALLTYPE InPlace_ContextSensitiveHelp(IOleInPlaceSite FAR* This, BOOL fEnterMode);
HRESULT STDMETHODCALLTYPE InPlace_CanInPlaceActivate(IOleInPlaceSite FAR* This);
HRESULT STDMETHODCALLTYPE InPlace_OnInPlaceActivate(IOleInPlaceSite FAR* This);
HRESULT STDMETHODCALLTYPE InPlace_OnUIActivate(IOleInPlaceSite FAR* This);
HRESULT STDMETHODCALLTYPE InPlace_GetWindowContext(IOleInPlaceSite FAR* This, LPOLEINPLACEFRAME FAR* lplpFrame,LPOLEINPLACEUIWINDOW FAR* lplpDoc,LPRECT lprcPosRect,LPRECT lprcClipRect,LPOLEINPLACEFRAMEINFO lpFrameInfo);
HRESULT STDMETHODCALLTYPE InPlace_Scroll(IOleInPlaceSite FAR* This, SIZE scrollExtent);
HRESULT STDMETHODCALLTYPE InPlace_OnUIDeactivate(IOleInPlaceSite FAR* This, BOOL fUndoable);
HRESULT STDMETHODCALLTYPE InPlace_OnInPlaceDeactivate(IOleInPlaceSite FAR* This);
HRESULT STDMETHODCALLTYPE InPlace_DiscardUndoState(IOleInPlaceSite FAR* This);
HRESULT STDMETHODCALLTYPE InPlace_DeactivateAndUndo(IOleInPlaceSite FAR* This);
HRESULT STDMETHODCALLTYPE InPlace_OnPosRectChange(IOleInPlaceSite FAR* This, LPCRECT lprcPosRect);
IOleInPlaceSiteVtbl MyIOleInPlaceSiteTable =  {InPlace_QueryInterface,
InPlace_AddRef,
InPlace_Release,
InPlace_GetWindow,
InPlace_ContextSensitiveHelp,
InPlace_CanInPlaceActivate,
InPlace_OnInPlaceActivate,
InPlace_OnUIActivate,
InPlace_GetWindowContext,
InPlace_Scroll,
InPlace_OnUIDeactivate,
InPlace_OnInPlaceDeactivate,
InPlace_DiscardUndoState,
InPlace_DeactivateAndUndo,
InPlace_OnPosRectChange};

typedef struct {
	IOleInPlaceSite			inplace;
	_IOleInPlaceFrameEx		frame;
} _IOleInPlaceSiteEx;

typedef struct {
	IDocHostUIHandler		ui;
} _IDocHostUIHandlerEx;

typedef struct {
	IOleClientSite			client;
	_IOleInPlaceSiteEx		inplace;
	_IDocHostUIHandlerEx	ui;
} _IOleClientSiteEx;

#define NOTIMPLEMENTED _ASSERT(0); return(E_NOTIMPL)

HRESULT STDMETHODCALLTYPE UI_QueryInterface(IDocHostUIHandler FAR* This, REFIID riid, LPVOID FAR* ppvObj)
{
	return(Site_QueryInterface((IOleClientSite *)((char *)This - sizeof(IOleClientSite) - sizeof(_IOleInPlaceSiteEx)), riid, ppvObj));
}

HRESULT STDMETHODCALLTYPE UI_AddRef(IDocHostUIHandler FAR* This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE UI_Release(IDocHostUIHandler FAR* This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE UI_ShowContextMenu(IDocHostUIHandler FAR* This, DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE UI_GetHostInfo(IDocHostUIHandler FAR* This, DOCHOSTUIINFO __RPC_FAR *pInfo)
{
	pInfo->cbSize = sizeof(DOCHOSTUIINFO);

	pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER;

	pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

	return(S_OK);
}

HRESULT STDMETHODCALLTYPE UI_ShowUI(IDocHostUIHandler FAR* This, DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE UI_HideUI(IDocHostUIHandler FAR* This)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE UI_UpdateUI(IDocHostUIHandler FAR* This)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE UI_EnableModeless(IDocHostUIHandler FAR* This, BOOL fEnable)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE UI_OnDocWindowActivate(IDocHostUIHandler FAR* This, BOOL fActivate)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE UI_OnFrameWindowActivate(IDocHostUIHandler FAR* This, BOOL fActivate)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE UI_ResizeBorder(IDocHostUIHandler FAR* This, LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE UI_TranslateAccelerator(IDocHostUIHandler FAR* This, LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID)
{
	return(S_FALSE);
}

HRESULT STDMETHODCALLTYPE UI_GetOptionKeyPath(IDocHostUIHandler FAR* This, LPOLESTR __RPC_FAR *pchKey, DWORD dw)
{
	return(S_FALSE);
}

HRESULT STDMETHODCALLTYPE UI_GetDropTarget(IDocHostUIHandler FAR* This, IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
{
    return(S_FALSE);
}

HRESULT STDMETHODCALLTYPE UI_GetExternal(IDocHostUIHandler FAR* This, IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	*ppDispatch = 0;
	return(S_FALSE);
}

HRESULT STDMETHODCALLTYPE UI_TranslateUrl(IDocHostUIHandler FAR* This, DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
{
	*ppchURLOut = 0;
    return(S_FALSE);
}

HRESULT STDMETHODCALLTYPE UI_FilterDataObject(IDocHostUIHandler FAR* This, IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet)
{
	*ppDORet = 0;
	return(S_FALSE);
}

HRESULT STDMETHODCALLTYPE Site_QueryInterface(IOleClientSite FAR* This, REFIID riid, void ** ppvObject)
{
	if (!memcmp(riid, &IID_IUnknown, sizeof(GUID)) || !memcmp(riid, &IID_IOleClientSite, sizeof(GUID)))
		*ppvObject = &((_IOleClientSiteEx *)This)->client;
	else if (!memcmp(riid, &IID_IOleInPlaceSite, sizeof(GUID)))
		*ppvObject = &((_IOleClientSiteEx *)This)->inplace;
	else if (!memcmp(riid, &IID_IDocHostUIHandler, sizeof(GUID)))
		*ppvObject = &((_IOleClientSiteEx *)This)->ui;
	else
	{
		*ppvObject = 0;
		return(E_NOINTERFACE);
	}

	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Site_AddRef(IOleClientSite FAR* This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE Site_Release(IOleClientSite FAR* This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE Site_SaveObject(IOleClientSite FAR* This)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Site_GetMoniker(IOleClientSite FAR* This, DWORD dwAssign, DWORD dwWhichMoniker, IMoniker ** ppmk)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Site_GetContainer(IOleClientSite FAR* This, LPOLECONTAINER FAR* ppContainer)
{
	*ppContainer = 0;

	return(E_NOINTERFACE);
}

HRESULT STDMETHODCALLTYPE Site_ShowObject(IOleClientSite FAR* This)
{
	return(NOERROR);
}

HRESULT STDMETHODCALLTYPE Site_OnShowWindow(IOleClientSite FAR* This, BOOL fShow)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Site_RequestNewObjectLayout(IOleClientSite FAR* This)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE InPlace_QueryInterface(IOleInPlaceSite FAR* This, REFIID riid, LPVOID FAR* ppvObj)
{
	return(Site_QueryInterface((IOleClientSite *)((char *)This - sizeof(IOleClientSite)), riid, ppvObj));
}

HRESULT STDMETHODCALLTYPE InPlace_AddRef(IOleInPlaceSite FAR* This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE InPlace_Release(IOleInPlaceSite FAR* This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE InPlace_GetWindow(IOleInPlaceSite FAR* This, HWND FAR* lphwnd)
{
	*lphwnd = ((_IOleInPlaceSiteEx FAR*)This)->frame.window;

	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlace_ContextSensitiveHelp(IOleInPlaceSite FAR* This, BOOL fEnterMode)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE InPlace_CanInPlaceActivate(IOleInPlaceSite FAR* This)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlace_OnInPlaceActivate(IOleInPlaceSite FAR* This)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlace_OnUIActivate(IOleInPlaceSite FAR* This)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlace_GetWindowContext(IOleInPlaceSite FAR* This, LPOLEINPLACEFRAME FAR* lplpFrame, LPOLEINPLACEUIWINDOW FAR* lplpDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	*lplpFrame = (LPOLEINPLACEFRAME)&((_IOleInPlaceSiteEx *)This)->frame;

	*lplpDoc = 0;

	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->hwndFrame = ((_IOleInPlaceFrameEx *)*lplpFrame)->window;
	lpFrameInfo->haccel = 0;
	lpFrameInfo->cAccelEntries = 0;

	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlace_Scroll(IOleInPlaceSite FAR* This, SIZE scrollExtent)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE InPlace_OnUIDeactivate(IOleInPlaceSite FAR* This, BOOL fUndoable)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlace_OnInPlaceDeactivate(IOleInPlaceSite FAR* This)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlace_DiscardUndoState(IOleInPlaceSite FAR* This)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE InPlace_DeactivateAndUndo(IOleInPlaceSite FAR* This)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE InPlace_OnPosRectChange(IOleInPlaceSite FAR* This, LPCRECT lprcPosRect)
{
	IOleObject			*browserObject;
	IOleInPlaceObject	*inplace;

	browserObject = *((IOleObject **)((char *)This - sizeof(IOleObject *) - sizeof(IOleClientSite)));
	if (!browserObject->lpVtbl->QueryInterface(browserObject, &IID_IOleInPlaceObject, (void**)&inplace))
	{
		inplace->lpVtbl->SetObjectRects(inplace, lprcPosRect, lprcPosRect);
		inplace->lpVtbl->Release(inplace);
	}

	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Frame_QueryInterface(IOleInPlaceFrame FAR* This, REFIID riid, LPVOID FAR* ppvObj)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_AddRef(IOleInPlaceFrame FAR* This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE Frame_Release(IOleInPlaceFrame FAR* This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE Frame_GetWindow(IOleInPlaceFrame FAR* This, HWND FAR* lphwnd)
{
	*lphwnd = ((_IOleInPlaceFrameEx *)This)->window;
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Frame_ContextSensitiveHelp(IOleInPlaceFrame FAR* This, BOOL fEnterMode)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_GetBorder(IOleInPlaceFrame FAR* This, LPRECT lprectBorder)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_RequestBorderSpace(IOleInPlaceFrame FAR* This, LPCBORDERWIDTHS pborderwidths)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_SetBorderSpace(IOleInPlaceFrame FAR* This, LPCBORDERWIDTHS pborderwidths)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_SetActiveObject(IOleInPlaceFrame FAR* This, IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Frame_InsertMenus(IOleInPlaceFrame FAR* This, HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_SetMenu(IOleInPlaceFrame FAR* This, HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Frame_RemoveMenus(IOleInPlaceFrame FAR* This, HMENU hmenuShared)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_SetStatusText(IOleInPlaceFrame FAR* This, LPCOLESTR pszStatusText)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Frame_EnableModeless(IOleInPlaceFrame FAR* This, BOOL fEnable)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Frame_TranslateAccelerator(IOleInPlaceFrame FAR* This, LPMSG lpmsg, WORD wID)
{
	NOTIMPLEMENTED;
}

void UnEmbedBrowserObject(HWND hwnd)
{
	IOleObject	**browserHandle;
	IOleObject	*browserObject;

	if ((browserHandle = (IOleObject **)GetWindowLong(hwnd, GWL_USERDATA)))
	{
		browserObject = *browserHandle;
		browserObject->lpVtbl->Close(browserObject, OLECLOSE_NOSAVE);
		browserObject->lpVtbl->Release(browserObject);

		GlobalFree(browserHandle);

		return;
	}

	_ASSERT(0);
}

#define WEBPAGE_GOBACK		0
#define WEBPAGE_GOFORWARD	1
#define WEBPAGE_GOHOME		2
#define WEBPAGE_SEARCH		3
#define WEBPAGE_REFRESH		4
#define WEBPAGE_STOP		5

void DoPageAction(HWND hwnd, DWORD action)
{	
	IWebBrowser2	*webBrowser2;
	IOleObject		*browserObject;

	browserObject = *((IOleObject **)GetWindowLong(hwnd, GWL_USERDATA));

	if (!browserObject->lpVtbl->QueryInterface(browserObject, &IID_IWebBrowser2, (void**)&webBrowser2))
	{
		switch (action)
		{
			case WEBPAGE_GOBACK:
			{
				webBrowser2->lpVtbl->GoBack(webBrowser2);
				break;
			}

			case WEBPAGE_GOFORWARD:
			{
				webBrowser2->lpVtbl->GoForward(webBrowser2);
				break;
			}

			case WEBPAGE_GOHOME:
			{
				webBrowser2->lpVtbl->GoHome(webBrowser2);
				break;
			}

			case WEBPAGE_SEARCH:
			{
				webBrowser2->lpVtbl->GoSearch(webBrowser2);
				break;
			}

			case WEBPAGE_REFRESH:
			{
				webBrowser2->lpVtbl->Refresh(webBrowser2);
				break;
			}

			case WEBPAGE_STOP:
			{
				webBrowser2->lpVtbl->Stop(webBrowser2);
				break;
			}
		}

		webBrowser2->lpVtbl->Release(webBrowser2);
	}
}

long DisplayHTMLStr(HWND hwnd, LPCTSTR string)
{	
	IWebBrowser2	*webBrowser2;
	LPDISPATCH		lpDispatch;
	IHTMLDocument2	*htmlDoc2;
	IOleObject		*browserObject;
	SAFEARRAY		*sfArray;
	VARIANT			myURL;
	VARIANT			*pVar;
	BSTR			bstr;

	browserObject = *((IOleObject **)GetWindowLong(hwnd, GWL_USERDATA));

	bstr = 0;

	if (!browserObject->lpVtbl->QueryInterface(browserObject, &IID_IWebBrowser2, (void**)&webBrowser2))
	{
		VariantInit(&myURL);
		myURL.vt = VT_BSTR;
		myURL.bstrVal = SysAllocString(L"about:blank");

		webBrowser2->lpVtbl->Navigate2(webBrowser2, &myURL, 0, 0, 0, 0);

		VariantClear(&myURL);

		if (!webBrowser2->lpVtbl->get_Document(webBrowser2, &lpDispatch))
		{
			if (!lpDispatch->lpVtbl->QueryInterface(lpDispatch, &IID_IHTMLDocument2, (void**)&htmlDoc2))
			{
				if ((sfArray = SafeArrayCreate(VT_VARIANT, 1, (SAFEARRAYBOUND *)&ArrayBound)))
				{
					if (!SafeArrayAccessData(sfArray, (void**)&pVar))
					{
						pVar->vt = VT_BSTR;
#ifndef UNICODE
						{
						wchar_t		*buffer;
						DWORD		size;

						size = MultiByteToWideChar(CP_ACP, 0, string, -1, 0, 0);
						if (!(buffer = (wchar_t *)GlobalAlloc(GMEM_FIXED, sizeof(wchar_t) * size))) goto bad;
						MultiByteToWideChar(CP_ACP, 0, string, -1, buffer, size);
						bstr = SysAllocString(buffer);
						GlobalFree(buffer);
						}
#else
						bstr = SysAllocString(string);
#endif
						if ((pVar->bstrVal = bstr))
						{
							htmlDoc2->lpVtbl->write(htmlDoc2, sfArray);

							htmlDoc2->lpVtbl->close(htmlDoc2);
							
						}
					}

					SafeArrayDestroy(sfArray);
				}

bad:			htmlDoc2->lpVtbl->Release(htmlDoc2);
			}

			lpDispatch->lpVtbl->Release(lpDispatch);
		}

		webBrowser2->lpVtbl->Release(webBrowser2);
	}

	if (bstr)
		return(0);

	return(-1);
}

long DisplayHTMLPage(HWND hwnd, LPTSTR webPageName)
{
	IWebBrowser2	*webBrowser2;
	VARIANT			myURL;
	IOleObject		*browserObject;

	browserObject = *((IOleObject **)GetWindowLong(hwnd, GWL_USERDATA));

	if (!browserObject->lpVtbl->QueryInterface(browserObject, &IID_IWebBrowser2, (void**)&webBrowser2))
	{
		VariantInit(&myURL);
		myURL.vt = VT_BSTR;

#ifndef UNICODE
		{
		wchar_t		*buffer;
		DWORD		size;

		size = MultiByteToWideChar(CP_ACP, 0, webPageName, -1, 0, 0);
		if (!(buffer = (wchar_t *)GlobalAlloc(GMEM_FIXED, sizeof(wchar_t) * size))) goto badalloc;
		MultiByteToWideChar(CP_ACP, 0, webPageName, -1, buffer, size);
		myURL.bstrVal = SysAllocString(buffer);
		GlobalFree(buffer);
		}
#else
		myURL.bstrVal = SysAllocString(webPageName);
#endif
		if (!myURL.bstrVal)
		{
badalloc:	webBrowser2->lpVtbl->Release(webBrowser2);
			return(-6);
		}

		webBrowser2->lpVtbl->Navigate2(webBrowser2, &myURL, 0, 0, 0, 0);

		VariantClear(&myURL);

		webBrowser2->lpVtbl->Release(webBrowser2);

		return(0);
	}

	return(-5);
}

void ResizeBrowser(HWND hwnd, DWORD width, DWORD height)
{
	IWebBrowser2	*webBrowser2;
	IOleObject		*browserObject;

	browserObject = *((IOleObject **)GetWindowLong(hwnd, GWL_USERDATA));

	if (!browserObject->lpVtbl->QueryInterface(browserObject, &IID_IWebBrowser2, (void**)&webBrowser2))
	{
		webBrowser2->lpVtbl->put_Width(webBrowser2, width);
		webBrowser2->lpVtbl->put_Height(webBrowser2, height);

		webBrowser2->lpVtbl->Release(webBrowser2);
	}
}

long EmbedBrowserObject(HWND hwnd)
{
	HRESULT hr;
	LPCLASSFACTORY		pClassFactory;
	IOleObject			*browserObject;
	IWebBrowser2		*webBrowser2;
	RECT				rect;
	char				*ptr;
	_IOleClientSiteEx	*_iOleClientSiteEx;

	if (!(ptr = (char *)GlobalAlloc(GMEM_FIXED, sizeof(_IOleClientSiteEx) + sizeof(IOleObject *))))
	{
		return(-1);
	}

	_iOleClientSiteEx = (_IOleClientSiteEx *)(ptr + sizeof(IOleObject *));
	_iOleClientSiteEx->client.lpVtbl = &MyIOleClientSiteTable;

	_iOleClientSiteEx->inplace.inplace.lpVtbl = &MyIOleInPlaceSiteTable;

	_iOleClientSiteEx->inplace.frame.frame.lpVtbl = &MyIOleInPlaceFrameTable;

	_iOleClientSiteEx->inplace.frame.window = hwnd;

	_iOleClientSiteEx->ui.ui.lpVtbl = &MyIDocHostUIHandlerTable;
	
	pClassFactory = 0;
	hr =  
		CoGetClassObject(
		&CLSID_WebBrowser, 
		CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, NULL, &IID_IClassFactory, (void **)&pClassFactory) && pClassFactory;

	if (!hr||!pClassFactory)
	{
		if (!pClassFactory->lpVtbl->CreateInstance(pClassFactory, 0, &IID_IOleObject, &browserObject))
		{
			pClassFactory->lpVtbl->Release(pClassFactory);

			*((IOleObject **)ptr) = browserObject;
			SetWindowLong(hwnd, GWL_USERDATA, (LONG)ptr);

			if (!browserObject->lpVtbl->SetClientSite(browserObject, (IOleClientSite *)_iOleClientSiteEx))
			{
				browserObject->lpVtbl->SetHostNames(browserObject, L"My Host Name", 0);

				GetClientRect(hwnd, &rect);

				if (!OleSetContainedObject((struct IUnknown *)browserObject, TRUE) &&

					!browserObject->lpVtbl->DoVerb(browserObject, OLEIVERB_SHOW, NULL, (IOleClientSite *)_iOleClientSiteEx, -1, hwnd, &rect) &&

					!browserObject->lpVtbl->QueryInterface(browserObject, &IID_IWebBrowser2, (void**)&webBrowser2))
				{
					webBrowser2->lpVtbl->put_Left(webBrowser2, 0);
					webBrowser2->lpVtbl->put_Top(webBrowser2, 0);
					webBrowser2->lpVtbl->put_Width(webBrowser2, rect.right);
					webBrowser2->lpVtbl->put_Height(webBrowser2, rect.bottom);

					webBrowser2->lpVtbl->Release(webBrowser2);

					return(0);
				}
			}

			UnEmbedBrowserObject(hwnd);
			return(-4);
		}

		pClassFactory->lpVtbl->Release(pClassFactory);
		GlobalFree(ptr);

		return(-3);
	}

	GlobalFree(ptr);

	return(-2);
}
LRESULT CALLBACK WebBrowser_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SIZE:
		{
			return(0);
		}

		case WM_CREATE:
		{
			if (EmbedBrowserObject(hwnd)) 
				return(-1);

			return(0);
		}

		case WM_DESTROY:
		{
			UnEmbedBrowserObject(hwnd);
			return(TRUE);
		}
	}

	return(DefWindowProc(hwnd, uMsg, wParam, lParam));
}

static const char* WEBBROWSER_CLASSNAME = "WEBBROWSER";
static HINSTANCE gs_hInstance = NULL;
static HWND gs_hWndWebBrowser = NULL;
static HWND gs_hWndParent = NULL;
int WebBrowser_Show(HWND hParent, const char* addr, const RECT* rc)
{
	if (gs_hWndWebBrowser)
		return 0;

	gs_hWndParent = hParent;
	gs_hWndWebBrowser = CreateWindowEx(0, WEBBROWSER_CLASSNAME, "web browser", WS_CHILD|WS_VISIBLE,
					rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top,
					hParent, NULL, gs_hInstance, 0);

	if (!gs_hWndWebBrowser)
		return 0;

	DisplayHTMLPage(gs_hWndWebBrowser, (LPTSTR)addr);

	ShowWindow(gs_hWndWebBrowser, SW_SHOW);
	UpdateWindow(gs_hWndWebBrowser);

	SetFocus(gs_hWndWebBrowser);
	return 1;
}

void WebBrowser_Move(const RECT* rc)
{
	MoveWindow(gs_hWndWebBrowser, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, 1 );
}

void WebBrowser_Hide()
{
	if (!gs_hWndWebBrowser)
		return;

	ShowWindow(gs_hWndWebBrowser, SW_HIDE);

	if (IsWindow(gs_hWndWebBrowser))
		DestroyWindow(gs_hWndWebBrowser);
	gs_hWndWebBrowser = NULL;

	SetFocus(gs_hWndParent);
}

int WebBrowser_IsVisible()
{
	return (gs_hWndWebBrowser != NULL);
}

void WebBrowser_Destroy()
{
	WebBrowser_Hide();
}

int WebBrowser_Startup(HINSTANCE hInstance)
{	
	if (OleInitialize(NULL) != S_OK)
	{
		return 0;
	}

	{
		WNDCLASSEX		wc;
		ZeroMemory(&wc, sizeof(WNDCLASSEX));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.hInstance = hInstance;
		wc.lpfnWndProc = WebBrowser_WindowProc;
		wc.lpszClassName = WEBBROWSER_CLASSNAME;
		RegisterClassEx(&wc);
	}
	gs_hInstance = hInstance;
	return 1;
}

void WebBrowser_Cleanup()
{
	if (gs_hInstance)
		UnregisterClass(WEBBROWSER_CLASSNAME, gs_hInstance);

	OleUninitialize();
}

