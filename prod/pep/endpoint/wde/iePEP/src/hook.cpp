#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600
#include "stdafx.h"
#include <string>
#include <Oleacc.h>
#include <commdlg.h>
#include "utils.h"
#include <vector>
#include <windows.h>
#include <ShlObj.h>
#include <map>
#include <Wininet.h>
#include "eframework/policy/comm_helper.hpp"
#include <boost/algorithm/string.hpp>
#include "resattrlib.h"
#include "resattrmgr.h"
#include "ActionHandler.h"
#include "mapix.h"
#include "mapi.h"
#include "celog.h"

#include "eframework/auto_disable/auto_disable.hpp"

#define CELOG_CUR_MODULE L"iePEP"
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_WDE_IEPEP_SRC_HOOK_CPP

using namespace std;

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning(pop)

#pragma warning( push )
#pragma warning( disable : 4819 )
#include "madCHook.h"
#pragma warning( pop )
#ifdef _M_IX86
#pragma  comment (lib,"madChook32.lib")
#elif defined (_M_AMD64)
#pragma  comment(lib,"madCHook64.lib")
#else
#error NoSupport this platform
#endif


#pragma comment(lib, "Comdlg32")
#pragma comment(lib, "Oleacc")

extern unsigned int g_ip;

extern std::wstring        g_activeAnchorUrl;
extern std::wstring        g_activeAnchorName;

extern std::wstring        g_strDownloadUrl;
extern std::wstring        g_strUploadUrl;
extern BOOL EvaluateDownload(LPCWSTR src, LPCWSTR dest);
extern BOOL EvaluateUpload(LPCWSTR src, LPCWSTR dest);
extern void ConvertURLCharacterW(std::wstring& strUrl);

const WCHAR DownloadFileTagName[] = L"NextlabsUrlOfTheTempFile";
WCHAR IETempPath[MAX_PATH] = { 0 };
int IETempPathLength = 0;
nextlabs::recursion_control hook_control;
CActionHandler* pActionHandler;

static const IID CLSID_PrintDocumentPackageTargetFactory = {0x348ef17d, 0x6c81, 0x4982, {0x92, 0xB4, 0xEE, 0x18, 0x8A, 0x43, 0x86, 0x7A}};
static const IID IID_IPrintDocumentPackageTargetFactory = {0xd2959bf7, 0xb31b, 0x4a3d, {0x96, 0x00, 0x71, 0x2e, 0xb1, 0x33, 0x5b, 0xa4}};

wstring FormatPath(const wstring& path)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: path=%ls \n", path.c_str());

	wstring ret = path;
	if (boost::algorithm::istarts_with(path, L"file:///"))
	{
		ret = path.substr(8);
	}

	ConvertURLCharacterW(ret);
	return ret;
}

std::wstring GetCommonComponentsDir()
{
    wchar_t szDir[MAX_PATH] = {0};
    if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
    {
#ifdef _WIN64
        wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
        wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
        CELOG_LOG(CELOG_DUMP, L"Local variables are: szDir=%ls \n", (szDir) );
        return szDir;
    }
    CELOG_LOG(CELOG_DUMP, L"Local variables are: szDir=%ls \n", (szDir) );

    return L"";
}

//////////////////////////////////////////////////////////////////////////

typedef int (*CreateAttributeManagerType)(ResourceAttributeManager **mgr);
typedef int (*AllocAttributesType)(ResourceAttributes **attrs);
typedef int (*ReadResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int (*ReadResourceAttributesForNTFSWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int (*GetAttributeCountType)(const ResourceAttributes *attrs);
typedef void (*FreeAttributesType)(ResourceAttributes *attrs);
typedef void (*CloseAttributeManagerType)(ResourceAttributeManager *mgr);
typedef void (*AddAttributeWType)(ResourceAttributes *attrs, const WCHAR *name, const WCHAR *value);
typedef const WCHAR *(*GetAttributeNameType)(const ResourceAttributes *attrs, int index);
typedef const WCHAR * (*GetAttributeValueType)(const ResourceAttributes *attrs, int index);
typedef int (*WriteResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int (*WriteResourceAttributesForNTFSWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);

CreateAttributeManagerType lfCreateAttributeManager = NULL;
AllocAttributesType lfAllocAttributes = NULL;
ReadResourceAttributesWType lfReadResourceAttributesW = NULL;
ReadResourceAttributesForNTFSWType lfReadResourceAttributesForNTFSW = NULL;
GetAttributeCountType lfGetAttributeCount = NULL;
FreeAttributesType lfFreeAttributes = NULL;
CloseAttributeManagerType lfCloseAttributeManager = NULL;
AddAttributeWType lfAddAttributeW = NULL;
GetAttributeNameType lfGetAttributeName = NULL;
GetAttributeValueType lfGetAttributeValue = NULL;
WriteResourceAttributesWType lfWriteResourceAttributesW = NULL;
WriteResourceAttributesForNTFSWType lfWriteResourceAttributesForNTFSW = NULL;

BOOL SetFileTags ( LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags, BOOL NTFSStream );
BOOL GetFileTags ( LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags, BOOL NTFSStream );

//////////////////////////////////////////////////////////////////////////
typedef long (__stdcall *TYPE_CFormElementSubmit)(ULONG_PTR p);
typedef int  (__stdcall *TYPE_fire_onsubmit)(void);
typedef long (__stdcall *TYPE_showModalDialog)(ULONG_PTR, WCHAR*, VARIANT *, VARIANT *, VARIANT *);

static BOOL (WINAPI * next_GetSaveFileNameW)(LPOPENFILENAME lpofn) = GetSaveFileNameW;
static BOOL (WINAPI * next_GetOpenFileNameW)(LPOPENFILENAME lpofn) = GetOpenFileNameW;
static int (WINAPI* next_StartDocW) ( HDC hdc, const DOCINFOW *lpdi) = StartDocW;

static BOOL (WINAPI* next_InternetQueryOptionW) ( HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, LPDWORD lpdwBufferLength ) = InternetQueryOptionW;
static BOOL (WINAPI* next_InternetSetOptionW) ( HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength ) = InternetSetOptionW;
static BOOL (WINAPI* next_CopyFileW) ( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists ) = CopyFileW;

static long (__stdcall *next_CFormElementSubmit)(ULONG_PTR p)      = 0;
static long (__stdcall *next_showModalDialog)(ULONG_PTR, WCHAR*, VARIANT *, VARIANT *, VARIANT *) = 0;

static BOOL (WINAPI * real_GetSaveFileNameW)(LPOPENFILENAME lpofn) = GetSaveFileNameW;
static BOOL (WINAPI * real_GetOpenFileNameW)(LPOPENFILENAME lpofn) = GetOpenFileNameW;
static int (WINAPI* read_StartDocW) ( HDC hdc, const DOCINFOW *lpdi) = StartDocW;

static BOOL (WINAPI* real_InternetQueryOptionW) ( HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, LPDWORD lpdwBufferLength ) = InternetQueryOptionW;
static BOOL (WINAPI* real_InternetSetOptionW) ( HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength ) = InternetSetOptionW;
static BOOL (WINAPI* real_CopyFileW) ( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists ) = CopyFileW;

static long (__stdcall *real_CFormElementSubmit)(ULONG_PTR p)      = 0;
static int  (__stdcall *real_fire_onsubmit)(void)                  = 0;
static long (__stdcall *real_showModalDialog)(ULONG_PTR, WCHAR*, VARIANT *, VARIANT *, VARIANT *) = 0;

long __stdcall Detour_CFormElementSubmit(ULONG_PTR p);
long __stdcall Detour_showModalDialog(ULONG_PTR pThis, WCHAR* url , VARIANT *varArgIn, VARIANT *varOptions, VARIANT *varArgOut);
BOOL WINAPI Detour_GetSaveFileNameW(LPOPENFILENAME lpofn);
BOOL WINAPI Detour_GetOpenFileNameW(LPOPENFILENAME lpofn);
int WINAPI Detour_StartDocW( HDC hdc, const DOCINFOW *lpdi);

BOOL WINAPI Detour_InternetQueryOptionW ( HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, LPDWORD lpdwBufferLength );
BOOL WINAPI Detour_InternetSetOptionW ( HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength );
BOOL WINAPI Detour_CopyFileW ( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists );

static HRESULT (WINAPI * next_CoCreateInstance)(
	REFCLSID rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD dwClsContext,
	REFIID riid,
	LPVOID * ppv
	) = CoCreateInstance;
static HRESULT (WINAPI * real_CoCreateInstance)(
	REFCLSID rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD dwClsContext,
	REFIID riid,
	LPVOID * ppv
	) = CoCreateInstance;
HRESULT WINAPI MyCoCreateInstance(
	REFCLSID rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD dwClsContext,
	REFIID riid,
	LPVOID * ppv
	);

LPMAPILOGONEX next_MAPILogonEx = NULL;

typedef ULONG (WINAPI *MAPISendMailType)(
						  _In_  LHANDLE lhSession,
						  _In_  ULONG_PTR ulUIParam,
						  _In_  lpMapiMessage lpMessage,
						  _In_  FLAGS flFlags,
						  ULONG ulReserved
						  );

MAPISendMailType next_MAPISendMail= NULL;


typedef HRESULT(STDMETHODCALLTYPE *PF_CreateDocumentPackageTargetForPrintJob)( 
	int *This,
	LPCWSTR printerName,
	LPCWSTR jobName,
	IStream *jobOutputStream,
	IStream *jobPrintTicketStream,
	LPVOID **docPackageTarget);

typedef HRESULT ( STDMETHODCALLTYPE *f_Show ) ( IFileSaveDialog* This, HWND hwndOwner );

typedef struct _Func_FileSaveDialog
{
	void*	pVtable;
	f_Show real_Show;
	f_Show next_Show;
}Func_FileSaveDialog;

LPVOID GetVFuncAdd(LPVOID pObject,const unsigned int nOffSet)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: pObject=%p, nOffSet=%u \n", pObject, nOffSet);

	if(pObject == NULL)	return NULL;
	return (LPVOID)*((INT_PTR*)*(INT_PTR*)pObject+nOffSet); 
}

map<IFileSaveDialog*,Func_FileSaveDialog> g_FileSaveDialog;
std::map<LPVOID, LPVOID> g_COMHooks;
CRITICAL_SECTION g_csCom;

//////////////////////////////////////////////////////////////////////////

HANDLE (WINAPI *NextSetClipboardData)(UINT uFormat, HANDLE hMem)=NULL;
using std::wstring;
extern bool DecodeHttpPath(const wstring& strTempPath,wstring& strTruePath);

extern std::wstring g_strPDFDocumentPath;

bool IsDenySaveAs(LPCWSTR source, LPCWSTR target)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: source=%ls, target=%ls \n",  (source),  (target));

	if (!source || !target)
	{
		return false;
	}

	std::wstring src = std::wstring(source);
	if (boost::algorithm::istarts_with(src, L"file:"))
	{
		src = src.substr(5, src.length() - 5);
		while (src.length() > 0)
		{
			if (src[0] != '/')
			{
				break;
			}
			else
			{
				src = src.substr(1, src.length() - 1);
			}

		}

	}

    if (!boost::algorithm::istarts_with(src, L"http") && !boost::algorithm::istarts_with(src, L"ftp"))
    {
        if(!boost::algorithm::istarts_with(src, L"\\\\") && src.length() >= 2 && src[1] != ':')
        {//UNC path
            src = L"\\\\" + src;
        }
    }

	boost::replace_all(src,L"/",L"\\");

	ConvertURLCharacterW(src);

	
	boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);

	nextlabs::eval_parms parm;
	parm.SetAction( L"COPY");
	parm.SetSrc(src.c_str());
	parm.SetTarget(target);
	parm.SetIp(g_ip);

	if(ptr->Query(&parm, NULL))
	{
		DP((L"query is denied: %d\n", ptr->IsDenied()));
		if (ptr->IsDenied())
		{
			return true; 
		}
	}
	else
		DP((L" query failed"));
	
	return false;
}

