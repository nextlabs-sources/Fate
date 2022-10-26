/*****************************************************************************
 *
 * ceBootstrapInjection.cpp
 *
 * Entry point for handling library injection.
 *
 ****************************************************************************/
#include <windows.h>
#include <string>
#include "madCHook_helper.h"
#include "celog.h"
#include "ModuleScope.h"
#include "CEAdapter.h"

#define CE_POLICY_CONTROLLER_UUID "b67546e2-6dc7-4d07-aa8a-e1647d29d4d7"
namespace {
//Get SDK DLL location
bool GetSDKDLLPath(std::string &p)
{
  LONG rstatus;
  HKEY hKey = NULL; 

  rstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
	  "SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller",
	  0,KEY_QUERY_VALUE,&hKey);
  if( rstatus != ERROR_SUCCESS ) {
    return false;
  }

  char enforcer_root[MAX_PATH];                 /* InstallDir */
  DWORD enforcer_root_size = sizeof(enforcer_root);

  rstatus = RegQueryValueExA(hKey,"InstallDir",
			     NULL,NULL,(LPBYTE)enforcer_root,
			     &enforcer_root_size);
  RegCloseKey(hKey);
  if( rstatus != ERROR_SUCCESS ) {
    return false;
  }

  char libpath[MAX_PATH];
  _snprintf_s(libpath,
	    sizeof(libpath)/sizeof(char),
	    _TRUNCATE,
	    "%sPolicy Controller\\bin",
	    enforcer_root);

  p=libpath;
  return true;
}
//Set DLL path (On Windows 2000, SetDllDirectory is not avalible)
bool SetSDKDllPath()
{
   char* pathVar;
   size_t requiredSize=0;
   std::string finalPath("");
   std::string sdkPath;
   errno_t errRet;

   if(!GetSDKDLLPath(sdkPath)) {
     return false;
   }

#pragma warning( push )
#pragma warning( disable : 6309 6387)
   getenv_s(&requiredSize, NULL, 0, "PATH");
#pragma warning(pop)
   if(requiredSize != 0) {
     pathVar = (char*) malloc(requiredSize * sizeof(char));
     if (!pathVar) {
       return false;
     }

     // Get the value of the PATH environment variable.
     errRet=getenv_s( &requiredSize, pathVar, requiredSize, "PATH" );
     if(errRet != 0) {
       free(pathVar);
       return false;
     }
     finalPath=pathVar;
     free(pathVar);
     if(strstr(finalPath.c_str(), sdkPath.c_str())) {
       //path exists already, return
       return true;
     }
   }

   // Attempt to change path. Note that this only affects
   // the environment variable of the current process. The command
   // processor's environment is not changed.
   finalPath+=";"+sdkPath;     
   if(_putenv_s( "PATH", finalPath.c_str())==0)
     return true;

   return false;
}
}
/* Determine if the caller is running in the context of the Policy Controller
   process.
 */
static bool is_policy_controller(void)
{
  HANDLE hPIDFileMapping;

  hPIDFileMapping = OpenFileMappingA(FILE_MAP_WRITE,FALSE,
				     CE_POLICY_CONTROLLER_UUID); // name of mapping object 

  if (hPIDFileMapping == NULL)
  {
    TRACE(CELOG_ERR, _T("OpenFileMapping failed %d\n"), GetLastError());
    return false;
  }

  DWORD* pid;

  pid = (DWORD*)MapViewOfFile(hPIDFileMapping,FILE_MAP_ALL_ACCESS,0,0,0);

  bool is_pc = false;

  /* Mapping must be successful and PID of current process and Policy Controller
     must match.
  */
  if( pid != NULL && *pid == GetCurrentProcessId() )
  {
    is_pc = true;
  }

  if( pid != NULL )
  {
    UnmapViewOfFile(pid);
  }

  CloseHandle(hPIDFileMapping);

  return is_pc;

}/* is_policy_controller */

DWORD cedi_tls_index; 

// Create thread-locale storage for current thread.
static void CreateTLS(void)
{
  TRACE(CELOG_DEBUG, _T("CreateTLS %d\n"), GetCurrentThreadId());
  adapter::state* f = new (std::nothrow) adapter::state();
  TlsSetValue(cedi_tls_index,(PVOID)f);
}/* CreateTLS */

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
  switch (ul_reason_for_call)
  {
    case DLL_THREAD_ATTACH:
    {
      CreateTLS(); /* Allocate thread-local storage for thread */
      break;
    }/* DLL_THREAD_ATTACH */

    case DLL_THREAD_DETACH:
    {
      break;
    }/* DLL_THREAD_DETACH */

    case DLL_PROCESS_ATTACH:
    {
      if(!SetSDKDllPath())
		  return FALSE;

      /* Configure logging */
	  TRACE(CELOG_DEBUG, _T("Attaching\n"));

      cedi_tls_index = TlsAlloc();  /* Allocate thread-local storage */

      if( cedi_tls_index == TLS_OUT_OF_INDEXES )
      {
		TRACE(CELOG_ERR, "TlsAlloc failed (%d)\n", GetLastError());
		return FALSE;
      }

      CreateTLS(); /* Allocate thread-local storage for process */

      InitializeMadCHook();
	
      if( is_policy_controller() == true )
      {
		return TRUE;
      }

      // Unload ceBootstrapInjection.dll from this process if it is not supposed to hook it.
      if ( !CModuleScope::GetInstance()->ManageModuleEnlistment() )
      {
		return FALSE;
      } 
     break;
    }/* DLL_PROCESS_ATTACH */
    
    case DLL_PROCESS_DETACH:
    {
      FinalizeMadCHook();

 	  //TRACE(CELOG_DEBUG, _T("Detach process return true\n"));
	  TlsFree(cedi_tls_index);  /* Free thread-local storage for process */
      break;
    }/* DLL_PROCESS_DETACH */


  }// switch

  return TRUE;
}/* DllMain */

// InstallHook
extern "C" BOOL WINAPI InstallHook( BOOL bActivate, 
				    BOOL bServer )
{
  return CModuleScope::GetInstance()->InstallHookMethod(bActivate,bServer);
}
