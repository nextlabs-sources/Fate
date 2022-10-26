// iePEP.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include <Shlobj.h>
#include "iePEP.h"
#pragma warning( push )
#pragma warning( disable : 4819 )
#include "madCHook.h"
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning(pop)

#include "ActionHandler.h"
#include "celog.h"

#define CELOG_CUR_MODULE L"iePEP"
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_WDE_IEPEP_SRC_IEPEP_CPP


HINSTANCE g_hInstance = NULL;



extern WCHAR IETempPath[MAX_PATH];
extern int IETempPathLength;
extern BOOL StartStopHook(BOOL bStart);
extern CActionHandler* pActionHandler;

// Force Starting Browser Extension
static
void ForceBrowserExtension()
{
    LONG    lResult = 0;
    DWORD   dwSubKeys = 0;
    int     i = 0;

    lResult = RegQueryInfoKey(HKEY_USERS, NULL, NULL, NULL, &dwSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    for(i=0; i<(int)dwSubKeys; i++)
    {
        wchar_t wzSubKeyValue[1024]; memset(wzSubKeyValue, 0, sizeof(wzSubKeyValue));
        wchar_t wzKeyValueData[MAX_PATH];
        DWORD   dwSize = 1023;
        FILETIME ftLastWriteTime;
        HKEY    hKey = NULL;
        DWORD   dwType = REG_SZ;
        if( ERROR_SUCCESS == RegEnumKeyEx(HKEY_USERS, i, wzSubKeyValue, &dwSize, NULL, NULL, NULL, &ftLastWriteTime) )
        {
           wcsncat_s(wzSubKeyValue, 1024, L"\\Software\\Microsoft\\Internet Explorer\\Main", _TRUNCATE);
            memset(wzKeyValueData, 0, sizeof(wzKeyValueData));
            dwSize = MAX_PATH -1;
            if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_USERS, wzSubKeyValue, 0, KEY_ALL_ACCESS, &hKey) && NULL!=hKey)
            {
                if(ERROR_SUCCESS == RegQueryValueEx(hKey, L"Enable Browser Extensions", 0, &dwType, (LPBYTE)wzKeyValueData, &dwSize) )
                {
                    if(0 != _wcsicmp(wzKeyValueData, L"yes"))
                        RegSetValueEx(hKey, L"Enable Browser Extensions", 0, REG_SZ, (const BYTE *)L"yes", 8);
                }
                RegCloseKey(hKey); hKey=NULL;
            }
        }
    }
    CELOG_LOG(CELOG_DUMP, L"Local variables are: lResult=0X%08X, dwSubKeys=%lu, i=%d \n", lResult,dwSubKeys,i );

}

VOID KeepIePepEnabled()
{
    ::RegDeleteKeyW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Ext\\Settings\\{FB159F40-0C40-4480-9A72-71C1D07606B7}");
}
//////////////////////////////////////////////////////////////////////////

/*
*\brief: somehow, ie run some low level program will pop-up a warning window with IE protected mode is on and uac is on.
*		 refer to: http://msdn.microsoft.com/en-us/library/Bb250462#upm_cfgpm
*/
static BOOL Is64bitSystem()
{
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);

	CELOG_LOG(CELOG_DUMP, L"Local variables are: si=%p \n", &si );

	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||    
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	} 

}

