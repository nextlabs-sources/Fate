#include <stdio.h>
#include "ApplicationScope.h"

#include <string>
#include <shlwapi.h>
#include <algorithm>

#pragma warning(push)
#pragma warning(disable: 4819)  // We won't fix code page issue in MADCHOOK header file, just ignore it here
#include "madCHook.h"
#pragma warning(pop)


namespace
{
    const WCHAR* kInjectionDriverName = L"NLInjection";

    const wchar_t* kExcludeProcesses = L"postgres.exe|pg_ctl.exe|postmaster.exe|Hotsync.exe|BbDevMgr.exe|RIMDeviceManager.exe|DesktopMgr.exe|clamwin.exe|clamscan.exe|vmtoolsd.exe|cepdpman.exe|edpmanager.exe|nlsce.exe|procexp.exe|taskhost.exe|cepdpman.exe|sandbox.exe|acrord32.exe|acrobat.exe|outlook.exe|DXSETUP.exe|officeclicktorun.exe|" \
        L"procexp64.exe|dbgview.exe|rdpclip.exe|msoia.exe|sihost.exe|taskhostw.exe|conhost.exe|mobsync.exe|RuntimeBroker.exe|sh.exe|mintty.exe|tm.exe|qq.exe|qqapp.exe|werfault.exe|cl.exe|make.exe|policystudio.exe|dwm.exe|MicrosoftEdge.exe|microsoftedgecp.exe|SearchUI.exe|ShellExperienceHost.exe|scclient.exe|Microsoft.Photos.exe|DllHost.exe|browser_broker.exe|";

	std::wstring GetConfigFilePath(WCHAR* pDllPath)
	{
		if (pDllPath == NULL)
		{
			return L"";
		}

		WCHAR configFilePath[MAX_PATH] = { 0 };
		wcscpy_s(configFilePath, MAX_PATH, pDllPath);

		WCHAR* pLastBackslash = wcsrchr(configFilePath, L'\\');
		if (pLastBackslash != NULL)
		{
			*pLastBackslash = L'\0';
			pLastBackslash = wcsrchr(configFilePath, L'\\');
			if (pLastBackslash != NULL)
			{
				*pLastBackslash = L'\0';
				pLastBackslash = wcsrchr(configFilePath, L'\\');
				if (pLastBackslash != NULL)
				{
					*pLastBackslash = L'\0';
				}
			}
		}

		wcscat_s(configFilePath, MAX_PATH, L"\\Policy Controller\\service\\injection.ini");
		return configFilePath;
	}

	BOOL isHookAll(const std::wstring& wstrConfigFilePath)
	{
		WCHAR szResult[MAX_PATH] = { 0 };
		GetPrivateProfileStringW(L"info", L"HookAll", NULL, szResult, MAX_PATH, wstrConfigFilePath.c_str());	
		if (szResult[0] == L'\0')
		{
			return TRUE;
		}

		return ( (0 == _wcsicmp(L"YES", szResult))  ||
			(0 == _wcsicmp(L"Y", szResult))    ||
			(0 == _wcsicmp(L"TRUE", szResult)) ||
			(0 == _wcsicmp(L"T", szResult)) );
	}

	std::wstring GetConfigInfo(BOOL bIsHookAll, const std::wstring& wstrConfigFilePath)
	{
		WCHAR szResult[4096] = { 0 };

		if (bIsHookAll)
		{
			GetPrivateProfileStringW(L"info", L"Exclude", NULL, szResult, 4096, wstrConfigFilePath.c_str());	
		}
		else
		{
			GetPrivateProfileStringW(L"info", L"Include", NULL, szResult, 4096, wstrConfigFilePath.c_str());	
		}

		if (szResult[0] == L'\0')
		{
			return L"";
		}

		std::transform(szResult, szResult + wcslen(szResult) + 1, szResult, tolower);
		return szResult;
	}

    void ReadLocalInjectConf(BOOL& bHookAll, std::wstring& strInclude, std::wstring& strExclude)
    {
		WCHAR szDllPath[MAX_PATH] = {0};

	#if defined(_WIN64)
		GetModuleFileName(GetModuleHandle(L"BasePEPPlugin"), szDllPath, MAX_PATH);
	#else
		GetModuleFileName(GetModuleHandle(L"BasePEPPlugin32"), szDllPath, MAX_PATH);
	#endif

		std::wstring wstrConfigFilePath = GetConfigFilePath(szDllPath);
		bHookAll = isHookAll(wstrConfigFilePath);
		std::wstring wstrConfigInfo = GetConfigInfo(bHookAll, wstrConfigFilePath);
		if (!wstrConfigInfo.empty())
		{
			if (bHookAll)
			{
				strExclude = wstrConfigInfo;
			}
			else
			{
				strInclude = wstrConfigInfo;
			}
		}
    }

