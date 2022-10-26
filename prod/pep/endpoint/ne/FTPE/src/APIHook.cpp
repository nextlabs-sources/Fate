#include <cassert>
#include "stdafx.h"
#include "APIHook.h"

#include "stdlib.h"
#include "WTypes.h"
#include "ImageHlp.h"

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
#include <atlcomcli.h>
using namespace std;
#include "MapperMgr.h"
#include "Utilities.h"
#include "sftplibImpl.h"

#include "FtpSocket.h"
#include "ParserResult.h"
#include "FtpDataConn.h"
#include "FtpCtrlConn.h"
#include "FtpConnMgr.h"
#include "eframework/auto_disable/auto_disable.hpp"
#include "timeout_list.hpp"


#define TEMP_DWNLD_FILE_TIMEOUT_VALUE (60 * 60 * 1000)
CTimeoutList g_Temp_DownloadURIList(TEMP_DWNLD_FILE_TIMEOUT_VALUE);
CreateFileWType2 g_realCreateFileW = NULL;
CloseHandleType2 g_realCloseHandle = NULL;
CAPIHook::SHFileOperationWType CAPIHook::real_SHFileOperationW = NULL;
extern nextlabs::cesdk_loader cesdkLoader;

CTimeoutList g_listForCut;
CTimeoutList g_listDeniedPath(30 * 60 * 1000);

wstring CombineEvalInfo( const FTP_EVAL_INFO evalInfo )
{
	wstring strRet ;
	strRet.append( evalInfo.pszDestFileName ) ;
	strRet.append(L";") ;

	strRet.append(evalInfo.pszFTPUserName) ;
	strRet.append(L";") ;
	strRet.append(evalInfo.pszServerIP);
	strRet.append(L";") ;

	strRet.append(evalInfo.pszServerPort) ;
	strRet.append(L";") ;
	strRet.append(evalInfo.pszSrcFileName) ;
	strRet.append(L";" ) ;
	wchar_t szType[10] = {0} ;
	_snwprintf_s( szType, 10, _TRUNCATE,L"%i",evalInfo.iProtocolType ) ;
	strRet.append(szType) ; 
	strRet.append(L";") ; 
	DPW((L"Combined String:%s",strRet.c_str())) ;
	return strRet ;
}
BOOL ParseEvalInfoString( const wstring combinedString,FTP_EVAL_INFO& evalInfo  )
{
	if( combinedString.empty() == FALSE)
	{
		wstring::size_type index = 0 ;
		wstring::size_type Startindex = 0 ;

		index = combinedString.find( L";",Startindex) ;
		if( index != wstring::npos )
		{
			evalInfo.pszDestFileName = combinedString.substr(Startindex,index-Startindex) ;
			DPW((L"pszDestFileName String:%s",evalInfo.pszFTPUserName.c_str())) ;
			Startindex = index+1 ;
		}
		index = combinedString.find( L";",Startindex) ;
		if( index != wstring::npos )
		{
			evalInfo.pszFTPUserName = combinedString.substr(Startindex,index-Startindex) ;
			DPW((L"pszFTPUserName String:%s",evalInfo.pszFTPUserName.c_str())) ;
			Startindex = index+1 ;
		}
		index = combinedString.find( L";",Startindex) ;
		if( index != wstring::npos )
		{
			evalInfo.pszServerIP = combinedString.substr(Startindex,index-Startindex) ;
			DPW((L"pszServerIP String:%s",evalInfo.pszServerIP.c_str())) ;
			Startindex = index+1 ;
		}
		index = combinedString.find( L";",Startindex) ;
		if( index != wstring::npos )
		{
			evalInfo.pszServerPort = combinedString.substr(Startindex,index-Startindex) ;
			DPW((L"pszServerPort String:%s",evalInfo.pszServerPort.c_str())) ;
			Startindex = index+1 ;
		}
		wstring type ;
		index = combinedString.find( L";",Startindex) ;
		if( index != wstring::npos )
		{
			evalInfo.pszSrcFileName = combinedString.substr(Startindex,index-Startindex) ;
				DPW((L"pszSrcFileName type:%s",evalInfo.pszSrcFileName.c_str())) ;
			Startindex = index+1 ;
		}
		index = combinedString.find( L";",Startindex) ;
		if( index != wstring::npos )
		{
			type = combinedString.substr(Startindex,index-Startindex) ;
				DPW((L" type:%s",type.c_str())) ;
			Startindex = index+1 ;
		}
		if( type.empty() == FALSE)
		{
			evalInfo.iProtocolType = ::_wtoi( type.c_str() ) ;
				DPW((L"iProtocolType type:%i",evalInfo.iProtocolType)) ;
		}
	}
	return TRUE ;
}

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
		DPW((L"_memicmp(strSrc.c_str(), strTemp.c_str(), strTemp.length():[%s]",strSrc.c_str())) ;
		if(g_Temp_DownloadURIList.FindItem(strSrc.c_str(), strRemoteURL))//Try to find the URL 
		{
			DPW((L"g_Temp_DownloadURIList")) ;
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
	DPW((L"Handle IE Copy:[Temp Path %s]\r\n [Source File Path%s]",buffer,strSrc.c_str())) ;

	wstring strRemoteURL;
	if(GetURLWithTempPath(strSrc,buffer, strRemoteURL))
	{
		//	add by Ben, Nov, 30th, 2009
		//	reason: the IE will retry my_CopyFileExW if download file from mail.cn.nextlabs.com

		FTP_EVAL_INFO evalInfo;
		ParseEvalInfoString( strRemoteURL, evalInfo) ;
		evalInfo.pszDestFileName =strDest ; 
		CPolicy *pPolicy = CPolicy::CreateInstance() ;
		CEEnforcement_t enforcement ;
		memset(&enforcement, 0, sizeof(CEEnforcement_t));

		FTPE_STATUS status = FTPE_SUCCESS ;
		if( evalInfo.iProtocolType == FPT_REGULAR )
		{
			status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftp, evalInfo , enforcement ) ;
		}else if(evalInfo.iProtocolType == FTP_FTPS_IMPLICIT )
		{
			status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftps, evalInfo , enforcement ) ;
		}
		else
		{
			status = FTPE_ERROR ;
			DPW((L"evaluate ftpe get unexpected protocol type %d", evalInfo.iProtocolType));
		}

		CEResponse_t response = enforcement.result;
		cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
		pPolicy->Release() ;
		if( response !=   CEAllow )
		{

			return FALSE;//denied
		}
		else
		{
			return TRUE ;
		}

	}

	return TRUE;
}
//	add a struct type
typedef struct
{
	HANDLE hDeniedFile;
	/************************************************************************/
	/* sDeniedFileName and dwTime are add for #10028, Sept/17/2009
	*	sDeniedFileName is denied file path name
	*	dwTime is set to trigger time out check for removing
	*/
	/************************************************************************/
	std::string sDeniedFileName;	
	DWORD dwTime;	
}DeniedWriteFileHandle;

/************************************************************************/
/* DENY_WRITEFILE_INVALID_TICK and DENY_WRITEFILE_TIMEOUT_VALUE are add for #10028, Sept/17/2009
*/
/************************************************************************/
#define DENY_WRITEFILE_INVALID_TICK	0
#define DENY_WRITEFILE_TIMEOUT_VALUE 1000

//	add a list type
typedef list<DeniedWriteFileHandle> DENIED_WRITEFILE_LIST;

//	add a static list variable
static DENIED_WRITEFILE_LIST listDeniedWriteFile;

/*	return false, not a previous denied file
*	return true, is a previous denied file
*/
static BOOL IsDeniedWritefile(HANDLE hFile, std::string sFile, DENIED_WRITEFILE_LIST& list);
static void PushDeniedWritefile(HANDLE hFile, std::string sFile, DENIED_WRITEFILE_LIST& list);
static void RemoveDeniedWriteFile(HANDLE hFile, DENIED_WRITEFILE_LIST& list);

CAPIHook::CopyFileExWType CAPIHook::real_CopyFileExW = NULL;
CAPIHook::MoveFileWType CAPIHook::real_MoveFileW = NULL;
CAPIHook::MoveFileExWType CAPIHook::real_MoveFileExW = NULL;
CAPIHook::MoveFileExAType CAPIHook::real_MoveFileExA = NULL;

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
	if(strTempPath.find("unc\\") == 0)
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

static BOOL IsDeniedWritefile(HANDLE hFile, std::string sFile, DENIED_WRITEFILE_LIST& list)
{
	::EnterCriticalSection(&CcriticalMngr::s_csDenyWriteFileHandle);
	
	if (list.empty())
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csDenyWriteFileHandle);
		return FALSE;
	}
	
	DENIED_WRITEFILE_LIST::iterator it = list.begin();

	for (; it != list.end(); )
	{
		if ( ( (*it).dwTime != DENY_WRITEFILE_INVALID_TICK ) && ( (*it).dwTime + DENY_WRITEFILE_TIMEOUT_VALUE < GetTickCount() ) )
		{
			//	timeout for cache, remove it
			DPA(("remove previous write file %d, %s\n", (*it).hDeniedFile, (*it).sDeniedFileName.c_str()));
			it = list.erase(it);

			continue;
		}

		if ((*it).hDeniedFile == hFile || sFile == (*it).sDeniedFileName)
		{
			DPA(("deny by previous write file\nprevious: [%s]\nnow: [%s]\n, previous: %d now: %d\n", \
				(*it).sDeniedFileName.c_str(), sFile.c_str(), (*it).hDeniedFile, hFile));
			::LeaveCriticalSection(&CcriticalMngr::s_csDenyWriteFileHandle);
			return TRUE;
		}

		it++;
	}

	::LeaveCriticalSection(&CcriticalMngr::s_csDenyWriteFileHandle);
	return FALSE;
}

static void PushDeniedWritefile(HANDLE hFile, std::string sFile, DENIED_WRITEFILE_LIST& list)
{
	::EnterCriticalSection(&CcriticalMngr::s_csDenyWriteFileHandle);

	DeniedWriteFileHandle DeniedFile;
	DeniedFile.hDeniedFile = hFile;
	DeniedFile.sDeniedFileName = sFile;
	DeniedFile.dwTime = DENY_WRITEFILE_INVALID_TICK;

	DPA(("push denied write file handle %d, name %s\n", hFile, sFile.c_str()));
	list.push_back(DeniedFile);

	::LeaveCriticalSection(&CcriticalMngr::s_csDenyWriteFileHandle);
}

static void RemoveDeniedWriteFile(HANDLE hFile, DENIED_WRITEFILE_LIST& list)
{
	if(!hFile)
	{
		return;
	}

	::EnterCriticalSection(&CcriticalMngr::s_csDenyWriteFileHandle);

	if (list.empty())
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csDenyWriteFileHandle);
		return;
	}

	DENIED_WRITEFILE_LIST::iterator it = list.begin();
	for (; it != list.end(); it++)
	{
		if (hFile == (*it).hDeniedFile && DENY_WRITEFILE_INVALID_TICK == (*it).dwTime)
		{
			DPA(("set timeout for previous write file %d, %s\n", hFile, (*it).sDeniedFileName.c_str()));
			(*it).dwTime = GetTickCount();
			::LeaveCriticalSection(&CcriticalMngr::s_csDenyWriteFileHandle);
			return;
		}
	}

	::LeaveCriticalSection(&CcriticalMngr::s_csDenyWriteFileHandle);
}

