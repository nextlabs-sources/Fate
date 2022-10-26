#include "stdafx.h"
#include <string>
#include <tchar.h>
#include <Strsafe.h>
#include <MAPI.h>
#include <vector>
#include "SvrAgent.h"

#pragma warning(push)
#pragma warning(disable:4819)
#include "madCHook.h"
#pragma warning(pop)
#include "nlexcept.h"
#ifdef _WIN64
#pragma comment(lib,"madCHook64.lib")
#else
#pragma comment(lib,"madCHook32.lib")
#endif

#ifndef  MapiFileDescW
typedef struct
{
    ULONG ulReserved;
    ULONG flFlags;
    ULONG nPosition;
    PWSTR lpszPathName;
    PWSTR lpszFileName;
    PVOID lpFileType;
} MapiFileDescW, *lpMapiFileDescW;

typedef struct
{
    ULONG ulReserved;
    ULONG ulRecipClass;
    PWSTR lpszName;
    PWSTR lpszAddress;
    ULONG ulEIDSize;
    PVOID lpEntryID;
} MapiRecipDescW, *lpMapiRecipDescW;

typedef struct
{
    ULONG ulReserved;
    PWSTR lpszSubject;
    PWSTR lpszNoteText;
    PWSTR lpszMessageType;
    PWSTR lpszDateReceived;
    PWSTR lpszConversationID;
    FLAGS flFlags;
    lpMapiRecipDescW lpOriginator;
    ULONG nRecipCount;
    lpMapiRecipDescW lpRecips;
    ULONG nFileCount;
    lpMapiFileDescW lpFiles;
} MapiMessageW, *lpMapiMessageW;
#endif
//////////////////////////////////////////////////////////////////////////
static CComPtr<IDispatch> g_pApp=NULL;
static	std::wstring g_strFullPath; 
static HMODULE g_hModule=NULL;
static HMODULE g_hMapi=NULL;

// added at 10-08-2008 17:00 in order to remove the unattached file from list in case of OE match failed
//////////////////////////////////////////////////////////////
std::vector<std::wstring> g_vecOutNoSupFile;
//////////////////////////////////////////////////////////////

typedef std::pair<void*,void* > FuncPair;

static int exception_state=0;
LONG (WINAPI* gDetourTransactionBegin)() = NULL;
LONG (WINAPI* gDetourTransactionCommit)() = NULL;
LONG (WINAPI* gDetourUpdateThread)(HANDLE hThread) = NULL;
LONG (WINAPI* gDetourAttach)(PVOID *ppPointer,PVOID pDetour) = NULL;
LONG (WINAPI* gDetourDetach)(PVOID *ppPointer,PVOID pDetour) = NULL;
//////////////////////////////////////////////////////////////////////////

