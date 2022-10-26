#include "stdafx.h"
#include "APIHook.h"

#include "stdlib.h"
#include "WTypes.h"
#include "ObjBase.h"
#include <shlobj.h>

#pragma comment(lib, "Dbghelp.lib")
#pragma warning( push )
#pragma warning( disable : 4819 )
#include "madCHook_helper.h"
#pragma warning( pop )

#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
using namespace std;
#include "MapperMgr.h"
#include "nlexcept.h"
#include "sftplibImpl.h"
#include "ModulVer.h"

#include "eframework/auto_disable/auto_disable.hpp"
#include "eframework/platform/cesdk_loader.hpp"
extern nextlabs::cesdk_loader cesdkLoader;

CreateFileWType2 g_realCreateFileW = NULL;
CloseHandleType2 g_realCloseHandle = NULL;


CAPIHook::CreateFileWType    CAPIHook::real_CreateFileW    = NULL;
CAPIHook::CreateFileAType    CAPIHook::real_CreateFileA    = NULL;
CAPIHook::CloseHandleType    CAPIHook::real_CloseHandle    = NULL;
CPolicy *CAPIHook::m_pEvaluaImp = NULL ;
HRESULT*   CAPIHook::real_CoCreateInstance   = NULL;
CAPIHook::TransmitFileType CAPIHook::real_TransmitFile = NULL;

std::list<APIRES::HOOKAPIINFO>	CAPIHook::m_listSmartFtp_UpFile ;

const wchar_t* g_szFilterFileTypes[] ={	
									L".DOC",
									L".XLS",
									L".PPT",
									L".DOCX",
									L".XLSX",
									L".PPTX",
									L".PDF",
									L".MPP",
									L".TXT"
							    };

const wchar_t* g_szSpecialPath_End[] =	{
											L"\\PIPE\\WKSSVC",
											L"\\PIPE\\SRVSVC",
											L"\\PIPE\\SAMR",
											L"\\PIPE\\LSARPC",
											L"\\PIPE\\NETLOGON",
											L"\\PIPE\\BROWSER"
										};

const wchar_t* g_szSpecailPath[] = {
									L"\\\\.\\PIPE",
									L"\\\\.\\WMIDATADEVICE",
									L"\\\\?\\ROOT#SYSTEM#0000#",
									L"ROOT#SYSTEM#0000#",
									L"\\\\.\\PHYSICALDRIVE",
									L"CONIN$",
									L"CONOUT$",
									L"\\\\.\\MAILSLOT",//fix bug9756
									L"\\\\?\\\\\\.\\MAILSLOT",
									L"\\\\.\\"
							 };

const std::pair<int, std::wstring> g_szIgnoredFolders[] =	
{
	std::pair<int, std::wstring>(CSIDL_INTERNET_CACHE, L""),
	std::pair<int, std::wstring>(CSIDL_APPDATA, L""),
	std::pair<int, std::wstring>(CSIDL_COOKIES, L""),
	std::pair<int, std::wstring>(CSIDL_LOCAL_APPDATA, L""),
	std::pair<int, std::wstring>(CSIDL_SYSTEM, L""),
							    };

nextlabs::recursion_control hpe_hook_control;

/*********************************************
function name: GetIgnoreFolderList
feature: This function was used to get the 
folders which were ignored.
**********************************************/
bool GetIgnoreFolderList(std::map<std::wstring, int>& mapIgnoreFolders)
{
	static bool bDone = false;
	static std::map<std::wstring, int> mapFolders;
	if(!bDone)
	{
		wchar_t* pBuf = new wchar_t[MAX_PATH * 2];
		if(pBuf)
		{	
			for(int i = 0; i < _countof(g_szIgnoredFolders); i++)
			{
				memset(pBuf, 0, sizeof(wchar_t) * (MAX_PATH * 2));
				if(SHGetSpecialFolderPath(NULL, pBuf, g_szIgnoredFolders[i].first, NULL))
				{
					wcsncat_s(pBuf, MAX_PATH * 2, g_szIgnoredFolders[i].second.c_str(), _TRUNCATE);
					mapFolders[pBuf] = g_szIgnoredFolders[i].first;
				}
			}
			delete [] pBuf;
			pBuf = NULL;
		}

		bDone = true;
	}
	if(mapFolders.size() > 0)
	{
		mapIgnoreFolders = mapFolders;
		return true;
	}
	return false;
}

