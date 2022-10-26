#include <cassert>
#include <windows.h>
#include <string>
#include <algorithm>
#include "madCHook_helper.h"
#include "celog.h"
#include "ModuleScope.h"
#include "CEAdapter.h"
#include "CEAdapterConfig.h"

#define INJECT_DRIVER_NAME_W        L"NLInjection"
//#define INJECT_DRIVER_DESC          L"NextLabs code injection driver."

//---------------------------------------------------------------------------
// Static member declarations
//---------------------------------------------------------------------------
CModuleScope* CModuleScope::sm_pInstance = NULL;
std::map<std::string, APIDetours*> CModuleScope::hookedAPIDetours;

BOOL BJUnicodeToAnsi(LPWSTR pszwUniString, LPSTR pszAnsiBuff, size_t cchDest)
{
  assert( cchDest > 0 );
  if ( cchDest <= 0 )		// no room to shorten input string by null terminating it
  {
    return FALSE;
  }
  size_t l = lstrlenW(pszwUniString);
  WCHAR ch = 0;
  if ( l >= cchDest )	// won't fit in dest buffer
  {
    // Copy as much of the string as possible
    ch = pszwUniString[cchDest-1];	// save char
    pszwUniString[cchDest-1] = 0;	// replace with null terminator
  }

  WideToAnsi (pszwUniString, pszAnsiBuff);

  if ( ch )	// restore character replaced by 0
    pszwUniString[cchDest-1] = ch;
  
  return TRUE;
}/* BJUnicodeToAnsi */

BOOL BJAnsiToUnicode(LPCSTR  pszAnsiBuff, LPWSTR lpWideCharStr, size_t cchDest)
{
  assert( cchDest > 0 );
  if ( cchDest <= 0 )		// no room to shorten input string by null terminating it
  {
    return FALSE;
  }

  size_t l = lstrlenA(pszAnsiBuff);
  CHAR ch = 0;
  if ( l >= cchDest )	// won't fit in dest buffer
  {
    // Copy as much of the string as possible
    ch = pszAnsiBuff[cchDest-1];	// save char
    ((LPSTR) pszAnsiBuff)[cchDest-1] = 0;	// replace with null terminator
  }

  AnsiToWide (pszAnsiBuff, lpWideCharStr);

  if ( ch )	// restore character replaced by 0
    ((LPSTR) pszAnsiBuff)[cchDest-1] = ch;
  
  return TRUE;
}/* BJAnsiToUnicode */

//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
CModuleScope::CModuleScope(void)
{
}

//---------------------------------------------------------------------------
// Destructor 
//---------------------------------------------------------------------------
CModuleScope::~CModuleScope()
{
  std::map<std::string, APIDetours*>::iterator it;
  for(it=hookedAPIDetours.begin(); it != hookedAPIDetours.end(); ++it)
    delete it->second;
}/* CModuleScope::~CModuleScope */

//---------------------------------------------------------------------------
// GetInstance
//---------------------------------------------------------------------------
CModuleScope* CModuleScope::GetInstance(void)
{
  if (!sm_pInstance)
  {
    static CModuleScope instance;
    sm_pInstance = &instance;
  }

  return sm_pInstance;
}/* CModuleScope::GetInstance */

DWORD WINAPI InitializeHookManagementWrapper( LPVOID lpParam ) {
    std::vector<HookDetour *> *hlist = (std::vector<HookDetour *>*) lpParam;

    CModuleScope::GetInstance()->InitializeHookManagement(hlist);
    LoadHookingConfiguration_free(*hlist);
    delete hlist;

    return 0;
}

