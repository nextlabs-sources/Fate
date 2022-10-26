#define WINVER _WIN32_WINNT_WINXP
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#define NTDDI_VERSION NTDDI_WINXPSP2

#include <cstdio>
#include <cstdlib>
#include <winsock2.h>
#include <windows.h>
#include <cassert>
#include <fltuser.h>
#include <tchar.h>
#include <WinIoCtl.h>
#include <Sddl.h>
#include "nl_tamper.h"
#include "nl_tamper_lib.h"
#include "CEsdk.h"
#include "nlTamperproofConfig.h"
#include "celog.h"
#include <psapi.h>

void SetStoppableFlag(HANDLE hPort)
{
    NLFltUnlaodControl structUnloadControl;
    structUnloadControl.bStoppable = 1;  
    DWORD dwUnused = 0;
    HRESULT res = FilterSendMessage(
                                    hPort,
                                    &structUnloadControl,
                                    sizeof(structUnloadControl),
                                    NULL,
                                    0,
                                    &dwUnused );
    TRACE( 0, L"SetStoppableFlag FilterSendMessage error: [0x%x]\n", res ); 
}

BOOL SetPrivilege(
                  HANDLE hToken,          // access token handle
                  LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
                  BOOL bEnablePrivilege   // to enable or disable privilege
                  ) 
{
 TOKEN_PRIVILEGES tp;
     LUID luid;
         
         if ( !LookupPrivilegeValue( 
                                     NULL,            // lookup privilege on local system
                                     lpszPrivilege,   // privilege to lookup 
                                     &luid ) )        // receives LUID of privilege
         {
 TRACE( 0, L"LookupPrivilegeValue error: %u\n", GetLastError() ); 
     return FALSE; 
         }
             
             tp.PrivilegeCount = 1;
         tp.Privileges[0].Luid = luid;
                                 if (bEnablePrivilege)
                                                          tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                                                          else
                                                                  tp.Privileges[0].Attributes = 0;
                                                          
                                                          // Enable the privilege or disable all privileges.
                                                          
                                                          if ( !AdjustTokenPrivileges(
                                                                                      hToken, 
                                                                                      FALSE, 
                                                                                      &tp, 
                                                                                      sizeof(TOKEN_PRIVILEGES), 
                                                                                      (PTOKEN_PRIVILEGES) NULL, 
                                                                                      (PDWORD) NULL) )
                                                          { 
 TRACE( 0, L"AdjustTokenPrivileges error: %u\n", GetLastError() ); 
     return FALSE; 
         } 
                                                                                                          
                                                                                                          if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
                                                                                                                                                           
                                                                                                                          {
 TRACE( 0, L"The token does not have the specified privilege. \n");
     return FALSE;
         } 
  
                                                                                                          TRACE( 0, L"enable unload driver priviledge succeed\n" ); 
                                                                                                              
                                                                                                              return TRUE;
                                                                                                                  }

void EnableFilterUnloadPriviledge()
{
 HANDLE hToken;
     if (!::OpenProcessToken( 
             GetCurrentProcess(),
         TOKEN_ADJUST_PRIVILEGES | // to adjust privileges
         TOKEN_QUERY,         // to get old privileges setting
         &hToken ) ) 
     {
 TRACE( 0, L"OpenProcessToken failed, [%u]\n", GetLastError());
     }
         SetPrivilege(
                      hToken,
                      SE_LOAD_DRIVER_NAME,
                      TRUE );
             ::CloseHandle( hToken );
                   }

/** TamperPluginContext
 *
 *  Context information for Tamper Resistance plug-in.
 */
typedef struct
{
    HANDLE th;            /* Thread handle */
    HANDLE port;          /* Filter communication port */
    HANDLE cancel_event;  /* Cancel support for PluginUnload */
} TamperPluginContext;

typedef struct
{
    ULONG      pid;                /* Process ID */
    CEAction_t action;             /* action */
    DWORD      evalTime;           /* Evaluation time */
} PolicyContext;