/*******************************************
Translate Path,
For example, try to attach a file in gmail from 
\\hz-ts02\upload\foo.doc
It's possible to get a path in "CreatFile" like:
\\?\UNC\hz-ts02\kzhou\upload\foo.doc
so, we need to translate this path to 
\\hz-ts02\upload\foo.doc
*******************************************/
static void TranslatePath(std::string& strPath)
{
	if(strPath.length() > 4 && strPath.find("\\\\?\\") != std::string::npos)
	{
		strPath = strPath.substr(4, strPath.length() - 4);
	}

	//For IE and some other applications, the UNC path will be "UNC\hz-ts02\upload\kzhou\...", so we need to translate it to "\\hz-ts02\upload\kzhou\.."
	std::string strTempPath = strPath;
	std::transform(strTempPath.begin(), strTempPath.end(), strTempPath.begin(), tolower);
	if(strTempPath.find("unc\\") == 0)//fix bug9952
	{
		strPath.replace(0, 4, "\\\\");
	}

	/*********************************************************
	convert "\filename" to "driver\filename"
	if user uses a path like this \a.txt,
	it will try to access the "a.txt" under current working
	driver, like d:\a.txt
	**********************************************************/
	if( strPath.length() >= 2 && (strPath[0] == '/' || strPath[0] == '\\') && (strPath[1] != '/' && strPath[1] != '\\'))
	{
		char szPath[MAX_PATH + 1] = {0};
		if(GetFullPathNameA(strPath.c_str(), MAX_PATH, szPath, NULL) > 0)
		{
			strPath = string(szPath);
		}
	}

	//it means this is a file name (since it doesn't has "/" and "\"), we need to get the current working directory and combine them to get a full path
	if(strPath.find("\\") == string::npos && strPath.find("/") == string::npos)
	{
		char szDir[MAX_PATH + 1] = {0};
		if( GetCurrentDirectoryA(MAX_PATH, szDir) > 0 )
		{
			if(szDir[strlen(szDir) - 1] != '\\')
			{
				strncat_s(szDir, MAX_PATH, "\\", _TRUNCATE);
			}
			strPath = string(szDir) + strPath;
		}
	}
}

