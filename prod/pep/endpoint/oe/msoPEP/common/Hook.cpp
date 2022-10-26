#include "stdafx.h"

#include "madCHook.h"
#include <set>
#include <map>
#pragma warning(push)
#pragma warning(disable: 6334)
#include <boost/algorithm/string.hpp>
#pragma warning(pop)
#include "Hook.h"
#include "FileCache.h"
#include "../outlook/outlookUtilities.h"
#include "../outlook/outlookObj.h"
#include "../outlook/InspEventDisp.h"
#include "../outlook/MailItemUtility.h"
#include "strsafe.h"
#include "../common/AttachmentFileMgr.h"
#include "../common/droptargetproxy.h"
#include "../common/CommonTools.h"
//#pragma warning(push)
//#pragma warning(disable: 4267)
#include "eframework/auto_disable/auto_disable.hpp"
//#include "nlexcept.h"
//#pragma warning(pop)

#pragma comment(lib, "Dbghelp")

extern _ext_AppType g_eAppType;
extern CFileCache   g_RealFileCache;
extern CFileCache   g_TempFileCache;
extern COutlookObj* g_pOutlookObj;
extern CItemEventList g_ItemCache ;

nextlabs::recursion_control mso_hook_control;
nextlabs::recursion_control mso_hook_createfile_control;

class CWndStatus
{
public:
	CWndStatus(HWND hWnd){m_hWnd=hWnd; m_bFirst=TRUE;}
	void set_Hwnd(HWND hWnd){m_hWnd=hWnd;}
	HWND get_Hwnd(){return m_hWnd;}
	void set_FirstRun(BOOL bFirst=FALSE){m_bFirst=bFirst;}
	BOOL get_FirstRun(){return m_bFirst;}
	BOOL operator ==(CWndStatus& other){return m_hWnd==other.get_Hwnd()?TRUE:FALSE;}
private:
	HWND m_hWnd;
	BOOL m_bFirst;
};

