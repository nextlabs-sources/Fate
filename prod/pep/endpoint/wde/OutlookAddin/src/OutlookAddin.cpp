// OutlookAddin.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "OutlookAddin.h"
#include "dllmain.h"

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
	 //HRESULT hr = S_OK;
    DWORD   dwDisposition = 0;
    HKEY    hKeyOutlook   = NULL;
    HKEY    hKeyAddin     = NULL;
    LONG    lResult       = 0;
    DWORD   dwVal         = 0;
    char    szVal[MAX_PATH];    memset(szVal, 0, sizeof(szVal));
    WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));
	
	wchar_t *AppName = L"Outlook" ;
    try
    {
        _snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Microsoft\\Office\\%s\\Addins", AppName);
        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, KEY_ALL_ACCESS, &hKeyOutlook);
        if(ERROR_SUCCESS != lResult)     // get office/outlook key
		{
			_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Office\\%s\\Addins", AppName);
			lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, KEY_ALL_ACCESS, &hKeyOutlook);
			if(ERROR_SUCCESS != lResult)     // get office/outlook key
			{
				
				hr = E_UNEXPECTED;
				throw;
			}
		}
        lResult = RegCreateKeyEx( hKeyOutlook, L"OutlookAddin.outlookImpl.1",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyAddin,&dwDisposition);
        if(ERROR_SUCCESS != lResult)
		{
		
            hr = E_UNEXPECTED;
            throw;
        }
        dwVal = 0;
        RegSetValueEx(hKeyAddin, L"CommandLineSafe", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
        dwVal = 3;
        RegSetValueEx(hKeyAddin, L"LoadBehavior", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
        _snprintf_s(szVal, MAX_PATH, _TRUNCATE, "BasePEP outlook plugin");
        RegSetValueExA(hKeyAddin, "FriendlyName", 0, REG_SZ, (const BYTE*)szVal, 1+(DWORD)strlen(szVal));
        RegSetValueExA(hKeyAddin, "Description", 0, REG_SZ, (const BYTE*)szVal, 1+(DWORD)strlen(szVal));
    }
    catch(...)
    {
        if(NULL != hKeyAddin) RegCloseKey(hKeyAddin);
        if(NULL != hKeyOutlook) RegCloseKey(hKeyOutlook);

        hKeyAddin   = NULL;
        hKeyOutlook = NULL;
        return hr;
    }

  //  return hr;
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	wchar_t *AppName = L"Outlook" ;
	WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));
	_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Microsoft\\Office\\%s\\Addins\\OutlookAddin.outlookImpl.1", AppName);
	HRESULT lResult =   RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
	if(ERROR_SUCCESS != lResult)
	{
		_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Office\\%s\\Addins\\OutlookAddin.outlookImpl.1", AppName);
		lResult =   RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
	}
  
	return hr;
}

// DllInstall - Adds/Removes entries to the system registry per user
//              per machine.	
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = E_FAIL;
    static const wchar_t szUserSwitch[] = _T("user");

    if (pszCmdLine != NULL)
    {
    	if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
    	{
    		AtlSetPerUserRegistration(true);
    	}
    }

    if (bInstall)
    {	
    	hr = DllRegisterServer();
    	if (FAILED(hr))
    	{	
    		DllUnregisterServer();
    	}
    }
    else
    {
    	hr = DllUnregisterServer();
    }

    return hr;
}


