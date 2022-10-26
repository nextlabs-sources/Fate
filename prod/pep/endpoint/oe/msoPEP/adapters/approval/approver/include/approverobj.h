// approverobj.h : Declaration of the Capproverobj

#pragma once
#include "resource.h"       // main symbols
#include "approver.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


extern _ATL_FUNC_INFO OnClickButtonInfo;
// Capproverobj
class ATL_NO_VTABLE Capproverobj :
    public CComObjectRootEx<CComSingleThreadModel>,
    //public IDispatch,
	public CComCoClass<Capproverobj, &CLSID_approverobj>,
    public IDispatchImpl<Iapproverobj, &IID_Iapproverobj, &LIBID_approverLib>,//, /*wMajor =*/ 1, /*wMinor =*/ 0>,
    public IDispatchImpl<_IDTExtensibility2, &__uuidof(_IDTExtensibility2), &LIBID_AddInDesignerObjects>//, /* wMajor = */ 1, /* wMinor = */ 0>
{
public:
	Capproverobj()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_APPROVEROBJ)

BEGIN_COM_MAP(Capproverobj)
	COM_INTERFACE_ENTRY(Iapproverobj)
    COM_INTERFACE_ENTRY2(IDispatch, Iapproverobj)
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

    // _IDTExtensibility2 Methods
public:
    STDMETHOD(OnAddInsUpdate)(SAFEARRAY * * custom){return	E_NOTIMPL;};
    STDMETHOD(OnStartupComplete)(SAFEARRAY * * custom){return	E_NOTIMPL;};
    STDMETHOD(OnBeginShutdown)(SAFEARRAY * * custom){return	E_NOTIMPL;};
    STDMETHOD(OnConnection)(LPDISPATCH Application, ext_ConnectMode ConnectMode, LPDISPATCH AddInInst, SAFEARRAY * * custom);
    STDMETHOD(OnDisconnection)(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom);
};

OBJECT_ENTRY_AUTO(__uuidof(approverobj), Capproverobj)