class CIgnorFiles
{
public:
	CIgnorFiles()
	{
		m_strWindowsDir  = GetWindowsPath();
		DP((L"Windows Path: %s\n", m_strWindowsDir.c_str()));
		m_strExplorer = m_strWindowsDir.append(L"explorer.exe");
		m_strSystemDir   = GetSystemPath();
		DP((L"System Path: %s\n", m_strSystemDir.c_str()));
		m_strAppDataDir  = GetApplicationDataPath();
		DP((L"AppData Path: %s\n", m_strAppDataDir.c_str()));
		m_strLocalAppDataDir = GetLocalAppDataPath();
		DP((L"Local AppData Path: %s\n", m_strLocalAppDataDir.c_str()));
		if(m_strLocalAppDataDir.length()) m_strOutlookDir = m_strLocalAppDataDir.append(L"Microsoft\\Outlook\\");
		DP((L"Outlook data Path: %s\n", m_strOutlookDir.c_str()));
		m_strInetCacheDir= GetInternetCachePath();
		DP((L"Interbet Cache Path: %s\n", m_strInetCacheDir.c_str()));
		if(m_strInetCacheDir.length())
        {
            m_strMsoCacheDir = m_strInetCacheDir;
            m_strMsoCacheDir.append(L"Content.W");
            m_strMsoCache2Dir = m_strInetCacheDir;
            m_strMsoCache2Dir.append(L"Content.MSO");
        }
        DP((L"MSO Cache Path: %s\n", m_strMsoCacheDir.c_str()));
        DP((L"MSO Cache 2 Path: %s\n", m_strMsoCache2Dir.c_str()));
		m_strOfficeDir = GetOfficePath();
		DP((L"Office Path: %s\n", m_strOfficeDir.c_str()));
		m_strCookieDir  = GetCookiesPath();
		m_strSpecical01 = L"\\\\.\\";
		m_strSpecical02 = L"\\\\?\\";
		m_strSpecical03 = L"C:\\Documents and Settings\\All Users\\Application Data\\Microsoft\\OFFICE\\DATA\\";
		m_strSpecical04 = L"C:\\Program Files\\Common Files\\Microsoft Shared\\Web Folders\\MSONSEXT.DLL";
		m_strSpecical05 = m_strLocalAppDataDir.append(L"Microsoft\\Office\\");
		m_strSpecical06 = L"C:\\Program Files\\Common Files\\System\\ado\\msadox.dll";
		m_strSpecical07 = m_strAppDataDir.append(L"Microsoft\\Office\\");
		m_strSpecical08 = m_strAppDataDir.append(L"Microsoft\\Outlook\\");
		m_strSpecical09 = L"C:\\Program Files\\Common Files\\Microsoft Shared\\";
		m_strSpecical10 = L"C:\\Documents and Settings\\Administrator\\Application Data\\Microsoft\\Outlook\\";
		m_strSpecical11 = L"C:\\Documents and Settings\\Administrator\\Local Settings\\History\\History.";
		m_strSpecical12 = L"C:\\MSOCache\\All Users\\";
	}
	virtual ~CIgnorFiles()
	{
	}
	BOOL IsIgnored(LPCWSTR pwzFileName)
	{
		if(pwzFileName==NULL)
			return TRUE;
		std::wstring strSuffix = GetSuffix(pwzFileName);
		//DP((L"Suffix = %s\n", strSuffix.c_str()));
		if(0==_wcsicmp(L".LNK", strSuffix.c_str()))
			return TRUE;
		if(0==wcsnicmp(pwzFileName, m_strSystemDir.c_str(), (int)m_strSystemDir.length())
			&& 0==_wcsicmp(L".DLL", strSuffix.c_str())
			)
		{
			return TRUE;
		}
		if(0==wcsnicmp(pwzFileName, m_strSpecical09.c_str(), (int)m_strSpecical09.length())
			&& 0==_wcsicmp(L".DLL", strSuffix.c_str())
			)
		{
			return TRUE;
		}
		if((m_strSpecical01.length()>0&&0==wcsnicmp(pwzFileName, m_strSpecical01.c_str(), (int)m_strSpecical01.length()))
			||(m_strSpecical02.length()>0&&0==wcsnicmp(pwzFileName, m_strSpecical02.c_str(), (int)m_strSpecical02.length()))
			||(m_strSpecical03.length()>0&&0==wcsnicmp(pwzFileName, m_strSpecical03.c_str(), (int)m_strSpecical03.length()))
			||(m_strSpecical04.length()>0&&0==wcsnicmp(pwzFileName, m_strSpecical04.c_str(), (int)m_strSpecical04.length()))
			||(m_strSpecical05.length()>0&&0==wcsnicmp(pwzFileName, m_strSpecical05.c_str(), (int)m_strSpecical05.length()))
			||(m_strSpecical06.length()>0&&0==wcsnicmp(pwzFileName, m_strSpecical06.c_str(), (int)m_strSpecical06.length()))
			||(m_strSpecical07.length()>0&&0==wcsnicmp(pwzFileName, m_strSpecical07.c_str(), (int)m_strSpecical07.length()))
			||(m_strSpecical08.length()>0&&0==wcsnicmp(pwzFileName, m_strSpecical08.c_str(), (int)m_strSpecical08.length()))
			||(m_strSpecical10.length()>0&&0==wcsnicmp(pwzFileName, m_strSpecical10.c_str(), (int)m_strSpecical10.length()))
			||(m_strSpecical11.length()>0&&0==wcsnicmp(pwzFileName, m_strSpecical11.c_str(), (int)m_strSpecical11.length()))
			||(m_strSpecical12.length()>0&&0==wcsnicmp(pwzFileName, m_strSpecical12.c_str(), (int)m_strSpecical12.length()))
			||(m_strExplorer.length()>0&&0==wcsnicmp(pwzFileName, m_strExplorer.c_str(), (int)m_strExplorer.length()))
			)
			return TRUE;
		if(( m_strCookieDir.length()>0 ) &&(0==wcsnicmp(pwzFileName, m_strCookieDir.c_str(), (int)m_strCookieDir.length())))
		{
			return TRUE ;
		}
		if((m_strOfficeDir.length()>0)&&0==wcsnicmp(pwzFileName, m_strOfficeDir.c_str(), (int)m_strOfficeDir.length()))
			return TRUE;
		if(m_strOutlookDir.length()>0&&(0==wcsnicmp(pwzFileName, m_strOutlookDir.c_str(), (int)m_strOutlookDir.length())))
			return TRUE;
		if(m_strMsoCacheDir.length()>0&&(0==wcsnicmp(pwzFileName, m_strMsoCacheDir.c_str(), (int)m_strMsoCacheDir.length())))
			return TRUE;
		if(m_strMsoCache2Dir.length()>0&&(0==wcsnicmp(pwzFileName, m_strMsoCache2Dir.c_str(), (int)m_strMsoCache2Dir.length())))
            return TRUE;
		return FALSE;
	}
protected:
	std::wstring GetApplicationDataPath()
	{
		return GetSpecialPath(CSIDL_APPDATA);
	}
	std::wstring GetCookiesPath()
	{
		return GetSpecialPath(CSIDL_COOKIES);
	}
	std::wstring GetLocalAppDataPath()
	{
		return GetSpecialPath(CSIDL_LOCAL_APPDATA);
	}
	std::wstring GetWindowsPath()
	{
		return GetSpecialPath(CSIDL_WINDOWS);
	}
	std::wstring GetSystemPath()
	{
		return GetSpecialPath(CSIDL_SYSTEM);
	}
	std::wstring GetInternetCachePath()
	{
		return GetSpecialPath(CSIDL_INTERNET_CACHE);
	}
	std::wstring GetOfficePath()
	{
		WCHAR wzPath[MAX_PATH+1];	memset(wzPath, 0, sizeof(wzPath));
		GetModuleFileNameW(NULL, wzPath, MAX_PATH);
		if(0 != wzPath[0])
		{
			WCHAR* pwzEnd = wcsrchr(wzPath, L'\\');
			if(pwzEnd) *(pwzEnd+1) = 0;
		}
		return wzPath;
	}
	std::wstring GetSpecialPath(int nPathId)
	{
		WCHAR wzPath[MAX_PATH];	memset(wzPath, 0, sizeof(wzPath));
		if(SUCCEEDED(SHGetFolderPath(NULL, nPathId, NULL, 0, wzPath)))
		{
			WCHAR* pwzLastSlash = wcsrchr(wzPath, L'\\');
			if(NULL==pwzLastSlash || 0!=*(pwzLastSlash+1))
				wcsncat_s(wzPath, MAX_PATH, L"\\", _TRUNCATE);
			return wzPath;
		}
		else
		{
			return L"";
		}
	}
	std::wstring GetSuffix(LPCWSTR pwzFile)
	{
		const WCHAR* pwzEnd = wcsrchr(pwzFile, L'.');
		if(NULL!=pwzEnd)
			return pwzEnd;
		else
			return L"";
	}
private:
	//std::vector<std::wstring>	m_vecIgnorFiles;
	std::wstring				m_strWindowsDir;
	std::wstring				m_strSystemDir;
	std::wstring				m_strAppDataDir;
	std::wstring				m_strLocalAppDataDir;
	std::wstring				m_strInetCacheDir;
	std::wstring				m_strOutlookDir;
	std::wstring				m_strMsoCacheDir;
	std::wstring				m_strMsoCache2Dir;
	std::wstring				m_strOfficeDir;
	std::wstring                m_strExplorer;
	std::wstring				m_strCookieDir;
	std::wstring				m_strSpecical01;
	std::wstring				m_strSpecical02;
	std::wstring				m_strSpecical03;
	std::wstring				m_strSpecical04;
	std::wstring				m_strSpecical05;
	std::wstring				m_strSpecical06;
	std::wstring				m_strSpecical07;
	std::wstring				m_strSpecical08;
	std::wstring				m_strSpecical09;
	std::wstring				m_strSpecical10;
	std::wstring				m_strSpecical11;
	std::wstring				m_strSpecical12;
};