// Initialize hook 
void CModuleScope::InitializeHookManagement( std::vector<HookDetour*>* hlist )
{
  CollectHooks();

  std::vector<HookDetour*>::iterator it;

  /* Install in reverse order */
  for( it = hlist->begin() ; it != hlist->end() ; ++it )
  {
    HookDetour* detour;

    detour = *it;

    /* Inject library */

    /* Load hook library */
    HMODULE injectLib;

    TRACE(CELOG_INFO, L"InitializeHookManagement: LoadLibrary %s\n", detour->hook_libname);

    injectLib = LoadLibraryW(detour->hook_libname);

    if( injectLib == NULL )
    {
        TRACE(CELOG_ERR, L"InitializeHookManagement: LoadLibrary %s failed (%d)\n",
              detour->hook_libname, GetLastError());
        continue;
    }

    /* Inject the adapter library into the current process.  The current process
       handle must be retreived for using in madCHook's InjectLibrary().
     */
    HANDLE curr_proc_handle = INVALID_HANDLE_VALUE;

    curr_proc_handle = OpenProcess(PROCESS_ALL_ACCESS,FALSE,GetCurrentProcessId());

    if( curr_proc_handle == NULL )
    {
      TRACE(CELOG_ERR, L"InitializeHookManagement: OpenProcess failed (%d)\n", GetLastError());
      FreeLibrary(injectLib);
      continue;
    }

    TRACE(CELOG_INFO, L"InitializeHookManagement: InjectLibrary %s\n", detour->hook_libname);

    BOOL result = FALSE;
    result = InjectLibrary(detour->hook_libname, curr_proc_handle);

    TRACE(CELOG_INFO, L"InitializeHookManagement: InjectLibrary %s : result %d\n",
	  detour->hook_libname, result);

    CloseHandle(curr_proc_handle);

    if( detour->preDetourFunc != NULL && detour->postDetourFunc != NULL )
    {
      PVOID pPreDetourFunc=NULL;
      PVOID pPostDetourFunc=NULL;

      TRACE(0, _T("InitializeHookManagement %hs (legacy)\n"), detour->funcName);

      pPreDetourFunc = NULL;
      pPostDetourFunc = NULL;

      if( detour->preDetourFunc )
      {
	TRACE(0, _T("InitializeHookManagement pre : %s\n"), detour->preDetourFunc);

	pPreDetourFunc = GetProcAddress(injectLib,detour->preDetourFunc);
	TRACE(0, _T("pre : %hs @ 0x%x (le %d)\n"),
	      detour->preDetourFunc, pPreDetourFunc, GetLastError());
      }

      if( detour->postDetourFunc )
      {
	TRACE(0, _T("InitializeHookManagement: post: %hs\n"), detour->postDetourFunc);

	pPostDetourFunc = GetProcAddress(injectLib,detour->postDetourFunc);
	TRACE(0, _T("post: %hs @ 0x%x (le %d)\n"),
	      detour->postDetourFunc, pPostDetourFunc, GetLastError());
      }

      if( pPreDetourFunc == NULL && pPostDetourFunc == NULL )
      {
	TRACE(0, _T("There are no {pre,post} hooks.\n"));
	continue;
      }

      char generalDetourName[MAX_PATH] = {0};
      _snprintf_s(generalDetourName,_countof(generalDetourName), _TRUNCATE, 
		  "CE_BootstrapInjection_Detour_%s", detour->funcName);

      if( hookedAPIDetours.find(generalDetourName) == hookedAPIDetours.end() )
      {
	PVOID *realAPIPtr = GetRealAPI(detour->funcName);
	PVOID generalDetourPtr = GetGeneralHookedAPI(generalDetourName);

	if( realAPIPtr != NULL && generalDetourPtr != NULL )
	{
	  hookedAPIDetours[generalDetourName] = new APIDetours();

	  char target_libname_ansi[MAX_PATH] = {0};
	  BJUnicodeToAnsi(detour->target_libname,target_libname_ansi,MAX_PATH);

	  BOOL bResult = HookAPI(target_libname_ansi,detour->funcName,generalDetourPtr,realAPIPtr,0);

	  TRACE(0, _T("HookAPI %s %hs return %hs\n"), detour->target_libname,detour->funcName,
		(bResult)?"true":"false");
	}
      }
      /* The hook may be 'legacy' with support of pre/post or it may be layered.  When
	 the post function is NULL this indicates the latter, layered.
      */
      if( pPostDetourFunc != NULL )
      {
	hookedAPIDetours[generalDetourName]->AddOneDetour(detour->preDetourFunc,
							  pPreDetourFunc,
							  detour->postDetourFunc,
							  pPostDetourFunc);
      }
    }/* if pre/post non-NULL */

    TRACE(CELOG_INFO, L"InitializeHookManagement: Installing adapter at level %d\n", detour->level);

    /**********************************************************************
     * Notify hook library that it is being injected.  All hook libraries
     * are called - even for legacy support which will not have the entry
     * point.
     *********************************************************************/
    typedef int (*adapter_entry_t)(void);
    adapter_entry_t adapter_entry;

    adapter_entry = (adapter_entry_t)GetProcAddress(injectLib,"AdapterEntry");

    if( adapter_entry == NULL )
    {
      /* No entry point 'AdapterEntry' */
      TRACE(CELOG_ERR, L"InitializeHookManagement: AdapterEntry missing\n", adapter_entry);
      continue;
    }

    int rv;

    TRACE(CELOG_INFO, L"InitializeHookManagement: AdapterEntry @ 0x%x : calling\n", adapter_entry);

    rv = adapter_entry();

    TRACE(CELOG_INFO, L"InitializeHookManagement: AdapterEntry @ 0x%x : result %d\n", adapter_entry, rv);

    /* If the entry point fails, unload the library. */
    if( rv != 0 )
    {
      TRACE(CELOG_INFO, L"InitializeHookManagement: AdapterEntry @ 0x%x : FreeLibrary\n", adapter_entry);
      FreeLibrary(injectLib);
    }

    TRACE(CELOG_INFO, L"InitializeHookManagement: AdapterEntry @ 0x%x : complete\n", adapter_entry);
  }/* for it of hlist */

  FlushHooks();
}/* CModuleScope::InitializeHookManagement */