HRESULT WINAPI MyMAPILogonEx ( ULONG_PTR ulUIParam, LPTSTR lpszProfileName, LPTSTR lpszPassword, ULONG ulFlags, LPMAPISESSION* lppSession )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: ulUIParam=%lu, lpszProfileName=%ls, lpszPassword=%ls, ulFlags=%lu, lppSession=%p \n", ulUIParam,   (lpszProfileName), (lpszPassword), ulFlags,lppSession);

	DP((L"MyMAPILogonEx, current file path: %s\n", g_strUploadUrl.c_str()));

	wstring path = FormatPath(g_strUploadUrl);

	boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);
	nextlabs::eval_parms parm;
	parm.SetAction(L"SEND");
	parm.SetSrc(path.c_str());
	parm.SetIp(g_ip);

	DP((L"Try to do query, path: %s, action: %s\n", path.c_str(), L"SEND"));

	if(ptr->Query(&parm))
	{
		if(ptr->IsDenied())
		{
			CELOG_LOG(CELOG_DUMP, L"Local variables are: path=%ls \n", path.c_str() );
			return MAPI_E_USER_CANCEL;
		}
	}	
	CELOG_LOG(CELOG_DUMP, L"Local variables are: path=%ls \n", path.c_str() );
	return next_MAPILogonEx(ulUIParam, lpszProfileName, lpszPassword, ulFlags, lppSession);
}

ULONG WINAPI MyMAPISendMail(
						  _In_  LHANDLE lhSession,
						  _In_  ULONG_PTR ulUIParam,
						  _In_  lpMapiMessage lpMessage,
						  _In_  FLAGS flFlags,
						  ULONG ulReserved
						  )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: lhSession=%lu, ulUIParam=%lu, lpMessage=%p, flFlags=%lu, ulReserved=%lu  \n", lhSession, ulUIParam,lpMessage, flFlags,ulReserved);

	DP((L"MyMAPISendMail, current file path: %s\n", g_strUploadUrl.c_str()));

	wstring path = FormatPath(g_strUploadUrl);

	boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);
	nextlabs::eval_parms parm;
	parm.SetAction(L"SEND");
	parm.SetSrc(path.c_str());
	parm.SetIp(g_ip);

	if(ptr->Query(&parm))
	{
		if(ptr->IsDenied())
		{
			return ERROR_ACCESS_DENIED;
		}
	}	
	CELOG_LOG(CELOG_DUMP, L"Local variables are: path=%ls \n", path.c_str() );

	return next_MAPISendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
}


static HRESULT STDMETHODCALLTYPE try_Show ( IFileSaveDialog* pThis, HWND hwndOwner )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: pThis=%p, hwndOwner=%p \n", pThis, hwndOwner);

	DP((L"iePep enters try_show()\n"));
	EnterCriticalSection(&g_csCom);
	map<IFileSaveDialog*,Func_FileSaveDialog>::iterator iter = g_FileSaveDialog.find(pThis);
	LeaveCriticalSection(&g_csCom);
	if(iter == g_FileSaveDialog.end())	return E_FAIL;

    if( hook_control.is_disabled() == true )
    {
        return iter->second.next_Show(pThis,hwndOwner);
    }

    nextlabs::recursion_control_auto auto_disable(hook_control);

	HRESULT hr = iter->second.next_Show(pThis,hwndOwner);
	
	if (SUCCEEDED(hr))
	{
		wchar_t szTarget[1024] = {0};
		IShellItem *psiResult;
		HRESULT hrTemp = pThis->GetResult(&psiResult);
		
		if (SUCCEEDED(hrTemp))
		{
			PWSTR pszFilePath = NULL;
			hrTemp = psiResult->GetDisplayName(SIGDN_FILESYSPATH, 
				&pszFilePath);
			if (SUCCEEDED(hrTemp))
			{	
				wcsncpy_s(szTarget, 1024, pszFilePath, _TRUNCATE);

				CoTaskMemFree ( pszFilePath );
			}
			psiResult->Release();
		}


		if (IsDenySaveAs(g_strUploadUrl.c_str(), szTarget))
		{
			return HRESULT_FROM_WIN32 ( ERROR_CANCELLED ); 
		}
	}
	
	CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X \n", hr );
	return hr;
}

