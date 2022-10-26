#include "stdafx.h"
#include <string>
#include <algorithm>
#include "utilities.h"
#include "APIHook.h"
#include "FilterRes.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <psapi.h>

#pragma warning(push)
#pragma warning(disable : 6387) 
#pragma warning(disable : 6011) 
#include <strsafe.h>
#pragma warning(pop)

#include <list>
#include "eframework/platform/policy_controller.hpp"
#include "eframework/platform/cesdk_loader.hpp"

#pragma comment(lib, "Psapi.lib")

#define BUFSIZE 512
#define EVALCACHE_TIMEOUT		100000
#define CONTROL_MANAGER_UUID L"b67546e2-6dc7-4d07-aa8a-e1647d29d4d7"

static BOOL g_bDetached = FALSE;//This flag determines if the "detach(DLLMain)" was called or not.
static std::list<EVALCACHE> g_listEvalCache;
CAPIHook g_ApiHook;
static BOOL g_bInit = FALSE;

extern nextlabs::recursion_control hook_control;

//	add by Ben, 2011/3/2, using platform api
nextlabs::cesdk_loader cesdkLoader;

static std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
		return szDir;
	}

	return L"";
}


BOOL GetFileNameFromHandle(HANDLE hFile, wstring& sFileName) 
{
  BOOL bSuccess = FALSE;
  TCHAR pszFilename[MAX_PATH+1];
  HANDLE hFileMap;

  // Get the file size.
  DWORD dwFileSizeHi = 0;
  DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 

  if( dwFileSizeLo == 0 && dwFileSizeHi == 0 )
  {
     return FALSE;
  }

  // Create a file mapping object.
  hFileMap = CreateFileMapping(hFile, 
                    NULL, 
                    PAGE_READONLY,
                    0, 
                    1,
                    NULL);

  if (hFileMap) 
  {
    // Create a file mapping to get the file name.
    void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

    if (pMem) 
    {
      if (GetMappedFileName (GetCurrentProcess(), 
                             pMem, 
                             pszFilename,
                             MAX_PATH)) 
      {

        // Translate path with device name to drive letters.
        TCHAR szTemp[BUFSIZE];
        szTemp[0] = '\0';

        if (GetLogicalDriveStrings(BUFSIZE-1, szTemp)) 
        {
          TCHAR szName[MAX_PATH];
          TCHAR szDrive[3] = TEXT(" :");
          BOOL bFound = FALSE;
          TCHAR* p = szTemp;

          do 
          {
            // Copy the drive letter to the template string
            *szDrive = *p;

            // Look up each device name
            if (QueryDosDevice(szDrive, szName, MAX_PATH))
            {
              size_t uNameLen = _tcslen(szName);

              if (uNameLen < MAX_PATH) 
              {
                bFound = _tcsnicmp(pszFilename, szName, 
                    uNameLen) == 0;

                if (bFound) 
                {
                  // Reconstruct pszFilename using szTempFile
                  // Replace device path with DOS path
                  TCHAR szTempFile[MAX_PATH];
                  StringCchPrintf(szTempFile,
                            MAX_PATH,
                            TEXT("%s%s"),
                            szDrive,
                            pszFilename+uNameLen);
                  StringCchCopyN(pszFilename, MAX_PATH+1, szTempFile, _tcslen(szTempFile));
                }
              }
            }

            // Go to the next NULL character.
            while (*p++);
          } while (!bFound && *p); // end of string
        }
      }
      bSuccess = TRUE;
      UnmapViewOfFile(pMem);
    } 

    CloseHandle(hFileMap);
  }

  sFileName = pszFilename;
  return(bSuccess);
}

BOOL GetCurrentProcessName(LPWSTR lpszProcessName, int nLen, HMODULE hMod)
{
	if(!lpszProcessName || nLen > 1024)
		return FALSE;
	
	if(hMod)
	{
		DWORD dwCount = GetModuleFileNameW(hMod, lpszProcessName, nLen);
		return dwCount > 0 ? TRUE: FALSE;
	}

	static wchar_t filename[1025] = {0};
	if( wcslen(filename) == 0)//only call GetModuleFileNameW at the first time. Kevin 20089-8-20
	{
		GetModuleFileNameW(hMod, filename, 1024);
	}
	
	if(wcslen(filename) > 0)
	{
		memcpy(lpszProcessName, filename, nLen * sizeof(wchar_t));
		return TRUE;
	}
	
	return FALSE;
}