//////////////////////////////////////////////////////////////////////////
// Original API's that we intercept
HRESULT (__stdcall* NextCoCreateInstance)(REFCLSID rclsid,
					  LPUNKNOWN pUnkOuter,
					  DWORD dwClsContext,
					  REFIID riid,
					  LPVOID * ppv );

HANDLE (WINAPI *NextGetClipboardData)(UINT uFormat);
HANDLE (WINAPI *NextSetClipboardData)(UINT uFormat, HANDLE hMem);
HANDLE (WINAPI *NextCreateFileW)(LPCWSTR lpFileName, 
				 DWORD dwDesiredAccess, 
				 DWORD dwShareMode, 
				 LPSECURITY_ATTRIBUTES lpSecurityAttributes,
				 DWORD dwCreationDisposition,  
				 DWORD dwFlagsAndAttributes, 
				 HANDLE hTemplateFile);

BOOL (WINAPI *NextCreateProcessW)(LPCTSTR lpApplicationName,
				  LPTSTR lpCommandLine,
				  LPSECURITY_ATTRIBUTES lpProcessAttributes,
				  LPSECURITY_ATTRIBUTES lpThreadAttributes,
				  BOOL bInheritHandles,
				  DWORD dwCreationFlags,
				  LPVOID lpEnvironment,
				  LPCTSTR lpCurrentDirectory,
				  LPSTARTUPINFO lpStartupInfo,
				  LPPROCESS_INFORMATION lpProcessInformation );

//Get the address to the variable that will store the real WinAPI.
PVOID * CModuleScope::GetRealAPI(const char *funcName)
{
  if(strncmp(funcName, "CoCreateInstance",sizeof("CoCreateInstance")) == 0)
    return (PVOID *)&NextCoCreateInstance;
  else if(strncmp(funcName, "CreateFileW",sizeof("CreateFileW")) == 0)
    return (PVOID *)&NextCreateFileW;
  else if(strncmp(funcName,"GetClipboardData", sizeof("GetClipboardData")) ==0)
    return (PVOID *)&NextGetClipboardData;
  else if(strncmp(funcName,"SetClipboardData", sizeof("SetClipboardData")) ==0)
    return (PVOID *)&NextSetClipboardData;
  else if(strncmp(funcName,"CreateProcessW", sizeof("CreateProcessW")) ==0)
    return (PVOID *)&NextCreateProcessW;

  //TRACE(0, _T("Can't find real API for function %s\n"), funcName);
  return NULL;
}/* CModuleScope::GetRealAPI */