static CEHandle connectHandle=NULL;
static std::multimap<wstring, PolicyContext> cachedPolicyEval;
static DWORD cacheTime=GetTickCount();

#define CONNECT_TIMEOUT 500 /* half second */
#define QUERY_TIMEOUT 10000 /* 10 seconds */
#define CACHE_TIMEOUT (1000 * 60 *10) /* 10 minutes */

static bool StartNLTamperService()
{
    SERVICE_STATUS_PROCESS ssStatus; 
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    DWORD dwOldCheckPoint; 
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
    DWORD dwBytesNeeded;

    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
    NULL,                    // servicesActive database 
    SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) {
        return false;
    }

    // Get a handle to the service.
    schService = OpenService( 
        schSCManager,         // SCM database 
    L"NLTamper",            // name of service 
    SERVICE_ALL_ACCESS);  // full access 
 
    if (schService == NULL) {
        CloseServiceHandle(schSCManager);
        return false;
    }   

    // Check the status in case the service is not stopped. 
    if (!QueryServiceStatusEx( 
            schService,                     // handle to service 
        SC_STATUS_PROCESS_INFO,         // information level
        (LPBYTE) &ssStatus,             // address of structure
        sizeof(SERVICE_STATUS_PROCESS), // size of structure
        &dwBytesNeeded ) )  {           // size needed if buffer is too small
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return false; 
    }

    // Check if the service is already running. It would be possible
    // to stop the service here, but for simplicity this example just returns. 
    if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING) {
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return true; 
    }

    // Wait for the service to stop before attempting to start it.
    while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
        // Save the tick count and initial checkpoint.
        dwStartTickCount = GetTickCount();
        dwOldCheckPoint = ssStatus.dwCheckPoint;

        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth of the wait hint but not less than 1 second  
        // and not more than 10 seconds. 
 
        dwWaitTime = ssStatus.dwWaitHint / 10;

        if( dwWaitTime < 1000 )
            dwWaitTime = 1000;
        else if ( dwWaitTime > 10000 )
            dwWaitTime = 10000;

        Sleep( dwWaitTime );

        // Check the status until the service is no longer stop pending. 
        if (!QueryServiceStatusEx( 
                schService,                     // handle to service 
            SC_STATUS_PROCESS_INFO,         // information level
            (LPBYTE) &ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded ) ) {            // size needed if buffer is too small
            CloseServiceHandle(schService); 
            CloseServiceHandle(schSCManager);
            return false; 
        }

        if ( ssStatus.dwCheckPoint > dwOldCheckPoint ) {
            // Continue to wait and check.
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        } else {
            if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint) {
                CloseServiceHandle(schService); 
                CloseServiceHandle(schSCManager);
                return false; 
            }
        }
    }

    // Attempt to start the service.
    if (!StartService(
            schService,  // handle to service 
        0,           // number of arguments 
        NULL) ) {    // no arguments 
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return false; 
    }

    // Check the status until the service is no longer start pending. 
    if (!QueryServiceStatusEx( 
            schService,                     // handle to service 
        SC_STATUS_PROCESS_INFO,         // info level
        (LPBYTE) &ssStatus,             // address of structure
        sizeof(SERVICE_STATUS_PROCESS), // size of structure
        &dwBytesNeeded ) ) {            // if buffer too small
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return false; 
    }
 
    // Save the tick count and initial checkpoint.
    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    while (ssStatus.dwCurrentState == SERVICE_START_PENDING)  { 
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth the wait hint, but no less than 1 second and no 
        // more than 10 seconds. 
 
        dwWaitTime = ssStatus.dwWaitHint / 10;

        if( dwWaitTime < 1000 )
            dwWaitTime = 1000;
        else if ( dwWaitTime > 10000 )
            dwWaitTime = 10000;

        Sleep( dwWaitTime );

        // Check the status again. 
 
        if (!QueryServiceStatusEx( 
                schService,             // handle to service 
            SC_STATUS_PROCESS_INFO, // info level
            (LPBYTE) &ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded ) )  {           // if buffer too small
            break; 
        }
 
        if ( ssStatus.dwCheckPoint > dwOldCheckPoint ) {
            // Continue to wait and check.
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        } else {
            if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint) {
                // No progress made within the wait hint.
                break;
            }
        }
    }

    // Determine whether the service is running.
    if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
    } else { 
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return false;
    } 

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);

    return true;
}

