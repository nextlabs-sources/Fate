// uccpPEP.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#pragma warning(push)
#pragma warning(disable:4100)
#include "brain.h"
#pragma warning(pop)


#include "platform.h"
#include "Blocks.h"

#include "celog.h"
extern CELog g_log;


#include "Helper.h"


#pragma warning(push)
#pragma warning(disable: 4819)  // We won't fix code page issue in MADCHOOK header file, just ignore it here
#include "madCHook_helper.h"
#pragma warning(pop)



#ifdef _MANAGED
#pragma managed(push, off)
#endif

namespace {
//---------------------------------------------------------------------------
// GetProcessHostFullName
//
// Return the path and the name of the current process
//---------------------------------------------------------------------------
BOOL GetProcessHostFullName(WCHAR* pszFullFileName)
{
	DWORD dwResult = 0;
	::ZeroMemory((PBYTE)pszFullFileName, sizeof(WCHAR) * MAX_PATH);
	if (TRUE != ::IsBadReadPtr((PBYTE)pszFullFileName, sizeof(WCHAR) * MAX_PATH))
		dwResult = ::GetModuleFileNameW(
			NULL,                   // handle to module
			pszFullFileName,        // file name of module
			MAX_PATH                // size of buffer
			);

	return (dwResult != 0);
}
//---------------------------------------------------------------------------
// GetProcessHostName
//
// Return the name of the current process
//---------------------------------------------------------------------------
BOOL GetProcessHostName(WCHAR* pszFullFileName)
{
	BOOL  bResult;

	bResult = GetProcessHostFullName(pszFullFileName);
	return bResult;
}
}

extern bool __stdcall HOOK_UCCP_CoCreateInstance(
	REFCLSID rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD dwClsContext,
	REFIID riid,
	LPVOID * ppv,
	HRESULT hResult
	);

HRESULT (WINAPI* NextCoCreateInstance)(
									   REFCLSID rclsid,
									   LPUNKNOWN pUnkOuter,
									   DWORD dwClsContext,
									   REFIID riid,
									   LPVOID * ppv
									   );

HRESULT WINAPI try_BJCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv )
{
	HRESULT hr = 0;
	hr = NextCoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv);

	HOOK_UCCP_CoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv,hr);

	return hr;
}

// Hook WinAPI SetClipboardData, Added By Jacky.Dong 2011-12-26
HANDLE (WINAPI* NextSetClipboardData)(UINT uFormat, HANDLE hMem);

HANDLE WINAPI try_SetClipboardData(UINT uFormat, HANDLE hMem)
{
	HANDLE hr = NULL;
	if(true == DoSetClipboardDataEval())
	{
		hr = NextSetClipboardData(uFormat, hMem);
	}
	return hr;
}

// Hook WinAPI CreateFileW, Added By Jacky.Dong 2011-12-02
// -------------------------------------------------------
HANDLE (WINAPI* NextCreateFileW)(
								 LPCWSTR lpFileName,
								 DWORD dwDesiredAccess,
								 DWORD dwShareMode,
								 LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								 DWORD dwCreationDisposition,
								 WORD dwFlagsAndAttributes,
								 HANDLE hTemplateFile
								 );

