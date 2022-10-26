// dllmain.cpp : Defines the entry point for the DLL application.
#include "../include/dllmain.h"
#include <string>
#include <boost/algorithm/string.hpp>

HMODULE hMod = NULL;
_Check_return_ static BOOL GetAdobeInstallDir(_Out_ std::wstring &OutPath, _In_ BOOL IsAcrobat)
{
    HKEY    hKey        = 0;
    LONG    lResult     = 0;
    
#ifdef _WIN64
    if (IsAcrobat == TRUE)
    {
	    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Adobe\\Adobe Acrobat", 0, KEY_READ, &hKey);
	}
	else
	{
	    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Adobe\\Acrobat Reader", 0, KEY_READ, &hKey);
	}
#else
    if (IsAcrobat == TRUE)
    {
	    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Adobe Acrobat", 0, KEY_READ, &hKey);
	}
	else
	{
	    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Acrobat Reader", 0, KEY_READ, &hKey);
	}
#endif
	if (ERROR_SUCCESS != lResult)
	{
	    return FALSE;
	}
	
	DWORD       size                = MAX_PATH;
	wchar_t     szData[MAX_PATH+1]  = {0};  
	int         i                   = 0;
    while (RegEnumKeyExW(hKey, i, szData, &size, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
		HKEY        hSubKey             = NULL;
    	wcsncat_s(szData, MAX_PATH, L"\\InstallPath", MAX_PATH-wcslen(szData));
        lResult = RegOpenKeyExW(hKey, szData, 0, KEY_READ, &hSubKey);
        if (ERROR_SUCCESS != lResult)
        {
    		size = MAX_PATH;
    		memset(szData, 0, sizeof(szData));
    		i++;
            continue;
        }
        
        DWORD dwType = REG_SZ;
		memset(szData, 0, sizeof(szData));
		size = MAX_PATH;
   		lResult = RegQueryValueExW(hSubKey, L"", 0, &dwType, (LPBYTE)szData, &size);
		RegCloseKey(hSubKey);
		
		if (ERROR_SUCCESS == lResult)
		{
			OutPath = szData;
		    
            RegCloseKey(hKey);
		    return TRUE;
		}
    		
		break;
    }
    
    RegCloseKey(hKey);
	return FALSE;
}

static BOOL GetWDEInstallDir ( std::wstring& InstallDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		return FALSE;
	}

	WCHAR WDEInstallDir[1024] = { 0 };
	DWORD BufferSize = sizeof( WDEInstallDir);

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"InstallDir", NULL, NULL, (LPBYTE)WDEInstallDir, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		return FALSE;
	}

	RegCloseKey ( hKey );

	InstallDir = WDEInstallDir;

	return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			wchar_t szModulePath[MAX_PATH] = {0};
			std::wstring strAdobeInstallDir;
			GetModuleFileName(NULL, szModulePath, MAX_PATH);
			if (boost::algorithm::iends_with(std::wstring(szModulePath), L"acrord32.exe"))
			{
				if (GetAdobeInstallDir(strAdobeInstallDir, FALSE) != TRUE)
				{
					break;
				}

				strAdobeInstallDir += L"\\Plug_ins\\CE_Reader32.api";

				// If CEAdobe32.api does not exist, not to load AdobePEPTrm32.dll.
				if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strAdobeInstallDir.c_str()))
				{
					break;
				}
			}
            else if(boost::algorithm::iends_with(std::wstring(szModulePath), L"acrobat.exe"))
            {
                if (GetAdobeInstallDir(strAdobeInstallDir, TRUE) != TRUE)
                {
                    break;
                }

                strAdobeInstallDir += L"\\Plug_ins\\CE_Acrobat32.api";

                // If CEAdobe32.api does not exist, not to load AdobePEPTrm32.dll.
                if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strAdobeInstallDir.c_str()))
                {
                    break;
                }
            }

			//try to load AdobePEPTrm
			std::wstring strPath;
			if(GetWDEInstallDir(strPath))
			{
				strPath.append(L"bin\\AdobePEPTrm32.dll");
				LoadLibraryW(strPath.c_str());
			}

			Hook( );


		}
		break;

	case DLL_THREAD_ATTACH:
		{
		}
		break;

	case DLL_THREAD_DETACH:
		{
		}
		break;

	case DLL_PROCESS_DETACH:
		{			
			Unhook( );
		}
		break;

	}
	return TRUE;
}