const wchar_t* g_szSpecailPath[] = {//Ignore these pathes in CreateFile, Added by Kevin 2009-7-9
	L"\\\\.\\PIPE",
	L"\\\\.\\WMIDATADEVICE",
	L"\\\\?\\ROOT#SYSTEM#0000#",
	L"ROOT#SYSTEM#0000#",
	L"\\\\.\\PHYSICALDRIVE",
	L"CONIN$",
	L"CONOUT$",
	L"\\\\.\\"
};

const wchar_t* g_szFiltedPath[] = {
	L"\\Device\\LanmanRedirector\\"
};

const std::pair<int, std::wstring> g_szIgnoredFolders[] =	
{
	std::pair<int, std::wstring>(CSIDL_APPDATA,  L"\\Microsoft"),
	std::pair<int, std::wstring>(CSIDL_LOCAL_APPDATA, L"\\Microsoft"),
};

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

nextlabs::recursion_control hook_control;

CAPIHook::CreateFileWType    CAPIHook::real_CreateFileW    = NULL;
CAPIHook::CreateFileAType    CAPIHook::real_CreateFileA    = NULL;
CAPIHook::WriteFileType      CAPIHook::real_WriteFile      = NULL;
CAPIHook::LoadLibraryAType   CAPIHook::real_LoadLibraryA   = NULL;
CAPIHook::CloseHandleType	 CAPIHook::real_CloseHandle = NULL;

HRESULT*   CAPIHook::real_CoCreateInstance   = NULL;


//// For SmartFtp FTPS
CAPIHook::EncryptMessageType   CAPIHook::real_EncryptMessage = NULL;
CAPIHook::DecryptMessageType   CAPIHook::real_DecryptMessage = NULL;

//// For CuteFtp FTPS
CAPIHook::SSL_ReadType  CAPIHook::real_SSL_Read  = NULL;
CAPIHook::SSL_WriteType CAPIHook::real_SSL_Write = NULL;

//// For CuteFtp SFTP
CAPIHook::CreateSFTPConnectionType CAPIHook::real_CreateSFTPConnection = NULL;
CAPIHook::WriteSFTPFile2Type       CAPIHook::real_WriteSFTPFile2       = NULL;
CAPIHook::ReadSFTPFileType         CAPIHook::real_ReadSFTPFile         = NULL;
CAPIHook::CloseSFTPConnectionType  CAPIHook::real_CloseSFTPConnection  = NULL;
CAPIHook::TransmitFileType CAPIHook::real_TransmitFile = NULL;

std::list<APIRES::HOOKAPIINFO>	CAPIHook::m_listSmartFtp_UpFile ;
std::list<APIRES::HOOKAPIINFO>	CAPIHook::m_listSmartFtp_DownFile ;
std::list<APIRES::HOOKAPIINFO>	CAPIHook::m_listSmartFtp_DownFileEx ;
std::list<wstring>   CAPIHook::m_listSmartSftpRemoveList ; 
CPolicy * CAPIHook::m_pPolicy= NULL ;

CAPIHook::DeleteFileWType CAPIHook::real_DeleteFileW = NULL ;
/* When an exception occurs diable hooking for the process.
*/

static bool FPTEIsDisabled(void)
{
        return hook_control.is_disabled();
}

/***********************************************
FTPE doesn't need to handle some special files,
like:
PIPE, DEVICE...
IsSupportFileType() will return false if the current
file doesn't need to handle.
***********************************************/
static bool IsSupportFileType(LPCWSTR lpFilePath)
{
	if(!lpFilePath)
		return false;

	std::wstring strFilePath(lpFilePath);
	std::transform(strFilePath.begin(), strFilePath.end(), strFilePath.begin(), towupper);

	//	Ben, 2011/March/10
	//	when ie download file from ftp server, it will save file to ie temp folder first, so we can't let this function return false, we must return true here
	if (IsProcess(L"iexplore.exe"))
	{
		//	try to get ie temp folder path
		static wchar_t tempBuf[ MAX_PATH + 1 ] = {0};
		if( 0 == tempBuf[0] )
		{
			//	only get it once
			SHGetSpecialFolderPath(NULL, tempBuf, CSIDL_INTERNET_CACHE, NULL);
			DPW((L"FTPE:: get IE temp folder only once: %s", tempBuf));
		}
		//	compare to see if lpFilePath is inside ie temp folder path
		if( tempBuf[0] &&  _memicmp(lpFilePath, tempBuf, wcslen(tempBuf) * sizeof(wchar_t)) == 0 )
		{
			//	yes, they are the same, so return true, this is the purpose of this new added code
			DPW((L"FTPE::This file was explicitly not ignored, %s\r\n", lpFilePath));
			return true;
		}
	}	//	end of Ben, 2011/March/10

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
					DPW((L"FTPE::This file was ignored, %s\r\n", lpFilePath));
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

	return true;
}

typedef struct
{
	DWORD dwSpecialValue;
	wchar_t suffix[MAX_PATH];
}SPEICAL_TEMP_PATH;

static SPEICAL_TEMP_PATH g_TempPath[] = {
	{CSIDL_INTERNET_CACHE, L"\0"},					//	for ie temp folder
	{CSIDL_LOCAL_APPDATA, L"\\opera\\opera"}		//	for opera temp folder
};


 static BOOL IsTempFolder(LPCWSTR lpFilePath)
{
	if(!lpFilePath)
		return false;

	wchar_t* pBuf = new wchar_t[MAX_PATH * 2];
	if(!pBuf)
	{
		return false;
	}

	for (int i = 0; i < _countof(g_TempPath); i++)
	{	
		memset(pBuf, 0, sizeof(wchar_t) * (MAX_PATH * 2));
		SHGetSpecialFolderPath(NULL, pBuf, g_TempPath[i].dwSpecialValue, NULL);
		wcsncat_s(pBuf, MAX_PATH * 2, g_TempPath[i].suffix, _TRUNCATE);
		if(_memicmp(lpFilePath, pBuf, wcslen(pBuf) * sizeof(wchar_t)) == 0)
		{
			delete []pBuf;
			return true;
		}
	}
	
		delete []pBuf;
   return false ;
}

 APIRES::HOOKPROCINFO myProcInfo_SmartCute[] =
 {
	 {"Kernel32.DLL", "DeleteFileW",  NULL, (PVOID *)&CAPIHook::real_DeleteFileW,  CAPIHook::try_DeleteFileW,  NULL }
 };

APIRES::HOOKPROCINFO myProcInfo[] =
{
	{"Kernel32.DLL", "CloseHandle",  NULL, (PVOID *)&CAPIHook::real_CloseHandle,  CAPIHook::try_CloseHandle,  NULL },
	{"Kernel32.DLL", "CreateFileW", NULL, (PVOID *)&CAPIHook::real_CreateFileW, CAPIHook::try_CreateFileW, NULL },
	{"Kernel32.DLL", "CreateFileA", NULL, (PVOID *)&CAPIHook::real_CreateFileA, CAPIHook::try_CreateFileA, NULL },
	{"Kernel32.DLL", "WriteFile",   NULL, (PVOID *)&CAPIHook::real_WriteFile,   CAPIHook::try_WriteFile,   NULL },
	{"Kernel32.DLL", "CopyFileExW",   NULL, (PVOID *)&CAPIHook::real_CopyFileExW,   CAPIHook::try_CopyFileExW,   NULL },
	{"Kernel32.DLL", "MoveFileW",   NULL, (PVOID *)&CAPIHook::real_MoveFileW,   CAPIHook::try_MoveFileW,   NULL },
	{"Kernel32.DLL", "MoveFileExW",   NULL, (PVOID *)&CAPIHook::real_MoveFileExW,   CAPIHook::try_MoveFileExW,   NULL },
	{"Kernel32.DLL", "MoveFileExA",   NULL, (PVOID *)&CAPIHook::real_MoveFileExA,   CAPIHook::try_MoveFileExA,   NULL },
	{"Shell32.DLL", "SHFileOperationW",   NULL, (PVOID *)&CAPIHook::real_SHFileOperationW,   CAPIHook::try_SHFileOperationW,   NULL },
	{"ole32.DLL", "CoCreateInstance",   NULL, (PVOID *)&CAPIHook::real_CoCreateInstance,   CAPIHook::try_CoCreateInstance,   NULL },
	//// For SmartFtp FTPS
	{"Secur32.dll",   "EncryptMessage",   NULL, (PVOID *)&CAPIHook::real_EncryptMessage,   CAPIHook::try_EncryptMessage,   NULL },
	{"Secur32.dll",   "DecryptMessage",   NULL, (PVOID *)&CAPIHook::real_DecryptMessage,   CAPIHook::try_DecryptMessage,   NULL },
	{"MSWSOCK.DLL",   "TransmitFile",   NULL, (PVOID *)&CAPIHook::real_TransmitFile,   CAPIHook::try_TransmitFile,   NULL },
	{NULL,NULL,NULL,NULL,NULL,NULL}
};
APIRES::HOOKPROCINFO myProcssl4FTPTEInfo[] =
{
	{"SSL.DLL", "SSL_Read", NULL, (PVOID *)&CAPIHook::real_SSL_Read, CAPIHook::MySSL_Read, NULL },
	{"SSL.DLL", "SSL_Write", NULL, (PVOID *)&CAPIHook::real_SSL_Write, CAPIHook::MySSL_Write, NULL },
	{NULL,NULL,NULL,NULL,NULL,NULL}
};
APIRES::HOOKPROCINFO myProcSFTP4FTPTEInfo[] =
{
	{"sftp21.DLL", "CreateSFTPConnection", NULL, (PVOID *)&CAPIHook::real_CreateSFTPConnection, CAPIHook::MyCreateSFTPConnection, NULL },
	{"sftp21.DLL", "WriteSFTPFile2", NULL, (PVOID *)&CAPIHook::real_WriteSFTPFile2, CAPIHook::MyWriteSFTPFile2, NULL },
	{"sftp21.DLL", "ReadSFTPFile", NULL, (PVOID *)&CAPIHook::real_ReadSFTPFile, CAPIHook::MyReadSFTPFile, NULL },
	{"sftp21.DLL", "CloseSFTPConnection", NULL, (PVOID *)&CAPIHook::real_CloseSFTPConnection, CAPIHook::MyCloseSFTPConnection, NULL },
	{NULL,NULL,NULL,NULL,NULL,NULL}
};
APIRES::HOOKADDRINFO hookAddrInfo[] = 
{
	{NULL,NULL,NULL,NULL,NULL}
};
APIRES::HOOKEXPIDINFO hookAPI_byIDInfo[] =
{
	{NULL,NULL,NULL,NULL,NULL}
};
BOOL CAPIHook::StartHook()
{
	InitializeMadCHook();

	std::map<std::wstring, int> mapIgnored;
	GetIgnoreFolderList(mapIgnored);

	m_pPolicy =	 CPolicy::CreateInstance() ;
	return HookAPIByInfoList() ;
}