static BOOL StopDependentServices(SC_HANDLE schSCManager, SC_HANDLE schService)
{
    DWORD i;
    DWORD dwBytesNeeded;
    DWORD dwCount;
	BOOL retVal = TRUE;

    LPENUM_SERVICE_STATUS   lpDependencies = NULL;
    ENUM_SERVICE_STATUS     ess;
    SC_HANDLE               hDepService;
    SERVICE_STATUS_PROCESS  ssp;

    DWORD dwStartTime = GetTickCount();
    DWORD dwTimeout = 30000; // 30-second time-out

    // Pass a zero-length buffer to get the required buffer size.
    if ( EnumDependentServices( schService, SERVICE_ACTIVE, 
                                lpDependencies, 0, &dwBytesNeeded, &dwCount ) ) {
        // If the Enum call succeeds, then there are no dependent
        // services, so do nothing.
        return TRUE;
    } else {
        if ( GetLastError() != ERROR_MORE_DATA )
            return FALSE; // Unexpected error

        // Allocate a buffer for the dependencies.
        lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc( 
            GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded );
  
        if ( !lpDependencies )
            return FALSE;

        __try {
            // Enumerate the dependencies.
            if ( !EnumDependentServices( schService, SERVICE_ACTIVE, 
                                         lpDependencies, dwBytesNeeded, &dwBytesNeeded,
                                         &dwCount ) )
           	{
            	retVal = FALSE;
				__leave;
            }

            for ( i = 0; i < dwCount; i++ ) {
                ess = *(lpDependencies + i);
                // Open the service.
                hDepService = OpenService( schSCManager, 
                                           ess.lpServiceName, 
                                           SERVICE_STOP | SERVICE_QUERY_STATUS );
                if ( !hDepService )
            	{
					retVal = FALSE;
					__leave;
            	}

                __try {
                    // Send a stop code.
                    if ( !ControlService( hDepService, 
                                          SERVICE_CONTROL_STOP,
                                          (LPSERVICE_STATUS) &ssp ) )
					{
						retVal = FALSE;
						__leave;
					}

                    // Wait for the service to stop.
                    while ( ssp.dwCurrentState != SERVICE_STOPPED ) {
                        Sleep( ssp.dwWaitHint );
                        if ( !QueryServiceStatusEx( 
                                 hDepService, 
                             SC_STATUS_PROCESS_INFO,
                             (LPBYTE)&ssp, 
                             sizeof(SERVICE_STATUS_PROCESS),
                             &dwBytesNeeded ) )
						{
							retVal = FALSE;
							__leave;
						}

                        if ( ssp.dwCurrentState == SERVICE_STOPPED )
                            break;

                        if ( GetTickCount() - dwStartTime > dwTimeout )
						{
							retVal = FALSE;
							__leave;
						}
                    }
                } 
                __finally {
                    // Always release the service handle.
                    CloseServiceHandle( hDepService );
                }

				if (retVal == FALSE)
				{
					__leave;
				}
            }
        } 
        __finally {
            // Always free the enumeration buffer.
            HeapFree( GetProcessHeap(), 0, lpDependencies );
        }
    } 
    return retVal;
}

