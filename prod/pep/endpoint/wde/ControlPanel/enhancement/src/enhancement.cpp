// enhancement.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "enhancement.h"
#include "dllmain.h"
#include <Sddl.h>

BOOL GetAccessPermissionsForLUAServer(SECURITY_DESCRIPTOR **ppSD)
{
	// Local call permissions to IU, SY
	LPWSTR lpszSDDL = L"O:BAG:BAD:(A;;0x3;;;IU)(A;;0x3;;;SY)";
	SECURITY_DESCRIPTOR *pSD;
	*ppSD = NULL;

	if (ConvertStringSecurityDescriptorToSecurityDescriptorW(lpszSDDL, SDDL_REVISION_1, (PSECURITY_DESCRIPTOR *)&pSD, NULL))
	{
		*ppSD = pSD;
		return TRUE;
	}

	return FALSE;
}

BOOL SetAccessPermissions(HKEY hkey, PSECURITY_DESCRIPTOR pSD)
{
	BOOL bResult = FALSE;
	DWORD dwLen = GetSecurityDescriptorLength(pSD);
	LONG lResult;
	lResult = RegSetValueExA(hkey, 
		"AccessPermission",
		0,
		REG_BINARY,
		(BYTE*)pSD,
		dwLen);
	if (lResult != ERROR_SUCCESS) goto done;
	bResult = TRUE;
done:
	return bResult;
}

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

	SECURITY_DESCRIPTOR * pSD = NULL;
	if (GetAccessPermissionsForLUAServer(&pSD))
	{
		// Main means to get path using registry
		HKEY hKey = (HKEY)INVALID_HANDLE_VALUE;

		const wchar_t * szKey = L"SOFTWARE\\Classes\\AppID\\{2A9EAF67-12C2-4655-A2DD-E7D988A3AE59}";
		DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,szKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwDisposition) == ERROR_SUCCESS)
		{
			SetAccessPermissions(hKey,pSD);
		}
	}

	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
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


