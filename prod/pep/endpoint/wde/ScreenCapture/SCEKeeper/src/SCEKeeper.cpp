// SCEKeeper.cpp :
//

#include "stdafx.h"

#define REG_KEY_INSTALL_PATH   "SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer"
#define REG_KEY_INSTALL        L"InstallDir"
#define NLSCENAME              L"nlsce.exe"

HANDLE                  ghSvcStopEvent = NULL;

vector<DWORD> GetRunningProcessList(const TCHAR* pstrProcess)
{
	vector<DWORD> vectorPID;
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		return vectorPID;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );
		return vectorPID;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		if(_tcsicmp(pe32.szExeFile, pstrProcess) == 0)
		{
			vectorPID.push_back(pe32.th32ProcessID);
		}
	}
	while( Process32Next( hProcessSnap, &pe32 ) );

	CloseHandle(hProcessSnap);
	return vectorPID;
}

BOOL GetLogonSID (HANDLE hToken, PSID *ppsid) 
{
   BOOL bSuccess = FALSE;
   DWORD dwIndex;
   DWORD dwLength = 0;
   PTOKEN_GROUPS ptg = NULL;

// Verify the parameter passed in is not NULL.
    if (NULL == ppsid)
        goto Cleanup;

// Get required buffer size and allocate the TOKEN_GROUPS buffer.

   if (!GetTokenInformation(
         hToken,         // handle to the access token
         TokenGroups,    // get information about the token's groups 
         (LPVOID) ptg,   // pointer to TOKEN_GROUPS buffer
         0,              // size of buffer
         &dwLength       // receives required buffer size
      )) 
   {
      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
         goto Cleanup;

      ptg = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(),
         HEAP_ZERO_MEMORY, dwLength);

      if (ptg == NULL)
         goto Cleanup;
   }

// Get the token group information from the access token.

   if (!GetTokenInformation(
         hToken,         // handle to the access token
         TokenGroups,    // get information about the token's groups 
         (LPVOID) ptg,   // pointer to TOKEN_GROUPS buffer
         dwLength,       // size of buffer
         &dwLength       // receives required buffer size
         )) 
   {
      goto Cleanup;
   }

// Loop through the groups to find the logon SID.

   for (dwIndex = 0; ptg && dwIndex < ptg->GroupCount; dwIndex++) 
      if ((ptg->Groups[dwIndex].Attributes & SE_GROUP_LOGON_ID)
             ==  SE_GROUP_LOGON_ID) 
      {
      // Found the logon SID; make a copy of it.

         dwLength = GetLengthSid(ptg->Groups[dwIndex].Sid);
         *ppsid = (PSID) HeapAlloc(GetProcessHeap(),
                     HEAP_ZERO_MEMORY, dwLength);
         if (*ppsid == NULL)
             goto Cleanup;
         if (!CopySid(dwLength, *ppsid, ptg->Groups[dwIndex].Sid)) 
         {
             HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
             goto Cleanup;
         }
         break;
      }

   bSuccess = TRUE;

Cleanup: 

// Free the buffer for the token groups.

   if (ptg != NULL)
      HeapFree(GetProcessHeap(), 0, (LPVOID)ptg);

   return bSuccess;
}

VOID FreeLogonSID (PSID *ppsid) 
{
    HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
}