static HRESULT STDMETHODCALLTYPE Hooked_CreateDocumentPackageTargetForPrintJob( 
	int *This,
	LPCWSTR printerName,
	LPCWSTR jobName,
	IStream *jobOutputStream,
	IStream *jobPrintTicketStream,
	LPVOID **docPackageTarget)
{
	PF_CreateDocumentPackageTargetForPrintJob next_func = NULL;
		
	LPVOID* pVtable = (*(LPVOID**)This);
	LPVOID pPrintJob = pVtable[3];
	EnterCriticalSection(&g_csCom);
	std::map<LPVOID, LPVOID>::iterator iter = g_COMHooks.find(pPrintJob);
	if (iter == g_COMHooks.end())
	{
		LeaveCriticalSection(&g_csCom);
		return 0x8007003f;//print cancel.
	}
	next_func = (PF_CreateDocumentPackageTargetForPrintJob)(*iter).second;
	if (hook_control.is_disabled())
	{
		LeaveCriticalSection(&g_csCom);
		return next_func(This, printerName, jobName, jobOutputStream, jobPrintTicketStream, docPackageTarget);
	}
	LeaveCriticalSection(&g_csCom);

	nextlabs::recursion_control_auto auto_disable(hook_control);
	if (!pActionHandler->PrintAction(g_strUploadUrl.c_str()))
	{
		 return 0x8007003f;
	}
   
    return next_func(This, printerName, jobName, jobOutputStream, jobPrintTicketStream, docPackageTarget);
}

// for adobe reader 9
static HANDLE WINAPI try_BJSetClipboardData(UINT uFormat, HANDLE hMem)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: uFormat=%u, hMem=%p \n", uFormat, hMem);
 	if( hook_control.is_disabled() == true )
 	{
 		return NextSetClipboardData(uFormat,hMem);
 	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if(!g_strPDFDocumentPath.empty())
	{
		bool bHttps = boost::algorithm::istarts_with(g_strPDFDocumentPath,L"https://");
	
		if(boost::algorithm::icontains(g_strPDFDocumentPath,PDF_KEYWORD))
		{
			wstring strTruePath;
			if(DecodeHttpPath(g_strPDFDocumentPath,strTruePath) && !strTruePath.empty())
			{
				g_strPDFDocumentPath = strTruePath;
			}
		}
		if (bHttps)	
		{
			boost::algorithm::replace_first(g_strPDFDocumentPath,L"http://",L"https://");
		}
	
        if (pActionHandler && !pActionHandler->PasteAction(g_strPDFDocumentPath.c_str(), L""))
        {
			SetLastError(ERROR_ACCESS_DENIED);
			return NULL;
		}
		
	}
	return NextSetClipboardData(uFormat,hMem);
}
//////////////////////////////////////////////////////////////////////////


const WCHAR* wzExplorerClassName = L"Internet Explorer_Server";

static BOOL UpPrivilege(HANDLE hProcess, LPCWSTR wzPrivilegeName)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: hProcess=%p, wzPrivilegeName=%ls \n", hProcess,  (wzPrivilegeName));

    HANDLE              hToken = NULL;
    TOKEN_PRIVILEGES    Privileges;
    LUID                luid;

    static BOOL bAdjusted = FALSE;
    if(!bAdjusted)
    {
        if(OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken) && NULL!=hToken)
        {
            Privileges.PrivilegeCount   = 1;
            if(LookupPrivilegeValue(NULL, wzPrivilegeName, &luid))
            {
                Privileges.Privileges[0].Luid = luid;
                Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                bAdjusted = AdjustTokenPrivileges(hToken, FALSE, &Privileges, NULL, NULL, NULL);
            }
            CloseHandle(hToken); hToken=NULL;
        }
    }
   CELOG_LOG(CELOG_DUMP, L"Local variables are: hToken=%p, Privileges=%p, luid=%p, bAdjusted=%s \n", hToken,&Privileges,&luid, bAdjusted?L"TRUE":L"FALSE" );

    return bAdjusted;
}

// HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Internet Explorer
static BOOL IsIE7()
{
    BOOL    bIsIE7    = FALSE;
    LONG    lResult   = 0;
    HKEY    hKey      = NULL;

    lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer", 0, KEY_ALL_ACCESS, &hKey);
    if(ERROR_SUCCESS==lResult && NULL!=hKey)
    {
        DWORD dwType = 0, cbData=MAX_PATH;
        char  szData[MAX_PATH+1];  memset(szData, 0, sizeof(szData));
        lResult = RegQueryValueEx(hKey, L"Version", 0, &dwType, (LPBYTE)szData, &cbData);
        if(ERROR_SUCCESS==lResult && '7'==szData[0])
            bIsIE7 = TRUE;
        RegCloseKey(hKey);
    }
	CELOG_LOG(CELOG_DUMP, L"Local variables are: bIsIE7=%s, lResult=0X%08X, hKey=%p  \n", bIsIE7? L"TRUE":L"FALSE",lResult,hKey );

    return bIsIE7;
}

// function is splitter by 5 "90"
static LPBYTE FindNextEntry(LPBYTE pBaseAddress, LPBYTE pMaxRange, int nEntrySize)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: pBaseAddress=%p, pMaxRange=%p, nEntrySize=%d \n", pBaseAddress, pMaxRange,nEntrySize);

    BYTE bBlanket[5] = {0x90, 0x90, 0x90, 0x90, 0x90};
    while (pBaseAddress < pMaxRange)
    {
        if(0x90 != *pBaseAddress)
        {
            pBaseAddress++; continue;
        }
        else
        {
            if(0 == memcmp(pBaseAddress, bBlanket, 5))
            {
                pBaseAddress += 5;
                if(0x90 != *pBaseAddress && pBaseAddress<pMaxRange)
                {
                    LPBYTE pEnd = pBaseAddress + nEntrySize;  // destination function body length should be 0x6E
                    if(0 == memcmp(pEnd, bBlanket, 5))
                        return pBaseAddress;
                }
            }
            else
            {
                pBaseAddress += 5;
            }
        }
    }
    return NULL;
}

const DWORD dwOffsetAIE6  = 0;
const DWORD dwCompareAIE6 = 80;
const BYTE  bCompareAIE6[80] = {0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x58,
                                0x83, 0x65, 0xC0, 0x00, 0x33, 0xC0, 0x56, 0x57,
                                0x8D, 0x7D, 0xD0, 0xAB, 0xAB, 0xAB, 0xAB, 0x33,
                                0xC0, 0x6A, 0x16, 0x59, 0x8D, 0x7D, 0xA8, 0xF3,
                                0xAB, 0x8B, 0x45, 0x0C, 0x83, 0x65, 0xC8, 0x00,
                                0x89, 0x45, 0xB4, 0x8B, 0x45, 0x10, 0x89, 0x45,
                                0xB8, 0x8B, 0x45, 0x14, 0x89, 0x45, 0xC4, 0x8B,
                                0x45, 0x18, 0x89, 0x45, 0xBC, 0x8B, 0x45, 0x08,
                                0x8B, 0x48, 0x10, 0x8D, 0x70, 0xF0, 0x8D, 0x45,
                                0xA8, 0x89, 0x4D, 0xE8, 0x50, 0x8B, 0xCE, 0xE8};
const DWORD dwOffsetBIE6  = 0x6E-0xA;
const DWORD dwCompareBIE6 = 10;
const BYTE  bCompareBIE6[10] = {0xFD, 0xFF, 0x5F, 0x8B, 0xC6, 0x5E, 0xC9, 0xC2, 0x14, 0x00};

const DWORD dwOffsetAIE7    = 0;
const DWORD dwCompareAIE7   = 13;
const BYTE bCompareAIE7[13] = {0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x58, 0x56, 0x8D, 0x4D, 0xA8, 0xE8};
const DWORD dwOffsetBIE7    = 15;
const DWORD dwCompareBIE7   = 57;
const BYTE bCompareBIE7[57] = {0xEE, 0xFF, 0x8B, 0x45, 0x0C, 0x83, 0x65, 0xC8,
                               0x00, 0x89, 0x45, 0xB4, 0x8B, 0x45, 0x10, 0x89,
                               0x45, 0xB8, 0x8B, 0x45, 0x14, 0x89, 0x45, 0xC4,
                               0x8B, 0x45, 0x18, 0x89, 0x45, 0xBC, 0x8B, 0x45,
                               0x08, 0x8B, 0x48, 0x14, 0x8D, 0x70, 0xF0, 0x8D,
                               0x45, 0xA8, 0x89, 0x4D, 0xE8, 0x50, 0x8B, 0xCE,
                               0xE8, 0x5D, 0xFA, 0xFF, 0xFF, 0x50, 0x8B, 0xCE,
                               0xE8};