PVOID CModuleScope::GetGeneralHookedAPI(const char *funcName)
{
  if(strncmp(funcName, "CE_BootstrapInjection_Detour_CoCreateInstance",
	     sizeof("CE_BootstrapInjection_Detour_CoCreateInstance")) == 0)
    return (PVOID)CE_BootstrapInjection_Detour_CoCreateInstance;
  else if(strncmp(funcName, "CE_BootstrapInjection_Detour_CreateFileW",
		  sizeof("CE_BootstrapInjection_Detour_CreateFileW")) == 0)
    return (PVOID)CE_BootstrapInjection_Detour_CreateFileW;
  else if(strncmp(funcName,"CE_BootstrapInjection_Detour_GetClipboardData", 
		  sizeof("CE_BootstrapInjection_Detour_GetClipboardData")) ==0)
    return (PVOID)CE_BootstrapInjection_Detour_GetClipboardData;
  else if(strncmp(funcName,"CE_BootstrapInjection_Detour_SetClipboardData", 
		  sizeof("CE_BootstrapInjection_Detour_SetClipboardData")) ==0)
    return (PVOID)CE_BootstrapInjection_Detour_SetClipboardData;
  else if(strncmp(funcName,"CE_BootstrapInjection_Detour_CreateProcessW", 
		  sizeof("CE_BootstrapInjection_Detour_CreateProcessW")) ==0)
    return (PVOID)CE_BootstrapInjection_Detour_CreateProcessW;
  
  TRACE(0, _T("Can't find general hooked API for function %hs\n"), funcName);
  return NULL;
}/* CModuleScope::GetGeneralHookedAPI */

/* Compare predicate for ascending order */
static bool cmp_ascending( const HookDetour* a , const HookDetour* b )
{
  return (a->level < b->level);
}/* cmp_ascending */

// Called on DLL_PROCESS_ATTACH DLL notification
BOOL CModuleScope::ManageModuleEnlistment(void)
{
  std::vector<HookDetour*> *hlist = new std::vector<HookDetour*>();

  WCHAR pname[MAX_PATH] = {0};
  if( GetModuleFileNameW(NULL,pname,_countof(pname)) == 0 )
  {
    return FALSE;
  }

  /* Read injection configuration */
  if( LoadHookingConfiguration(pname,*hlist) == false )
  {
    TRACE(0,_T("The function LoadHookingConfiguration failed\n"));
    return FALSE;
  }

  BOOL bResult = FALSE;

  /* The hook libraries are sorted according to adapter install level. */
  std::sort(hlist->begin(),hlist->end(),cmp_ascending);

  if (hlist->size() > 0) {
      /*
       * This thread will perform the library loading and injection.  This can not be
       * done in DllMain, so we farm it out to a separate thread.  This thread will only
       * start executing once DllMain has finished running.
       */
      CreateThread(NULL,  // default security attributes
                   0,     // default stack size
                   InitializeHookManagementWrapper,
                   (LPVOID)hlist,
                   0,     // default creation flags
                   NULL);
      // Inform DllMain of success.  ceinjection will remain in the process
      bResult = TRUE;
  } else {
      // Delete the (empty) vector.  We will return failure and ceinjection will be removed from the process
      delete hlist;
  }


  return bResult;
}//CModuleScope::ManageModuleEnlistment

std::wstring CModuleScope::GetConfigFilePath(WCHAR* pDllPath)
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
		}
	}

	wcscat_s(configFilePath, MAX_PATH, L"\\service\\injection.ini");
	return configFilePath;
}

BOOL CModuleScope::isHookAll(const std::wstring& wstrConfigFilePath)
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

std::wstring CModuleScope::GetConfigInfo(BOOL bIsHookAll, const std::wstring& wstrConfigFilePath)
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