VOID CAPIHook::EndHook()
{
	m_pPolicy->Release() ;

	g_realCreateFileW = NULL;
	g_realCloseHandle = NULL;

	if( !IsProcess(L"firefox.exe") && !IsProcess(L"SmartFTP.exe")  )
	{
		FinalizeMadCHook();
	}

	return; //ignore to call "UnhookAPI", kevin 2009-9-8
}

/*
GetExportFunc_ByID
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
GetFuncAddr_ByOffset
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
BOOL CAPIHook::DoHookSSL4FTPTE(void) 
{
	BOOL bRet = TRUE ;
	for(int iCount = 0; iCount < sizeof(myProcssl4FTPTEInfo) / sizeof(APIRES::HOOKPROCINFO); iCount++)
	{
		if(myProcssl4FTPTEInfo[iCount].pNewProc != NULL )
		{
			if(!HookAPI(myProcssl4FTPTEInfo[iCount].pszModule, myProcssl4FTPTEInfo[iCount].pszOrigName, myProcssl4FTPTEInfo[iCount].pNewProc, myProcssl4FTPTEInfo[iCount].pNextProc))
				return FALSE;
		}
	}
	return bRet ;
}
BOOL CAPIHook::DoHookSFTP4FTPTE(void)
{
	BOOL bRet = TRUE ;
	for(int iCount = 0; iCount < sizeof(myProcSFTP4FTPTEInfo) / sizeof(APIRES::HOOKPROCINFO); iCount++)
	{
		if(myProcSFTP4FTPTEInfo[iCount].pNewProc != NULL )
		{
			if(!HookAPI(myProcSFTP4FTPTEInfo[iCount].pszModule, myProcSFTP4FTPTEInfo[iCount].pszOrigName, myProcSFTP4FTPTEInfo[iCount].pNewProc, myProcSFTP4FTPTEInfo[iCount].pNextProc))
					return FALSE;
		}
	}
	return bRet ;
}
BOOL CAPIHook::HookAPIByInfoList() 
{
	BOOL bRet = FALSE ;
	int iCount = 0;
	if( IsProcess( L"ftpte.exe" ) )
	{

		HMODULE hssl = LoadLibraryW( L"ssl.dll" ) ;
		if( hssl != NULL )
		{
			bRet = DoHookSSL4FTPTE() ;
			hssl = NULL ;
		}
		hssl = LoadLibraryW( L"sftp21.dll" ) ; 
		if(	 hssl )
		{
		  bRet = DoHookSFTP4FTPTE() ;
		}
		
	}
	for(iCount = 0; iCount < sizeof(myProcInfo) / sizeof(APIRES::HOOKPROCINFO); iCount++)
	{
		if(myProcInfo[iCount].pNewProc != NULL )
		{
			
			if( _strnicmp( myProcInfo[iCount].pszOrigName, "CoCreateInstance",MAX_PATH ) ==0 )
			{
				if( IsProcess(L"SmartFTP.exe") )
					{
						DP((L"It is the smart FTP")) ;
						if(!HookAPI(myProcInfo[iCount].pszModule, myProcInfo[iCount].pszOrigName, myProcInfo[iCount].pNewProc, myProcInfo[iCount].pNextProc))
							return FALSE;
					}
				}
			else
			{
				if(!HookAPI(myProcInfo[iCount].pszModule, myProcInfo[iCount].pszOrigName, myProcInfo[iCount].pNewProc, myProcInfo[iCount].pNextProc))
				{
					DPA(("failed to hook API: %s::%s",myProcInfo[iCount].pszModule,myProcInfo[iCount].pszOrigName));
					return FALSE;
				}
			}
		}
			
	}
	 
	for(iCount = 0; iCount < sizeof(hookAddrInfo) / sizeof(APIRES::HOOKADDRINFO); iCount++)
	{
		if( hookAddrInfo[iCount].pNewProc != NULL )
		{

			hookAddrInfo[iCount].pOldProc = GetFuncAddr_ByOffset( hookAddrInfo[iCount].dOffset,hookAddrInfo[iCount].pszModuleName ) ;
			HookCode( hookAddrInfo[iCount].pOldProc ,hookAddrInfo[iCount].pNewProc, hookAddrInfo[iCount].pNextProc ) ;
			bRet = TRUE ;
		}
		
	}
	
	for(iCount = 0; iCount < sizeof(hookAPI_byIDInfo) / sizeof(APIRES::HOOKEXPIDINFO); iCount++)
	{
		if( hookAPI_byIDInfo[iCount].pNewProc != NULL )
		{
			hookAPI_byIDInfo[iCount].pOldProc = GetExportFunc_ByID( hookAPI_byIDInfo[iCount].pszModuleName, hookAPI_byIDInfo[iCount].dID ) ;
			HookCode( hookAPI_byIDInfo[iCount].pOldProc ,hookAPI_byIDInfo[iCount].pNewProc, hookAPI_byIDInfo[iCount].pNextProc ) ;
			
			bRet = TRUE ;
		}
		
	}

	//hook delete file for SmartFTP and CuteFTP
	if(IsProcess(L"SmartFTP.exe") )
	{
		for(iCount = 0; iCount < _countof(myProcInfo_SmartCute); iCount++)
		{
			if(!HookAPI(myProcInfo_SmartCute[iCount].pszModule, myProcInfo_SmartCute[iCount].pszOrigName, myProcInfo_SmartCute[iCount].pNewProc, myProcInfo_SmartCute[iCount].pNextProc))
			{
				return FALSE;
			}
		}
	}

	g_realCreateFileW = CAPIHook::real_CreateFileW;
	g_realCloseHandle = CAPIHook::real_CloseHandle;

	return TRUE;
}
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
			DP((L"Hook API Enter,Origin[%d],my[%d],real[%d],Object[%d],Thread ID[%d]",pOriginFunc,pmyFunc,real_APIAddress,pObjectAddress,_iTID ) ) ;
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
	if(GetDetachFlag() && real_CreateFileW)
	{
		return real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	

	if( FPTEIsDisabled() == true )
	{
		if(!real_CreateFileW)
		{
			return INVALID_HANDLE_VALUE;
		}
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

HANDLE WINAPI CAPIHook::MyCreateFileAW(
		LPCWSTR lpFileNameWChar,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile,
		BOOL bIsUnicode,
		LPCSTR lpFileNameChar)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	if((bIsUnicode==TRUE&&!real_CreateFileW)||
		(bIsUnicode==FALSE&&!real_CreateFileA))
		return INVALID_HANDLE_VALUE;

	if(lpFileNameWChar && !IsSupportFileType(lpFileNameWChar))//Ignore the "not supported" files. Kevin 2009-7-9
	{
		if(bIsUnicode==TRUE)
			return real_CreateFileW(lpFileNameWChar, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
							dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		else
			return real_CreateFileA(lpFileNameChar, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
							dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	if((dwDesiredAccess & GENERIC_READ) && (dwCreationDisposition & OPEN_EXISTING))
	{
		CMapperMgr& ins = CMapperMgr::Instance();
		HANDLE hFile = NULL;
		
		if(bIsUnicode==TRUE)
			hFile=real_CreateFileW(lpFileNameWChar,            // file to open
										GENERIC_READ,          // open for reading
										FILE_SHARE_READ | FILE_SHARE_WRITE,       // share for reading
										NULL,                  // default security
										OPEN_EXISTING,         // existing file only
										FILE_ATTRIBUTE_NORMAL, // normal file
										NULL);                 // no attr. template
		else
			hFile=real_CreateFileA(lpFileNameChar,            // file to open
										GENERIC_READ,          // open for reading
										FILE_SHARE_READ | FILE_SHARE_WRITE,       // share for reading
										NULL,                  // default security
										OPEN_EXISTING,         // existing file only
										FILE_ATTRIBUTE_NORMAL, // normal file
										NULL);   
		if(hFile != INVALID_HANDLE_VALUE )
		{
			if( GetFileType(hFile) == FILE_TYPE_DISK)
			{
				char buf[CMapperMgr::MAX_CONTENT_SIZE];
				DWORD dwBytesRead = 0;
				if(ReadFile(hFile, buf, CMapperMgr::MAX_CONTENT_SIZE, &dwBytesRead, NULL))
				{
					if(dwBytesRead > 0)
					{
						string sContent(buf, dwBytesRead);
						string sPath;
						/*
						modified by kevin 2009-7-20
						*/
						//-----------------------------------------
						if(lpFileNameWChar)
						{
							wstring strFilePath(lpFileNameWChar);
							sPath = MyWideCharToMultipleByte(strFilePath);
									
							//-----------------------------------------
							TranslatePath(sPath);
							ins.PushFileInfoToList(sPath, sContent);
						}
					}
				}
			}
			if(real_CloseHandle)
			{
				real_CloseHandle(hFile);
			}
			else
			{
				CloseHandle(hFile);
			}
		}
	}

	HANDLE hFile = NULL;
	if(bIsUnicode==TRUE)
		hFile=real_CreateFileW(lpFileNameWChar, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
									dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	else
		hFile=real_CreateFileA(lpFileNameChar, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
									dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	if(hFile != INVALID_HANDLE_VALUE && GetFileType(hFile) == FILE_TYPE_DISK)
	{
		if( ((dwDesiredAccess & GENERIC_WRITE)&&(dwCreationDisposition & CREATE_NEW))    ||
			((dwDesiredAccess & GENERIC_WRITE)&&(dwCreationDisposition & CREATE_ALWAYS)) ||
			((dwDesiredAccess & GENERIC_WRITE)&&(dwCreationDisposition & OPEN_ALWAYS))    )
		{
			CMapperMgr& ins = CMapperMgr::Instance();
			string sPath;
			/*
			modified by kevin 2009-7-20
			*/
			//-----------------------------------------
			size_t size = 0;
			wcstombs_s(&size, NULL, 0, lpFileNameWChar, 0/* ignored parameter */);
			char *mbFileName = new char[size];
			memset(mbFileName, 0, size);
			wcstombs_s(&size, mbFileName, size, lpFileNameWChar, _TRUNCATE);
			sPath = string(mbFileName);
			if(mbFileName)
			{
				delete []mbFileName;
				mbFileName = NULL;
			}
			//-----------------------------------------
			TranslatePath(sPath);
			ins.PushHandleName(hFile, sPath);
		}
	}

	return hFile;

}
HANDLE WINAPI CAPIHook::MyCreateFileW(LPCWSTR lpFileName,
								      DWORD dwDesiredAccess,
								      DWORD dwShareMode,
								      LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								      DWORD dwCreationDisposition,
								      DWORD dwFlagsAndAttributes,
								      HANDLE hTemplateFile)
{
	return MyCreateFileAW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
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
	if(GetDetachFlag() && real_CreateFileA)
	{
		return real_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, 
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	if( FPTEIsDisabled() == true )
	{
		if (!real_CreateFileA)
			return NULL;
		return real_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, 
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	__try
	{
		return MyCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, 
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		 ;
	}
	return NULL ;
}
HANDLE WINAPI CAPIHook::MyCreateFileA(LPCSTR lpFileName,
								      DWORD dwDesiredAccess,
									  DWORD dwShareMode,
								      LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								      DWORD dwCreationDisposition,
								      DWORD dwFlagsAndAttributes,
								      HANDLE hTemplateFile)
{
	size_t size = 0;
	mbstowcs_s(&size,NULL,0,lpFileName,0);
	WCHAR *wzFileName = new WCHAR[size];
	memset(wzFileName, 0, size);
	mbstowcs_s(&size, wzFileName, size, lpFileName, _TRUNCATE);

	std::wstring wstrFileName=wzFileName;
	delete[] wzFileName;
	return MyCreateFileAW(wstrFileName.c_str(),dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile,FALSE,lpFileName);
}
BOOL WINAPI CAPIHook::try_WriteFile(HANDLE hFile,
									LPCVOID lpBuffer,
									DWORD nNumberOfBytesToWrite,
									LPDWORD lpNumberOfBytesWritten,
									LPOVERLAPPED lpOverlapped)
{
	if(GetDetachFlag() && real_WriteFile)
	{
		return real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	}
	

	if( FPTEIsDisabled() == true )
	{
		if (!real_WriteFile)
			return FALSE;
		return real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	}
	__try
	{
		return MyWriteFile( hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
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
	
	if(hFile != INVALID_HANDLE_VALUE &&
	   GetFileType(hFile) == FILE_TYPE_DISK &&
	   lpBuffer != NULL &&
	   nNumberOfBytesToWrite > 0)
	{
		CMapperMgr& ins = CMapperMgr::Instance();
		string sLocalPath = ins.PopHandleName(hFile);

		/*	firstly, try to filter by previous denied file list.
		*	only opera and ie use this.
		*/
		if ( (IsProcess(L"opera.exe") || IsProcess(L"iexplore.exe")) && IsDeniedWritefile(hFile, sLocalPath, listDeniedWriteFile) )
		{
			return FALSE;
		}

		if(sLocalPath != "")
		{

			/*
			Modified by chellee on 17-07-2009, for the bug 9470
			*/
			//--------------------------------------------------------------------------------------------------
			FTP_EVAL_INFO evalInfo ;
			/*	modified by ben, on 22-07-2009, for bug 9482, which is the case for using IE, opera, chrome to send mail with attachment download from ftp.
			*	using IE,opera or chrome to send yahoo mail with attachment download from FTP server, the IE etc. process will create 2 files and 
			*	try to write the file data into each file until one write process succeed. So we can not remove the data evaluate item from list in this case.
			*/
			if( (IsProcess(L"chrome.exe") || IsProcess(L"opera.exe") || IsProcess(L"iexplore.exe") || IsProcess(L"explorer.exe")) && (IsTempFolder(   StringT1toT2<char, wchar_t>(sLocalPath).c_str() ) ) )
			{
				evalInfo = ins.PopSocketBufEval4Explorer(string((const char*)lpBuffer, nNumberOfBytesToWrite));
			}
			else
			{
				evalInfo = ins.PopSocketBufEval(string((const char*)lpBuffer, nNumberOfBytesToWrite));
			}
			//--------------------------------------------------------------------------------------------------
			evalInfo.pszSrcFileName = evalInfo.pszDestFileName ;
			evalInfo.pszDestFileName = StringT1toT2<char, wchar_t>(sLocalPath);
			wchar_t szPath[MAX_PATH * 2 + 1] = {0};
			if(	GetLongPathNameW(evalInfo.pszDestFileName.c_str(), szPath, MAX_PATH * 2) > 0)
			{
				evalInfo.pszDestFileName = szPath;
			}
			if(!sLocalPath.empty() && !evalInfo.pszSrcFileName.empty())
			{
						

				if(IsProcess(L"iexplore.exe"))//Ignore to do evaluation if the destination is temp folder of IE.
				{
					static wchar_t szIETemp[MAX_PATH * 2 + 1] = {0};
					if( *szIETemp == 0)
					{
						SHGetSpecialFolderPath(NULL, szIETemp, CSIDL_INTERNET_CACHE, FALSE);
					}
					DPW((L"SHGetSpecialFolderPath Path:[%s],[local:[%s]]",szIETemp,evalInfo.pszDestFileName.c_str() ));
					if(evalInfo.pszDestFileName .length() > wcslen(szIETemp) && _memicmp(evalInfo.pszDestFileName.c_str(), szIETemp, wcslen(szIETemp) * sizeof(wchar_t)) == 0)
					{
						 wstring strTemp = CombineEvalInfo(evalInfo) ;
						g_Temp_DownloadURIList.AddItem(evalInfo.pszDestFileName, strTemp);
						/*
						Removed by chellee for the bug 863, also do the evaluation for the temp file...
						*/
					}
				}

				if(IsProcess(L"ftp.exe")||IsProcess(L"rundll32.exe") ||IsProcess(L"chrome.exe") || IsProcess(L"firefox.exe"))//Ignore to do evaluations if the destination is temp folder.
				{
					static wchar_t szTemp[MAX_PATH * 2 + 1] = {0};
					if( *szTemp == 0)
					{
						SHGetSpecialFolderPath(NULL, szTemp, CSIDL_APPDATA, FALSE);
					}
					wstring strTemp = wstring(szTemp) + L"\\local\\Temp";//the temp folder of Chrome/Firefox
					DPW((L"SHGetSpecialFolderPath Path:[%s]",strTemp.c_str() ));
					DWORD dwMajor = 0, dwMinor = 0;
					if(GetOSInfo(dwMajor, dwMinor) && dwMajor == 6)//Vista, win7
					{//don't use temp folder to determine.
						strTemp = L"";
					}

					if(sLocalPath.length() > strTemp.length() && _memicmp(sLocalPath.c_str(), strTemp.c_str(), strTemp.length() * sizeof(wchar_t)) == 0)
					{//The local file is in temp folder, ignore to do evaluations, just add the related information in list. "my_MoveFile" will try to get the URL with this list.
						 wstring strTemp_l = CombineEvalInfo(evalInfo) ;
						g_Temp_DownloadURIList.AddItem(evalInfo.pszDestFileName, strTemp_l);
						/*
						Removed by chellee for the bug 863, also do the evaluation for the temp file...
						*/
					}
					else
					{
						strTemp = wstring(szTemp) +  L"\\Local Settings\\Temp";//the temp folder of Chrome/Firefox
						if(sLocalPath.length() > strTemp.length() && _memicmp(sLocalPath.c_str(), strTemp.c_str(), strTemp.length() * sizeof(wchar_t)) == 0)
						{
							wstring strTemp_l = CombineEvalInfo(evalInfo) ;
							g_Temp_DownloadURIList.AddItem(evalInfo.pszDestFileName, strTemp_l);
						}
					}
				}
			}
			if(evalInfo.IsValid())
			{
				wostringstream oss;
				oss<<L"FTP user: "<<evalInfo.pszFTPUserName<<L"in mywritefile is trying to DOWNLOAD "<<evalInfo.pszDestFileName<<L" from ftp://" \
					<<evalInfo.pszServerIP<<L":"<<evalInfo.pszServerPort<<L" on ftp location: "<<evalInfo.pszSrcFileName<<endl;
				DPW((L"%s", oss.str().c_str()));
				/*
				Download file to do evaluation
				*/
				CPolicy *pPolicy = CPolicy::CreateInstance() ;
				CEEnforcement_t enforcement ;
				memset(&enforcement, 0, sizeof(CEEnforcement_t));

				FTPE_STATUS status = FTPE_SUCCESS ;
				if( evalInfo.iProtocolType == FPT_REGULAR )
				{
					status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftp, evalInfo , enforcement ) ;
				}else if(evalInfo.iProtocolType == FTP_FTPS_IMPLICIT )
				{
					status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftps, evalInfo , enforcement ) ;
				}
				else
				{
					status = FTPE_ERROR ;
					DPW((L"evaluate ftpe get unexpected protocol type %d", evalInfo.iProtocolType));
				}

				CEResponse_t response = enforcement.result;
				cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

				pPolicy->Release() ;
				if(status == FTPE_SUCCESS)
				{
					switch(response)
					{
					case CEAllow:
						DPW((L"allow ftpe %s, %s", evalInfo.pszDestFileName.c_str(), evalInfo.pszSrcFileName.c_str()));
						break ;
					case CEDeny:
						{
							/*	adjust denied write file list,
							*	only opera use this.
							*/
							if (IsProcess(L"opera.exe") || IsProcess(L"iexplore.exe"))
							{
								PushDeniedWritefile(hFile, sLocalPath, listDeniedWriteFile);
							}

							DPW((L"Deny ftpe %s, %s", evalInfo.pszDestFileName.c_str(), evalInfo.pszSrcFileName.c_str()));
							ins.PushDelHandleName(hFile, sLocalPath);

							//Add the denied file to a list, will check this list in "dele" command of FTP/FTPS
 							if(IsProcess(L"ftpte.exe") || IsProcess(L"explorer.exe"))
 							{
 								std::wstring strDeniedFileName = StringT1toT2<char, wchar_t>(sLocalPath);
								
								size_t uPos = strDeniedFileName.rfind('\\');
 								if(uPos != std::wstring::npos)
 								{
 									strDeniedFileName = strDeniedFileName.substr(uPos + 1, strDeniedFileName.length() - uPos - 1);
 									g_listForCut.AddItem(strDeniedFileName);
 								}
 							}
							
							if(IsProcess(L"bpftpclient.exe"))
							{
								return TRUE;
							}
							else
							{
								if(IsProcess(L"ftpte.exe"))
								{
									real_CloseHandle(hFile);
								}

								if(IsProcess(L"smartftp.exe"))
								{
									SetLastError(ERROR_ACCESS_DENIED);
								}
								return FALSE;
							}
						}
					default:
						break;
					}
				}
			}
		}
	}

	return real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

HMODULE WINAPI CAPIHook::try_LoadLibraryA(LPCSTR lpFileName) 
{
	if(GetDetachFlag() && real_LoadLibraryA)
	{
		return real_LoadLibraryA(lpFileName) ;
	}
	

	if( FPTEIsDisabled() == true )
	{
		if (!real_LoadLibraryA)
			return NULL;
		return real_LoadLibraryA(lpFileName) ;
	}
	__try
	{
		return MyLoadLibraryA(lpFileName) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return NULL ;
}

HMODULE WINAPI CAPIHook::MyLoadLibraryA(LPCSTR lpFileName) 
{
	if(!real_LoadLibraryA)
		return NULL;

	HMODULE hModule = real_LoadLibraryA(lpFileName);
	if(hModule)
	{
		string sFileName(lpFileName);
		if(StringFindNoCase(sFileName,"globalscape") != string::npos)
		{
			string::size_type pos = StringFindNoCase(sFileName,"\\ssl.dll");
			if(pos != string::npos)
			{
				if(pos + strlen("\\ssl.dll") == sFileName.length())
				{
					HookAPI("SSL.dll", "SSL_Read",  CAPIHook::MySSL_Read,  (PVOID*)&CAPIHook::real_SSL_Read);
					HookAPI("SSL.dll", "SSL_Write", CAPIHook::MySSL_Write, (PVOID*)&CAPIHook::real_SSL_Write);
				}
			}
			pos = StringFindNoCase(sFileName, "\\sftp21.dll");
			if(pos != string::npos)
			{
				if(pos + strlen("\\sftp21.dll") == sFileName.length())
				{
					HookAPI("sftp21.dll", "CreateSFTPConnection", CAPIHook::MyCreateSFTPConnection, (PVOID*)&CAPIHook::real_CreateSFTPConnection);
					HookAPI("sftp21.dll", "WriteSFTPFile2",       CAPIHook::MyWriteSFTPFile2,       (PVOID*)&CAPIHook::real_WriteSFTPFile2);
					HookAPI("sftp21.dll", "ReadSFTPFile",         CAPIHook::MyReadSFTPFile,         (PVOID*)&CAPIHook::real_ReadSFTPFile);
					HookAPI("sftp21.dll", "CloseSFTPConnection",  CAPIHook::MyCloseSFTPConnection,  (PVOID*)&CAPIHook::real_CloseSFTPConnection);
				}
			}
		}
	}
	return hModule;
}

//// For SmartFtp FTPS
SECURITY_STATUS __stdcall CAPIHook::try_EncryptMessage(
	PCtxtHandle phContext,
	ULONG fQOP,
	PSecBufferDesc pMessage,
	ULONG MessageSeqNo
	)
{
	if(GetDetachFlag() && real_EncryptMessage)
	{
		return real_EncryptMessage(phContext, fQOP, pMessage, MessageSeqNo);
	}
	

	if( FPTEIsDisabled() == true )
	{
		if (!real_EncryptMessage)
			return SEC_E_CRYPTO_SYSTEM_INVALID;
		return real_EncryptMessage(phContext, fQOP, pMessage, MessageSeqNo);
	}
	__try
	{
		return MyEncryptMessage(phContext, fQOP, pMessage, MessageSeqNo);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		 ;
	}
	return SEC_E_CRYPTO_SYSTEM_INVALID ;
}


SECURITY_STATUS __stdcall CAPIHook::MyEncryptMessage(
	  PCtxtHandle phContext,
	  ULONG fQOP,
	  PSecBufferDesc pMessage,
	  ULONG MessageSeqNo
	)
{
	if(!real_EncryptMessage)
		return SEC_E_CRYPTO_SYSTEM_INVALID;
	for(ULONG ulBuf = 0; ulBuf < pMessage->cBuffers; ++ulBuf)
	{
		SecBuffer buf = pMessage->pBuffers[ulBuf];
		if(buf.BufferType == SECBUFFER_DATA)
		{
			if(buf.pvBuffer && buf.cbBuffer > 0)
			{
				string strBuf((const char*)buf.pvBuffer, buf.cbBuffer);
				DWORD dwThreadID = ::GetCurrentThreadId();
				CMapperMgr& ins = CMapperMgr::Instance();
				ins.PushThreadContent(dwThreadID, strBuf);
			}
		}
	}
	return real_EncryptMessage(phContext, fQOP, pMessage, MessageSeqNo);
}
SECURITY_STATUS __stdcall CAPIHook::try_DecryptMessage(
	PCtxtHandle phContext,
	PSecBufferDesc pMessage,
	ULONG MessageSeqNo,
	PULONG pfQOP
	)
{
	if(GetDetachFlag() && real_DecryptMessage)
	{
		return real_DecryptMessage(phContext, pMessage, MessageSeqNo, pfQOP);
	}
	

	if( FPTEIsDisabled() == true )
	{
		if (!real_DecryptMessage)
			return SEC_E_CRYPTO_SYSTEM_INVALID;
		return real_DecryptMessage(phContext, pMessage, MessageSeqNo, pfQOP);
	}
	__try
	{
		return MyDecryptMessage(phContext, pMessage, MessageSeqNo, pfQOP);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		 ;
	}
	return SEC_E_CRYPTO_SYSTEM_INVALID ;
}

SECURITY_STATUS __stdcall CAPIHook::MyDecryptMessage(
	  PCtxtHandle phContext,
	  PSecBufferDesc pMessage,
	  ULONG MessageSeqNo,
	  PULONG pfQOP
	)
{
	if(!real_DecryptMessage)
		return SEC_E_CRYPTO_SYSTEM_INVALID;
	SECURITY_STATUS ss = real_DecryptMessage(phContext, pMessage, MessageSeqNo, pfQOP);
	if(ss == SEC_E_OK)
	{
		for(ULONG ulBuf = 0; ulBuf < pMessage->cBuffers; ++ulBuf)
		{
			SecBuffer buf = pMessage->pBuffers[ulBuf];
			if(buf.BufferType == SECBUFFER_DATA)
			{
				if(buf.pvBuffer && buf.cbBuffer > 0)
				{
					DWORD dwThreadID = ::GetCurrentThreadId();
					CMapperMgr& mapperIns = CMapperMgr::Instance();
					SOCKET sock = mapperIns.PopThreadSocket(dwThreadID);
					if(sock != INVALID_SOCKET)
					{
						CFtpConnMgr& ins = CFtpConnMgr::Instance();
						ins.SetFtpProtocolType(sock, FTP_FTPS_IMPLICIT);
						string strBuf((const char*)buf.pvBuffer, buf.cbBuffer);
						ParserResult pr = ins.ParseRecv(sock, strBuf);
						if(pr == PARSER_DENY)
						{
							// *lpErrno = ?;
							return SEC_E_CRYPTO_SYSTEM_INVALID;
						}
						else if(pr == PARSER_BUF_MODIFIED)
						{
							for(DWORD idxBuf = 0; idxBuf < buf.cbBuffer; ++idxBuf)
							{
								if(strBuf[idxBuf] != ((char*)buf.pvBuffer)[idxBuf])
								{
									((char*)buf.pvBuffer)[idxBuf] = strBuf[idxBuf];
								}
							}
						}
					}
				}
			}
		}
	}
	return ss;
}

//// For CuteFtp FTPS
int WINAPIV CAPIHook::MySSL_Read( void *s, void *buf,int num)
{
	if (!real_SSL_Read)
		return -1;

	INT iRet  = real_SSL_Read(s, buf, num);
	if(buf && iRet > 0)
	{
		DWORD dwThreadID = ::GetCurrentThreadId();
		CMapperMgr& mapperIns = CMapperMgr::Instance();
		SOCKET sock = mapperIns.PopThreadSocket(dwThreadID);
		if(sock != INVALID_SOCKET)
		{
			CFtpConnMgr& ins = CFtpConnMgr::Instance();
			ins.SetFtpProtocolType(sock, FTP_FTPS_IMPLICIT);
			std::string strBuf((const char*)buf, iRet);
			ParserResult pr = ins.ParseRecv(sock, strBuf);
			if(pr == PARSER_DENY)
			{
				return -1;
			}
			else if(pr == PARSER_BUF_MODIFIED)
			{
				for(int idxBuf = 0; idxBuf < iRet; ++idxBuf)
				{
					if(strBuf[idxBuf] != ((char*)buf)[idxBuf])
					{
						((char*)buf)[idxBuf] = strBuf[idxBuf];
					}
				}
			}
		}
	}
	return iRet ;

}

int WINAPIV CAPIHook::MySSL_Write( void *s,const void *buf,int num)
{
	if (!real_SSL_Write)
		return -1;

	if(buf && num > 0)
	{
		string strBuf((const char*)buf, num);
		DWORD dwThreadID = ::GetCurrentThreadId();
		CMapperMgr& ins = CMapperMgr::Instance();
		ins.PushThreadContent(dwThreadID, strBuf);
	}
	
	return real_SSL_Write(s, buf, num);
}

//// For CuteFtp SFTP
int __cdecl CAPIHook::MyCreateSFTPConnection(DWORD* pConnHandle, int arg_1, int arg_2, int arg_3, const char* user, const char* pwd, int arg_6, int arg_7, int arg_8, int arg_9, char *Src, int arg_11, int arg_12, const char* ip, __int16 port)
{
	if(!real_CreateSFTPConnection)
	{
		return 0;
	}
	int iRet = real_CreateSFTPConnection(pConnHandle, arg_1, arg_2, arg_3, user, pwd, arg_6, arg_7, arg_8, arg_9, Src, arg_11, arg_12, ip, port);
	if(iRet != 0)
	{
		FTP_EVAL_INFO evalInfo;
		evalInfo.pszServerIP = StringT1toT2<char, wchar_t>(ip);
		wchar_t wszPortBuf[10];
		_snwprintf_s(wszPortBuf ,10, _TRUNCATE, L"%d", port);
		evalInfo.pszServerPort = wszPortBuf;
		evalInfo.pszFTPUserName = StringT1toT2<char, wchar_t>(user);
		CMapperMgr& ins = CMapperMgr::Instance();
		ins.AddConhandleEval(*pConnHandle, evalInfo);
	}
	return iRet;
}

int __cdecl CAPIHook::MyWriteSFTPFile2(DWORD connHandle, const char* svrFilePath, HANDLE hFile, int arg_3, int arg_4)
{
	if(!real_WriteSFTPFile2)
	{
		return -1;
	}
	CMapperMgr& ins = CMapperMgr::Instance();
	FTP_EVAL_INFO evalInfo = ins.GetEvalByConhandle(connHandle);
	if(evalInfo.pszServerIP.length() > 0 && evalInfo.pszServerPort.length() > 0 )
	{
		if(GetFileNameFromHandle(hFile, evalInfo.pszSrcFileName))
		{
			for(int i = 0; i < _countof(g_szFiltedPath); i++)
			{
				if (!_wcsnicmp(evalInfo.pszSrcFileName.c_str(), g_szFiltedPath[i], wcslen(g_szFiltedPath[i])))
				{
					wchar_t temp[MAX_PATH*2] = {0};
					wcsncpy_s(temp, MAX_PATH*2, L"\\\\", _TRUNCATE);
					wcsncat_s(temp, MAX_PATH*2, evalInfo.pszSrcFileName.c_str() + wcslen(g_szFiltedPath[i]), _TRUNCATE);
					evalInfo.pszSrcFileName = temp;
					break;
				}
			}

			evalInfo.pszDestFileName =  L"FTP://" +evalInfo.pszServerIP +StringT1toT2<char, wchar_t>(svrFilePath);
			wostringstream oss;
			oss<<L"FTP user: "<<evalInfo.pszFTPUserName<<L" is trying to UPLOAD "<<evalInfo.pszSrcFileName<<L" to ftp://" \
				<<evalInfo.pszServerIP<<L":"<<evalInfo.pszServerPort<<evalInfo.pszDestFileName<<endl;
			DPW((L"%s", oss.str().c_str()));

			/*
			Download file to do evaluation
			*/
			CPolicy *pPolicy = CPolicy::CreateInstance() ;
			CEEnforcement_t enforcement ;
			memset(&enforcement, 0, sizeof(CEEnforcement_t));
			FTPE_STATUS status = pPolicy->QuerySingleFilePolicy(CPolicy::m_sftp, evalInfo , enforcement ) ;

			CEResponse_t response = enforcement.result;
			cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

			pPolicy->Release() ;
			if(status == FTPE_SUCCESS)
			{
				switch(response)
				{
				case CEAllow:
					DPW((L"allow sftp in mywritesftpfile2, %s, %s", evalInfo.pszDestFileName.c_str(), evalInfo.pszSrcFileName.c_str()));
					break ;
				case CEDeny:
					DPW((L"deny sftp in mywritesftpfile2, %s, %s", evalInfo.pszDestFileName.c_str(), evalInfo.pszSrcFileName.c_str()));
					return -1;
					break ;
				default:
					DPW((L"unknown result sftp in mywritesftpfile2, %s, %s", evalInfo.pszDestFileName.c_str(), evalInfo.pszSrcFileName.c_str()));
					break;
				}
			}

		}
	}
	int iRet = real_WriteSFTPFile2(connHandle, svrFilePath, hFile, arg_3, arg_4);
	return iRet;
}

int __cdecl CAPIHook::MyReadSFTPFile(DWORD connHandle, const char* svrFilePath, HANDLE hFile, int arg_3, int arg_4, DWORD dwFileSize, int arg_6)
{
	if(!real_ReadSFTPFile)
	{
		return -1;
	}

	CMapperMgr& ins = CMapperMgr::Instance();
	FTP_EVAL_INFO evalInfo = ins.GetEvalByConhandle(connHandle);
	if(evalInfo.pszServerIP.length() > 0 && evalInfo.pszServerPort.length() > 0 )
	{
		string sLocalFile = ins.PopHandleName(hFile);
		if(sLocalFile.length() > 0)
		{
			evalInfo.pszDestFileName = StringT1toT2<char, wchar_t>(sLocalFile);
			evalInfo.pszSrcFileName = L"FTP://" +evalInfo.pszServerIP + StringT1toT2<char, wchar_t>(svrFilePath);
			wostringstream oss;
			oss<<L"FTP user: "<<evalInfo.pszFTPUserName<<L"in myreadsftpfile is trying to DOWNLOAD "<<evalInfo.pszDestFileName<<L" from ftp://" \
				<<evalInfo.pszServerIP<<L":"<<evalInfo.pszServerPort<<L" on ftp location: "<<evalInfo.pszSrcFileName<<endl;
			DPW((L"%s", oss.str().c_str()));

			/*
			Download file to do evaluation
			*/
			CPolicy *pPolicy = CPolicy::CreateInstance() ;
			CEEnforcement_t enforcement ;
			memset(&enforcement, 0, sizeof(CEEnforcement_t));
			FTPE_STATUS status = pPolicy->QuerySingleFilePolicy( CPolicy::m_sftp, evalInfo , enforcement ) ;
			
			CEResponse_t response = enforcement.result;
			cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

			pPolicy->Release() ;
			if(status == FTPE_SUCCESS)
			{
				switch(response)
				{
				case CEAllow:
					DPW((L"allow sftp in myreadsftpfile, %s, %s", evalInfo.pszDestFileName.c_str(), evalInfo.pszSrcFileName.c_str()));
					break ;
				case CEDeny:
					DPW((L"deny sftp in myreadsftpfile, %s, %s", evalInfo.pszDestFileName.c_str(), evalInfo.pszSrcFileName.c_str()));
					return -1;
					break ;
				default:
					DPW((L"unknown sftp in myreadsftpfile, %s, %s", evalInfo.pszDestFileName.c_str(), evalInfo.pszSrcFileName.c_str()));
					break;
				}
			}

		}
	}

	int iRet = real_ReadSFTPFile(connHandle, svrFilePath, hFile, arg_3, arg_4, dwFileSize, arg_6);
	return iRet;
}

int __cdecl CAPIHook::MyCloseSFTPConnection(DWORD connHandle)
{
	if(!real_CloseSFTPConnection)
	{
		return -1;
	}
	CMapperMgr& ins = CMapperMgr::Instance();
	ins.RemoveConhandleEval(connHandle);
	return real_CloseSFTPConnection(connHandle);
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
		return ((CoCreateInstanceType)real_CoCreateInstance)(rclsid,pUnkOuter,dwClsContext,riid,ppv); 
	}
	

	if( FPTEIsDisabled() == true )
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
			bRet = HookComInterface(tid,m_listSmartFtp_DownFile, proc[0][103], (LONG_PTR) try_DownloadFile, *ppv ) ;
			bRet = HookComInterface(tid,m_listSmartFtp_DownFileEx, proc[0][104],  (LONG_PTR)try_DownloadFileEx, *ppv ) ; 
			bRet = HookComInterface(tid,m_listSmartFtp_UpFile, proc[0][105],  (LONG_PTR)try_UploadFile, *ppv ) ;
		}
		else
		{
			bRet = HookComInterface(tid,m_listSmartFtp_DownFile, proc[0][99], (LONG_PTR) try_DownloadFile, *ppv ) ;
			bRet = HookComInterface(tid,m_listSmartFtp_DownFileEx, proc[0][100],  (LONG_PTR)try_DownloadFileEx, *ppv ) ; 
			bRet = HookComInterface(tid,m_listSmartFtp_UpFile, proc[0][101],  (LONG_PTR)try_UploadFile, *ppv ) ;
		}
	}
	return bRet ;
}
HRESULT WINAPI CAPIHook::retReal_DownloadFile(
		PVOID pthis,
		BSTR bstrRemoteFile, 
		VARIANT varLocalFile, 
		long nRemoteStartPosLo, 
		long nRemoteStartPosHi, 
		long nLocalStartPosLo, 
		long nLocalStartPosHi, 
		PVOID pError)
{
			LONG_PTR orig_DropProc = NULL;
		std::list<APIRES::HOOKAPIINFO>::iterator iter = m_listSmartFtp_DownFile.begin() ;
		for (  ; iter != m_listSmartFtp_DownFile.end() ; iter++ )
		{
			if( pthis == (*iter).pObjectAddress )
			{
				orig_DropProc = (LONG_PTR)(*iter).pRealAPIAddress;
				break ;
			}
		}
		if( orig_DropProc )
		{
			return ((DownloadFileType)orig_DropProc)(	pthis, bstrRemoteFile,	varLocalFile,nRemoteStartPosLo, nRemoteStartPosHi, nLocalStartPosLo, nLocalStartPosHi,   pError ) ;
		}
		return S_FALSE ;
}
HRESULT WINAPI CAPIHook::try_DownloadFile(PVOID pthis,
										  BSTR bstrRemoteFile, 
										  VARIANT varLocalFile, 
										  long nRemoteStartPosLo, 
										  long nRemoteStartPosHi, 
										  long nLocalStartPosLo, 
										  long nLocalStartPosHi, 
										  PVOID pError) 
{
	if( FPTEIsDisabled() == true )
	{
		return retReal_DownloadFile(	pthis, bstrRemoteFile,	varLocalFile,nRemoteStartPosLo, nRemoteStartPosHi, nLocalStartPosLo, nLocalStartPosHi,   pError ) ;
	}
	__try
	{
		return myDownloadFile(	pthis, bstrRemoteFile,	varLocalFile,nRemoteStartPosLo, nRemoteStartPosHi, nLocalStartPosLo, nLocalStartPosHi,   pError ) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		 ;
	}
	return S_FALSE ;

}

HRESULT WINAPI CAPIHook::myDownloadFile(PVOID pthis,
										BSTR bstrRemoteFile, 
										VARIANT varLocalFile, 
										long nRemoteStartPosLo, 
										long nRemoteStartPosHi, 
										long nLocalStartPosLo, 
										long nLocalStartPosHi, 
										PVOID pError)  
{
	HRESULT hr = S_OK ;

	LONG_PTR orig_DropProc = NULL;
	std::list<APIRES::HOOKAPIINFO>::iterator iter = m_listSmartFtp_DownFile.begin() ;
	for (  ; iter != m_listSmartFtp_DownFile.end() ; iter++ )
	{
		if( pthis == (*iter).pObjectAddress )
		{
			orig_DropProc = (LONG_PTR)(*iter).pRealAPIAddress;
			break ;
		}
	}
	if( orig_DropProc )
	{
		DP((L"myDownloadFile Remote File")) ;
		DP((L"myDownloadFile Remote File[%s],Local Type:[%i]",bstrRemoteFile,varLocalFile.vt)) ;
		if( varLocalFile.vt == VT_BSTR )
		{
			DP((L"Local File[%s]",varLocalFile.bstrVal)) ;
		}
		hr = ((DownloadFileType)orig_DropProc)(	pthis, bstrRemoteFile,	varLocalFile,nRemoteStartPosLo, nRemoteStartPosHi, nLocalStartPosLo, nLocalStartPosHi,   pError ) ;
	}
	return hr ;
}
HRESULT WINAPI CAPIHook::retReal_DownloadFileEx(
		PVOID pthis,
		BSTR bstrRemoteFile, 
		VARIANT varLocalFile, 
		ULONGLONG nRemoteStartPos, 
		ULONGLONG nLocalStartPos, 
		long dwCreateDeposition,  
		PVOID retval ) 
 {
	 	LONG_PTR orig_DropProc = NULL;
		std::list<APIRES::HOOKAPIINFO>::iterator iter = m_listSmartFtp_DownFileEx.begin() ;
		for (  ; iter != m_listSmartFtp_DownFileEx.end() ; iter++ )
		{
			if( pthis == (*iter).pObjectAddress )
			{
				orig_DropProc = (LONG_PTR)(*iter).pRealAPIAddress;
				break ;
			}
		}
		if( orig_DropProc )
		{
			return  ((DownloadFileExType)orig_DropProc)(	pthis, bstrRemoteFile,	varLocalFile,nRemoteStartPos, nLocalStartPos, dwCreateDeposition, retval ) ;
		}
		return S_FALSE ;
 }
HRESULT WINAPI CAPIHook::try_DownloadFileEx(PVOID pthis,
											BSTR bstrRemoteFile, 
											VARIANT varLocalFile, 
											ULONGLONG nRemoteStartPos, 
											ULONGLONG nLocalStartPos, 
											long dwCreateDeposition, 
											PVOID retval )
{
	if( FPTEIsDisabled() == true )
	{
		 return	 retReal_DownloadFileEx(		pthis, bstrRemoteFile,	varLocalFile,nRemoteStartPos, nLocalStartPos, dwCreateDeposition, retval ) ;
	}
	__try
	{
		return myDownloadFileEx(		pthis, bstrRemoteFile,	varLocalFile,nRemoteStartPos, nLocalStartPos, dwCreateDeposition, retval ) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		 ;
	}
	return S_FALSE ;

}
HRESULT WINAPI CAPIHook::myDownloadFileEx(PVOID pthis,
										  BSTR bstrRemoteFile, 
										  VARIANT varLocalFile, 
										  ULONGLONG nRemoteStartPos, 
										  ULONGLONG nLocalStartPos, 
										  long dwCreateDeposition, 
										  PVOID retval ) 
{
	HRESULT hr = S_OK ;

	LONG_PTR orig_DropProc = NULL;
	std::list<APIRES::HOOKAPIINFO>::iterator iter = m_listSmartFtp_DownFileEx.begin() ;
	for (  ; iter != m_listSmartFtp_DownFileEx.end() ; iter++ )
	{
		if( pthis == (*iter).pObjectAddress )
		{
			orig_DropProc = (LONG_PTR)(*iter).pRealAPIAddress;
			break ;
		}
	}
	if( orig_DropProc )
	{
		DP((L"myDownloadFileEx ")) ;
		DP((L"myDownloadFileEx Remote File[%s],Local Type:[%i]",bstrRemoteFile,varLocalFile.vt)) ;
		std::wstring  strRemoteFile = bstrRemoteFile;
		if( varLocalFile.vt == VT_BSTR )
		{
			DP((L"Local File[%s]",varLocalFile.bstrVal)) ;
			
		}
		CSftplibImpl sftplibImpl ;
		std::wstring strHost ;
		LONG lPort ;
		std::wstring strUserName ;
		std::wstring strLastPath ;
		sftplibImpl.GetHost( (IDispatch *)pthis,strHost ) ;
		sftplibImpl.GetPort( (IDispatch *)pthis,lPort ) ;
		sftplibImpl.GetUserNameW( (IDispatch *)pthis,strUserName ) ;
		sftplibImpl.GetLastPath( (IDispatch *)pthis,strLastPath ) ;
		strRemoteFile = L"FTP://" +strHost+ strRemoteFile ;
		/*
		Smart FTP download
		*/
		FTP_EVAL_INFO evalInfo ;
		evalInfo.pszServerIP = strHost;
		wchar_t szPort[11] = {0};
		_itow_s(lPort, szPort, 10, 10);
		evalInfo.pszServerPort = szPort ;
		evalInfo.pszFTPUserName = strUserName ;
		evalInfo.pszDestFileName = varLocalFile.bstrVal ;
		evalInfo.pszSrcFileName = strRemoteFile ;
		CPolicy *pPolicy = CPolicy::CreateInstance() ;
		CEEnforcement_t enforcement ;
		memset(&enforcement, 0, sizeof(CEEnforcement_t));
		FTPE_STATUS status = pPolicy->QuerySingleFilePolicy( CPolicy::m_sftp, evalInfo , enforcement ) ;
		status;

		CEResponse_t response = enforcement.result;
		cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

		pPolicy->Release() ;
		if( response ==   CEAllow )
		{
			DPW((L"allow in myDownloadFileEx, %s, %s", evalInfo.pszDestFileName.c_str(), evalInfo.pszSrcFileName.c_str()));
			hr = ((DownloadFileExType)orig_DropProc)(	pthis, bstrRemoteFile,	varLocalFile,nRemoteStartPos, nLocalStartPos, dwCreateDeposition, retval ) ;
		}
	}
	return hr ;
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
	if( FPTEIsDisabled() == true )
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
	/*
	 Modified by chellee on 17-07-2009, for the bug 9472
	*/
	nextlabs::recursion_control_auto auto_disable(hook_control);
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
		DP((L"myUploadFile Remote File[%s],Local Type:[%i]",bstrRemoteFile,varLocalFile.vt)) ;
		std::wstring  strRemoteFile = bstrRemoteFile ;
		if( varLocalFile.vt == VT_BSTR )
		{
			DP((L"Local File[%s]",varLocalFile.bstrVal)) ;

		}
		CSftplibImpl sftplibImpl ;
		std::wstring strHost ;
		LONG lPort ;
		std::wstring strUserName ;
		std::wstring strLastPath ;

		sftplibImpl.GetHost( (IDispatch *)pthis,strHost ) ;
		sftplibImpl.GetPort( (IDispatch *)pthis,lPort ) ;
		sftplibImpl.GetUserNameW( (IDispatch *)pthis,strUserName ) ;
		sftplibImpl.GetLastPath( (IDispatch *)pthis,strLastPath ) ;
		strRemoteFile = L"FTP://" +strHost+ strRemoteFile ;
		/*
		Smart FTP upload
		*/
		FTP_EVAL_INFO evalInfo ;
		evalInfo.pszServerIP = strHost;
		wchar_t szPort[11] = {0};
		_itow_s(lPort, szPort, 10, 10);
		evalInfo.pszServerPort = szPort ;
		evalInfo.pszFTPUserName = strUserName ;
		evalInfo.pszDestFileName = strRemoteFile ;
		evalInfo.pszSrcFileName = varLocalFile.bstrVal ;
		CPolicy *pPolicy = CPolicy::CreateInstance() ;
		CEEnforcement_t enforcement ;
		memset(&enforcement, 0, sizeof(CEEnforcement_t));

		pPolicy->QuerySingleFilePolicy( CPolicy::m_sftp, evalInfo , enforcement ) ;

		CEResponse_t response = enforcement.result;
		cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

		pPolicy->Release() ;
		if( response ==   CEAllow )
		{
			DPW((L"allow in myUploadFile, %s, %s", evalInfo.pszDestFileName.c_str(), evalInfo.pszSrcFileName.c_str()));
			/*
			Modified by chellee on 17-07-2009, for the bug 9472
			*/
			//----------------------------------------------------------------
			std::list<APIRES::HOOKAPIINFO>::reverse_iterator	riter = m_listSmartFtp_UpFile.rbegin() ;
			orig_DropProc =	 (LONG_PTR)(*riter).pRealAPIAddress ;
			//----------------------------------------------------------------
			hr = ((UploadFileType)orig_DropProc)(	pthis, varLocalFile,	bstrRemoteFile,nLocalStartPosLo, nLocalStartPosHi, nRemoteStartPosLo,nRemoteStartPosHi, retval ) ;
		}
		else
		{
			EnterCriticalSection(&CcriticalMngr::s_csDelete);
			m_listSmartSftpRemoveList.push_back(evalInfo.pszSrcFileName) ;
			LeaveCriticalSection(&CcriticalMngr::s_csDelete);
		}
	}
	
	return hr ;
}
HRESULT CAPIHook::GetUnknownPointer( LPVOID pIDispatch, REFIID refID , LPUNKNOWN &lpUnknown )
{
	HRESULT hr = S_OK ;
	CComPtr<IDispatch> lpDispSFTPConn;

	hr = ((LPUNKNOWN)(pIDispatch))->QueryInterface(refID, (void **)&lpDispSFTPConn);
	if (SUCCEEDED(hr) && lpDispSFTPConn != NULL)
	{
		hr = ((LPUNKNOWN)(pIDispatch))->QueryInterface(IID_IUnknown, (void **)&lpUnknown);
	}
	return hr ;
}
DWORD CAPIHook::GetVersionNumber( std::wstring szModuleName, std::wstring szKeyName )
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

BOOL WINAPI CAPIHook::try_CloseHandle(HANDLE hObject)
{
	if(GetDetachFlag() && real_CloseHandle)
	{
		return real_CloseHandle(hObject) ; 
	}
	

	if( FPTEIsDisabled() == true )
	{
		if (!real_CloseHandle)
			return FALSE;
		return real_CloseHandle(hObject) ;
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
	nextlabs::recursion_control_auto auto_disable(hook_control);
	if(!real_CloseHandle)
		return FALSE;

	BOOL bRet = real_CloseHandle(hObject);
	if(bRet)
	{
		CMapperMgr& ins = CMapperMgr::Instance();
		string filename = ins.PopDelHandleName(hObject);
		if(filename.length() > 0)
		{
			DeleteFileA(filename.c_str());
		}

		if (IsProcess(L"iexplore.exe") || IsProcess(L"opera.exe"))
		{
			RemoveDeniedWriteFile(hObject, listDeniedWriteFile);
		}

	}
	return bRet;
}

BOOL WINAPI CAPIHook::try_DeleteFileW(   LPCWSTR lpFileName ) 
{
	if(GetDetachFlag() && real_DeleteFileW)
	{
		return real_DeleteFileW(lpFileName);
	}

	if( FPTEIsDisabled() == true )
	{
		if( real_DeleteFileW )
		   return real_DeleteFileW(lpFileName);
		return FALSE ;
	}
	__try
	{
		return myDeleteFileW(lpFileName);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		 ;
	}
	return FALSE ;

}

BOOL WINAPI CAPIHook::myDeleteFileW(   LPCWSTR lpFileName )
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	BOOL bRet = FALSE ;
	if( real_DeleteFileW ==  NULL )
	{
		return FALSE ;
	}

	if(lpFileName == NULL)
	{
		return real_DeleteFileW(lpFileName);
	}

	wstring strtemp = lpFileName ; 
	EnterCriticalSection(&CcriticalMngr::s_csDelete);
	std::list<wstring>::iterator itor= m_listSmartSftpRemoveList.begin() ;
	for(  ; itor !=	m_listSmartSftpRemoveList.end() ; itor++ )
	{
		if( ::_wcsicmp((*itor).c_str(), strtemp.c_str()) == 0 )
		{
			m_listSmartSftpRemoveList.erase(itor) ;
			LeaveCriticalSection(&CcriticalMngr::s_csDelete);
			return TRUE ;
		}
	}
	LeaveCriticalSection(&CcriticalMngr::s_csDelete);
	bRet = real_DeleteFileW(   lpFileName ) ;
	return bRet ;
}

/********************************************************
fix bug11157
iwindows has an embedec ftp program,
c:\windows\system32\ftp.exe
in this case, ftp.exe won't call WSPSend to send the data.
it will call transmitfile to send the file data to FTP 
Server.
So, we need to hook this API.
**********************************************************/
BOOL CAPIHook::my_TransmitFile( SOCKET hSocket, HANDLE hFile, DWORD nNumberOfBytesToWrite, DWORD nNumberOfBytesPerSend, LPOVERLAPPED lpOverlapped, LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers, DWORD dwFlags )
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	char buf[CMapperMgr::MAX_CONTENT_SIZE] = {0};
	DWORD dwBytesRead = 0;
	if(hFile && ReadFile(hFile, buf, CMapperMgr::MAX_CONTENT_SIZE, &dwBytesRead, NULL))
	{
		SetFilePointer (hFile, 0, NULL, FILE_BEGIN);

		CFtpConnMgr& ftpMgrIns = CFtpConnMgr::Instance();
		if(ftpMgrIns.GetFtpProtocolType(hSocket) == FPT_REGULAR)
		{
			string sBuf(buf, dwBytesRead);
			ParserResult pr = ftpMgrIns.ParseSend(hSocket, sBuf);
			if(pr == PARSER_DENY)
			{
				return FALSE;
			}

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


	if( FPTEIsDisabled() == true )
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


BOOL WINAPI CAPIHook::try_CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
	
	if( GetDetachFlag() ||FPTEIsDisabled() == true )
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
		else if(IsProcess(L"rundll32.exe") ||IsProcess(L"chrome.exe"))
		{
			DPW(( L"my_CopyFileExW, src: %s, dest: %s", strSrc.c_str(), strDest.c_str()));

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
				FTP_EVAL_INFO evalInfo;
				ParseEvalInfoString( strURL, evalInfo) ;
				evalInfo.pszDestFileName =lpNewFileName ; 
				CPolicy *pPolicy = CPolicy::CreateInstance() ;
				CEEnforcement_t enforcement ;
				memset(&enforcement, 0, sizeof(CEEnforcement_t));

				FTPE_STATUS status = FTPE_SUCCESS ;
				if( evalInfo.iProtocolType == FPT_REGULAR )
				{
					status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftp, evalInfo , enforcement ) ;
				}else if(evalInfo.iProtocolType == FTP_FTPS_IMPLICIT )
				{
					status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftps, evalInfo , enforcement ) ;
				}
				else
				{
					status = FTPE_ERROR ;
					DPW((L"evaluate ftpe get unexpected protocol type %d", evalInfo.iProtocolType));
				}

				CEResponse_t response = enforcement.result;
				cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
				pPolicy->Release() ;
				if( response ==   CEDeny )
				{
					return FALSE ;
				}

			}
		}

	}

	return real_CopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags) ; 
}