static ULONG_PTR SearchShowModalWindowBaseCOmWindowProxy(LPBYTE pBaseAddress, BOOL isIE6)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: pBaseAddress=%p, isIE6=%s \n", pBaseAddress, isIE6 ? L"TRUE" : L"FALSE");

    LPBYTE pPos   = pBaseAddress;
    int    nfuncBodySize = isIE6?0x6E:0x5D;
    LPBYTE pMaxRange = (LPBYTE)((LPBYTE)pBaseAddress + 0x100000);
    while (NULL != (pPos=FindNextEntry(pPos, pMaxRange, nfuncBodySize)))
    {
        if(isIE6 && 0==memcmp(pPos, bCompareAIE6, dwCompareAIE6) && 0==memcmp(pPos+dwOffsetBIE6, bCompareBIE6, dwCompareBIE6))
            return (ULONG_PTR)pPos;
        if(!isIE6 && 0==memcmp(pPos, bCompareAIE7, dwCompareAIE7) && 0==memcmp(pPos+dwOffsetBIE7, bCompareBIE7, dwCompareBIE7))
            return (ULONG_PTR)pPos;
        pPos += nfuncBodySize + 5;
    }
    CELOG_LOG(CELOG_DUMP, L"Local variables are: pPos=%p, nfuncBodySize=%d, pMaxRange=%p \n", pPos,nfuncBodySize,pMaxRange );

    return NULL;
}
void HookShowModalDialog(CComQIPtr<IHTMLDocument2> spDocument2, BOOL bHook)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are:  bHook=%s \n",  bHook?L"TRUE": L"FALSE");

    static BOOL bShowModalDialogHooked = FALSE;
    static int  nTryRef = 0;
    HRESULT     hr = S_OK;

    if(bHook)
    {
        // Only try 3 times
        if(!bShowModalDialogHooked && 3>nTryRef++)
        {
            CComPtr<IHTMLWindow2>   spWindow2 = 0;   
            CComPtr<IDispatch>      spDisp = 0;
            hr = spDocument2->get_parentWindow(&spWindow2);
            if(SUCCEEDED(hr) && spWindow2.p)
            {
                BOOL bIsIE6 = IsIE7()?FALSE:TRUE;
                ULONG_PTR** pVTlb = (ULONG_PTR**)spWindow2.p;
                ULONG_PTR*  p = pVTlb[0];
                ULONG_PTR   pShowModalWindow = SearchShowModalWindowBaseCOmWindowProxy((LPBYTE)(p[55]), bIsIE6);
                real_showModalDialog = (TYPE_showModalDialog)pShowModalWindow;
                DP((L"real_showModalDialog = %p\n", pShowModalWindow));

                if(HookCode((PVOID)real_showModalDialog,(PVOID)Detour_showModalDialog,(PVOID*)&next_showModalDialog)==FALSE)
                {
                    DP((L"Failed to hook showModalDialog\n"));
                }
                else
                {
                    bShowModalDialogHooked = TRUE;
                    DP((L"showModalDialog is hooked\n"));
                }
            }
        }
    }
    else
    {
        if(bShowModalDialogHooked && NULL!=next_showModalDialog)
        {
            if(UnhookCode((PVOID*)&next_showModalDialog)==FALSE)
            {
                DP((L"Failed to unhook showModalDialog\n"));
            }
            bShowModalDialogHooked = FALSE;
        }
    }
	CELOG_LOG(CELOG_DUMP, L"Local variables are: bShowModalDialogHooked=%s, nTryRef=%d, hr=0X%08X \n", bShowModalDialogHooked?L"TRUE":L"FALSE",nTryRef,hr );

}

// pBase:    search from where
// pNewBase: address of next up function
// return:   new function's length
static ULONG FindNextUpFunc(ULONG_PTR pBase, ULONG_PTR* ppNewBase)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: pBase=%lu, ppNewBase=%p \n", pBase, ppNewBase);

    ULONG   ulSize = 1;
    BYTE*   pCursor = (BYTE*)pBase;

    // move to end
    while (0x90 != *(pCursor--)){}
    while (0x90 == *(pCursor--)){}

    // count function body
    do
    {
        ulSize++;
    }while (0x90 != *(--pCursor));

    // get new base
    *ppNewBase = (ULONG_PTR)(pCursor+1);
    CELOG_LOG(CELOG_DUMP, L"Local variables are: ulSize=%lu, pCursor=%p  \n", ulSize,pCursor );

    return ulSize;
}

static ULONG_PTR FindFireFunc(ULONG_PTR pBase)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: pBase=%lu \n", pBase);

    const BYTE  pStart[]   = {0x33,0xC0,0x50,0x50,0x6A,0xFF,0x50,0x6A,0x01};
    const BYTE  pEnd[]     = {0xF7,0xD8,0x1B,0xC0,0xF7,0xD8,0xC3};
    const ULONG ulFuncSize = 0x1A;
    int   i = 0, nFind=0;
    ULONG_PTR   pCursor = pBase;
    ULONG_PTR   pFire   = pBase;
    BOOL        bFind   = FALSE;// change TRUE to FALSE. Kevin Zhou 2008-4-28
    
    for(i=0; i<100; i++)
    {
        ULONG ulBodySize = FindNextUpFunc(pCursor, &pFire);
        if(ulFuncSize == ulBodySize)
        {
            BYTE* pCompare = (BYTE*)pFire;
            if(0==memcmp(pStart, pCompare, sizeof(pStart))
                && 0==memcmp(pEnd, (pCompare+19), sizeof(pEnd))
                )
            {
                nFind++;
                if(2 == nFind)
                {
                    bFind = TRUE;
                    break;
                }
            }
        }
        pCursor = pFire;
    }
	CELOG_LOG(CELOG_DUMP, L"Local variables are: ulFuncSize=%lu, i=%d, nFind=%d, pCursor=%lu, pFire=%lu, bFind=%s  \n", ulFuncSize,i,nFind,pCursor,pFire,bFind?L"TRUE":L"FALSE" );


    if(bFind)
        return pFire;
    else
        return NULL;
}

void HookSubmit(ULONG_PTR lpSubmit, BOOL bHook)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: lpSubmit=%lu, bHook=%s \n", lpSubmit, bHook?L"TRUE":L"FALSE");

    static BOOL m_bHookedSubmit = FALSE;

    if(bHook)
    {
        if(!m_bHookedSubmit)
        {
            real_CFormElementSubmit = (TYPE_CFormElementSubmit)lpSubmit;
            real_fire_onsubmit      = (TYPE_fire_onsubmit)FindFireFunc(lpSubmit);
            DP((L"HOOK CFormElement::submit()\n"));
            DP((L"    Submit Addr      = %p\n", lpSubmit));
            DP((L"    Fire_Submit Addr = %p\n", real_fire_onsubmit));

            if(real_CFormElementSubmit && real_fire_onsubmit)
            {
                if(HookCode((PVOID)real_CFormElementSubmit,(PVOID)Detour_CFormElementSubmit,(PVOID*)&next_CFormElementSubmit)==FALSE)
                {
                    DP((L"Failed to hook CFormElementSubmit\n"));
                }
                else
                {
                    m_bHookedSubmit = TRUE;
                    DP((L"CFormElementSubmit is hooked\n"));
                }
            }
        }
    }
    else
    {
        if(m_bHookedSubmit && NULL!=next_CFormElementSubmit)
        {
            if(UnhookCode((PVOID*)&next_CFormElementSubmit)==FALSE)
            {
                DP((L"Failed to unhook CFormElementSubmit\n"));
            }
            m_bHookedSubmit = FALSE;
        }
    }
	CELOG_LOG(CELOG_DUMP, L"Local variables are: m_bHookedSubmit=%s \n", m_bHookedSubmit?L"TRUE":L"FALSE" );

}

