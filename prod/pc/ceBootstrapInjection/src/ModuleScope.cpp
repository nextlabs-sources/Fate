/*
* ModuleScope.cpp 
* Author: Helen Friedland
* All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
* Redwood City CA, Ownership remains with Blue Jungle Inc, 
* All rights reserved worldwide. 
*/

#include <Windows.h>
#define SECURITY_WIN32
#include "ModuleScope.h"
#include "brain.h"
#include <winbase.h>
#include "madCHook - static.h"

//---------------------------------------------------------------------------
//
// Some external variables or functions
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//
// Static member declarations
//
//---------------------------------------------------------------------------

CModuleScope* CModuleScope::sm_pInstance      = NULL;
HANDLE CModuleScope::hookedAPIDetourMutex =NULL;
std::map<std::wstring, APIDetours*> CModuleScope::hookedAPIDetours;

//////////////////////////////////////////////////////////////////////////
// Original API's that we intercept
HRESULT (__stdcall* NextCoCreateInstance)(
	REFCLSID rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD dwClsContext,
	REFIID riid,
	LPVOID * ppv
	);

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

//////////////////////////////////////////////////////////////////////////
// Some utility functions
namespace {

//---------------------------------------------------------------------------
// GetProcessHostFullName
//
// Return the path and the name of the current process
//---------------------------------------------------------------------------
static BOOL GetProcessHostFullName(WCHAR* pszFullFileName, ULONG len )
{
  DWORD dwResult = 0;

  ::ZeroMemory((PBYTE)pszFullFileName,len);

  // Read the image full path
  dwResult = ::GetModuleFileNameW(NULL,pszFullFileName,len);

  return (dwResult != 0);
}
//---------------------------------------------------------------------------
// GetProcessHostName
//
// Return the name of the current process
//---------------------------------------------------------------------------
static BOOL GetProcessHostName(WCHAR* pszFullFileName, ULONG len )
{
  BOOL  bResult;

  bResult = GetProcessHostFullName(pszFullFileName,len);
  return bResult;
}
BOOL BJUnicodeToAnsi(LPWSTR pszwUniString, LPSTR pszAnsiBuff, int cchDest)
{
	// Call MadCodeHook
	// Avoids problems with calling WideCharToMultiByte from hook

	if ( cchDest <= 0 )		// no room to shorten input string by null terminating it
		return (!pszwUniString && !(*pszwUniString));	// return TRUE only if source string is empty

	int l = lstrlenW(pszwUniString);
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
}

BOOL BJAnsiToUnicode (LPCSTR  pszAnsiBuff, LPWSTR lpWideCharStr, int cchDest)
{
	// Call MadCodeHook
	// Avoids problems with calling MultiByteToWideChar from hook

	if ( cchDest <= 0 )		// no room to shorten input string by null terminating it
		return (!pszAnsiBuff && !(*pszAnsiBuff));	// return TRUE only if source string is empty

	int l = lstrlenA(pszAnsiBuff);
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
}

/* The followings are temperal hooking configuration functions
 * Loading the dynamic injection service configuration information for
 * this process.
 *
 * LoadHookingConfiguration_free() must be called on a successful call to
 * this function.
 *
 * Return false when an error occurs.  A return of true indicates there
 * are [0,n] hooks.  It is possible there are no hooks to install.
 */
static bool LoadHookingConfiguration(const WCHAR *m_szProcessName, //The current process name 
			      const DWORD m_dwProcessId, //The current process id
			      bool &bHookProcess, //output:if true, hooking into this process
			      WCHAR **injectLibName, //output: if not NULL, the lib to be injected into process 
			      int &numHookAPI, //output: the number of Win APIs to be hooked
			      HookDetour **pHookDetours) //ouput: if not null, point to the array of hook detours
{

  /* By default process should not be hooked.  A dynamic injection configuration
     for this processes is seached for.  If it does not exist, the process is not
     hooked.
  */
  bHookProcess = false; 

  /* Generate the path for injection configuration of this process.  The
     registry containts the Policy Controller root (InstallDir) which
     containts the application configuration below.

     [InstallDir]/service/injection/[exe_name].ini
   */
  WCHAR config_file[MAX_PATH];
  LONG rstatus;
  HKEY hKey = NULL; 

  rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
			  TEXT("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller"),
			  0,
			  KEY_QUERY_VALUE,
			  &hKey);

