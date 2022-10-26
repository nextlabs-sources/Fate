#include "stdafx.h"
#include "Utils.h"
#include "PromptDlg.h"
#include "FileTagViewDlg.h"
#include "WorkThreads.h"
#include "resattrmgr.h"
#include <algorithm>
#include "DlgTagError.h"
#include "FileTag.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <Strsafe.h>
#include <boost/algorithm/string.hpp>

#define  FILETAGGNING_FILESIZE      1000000
#define  FILETAGGNING_FILESIZE_OFFICE2K7 100000

#define FILETAGGING_INSTALL_PATH_REGKEY			L"SOFTWARE\\Nextlabs\\FileTagging"
#define FILETAGGING_INSTALL_PATH				L"path"

wchar_t g_szSeparator_IndexTag[2] = {L';', 0x00};//Use this as the separator for "Index Tag". kevin 2008-11-7

const DWORD g_kdwLastModifyTimeInterval = 2 * 1000 * 1000 * 10;	// UTC time, unit: 100ns

const DWORD g_kdwMax = (DWORD)~((DWORD)0);

BOOL IsOffice2k7File(LPCWSTR pszFileName)
{
	if(!pszFileName)
		return FALSE;

	LPCWSTR pSuffix = wcsrchr(pszFileName, L'.');
	if(NULL == pSuffix) 
		return FALSE;

	std::vector<std::wstring> vecOffice;
	vecOffice.push_back(L".docx");
	vecOffice.push_back(L".docm");
	vecOffice.push_back(L".dotx");
	vecOffice.push_back(L".dotm");

	vecOffice.push_back(L".xlsm");
	vecOffice.push_back(L".xlsb");
	vecOffice.push_back(L".xlsx");
	vecOffice.push_back(L".xltm");
	vecOffice.push_back(L".xltx");
	vecOffice.push_back(L".xlam");

	vecOffice.push_back(L".potx");
	vecOffice.push_back(L".ppsm");
	vecOffice.push_back(L".ppsx");
	vecOffice.push_back(L".pptm");
	vecOffice.push_back(L".pptx");
	vecOffice.push_back(L".potm");
	vecOffice.push_back(L".ppam");

	wchar_t szSuffix[MAX_PATH + 1] = {0};
	wcsncpy_s(szSuffix, MAX_PATH+1, pSuffix, _TRUNCATE);
	_wcslwr_s(szSuffix, MAX_PATH);
	if(std::find(vecOffice.begin(), vecOffice.end(), szSuffix) != vecOffice.end())
		return TRUE;
	else
		return FALSE;
}

void PrintLogA(const char* _Fmt, ...)
{
	va_list args;
	int     len;
	char    *buffer;

	// retrieve the variable arguments
	va_start( args, _Fmt );

	len = _vscprintf_l( _Fmt, 0, args ) // _vscprintf doesn't count
		+ 1; // terminating '\0'

	buffer = (char*)malloc( len * sizeof(char) );

	if(buffer != NULL)
	{
	vsprintf_s( buffer, len, _Fmt, args ); // C4996
	// Note: vsprintf is deprecated; consider using vsprintf_s instead
	OutputDebugStringA( buffer );

	free( buffer );
	}
	va_end(args);
}

void PrintLogW(const WCHAR* _Fmt, ...)
{
	va_list args;
	int     len;
	WCHAR   *buffer;

	// retrieve the variable arguments
	va_start( args, _Fmt );

	len = _vscwprintf_l( _Fmt, 0, args ) // _vscprintf doesn't count
		+ 1; // terminating '\0'

	buffer = (WCHAR*)malloc( len * sizeof(WCHAR) );

	if(buffer != NULL)
	{
	vswprintf_s( buffer, len, _Fmt, args ); // C4996
	// Note: vsprintf is deprecated; consider using vsprintf_s instead
	OutputDebugStringW( buffer );

	free( buffer );
	}
	va_end(args);
}

BOOL CheckFileTaggingError(LPCWSTR pszFileName, HWND /*hParentWnd*/, DWORD& dwErrorID, int nType)/*0: add, 1:remove, 2: read*/
{
	DWORD dwRet = ::GetFileAttributesW(pszFileName);
	
	if(0xFFFFFFFF == dwRet)
	{//No this file
	//	PopupMessageBox(IDS_FILE_NOT_EXIST, pszFileName, hParentWnd);
		if( 0 == nType )
			dwErrorID = IDS_FILE_NOT_EXIST; 
		else if(1 == nType)
			dwErrorID = IDS_FILE_NOT_EXIST_REMOVE; 
		
		return FALSE;
	}
	else if((FILE_ATTRIBUTE_READONLY & dwRet) == FILE_ATTRIBUTE_READONLY)
	{//readonly
	//	PopupMessageBox(IDS_FILE_READONLY, pszFileName,hParentWnd);
		if( 0 == nType)
			dwErrorID = IDS_FILE_READONLY;
		else if( 1 == nType)
			dwErrorID = IDS_FILE_READONLY_REMOVE;
		
		return FALSE;
	}
	else
	{
/*		size_t size = wcstombs(NULL, pszFileName, 0);
		char * mbFileName = new char[size+1];
		wcstombs(mbFileName, pszFileName, size+1);*/

		HANDLE hHandle = NULL;
		for(int i = 0; i < 3; i++)
		{
			
			hHandle = CreateFileW(pszFileName,     
										GENERIC_READ | GENERIC_WRITE,   
										0,   
										NULL,   
										OPEN_EXISTING,   
										0,   
										NULL);

			if(INVALID_HANDLE_VALUE == hHandle)
			{
				if(GetLastError() == ERROR_SHARING_VIOLATION)
				{
					Sleep(500);
					continue;
				}

				break;
			}

			break;
		}

		if(INVALID_HANDLE_VALUE == hHandle)
		{
	//		PopupMessageBox(IDS_FILE_OPENNING, pszFileName, hParentWnd);
			if( 0 == nType)
				dwErrorID = IDS_FILE_OPENNING;
			else if( 1 == nType)
				dwErrorID = IDS_FILE_OPENNING_REMOVE;
			return FALSE;
		}
		CloseHandle(hHandle);
	}

	return TRUE;
}