BOOL WINAPI Detour_InternetSetOptionW ( HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength )
{
    if( hook_control.is_disabled() == true )
    {
        return next_InternetSetOptionW(hInternet, dwOption, lpBuffer, dwBufferLength);
    }

    nextlabs::recursion_control_auto auto_disable(hook_control);

    if ( 116 == dwOption && NULL != hInternet && NULL != lpBuffer )
    {
        DWORD BufferSize = 0;

        next_InternetQueryOptionW(hInternet, INTERNET_OPTION_DATAFILE_NAME, NULL, &BufferSize);

        BOOL bDeny = FALSE;

        if ( ERROR_INSUFFICIENT_BUFFER == GetLastError())
        {
            BufferSize++;

            WCHAR* pwzBuffer = new WCHAR[BufferSize];

            std::map<std::wstring, std::wstring> FileTag;

            if ( next_InternetQueryOptionW(hInternet, INTERNET_OPTION_DATAFILE_NAME, pwzBuffer, &BufferSize)
                && 0 == _wcsnicmp ( IETempPath, pwzBuffer, IETempPathLength )
                && GetFileTags(pwzBuffer, FileTag, TRUE) )
            {
                std::map<std::wstring, std::wstring>::const_iterator cit = FileTag.find(DownloadFileTagName);

                //have url tag
                if ( FileTag.end() != cit )
                {
                    boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);
                    nextlabs::eval_parms parm;
                    parm.SetAction(L"COPY");
                    parm.SetSrc(cit->second.c_str());
                    parm.SetTarget((WCHAR*)lpBuffer);
					parm.SetIp(g_ip);

                    if (ptr->Query(&parm) && ptr->IsDenied())
                    {
                        bDeny = TRUE;
                    }
                    else
                    {
                        //Do evaluation again for temp file
                        parm.SetSrc(pwzBuffer);
						parm.SetIp(g_ip);

                        if (ptr->Query(&parm) && ptr->IsDenied())
                        {
                            bDeny = TRUE;
                        }
                    }	
                }
            }

            delete[] pwzBuffer;
        }

        if (bDeny)
        {
            DeleteFileW((WCHAR*)lpBuffer);
            return TRUE;
        }
    }

    return next_InternetSetOptionW(hInternet, dwOption, lpBuffer, dwBufferLength);
}

BOOL StartStopHook(BOOL bStart)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: bStart=%s \n", bStart?L"TRUE" :L"FALSE");

    static BOOL Hooked = FALSE;
    if(bStart)
    {
        if(!Hooked)
        {
			::InitializeCriticalSection(&g_csCom);


            BOOL bAdjust = FALSE;
            bAdjust = UpPrivilege(GetCurrentProcess(), SE_DEBUG_NAME);

			if(HookCode((PVOID)real_GetOpenFileNameW,(PVOID)Detour_GetOpenFileNameW,(PVOID*)&next_GetOpenFileNameW)==FALSE)
			{
				 DP((L"Failed to hook GetOpenFileNameW"));
			}
			else
	     	{
				 DP((L"GetOpenFileNameW is hooked\n"));
			}

			if(HookCode((PVOID)read_StartDocW,(PVOID)Detour_StartDocW,(PVOID*)&next_StartDocW)==FALSE)
			{
				DP((L"Failed to hook StartDocW"));
			}
			else
			{
				DP((L"StartDocW is hooked\n"));
			}

            if(HookCode((PVOID)real_InternetQueryOptionW,(PVOID)Detour_InternetQueryOptionW,(PVOID*)&next_InternetQueryOptionW)==FALSE)
            {
                DP((L"Failed to hook InternetQueryOptionW"));
            }
            else
            {
                DP((L"InternetQueryOptionW is hooked\n"));
            }

            if(HookCode((PVOID)real_InternetSetOptionW,(PVOID)Detour_InternetSetOptionW,(PVOID*)&next_InternetSetOptionW)==FALSE)
            {
                DP((L"Failed to hook InternetSetOptionW"));
            }
            else
            {
                DP((L"InternetSetOptionW is hooked\n"));
            }

            HookAPI ( "kernel32.dll", "CopyFileW", Detour_CopyFileW, (PVOID*) &next_CopyFileW, 0 );

            if(HookCode((PVOID)real_GetSaveFileNameW,(PVOID)Detour_GetSaveFileNameW,(PVOID*)&next_GetSaveFileNameW)==FALSE)
            {
                DP((L"Failed to hook GetSaveFileNameW"));
            }
            else
            {
                DP((L"GetSaveFileNameW is hooked\n"));
            }
			if(HookAPI("user32.dll","SetClipboardData",try_BJSetClipboardData,(PVOID*)&NextSetClipboardData))
			{
				DP((L"SetClipboardData is hooked\n"));
			}
			else
			{
				DP((L"Failed to hook SetClipboardData\n"));
			}

			/************************************************************************/
			/* Replace function from HookCode to HookAPI
			*  It would be failed to hook CreateDocumentPackageTargetForPrintJob interface if we use HookCode to hook 
			*  CoCreateInstance.
			*/
			/************************************************************************/
			/*HookCode((PVOID)real_CoCreateInstance, (PVOID)MyCoCreateInstance, (PVOID*)&next_CoCreateInstance);*/
			HookAPI("ole32", "CoCreateInstance", (void*)MyCoCreateInstance, (void**)&next_CoCreateInstance);
			HookAPI ( "MAPI32.dll", "MAPILogonEx", MyMAPILogonEx, (PVOID*) &next_MAPILogonEx, 0 );
			HookAPI ( "MAPI32.dll", "MAPISendMail", MyMAPISendMail, (PVOID*) &next_MAPISendMail, 0 );

            Hooked = TRUE;
        }
    }
    else
    {
        if(Hooked)
        {
			::DeleteCriticalSection(&g_csCom);

             if(UnhookCode((PVOID*)&next_GetOpenFileNameW)==FALSE)
             {
                 DP((L"Failed to unhook GetOpenFileNameW\n"));
             }
             else
             {
                 DP((L"Unhook GetOpenFileNameW\n"));
             }

            if(UnhookCode((PVOID*)&next_InternetSetOptionW)==FALSE)
            {
                DP((L"Failed to unhook InternetSetOptionW\n"));
            }
            else
            {
                DP((L"Unhook InternetSetOptionW\n"));
            }

            if(UnhookCode((PVOID*)&next_InternetQueryOptionW)==FALSE)
            {
                DP((L"Failed to unhook InternetQueryOptionW\n"));
            }
            else
            {
                DP((L"Unhook InternetQueryOptionW\n"));
            }

            UnhookCode((PVOID*)&next_CopyFileW);

			if(UnhookCode((PVOID*)&next_StartDocW)==FALSE)
			{
				DP((L"Failed to unhook StartDocW\n"));
			}
			else
			{
				DP((L"Unhook StartDocW\n"));
			}

            if(UnhookCode((PVOID*)&next_GetSaveFileNameW)==FALSE)
            {
                DP((L"Failed to unhook GetSaveFileNameW\n"));
            }
            else
            {
                DP((L"Unhook GetSaveFileNameW\n"));
            }

			if(UnhookCode((PVOID*)&NextSetClipboardData)==FALSE)
			{
				DP((L"Failed to unhook SetClipboardData\n"));
			}
			else
			{
				DP((L"Unhook SetClipboardData\n"));
			}

			UnhookCode((PVOID*)&next_CoCreateInstance);

			if (next_MAPILogonEx != NULL)
			{
				UnhookAPI((PVOID*)&next_MAPILogonEx);
			}
			if (next_MAPISendMail != NULL)
			{
				UnhookAPI((PVOID*)&next_MAPISendMail);
			}
			

		CComQIPtr<IHTMLDocument2> spDoc;
            HookShowModalDialog(spDoc, FALSE);
        }
        Hooked = FALSE;
    }
	CELOG_LOG(CELOG_DUMP, L"Local variables are: Hooked=%s \n", Hooked?L"TRUE":L"FALSE" );

    return TRUE;
}

// hooked_Functions
BOOL WINAPI Detour_GetSaveFileNameW(LPOPENFILENAME lpofn)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: hwnd=%p \n", lpofn);

    if( hook_control.is_disabled() == true )
    {
        return next_GetSaveFileNameW(lpofn);
    }

    nextlabs::recursion_control_auto auto_disable(hook_control);

    if(!next_GetSaveFileNameW(lpofn)) return FALSE;

    if (NULL == lpofn)
    {
        return TRUE;
    }


    boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);

    nextlabs::eval_parms parm;
    parm.SetAction( L"COPY");
    parm.SetSrc(g_strPDFDocumentPath.c_str());
    parm.SetTarget(lpofn->lpstrFile);
	parm.SetIp(g_ip);

    if(ptr->Query(&parm, NULL))
    {
        if (ptr->IsDenied())
        {
            lpofn->lpstrFile[0] = L'\0';
            return FALSE;
        }
    }

	//handle save as
	DP((L"Handle save as, source %s, target: %s\n", g_strUploadUrl.c_str(), lpofn->lpstrFile));

	if(IsDenySaveAs(g_strUploadUrl.c_str(), lpofn->lpstrFile))
		return FALSE;

    return TRUE;
}