  if( rstatus != ERROR_SUCCESS )
  {
    TRACE(0, _T("LoadHookingConfiguration: %d no reg.\n"), rstatus);
    return true;
  }

  WCHAR ce_root[MAX_PATH];                 /* InstallDir */
  DWORD ce_root_size = sizeof(ce_root);

  rstatus = RegQueryValueExW(hKey,               /* handle to reg */      
			     TEXT("PolicyControllerDir"), /* key to read */
			     NULL,               /* reserved */
			     NULL,               /* type */
			     (LPBYTE)ce_root,    /* InstallDir */
			     &ce_root_size       /* size (in/out) */
			     );

  RegCloseKey(hKey);

  if( rstatus != ERROR_SUCCESS )
  {
    TRACE(0, _T("LoadHookingConfiguration: No configuration path.\n"));
    return true;
  }

  const WCHAR* p;

  p = wcsrchr(m_szProcessName,'\\');

  /* The image name must have been found and have at least on character. */
  if( p == NULL || wcslen(p) <= 1 )
  {
    return 0;
  }

  p++;  /* move past '\\' */

  /* Generate path to configuration file */
  _snwprintf(config_file,MAX_PATH,
  	     TEXT("%s\\service\\injection\\%s.ini"),
  	     ce_root, p);

  FILE* fp;

  fp = _wfopen(config_file,TEXT("r"));

  if( fp == NULL )
  {
    /* There are no hooks to install */
    TRACE(0, _T("LoadHookingConfiguration: No configuration.\n"));
    return true;
  }

  /* The SDK located in OCE must be loaded as a result of the configuration file
     and the enforcement via UCCP.  OCE will be one directory back from the Policy
     Controller ([Program Files]/Compliant Enterprise/Office Communicator Enforcer/)
   */
  WCHAR dll_path[MAX_PATH];
  _snwprintf(dll_path,MAX_PATH,
	     TEXT("%s\\..\\Office Communicator Enforcer\\bin"),ce_root);
  SetDllDirectoryW(dll_path);

  /* Line length is a possible full path (MAX_PATH) with library symbols (512)*/
  char temp[MAX_PATH + 512];

  /* Determine how many hooks there are by counting lines */
  numHookAPI = 0;
  while( fgets(temp,sizeof(temp),fp) != NULL )
  {
    numHookAPI++;
  }

  rewind(fp);  /* back to start of file */

  *pHookDetours= new HookDetour[numHookAPI];

  numHookAPI = 0;