BOOL CAPIHook::try_MoveFileExW(_In_ LPCWSTR lpExistingFileName, _In_ LPCWSTR lpNewFileName, _In_ DWORD dwFlags )
{
	DPW(( L"try_MoveFileExW, src: %s, dest: %s", lpExistingFileName?lpExistingFileName: L"NULL", lpNewFileName?lpNewFileName: L"NULL"));
	if(GetDetachFlag() || FPTEIsDisabled() == true )
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

	if((IsProcess(L"rundll32.exe") ||IsProcess(L"chrome.exe") || IsProcess(L"firefox.exe"))&& lpExistingFileName && lpNewFileName)
	{
		DPW((L"FTPE::MoveFileEx, src: %s, dest: %s", lpExistingFileName, lpNewFileName));

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
			DPW((L"FTPE::MoveFileEx, This path was denied. %s", sLocalPath.c_str()));
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}

		if ( !MovePhaseEval(lpExistingFileName, lpNewFileName) ) 
		{
			//	be denied
			if(IsProcess(L"firefox.exe"))
			{
				g_listDeniedPath.AddItem(sLocalPath);

				DPW(( L"FTPE::MoveFileEx, Add the denied file name to a list, so that \"myWriteFile\" can check this list to see if the current file was denied. %s", sLocalPath.c_str()));
			}

			SetLastError(ERROR_WRITE_PROTECT);
			return FALSE;
		}
	}

	return real_MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags ) ; 
}

