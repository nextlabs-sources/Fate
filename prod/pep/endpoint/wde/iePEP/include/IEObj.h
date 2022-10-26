// IEObj.h : Declaration of the CIEObj

#pragma once
#include "resource.h"       // main symbols

#include "iePEP.h"

#include "eframework/policy/comm_helper.hpp"
#include "ActionHandler.h"

extern nextlabs::cesdk_context cesdk_context_instance;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

DEFINE_GUID(CATID_AppContainerCompatible, 
			0x59fb2056,0xd625,0x48d0,0xa9,0x44,0x1a,0x85,0xb5,0xab,0x26,0x40);

// CIEObj

class ATL_NO_VTABLE CIEObj :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CIEObj, &CLSID_IEObj>,
	public IObjectWithSiteImpl<CIEObj>,
	public IDispatchImpl<IIEObj, &IID_IIEObj, &LIBID_iePEPLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CIEObj()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_IEOBJ)


BEGIN_COM_MAP(CIEObj)
	COM_INTERFACE_ENTRY(IIEObj)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
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
    // IOleObjectWithSite Methods
    STDMETHOD(SetSite)(IUnknown *pUnkSite);

private:
    //std::vector<CObjectHandler<IWebBrowser2, CWebBrowserEventDisp>*>   m_vWebBrowser2Handler;
};

OBJECT_ENTRY_AUTO(__uuidof(IEObj), CIEObj)