static void StopNLTamperService()
{
    SERVICE_STATUS_PROCESS ssp;
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    DWORD dwStartTime = GetTickCount();
    DWORD dwBytesNeeded;
    DWORD dwTimeout = 30000; // 30-second time-out
    DWORD dwWaitTime;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
    NULL,                    // ServicesActive database 
    SC_MANAGER_ALL_ACCESS);  // full access rights 
    if (NULL == schSCManager) {
        return;
    }

    // Get a handle to the service.
    schService = OpenService( 
        schSCManager,         // SCM database 
    L"NLTamper",            // name of service 
    SERVICE_STOP | 
    SERVICE_QUERY_STATUS | 
    SERVICE_ENUMERATE_DEPENDENTS);  
    if (schService == NULL) { 
        CloseServiceHandle(schSCManager);
        return;
    }    

    // Make sure the service is not already stopped.
    if ( !QueryServiceStatusEx( 
             schService, 
         SC_STATUS_PROCESS_INFO,
         (LPBYTE)&ssp, 
         sizeof(SERVICE_STATUS_PROCESS),
         &dwBytesNeeded ) ) {
        goto stop_cleanup;
    }

    if ( ssp.dwCurrentState == SERVICE_STOPPED ) {
        goto stop_cleanup;
    }

    // If a stop is pending, wait for it.
    while ( ssp.dwCurrentState == SERVICE_STOP_PENDING ) {
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth of the wait hint but not less than 1 second  
        // and not more than 10 seconds. 
        dwWaitTime = ssp.dwWaitHint / 10;

        if( dwWaitTime < 1000 )
            dwWaitTime = 1000;
        else if ( dwWaitTime > 10000 )
            dwWaitTime = 10000;

        Sleep( dwWaitTime );

        if ( !QueryServiceStatusEx( 
                 schService, 
             SC_STATUS_PROCESS_INFO,
             (LPBYTE)&ssp, 
             sizeof(SERVICE_STATUS_PROCESS),
             &dwBytesNeeded ) ) {
            goto stop_cleanup;
        }

        if ( ssp.dwCurrentState == SERVICE_STOPPED ) {
            goto stop_cleanup;
        }

        if ( GetTickCount() - dwStartTime > dwTimeout ) {
            goto stop_cleanup;
        }
    }

    // If the service is running, dependencies must be stopped first.
    StopDependentServices(schSCManager, schService);

    // Send a stop code to the service.
    if ( !ControlService( 
             schService, 
         SERVICE_CONTROL_STOP, 
         (LPSERVICE_STATUS) &ssp ) ) {
        goto stop_cleanup;
    }

    // Wait for the service to stop.
    while ( ssp.dwCurrentState != SERVICE_STOPPED ) {
        Sleep( ssp.dwWaitHint );
        if ( !QueryServiceStatusEx( 
                 schService, 
             SC_STATUS_PROCESS_INFO,
             (LPBYTE)&ssp, 
             sizeof(SERVICE_STATUS_PROCESS),
             &dwBytesNeeded ) ) {
            goto stop_cleanup;
        }

        if ( ssp.dwCurrentState == SERVICE_STOPPED )
            break;

        if ( GetTickCount() - dwStartTime > dwTimeout ) {
            goto stop_cleanup;
        }
    }
 stop_cleanup:
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
    return;
}

static bool GetProcessNamebyID(long processID, WCHAR *procName, int nameLen )
{
    bool bSuccess=false;
  
    // Get a handle to the process.
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );
    if( hProcess == NULL )
    {
        return bSuccess;
    }

    // Get the process name.
    HMODULE hMod;
    DWORD cbNeeded;
    if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
                             &cbNeeded) ) {
        if( GetModuleBaseName( hProcess, hMod, procName,nameLen) != 0 )
        {
            bSuccess=true;
        }
    }

    // Print the process name and identifier.
    /*if(bSuccess)
      TRACE(CELOG_INFO, _T("GetProcessNamebyID %s  (PID: %u)\n"), 
      procName, processID );
      else
      TRACE(CELOG_ERR, _T("Failed to get process name of PID: %u\n"), 
      processID );*/

    CloseHandle( hProcess );
    return bSuccess;
}

/** GetProcessUserSID
 *
 *  \param pid (in)     Process ID.
 *  \param sid (in-out) Storage for SID of processes' owner.
 *  \param size (in)    Size (in characters) of storage for sid parameter including
 *                      NULL termination.
 *
 */