CIgnorFiles		g_IgnoredFiles;

typedef std::set<HWND> MailWndSet;
MailWndSet      g_MailWnds;

typedef std::pair<HWND,CInspEventDisp*> InspSinkPair;
typedef std::map<HWND,CInspEventDisp*>  InspSinkObjMap;
InspSinkObjMap  g_InspSinkObjs;

CInspEventDisp* OnNewInspector(CComQIPtr<Outlook::_Inspector> Inspector);

//enum {OUTLOOK_VER_NUM_2000 = 9, OUTLOOK_VER_NUM_XP = 10, OUTLOOK_VER_NUM_2003 = 11, OUTLOOK_VER_NUM_2007 = 12};

/* Exception state.  When 'exception_state' has a non-zero value an exception
 * has occurred.
 */
static int exception_state = 0;

//////////////////////////////////////////////////////////////////////////
static HANDLE (WINAPI * next_CreateFileW)(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)=0;
static HWND   (WINAPI * next_CreateWindowExW)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)=0;
static BOOL   (WINAPI * next_ShowWindow)(HWND hWnd,int nCmdShow)=0;
static BOOL   (WINAPI * next_DestroyWindow)(HWND hWnd)=0;
static BOOL   (WINAPI * next_CopyFileW)(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)=0;
static BOOL   (WINAPI * next_CopyFileExW)(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)=0;
static HRESULT(WINAPI*  next_RegisterDragDrop)(HWND hwnd, IDropTarget * pDropTarget)=0;
static BOOL   (WINAPI*  next_DeleteFileW)(__in  LPCTSTR lpFileName) = 0;

static HANDLE  (WINAPI * next_KernelBaseCreateFileW)(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)=0;

static HANDLE (WINAPI * real_CreateFileW)(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)=CreateFileW;
static HWND   (WINAPI * real_CreateWindowExW)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)=CreateWindowExW;
static BOOL   (WINAPI * real_ShowWindow)(HWND hWnd,int nCmdShow)=ShowWindow;
static BOOL   (WINAPI * real_DestroyWindow)(HWND hWnd)=DestroyWindow;
static BOOL   (WINAPI * real_CopyFileW)(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)=CopyFileW;
static BOOL   (WINAPI * real_CopyFileExW)(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)=CopyFileExW;
static HRESULT(WINAPI * real_RegisterDragDrop)(HWND hwnd, IDropTarget * pDropTarget)=RegisterDragDrop;
static BOOL   (WINAPI*  real_DeleteFileW)(__in  LPCTSTR lpFileName) = DeleteFileW;