void PopupMessageBox(DWORD dwStringID, LPCWSTR /*pszFileName*/, HWND hParentWnd)
{
	wchar_t szHint[1000] = {0};
	LoadStringW(g_hInstance, dwStringID, szHint, 1000);

	wchar_t szTemp[1000] = {0};
//	wsprintf(szTemp, szHint, pszFileName);
	_snwprintf_s(szTemp, 1000, _TRUNCATE, szHint, L"");//fix bug 444 kevin zhou 2008-11-16


	wchar_t szCaption[100] = {0};
	LoadStringW(g_hInstance, IDS_CAPTION, szCaption, 100);

	::MessageBoxW(hParentWnd, szTemp, szCaption, MB_OK|MB_ICONERROR);

}



BOOL NeedPopupPromptDlg(LPCWSTR pszFileName)
{
	if(!pszFileName || _tcslen(pszFileName) <= 3)
		return FALSE;

	std::wstring strFileName(pszFileName);
	std::wstring strFileType = strFileName.substr(strFileName.length() - 3, 3);
	BOOL bNeed = FALSE;
	if(0 == _wcsicmp(strFileType.c_str(), L"pdf"))
	{
		HANDLE hHandle = CreateFileW(pszFileName,     
										GENERIC_READ ,   
										FILE_SHARE_READ,   
										NULL,   
										OPEN_EXISTING,   
										0,   
										NULL);
		if(hHandle)
		{
			DWORD dwLen = 0;
			dwLen = GetFileSize(hHandle, NULL);
			if(dwLen > FILETAGGNING_FILESIZE)
				bNeed = TRUE;
			CloseHandle(hHandle);
		}
	}
/*	else if(IsOffice2k7File(pszFileName))
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		hFind = FindFirstFileW( pszFileName, &FindFileData );
		if(hFind == INVALID_HANDLE_VALUE) return false;
		FindClose(hFind);
		if(FindFileData.nFileSizeLow > FILETAGGNING_FILESIZE_OFFICE2K7) 
			bNeed = TRUE;

	}*///fix bug 8429. this is not a good idea. need to re-consider this bug after EDLP4.0

	return bNeed;
}