static bool IsSupportFileType(LPCWSTR lpFilePath)
{
	if(!lpFilePath)
		return false;

	std::wstring strFilePath(lpFilePath);
	std::transform(strFilePath.begin(), strFilePath.end(), strFilePath.begin(), towupper);

	int i = 0;

	std::map<std::wstring, int> mapIgnored;
	if( GetIgnoreFolderList(mapIgnored) )//Try to get the list of folders which were ignored
	{	
		std::map<std::wstring, int>::iterator itr;
		
		for( itr = mapIgnored.begin(); itr != mapIgnored.end(); itr++)
		{
			std::pair<std::wstring, int> pairPath = (std::pair<std::wstring, int>)*itr;

			if(strFilePath.length() >= pairPath.first.length())
			{
				if(_memicmp(lpFilePath, pairPath.first.c_str(), pairPath.first.length() * sizeof(wchar_t)) == 0)
				{
					if(pairPath.second == CSIDL_INTERNET_CACHE)
					{
						std::wstring::size_type nIndex = strFilePath.rfind(L'\\');
						if(  nIndex == string::npos)
						{
							//	it is an unexpected case, ignore it
							
							return false;
						}
						std::wstring strFileName;
						strFileName = strFilePath.substr(nIndex + 1, strFilePath.length() - nIndex -1);

						nIndex = strFileName.rfind(L'.');
						if(  nIndex == string::npos)
						{
							//	this file do not have a postfix, ignore it
							
							return false;
						}
						std::wstring strPostfix;
						strPostfix = strFileName.substr(nIndex, strFileName.length() - nIndex);

						int j = 0;
						for ( ; j < _countof(g_szFilterFileTypes); j++)
						{
							if (!_wcsicmp(strPostfix.c_str(), g_szFilterFileTypes[j]))
							{
								//	it is not a ignored file

								break;
							}
						}
						if (j < _countof(g_szFilterFileTypes))
						{
							//	it is not a ignored file
							break;
						}
					}

					return false;
					
			}
		}
		}
				}

	for(i = 0; i < _countof(g_szSpecailPath); i++)
	{
		if(strFilePath.find(g_szSpecailPath[i]) == 0 )
		{
			return false;
		}
	}

	for(i = 0; i < _countof(g_szSpecialPath_End); i++)//fix bug9723 filter the PIPE in sharepoint. like PIPE\WKSSVC... kevin zhou 2009-8-14
	{
		std::wstring::size_type nIndex = strFilePath.find(g_szSpecialPath_End[i]);
		if( nIndex != std::string::npos)
		{
			if(nIndex == strFilePath.length() - wcslen(g_szSpecialPath_End[i]))
			{
				return false;
			}
		}
	}

	return true;
}

APIRES::HOOKPROCINFO myProcInfo[] =
{
	{"Kernel32.DLL", "CreateFileW", NULL, (PVOID *)&CAPIHook::real_CreateFileW, CAPIHook::try_MyCreateFileW, NULL },
	{"Kernel32.DLL", "CreateFileA", NULL, (PVOID *)&CAPIHook::real_CreateFileA, CAPIHook::try_MyCreateFileA, NULL },
	{"Kernel32.DLL", "CloseHandle", NULL, (PVOID *)&CAPIHook::real_CloseHandle, CAPIHook::try_MyCloseHandle, NULL },
	{"ole32.DLL", "CoCreateInstance",   NULL, (PVOID *)&CAPIHook::real_CoCreateInstance,   CAPIHook::try_CoCreateInstance,   NULL },
	{"MSWSOCK.DLL",   "TransmitFile",   NULL, (PVOID *)&CAPIHook::real_TransmitFile,   CAPIHook::try_TransmitFile,   NULL },
	{NULL,NULL,NULL,NULL,NULL,NULL}
};

BOOL CAPIHook::StartHook()
{
	InitializeMadCHook();
	
	std::map<std::wstring, int> mapIgnored;
	GetIgnoreFolderList(mapIgnored);

	m_pEvaluaImp = CPolicy::CreateInstance() ;
	return HookAPIByInfoList() ;
}

VOID CAPIHook::EndHook()
{
	m_pEvaluaImp->Release() ;

	g_realCreateFileW = NULL;
	g_realCloseHandle = NULL;

	if( !IsProcess(L"firefox.exe") && !IsProcess(L"SmartFTP.exe")  )
	{
		FinalizeMadCHook();
	}

	return;//ignore to call "unhookAPI" kevin 2009-9-8
}

BOOL CAPIHook::HookAPIByInfoList() 
{
	int iCount = 0;
	for( ; iCount < _countof(myProcInfo); iCount++)
	{
		if(myProcInfo[iCount].pNewProc != NULL)
		{
			if( _strnicmp( myProcInfo[iCount].pszOrigName, "CoCreateInstance",MAX_PATH ) ==0 )
			{
				if( IsProcess( L"SmartFTP.exe" ) )
				{
					if(!HookAPI(myProcInfo[iCount].pszModule, myProcInfo[iCount].pszOrigName, myProcInfo[iCount].pNewProc, myProcInfo[iCount].pNextProc))
						return FALSE;
				}
			}
			else
			{
				if(!HookAPI(myProcInfo[iCount].pszModule, myProcInfo[iCount].pszOrigName, myProcInfo[iCount].pNewProc, myProcInfo[iCount].pNextProc))
					return FALSE;
			}
		}
	}

	g_realCreateFileW = CAPIHook::real_CreateFileW;
	g_realCloseHandle = CAPIHook::real_CloseHandle;
	return TRUE;
}

