// dllmain.cpp : Defines the entry point for the DLL application.
#include "../include/dllmain.h"
#include <string>
#include <tlhelp32.h>
#include "celog.h"

#define print_string(s)  s?s:" "
#define print_long_string(s) s?s:L" "
#define print_non_string(s) s?*s:0 
#define CELOG_CUR_MODULE L"ADOBEPEP"
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_FORCEADOBEPEP_SRC_DLLMAIN_CPP

extern CRITICAL_SECTION g_showbubbleCriticalSection;

bool g_bIsReader = false;
int g_iReaderVersion = 0;

bool g_bIsOtherProcess = false;
DWORD g_ParentProcess = 0;

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


#if 1

// return 0 means it's owner, no parent
static DWORD GetParentProcessID(DWORD dwProcessID)
{
    DWORD dwParentID = 0;
    HANDLE hProcessSnap = NULL;
    //HANDLE hProcess = NULL; not used anymore
    PROCESSENTRY32 pe32;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        CELOG_LOG(CELOG_DEBUG, L"CreateToolhelp32Snapshot (of processes) failed.......\n");
        return(FALSE);
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);    // Must clean up the
        //   snapshot object!
        return(FALSE);
    }

    do
    {
        if (dwProcessID == pe32.th32ProcessID)
        {
            dwParentID = pe32.th32ParentProcessID;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return dwParentID;
}

// check if this process id is same process as the process name
static bool IsTheSameProcess(const DWORD& dwProcessID, const wchar_t* szProcessName)
{
	bool bSame = false;
	HANDLE hProcessSnap = NULL;
	//HANDLE hProcess = NULL;not used anymore
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		CELOG_LOG(CELOG_DEBUG, L"CreateToolhelp32Snapshot (of processes) failed.......\n");
		return (bSame);
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);    // Must clean up the
		//   snapshot object!
		return (bSame);
	}

	bool bFound = false;

	do
	{
		if (dwProcessID == pe32.th32ProcessID)
		{
			bFound = true;

			if (_wcsicmp(szProcessName, pe32.szExeFile) == 0)
			{
				bSame = true;
				break;
			}
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);

	if (!bFound)
	{
		return true;
	}

	return bSame;
}



#endif
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
					return FALSE;
				}

				strAdobeInstallDir += L"\\Plug_ins\\NlReaderPEP32.api";

				// If NlReaderPEP32.api does not exist, not to load AdobePEPTrm32.dll.
				if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strAdobeInstallDir.c_str()))
				{
					return FALSE;
				}
                {
                    DWORD dwProcID = GetCurrentProcessId();

                    DWORD dwParentID = GetParentProcessID(dwProcID);
                    if (dwParentID > 0 &&
                        IsTheSameProcess(dwParentID, L"acrord32.exe"))
                    {
                        // child process, we don't need to have this module, just free itself
                        return FALSE;
                    }
					if (dwParentID == 0)
					{
						return FALSE;
					}
					
                }

				g_bIsReader = true;
				
				if (boost::algorithm::iends_with(szModulePath, L"\\Adobe\\Reader 11.0\\Reader\\AcroRd32.exe"))
				{
					g_iReaderVersion = 11;
				}
				else if (boost::algorithm::iends_with(szModulePath, L"\\Adobe\\Reader 10.0\\Reader\\AcroRd32.exe"))
				{
					g_iReaderVersion = 10;
				}
			}
			else if (boost::algorithm::iends_with(std::wstring(szModulePath), L"acrobat.exe"))
			{
				if (GetAdobeInstallDir(strAdobeInstallDir, TRUE) != TRUE)
				{
					return FALSE;
				}

				strAdobeInstallDir += L"\\Plug_ins\\NlAcrobatPEP32.api";

				// If NlAcrobatPEP32.api does not exist, not to load AdobePEPTrm32.dll.
				if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strAdobeInstallDir.c_str()))
				{
					return FALSE;
				}
			}
            else
            {
				g_bIsOtherProcess = true;
			
				g_ParentProcess = GetParentProcessID(GetCurrentProcessId());

				HookForOtherProcess ( );

                return TRUE;
            }

			std::wstring strPath;
			if(GetWDEInstallDir(strPath))
			{
				strPath.append(L"\\bin\\basepep32.dll");
				hMod = LoadLibraryW(strPath.c_str());
			}

			Hook ( );

            InitializeCriticalSection(&g_showbubbleCriticalSection);
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
			if (g_bIsOtherProcess)
			{
				UnhookForOtherProcess ( );
				return TRUE;
			}

			typedef void (*DoUnLoad)();
			DoUnLoad fnDoUnLoad = NULL;
			if (hMod)
			{
				fnDoUnLoad = (DoUnLoad)GetProcAddress(hMod, "DoUnLoad");
				if(fnDoUnLoad) fnDoUnLoad();
			}		
			Unhook ( );
            DeleteCriticalSection(&g_showbubbleCriticalSection);
		}
		break;

	}
	return TRUE;
}