  /* Read file line by line and parse hook configuration for each target
     library.

     Params from the line in the follwing format:
       [hook.dll],[target.dll],[target function],[prehook],[posthook]

     Lines beginning with '#' or ';' are considered comments and ignored.
  */
  while( fgets(temp,sizeof(temp),fp) != NULL )
  {
    if( strlen(temp) <= 1 || temp[0] == '#' || temp[0] == ';' )
    {
      TRACE(0, _T("LoadHookingConfiguration: ignore empty/comment line\n"));
      continue; /* ignore line - empty or comment */
    }

    /* trim key assignment '[name]=' */
    char* p;

    p = (char*)strstr(temp,"=");

    if( p == NULL || strlen(p) <= 0 )
    {
      TRACE(0, _T("LoadHookingConfiguration: ignore line\n"));
      continue; /* ignore line */
    }

    p++;  /* pass '=' delimiter */

    /* Pre-hook or post-hook function may be NULL to indicate there is no
       hook to install.
    */
    char* token = NULL;          /* current token */
    char* next_token = NULL;     /* parse context for strtok_s */
    char* delim = ",\n";         /* delimiter - '\n' is used to avoid manual trim */

    char* hookName = NULL;       /* hook parameters */
    char* dllName = NULL;
    char* funcName = NULL;
    char* preDetourFunc = NULL;
    char* postDetourFunc = NULL;

    next_token = (char*)p;       /* set context for strtok_s */

    hookName = strtok_s(next_token,delim,&next_token);         /* hook library */
    dllName = strtok_s(next_token,delim,&next_token);          /* target library */
    funcName = strtok_s(next_token,delim,&next_token);         /* target function */
    preDetourFunc = strtok_s(next_token,delim,&next_token);    /* pre-hook function */
    postDetourFunc = strtok_s(next_token,delim,&next_token);   /* post-hook function */

    /* all hook parameters must be present */
    if( hookName == NULL || dllName == NULL || funcName == NULL ||
	preDetourFunc == NULL || postDetourFunc == NULL )
    {
      TRACE(0, _T("LoadHookingConfiguration: malformed hook configuration\n"));
      continue;  /* malformed entry */
    }

    HookDetour& hook = (*pHookDetours)[numHookAPI];          /* reference to current hook */
    int len = 0;

    memset(&hook,0x00,sizeof(HookDetour));

    len = strlen(hookName) + 1;                              /* hook library */
    *injectLibName= new WCHAR[ len ];
    BJAnsiToUnicode(hookName,*injectLibName,len);

    len = strlen(dllName) + 1;                               /* target library */
    hook.dllName = new WCHAR[ len ];
    BJAnsiToUnicode(dllName,hook.dllName,len);

    len = strlen(funcName) + 1;                              /* target function */       
    hook.funcName = new WCHAR[ len ];
    BJAnsiToUnicode(funcName,hook.funcName,len);

    if( _stricmp(preDetourFunc,"NULL") != 0 )                /* pre-hook function */
    {
      len = strlen(preDetourFunc) + 1;
      hook.preDetourFunc = new WCHAR[ len ];
      BJAnsiToUnicode(preDetourFunc,hook.preDetourFunc,len);
    }

    if( _stricmp(postDetourFunc,"NULL") != 0 )               /* post-hook function */
    {
      len = strlen(postDetourFunc) + 1;
      hook.postDetourFunc = new WCHAR[ len ];
      BJAnsiToUnicode(postDetourFunc,hook.postDetourFunc,len);
    }

    hook.priority = HIGH;

    numHookAPI++;
  }

  TRACE(0, _T("LoadHookingConfiguration: %d hook(s) read\n"), numHookAPI);

  fclose(fp);

  /* process should be hooked based on configuration file read above */
  bHookProcess = true;

  return true;
}/* LoadHookingConfiguration */

static void LoadHookingConfiguration_free(WCHAR *injectLibName, HookDetour *pHookDetours, 
								   int numHookDetours)
{
  if(injectLibName)
    delete injectLibName;

  if(pHookDetours) {
    for(int i=0; i<numHookDetours;i++) {
      delete [] pHookDetours[i].dllName;
      delete [] pHookDetours[i].funcName;
      if(pHookDetours[i].preDetourFunc)
	delete [] pHookDetours[i].preDetourFunc;
      if(pHookDetours[i].postDetourFunc)
	delete [] pHookDetours[i].postDetourFunc;
    }

    delete [] pHookDetours;
  }
}
}
//---------------------------------------------------------------------------
//
// Constructor
//
//---------------------------------------------------------------------------
CModuleScope::CModuleScope(void)
{
    //
    // Get the name of the current process
    //
    GetProcessHostName(m_szProcessName,sizeof(m_szProcessName));

    //
    // and its process id
    //
    m_dwProcessId = ::GetCurrentProcessId();

    hookedAPIDetourMutex=CreateMutex(NULL, false, NULL);
}

//---------------------------------------------------------------------------
//
// Destructor 
//
//---------------------------------------------------------------------------
CModuleScope::~CModuleScope()
{
	std::map<std::wstring, APIDetours*>::iterator it;
	WaitForSingleObject(hookedAPIDetourMutex, INFINITE);
	for(it=hookedAPIDetours.begin(); it != hookedAPIDetours.end(); it++)
		delete it->second;
	ReleaseMutex(hookedAPIDetourMutex);

	CloseHandle(hookedAPIDetourMutex);
}

//---------------------------------------------------------------------------
// GetInstance
//
// Implements the "double-checking" locking pattern combined with 
// Scott Meyers single instance
// For more details see - 
// 1. "Modern C++ Design" by Andrei Alexandrescu - 6.9 Living in a 
//     Multithreaded World
// 2. "More Effective C++" by Scott Meyers - Item 26
//---------------------------------------------------------------------------
CModuleScope* CModuleScope::GetInstance(void)
{
  if (!sm_pInstance)
  {
    static CModuleScope instance;
    sm_pInstance = &instance;
  }

  return sm_pInstance;
}