BOOL WINAPI Detour_InternetQueryOptionW ( HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, LPDWORD lpdwBufferLength )
{
    if( hook_control.is_disabled() == true )
    {
        return next_InternetQueryOptionW(hInternet, dwOption, lpBuffer, lpdwBufferLength);
    }

    nextlabs::recursion_control_auto auto_disable(hook_control);

	CELOG_LOG(CELOG_DUMP, L" The Parameters are: hInternet=%p, dwOption=%lu, lpBuffer=%p, lpdwBufferLength=%p \n", hInternet, dwOption,lpBuffer, lpdwBufferLength);

	BOOL bRet = next_InternetQueryOptionW(hInternet, dwOption, lpBuffer, lpdwBufferLength);

	if ( bRet && ERROR_INSUFFICIENT_BUFFER != GetLastError ( ) && INTERNET_OPTION_DATAFILE_NAME == dwOption )
	{
		DWORD BufferSize = 0;

		next_InternetQueryOptionW(hInternet, INTERNET_OPTION_URL, NULL, &BufferSize);

		if ( ERROR_INSUFFICIENT_BUFFER == GetLastError())
		{
			BufferSize++;

			WCHAR* pwzBuffer = new WCHAR[BufferSize];
			
			if ( next_InternetQueryOptionW(hInternet, INTERNET_OPTION_URL, pwzBuffer, &BufferSize))
			{
				std::map<std::wstring, std::wstring> FileTag;
				
				std::wstring Url = pwzBuffer;

				ConvertURLCharacterW(Url);

				FileTag[DownloadFileTagName] = Url;
				
				SetFileTags((WCHAR*)lpBuffer, FileTag, TRUE);
			}

			delete[] pwzBuffer;
		}
	}
	CELOG_LOG(CELOG_DUMP, L"Local variables are: bRet=%s \n", bRet?L"TRUE":L"FALSE" );

	return bRet;
}

BOOL WINAPI Detour_CopyFileW ( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists )
{
    if( hook_control.is_disabled() == true )
    {
        return next_CopyFileW( lpExistingFileName, lpNewFileName, bFailIfExists );
    }

    nextlabs::recursion_control_auto auto_disable(hook_control);

    if ( NULL != lpExistingFileName && 0 == _wcsnicmp ( IETempPath, lpExistingFileName, IETempPathLength ) )
    {	
        std::map<std::wstring, std::wstring> FileTag;

        if ( GetFileTags(lpExistingFileName, FileTag, TRUE) )
        {
            std::map<std::wstring, std::wstring>::const_iterator cit = FileTag.find(DownloadFileTagName);

            //have url tag
            if ( FileTag.end() != cit )
            {
                boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);
                nextlabs::eval_parms parm;
                parm.SetAction(L"COPY");
                parm.SetSrc(cit->second.c_str());
                parm.SetTarget(lpNewFileName);
				parm.SetIp(g_ip);

                if (ptr->Query(&parm))
                {
                    BOOL bDeny = FALSE;

                    if (ptr->IsDenied())
                    {
                        bDeny = TRUE;
                    }
                    else
                    {
                        //Do evaluation again for temp file
                        parm.SetSrc(lpExistingFileName);
						parm.SetIp(g_ip);

                        if (ptr->Query(&parm) && ptr->IsDenied())
                        {
                            bDeny = TRUE;
                        }
                    }

                    if (bDeny)
                    {
                        DeleteFileW(lpNewFileName);
                        return TRUE;
                    }
                }	
            }
        }
    }

    return next_CopyFileW( lpExistingFileName, lpNewFileName, bFailIfExists );
}

int WINAPI Detour_StartDocW( HDC hdc,const DOCINFOW *lpdi)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: hdc=%p, lpdi=%p  \n", hdc, lpdi);

	if ( NULL != pActionHandler )
	{
		std::wstring ActiveUrl = g_strUploadUrl;	

		if ( !pActionHandler->PrintAction ( lpdi, ActiveUrl.c_str() ) )
		{
			return 0;
		}
	}
	
	return next_StartDocW(hdc, lpdi);
}

BOOL WINAPI Detour_GetOpenFileNameW(LPOPENFILENAME lpofn)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: lpofn=%p \n", lpofn);

    std::wstring    strUploadUrl(g_strUploadUrl);
    ConvertURLCharacterW(strUploadUrl);
    DP((L"GetOpenFileNameW! upload url is %s\n", strUploadUrl.c_str()));
 //   BOOL bMultiple = (lpofn->Flags&OFN_ALLOWMULTISELECT)?TRUE:FALSE;
    std::wstring    strDir;
    std::wstring    strFilePath;
    std::wstring    strFileName;
    std::vector<std::wstring>   vec_uploadfiles;
    LPCWSTR pwzFile = NULL;

	memset(lpofn->lpstrFile, 0, lpofn->nMaxFile);
    if(!next_GetOpenFileNameW(lpofn))
	{
	    CELOG_LOG(CELOG_DUMP, L"Local variables are: strDir=%ls, strFilePath=%ls, strFileName=%ls, vec_uploadfiles=%p, pwzFile=%ls \n", strDir.c_str(),strFilePath.c_str(),strFileName.c_str(), &vec_uploadfiles, (pwzFile) );
        return FALSE;
	}

    // If the file path is empty, return
    if(L'\0' == lpofn->lpstrFile[0])
	{
	    CELOG_LOG(CELOG_DUMP, L"Local variables are: strDir=%ls, strFilePath=%ls, strFileName=%ls, vec_uploadfiles=%p, pwzFile=%ls \n", strDir.c_str(),strFilePath.c_str(),strFileName.c_str(), &vec_uploadfiles, (pwzFile) );
        return TRUE;
	}

	if ( NULL != pActionHandler )
	{
		std::wstring strSource;
		strSource.append(lpofn->lpstrFile, lpofn->nMaxFile);

		int begin = 0;
		begin = strSource.find(L'\0', begin);

		if (begin == 0)
		{
			return TRUE;
		}

		std::wstring strDirOrPath = strSource.substr(0, begin);

		strSource = strSource.substr(begin + 1, std::wstring::npos);
		begin = strSource.find(L'\0');
		if (begin == 0 || begin == -1)
		{
			if(!pActionHandler->UploadAction( lpofn, strUploadUrl))
			{
				lpofn->lpstrFile[0] = L'\0';
				return FALSE;
			}
			return TRUE;
		}
		else
		{
			memset(lpofn->lpstrFile, 0, lpofn->nMaxFile);
			memcpy(lpofn->lpstrFile, strDirOrPath.c_str(), wcslen(strDirOrPath.c_str())*sizeof(WCHAR));
			strDirOrPath.append(L"\\");
			LPWSTR tempAddress = lpofn->lpstrFile + wcslen(lpofn->lpstrFile) + 1;
			std::wstring strSingleTemp;
			int nCount = 0;
			do 
			{
				begin = strSource.find(L'\0');
				if (begin == 0 || begin == -1)
				{
					break;
				}
				std::wstring strEvaluation = strDirOrPath;

				if(pActionHandler->UploadAction(strEvaluation.append(strSource.c_str()), strUploadUrl))
				{
					++nCount;
					strSingleTemp.append(strEvaluation);
					memcpy(tempAddress, strSource.c_str(), wcslen(strSource.c_str()) * sizeof(WCHAR));
					++tempAddress;
					tempAddress += wcslen(tempAddress) + 1;
				}

				strSource = strSource.substr(begin + 1, std::wstring::npos);
			} while (begin != -1);
			if (nCount == 1)
			{
				memset(lpofn->lpstrFile, 0, lpofn->nMaxFile);
				memcpy(lpofn->lpstrFile, strSingleTemp.c_str(), wcslen(strSingleTemp.c_str())*sizeof(WCHAR));
			}
		}        
		CELOG_LOG(CELOG_DUMP, L"Local variables are: strDir=%ls, strFilePath=%ls, strFileName=%ls, vec_uploadfiles=%p, pwzFile=%ls \n", strDir.c_str(),strFilePath.c_str(),strFileName.c_str(), &vec_uploadfiles, (pwzFile) );
		return TRUE;
	}
   
    return TRUE;
}
extern "C" void Fire_OnSubmit(ULONG_PTR* pFunc);
long __stdcall Detour_CFormElementSubmit(ULONG_PTR p)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: p=%lu \n", p);

    if(NULL != real_fire_onsubmit)
    {
#ifdef _M_IX86
		__asm
		{
			push    ecx
			mov     ecx, [ebp+8]	// save this pointer into ecx
			call    real_fire_onsubmit
			pop     ecx
		};
#else
		Fire_OnSubmit((ULONG_PTR*)real_fire_onsubmit);
#endif
    }
    return next_CFormElementSubmit(p);
}