static bool GetProcessUserSID(ULONG pid, WCHAR *sid, int size)
{
    bool bRet=false;
    HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );
    if( hProcess == NULL ) {
        return bRet;
    }

    HANDLE hProcessToken   = INVALID_HANDLE_VALUE;
    PTOKEN_USER pTokenUser = NULL;
    WCHAR *pSid          = NULL;

    if (!sid)    return bRet;
    if (size <= 0)  return bRet;

    // According to MSDN. GetCurrentProcess actually return (HANDLE)-1
    if (!OpenProcessToken (hProcess, TOKEN_QUERY, &hProcessToken)) {
        goto failed_and_cleanup;
    }

    // From MSDN, we should make it fail first, to get the right size
    DWORD len;
    GetTokenInformation(hProcessToken, TokenUser, NULL, 0, &len);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        pTokenUser = (PTOKEN_USER) malloc (len);
        if (!pTokenUser) goto failed_and_cleanup;

        GetTokenInformation (hProcessToken, TokenUser, pTokenUser, len, &len);

        ConvertSidToStringSidW (pTokenUser->User.Sid, &pSid);

        size_t sidLen = nlstrlen (pSid);

        // Bail if we don't have enough buffer
        if ((sidLen + 1) > static_cast<size_t>(size)) {
            goto failed_and_cleanup;
        }
    
        wcsncpy_s (sid, size, pSid,_TRUNCATE);
        bRet = true;
    }

 failed_and_cleanup:
    CloseHandle(hProcess);
    if (hProcessToken != INVALID_HANDLE_VALUE) CloseHandle (hProcessToken);
    if (pTokenUser) free (pTokenUser);
    if (pSid)       LocalFree (pSid);

    return bRet;
}

//Read tamper resistance configuration files for enforcers and protect their files
static void ReadTamperConfig()
{
    NLTamperproofMap *o=NULL;
    NLTamperproofMap::iterator it;
    NLTamperproofMap::iterator eit;
    int flag;

    bool r=NLTamperproofConfiguration_Load(NL_TAMPERPROOF_TYPE_FILE, &o);
    if(r && o) {
        eit=o->end();
        for(it=o->begin(); it!=eit; it++) {
            if(it->second.access == NL_TAMPERPROOF_ACCESS_RO) 
                flag=NL_TAMPER_FLAG_DENY_WRITE|NL_TAMPER_FLAG_NOTIFY;
            else if(it->second.access == NL_TAMPERPROOF_ACCESS_NONE)
                flag=NL_TAMPER_FLAG_DENY_ALL|NL_TAMPER_FLAG_NOTIFY;
            else
                continue;
            NLTamper_ProtectFile(it->first.c_str(), flag);
            /****************************************************************************
             * TBD: Once we find the offical way to get full path name of I/O request
             * process, we will complete this functionality. 
             ***************************************************************************/
            /*std::set<nlstring>::iterator pit=it->second.exemptProc.begin();
              std::set<nlstring>::iterator epit=it->second.exemptProc.end();
              for(; pit!=epit; pit++) {
              NLTamper_FileExemptProcessName(pit->c_str(), it->first.c_str());
              }*/
        }
        NLTamperproofConfiguration_Free(o);  
    }
}

static bool Connect(void)
{
    CEUser          user;
    CEApplication app;
    CEResult_t res;

    user.userID=NULL;
    user.userName=NULL;

    memset(&app,0x00,sizeof(app));
    app.appName   = CEM_AllocateString(L"NLTAMPER Plugin");  // Application
    app.appPath  = CEM_AllocateString(L"");

    res = CECONN_DLL_Activate(app,user,NULL,&connectHandle,CONNECT_TIMEOUT); 

    CEM_FreeString(app.appName);
    CEM_FreeString(app.appPath);

    if(res != CE_RESULT_SUCCESS) {
        connectHandle=NULL;
    }

    return (res==CE_RESULT_SUCCESS)?true:false;
}/* Connect */

static void Disconnect(void)
{ 
    if(connectHandle)
        CECONN_DLL_Deactivate(connectHandle,CE_INFINITE);
    connectHandle=NULL;
}/* Disconnect */

