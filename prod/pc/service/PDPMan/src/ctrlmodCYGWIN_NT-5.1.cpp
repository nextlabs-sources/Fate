/*
*
* Author: Helen Friedland
* All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
* Redwood City CA, Ownership remains with Blue Jungle Inc, 
* All rights reserved worldwide. 
*
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#include "brain.h"
#include "ctrlmodCYGWIN_NT-5.1.h"
#include "tamperCYGWIN_NT-5.1.h"
#include "CEsdk.h" // for CE_RESULT_* error code

// CELog
#include "celog.h"
#include "celog_policy_file.hpp"
#include "celog_policy_windbg.hpp"
#include "nlconfig.hpp"
#include "nlthread.h"

// JNI
#include <jni.h>
#include "JavaConstants.h"

#include <wtsapi32.h> // or WTS functions
#include <Sddl.h> // for ConvertSidToStringSid()
#include <Userenv.h>

extern JavaVM *PDP_jvm; // external variable in PDPMan.cpp
extern nlthread_sem_t pdpInitializedLock;

void ServiceStart(int argc, char** argv);
LPSTR convertArgListToArgString(LPSTR lpszTarget, DWORD dwStart, 
				DWORD dwArgc, LPSTR *lpszArgv);
LPSTR *convertArgStringToArgList(LPSTR *lpszArgs, PDWORD pdwLen, 
				 LPSTR lpszArgstring);
//
//  FUNCTION: getStringValue()
//
//  PURPOSE: Fetches a REG_SZ or REG_EXPAND_SZ string value
//           from a specified registry key    
//
//  PARAMETERS:
//    lpVal - a string buffer for the desired value
//    lpcbLen  - pointer to LONG value with buffer length
//    hkRoot - the primary root key, e.g. HKEY_LOCAL_MACHINE
//    lpszPath - the registry path to the subkey containing th desired value
//    lpszValue - the name of the desired value    
//
//  RETURN VALUE:
//    0 on success, 1 on failure
//
int getStringValue(LPBYTE lpVal, LPDWORD lpcbLen, HKEY hkRoot, 
		   LPCSTR lpszPath, LPSTR lpszValue);
//
//  FUNCTION: setStringValue()
//
//  PURPOSE: Assigns a REG_SZ value to a 
//           specified registry key    
//
//  PARAMETERS:
//    lpVal - Constant byte array containing the value
//    cbLen  - data length
//    hkRoot - the primary root key, e.g. HKEY_LOCAL_MACHINE
//    lpszPath - the registry path to the subkey containing th desired value
//    lpszValue - the name of the desired value    
//
//  RETURN VALUE:
//    0 on success, 1 on failure
//
int setStringValue(CONST BYTE *lpVal, DWORD cbLen, HKEY hkRoot, 
		   LPCSTR lpszPath, LPCSTR lpszValue);
//
//  FUNCTION: makeNewKey()
//
//  PURPOSE: Creates a new key at the specified path  
//
//  PARAMETERS:
//    hkRoot - the primary root key, e.g. HKEY_LOCAL_MACHINE
//    lpszPath - the registry path to the new subkey
//
//  RETURN VALUE:
//    0 on success, 1 on failure
//
int makeNewKey(HKEY hkRoot, LPCSTR lpszPath);
//End of External Functions 

//global variables
static SERVICE_STATUS          ssStatus;       
static SERVICE_STATUS_HANDLE   sshStatusHandle;
static DWORD                   dwErr = 0;
static BOOL                    bConsole = FALSE;
static TCHAR                   szErr[256];

using namespace ctrlmod_key;

// This is pretty bad.  I don't know why there is a function that's 
// shared to the outside world, and it's mixed with the namespace

// Return the SID of the specified user. pszSID must be deallocated by caller
// This function was copied from UserUtils.cpp in Destiny
void GetUserSID(TCHAR* pszUserName, TCHAR **ppszSID)
{
    UCHAR          psnuType[2048];
    UCHAR          userSID[1024];
    DWORD          dwSIDBufSize=1024;
    TCHAR          lpszDomain[2048];
    DWORD          dwDomainLength = 250;
    
    if (LookupAccountName(NULL,
			  pszUserName,
			  userSID,
			  &dwSIDBufSize,
			  lpszDomain,
			  &dwDomainLength,
			  (PSID_NAME_USE) psnuType))
	{
	    ConvertSidToStringSid((PSID) userSID, ppszSID);
	}
}


bool IsEdpManagerRunning(unsigned long uSessionID);


std::string GetEDPMgrExePath()
{
	const char* pPCRegPath = "SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller";
	char wszEDPMgrDir[MAX_PATH] = { 0 };
	DWORD dwLen = MAX_PATH * sizeof(wszEDPMgrDir[0]);
	getStringValue((LPBYTE)wszEDPMgrDir, &dwLen, HKEY_LOCAL_MACHINE, pPCRegPath, "ControlPanelDir");
	std::string strEDPMgrExePath = wszEDPMgrDir;
	strEDPMgrExePath += "bin\\edpmanager.exe";
	return strEDPMgrExePath;
}

//
bool StartEdpManagerAsLogonUser(DWORD dwRDSeessionID)
{
	const char* pPCRegPath = "SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller";
	char wszEDPMgrDir[MAX_PATH] = { 0 };
	DWORD dwLen = MAX_PATH * sizeof(wszEDPMgrDir[0]);
	getStringValue((LPBYTE)wszEDPMgrDir, &dwLen, HKEY_LOCAL_MACHINE, pPCRegPath, "ControlPanelDir");
	std::string strEDPMgrExePath = wszEDPMgrDir;
	strEDPMgrExePath += "bin\\edpmanager.exe";

	//get session token
	HANDLE tk = NULL;
	if (!WTSQueryUserToken(dwRDSeessionID, &tk)) {
		TRACE(CELOG_ERR, _T("StartEdpManagerAsLogonUser, WTSQueryUserToken failed \n"));
		return false;
	}
	if (NULL == tk) {
		TRACE(CELOG_ERR, _T("StartEdpManagerAsLogonUser, token==NULL \n"));
		return false;
	}

	//start edpmanager
	STARTUPINFOA   si;
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = "WinSta0\\Default";

	std::string strEdpDirectory = wszEDPMgrDir;
	strEdpDirectory += "bin\\";

	void* pEnvironment = NULL;
	bool bCreateEnv = CreateEnvironmentBlock(&pEnvironment, tk, false);

	if (!::CreateProcessAsUserA(tk, strEDPMgrExePath.c_str(), NULL, NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, pEnvironment, strEdpDirectory.c_str(), &si, &pi)) {
	
		TRACE(CELOG_ERR, _T("StartEdpManagerAsLogonUser, CreateProcessAsUserA failed,  lastError=%u\n"), GetLastError() );
		CloseHandle(tk);
		if (pEnvironment)
		{
			DestroyEnvironmentBlock(pEnvironment);
		}
		return false;
	}

	if (pEnvironment)
	{
		DestroyEnvironmentBlock(pEnvironment);
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(tk);

	return true;

}


namespace ctrlmod_win {
    
// If running as a service, use event logging to post a message
// If not, display the message on the console.
VOID AddToMessageLog(LPTSTR lpszMsg)
{
  TCHAR   szMsg[256];
  HANDLE  hEventSource;
  LPTSTR  lpszStrings[2];

  if (!bConsole) {
    dwErr = GetLastError();

    hEventSource = RegisterEventSource(NULL, SZSERVICENAME);

    _sntprintf_s(szMsg, _countof(szMsg), _TRUNCATE, _T("%s error: %d"), SZSERVICENAME, dwErr);
    lpszStrings[0] = szMsg;
    lpszStrings[1] = lpszMsg;
    
    if (hEventSource != NULL) 
    {
      ReportEvent(hEventSource, 
                  EVENTLOG_ERROR_TYPE,  
                  0,                    
                  0,                    
                  NULL,                 
                  2,                    
                  0,                    
                  (LPCTSTR *) lpszStrings,          
                  NULL);                
      
      DeregisterEventSource(hEventSource);
    }
  }
  else
  {
    _tprintf(TEXT("%s\n"), lpszMsg);
  }
}
}


namespace {
//Constant
#define SZFAILURE      "StartServiceControlDispatcher failed!"
#define SZSCMGRFAILURE "OpenSCManager failed - %s\n"
#define SZAPPPARAMS    "AppParameters"

// Create an error message from GetLastError() using the
// FormatMessage API Call...
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize )
{
  DWORD dwRet;
  LPTSTR lpszTemp = NULL;

  dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			 FORMAT_MESSAGE_FROM_SYSTEM |
			 FORMAT_MESSAGE_ARGUMENT_ARRAY,
			 NULL,
			 GetLastError(),
			 LANG_NEUTRAL,
			 (LPTSTR)&lpszTemp,
			 0,
			 NULL);

  // supplied buffer is not long enough
  if (!dwRet || ((long)dwSize < (long)dwRet+14)) {
    lpszBuf[0] = TEXT('\0');
  } else {
    //remove cr and newline character
    lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0'); 
    _sntprintf_s( lpszBuf, 256, _TRUNCATE, TEXT("%s (0x%x)"), lpszTemp, GetLastError());
  }

  if (lpszTemp) {
    GlobalFree((HGLOBAL) lpszTemp);
  }

  return lpszBuf;
}

// We'll try to install the service with this function, and save any
// runtime args for the service itself as a REG_SZ value in a registry 
// subkey
void InstallService(int argc, char **argv)
{
  SC_HANDLE   schService;
  SC_HANDLE   schSCManager;
  TCHAR szPath[512];

  // Get the full path and filename of this program
  if ( GetModuleFileName( NULL, szPath, 512 ) == 0 ) {
    TRACE(0, _T("Unable to install %s - %s\n"), SZSERVICEDISPLAYNAME, 
	  GetLastErrorText(szErr, 256));
    return;
  }

  // Next, get a handle to the service control manager
  schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if ( schSCManager ) 
  {
        schService = CreateService(
			           schSCManager,         // SCManager database
			           SZSERVICENAME,        // name of service
			           SZSERVICEDISPLAYNAME, // name to display
			           SERVICE_ALL_ACCESS,         // desired access
			           SERVICE_WIN32_OWN_PROCESS,  // service type
			           SERVICESTARTTYPE    ,       // start type
			           SERVICE_ERROR_NORMAL,  // error control type
			           szPath,                // service's binary
			           NULL,                  // no load ordering group
			           NULL,                  // no tag identifier
			           SZDEPENDENCIES,       // dependencies
			           NULL,                 // LocalSystem account
			           _T(""));              // no password
    
        if (schService) 
        {
          _tprintf(_T("%s installed.\n"), SZSERVICEDISPLAYNAME );

          // Close the handle to this service object
          CloseServiceHandle(schService);

        }
        else
        {
            _tprintf(TEXT("CreateService failed - %s\n"), GetLastErrorText(szErr, 256));
        }

        // Close the handle to the service control manager database
        CloseServiceHandle(schSCManager);
    }
    else
    {
        _tprintf(TEXT(SZSCMGRFAILURE), GetLastErrorText(szErr,256));
    }
}


// We'll try to stop, and then remove the service using this function.
void RemoveService()
{
  // First, get a handle to the service control manager
  const SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, 
					       SC_MANAGER_ALL_ACCESS);

  if (schSCManager) {
    // Next get the handle to this service...
    const SC_HANDLE schService = OpenService(schSCManager, SZSERVICENAME, 
					     SERVICE_ALL_ACCESS);

    if (schService) {
      // Poll the status of the service for SERVICE_STOP_PENDING
      if (QueryServiceStatus( schService, &ssStatus) && 
	  ssStatus.dwCurrentState == SERVICE_STOPPED ) {
	if(DeleteService(schService)) {
	  TRACE(0, _T("%s removed.\n"), SZSERVICEDISPLAYNAME);
	} else {
	  TRACE(0, _T("DeleteService failed - %s\n"), 
		GetLastErrorText(szErr,256));
	}
      } else {
	TRACE(0, _T("\n%s is not stopped, cannot remove service.\n"), 
	      SZSERVICEDISPLAYNAME);
      }

      //Close this service object's handle to the service control manager
      CloseServiceHandle(schService);
    } else {
      _tprintf(TEXT("OpenService failed - %s\n"), GetLastErrorText(szErr,256));
    }

    // Finally, close the handle to the service control manager's database
    CloseServiceHandle(schSCManager);
  } else {
    _tprintf(TEXT(SZSCMGRFAILURE), GetLastErrorText(szErr,256));
  }
}

// This function permits running the Java application from the 
// console.
void RunService(int argc, char ** argv)
{
  DWORD dwArgc;
  LPSTR *lpszArgv;

  dwArgc   = (DWORD) argc;
  lpszArgv = argv;

  TRACE(0, _T("Running %s.\n"), SZSERVICEDISPLAYNAME);

  //CreateEventLogSource();

  // Do it! But since this is a console start, skip the first two
  // arguments in the arg list being passed, and reduce its size by
  // two also. (The first two command line args should be ignored
  // in a console run.)
  ServiceStart( dwArgc - 2, lpszArgv + 2);
}


void CreateEventLogSource ()
{
    LONG result;
    HKEY hKey;
    DWORD disposition;
    DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE; 
    TCHAR eventDll [MAX_PATH];
    TCHAR* pFolderSeparator = NULL;
    GetModuleFileName (NULL, eventDll, MAX_PATH);
    pFolderSeparator = _tcsrchr (eventDll, L'\\');
    if (pFolderSeparator)
    {
        pFolderSeparator [0] = 0;
        pFolderSeparator = _tcsrchr (eventDll, L'\\');
    }
    if (pFolderSeparator)
    {
        *(pFolderSeparator + 1) = 0;
    }
    _tcsncat_s (eventDll, MAX_PATH, EVENT_MESSAGE_DLL, _TRUNCATE);

    result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
                                EVENT_VIEWER_KEY, 
                                0, 
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                (PHKEY)&hKey,
                                (LPDWORD) &disposition);

    if(result != ERROR_SUCCESS)
    {
        return;
    }

    result = RegSetValueEx(
        hKey,
        _T("EventMessageFile"), 
        (DWORD)0, 
        REG_EXPAND_SZ, 
        (LPBYTE) eventDll, 
        (DWORD)(_tcslen(eventDll) + 1) * sizeof (TCHAR));    


    result = RegSetValueEx(
        hKey,
        _T("TypesSupported"), 
        (DWORD)0, 
        REG_DWORD, 
        (LPBYTE) &dwData,  // pointer to value data 
        sizeof(DWORD));   // length of value data, 
        

    RegCloseKey(hKey);
}

void AddToEventLog(WORD type, DWORD eventId, LPCTSTR* lpszMsgs, WORD numStrings)
{
  HANDLE  hEventSource;

  hEventSource = RegisterEventSource(NULL, SZ_EVENT_VIEWER_SOURCE);

  if (hEventSource != NULL) {
    ReportEvent(hEventSource, 
		type,  
		0,                    
		eventId,                    
		NULL,                 
		numStrings,                    
		0,                    
		lpszMsgs,          
		NULL);     

    DeregisterEventSource(hEventSource);
  }

}

// Throughout the program, calls to SetServiceStatus are required
// which are handled by calling this function. Here, the non-constant
// members of the SERVICE_STATUS struct are assigned and SetServiceStatus
// is called with the struct. Note that we will not report to the service
// control manager if we are running as  console application.
BOOL ReportStatus(DWORD dwCurrentState,
                  DWORD dwWin32ExitCode,
                  DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL bResult = TRUE;

    if ( !bConsole ) 
    {
	ssStatus.dwControlsAccepted = SERVICE_ACCEPT_SESSIONCHANGE;

        ssStatus.dwCurrentState = dwCurrentState;
        ssStatus.dwWin32ExitCode = dwWin32ExitCode;
        ssStatus.dwWaitHint = dwWaitHint;

        if ( ( dwCurrentState == SERVICE_RUNNING ) ||
            ( dwCurrentState == SERVICE_STOPPED ) )
            ssStatus.dwCheckPoint = 0;
        else
            ssStatus.dwCheckPoint = dwCheckPoint++;
            
        bResult = SetServiceStatus( sshStatusHandle, &ssStatus);
       if (bResult != TRUE) 
        {
          ctrlmod_win::AddToMessageLog(TEXT("SetServiceStatus"));
        }
    }

    return bResult;
}

					    
// set up JVM methods for handleLogon() and handleLogoff()
// I do NOT clean up the objects allocated here because the appropriate cleanup time is when the process dies.
// When the process dies, everything is cleaned up anyway.
CEResult_t setupLogonLogoff(JNIEnv **jniEnv, jclass *cmStubClass, jmethodID *handleLogonMethod, jmethodID *handleLogoffMethod, jobject *cmStubObj)
{
    CEResult_t result = CE_RESULT_SUCCESS;
    
    TRACE(CELOG_DEBUG, _T("setupLogonLogoff() start [PDP_jvm = %x]\n"), PDP_jvm);

    jint res = PDP_jvm->AttachCurrentThread((void**)jniEnv, NULL);
    if (res<0) {
	TRACE(CELOG_ERR, _T("Cannot attach JNI to PDP master thread.\n"));
	result = CE_RESULT_GENERAL_FAILED;
	goto end;
    }

    //initalize class ControlManagerStub
    *cmStubClass = (*jniEnv)->FindClass(CONTROLMGR_STUB_CLASS);
    if (*cmStubClass == NULL) {
	TRACE(CELOG_ERR, _T("Cannot get control manager stub class.\n"));
	result = CE_RESULT_GENERAL_FAILED;
	goto end;
    }
    
    // get static method to get instance
    char buf[1024];
    _snprintf_s (buf, _countof(buf), _TRUNCATE, "()L%s;", CONTROLMGR_STUB_CLASS);  
    jmethodID getInstanceMethod = (*jniEnv)->GetStaticMethodID (*cmStubClass, 
							     CM_STUB_GETINSTANCE_M, 
							     buf);
    if(getInstanceMethod == NULL) {
	TRACE(CELOG_ERR, _T("Cannot get instance of control manager stub.\n"));
	result = CE_RESULT_GENERAL_FAILED;
	goto end;
    }

    //Get instance of ControlManagerStub
    *cmStubObj = (*jniEnv)->CallStaticObjectMethod(
					       *cmStubClass,
					       getInstanceMethod);
    if (*cmStubObj == NULL) {
	TRACE(CELOG_ERR, _T("Cannot call control manager stub instance method.\n"));
	result = CE_RESULT_GENERAL_FAILED;
	goto end;
    }
    //get handleLogon() method
    *handleLogonMethod = (*jniEnv)->GetMethodID (*cmStubClass,
						 CM_HANDLE_LOGON_EVENT_M,
						 "(Ljava/lang/String;)V");
    if (*handleLogonMethod == NULL) {
	TRACE(CELOG_ERR, _T("Cannot retrieve handle logon method\n"));
	result = CE_RESULT_GENERAL_FAILED;
	
	goto end;
    }
    
    //get handleLogoff() method
    *handleLogoffMethod = (*jniEnv)->GetMethodID (*cmStubClass,
						  CM_HANDLE_LOGOFF_EVENT_M,
						  "(Ljava/lang/String;)V");
    if (*handleLogoffMethod == NULL) {
	TRACE(CELOG_ERR, _T("Cannot retrieve handle logoff method\n"));
	result = CE_RESULT_GENERAL_FAILED;
	goto end;
    }
    result = CE_RESULT_SUCCESS;
 end:
    if (result != CE_RESULT_SUCCESS)
    	*jniEnv = NULL;
    
    return result;
}
    
// clean up JVM methods for handleLogon() and handleLogoff()
void cleanupLogonLogoff(JNIEnv *jniEnv, jclass cmStubClass, jobject cmStubObj)
{
    if (jniEnv != NULL) {
	if (cmStubObj != NULL)
	    jniEnv->DeleteLocalRef(cmStubObj);
	if (cmStubClass != NULL)
	    jniEnv->DeleteLocalRef(cmStubClass);
	PDP_jvm->DetachCurrentThread();
    }
}


// handle logon / logoff event
// IN lpEventData: event data from controlHandler()
// IN isLogon: TRUE means logon.  FALSE means logoff.
CEResult_t handleLogonLogoff(LPVOID lpEventData, BOOL isLogon)
{
    TRACE(CELOG_DEBUG, _T("handleLogonLogoff() start - checking if pdp up\n"));

    // Wait until the PDP is actually up and running
    nlthread_sem_wait(&pdpInitializedLock);
    nlthread_sem_post(&pdpInitializedLock);

    TRACE(CELOG_DEBUG, _T("handleLogonLogoff() pdp is up\n"));

    CEResult_t result = CE_RESULT_SUCCESS;
    TCHAR* pszSID = NULL;
    LPTSTR username = NULL, domain = NULL;
    // JNI variables
    JNIEnv *jniEnv = NULL;
    jclass cmStubClass = NULL;
    jmethodID handleLogonMethod = NULL, handleLogoffMethod = NULL; 
    jobject cmStubObj = NULL;
    jstring jarg = NULL;
    DWORD bytesReturned;
    
    // get user SID
    WTSSESSION_NOTIFICATION *notification = (WTSSESSION_NOTIFICATION *)lpEventData;

    BOOL rv = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, notification->dwSessionId,
					 WTSUserName, &username, &bytesReturned);
    if (rv == TRUE) {
	TRACE(CELOG_DEBUG, _T("User name: %s\n"), username);
    } else {
	TRACE(CELOG_ERR, _T("Error getting username: %d\n"), GetLastError());
	result = CE_RESULT_GENERAL_FAILED;
	goto end;
    }

    rv = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, notification->dwSessionId,
					 WTSDomainName, &domain, &bytesReturned);
    if (rv == TRUE) {
	TRACE(CELOG_DEBUG, _T("Domain: %s\n"), domain);
    } else {
	TRACE(CELOG_ERR, _T("Error getting domain: %d\n"), GetLastError());
	result = CE_RESULT_GENERAL_FAILED;
	goto end;
    }
    
    size_t pszUserBufLen = _tcslen (username) + _tcslen (domain) + 2;

    TCHAR* pszUser = new TCHAR [pszUserBufLen];
    _sntprintf_s (pszUser, pszUserBufLen, _TRUNCATE, _T("%s\\%s"), domain, username);

    GetUserSID (pszUser, &pszSID);
    if (pszSID == NULL) {
	TRACE(CELOG_ERR, _T("Error getting user SID for %s: %d\n"), pszUser, GetLastError());
	result = CE_RESULT_GENERAL_FAILED;
	goto end;
    }
    TRACE(CELOG_DEBUG, _T("User SID: %s\n"), pszSID);

    result = setupLogonLogoff(&jniEnv, &cmStubClass, &handleLogonMethod, &handleLogoffMethod, &cmStubObj);
    if (result != CE_RESULT_SUCCESS) {
	TRACE(CELOG_ERR, _T("Error setting up JNI methods\n"));
	jniEnv = NULL;
	goto end;
    }

    jarg = jniEnv->NewString((const jchar *)pszSID, (jsize)_tcslen(pszSID));
    if (jarg == NULL) {
	TRACE(CELOG_ERR, _T("Cannot allocate the argument to handleLogon\n"));
	result = CE_RESULT_GENERAL_FAILED;
	goto end;
    }

    if (isLogon == TRUE) {
	TRACE(CELOG_INFO, _T("Call handleLogon()\n"));
	jniEnv->CallVoidMethod(cmStubObj, handleLogonMethod, jarg);
        TRACE(CELOG_INFO, _T("Invoked handleLogonMethod()\n"));
	}else {
		TRACE(CELOG_INFO, _T("Call handleLogoff()\n"));
	jniEnv->CallVoidMethod(cmStubObj, handleLogoffMethod, jarg);
        TRACE(CELOG_INFO, _T("Invoked handleLogoffMethod()\n"));
    }

 end:
    // clean up
    if (username != NULL)
	WTSFreeMemory(username);
    if (domain != NULL)
	WTSFreeMemory(domain);
    if (pszSID != NULL)
	::LocalFree (pszSID);
    if (jniEnv != NULL) {
	if (jarg != NULL)
	    jniEnv->DeleteLocalRef(jarg);
	cleanupLogonLogoff(jniEnv, cmStubClass, cmStubObj);
    }
    
    return result;
}

// Each Win32 service must have a control handler to respond to
// control requests from the dispatcher.
// 3/29/2010: Changed to HandlerEx signature from Handler, so that this can receive SERVICE_CONTROL_SESSIONCHANGE
DWORD WINAPI controlHandler(DWORD dwCtrlCode,
			    DWORD dwEventType,
			    LPVOID lpEventData,
			    LPVOID lpContext
)
{
    TRACE(CELOG_DEBUG, _T("controlHandler(%d)\n"), dwCtrlCode);

    switch(dwCtrlCode)
    {
    case SERVICE_CONTROL_STOP:
        // Request to stop the service. Report SERVICE_STOP_PENDING
        // to the service control manager before calling ServiceStop()
        // to avoid a "Service did not respond" error.
        ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
        //ServiceStop(bConsole);

        return NO_ERROR;


    case SERVICE_CONTROL_INTERROGATE:
        // This case MUST be processed, even though we are not
        // obligated to do anything substantial in the process.
        break;

    case SERVICE_CONTROL_SESSIONCHANGE:
	TRACE(CELOG_DEBUG, _T("SERVICE_CONTROL_SESSIONCHANGE dwCtrlCode=%d, dwEventType=%d\n"), dwCtrlCode, dwEventType);

	if (dwEventType == WTS_SESSION_LOGON) {
	    handleLogonLogoff(lpEventData, TRUE);
	} else if (dwEventType == WTS_SESSION_LOGOFF) {
	    handleLogonLogoff(lpEventData, FALSE);
	}
	
	break;
	
    default:
        // Any other cases...
        break;

    }

    // After invocation of this function, we MUST call the SetServiceStatus
    // function, which is accomplished through our ReportStatus function. We
    // must do this even if the current status has not changed.
    ReportStatus(ssStatus.dwCurrentState, NO_ERROR, 0);

    return NO_ERROR;
}

// The ServiceMain function is the entry point for the service.
typedef struct _SVC_START_THREAD_PARAM
{
    
    int     argc;
    char**  argv;
}SVC_START_THREAD_PARAM, *PSVC_START_THREAD_PARAM;
SVC_START_THREAD_PARAM svcStartThreadParam;

static VOID ServiceStartThread(PSVC_START_THREAD_PARAM psvcstart_param)
{
  ServiceStart(psvcstart_param->argc, psvcstart_param->argv);

  // Exiting gracefully, otherwise the system is going to 
  // restart the service
  if (sshStatusHandle) {  
    ReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
  }
}

void WINAPI serviceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
  char szAppParameters[10000] = { 0 };
  char serviceName[512] = { 0 };
  LPSTR *lpszNewArgv = NULL;
  DWORD dwNewArgc=0;

  CreateEventLogSource();
  WideCharToMultiByte(CP_ACP,0,lpszArgv[0], -1, serviceName, 512, NULL, NULL);

  BOOL bStartSuccess = FALSE;
  __try
  {
      // 3/30/2010: Use Ex version to change signature of handler from Handler to HandlerEx, so that it can receive SERVICE_CONTROL_SESSIONCHANGE
      sshStatusHandle = RegisterServiceCtrlHandlerEx( lpszArgv[0], controlHandler, NULL);

      if(!sshStatusHandle) __leave;


      // the service is pendding
      ssStatus.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
      ssStatus.dwCurrentState=SERVICE_START_PENDING;   
      ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE;
      ssStatus.dwWin32ExitCode=NO_ERROR;   
      ssStatus.dwCheckPoint=0;   
      ssStatus.dwWaitHint=0;
      SetServiceStatus(sshStatusHandle,&ssStatus);

      char* pCommandLine = GetCommandLineA();
      const char* szName = "cepdpman.exe";
      char* strParam = strstr(pCommandLine, szName);
      const char* strFakeParam = "Start Service: parse parameter failed, serviceMain parameter should be like below:-Djava.class.path=\"C:\\Program Files\\NextLabs\\Policy Controller\\jlib\\agent-controlmanager.jar;\" -Djava.library.path=\"C:\\Program Files\\NextLabs\\Policy Controller\\bin\\\" -Djava.util.logging.config.file=\"C:\\Program Files\\NextLabs\\Policy Controller\\config\\logging.properties\" -Xmx512m -Xrs wrkdir=\"C:\\Program Files\\NextLabs\\Policy Controller\\\" DESKTOP none.\n";
      if (strParam == NULL)
      {
          dwNewArgc = 0;
          lpszNewArgv = NULL;
          OutputDebugStringA(strFakeParam);
      }
      else
      {
#if 1
          strParam += strlen(szName) + 2;
#else
          strParam = "-Djava.class.path=\"C:\\Program Files\\NextLabs\\Policy Controller\\jlib\\agent-controlmanager.jar;\" -Djava.library.path=\"C:\\Program Files\\NextLabs\\Policy Controller\\bin\\\" -Djava.util.logging.config.file=\"C:\\Program Files\\NextLabs\\Policy Controller\\config\\logging.properties\" -Xmx512m -Xrs wrkdir=\"C:\\Program Files\\NextLabs\\Policy Controller\\\" DESKTOP none";
#endif
          OutputDebugStringA(strParam);
          strcpy_s(szAppParameters, strParam);
          lpszNewArgv = convertArgStringToArgList(lpszNewArgv, &dwNewArgc, szAppParameters);
      }

      // Start service
      svcStartThreadParam.argc = dwNewArgc;
      svcStartThreadParam.argv = lpszNewArgv;
      HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServiceStartThread, (LPVOID)(&svcStartThreadParam), 0, NULL);
      if(NULL != hThread)
      {
          CloseHandle(hThread);
          hThread = 0;
          bStartSuccess = TRUE;
      }
  }
  __finally
  {
      ssStatus.dwServiceType     = SERVICE_WIN32_OWN_PROCESS;
      ssStatus.dwCurrentState    = bStartSuccess?SERVICE_RUNNING:SERVICE_STOPPED;   
      ssStatus.dwControlsAccepted = SERVICE_ACCEPT_SESSIONCHANGE;
      ssStatus.dwWin32ExitCode   = NO_ERROR;   
      ssStatus.dwCheckPoint      = 0;   
      ssStatus.dwWaitHint        = 0;
      if (sshStatusHandle) SetServiceStatus(sshStatusHandle,&ssStatus);
  }
}


// Display instructions for use...

void printUsage()
{
    printf("\nUsage:\n\n");
    wprintf(_T("%s <options> [<-D|-X>JavaVMArg1 <-D|-X>JavaVMArg2...] [JavaAppArg1 JavaAppArg2...] [wrkdir=pathname]\n\n"),
        SZAPPNAME);
    printf("Options:\n\n");
    printf("-i  to install the service\n");
    printf("-r  to remove the service\n");
    printf("-c  to run as a console app\n\n");
    printf("-D  precedes arguments to be passed to the Java VM\n");
    printf("-X  precedes arguments to be passed to the Java VM\n");
    printf("wrkdir=path to working directory. winnt/system32 is the default\n");
    printf("       if none is specified.\n\n");
    printf("All other arguments are passed to the Java application.\n\n");
    printf("Example:\n\n");
    wprintf(_T("%s -i -Djava.class.path=c:\\myapp\\lib\\myapp.jar apparg1 wrkdir=c:\\myapp\\support\n\n"), SZAPPNAME);
    printf("The above installs the coded Java app as a service, supplies the -D prefixed\n");
    printf("arguments to the JVM, and changes to the specified working directory. Other\n");
    printf("arguments are supplied to the Java app's main class when the service starts.\n");
}
}


using namespace ctrlmod_win;


//Entry point for control module (win32)
int  main(int argc, char **argv)
{
  // The StartServiceCtrlDispatcher requires this table to specify
  // the ServiceMain function to run in the calling process. The first
  // member in this example is actually ignored, since we will install
  // our service as a SERVICE_WIN32_OWN_PROCESS service type. The NULL
  // members of the last entry are necessary to indicate the end of 
  // the table;
  SERVICE_TABLE_ENTRY serviceTable[] = {
    { SZSERVICENAME, (LPSERVICE_MAIN_FUNCTION)serviceMain },
    { NULL, NULL }
  };

  if(NULL == strstr((LPSTR) SZPARAMKEY, (LPSTR) SZSERVICENAME)) {
    TRACE(0, _T("\nSZSERVICENAME service name does not match SZPARAMKEY\n"));
    TRACE(0, _T("Please correct this in service.h and recompile.\n"));
    exit(0);
  }

  if (_stricmp(argv[1], ("install")) == 0 || _stricmp(argv[1], "-i") ==0 
      || _stricmp(argv[1], "/i") == 0 || _stricmp(argv[1], ("-install")) == 0
      || _stricmp(argv[1], ("/install")) == 0)
  {
      InstallService(argc, argv);
      return 0;
  }
  else if (!_stricmp(argv[1], "-?") || !_stricmp(argv[1], "/?")) {
      printUsage();
      return 0;
  }

  // If main is called without any arguments, it will probably be by the
  // service control manager, in which case StartServiceCtrlDispatcher
  // must be called here. A message will be printed just in case this 
  // happens from the console.

  printf("\nCalling StartServiceCtrlDispatcher...please wait.\n");
  if (!StartServiceCtrlDispatcher(serviceTable)) {
    TRACE(0, _T("\n%s\n"), SZFAILURE);    
    TRACE(0, _T("\nFor help, type\n\n%s /?\n\n"), SZAPPNAME);
    AddToMessageLog(TEXT(SZFAILURE));
  }
  return (0);
}
