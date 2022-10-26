#include <cassert>
#include "stdafx.h"
#include "APIHook.h"

#include "stdlib.h"
#include "WTypes.h"
#include "ImageHlp.h"

#include "ObjBase.h"
#include <shlobj.h>
#pragma comment(lib, "Dbghelp.lib")

#pragma warning(push)
#pragma warning(disable: 4819)
#include "madCHook_helper.h"
#pragma warning(pop)

#include "httpmsg.h"
#include "httpprocessor.h"
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include "httpcollector.h"
#include "MapperMngr.h"
#include "eframework/auto_disable/auto_disable.hpp"
#include "timeout_list.hpp"
#include "httpprocessor.h"
#include "SharedMemory.h"
#include "HTTPMgr.h"
#include "Utilities.h"
#include <boost/algorithm/string.hpp>

using namespace std;

#define FIREFOX_ENCRYPT 0x104
#define FIREFOX_DECRYPT 0x105
#define MAX_FILE_CONTENT_SIZE	50 * 1024

extern CTimeoutList g_DownloadURIList;
extern CTimeoutList g_Temp_DownloadURIList;

CAPIHook::CreateFileWType g_realCreateFileW = NULL;
CAPIHook::CloseHandleType g_realCloseHandle = NULL;

CAPIHook::CreateFileWType    CAPIHook::real_CreateFileW    = NULL;
CAPIHook::WriteFileType      CAPIHook::real_WriteFile      = NULL;
CAPIHook::CloseHandleType	 CAPIHook::real_CloseHandle = NULL;
HRESULT*   CAPIHook::real_CoCreateInstance   = NULL;
CAPIHook::ReadFileType CAPIHook::real_ReadFile = NULL ;
CAPIHook::EncryptMessageType	CAPIHook::real_EncryptMessage = NULL ;
CAPIHook::DecryptMessageType	CAPIHook::real_DecryptMessage = NULL ; 

CAPIHook::CopyFileType CAPIHook::real_CopyFileW = NULL;
CAPIHook::CopyFileExWType CAPIHook::real_CopyFileExW = NULL;
CAPIHook::MoveFileWType CAPIHook::real_MoveFileW = NULL;
CAPIHook::MoveFileExWType CAPIHook::real_MoveFileExW = NULL;

CAPIHook::PK11_CipherOpType CAPIHook::real_PK11_CipherOp = NULL;
CAPIHook::PK11_DigestFinalType CAPIHook::real_PK11_DigestFinal = NULL;

CAPIHook::MoveFileWithProgressWType CAPIHook::real_MoveFileWithProgressW = NULL;

CAPIHook::SHFileOperationWType CAPIHook::real_SHFileOperationW = NULL;

CAPIHook::CreateFileAType CAPIHook::real_CreateFileA = NULL; 

CTimeoutList CAPIHook::m_listFireFox_Download;
string CAPIHook::m_strLastDigest;

CTimeoutList g_listDeniedPath(30 * 60 * 1000);

CTimeoutList g_listDeniedHandle(5 * 60 * 1000);

CTimeoutList g_listContextDecrypt(3 * 1000);

CTimeoutList g_listWriteFileCache(3 * 1000);
CTimeoutList g_listReadFileCache(3 * 1000);
/* When an exception occurs diable hooking for the process.
*/
nextlabs::recursion_control hook_control;

static bool HTTPEIsDisabled(void)
{
	return hook_control.is_disabled();
}


APIRES::HOOKPROCINFO myProcInfo[] =
{
	{"Kernel32.DLL", "CloseHandle",  NULL, (PVOID *)&CAPIHook::real_CloseHandle,  CAPIHook::try_CloseHandle,  NULL },
	{"Kernel32.DLL", "CreateFileW", NULL, (PVOID *)&CAPIHook::real_CreateFileW, CAPIHook::try_CreateFileW, NULL },
	{"Kernel32.DLL", "CreateFileA", NULL, (PVOID *)&CAPIHook::real_CreateFileA, CAPIHook::try_CreateFileA, NULL },
	{"Kernel32.DLL", "WriteFile",   NULL, (PVOID *)&CAPIHook::real_WriteFile,   CAPIHook::try_WriteFile,   NULL },
	{"Kernel32.DLL", "ReadFile",   NULL, (PVOID *)&CAPIHook::real_ReadFile,   CAPIHook::try_ReadFile,   NULL },
	{"Kernel32.DLL", "CopyFileExW",   NULL, (PVOID *)&CAPIHook::real_CopyFileExW,   CAPIHook::try_CopyFileExW,   NULL },
	{"Kernel32.DLL", "MoveFileW",   NULL, (PVOID *)&CAPIHook::real_MoveFileW,   CAPIHook::try_MoveFileW,   NULL },
	{"Kernel32.DLL", "MoveFileExW",   NULL, (PVOID *)&CAPIHook::real_MoveFileExW,   CAPIHook::try_MoveFileExW,   NULL },
	{"Shell32.DLL", "SHFileOperationW",   NULL, (PVOID *)&CAPIHook::real_SHFileOperationW,   CAPIHook::try_SHFileOperationW,   NULL }
};

//Chrome
APIRES::HOOKPROCINFO myChromeProcInfo[] = 
{
	{"Kernel32.DLL", "MoveFileWithProgressW", NULL, (PVOID *)&CAPIHook::real_MoveFileWithProgressW, CAPIHook::try_MoveFileWithProgressW, NULL },
};


/*
IE&Chrome
*/
APIRES::HOOKPROCINFO myIEHTTPSInfo[] =
{
	{"secur32.DLL", "EncryptMessage",   NULL, (PVOID *)&CAPIHook::real_EncryptMessage,   CAPIHook::try_EncryptMessage,   NULL },
	{"secur32.DLL", "DecryptMessage",   NULL, (PVOID *)&CAPIHook::real_DecryptMessage,   CAPIHook::try_DecryptMessage,   NULL },
};
/*
FireFoxs
*/
APIRES::HOOKPROCINFO myFFXHTTPSInfo[] =
{
	{"nss3.DLL", "PK11_CipherOp",   NULL, (PVOID *)&CAPIHook::real_PK11_CipherOp,   CAPIHook::try_PK11_CipherOp,   NULL },
	{"nss3.DLL", "PK11_DigestFinal",   NULL, (PVOID *)&CAPIHook::real_PK11_DigestFinal,   CAPIHook::try_PK11_DigestFinal,   NULL },
	{"secur32.DLL", "EncryptMessage",   NULL, (PVOID *)&CAPIHook::real_EncryptMessage,   CAPIHook::try_EncryptMessage,   NULL },
	{"secur32.DLL", "DecryptMessage",   NULL, (PVOID *)&CAPIHook::real_DecryptMessage,   CAPIHook::try_DecryptMessage,   NULL },
}	;
APIRES::HOOKADDRINFO hookAddrInfo[] = 
{
	{NULL,NULL,NULL,NULL,NULL}
};
APIRES::HOOKEXPIDINFO hookAPI_byIDInfo[] =
{
	{NULL,NULL,NULL,NULL,NULL}
};
/*
Package the Hook API for Mathook.
It is easy to do replace for the mathook.
*/
//---------------------------------------------------------------------------------------------------------------
BOOL CAPIHook::HookModuleAPI(  LPCSTR pszModule,  LPCSTR pszFuncName,  PVOID  pCallbackFunc,  PVOID  *pNextHook ) 
{
	return HookAPI(pszModule, pszFuncName, pCallbackFunc, pNextHook)	 ;
}
BOOL CAPIHook::HookCodeByAddr(  PVOID  pCode,  PVOID  pCallbackFunc,  PVOID  *pNextHook ) 
{
	return HookCode(pCode ,pCallbackFunc, pNextHook ) ;
}
BOOL CAPIHook::UnHookModuleAPI(   PVOID  *pNextHook )
{
	return	 UnhookAPI(pNextHook);
}
BOOL CAPIHook::UnHookCodeByAddr(  PVOID  *pNextHook )
{
	return	UnhookCode(pNextHook);	
}
//---------------------------------------------------------------------------------------------------------------

