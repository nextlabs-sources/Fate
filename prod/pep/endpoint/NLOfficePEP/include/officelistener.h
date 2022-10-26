#pragma once

//
// helper base class
//
namespace{
	template <const IID* _ptr_eventiid>
	class base_event_handler
	{
	public:
		explicit base_event_handler() : m_spUnkCP(0), m_spUnkDispSink(0), m_dwSinkID(0){};
		~base_event_handler(void){};

		virtual HRESULT OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, 
			VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr){ return	E_NOTIMPL; };

	public:
		template <const IID* _ptr_eventiid>
		class event_disp :
			public CComObjectRootEx<CComSingleThreadModel>,
			public IDispatch
		{
		public:
			event_disp() :m_spHandler(0){};

			BEGIN_COM_MAP(event_disp)
				COM_INTERFACE_ENTRY(IDispatch)
				COM_INTERFACE_ENTRY_IID((*_ptr_eventiid), IDispatch)
			END_COM_MAP()

			friend class base_event_handler<_ptr_eventiid>;

			STDMETHOD(GetTypeInfoCount)(UINT* pctinfo){ return	E_NOTIMPL; };
			STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo){ return	E_NOTIMPL; };
			STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid){ return	E_NOTIMPL; };
			STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
			{
				if (m_spHandler)
					return m_spHandler->OnInvoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
				return S_OK;
			}

		private:
			base_event_handler<_ptr_eventiid>*   m_spHandler;
		};

		BOOL RegisterEventDispatch(IUnknown* spUnkCP)
		{
			HRESULT hr = S_OK;
			CComObject<event_disp<_ptr_eventiid>>*    spDisp = 0;
			hr = CComObject<event_disp<_ptr_eventiid>>::CreateInstance(&spDisp);
			if (FAILED(hr) || !spDisp)
			{
				//DP((L"RegisterEventDispatch:: Fail to create instance\n"));
				return FALSE;
			}

			spDisp->QueryInterface(IID_IUnknown, (void**)&m_spUnkDispSink);
			spDisp->m_spHandler = this;
			hr = AtlAdvise(spUnkCP, m_spUnkDispSink, (*_ptr_eventiid), &m_dwSinkID);
			if (FAILED(hr))
			{
				//DP((L"RegisterEventDispatch:: Fail to Advise event (%X)\n", hr));
				m_spUnkDispSink.Release();
				m_dwSinkID = 0;
				return FALSE;
			}

			m_spUnkCP = spUnkCP;
			return TRUE;
		}

		BOOL UnregisterEventDispatch()
		{
			HRESULT hr = S_OK;
			if (m_spUnkCP && m_dwSinkID)
			{
				AtlUnadvise(m_spUnkCP, (*_ptr_eventiid), m_dwSinkID);
				CComPtr<event_disp<_ptr_eventiid>> spDisp = 0;
				hr = m_spUnkDispSink->QueryInterface(*_ptr_eventiid, (void**)&spDisp);
				if (spDisp) spDisp->m_spHandler = NULL;
				m_spUnkDispSink.Release();
				m_dwSinkID = 0;
				m_spUnkCP = 0;
			}
			return TRUE;
		}

	private:
		CComPtr<IUnknown>   m_spUnkCP;
		CComPtr<IUnknown>   m_spUnkDispSink;
		DWORD               m_dwSinkID;
	};
}; // end namespece



/************************************************************************/
//Listener is used to accept events from APP level
//Use COM standard events notify mechanism  
/************************************************************************/
class COfficeListener
{
public:
	COfficeListener( );
	virtual ~COfficeListener(void);

	virtual bool NLSinkEvent( _In_ IDispatch* pApplication ) = NULL;

	virtual bool NLUnSinkEvent() = NULL;
};

class CWordListener : 
	public base_event_handler<&__uuidof(Word::ApplicationEvents4)>,
	public COfficeListener
{
public:
	CWordListener();
	~CWordListener(void);

public:
	HRESULT OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, 
		VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

	virtual bool NLSinkEvent(_In_ IDispatch* pApplication);

	virtual bool NLUnSinkEvent();
};


class CExcelListener : 
	public base_event_handler<&__uuidof(Excel::AppEvents)>,
	public COfficeListener
{
public:
	CExcelListener();
	virtual ~CExcelListener(void);

public:
	HRESULT OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, 
		VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

	virtual bool NLSinkEvent(_In_ IDispatch* pApplication);

	virtual bool NLUnSinkEvent();

private:
	void HookKeyboardMsg();
	void UnHookKeyboardMsg();

private:
	static LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);

	static void NLSetNeedDenyFlag(_In_ BOOL bNeedDeny);
	static BOOL NLGetNeedDenyFlag();

	static HHOOK m_hHookKeyBoard;
	static BOOL m_sbNeedDeny;
};

class CPPTListener : 
	public base_event_handler<&__uuidof(PPT::EApplication)>,
	public COfficeListener
{
public:
	CPPTListener();
	virtual ~CPPTListener(void);

public:
	virtual bool NLSinkEvent(_In_ IDispatch* pApplication);

	virtual bool NLUnSinkEvent();

	HRESULT OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, 
		VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

};