#include <process.h>
#include <windows.h>
#include <cassert>
#include <winbase.h>

#include "nlca_client.h"

/****************************************************************************
 * !!!ToDo: Decide the path of nlca_service.exe 
 ***************************************************************************/

/********************************************************************************************
 *
 * Summary
 *
 * When the plug-in is started the NLCA Service (nlca_service.exe) will be started.  That
 * service creates a named mutex called "NLCA_ServiceMutex" which it also owns.  This
 * object is used to identify if the service is running and when it completes normally or
 * abnormally.
 *
 * Common cases:
 *
 *   (1) Plug-in starts, NLCA Service is not running.
 *       NLCA Service is started.  Retreive named mutex for process monitoring.
 *
 *   (2) Plug-in starts, NLCA Service is running.
 *       Retreive named mutex for process monitoring.
 *
 *   (3) Plug-in running, NLCA Service exits.
 *       Plug-in receives named mutex signal and restart NLCA Service.
 *
 *   (4) Plug-in running, NLCA Serivce running, Plug-in unload issued.
 *       NLCA Service is issued "Stop" and plug-in waits for named mutex to become signaled.
 *
 *******************************************************************************************/

namespace {

/** NLCAPluginContext
 *
 *  \brief Context information for NLCA Plugin
 */
struct NLCAPluginContext
{
  HANDLE nlca_mutex;       // Handle to NLCA service named mutex
  HANDLE stopPluginSignal; //mutex for stopping service event
  HANDLE monitorThreadID;  //thread handle of monitoring thread
};/* NLCAPluginContext */

void GetNLCAServicePath(WCHAR *path, int len)
{
  /* Determine NextLabs root directory */
  LONG rstatus;
  HKEY hKey = NULL; 
  rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
	  TEXT("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller"),
			  0,KEY_QUERY_VALUE,&hKey);
  if( rstatus != ERROR_SUCCESS) {
    _snwprintf_s(path,len, _TRUNCATE,L"..\\nlca_service.exe");
    return;
  }

  //Get InstallDir
  WCHAR nlDir[MAX_PATH];  
  DWORD nlDir_size = sizeof(nlDir);
  rstatus = RegQueryValueExW(hKey,               /* handle to reg */      
			    L"InstallDir",      /* key to read */
			     NULL,               /* reserved */
			     NULL,               /* type */
			     (LPBYTE)nlDir,    /* InstallDir */
			     &nlDir_size       /* size (in/out) */
			    );
  RegCloseKey(hKey);
  if( rstatus != ERROR_SUCCESS ) {
    _snwprintf_s(path,len, _TRUNCATE,L"..\\nlca_service.exe");
    return;
  }

  _snwprintf_s(path,len, _TRUNCATE,L"%sPolicy Controller\\bin\\nlca_service.exe", nlDir);
}/* GetNLCAServicePath */


/** start_nlca_service
 *
 *  \brief Start the NLCA Service (nlca_service.exe)
 *  \return true on success process start, otherwise false.
 */
bool start_nlca_service( NLCAPluginContext* context )
{
  assert( context != NULL );
  assert( context->nlca_mutex == NULL );

  if( context == NULL )
  {
    return false;
  }

  /* If the named nlca service mutex exists, then the nlca service is already
   * running and owns the mutex.  There is no need to start the process again.
   */
  context->nlca_mutex = CreateMutexA(NULL,FALSE,"NLCA_ServiceMutex");
  if( context->nlca_mutex == NULL )
  {
    return false;
  }

  if( GetLastError() == ERROR_ALREADY_EXISTS )
  {
    return true;
  }

  WCHAR servicePath[MAX_PATH];
  GetNLCAServicePath(servicePath, MAX_PATH);

  STARTUPINFO si = {0};
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi;
  BOOL rv = CreateProcessW(servicePath,
			   NULL,NULL,NULL,FALSE,
			   NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,
			   NULL,NULL,&si,&pi);
  if( rv != TRUE )
  {
    return false;  // it may already be running
  }
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  Sleep(100);  /* permit nlca_service.exe time to acquire NLCA_ServiceMutext */

  return true;
}/* start_nlca_service */

unsigned int __stdcall monitor(void *arg) 
{
  assert( arg != NULL );
  struct NLCAPluginContext* context=(NLCAPluginContext*)arg;

  if( start_nlca_service(context) == false )
  {
    _endthreadex(0); /* failed to start service (incorrect path?) */
  }

  while(1)
  {
    /****************************************************************************
     * Wait cases:
     *
     *  (1) PluginUnload was issued.
     *  (2) NLCA Service process was stopped.
     *
     *  Only one of these object may become signaled per WaitForMultipleObjects
     *  documentation.
     ***************************************************************************/
    HANDLE wait_objects[] = { context->stopPluginSignal , context->nlca_mutex };
    DWORD wait_result;
    wait_result = WaitForMultipleObjects(_countof(wait_objects),wait_objects,FALSE,INFINITE);

    if( wait_result == WAIT_OBJECT_0 )          // Case (1) - PluginUnload issued
    {
      break;
    }

    if( wait_result == WAIT_OBJECT_0 + 1 ||     // Case (2) - NLCA Service stopped
	wait_result == WAIT_ABANDONED_0 + 1 )
    {
      CloseHandle(context->nlca_mutex);
      context->nlca_mutex = NULL;
      start_nlca_service(context);
    }
  }
  _endthreadex(0);
  return NULL;
}/* monitor */

}
/****************************************************************************
 * Plug-in Entry Points
 ***************************************************************************/
extern "C" __declspec(dllexport) int PluginEntry( void** in_context )
{
  assert( in_context != NULL );
  if( in_context == NULL )
  {
    return 0; /* fail */
  }

  int bootType = GetSystemMetrics(SM_CLEANBOOT);

  if (bootType == 1) {
      // Safe mode without networking. The NLCA service can't run in
      // this mode, so we exit
      return 0;
  }

  
  //Initialize plugin context
  NLCAPluginContext* context = new NLCAPluginContext;
  memset(context,0x00,sizeof(NLCAPluginContext));
  *in_context = context;

  context->stopPluginSignal = CreateEventA(NULL,TRUE,FALSE,NULL);
  unsigned int thread_id = 0;
  context->monitorThreadID = (HANDLE)_beginthreadex(NULL,0,monitor,context,0,&thread_id);

  return 1; /* success */
}/* PluginEntry */

extern "C" __declspec(dllexport) int PluginUnload( void* in_context )
{
  assert( in_context != NULL );
  if( in_context == NULL )
  {
    return 0; /* fail */
  }
  NLCAPluginContext* context = (NLCAPluginContext*)in_context;

  /* issue signal to service monitor thread */
  SetEvent(context->stopPluginSignal);

  /* issue stop to NLCA service - signal service monitor thread */
  NLCAService_Stop();
  if( context->nlca_mutex != NULL )
  {
    WaitForSingleObject(context->nlca_mutex,INFINITE);
    CloseHandle(context->nlca_mutex);
  }

  //Wait monitor thread to exit
  WaitForSingleObject(context->monitorThreadID,INFINITE);
  CloseHandle(context->monitorThreadID);

  //Clean up
  delete context;
  return 1; /* success */
}/* PluginUnload */