BOOL GetURLWithTempPath(wstring strSrc, wstring strTemp, wstring& strURL)
{
	wchar_t szPath[MAX_PATH * 2 +1] = {0};
	if(GetLongPathNameW(strSrc.c_str(), szPath, MAX_PATH * 2) > 0)
	{
		strSrc = wstring(szPath);
	}

	/*****************************************************************************************************
	We will look for the related URL if the src file is temp file.
	For example:
	Firefox will save the file to temp folder when we try to download a file, and then firefox will call
	"MoveFile" to move the file from temp folder destination folder.
	Solution:
	1. Save the "url<->temp file path" to a list in "WriteFile",
	2. Try to find the URL with "src" file in "myMoveFile" if the "src" path is a temp path.
	3. Do evaluation with the URL and destination path.
	******************************************************************************************************/
	if(strSrc.length() > strTemp.length() && _memicmp(strSrc.c_str(), strTemp.c_str(), strTemp.length() * sizeof(wchar_t)) == 0)
	{
		wstring strRemoteURL;
		if(g_Temp_DownloadURIList.FindItem(strSrc.c_str(), strRemoteURL))//Try to find the URL 
		{
			//	Ben, new comments, 2011/March/11
			//	new version chrome will try to move temp file to destination twice, so, we can't delete this item.
			//	yes, it is very long time for the cache to be timeout.
			//	but the temp file' name will be unique at a long time, so don't delete the cache item is not very serious.
			//	so, if the process is chrome, we don't delete item. if the process is not chrome, we don't change existing code -- delete item.
			//	end for Ben, new comments, 2011/March/11, this is for bug 13772
			if (!IsProcess(L"chrome.exe"))
			{
				g_Temp_DownloadURIList.DeleteItem(strSrc.c_str());//The "time out" value of this list is very big, so we need to delete the item.
			}
			strURL = strRemoteURL;
			return TRUE;
		}
		else
		{
			wstring strKey, strValue;
			if(g_DownloadURIList.GetLastItem(strKey, strValue))//Check if chrome/firefox uses "cache" file.
			{
				g_DownloadURIList.DeleteItem(strKey);
				strURL = strValue;
				g_log.Log(CELOG_DEBUG, "HTTPE::\"304 NOT MODIFIED \" response.");
				return TRUE;
			}
		}

	}
	return FALSE;
}

BOOL CAPIHook::StartHook()
{
	InitializeMadCHook();

	return HookAPIByInfoList() ;
}
VOID CAPIHook::EndHook()
{
	g_realCloseHandle = NULL;
	
	if( !IsProcess(L"firefox.exe") && !IsProcess(L"SmartFTP.exe")  )
	{
		FinalizeMadCHook();
	}	
}

/*
Get the function address by the exported ID.
The ID value of the function based on the export list.
*/
PVOID CAPIHook::GetExportFunc_ByID( const wchar_t *pszModuleName, DWORD i_dID)
{
	if( pszModuleName == NULL )
	{
		return NULL ;
	}
	HMODULE hmod = ::GetModuleHandle( pszModuleName ) ;
	if( hmod == NULL )
	{
		//GetModuleHandle Failure!
		return  NULL;
	}
	ULONG   ulSize;   
	/*
	Receive   the   PIMAGE_EXPORT_DIRECTORY   block  
	*/
	PIMAGE_EXPORT_DIRECTORY   pExportDesc   =     
		(PIMAGE_EXPORT_DIRECTORY)
		ImageDirectoryEntryToData(hmod,   TRUE,     
		IMAGE_DIRECTORY_ENTRY_EXPORT,   &ulSize);   
	if(pExportDesc   ==   NULL)   
	{   

		return  NULL ;   
	}     
	PDWORD  pdwOffsetOfFunctions   =     (PDWORD)((PBYTE)hmod   +   pExportDesc->AddressOfFunctions);    

	DWORD	dCount = pExportDesc->NumberOfFunctions ;

	if(  dCount > i_dID )
	{
		PROC pfnFunction =(PROC)( (PBYTE)hmod + pdwOffsetOfFunctions[ i_dID -1] ); 
		return (PVOID)pfnFunction ;
	}
	return NULL ;
}

/*
Get the function address by the offset of the module.
*/

PVOID CAPIHook::GetFuncAddr_ByOffset(DWORD i_dOffset, char *i_pszModuleName)
{
	HMODULE hmod = GetModuleHandleA( i_pszModuleName ) ;
	if( hmod == NULL ) 
	{
		//get module handle failue! the module specified by i_pszModuleName is NULL in this process!
		return NULL;
	}
	PVOID pPointer = (PVOID)((BYTE*)hmod + i_dOffset ) ;

	return pPointer ;
}
/*
Hook all the function which in the special list
*/
BOOL CAPIHook::HookAPIByInfoList() 
{
	BOOL bRet = FALSE ;
	int iCount = 0;

	/*
	IE/Chrome
	*/
	//------------------------------------------------------------------------------------------
	if( IsProcess(L"IExplore.exe") || IsProcess(L"chrome.exe") || IsProcess(L"explorer.exe"))
	{
		for(iCount = 0; iCount < sizeof(myIEHTTPSInfo) / sizeof(APIRES::HOOKPROCINFO); iCount++)
		{
			if(myIEHTTPSInfo[iCount].pNewProc != NULL )
			{

				if(!HookModuleAPI(myIEHTTPSInfo[iCount].pszModule, myIEHTTPSInfo[iCount].pszOrigName, myIEHTTPSInfo[iCount].pNewProc, myIEHTTPSInfo[iCount].pNextProc))
					return FALSE;

			}

		}
	}

	//Chrome
	if(IsProcess(L"Chrome.exe"))
	{
		for(iCount = 0; iCount < sizeof(myChromeProcInfo) / sizeof(APIRES::HOOKPROCINFO); iCount++)
		{
			if(myChromeProcInfo[iCount].pNewProc != NULL)
			{
				if(!HookModuleAPI(myChromeProcInfo[iCount].pszModule, myChromeProcInfo[iCount].pszOrigName, myChromeProcInfo[iCount].pNewProc, myChromeProcInfo[iCount].pNextProc) )
					return FALSE;
			}
		}
	}
	/*
	FireFoxe
	*/
	if(	 IsProcess(L"Firefox.exe") )
	{
		for(iCount = 0; iCount < sizeof(myFFXHTTPSInfo) / sizeof(APIRES::HOOKPROCINFO); iCount++)
		{
			if(myFFXHTTPSInfo[iCount].pNewProc != NULL )
			{

				if(!HookModuleAPI(myFFXHTTPSInfo[iCount].pszModule, myFFXHTTPSInfo[iCount].pszOrigName, myFFXHTTPSInfo[iCount].pNewProc, myFFXHTTPSInfo[iCount].pNextProc))
					return FALSE;

			}

		}
		
	
	}
	//------------------------------------------------------------------------------------------
	for(iCount = 0; iCount < sizeof(myProcInfo) / sizeof(APIRES::HOOKPROCINFO); iCount++)
	{
		if(myProcInfo[iCount].pNewProc != NULL )
		{
			if(!HookModuleAPI(myProcInfo[iCount].pszModule, myProcInfo[iCount].pszOrigName, myProcInfo[iCount].pNewProc, myProcInfo[iCount].pNextProc))
				return FALSE;

		}

	}

	for(iCount = 0; iCount < sizeof(hookAddrInfo) / sizeof(APIRES::HOOKADDRINFO); iCount++)
	{
		if( hookAddrInfo[iCount].pNewProc != NULL )
		{

			hookAddrInfo[iCount].pOldProc = GetFuncAddr_ByOffset( hookAddrInfo[iCount].dOffset,hookAddrInfo[iCount].pszModuleName ) ;
			HookCodeByAddr( hookAddrInfo[iCount].pOldProc ,hookAddrInfo[iCount].pNewProc, hookAddrInfo[iCount].pNextProc ) ;
			bRet = TRUE ;

		}

	}

	for(iCount = 0; iCount < sizeof(hookAPI_byIDInfo) / sizeof(APIRES::HOOKEXPIDINFO); iCount++)
	{
		if( hookAPI_byIDInfo[iCount].pNewProc != NULL )
		{
			hookAPI_byIDInfo[iCount].pOldProc = GetExportFunc_ByID( hookAPI_byIDInfo[iCount].pszModuleName, hookAPI_byIDInfo[iCount].dID ) ;
			HookCodeByAddr( hookAPI_byIDInfo[iCount].pOldProc ,hookAPI_byIDInfo[iCount].pNewProc, hookAPI_byIDInfo[iCount].pNextProc ) ;

			bRet = TRUE ;

		}

	}

	g_realCreateFileW = CAPIHook::real_CreateFileW;
	g_realCloseHandle = CAPIHook::real_CloseHandle;

	return TRUE;
}
/*
Hook the com object interface.
*/
BOOL CAPIHook::HookComInterface( UINT _iTID, std::list<APIRES::HOOKAPIINFO> &hookAPIInfo, LONG_PTR pOriginFunc, LONG_PTR  pmyFunc ,PVOID pObjectAddress) 
{
	BOOL bRet = FALSE ;
	BOOL bUpdate = FALSE;
	LONG_PTR real_APIAddress = NULL ;
	static INT iCount = 0 ;
	if(pOriginFunc != NULL)
	{
		for ( std::list<APIRES::HOOKAPIINFO>::iterator iter = hookAPIInfo.begin() ; iter != hookAPIInfo.end() ; iter++ )
		{
			if ((_iTID == (*iter).threadID))
			{
				return bRet ;

			}
		}
		if(pOriginFunc != pmyFunc )
		{

			bRet = HookCodeByAddr( (PVOID)pOriginFunc,(PVOID)pmyFunc ,(PVOID*)&real_APIAddress ) ;
		}
		else
		{
			return bRet ;
		}
		if( bUpdate != TRUE )
		{
			APIRES::HOOKAPIINFO apiInfo ;
			apiInfo.threadID = _iTID ; 
			apiInfo.pRealAPIAddress = (VOID*)real_APIAddress ;
			apiInfo.orig_APIAddress = pOriginFunc ;	
			apiInfo.pObjectAddress = pObjectAddress ; 
			hookAPIInfo.push_back( apiInfo ) ;
			real_APIAddress = NULL ;

		}
	}
	return bRet ;
}