// Activate/Deactivate hooking engine
BOOL CModuleScope::InstallHookMethod( BOOL bActivate, 
				      BOOL bServer )
{
  const wchar_t* kExcludeProcesses = L"postgres.exe|pg_ctl.exe|postmaster.exe|Hotsync.exe|BbDevMgr.exe|RIMDeviceManager.exe|DesktopMgr.exe|clamwin.exe|clamscan.exe|vmtoolsd.exe|cepdpman.exe|edpmanager.exe|nlsce.exe|procexp.exe|taskhost.exe|cepdpman.exe|sandbox.exe|outlook.exe|DXSETUP.exe|officeclicktorun.exe|" \
		L"procexp64.exe|dbgview.exe|rdpclip.exe|msoia.exe|sihost.exe|taskhostw.exe|conhost.exe|mobsync.exe|RuntimeBroker.exe|sh.exe|mintty.exe|tm.exe|qq.exe|qqapp.exe|werfault.exe|cl.exe|make.exe|policystudio.exe|dwm.exe|MicrosoftEdge.exe|microsoftedgecp.exe|SearchUI.exe|ShellExperienceHost.exe|scclient.exe|Microsoft.Photos.exe|DllHost.exe|browser_broker.exe|";
  std::wstring strExcludeProcesses = kExcludeProcesses;

  WCHAR szDllPath[MAX_PATH] = {0};
  ULONG excludePIDs[] = {(ULONG)GetCurrentProcessId(),
			 0};
  BOOL bResult;

#if defined(_WIN64)
  //On 64bit platform, inject both 64bit ceInjection.dll and 
  //32bit ceInjection32.dll
  if( GetModuleFileName(GetModuleHandleA("ceInjection"),
			szDllPath,MAX_PATH) == 0 )
  {
    return FALSE;
  }

  std::wstring wstrConfigFilePath = GetConfigFilePath(szDllPath);

  BOOL bIsHookAll = isHookAll(wstrConfigFilePath);

  std::wstring wstrConfigInfo = GetConfigInfo(bIsHookAll, wstrConfigFilePath);

  const WCHAR* pstrInclude = NULL;
  const WCHAR* pstrExclude = NULL;

	  if (bIsHookAll)
	  {
	  strExcludeProcesses += wstrConfigInfo;
	  std::transform(strExcludeProcesses.begin(), strExcludeProcesses.end(), strExcludeProcesses.begin(), tolower);
	  pstrExclude = strExcludeProcesses.c_str();
	  }
	  else
	  {
		  pstrInclude = wstrConfigInfo.c_str();
	  }

  if (bActivate)
  {
    //first inject 64bit ceInjection.dll
    bResult=InjectLibraryW(INJECT_DRIVER_NAME_W,
			   szDllPath,
			   ALL_SESSIONS,
			   FALSE,
			   pstrInclude,
			   pstrExclude,
			   (bServer)?excludePIDs:NULL);
    wprintf(L"Inject Library 64bit %s\n", szDllPath);
  }
  else
  {
    //first uninject 64bit ceInjection.dll
    bResult = UninjectLibraryW(INJECT_DRIVER_NAME_W,
			       szDllPath,
			       ALL_SESSIONS,
			       FALSE,
			       pstrInclude,
			       pstrExclude,
			       (bServer)?excludePIDs:NULL);
  }
  if(bResult) {
    //First step succeed
    _wcslwr_s(szDllPath, MAX_PATH);
    WCHAR *pstr=wcsstr(szDllPath, L".dll");
    if(pstr == NULL) {
      return bResult;
    }
    szDllPath[pstr-szDllPath]=L'\0';
    wcsncat_s(szDllPath, MAX_PATH, L"32.dll", _TRUNCATE);
    wprintf(L"Inject Library 32bit %s\n", szDllPath);
    if(bActivate) {
      //Second inject 32bit ceInjection32.dll
      bResult=InjectLibraryW(INJECT_DRIVER_NAME_W,
			     szDllPath,
			     ALL_SESSIONS,
			     FALSE,
			     pstrInclude,
			     pstrExclude,
			     (bServer)?excludePIDs:NULL);
      if(bResult) {
	wprintf(L"Successfully Inject Library 32bit %s\n", szDllPath);

      } else {
	wprintf(L"failed to Inject Library 32bit %s\n", szDllPath);

      }
    }
    else {
      //second uninject 32bit ceInjection32.dll
      bResult = UninjectLibraryW(INJECT_DRIVER_NAME_W,
				 szDllPath,
				 ALL_SESSIONS,
				 FALSE,
				 pstrInclude,
				 pstrExclude,
				 (bServer)?excludePIDs:NULL);
    }
  }
#else
  //On 32bit platform, inject 32bit ceInjection32.dll
  if( GetModuleFileName(GetModuleHandleA("ceInjection32"),
			szDllPath,MAX_PATH) == 0 )
  {
    return FALSE;
  }

  std::wstring wstrConfigFilePath = GetConfigFilePath(szDllPath);

  BOOL bIsHookAll = isHookAll(wstrConfigFilePath);

  std::wstring wstrConfigInfo = GetConfigInfo(bIsHookAll, wstrConfigFilePath);

  const WCHAR* pstrInclude = NULL;
  const WCHAR* pstrExclude = NULL;

	  if (bIsHookAll)
	  {
	  strExcludeProcesses += wstrConfigInfo;
	  std::transform(strExcludeProcesses.begin(), strExcludeProcesses.end(), strExcludeProcesses.begin(), tolower);
	  pstrExclude = strExcludeProcesses.c_str();
	  }
	  else
	  {
		  pstrInclude = wstrConfigInfo.c_str();
	  }

  if (bActivate)
  {
    bResult=InjectLibraryW(INJECT_DRIVER_NAME_W,
			   szDllPath,
			   ALL_SESSIONS,
			   FALSE,
			   pstrInclude,
			   pstrExclude,
			   (bServer)?excludePIDs:NULL);
  }
  else
  {
    bResult = UninjectLibraryW(INJECT_DRIVER_NAME_W,
			       szDllPath,
			       ALL_SESSIONS,
			       FALSE,
			       pstrInclude,
			       pstrExclude,
			       (bServer)?excludePIDs:NULL);
  }
#endif
  return bResult;
}//CModuleScope::InstallHookMethod

