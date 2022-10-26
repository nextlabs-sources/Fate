#include "stdafx.h"
#include <tlhelp32.h>
#include "nlconfig.hpp"

#include "utils.h"
#include "dllmain.h"


/*
*	Debug level:
*		Debug level 1: output more logs
*		Debug level 2: pop up message box Allow/Deny as the result of the evaluation
*/

BOOL g_bDebugMode_L1 = /*TRUE*/ FALSE;  // used to control log, new no use it.
BOOL g_bDebugMode_L2 = FALSE;	 // used to control policy

std::wstring g_cacheFilePath = L""; // used to cache file path when insert file in word by Insert->Object->Text From File from sharePoint.

bool g_bFourceLogDefault = true;
BOOL g_bSavePressed = FALSE;

// these constant used to record the proportion range that OK button account for the entire dialog.
// for ppt  
const DOUBLE  PptOkDlgRtioXMin = 0.70;
const DOUBLE  PptOkDlgRtioXMax = 0.84;
const DOUBLE  PptOkDlgRtioYMin = 0.88;
const DOUBLE  PptOkDlgRtioYMax = 0.96;
// for excel
const DOUBLE  ExcelOkDlgRtioXMin = 0.64;
const DOUBLE  ExcelOkDlgRtioXMax = 0.78;
const DOUBLE  ExcelOkDlgRtioYMin = 0.84;
const DOUBLE  ExcelOkDlgRtioYMax = 0.90;


////////////////////////////CELog2//////////////////////////////////////////////
// for CELog2 we should define: CELOG_CUR_FILE
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_UTILS)
//////////////////////////////////////////////////////////////////////////

////////////////////////////for debug function life//////////////////////////////////////////////
CFunTraceLog::CFunTraceLog( _In_ const wstring& wstrFuncName, _In_ const unsigned long& unlLine ) : m_wstrFuncName(wstrFuncName), m_unlStartLine(unlLine)
{
	wstring wstrTemp( L"->>-->>-->>-->>-->>-->>------->>>>>>>>>>>>>IN::" );
	NLPrintLogW( g_bFourceLogDefault, L"\n%s, %s,start line:[%u]\n", wstrTemp.c_str(), m_wstrFuncName.c_str(), m_unlStartLine );
	NLCELOG_LOG( L"\n%s, %s,start line:[%u]\n", wstrTemp.c_str(), m_wstrFuncName.c_str(), m_unlStartLine );
}

CFunTraceLog::~CFunTraceLog()
{
	wstring wstrTemp( L"-<<--<<--<<--<<--<<--<<-------<<<<<<<<<<<<<OUT::" );
	NLPrintLogW( g_bFourceLogDefault, L"\n%s,%s,start line:[%u]\n", wstrTemp.c_str(), m_wstrFuncName.c_str(), m_unlStartLine );
	NLCELOG_LOG( L"\n%s, %s,start line:[%u]\n", wstrTemp.c_str(), m_wstrFuncName.c_str(), m_unlStartLine );
}
///////////////////////////////////end///////////////////////////////////////

void NLPrintLogW( _In_ const bool bFourceLog, _In_ const wchar_t* _Fmt, ... )
{
	// Here we should consider the debug level

	if ( !bFourceLog )
	{
		if ( !g_bDebugMode_L1 )
		{
			//No logs the debug level is false
			return ;
		}
	}
	
	// To ensure that this function is safe, even if an exception occurs.
	__try
	{
		va_list  args;
		int      len = 0;
		wchar_t* pwchBuffer = NULL;

		// retrieve the variable arguments
		va_start( args, _Fmt );

		len = _vscwprintf_l( _Fmt, 0, args ) + 1; // _vscprintf doesn't count, terminating '\0' 

		pwchBuffer = (wchar_t*)malloc( len * sizeof(wchar_t) );

		if( NULL != pwchBuffer )
		{
			__try
			{
				vswprintf_s( pwchBuffer, len, _Fmt, args );
				::OutputDebugStringW( pwchBuffer );
				NLCELOG_LOG( L"\n%s \n", pwchBuffer );
			}
			__finally
			{
				free( pwchBuffer ); // To make sure that this code should be execute,
			}
		}
		va_end(args);
	}
	__except ( EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) // To ensure that the global expansion 
	{
		// nothing to do, maybe a serious error happened
		::OutputDebugStringW( L"PrintlogW function, an exception happened \n" );
		NLCELOG_LOG( L"PrintlogW function, an exception happened \n" );
	}
}

