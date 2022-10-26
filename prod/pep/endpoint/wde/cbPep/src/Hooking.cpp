#include "StdAfx.h"
#include "Hooking.h"
#include <ShObjIdl.h>
#include "eframework/auto_disable/auto_disable.hpp"
#include "NxtReceiver.h"
#include "FileOperationHooking.h"
#include "NxtMgr.h"
#include <string>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "celog.h"

extern CELog cbPepLog;

BOOL (WINAPI * NextMoveFileW)(
								   LPCWSTR lpExistingFileName,
								   LPCWSTR lpNewFileName);


BOOL (WINAPI * NextCopyFileExW)( LPCWSTR lpExistingFileName,
						  LPCWSTR lpNewFileName,
						  LPPROGRESS_ROUTINE lpProgressRoutine,
						  LPVOID lpData,
						  LPBOOL pbCancel,
						  DWORD dwCopyFlags);

BOOL (WINAPI * NextMoveFileWithProgressW)(
								 _In_      LPCWSTR lpExistingFileName,
								 _In_opt_  LPCWSTR lpNewFileName,
								 _In_opt_  LPPROGRESS_ROUTINE lpProgressRoutine,
								 _In_opt_  LPVOID lpData,
								 _In_      DWORD dwFlags
								 );


HRESULT (WINAPI* NextCoCreateInstance)(
									   REFCLSID rclsid,
									   LPUNKNOWN pUnkOuter,
									   DWORD dwClsContext,
									   REFIID riid,
									   LPVOID * ppv
									   ) = 0;


HookEntry CHooking::HookTable[] ={
	{ "Kernel32.DLL",	"MoveFileW",			MyMoveFileW,	       	  (PVOID*)&NextMoveFileW},
	{ "Kernel32.DLL",	"CopyFileExW",			MyCopyFileExW,	       	  (PVOID*)&NextCopyFileExW},
	{ "Kernel32.DLL",	"MoveFileWithProgressW",			MyMoveFileWithProgressW,	       	  (PVOID*)&NextMoveFileWithProgressW},
	{ "Ole32.DLL",     "CoCreateInstance",			MyCoCreateInstance,	       	  (PVOID*)&NextCoCreateInstance },
};


nextlabs::recursion_control hook_control;           // control recursion for hooks

CHooking::CHooking(void)
{
}

CHooking::~CHooking(void)
{
}

CHooking* CHooking::GetInstance()
{
	static CHooking inst;
	return &inst;
}

bool CHooking::StartHook()
{
	static bool bInit = false;

	if (!bInit)
	{
		InitializeMadCHook();

		int nHooks = sizeof(HookTable) / sizeof(HookEntry); 
		for (int i = 0; i < nHooks; i++)
		{
			if(!HookAPI (HookTable[i].dllName, HookTable[i].funcName, HookTable[i].newFunc, HookTable[i].oldFunc, 0))
			{
				cbPepLog.Log(CELOG_DEBUG,L"Hook failed\n");
				//OutputDebugStringW(L"Hook failed\n");
			}
		}

		bInit = true;
	}
	
	return true;
}

bool CHooking::EndHook()
{
	static bool bInit = false;

	if (!bInit)
	{
		int nHooks = sizeof(HookTable) / sizeof(HookEntry); 
		for (int i = 0; i < nHooks; i++)
		{
			UnhookAPI(HookTable[i].oldFunc);	
		}

		FinalizeMadCHook();

		bInit = true;
	}

	return true;
}

BOOL CHooking::MyMoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
	if ( hook_control.is_disabled() == true)
	{
		return NextMoveFileW(lpExistingFileName, lpNewFileName);
	}
	
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if(!CNxtMgr::Instance()->PreHandleEncryption(lpExistingFileName, lpNewFileName))
		return TRUE;

	BOOL ret = NextMoveFileW(lpExistingFileName, lpNewFileName);
	DWORD dwError = GetLastError();

	CNxtMgr::Instance()->PostHandleEncryption(lpExistingFileName, lpNewFileName);

	SetLastError(dwError);
	return ret;
}

BOOL CHooking::MyMoveFileWithProgressW( _In_ LPCWSTR lpExistingFileName, _In_opt_ LPCWSTR lpNewFileName, _In_opt_ LPPROGRESS_ROUTINE lpProgressRoutine, _In_opt_ LPVOID lpData, _In_ DWORD dwFlags )
{
	if ( hook_control.is_disabled() == true)
	{
		return NextMoveFileWithProgressW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	if(!CNxtMgr::Instance()->PreHandleEncryption(lpExistingFileName, lpNewFileName))
	{
		return TRUE;
	}

	BOOL ret = NextMoveFileWithProgressW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);

	DWORD dwLastError = GetLastError();

	CNxtMgr::Instance()->PostHandleEncryption(lpExistingFileName, lpNewFileName);

	SetLastError(dwLastError);
	return ret;
}

/*************************************************************************
for the case, go to folder c:\test, copy c:\test\a.txt under c:\test
the function CopyFileEx will be called 2 times.
first time, the source and destination are same, and CopyFileEx will fail.
then, at second time call, the destination file will be renamed to another 
name.
*************************************************************************/
BOOL CHooking::MyCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
	if ( hook_control.is_disabled() == true)
	{
		return NextCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	/*
	fix bug 18404
	18404 only happens on win xp.
	this is very interesting, once we called SE_IsEncrypted() with path C:\DOCUME~1\ALLUSE~1\APPLIC~1\MICROS~1\OFFICE\DATA\OPA12.BAK,
	this problem will happen.
	I guess there are some access violation during this period. 
	here we just use a work around.
	*/
	if (lpExistingFileName)
	{
		std::wstring src(lpExistingFileName);
		std::transform(src.begin(), src.end(), src.begin(), tolower);
		if (boost::algorithm::iends_with(src, L".bak") && src.find(L"office\\data\\opa") != std::wstring::npos)
		{
			return NextCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
		}
	}
	

	if(!CNxtMgr::Instance()->PreHandleEncryption(lpExistingFileName, lpNewFileName))
	{
		return TRUE;
	}

	BOOL ret = NextCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	DWORD dwLastError = GetLastError();
	
	CNxtMgr::Instance()->PostHandleEncryption(lpExistingFileName, lpNewFileName);

	SetLastError(dwLastError);
	return ret;
}

HRESULT CHooking::MyCoCreateInstance( REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv )
{
	if ( hook_control.is_disabled() == true)
	{
		return NextCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	HRESULT hr = NextCoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv);

	if(SUCCEEDED(hr) && (*ppv) != NULL &&
		::IsEqualCLSID(rclsid,CLSID_FileOperation) &&
		::IsEqualIID(riid,IID_IFileOperation))
	{
		cbPepLog.Log(CELOG_DEBUG,L"MyCoCreateInstance, hooked a IFileOperation.\n");
		//OutputDebugStringW(L"MyCoCreateInstance, hooked a IFileOperation.\n");

		IFileOperation* obj = (IFileOperation*)(*ppv);
		CFileOperationHooking::GetInstance()->Hook(obj);
	}

	return hr;
}