HANDLE WINAPI hooked_CreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);
HWND   WINAPI hooked_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
BOOL   WINAPI hooked_ShowWindow(HWND hWnd, int nCmdShow);
BOOL   WINAPI hooked_DestroyWindow(HWND hWnd);
BOOL   WINAPI hooked_CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists);
BOOL WINAPI hooked_CopyFileExW(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
HRESULT WINAPI hooked_RegisterDragDrop(HWND hwnd, IDropTarget * pDropTarget);
BOOL WINAPI hooked_DeleteFileW(__in  LPCTSTR lpFileName);

HANDLE WINAPI hooked_KernelBaseCreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);

BOOL IsMeetingSaveTipWinodw(HWND hWnd);
void AutoCloseMeetingSaveTipWindow(HWND hWnd);

//////////////////////////////////////////////////////////////////////////
void NLHookFunction( _In_ LPCSTR pszModule, _In_ LPCSTR pszFuncName, _In_ PVOID pCallbackFunc, _In_ PVOID* ppNextHook, _In_ const wchar_t* kpwchDebugString )
{
    if ( NULL != ppNextHook )
    {
        bool bHook = HookAPI( pszModule, pszFuncName, pCallbackFunc, ppNextHook );
        if ( bHook )
        {
            NLPRINT_DEBUGVIEWLOG( L"hook ShowWindow success: debug string[%s] \n", ((NULL==kpwchDebugString) ? L"NULL" : kpwchDebugString) );
        }
        else
        {
            NLPRINT_DEBUGVIEWLOG( L"hook ShowWindow failed: debug string[%s] \n", ((NULL==kpwchDebugString) ? L"NULL" : kpwchDebugString) );
            *ppNextHook = NULL;
        }
    }
}

void NLUnHookFunction( _Inout_ LPVOID* ppFunction, _In_ const wchar_t* kpwchDebugString )
{
    if ( NULL != ppFunction && NULL != *ppFunction )
    {
        bool bUnHook = UnhookAPI( ppFunction );
        if( bUnHook )
        {
            NLPRINT_DEBUGVIEWLOG( L"Unhook function success: debug string[%s] \n", ((NULL==kpwchDebugString) ? L"NULL" : kpwchDebugString) );
            *ppFunction = NULL;
        }
        else
        {
            NLPRINT_DEBUGVIEWLOG( L"Unhook function failed: debug string[%s] \n", ((NULL==kpwchDebugString) ? L"NULL" : kpwchDebugString) );
        }
    }
}

