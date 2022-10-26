// InspEventDisp.h : Declaration of the CInspEventDisp

#pragma once
#include "resource.h"       // main symbols

#include "msoPEP.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CInspEventDisp

class ATL_NO_VTABLE CInspEventDisp :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatch,
	public CComCoClass<CInspEventDisp, &CLSID_InspEventDisp>,
	public IDispatchImpl<IInspEventDisp, &IID_IInspEventDisp, &LIBID_msoPEPLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CInspEventDisp()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_INSPEVENTDISP)


BEGIN_COM_MAP(CInspEventDisp)
	COM_INTERFACE_ENTRY(IInspEventDisp)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_IID(__uuidof(Outlook::InspectorEvents), IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo){UNREFERENCED_PARAMETER(pctinfo); return	E_NOTIMPL;};
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
    {
        UNREFERENCED_PARAMETER(itinfo);
        UNREFERENCED_PARAMETER(lcid);
        UNREFERENCED_PARAMETER(pptinfo);
        return	E_NOTIMPL;
    };
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
    {
        UNREFERENCED_PARAMETER(riid);
        UNREFERENCED_PARAMETER(rgszNames);
        UNREFERENCED_PARAMETER(cNames);
        UNREFERENCED_PARAMETER(lcid);
        UNREFERENCED_PARAMETER(rgdispid);
        return	E_NOTIMPL;
    };
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

    BOOL InitInspSink(CComPtr<Outlook::_Inspector> spInsp, CComPtr<IUnknown> spInpSinkObj, DWORD dwInspSinkCookie);
	CComPtr<IDispatch>& Get_MailItemDisp(void)
	{
		return m_spMailItem ;
	}


protected:
    void OnActivate();
    void OnDeactivate();
    void OnClose();

private:
	

 
	CComPtr<IDispatch>		m_spMailItem ;
	ITEM_TYPE				m_mailItemType ;
    CComPtr<IUnknown>       m_spInspSinkObj;
    DWORD                   m_spInspSinkCookie;
    CComPtr<IUnknown>       m_spMailItemSinkObj;
    DWORD                   m_spMailItemSinkCookie;
    BOOL                    m_bClosed;
};

OBJECT_ENTRY_AUTO(__uuidof(InspEventDisp), CInspEventDisp)
