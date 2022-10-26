// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif


//#if defined(_WIN64)
// typedef __int64 LONG_PTR; 
//#else
//#ifndef LONG_PTR
// typedef long LONG_PTR;
//#endif
//#endif

// Windows Header Files:
#include <windows.h>
#include <CommCtrl.h>
#include <Psapi.h>
#include <assert.h>

#pragma comment( lib, "Psapi.lib" )

#define _ATL_ALL_WARNINGS
#include <atlbase.h>
#include <atlcom.h>
#include <objbase.h>
#include <map>
#include <list>
#include <string>

#pragma warning(push)
#pragma warning(disable:4267)
#include "nlconfig.hpp"
#pragma warning(pop)

#include "nlexcept.h"
using namespace ATL;

#pragma warning(push)
#pragma warning(disable:4192)

#import "..\\src\\CONFAPI.dll" rename_namespace("CONFAPI") \
	rename ( "LONG_PTR"	, "CONFAPILONG_PTR") \
	raw_interfaces_only, raw_native_types, named_guids, auto_search
using namespace CONFAPI;

#import "..\\src\\Uccp.dll" rename_namespace("UCCP")\
	rename ( "LONG_PTR"	, "UCCPLONG_PTR") \
	rename ( "SendMessage"	, "UCCPSendMessage") \
	raw_interfaces_only,raw_native_types,named_guids,auto_search
using namespace UCCP;

#import "..\\src\\Collaborate.dll" rename_namespace("LMC_Coll") \
	rename ( "LONG_PTR"	, "LMC_CollLONG_PTR") \
	raw_interfaces_only,raw_native_types,named_guids,auto_search
using namespace LMC_Coll;


// maybe we need rename optional
#import "..\\src\\AppShare.dll" exclude("IBaseObjects","IBaseObject","IBaseEvent","IBaseNotifications",\
	"ICollaborateApp","ICollaborateViewerBuffer","IErrorInfo"),\
	rename_namespace("LMC_AppShare")\
	raw_interfaces_only,raw_native_types,named_guids,auto_search 
using namespace LMC_AppShare;


#import "..\\src\\ImportUtil.dll" rename_namespace("ImportUtilLib") \
    raw_interfaces_only,raw_native_types,named_guids,auto_search
using namespace ImportUtilLib;

#import "..\\src\\LMDIView.dll" rename_namespace("MODI") \
    raw_interfaces_only,raw_native_types ,named_guids,auto_search
using namespace MODI;

#pragma warning(pop)
//#import "C:\WINDOWS\system32\Macromed\Flash\Flash.ocx" rename_namespace("Flash") \

extern const GUID __declspec(selectany) CLSID_ShockwaveFlash =
    {0xd27cdb6e,0xae6d,0x11cf,{0x96,0xb8,0x44,0x45,0x53,0x54,0x00,0x00}};

struct __declspec(uuid("d27cdb6c-ae6d-11cf-96b8-444553540000"))
/* dual interface */ IShockwaveFlash;
//#import "c:\WINDOWS\system32\Macromed\Flash\Flash10e.ocx" rename_namespace("Flash") \
//    raw_interfaces_only,raw_native_types,named_guids,auto_search
//using namespace Flash;


//#import "C:\WINDOWS\system32\msxml.dll" rename_namespace("MSXML") \
//    raw_interfaces_only,raw_native_types,named_guids,auto_search
//using namespace MSXML;



//#import "..\\inc\\MSPTLS.dll" rename_namespace("LMC_Coll") \
//    raw_interfaces_only,raw_native_types,named_guids,auto_search
//using namespace LMC_Coll;

#pragma comment(lib, "Dbghelp.lib")

#pragma warning(push)
#pragma warning(disable: 4819)  // We won't fix code page issue in MADCHOOK header file, just ignore it here
#include "madCHook_helper.h"
#pragma warning(pop)

//////////////////////////////////////////////////////////////////////////
// detours function pointers
//extern LONG (WINAPI* gDetourTransactionBegin)();
//extern LONG (WINAPI* gDetourTransactionCommit)();
//extern LONG (WINAPI* gDetourUpdateThread)(HANDLE hThread);
//extern LONG (WINAPI* gDetourAttach)(PVOID *ppPointer,PVOID pDetour);
//extern LONG (WINAPI* gDetourDetach)(PVOID *ppPointer,PVOID pDetour);