void StartStopHook(bool bStart)
{
	__try
	{
        bool bNeedHookKernelBaseCreateFile = CAttachmentFileMgr::IsNeedUseKernelBaseCreateFile(OLUtilities::GetVersionNumber(), GetOutlookImageType(), GetPlateformVersion(), GetOSImageType());
		bool bNeedHookDragDrop = CAttachmentFileMgr::IsNeedUseDragDrop(OLUtilities::GetVersionNumber(), GetOutlookImageType(), GetPlateformVersion(), GetOSImageType() );
		DP((L"bNeedHookDragDrop=%d, bNeedHookKernelBaseCreateFile=%d \n", bNeedHookDragDrop, bNeedHookKernelBaseCreateFile));
		if(bStart==true)
		{
			InitializeMadCHook();
			SetMadCHookOption(DISABLE_CHANGED_CODE_CHECK, NULL);
			if(HookCode((PVOID)real_CreateFileW,(PVOID)hooked_CreateFileW,(PVOID*)&next_CreateFileW)==FALSE) {DP((L"Failed to hook CreateFileW"));}
			if(HookCode((PVOID)real_CreateWindowExW,(PVOID)hooked_CreateWindowExW,(PVOID*)&next_CreateWindowExW)==FALSE) {DP((L"Failed to hook CreateWindowExW"));}
			if(HookCode((PVOID)real_ShowWindow,(PVOID)hooked_ShowWindow,(PVOID*)&next_ShowWindow)==FALSE) {DP((L"Failed to hook ShowWindow"));}
			if(HookCode((PVOID)real_DestroyWindow,(PVOID)hooked_DestroyWindow,(PVOID*)&next_DestroyWindow)==FALSE) {DP((L"Failed to hook DestroyWindow"));}
			if(HookCode((PVOID)real_CopyFileW,(PVOID)hooked_CopyFileW,(PVOID*)&next_CopyFileW)==FALSE) {DP((L"Failed to hook CopyFileW"));}
            if(HookCode((PVOID)real_CopyFileExW,(PVOID)hooked_CopyFileExW,(PVOID*)&next_CopyFileExW)==FALSE) {DP((L"Failed to hook CopyFileExW"));}
            if(HookCode((PVOID)real_DeleteFileW,(PVOID)hooked_DeleteFileW,(PVOID*)&next_DeleteFileW)==FALSE) {DP((L"Failed to hook DeleteFileW"));}
			if(bNeedHookDragDrop&& (HookCode((PVOID)real_RegisterDragDrop,(PVOID)hooked_RegisterDragDrop,(PVOID*)&next_RegisterDragDrop)==FALSE)) {DP((L"Failed to hook RegisterDragDrop"));}

            if (bNeedHookKernelBaseCreateFile)
            {
                NLHookFunction("kernelbase.dll","CreateFileW",hooked_KernelBaseCreateFileW,(PVOID*)&next_KernelBaseCreateFileW, L"hooked_KernelBaseCreateFileW");
            }
		}
		else
		{//must be unhooked.otherwise there will be core down where register the dll.
			
			if(UnhookCode((PVOID*)&next_CreateFileW)==FALSE) {DP((L"Failed to unhook CreateFileW"));}
			if(UnhookCode((PVOID*)&next_CreateWindowExW)==FALSE) {DP((L"Failed to unhook CreateWindowExW"));}
			if(UnhookCode((PVOID*)&next_ShowWindow)==FALSE) {DP((L"Failed to unhook ShowWindow"));}
			if(UnhookCode((PVOID*)&next_DestroyWindow)==FALSE) {DP((L"Failed to unhook DestroyWindow"));}
			if(UnhookCode((PVOID*)&next_CopyFileW)==FALSE) {DP((L"Failed to unhook CopyFileW"));}
            if(UnhookCode((PVOID*)&next_CopyFileExW)==FALSE) {DP((L"Failed to unhook CopyFileW"));}
            if(UnhookCode((PVOID*)&next_DeleteFileW)==FALSE) {DP((L"Failed to unhook DeleteFileW"));}
			if(bNeedHookDragDrop && UnhookCode((PVOID*)&next_RegisterDragDrop)==FALSE) {DP((L"Failed to unhook RegisterDragDrop"));}

            if (bNeedHookKernelBaseCreateFile)
            {
                NLUnHookFunction((PVOID*)&next_KernelBaseCreateFileW, L"hooked_KernelBaseCreateFileW");
            }

			FinalizeMadCHook();
		}
	}
    __except (1)
    {
      /* empty */
        ;
    }
}

//////////////////////////////////////////////////////////////////////////
static std::wstring GetCurrentProfileOstFile(_In_ BOOL bForce)
{
    static std::wstring g_strOstFile;

    HKEY    hKey;
    WCHAR   wzPath[MAX_PATH+1];
    DWORD   dwPathLen = (MAX_PATH+1)*sizeof(WCHAR);
    DWORD   dwType = 0;
    const WCHAR* wzKeyName   = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows Messaging Subsystem\\Profiles\\Outlook\\13dbb0c8aa05101a9bb000aa002fc45a";
    const WCHAR* wzValueName = L"001f6610";

    if(!bForce && !g_strOstFile.empty())
        return g_strOstFile;

    memset(wzPath, 0, sizeof(wzPath));
    if(ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_CURRENT_USER, wzKeyName, 0, KEY_READ, &hKey))
        return g_strOstFile;

    if(ERROR_SUCCESS != RegQueryValueEx(hKey, wzValueName, 0, &dwType, (LPBYTE)wzPath, &dwPathLen))
        return g_strOstFile;

    g_strOstFile = wzPath;
    return g_strOstFile;
}

HANDLE WINAPI hooked_CreateFileWCore(LPCTSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
    nextlabs::recursion_control_auto auto_disable(mso_hook_control);//leo 20170523
	HANDLE hFile = next_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if ((NULL != lpFileName) && (INVALID_HANDLE_VALUE != hFile))
    {
        /*
            For msg format file, outlook do not create "outlook temp" file anymore, no CopyFileW.
            We cannot get the source file path by using "Attachment SaveAs" and "Hook CopyFileW".
            We cache the source file path here for ".msg" file.
        */
        CAttachmentFileMgr& theAttachmentFileMgr = CAttachmentFileMgr::GetInstance();
        if (theAttachmentFileMgr.GetInitFlag())
        {
            theAttachmentFileMgr.SetMsgSrcFilePath(hFile, lpFileName);
        }
    }
    return hFile;
}