/**********************************************************************************
 * Legacy implementation of pre/post hook functions.
 *********************************************************************************/

//The followings are the general detours of Win APIs
typedef bool (WINAPI *PreDetour_GetClipboardData)(UINT);
typedef bool (WINAPI *PostDetour_GetClipboardData)(UINT, HANDLE);
HANDLE WINAPI CModuleScope::CE_BootstrapInjection_Detour_GetClipboardData (UINT uFormat)
{
  std::map<std::string, APIDetours*>::iterator it;
  std::multimap<DetourPriority, DetourItem *>::iterator dit;
  bool bContinue=true;
  HANDLE realResult=NULL;
  APIDetours *apiDetours=NULL;

  //Get detours
  it=hookedAPIDetours.find("CE_BootstrapInjection_Detour_GetClipboardData");
  if(it != hookedAPIDetours.end()) 
  {
    apiDetours=it->second;

    //Call all pre detours
    for(dit=apiDetours->preDetourFuncs.begin();
	dit!= apiDetours->preDetourFuncs.end(); dit++) {
      bContinue=((PreDetour_GetClipboardData)(dit->second->func))(uFormat);
      if(!bContinue)
	break;
    }
  } 

  //If bContinie is true, do the followings
  if(bContinue){
    //Call the original Win API
    realResult=NextGetClipboardData(uFormat);
    
    //Call all post detours
    if(apiDetours && realResult) {
      for(dit=apiDetours->postDetourFuncs.begin();
	  dit!= apiDetours->postDetourFuncs.end(); ++dit) {
	bContinue=((PostDetour_GetClipboardData)(dit->second->func))(uFormat, realResult);
	if(!bContinue)
	  break;
      }
    }
  }

  return realResult;
}

typedef bool (__stdcall *PreDetour_CoCreateInstance)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);
typedef bool (__stdcall *PostDetour_CoCreateInstance)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*, HRESULT);
HRESULT __stdcall CModuleScope::CE_BootstrapInjection_Detour_CoCreateInstance(
	REFCLSID rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD dwClsContext,
	REFIID riid,
	LPVOID * ppv
)
{
  std::map<std::string, APIDetours*>::iterator it;
  std::multimap<DetourPriority, DetourItem *>::iterator dit;
  bool bContinue=true;
  HRESULT realResult=NULL;
  APIDetours *apiDetours=NULL;

  //Get detours
  it=hookedAPIDetours.find("CE_BootstrapInjection_Detour_CoCreateInstance");
  if(it != hookedAPIDetours.end()) 
  {
    apiDetours=it->second;

    //Call all pre detours
    for(dit=apiDetours->preDetourFuncs.begin();
	dit!= apiDetours->preDetourFuncs.end(); ++dit) {
      bContinue=((PreDetour_CoCreateInstance)(dit->second->func))(rclsid,
								  pUnkOuter,
								  dwClsContext,
								  riid,
								  ppv);
      if(!bContinue)
	break;
    }
  } 

  //If bContinie is true, do the followings
  if(bContinue){
    //Call the original Win API
    realResult = NextCoCreateInstance(
				      rclsid,
				      pUnkOuter,
				      dwClsContext,
				      riid,
				      ppv
				      );

    //Call all post detours
    if(apiDetours && realResult==S_OK) {
      for(dit=apiDetours->postDetourFuncs.begin();
	  dit!= apiDetours->postDetourFuncs.end(); ++dit) {
	bContinue=((PostDetour_CoCreateInstance)(dit->second->func))(rclsid,
								     pUnkOuter,
								     dwClsContext,
								     riid,
								     ppv, 
								     realResult);
	if(!bContinue)
	  break;
      }
    }
  }
  
  return realResult;
}