static void Eval(UINT32 access, 
                 const TCHAR* resource_from, 
                 const TCHAR* resource_to,
                 const TCHAR* appName,
                 const TCHAR* sid,
                 ULONG pid)
{
    if(connectHandle == NULL) {
        if(!Connect()) 
            return;
    }


    //compose action
    CEAction_t      action;
    if(access & (FILE_WRITE_ACCESS|FILE_APPEND_DATA|FILE_WRITE_DATA))
        action=CE_ACTION_WRITE;
    else if(access & (FILE_WRITE_EA|FILE_WRITE_ATTRIBUTES))
        action=CE_ACTION_CHANGE_ATTR_FILE;
    else if (access & DELETE)
        action=CE_ACTION_DELETE;
    else
        action=CE_ACTION_READ;

    //Check if this evaluation just happened recently
    std::multimap<wstring, PolicyContext>::iterator it=cachedPolicyEval.find(resource_from);
    if(it != cachedPolicyEval.end()) {
        std::multimap<wstring, PolicyContext>::iterator eit=cachedPolicyEval.upper_bound(resource_from);
        for ( ; it != eit; ++it) {
            if(it->second.pid == pid && it->second.action==action) {
                double currentTime=GetTickCount();
                if((currentTime - it->second.evalTime) <= 2000) {
                    //not older than 2 seconds, don't need to call CheckFile function
                    return;
                } else {//erase old record
                    cachedPolicyEval.erase(it);
                    break;
                }
            }
        } /*for ( ; it != eit; ++it)*/
    }/* if(it != cachedPolicyEval.end()) */

    CEAttribute     attr[2];
    CEApplication   appRun;
    CEString        source = CEM_AllocateString(resource_from);
    CEString        target = NULL;
    CEUser          user;
    CEAttributes    sourceAttributes;
    CEAttributes    targetAttributes;
 
    //compose user
    user.userID = CEM_AllocateString(sid);
    user.userName = NULL;

    //compose target resource
    if(resource_to != NULL && _tcslen(resource_to)>0 ) 
        target=CEM_AllocateString(resource_to);

    //compose application
    memset(&appRun,0x00,sizeof(appRun));
    appRun.appPath = CEM_AllocateString(appName);

    //compose source resource attribute
    memset(&sourceAttributes, 0,sizeof(sourceAttributes));
    sourceAttributes.count = 2;
    sourceAttributes.attrs = attr;
    sourceAttributes.attrs[0].key   = CEM_AllocateString(CE_ATTR_LASTWRITE_TIME);
    sourceAttributes.attrs[0].value = CEM_AllocateString(L"123456789");
    sourceAttributes.attrs[1].key   = CEM_AllocateString(_T("tamper-resistance-decision"));
    sourceAttributes.attrs[1].value = CEM_AllocateString(_T("deny"));

    //compose target resource attribute
    memset(&targetAttributes, 0,sizeof(targetAttributes));
    targetAttributes.count = 0;
    targetAttributes.attrs = NULL;

    CEResult_t res;
    CEEnforcement_t enforcement;
    memset(&enforcement,0x00,sizeof(enforcement));

    //Notify PC
    res = CEEVALUATE_CheckFile(connectHandle,action,
                               source,&sourceAttributes,
                               target,&targetAttributes,
                               0,&user,&appRun,
                               CETrue,CE_NOISE_LEVEL_USER_ACTION,
                               &enforcement,QUERY_TIMEOUT);

    if( res != CE_RESULT_SUCCESS ) {
        if(res == CE_RESULT_CONN_FAILED) {
            //call disconnect to free connectHandle
            Disconnect();
        }
    } else {
        CEEVALUATE_FreeEnforcement(enforcement);
    }

    DWORD elapsed=GetTickCount()-cacheTime;
    if(cachedPolicyEval.size() > MAX_PATH || 
       (elapsed > CACHE_TIMEOUT) || (elapsed < 0)) {
        //clean off the cache
        cachedPolicyEval.clear();  
        cacheTime=GetTickCount();
    }
    //Cache this evaluation
    PolicyContext c;
    c.pid=pid;
    c.action=action;
    c.evalTime=GetTickCount();
    cachedPolicyEval.insert(make_pair(wstring(resource_from), c));

    //clean up
    CEM_FreeString(source);
    CEM_FreeString(user.userID);
    if(target)
        CEM_FreeString(target);
    CEM_FreeString(appRun.appPath);
    CEM_FreeString(sourceAttributes.attrs[0].key);
    CEM_FreeString(sourceAttributes.attrs[0].value);
    CEM_FreeString(sourceAttributes.attrs[1].key);
    CEM_FreeString(sourceAttributes.attrs[1].value);
}/* Eval */