HANDLE WINAPI CAPIHook::try_CreateFileW(LPCWSTR lpFileName,
										DWORD dwDesiredAccess,
										DWORD dwShareMode,
										LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										DWORD dwCreationDisposition,
										DWORD dwFlagsAndAttributes,
										HANDLE hTemplateFile)
{
	if(GetDetachFlag() || HTTPEIsDisabled() == true )
	{
		if(!real_CreateFileW)
			return INVALID_HANDLE_VALUE;
		return real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	__try
	{
		return MyCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return INVALID_HANDLE_VALUE ;
}
HANDLE WINAPI CAPIHook::MyCreateFileW(LPCWSTR lpFileName,
									  DWORD dwDesiredAccess,
									  DWORD dwShareMode,
									  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
									  DWORD dwCreationDisposition,
									  DWORD dwFlagsAndAttributes,
									  HANDLE hTemplateFile)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	if(!real_CreateFileW)
		return INVALID_HANDLE_VALUE;
	if( (lpFileName==NULL) || (!CUtility::IsSupportFileType(lpFileName)))
	{
		return real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	wstring strFilePath = lpFileName ; 
	HANDLE hFile =real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
		dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	{	
		CMapperMngr& ins = CMapperMngr::Instance();
		DWORD dID = GetCurrentThreadId() ;
		ins.SaveLocalFileHandle( hFile,strFilePath,dID ) ;
	}
	return hFile ;

}

BOOL WINAPI CAPIHook::try_WriteFile(HANDLE hFile,
									LPCVOID lpBuffer,
									DWORD nNumberOfBytesToWrite,
									LPDWORD lpNumberOfBytesWritten,
									LPOVERLAPPED lpOverlapped)
{
	if( GetDetachFlag() ||HTTPEIsDisabled() == true )
	{
		if(real_WriteFile)
		{
			return real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
		}
	}
	__try
	{
		return MyWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}



	return FALSE ;
}


BOOL WINAPI CAPIHook::MyWriteFile(HANDLE hFile,
								  LPCVOID lpBuffer,
								  DWORD nNumberOfBytesToWrite,
								  LPDWORD lpNumberOfBytesWritten,
								  LPOVERLAPPED lpOverlapped)															    
{

	nextlabs::recursion_control_auto auto_disable(hook_control);
	if(!real_WriteFile)
		return FALSE;

	wchar_t szKey[100] = {0};
	_snwprintf_s(szKey, 100, _TRUNCATE, L"%d", (INT_PTR)hFile);
	
	DWORD dwSize = nNumberOfBytesToWrite;

	wstring strSize;
	if(g_listWriteFileCache.FindItem(szKey, strSize))
	{
		dwSize = (DWORD)::_wtoi(strSize.c_str());
		if(dwSize > MAX_FILE_CONTENT_SIZE)
		{
			return real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
		}
		else
		{
			dwSize += nNumberOfBytesToWrite;
		}
	}

	wchar_t szValue[100] = {0};
	_snwprintf_s(szValue, 100, _TRUNCATE, L"%d", dwSize);

	g_listWriteFileCache.AddItem(szKey, szValue);


	if(hFile != INVALID_HANDLE_VALUE &&
	   GetFileType(hFile) == FILE_TYPE_DISK &&
	   lpBuffer != NULL &&
	   nNumberOfBytesToWrite > 0)
	{
	
		CMapperMngr& ins = CMapperMngr::Instance();

		//	check if there is already a evaluation result with handle
		EWriteFile_EvalResult evalRes = ins.GetWriteEvalResultByHandle(hFile);
		if (WF_EvR_Deny == evalRes)
		{
			//	there is already a deny evaluation result
			//	so return directly.
			SetLastError(ERROR_ACCESS_DENIED); 
			g_listWriteFileCache.DeleteItem(szKey);
			return FALSE;
		}
		else if (WF_EvR_Allow == evalRes)
		{
			//	there is already an allow evaluation result
			//	so return directly.
			return real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
		}
	
		wstring sLocalPath = ins.GetLocalPathByHandle( hFile ) ;

		if(!CUtility::IsSupportFileType(sLocalPath.c_str()))
		{
			return real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
		}

		//	check if there is already a evaluation result with filename
		EWriteFile_EvalResult eEvalResWithFilename = WF_EvR_Unset;
		ins.GetWriteEvalResultByFileName(sLocalPath, eEvalResWithFilename);
		if (eEvalResWithFilename == WF_EvR_Deny)
		{
			//	before return, reset deny result.
			ins.SetWriteEvalResultByFileName(sLocalPath, WF_EvR_Deny);
			//	it is denied, so return directly.
			SetLastError(ERROR_ACCESS_DENIED); 
			g_listWriteFileCache.DeleteItem(szKey);
			return FALSE;
		}
		else if(eEvalResWithFilename == WF_EvR_Allow)
		{
			return real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
		}

		
		if (TestStringAppendError(lpBuffer, nNumberOfBytesToWrite))
		{
			return real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
		}
	
		string sBuf;	
		sBuf.append((char*)lpBuffer, nNumberOfBytesToWrite < CMapperMngr::MAX_CONTENT_SIZE? nNumberOfBytesToWrite: CMapperMngr::MAX_CONTENT_SIZE);
		

		//	modified by Ben, Jan 28 2010
		//	get the related downloading socket while getting the remote url for WS_FTP bug #10662
		//	check if the process is WS_FTP
		SOCKET s = 0;	//	0 is a invalid socket handle value.
		wstring sRemotePath;
		if (IsProcess(L"wsftpgui.exe"))
		{
			//	yes, the process is ws_ftp, get the related downloading socket while getting the remote url for bug #10662
			sRemotePath = ins.GetRemotePathByData_withSocket(sBuf, s);
		}
		else
		{
			//	no, the process is not ws_ftp, following the original logic
			sRemotePath = ins.GetRemotePathByData(sBuf);
		}

		//Try to get the IE temp path
		static wchar_t szIETempPath[MAX_PATH * 2 + 1] = {0};
		if( *szIETempPath == 0)
		{
			SHGetSpecialFolderPath(NULL, szIETempPath, CSIDL_INTERNET_CACHE, FALSE);
		}

		if(!IsProcess(L"devenv.exe") && !boost::algorithm::istarts_with(sLocalPath, szIETempPath))
		{//Don't need to call GetLongPathNameW for the path which contains IE temp path, since IE temp path is a complete path, these code can improve the performance.
		wchar_t szPath[MAX_PATH * 2 + 1] = {0};
		if(	GetLongPathNameW(sLocalPath.c_str(), szPath, MAX_PATH * 2) > 0)
		{
			sLocalPath = wstring(szPath);
		}
		}

		if(IsProcess(L"FireFox.exe") && g_listDeniedPath.FindItem(sLocalPath ))
		{
			SetLastError(ERROR_ACCESS_DENIED); 
			g_log.Log(CELOG_DEBUG, L"HTTPE::myWriteFile, this file was denied. %s", sLocalPath.c_str());
			g_listWriteFileCache.DeleteItem(szKey);
			return FALSE;
		}

		/***************************************************************************************
		There will have 2 processes involved if we try to download a file with "embedded Explorer"
		in IE (Sharepoint).
		For this case, IE will download the file data, and then Explorer will try to write the 
		local file with these data.
		Here, we used "shared memory" to resolve the communication between IE and Explorer.
		****************************************************************************************/
		if(!sLocalPath.empty() && sRemotePath.empty() && IsProcess(L"explorer.exe") )
		{//Try to get the URL from shared memory.
			static CSharedMemory sm_IE2(SHARED_MEMORY_NAME_DOWNLOAD_EMBEDDED_EXPLORER, MAX_FILEDATA_SIZE + MAX_URL_SIZE + 4);
			char* pTemp = new char[MAX_FILEDATA_SIZE + MAX_URL_SIZE + 4];
			if(pTemp)
			{
				memset(pTemp, 0, MAX_FILEDATA_SIZE + MAX_URL_SIZE + 4);
				if(sm_IE2.ReadSharedMemory(pTemp, MAX_FILEDATA_SIZE + MAX_URL_SIZE))
				{
					char szURL[MAX_URL_SIZE] = {0};
					memcpy_s(szURL, MAX_URL_SIZE, pTemp, MAX_URL_SIZE);
					unsigned uDataLen = 0;
					memcpy_s(&uDataLen, 4, pTemp + MAX_URL_SIZE, 4);
					string strFileData(pTemp + MAX_URL_SIZE + 4, uDataLen);

					if(strFileData.find(sBuf) != wstring::npos || sBuf.find(strFileData) != wstring::npos)
					{
						string temp = string(szURL);
						sRemotePath = wstring(temp.begin(), temp.end());
					}
					g_log.Log(CELOG_DEBUG, "HTTPE::Get the URL from shared memory. %s", szURL);

				}
				delete []pTemp;
				pTemp = NULL;
			}
		}
		
		//We will try to do evaluations if we got 2 paths.
		if(!sLocalPath.empty() && !sRemotePath.empty())
		{
			if(IsProcess(L"iexplore.exe"))//Ignore to do evaluation if the destination is temp folder of IE.
			{
				static wchar_t szIETemp[MAX_PATH * 2 + 1] = {0};
				if( *szIETemp == 0)
				{
					SHGetSpecialFolderPath(NULL, szIETemp, CSIDL_INTERNET_CACHE, FALSE);
				}
				
				if(sLocalPath.length() > wcslen(szIETemp) && _memicmp(sLocalPath.c_str(), szIETemp, wcslen(szIETemp) * sizeof(wchar_t)) == 0)
				{
					g_Temp_DownloadURIList.AddItem(sLocalPath, sRemotePath);
					/*
						Removed by chellee for the bug 863, also do the evaluation for the temp file...
					*/
				}
			}

			if(IsProcess(L"chrome.exe") || IsProcess(L"firefox.exe"))//Ignore to do evaluations if the destination is temp folder.
			{
				static wchar_t szTemp[MAX_PATH * 2 + 1] = {0};
				if( *szTemp == 0)
				{
					SHGetSpecialFolderPath(NULL, szTemp, CSIDL_PROFILE, FALSE);
				}
				wstring strTemp = wstring(szTemp) + L"\\Local Settings\\Temp";//the temp folder of Chrome/Firefox

				DWORD dwMajor = 0, dwMinor = 0;
				if(GetOSInfo(dwMajor, dwMinor) && dwMajor == 6)//Vista, win7
				{//don't use temp folder to determine.
					strTemp = L"";
				}

				if(sLocalPath.length() > strTemp.length() && _memicmp(sLocalPath.c_str(), strTemp.c_str(), strTemp.length() * sizeof(wchar_t)) == 0)
				{//The local file is in temp folder, ignore to do evaluations, just add the related information in list. "my_MoveFile" will try to get the URL with this list.
					g_Temp_DownloadURIList.AddItem(sLocalPath, sRemotePath);
					/*
						Removed by chellee for the bug 863, also do the evaluation for the temp file...
					*/
				}
				else
				{
					if( IsProcess(L"chrome.exe") && dwMajor< 6) 
					{
						g_log.Log(CELOG_DEBUG, L"Add to the download list  %s; %s\r\n", sLocalPath.c_str(),sRemotePath.c_str());
						g_Temp_DownloadURIList.AddItem(sLocalPath, sRemotePath);
					}
				}
			}

			if(CHttpProcessor::DoEvaluation(HTTP_OPEN, sRemotePath, sLocalPath) != 0)//denied
			{
				SetLastError(ERROR_ACCESS_DENIED); 

				if(IsProcess(L"opera.exe") && sRemotePath.find(L"http://docs.google") != wstring::npos && sLocalPath.length() > 6 && sLocalPath.find(L".htm") == sLocalPath.length() - 4)
				{
					wchar_t szHandle[20] = {0};
					_snwprintf_s(szHandle, 20, _TRUNCATE, L"%p", hFile);
					g_listDeniedHandle.AddItem(szHandle, sLocalPath);
					g_log.Log(CELOG_DEBUG, L"HTTPE::Add the file handle and file path to a list. this file will be deleted in \"closehandle\". %s\r\n", sLocalPath.c_str());
					return real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
				}

				//	if the process is ws_ftp, we need to set the related socket handle to a invalid status
				//	for bug #10662
				if(IsProcess(L"wsftpgui.exe") && s != 0)
				{
					closesocket(s);
					//	return directly, we don't set any WF_EvR_Deny into cache,
					//	we want ws_ftp go through mywritefile every time, then if the writefile is denied, 
					//	we can have chance to close socket here
					g_listWriteFileCache.DeleteItem(szKey);
					return FALSE;
				}

				ins.SetWriteEvalResultByHandle(hFile, WF_EvR_Deny);
				ins.SetWriteEvalResultByFileName(sLocalPath, WF_EvR_Deny);
				
				g_listWriteFileCache.DeleteItem(szKey);
				return FALSE;
			}
			ins.SetWriteEvalResultByFileName(sLocalPath, WF_EvR_Allow);
		}
			
	}
	
	return real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

BOOL WINAPI CAPIHook::try_CloseHandle(HANDLE hObject)
{
	if( GetDetachFlag() || HTTPEIsDisabled() == true )
	{
	if( real_CloseHandle)
	{
	return real_CloseHandle(hObject) ; 
	}

	}
	__try
	{
		return MyCloseHandle(hObject) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	return FALSE;
}

BOOL WINAPI CAPIHook::MyCloseHandle(HANDLE hObject)
{

	if(!real_CloseHandle)
		return FALSE;

	BOOL bRet = real_CloseHandle(hObject);

	if(IsProcess(L"opera.exe"))
	{
		wchar_t szHandle[20] = {0};
		_snwprintf_s(szHandle, 20, _TRUNCATE, L"%p", hObject);
		wstring strValue;
		if(g_listDeniedHandle.FindItem(szHandle, strValue))
		{
			g_listDeniedHandle.DeleteItem(szHandle);
			g_log.Log(CELOG_DEBUG, L"HTTPE::Delete file, %s", strValue.c_str());
			DeleteFile(strValue.c_str());
		}
	}
	
	return bRet;
}



BOOL WINAPI CAPIHook::try_ReadFile(
								   HANDLE hFile,
								   LPVOID lpBuffer,
								   DWORD nNumberOfBytesToRead,
								   LPDWORD lpNumberOfBytesRead,
								   LPOVERLAPPED lpOverlapped
								   )
{
	if( GetDetachFlag() || HTTPEIsDisabled() == true )
	{
		if (!real_ReadFile)
			return NULL;
		return real_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,	lpNumberOfBytesRead, lpOverlapped) ;
	}
	__try
	{
		return my_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,	lpNumberOfBytesRead, lpOverlapped) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return 0 ;

}
BOOL WINAPI CAPIHook::my_ReadFile(
								  HANDLE hFile,
								  LPVOID lpBuffer,
								  DWORD nNumberOfBytesToRead,
								  LPDWORD lpNumberOfBytesRead,
								  LPOVERLAPPED lpOverlapped
								  )
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	if (!real_ReadFile)
		return FALSE;

	BOOL bRet =  real_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,	lpNumberOfBytesRead, lpOverlapped) ;
	if((!bRet))
	{
		return	bRet ;
	}

	if(hFile == INVALID_HANDLE_VALUE ||
	   //GetFileType(hFile) != FILE_TYPE_DISK ||		//	comments, 2011/March/16, for Bug 13787, for example, upload c:\kaka\1.txt to yahoo mail, the handle for \\?\c:\kaka\1.txt is FILE_TYPE_PIPE
	   lpBuffer == NULL ||
	   nNumberOfBytesToRead <= 0||(	lpOverlapped != NULL &&	lpOverlapped->Offset != 0 ))
	{
		return bRet;  
	}

	wchar_t szKey[100] = {0};
	_snwprintf_s(szKey, 100, _TRUNCATE, L"%d", (INT_PTR)hFile);

	DWORD dwSize = lpNumberOfBytesRead? *lpNumberOfBytesRead: nNumberOfBytesToRead;

	wstring strSize;
	if(g_listReadFileCache.FindItem(szKey, strSize))
	{
		dwSize = (DWORD)::_wtoi(strSize.c_str());
		if(dwSize > MAX_FILE_CONTENT_SIZE)
		{
			return bRet;
		}
		else
		{
			dwSize += lpNumberOfBytesRead? *lpNumberOfBytesRead: nNumberOfBytesToRead;
		}
	}

	wchar_t szValue[100] = {0};
	_snwprintf_s(szValue, 100, _TRUNCATE, L"%d", dwSize);

	g_listReadFileCache.AddItem(szKey, szValue);

	
	std::wstring strFilePath ;
	if( (!GetFileNameFromHandle(	hFile,strFilePath ) )||(!CUtility::IsSupportFileType( strFilePath.c_str())))
	{
		if( strFilePath.empty() )
		{
			CMapperMngr& ins = CMapperMngr::Instance();
			strFilePath = ins.GetLocalPathByHandle( hFile ) ;
			if((strFilePath.empty()) ||(!CUtility::IsSupportFileType( strFilePath.c_str())))
			{
				return bRet; 
			}
		}
		else
		{
			return bRet; 
		}
	}

	if(lpNumberOfBytesRead && *lpNumberOfBytesRead > 0) 
	{	  
		string sBuf;
		sBuf.append((char*)lpBuffer, *lpNumberOfBytesRead < CMapperMngr::MAX_CONTENT_SIZE? *lpNumberOfBytesRead: CMapperMngr::MAX_CONTENT_SIZE);

		CMapperMngr& ins = CMapperMngr::Instance();
		ins.SaveLocalHandleAndContent(	hFile, sBuf,  strFilePath) ;
		// This is the end of the file. 
	}
	return bRet ;
}
/*
IE & Chrome encrypt message for HTTPS
*/
SECURITY_STATUS WINAPI CAPIHook::try_EncryptMessage( PVOID phContext,ULONG fQOP, PVOID pMessage, ULONG MessageSeqNo )
{
	if( GetDetachFlag() ||HTTPEIsDisabled() == true )
	{
		if (!real_EncryptMessage)
			return NULL;
		return real_EncryptMessage(phContext, fQOP, pMessage,	MessageSeqNo) ;
	}
	__try
	{
		return my_EncryptMessage(phContext, fQOP, pMessage,	MessageSeqNo) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return 0 ;
}

SECURITY_STATUS WINAPI CAPIHook::my_EncryptMessage( PVOID phContext,ULONG fQOP, PVOID pMessage, ULONG MessageSeqNo )
{

	nextlabs::recursion_control_auto auto_disable(hook_control);
	if (!real_EncryptMessage)
		return NULL;
	SECURITY_STATUS status =  SEC_E_INVALID_HANDLE ;
	PSecBufferDesc pSecBuf =  (PSecBufferDesc)pMessage ;
	if( pSecBuf->cBuffers  > 0 )
	{
		ULONG i = 0 ;
		string plain = "";	

		while(  i < pSecBuf->cBuffers )
		{
			if ( pSecBuf->pBuffers[i].BufferType == SECBUFFER_DATA )
			{
				SecBuffer secBuf = pSecBuf->pBuffers[i];	
				if( secBuf.pvBuffer && secBuf.cbBuffer > 0)
				{
					plain.append( (char*)secBuf.pvBuffer, secBuf.cbBuffer ); 
				}
			}
			i++ ;
		}
		
		status = real_EncryptMessage(phContext, fQOP, pMessage,	MessageSeqNo ) ;
			
		if( status == SEC_E_OK )
			{
				i = 0   ;
				string encrypt = "" ;
				while(  i < pSecBuf->cBuffers )
				{
					if ( pSecBuf->pBuffers[i].BufferType == SECBUFFER_DATA )
					{
						SecBuffer secBuf = pSecBuf->pBuffers[i];	
						if(secBuf.pvBuffer && secBuf.cbBuffer > 0)
						{
							encrypt.append((char*)secBuf.pvBuffer, secBuf.cbBuffer);	
						}
					}
					i++ ;
				}
				
				CMapperMngr& ins = CMapperMngr::Instance();
				ins.MapEncryptAndDecryptData( plain,encrypt) ;
				
			}
	}
	else
	{
		status = real_EncryptMessage(phContext, fQOP, pMessage,	MessageSeqNo ) ;
	}

	return	  status ;

}
/*
IE & Chorme Decrypt message for HTTPS
*/
SECURITY_STATUS WINAPI CAPIHook::try_DecryptMessage(PVOID phContext,  PVOID pMessage,  ULONG MessageSeqNo,  PVOID pfQOP )
{
	if(GetDetachFlag() || HTTPEIsDisabled() == true )
	{
		if (!real_DecryptMessage)
			return NULL;
		return real_DecryptMessage(phContext, pMessage, MessageSeqNo,	pfQOP ) ;
	}
	__try
	{
		return my_DecryptMessage(phContext, pMessage, MessageSeqNo,	pfQOP ) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return NULL ;
}
SECURITY_STATUS WINAPI CAPIHook::my_DecryptMessage(PVOID phContext,  PVOID pMessage,  ULONG MessageSeqNo,  PVOID pfQOP )
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	if (!real_DecryptMessage)
		return NULL;
	SECURITY_STATUS status =  SEC_E_INVALID_HANDLE ;
	PSecBufferDesc pSecBuf =  (PSecBufferDesc)pMessage ;
	if( pSecBuf->cBuffers > 0 )
	{
		ULONG i = 0 ;
		string encrypt = "" ; 

		while(  i < pSecBuf->cBuffers )
			{
				if ( pSecBuf->pBuffers[i].BufferType == SECBUFFER_DATA )
				{
					SecBuffer secBuf = pSecBuf->pBuffers[i];	
					if(secBuf.pvBuffer && secBuf.cbBuffer > 0)
					{
						string strData((char *)secBuf.pvBuffer , secBuf.cbBuffer);
						encrypt += strData;	
					}
				}
				i++ ;
			}

		status = real_DecryptMessage(phContext, pMessage, MessageSeqNo,	pfQOP ) ;

		if( status == SEC_E_OK )
		{
			i = 0   ;
			string plain = ""; 
			while(  i < pSecBuf->cBuffers )
				{
					if ( pSecBuf->pBuffers[i].BufferType == SECBUFFER_DATA )
					{
						SecBuffer secBuf = pSecBuf->pBuffers[i];	
						if( secBuf.pvBuffer && secBuf.cbBuffer > 0)
						{
							string strData((char *)secBuf.pvBuffer , secBuf.cbBuffer);
							plain += strData; 
						}
					}
					i++ ;
				}

			CMapperMngr& ins = CMapperMngr::Instance();

			SOCKET s;
			vector<SOCKET> handledSocks;
			BOOL bFound = FALSE;
			wchar_t szContext[20] = {0};
			_snwprintf_s(szContext, 20, _TRUNCATE, L"%d", (INT_PTR)phContext);

			while(ins.GetSocketByEncryptedData(encrypt, s))
			{
				/***********************************************************
				Add the socket and context to map.
				Sometimes, the match with "enctryped dat" will fail. 
				if so, we can use context value as the key to find the related 
				socket id.
				************************************************************/
				if(IsProcess(L"iexplore.exe") || IsProcess(L"chrome.exe"))
				{
					bFound = TRUE;
					wchar_t szSocket[20] = {0};
					_snwprintf_s(szSocket, 20, _TRUNCATE, L"%d", s);
					g_listContextDecrypt.AddItem(szContext, szSocket);
				}

				if(find(handledSocks.begin(), handledSocks.end(), s) == handledSocks.end())
				{
					CHttpMgr& mgr = CHttpMgr::GetInstance();
					/*
					Remove HTTPS redirect, right now it has some problem
					*/
					mgr.ProcessHTTPData2(s, plain, true, 1);   		
					handledSocks.push_back(s);
				}
			}

			if( (IsProcess(L"iexplore.exe") || IsProcess(L"chrome.exe") ) && !bFound)//it means "match with encrypted data" failed.
			{//Use context to find the related socket id. (the context value should be same in one transaction)
				wstring strValue;
				if(g_listContextDecrypt.FindItem(szContext, strValue))
				{
					SOCKET sock = _wtoi(strValue.c_str());

					CHttpMgr& mgr = CHttpMgr::GetInstance();

					mgr.ProcessHTTPData2(sock, plain, true, 1);   		
					handledSocks.push_back(sock);

					g_log.Log(CELOG_DEBUG, "HTTPE::(API DecryptMessage, handle \"Download with HTTPS\")Use context address to find the same transaction(it will be used when the \"match with encrypted data\" failed.). socket: %d\r\n", sock);
				}

			}

		}
	}
	else
	{
		status =  real_DecryptMessage(phContext, pMessage, MessageSeqNo,	pfQOP ) ;
	}

	return	  status ;

}

BOOL WINAPI CAPIHook::try_CopyFileW(_In_ LPCTSTR lpExistingFileName, _In_ LPCTSTR lpNewFileName, _In_ BOOL bFailIfExists )
{g_log.Log(CELOG_DEBUG, L"CopyFile, src: %s, dest: %s", lpExistingFileName?lpExistingFileName: L"NULL", lpNewFileName?lpNewFileName: L"NULL");
	if( GetDetachFlag() ||HTTPEIsDisabled() == true )
	{
		if( real_CopyFileW)
		{
			return real_CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists) ; 
		}

	}
	__try
	{
		return my_CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists) ; 
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	return FALSE;
}

BOOL CAPIHook::my_CopyFileW(_In_ LPCTSTR lpExistingFileName, _In_ LPCTSTR lpNewFileName, _In_ BOOL bFailIfExists )
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	return real_CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists) ; 
}