HANDLE WINAPI hooked_CreateFileW(LPCTSTR lpFileName,DWORD dwDesiredAccess,
                  DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                  HANDLE hTemplateFile)
{
	//begin leo 20170523
	if (mso_hook_control.is_disabled())
	{
		if(NULL==next_CreateFileW)
			return FALSE;
		return next_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	//end
	__try
    {
        return hooked_CreateFileWCore(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    __except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return next_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
}


HWND   WINAPI hooked_CreateWindowExWCore(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    nextlabs::recursion_control_auto auto_disable(mso_hook_control);
	HWND hWnd = next_CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	BOOL bIsChild  = (0 != (WS_CHILD&dwStyle))?TRUE:FALSE;
	BOOL bIsCaption= (0 != (WS_CAPTION&dwStyle))?TRUE:FALSE;
	if(!bIsChild && bIsCaption)// && NULL!=lpWindowName && 0==memcmp(DEST_CLASS_NAME, lpWindowName, 30))
	{
		if(0!=lpClassName && 0xFFFFFFFF!=(ULONG_PTR)lpClassName && !IsBadStringPtrW(lpClassName, 15))
		{
			if(NULL !=g_pOutlookObj && NULL!=hWnd && 0==wcscmp(DEST_CLASS_NAME, lpClassName))
			{
				CComPtr<Outlook::_Inspectors> spInspectors;
                MailWndSet::iterator it = g_MailWnds.find(hWnd);
                if (it == g_MailWnds.end())
                {
                    g_MailWnds.insert(hWnd);
                }
			}
		}
	}

    return hWnd;
}

HWND WINAPI hooked_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName,
                LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth,
                int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance,
                LPVOID lpParam)
{
    if (mso_hook_control.is_disabled())
    {
        if(NULL==next_CreateWindowExW)
            return NULL;
        return next_CreateWindowExW(dwExStyle, lpClassName, lpWindowName,
            dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu,
            hInstance, lpParam);
    }

    __try
    {
        return hooked_CreateWindowExWCore(dwExStyle, lpClassName, lpWindowName,
                   dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu,
                   hInstance, lpParam);
    }
    __except (0)
    {
        /* empty */
        ;
    }
    return NULL; /* fail */
}

/*
* \brief: sink inspector for sending by hook showwindow function
*/
BOOL InsertInspEventDisp(HWND hWnd)
{
	HRESULT hr = S_OK;
	CComQIPtr<Outlook::_Inspector>  spInspector;
	CComPtr<Outlook::_Inspectors> spInspectors;
	long                          lCount = 0;

	hr = g_pOutlookObj->get_App()->get_Inspectors(&spInspectors);
    if (SUCCEEDED(hr) && NULL != spInspectors.p)
    {
        spInspectors->get_Count(&lCount);
        CComVariant varIndex(lCount);
        spInspectors->Item(varIndex, &spInspector);
        if (NULL != spInspector)
        {
            CInspEventDisp* pInspSink = OnNewInspector(spInspector);
            if (pInspSink)
            {
                DP((L" Add sink obj Wnd=0x%p, obj=0x%p\n", hWnd, pInspSink));
                g_InspSinkObjs.insert(InspSinkPair(hWnd, pInspSink));
                return TRUE;
            }
        }
    }
	return FALSE;
}


BOOL   WINAPI hooked_ShowWindowCore(HWND hWnd, int nCmdShow)
{
    nextlabs::recursion_control_auto auto_disable(mso_hook_control);

	//close Meeting Save tip Window
	if(g_pOutlookObj->NeedAutoCloseMeetingSaveTipWindow() && IsMeetingSaveTipWinodw(hWnd) )
	{
		AutoCloseMeetingSaveTipWindow(hWnd);
		nCmdShow = SW_HIDE;
	}

    MailWndSet::iterator it = g_MailWnds.find(hWnd);
	// It is a Inspector Window
	if(it != g_MailWnds.end())
    {
		DP((L"hooked_ShowWindow:: nCmdShow=%d, wnd=0x%p\n", nCmdShow, hWnd));
        if(!IsWindowVisible(hWnd) && (SW_SHOWNORMAL==nCmdShow || SW_SHOWMAXIMIZED==nCmdShow))
        {
            InsertInspEventDisp(hWnd);
        }
        else if(SW_HIDE==nCmdShow)  // the mail is destoryed
        {
            DP((L"Close Email: wnd=0x%p\n", hWnd));
            InspSinkObjMap::iterator itso = g_InspSinkObjs.find(hWnd);
            if(itso != g_InspSinkObjs.end())
            {
                DP((L"hooked_ShowWindow:: Release sink obj Wnd=0x%p, obj=0x%p\n", (*itso).first, (*itso).second));
                if( NULL!= (*itso).second ) ((*itso).second)->Release();
                g_InspSinkObjs.erase(itso);
            }
        }
    }

	return next_ShowWindow(hWnd, nCmdShow);
}

BOOL WINAPI hooked_ShowWindow(HWND hWnd, int nCmdShow)
{
    if (mso_hook_control.is_disabled())
    {
        if(NULL==next_ShowWindow)
            return FALSE;
        return next_ShowWindow(hWnd, nCmdShow);
    }

    __try
    {
        return hooked_ShowWindowCore(hWnd, nCmdShow);
    }
    __except (0)
    {
        /* empty */
        ;
    }
    return FALSE; /* fail */
}

BOOL   WINAPI hooked_DestroyWindowCore(HWND hWnd)
{
    nextlabs::recursion_control_auto auto_disable(mso_hook_control);
    InspSinkObjMap::iterator itSink = g_InspSinkObjs.find(hWnd);
    if(itSink != g_InspSinkObjs.end())
    {
        DP((L"hooked_DestroyWindow:: Release sink obj Wnd=0x%p, obj=0x%p\n", (*itSink).first, (*itSink).second));
        if( NULL!= (*itSink).second ) 
		{
			((*itSink).second)->Release();
			g_ItemCache.DeleteItem(((*itSink).second)->Get_MailItemDisp() ) ;
		}		
        g_InspSinkObjs.erase(itSink);
    }

    MailWndSet::iterator it = g_MailWnds.find(hWnd);
    if(it != g_MailWnds.end())
    {
        DP((L"#####   Windows is destroyed: 0x%p\n", hWnd));
        g_MailWnds.erase(it);
    }

	return next_DestroyWindow(hWnd);
}

BOOL WINAPI hooked_DestroyWindow(HWND hWnd)
{
    if (mso_hook_control.is_disabled())
    {
        if(NULL==next_DestroyWindow)
            return FALSE;
        return next_DestroyWindow(hWnd);
    }

    __try
    {
        return hooked_DestroyWindowCore(hWnd);
    }
    __except (0)
    {
        /* empty */
        ;
    }
    return FALSE; /* fail */
}

// sink event
CInspEventDisp* OnNewInspector(CComQIPtr<Outlook::_Inspector> Inspector)
{
    HRESULT hr = S_OK;
    CComObject<CInspEventDisp>*	pInspDisp = NULL;
    DWORD dwInspSinkCookie = 0;

    if(Inspector)
    {
        CComPtr<IDispatch>  lpDisp = 0;
        hr = Inspector->get_CurrentItem(&lpDisp);
        if (SUCCEEDED(hr) && lpDisp)
        {
            ITEM_TYPE itemType = DEFAULT;
            if (OLUtilities::CheckGetMailItemType(lpDisp, itemType) == TRUE)
            {
                hr = CComObject<CInspEventDisp>::CreateInstance(&pInspDisp);
                if (SUCCEEDED(hr) && pInspDisp)
                {
                    CComPtr<IUnknown> spInspSinkObj = NULL;
                    hr = pInspDisp->QueryInterface(IID_IUnknown, (void**)&spInspSinkObj);
                    if (SUCCEEDED(hr) && spInspSinkObj)
                    {
                        hr = AtlAdvise(Inspector, spInspSinkObj, __uuidof(Outlook::InspectorEvents), &dwInspSinkCookie);
                        if (SUCCEEDED(hr) && dwInspSinkCookie)
                        {
                            pInspDisp->InitInspSink(Inspector, spInspSinkObj, dwInspSinkCookie);
                        }
                    }
                }
            }
        }
    }

    return pInspDisp;
}
  

HANDLE CallNextCreateFileFunction(LPCTSTR lpFileName,DWORD dwDesiredAccess,
								  DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
								  HANDLE hTemplateFile)
{
	if(NULL==next_CreateFileW)
		return INVALID_HANDLE_VALUE;
	return next_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
		lpSecurityAttributes, dwCreationDisposition,
		dwFlagsAndAttributes, hTemplateFile);
}

BOOL IsMeetingSaveTipWinodw(HWND hWnd)
{
   //first. check the window class name
	const int nClassNameLen = 100;
	wchar_t wszClsName[nClassNameLen] ={0};
	int nGetCls = ::GetClassNameW(hWnd, wszClsName, nClassNameLen);
	if( (nGetCls<=0) || (_wcsicmp(wszClsName, L"#32770")!=0) )
	{
      return FALSE;
	}

	//second. check the text of the first child window
	HWND hChildWnd = ::GetWindow(hWnd, GW_CHILD);
	if(hWnd)
	{
		const int nWndTextLen = 1024;
		wchar_t szWindowText[nWndTextLen+1] = {0};
		::GetWindowTextW(hChildWnd, szWindowText, nWndTextLen);
		if(wcsstr(szWindowText, L"Choose one of the following:"))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void AutoCloseMeetingSaveTipWindow(HWND hWnd)
{
	::PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(1,0), 0);
}

BOOL WINAPI hooked_CopyFileWCore(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
{
	nextlabs::recursion_control_auto auto_disable(mso_hook_control);
	BOOL bRet = next_CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);
	if (bRet)
	{
		if ((NULL != lpExistingFileName) && (NULL != lpNewFileName))
		{
			CAttachmentFileMgr& theAttachmentFileMgr = CAttachmentFileMgr::GetInstance();
			if (theAttachmentFileMgr.GetInitFlag())
			{
				theAttachmentFileMgr.AddFileIntoCache(lpExistingFileName, lpNewFileName);
			}
		}
	}
	return bRet;
}

BOOL WINAPI hooked_CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
{
	__try
	{
		logd(L"[hooked_CopyFileWCore]lpExistingFileName=%s, lpNewFileName=%s", lpExistingFileName, lpNewFileName);
		return hooked_CopyFileWCore(lpExistingFileName, lpNewFileName, bFailIfExists);
	}
	__except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		DP((L"!!!!!!Exception: hook hooked_CopyFileWCore exception \n"));
		return next_CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);
	}
    return FALSE;
}