//
// Initialize hook 
bool CModuleScope::InitializeHookManagement(HMODULE injectLib, int numHookAPI, HookDetour *pHookDetours)
{   
	//
	// and now we can set-up some custom hooks
	//
	TRACE(0, _T("InitializeHookManagement 1\n"));

	// Hook the API's
	CollectHooks();

	int i;
	PVOID pPreDetourFunc=NULL;
	PVOID pPostDetourFunc=NULL;
	WCHAR generalDetourName[MAX_PATH];
	CHAR ansiBuf[MAX_PATH];
	CHAR ansiBuf1[MAX_PATH];
	for (i = 0; i < numHookAPI; i++) {
		TRACE(0, _T("InitializeHookManagement 2 i=%d func=%s\n"), i, pHookDetours[i].funcName);
		pPreDetourFunc=NULL;
		pPostDetourFunc=NULL;
		if(pHookDetours[i].preDetourFunc) {
			TRACE(0, _T("InitializeHookManagement 2.1 i=%d pre-func=%s\n"), i, pHookDetours[i].preDetourFunc);
			BJUnicodeToAnsi (pHookDetours[i].preDetourFunc, ansiBuf, MAX_PATH);
			pPreDetourFunc=::GetProcAddress(injectLib, ansiBuf);
			if(pPreDetourFunc==NULL)
				TRACE(0, _T("%s is null\n"), pHookDetours[i].preDetourFunc);
		}
			
		if(pHookDetours[i].postDetourFunc) {
			TRACE(0, _T("InitializeHookManagement 2.2 i=%d post-func=%s\n"), i, pHookDetours[i].postDetourFunc);
			BJUnicodeToAnsi (pHookDetours[i].postDetourFunc, ansiBuf, MAX_PATH);
			pPostDetourFunc=::GetProcAddress(injectLib, ansiBuf);
			if(pPostDetourFunc==NULL)
				TRACE(0, _T("%s is null\n"), pHookDetours[i].postDetourFunc);
		}
	
		if(pPreDetourFunc==NULL && pPostDetourFunc==NULL) 			
			continue;

		TRACE(0, _T("InitializeHookManagement 3\n"));
		_snwprintf_s(generalDetourName, MAX_PATH, _TRUNCATE, L"CE_BootstrapInjection_Detour_%s", pHookDetours[i].funcName);
		WaitForSingleObject(hookedAPIDetourMutex, INFINITE);
		if(hookedAPIDetours.find(generalDetourName) == hookedAPIDetours.end()) {
			PVOID *realAPIPtr=GetRealAPI(pHookDetours[i].funcName);
			PVOID generalDetourPtr=GetGeneralHookedAPI(generalDetourName);
			if(realAPIPtr == NULL || generalDetourPtr == NULL) {
				//This API doesn't have general detour, skip it for now
				ReleaseMutex(hookedAPIDetourMutex);
				continue;				
			}
			hookedAPIDetours[generalDetourName]=new APIDetours(pHookDetours[i].dllName,
															pHookDetours[i].funcName,
															realAPIPtr);
			BJUnicodeToAnsi (pHookDetours[i].dllName, ansiBuf, MAX_PATH);
			BJUnicodeToAnsi (pHookDetours[i].funcName, ansiBuf1, MAX_PATH);
			BOOL bResult = HookAPI (ansiBuf, ansiBuf1, generalDetourPtr, realAPIPtr, 0);
			TRACE(0, _T("HookAPI %s %s return %s\n"), pHookDetours[i].dllName,pHookDetours[i].funcName, (bResult)?_T("true"):_T("false"));
		}
		ReleaseMutex(hookedAPIDetourMutex);
		TRACE(0, _T("InitializeHookManagement 4\n"));
		hookedAPIDetours[generalDetourName]->AddOneDetour(pHookDetours[i].preDetourFunc,
															pPreDetourFunc,
															pHookDetours[i].priority,
															pHookDetours[i].postDetourFunc,
															pPostDetourFunc);
		TRACE(0, _T("InitializeHookManagement 5\n"));
	}

	FlushHooks();
	return true;
}