void NLPrintTagPairW( _In_ const vector<pair<wstring, wstring>>& vecTagPair, _In_opt_ const wchar_t* pwchBeginFlag, _In_opt_ const wchar_t* pwchEndFlag )
{
	NLPrintLogW( g_bFourceLogDefault, L"[%s] [%s] [%s]\n", g_pwchToolSepratorFlag, NULL == pwchBeginFlag ?  L"Begin" : pwchBeginFlag, g_pwchToolSepratorFlag );
	for ( vector<pair<wstring, wstring>>::const_iterator cItr = vecTagPair.begin(); cItr != vecTagPair.end(); cItr++ )
	{
		NLPrintLogW( g_bFourceLogDefault, L"tag name:[%s], tag value:[%s]\n", cItr->first.c_str(), cItr->second.c_str() );
	}
	NLPrintLogW( g_bFourceLogDefault, L"[%s] [%s] [%s]\n", g_pwchToolSepratorFlag, NULL == pwchEndFlag ?  L"End" : pwchEndFlag, g_pwchToolSepratorFlag );
}

void NLCELogPrintTagPairW( _In_ const vector<pair<wstring, wstring>>& vecTagPair, _In_opt_ const wchar_t* pwchBeginFlag, _In_opt_ const wchar_t* pwchEndFlag )
{
	NLCELOG_LOG( L"[%s] [%s] [%s]\n", g_pwchToolSepratorFlag, NULL == pwchBeginFlag ?  L"Begin" : pwchBeginFlag, g_pwchToolSepratorFlag );
	for ( vector<pair<wstring, wstring>>::const_iterator cItr = vecTagPair.begin(); cItr != vecTagPair.end(); cItr++ )
	{
		NLCELOG_LOG( L"tag name:[%s], tag value:[%s]\n", cItr->first.c_str(), cItr->second.c_str() );
	}
	NLCELOG_LOG( L"[%s] [%s] [%s]\n", g_pwchToolSepratorFlag, NULL == pwchEndFlag ?  L"End" : pwchEndFlag, g_pwchToolSepratorFlag );
}

bool g_bDebugMessageBox = false;

int NLMessageBox(	_In_opt_ const wchar_t* pwchText, _In_opt_ const wchar_t* pwchCaption, _In_ UINT uType )
{
	int nRet = IDCANCEL;
	if ( g_bDebugMessageBox )
	{
		nRet = ::MessageBoxW( GetActiveWindow(), pwchText, pwchCaption, uType );
	}
	return nRet;
}



//others

typedef void (WINAPI *RtlGetVersion_FUNC)(OSVERSIONINFOEXW* );

BOOL GetOSInfo(OSVERSIONINFOEX* os)
{
    RtlGetVersion_FUNC func;
#ifdef UNICODE
    OSVERSIONINFOEXW* osw=os;
#else
    OSVERSIONINFOEXW o;
    OSVERSIONINFOEXW* osw=&o;
#endif

    HMODULE hMod= LoadLibrary(L"ntdll.dll");
    if(hMod)
    {
        func=(RtlGetVersion_FUNC)GetProcAddress(hMod,"RtlGetVersion");
        if(func==0)
        {
            FreeLibrary(hMod);
            return FALSE;
        }
        ZeroMemory(osw,sizeof(*osw));
        osw->dwOSVersionInfoSize=sizeof(*osw);
        func(osw);
#ifndef	UNICODE
        os->dwBuildNumber=osw->dwBuildNumber;
        os->dwMajorVersion=osw->dwMajorVersion;
        os->dwMinorVersion=osw->dwMinorVersion;
        os->dwPlatformId=osw->dwPlatformId;
        os->dwOSVersionInfoSize=sizeof(*os);
        DWORD sz=sizeof(os->szCSDVersion);
        WCHAR* src=osw->szCSDVersion;
        unsigned char* dtc=(unsigned char*)os->szCSDVersion;
        while(*src)
            *dtc++ =(unsigned char) *src++;
        *dtc='\0';
#endif

    }
    else
    {
        return FALSE;
    }

    FreeLibrary(hMod);
    return TRUE;
}

