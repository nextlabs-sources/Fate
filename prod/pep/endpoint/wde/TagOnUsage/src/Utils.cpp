#include "stdafx.h"
#include "Utils.h"
#include <time.h>
#include "resource.h"
#include <Windows.h>
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"
using namespace std;

#define NAME_SHAREDMEMORY			L"nextlabs_pdp_addtags"
#define MAX_CREATE_IDENTIFIER		10000

HANDLE g_hSharedMemory = NULL;
CELog g_log;

wchar_t* g_szOfficeExt[] = {L"doc", L"xls", L"ppt"};

const wchar_t* g_szNeedProcessDlgFileTypes[] = {L".docx", L".docm", L".dotx", L".dotm", L".xlsm", L".xlsb", L".xlsx", L".xltm", L".xltx", L".xlam", L".potx", L".ppsm",
											L".ppsx", L".pptm", L".pptx", L".potm", L".ppam", L".pdf"};

const wchar_t* g_szCATIAFileTypes[] = {L".catpart", L".catalog", L".catanalysis", L".catdrawing", L".catfct", L".catknowkedge", L"catmaterial", L".catprocess", L".catproduct", L".catraster", L".catresource",
										L".catshape", L".catswl", L".catsystem", L".model", L".session", L".library", L".igs", L".wrl", L".stp", L".step", L".cgm", L".gl", L".gl2", L".hpgl", L".3dmap", L".3dxml",
										L".act", L".asm", L".bdf", L".brd",L".cdd", L".dwg", L".dxf", L".ql", L".idf", L".ig2", L".pdb", L".ps", L".stbom", L".svg", L".tdg", L".dws", L".dwt"};

time_t GetMilliSecTime(PSYSTEMTIME pSysTime = NULL ) 
{
	time_t rtTime = 0;
	tm     rtTM;
	BOOL bFlag = FALSE ;
	if( pSysTime == NULL )
	{
		pSysTime = new SYSTEMTIME() ;
		GetSystemTime( pSysTime ) ;
		bFlag = TRUE ;
	}
	rtTM.tm_year = pSysTime->wYear - 1900;
	rtTM.tm_mon  = pSysTime->wMonth - 1;
	rtTM.tm_mday = pSysTime->wDay;
	rtTM.tm_hour = pSysTime->wHour;
	rtTM.tm_min  = pSysTime->wMinute;
	rtTM.tm_sec  = pSysTime->wSecond;
	rtTM.tm_wday = pSysTime->wDayOfWeek;
	rtTM.tm_isdst = -1;     // Let CRT Lib compute whether DST is in effect,
	// assuming US rules for DST.
	rtTime = mktime(&rtTM); // get the second from Jan. 1, 1970

	if (rtTime == (time_t) -1)
	{
		if (pSysTime->wYear <= 1970)
		{
			// Underflow.  Return the lowest number possible.
			rtTime = (time_t) 0;
		}
		else
		{
			// Overflow.  Return the highest number possible.
			rtTime = (time_t) _I64_MAX;
		}
	}
	else
	{
		rtTime*= 1000;          // get millisecond
	}
	if( bFlag == TRUE )
	{
		delete pSysTime ;
	}
	return rtTime;
};

