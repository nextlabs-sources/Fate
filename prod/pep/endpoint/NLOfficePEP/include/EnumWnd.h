#pragma once
#include "Itemmanager.h"
#define KLA_KEYWORD "blackbox.aspx"
#define KLA_ENCODEINDEX "?rf="
#define IE6_KEYWORD "- Microsoft internet explorer"
#define IE_KEYWOD " - windows internet explorer"
#define KLA_SUBKEYWORD	L"blackbox"
#define ADOBE_READER_SUFFIX	L" - Adobe Reader"
#define ADOBE_SECURE_SUFFIX L" (SECURED) - Adobe Reader"

wchar_t g_strWinCaption[1025] = {0};	
wchar_t g_strWinClass[MAX_PATH] = {0};

#define ADOBE_TYPE 4
#define PPT_FULLSCREEN_TYPE 5
#define ADOBE_ADDIN_TYPE	6

static int g_dwAppType = -1;

extern void ConvertURLCharacterW(std::wstring& strUrl);

typedef struct WndInfo
{
	wstring strUrlPath;
	wstring strClassName;
	wstring strTitleName;
	HWND	hWnd;
	DWORD   dwGetType;	// 1 is get class,2 is get title,3,is get hwnd.
	bool	bReliableMatch;	// need two string are all match, only the dwGetType == 3	
}WND_INFO,*PWND_INFO;

typedef struct _ADOBE_WND_INFO
{
	HWND	hAdobeView;
	HWND	hAdobeFrame;
	wstring strFilePath;
}ADOBE_WND_INFO,*PADOBE_WND_INFO;

struct AppWndParams
{
	wstring strFileTitle;	// the file name without the suffix, file name is:a.doc, the file title is: A
	wstring strFileTypeCaption;	// the word is "MIcrosoft Word" ,capital
	wstring strChildWndTitle;
	wstring str2010PPTCaption; //for 2010 ppt caption.
	int		nAppType;
	HWND	hFrame;
	HWND	hView;
	wstring strPPTFullClassName;	// for ppt full screen
	DWORD   dwOfficeVer;
};
//////////////////////////////////////////////////////////////////////////

// get file title without extension
wstring GetFileNameEx(const wstring& str,bool& bDot )
{
	if(str.empty())
	{
		return str;
	}
	std::wstring::size_type pos, len, dotpos;
	char cslash = '\\';
	pos = str.find('/');
	if(pos != std::wstring::npos)
	{
		cslash = '/';
	}
	pos = str.rfind(cslash);
	if(pos == std::wstring::npos)
	{
		len = str.length();
		dotpos = str.rfind('.');
		if(dotpos == std::wstring::npos)
		{
			return str;
		}
		bDot = true;
		return str.substr(0,dotpos);
	}
	len = str.length();
	dotpos = str.rfind('.');
	if((dotpos == std::wstring::npos)||(dotpos < pos))
	{
		return str.substr(pos + 1,len - pos);
	}
	bDot = true;
	return str.substr(pos + 1,dotpos - pos - 1);
}

int GetProgressType()
{
	bool bDot = false;
	HMODULE  hModel = ::GetModuleHandle(NULL);
	wchar_t strModeName[1025] = {0};
	if(hModel != NULL)
	{
		::GetModuleFileName(hModel,strModeName,1024);
	}
	else
	{
		return -1;
	}
	wstring strModeExEName = strModeName;
	if(!strModeExEName.empty())
	{
		wstring strExeName = GetFileNameEx(strModeExEName,bDot);
		if(_wcsicmp(strExeName.c_str(),L"WINWORD") == 0) 
		{
			return kAppWord;
		}
		else if(_wcsicmp(strExeName.c_str(),L"POWERPNT") == 0) 
		{
			return kAppPPT;
		}
		else if(_wcsicmp(strExeName.c_str(),L"EXCEL") == 0) 
		{
			return kAppExcel;
		}
		else if(_wcsicmp(strExeName.c_str(),L"AcroRd32") == 0)
		{
			return ADOBE_TYPE;
		}
		else if(_wcsicmp(strExeName.c_str(),L"iexplore") == 0 )
		{
			return ADOBE_ADDIN_TYPE;
		}
	}
	else
	{
		return -1;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
//	Process related function
std::string MyWideCharToMultipleByte(const std::wstring & strValue)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0, NULL, NULL); 

	char* pBuf = new char[nLen + 1];
	if(!pBuf)
		return "";
	memset(pBuf, 0, nLen +1);
	nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen, NULL, NULL); 

	std::string strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