//////////////////////////////////////////////////////////////////////////


#include "eframework/auto_disable/auto_disable.hpp"
extern nextlabs::recursion_control hook_control;
static void exception_cb( NLEXCEPT_CBINFO* cb_info )
{
	hook_control.process_disable();
	if( cb_info != NULL )
	{
		wchar_t comp_root[MAX_PATH * 2] = {0}; // component root for HTTPE
		if( NLConfig::GetComponentInstallPath(L"Enterprise DLP\\LiveMeeting Enforcer",comp_root,_countof(comp_root)) == true )
		{
			wcsncat_s(comp_root,_countof(comp_root),L"\\diags\\dumps",_TRUNCATE);
			wcsncpy_s(cb_info->dump_root,_countof(cb_info->dump_root),comp_root,_TRUNCATE);
			cb_info->use_dump_root = 1;
		}
	}
}/* exception_cb */
static bool LMEIsDisabled(void)
{
	return hook_control.is_disabled();
}
//#include "PartDB.h"
#include "log.h"
#include "Sink.h"
#include "SinkConfSessPart.h"
#include "SinkConfMediaChannelCollection.h"
#include "SinkConfMediaChannel.h"
#include "SinkSessPart.h"
#include "SinkSessPartEndpointCollection.h"
#include "SinkConfEntityView.h"
#include "SinkConfEntityViewCollectionEvents.h"
#include "SinkMediaChannel.h"
#include "SinkConfSess.h"
#include "SinkSess.h"
#include "SinkSessPartCollection.h"
#include "SinkUnknown.h"



#include "HookedMediaChannel.h"
#include "HookedConfEntityView.h"
#include "HookedConfSessPart.h"
#include "HookedConfSessPartEndPoint.h"
#include "HookedSignalingChannel.h"
#include "HookedSessPart.h"
#include "HookedOperationContext.h"
#include "HookedSignalingMessage.h"



extern void WriteLog(const char* strFilePath,const char* strContent);
// TODO: reference additional headers your program requires here
extern bool OutPutLog(const char* strContext,const int nDebugOutPut=1,const char* strPath="HookInfo.txt");
extern bool OutPutLog(const TCHAR* strContext,const int nDebugOutPut=1,const TCHAR* strPath=L"HookInfo.txt");

class Mutex 
{
	friend class MUTEX;
public:
	Mutex()
	{
		InitializeCriticalSection(&cs);
	}
	~Mutex()
	{
		DeleteCriticalSection(&cs);
	}
private:
	void lock()
	{
		EnterCriticalSection(&cs);
	}
	void unlock()
	{
		LeaveCriticalSection(&cs);
	}
private:
	Mutex(const Mutex& mutex);
	void operator=(const Mutex& mutex);
private:	
	CRITICAL_SECTION cs;
};

class MUTEX
{
public:
	MUTEX(Mutex *mutex):_mutex(mutex)
	{
		_mutex->lock();
	}
	~MUTEX()
	{
		_mutex->unlock();
	}
private:
	Mutex* _mutex;
};

const TCHAR* GetNameWithoutPath( const TCHAR* pstrPath, DWORD nLen );
bool IsExecute( const TCHAR* pMsg );
bool AllowToContinue( const TCHAR* pMsg );
bool ShowInfo( const TCHAR* pMsg );
bool DemoShowInfo( const TCHAR* pMsg, const TCHAR* pInfoBox);
bool MsgBoxAllowOrDeny( const TCHAR* pMsg, const TCHAR* pMsgBoxInfo, const TCHAR* pstrAttachment = 0 );
HWND GetWindowHwd();
const TCHAR* GetProcessName( UINT32 nProcessId );
void AllowShare();
void AllowChat();
BOOL GetModuleName(LPWSTR lpszProcessName, int nLen, HMODULE hMod) ;
BOOL IsProcess(LPCWSTR lpProcessName) ;

#include "eframework\platform\cesdk_loader.hpp"
#include "eframework\platform\cesdk_obligations.hpp"
#include "eframework\platform\policy_controller.hpp"
bool DoEvaluate( const TCHAR* pstrRes , wchar_t* action);
bool DoAppEvaluate( const TCHAR* pMagic,const TCHAR* pstrRes, wchar_t*  action,const wchar_t* pszDest=NULL );
#ifndef LME_MAGIC_STRING
#define LME_MAGIC_STRING L"C:\\No_attachment.ice"
#endif