BOOL getFileLastModifiedTime( const wchar_t *pszFileName, LPSYSTEMTIME lpLastModifiedTime /*UTC time*/) 
{
	if( !pszFileName )
	{
		return FALSE;
	}
	HANDLE hFile = INVALID_HANDLE_VALUE;

	BOOL bRet = FALSE;
	hFile = CreateFileW(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(INVALID_HANDLE_VALUE != hFile)
	{
		FILETIME ftLastModify;  memset(&ftLastModify, 0, sizeof(FILETIME));
		if(GetFileTime(hFile, NULL, NULL, &ftLastModify))
		{

			if(FileTimeToSystemTime(&ftLastModify, lpLastModifiedTime))
			{
				bRet = TRUE;

			}
		}
		CloseHandle(hFile);
	}
	else
	{
		DWORD dwError = GetLastError();
		g_log.Log(CELOG_DEBUG, L"getFileLastModifiedTime() failed, %d\n", dwError);
	}

	return bRet ;
};

BOOL IsCreateBehaviorForExplorer(LPCWSTR lpszFilePath)
{
	if(!lpszFilePath)
	{
		return FALSE;
	}

	SYSTEMTIME lastModified;
	memset(&lastModified, 0, sizeof(SYSTEMTIME));
	int i = 0;
	for (; i < 10; i ++)
	{
		if(getFileLastModifiedTime(lpszFilePath, &lastModified))
			break;
		Sleep(1000);
	}
	if(i >= 10)
		return FALSE;

	SYSTEMTIME curTime;
	GetSystemTime(&curTime);

	wchar_t szLastModifiedTime[200] = {0};
	wchar_t szCurTime[200] = {0};
	_snwprintf_s(szLastModifiedTime, 200, _TRUNCATE, L"%d-%d-%d, %d:%d:%d:%d", lastModified.wYear, lastModified.wMonth, lastModified.wDay, lastModified.wHour, lastModified.wMinute, lastModified.wSecond, lastModified.wMilliseconds);
	_snwprintf_s(szCurTime, 200, _TRUNCATE, L"%d-%d-%d, %d:%d:%d:%d", curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond, curTime.wMilliseconds);

	time_t nRet = GetMilliSecTime(&curTime) - GetMilliSecTime(&lastModified);

	BOOL bRet = nRet < MAX_CREATE_IDENTIFIER? TRUE: FALSE; 
	g_log.Log(CELOG_DEBUG, L"WdeAddTags, file: %s, local machine UTC time: %s, last modified UTC time: %s, the difference time between local system and last modify time: %d, the condition value: %d. return value: %d\n",
							lpszFilePath, szCurTime, szLastModifiedTime, (int)nRet, MAX_CREATE_IDENTIFIER, bRet);


	return bRet;
}

BOOL IsWritable(LPCWSTR lpszFilePath)
{
	if(!lpszFilePath)
	{
		return FALSE;
	}

	HANDLE h = CreateFileW(lpszFilePath,     
						/*GENERIC_ALL*/GENERIC_READ | GENERIC_WRITE,   //GENERIC_ALL will have a problem on windows 7, it will be denied for non-administrator sometimes.
						0,   
						NULL,   
						OPEN_EXISTING,   
						0,   
						NULL);

	BOOL bRet = ( INVALID_HANDLE_VALUE == h? FALSE: TRUE );

	if(h)
	{
		CloseHandle(h);
	}

	/******************************************************
	I did some tests for WORD/EXCEL. I found there will have 
	a short time that the current file is writable when it is 
	opened. The frequency is about 5%.
	So, I added below code to make sure the current file is 
	writable.
	*******************************************************/
	for(int i = 0; i < 2 && bRet; i++)
	{
		Sleep(200);

		h = CreateFileW(lpszFilePath,     
			GENERIC_READ | GENERIC_WRITE,   
			0,   
			NULL,   
			OPEN_EXISTING,   
			0,   
			NULL);

		bRet = ( INVALID_HANDLE_VALUE == h? FALSE: TRUE );

		if(!bRet)
		{
			g_log.Log(CELOG_DEBUG, L"addTags::The current file is not writable for 2nd check.");
		}

		if(h)
		{
			CloseHandle(h);
		}
	}
	
	return bRet;
}


BOOL CreateSharedMemory()
{
	g_hSharedMemory = CreateFileMapping(
									INVALID_HANDLE_VALUE,    // use paging file
									NULL,                    // default security 
									PAGE_READWRITE,          // read/write access
									0,                       // max. object size 
									MAX_LEN_SHAREDMEMORY,                // buffer size  
									NAME_SHAREDMEMORY);                 // name of mapping object

	if (g_hSharedMemory == NULL) 
	{ 
		return FALSE;
	}
	return TRUE;
}

void CloseSharedMemory()
{
	if(g_hSharedMemory != NULL)
	{
		CloseHandle(g_hSharedMemory);
	}
}

BOOL WriteSharedMemory(LPCWSTR lpszInfo, DWORD dwLen)
{
	if(!lpszInfo || dwLen * sizeof(wchar_t) > MAX_LEN_SHAREDMEMORY)
	{
		return FALSE;
	}

	HANDLE hMapFile;
	LPVOID pBuf;

	hMapFile = OpenFileMapping(
							FILE_MAP_ALL_ACCESS,   // read/write access
							FALSE,                 // do not inherit the name
							NAME_SHAREDMEMORY);               // name of mapping object 

	if (hMapFile == NULL) 
	{ 
		printf("Could not open file mapping object (%d).\n", 
			GetLastError());
		
		return FALSE;
	} 

	pBuf =  MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,                   
		0,                   
		MAX_LEN_SHAREDMEMORY);           

	if (pBuf == NULL) 
	{ 
		return FALSE;
	}

	memcpy((PVOID)pBuf, lpszInfo, dwLen * sizeof(wchar_t));

	UnmapViewOfFile(pBuf);

	CloseHandle(hMapFile);

	return TRUE;
}

