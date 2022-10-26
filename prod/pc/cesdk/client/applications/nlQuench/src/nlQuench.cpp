#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <psapi.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#include "quenchhash.h"

namespace {
#define CE_POLICY_CONTROLLER_UUID "b67546e2-6dc7-4d07-aa8a-e1647d29d4d7"
FILE *logFp=NULL;
WCHAR nlDir[MAX_PATH];  

void  AddTimestampToLog()
{
  /* Add timestamp to log message */
  struct _timeb timebuffer;
  char timeline[32];
   _ftime64_s(&timebuffer);
  if( ctime_s(timeline,_countof(timeline),&timebuffer.time) == 0 ) {
    timeline[strlen(timeline)-1] = (char)NULL;  /* remove newline */
    fwprintf_s(logFp,L"%.19hs.%03hu %hs: ",
	       timeline,timebuffer.millitm,&timeline[20]);
  }
}

bool IsTamperResistanceDown() 
{

  WCHAR sDir[MAX_PATH];
  _snwprintf_s(sDir,
	     _countof(sDir), _TRUNCATE,
	     L"%sPolicy Controller\\config\\tamper_resistance\\*",
	     nlDir);
  AddTimestampToLog();
  fwprintf_s(logFp, 
	     L"IsTamperResistanceDown: try to access directory %s\n",nlDir);

  WIN32_FIND_DATA ffd;
  
  HANDLE hFind = INVALID_HANDLE_VALUE;

  int count=100;
  DWORD start_time = GetTickCount();
  DWORD runtime=0;

  //Try 100 times to retrieve the directory to 
  //see if tamper resistance is down.  
  while(count) {
    hFind = FindFirstFile(sDir, &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {
      AddTimestampToLog();
      fwprintf_s(logFp, 
     L"IsTamperResistanceDown: No.%d try to access directory %s failed: %d\n", 
		 100-count+1, sDir, GetLastError());
      --count;
      Sleep(100);
    } else {
      runtime=GetTickCount()-start_time;
      FindClose(hFind);
      AddTimestampToLog();
      fwprintf_s(logFp, 
     L"IsTamperResistanceDown: Able to access directory %s after %d tries in %d ms \n", 
		 sDir, 100-count+1, runtime);
      return true;
    }
  }
  
  return false;
}

bool CheckIfPolicyController( DWORD processID )
{
  wchar_t szProcessName[MAX_PATH] = L"<unknown>";
  bool bFound=false;

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );

    // Get the process name.
    if (NULL != hProcess )
    {
        if (GetProcessImageFileNameW(hProcess,
                                     szProcessName,
                                     MAX_PATH) == 0) {
            fwprintf(logFp, L"  %u: <unknown> (%d)\n", processID, ::GetLastError());
        } else {
            fwprintf_s(logFp, L"  %u: %s\n", processID, szProcessName);
        }
        CloseHandle( hProcess );
    } else {
        fwprintf_s(logFp, L"  %u: %s (%ld)\n", processID, L"Unable to open process", (long int)::GetLastError());
    }

    //compare the process name.
    if(_tcsstr(szProcessName, _T("cepdpman.exe")))
	 bFound=true;

    return bFound;
}

DWORD EnumAllProcessGetPid()
{
  // Get the list of process identifiers.
  DWORD aProcesses[1024], cbNeeded, cProcesses;
  unsigned int i;

  if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
    return static_cast<DWORD>(-1);

  // Calculate how many process identifiers were returned.
  cProcesses = cbNeeded / sizeof(DWORD);

  // Print the name and process identifier for each process.
  for ( i = 0; i < cProcesses; i++ )
    if( aProcesses[i] != 0 )
      if(CheckIfPolicyController(aProcesses[i]))
	return aProcesses[i];

  return static_cast<DWORD>(-1);
}

/* Get Policy Controller PID */
DWORD get_policy_controller_pid(void)
{
  HANDLE hPIDFileMapping;

  hPIDFileMapping = OpenFileMappingA(FILE_MAP_READ, FALSE, CE_POLICY_CONTROLLER_UUID); // name of mapping object 

  if (hPIDFileMapping == NULL) {
      DWORD err = ::GetLastError();
      AddTimestampToLog();
      fwprintf_s(logFp,L"OpenFileMapping failed %d\n", err);
      return static_cast<DWORD>(-1);
  }

  DWORD* pid;
  DWORD  pidV=static_cast<DWORD>(-1);

  pid = (DWORD*)MapViewOfFile(hPIDFileMapping,FILE_MAP_READ,0,0,0);
  if( pid != NULL )
  {
    pidV=*pid;
    UnmapViewOfFile(pid);
  }
  else 
  {
      DWORD err = ::GetLastError();
      AddTimestampToLog();
      fwprintf_s(logFp, L"MapViewOfFile failed %d\n", err);
  }

  CloseHandle(hPIDFileMapping);

  return pidV;

}/* is_policy_controller */

bool GenQuenchFile()
{
  /* Determine NextLabs root directory */
  LONG rstatus;
  HKEY hKey = NULL; 
  rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
	  TEXT("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller"),
			  0,KEY_QUERY_VALUE,&hKey);
  if( rstatus != ERROR_SUCCESS )
  {
    AddTimestampToLog();
    fwprintf_s(logFp,L"GenQuenchFile: cannot read root path\n");
    return false;
  }