BOOL GetCurrentProcessName2(LPSTR lpszProcessName, int nLen, HMODULE hMod)
{
	if(!lpszProcessName || nLen > 1024)
		return FALSE;

	if(hMod)
	{
		DWORD dwCount = GetModuleFileNameA(hMod, lpszProcessName, nLen);
		return dwCount > 0 ? TRUE: FALSE;
	}

	static char filename[1025] = {0};
	if( strlen(filename) == 0)//only call GetModuleFileNameW at the first time. Kevin 20089-8-20
	{
		GetModuleFileNameA(hMod, filename, 1024);
	}

	if(strlen(filename) > 0)
	{
		memcpy(lpszProcessName, filename, nLen * sizeof(char));
		return TRUE;
	}

	return FALSE;
}

BOOL IsProcess(LPCWSTR lpProcessName)
{
	if(!lpProcessName)
	{
		return FALSE;
	}

	wchar_t filename[1024] = {0};
	GetCurrentProcessName(filename, 1023, NULL);

	wchar_t* p = wcsrchr(filename, '\\');

	if(p && *(p + 1) != '\0')
	{
		if(_wcsicmp(p + 1, lpProcessName) == 0)
		return TRUE;
	}

	return FALSE;

}

void SetDetachFlag(BOOL bFlag)
{
	::EnterCriticalSection(&CcriticalMngr::s_csDetached);
	g_bDetached = bFlag;
	::LeaveCriticalSection(&CcriticalMngr::s_csDetached);
}

BOOL GetDetachFlag()
{
	::EnterCriticalSection(&CcriticalMngr::s_csDetached);
	BOOL bDetached =  g_bDetached;
	::LeaveCriticalSection(&CcriticalMngr::s_csDetached);
	return bDetached;
}

void PushEvalCache(LPCWSTR lpszAction, LPCWSTR lpszSrc, LPCWSTR lpszDest, BOOL bAllow)
{
	if(!lpszAction || !lpszSrc || !lpszDest)
	{
		return;
	}
	EVALCACHE cache;
	cache.strAction = lpszAction;
	cache.strSrc = lpszSrc;
	cache.strDest = lpszDest;
	cache.bAllow = bAllow;

	::EnterCriticalSection(&CcriticalMngr::s_csEvalCache);
	
	//Remove the items which were expired.
	std::list<EVALCACHE>::iterator itr;
	for(itr = g_listEvalCache.begin(); itr != g_listEvalCache.end(); )
	{
		if(GetTickCount() - (*itr).dwTimeEntry > EVALCACHE_TIMEOUT)
		{
			itr = g_listEvalCache.erase(itr);
			continue;
		}
		else
		{
			itr++;
		}
	}

	//check if the cache exists already. delete it if it exists.
	for(itr = g_listEvalCache.begin(); itr != g_listEvalCache.end(); itr++)
	{
		if(_wcsicmp((*itr).strAction.c_str(), lpszAction) == 0 && _wcsicmp((*itr).strSrc.c_str(), lpszSrc) == 0 && _wcsicmp((*itr).strDest.c_str(), lpszDest) == 0)
		{
			g_listEvalCache.erase(itr);
			break;
		}
	}
	cache.dwTimeEntry = GetTickCount();
	g_listEvalCache.push_back(cache);
	::LeaveCriticalSection(&CcriticalMngr::s_csEvalCache);
}

BOOL GetEvalCache(LPCWSTR lpszAction, LPCWSTR lpszSrc, LPCWSTR lpszDest, OUT BOOL& bAllow)
{
	if(!lpszAction || !lpszSrc || !lpszDest)
	{
		return FALSE;
	}

	BOOL bRet = FALSE;

	std::list<EVALCACHE>::iterator itr;
	::EnterCriticalSection(&CcriticalMngr::s_csEvalCache);
	
	for(itr = g_listEvalCache.begin(); itr != g_listEvalCache.end(); itr++)
	{
		if(_wcsicmp((*itr).strAction.c_str(), lpszAction) == 0 && _wcsicmp((*itr).strSrc.c_str(), lpszSrc) == 0 && _wcsicmp((*itr).strDest.c_str(), lpszDest) == 0)
		{
			if(GetTickCount() - (*itr).dwTimeEntry < EVALCACHE_TIMEOUT)
			{
				bAllow = (*itr).bAllow;
				bRet = TRUE;
			}
			else
			{
				g_listEvalCache.erase(itr);
			}
			break;
		}
	}

	::LeaveCriticalSection(&CcriticalMngr::s_csEvalCache);

	return bRet;
}