typedef ULONG (_stdcall* LPMAPISendMail)(LHANDLE lhSession,  ULONG ulUIParam,  lpMapiMessage lpMessage,  FLAGS flFlags,  ULONG ulReserved);
LPMAPISendMail	g_OldSendMail=NULL;
typedef ULONG(WINAPI* LPMAPISendMailW)(LHANDLE lhSession,ULONG_PTR ulUIParam,lpMapiMessageW lpMessage,FLAGS flFlags,ULONG ulReserved);
LPMAPISendMailW g_OldSendMailW = NULL;
//////////////////////////////////////////////////////////////////////////
// TRUE: supported, otherwise return FALSE
static BOOL IsSupportedFile(const std::wstring& strName)
{
    // modified it into following at 10-08-2008 17:00 in order to remove the unattached file from list in case of OE match failed
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (g_vecOutNoSupFile.empty())
    {
        g_vecOutNoSupFile.push_back(L".ADE");
        g_vecOutNoSupFile.push_back(L".ADP");
        g_vecOutNoSupFile.push_back(L".APP");
        g_vecOutNoSupFile.push_back(L".ASP");
        g_vecOutNoSupFile.push_back(L".BAS");
        g_vecOutNoSupFile.push_back(L".BAT");
        g_vecOutNoSupFile.push_back(L".CER");
        g_vecOutNoSupFile.push_back(L".CHM");
        g_vecOutNoSupFile.push_back(L".CMD");
        g_vecOutNoSupFile.push_back(L".COM");
        g_vecOutNoSupFile.push_back(L".CPL");
        g_vecOutNoSupFile.push_back(L".CRT");
        g_vecOutNoSupFile.push_back(L".CSH");
        g_vecOutNoSupFile.push_back(L".DER");
        g_vecOutNoSupFile.push_back(L".EXE");
        g_vecOutNoSupFile.push_back(L".FXP");
        g_vecOutNoSupFile.push_back(L".HLP");
        g_vecOutNoSupFile.push_back(L".HTA");
        g_vecOutNoSupFile.push_back(L".INF");
        g_vecOutNoSupFile.push_back(L".INS");
        g_vecOutNoSupFile.push_back(L".ISP");
        g_vecOutNoSupFile.push_back(L".ITS");
        g_vecOutNoSupFile.push_back(L".JS");
        g_vecOutNoSupFile.push_back(L".JSE");
        g_vecOutNoSupFile.push_back(L".KSH");
        g_vecOutNoSupFile.push_back(L".LNK");
        g_vecOutNoSupFile.push_back(L".MAD");
        g_vecOutNoSupFile.push_back(L".MAF");
        g_vecOutNoSupFile.push_back(L".MAG");
        g_vecOutNoSupFile.push_back(L".MAM");
        g_vecOutNoSupFile.push_back(L".MAQ");
        g_vecOutNoSupFile.push_back(L".MAR");
        g_vecOutNoSupFile.push_back(L".MAS");
        g_vecOutNoSupFile.push_back(L".MAT");
        g_vecOutNoSupFile.push_back(L".MAU");
        g_vecOutNoSupFile.push_back(L".MAV");
        g_vecOutNoSupFile.push_back(L".MAW");
        g_vecOutNoSupFile.push_back(L".MDA");
        g_vecOutNoSupFile.push_back(L".MDB");
        g_vecOutNoSupFile.push_back(L".MDE");
        g_vecOutNoSupFile.push_back(L".MDT");
        g_vecOutNoSupFile.push_back(L".MDW");
        g_vecOutNoSupFile.push_back(L".MDZ");
        g_vecOutNoSupFile.push_back(L".MSC");
        g_vecOutNoSupFile.push_back(L".MSH");
        g_vecOutNoSupFile.push_back(L".MSH1");
        g_vecOutNoSupFile.push_back(L".MSH2");
        g_vecOutNoSupFile.push_back(L".MSHXML");
        g_vecOutNoSupFile.push_back(L".MSH1XML");
        g_vecOutNoSupFile.push_back(L".MSH2XML");
        g_vecOutNoSupFile.push_back(L".MSI");
        g_vecOutNoSupFile.push_back(L".MSP");
        g_vecOutNoSupFile.push_back(L".MST");
        g_vecOutNoSupFile.push_back(L".OPS");
        g_vecOutNoSupFile.push_back(L".PCD");
        g_vecOutNoSupFile.push_back(L".PIF");
        g_vecOutNoSupFile.push_back(L".PLG");
        g_vecOutNoSupFile.push_back(L".PRF");
        g_vecOutNoSupFile.push_back(L".PRG");
        g_vecOutNoSupFile.push_back(L".PST");
        g_vecOutNoSupFile.push_back(L".REG");
        g_vecOutNoSupFile.push_back(L".SCF");
        g_vecOutNoSupFile.push_back(L".SCR");
        g_vecOutNoSupFile.push_back(L".SCT");
        g_vecOutNoSupFile.push_back(L".SHB");
        g_vecOutNoSupFile.push_back(L".SHS");
        g_vecOutNoSupFile.push_back(L".TMP");
        g_vecOutNoSupFile.push_back(L".URL");
        g_vecOutNoSupFile.push_back(L".VB");
        g_vecOutNoSupFile.push_back(L".VBE");
        g_vecOutNoSupFile.push_back(L".VBS");
        g_vecOutNoSupFile.push_back(L".VSMACROS");
        g_vecOutNoSupFile.push_back(L".VSW");
        g_vecOutNoSupFile.push_back(L".WS");
        g_vecOutNoSupFile.push_back(L".WSC");
        g_vecOutNoSupFile.push_back(L".WSF");
        g_vecOutNoSupFile.push_back(L".WSH");
    }

    LPCWSTR pSuffix = wcsrchr(strName.c_str(), L'.');
    BOOL    bSupport = TRUE;

    if (NULL != pSuffix)
    {
        for (size_t iv = 0; iv < g_vecOutNoSupFile.size(); iv++)
        {
            if (_wcsicmp(pSuffix, g_vecOutNoSupFile[iv].c_str()) == 0)
            {
                bSupport = FALSE;
                break;
            }
        }
    }
    return bSupport;
}
ULONG _stdcall g_NewSendMailCore(LHANDLE lhSession,  ULONG ulUIParam,  lpMapiMessage lpMessage,  FLAGS flFlags,  ULONG ulReserved)
{
    if (lpMessage == NULL || lpMessage->lpFiles == NULL || lpMessage->lpszSubject == NULL || lpMessage->nFileCount < 1 || lpMessage->lpFiles->lpszPathName == NULL)
        return g_OldSendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);

	int nAttachment = lpMessage->nFileCount;
	lpMapiFileDesc p = lpMessage->lpFiles;

	wchar_t strTemp[512]=L"\0";
	MultiByteToWideChar( CP_ACP, 0, (char*)lpMessage->lpszSubject,
		(int)strlen((char*)lpMessage->lpszSubject)+1, strTemp,   
		512 );
	KeyWords theNode;
	theNode.strSubject = strTemp;
	theNode.uAttachmentCount=nAttachment;
	std::wstring strName=L"",strPath=L"";
	for (int i=0; i<(int)lpMessage->nFileCount; i++)
	{
		if(p && p->lpszPathName) 
		{
			MultiByteToWideChar( CP_ACP, 0, (char*)p->lpszPathName,
				(int)strlen((char*)p->lpszPathName)+1, strTemp,   
				512 );
			strPath = strTemp;

            char* pName = p->lpszFileName;
            if (pName == NULL)
            {
                pName = strrchr(p->lpszPathName, '\\');
                if (pName == NULL)  pName = strrchr(p->lpszPathName, L'/');
                if (pName == NULL)  pName = p->lpszPathName;
                else pName++;
            }

            if (pName != NULL)
            {
                MultiByteToWideChar(CP_ACP, 0, (char*)pName,
                    (int)strlen((char*)pName) + 1, strTemp,
                    512);
                strName = strTemp;
            }
            BOOL bSupport = IsSupportedFile(strName);
			
			if (bSupport)
			{
                wchar_t strInfo[4097] = { 0 };
                StringCchPrintfW(strInfo, 4096, L"==========> send file of %s from explore to outllook, subject is: %s, path is: %s.\n",
                    strName.c_str(), theNode.strSubject.c_str(), strPath.c_str());
                OutputDebugStringW(strInfo);
				theNode.vecFiles.push_back(FilePair(strName,strPath));
			}
		}
		p++;
	}
	if(theNode.vecFiles.size() > 0)
		CTransferInfo::put_FileInfo(theNode.strSubject.c_str(),(int)theNode.vecFiles.size(),theNode.vecFiles);

	return g_OldSendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
}
ULONG _stdcall g_NewSendMailCoreW(LHANDLE lhSession, ULONG_PTR ulUIParam, lpMapiMessageW lpMessage, FLAGS flFlags, ULONG ulReserved)
{
    if (lpMessage == NULL || lpMessage->lpFiles == NULL || lpMessage->lpszSubject == NULL || lpMessage->nFileCount < 1 || lpMessage->lpFiles->lpszPathName == NULL)
        return g_OldSendMailW(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);

    int nAttachment = lpMessage->nFileCount;
    lpMapiFileDescW p = lpMessage->lpFiles;

    KeyWords theNode;
    theNode.strSubject = lpMessage->lpszSubject;
    theNode.uAttachmentCount = nAttachment;
    std::wstring strName = L"" , strPath=L"";
    for (int i = 0; i < (int)lpMessage->nFileCount  && p != NULL; i++)
    {
        if (p && p->lpszPathName)
        {
            strPath = p->lpszPathName;
            if (p->lpszFileName != NULL)    strName = p->lpszFileName;
            else
            {
                const wchar_t* pName = wcsrchr(p->lpszPathName, L'\\');
                if (pName == NULL)  pName = wcsrchr(p->lpszPathName, L'/');
                if (pName == NULL)  strName = strPath;
                else strName = pName + 1;
            }
            BOOL bSupport = IsSupportedFile(strName);
            if (bSupport)
            {
                wchar_t strInfo[4097] = { 0 };
                StringCchPrintfW(strInfo, 4096, L"==========> send file of %s from explore to outllook, subject is: %s, path is: %s.\n", 
                    strName.c_str(), theNode.strSubject.c_str(), strPath.c_str());
                OutputDebugStringW(strInfo);
                theNode.vecFiles.push_back(FilePair(strName, strPath));
            }
        }
        p++;
    }
    if (theNode.vecFiles.size() > 0)
        CTransferInfo::put_FileInfo(theNode.strSubject.c_str(), (int)theNode.vecFiles.size(), theNode.vecFiles);

    return g_OldSendMailW(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
}
ULONG _stdcall g_NewSendMail(LHANDLE lhSession, ULONG ulUIParam, lpMapiMessage lpMessage, FLAGS flFlags, ULONG ulReserved)
{
	__try
	{
		return g_NewSendMailCore(lhSession,ulUIParam,lpMessage,flFlags,ulReserved);
	}
	__except( NLEXCEPT_FILTER_EX(&exception_state) )
	{
		/* empty */
        ;
	}
	return 0; /* fail */
}
ULONG _stdcall g_NewSendMailW(LHANDLE lhSession, ULONG_PTR ulUIParam, lpMapiMessageW lpMessage, FLAGS flFlags, ULONG ulReserved)
{
    __try
    {
        if(g_NewSendMailW != NULL)  return g_NewSendMailCoreW(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
    }
    __except (NLEXCEPT_FILTER_EX(&exception_state))
    {
        /* empty */
        ;
    }
    return MAPI_E_FAILURE; /* fail */
}
//////////////////////////////////////////////////////////////////////////
bool GetOSInfo(DWORD& dwMajor, DWORD& dwMinor)
{
    static DWORD sMajor = 0;
    static DWORD sMinor = 0;

    if (sMajor == 0 && sMinor == 0)
    {
        OSVERSIONINFOEX osvi;
        BOOL bOsVersionInfoEx;

        // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
        // If that fails, try using the OSVERSIONINFO structure.
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

#pragma warning( push )
#pragma warning( disable: 4996 )    // Function GetVersionEx is deprecated, we can use IsWindows10OrGreater to instead, but this only can used in win10+ OS
        bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi);
        if (!bOsVersionInfoEx)
        {
            // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            if (!GetVersionEx((OSVERSIONINFO *)&osvi))
            {
                return false;
            }
        }
#pragma warning( pop )

        sMajor = osvi.dwMajorVersion;
        sMinor = osvi.dwMinorVersion;

    }

    //5,0 win2k, 5,1 winxp
    dwMajor = sMajor;
    dwMinor = sMinor;

    return true;
}

// versions: 6.2
bool IsWin10(void)
{
    static DWORD dwXPMajor = 0;
    static DWORD dwXPMinor = 0;
    if ((6 < dwXPMajor) || ((6 == dwXPMajor) && (2 <= dwXPMinor)))
    {
        return true;
    }
    else if ((0 != dwXPMajor) || (0 != dwXPMinor))
    {
        return false;
    }

    if (GetOSInfo(dwXPMajor, dwXPMinor) && ((6 < dwXPMajor) || (6 == dwXPMajor) && (2 <= dwXPMinor)))
    {
        return true;
    }
    return false;
}
//////////////////////////////////////////////////////////////////////////
bool SetHook()
{
    InitializeMadCHook();

    if (GetModuleHandleW(L"MAPI32.dll") == NULL)
    {
        ::LoadLibraryW(L"MAPI32.dll");
    }
    if (IsWin10())
    {
        (void)HookAPI("MAPI32.dll", "MAPISendMailW", g_NewSendMailW, (PVOID*)&g_OldSendMailW);
    }
    else
    {
        (void)HookAPI("MAPI32.dll", "MAPISendMail", g_NewSendMail, (PVOID*)&g_OldSendMail);
    }

	return true;
}
bool UnsetHook()
{    

    if (IsWin10() && g_OldSendMailW != NULL) UnhookAPI((PVOID*)&g_OldSendMailW);
    else if (g_OldSendMail!=NULL)	UnhookAPI((PVOID*)&g_OldSendMail);

FinalizeMadCHook();
	return true;
}