BOOL ReadSharedMemory(LPWSTR lpszInfo, DWORD dwLen)
{
	if(!lpszInfo || dwLen * sizeof(wchar_t) > MAX_LEN_SHAREDMEMORY)
	{
		return FALSE;
	}

	HANDLE hMapFile;
	LPVOID pBuf;

	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		NAME_SHAREDMEMORY);               // name of mapping object 

	if (hMapFile == NULL) 
	{ 
		printf("Could not open file mapping object (%d).\n", 
			GetLastError());
		return FALSE;
	} 

	pBuf =  MapViewOfFile(hMapFile, // handle to map object
								FILE_MAP_ALL_ACCESS,  // read/write permission
								0,                    
								0,                    
								MAX_LEN_SHAREDMEMORY);                   

	if (pBuf == NULL) 
	{ 
		printf("Could not map view of file (%d).\n", 
			GetLastError()); 
	
		return FALSE;
	}

	memcpy(lpszInfo, pBuf, dwLen * sizeof(wchar_t));//read the data from Shared Memory

	UnmapViewOfFile(pBuf);

	CloseHandle(hMapFile);


	return TRUE;
}

BOOL ExistsInROT(LPCWSTR pszFilePath )
{
	if(!pszFilePath)
	{
		return FALSE;
	}

	CComPtr<IRunningObjectTable> lpROT = NULL;
	CComPtr<IEnumMoniker> pEnum = NULL;
	CComPtr<IMalloc> pMalloc = NULL;
	CComPtr<IMoniker> pMoniker = NULL;
	CComPtr<IBindCtx> pCtx = NULL;
	BOOL bFind = FALSE;

	std::vector<std::wstring> vecRotFile;

	HRESULT hr = GetRunningObjectTable(0, &lpROT);
	BOOL bInitCom = FALSE;
	if(hr == CO_E_NOTINITIALIZED)
	{
		CoInitialize(NULL);
		bInitCom = TRUE;
		hr = GetRunningObjectTable(0,&lpROT);
	}

	if(FAILED(hr))
	{
		goto _EXIT_GETFILEPATH_FROM_ROT_;
	}

	hr = lpROT->EnumRunning(&pEnum);
	if(FAILED(hr))
	{
		goto _EXIT_GETFILEPATH_FROM_ROT_;
	}


	hr = CoGetMalloc(1, &pMalloc);
	if(FAILED(hr))
	{
		goto _EXIT_GETFILEPATH_FROM_ROT_;
	}

	ULONG lFetch = 0;

	hr = CreateBindCtx(0, &pCtx);
	if(FAILED(hr))
	{
		goto _EXIT_GETFILEPATH_FROM_ROT_;
	}

	LPOLESTR pDisplayName = NULL;

	
	while((hr = pEnum->Next(1, &pMoniker, &lFetch)) == S_OK && !bFind)
	{
		hr = pMoniker->GetDisplayName(pCtx, NULL, &pDisplayName);
		if(SUCCEEDED(hr))
		{
			std::wstring strDisplayName(pDisplayName);
			pMalloc->Free(pDisplayName);
			
			wchar_t buffer[1000] = {0};
			_snwprintf_s(buffer, 1000, _TRUNCATE, L"ROT file: %s", strDisplayName.c_str());
			g_log.Log(CELOG_DEBUG, buffer);

			if(_wcsicmp(strDisplayName.c_str(), pszFilePath) == 0)
			{
				bFind = TRUE;
				break;
			}
		}	
	}
_EXIT_GETFILEPATH_FROM_ROT_:

	return bFind;
}

