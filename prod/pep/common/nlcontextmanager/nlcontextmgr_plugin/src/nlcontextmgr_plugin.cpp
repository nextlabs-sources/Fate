// nlcontextmgr_plugin.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <vector>
#include <Tlhelp32.h>

using namespace std;

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
		if(_wcsicmp(pe32.szExeFile, pstrProcess) == 0)
		{
			vectorPID.push_back(pe32.th32ProcessID);
		}
	}
	while( Process32Next( hProcessSnap, &pe32 ) );

	CloseHandle(hProcessSnap);
	return vectorPID;
}

/****************************************************************************
* Plug-in Entry Points
***************************************************************************/
extern "C" __declspec(dllexport) int PluginEntry( void** in_context )
{
	in_context;

	vector<DWORD> vProcess = GetRunningProcessList(L"nlcontextmgr.exe");
	if (vProcess.size() > 0)//it means nlcontextmgr.exe is running already
	{
		return 1;
	}
	

	LONG rstatus;
	HKEY hKey = NULL; 

	rstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\NextLabs\\CommonLibraries",
		0,KEY_QUERY_VALUE,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		return 0;
	}

	WCHAR path[MAX_PATH] = {0};                 /* InstallDir */
	DWORD path_len = sizeof(path);

	rstatus = RegQueryValueExW(hKey,L"InstallDir",NULL,NULL,(LPBYTE)path,&path_len);
	RegCloseKey(hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		return 0;
	}

#ifdef _WIN64
	wcsncat_s(path, MAX_PATH, L"\\bin64\\nlcontextmgr.exe", _TRUNCATE);
#else
	wcsncat_s(path, MAX_PATH, L"\\bin32\\nlcontextmgr.exe", _TRUNCATE);
#endif


	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb= sizeof(STARTUPINFO);
	si.lpDesktop = TEXT("winsta0\\default");
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	BOOL bResult = CreateProcessW(path, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
	
	if(bResult)
	{
		if(pi.hProcess != INVALID_HANDLE_VALUE)
			CloseHandle(pi.hProcess);
		if(pi.hThread != INVALID_HANDLE_VALUE)
			CloseHandle(pi.hThread);
	}
	

	return 1;
}/* PluginEntry */

/*****************************************************************************
* PluginUnload
****************************************************************************/
extern "C" __declspec(dllexport) int PluginUnload( void* in_context )
{
	in_context;

	return 1;
}/* PluginUnload */