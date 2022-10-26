#include "stdafx.h"

#include "madCHook.h"
#include <set>
#include <map>
#include "nl_sysenc_lib.h"
#include "Hook.h"
#include "celog.h"


//#pragma warning(push)
//#pragma warning(disable: 4267)
//#include "eframework/auto_disable/auto_disable.hpp"
//#include "nlexcept.h"
//#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable: 4267)
#include "nlconfig.hpp"
#pragma warning(pop)
#pragma comment(lib, "Dbghelp")

#ifdef _M_IX86
#pragma  comment (lib,"madChook32.lib")
#elif defined (_M_AMD64)
#pragma  comment(lib,"madCHook64.lib")
#else
#error NoSupport this platform
#endif


//
//nextlabs::recursion_control ot_hook_control;
////nextlabs::recursion_control mso_hook_createfile_control;
////
////
////
//static std::wstring g_dumproot;
//void exception_cb( NLEXCEPT_CBINFO* cb_info )
//{
//    wcsncpy_s(cb_info->dump_root, MAX_PATH, g_dumproot.c_str(), _TRUNCATE);
//    mso_hook_control.process_disable();
//}/* exception_cb */





static int exception_state = 0;
extern CELog OutlookAddinLog;



static PVOID WINAPI myMsoCreateIOLDocFromWZPersistentName( PVOID arg1, PVOID arg2, PVOID arg3 ) ;
typedef PVOID ( WINAPI* MsoCreateIOLDocFromWZPersistentNameType)( PVOID arg1, PVOID arg2, PVOID arg3 ) ; 
static MsoCreateIOLDocFromWZPersistentNameType real_MsoCreateIOLDocFromWZPersistentName = NULL ;

//////////////////////////////////////////////////////////////////////////
void StartStopHook(bool bStart)
{
	/*__try
	{*/
		if(bStart==true)
		{
			InitializeMadCHook();

			#ifdef _M_X64
				if(HookAPI("Mso.dll","MsoCreateIOLDocFromWzPersistentName",(PVOID)myMsoCreateIOLDocFromWZPersistentName,(PVOID*)&real_MsoCreateIOLDocFromWZPersistentName) == FALSE)
				{
					OutlookAddinLog.Log(CELOG_DEBUG,L"Failed to hook _MsoCreateIOLDocFromWzPersistentName");
					//::OutputDebugStringW(L"Failed to hook _MsoCreateIOLDocFromWzPersistentName");
				}
			#else
				if(HookAPI("Mso.dll","_MsoCreateIOLDocFromWzPersistentName@12",(PVOID)myMsoCreateIOLDocFromWZPersistentName,(PVOID*)&real_MsoCreateIOLDocFromWZPersistentName) == FALSE)
				{
					OutlookAddinLog.Log(CELOG_DEBUG,L"Failed to hook _MsoCreateIOLDocFromWzPersistentName");
					//::OutputDebugStringW(L"Failed to hook _MsoCreateIOLDocFromWzPersistentName");
				}
			#endif
		}
		else
		{//must be unhooked.otherwise there will be core down where register the dll.
			FinalizeMadCHook();
			if(UnhookAPI((PVOID*)&real_MsoCreateIOLDocFromWZPersistentName) == FALSE)
				{
					OutlookAddinLog.Log(CELOG_DEBUG,L"Failed to unhook real_MsoCreateIOLDocFromWZPersistentName");
					//::OutputDebugStringW(L"Failed to unhook real_MsoCreateIOLDocFromWZPersistentName");
				}
		}
	//}
 //   __except( NLEXCEPT_FILTER_EX2(&exception_state, exception_cb) )
 //   {
 //     /* empty */
 //       ;
 //   }
}

BOOL WINAPI DoSEAutoWrap(const wchar_t* szSrouce,const wchar_t* szTemp, wchar_t *strDestPath )
{
	//BOOL status = TRUE;
	std::wstring strSrcPath =szSrouce;
	std::wstring strFileName ;
	BOOL bKeepEncrypted = FALSE;
	
	if(NULL==szSrouce || L'\0'==szSrouce[0] || INVALID_FILE_ATTRIBUTES == GetFileAttributesW(szSrouce)) {
		return FALSE;
	}
	
	bKeepEncrypted = SE_IsEncrypted(szSrouce);
	if( bKeepEncrypted == FALSE)
	{
		OutlookAddinLog.Log(CELOG_DEBUG,L"Current is not wrapped");
		//::OutputDebugStringW( L"Current is not wrapped");
		return FALSE ;
	}

	std::wstring strExt = L"";
	if (strSrcPath.find_last_of(L'.') != std::wstring::npos)
	{
		strExt = strSrcPath.substr(strSrcPath.find_last_of(L'.'));
		OutlookAddinLog.Log(CELOG_DEBUG,L"SE_WrapEncryptedFile: file extension=");
		//::OutputDebugStringW( L"SE_WrapEncryptedFile: file extension=") ;
		OutlookAddinLog.Log(CELOG_DEBUG,L"%s",strExt.c_str());
		//::OutputDebugStringW( strExt.c_str());
	}
	if (strExt.compare(L".nxl") == 0)
	{
		OutlookAddinLog.Log(CELOG_DEBUG,L"SE_WrapEncryptedFile already encrypted with .nxl. Ignore it!" );
		//::OutputDebugStringW( L"SE_WrapEncryptedFile already encrypted with .nxl. Ignore it!" ) ;
		return FALSE ;
	}
	MessageBoxW( ::GetForegroundWindow(),L"You cannot attach NextLabs Protected file(s) in this manner.\r\nTo attach this file, copy the file in Windows Explorer and paste it in an email message.",L"NextLabs Attach File Denied",MB_OK|MB_ICONERROR|MB_TOPMOST) ;
	return TRUE ;

}

PVOID WINAPI myMsoCreateIOLDocFromWZPersistentName( PVOID arg1, PVOID arg2, PVOID arg3)
{
	//PVOID pRet = NULL ;
	BOOL bRet = FALSE ;
	LPWSTR strFile = (LPWSTR)arg2 ; 
	wchar_t szDestPath[MAX_PATH+1] = {0} ;
	bRet =	DoSEAutoWrap(strFile ,NULL,szDestPath) ;
	if( bRet == FALSE)
	{
		return  ( (MsoCreateIOLDocFromWZPersistentNameType)real_MsoCreateIOLDocFromWZPersistentName)( arg1, arg2, arg3 ) ;
	}
	else
	{
		::ZeroMemory( arg2,wcslen(strFile)*sizeof(wchar_t) ) ;
		return  ( (MsoCreateIOLDocFromWZPersistentNameType)real_MsoCreateIOLDocFromWZPersistentName)( arg1, arg2, arg3 ) ;
	}
	
	
}