BOOL IsOfficeFile(LPCWSTR pszFilePath)
{
	if(!pszFilePath)
		return FALSE;

	WCHAR* p = (WCHAR*)wcsrchr(pszFilePath, '.');
	if(p)
	{
		p = p + 1;
		
		std::wstring strExtension(p);

		std::transform(strExtension.begin(), strExtension.end(), strExtension.begin(), towlower);

		for(int i = 0; i < _countof(g_szOfficeExt); i++)
		{
			if(_wcsicmp(strExtension.c_str(), g_szOfficeExt[i]) == 0)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL NeedCheckWnd(LPCWSTR pszAppName, LPCWSTR pszFileName)
{
	if(!pszAppName || !pszFileName)
		return FALSE;

// 	const wchar_t* ext = wcsrchr(pszFileName, '.');
// 	if(ext && (_wcsicmp(ext, L".pps") == 0 || _wcsicmp(ext, L".ppsx") == 0 ))
// 	{
// 		if(FindWindow(NULL, L"Slide Show") != NULL)
// 			return FALSE;
// 	}

	if(		_wcsicmp(pszAppName, L"winword.exe") == 0 
		||	_wcsicmp(pszAppName, L"excel.exe") == 0 
		||	_wcsicmp(pszAppName, L"powerpnt.exe") == 0 
		||  _wcsicmp(pszAppName, L"cnext.exe") == 0 || _wcsicmp(pszAppName, L"acrobat.exe") == 0 || _wcsicmp(pszAppName, L"acrord32.exe") == 0
		||  _wcsicmp(pszAppName, L"acad.exe") == 0
		||  (_wcsicmp(pszAppName, L"explorer.exe") == 0 && IsCATIAFileTypes(pszFileName)) )
	{
		return TRUE;
	}

	return FALSE;
}

BOOL GetRelatedWndTxt(LPCWSTR pszFileName, LPCWSTR pszAppName, std::vector<std::wstring>& vWndTitle, std::vector<std::wstring>& vAppTxt)
{
	if(!pszFileName || !pszAppName)
		return FALSE;

	std::wstring strFileNameWithoutExt;
	if(wcsrchr(pszFileName, '.') != NULL )
	{
		strFileNameWithoutExt = pszFileName;
		strFileNameWithoutExt = strFileNameWithoutExt.substr(0, strFileNameWithoutExt.rfind('.'));
	}

	const wchar_t* ext = wcsrchr(pszFileName, '.');

	if(_wcsicmp(pszAppName, L"winword.exe") == 0)
	{	
		std::wstring strAppTxt(L"microsoft WORD");
		vAppTxt.push_back(strAppTxt);
		vWndTitle.push_back(std::wstring(pszFileName) + L" - " + strAppTxt);
		if(!strFileNameWithoutExt.empty())
		{
			vWndTitle.push_back(strFileNameWithoutExt + L" - " + strAppTxt);
		}
		if(ext && _wcsicmp(ext, L".pdf") == 0)
		{
			vWndTitle.push_back(std::wstring(pszFileName) + L" - Adobe Acrobat Pro");
			vWndTitle.push_back(std::wstring(ADOBE_ACROBAT_STANDARD) + std::wstring(L"[") + std::wstring(pszFileName) + std::wstring(L"]"));
		}
	}
	else if(_wcsicmp(pszAppName, L"excel.exe") == 0)
	{
		std::wstring strAppTxt = L"microsoft excel";
		vAppTxt.push_back(strAppTxt);
		vWndTitle.push_back(strAppTxt + L" - " + std::wstring(pszFileName));
	//	vWndTitle.push_back(strAppTxt + L" - " + std::wstring(pszFileName) + L" [shared]");
	//	vWndTitle.push_back(strAppTxt + L" - " + std::wstring(pszFileName) + L"  [shared]");
		vWndTitle.push_back(std::wstring(pszFileName) + L" - " + strAppTxt);
	//	vWndTitle.push_back(std::wstring(pszFileName) + L" - " + strAppTxt + L" [shared]");
	//	vWndTitle.push_back(std::wstring(pszFileName) + L" - " + strAppTxt + L"  [shared]");
		if(!strFileNameWithoutExt.empty())
		{
			vWndTitle.push_back(strFileNameWithoutExt + L" - " + strAppTxt);
			vWndTitle.push_back(strAppTxt + L" - " + strFileNameWithoutExt);
		}

		if(ext && _wcsicmp(ext, L".pdf") == 0)
		{
			vWndTitle.push_back(std::wstring(pszFileName) + L" - Adobe Acrobat Pro");
			vWndTitle.push_back(std::wstring(ADOBE_ACROBAT_STANDARD) + std::wstring(L"[") + std::wstring(pszFileName) + std::wstring(L"]"));
		}
	}
	else if(_wcsicmp(pszAppName, L"powerpnt.exe") == 0)
	{
		std::wstring strAppTxt = L"microsoft powerpoint";
		vAppTxt.push_back(strAppTxt);
		vWndTitle.push_back(strAppTxt + L" - [" + std::wstring(pszFileName) + L"]");
		vWndTitle.push_back(std::wstring(pszFileName) + L" - " + strAppTxt);
		vWndTitle.push_back(L"Powerpoint Slide Show - [" + std::wstring(pszFileName) + L"]");
		vWndTitle.push_back(std::wstring(pszFileName) + L" [compatibility mode] - " + strAppTxt);
		if(!strFileNameWithoutExt.empty())
		{
			vWndTitle.push_back(strAppTxt + L" - [" + strFileNameWithoutExt + L"]");
			vWndTitle.push_back(strFileNameWithoutExt + L" - " + strAppTxt);
			vWndTitle.push_back(L"Powerpoint Slide Show - [" + strFileNameWithoutExt + L"]");
		}

		if(ext && _wcsicmp(ext, L".pdf") == 0)
		{	
			vWndTitle.push_back(std::wstring(pszFileName) + L" - Adobe Acrobat Pro");
			vWndTitle.push_back(std::wstring(ADOBE_ACROBAT_STANDARD) + std::wstring(L"[") + std::wstring(pszFileName) + std::wstring(L"]"));
		}
	}
	else if( _wcsicmp(pszAppName, L"cnext.exe") == 0)
	{
		vAppTxt.push_back(L"CATIA V5");
		vAppTxt.push_back(L"CATIA V4");
		vWndTitle.push_back(std::wstring(L"CATIA V5") + L" - [" + std::wstring(pszFileName) + L"]");
		vWndTitle.push_back(std::wstring(L"CATIA V4") + L" - [" + std::wstring(pszFileName) + L"]");
		if(!strFileNameWithoutExt.empty())
		{
			vWndTitle.push_back(std::wstring(L"CATIA V5") + L" - [" + strFileNameWithoutExt + L"]");
			vWndTitle.push_back(std::wstring(L"CATIA V4") + L" - [" + strFileNameWithoutExt + L"]");
		}
	}
	else if(_wcsicmp(pszAppName, L"acrobat.exe") == 0)
	{
		vAppTxt.push_back(L"Adobe Acrobat");
		vWndTitle.push_back(std::wstring(pszFileName) + L" - Adobe Acrobat Pro");//Acrobat will show the extension name on caption even if we "hide known file types".
		vWndTitle.push_back(std::wstring(ADOBE_ACROBAT_STANDARD) + std::wstring(L"[") + std::wstring(pszFileName) + std::wstring(L"]"));
	}
	else if(_wcsicmp(pszAppName, L"acad.exe") == 0)
	{
		vAppTxt.push_back(L"AutoCad");
	}
	else if(_wcsicmp(pszAppName, L"explorer.exe") == 0)
	{
		vAppTxt.push_back(L"AutoCad");
		vAppTxt.push_back(L"CATIA V5");
		vAppTxt.push_back(L"CATIA V4");
		vWndTitle.push_back(std::wstring(L"CATIA V5") + L" - [" + std::wstring(pszFileName) + L"]");
		vWndTitle.push_back(std::wstring(L"CATIA V4") + L" - [" + std::wstring(pszFileName) + L"]");
		if(!strFileNameWithoutExt.empty())
		{
			vWndTitle.push_back(std::wstring(L"CATIA V5") + L" - [" + strFileNameWithoutExt + L"]");
			vWndTitle.push_back(std::wstring(L"CATIA V4") + L" - [" + strFileNameWithoutExt + L"]");
		}
	}
	else if (_wcsicmp(pszAppName, L"acrord32.exe") == 0)
	{
		vAppTxt.push_back(L"Adobe Reader");
		vWndTitle.push_back(std::wstring(pszFileName) + L" - Adobe Reader");//Acrobat will show the extension name on caption even if we "hide known file types".
	}

	BOOL bRet = FALSE;
	if(vWndTitle.size() > 0 && vWndTitle.size() > 0)
		bRet = TRUE;

	return bRet;
}

BOOL GetOpenSaveAsWnd()
{
	if(FindWindow(NULL, L"Open") || FindWindow(NULL, L"Save as") || FindWindow(NULL, L"File selection"))
	{
		g_log.Log(CELOG_DEBUG, L"There is a window which caption is \"open/save as\"");
		return TRUE;
	}
	return FALSE;
}

BOOL CanTag(LPCWSTR pszFileName, DWORD& dwErrorID)
{
	if(!pszFileName)
		return FALSE;

	DWORD dwRet = ::GetFileAttributesW(pszFileName);

	if(0xFFFFFFFF == dwRet)
	{
		dwErrorID = IDS_NO_FILE;
		return FALSE;
	}
	else if((FILE_ATTRIBUTE_READONLY & dwRet) == FILE_ATTRIBUTE_READONLY)
	{//readonly
		dwErrorID = IDS_READONLY;
		return FALSE;
	}
	else
	{
		HANDLE hHandle = CreateFileW(pszFileName,     
			GENERIC_READ | GENERIC_WRITE,   
			0,   
			NULL,   
			OPEN_EXISTING,   
			0,   
			NULL);

		if(INVALID_HANDLE_VALUE == hHandle)
		{
			dwErrorID = IDS_NOT_WRITABLE;
			return FALSE;
		}
		CloseHandle(hHandle);
	}
	return TRUE;
}

void PopupErrorDlg(HWND hParentWnd, HINSTANCE hInstance, DWORD dwErrorID)
{
	wchar_t szError[201] = {0};
	LoadStringW(hInstance, dwErrorID, szError, 200);
	wchar_t szCaption[51] = {0};
	LoadStringW(hInstance, IDS_ERROR_CAPTION, szCaption, 50);

	MessageBox(hParentWnd, szError, szCaption, MB_OK|MB_ICONERROR);
}

std::wstring GetProbableFileName(LPCWSTR lpszFileName)
{
	if(!lpszFileName)
		return L"";

	std::wstring strFileName(lpszFileName);
	std::wstring strProbableName = strFileName;

	const wchar_t* ext = wcsrchr(lpszFileName, '.');
	if(_wcsicmp(ext, L".xlt") == 0 || _wcsicmp(ext, L".xltx") == 0 || _wcsicmp(ext, L".xltm") == 0)
	{
		strProbableName = strFileName.substr(0, strFileName.rfind(L"."));
	}
	else if(_wcsicmp(ext, L".dot") == 0 || _wcsicmp(ext, L".dotx") == 0 || _wcsicmp(ext, L".dotm") == 0)
	{
		strProbableName = L"Document";
	}
	else if(_wcsicmp(ext, L".pot") == 0 || _wcsicmp(ext, L".potx") == 0 || _wcsicmp(ext, L".potm") == 0 || _wcsicmp(ext, L".ppsm") == 0 || _wcsicmp(ext, L".pps") == 0 || _wcsicmp(ext, L".ppsx") == 0)
	{
		strProbableName = L"Presentation";
	}

	return strProbableName;
}

BOOL IsTemplateFile(LPCWSTR lpszFileName)
{
	if(!lpszFileName)
		return FALSE;

	const wchar_t* ext = wcsrchr(lpszFileName, '.');
	if(ext && 
			(_wcsicmp(ext, L".xlt") == 0 || _wcsicmp(ext, L".xltx") == 0 || _wcsicmp(ext, L".xltm") == 0 
			|| _wcsicmp(ext, L".dot") == 0 || _wcsicmp(ext, L".dotx") == 0 || _wcsicmp(ext, L".dotm") == 0
			|| _wcsicmp(ext, L".pot") == 0 || _wcsicmp(ext, L".potx") == 0 || _wcsicmp(ext, L".potm") == 0 || _wcsicmp(ext, L".ppsm") == 0 || _wcsicmp(ext, L".pps") == 0 || _wcsicmp(ext, L".ppsx") == 0)
	  )
	{
		return TRUE;
	}

	return FALSE;
}

BOOL NeedProcessDlg(LPCWSTR lpszFilePath)
{
	if(!lpszFilePath)
		return FALSE;

	const wchar_t* ext = wcsrchr(lpszFilePath, '.');
	if(ext)
	{
		for(int i = 0; i < _countof(g_szNeedProcessDlgFileTypes); i++)
		{
			if(_wcsicmp(ext, g_szNeedProcessDlgFileTypes[i]) == 0)
				return TRUE;
		}
	}
	return FALSE;
}

BOOL IsReadOnlyFile(LPCWSTR pszFileName)
{
	if(!pszFileName)
		return FALSE;

	DWORD dwRet = ::GetFileAttributesW(pszFileName);

	if(dwRet != INVALID_FILE_ATTRIBUTES && (FILE_ATTRIBUTE_READONLY & dwRet) == FILE_ATTRIBUTE_READONLY)
	{//readonly
		return TRUE;
	}

	return FALSE;
}

BOOL IsCATIAFileTypes(LPCWSTR pszFileName)
{
	if(!pszFileName)
		return FALSE;

	const wchar_t* ext = wcsrchr(pszFileName, '.');
	if(ext)
	{
		for(int i = 0; i < _countof(g_szCATIAFileTypes); i++)
		{
			if(_wcsicmp(ext, g_szCATIAFileTypes[i]) == 0)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

bool IsSameFile(LPCWSTR szPath1, LPCWSTR szPath2) 
{ 
	//Validate the input 
	if(szPath1 == NULL || szPath2 == NULL)
	{
		return false;
	}

	//Get file handles 
	HANDLE handle1 = ::CreateFileW(szPath1, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);  
	HANDLE handle2 = ::CreateFileW(szPath2, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);  

	bool bResult = false; 

	//if we could open both paths... 
	if (handle1 != NULL && handle2 != NULL) 
	{ 
		BY_HANDLE_FILE_INFORMATION fileInfo1; 
		BY_HANDLE_FILE_INFORMATION fileInfo2; 
		if (::GetFileInformationByHandle(handle1, &fileInfo1) && ::GetFileInformationByHandle(handle2, &fileInfo2)) 
		{ 
			//the paths are the same if they refer to the same file (fileindex) on the same volume (volume serial number) 
			bResult = fileInfo1.dwVolumeSerialNumber == fileInfo2.dwVolumeSerialNumber && 
				fileInfo1.nFileIndexHigh == fileInfo2.nFileIndexHigh && 
				fileInfo1.nFileIndexLow == fileInfo2.nFileIndexLow; 
		} 
	} 

	//free the handles 
	if (handle1 != NULL) 
	{ 
		::CloseHandle(handle1); 
	} 

	if (handle2 != NULL) 
	{ 
		::CloseHandle(handle2); 
	} 

	//return the result 
	return bResult; 
} 

BOOL GetModuleName(LPWSTR lpszProcessName, int nLen, HMODULE hMod)
{
	if(!lpszProcessName || nLen > 1024)
		return FALSE;

	if(hMod)
	{
		DWORD dwCount = GetModuleFileNameW(hMod, lpszProcessName, nLen);
		return dwCount > 0 ? TRUE: FALSE;
	}

	static wchar_t filename[1025] = {0};
	if( *filename == 0 )//only call GetModuleFileNameW at the first time. Kevin 20089-8-20
	{
		GetModuleFileNameW(hMod, filename, 1024);
	}

	if( *filename != 0 )
	{
		memcpy(lpszProcessName, filename, nLen * sizeof(wchar_t));
		return TRUE;
	}

	return FALSE;
}

std::string MyWideCharToMultipleByte(const std::wstring strValue)
{
	int nLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, strValue.c_str(), static_cast<int>(strValue.length()), NULL, 0, NULL, NULL); 

	char* pBuf = new char[nLen + 1];
	if(!pBuf)
		return "";
	memset(pBuf, 0, nLen +1);
	nLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, strValue.c_str(), static_cast<int>(strValue.length()), pBuf, nLen, NULL, NULL); 

	std::string strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

std::wstring MyMultipleByteToWideChar(const std::string strValue)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), static_cast<int>(strValue.length()), NULL, 0 );
	DWORD dError = 0 ;
	if( nLen == 0 )
	{
		dError = ::GetLastError() ;
		if( dError == ERROR_INVALID_FLAGS )
		{
			nLen = MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), static_cast<int>(strValue.length()), NULL, 0 );
		}
	}
	wchar_t* pBuf = new wchar_t[nLen + 1];
	if(!pBuf)
		return L"";

	memset(pBuf, 0, (nLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), static_cast<int>(strValue.length()), pBuf, nLen);
	dError = ::GetLastError() ;
	if( dError == ERROR_INVALID_FLAGS )
	{
		MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), static_cast<int>(strValue.length()), pBuf, nLen);
	}
	std::wstring strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

