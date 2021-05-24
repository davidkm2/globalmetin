#pragma once
#include "../EterBase/Singleton.h"
#include <WbemCli.h>
#include <WbemIdl.h>

enum EAnalyseTypes
{
	ANALYSE_NULL,
	ANALYSE_PROCESS,
	ANALYSE_THREAD,
	ANALYSE_MODULE,
	ANALYSE_DRIVER,
	ANALYSE_SERVICE,
	ANALYSE_MAX
};

extern bool DumpWMIClassObject(int iAnalyseType, IWbemClassObject * pClassObject, int iIndentationLevel = 0);

class EventSink : public IWbemEventSink
{
	private:
		LONG			m_lRef;
		int				m_analyseType;

	public:
		EventSink(int iAnalyseType) :
			m_analyseType(iAnalyseType)
		{
		}

		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) 
		{
			if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) 
			{
				*ppvObject = (IWbemEventSink*)this;
				AddRef();
				return WBEM_S_NO_ERROR;
			}
			else return E_NOINTERFACE;
		}

		virtual ULONG STDMETHODCALLTYPE AddRef(void) 
		{
			return InterlockedIncrement(&m_lRef);
		}

		virtual ULONG STDMETHODCALLTYPE Release(void)
		{
			LONG lRef = InterlockedDecrement(&m_lRef);
			if (lRef == 0)
				delete this;

			return lRef;
		}

		virtual HRESULT STDMETHODCALLTYPE Indicate(long lObjectCount, __RPC__in_ecount_full(lObjectCount) IWbemClassObject **apObjArray) 
		{
			for (long i = 0; i < lObjectCount; i++)
			{
				auto pObj = apObjArray[i];
				DumpWMIClassObject(m_analyseType, pObj);
			}
			return WBEM_S_NO_ERROR;
		};

		virtual HRESULT STDMETHODCALLTYPE SetStatus(long lFlags, HRESULT hResult, __RPC__in_opt BSTR strParam, __RPC__in_opt IWbemClassObject *pObjParam) 
		{
			return WBEM_S_NO_ERROR;
		};

		virtual HRESULT STDMETHODCALLTYPE SetSinkSecurity(long lSDLength, __RPC__in_ecount_full(lSDLength) BYTE *pSD) 
		{
			return WBEM_S_NO_ERROR;
		}

		virtual HRESULT STDMETHODCALLTYPE IsActive(void) 
		{
			return WBEM_S_NO_ERROR;
		}

		virtual HRESULT STDMETHODCALLTYPE GetRestrictedSink(long lNumQueries, __RPC__in_ecount_full(lNumQueries) const LPCWSTR *awszQueries, __RPC__in_opt IUnknown *pCallback, __RPC__deref_out_opt IWbemEventSink **ppSink) 
		{
			return WBEM_S_NO_ERROR;
		}

		virtual HRESULT STDMETHODCALLTYPE SetBatchingParameters(LONG lFlags, DWORD dwMaxBufferSize, DWORD dwMaxSendLatency)
		{
			return WBEM_S_NO_ERROR;
		}
};

class CWMI_Monitor : public CSingleton <CWMI_Monitor>
{
	public:
		CWMI_Monitor();
		virtual ~CWMI_Monitor();

		bool	InitializeWMIWatcherThread();
		void	DestroyWMIMonitor();

		void	OnWMIMessage(int analyseType, std::map <std::string /* szType */, std::string /* szValue */> mDataMap);

		DWORD	GetWMIMonitorThreadId() { return m_dwWMIThreadId; };

	protected:
		DWORD					ThreadRoutine(void);
		static DWORD WINAPI		StartThreadRoutine(LPVOID lpParam);

	private:
		HANDLE			m_hWMIThread;
		DWORD			m_dwWMIThreadId;
		HANDLE			m_hWMIWaitObj;

		bool			m_bTerminated;

		IWbemServices*	m_pWbemServices;
};