typedef bool (WINAPI *PreDetour_SetClipboardData)(UINT, HANDLE);
typedef bool (WINAPI *PostDetour_SetClipboardData)(UINT, HANDLE, HANDLE);
HANDLE WINAPI CModuleScope::CE_BootstrapInjection_Detour_SetClipboardData(UINT uFormat, 
																		HANDLE hMem)
{
  std::map<std::string, APIDetours*>::iterator it;
  std::multimap<DetourPriority, DetourItem *>::iterator dit;
  bool bContinue=true;
  HANDLE realResult=NULL;
  APIDetours *apiDetours=NULL;

  //Get detours
  it=hookedAPIDetours.find("CE_BootstrapInjection_Detour_SetClipboardData");
  if(it != hookedAPIDetours.end()) 
  {
    apiDetours=it->second;
    
    //Call all pre detours
    for(dit=apiDetours->preDetourFuncs.begin();
	dit!= apiDetours->preDetourFuncs.end(); ++dit) {
      bContinue=((PreDetour_SetClipboardData)(dit->second->func))(uFormat, hMem);
      if(!bContinue)
	break;
    }
  } 
  
  //If bContinie is true, do the followings
  if(bContinue){
    //Call the original Win API
    realResult= NextSetClipboardData (uFormat, hMem);
    //Call all post detours
    if(apiDetours && realResult) {
      for(dit=apiDetours->postDetourFuncs.begin();
	  dit!= apiDetours->postDetourFuncs.end(); ++dit) {
	bContinue=((PostDetour_SetClipboardData)(dit->second->func))(uFormat, hMem, realResult);
	if(!bContinue)
	  break;
      }
    }
  }
  return realResult;
}

typedef bool (__stdcall *PreDetour_CreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES,
						DWORD, DWORD, HANDLE);
typedef bool (__stdcall *PostDetour_CreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES,
						 DWORD, DWORD, HANDLE, HANDLE);
HANDLE __stdcall CModuleScope::CE_BootstrapInjection_Detour_CreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
)
{
  std::map<std::string, APIDetours*>::iterator it;
  std::multimap<DetourPriority, DetourItem *>::iterator dit;
  bool bContinue=true;
  HANDLE realResult = INVALID_HANDLE_VALUE;
  APIDetours *apiDetours=NULL;

  //TRACE(0, _T("Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
  //Get detours
  it=hookedAPIDetours.find("CE_BootstrapInjection_Detour_CreateFileW");
  if(it != hookedAPIDetours.end()) 
  {
    apiDetours=it->second;

    //Call all pre detours
    //TRACE(0, _T("1. Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
    for(dit=apiDetours->preDetourFuncs.begin();
	dit!= apiDetours->preDetourFuncs.end(); ++dit) {
      bContinue=((PreDetour_CreateFileW)(dit->second->func))(lpFileName,
							     dwDesiredAccess,
							     dwShareMode,
							     lpSecurityAttributes,
							     dwCreationDisposition,
							     dwFlagsAndAttributes,
							     hTemplateFile);
      if(!bContinue)
	break;
    }
  } 

  //If bContinie is true, do the followings
  if(bContinue){
    //Call the original Win API
    //TRACE(0, _T("1.1 Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
    realResult = NextCreateFileW(lpFileName,
				 dwDesiredAccess,
				 dwShareMode,
				 lpSecurityAttributes,
				 dwCreationDisposition,
				 dwFlagsAndAttributes,
				 hTemplateFile);

    //TRACE(0, _T("1.2 Inside CE_BootstrapInjection_Detour_CreateFileW %d\n"), realResult);
    //Call all post detours
    if(apiDetours && realResult!=INVALID_HANDLE_VALUE) {
      for(dit=apiDetours->postDetourFuncs.begin();
	  dit!= apiDetours->postDetourFuncs.end(); ++dit) {
	bContinue=((PostDetour_CreateFileW)(dit->second->func))(lpFileName,
								dwDesiredAccess,
								dwShareMode,
								lpSecurityAttributes,
								dwCreationDisposition,
								dwFlagsAndAttributes,
								hTemplateFile,
								realResult);
	if(!bContinue)
	  break;
      }
    }
  }
  
  //TRACE(0, _T("3. Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
  return realResult;
}

