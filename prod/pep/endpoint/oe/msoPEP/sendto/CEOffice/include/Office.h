// Office.h : Declaration of the COffice

#pragma once
#include "resource.h"       // main symbols

#include "CEOffice.h"
#include "Hook.h"

extern CComPtr<IDispatch> g_pApp;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// COffice

class ATL_NO_VTABLE COffice :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<COffice, &CLSID_Office>,
	public IDispatchImpl<IOffice, &IID_IOffice, &LIBID_CEOfficeLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDispatchImpl<_IDTExtensibility2, &__uuidof(_IDTExtensibility2), &LIBID_AddInDesignerObjects, /* wMajor = */ 1>
{
public:
	COffice()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_OFFICE)


	BEGIN_COM_MAP(COffice)
		COM_INTERFACE_ENTRY(IOffice)
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

public:


	// _IDTExtensibility2 Methods
public:
	STDMETHOD(OnConnection)(LPDISPATCH Application, ext_ConnectMode ConnectMode, LPDISPATCH AddInInst, SAFEARRAY * * custom)
	{
        UNREFERENCED_PARAMETER(custom);
        UNREFERENCED_PARAMETER(AddInInst);
        UNREFERENCED_PARAMETER(ConnectMode);
		g_pApp = Application;
		SetHook();

		tagDocType theDocType = SetType();

		HRESULT hr=	::CoMarshalInterThreadInterfaceInStream(GetAppIIDFromDocType(theDocType), g_pApp, &g_pStream); 
		if (FAILED(hr) || g_pStream == NULL)
		{
			OutputDebugStringW(L"\nMarshal Failed in OnConnection\n");
		}
		return S_OK;
	}

	STDMETHOD(OnDisconnection)(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom)
    {
        UNREFERENCED_PARAMETER(custom);
        UNREFERENCED_PARAMETER(RemoveMode);

		if (g_pStream != NULL)
		{
			tagDocType theDocType = SetType();
			HRESULT hr = ::CoGetInterfaceAndReleaseStream(g_pStream, GetAppIIDFromDocType(theDocType), (LPVOID*)&g_pApp); 
			if (FAILED(hr))
			{
				OutputDebugStringW(L"\n  unmarshal failed in disconnect\n");
			}
		}

		UnsetHook();
		return S_OK;
	}
	STDMETHOD(OnAddInsUpdate)(SAFEARRAY * * custom)
    {
        UNREFERENCED_PARAMETER(custom);
		return E_NOTIMPL;
	}
	STDMETHOD(OnStartupComplete)(SAFEARRAY * * custom)
    {
        UNREFERENCED_PARAMETER(custom);
		return E_NOTIMPL;
	}
	STDMETHOD(OnBeginShutdown)(SAFEARRAY * * custom)
    {
        UNREFERENCED_PARAMETER(custom);
		return E_NOTIMPL;
	}
};

OBJECT_ENTRY_AUTO(__uuidof(Office), COffice)