HANDLE WINAPI try_CreateFileW(
							  LPCWSTR lpFileName,
							  DWORD dwDesiredAccess,
							  DWORD dwShareMode,
							  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							  DWORD dwCreationDisposition,
							  WORD dwFlagsAndAttributes,
							  HANDLE hTemplateFile
							  )
{
	if(!(wcsstr(lpFileName, L"Microsoft Office Communicator") || wcsstr(lpFileName, L"WINDOWS\\system32"))) {
			//We are not interested in OC's own files
			AddFileMappingForPolicyEval(GetCurrentThreadId(), lpFileName, NULL);	
	}

	return NextCreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile
		);
}
// -------------------------------------------------------

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	lpReserved;
	hModule;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			OutputDebugStringA("enter oce");
					
					
			WCHAR  m_szProcessName[MAX_PATH];
			//
			// Get the name of the current process
			//
			GetProcessHostName(m_szProcessName);
			
			
			//Initialization for policy evaluation. If it fails, we unload 
			//hookUccp.dll from this process
			if(InitPolicyEval(m_szProcessName) != TRUE)
				return FALSE;
			
			InitLog();
			
			BOOL bResult = HookAPI ("Ole32.DLL", "CoCreateInstance", try_BJCoCreateInstance, (PVOID*)&NextCoCreateInstance, 0);//HookAPI ("Ole32.DLL", "CoCreateInstance", try_BJCoCreateInstance, (PVOID*)&NextCoCreateInstance, 0);
			if( bResult == 0 )
			{
				g_log.Log(CELOG_ERR,"hook CoCreateInstance: (failed)\n");
			}
			else
			{
				g_log.Log(CELOG_ERR,"hook CoCreateInstance: (ok)\n");
			}

			// Hook WinAPI SetClipboardData, Added By Jacky.Dong 2011-11-23
			// ------------------------------------------------------------
			bResult = HookAPI ("User32.dll", "SetClipboardData", try_SetClipboardData, (PVOID*)&NextSetClipboardData, 0);
			g_log.Log(CELOG_ERR,"hook SetClipboardData: (%s)\n", bResult == 0 ? "success" : "failed");
			// ------------------------------------------------------------

			// Hook WinAPI SetClipboardData, Added By Jacky.Dong 2011-12-02
			// ------------------------------------------------------------
			bResult = HookAPI ("kernel32.dll", "CreateFileW", try_CreateFileW, (PVOID*)&NextCreateFileW, 0);
			g_log.Log(CELOG_ERR,"hook CreateFileW: (%s)\n", bResult == 0 ? "success" : "failed");
			// ------------------------------------------------------------

			g_log.Log(CELOG_DEBUG, _T("Loading uccpHook DLL done\n"));
			break;
		}
	case DLL_PROCESS_DETACH:
		{
			OutputDebugStringA("leave oce");
			break;
		}
	}
	return TRUE;
}

bool __stdcall HOOK_UCCP_CoCreateInstance(
	REFCLSID rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD dwClsContext,
	REFIID riid,
	LPVOID * ppv,
	HRESULT hResult
)
{
	hResult;
	dwClsContext;
	pUnkOuter;

	bool bContinue=true;
	//g_log.Log(CELOG_DEBUG, _T("HOOK_UCCP_CoCreateInstance\n"));

	if( (memcmp(&UCCP::CLSID_UccPlatform,&rclsid,sizeof(rclsid))==0) &&
		(memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0) ) 
	{
		IUccPlatform* pUccPlatform = (IUccPlatform*)(*ppv);
		g_log.Log(CELOG_DEBUG, "It's IUccPlatform\n");
		HookPlatform(pUccPlatform);
	} 
	else if((memcmp(&UCCP::IID_IUccEndpoint,&riid,sizeof(riid))==0)) 
	{
		g_log.Log(CELOG_DEBUG, "It's IUccEndpoint\n");
	} 
	else if((memcmp(&UCCP::IID_IUccSessionManager,&riid,sizeof(riid))==0)) 
	{
		g_log.Log(CELOG_DEBUG, "It's IUccSessionManager\n");
	} 
	else if((memcmp(&UCCP::DIID__IUccSessionManagerEvents,&riid,sizeof(riid))==0)) 
	{
		g_log.Log(CELOG_DEBUG, "It's IUccSessionManagerEvents\n");
	} 
	else if((memcmp(&UCCP::DIID__IUccEndpointEvents,&riid,sizeof(riid))==0)) 
	{
		g_log.Log(CELOG_DEBUG, "It's IUccEndpointEvents\n");
	} 
	//else 
		//g_log.Log(CELOG_DEBUG, _T("HOOK_UCCP_CoCreateInstance: %d\n"), riid);
	return bContinue;
}

bool __stdcall HOOK_UCCP_CreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile,
	HANDLE result
)
{
	dwFlagsAndAttributes;
	hTemplateFile;
	dwCreationDisposition;
	lpSecurityAttributes;
	dwShareMode;
	dwDesiredAccess;

	bool bContinue=true;
	//g_log.Log(CELOG_DEBUG, _T("HOOK_UCCP_CreateFileW: tid=%d %s\n"), GetCurrentThreadId(), lpFileName);
	if(!(wcsstr(lpFileName, L"Microsoft Office Communicator") ||
		wcsstr(lpFileName, L"WINDOWS\\system32"))) {
		//We are not interested in OC's own files
		AddFileMappingForPolicyEval(GetCurrentThreadId(), lpFileName, result);	
	}
	return bContinue;
}