static DWORD WINAPI tamper_plugin_worker( LPVOID in_context )
{
    assert( in_context != NULL );
    TamperPluginContext* context = (TamperPluginContext*)in_context;

    for( ;; )
    {
        recv_msg_t recv_msg;
        OVERLAPPED io;
        memset(&io,0x00,sizeof(io));
        io.hEvent = CreateEventA(NULL,TRUE,FALSE,NULL);

        if(io.hEvent == NULL) {
            break;
        }

        HRESULT hr;
        memset(&recv_msg,0x00,sizeof(recv_msg));
        hr = FilterGetMessage(context->port,(PFILTER_MESSAGE_HEADER)&recv_msg,sizeof(recv_msg),&io);
        if( hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING) )
        {
            break;
        }

        /* Wait for cancel or event from nltamper driver */
        HANDLE wait_handles[] = { context->cancel_event , io.hEvent };
        DWORD wait_result = WaitForMultipleObjects(2,wait_handles,FALSE,INFINITE);

        /* If cancellation occurs from the unload callback the cancel event will be
         * in a signaled stage.  Test for signaled state.
         *
         * WaitForMultipleObjects returns the lowest array index which is the cancel
         * event.
         */
        if( wait_result == WAIT_OBJECT_0 )
        {
            CancelIo(context->port);
            CloseHandle(io.hEvent);
            break;
        }

        int status = 0;
        if( wait_result == (WAIT_OBJECT_0 + 1) )     /* IO event */
        {
            DWORD out_size = 0;
            if( GetOverlappedResult(context->port,&io,&out_size,FALSE) == TRUE )
            {
                status = 1;  /* IO completed with success */
            }
            CloseHandle(io.hEvent);
        }

        if( status )
        {
            WCHAR sid[MAX_PATH];
            WCHAR pname[MAX_PATH];
            size_t pname_size=wcslen(recv_msg.fsm.pname); 
            if(!GetProcessUserSID(recv_msg.fsm.pid, sid, MAX_PATH)) {
                _snwprintf_s(sid, MAX_PATH, _TRUNCATE,L"S-1-0-0");
            }
            if(pname_size > 0) {
                NLTamper_TranslatePath(recv_msg.fsm.pname,_countof(recv_msg.fsm.pname)); 
            } else if(!GetProcessNamebyID(recv_msg.fsm.pid, pname, MAX_PATH)) {
                _snwprintf_s(pname, MAX_PATH, _TRUNCATE, L"process name not avaliable");
            }
            NLTamper_TranslatePath(recv_msg.fsm.fname,_countof(recv_msg.fsm.fname));
            Eval(recv_msg.fsm.access,recv_msg.fsm.fname, NULL, 
                 (pname_size > 0)?recv_msg.fsm.pname:pname, sid, recv_msg.fsm.pid);
        }
    }/* for */
    return 0;
}/* tamper_plugin_worker */

/****************************************************************************
 * Plug-in Entry Points
 ***************************************************************************/