BOOL WINAPI CAPIHook::try_CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{

	if( GetDetachFlag() ||HTTPEIsDisabled() == true )
	{
		if( real_CopyFileExW)
		{
			return real_CopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags) ; 
		}

	}
	__try
	{
		return my_CopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags) ; 
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	return FALSE;
}

BOOL Handle_IE_COPY(wstring strSrc, wstring strDest)
{
	static wchar_t buffer[MAX_PATH * 2 + 1] = {0};
	if( *buffer == 0 )
	{
		SHGetSpecialFolderPath(NULL, buffer, CSIDL_INTERNET_CACHE, FALSE);
	}

	if(strSrc.length() > wcslen(buffer) && _memicmp(strSrc.c_str(), buffer, wcslen(buffer) * sizeof(wchar_t) ) == 0)
	{
		wstring strRemoteURL;
		if(g_Temp_DownloadURIList.FindItem(strSrc, strRemoteURL))
		{
			//	add by Ben, Nov, 30th, 2009
			//	reason: the IE will retry my_CopyFileExW if download file from mail.cn.nextlabs.com

			g_log.Log(CELOG_DEBUG, L"HTTPE::my_CopyFileExW/SHFileOperationW, from temp folder, \"%s\"", strSrc.c_str());
			if(CHttpProcessor::DoEvaluation(HTTP_OPEN, strRemoteURL, strDest) != 0)//denied.
			{
				return FALSE;
			}
		}
		else
		{
			/********************************************************
			for bug10806.
			This is a corner issue.
			The extension name of the office2007 will be converted to ZIP 
			in OWA.
			It seems it is caused by exchange server.
			IIS6 doesn't know office2007. so it convert it to a ZIP file.
			The problem is:
			temp file is something like: xxxxx\a.docx
			destination file is: xxxx\a.zip
			*********************************************************/
			std::wstring::size_type uIndex = strSrc.rfind(L".");
			if(uIndex != wstring::npos)
			{
				wstring strTempName = strSrc.substr(0, uIndex);
				wstring strOriKey;
				wstring strURI;
				if(g_Temp_DownloadURIList.ContainKey(strTempName, strOriKey, strURI))
				{
					g_log.Log(CELOG_DEBUG, L"HTTPE::Found the temp file of IE. file name: %s, temp file name: %s, \r\nURL: %s\r\ndest path: %s", strSrc.c_str(), strOriKey.c_str(), strURI.c_str(), strDest.c_str());
					uIndex = strOriKey.rfind(L".");
					if(uIndex != wstring::npos && _wcsicmp(strTempName.c_str(), strOriKey.substr(0, uIndex).c_str()) == 0)
					{
						if(CHttpProcessor::DoEvaluation(HTTP_OPEN, strURI, strDest) != 0)//denied.
						{
							return FALSE;
						}
					}
				}
			}


			//Check if IE uses "cache file", reponse: 304 NOT MODIFIED
			std::wstring::size_type uStart = wstring::npos;
			if((uStart = strSrc.rfind('[')) != wstring::npos)
			{
				size_t uEnd = strSrc.rfind(']');
				if(uEnd != wstring::npos && uEnd > uStart)
				{
					strSrc.replace(uStart, uEnd - uStart + 1, L"");
				}
			}
			uStart = strSrc.rfind('\\');
			if(uStart != wstring::npos)
			{
				wstring strFileName = strSrc.substr(uStart + 1, strSrc.length() - uStart - 1);

				//try to get the remote URL.
				std::wstring strValue;
				if(g_DownloadURIList.FindItem(strFileName, strValue))
				{
					g_log.Log(CELOG_DEBUG, L"HTTPE::(CopyFileEx)Do evaluation download: \r\n remote%s, \r\n local: %s", strValue.c_str(), strDest.c_str());
					//Do evaluation
					if(CHttpProcessor::DoEvaluation(HTTP_OPEN, strValue, strDest) != 0)//denied.
					{
						return FALSE;
					}
				}
			}
		}


	}

	return TRUE;
}
BOOL WINAPI CAPIHook::my_CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if(lpExistingFileName && lpNewFileName)
	{
		wstring strSrc(lpExistingFileName);
		wstring strDest(lpNewFileName);

		if(IsProcess(L"iexplore.exe"))
		{
			if(!Handle_IE_COPY(strSrc, strDest))
			{
				return FALSE;
			}
		}
		else if(IsProcess(L"chrome.exe"))
		{
			g_log.Log(CELOG_DEBUG, L"my_CopyFileExW, src: %s, dest: %s", strSrc.c_str(), strDest.c_str());

			if(g_listDeniedPath.FindItem(strSrc + strDest))
			{
				g_listDeniedPath.DeleteItem(strSrc + strDest);
				return FALSE;
			}

			static wchar_t szTemp[MAX_PATH * 2 + 1] = {0};
			if( *szTemp == 0 )
			{
				SHGetSpecialFolderPath(NULL, szTemp, CSIDL_PROFILE, FALSE);
			}
			wstring strTemp = wstring(szTemp) + L"\\Local Settings\\Temp";//the temp folder of Chrome

			DWORD dwMajor = 0, dwMinor = 0;
			if(GetOSInfo(dwMajor, dwMinor) && dwMajor == 6)//Vista, win7
			{//don't use temp folder to determine.
				strTemp = L"";
			}

			wstring strURL;
			if(GetURLWithTempPath(strSrc, strTemp, strURL))
			{
				if(CHttpProcessor::DoEvaluation(HTTP_OPEN, strURL, strDest) != 0)//denied.
				{
					return FALSE;
				}

			}
		}
		
	}
	
	return real_CopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags) ; 
}

