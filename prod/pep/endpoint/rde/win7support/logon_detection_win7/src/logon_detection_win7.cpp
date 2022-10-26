// logon_detection_win7.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <vector>
#include <Tlhelp32.h>
#include <algorithm>
#include <sddl.h>

using namespace std;

static vector<DWORD> GetRunningProcessList(const wchar_t* pstrProcess)
{
	vector<DWORD> vectorPID;
	HANDLE hProcessSnap;
	PROCESSENTRY32W pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		return vectorPID;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32W );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32FirstW( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );
		return vectorPID;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		if(_wcsicmp(pe32.szExeFile, pstrProcess) == 0)
		{
			vectorPID.push_back(pe32.th32ProcessID);
		}
	}
	while( Process32NextW( hProcessSnap, &pe32 ) );

	CloseHandle(hProcessSnap);
	return vectorPID;
}

static BOOL GetUserSID (HANDLE hToken, PSID *ppsid) 
{
	DWORD dwSize = 1000;
	TOKEN_USER* pUser = NULL;
	
	BYTE szTemp[1000] = {0};
	pUser = (TOKEN_USER*)szTemp;
	if(pUser)
	{
		if(GetTokenInformation(hToken, TokenUser, pUser, dwSize, &dwSize))
		{
			DWORD dwLength = GetLengthSid(pUser->User.Sid);
			*ppsid = (PSID) HeapAlloc(GetProcessHeap(),
				HEAP_ZERO_MEMORY, dwLength);

			CopySid(dwLength, *ppsid, pUser->User.Sid);

			
			return TRUE;
		}
		
	}
	
	return FALSE;
}

typedef void (*device_query_type)(LPCWSTR lpszSid);

bool GetInstallPath(wchar_t* pPath, DWORD dwLen)
{
	if( !pPath )
	{
		return false;
	}

	LONG rstatus = 0;
	HKEY hKey = NULL; 

	rstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
				  "SOFTWARE\\NextLabs\\Compliant Enterprise\\Removable Device Enforcer",
				  0,KEY_QUERY_VALUE,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		return false;
	}

	WCHAR rde_root[MAX_PATH + 1] = {0};  /* InstallDir */
	DWORD rde_root_size = sizeof(rde_root);

	rstatus = RegQueryValueExW(hKey,L"InstallDir",NULL,NULL,(LPBYTE)rde_root,&rde_root_size);
	RegCloseKey(hKey);

	wcsncpy_s(pPath, dwLen, rde_root, _TRUNCATE);

	return true;
}

void Call_DevQuery(LPCWSTR lpszSid)
{
	wchar_t szDevEnfPath[MAX_PATH + 1] = {0};
	if(GetInstallPath(szDevEnfPath, MAX_PATH))
	{
#ifdef _WIN64
		wcsncat_s(szDevEnfPath, sizeof(szDevEnfPath)/sizeof(wchar_t), L"\\bin\\nl_devenf_plugin.dll", _TRUNCATE);
#else
		wcsncat_s(szDevEnfPath, sizeof(szDevEnfPath)/sizeof(wchar_t), L"\\bin\\nl_devenf_plugin32.dll", _TRUNCATE);
#endif 

		{
			HMODULE hMod = LoadLibraryW(szDevEnfPath);
			if(hMod)
			{
				device_query_type pdevice_query = (device_query_type)GetProcAddress(hMod, "device_query");
				if(pdevice_query)
				{
					pdevice_query(lpszSid);//call to do evaluation for device
				}
				FreeLibrary(hMod);
			}
		}


	}
}

void FreeSID(PSID * ppsid)
{
	HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
}

int _tmain(int argc, _TCHAR* argv[])
{
	argc;
	argv;
	size_t uCount = 0xFFFFFFFF;
	
	vector<DWORD> vExistingProcesses;
	bool bFinished = false;
	while(!bFinished)
	{
		vector<DWORD> vExplorers = GetRunningProcessList(L"explorer.exe");

		if(uCount == 0xFFFFFFFF)
		{
			uCount = vExplorers.size();
			vExistingProcesses = vExplorers;
		}
		else
		{
			if(uCount != vExplorers.size())
			{//There are new "explore" processes were created.

				for(vector<DWORD>::iterator itExplorer = vExplorers.begin(); itExplorer != vExplorers.end(); ++itExplorer)
				{
					bFinished = true;

					if(find(vExistingProcesses.begin(), vExistingProcesses.end(), *itExplorer) != vExistingProcesses.end())
					{//this process has been handled, don't need to handle again.
						continue;
					}
					
					//try to get the sid for current process
					HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, *itExplorer);
					if(hProcess != NULL)
					{
						HANDLE hToken = NULL;
						if(OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
						{
							PSID psid = NULL;

							if(GetUserSID(hToken, &psid) && psid != NULL)
							{
								LPWSTR strSID = 0;
								if(ConvertSidToStringSid(psid, &strSID))//get the sid of user of current process
								{
									FreeSID(&psid);

									wchar_t wzSid[MAX_PATH] = {0};
									wcsncpy_s(wzSid, MAX_PATH, strSID, _TRUNCATE);
									if(strSID) LocalFree(strSID);

									Call_DevQuery(wzSid);//Call nl_devenf_plugin.dll to do evaluation
								}
								
							}
						}
						CloseHandle(hProcess);
					}				
				}
			}
		}
		Sleep(100);	
	}

	return 0;
}