HANDLE WINAPI CAPIHook::try_MyCreateFileW(LPCWSTR lpFileName,
								      DWORD dwDesiredAccess,
								      DWORD dwShareMode,
								      LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								      DWORD dwCreationDisposition,
								      DWORD dwFlagsAndAttributes,
								      HANDLE hTemplateFile)
{
	if(GetDetachFlag() && real_CreateFileW)
	{
		return real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	

	if(hpe_hook_control.is_disabled())
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
	__except( NLEXCEPT_FILTER_EX2(NULL,exception_cb) )
	{		;
	}
	return INVALID_HANDLE_VALUE;
}

HANDLE WINAPI CAPIHook::MyCreateFileW(LPCWSTR lpFileName,
								      DWORD dwDesiredAccess,
								      DWORD dwShareMode,
								      LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								      DWORD dwCreationDisposition,
								      DWORD dwFlagsAndAttributes,
								      HANDLE hTemplateFile)
{

        nextlabs::recursion_control_auto auto_disable(hpe_hook_control);

	if(!real_CreateFileW)
		return INVALID_HANDLE_VALUE;

	HANDLE hFile = real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
									dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	if(hFile != INVALID_HANDLE_VALUE)
	{
		
		if((dwDesiredAccess & GENERIC_READ) && (dwCreationDisposition & OPEN_EXISTING) && IsSupportFileType(lpFileName))
		{
			CMapperMgr& ins = CMapperMgr::Instance();
			string sPath;
			/*
			modified by kevin 2009-7-20
			*/
			//-----------------------------------------
			if(lpFileName)
			{
				wstring strFilePath(lpFileName);

				sPath = MyWideCharToMultipleByte(strFilePath);
			//-----------------------------------------
			TranslatePath(sPath);
			ins.AddHandleName(hFile, sPath);
		}
			
		}
	}

	return hFile;
}

//	2011-4-2, this is for bug 13980, coreftp opens file using CreateFileA and no CreateFileW is called.
//	here, we hook CreateFileA, then call MyCreateFileW instead.
//	although this change is for coreftp's bug, but I suppose this solution can be used for all other processes.
//	end of 2011-4-2
HANDLE WINAPI CAPIHook::try_MyCreateFileA(LPCSTR lpFileName,
								      DWORD dwDesiredAccess,
									  DWORD dwShareMode,
								      LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								      DWORD dwCreationDisposition,
								      DWORD dwFlagsAndAttributes,
								      HANDLE hTemplateFile)
{
	//	translate lpFileName to local string value first, don't use it directly to secure lpFileName is not changed for unknown reason .
	string strFileName(lpFileName);
	wstring wstrFileName = MyMultipleByteToWideChar(strFileName);

	return CreateFileW(wstrFileName.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HANDLE WINAPI CAPIHook::MyCreateFileA(LPCSTR lpFileName,
								      DWORD dwDesiredAccess,
									  DWORD dwShareMode,
								      LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								      DWORD dwCreationDisposition,
								      DWORD dwFlagsAndAttributes,
								      HANDLE hTemplateFile)
{
	//CAutoDisable autoDisable;
	if (real_CreateFileA)
	{
		return real_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, 
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	return INVALID_HANDLE_VALUE;
}

BOOL WINAPI CAPIHook::try_MyCloseHandle(HANDLE hObject)
{
	if(GetDetachFlag() && real_CloseHandle)
	{
		return real_CloseHandle(hObject);
	}
	

	if(hpe_hook_control.is_disabled())
	{
		if(real_CloseHandle == FALSE)
			return FALSE;
		return real_CloseHandle(hObject);
	}
	__try
	{
		return MyCloseHandle(hObject);
	}
	__except( NLEXCEPT_FILTER_EX2(NULL,exception_cb) )
	{	;
	}
	return FALSE;
}

BOOL WINAPI CAPIHook::MyCloseHandle(HANDLE hObject)
{
        nextlabs::recursion_control_auto auto_disable(hpe_hook_control);
	/*
	*	modified by ben, on 20-08-2009, delay remove from closefile to here for coreftp, #9772
	*	modified by ben, on 2011/Mar/18, don't remove item in closehandle, for bug 13817, don't remove until time expire.
	*/
	if(hObject != INVALID_HANDLE_VALUE && !IsProcess(L"iexplore.exe") && !IsProcess(L"Opera.exe") && !IsProcess(L"Explorer.exe") && \
		!IsProcess(L"chrome.exe") && !IsProcess(L"coreftp.exe") && !IsProcess(L"firefox.exe") )
	{
		CMapperMgr& ins = CMapperMgr::Instance();
		ins.RemoveHandleName(hObject);
	}
	return real_CloseHandle(hObject);
}
/*
SmartFtp: for the SFTP.
*/
struct __declspec(uuid("aa8fbbfd-22df-41ef-83d0-5ecdb6bd47e6"))
	ISFTPConnection;

HRESULT WINAPI  CAPIHook::try_CoCreateInstance(REFCLSID rclsid, void *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
	if(GetDetachFlag() && real_CoCreateInstance)
	{
		return  ((CoCreateInstanceType)real_CoCreateInstance)(rclsid,pUnkOuter,dwClsContext,riid,ppv); 
	}
	

	if(hpe_hook_control.is_disabled())
	{
		if( real_CoCreateInstance == NULL )
		{
			return S_FALSE ;
		}
		return  ((CoCreateInstanceType)real_CoCreateInstance)(rclsid,pUnkOuter,dwClsContext,riid,ppv); 
	}
	__try
	{
		return MyCoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv); 
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return S_FALSE ;
}
HRESULT WINAPI  CAPIHook::MyCoCreateInstance(REFCLSID rclsid, void *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
	HRESULT hr = S_FALSE ;
	if( real_CoCreateInstance )
	{
		LPOLESTR str = SysAllocString(L"") ;
		LPOLESTR strRiid = SysAllocString(L"") ;
		StringFromCLSID( rclsid,  &str ) ;
		StringFromCLSID( riid,  &strRiid ) ;
		hr = ((CoCreateInstanceType)real_CoCreateInstance)(rclsid,pUnkOuter,dwClsContext,riid,ppv);
		if( IsEqualGUID( riid, __uuidof( ISFTPConnection ) ) == TRUE )
		{
			DP((L"It is the Smart FTP SFTP object")) ;
			ImpSmartFtpSFTP( pUnkOuter, ppv ) ; 
		}
	}
	return hr ;
}
/*
for version 1.5.16 of sftplib.dll
Offset of IISFTPConnection:
93+7:	HRESULT DownloadFile(BSTR bstrRemoteFile,VARIANT varLocalFile,long nRemoteStartPosLo, long nRemoteStartPosHi,long nLocalStartPosLo,long nLocalStartPosHi,enum enumError * retval)
94+7:	HRESULT DownloadFileEx(BSTR bstrRemoteFile,VARIANT varLocalFile,unsigned __int64 nRemoteStartPos,,unsigned __int64 nLocalStartPos,long dwCreateDeposition,enum enumError * retval )
95+7:	HRESULT UploadFile(VARIANT varLocalFile,BSTR bstrRemoteFile,long nLocalStartPosLo,long nLocalStartPosHi,long nRemoteStartPosLo,long nRemoteStartPosHi,enum enumError * retval )
96+7:	HRESULT Rename(BSTR bstrOld,BSTR bstrNew,long nFlags,enum enumError * retval )
97+7:	HRESULT RemoveFile( BSTR bstrFile,enum enumError * retval ) 

for version 1.5.17.32 of sftplib.dll
97+7:	HRESULT DownloadFile(BSTR bstrRemoteFile,VARIANT varLocalFile,long nRemoteStartPosLo, long nRemoteStartPosHi,long nLocalStartPosLo,long nLocalStartPosHi,enum enumError * retval)
98+7:	HRESULT DownloadFileEx(BSTR bstrRemoteFile,VARIANT varLocalFile,unsigned __int64 nRemoteStartPos,,unsigned __int64 nLocalStartPos,long dwCreateDeposition,enum enumError * retval )
99+7:	HRESULT UploadFile(VARIANT varLocalFile,BSTR bstrRemoteFile,long nLocalStartPosLo,long nLocalStartPosHi,long nRemoteStartPosLo,long nRemoteStartPosHi,enum enumError * retval )
100+7:	HRESULT Rename(BSTR bstrOld,BSTR bstrNew,long nFlags,enum enumError * retval )
101+7:	HRESULT RemoveFile( BSTR bstrFile,enum enumError * retval ) 
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

			bRet = HookCode( (PVOID)pOriginFunc,(PVOID)pmyFunc ,(PVOID*)&real_APIAddress ) ;
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
 static DWORD GetVersionNumber( std::wstring szModuleName, std::wstring szKeyName )
{
	DWORD dNumb = 0 ;
	if( ! szModuleName.empty() ) 
	{
		ModuleVer::DllVersion::CModuleVersion ver ;
		if(ver.GetFileVersionInfo(szModuleName.c_str()) == FALSE )
		{
			return dNumb ;
		}
		std::wstring strRet  = ver.GetValue(szKeyName.c_str() ) ;
		if( strRet.length() >0 )
		{
			std::wstring::iterator itor = strRet.begin() ;
			for( itor ;itor!= strRet.end() ; itor ++ )
			{
				wchar_t temp = (*itor) ;
				if( temp == '.' )
				{
					strRet.erase( itor ) ;
					if( strRet.find( L".") )
					{
						itor = strRet.begin() ;
					}
				}
			}
			dNumb = static_cast<DWORD>(_wtof((wchar_t*) strRet.c_str() ) ) ;
		}
	}
	return dNumb ;
}
BOOL CAPIHook::ImpSmartFtpSFTP( LPVOID pUnkOuter, LPVOID* ppv   )
{
	pUnkOuter;
	BOOL bRet = FALSE ;
	if( ppv != NULL )
	{
		LONG_PTR** proc = (LONG_PTR**)(*ppv);
		DWORD tid = ::GetCurrentThreadId();
		DWORD dVerNumb = GetVersionNumber( L"sfFTPLib.dll", L"ProductVersion" ) ;
		if( dVerNumb >= 151732 )
		{
			bRet = HookComInterface(tid,m_listSmartFtp_UpFile, proc[0][105],  (LONG_PTR)try_UploadFile, *ppv ) ;
		}
		else
		{
			bRet = HookComInterface(tid,m_listSmartFtp_UpFile, proc[0][101],  (LONG_PTR)try_UploadFile, *ppv ) ;
		}
	}
	return bRet ;
}
HRESULT WINAPI CAPIHook::retReal_UploadFile(
		PVOID pthis,
		VARIANT varLocalFile, 
		BSTR bstrRemoteFile, 
		long nLocalStartPosLo, 
		long nLocalStartPosHi, 
		long nRemoteStartPosLo, 
		long nRemoteStartPosHi, 
		PVOID retval ) 
	 {
		 	LONG_PTR orig_DropProc = NULL;
		std::list<APIRES::HOOKAPIINFO>::iterator iter = m_listSmartFtp_UpFile.begin() ;
		for (  ; iter != m_listSmartFtp_UpFile.end() ; iter++ )
		{
			if( pthis == (*iter).pObjectAddress )
			{
				orig_DropProc = (LONG_PTR)(*iter).pRealAPIAddress;
				break ;
			}
		}
		if( orig_DropProc )
		{
			return  ((UploadFileType)orig_DropProc)(	pthis, varLocalFile,	bstrRemoteFile,nLocalStartPosLo, nLocalStartPosHi, nRemoteStartPosLo,nRemoteStartPosHi, retval ) ;
		}
		return S_FALSE ;
	 }

HRESULT WINAPI CAPIHook::try_UploadFile( PVOID pthis,
										VARIANT varLocalFile, 
										BSTR bstrRemoteFile, 
										long nLocalStartPosLo, 
										long nLocalStartPosHi, 
										long nRemoteStartPosLo, 
										long nRemoteStartPosHi, 
										PVOID retval ) 
{
	if(GetDetachFlag())
	{
		return retReal_UploadFile(	pthis, varLocalFile,	bstrRemoteFile,nLocalStartPosLo, nLocalStartPosHi, nRemoteStartPosLo,nRemoteStartPosHi, retval ) ;
	}

		if(hpe_hook_control.is_disabled())
	{
		   return retReal_UploadFile(	pthis, varLocalFile,	bstrRemoteFile,nLocalStartPosLo, nLocalStartPosHi, nRemoteStartPosLo,nRemoteStartPosHi, retval ) ;
	}
	__try
	{
		return myUploadFile(	pthis, varLocalFile,	bstrRemoteFile,nLocalStartPosLo, nLocalStartPosHi, nRemoteStartPosLo,nRemoteStartPosHi, retval ) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		 ;
	}
	return S_FALSE ;

}

HRESULT WINAPI CAPIHook::myUploadFile( PVOID pthis,
									  VARIANT varLocalFile, 
									  BSTR bstrRemoteFile, 
									  long nLocalStartPosLo, 
									  long nLocalStartPosHi, 
									  long nRemoteStartPosLo, 
									  long nRemoteStartPosHi, 
									  PVOID retval ) 
{
	  nextlabs::recursion_control_auto auto_disable(hpe_hook_control);
	HRESULT hr = S_OK ;
	LONG_PTR orig_DropProc = NULL;
	std::list<APIRES::HOOKAPIINFO>::iterator iter = m_listSmartFtp_UpFile.begin() ;
	for (  ; iter != m_listSmartFtp_UpFile.end() ; iter++ )
	{
		if( pthis == (*iter).pObjectAddress )
		{
			orig_DropProc = (LONG_PTR)(*iter).pRealAPIAddress;
			
			break ;
		}
	}
	if( orig_DropProc )
	{
		std::wstring  strRemoteFile = bstrRemoteFile ;
		if( varLocalFile.vt == VT_BSTR )
		{
		}
		CSftplibImpl sftplibImpl ;
		std::wstring strHost ;
		LONG lPort ;
		std::wstring strUserName ;
		std::wstring strLastPath ;

		sftplibImpl.GetHost( (IDispatch *)pthis,strHost ) ;
		sftplibImpl.GetPort( (IDispatch *)pthis,lPort ) ;
		
		
		/*
		Smart FTP upload
		*/
		FTP_EVAL_INFO evalInfo ;
		evalInfo.pszServerIP = strHost;
		wchar_t szPort[11] = {0};
		_itow_s(lPort, szPort, 10, 10);
		evalInfo.pszServerPort = szPort ;
		evalInfo.pszFTPUserName = strUserName ;
		strRemoteFile = L"server://" +strHost +L":" + szPort;
		evalInfo.pszDestFileName = strRemoteFile ;
		evalInfo.pszSrcFileName = varLocalFile.bstrVal ;
		CPolicy *pPolicy = CPolicy::CreateInstance() ;
		CEEnforcement_t enforcement ;
		memset(&enforcement, 0, sizeof(CEEnforcement_t));
		pPolicy->QuerySingleFilePolicy( CPolicy::m_networkAccess, evalInfo , enforcement ) ;

		CEResponse_t response = enforcement.result;
		cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

		pPolicy->Release() ;
		if( response ==   CEAllow )
		{
			//----------------------------------------------------------------
			std::list<APIRES::HOOKAPIINFO>::reverse_iterator	riter = m_listSmartFtp_UpFile.rbegin() ;
			orig_DropProc =	 (LONG_PTR)(*riter).pRealAPIAddress ;
			//----------------------------------------------------------------
			hr = ((UploadFileType)orig_DropProc)(	pthis, varLocalFile,	bstrRemoteFile,nLocalStartPosLo, nLocalStartPosHi, nRemoteStartPosLo,nRemoteStartPosHi, retval ) ;
		}
	}
	return hr ;
}

/********************************************************
fix bug11218
Windows has an embeded FTP program,
c:\windows\system32\ftp.exe
in this case, ftp.exe won't call WSPSend to send the data.
it will call transmitfile to send the file data to FTP 
Server.
So, we need to hook this API.
**********************************************************/
BOOL CAPIHook::my_TransmitFile( SOCKET hSocket, HANDLE hFile, DWORD nNumberOfBytesToWrite, DWORD nNumberOfBytesPerSend, LPOVERLAPPED lpOverlapped, LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers, DWORD dwFlags )
{
	nextlabs::recursion_control_auto auto_disable(hpe_hook_control);

	struct sockaddr  name ;
	INT iLen = sizeof(sockaddr)  ;
	if(getpeername( hSocket, &name, &iLen )  == 0 )
	{
		string sPeerIP = AddressToString(&name, sizeof(sockaddr), false);
		std::wstring strDestIP = StringT1toT2<char, wchar_t>(sPeerIP);
		if(IsIgnoredIP(strDestIP.c_str()))//added by kevin 2009-7-1
		{
			return real_TransmitFile(hSocket, hFile, nNumberOfBytesToWrite, nNumberOfBytesPerSend, lpOverlapped, lpTransmitBuffers, dwFlags);
		}

		struct sockaddr_in* addr_v4 = (sockaddr_in*)&name;
		int nRemotePort = ntohs(addr_v4->sin_port);
		wchar_t wszPortBuf[20] = {0};
		_snwprintf_s(wszPortBuf, 20, _TRUNCATE, L"%d", nRemotePort);

		if( CheckNetworkAccess(hSocket,strDestIP.c_str(),wszPortBuf) ==	 FALSE )
		{
			WSASetLastError(WSAESHUTDOWN);
			return FALSE;
		}

	}

	return real_TransmitFile(hSocket, hFile, nNumberOfBytesToWrite, nNumberOfBytesPerSend, lpOverlapped, lpTransmitBuffers, dwFlags);
}

BOOL CAPIHook::try_TransmitFile( SOCKET hSocket, HANDLE hFile, DWORD nNumberOfBytesToWrite, DWORD nNumberOfBytesPerSend, LPOVERLAPPED lpOverlapped, LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers, DWORD dwFlags )
{
	if(GetDetachFlag() && real_TransmitFile)
	{
		return real_TransmitFile(hSocket, hFile, nNumberOfBytesToWrite, nNumberOfBytesPerSend, lpOverlapped, lpTransmitBuffers, dwFlags);
	}


	if( hpe_hook_control.is_disabled() )
	{
		if (!real_TransmitFile)
			return FALSE;
		return real_TransmitFile(hSocket, hFile, nNumberOfBytesToWrite, nNumberOfBytesPerSend, lpOverlapped, lpTransmitBuffers, dwFlags);
	}

	__try
	{
		return my_TransmitFile(hSocket, hFile, nNumberOfBytesToWrite, nNumberOfBytesPerSend, lpOverlapped, lpTransmitBuffers, dwFlags);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return FALSE;
}