BOOL CAPIHook::try_MoveFileExW(_In_ LPCWSTR lpExistingFileName, _In_ LPCWSTR lpNewFileName, _In_ DWORD dwFlags )
{g_log.Log(CELOG_DEBUG, L"try_MoveFileExW, src: %s, dest: %s", lpExistingFileName?lpExistingFileName: L"NULL", lpNewFileName?lpNewFileName: L"NULL");
	if(GetDetachFlag() || HTTPEIsDisabled() == true )
	{
		if( real_MoveFileExW)
		{
			return real_MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags ) ; 
		}

	}
	__try
	{
		return my_MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags ) ;  
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	return FALSE;
}

BOOL CAPIHook::my_MoveFileExW(_In_ LPCWSTR lpExistingFileName, _In_ LPCWSTR lpNewFileName, _In_ DWORD dwFlags )
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if((IsProcess(L"chrome.exe") || IsProcess(L"firefox.exe"))&& lpExistingFileName && lpNewFileName)
	{
		g_log.Log(CELOG_DEBUG, L"HTTPE::MoveFileEx, src: %s, dest: %s", lpExistingFileName, lpNewFileName);

		wchar_t szPath[MAX_PATH * 2 + 1] = {0};
		wstring sLocalPath(lpExistingFileName);
		if(	GetLongPathNameW(sLocalPath.c_str(), szPath, MAX_PATH * 2) > 0)
		{
			sLocalPath = szPath;
		}
		 if( IsProcess(L"firefox.exe") )
		{
			HANDLE hFile = real_CreateFileW(   sLocalPath.c_str(), GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING, NULL,NULL);            
			if( hFile != NULL )
			{
				DWORD dwFileSizeHi = 0 ;
				DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 
				if( dwFileSizeLo == 0 && dwFileSizeHi == 0 )
				{
					real_CloseHandle( hFile ) ;
					return real_MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags ) ; 
				}
				real_CloseHandle( hFile ) ;
			}
		}
		if(g_listDeniedPath.FindItem(sLocalPath))
		{
			g_listDeniedPath.DeleteItem(sLocalPath);
			g_log.Log(CELOG_DEBUG, L"HTTPE::MoveFileEx, This path was denied. %s", sLocalPath.c_str());
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}

		if ( !MovePhaseEval(lpExistingFileName, lpNewFileName) ) 
		{
			//	be denied
			if(IsProcess(L"firefox.exe"))
			{
				g_listDeniedPath.AddItem(sLocalPath);

				g_log.Log(CELOG_DEBUG, L"HTTPE::MoveFileEx, Add the denied file name to a list, so that \"myWriteFile\" can check this list to see if the current file was denied. %s", sLocalPath.c_str());
			}
		
			SetLastError(ERROR_WRITE_PROTECT);
			return FALSE;
		}
	}

	return real_MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags ) ; 
}

