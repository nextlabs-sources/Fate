// approver.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "approver.h"

HINSTANCE   g_hInstance = NULL;

static HRESULT RegisterDll(LPCWSTR AppName)
{
    DWORD   dwDisposition = 0;
    HKEY    hKeyOutlook   = NULL;
    HKEY    hKeyAddin     = NULL;
    LONG    lResult       = 0;
    DWORD   dwVal         = 0;
    char    szVal[MAX_PATH];
    WCHAR   wzKeyName[MAX_PATH];

	HRESULT hRes = S_OK;
    __try
    {
        _snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Microsoft\\Office\\%s\\Addins", AppName);
        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, KEY_ALL_ACCESS, &hKeyOutlook);
        if(ERROR_SUCCESS != lResult)     // get office/outlook key
        {
            hRes = E_UNEXPECTED;
			__leave;
        }
        lResult = RegCreateKeyEx( hKeyOutlook, L"approver.approverobj.1",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyAddin,&dwDisposition);
        if(ERROR_SUCCESS != lResult)
        {
			hRes = E_UNEXPECTED;
			__leave;
        }
        dwVal = 0;
        RegSetValueEx(hKeyAddin, L"CommandLineSafe", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
        dwVal = 3;
        RegSetValueEx(hKeyAddin, L"LoadBehavior", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
        _snprintf_s(szVal, MAX_PATH, _TRUNCATE, "Compliant Enterprise Approver");
        RegSetValueExA(hKeyAddin, "FriendlyName", 0, REG_SZ, (const BYTE*)szVal, 1+(DWORD)strlen(szVal));
        RegSetValueExA(hKeyAddin, "Description", 0, REG_SZ, (const BYTE*)szVal, 1+(DWORD)strlen(szVal));
    }
    __finally
    {
        if(NULL != hKeyAddin) RegCloseKey(hKeyAddin);
        if(NULL != hKeyOutlook) RegCloseKey(hKeyOutlook);

        hKeyAddin   = NULL;
        hKeyOutlook = NULL;
    }

    return hRes;
}

static HRESULT UnRegisterDll(LPCWSTR AppName)
{
    WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));
    _snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Microsoft\\Office\\%s\\Addins\\approver.approverobj.1", AppName);
    RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
    return S_OK;
}


class CapproverModule : public CAtlDllModuleT< CapproverModule >
{
public :
	DECLARE_LIBID(LIBID_approverLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_APPROVER, "{51C7EF58-0181-414D-B49C-892997951918}")
};

CapproverModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
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
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
    RegisterDll(L"Outlook");
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
    HRESULT hr = _AtlModule.DllUnregisterServer();
    UnRegisterDll(L"Outlook");
	return hr;
}