static void SetInjectToolRunLevel()
{
	// read OS version.
	wchar_t* pKeyPath = L"SOFTWARE\\Microsoft\\Internet Explorer\\Low Rights\\ElevationPolicy\\{FB159F4A-0C4A-448A-9A7A-71C1D07606BA}";
	BOOL b64Bit=Is64bitSystem();
	HKEY hDestKey;
	long nRet = 0;
	nRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE,pKeyPath,0,KEY_WOW64_64KEY|KEY_QUERY_VALUE, &hDestKey);
	if(ERROR_SUCCESS == nRet)	
	{
		RegCloseKey(hDestKey);
	    CELOG_LOG(CELOG_DUMP, L"Local variables are: pKeyPath=%ls, nRet=0X%08X \n", pKeyPath,nRet);
		return ;
	}
	wchar_t szAppPath[512]={0};
	// read app path
	wchar_t* pWDEPath=L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer";
	HKEY hKey = NULL;
	nRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE,pWDEPath,0,KEY_WOW64_64KEY|KEY_QUERY_VALUE,&hKey);
	if(nRet != ERROR_SUCCESS)	return;
	DWORD dwPathLen=512;
	nRet = RegQueryValueExW(hKey,L"InstallDir",NULL,NULL,(LPBYTE)szAppPath,&dwPathLen);
	RegCloseKey(hKey);
	if(nRet != ERROR_SUCCESS)	return ;
	if(!boost::algorithm::iends_with(szAppPath,L"\\bin"))
	{
		wcsncat_s(szAppPath,512,L"bin", _TRUNCATE);
	}
	const wchar_t* szAppName=L"InjectTool32.exe";
	DWORD dwPolicy = 3;
	if(b64Bit)
	{
		nRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
			pKeyPath,
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_WOW64_32KEY|KEY_ALL_ACCESS, NULL, &hDestKey, NULL);
		if(ERROR_SUCCESS != nRet)
		{
			DP((L"RegCreateKeyEx Fail, last error is [%d]\n",nRet));
			RegCloseKey(hDestKey);
		}
		else
		{
			nRet = RegSetValueEx(hDestKey,L"AppName",NULL,REG_SZ,(LPBYTE)szAppName,static_cast<DWORD> (wcslen(szAppName)*2+2));
			nRet = RegSetValueEx(hDestKey,L"AppPath",NULL,REG_SZ,(LPBYTE)szAppPath,static_cast<DWORD> (wcslen(szAppPath)*2+2));
			nRet = RegSetValueEx(hDestKey,L"Policy",0,REG_DWORD,(LPBYTE)&dwPolicy,sizeof(DWORD));
			RegCloseKey(hDestKey);
		}
	}

	nRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		pKeyPath,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_WOW64_64KEY|KEY_ALL_ACCESS, NULL, &hDestKey, NULL);
	if(ERROR_SUCCESS != nRet)
	{
		DP((L"RegCreateKeyEx Fail, last error is [%d]\n",nRet));
		RegCloseKey(hDestKey);
	}
	else
	{
		nRet = RegSetValueEx(hDestKey,L"AppName",NULL,REG_SZ,(LPBYTE)szAppName,static_cast<DWORD> (wcslen(szAppName)*2+2));
		nRet = RegSetValueEx(hDestKey,L"AppPath",NULL,REG_SZ,(LPBYTE)szAppPath,static_cast<DWORD>(wcslen(szAppPath)*2+2));
		nRet = RegSetValueEx(hDestKey,L"Policy",0,REG_DWORD,(LPBYTE)&dwPolicy,sizeof(DWORD));
		RegCloseKey(hDestKey);
	}
    CELOG_LOG(CELOG_DUMP, L"Local variables are: pKeyPath=%ls, nRet=0X%08X, szAppPath=%ls \n", pKeyPath,nRet,szAppPath);

}
//////////////////////////////////////////////////////////////////////////

class CiePEPModule : public CAtlDllModuleT< CiePEPModule >
{
public :
	DECLARE_LIBID(LIBID_iePEPLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_IEPEP, "{58FF7086-E186-4B38-88DB-29CB5FC59EDE}")
};

CiePEPModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    hInstance;
    
    
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        ::DisableThreadLibraryCalls((HMODULE)hInstance);

        
        pActionHandler = CActionHandler::GetInstance ( );
        if (!pActionHandler)
        {
            return FALSE;
        }

        g_hInstance = hInstance;
       

		SHGetSpecialFolderPath ( NULL, IETempPath, CSIDL_INTERNET_CACHE, FALSE );
		IETempPathLength = static_cast<int>(wcslen ( IETempPath ));
		
		

		InitializeMadCHook();
        StartStopHook(TRUE);
        DP((L"OK 1\n"));
        break;

    case DLL_PROCESS_DETACH:
		
		//To be symmetric with PROCESS_ATTACH, no need to stop hooking
		//for explorer.exe
		StartStopHook(FALSE);
		FinalizeMadCHook();
		

        ForceBrowserExtension();
        KeepIePepEnabled();
        break;

    default:
        break;
    }

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
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: rclsid=%p, riid=%p, ppv=%p \n", &rclsid, &riid, ppv);

    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    
    HRESULT hr = _AtlModule.DllRegisterServer();
    if(SUCCEEDED(hr))
    {
        // write the IE obj
        HKEY hDestKey;
        
        long nRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\{FB159F40-0C40-4480-9A72-71C1D07606B7}"),
            0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hDestKey, NULL);
        if(ERROR_SUCCESS == nRet)
        {
            // ignore Explorer
            int v = 1;
            RegSetValueExW(hDestKey, L"NoExplorer", 0, REG_DWORD, (byte*)&v, sizeof(int));
            RegCloseKey(hDestKey);
        }
        

        // Enable Third-Part Browser Extension
        ForceBrowserExtension();
        
    }
	SetInjectToolRunLevel();
	CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X \n", hr );
    return hr;
}

// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
    HRESULT hr = _AtlModule.DllUnregisterServer();
    if(SUCCEEDED(hr))
        RegDeleteKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\{FB159F40-0C40-4480-9A72-71C1D07606B7}");
	CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X \n", hr );
 
    return hr;
}