BOOL CAPIHook::try_MoveFileW(_In_ LPCWSTR lpExistingFileName, _In_ LPCWSTR lpNewFileName )
{g_log.Log(CELOG_DEBUG, L"HTTPE::MoveFile, src: %s, dest: %s", lpExistingFileName, lpNewFileName);
	if( GetDetachFlag() ||HTTPEIsDisabled() == true )
	{
		if( real_MoveFileW)
		{
			return real_MoveFileW(lpExistingFileName, lpNewFileName) ; 
		}

	}
	__try
	{
		return my_MoveFileW(lpExistingFileName, lpNewFileName) ; 
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	return FALSE;
}

BOOL CAPIHook::my_MoveFileW(_In_ LPCWSTR lpExistingFileName, _In_ LPCWSTR lpNewFileName )
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if((IsProcess(L"chrome.exe") || IsProcess(L"firefox.exe"))&& lpExistingFileName && lpNewFileName)
	{
		g_log.Log(CELOG_DEBUG, L"HTTPE::MoveFile, src: %s, dest: %s", lpExistingFileName, lpNewFileName);

		wchar_t szPath[MAX_PATH * 2 + 1] = {0};
		wstring sLocalPath(lpExistingFileName);
		if(	GetLongPathNameW(sLocalPath.c_str(), szPath, MAX_PATH * 2) > 0)
		{
			sLocalPath = szPath;
		}

		if(g_listDeniedPath.FindItem(sLocalPath))
		{
			g_listDeniedPath.DeleteItem(sLocalPath);
			g_log.Log(CELOG_DEBUG, L"HTTPE::MoveFile, This path was denied. %s", sLocalPath.c_str());
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}
		if( IsProcess(L"firefox.exe") )
		{
			HANDLE hFile = real_CreateFileW(   sLocalPath.c_str(), GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING, NULL,NULL);            
			if( hFile != NULL )
			{
				DWORD dwFileSizeHi = 0 ;
				DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 
				if( dwFileSizeLo == 0 && dwFileSizeHi == 0 )
				{
					g_log.Log(CELOG_DEBUG, "Cannot evaluation a file with a length of zero.\n");
					real_CloseHandle( hFile ) ;
					return  real_MoveFileW(lpExistingFileName, lpNewFileName) ; 
				}
				real_CloseHandle( hFile ) ;
			}
		}
		if ( !MovePhaseEval(lpExistingFileName, lpNewFileName) ) 
		{
			//	be denied
			if(IsProcess(L"firefox.exe"))
			{
				g_listDeniedPath.AddItem(sLocalPath);

				g_log.Log(CELOG_DEBUG, L"HTTPE::MoveFile, Add the denied file name to a list, so that \"myWriteFile\" can check this list to see if the current file was denied. %s", sLocalPath.c_str());
			}

			SetLastError(ERROR_WRITE_PROTECT);
			return FALSE;
		}
	}
	
	return real_MoveFileW(lpExistingFileName, lpNewFileName) ; 
}


