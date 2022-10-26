// msoObj.h : Declaration of the CmsoObj

#pragma once
#include "resource.h"       // main symbols

#include "msoPEP.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



class NLComAddIn
{
private:
	NLComAddIn(void);
	~NLComAddIn(void);
public:
	static NLComAddIn& Instance();
	void ReActiveAddIn();
private:
    CComPtr<IDispatch> GetOEObject();
    static void ActiveAddIn(void* pArguments);
    void QuitApp(CComPtr<IDispatch> pDisp);
    static bool ActiveOEInHKURegEntry();
    static bool ReConnect(CComPtr<IDispatch> pObject);
    static bool IsItemContentChanged(CComPtr<IDispatch> pdisp);
private:
	bool m_bLoadItself;
};



// CmsoObj


class ATL_NO_VTABLE CmsoObj :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatch,
	public CComCoClass<CmsoObj, &CLSID_msoObj>,
    public IDispatchImpl<ImsoObj, &IID_ImsoObj, &LIBID_msoPEPLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
    public IDispatchImpl<_IDTExtensibility2, &__uuidof(_IDTExtensibility2), &LIBID_AddInDesignerObjects, /* wMajor = */ 1, /* wMinor = */ 0>
{
public:
	CmsoObj()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MSOOBJ)


	BEGIN_COM_MAP(CmsoObj)
		COM_INTERFACE_ENTRY(ImsoObj)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(_IDTExtensibility2)
		COM_INTERFACE_ENTRY_IID(__uuidof(Outlook::ApplicationEvents), IDispatch)
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


    // _IDTExtensibility2 Methods
public:
    STDMETHOD(OnAddInsUpdate)(SAFEARRAY * * custom){UNREFERENCED_PARAMETER(custom); return	E_NOTIMPL;};
    STDMETHOD(OnStartupComplete)(SAFEARRAY * * custom);//{UNREFERENCED_PARAMETER(custom); return	E_NOTIMPL;};
    STDMETHOD(OnBeginShutdown)(SAFEARRAY * * custom){UNREFERENCED_PARAMETER(custom); return	E_NOTIMPL;};
    STDMETHOD(OnConnection)(LPDISPATCH Application, ext_ConnectMode ConnectMode, LPDISPATCH AddInInst, SAFEARRAY * * custom);
    STDMETHOD(OnDisconnection)(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom);
private:
	static BOOL CALLBACK EnumMailWinProc(HWND hwnd, LPARAM lParam);
};

OBJECT_ENTRY_AUTO(__uuidof(msoObj), CmsoObj)