typedef bool (__stdcall *PreDetour_CreateProcessW)(LPCTSTR,
						   LPTSTR,
						   LPSECURITY_ATTRIBUTES,
						   LPSECURITY_ATTRIBUTES,
						   BOOL,
						   DWORD,
						   LPVOID,
						   LPCTSTR,
						   LPSTARTUPINFO,
						   LPPROCESS_INFORMATION);
typedef bool (__stdcall *PostDetour_CreateProcessW)(LPCTSTR,
						    LPTSTR,
						    LPSECURITY_ATTRIBUTES,
						    LPSECURITY_ATTRIBUTES,
						    BOOL,
						    DWORD,
						    LPVOID,
						    LPCTSTR,
						    LPSTARTUPINFO,
						    LPPROCESS_INFORMATION,
						    BOOL);
BOOL __stdcall CModuleScope::CE_BootstrapInjection_Detour_CreateProcessW(LPCTSTR lpApplicationName,
									 LPTSTR lpCommandLine,
									 LPSECURITY_ATTRIBUTES lpProcessAttributes,
									 LPSECURITY_ATTRIBUTES lpThreadAttributes,
									 BOOL bInheritHandles,
									 DWORD dwCreationFlags,
									 LPVOID lpEnvironment,
									 LPCTSTR lpCurrentDirectory,
									 LPSTARTUPINFO lpStartupInfo,
									 LPPROCESS_INFORMATION lpProcessInformation )
{
  std::map<std::string, APIDetours*>::iterator it;
  std::multimap<DetourPriority, DetourItem *>::iterator dit;
  bool bContinue=true;
  BOOL realResult=false;
  APIDetours *apiDetours=NULL;

  TRACE(0, _T("CreateProcess: %s %s\n"), 
  	lpApplicationName,
	lpCommandLine == NULL ? _T("") : lpCommandLine );

  //Get detours
  it=hookedAPIDetours.find("CE_BootstrapInjection_Detour_CreateProcessW");

  if(it != hookedAPIDetours.end()) 
  {
    apiDetours=it->second;

    //Call all pre detours
    //TRACE(0, _T("1. Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
    for(dit=apiDetours->preDetourFuncs.begin();
	dit!= apiDetours->preDetourFuncs.end(); ++dit) {
      bContinue=((PreDetour_CreateProcessW)(dit->second->func))(
								lpApplicationName,
								lpCommandLine,
								lpProcessAttributes,
								lpThreadAttributes,
								bInheritHandles,
								dwCreationFlags,
								lpEnvironment,
								lpCurrentDirectory,
								lpStartupInfo,
								lpProcessInformation);
      if(!bContinue)
	break;
    }
  } 

  //If bContinie is true, do the followings
  if(bContinue){
    realResult = NextCreateProcessW(lpApplicationName,
				    lpCommandLine,
				    lpProcessAttributes,
				    lpThreadAttributes,
				    bInheritHandles,
				    dwCreationFlags,
				    lpEnvironment,
				    lpCurrentDirectory,
				    lpStartupInfo,
				    lpProcessInformation);

    TRACE(0, _T("CreateProcess: pid %d\n"), GetCurrentProcessId());
    //Call all post detours
    if(apiDetours && realResult!=FALSE) {
      //TRACE(0, _T("2. Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
      for(dit=apiDetours->postDetourFuncs.begin();
	  dit!= apiDetours->postDetourFuncs.end(); ++dit) {
	bContinue=((PostDetour_CreateProcessW)(dit->second->func))(
								   lpApplicationName,
								   lpCommandLine,
								   lpProcessAttributes,
								   lpThreadAttributes,
								   bInheritHandles,
								   dwCreationFlags,
								   lpEnvironment,
								   lpCurrentDirectory,
								   lpStartupInfo,
								   lpProcessInformation,
								   realResult);
	if(!bContinue)
	  break;
      }
    }
  }
  return realResult;

}/* CModuleScope::CE_BootstrapInjection_Detour_CreateProcessW */