int CAPIHook::try_PK11_CipherOp(void *context, unsigned char *out, int *outlen, int maxout, unsigned char *in, int inlen)
{
	if(GetDetachFlag() || HTTPEIsDisabled() == true )
	{
		if( real_PK11_CipherOp)
		{
			return real_PK11_CipherOp(context, out, outlen, maxout, in, inlen) ;
		}

	}
	__try
	{
		return my_PK11_CipherOp(context, out, outlen, maxout, in, inlen) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	return 0 ;
}
int CAPIHook::my_PK11_CipherOp(void *context, unsigned char * out, int *outlen, 
									int maxout, unsigned char *in, int inlen)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	if (!real_PK11_CipherOp)
	{
		return 0;
	}

	if(!context || !in)
	{
		return real_PK11_CipherOp(context, out, outlen, maxout, in, inlen);
	}
	
	/************************************************************************
	context is a structure in Firefox.
	We only care the 4th bytes, it contains the type of "PK11_CipherOp".
	0x104 is Encrypt
	0x105 is Decrypt
	************************************************************************/
	unsigned long int uType = 0;
	memcpy_s(&uType, sizeof(unsigned long int), context, sizeof(unsigned long int));

	string strIn((char*)in, inlen);
	if(FIREFOX_ENCRYPT == uType)//Try to get the plain text. the "in" is possible to be changed after call "PK11_CipherOp", so we need to get it before this call.
	{
		/**************************************************************
		Check Digest
		Maybe the plain text contains "digestfinal",
		like:
		12345678xxxxxx
		xxxxx is a "digestfinal" which generated in PK11_DigestFinal
		We need to remove the "digestfinal" if plain text contains this.
		****************************************************************/
		std::string::size_type uIndex = strIn.find(m_strLastDigest);
		if(uIndex != string::npos)
		{
			strIn = strIn.substr(0, uIndex); 
		}

	}

	int iRet = real_PK11_CipherOp(context, out, outlen, maxout, in, inlen);
	
	
	if(out && in && outlen)
	{
		//For Navigation and Upload
		CMapperMngr& ins = CMapperMngr::Instance();
		if(FIREFOX_ENCRYPT == uType)
		{
			string plain = strIn;
			string encrypt((char*)out, *outlen);
			
			ins.MapEncryptAndDecryptData( plain,encrypt);
		}
		else if(FIREFOX_DECRYPT == uType)//For downloand
		{
			string encrypt = strIn;
			string plain((char*)out, *outlen);
			

			SOCKET s;
			if(ins.GetSocketByEncryptedData(encrypt, s))
			{
				WCHAR szSocket[10] = {0};
				_snwprintf_s(szSocket, 10, _TRUNCATE, L"%d", s);
				m_listFireFox_Download.AddItem(wstring(plain.begin(), plain.end()), szSocket);  				
			}
		}
	}
	

	return	  iRet ;
}