    BOOL WINAPI InstallHook(BOOL bActivate, BOOL bServer)
    {
        OutputDebugStringW(L"try to install Hook");

        if (!bActivate)
            return TRUE;

        // try to read local injection configure file.
        BOOL bHookAll;
        std::wstring strIncludeProcesses;
        std::wstring strExcludeProcesses;
        ReadLocalInjectConf(bHookAll, strIncludeProcesses, strExcludeProcesses);

        LPCWSTR pInclude = NULL;
        LPCWSTR pExclude = NULL;
        if (bHookAll)
        {
            strExcludeProcesses = kExcludeProcesses + strExcludeProcesses;
            std::transform(strExcludeProcesses.begin(), strExcludeProcesses.end(), strExcludeProcesses.begin(), tolower);
            pExclude = strExcludeProcesses.c_str();
        }
        else
        {
            std::transform(strIncludeProcesses.begin(), strIncludeProcesses.end(), strIncludeProcesses.begin(), tolower);
            pInclude = strIncludeProcesses.c_str();
        }


        WCHAR szDllPath[1024] = {0};
        BOOL bResult = FALSE;

        WCHAR fullPath[MAX_PATH] = {0};
        WCHAR folder[MAX_PATH] = {0};


        //compose injection library path
#if defined(_WIN64)
        //On 64bit platform, load 64bit basepep.dll. Inject both 64bit basepep.dll
        //and 32bit basepep32.dll
        ::GetModuleFileNameW(GetModuleHandle(L"BasePEPPlugin"), fullPath, MAX_PATH);
        wchar_t* pname = ::PathFindFileNameW(fullPath);
        wcsncpy_s(folder, MAX_PATH, fullPath, pname - fullPath);

        wcsncat_s(szDllPath, folder, _TRUNCATE);
        wcsncat_s(szDllPath, L"basepep.dll", _TRUNCATE);

        if(bActivate)
        {
            bResult=InjectLibrary(kInjectionDriverName,
                szDllPath,
                ALL_SESSIONS,
                FALSE,
                pInclude,
                pExclude,
                NULL); 
        }
        else
        {
            bResult=UninjectLibraryW(kInjectionDriverName,
                szDllPath,
                ALL_SESSIONS,
                FALSE,
                pInclude,
                pExclude,
                NULL); 
        }

        if(bResult) {
            memset(szDllPath, 0, sizeof(szDllPath));

            wcsncat_s(szDllPath, folder, _TRUNCATE);
            wcsncat_s(szDllPath, L"basepep32.dll", _TRUNCATE);
            
            if(bActivate)
            {
                bResult=InjectLibrary(kInjectionDriverName,
                    szDllPath,
                    ALL_SESSIONS,
                    FALSE,
                    pInclude,
                    pExclude,
                    NULL); 
            }
            else
            {
                bResult=UninjectLibraryW(kInjectionDriverName,
                    szDllPath,
                    ALL_SESSIONS,
                    FALSE,
                    pInclude,
                    pExclude,
                    NULL); 
            }

            
        }
#else
        //On 32bit platform, load 32bit basepep.dll. Inject 32bit basepep32.dll  
        // path
        ::GetModuleFileNameW(GetModuleHandle(L"BasePEPPlugin32"), fullPath, MAX_PATH);
        wchar_t* pname = ::PathFindFileNameW(fullPath);
        wcsncpy_s(folder, MAX_PATH, fullPath, pname - fullPath);

        wcsncat_s(szDllPath, folder, _TRUNCATE);
        wcsncat_s(szDllPath, L"basepep32.dll", _TRUNCATE);

        if(bActivate)
        {
            bResult=InjectLibrary(kInjectionDriverName,
                szDllPath,
                ALL_SESSIONS,
                FALSE,
                pInclude,
                pExclude,
                NULL); 
        }
        else
        {
            bResult=UninjectLibraryW(kInjectionDriverName,
                szDllPath,
                ALL_SESSIONS,
                FALSE,
                pInclude,
                pExclude,
                NULL); 
        }
#endif

        if( !bResult ) {
            WCHAR buf[1024] = {0};
            swprintf_s(buf, L"Inject (%s) failed (%d,le %d)\n", szDllPath, bResult, GetLastError());
            OutputDebugStringW(buf);
        }

        return bResult;
    }
}

/****************************************************************************
 * Plug-in Entry Points
 ***************************************************************************/
extern "C" __declspec(dllexport) int PluginEntry( void** in_context )
{

  InstallHook(TRUE, TRUE);

  return 1;
}/* PluginEntry */

/*****************************************************************************
 * PluginUnload
 ****************************************************************************/
extern "C" __declspec(dllexport) int PluginUnload( void* in_context )
{
 
  InstallHook (FALSE, TRUE);

  return 1;
}/* PluginUnload */