bool IsPolicyControllerUp(void)
{
	return nextlabs::policy_controller::is_up();
}

std::string MyWideCharToMultipleByte(const std::wstring & strValue)
{
        int nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0, NULL, NULL); 

	char* pBuf = new char[nLen + 1];
	if(!pBuf)
		return "";
	memset(pBuf, 0, nLen +1);
	nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen, NULL, NULL); 

	std::string strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

std::wstring MyMultipleByteToWideChar(const std::string & strValue)
{
        int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), NULL, 0 );
	DWORD dError = 0 ;
	if( nLen == 0 )
	{
		dError = ::GetLastError() ;
		if( dError == ERROR_INVALID_FLAGS )
		{
		  nLen = MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0 );
		}
	}
	wchar_t* pBuf = new wchar_t[nLen + 1];
	if(!pBuf)
		return L"";

	memset(pBuf, 0, (nLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	dError = ::GetLastError() ;
	if( dError == ERROR_INVALID_FLAGS )
	{
	  MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	}
	std::wstring strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

void exception_cb( NLEXCEPT_CBINFO* cb_info )
{
	hook_control.process_disable();

	if( cb_info != NULL )
	{
		wchar_t comp_root[MAX_PATH * 2] = {0}; // component root for HTTPE
		if( NLConfig::GetComponentInstallPath(L"Enterprise DLP\\Network Enforcer",comp_root,_countof(comp_root)) == true )
		{
			wcsncat_s(comp_root,_countof(comp_root),L"\\diags\\dumps",_TRUNCATE);
			wcsncpy_s(cb_info->dump_root,_countof(cb_info->dump_root),comp_root,_TRUNCATE);
			cb_info->use_dump_root = 1;
		}
		DPW ( (L"EXCEPTION 0x%X : PID %d TID %d : %hs [%d] \n",
			cb_info->code,GetCurrentProcessId(), GetCurrentThreadId(),
			cb_info->source_file,cb_info->source_line) );
	}

}/* exception_cb */

/*************************************************************
On windows 7, we will get the path like below for \\hz-srv01
\device\mup\hz-srv01.
It only happens on windows 7.
Here, we convert \device\mup\hz-srv01 to \\hz-srv01

*************************************************************/
void ConvertUNCPath(wstring& strPath)
{

	wstring strTemp = strPath;
	transform(strTemp.begin(), strTemp.end(), strTemp.begin(), tolower);

	const wstring strPattern(L"\\device\\mup");
	if(strTemp.find(strPattern) == 0)
	{
		strPath = strPath.replace(0, strPattern.length(), L"\\");
	}

}

void FTPE_Init()
{
	if(!g_bInit)
	{
		CCommonLog::Initialize();
		if(CFilterRes::IsSupportedProcess())
		{
			std::wstring wstrDir = GetCommonComponentsDir();
			if( FALSE == cesdkLoader.load(wstrDir.c_str()) )
			{
				CPolicy::m_bSDK = FALSE;
			}
			else
			{
				CPolicy::m_bSDK = TRUE;
			}

			g_ApiHook.StartHook();
		}
		g_bInit = TRUE;
	}
}

void FTPE_Finalize()
{
	if(g_bInit)
	{
		SetDetachFlag(TRUE);
		g_ApiHook.EndHook();
		if (cesdkLoader.is_loaded())
		{
			cesdkLoader.unload();
		}
	}
}
BOOL GetOSInfo(DWORD& dwMajor, DWORD& dwMinor)
{
	static DWORD sMajor = 0;
	static DWORD sMinor = 0;

	if(sMajor == 0 && sMinor == 0)
	{
		OSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx;

		// Try calling GetVersionEx using the OSVERSIONINFOEX structure.

		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
		if( !bOsVersionInfoEx )
		{
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
				return FALSE;
		}

		sMajor = osvi.dwMajorVersion;
		sMinor = osvi.dwMinorVersion;

		DPW((L"FTPE::OS version, Major: %d, Minor: %d", sMajor, sMinor));
	}
	

	//5,0 win2k, 5,1 winxp
	dwMajor = sMajor;
	dwMinor = sMinor;

	return TRUE;
	
}