std::wstring MyMultipleByteToWideChar(const std::string & strValue)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), NULL, 0 );
	DWORD dError = 0 ;
	if( nLen == 0 )
	{
		dError = ::GetLastError() ;
		if( dError == ERROR_INVALID_FLAGS )
		{
			nLen = MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0 );
		}
	}
	wchar_t* pBuf = new wchar_t[nLen + 1];
	if(!pBuf)
		return L"";

	memset(pBuf, 0, (nLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	dError = ::GetLastError() ;
	if( dError == ERROR_INVALID_FLAGS )
	{
		MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	}
	std::wstring strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}


//////////////////////////////////////////////////////////////////////////
// enum PDF
//////////////////////////////////////////////////////////////////////////
/*
*\ brief: Enum to get Adobe View window ,it is overlay window's parent window, the PDF docuemnt was opened into IE
* strPDFDocPath
*/
//////////////////////////////////////////////////////////////////////////
// For KLA:  Open PDF document in IE 

static inline DWORD GetIEVersion()
{
	LONG    lResult   = 0;
	HKEY    hKey      = NULL;
	DWORD dwIEVersion = 8;

	lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer", 0, KEY_ALL_ACCESS, &hKey);
	if(ERROR_SUCCESS==lResult && NULL!=hKey)
	{
		DWORD dwType = 0, cbData=MAX_PATH;
		char  szData[MAX_PATH+1];  memset(szData, 0, sizeof(szData));
		lResult = RegQueryValueEx(hKey, L"Version", 0, &dwType, (LPBYTE)szData, &cbData);
		if(ERROR_SUCCESS==lResult)
		{
			if('7'==szData[0])	dwIEVersion = 7;
			else if('6' == szData[0])	dwIEVersion = 6;
			else if('8' == szData[0])	dwIEVersion = 8;
			else if('9' == szData[0])	dwIEVersion = 9;
		}
		RegCloseKey(hKey);
	}
	return dwIEVersion;
}

BOOL CALLBACK EnumIEWindow(HWND hwnd, LPARAM lParam) 
{
	vector<HWND>* pVec = (vector<HWND>*)(lParam);
	if(hwnd != NULL)
	{
		::GetClassName(hwnd,g_strWinClass,MAX_PATH);
		if(_wcsicmp(g_strWinClass,L"IEFrame") != 0)	return TRUE;

		static bool bIsIE8 = GetIEVersion()>7?true:false;
		if(!bIsIE8)
		{
			DWORD dwPID=0;
			GetWindowThreadProcessId(hwnd,&dwPID);
			if(dwPID == GetCurrentProcessId())	pVec->push_back(hwnd);
		}
		else
		{
			// we need to get all to check in detail for itself's adobe reader window
			pVec->push_back(hwnd);
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// enum normal PDF 
BOOL CALLBACK EnumAdobeChildProc(HWND hwnd, LPARAM lParam) 
{ 
	if(hwnd != NULL)
	{
		::GetWindowText(hwnd,g_strWinCaption,1024);
		if(0 == wcscmp(g_strWinCaption,L"AVPageView"))
		{
			PADOBE_WND_INFO pAdobeWndInfo = reinterpret_cast<PADOBE_WND_INFO>(lParam);
			if(pAdobeWndInfo != NULL)	pAdobeWndInfo->hAdobeView = hwnd;
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CALLBACK EnumAdobeProc(HWND hwnd, LPARAM lParam) 
{ 
	if(hwnd != NULL)
	{
		PADOBE_WND_INFO pAdobeWndInfo = reinterpret_cast<PADOBE_WND_INFO>(lParam);
		if(pAdobeWndInfo == NULL)	return TRUE;
		const wchar_t* strFileName = pAdobeWndInfo->strFilePath.c_str();
		::GetWindowText(hwnd,g_strWinCaption,1024);

		//we only care open local in browser window.
		if(!boost::algorithm::iends_with(g_strWinCaption,IE_KEYWOD) &&
			!boost::algorithm::iends_with(g_strWinCaption,IE6_KEYWORD))	return TRUE;

		bool bDot = false;
		wstring strTitle = GetFileNameEx(g_strWinCaption,bDot);
		if(strTitle.empty())	return TRUE;

		wstring strpath(strFileName);
		wstring strcaption(g_strWinCaption);
		if(boost::algorithm::icontains(strFileName,strTitle))
		{
			::EnumChildWindows(hwnd,EnumAdobeChildProc,lParam);
			if(pAdobeWndInfo->hAdobeView != NULL)
			{
				// we need to check the static class window to compare the path
				pAdobeWndInfo->hAdobeFrame = hwnd;
				return FALSE;
			}
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// For KLA: Get exact view hwnd for opening office document in KLA's env
// the key words we only can get for KLA is:
// the key words is: classname _WwG, the title is: Microsoft Word Document
// the key words 2 is: classname _WwB, the title is: blackbox.aspx
//
// if the office was opened in IE ,the title wil contains the sub path
// otherwise ,nothing on word instance.
BOOL CALLBACK EnumViewWndInIEForKLA(HWND hwnd,LPARAM lParam)
{
	if(hwnd != NULL)
	{
		::GetWindowText(hwnd,g_strWinCaption,1024);
		::GetClassName(hwnd,g_strWinClass,MAX_PATH);

		if(g_dwAppType == kAppWord)
		{
			if(_wcsicmp(g_strWinClass,L"_WwG") == 0)	// word
			{
				if(_wcsicmp(g_strWinCaption,L"Microsoft Word Document") == 0)
				{

					// okay ,we get what we wanted.
					PWND_INFO pInfo = reinterpret_cast<PWND_INFO>(lParam);
					pInfo->hWnd = hwnd;
					return FALSE;
				}
			}
		}
		else if(g_dwAppType == kAppPPT)
		{
			if(_wcsicmp(g_strWinClass,L"paneClassDC") == 0)	// ppt
			{
				if( _wcsicmp(g_strWinCaption,L"Slide Show") == 0	|| 
					_wcsicmp(g_strWinCaption,L"Slide") == 0			|| 
					_wcsicmp(g_strWinCaption,L"Slide Sorter") == 0	||  
					_wcsicmp(g_strWinCaption,L"Notes") == 0			|| 
					_wcsicmp(g_strWinCaption,L"Slide Master") == 0	|| 
					_wcsicmp(g_strWinCaption,L"Handout Master") == 0 ||
					_wcsicmp(g_strWinCaption,L"Notes Master") == 0  )
				{
					if(	!CItemManager::GetInstance()->CheckIfExistItem(hwnd))
					{
						WORKITEM theItem ;
						theItem.hKeyWnd = hwnd;
						CItemManager::GetInstance()->AddItem(theItem);
						
						// okay ,we get what we wanted.
						PWND_INFO pInfo = reinterpret_cast<PWND_INFO>(lParam);
						pInfo->hWnd = hwnd;
						return FALSE;
					}
					
				}
			}
		}
	}
	return TRUE;
}

BOOL CALLBACK EnumIEOfficeSubChildWnd(HWND hwnd,LPARAM lParam)
{
	if(g_dwAppType == -1)
	{
		g_dwAppType = GetProgressType();
	}
	if(hwnd != NULL)
	{
		::GetWindowText(hwnd,g_strWinCaption,1024);
		::GetClassName(hwnd,g_strWinClass,MAX_PATH);

		if(g_dwAppType == kAppWord)
		{
			if(_wcsicmp(g_strWinClass,L"_WwB") == 0)	// word
			{
				if(boost::algorithm::icontains(g_strWinCaption,KLA_SUBKEYWORD))
				{
					if(	!CItemManager::GetInstance()->CheckIfExistItem(hwnd))
					{
						WORKITEM theItem ;
						theItem.hKeyWnd = hwnd;
						CItemManager::GetInstance()->AddItem(theItem);
						// okay ,we need to go to enum the view window
						if(!EnumChildWindows(hwnd,EnumViewWndInIEForKLA,lParam))	return FALSE;
					}
				}
			}
		}
		else if(g_dwAppType == kAppExcel)
		{
			if(_wcsicmp(g_strWinClass,L"EXCEL7") == 0)	// Excel
			{
				if(boost::algorithm::icontains(g_strWinCaption,KLA_SUBKEYWORD))
				{
					if(	!CItemManager::GetInstance()->CheckIfExistItem(hwnd))
					{
						WORKITEM theItem ;
						theItem.hKeyWnd = hwnd;
						CItemManager::GetInstance()->AddItem(theItem);
						// okay ,This is what we want
						PWND_INFO pInfo = reinterpret_cast<PWND_INFO>(lParam);
						pInfo->hWnd = hwnd;
						return FALSE;	
					}
				}
			}
		}
		else if(g_dwAppType == kAppPPT)
		{
			if(_wcsicmp(g_strWinClass,L"childClass") == 0 || _wcsicmp(g_strWinClass,L"mdiClass") == 0 )	// PPT
			{
				if(boost::algorithm::icontains(g_strWinCaption,KLA_SUBKEYWORD))
				{
					// okay ,we need to go to enum the view window
					if(!EnumChildWindows(hwnd,EnumViewWndInIEForKLA,lParam))	return FALSE;

				}
			}
		}
	}
	return TRUE;
}

BOOL CALLBACK EnumOfficeAppInIEForKLA(HWND hwnd,LPARAM lParam)
{
	if(hwnd != NULL)
	{
		::GetWindowText(hwnd,g_strWinCaption,1024);
		::GetClassName(hwnd,g_strWinClass,MAX_PATH);

		/*
		*\ brief: in ie,we check IEFrame, in APP we check the title 
		*		  contain blackbox.aspx 
		*/
		if(_wcsicmp(g_strWinClass,L"IEFrame") == 0 ||
			boost::algorithm::icontains(g_strWinCaption,KLA_SUBKEYWORD))
		{
			if(!EnumChildWindows(hwnd,EnumIEOfficeSubChildWnd,lParam))	
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

bool GetOfficeViewHwndForKLA(const wstring& strUrl,HWND& hViewWnd)
{
	WND_INFO theFrame;
	theFrame.strUrlPath = strUrl;
	theFrame.strClassName = L"";
	theFrame.strTitleName = L"";
	theFrame.dwGetType = 3;
	theFrame.hWnd = NULL;
	theFrame.bReliableMatch = false;
	if(!::EnumWindows(EnumOfficeAppInIEForKLA,(LPARAM)&theFrame) && theFrame.hWnd != NULL)
	{
		hViewWnd = theFrame.hWnd;
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////

void DeleteCaptionKeyName(wstring &strTile,AppType emOfficeType)
{
	if(emOfficeType == kAppWord)
	{
		boost::algorithm::replace_all(strTile,L" [Compatibility Mode]",L"");
		boost::algorithm::replace_all(strTile,L" (Read-Only)",L"");
		boost::algorithm::replace_all(strTile,L" [Shared]",L"");
		boost::algorithm::replace_all(strTile,L" [Protected View]",L"");
		boost::algorithm::replace_all(strTile,L" (Product Activation Failed)",L"");

		boost::algorithm::replace_all(strTile,L" (Compatibility Mode)",L"");
		boost::algorithm::replace_all(strTile,L" [Read-Only]",L"");
		boost::algorithm::replace_all(strTile,L" (Shared)",L"");
		boost::algorithm::replace_all(strTile,L" (Protected View)",L"");
		boost::algorithm::replace_all(strTile,L" [Product Activation Failed]",L"");
	}
	if (emOfficeType == kAppPPT)
	{
		boost::algorithm::replace_all(strTile,L" (Product Activation Failed)",L"");
		boost::algorithm::replace_all(strTile,L" [Compatibility Mode]",L"");
		boost::algorithm::replace_all(strTile,L" [Read-Only]",L"");
		boost::algorithm::replace_all(strTile,L" [Shared]",L"");
		boost::algorithm::replace_all(strTile,L" [Protected View]",L"");

		boost::algorithm::replace_all(strTile,L" [Product Activation Failed]",L"");
		boost::algorithm::replace_all(strTile,L" (Compatibility Mode)",L"");
		boost::algorithm::replace_all(strTile,L" (Read-Only)",L"");
		boost::algorithm::replace_all(strTile,L" (Shared)",L"");
		boost::algorithm::replace_all(strTile,L" (Protected View)",L"");
	}
	if (emOfficeType == kAppExcel)
	{
		boost::algorithm::replace_all(strTile,L"  (Compatibility Mode)",L"");
		boost::algorithm::replace_all(strTile,L"  (Read-Only)",L"");
		boost::algorithm::replace_all(strTile,L"  (Shared)",L"");
		boost::algorithm::replace_all(strTile,L"  (Protected View)",L"");
		boost::algorithm::replace_all(strTile,L" [Product Activation Failed]",L"");

		boost::algorithm::replace_all(strTile,L"  [Compatibility Mode]",L"");
		boost::algorithm::replace_all(strTile,L"  [Read-Only]",L"");
		boost::algorithm::replace_all(strTile,L"  [Shared]",L"");
		boost::algorithm::replace_all(strTile,L"  [Protected View]",L"");
		boost::algorithm::replace_all(strTile,L" (Product Activation Failed)",L"");
	}
}

//////////////////////////////////////////////////////////////////////////
// For enum local office opened in Office Application
BOOL CALLBACK EnumLocalPPTViewProc(HWND hwnd,LPARAM lParam)
{
	if(hwnd != NULL)
	{
		AppWndParams *pParmes = (AppWndParams *)lParam;
		::GetClassNameW(hwnd,g_strWinClass,MAX_PATH);
		if(_wcsicmp(g_strWinClass,L"paneClassDC") == 0)	// ppt
		{
			pParmes->hView = hwnd;
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CALLBACK EnumLocalOfficeAppViewProc(HWND hwnd,LPARAM lParam)
{
	bool bDot = false;
	AppWndParams *pParmes = (AppWndParams *)lParam;
	if(hwnd != NULL)
	{
		::GetClassNameW(hwnd,g_strWinClass,MAX_PATH);
		if(pParmes->nAppType == PPT_FULLSCREEN_TYPE)
		{
			if(_wcsicmp(g_strWinClass,pParmes->strPPTFullClassName.c_str()) == 0)
			{
				pParmes->hView = hwnd;
				return FALSE;
			}
		}
		
		::GetWindowText(hwnd,g_strWinCaption,1024);
		wstring strTile = GetFileNameEx(g_strWinCaption,bDot);
		

		if (pParmes->nAppType == kAppWord)
		{
			if(0 == _wcsicmp(strTile.c_str(),L"Microsoft Word Document") )
			{
				pParmes->hView = hwnd;
				return FALSE;
			}
		}
		else if (pParmes->nAppType == kAppPPT)
		{
			if(_wcsicmp(g_strWinClass,L"childClass") == 0 || _wcsicmp(g_strWinClass,L"mdiClass") == 0 )	// PPT
			{
				pParmes->hView = hwnd;
				return FALSE;
			}
		}
		else if (pParmes->nAppType == kAppExcel)
		{
			DeleteCaptionKeyName(strTile,kAppExcel);
			if(0 == _wcsicmp(strTile.c_str(),pParmes->strChildWndTitle.c_str()))
			{
				pParmes->hView = hwnd;
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL CALLBACK EnumLocalOfficeAppFrameProc(HWND hwnd,LPARAM lParam)
{
	bool bDot = false;
	if(hwnd != NULL)
	{

		::GetWindowText(hwnd,g_strWinCaption,1024);
		wstring strTile = GetFileNameEx(g_strWinCaption,bDot);

		if(strTile.empty())
		{
			return TRUE;
		}

		AppWndParams *pParmes = (AppWndParams *)lParam;

		if (pParmes->nAppType == kAppExcel)
		{
			DeleteCaptionKeyName(strTile,kAppExcel);
			if(0 == _wcsicmp(strTile.c_str(),pParmes->strFileTypeCaption.c_str())
				||0 == _wcsicmp(strTile.c_str(),L"Microsoft Excel"))
			{
				::EnumChildWindows(hwnd,EnumLocalOfficeAppViewProc,(LPARAM)lParam);
			}
		}
		if (pParmes->nAppType == kAppWord)
		{
			DeleteCaptionKeyName(strTile,kAppWord);

			if(0 == wcscmp(strTile.c_str(),pParmes->strFileTitle.c_str())
				||0 == wcscmp(strTile.c_str(),pParmes->strFileTypeCaption.c_str())
				||0 == wcscmp(strTile.c_str(),L"Microsoft Word"))
			{
				::EnumChildWindows(hwnd,EnumLocalOfficeAppViewProc,(LPARAM)lParam);
			}
		}
		else if(pParmes->nAppType == kAppPPT || pParmes->nAppType ==  PPT_FULLSCREEN_TYPE )
		{
			wstring strTemp = strTile; // 2k7 ppt open in ie on sharepoint
			strTile += L"]";
			
			DeleteCaptionKeyName(strTile,kAppPPT);
			DeleteCaptionKeyName(strTemp,kAppPPT);

			if(0 == wcscmp(strTile.c_str(),pParmes->strFileTitle.c_str())
				||0 == wcscmp(strTile.c_str(),pParmes->strFileTypeCaption.c_str())
				||0 == wcscmp(strTile.c_str(),pParmes->str2010PPTCaption.c_str())
				||0 == wcscmp(strTile.c_str(),pParmes->strChildWndTitle.c_str())
				||0 == wcscmp(strTemp.c_str(),pParmes->strFileTitle.c_str())
				||0 == wcscmp(strTemp.c_str(),pParmes->strFileTypeCaption.c_str())
				||0 == wcscmp(strTemp.c_str(),pParmes->str2010PPTCaption.c_str())
				||0 == wcscmp(strTemp.c_str(),pParmes->strChildWndTitle.c_str()))
			{
			
				::EnumChildWindows(hwnd,EnumLocalOfficeAppViewProc,(LPARAM)lParam);
			}
		}
		if(pParmes->hView != NULL)
		{
			pParmes->hFrame = hwnd;
			return FALSE;
		}
	}
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