static CComPtr<IHTMLFrameBase2> GetFirstFrameinDoc(CComPtr<IHTMLDocument2> spDoc)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: spDoc=%p \n", &spDoc);

    HRESULT          hr           = S_OK;
    CComPtr<IHTMLFrameBase2> spFramebase2 = NULL;

    CComPtr<IHTMLElementCollection> spElemColl = 0;
    hr = spDoc->get_all(&spElemColl);
    if(SUCCEEDED(hr) && spElemColl)
    {
        long    lElems = 0;
        hr = spElemColl->get_length(&lElems);
        if(SUCCEEDED(hr) && lElems)
        {
            for(long i=0; i<lElems; i++)
            {
                CComVariant varIndex(i);
                CComPtr<IDispatch> spDisp = 0;
                hr = spElemColl->item(varIndex, varIndex, &spDisp);
                if (SUCCEEDED(hr) && spDisp)
                {
                    CComPtr<IHTMLElement>  spElem = 0;
                    hr = spDisp->QueryInterface(IID_IHTMLElement, (void**)&spElem);
                    if(SUCCEEDED(hr) && spElem)
                    {
                        hr = spElem->QueryInterface(IID_IHTMLFrameBase2,(void**)&spFramebase2);
                        if(SUCCEEDED(hr) && spFramebase2)
						{
							CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X, spFramebase2=%p \n", hr,&spFramebase2 );
                            return spFramebase2;
						}
                        else
                            spFramebase2 = NULL;
                    }
                }
            }
        }
    }
    CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X, spFramebase2=%p \n", hr,&spFramebase2 );

    return spFramebase2;
}

static CComPtr<IHTMLDocument2> GetDocument2FromFrame(CComPtr<IHTMLFrameBase2> spFrameBase2)
{
    CComPtr<IHTMLDocument2> spDoc = NULL;
    HRESULT hr = S_OK;
    CComPtr<IHTMLWindow2> spWindow2;
    hr=spFrameBase2->get_contentWindow(&spWindow2);
    if(hr==S_OK)
    {
        hr=spWindow2->get_document(&spDoc);
        if(SUCCEEDED(hr) && spDoc)
		{
		    CELOG_LOG(CELOG_DUMP, L"Local variables are:  hr=0X%08X \n",hr );
            return spDoc;
		}
        else
            spDoc = NULL;
    }
   CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X \n",hr );

    return spDoc;
}

static CComPtr<IHTMLDocument2> GetDocument2FromHwnd( HWND hPop )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: hPop=%p \n", hPop);

    HWND hwndIES = ::FindWindowEx( hPop , NULL , _T("Internet Explorer_Server") , NULL ) ;
    if(NULL == hwndIES)
        return NULL;

    CComPtr<IHTMLDocument2> spDoc = NULL;
    UINT nMsg = ::RegisterWindowMessage( _T("WM_HTML_GETOBJECT") );
    LRESULT lRes;
    ::SendMessageTimeout( hwndIES , nMsg, 0L, 0L, SMTO_ABORTIFHUNG, 1000 , (DWORD_PTR*) &lRes );

    HRESULT hr = ::ObjectFromLresult (lRes, IID_IHTMLDocument2 , 0 , (LPVOID *)&spDoc);
    if( FAILED(hr) )
        spDoc = NULL;

    return spDoc;
}

BOOL DisableInput(CComQIPtr<IHTMLDocument2> spDoc)
{
    HRESULT     hr  = S_OK;
    BOOL        bRet= FALSE;

    CComPtr<IHTMLElementCollection> spElemColl = 0;
    hr = spDoc->get_all(&spElemColl);
    if(SUCCEEDED(hr) && spElemColl)
    {
        long    lElems = 0;
        hr = spElemColl->get_length(&lElems);
        if(SUCCEEDED(hr) && lElems)
        {
            for(long i=0; i<lElems; i++)
            {
                CComVariant varIndex(i);
                CComPtr<IDispatch> spDisp = 0;
                hr = spElemColl->item(varIndex, varIndex, &spDisp);
                if (SUCCEEDED(hr) && spDisp)
                {
                    CComPtr<IHTMLElement>  spElem = 0;
                    hr = spDisp->QueryInterface(IID_IHTMLElement, (void**)&spElem);
                    if(SUCCEEDED(hr) && spElem)
                    {
                        CComPtr<IHTMLInputFileElement>  spInputFile = NULL;
                        hr = spElem->QueryInterface(IID_IHTMLInputFileElement, (void**)&spInputFile);
                        if(SUCCEEDED(hr) && spInputFile)
                        {
							CComBSTR value(L"");//empty the value of edit element of "input:file" control. Kevin Zhou 2008-4-28
							hr = spInputFile->put_value(value);

							CComBSTR attrName(L"contenteditable");
							CComVariant attrValue(L"false");
								
                            hr = spElem->setAttribute(attrName, attrValue, 0);
                            if(SUCCEEDED(hr))
                            {
                                bRet = TRUE;
                                DP((L"Succeed reset the input file attribute\n"));
                            }
                        }
                    }
                }
            }
        }
    }
	CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X, bRet=%s \n", hr, bRet?L"TRUE":L"FALSE" );

    return bRet;
}


static void ProcessOWAAttachDialog()
{
    CoInitialize(NULL);
    int nLoop = 10;
    for(int i=0; i<nLoop; i++)
    {
        Sleep(500);    // Stop 1 second to wait for the attachment dialog
        DP((L"ProcessOWAAttachDialog:: loop %d\n", i));

        HWND    hWnd = GetForegroundWindow();
        CComPtr<IHTMLDocument2> spDoc = GetDocument2FromHwnd(hWnd);
        if(NULL != spDoc)
        {
            CComPtr<IHTMLFrameBase2> spFrameBase2 = GetFirstFrameinDoc(spDoc);
            if(NULL != spFrameBase2)
            {
                CComQIPtr<IHTMLDocument2> spFrame2Doc = GetDocument2FromFrame(spFrameBase2);
                if(NULL != spFrame2Doc)
                {
                    if(DisableInput(spFrame2Doc))
                        goto error_exit;
                }
            }
        }
    }
error_exit:
    CELOG_LOG(CELOG_DUMP, L"Local variables are: nLoop=%d \n", nLoop );

    CoUninitialize();
}
long __stdcall Detour_showModalDialog(ULONG_PTR pThis, WCHAR* url , VARIANT *varArgIn, VARIANT *varOptions, VARIANT *varArgOut)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: pThis=%lu, url=%ls, varArgIn=%p, varOptions=%p, varArgOut=%p \n", pThis,  (url),varArgIn,varOptions,varArgOut);

    if( NULL!=url
        && NULL!=wcsstr(url, L"/exchange/")
        && NULL!=wcsstr(url, L"?Cmd=dialog&template=dlg_attach&ver="))
    {
        DP((L"Here is the OWA pop up attach window"));
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProcessOWAAttachDialog, 0, 0, NULL);
        if(NULL!=hThread) CloseHandle(hThread);
    }
    return next_showModalDialog(pThis, url, varArgIn, varOptions, varArgOut);
}