// versions: 5.1 Xp
BOOL IsXp(void)
{
	static DWORD dwXPMajor = 0;
	static DWORD dwXPMinor = 0;

	if ((5 == dwXPMajor) && (1 == dwXPMinor))
	{
		return TRUE;
	}
	else if ((0 != dwXPMajor) || (0 != dwXPMinor))
	{
		return FALSE;
	}

    OSVERSIONINFOEX os;
	if (GetOSInfo(&os) && (5 == (dwXPMajor = os.dwMajorVersion)) && (1 == (dwXPMinor = os.dwMinorVersion)))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL IsWin7(void /*include Vista*/)
{
	static DWORD dwMajor = 0;
	if (dwMajor == 6)	return TRUE;
	else if (dwMajor != 0)	return FALSE;

    OSVERSIONINFOEX os;
	if (GetOSInfo(&os) && (dwMajor = os.dwMajorVersion) == 6)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL IsWin10()
{
	static DWORD dwMajor = 0;
	if (dwMajor == 10)	return TRUE;
	else if (dwMajor != 0)	return FALSE;

    OSVERSIONINFOEX os;
	if (GetOSInfo(&os) && (dwMajor = os.dwMajorVersion) == 10)
	{
		return TRUE;
	}
	return FALSE;
}


wstring GetCommonComponentsDir()
{
	wchar_t wszDir[MAX_PATH + 1] = { 0 };
	if (NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", wszDir, MAX_PATH))
	{
#ifdef _M_IX86
		wcscat_s(wszDir, MAX_PATH, L"bin32\\");
#else
		wcscat_s(wszDir, MAX_PATH, L"bin64\\");
#endif
	}

	return wszDir;
}

bool IsLocalDriver(const wstring& wstrPath)
{
	// 1. check parameter
	if (1 != wstrPath.find(L":"))
	{
		return false;		// the path should be like C: or D:
	}

	wstring strSub = wstrPath.substr(0, 2) + L"\\";
	return DRIVE_FIXED == GetDriveTypeW(strSub.c_str());
}

void ConvertURLCharacterW(_Inout_ wstring& strUrl)
{
    // sanity check ,only works on http path or other net path (such as \\rms-sp2013\).
    if(!NLIsHttpPath(strUrl) && !NLIsFilePath(strUrl))
        return;

    if ( NLIsFilePath(strUrl))
    {
         boost::algorithm::ireplace_first(strUrl, L"file://", L"\\\\");
         boost::algorithm::ireplace_all(strUrl, L"/", L"\\");
    }
    
	/*
	*@Add for change '%5c%5b'->'\['->'[',  '%5c%5d'->'\]'->']' for bug 9339
	*/
	boost::replace_all(strUrl, L"%5c%5b", L"[");//'%5c%5b' -> '['
	boost::replace_all(strUrl, L"%5c%5d", L"]");	//'%5c%5d' -> ']'

	boost::replace_all(strUrl, L"%24", L"$");
	boost::replace_all(strUrl, L"%5e", L"^");
	boost::replace_all(strUrl, L"%26", L"&");
	boost::replace_all(strUrl, L"%5b", L"[");
	boost::replace_all(strUrl, L"%5d", L"]");
	boost::replace_all(strUrl, L"%20", L" ");	//'%20' -> ' '
	boost::replace_all(strUrl, L"%2e", L".");	// '%2e  -> '.'
	boost::replace_all(strUrl, L"%2f", L"/");	// '%2f' -> '/'
	boost::replace_all(strUrl, L"%5f", L"_");	// '%5f' -> '_'
	boost::replace_all(strUrl, L"+", L" ");		// '+'   -> ' '
	boost::replace_all(strUrl, L"%2d", L"-");	//'%2d' -> '-'
	boost::replace_all(strUrl, L"%3a", L":");	//'%2d' -> '-'
	boost::replace_all(strUrl, L"%28", L"(");	//'%28'-> '('
	boost::replace_all(strUrl, L"%29", L")");	//'%29'-> ')'
	boost::replace_all(strUrl, L"%2520", L" ");//'%2520'->' '
}


wstring GetSuffixFromFileName(_In_ const wstring& wstrFileName)
{
	wstring wstrRet = L"";
	wstring::size_type nPos = wstrFileName.rfind(L".");
	if (nPos != wstring::npos)
		wstrRet = wstrFileName.substr(nPos + 1, wstrFileName.size() - nPos - 1);

	return wstrRet;
}

wstring GetFilePath(const wstring& wstrFilePath)
{
	wstring wstrRet = L"";
	wstring::size_type nPos = wstrFilePath.rfind(L"\\");
	if (nPos != wstring::npos)
		wstrRet = wstrFilePath.substr(0, nPos + 1);
	return wstrRet;
}

wstring GetFileName(const wstring& wstrFilePath)
{
	wstring wstrRet = L"";
	wstring::size_type nPos = wstrFilePath.rfind(L"\\");
	if (nPos != wstring::npos)
		wstrRet = wstrFilePath.substr(nPos + 1, wstrFilePath.length() - 1);
	return wstrRet;
}

wstring GetNetFileNameByFilePath(const wstring& wstrFilePath)
{
	wstring wstrFileName = L"";
	wstring::size_type nPos = wstrFilePath.rfind(L'/');
	if (wstring::npos != nPos)
	{
		wstrFileName = wstrFilePath.substr(nPos + 1);
	}
	return wstrFileName;
}

wstring GetUpperFolder(const wstring& strFilePath)
{
	wstring::size_type nPos = strFilePath.rfind(L"\\");
	if (nPos == strFilePath.length() - 1)
		strFilePath.substr(0, nPos - 1);
	wstring strRet = L"";
	nPos = strFilePath.rfind(L"\\");
	if (nPos != wstring::npos)
		strRet = strFilePath.substr(0, nPos);
	return strRet;
}

bool GetCrackURL(HINTERNET hIns, wstring& strUrlPath)
{
	bool bGet = false;
	DWORD dwSize = 2046;
	byte szbuf[2048] = { 0 };
	if (!WinHttpQueryOption(hIns, WINHTTP_OPTION_URL, szbuf, &dwSize))
	{
		DWORD dwError = GetLastError();
		if (dwError & ERROR_WINHTTP_INCORRECT_HANDLE_STATE)
		{
			OutputDebugStringW(L"ERROR_WINHTTP_INCORRECT_HANDLE_STATE\n");
		}
		else if (dwError & ERROR_WINHTTP_INCORRECT_HANDLE_TYPE)
		{
			OutputDebugStringW(L"ERROR_WINHTTP_INCORRECT_HANDLE_TYPE\n");
		}
		else if (dwError & ERROR_WINHTTP_INTERNAL_ERROR)
		{
			OutputDebugStringW(L"ERROR_WINHTTP_INTERNAL_ERROR\n");
		}
		else if (dwError & ERROR_WINHTTP_INVALID_OPTION)
		{
			OutputDebugStringW(L"ERROR_WINHTTP_INVALID_OPTION\n");
		}
		else if (dwError & ERROR_NOT_ENOUGH_MEMORY)
		{
			OutputDebugStringW(L"ERROR_NOT_ENOUGH_MEMORY\n");
		}
	}
	else
	{
		strUrlPath = (wchar_t*)szbuf;
		bGet = true;
	}
	return bGet;
}

wstring GetUserAppDataPath()
{
    TCHAR szPath[MAX_PATH] = {0};
    ::SHGetSpecialFolderPathW(NULL, szPath, CSIDL_APPDATA, FALSE);  
    std::wstring wstrTmpPath(szPath);
    size_t nPos = wstrTmpPath.rfind('\\');
    if (nPos != std::wstring::npos)
    {
        wstrTmpPath = wstrTmpPath.substr(0,nPos);  
    }
    return wstrTmpPath;
}

DWORD GetParentProcessID(DWORD dwProcessID)
{
	DWORD dwParentID = 0;
	HANDLE hProcessSnap = NULL;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return(FALSE);
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);    // Must clean up the
		//   snapshot object!
		return(FALSE);
	}

	do
	{
		if (dwProcessID == pe32.th32ProcessID)
		{
			dwParentID = pe32.th32ParentProcessID;
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return dwParentID;
}

void NLCloseHandle(_Inout_ HANDLE& hHandle)
{
	if (NULL != hHandle)
	{
		CloseHandle(hHandle);
		hHandle = NULL;
	}
}

BOOL IsInOKRect(HWND hwnd, POINT point)
{
    RECT rect;
    ::GetWindowRect(hwnd,&rect);
    LONG DlgWidth = rect.right - rect.left;
    LONG DlgHeight = rect.bottom - rect.top;

    DOUBLE XRatio = (point.x) / (DOUBLE)DlgWidth;
    DOUBLE YRatio = (point.y) / (DOUBLE)DlgHeight;

    if ( pep::isPPtApp() && XRatio > PptOkDlgRtioXMin && XRatio < PptOkDlgRtioXMax &&
        YRatio > PptOkDlgRtioYMin && YRatio < PptOkDlgRtioYMax )
    {
        return TRUE;
    }
    else if ( pep::isExcelApp() && XRatio > ExcelOkDlgRtioXMin && XRatio < ExcelOkDlgRtioXMax &&
             YRatio > ExcelOkDlgRtioYMin && YRatio < ExcelOkDlgRtioYMax )
    {
         return TRUE;
    }
    
    
    return FALSE;
}

BOOL isContentData(IDataObject *pDataObject)
{
	CComPtr<IEnumFORMATETC> pIEnumFORMATETC = NULL;

	//Enumerate clipboard data
	HRESULT hr = pDataObject->EnumFormatEtc(DATADIR_GET, &pIEnumFORMATETC);
	BOOL bContent = FALSE;
	if(FAILED(hr) || NULL == pIEnumFORMATETC) 
	{
		return bContent;
	}

	FORMATETC etc;
	while (1) 
	{
		memset(&etc, 0, sizeof(etc));
		if(S_FALSE == pIEnumFORMATETC->Next(1, &etc, NULL))
		{
			break;
		}

		// Standard
		if(CF_TEXT == etc.cfFormat) 
		{
			bContent = TRUE;
		} 
		else if(CF_BITMAP == etc.cfFormat) 
		{
			bContent = TRUE;
		} 
		else if(CF_OEMTEXT == etc.cfFormat) 
		{
			bContent = TRUE;
		} 
		else if(CF_UNICODETEXT == etc.cfFormat) 
		{
			bContent = TRUE;
		} 
		else 
		{
			WCHAR szFormat[256] = {0};
			GetClipboardFormatNameW(etc.cfFormat, szFormat, 256);
			if(wcscmp(szFormat, L"FileGroupDescriptor" ) == 0) 
			{
				//this email attachment
				bContent = FALSE;
				break;
			}
		} 
	}

	pIEnumFORMATETC.Release();

	return bContent;
}

BOOL GetOleContentClipboardDataSource(IDataObject *pDataObject, const std::wstring& clipboardInfo, __out std::wstring& srcFilePath)
{
	HRESULT hr;
	STGMEDIUM medium;
	OBJECTDESCRIPTOR  *od = NULL;
	BOOL bRet = FALSE;

	if(pDataObject == NULL) 
	{
		return FALSE;
	}
	FORMATETC fe = {(CLIPFORMAT)RegisterClipboardFormat(L"Object Descriptor"), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	hr = pDataObject->QueryGetData(&fe);
	if(SUCCEEDED(hr)) 
	{
		hr = pDataObject->GetData(&fe, &medium);
		if(SUCCEEDED(hr)) 
		{
			od = (OBJECTDESCRIPTOR*)GlobalLock(medium.hGlobal);
			if(od)
			{
				srcFilePath = ((WCHAR*)((char*)od + od->dwSrcOfCopy));
				if(std::wstring::npos != srcFilePath.find(L"Temporary Internet Files"))
				{
					//This is the temporary name for a file downloaded from sharepoint
					srcFilePath = L"";
				} 
				else if(std::wstring::npos != srcFilePath.find(L"Microsoft Office Word Document"))
				{
					if(clipboardInfo.length())
					{
						srcFilePath = clipboardInfo;
					}
					else
					{
						srcFilePath = L"";
					}
				}
				else 
				{
					size_t len = srcFilePath.size();
					if(len > 2)
					{
						if(srcFilePath[len-1] == L'1' && srcFilePath[len-2] == L'!')
						{
							//There is strange ending with !1; remove them
							srcFilePath.erase(srcFilePath.begin() + len - 2, srcFilePath.end());
						}
					}

					if (std::wstring::npos != srcFilePath.find(L"\\") || std::wstring::npos != srcFilePath.find(L"/"))
					{
						bRet = TRUE;	
						//for non-ole application to get the source 
						//clipboardInfo = srcFilePath;
					}
				}
				GlobalUnlock(medium.hGlobal);
			}
			ReleaseStgMedium(&medium);
		} 
	}
	return bRet;
}

std::wstring GetCurProcessFolderPath()
{
    static std::wstring wstrCurProcessFolderPath = L"";

    if (wstrCurProcessFolderPath.empty())
    {
        wchar_t wszModule[1024] = { 0 };
        ::GetModuleFileNameW(NULL, wszModule, 1024);
        wchar_t* pLastBackslash = wcsrchr(wszModule, L'\\');
        if (pLastBackslash != NULL)
        {
            *pLastBackslash = L'\0';
        }
        wstrCurProcessFolderPath = wszModule;
    }
    return wstrCurProcessFolderPath;
}