//Get the address to the variable that will store the real WinAPI.
PVOID * CModuleScope::GetRealAPI(WCHAR *funcName)
{
	if(wcsncmp(funcName, L"CoCreateInstance",wcslen(L"CoCreateInstance")) == 0)
		return (PVOID *)&NextCoCreateInstance;
	else if(wcsncmp(funcName, L"CreateFileW",wcslen(L"CreateFileW")) == 0)
		return (PVOID *)&NextCreateFileW;
	else if(wcsncmp(funcName,L"GetClipboardData", wcslen(L"GetClipboardData")) ==0)
		return (PVOID *)&NextGetClipboardData;
	else if(wcsncmp(funcName,L"SetClipboardData", wcslen(L"SetClipboardData")) ==0)
		return (PVOID *)&NextSetClipboardData;
	else if(wcsncmp(funcName,L"CreateProcessW", wcslen(L"CreateProcessW")) ==0)
		return (PVOID *)&NextCreateProcessW;

	TRACE(0, _T("Can't find real API for function %s\n"), funcName);
	return NULL;
}

PVOID CModuleScope::GetGeneralHookedAPI(WCHAR *funcName)
{
	if(wcsncmp(funcName, L"CE_BootstrapInjection_Detour_CoCreateInstance",
		wcslen(L"CE_BootstrapInjection_Detour_CoCreateInstance")) == 0)
		return (PVOID)CE_BootstrapInjection_Detour_CoCreateInstance;
	else if(wcsncmp(funcName, L"CE_BootstrapInjection_Detour_CreateFileW",
		wcslen(L"CE_BootstrapInjection_Detour_CreateFileW")) == 0)
		return (PVOID)CE_BootstrapInjection_Detour_CreateFileW;
	else if(wcsncmp(funcName,L"CE_BootstrapInjection_Detour_GetClipboardData", 
		wcslen(L"CE_BootstrapInjection_Detour_GetClipboardData")) ==0)
		return (PVOID)CE_BootstrapInjection_Detour_GetClipboardData;
	else if(wcsncmp(funcName,L"CE_BootstrapInjection_Detour_SetClipboardData", 
		wcslen(L"CE_BootstrapInjection_Detour_SetClipboardData")) ==0)
		return (PVOID)CE_BootstrapInjection_Detour_SetClipboardData;
	else if(wcsncmp(funcName,L"CE_BootstrapInjection_Detour_CreateProcessW", 
		wcslen(L"CE_BootstrapInjection_Detour_CreateProcessW")) ==0)
		return (PVOID)CE_BootstrapInjection_Detour_CreateProcessW;

	TRACE(0, _T("Can't find general hooked API for function %s\n"), funcName);
	return NULL;
}

//
// Called on DLL_PROCESS_ATTACH DLL notification
//
BOOL CModuleScope::ManageModuleEnlistment()
{
    BOOL bResult = FALSE;

    WCHAR *injectLibName=NULL;
    HookDetour *pHookDetours=NULL;
    int numHookAPI;
    bool bHookProcess=false;

    //Loading the dynamic injection service configuration information for
    //this process
    if(LoadHookingConfiguration(m_szProcessName, //The current process name 
				m_dwProcessId, //The current process id
				bHookProcess, //output:if true, hooking into this process
				&injectLibName, //output: if not NULL, the lib to be injected into process 
				numHookAPI, //output: the number of Win APIs to be hooked
				&pHookDetours //ouput: if not null, point to the array of hook detours
				) == false) {
      TRACE(0,_T("The function LoadHookingConfiguration failed\n"));
      return FALSE;
    }
    
    if(bHookProcess == false) {//don't hook into this process		
      return FALSE;
    } else 
      TRACE(0, _T("Hook into the process %s\n"), m_szProcessName);

    if(injectLibName==NULL) {
      //we just inject ceBootstrapInjection.dll into process;
      //This allows hooking APIs later.
      return TRUE;
    } else {
      //Loading injecting library
      HMODULE injectLib=NULL;
      TCHAR buffer[MAX_PATH];
      
      _stprintf(buffer, _T("%s"), injectLibName);
      
      injectLib = ::LoadLibrary(buffer);
      if(injectLib == NULL) {
	TRACE(0, _T("Failed to load library %s: %d\n"), 
	      injectLibName, GetLastError());
	bResult=FALSE;
      } else {
	//Now, let's check if we need to hook and detour some APIs
	if(numHookAPI == 0 || pHookDetours == NULL) 
	  bResult=TRUE; //nothing to be hooked and detoured 
	else {
	  if(InitializeHookManagement(injectLib, numHookAPI, pHookDetours))
	    bResult=TRUE;
	}
      }
      LoadHookingConfiguration_free(injectLibName, pHookDetours, numHookAPI);
      
      /* Now that hooks have been configured.  Call into the injection library
	 to inform it that hook entry is done.
      */
      typedef bool (*HookEntry_fn)(int,WCHAR*);
      
      HookEntry_fn hook_entry;
      
      hook_entry = (HookEntry_fn)GetProcAddress(injectLib,"HookEntry");
      
      TRACE(0, _T("Calling HookEntry @ %x\n"), hook_entry);
      
      if( hook_entry != NULL )
	{
	  bool result;
	  
	  result = hook_entry((int)0,(WCHAR*)NULL);
	}
    }
    return bResult;
}//CModuleScope::ManageModuleEnlistment