HRESULT WINAPI MyCoCreateInstance(
								  REFCLSID rclsid,
								  LPUNKNOWN pUnkOuter,
								  DWORD dwClsContext,
								  REFIID riid,
								  LPVOID * ppv
								  )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: rclsid=%p, pUnkOuter=%p, dwClsContext=%lu, riid=%p, ppv=%p \n", &rclsid, pUnkOuter,dwClsContext, &riid, ppv );
	HRESULT hr = next_CoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv);
	if (SUCCEEDED(hr) && (*ppv) != NULL &&
		::IsEqualCLSID(rclsid, CLSID_PrintDocumentPackageTargetFactory) &&
		::IsEqualIID(riid, IID_IPrintDocumentPackageTargetFactory))
	{

		void* vt = NULL;
		memcpy(&vt, (*ppv), sizeof(void*));  // copy first 4/8 bytes, this is vitual table pointer
		void** vTable = (void**)vt;
		LPVOID pPrintJob = vTable[3];  // get 4th virtual function pointer


		EnterCriticalSection(&g_csCom);

		if (g_COMHooks.find(pPrintJob) == g_COMHooks.end())
		{
			LPVOID* pnext_PrintJob = new LPVOID();

			if (HookCode((LPVOID)pPrintJob, (PVOID)Hooked_CreateDocumentPackageTargetForPrintJob, (LPVOID*)pnext_PrintJob) && *pnext_PrintJob)
			{
				DP((L"hook IPrintDocumentPackageTargetFactory\n"));
				g_COMHooks[pPrintJob] = *pnext_PrintJob;
			}
		}
		LeaveCriticalSection(&g_csCom);
	}

	if(SUCCEEDED(hr) && (*ppv) != NULL &&
		::IsEqualCLSID(rclsid,CLSID_FileSaveDialog) )
	{
		IFileSaveDialog* pObject = (IFileSaveDialog*)(*ppv);

		Func_FileSaveDialog theFunc = { 0 };

		theFunc.pVtable = (*(void***)pObject);

		theFunc.real_Show = (f_Show)GetVFuncAdd((LPVOID)pObject,3);

		DP((L"enter MyCoCreateInstance()\n"));

		EnterCriticalSection(&g_csCom);
		bool hooked = false;
		for(map<IFileSaveDialog*,Func_FileSaveDialog>::iterator iter = g_FileSaveDialog.begin(); iter != g_FileSaveDialog.end(); iter++)
		{
			if ((*iter).second.pVtable == theFunc.pVtable)
			{
				hooked = true;
				g_FileSaveDialog[pObject] = (*iter).second;
				break;
			}
		}
		if (!hooked)
		{
			DP((L"hook IFileSaveDialog\n"));
			LPVOID* pnext_Show = new LPVOID();
			HookCode( (PVOID)theFunc.real_Show,(PVOID)try_Show ,(PVOID*)pnext_Show) ;
			theFunc.next_Show = (f_Show)*pnext_Show;
			g_FileSaveDialog[pObject] = theFunc;
		}
		
		LeaveCriticalSection(&g_csCom);
	}
    CELOG_LOG(CELOG_DUMP, L"Local variables are: hr=0X%08X \n", hr );

	return hr;
}

bool InitResattr()
{
	static bool bInit = false;
	if(!bInit)
	{
		std::wstring strCommonPath = GetCommonComponentsDir();
		if(strCommonPath.empty())
		{
			CELOG_LOG(CELOG_DUMP, L"Local variables are: bInit=%s \n", bInit?L"TRUE":L"FALSE" );
			return false;
		}
		else
		{
			SetDllDirectoryW(strCommonPath.c_str());

			#ifdef _WIN64
				std::wstring strLib = strCommonPath + L"\\resattrlib.dll";
				std::wstring strMgr = strCommonPath + L"\\resattrmgr.dll";
			#else
				std::wstring strLib = strCommonPath + L"\\resattrlib32.dll";
				std::wstring strMgr = strCommonPath + L"\\resattrmgr32.dll";
			#endif

			HMODULE hModLib = (HMODULE)LoadLibraryW(strLib.c_str());
			HMODULE hModMgr = (HMODULE)LoadLibraryW(strMgr.c_str());

			if( !hModLib || !hModMgr)
			{
				CELOG_LOG(CELOG_DUMP, L"Local variables are: bInit=%s \n", bInit?L"TRUE":L"FALSE" );
				return false;
			}

			lfCreateAttributeManager = (CreateAttributeManagerType)GetProcAddress(hModMgr, "CreateAttributeManager");
			lfAllocAttributes = (AllocAttributesType)GetProcAddress(hModLib, "AllocAttributes");
			lfReadResourceAttributesW = (ReadResourceAttributesWType)GetProcAddress(hModMgr, "ReadResourceAttributesW");
			lfReadResourceAttributesForNTFSW = (ReadResourceAttributesForNTFSWType)GetProcAddress(hModMgr, "ReadResourceAttributesForNTFSW");
			lfGetAttributeCount = (GetAttributeCountType)GetProcAddress(hModLib, "GetAttributeCount");
			lfFreeAttributes = (FreeAttributesType)GetProcAddress(hModLib, "FreeAttributes");
			lfCloseAttributeManager = (CloseAttributeManagerType)GetProcAddress(hModMgr, "CloseAttributeManager");
			lfAddAttributeW = (AddAttributeWType)GetProcAddress(hModLib, "AddAttributeW");
			lfGetAttributeName = (GetAttributeNameType)GetProcAddress(hModLib, "GetAttributeName");
			lfGetAttributeValue = (GetAttributeValueType)GetProcAddress(hModLib, "GetAttributeValue");
			lfWriteResourceAttributesW = (WriteResourceAttributesWType)GetProcAddress(hModMgr, "WriteResourceAttributesW");
			lfWriteResourceAttributesForNTFSW = (WriteResourceAttributesForNTFSWType)GetProcAddress(hModMgr, "WriteResourceAttributesForNTFSW");

			if( !(lfCreateAttributeManager && lfAllocAttributes &&
				lfReadResourceAttributesW && lfGetAttributeCount &&
				lfFreeAttributes && lfCloseAttributeManager && lfAddAttributeW &&
				lfGetAttributeName && lfGetAttributeValue &&
				lfWriteResourceAttributesW && lfWriteResourceAttributesForNTFSW && lfReadResourceAttributesForNTFSW ) )
			{
				CELOG_LOG(CELOG_DUMP, L"Local variables are: bInit=%s \n", bInit?L"TRUE":L"FALSE" );
				return false;
			}

			bInit = true;
			CELOG_LOG(CELOG_DUMP, L"Local variables are: bInit=%s \n", bInit?L"TRUE":L"FALSE" );
			return true;
		}
	}
	CELOG_LOG(CELOG_DUMP, L"Local variables are: bInit=%s \n", bInit?L"TRUE":L"FALSE" );

	return bInit;
}

BOOL SetFileTags ( LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags, BOOL NTFSStream )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: lpszFilePath=%ls, mapTags=%p, NTFSStream=%s \n",  (lpszFilePath), &mapTags, NTFSStream?L"TRUE":L"FALSE");

	if ( NULL == lpszFilePath || !InitResattr ( ) )
	{
		return FALSE;
	}

	ResourceAttributeManager *mgr = NULL;
	lfCreateAttributeManager(&mgr);

	if(!mgr)
		return FALSE;

	ResourceAttributes *attrs;
	lfAllocAttributes(&attrs);

	int nRet = 0;
	if(attrs)
	{
		std::map<std::wstring, std::wstring>::iterator itr;
		for(itr = mapTags.begin(); itr != mapTags.end(); itr++)
		{
			lfAddAttributeW(attrs, (*itr).first.c_str(), (*itr).second.c_str());
		}

		if ( NTFSStream )
		{
			nRet = lfWriteResourceAttributesForNTFSW(mgr, lpszFilePath, attrs);
		}
		else
		{
			nRet = lfWriteResourceAttributesW(mgr, lpszFilePath, attrs);
		}

		lfFreeAttributes(attrs);
	}

	if(mgr)
	{
		lfCloseAttributeManager(mgr);
		mgr = NULL;
	}

	return TRUE;
}

BOOL GetFileTags ( LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags, BOOL NTFSStream )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: lpszFilePath=%ls, mapTags=%p, NTFSStream=%s \n",  (lpszFilePath), &mapTags, NTFSStream?L"TRUE":L"FALSE");

	if ( NULL == lpszFilePath || !InitResattr ( ) )
	{
		return FALSE;
	}

	mapTags.clear ( );

	ResourceAttributeManager *mgr = NULL;
	lfCreateAttributeManager(&mgr);

	ResourceAttributes *attrs;
	lfAllocAttributes(&attrs);

	if(!mgr || !attrs)
		return FALSE;

	BOOL bRet = FALSE;
	if(attrs)
	{
		int nRet = 0;

		if ( NTFSStream )
		{
			nRet = lfReadResourceAttributesForNTFSW(mgr, lpszFilePath, attrs);
		}
		else
		{
			nRet = lfReadResourceAttributesW(mgr, lpszFilePath, attrs);
		}
			
		int size = lfGetAttributeCount(attrs);

		for (int i = 0; i < size; ++i)
		{
			WCHAR *tagName = (WCHAR *)lfGetAttributeName(attrs, i);
			WCHAR *tagValue = (WCHAR *)lfGetAttributeValue(attrs, i);

			if(tagName && tagValue)
			{
				mapTags[tagName] = tagValue;
				bRet = TRUE;
			}
		}
	}

	lfFreeAttributes(attrs);
	lfCloseAttributeManager(mgr);

	return bRet;
}
