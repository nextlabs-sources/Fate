// dllmain.cpp : Implementation of DllMain.
#include "stdafx.h"
#include "resource.h"
#include "dllmain.h"


CNxtOfficePEPModule _AtlModule;

// try to delete it
HINSTANCE g_hInstance=NULL;

CRITICAL_SECTION showbubbleCriticalSection;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch ( dwReason )
	{
	case DLL_PROCESS_ATTACH:
		_AtlModule.OnDllAttach(hInstance);			
		break;
	case DLL_PROCESS_DETACH:	
		_AtlModule.OnDllDetach();			
		break;
	default:
		break;
	}
	return _AtlModule.DllMain(dwReason, lpReserved); 
}




// COM Component Requeired

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


HRESULT RegisterDll(_In_ LPCWSTR AppName, _In_ LPCWSTR strAddInName)
{
	NLPRINT_DEBUGLOG(L" The Parameters are: AppName=%ls, strAddInName=%ls \n", print_long_string(AppName), print_long_string(strAddInName));
#define VERSION_PRODUCT_WDE "Desktop Enforcer for Microsoft Windows"
	HRESULT hr = S_OK;
	DWORD   dwDisposition = 0;
	HKEY    hKeyOffice = NULL;
	HKEY    hKeyAddin = NULL;
	LONG    lResult = 0;
	DWORD   dwVal = 0;
	char    szVal[MAX_PATH + 1];    memset(szVal, 0, sizeof(szVal));
	WCHAR   wzKeyName[MAX_PATH + 1]; memset(wzKeyName, 0, sizeof(wzKeyName));

	try
	{
		swprintf_s(wzKeyName, MAX_PATH, L"SOFTWARE\\Microsoft\\Office\\%s\\Addins", AppName);
		lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, wzKeyName, 0, KEY_ALL_ACCESS, &hKeyOffice);
		if (ERROR_SUCCESS != lResult)     // get office/outlook key
		{
			// here we need to create it at the beginning
			lResult = RegCreateKeyExW(HKEY_LOCAL_MACHINE, wzKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyOffice, &dwDisposition);
			if (ERROR_SUCCESS != lResult)
			{
				hr = E_UNEXPECTED;
				throw;
			}
		}
		lResult = RegCreateKeyEx(hKeyOffice, strAddInName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyAddin, &dwDisposition);
		if (ERROR_SUCCESS != lResult)
		{
			hr = E_UNEXPECTED;
			throw;
		}
		dwVal = 0;
		RegSetValueEx(hKeyAddin, L"CommandLineSafe", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
		dwVal = 3;
		RegSetValueEx(hKeyAddin, L"LoadBehavior", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
		sprintf_s(szVal, MAX_PATH, VERSION_PRODUCT_WDE/*"Compliant Enterprise Office PEP"*/);
		RegSetValueExA(hKeyAddin, "FriendlyName", 0, REG_SZ, (const BYTE*)szVal, 1 + (DWORD)strlen(szVal));
		RegSetValueExA(hKeyAddin, "Description", 0, REG_SZ, (const BYTE*)szVal, 1 + (DWORD)strlen(szVal));

		if (NULL != hKeyAddin) RegCloseKey(hKeyAddin);
		if (NULL != hKeyOffice) RegCloseKey(hKeyOffice);
	}
	catch (...)
	{
		if (NULL != hKeyAddin) RegCloseKey(hKeyAddin);
		if (NULL != hKeyOffice) RegCloseKey(hKeyOffice);

		hKeyAddin = NULL;
		hKeyOffice = NULL;

		NLPRINT_DEBUGLOG(L"Local variables are: hr=0X%08X, dwDisposition=%lu, hKeyOffice=%p, hKeyAddin=%p, lResult=%ld, dwVal=%lu, szVal=%hs, wzKeyName=%ls \n", hr, dwDisposition, hKeyOffice, hKeyAddin, lResult, dwVal, print_string(szVal), print_long_string(wzKeyName));
		return hr;
	}
	NLPRINT_DEBUGLOG(L"Local variables are: hr=0X%08X, dwDisposition=%lu, hKeyOffice=%p, hKeyAddin=%p, lResult=%ld, dwVal=%lu, szVal=%hs, wzKeyName=%ls \n", hr, dwDisposition, hKeyOffice, hKeyAddin, lResult, dwVal, print_string(szVal), print_long_string(wzKeyName));
	return hr;
}

HRESULT UnRegisterDll(LPCWSTR AppName, LPCWSTR strAddInName)
{
	WCHAR   wzKeyName[MAX_PATH + 1]; memset(wzKeyName, 0, sizeof(wzKeyName));
	swprintf_s(wzKeyName, MAX_PATH, L"SOFTWARE\\Microsoft\\Office\\%s\\Addins\\%s", AppName, strAddInName);
	RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
	return S_OK;
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	HRESULT hr = _AtlModule.DllRegisterServer();
	RegisterDll(L"Word", L"NLOfficePEP.1");
	RegisterDll(L"Excel", L"NLOfficePEP.1");
	RegisterDll(L"PowerPoint", L"NLOfficePEP.1");
	NLPRINT_DEBUGLOG(L"Local variables are: hr=0X%08X \n", hr);
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	UnRegisterDll(L"Word", L"NLOfficePEP.1");
	UnRegisterDll(L"Excel", L"NLOfficePEP.1");
	UnRegisterDll(L"PowerPoint", L"NLOfficePEP.1");
	NLPRINT_DEBUGLOG(L"Local variables are: hr=0X%08X \n", hr);
	return hr;
}

// DllInstall - Adds/Removes entries to the system registry per user
//              per machine.	
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	NLPRINT_DEBUGLOG(L" The Parameters are: bInstall=%ls, pszCmdLine=%ls \n", bInstall ? L"TRUE" : L"FALSE", print_long_string(pszCmdLine));
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
	NLPRINT_DEBUGLOG(L"Local variables are: hr=0X%08X, szUserSwitch=%ls \n", hr, print_long_string(szUserSwitch));
	return hr;
}