  //Get InstallDir
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
    AddTimestampToLog();
    fwprintf_s(logFp,L"GenQuenchFile: cannot read InstallDir.\n");
    return false;
  }

  WCHAR qFile[512];
  _snwprintf_s(qFile, _countof(qFile), _TRUNCATE, L"%snlQuench.txt", nlDir);
  FILE *fp = NULL;
  _wfopen_s(&fp, qFile, L"w");
  if (fp == NULL)
  {
  	AddTimestampToLog();
    fwprintf_s(logFp,
	       L"GenQuenchFile: failed to create quench file %s (%d)\n", 
	    qFile, GetLastError());
    return false;
  }

  fputs(generateData(QUENCH_SHARED_SECRET).c_str(), fp);
  fclose(fp);
  AddTimestampToLog();
  fwprintf_s(logFp,L"GenQuenchFile succeeded: %s\n", qFile);
  return true;
}
}

int main(int argc, char **argv) 
{
  WCHAR lpPathBuffer[MAX_PATH];
  WCHAR lpFNameBuffer[MAX_PATH*2];
  
  // Get the temp path.
  DWORD dwRetVal = GetTempPathW(MAX_PATH, //length of the buffer
                         lpPathBuffer); // buffer for path 
  if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
    AddTimestampToLog();
    fwprintf_s (stderr, L"GetTempPath failed (%d)\n", GetLastError());
    logFp=stderr;
  } else {
    _snwprintf_s(lpFNameBuffer, _countof(lpFNameBuffer), _TRUNCATE,
	     L"%snlQuenchLog.txt", lpPathBuffer);
    wprintf(L"nlQuench log file: %s\n", lpFNameBuffer);
    errno_t err = _wfopen_s(&logFp, lpFNameBuffer, L"w");
	if (0 != err)
	{
      wprintf(L"Failed to generate log file %s error=%d\n", 
	      lpFNameBuffer, GetLastError());
      logFp=stderr;
    }
  }
  
  DWORD pcPid=get_policy_controller_pid();

  if(pcPid == -1) {
    AddTimestampToLog();
    fwprintf_s(logFp, L"Call EnumAllProcess to get Policy Controller pid\n");
    pcPid=EnumAllProcessGetPid();
  }

  AddTimestampToLog();
  fwprintf_s(logFp, L"Get Policy Controller pid=%d\n", pcPid);    
  if(pcPid != -1) {
    HANDLE  hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pcPid);
    if( hProcess == NULL ) {
      AddTimestampToLog();
      fwprintf_s(logFp, L"OpenProcess failed: %d\n", GetLastError());
      return -1;
    } else {
      if(GenQuenchFile()) {
	if(!TerminateProcess(hProcess, 0)) {
	  AddTimestampToLog();
	  fwprintf_s(logFp, 
		     L"TerminateProcess failed: %d\n", GetLastError());
	  return -1;
	} else {
	  if(IsTamperResistanceDown()) {
	    AddTimestampToLog();
	    fwprintf_s(logFp, L"nlQuench succeed\n");
	  } else {
	    AddTimestampToLog();
	    fwprintf(logFp, 
		     L"Tamper resistance is still up. nlQuench failed\n");
	    return -1;
	  }
	}
      } else
	return -1;
      CloseHandle(hProcess);
    }    
  } //else process not exists

  return 0;
}