BOOL CAPIHook::try_MoveFileExA(_In_ LPCSTR lpExistingFileName, _In_ LPCSTR lpNewFileName, _In_ DWORD dwFlags )
{
	DPA(( "try_MoveFileExA, src: %s, dest: %s", lpExistingFileName?lpExistingFileName: "NULL", lpNewFileName?lpNewFileName: "NULL"));
	if(GetDetachFlag() || FPTEIsDisabled() == true )
	{
		if( real_MoveFileExA)
		{
			return real_MoveFileExA(lpExistingFileName, lpNewFileName, dwFlags ) ; 
		}

	}
	__try
	{
		return my_MoveFileExA(lpExistingFileName, lpNewFileName, dwFlags ) ;  
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	return FALSE;
}

BOOL CAPIHook::my_MoveFileExA(_In_ LPCSTR lpExistingFileName, _In_ LPCSTR lpNewFileName, _In_ DWORD dwFlags )
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	if(IsProcess(L"ftp.exe"))
	{
		std::string srcFile(lpExistingFileName) ;
		std::string desFile(lpNewFileName) ;
		std::wstring sLocalPath =  MyMultipleByteToWideChar(srcFile);
		std::wstring strDesFile =  MyMultipleByteToWideChar(desFile);
		DPW((L"FTPE::MoveFileEx, src: %s, dest: %s", sLocalPath.c_str(), strDesFile.c_str()));
		wchar_t szPath[MAX_PATH * 2 + 1] = {0};
		if(	GetLongPathNameW(sLocalPath.c_str(), szPath, MAX_PATH * 2) > 0)
		{
			sLocalPath = szPath;
		}
		if(g_listDeniedPath.FindItem(sLocalPath))
		{
			g_listDeniedPath.DeleteItem(sLocalPath);
			DPW((L"FTPE::MoveFileEx, This path was denied. %s", sLocalPath.c_str()));
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}

		if ( !MovePhaseEval(sLocalPath, strDesFile) ) 
		{
			SetLastError(ERROR_WRITE_PROTECT);
			return FALSE;
		}
	}

	return real_MoveFileExA(lpExistingFileName, lpNewFileName, dwFlags ) ; 
}