BOOL WINAPI hooked_CopyFileExWCore(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
    bool bRet = next_CopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    if (bRet)
    {
        if ((NULL != lpExistingFileName) && (NULL != lpNewFileName))
        {
            CAttachmentFileMgr& theAttachmentFileMgr = CAttachmentFileMgr::GetInstance();
            if (theAttachmentFileMgr.GetInitFlag())
            {
                theAttachmentFileMgr.AddSourceFilesInFileServerCache(lpExistingFileName, lpNewFileName);
            }
        }
    }
    return bRet;
}

BOOL WINAPI hooked_CopyFileExW(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
    __try
    {
        logd(L"[hooked_CopyFileExWCore]lpExistingFileName=%s, lpNewFileName=%s", lpExistingFileName, lpNewFileName);
		return hooked_CopyFileExWCore(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    }
    __except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return next_CopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    }
}

BOOL WINAPI hooked_DeleteFileWCore(__in  LPCTSTR lpFileName)
{
    // Must get long file path before delete, because we cannot convert a not exist short path to long path.
    const std::wstring kwstrTempFileFullPath = (NULL != lpFileName) ? NLGetLongFilePathEx(lpFileName) : L"";
    BOOL bRet = next_DeleteFileW(lpFileName);
    if (bRet)
    {
        if (NULL != lpFileName)
        {
            CAttachmentFileMgr& theAttachmentFileMgr = CAttachmentFileMgr::GetInstance();
            if (theAttachmentFileMgr.GetInitFlag())
            {
                theAttachmentFileMgr.DeleteFileFromCache(kwstrTempFileFullPath);
            }
        }
    }
    return bRet;
}