std::wstring FormatCurrentTime()
{
	SYSTEMTIME st;
	GetLocalTime(&st); 
	
	wchar_t szBuffer[100] = {0};
	_snwprintf_s(szBuffer, 100, _TRUNCATE, L"%4d-%2d-%2d %2d:%2d:%2d:%4d", st.wYear,st.wMonth,st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return std::wstring(szBuffer);
}

BOOL GetFileNameFromPath(LPCWSTR pszPath, std::wstring& strFileName)
{
	wchar_t drive[_MAX_DRIVE + 1] = {0};
	wchar_t dir[_MAX_DIR + 1] = {0};
	wchar_t fname[_MAX_FNAME + 1] = {0};
	wchar_t ext[_MAX_EXT + 1] = {0};

	if( 0 != _wsplitpath_s( pszPath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname,
		_MAX_FNAME, ext, _MAX_EXT ))
		return FALSE;

	strFileName = std::wstring(fname);
	strFileName.append(ext);
	return TRUE;

}

BOOL CheckAndCreateRandTemp( std::wstring strFilePath, std::wstring &strTempFolder ) 
{
	BOOL bRet = FALSE ;
	struct _stat stat_buffer;
	if (_wstat(strFilePath.c_str(), &stat_buffer) == 0)
	{
		/* If the encrypted file exists, create a random temp folder to hold the new one. */

		WCHAR wzFolderName[MAX_PATH];
		DWORD dwCurrentTick = 0;

		dwCurrentTick = GetTickCount();
		_snwprintf_s(wzFolderName, MAX_PATH, _TRUNCATE, L"%8x", dwCurrentTick);

		//
		strTempFolder += wzFolderName;
		strTempFolder += L"\\";
		CreateDirectoryW(strTempFolder.c_str(), NULL);
		bRet = TRUE ;
	}
	return bRet ;
}

BOOL Split_NXT_Tags(IN std::wstring strIndexTag, OUT std::list<std::wstring>& listTags)
{
	std::wstring strTemp = strIndexTag;

	int nIndex = -1;
	BOOL bSplitted = FALSE;
	while((nIndex = (int)(strTemp.find(g_szSeparator_IndexTag)) ) >= 0)
	{
		bSplitted = TRUE;
		listTags.push_back(strTemp.substr(0, nIndex ));
//		DP((L"Tags in \"Index Tag\", %s\r\n", strTemp.substr(0, nIndex ).c_str()));

		if(nIndex >= (int)(strTemp.length() - 1))
			break;
		strTemp = strTemp.substr(nIndex + 1, strTemp.length() - nIndex - 1);
	}

	return bSplitted;	
}

BOOL GetIndexTag(std::list<smart_ptr<FILETAG_PAIR>>* pList, LPCWSTR pszFileName, OUT std::wstring& strValue)
{
	if(!pList || !pszFileName)
		return FALSE;

	std::list<smart_ptr<FILETAG_PAIR>>::iterator itr;
	for(itr = pList->begin(); itr != pList->end(); itr++)
	{
		smart_ptr<FILETAG_PAIR> spPair = *itr;
		if(_wcsicmp(spPair->strTagName.c_str(), NXTLBS_INDEX_TAG) == 0)
		{
			strValue = spPair->strTagValue;
			DP((L"NEXT_LABS Index Tag: %s \r\n", strValue.c_str()));
			return TRUE;
		}
	}
	DP((L"No Index Tag, GetIndexTag().\r\n"));
	return FALSE;
}

BOOL TagExists(LPCWSTR pszTagName, std::list<std::wstring>& listTags, OUT std::list<std::wstring>::iterator& n_itr)
{
	if(!pszTagName)
		return FALSE;

	std::list<std::wstring>::iterator itr;
	for(itr = listTags.begin(); itr != listTags.end(); itr++)
	{
		std::wstring strTag = *itr;
		if(_wcsicmp(pszTagName, strTag.c_str()) == 0)
		{
			n_itr = itr;
			return TRUE;
		}
	}
	return FALSE;
}

wstring newGUID()
{
	wchar_t wszGuid[65] = {0};
	GUID guid = {0};
	HRESULT hr = ::CoCreateGuid(&guid);
	if (SUCCEEDED(hr))
	{
		swprintf_s(wszGuid, 64, L"{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"
			, guid.Data1
			, guid.Data2
			, guid.Data3
			, guid.Data4[0], guid.Data4[1]
		, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
		, guid.Data4[6], guid.Data4[7]
		);
		return wszGuid;
	}
	return L"";
}
/*
* brief: in order to avoid corrupting the file by our pdf tag lib
         we will duplicate a file and test it first.
         1: means verify success or the file is not pdf file.
         2: means verify failed.
*/
static int VerifyPDFTag(LPCWSTR pszFileName, DWORD &dwError)
{
    DWORD dwRet = 0;
    if (pszFileName == NULL && wcslen(pszFileName) < 4) return 1;
    if (_wcsicmp(pszFileName + wcslen(pszFileName) - 4, L".pdf") != 0)      return 1;   // it is not a pdf file, so do nothing

    wchar_t szTempFileName[1024] = { 0 };
    wchar_t lpTempPathBuffer[512] = { 0 };
    dwError = 1;
    // try to get temp path with another solution
    if (!SHGetSpecialFolderPath(NULL, lpTempPathBuffer, CSIDL_LOCAL_APPDATA, FALSE))
    {
        if (!SHGetSpecialFolderPath(NULL, lpTempPathBuffer, CSIDL_INTERNET_CACHE, FALSE))
        {
            DWORD dwRetVal = GetTempPathW(MAX_PATH, lpTempPathBuffer);
            if (dwRetVal > MAX_PATH || (dwRetVal == 0))
            {
                OutputDebugStringW(L"--------------------------- get temp file path in VerifyPDFTag function failed, return out.\n");
                return 2; // if we still can't generate temp path, tag file should not work either.
            }
        }
    }
    else
    {
        wcscat_s(lpTempPathBuffer, L"\\Temp");
    }

    DWORD uRetVal = GetTempFileNameW(lpTempPathBuffer, // directory for tmp files
        NULL,     // temp file name prefix 
        0,                // create unique name 
        szTempFileName);  // buffer for name 
    if (uRetVal == 0)
    {
        wcscpy_s(szTempFileName, lpTempPathBuffer);
        wcscat_s(szTempFileName, L"\\");
        wcscat_s(szTempFileName, newGUID().c_str());
    }
    wcscat_s(szTempFileName, L".pdf");
    if (!CopyFileW(pszFileName, szTempFileName, FALSE))
    {
        wchar_t szlog[2049] = { 0 };
        StringCchPrintfW(szlog,2048, L"------------ copy file from %s to %s failed, error is %d.\n", pszFileName,szTempFileName,GetLastError());
        OutputDebugStringW(szlog);
        return 2;
    }

    CFileTag FileTag;
    wstring strTagName = newGUID();
    if (strTagName.empty())        strTagName = L"NextLabsTestTagName";

    wstring strTagValue = newGUID();
    if (strTagValue.empty())        strTagValue = L"NextLabsTestTagValue";
    dwRet = 2;
    dwError = FileTag.AddTag(strTagName.c_str(), strTagValue.c_str(), szTempFileName);
    if (dwError == 0)
    {
        wstring strTestTagValue = L"";
        dwError = FileTag.GetTagValueByName(strTagName.c_str(), szTempFileName, strTestTagValue);
        if (dwError == 0)
        {
            if (0 == _wcsicmp(strTestTagValue.c_str(), strTagValue.c_str()))
            {
                dwRet = 1;
                dwError = 0;
            }
        }
    }
    DeleteFileW(szTempFileName);
    return dwRet;
}


BOOL AddTagEx(std::list<smart_ptr<FILETAG_PAIR>>* pList, LPCWSTR pszFileName, CFileTagMgr* pMgr, LPCWSTR szErrorAction /*= NULL*//*block or continue*/, LPCWSTR szMessageIfBlcok/* = NULL*/)
{
	if(!pList || !pszFileName || !pMgr)
		return FALSE;

	LPMANUALTHREADPARAM pParam = new MANUALTHREADPARAM;
	if(!pParam)
		return FALSE;

	DWORD dwError = 0;
    if (VerifyPDFTag(pszFileName, dwError) == 1)
	{
		CPromptDlg dlg;
		dlg.SetEndFlag(FALSE);
		dlg.SetPathInfo(pszFileName);

		pParam->pList = pList;
		pParam->pMgr = pMgr;
		pParam->pDlg = &dlg;
		pParam->pszFileName = pszFileName;


		if(NeedPopupPromptDlg(pszFileName))
		{
			unsigned dwThreadID;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &DoManualTaggingThread, pParam, 0, &dwThreadID );
			if(!hThread)
			{
				DP((L"Failed to begin thread for DoManualTaggingThread()"));
				if(pParam)
					delete pParam;
				return FALSE;
			}
			dlg.DoModal();
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
		else
		{
			pParam->pDlg = NULL;
			DoManualTaggingThread(pParam);
		}
	}
	else
	{
		pParam->pszFileName = pszFileName;
		pParam->bSucceed = FALSE;
		pParam->dwErrorID = dwError;
	}
	

	BOOL bTag = pParam->bSucceed;
	if (!bTag)
	{
		if (!g_bLoadByOE)
		{
			PopupMessageBox(pParam->dwErrorID, pParam->pszFileName, pMgr->GetFileTag()->GetParentWnd());
		}
		else
		{
			if ((szErrorAction!=NULL) && (_wcsicmp(szErrorAction, ERROR_ACTION_BLOCK) == 0))
		    {
				ShowTagErrorBlockMessage(pMgr->GetFileTag()->GetParentWnd(), pParam->pszFileName, szMessageIfBlcok);
		    }
            else
            {
                if (pParam)
                    delete pParam;
                return TRUE;
            }
		}
		
	}
	
	if(pParam)
		delete pParam;

	return bTag;
}

BOOL AddTagEx(LPCWSTR pszTagName, LPCWSTR pszTagValue, LPCWSTR pszFileName, CFileTagMgr* pMgr)
{
	if(!pszTagName || !pszTagValue || !pszFileName || !pMgr)
		return FALSE;

	std::list<smart_ptr<FILETAG_PAIR>> listTags;
	smart_ptr<FILETAG_PAIR> sp_Pair( new FILETAG_PAIR);
	sp_Pair->strTagName = std::wstring(pszTagName);
	sp_Pair->strTagValue = std::wstring(pszTagValue);
	listTags.push_back(sp_Pair);

	return AddTagEx(&listTags, pszFileName, pMgr);

}

BOOL RemoveTagEx(std::list<std::wstring>* pList, LPCWSTR pszFileName, CFileTagMgr* pMgr)
{
	if(!pList || !pszFileName || !pMgr)
		return FALSE;

	LPREMOVETAGTHREADPARAM pParam = new REMOVETAGTHREADPARAM;
	if(!pParam)
		return FALSE;

	CPromptDlg dlg;
	dlg.SetEndFlag(FALSE);
	dlg.SetPathInfo(pszFileName);

	pParam->pList = pList;
	pParam->pMgr = pMgr;
	pParam->pDlg = &dlg;
	pParam->pszFileName = pszFileName;

	if(NeedPopupPromptDlg(pszFileName))
	{
		unsigned dwThreadID;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &DoRemoveTagsThread, pParam, 0, &dwThreadID );
		if(!hThread)
		{
			DP((L"Failed to begin thread for DoRemoveTagsThread()"));
			if(pParam)
				delete pParam;
			return FALSE;
		}
		dlg.DoModal();
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
	else
	{
		pParam->pDlg = NULL;
		DoRemoveTagsThread(pParam);
	}

	BOOL bSucceed = pParam->bSucceed;
	if(!bSucceed)
	{
        if (!g_bLoadByOE)
        {
            // Fix Bug 35130, OE no need this warining box
		    PopupMessageBox(pParam->dwErrorID, pParam->pszFileName, pMgr->GetFileTag()->GetParentWnd());
        }
	}

	if(pParam)
		delete pParam;

	return bSucceed;
}

BOOL GetAllTagsEx( LPCWSTR pszFileName, CFileTagMgr* pMgr, OUT std::list<smart_ptr<FILETAG_PAIR>>* pList)
{
	if(!pszFileName || !pMgr || !pList)
		return FALSE;

	LPGETALLTAGVALUESTHREADPARAM pParam = new GETALLTAGVALUESTHREADPARAM;
	if(!pParam)
		return FALSE;
	pParam->pList = pList;
	pParam->pszFileName = pszFileName;
	pParam->pMgr = pMgr;

	if(NeedPopupPromptDlg(pszFileName))
	{
		CPromptDlg dlg;
		dlg.SetEndFlag(FALSE);
		dlg.SetPathInfo(pszFileName);
		pParam->pDlg = &dlg;

		unsigned dwThreadID;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &GetAllTagValuesThread, pParam, 0, &dwThreadID );
		if(!hThread)
		{
			DP((L"Failed to begin thread GetAllTagValuesThread()\r\n"));
			if(pParam)
				delete pParam;
			return FALSE;
		}
		dlg.DoModal();
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
	else
	{
		pParam->pDlg = NULL;
		GetAllTagValuesThread(pParam);
	}
	BOOL bSuccess = pParam->bSuccess;
	if(!pParam->bSuccess)
		PopupMessageBox(pParam->dwErrorID, pszFileName,pMgr->GetFileTag()->GetParentWnd());

	if(pParam)
		delete pParam;


	return bSuccess;
}

std::wstring GetInstallPath(HMODULE hModule)
{
	if ( 0 == (GetVersion() & 0x80000000UL) )
	{
		CRegKey theReg;
		LONG lRet = theReg.Open(HKEY_LOCAL_MACHINE, FILETAGGING_INSTALL_PATH_REGKEY);

		if(ERROR_SUCCESS == lRet)
		{
			wchar_t szPath[MAX_PATH * 2 + 1] = {0};
			ULONG lCount = MAX_PATH;
			theReg.QueryStringValue(FILETAGGING_INSTALL_PATH, szPath, &lCount);
			if(wcslen(szPath) > 3)
			{
				std::wstring strTemp(szPath);
				if(strTemp[strTemp.length() - 1] != '\\')
					strTemp.append(L"\\");
				return strTemp;
			}
			theReg.Close();
		}
	}

	if(hModule)
	{
		wchar_t szPath[MAX_PATH * 2] = {0};
		GetModuleFileName(hModule, szPath, MAX_PATH);
		wchar_t* p = wcsrchr(szPath, '\\');
		if(p)
			*(p + 1) = '\0';
		return szPath;
	}
	else
	return L"";
}

BOOL DoAutoTag(LPAUTOTHREADPARAM& pParam, HWND hParentWnd, LPCWSTR szErrorAction/* = NULL*//*block or continue*/, LPCWSTR szMessageIfBlcok/* = NULL*/)
{
	if(!pParam)
		return FALSE;

	DWORD dwError = 0;
    if (VerifyPDFTag(pParam->pszFileName, dwError) == 1)
	{
		if(NeedPopupPromptDlg(pParam->pszFileName))
		{
			CPromptDlg dlg;
			dlg.SetEndFlag(FALSE);
			dlg.SetPathInfo(pParam->pszFileName);
			pParam->pDlg = &dlg;

			unsigned dwThreadID = 0;

			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &DoAutomaticTaggingThread, pParam, 0, &dwThreadID );
			if(!hThread)
			{
				DP((L"Failed to begin thread for DoAutomaticTaggingThread()"));
				return FALSE;
			}
			dlg.DoModal();
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
		else
		{
			pParam->pDlg = NULL;
			DoAutomaticTaggingThread(pParam);
		}
	}
	else
	{
		pParam->bSucceed = FALSE;
		pParam->dwErrorID = dwError;
	}

	

	if(!pParam->bSucceed)
	{
		if (szErrorAction==NULL)   //old mode
		{
			PopupMessageBox(pParam->dwErrorID, pParam->pszFileName, hParentWnd);//fix bug292
			return FALSE;
		}
		else//OE 8.2
		{
			if (_wcsicmp(szErrorAction, ERROR_ACTION_BLOCK)==0)
			{
				ShowTagErrorBlockMessage(hParentWnd, pParam->pszFileName, szMessageIfBlcok);
				return FALSE;
			}
			else
			{	//continue
				return TRUE;
			}
		}
	}
	return TRUE;
}

int ShowTagErrorBlockMessage(HWND hParent, LPCWSTR szFileName, LPCWSTR szMessage)
{
	std::wstring wstrMsg = szMessage;
	const wchar_t* pszFileNameKey = L"<filename>";
	size_t nPos = wstrMsg.find(pszFileNameKey);
	if (nPos != std::wstring::npos)
	{
		std::wstring wstrName = szFileName; 
		size_t nPosName = wstrName.find_last_of(L'\\');
		if (nPosName==std::wstring::npos)
		{
			nPosName = wstrName.find_last_of(L'/');
		}
		if (nPosName != std::wstring::npos)
		{
			wstrName = wstrName.substr(nPosName + 1);
		}
		wstrMsg = wstrMsg.replace(nPos, wcslen(pszFileNameKey), wstrName.c_str());
	}
	

	CDlgTagError dlgTagError(IDD_DLG_TAGERROR_BLOCK, wstrMsg.c_str());
	dlgTagError.DoModal(hParent == NULL ? GetForegroundWindow() : hParent);

	return 1;
}

BOOL IsLoadByOE()
{
	DWORD dwBufLen = 1024;
	wchar_t* pwszExeFileName = NULL;
	do 
	{
		pwszExeFileName = new wchar_t[dwBufLen];
		DWORD dwGetLen = GetModuleFileNameW(NULL, pwszExeFileName, dwBufLen);
	    if (dwGetLen<=0)//error
	    {
			delete[] pwszExeFileName;
			return FALSE;
	    }
		else if (dwGetLen<dwBufLen)	 //correct
		{
			pwszExeFileName[dwGetLen] = 0;
			BOOL bEndWithOutlook = boost::algorithm::iends_with(pwszExeFileName, L"outlook.exe");
			delete[] pwszExeFileName;
			DP((L"IsLoadByOE return %d.\n", bEndWithOutlook));
			return bEndWithOutlook;
		}
		else if (dwGetLen == dwBufLen) 	//dwBufLen==dwBufLen means the buffer is too small, we need GetModuleFileNameW will larger buffer again
		{
			delete[] pwszExeFileName;
			pwszExeFileName = NULL;
			dwBufLen += 1024;
		}	    

	} while (TRUE); 
}

std::wstring DwordToString(_In_ const DWORD kdwIn)
{
    static const DWORD kdwRax = 10;
    std::wstring wstrOut = L"";
    DWORD dwQuotient = kdwIn;
    DWORD dwRemainder = 0;
    do
    {
        dwRemainder = dwQuotient % kdwRax;
#pragma warning ( push )
#pragma warning ( disable: 4244 )
        wstrOut.push_back(dwRemainder + '0');   // warning 4244, DWORD to wchar_t maybe lose data
#pragma warning ( pop )

        dwQuotient = dwQuotient / kdwRax;
    } while (dwQuotient);

    size_t stLen = wstrOut.length();
    size_t stTempIndex = 0;
    for (size_t stIndex = 0; stIndex < (stLen / 2); ++stIndex)
    {
        stTempIndex = stLen - stIndex - 1;
        wstrOut[stTempIndex] = wstrOut[stIndex] ^ wstrOut[stTempIndex];
        wstrOut[stIndex] = wstrOut[stIndex] ^ wstrOut[stTempIndex];
        wstrOut[stTempIndex] = wstrOut[stIndex] ^ wstrOut[stTempIndex];
    }
    return wstrOut;
}

DWORD StringToDword(_In_ const std::wstring& kwstrIn)
{
    static const DWORD kdwRax = 10;

    // First get the legal string length
    size_t stLegalLen = 0;
    for (stLegalLen = 0; stLegalLen < kwstrIn.length(); ++stLegalLen)
    {
        if (!isdigit(kwstrIn[stLegalLen]))
        {
            break;
        }
        continue;
    }

    DWORD dwOut = 0;
    size_t stIndex = stLegalLen - 1;
    DWORD dwDigit = 1;
    while (0 < stLegalLen)
    {
        dwOut += (kwstrIn[stIndex] - '0') * dwDigit;

        if (0 == stIndex)
        {
            break;
        }
        --stIndex;
        dwDigit *= kdwRax;
    }
    return dwOut;
}

// 0: CreateTime, 1: LastModifyTime, 2: LastAccessTime
bool GetFileTimeByType(_In_ const std::wstring& kwstrFileFullPath, _In_ const int knType, _Out_ FILETIME* pstuFileTime)
{
    if (kwstrFileFullPath.empty() || (!PathFileExistsW(kwstrFileFullPath.c_str())))
    {
        DP((L"Parameter error, file path:[%s] is empty or not exist\n", kwstrFileFullPath.c_str()));
        return false;
    }

    if (NULL == pstuFileTime)
    {
        DP((L"Parameter error, pstuFileTime is null\n"));
        return false;
    }

    if ((2 < knType) || (0 > knType))
    {
        DP((L"Parameter error, knType:[%d] is unknown, now only support 0, 1, 2\n", knType));
        return false;
    }

    bool bRet = false;
    pstuFileTime->dwHighDateTime = 0;
    pstuFileTime->dwLowDateTime = 0;

    WIN32_FIND_DATA FindFileData = { 0 };
    ::SetLastError(ERROR_SUCCESS);
    HANDLE hFind = FindFirstFile(kwstrFileFullPath.c_str(), &FindFileData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        FindClose(hFind);
        bRet = true;
        if (0 == knType)    // Create time
        {
            (*pstuFileTime) = FindFileData.ftCreationTime;
        }
        else if (1 == knType)   // Last modify time
        {
            (*pstuFileTime) = FindFileData.ftLastWriteTime;
        }
        else if (2 == knType)   // Last access time
        {
            (*pstuFileTime) = FindFileData.ftLastAccessTime;
        }
        else
        {
            bRet = false;
        }
    }
    else
    {
        DP((L"Failed to get file:[%s] attribute with last error:[%d]\n", kwstrFileFullPath.c_str(), ::GetLastError()));
    }
    return bRet;
}

bool AreAllDigits(_In_ const std::wstring& kwstrIn)
{
    if (kwstrIn.empty())
    {
        return false;
    }
    bool bRet = true;
    for (size_t stIndex = 0; stIndex < kwstrIn.length(); ++stIndex)
    {
        if (!isdigit(kwstrIn[stIndex]))
        {
            bRet = false;
            break;
        }
        continue;
    }
    return bRet;
}

bool ComvertStrTimeToFileTime(_In_ const std::wstring& kwstrFileTime, _Out_ FILETIME* pstuFileTime, _In_ const wchar_t kwchSep, _In_ const bool kbStrict)
{
    // Check parameter
    if (kwstrFileTime.empty() || (NULL == pstuFileTime))
    {
        return false;
    }

    bool bRet = false;
    pstuFileTime->dwLowDateTime = 0;
    pstuFileTime->dwHighDateTime = 0;

    size_t stPos = kwstrFileTime.find(kwchSep);
    if (stPos != std::wstring::npos)
    {
        std::wstring wstrHigh32 = kwstrFileTime.substr(0, stPos);
        std::wstring wstrLow32 = kwstrFileTime.substr(stPos + 1, kwstrFileTime.length() - stPos - 1);
        bRet = (!kbStrict) || (kbStrict && AreAllDigits(wstrHigh32) && AreAllDigits(wstrLow32));
        if (bRet)
        {
            pstuFileTime->dwHighDateTime = StringToDword(wstrHigh32.c_str());
            pstuFileTime->dwLowDateTime = StringToDword(wstrLow32.c_str());
            DP((L"kbStrict:[%s], High:[%u]==[%s], Low:[%u]==[%s]\n", kbStrict ? L"true" : L"false", pstuFileTime->dwHighDateTime, wstrHigh32.c_str(), pstuFileTime->dwLowDateTime, wstrLow32.c_str()));
        }
    }
    return bRet;
}

std::wstring ComvertFileTimeToString(_In_ const FILETIME& kstuFileTime, _In_ const wchar_t kwchSep)
{
    std::wstring wstrRetFileTime = DwordToString(kstuFileTime.dwHighDateTime);
    wstrRetFileTime += kwchSep;
    wstrRetFileTime += DwordToString(kstuFileTime.dwLowDateTime);
    DP((L"FileTime:[%u]-[%u]==[%s]\n", kstuFileTime.dwHighDateTime, kstuFileTime.dwLowDateTime, wstrRetFileTime.c_str()));
    return wstrRetFileTime;
}

FILETIME NLGetAbsDiffFileTime(_In_ const FILETIME& kstuFirst, _In_ const FILETIME& kstuSecond)
{
    FILETIME stuRet = {0, 0};
    FILETIME stuBig = kstuFirst;
    FILETIME stuSmall = kstuSecond;
    if (kstuFirst.dwHighDateTime < kstuSecond.dwHighDateTime)
    {
        stuBig = kstuSecond;
        stuSmall = kstuFirst;
    }
    else if ((kstuFirst.dwHighDateTime == kstuSecond.dwHighDateTime) && (kstuFirst.dwLowDateTime < kstuSecond.dwLowDateTime))
    {
        stuBig = kstuSecond;
        stuSmall = kstuFirst;
    }

    if (stuBig.dwLowDateTime >= stuSmall.dwLowDateTime)
    {
        stuRet.dwHighDateTime = stuBig.dwHighDateTime - stuSmall.dwHighDateTime;
        stuRet.dwLowDateTime = stuBig.dwLowDateTime - stuSmall.dwLowDateTime;
    }
    else
    {
        stuRet.dwHighDateTime = stuBig.dwHighDateTime - stuSmall.dwHighDateTime - 1;
        DWORD dwTemp = stuSmall.dwLowDateTime - stuBig.dwLowDateTime;
        stuRet.dwLowDateTime = g_kdwMax - dwTemp;
        stuRet.dwLowDateTime += 1;
    }
    return stuRet;
}

void NLAddFileTime(_In_ FILETIME& kstuIn, _In_ const DWORD kdwIn)
{
    if (0 == kstuIn.dwLowDateTime)
    {
        kstuIn.dwLowDateTime += kdwIn;
    }
    else
    {
        DWORD dwTemp = g_kdwMax- kstuIn.dwLowDateTime;
        dwTemp += 1;
        if (dwTemp > kdwIn)
        {
            kstuIn.dwLowDateTime += kdwIn;
        }
        else
        {
            kstuIn.dwLowDateTime = kdwIn - dwTemp;
            kstuIn.dwHighDateTime += 1;
        }
    }
}

bool NLIsUNCPath(_In_ const wstring& wstrFilePath)
{
    bool bIsUNCPath = false;
    // UNC path is start with "\\*"
    if (3 <= wstrFilePath.length())
    {
        bIsUNCPath = (L'\\' == wstrFilePath[0]) && (L'\\' == wstrFilePath[1]) && (L'\\' != wstrFilePath[2]);
    }
    return bIsUNCPath;
}

// kwstrNLLastModifyTime: a string file time which record in the file by NEXTLABS HSC obligation, format is "high32-low32
// stuFileLastModifyTime: current file last modify time
bool IsTheFileModified(_In_ const std::wstring& kwstrNLLastModifyTime, _In_ const FILETIME& stuFileLastModifyTime, _In_ const DWORD knTimeInterval)
{
    bool bHasModified = true;
    if ((!kwstrNLLastModifyTime.empty()) && (0 < knTimeInterval) && ((0 != stuFileLastModifyTime.dwHighDateTime) || (0 != stuFileLastModifyTime.dwLowDateTime)))
    {
        FILETIME stuNLLastModifyTime = { 0, 0 };
        if (ComvertStrTimeToFileTime(kwstrNLLastModifyTime, &stuNLLastModifyTime, pafUI::g_kwchSepLastModifyTimeHighAndLow, true))
        {
            // Compare modify time, only check the low 32bit
            FILETIME stuAbsDiff = NLGetAbsDiffFileTime(stuFileLastModifyTime, stuNLLastModifyTime);
            if (0 == stuAbsDiff.dwHighDateTime)
            {
                bHasModified = (knTimeInterval < stuAbsDiff.dwLowDateTime);
            }
            NLPRINT_DEBUGVIEWLOG(L"Real Time interval:[%u-%u], acceptable interval:[%u], Modified:[%s]\n", stuAbsDiff.dwHighDateTime, stuAbsDiff.dwLowDateTime, knTimeInterval, bHasModified?L"true":L"false");
        }
    }
    return bHasModified;
}

bool AddNLFileLastModifyTime(_Inout_ std::list<smart_ptr<FILETAG_PAIR> >& lstFileTag, _In_ std::wstring& wstrFilePath)
{
    bool bRet = false;
    // Get current system time
    SYSTEMTIME stuCurTime = { 0 };
    GetSystemTime(&stuCurTime);

    // Convert system time to file time
    FILETIME stuCurFileTime = { 0, 0 };
    if (SystemTimeToFileTime(&stuCurTime, &stuCurFileTime))
    {
        if (NLIsUNCPath(wstrFilePath))
        {
           NLAddFileTime(stuCurFileTime, g_kdwLastModifyTimeInterval);  // UNC file, last modify time  error compensator.
        }
        
        std::wstring wstrCurFileTime = ComvertFileTimeToString(stuCurFileTime, pafUI::g_kwchSepLastModifyTimeHighAndLow);
        if (!wstrCurFileTime.empty())
        {
            NLPRINT_DEBUGVIEWLOG(L"Tagged FileTime:[%s]\n", wstrCurFileTime.c_str());
            smart_ptr<FILETAG_PAIR> sptagPairFileLastModifyTime = smart_ptr<FILETAG_PAIR>(new FILETAG_PAIR());
            sptagPairFileLastModifyTime->strTagName = NLLASTMODIFYTIMETAGNAME;
            sptagPairFileLastModifyTime->strTagValue = wstrCurFileTime;
            SetTagValueToTagPair(lstFileTag, sptagPairFileLastModifyTime);
            bRet = true;
        }
    }
    return bRet;
}

bool SetTagValueToTagPair(_Inout_ std::list<smart_ptr<FILETAG_PAIR> >& lstFileTag, _In_ const smart_ptr<FILETAG_PAIR>& spPairTagVlaue)
{
    if (NULL == spPairTagVlaue.get())
    {
        return false;
    }

    bool bFind = false;
    for (std::list<smart_ptr<FILETAG_PAIR> >::iterator kitr = lstFileTag.begin(); kitr != lstFileTag.end(); ++kitr)
    {
        if (0 == _wcsicmp(((*kitr)->strTagName).c_str(), (spPairTagVlaue->strTagName).c_str()))
        {
            (*kitr)->strTagValue = spPairTagVlaue->strTagValue;
            bFind = true;
            break;
        }
    }
    if (!bFind)
    {
        lstFileTag.push_back(spPairTagVlaue);
    }
    return true;
}

std::wstring GetTagValueFromTagPair(_In_ const std::list<smart_ptr<FILETAG_PAIR> >& lstFileTag, _In_ const std::wstring& kwstrTagName)
{
    if (kwstrTagName.empty())
    {
        return L"";
    }

    std::wstring wstrFileTime = L"";
    for (std::list<smart_ptr<FILETAG_PAIR> >::const_iterator kitr = lstFileTag.begin(); kitr != lstFileTag.end(); ++kitr)
    {
        if (0 == _wcsicmp(((*kitr)->strTagName).c_str(), kwstrTagName.c_str()))
        {
            wstrFileTime = (*kitr)->strTagValue;
            break;
        }
    }
    return wstrFileTime;
}
