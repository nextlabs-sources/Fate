/*========================tamperCYGWIN_NT-5.1.cpp===========================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2008 by NextLabs,     *
 * San Mateo, CA, Ownership remains with NextLabs Inc,                      * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author :                                                                 *
 * Date   : 03/06/2008                                                      *
 * Note   : The file/proess protection module using dynamic injection       *
 *          service (a.k.a ceInjection module).                             *
 *          This is only applicable to Windows platform                     *
 *==========================================================================*/
#include <map>
#include <AccCtrl.h>
#include "tamper_private.h"
#include "CEAdapter.h"

// from ntstatus.h
#define STATUS_ACCESS_DENIED 0xC0000022L 

namespace {
class BookKeeper {
public:
  map<HANDLE, nlstring> handleFileMapping;
  nlthread_mutex_t hfMappingMutex;

  //Constructor
  BookKeeper() {
    nlthread_mutex_init(&hfMappingMutex);
  }

  //Destructor
  ~BookKeeper() {
    nlthread_mutex_destroy(&hfMappingMutex);
  }
};

TamperPolicy policy;
BookKeeper bookkeeper;

/********************************************************
 * The following functions are scoped in this file only *
 ********************************************************/

static DWORD getIdFromHandle(HANDLE hProcess)
{
    HANDLE hProc = 0;

    HANDLE hCurrent = ::GetCurrentProcess();

    if (!::DuplicateHandle (hCurrent, hProcess, hCurrent, &hProc,
                            PROCESS_QUERY_INFORMATION, FALSE, 0)) {

        // Get a handle to ourselves.  We want the ability to duplicate other handles, so the pseudo
        // handle returned by GetCurrentProcess might not be enough (and isn't for Vista/Windows 7,
        // although we'll take it as a last resort)
        if (!::DuplicateHandle(::GetCurrentProcess(),
                               ::GetCurrentProcess(),
                               ::GetCurrentProcess(),
                               &hCurrent,
                               PROCESS_DUP_HANDLE,
                               FALSE,
                               0)) {
            // We are stuck
            return 0;
        }

        if (!::DuplicateHandle (hCurrent, hProcess, hCurrent, &hProc,
                                PROCESS_QUERY_LIMITED_INFORMATION, FALSE, 0)) {
        }
    }

    return GetProcessId(hProc);
}

/*The detour of Win32 function "TerminateProcess"*/
BOOL WINAPI myTerminateProcess(HANDLE hProcess, UINT uExitCode)
{
  adapter::SetAdapterState("kernel32.dll","TerminateProcess");

  DWORD processID=getIdFromHandle(hProcess);

  if(processID == 0) {
  } else {
      nlchar processIDStr[128];
      nlsprintf(processIDStr, _countof(processIDStr), _T("%d"),processID); 
      if( policy.evaluation(CE_ACTION_PROCESS_TERMINATE,processIDStr) == false) {
          adapter::SetDenyReturnValue<BOOL>(FALSE);
      }
  }
  
  return adapter::Next<BOOL>(hProcess, uExitCode);
}/* myTerminateProcess */

// detour for Win32 function "myNtSuspendProcess"
// This is a deprecated function as of January 2010.
// But Process Explorer invokes this when "suspend" button is clicked. 
UINT WINAPI myNtSuspendProcess(HANDLE hProcess)
{
    
    adapter::SetAdapterState("ntdll.dll","NtSuspendProcess");
    
    DWORD processID= getIdFromHandle(hProcess);

    if(processID == 0) {
    } else {
	nlchar processIDStr[128];
	nlsprintf(processIDStr, _countof(processIDStr), _T("%d"),processID); 
	if( policy.evaluation(CE_ACTION_PROCESS_TERMINATE,processIDStr) == false) {
	    adapter::SetDenyReturnValue<UINT>(STATUS_ACCESS_DENIED);
	} 
    }
    
    return adapter::Next<UINT>(hProcess);
}


}

/*********************************************************************** 
 * Entry point.  Initialize state and add hooks.  This symbol 
 * must be exported.
 **********************************************************************/
extern "C" int AdapterEntry(void)
{
  bool bresult=true;
  /* The following function(s) need to be hooked for tamper proof purpose */
  /* TerminateProcess */
  if( bresult == true )
    bresult = adapter::AddHook("kernel32.dll",
			       "TerminateProcess",
			       myTerminateProcess);
  /**end**The following functions need to be hooked for tamper proof purpose */

  /* When initialization fails the return value will force the unloading
     of the hook library.
  */
  if( bresult == false ) {
    return -1;
  }

  // similarly to TerminateProcess, we will hook NtSuspendProcess
  bresult = adapter::AddHook("ntdll.dll",
			     "NtSuspendProcess",
			     myNtSuspendProcess);

  // When initialization fails the return value will force the unloading
  // of the hook library.
  if( bresult == false ) {
    return -1;
  }
  
  return 0;
}/* AdapterEntry */
