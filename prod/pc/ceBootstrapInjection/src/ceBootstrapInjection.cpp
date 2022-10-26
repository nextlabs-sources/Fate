// ceBootstrapInjection.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "brain.h"
#include "madCHook - static.h"
#pragma comment(lib, "madCHook-static.lib")
#include "ModuleScope.h"

#define CE_POLICY_CONTROLLER_UUID _T("b67546e2-6dc7-4d07-aa8a-e1647d29d4d7")

#ifdef _MANAGED
#pragma managed(push, off)
#endif

//---------------------------------------------------------------------------
// Shared by all processes variables
//---------------------------------------------------------------------------
#pragma data_seg(".HKT")
#pragma data_seg()

/* Determine if the caller is running in the context of the Policy Controller
   process.
 */
static bool is_policy_controller(void)
{
  HANDLE hPIDFileMapping;

  hPIDFileMapping = OpenFileMapping(FILE_MAP_WRITE,
				    FALSE,
				    CE_POLICY_CONTROLLER_UUID); // name of mapping object 

  if (hPIDFileMapping == NULL)
  {
    TRACE(0, _T("OpenFileMapping failed %d\n"), GetLastError());
    return false;
  }

  DWORD* pid;

  pid = (DWORD*)MapViewOfFile(hPIDFileMapping,      // handle to mapping object 
			      FILE_MAP_ALL_ACCESS,  // read/write permission 
			      0,                    // max. object size 
			      0,                    // size of hFile 
			      0);                   // map entire file 	

  bool is_pc = false;

  /* Mapping must be successful and PID of current process and Policy Controller
     must match.
  */
  if( pid != NULL && *pid == ::GetCurrentProcessId() )
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

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    {
      TRACE(0, _T("Attaching\n"));

      InitializeMadCHook();

      /* We disable thread notifications.  Prevent the system from calling
	 DllMain when threads are created or destroyed.
      */
      ::DisableThreadLibraryCalls( (HINSTANCE)hModule );

      if( is_policy_controller() == true )
      {
	return TRUE;
      }
		
      // Unload ceBootstrapInjection.dll from this process if it is not supposed to hook it.
      if ( !CModuleScope::GetInstance()->ManageModuleEnlistment() )
      {
	FinalizeMadCHook();
	return FALSE;
      } 
      break;
    }
    case DLL_PROCESS_DETACH:
    {
      FinalizeMadCHook();
      //
      // The DLL is being unmapped from the process's address space.
      //
      CModuleScope::GetInstance()->ManageModuleDetachment();
      break;
    }
  } // switch

  return TRUE;
}
//---------------------------------------------------------------------------
// InstallHook
//
//---------------------------------------------------------------------------
BOOL WINAPI InstallHook( BOOL bActivate, 
			 BOOL bServer )
{
  return CModuleScope::GetInstance()->InstallHookMethod(bActivate,bServer);
}
#ifdef _MANAGED
#pragma managed(pop)
#endif