//
// Called on DLL_PROCESS_DETACH notification
//
void CModuleScope::ManageModuleDetachment()
{
}

//
// Activate/Deactivate hooking engine
//
BOOL CModuleScope::InstallHookMethod( BOOL bActivate, 
				      BOOL bServer )
{
    BOOL bResult;
    DWORD dwFlags = ALL_SESSIONS;
	
    // Don't hook Control Module process
    if ( bServer )
    {
      dwFlags &= ~CURRENT_PROCESS;
    }

    TCHAR szDllPath[MAX_PATH];
    GetModuleFileName( GetModuleHandleA("ceBootstrapInjection"), szDllPath, MAX_PATH );

    CHAR szAnsiPath[MAX_PATH];
    BJUnicodeToAnsi (szDllPath, szAnsiPath, MAX_PATH);

    if (bActivate)
    {
        bResult = InjectLibrary ((HANDLE)dwFlags, szDllPath);
    }
    else
    {
      bResult = UninjectLibrary ((HANDLE)dwFlags, szDllPath);
    }
    return bResult;
}//CModuleScope::InstallHookMethod

//
// Accessor method
//
WCHAR* CModuleScope::GetProcessName() const
{
    return const_cast<WCHAR*>(m_szProcessName);
}

//
// Accessor method
//
DWORD CModuleScope::GetProcessId() const
{
    return m_dwProcessId;
}

