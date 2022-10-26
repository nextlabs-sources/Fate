// outlookImpl.h : Declaration of the CoutlookImpl

#pragma once
#include "resource.h"       // main symbols

#include "OutlookAddin.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

#import "..\import\2k3\MSADDNDR.DLL" \
	raw_interfaces_only, raw_native_types, no_namespace, named_guids, auto_search
#import "..\import\2k3\MSOUTL.OLB" \
	raw_interfaces_only, raw_native_types, no_namespace, named_guids, auto_search

// CoutlookImpl

class ATL_NO_VTABLE CoutlookImpl :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CoutlookImpl, &CLSID_outlookImpl>,
	public IDispatchImpl<IoutlookImpl, &IID_IoutlookImpl, &LIBID_OutlookAddinLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDispatchImpl<_IDTExtensibility2, &__uuidof(_IDTExtensibility2), &LIBID_AddInDesignerObjects, /* wMajor = */ 1, /* wMinor = */ 0>
{
public:
	CoutlookImpl()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_OUTLOOKIMPL)


	BEGIN_COM_MAP(CoutlookImpl)
		COM_INTERFACE_ENTRY(IoutlookImpl)
		COM_INTERFACE_ENTRY2(IDispatch, _IDTExtensibility2)
		COM_INTERFACE_ENTRY(_IDTExtensibility2)
	END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}
  STDMETHOD(GetTypeInfoCount)(UINT* pctinfo){UNREFERENCED_PARAMETER(pctinfo); return	E_NOTIMPL;};
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
    {
        UNREFERENCED_PARAMETER(itinfo); 
        UNREFERENCED_PARAMETER(lcid);
        UNREFERENCED_PARAMETER(pptinfo);
        return	E_NOTIMPL;
    }
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
    {
        UNREFERENCED_PARAMETER(riid); 
        UNREFERENCED_PARAMETER(rgszNames);; 
        UNREFERENCED_PARAMETER(cNames);
        UNREFERENCED_PARAMETER(lcid);
        UNREFERENCED_PARAMETER(rgdispid);
        return	E_NOTIMPL;
    }
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) ;
public:
	STDMETHOD(OnConnection)(LPDISPATCH Application, ext_ConnectMode ConnectMode, LPDISPATCH AddInInst, SAFEARRAY * * custom) ;

	//When the application close, the application will map the virtual function of OnDisconnection.
	STDMETHOD(OnDisconnection)(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom) ;

	STDMETHOD(OnAddInsUpdate)(SAFEARRAY * * custom);

	STDMETHOD(OnBeginShutdown)(SAFEARRAY * * custom);

	STDMETHOD(OnStartupComplete)(SAFEARRAY * * custom) ;
};

OBJECT_ENTRY_AUTO(__uuidof(outlookImpl), CoutlookImpl)