BOOL WINAPI hooked_DeleteFileW(__in  LPCTSTR lpFileName)
{
    __try
    {
        return hooked_DeleteFileWCore(lpFileName);
    }
    __except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return next_DeleteFileW(lpFileName);
    }
}

HRESULT WINAPI Hooked_RegisterDragDropCore(HWND hwnd, IDropTarget * pDropTarget)
{
	nextlabsoe::DropTargetProxy* proxy = new nextlabsoe::DropTargetProxy(hwnd, pDropTarget);
	return next_RegisterDragDrop(hwnd, proxy);
}

HRESULT WINAPI hooked_RegisterDragDrop(HWND hwnd, IDropTarget * pDropTarget)
{
	__try
	{
		return Hooked_RegisterDragDropCore(hwnd, pDropTarget);
	}
	__except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		DP((L"!!!!!!Exception: hook Hooked_RegisterDragDropCore exception \n"));
		return next_RegisterDragDrop(hwnd, pDropTarget);
	}
}

HANDLE WINAPI hooked_KernelBaseCreateFileWCore(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	nextlabs::recursion_control_auto auto_disable(mso_hook_control);
    HANDLE hFile = next_KernelBaseCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if ((NULL != lpFileName) && (INVALID_HANDLE_VALUE != hFile))
    {
        /*
            For win10 x86 outlook2016 x86, Ctrl + c/v case, no CopyFileW, CopyFileExW.
            Here hook CreateFileW in kernelbase to cache the file path.
        */
        DWORD dwFileAttr = GetFileAttributesW(lpFileName);
        if ( (INVALID_FILE_ATTRIBUTES!=dwFileAttr)
            && (!(FILE_ATTRIBUTE_DIRECTORY&dwFileAttr))
            && (!g_IgnoredFiles.IsIgnored(lpFileName))
          )
        {
            CAttachmentFileMgr& theAttachmentFileMgr = CAttachmentFileMgr::GetInstance();
            if (theAttachmentFileMgr.GetInitFlag())
            {
                theAttachmentFileMgr.SetFilePathIntoFinallyBackCache(hFile, lpFileName);
            }
        }
    }
    return hFile;
}

HANDLE WINAPI hooked_KernelBaseCreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	if (mso_hook_control.is_disabled())
	{
		if(NULL==next_KernelBaseCreateFileW)
			return FALSE;
		return next_KernelBaseCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
    __try
    {
        return hooked_KernelBaseCreateFileWCore(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    __except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return next_KernelBaseCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
}
