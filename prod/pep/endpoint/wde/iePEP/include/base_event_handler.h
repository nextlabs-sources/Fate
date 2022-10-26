
#pragma once
#ifndef _BASE_EVENT_HANDLER_H_
#define _BASE_EVENT_HANDLER_H_

#define SELF_DELETE()   {delete this;}

template <const IID* _ptr_eventiid>
class base_event_handler
{
public:
    explicit base_event_handler() : m_spUnkCP(0), m_spUnkDispSink(0),m_dwSinkID(0){};
    ~base_event_handler(void){ UnregisterEventDispatch();};

    virtual HRESULT OnInvoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
    {
        UNREFERENCED_PARAMETER(puArgErr);
        UNREFERENCED_PARAMETER(pexcepinfo);
        UNREFERENCED_PARAMETER(pvarResult);
        UNREFERENCED_PARAMETER(pdispparams);
        UNREFERENCED_PARAMETER(wFlags);
        UNREFERENCED_PARAMETER(lcid);
        UNREFERENCED_PARAMETER(riid);
        UNREFERENCED_PARAMETER(dispidMember);
        return	E_NOTIMPL;
    }
 
public:
    template <const IID* _ptr_eventiid>
    class event_disp :
        public CComObjectRootEx<CComSingleThreadModel>,
        public IDispatch
    {
    public:
        event_disp():m_spHandler(0){};

        BEGIN_COM_MAP(event_disp)
            COM_INTERFACE_ENTRY(IDispatch)
            COM_INTERFACE_ENTRY_IID((*_ptr_eventiid), IDispatch)
        END_COM_MAP()

        friend class base_event_handler<_ptr_eventiid>;

        STDMETHOD(GetTypeInfoCount)(UINT* pctinfo){UNREFERENCED_PARAMETER(pctinfo); return	E_NOTIMPL;};
        STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
        {
            UNREFERENCED_PARAMETER(pptinfo);
            UNREFERENCED_PARAMETER(lcid);
            UNREFERENCED_PARAMETER(itinfo);
            return	E_NOTIMPL;
        }
        STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
        {
            UNREFERENCED_PARAMETER(rgdispid);
            UNREFERENCED_PARAMETER(lcid);
            UNREFERENCED_PARAMETER(cNames);
            UNREFERENCED_PARAMETER(rgszNames);
            UNREFERENCED_PARAMETER(riid);
            return	E_NOTIMPL;
        }
        STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
        {
            return m_spHandler->OnInvoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        }

    private:
        base_event_handler<_ptr_eventiid>*   m_spHandler;
    };

    BOOL RegisterEventDispatch(CComQIPtr<IUnknown,&IID_IUnknown> spUnkCP)
    {
        HRESULT hr = S_OK;
        CComObject<event_disp<_ptr_eventiid>>*    spDisp = 0;
        hr = CComObject<event_disp<_ptr_eventiid>>::CreateInstance(&spDisp);
        if(FAILED(hr) || !spDisp)
        {
            DP((L"RegisterEventDispatch:: Fail to create instance\n"));
            return FALSE;
        }

        spDisp->QueryInterface(IID_IUnknown, (void**)&m_spUnkDispSink);
        spDisp->m_spHandler = this;
        hr = AtlAdvise(spUnkCP, m_spUnkDispSink, (*_ptr_eventiid), &m_dwSinkID);
        if(FAILED(hr))
        {
            DP((L"RegisterEventDispatch:: Fail to Advise event\n"));
            m_dwSinkID      = 0;
            return FALSE;
        }

        m_spUnkCP = spUnkCP;
        return TRUE;
    }

    BOOL UnregisterEventDispatch()
    {
        if(m_spUnkCP && m_dwSinkID)
        {
            AtlUnadvise(m_spUnkCP, (*_ptr_eventiid), m_dwSinkID);

            m_dwSinkID = 0;
        }
        return TRUE;
    }

private:
    CComQIPtr<IUnknown,&IID_IUnknown>   m_spUnkCP;
    CComQIPtr<IUnknown,&IID_IUnknown>   m_spUnkDispSink;
    DWORD       m_dwSinkID;
};

#endif