BOOL InitLog()
{
	// If debug mode is enabled write to log file as well
	if( NLConfig::IsDebugMode() == true )
	{
		/* Generate a path using the image name of the current process.  Set log policy for DebugView
		* and file on log instance.  Path will be [NextLabs]/Network Enforcer/diags/logs/.
		*/
		wchar_t image_name[MAX_PATH * 2 + 1] = {0};

		if( !GetModuleName(image_name, MAX_PATH * 2, NULL) )
		{
			return FALSE;
		}
		wchar_t* image_name_ptr = wcsrchr(image_name,'\\'); // get just image name w/o full path
		if( image_name_ptr == NULL )
		{
			image_name_ptr = image_name;                   // when failed use default image name
		}
		else
		{
			image_name_ptr++;                              // move past '\\'
		}

		wchar_t install_path[MAX_PATH] = {0};
		if( !NLConfig::GetComponentInstallPath(L"Compliant Enterprise\\Desktop Enforcer",install_path,_countof(install_path)) )
		{
			wcsncpy_s(install_path, MAX_PATH, L"C:\\Program Files\\Nextlabs\\Desktop Enforcer", _TRUNCATE);
		}
		
		wchar_t log_file[MAX_PATH * 3 + 1] = {0};
		_snwprintf_s(log_file,_countof(log_file), _TRUNCATE,L"%s\\diags\\logs\\tagonusage_%s.txt",install_path,image_name_ptr);

		wstring temp = wstring(log_file);
		std::string strLogPath = MyWideCharToMultipleByte(temp);
		g_log.SetPolicy( new CELogPolicy_File(strLogPath.c_str()) );
		

		g_log.SetPolicy( new CELogPolicy_WinDbg() ); // output to DebugView
		g_log.Enable();                              // enable log
		g_log.SetLevel(CELOG_DEBUG);                 // log threshold to debug level
	}

	return TRUE;
}