BOOL CAPIHook::MovePhaseEval(const wstring & srcMoveFile, const wstring & dstMoveFile)
{
	static wchar_t szTemp[MAX_PATH * 2 + 1] = {0};
	if( *szTemp == 0 )
	{
		SHGetSpecialFolderPath(NULL, szTemp, CSIDL_PROFILE, FALSE);
	}
	wstring strTemp = wstring(szTemp) + L"\\Local Settings\\Temp";//the temp folder of Chrome
	
	DWORD dwMajor = 0, dwMinor = 0;
	if(GetOSInfo(dwMajor, dwMinor) && dwMajor == 6)//Vista, win7
	{
		strTemp = L"";
	}

	wstring strURL;
	if(GetURLWithTempPath(srcMoveFile, strTemp, strURL))
	{
		if(CHttpProcessor::DoEvaluation(HTTP_OPEN, strURL, dstMoveFile) != 0)//denied.
		{
			return FALSE;
		}
	}
	return TRUE;
}

int CAPIHook::try_PK11_DigestFinal(void *context, unsigned char *data, unsigned int *outLen, unsigned int length)
{
	if(GetDetachFlag() || HTTPEIsDisabled() == true )
	{
		if( real_PK11_DigestFinal)
		{
			return real_PK11_DigestFinal(context, data, outLen, length);
		}

	}
	__try
	{
		return my_PK11_DigestFinal(context, data, outLen, length) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	return 0 ;

	
}

int CAPIHook::my_PK11_DigestFinal(void *context, unsigned char *data, unsigned int *outLen, unsigned int length)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if(real_PK11_DigestFinal)
	{
		int nRet = real_PK11_DigestFinal(context, data, outLen, length);
		string strDigest((char*)data, *outLen);
		m_strLastDigest = strDigest;

		wstring strValue;
		wstring strOriginalKey;
		if(m_listFireFox_Download.ContainKey(wstring(strDigest.begin(), strDigest.end()), strOriginalKey, strValue, true))
		{
			std::string::size_type uIndex = string::npos;
			if((uIndex = strOriginalKey.find(wstring(strDigest.begin(), strDigest.end()))) != string::npos)
			{
				strOriginalKey = strOriginalKey.substr(0, uIndex);
				CHttpMgr& mgr = CHttpMgr::GetInstance();
				mgr.ProcessHTTPData2(_wtoi(strValue.c_str()), string(strOriginalKey.begin(), strOriginalKey.end()), true, 1);  
			}

		}

		return nRet;
	}
	return 0;
}


BOOL CAPIHook::try_MoveFileWithProgressW(_In_          LPCWSTR lpExistingFileName,
										_In_          LPCWSTR lpNewFileName,
										_In_          LPPROGRESS_ROUTINE lpProgressRoutine,
										_In_          LPVOID lpData,
										_In_          DWORD dwFlags
										)
{g_log.Log(CELOG_DEBUG, L"MoveFileWithProgressW, src: %s, dest: %s", lpExistingFileName, lpNewFileName);
	if(GetDetachFlag() || HTTPEIsDisabled() == true )
	{
		if( real_MoveFileWithProgressW)
		{
			return real_MoveFileWithProgressW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
		}

	}
	__try
	{
		return my_MoveFileWithProgressW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	return 0 ;
	
}

BOOL CAPIHook::my_MoveFileWithProgressW(_In_          LPCWSTR lpExistingFileName,
										_In_          LPCWSTR lpNewFileName,
										_In_          LPPROGRESS_ROUTINE lpProgressRoutine,
										_In_          LPVOID lpData,
										_In_          DWORD dwFlags
										)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if(lpExistingFileName && lpNewFileName)
	{
		if(IsProcess(L"Chrome.exe"))
		{
			wstring strSrc(lpExistingFileName);
			wstring strDest(lpNewFileName);

			g_log.Log(CELOG_DEBUG, L"my_MoveFileWithProgressW, src: %s, dest: %s", strSrc.c_str(), strDest.c_str());

			static wchar_t szTemp[MAX_PATH * 2 + 1] = {0};
			if( *szTemp == 0 )
			{
				SHGetSpecialFolderPath(NULL, szTemp, CSIDL_PROFILE, FALSE);
			}
			wstring strTemp = wstring(szTemp) + L"\\Local Settings\\Temp";//the temp folder of Chrome

			DWORD dwMajor = 0, dwMinor = 0;
			if(GetOSInfo(dwMajor, dwMinor) && dwMajor == 6)//Vista, win7
			{//don't use temp folder to determine.
				strTemp = L"";
			}

			wstring strURL;
			if(GetURLWithTempPath(strSrc, strTemp, strURL))
			{
				if(CHttpProcessor::DoEvaluation(HTTP_OPEN, strURL, strDest) != 0)//denied.
				{
					g_listDeniedPath.AddItem(strSrc + strDest);
					return FALSE;
				}

			}
		}
	}
	
	return real_MoveFileWithProgressW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
}


int CAPIHook::try_SHFileOperationW(LPSHFILEOPSTRUCT lpFileOp )
{
	if(GetDetachFlag() || HTTPEIsDisabled() == true )
	{
		if( real_SHFileOperationW)
		{
			return real_SHFileOperationW(lpFileOp);
		}

	}
	__try
	{
		return my_SHFileOperationW(lpFileOp);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	return 0 ;
}

int CAPIHook::my_SHFileOperationW(LPSHFILEOPSTRUCT lpFileOp )
{
	if(lpFileOp && lpFileOp->pFrom && lpFileOp->pTo)
	{
		g_log.Log(CELOG_DEBUG, L"HTTPE::SHFileOperationW, type: %d, src: %s, dest: %s\r\n", lpFileOp->wFunc, lpFileOp->pFrom, lpFileOp->pTo);
		switch(lpFileOp->wFunc)
		{
		case FO_COPY:
			{
				if(IsProcess(L"iexplore.exe"))
				{
					if(!Handle_IE_COPY(lpFileOp->pFrom, lpFileOp->pTo))
					{
						return 0x78; //DE_ACCESSDENIEDSRC
					}
				}
			}
			break;
		case FO_MOVE://Chrome.exe will call this function to move the file to destination user user tries to download a file to share folder.
			{
				if(lpFileOp->pFrom && lpFileOp->pTo)
				{
					if(IsProcess(L"Chrome.exe"))
					{
						wstring strSrc(lpFileOp->pFrom);
						wstring strDest(lpFileOp->pTo);

						g_log.Log(CELOG_DEBUG, L"my_SHFileOperationW, src: %s, dest: %s\r\n", strSrc.c_str(), strDest.c_str());

						wstring strURL;
						if(GetURLWithTempPath(strSrc, L"", strURL))
						{
							if(CHttpProcessor::DoEvaluation(HTTP_OPEN, strURL, strDest) != 0)//denied.
							{
								g_listDeniedPath.AddItem(strSrc + strDest);
								return 0x78;//denied
							}
							
						}
					}
				}
			}
			break;
		default:
			break;
		}
	}
	return real_SHFileOperationW(lpFileOp);
}

HANDLE WINAPI CAPIHook::try_CreateFileA(
										LPCSTR lpFileName,
										DWORD dwDesiredAccess,
										DWORD dwShareMode,
										LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										DWORD dwCreationDisposition,
										DWORD dwFlagsAndAttributes,
										HANDLE hTemplateFile)
{
	if(lpFileName)
	{
		string strFileName(lpFileName);
		wstring wstrFileName = MyMultipleByteToWideChar(strFileName);

		return try_CreateFileW(wstrFileName.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	else
		return real_CreateFileA(lpFileName,dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
								dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