//The followings are the general detours of Win APIs
typedef bool (WINAPI *PreDetour_GetClipboardData)(UINT);
typedef bool (WINAPI *PostDetour_GetClipboardData)(UINT, HANDLE);
HANDLE WINAPI CModuleScope::CE_BootstrapInjection_Detour_GetClipboardData (UINT uFormat)
{
	std::map<std::wstring, APIDetours*>::iterator it;
	std::multimap<DetourPriority, DetourItem *>::iterator dit;
	bool bContinue=true;
	HANDLE realResult=NULL;
	APIDetours *apiDetours=NULL;

	//Get detours
	WaitForSingleObject(hookedAPIDetourMutex, INFINITE);
	it=hookedAPIDetours.find(L"CE_BootstrapInjection_Detour_GetClipboardData");
	if(it == hookedAPIDetours.end()) 
		ReleaseMutex(hookedAPIDetourMutex);
	else {
		apiDetours=it->second;
		ReleaseMutex(hookedAPIDetourMutex);

		//Call all pre detours
		WaitForSingleObject(apiDetours->preDetourMutex, INFINITE);
		for(dit=apiDetours->preDetourFuncs.begin();
			dit!= apiDetours->preDetourFuncs.end(); dit++) {
			bContinue=((PreDetour_GetClipboardData)(dit->second->func))(uFormat);
			if(!bContinue)
				break;
		}
		ReleaseMutex(apiDetours->preDetourMutex);
	} 

	//If bContinie is true, do the followings
	if(bContinue){
		//Call the original Win API
		realResult=NextGetClipboardData(uFormat);

		//Call all post detours
		if(apiDetours && realResult) {
			WaitForSingleObject(apiDetours->postDetourMutex, INFINITE);
			for(dit=apiDetours->postDetourFuncs.begin();
				dit!= apiDetours->postDetourFuncs.end(); dit++) {
				bContinue=((PostDetour_GetClipboardData)(dit->second->func))(uFormat, realResult);
				if(!bContinue)
					break;
			}
			ReleaseMutex(apiDetours->postDetourMutex);
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
	std::map<std::wstring, APIDetours*>::iterator it;
	std::multimap<DetourPriority, DetourItem *>::iterator dit;
	bool bContinue=true;
	HRESULT realResult=NULL;
	APIDetours *apiDetours=NULL;

	//Get detours
	WaitForSingleObject(hookedAPIDetourMutex, INFINITE);
	it=hookedAPIDetours.find(L"CE_BootstrapInjection_Detour_CoCreateInstance");
	if(it == hookedAPIDetours.end()) 
		ReleaseMutex(hookedAPIDetourMutex);
	else {
		apiDetours=it->second;
		ReleaseMutex(hookedAPIDetourMutex);

		//Call all pre detours
		WaitForSingleObject(apiDetours->preDetourMutex, INFINITE);
		for(dit=apiDetours->preDetourFuncs.begin();
			dit!= apiDetours->preDetourFuncs.end(); dit++) {
			bContinue=((PreDetour_CoCreateInstance)(dit->second->func))(rclsid,
										pUnkOuter,
										dwClsContext,
										riid,
										ppv);
			if(!bContinue)
				break;
		}
		ReleaseMutex(apiDetours->preDetourMutex);
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
			WaitForSingleObject(apiDetours->postDetourMutex, INFINITE);
			for(dit=apiDetours->postDetourFuncs.begin();
				dit!= apiDetours->postDetourFuncs.end(); dit++) {
				bContinue=((PostDetour_CoCreateInstance)(dit->second->func))(rclsid,
																pUnkOuter,
																dwClsContext,
																riid,
																ppv, 
																realResult);
				if(!bContinue)
					break;
			}
			ReleaseMutex(apiDetours->postDetourMutex);
		}
	}

	return realResult;
}

typedef bool (WINAPI *PreDetour_SetClipboardData)(UINT, HANDLE);
typedef bool (WINAPI *PostDetour_SetClipboardData)(UINT, HANDLE, HANDLE);
HANDLE WINAPI CModuleScope::CE_BootstrapInjection_Detour_SetClipboardData(UINT uFormat, 
																		HANDLE hMem)
{
	std::map<std::wstring, APIDetours*>::iterator it;
	std::multimap<DetourPriority, DetourItem *>::iterator dit;
	bool bContinue=true;
	HANDLE realResult=NULL;
	APIDetours *apiDetours=NULL;

	//Get detours
	WaitForSingleObject(hookedAPIDetourMutex, INFINITE);
	it=hookedAPIDetours.find(L"CE_BootstrapInjection_Detour_SetClipboardData");
	if(it == hookedAPIDetours.end()) 
		ReleaseMutex(hookedAPIDetourMutex);
	else {
		apiDetours=it->second;
		ReleaseMutex(hookedAPIDetourMutex);

		//Call all pre detours
		WaitForSingleObject(apiDetours->preDetourMutex, INFINITE);
		for(dit=apiDetours->preDetourFuncs.begin();
			dit!= apiDetours->preDetourFuncs.end(); dit++) {
			bContinue=((PreDetour_SetClipboardData)(dit->second->func))(uFormat, hMem);
			if(!bContinue)
				break;
		}
		ReleaseMutex(apiDetours->preDetourMutex);
	} 

	//If bContinie is true, do the followings
	if(bContinue){
		//Call the original Win API
		realResult= NextSetClipboardData (uFormat, hMem);
		//Call all post detours
		if(apiDetours && realResult) {
			WaitForSingleObject(apiDetours->postDetourMutex, INFINITE);
			for(dit=apiDetours->postDetourFuncs.begin();
				dit!= apiDetours->postDetourFuncs.end(); dit++) {
				bContinue=((PostDetour_SetClipboardData)(dit->second->func))(uFormat, hMem, realResult);
				if(!bContinue)
					break;
			}
			ReleaseMutex(apiDetours->postDetourMutex);
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
	std::map<std::wstring, APIDetours*>::iterator it;
	std::multimap<DetourPriority, DetourItem *>::iterator dit;
	bool bContinue=true;
	HANDLE realResult = NULL;
	APIDetours *apiDetours=NULL;

	//TRACE(0, _T("Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
	//Get detours
	WaitForSingleObject(hookedAPIDetourMutex, INFINITE);
	it=hookedAPIDetours.find(L"CE_BootstrapInjection_Detour_CreateFileW");
	if(it == hookedAPIDetours.end()) 
		ReleaseMutex(hookedAPIDetourMutex);
	else {
		apiDetours=it->second;
		ReleaseMutex(hookedAPIDetourMutex);

		//Call all pre detours
		WaitForSingleObject(apiDetours->preDetourMutex, INFINITE);
		//TRACE(0, _T("1. Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
		for(dit=apiDetours->preDetourFuncs.begin();
			dit!= apiDetours->preDetourFuncs.end(); dit++) {
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
		ReleaseMutex(apiDetours->preDetourMutex);
	} 

	//If bContinie is true, do the followings
	if(bContinue){
		//Call the original Win API
		//TRACE(0, _T("1.1 Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
		realResult = NextCreateFileW(
					lpFileName,
					dwDesiredAccess,
					dwShareMode,
					lpSecurityAttributes,
					dwCreationDisposition,
					dwFlagsAndAttributes,
					hTemplateFile
					);

		//TRACE(0, _T("1.2 Inside CE_BootstrapInjection_Detour_CreateFileW %d\n"), realResult);
		//Call all post detours
		if(apiDetours && realResult!=INVALID_HANDLE_VALUE) {
			WaitForSingleObject(apiDetours->postDetourMutex, INFINITE);
			//TRACE(0, _T("2. Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
			for(dit=apiDetours->postDetourFuncs.begin();
				dit!= apiDetours->postDetourFuncs.end(); dit++) {
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
			ReleaseMutex(apiDetours->postDetourMutex);
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
BOOL __stdcall CModuleScope::CE_BootstrapInjection_Detour_CreateProcessW(
									 LPCTSTR lpApplicationName,
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
	std::map<std::wstring, APIDetours*>::iterator it;
	std::multimap<DetourPriority, DetourItem *>::iterator dit;
	bool bContinue=true;
	BOOL realResult=false;
	APIDetours *apiDetours=NULL;

	TRACE(0, _T("CreateProcess: %s %s\n"), 
	lpApplicationName,
	lpCommandLine == NULL ? _T("") : lpCommandLine );

	//Get detours
	WaitForSingleObject(hookedAPIDetourMutex, INFINITE);
	it=hookedAPIDetours.find(L"CE_BootstrapInjection_Detour_CreateProcessW");
	if(it == hookedAPIDetours.end()) 
		ReleaseMutex(hookedAPIDetourMutex);
	else {
		apiDetours=it->second;
		ReleaseMutex(hookedAPIDetourMutex);

		//Call all pre detours
		WaitForSingleObject(apiDetours->preDetourMutex, INFINITE);
		//TRACE(0, _T("1. Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
		for(dit=apiDetours->preDetourFuncs.begin();
			dit!= apiDetours->preDetourFuncs.end(); dit++) {
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
		ReleaseMutex(apiDetours->preDetourMutex);
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

		TRACE(0, _T("CreateProcess: pid %d\n"), lpProcessInformation->dwProcessId);
		//Call all post detours
		if(apiDetours && realResult!=FALSE) {
			WaitForSingleObject(apiDetours->postDetourMutex, INFINITE);
			//TRACE(0, _T("2. Inside CE_BootstrapInjection_Detour_CreateFileW\n"));
			for(dit=apiDetours->postDetourFuncs.begin();
				dit!= apiDetours->postDetourFuncs.end(); dit++) {
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
			ReleaseMutex(apiDetours->postDetourMutex);
		}
	}
	return realResult;

}/* CModuleScope::CE_BootstrapInjection_Detour_CreateProcessW */