//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//
unsigned int __stdcall SvcInit(void*)
{

	LONG rstatus;
	HKEY hKey = NULL; 

	rstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
		REG_KEY_INSTALL_PATH,
		0,KEY_QUERY_VALUE,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		return 0;
	}

	WCHAR enforcer_root[MAX_PATH];                 /* InstallDir */
	DWORD enforcer_root_size = sizeof(enforcer_root);

	rstatus = RegQueryValueExW(hKey,REG_KEY_INSTALL,NULL,NULL,(LPBYTE)enforcer_root,&enforcer_root_size);
	RegCloseKey(hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		return 0;
	}

	WCHAR scepath[MAX_PATH];
	_snwprintf_s(scepath, MAX_PATH, _TRUNCATE, L"%sbin\\%s", enforcer_root, NLSCENAME);


    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.

    ghSvcStopEvent = CreateEvent(
                         NULL,    // default security attributes
                         TRUE,    // manual reset event
                         FALSE,   // not signaled
                         NULL);   // no name

    if ( ghSvcStopEvent == NULL)
    {
        return 0;
    }


    // TO_DO: Perform work until service stops.
	for(;;)
	{
		DWORD dwEvent = WaitForSingleObject(ghSvcStopEvent, 2000);
		if(dwEvent == WAIT_TIMEOUT)
		{
			vector<PSID> vectorPSIDExplorer;
			vector<HANDLE> vectorTokenExplorer;
			vector<DWORD> vectorExplorer = GetRunningProcessList(TEXT("explorer.exe"));
			for(vector<DWORD>::iterator itExplorer = vectorExplorer.begin(); itExplorer != vectorExplorer.end(); ++itExplorer)
			{
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, *itExplorer);
				if(hProcess != NULL)
				{
					HANDLE hToken = NULL;
					if(OpenProcessToken(hProcess, TOKEN_QUERY|TOKEN_DUPLICATE|TOKEN_ASSIGN_PRIMARY, &hToken))
					{
						PSID psid;
						if(GetLogonSID(hToken, &psid) && psid != NULL)
						{
							vectorTokenExplorer.push_back(hToken);
							vectorPSIDExplorer.push_back(psid);
						}
					}
					CloseHandle(hProcess);
				}				
			}

			vector<DWORD> vectorSCE = GetRunningProcessList(NLSCENAME);
			for(DWORD dwIndex = 0; dwIndex < vectorPSIDExplorer.size(); ++dwIndex)
			{
				PSID psidExpl = vectorPSIDExplorer[dwIndex];
				vector<DWORD>::iterator itSCE = vectorSCE.begin();
				for(; itSCE != vectorSCE.end(); ++itSCE)
				{
					HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, *itSCE);
					if(hProcess != NULL)
					{
						HANDLE hToken = NULL;
						if(OpenProcessToken(hProcess, TOKEN_QUERY , &hToken))
						{
							PSID psid;
							if(GetLogonSID(hToken, &psid) && psid != NULL)
							{
								if(EqualSid(psid, psidExpl))
								{
									FreeLogonSID(&psid);
									CloseHandle(hToken);
									CloseHandle(hProcess);
									break;
								}
								FreeLogonSID(&psid);
							}
							CloseHandle(hToken);
						}
						CloseHandle(hProcess);
					}				
				}
				if(itSCE == vectorSCE.end())
				{
					PROCESS_INFORMATION pi;
					STARTUPINFO si;
					ZeroMemory(&si, sizeof(STARTUPINFO));
					si.cb= sizeof(STARTUPINFO);
					si.lpDesktop = TEXT("winsta0\\default");
					si.dwFlags = STARTF_USESHOWWINDOW;
					si.wShowWindow = SW_HIDE;

					BOOL bResult = ::CreateProcessAsUser(vectorTokenExplorer[dwIndex],
														 scepath,
														 NULL,
														 NULL,
														 NULL,
														 FALSE,
														 NORMAL_PRIORITY_CLASS,
														 NULL,
														 NULL,
														 &si,
														 &pi);
					if(bResult)
					{
						if(pi.hProcess != INVALID_HANDLE_VALUE)
							CloseHandle(pi.hProcess);
						if(pi.hThread != INVALID_HANDLE_VALUE)
							CloseHandle(pi.hThread);
					}
				}
				FreeLogonSID(&vectorPSIDExplorer[dwIndex]);
				CloseHandle(vectorTokenExplorer[dwIndex]);
			}
		}
		else
		{
            vector<DWORD> SCEProcessIDs = GetRunningProcessList(NLSCENAME);
			for(vector<DWORD>::iterator it = SCEProcessIDs.begin(); it != SCEProcessIDs.end(); ++it)
			{
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, *it);
				if(hProcess != NULL)
				{
					TerminateProcess(hProcess, 0);
					CloseHandle(hProcess);
				}
			}

			CloseHandle(ghSvcStopEvent);
			return 0;
}
    }
}