BOOL CAPIHook::try_MoveFileW(_In_ LPCWSTR lpExistingFileName, _In_ LPCWSTR lpNewFileName )
{
	DPW(( L"FTPE::MoveFile, src: %s, dest: %s", lpExistingFileName, lpNewFileName));
	if( GetDetachFlag() ||FPTEIsDisabled() == true )
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

	if((IsProcess(L"rundll32.exe") ||IsProcess(L"chrome.exe") || IsProcess(L"firefox.exe"))&& lpExistingFileName && lpNewFileName)
	{
		DPW(( L"FTPE::MoveFile, src: %s, dest: %s", lpExistingFileName, lpNewFileName));

		wchar_t szPath[MAX_PATH * 2 + 1] = {0};
		wstring sLocalPath(lpExistingFileName);
		if(	GetLongPathNameW(sLocalPath.c_str(), szPath, MAX_PATH * 2) > 0)
		{
			sLocalPath = szPath;
		}

		if(g_listDeniedPath.FindItem(sLocalPath))
		{
			g_listDeniedPath.DeleteItem(sLocalPath);
			DPW(( L"FTPE::MoveFile, This path was denied. %s", sLocalPath.c_str()));
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
					DP((L"Cannot evaluation a file with a length of zero.\n")) ;
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

				DPW(( L"FTPE::MoveFile, Add the denied file name to a list, so that \"myWriteFile\" can check this list to see if the current file was denied. %s", sLocalPath.c_str()));
			}

			SetLastError(ERROR_WRITE_PROTECT);
			return FALSE;
		}
	}

	return real_MoveFileW(lpExistingFileName, lpNewFileName) ; 
}
BOOL CAPIHook::MovePhaseEval(const wstring & srcMoveFile, const wstring & dstMoveFile)
{
	static wchar_t szTemp[MAX_PATH * 2 + 1] = {0};
	if( *szTemp == 0 )
	{
		SHGetSpecialFolderPath(NULL, szTemp, CSIDL_APPDATA, FALSE);
	}
	wstring strTemp = wstring(szTemp) + L"\\Local\\Temp";//the temp folder of Chrome
	
	DWORD dwMajor = 0, dwMinor = 0;
	if(GetOSInfo(dwMajor, dwMinor) && dwMajor == 6)//Vista, win7
	{
		strTemp = L"";
	}

	wstring strURL;
	BOOL bGetRet = GetURLWithTempPath(srcMoveFile, strTemp, strURL) ;
	if( bGetRet == FALSE)
	{
		strTemp = wstring(szTemp) +  L"\\Local Settings\\Temp";//the temp folder of Chrome/Firefox
		bGetRet= GetURLWithTempPath(srcMoveFile, strTemp, strURL) ;
	}
	if(bGetRet)
	{
		FTP_EVAL_INFO evalInfo;
		 ParseEvalInfoString( strURL, evalInfo) ;
		evalInfo.pszDestFileName =dstMoveFile ; 
		CPolicy *pPolicy = CPolicy::CreateInstance() ;
		CEEnforcement_t enforcement ;
		memset(&enforcement, 0, sizeof(CEEnforcement_t));

		FTPE_STATUS status = FTPE_SUCCESS ;
		if( evalInfo.iProtocolType == FPT_REGULAR )
		{
			status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftp, evalInfo , enforcement ) ;
		}else if(evalInfo.iProtocolType == FTP_FTPS_IMPLICIT )
		{
			status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftps, evalInfo , enforcement ) ;
		}
		else
		{
			status = FTPE_ERROR ;
			DPW((L"evaluate ftpe get unexpected protocol type %d", evalInfo.iProtocolType));
		}

		CEResponse_t response = enforcement.result;
		cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
		pPolicy->Release() ;
		if( response ==   CEAllow )
		{
			return TRUE ;
		}
		else
		{
			return FALSE ;
		}
	}
	return TRUE;
}
int CAPIHook::try_SHFileOperationW(LPSHFILEOPSTRUCT lpFileOp )
{
	if(GetDetachFlag() || FPTEIsDisabled() == true )
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
		DPW(( L"HTTPE::SHFileOperationW, type: %d, src: %s, dest: %s\r\n", lpFileOp->wFunc, lpFileOp->pFrom, lpFileOp->pTo));
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
					if(IsProcess(L"rundll32.exe") ||IsProcess(L"Chrome.exe"))
					{
						wstring strSrc(lpFileOp->pFrom);
						wstring strDest(lpFileOp->pTo);

						DPW(( L"my_SHFileOperationW, src: %s, dest: %s\r\n", strSrc.c_str(), strDest.c_str()));

						wstring strURL;
						if(GetURLWithTempPath(strSrc, L"", strURL))
						{
							FTP_EVAL_INFO evalInfo;
							ParseEvalInfoString( strURL, evalInfo) ;
							evalInfo.pszDestFileName =strDest ; 
							CPolicy *pPolicy = CPolicy::CreateInstance() ;
							CEEnforcement_t enforcement ;
							memset(&enforcement, 0, sizeof(CEEnforcement_t));

							FTPE_STATUS status = FTPE_SUCCESS ;
							if( evalInfo.iProtocolType == FPT_REGULAR )
							{
								status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftp, evalInfo , enforcement ) ;
							}else if(evalInfo.iProtocolType == FTP_FTPS_IMPLICIT )
							{
								status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftps, evalInfo , enforcement ) ;
							}
							else
							{
								status = FTPE_ERROR ;
								DPW((L"evaluate ftpe get unexpected protocol type %d", evalInfo.iProtocolType));
							}

							CEResponse_t response = enforcement.result;
							cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
							pPolicy->Release() ;
							if( response !=   CEAllow )
							{

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