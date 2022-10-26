// CEOffice.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "CEOffice.h"
#include "ProductVersions.h"

#define VERSION_PRODUCT_OE			"Enforcer for Microsoft Outlook "

class CCEOfficeModule : public CAtlDllModuleT< CCEOfficeModule >
{
public :
	DECLARE_LIBID(LIBID_CEOfficeLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_CEOFFICE, "{8F1947F8-E4FE-4EDF-AEEC-915E4D043686}")
};

CCEOfficeModule _AtlModule;


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

HRESULT RegisterDll(LPCWSTR AppName)
{
    HRESULT hr = S_OK;
	DWORD   dwDisposition = 0;
	HKEY    hKeyOutlook   = NULL;
	HKEY    hKeyAddin     = NULL;
	LONG    lResult       = 0;
	DWORD   dwVal         = 0;
    char    szVal[MAX_PATH] = {0};
    WCHAR   wzKeyName[MAX_PATH] = {0};

	__try
	{
		_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Microsoft\\Office\\%s\\Addins", AppName);
		lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyOutlook, NULL);
		if (ERROR_SUCCESS!=lResult || hKeyOutlook==NULL)
		{
			hr = E_UNEXPECTED;
			__leave;
		}

		lResult = RegCreateKeyEx( hKeyOutlook, L"OEOffice.Office.1",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyAddin,&dwDisposition);
		if(ERROR_SUCCESS != lResult)
        {
            hr = E_UNEXPECTED;
            __leave;
		}
		dwVal = 0;
		RegSetValueEx(hKeyAddin, L"CommandLineSafe", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
		dwVal = 3;
		RegSetValueEx(hKeyAddin, L"LoadBehavior", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
		_snprintf_s(szVal, MAX_PATH, _TRUNCATE, VERSION_PRODUCT_OE/*"Compliant Enterprise Office PEP"*/);
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

	return hr;
}

HRESULT UnRegisterDll(LPCWSTR AppName)
{
	WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));
	_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Microsoft\\Office\\%s\\Addins\\OEOffice.Office.1", AppName);
	RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
	return S_OK;
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
	RegisterDll(L"Word");
	RegisterDll(L"Excel");
	RegisterDll(L"PowerPoint");
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	UnRegisterDll(L"Word");
	UnRegisterDll(L"Excel");
	UnRegisterDll(L"PowerPoint");
	return hr;
}