extern "C" __declspec(dllexport) int PluginEntry( void** in_context )
{
    int result = 0;
    HRESULT hr;

    assert( in_context != NULL );
    if( in_context == NULL )
    {
        return 0;
    }


    EnableFilterUnloadPriviledge();


    TamperPluginContext* context =  (TamperPluginContext*)malloc(sizeof(TamperPluginContext));
    if( context == NULL )
    {
        return 0;  /* out of memory */
    }

    *in_context = (void*)context;
    memset(context,0x00,sizeof(TamperPluginContext));

    /* Cancel event in a non-signaled state */
    context->cancel_event = CreateEventA(NULL,TRUE,FALSE,NULL);
    if( context->cancel_event == NULL )
    {
        goto PluginEntry_complete;
    }

    /****************************************************************************
     * Policy Controller is running as SYSTEM who doesn't have the privilege,
     * SE_LOAD_DRIVER_PRIVILEGE. Thus, FilterLoad/FilterUnload will be failed.  
     * Using StartService to start NLTamper filter.
     ***************************************************************************/
    /*hr = FilterLoad(L"NLTamper");
      if( !(hr == S_OK ||
      hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) ||
      hr == HRESULT_FROM_WIN32(ERROR_SERVICE_ALREADY_RUNNING) ) )
      {
      WCHAR debugStr[MAX_PATH];
      swprintf(debugStr, L"nltamper plugin FilterLoad failed 0x%x\n", hr);
      OutputDebugString(debugStr);
      goto PluginEntry_complete;
      }*/

    /* Ensure the filter is started */
    if(!StartNLTamperService()) {
        goto PluginEntry_complete;
    }

    hr = FilterConnectCommunicationPort(NLTAMPER_PORT_NAME,0,NULL,0,NULL,&context->port);
    if( IS_ERROR(hr) )
    {
        goto PluginEntry_complete;
    }

    /* Start worker thread for file system event notifications */
    context->th = CreateThread(NULL,0,tamper_plugin_worker,*in_context,0,NULL);
    if( context->th != NULL )
    {
        result = 1;
    }

    /* Exempt default processes */
    NLTamper_ExemptProcessId(GetCurrentProcessId());
    //NLTamper_ExemptProcessName(L"cepdpman.exe");
    //NLTamper_ExemptProcessName(L"devenv.exe");
  
    /*******************************************************************************
     * Read tamper configuration for files
     ******************************************************************************/
    ReadTamperConfig();

 PluginEntry_complete:

    if( result == 0 )   /* cleanup on failure */
    {
        if( context->port != INVALID_HANDLE_VALUE )
        {
            CloseHandle(context->port);
        }
        if( context->cancel_event != INVALID_HANDLE_VALUE &&
            context->cancel_event != NULL ) {
            CloseHandle(context->cancel_event);
        }
        free(context);
        context = NULL;
    }

    return result;
}/* PluginEntry */

/*****************************************************************************
 * PluginUnload
 ****************************************************************************/
extern "C" __declspec(dllexport) int PluginUnload( void* in_context )
{
    Disconnect();

    assert( in_context != NULL );
    if( in_context == NULL )
    {
        return 0;
    }

    TamperPluginContext* context = (TamperPluginContext*)in_context;

    /*  send driver message, set driver's stoppable flag */
    SetStoppableFlag(context->port);

    /* Signal thread to complete */
    SetEvent(context->cancel_event);
    WaitForSingleObject(context->th,INFINITE);
    CloseHandle(context->th);
    CloseHandle(context->port);
    CloseHandle(context->cancel_event);

    delete in_context;

    /****************************************************************************
     * Policy Controller is running as SYSTEM who doesn't have the privilege,
     * SE_LOAD_DRIVER_PRIVILEGE. Thus, FilterLoad/FilterUnload will be failed.  
     * Using ControlService to stop NLTamper filter.
     *
     *  new comment, 2011-5-3, I add privilege by call to EnableFilterUnloadPriviledge()
     *  so, I can use FilterUnload.
     *  the reason I want to use FilterUnlaod is because FilterUnload is a non-mandatory unload request
     *  while sc manager is mandatory unload request, I want to disable all mandatory unload request in driver,
     *  and only allow non-mandatory unload request when a control flag is enabled in driver.
     *  this is for bug 14289.
     ***************************************************************************/
    HRESULT res = FilterUnload(L"NLTamper");
    TRACE( 0, L"FilterUnload error: [0x%x]\n", res ); 
    //StopNLTamperService();
    return 1;
}/* PluginUnload */
