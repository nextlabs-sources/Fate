// CE_Explorer.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "CE_Explorer.h"


class CCE_ExplorerModule : public CAtlDllModuleT< CCE_ExplorerModule >
{
public :
	DECLARE_LIBID(LIBID_CE_ExplorerLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_CE_EXPLORER, "{52575F50-B968-493B-865F-A80C80622AA0}")
};

CCE_ExplorerModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

HINSTANCE g_hInstance;
// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	g_hInstance = hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif




// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	if ( 0 == (GetVersion() & 0x80000000UL) )
	{
		CRegKey theReg;
		LONG lRet = theReg.Create(HKEY_CLASSES_ROOT,L"*\\shellex\\ContextMenuHandlers\\CE_Explorer");
		if(ERROR_SUCCESS == lRet)
		{
			lRet = theReg.SetStringValue(NULL,L"{38B14C4F-31AE-468B-8BD2-DCB57645074A}");
			theReg.Close();
		}
		else
			return E_ACCESSDENIED;
		lRet = theReg.Create(HKEY_CLASSES_ROOT,L"Directory\\shellex\\ContextMenuHandlers\\CE_Explorer");
		if(ERROR_SUCCESS == lRet)
		{
			lRet = theReg.SetStringValue(NULL,L"{38B14C4F-31AE-468B-8BD2-DCB57645074A}");
			theReg.Close();
		}
		else
			return E_ACCESSDENIED;
	}
	// registers object, typelib and all interfaces in typelib
	HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	if ( 0 == (GetVersion() & 0x80000000UL) )
	{
		CRegKey theReg;
		LONG lRet = theReg.Open(HKEY_CLASSES_ROOT,L"*\\shellex\\ContextMenuHandlers");
		if(lRet == ERROR_SUCCESS)
		{
			lRet = theReg.DeleteSubKey(L"CE_Explorer");
			theReg.Close();
		}
		lRet = theReg.Open(HKEY_CLASSES_ROOT,L"Directory\\shellex\\ContextMenuHandlers");
		if(lRet == ERROR_SUCCESS)
		{
			lRet = theReg.DeleteSubKey(L"CE_Explorer");
			theReg.Close();
		}
	}
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}