bool WINAPI HOOK_UCCP_SetClipboardData(UINT uFormat, HANDLE hMem)
{
	hMem;
	uFormat;


	g_log.Log(CELOG_DEBUG, _T("HOOK_UCCP_SetClipboardData\n"));

	// DWORD tid=GetCurrentThreadId();
	HWND hWnd = GetForegroundWindow();
	bool bAllow;
	bool bResultUnknown=GetCachedCopyEvalResult(hWnd, bAllow);

	g_log.Log(CELOG_DEBUG, _T("HOOK_UCCP_SetClipboardData: cached result %s\n"), bResultUnknown?L"unknown":L"known");
	if(bResultUnknown) {
		g_log.Log(CELOG_DEBUG, _T("HOOK_UCCP_SetClipboardData: do policy evaluation\n"));
		bAllow=PolicyEvalCopyAction();
		CacheCopyEvalResult(hWnd, bAllow);
	}
	g_log.Log(CELOG_DEBUG, _T("HOOK_UCCP_SetClipboardData: %s\n"), bAllow?L"Allowed":L"Denied");
    return bAllow;
}

bool WINAPI HOOK_UCCP_PreCreateProcessW(LPCTSTR lpApplicationName,// in
				     LPTSTR lpCommandLine,                        // in
				     LPSECURITY_ATTRIBUTES lpProcessAttributes,   // in
				     LPSECURITY_ATTRIBUTES lpThreadAttributes,    // in
				     BOOL bInheritHandles,                        // in
				     DWORD dwCreationFlags,                       // in
				     LPVOID lpEnvironment,                        // in
				     LPCTSTR lpCurrentDirectory,                  // in
				     LPSTARTUPINFO lpStartupInfo,                 // out
				     LPPROCESS_INFORMATION lpProcessInformation ) // out
{
	lpProcessInformation;
	lpStartupInfo;
	lpEnvironment;
	dwCreationFlags;
	bInheritHandles;
	lpThreadAttributes;
	lpProcessAttributes;
	lpCurrentDirectory;

	g_log.Log(CELOG_DEBUG, _T("HOOK_UCCP_PreCreateProcessW %s %s\n"), lpApplicationName, lpCommandLine == NULL ? _T("") : lpCommandLine );
	bool bAllow=true;

	if( lpCommandLine && wcsstr(lpCommandLine, L"LiveMeeting") && wcsstr(lpApplicationName, L"rundll32.exe")) {
		//Create Live Meeting do policy evaluation
		//get confURI
		wchar_t *confURI=wcsstr(lpCommandLine, L"meet:");
		if(confURI) {
			confURI+=wcslen(L"meet:");
			bAllow=PolicyEvalLivemeeting(confURI);
			if(!bAllow) 
				g_log.Log(CELOG_DEBUG, _T("Not allow to setup livingmeeting\n"));
		}
	}
	return bAllow;
}/* HOOK_UCCP_PreCreateProcessW */

bool WINAPI HOOK_UCCP_PostCreateProcessW(LPCTSTR lpApplicationName, // in
				     LPTSTR lpCommandLine,                        // in
				     LPSECURITY_ATTRIBUTES lpProcessAttributes,   // in
				     LPSECURITY_ATTRIBUTES lpThreadAttributes,    // in
				     BOOL bInheritHandles,                        // in
				     DWORD dwCreationFlags,                       // in
				     LPVOID lpEnvironment,                        // in
				     LPCTSTR lpCurrentDirectory,                  // in
				     LPSTARTUPINFO lpStartupInfo,                 // out
				     LPPROCESS_INFORMATION lpProcessInformation,  // out
					 BOOL realResult)                             // in
{
	realResult;
	lpProcessInformation;
	lpStartupInfo;
	lpCurrentDirectory;
	lpEnvironment;
	dwCreationFlags;
	bInheritHandles;
	lpThreadAttributes;
	lpProcessAttributes;

  g_log.Log(CELOG_DEBUG, _T("HOOK_UCCP_PostCreateProcessW appName=%s commandLine=%s\n"),
	lpApplicationName,
	lpCommandLine == NULL ? _T("") : lpCommandLine );

  //if(wcsstr(lpApplicationName, L"")
  return true;
}/* HOOK_UCCP_PostCreateProcessW */

#ifdef _MANAGED
#pragma managed(pop)
#endif

