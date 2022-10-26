#if defined (WIN32) 
#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <windef.h>

#include "celog.h"

//---------------------------------------------------------------------------
//
// Prototype of the main hook function
//
//---------------------------------------------------------------------------
typedef BOOL (WINAPI *PFN_INSTALLHOOK)(	BOOL bActivate, BOOL bServer);

namespace {
//Check if OCE installed on the system.
//OCE runs on Windows 2003 and later platform.
bool HasOCE()
{
  LONG rstatus;
  HKEY hKey = NULL; 

  rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
	  TEXT("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller"),
			  0,
			  KEY_QUERY_VALUE,
			  &hKey);

  if( rstatus != ERROR_SUCCESS ) {
    TRACE(CELOG_ERR, _T("LoadHookingConfiguration: %d no reg.\n"), rstatus);
    return false;
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
  if( rstatus != ERROR_SUCCESS ) {
    TRACE(CELOG_ERR, _T("LoadHookingConfiguration: No configuration path.\n"));
    return false;
  }

  WCHAR dll_path[MAX_PATH];
  _snwprintf_s(dll_path, MAX_PATH, _TRUNCATE,
               TEXT("%s..\\Office Communicator Enforcer\\bin"),ce_root);

  //First Check if OCE binary exists or not.
  HANDLE hFile=::CreateFileW(dll_path, GENERIC_READ, 0x7, 
			     NULL, OPEN_EXISTING, 0, NULL);
  if(hFile != INVALID_HANDLE_VALUE) {
    //OCE binary exists
    CloseHandle(hFile);
    TRACE(CELOG_INFO, _T("OCE exists: %s\n"), dll_path);
    return true;
  }else {
    int errorno=GetLastError();
    if(errorno==ERROR_ACCESS_DENIED) {
      TRACE(CELOG_INFO, _T("OCE exists: %s\n"), dll_path);
      return true;
    }
  }
  
  return false;
}
}

#if defined(_WIN64)
#define INJECTION_LIBRARY_NAME "ceInjection.dll"
#else
#define INJECTION_LIBRARY_NAME "ceInjection32.dll"
#endif

/* InstallHook
 *
 * Load 'ceInjection.dll' and call InstallHook() in it.
 */
static void InstallHook(BOOL bActivate,  BOOL bServer)
{
  HMODULE libh;
  PFN_INSTALLHOOK m_pfnInstallHook;

  TRACE(CELOG_INFO,_T("InstallHook: activate %d\n"), bActivate);

  libh = LoadLibraryA(INJECTION_LIBRARY_NAME);

  if( libh == NULL )
  {
    TRACE(CELOG_ERR,_T("LoadLibrary failed: %S (le %d)\n"), INJECTION_LIBRARY_NAME, GetLastError());
    return;
  }

  m_pfnInstallHook = (PFN_INSTALLHOOK)GetProcAddress(libh,"InstallHook");

  if( m_pfnInstallHook == NULL )
  {
    TRACE(CELOG_ERR,_T("GetProcAddress failed: InstallHook (le %d)\n"), GetLastError());
    return;
  }

  m_pfnInstallHook(bActivate,bServer);

  FreeLibrary(libh);

}/* InstallHook */

/* InstallHook
 *
 * Load 'ceInjection.dll' and call InstallHook() in it.
 */
#if defined(_WIN64)
#define BOOTSTRAP_LIBRARY_NAME "ceBootstrapInjection.dll"
#else
#define BOOTSTRAP_LIBRARY_NAME "ceBootstrapInjection32.dll"
#endif

static void InstallBootstrapHook(BOOL bActivate,  BOOL bServer)
{
  HMODULE libh;
  PFN_INSTALLHOOK m_pfnInstallHook;

  TRACE(CELOG_INFO,_T("InstallHook: activate %d\n"), bActivate);

  libh = LoadLibraryA(BOOTSTRAP_LIBRARY_NAME);

  if( libh == NULL )
  {
    TRACE(CELOG_ERR,_T("LoadLibrary failed: %S (le %d)\n"), BOOTSTRAP_LIBRARY_NAME, GetLastError());
    return;
  }

  m_pfnInstallHook = (PFN_INSTALLHOOK)GetProcAddress(libh,"InstallHook");

  if( m_pfnInstallHook == NULL )
  {
    TRACE(CELOG_ERR,_T("GetProcAddress failed: InstallHook (le %d)\n"), GetLastError());
    return;
  }

  m_pfnInstallHook(bActivate,bServer);

  FreeLibrary(libh);

}/* InstallBootstrapHook */

void StartBootstrapInjection()
{
  InstallHook(true, true);
  if(HasOCE()) {
    //The DLL ceBootStrapInjection.dll is for OCE only
    InstallBootstrapHook(true, true);
  }
}

void StopBootstrapInjection()
{
    // We used to call InstallHook(false, true) here, to uninject the
    // various libraries that we had injected (ceInjection.dll and
    // ceBootstrapInjection.dll).  Unfortunatly, on Windows 7 (and
    // Vista?) this can cause crashes when we re-inject these
    // libraries and this does not appear to be easily solveable.
    // Forget it.  We'll leave the injected libraries around.  They
    // all check the status of cepdpman before doing anything, so they
    // won't actually do anything harmful.
}
#else 
void StartBootstrapInjection()
{}
void StopBootstrapInjection()
{}
#endif 
