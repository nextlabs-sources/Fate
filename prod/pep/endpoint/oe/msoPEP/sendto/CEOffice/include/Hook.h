#include "stdafx.h"
#include <string>

#include <MAPI.h>
#include <vector>
#include "SvrAgent.h"

#pragma warning(push)
#pragma warning(disable:4819)
#include "madCHook.h"
#pragma warning(pop)

#include "nlexcept.h"
#include "strsafe.h"

#ifdef _M_X64
	#pragma comment(lib,"madCHook64.lib")
#else
	#pragma comment(lib,"madCHook32.lib")
#endif


//////////////////////////////////////////////////////////////////////////
static CComPtr<IDispatch> g_pApp=NULL;

////added for marshaling application interfaces
IStream* g_pStream = NULL;
const IID IID_WordApplication =    {0x00020970,0,0,{0xc0,0,0,0,0,0,0,0x46}};
const IID IID_ExcelApplication =   {0x000208d5,0,0,{0xc0,0,0,0,0,0,0,0x46}};
const IID IID_PPTApplication =    {0x91493442,0x5a91,0x11cf,{0x87,0,0,0xaa,0,0x60,0x26,0x3b}};
const wchar_t* g_pkwchVersion2013  = L"15.0";

IID GetAppIIDFromDocType(const tagDocType dtType)
{
	if (dtType == tagExcel)
	{
		return IID_ExcelApplication;
	}
	else if (dtType == tagPPT)
	{
		return IID_PPTApplication;
	}
	return IID_WordApplication;
}

static int exception_state=0;
HRESULT AutoWrap(int autoType, VARIANT *pvResult, CComPtr<IDispatch> pDisp, LPOLESTR ptName, int cArgs...)
{
	// Variables used...
	DISPPARAMS  dp          = { NULL, NULL, 0, 0 };
	DISPID      dispidNamed = DISPID_PROPERTYPUT;
	DISPID      dispID;
	HRESULT     hr;
    char        szName[MAX_PATH+1] = {0};

	// Convert down to ANSI
	WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, MAX_PATH, NULL, NULL);

	// Get DISPID for name passed...
	hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
	if(FAILED(hr))
	{
		return hr;
	}

	// Allocate memory for arguments...
	VARIANT *pArgs = new VARIANT[cArgs+1];

	// Extract arguments...
	// Begin variable-argument list...
	va_list marker;
	va_start(marker, cArgs);
	
	for(int i=0; i<cArgs; i++) {
		pArgs[i] = va_arg(marker, VARIANT);
	}
	// End variable-argument section...
	va_end(marker);

	// Build DISPPARAMS
	dp.cArgs = cArgs;
	dp.rgvarg = pArgs;

	// Handle special-case for property-puts!
	if(autoType & DISPATCH_PROPERTYPUT)
	{
		dp.cNamedArgs = 1;
		dp.rgdispidNamedArgs = &dispidNamed;
	}

	// Make the call!
	hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, (WORD)autoType, &dp, pvResult, NULL, NULL);
	delete [] pArgs;
	return hr;
}
tagDocType SetType()
{
	static tagDocType theDocType = tagUnknown;
	if(theDocType != tagUnknown)	return theDocType;
	ATLASSERT(g_pApp);
	CComVariant varResult;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,g_pApp,L"Name",0);
	if(SUCCEEDED(hr))
	{
		std::wstring strName = varResult.bstrVal;
		if(strName.find(L"Word") != std::wstring::npos)
		{
			theDocType = tagWord;
		}
		else if(strName.find(L"Excel") != std::wstring::npos)
		{
			theDocType = tagExcel;
		}
		else if(strName.find(L"PowerPoint") != std::wstring::npos)
		{
			theDocType = tagPPT;
		}
	}
	return theDocType;
}

bool IsOfficeVersion2013()
{
    if (NULL != g_pApp)
    {
        ATL::CComVariant theResult;
        HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &theResult, g_pApp, L"Version", 0);
        if (SUCCEEDED(hr))
        {
            std::wstring strVersion = theResult.bstrVal;
            if (0 == _wcsicmp(strVersion.c_str(), g_pkwchVersion2013))
            {
                return true;
            }
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////

typedef ULONG (_stdcall* LPMAPISendMail)(LHANDLE lhSession,  ULONG ulUIParam,  lpMapiMessage lpMessage,  FLAGS flFlags,  ULONG ulReserved);
LPMAPISendMail	g_OldSendMail=NULL;
LPMAPISendMail	g_OldSendMailW=NULL;

ULONG _stdcall g_NewSendMailCore(LHANDLE lhSession,  ULONG ulUIParam,  lpMapiMessage lpMessage,  FLAGS flFlags,  ULONG ulReserved)
{
	if (lpMessage == NULL || lpMessage->lpFiles == NULL || lpMessage->lpFiles->lpszFileName == NULL)
		return g_OldSendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);

#define MAX_PATH_NEW 1024
	KeyWords theNode;
	HRESULT hr = S_OK;
	wchar_t strTemp[MAX_PATH_NEW]={0};
	MultiByteToWideChar( CP_ACP, 0, (char*)lpMessage->lpszSubject,
		(int)strlen((char*)lpMessage->lpszSubject)+1, strTemp,   
		MAX_PATH_NEW );

	theNode.strSubject = strTemp;
	theNode.uAttachmentCount = lpMessage->nFileCount;
	lpMapiFileDesc pFileDesc = lpMessage->lpFiles;

	strTemp[0]='\0';
	MultiByteToWideChar( CP_ACP, 0, (char*)pFileDesc->lpszFileName,
		(int)strlen((char*)pFileDesc->lpszFileName)+1, strTemp,   
		MAX_PATH_NEW );

    wchar_t* szAppType = NULL;
	tagDocType theDocType = SetType();
    switch (theDocType)
    {
    case tagWord:
        szAppType = L"ActiveDocument";
        break;
    case tagExcel:
        szAppType = L"ActiveWorkbook";
        break;
    case tagPPT:
        szAppType = L"ActivePresentation";
        break;
    }
    if (szAppType == NULL)  return g_OldSendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);

	std::wstring strname = strTemp;
	std::wstring strFullName = L"";
	CComPtr<IDispatch> pAppDisp = NULL;

    if (g_pStream != NULL)
    {
        hr = ::CoGetInterfaceAndReleaseStream(g_pStream, GetAppIIDFromDocType(theDocType), (LPVOID*)&pAppDisp);

        if (FAILED(hr) || pAppDisp == NULL)
        {
            OutputDebugStringW(L"\nunmarshall failed\n");
            pAppDisp = g_pApp;
        }
        else
        {
            g_pStream = NULL;
        }

    }
    CComVariant varResult;
    hr = AutoWrap(DISPATCH_PROPERTYGET, &varResult, pAppDisp, szAppType, 0);
    if (SUCCEEDED(hr) && varResult.pdispVal != NULL)
    {
        CComPtr<IDispatch> pDoc = varResult.pdispVal;
        varResult.Clear();
        hr = AutoWrap(DISPATCH_PROPERTYGET, &varResult, pDoc, L"FullName", 0);
        if (SUCCEEDED(hr) && varResult.bstrVal != NULL)
        {
            strFullName = varResult.bstrVal;
        }
    }

	hr=	::CoMarshalInterThreadInterfaceInStream(GetAppIIDFromDocType(theDocType), pAppDisp, &g_pStream); 
	if (FAILED(hr) || g_pStream == NULL)
	{
		OutputDebugStringW(L"\nMarshall failed after send email-\n");
	}

	//////////////////////////////////////////////////////////////////////////////////////////		//Modify at 09/24/2008 15:30
	if(strFullName.find(L"\\") == std::wstring::npos && strFullName.find(L"/") == std::wstring::npos)	// need to matiain
	{
		// get temp path because it's a new document but not be saved 
		// we need use temp filepath
		strTemp[0]='\0';
		MultiByteToWideChar( CP_ACP, 0, (char*)pFileDesc->lpszPathName,
			static_cast<int>(strlen((char*)pFileDesc->lpszPathName)+1), strTemp,   
			MAX_PATH_NEW );

		const wchar_t* pSuffixName = wcsrchr(strname.c_str(), L'.') ;
		if (pSuffixName == NULL)
		{
			pSuffixName = L"";
		}

		if ((0 == _wcsicmp(wcsrchr(strTemp, L'.'), L".TMP")&& 0 == _wcsicmp(pSuffixName, L".XLS"))
			||( 0 == _wcsicmp(pSuffixName, L".PPT") && 0 != _wcsicmp(strname.c_str(), wcsrchr(strTemp, L'\\')+1)))
		{
			std::wstring wstrDst(strTemp);
			wstrDst = wstrDst.substr(0, wstrDst.rfind(L'\\') + 1).append(strname);
			::CopyFileW(strTemp, wstrDst.c_str(), FALSE);
			strFullName = wstrDst;
		}
		else
		{
			strFullName = strTemp;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////

	if(!strFullName.empty())
	{
		theNode.vecFiles.push_back(FilePair(strname,strFullName));
		CTransferInfo::put_FileInfo(theNode.strSubject.c_str(),theNode.uAttachmentCount,theNode.vecFiles);
        wchar_t buf[1024] = {0};
        wnsprintf(buf, 1024, L"CEOffice--put_FileInfo:Subject = %s, strname = %s, strFullName = %s\n", 
            theNode.strSubject.c_str(), strname.c_str(), strFullName.c_str());
        ::OutputDebugStringW(buf);
    }
	return g_OldSendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
}

// Add for Powerpoint 2013 some version hook OLMAPI32.dll MAPISendMailW function.
ULONG _stdcall g_NewSendMailCoreW(LHANDLE lhSession,  ULONG ulUIParam,  lpMapiMessage lpMessage,  FLAGS flFlags,  ULONG ulReserved)
{
    if (lpMessage == NULL || lpMessage->lpFiles == NULL || lpMessage->lpFiles->lpszFileName == NULL)
        return g_OldSendMailW(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);

    KeyWords theNode;
    HRESULT hr = S_OK;
    theNode.strSubject = (wchar_t*)(lpMessage->lpszSubject);
    theNode.uAttachmentCount = lpMessage->nFileCount;
    lpMapiFileDesc pFileDesc = lpMessage->lpFiles;

    wchar_t* szAppType = L"ActivePresentation";
    std::wstring strname = (wchar_t*)(pFileDesc->lpszFileName);
    std::wstring strFullName = L"";
    CComVariant varResult;
    hr = AutoWrap(DISPATCH_PROPERTYGET, &varResult, g_pApp, szAppType, 0);
    if (SUCCEEDED(hr) && varResult.pdispVal != NULL)
    {
        CComPtr<IDispatch> pDoc = varResult.pdispVal;
        varResult.Clear();
        hr = AutoWrap(DISPATCH_PROPERTYGET, &varResult, pDoc, L"FullName", 0);
        if (SUCCEEDED(hr) && varResult.bstrVal != NULL)
        {
            strFullName = varResult.bstrVal;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////		//Modify at 09/24/2008 15:30
    if(strFullName.find(L"\\") == std::wstring::npos && strFullName.find(L"/") == std::wstring::npos)	// need to matiain
    {
        // get temp path because it's a new document but not be saved 
        // we need use temp filepath
        if(!strname.empty())
        {
            std::wstring strTemp = (wchar_t*)pFileDesc->lpszPathName;
            const wchar_t* pSuffixName = wcsrchr(strname.c_str(), L'.');
            if(!strTemp.empty() && pSuffixName[0] != '\0')
            {
                if ((0 == _wcsicmp(wcsrchr(strTemp.c_str(), L'.'), L".TMP")&& 0 == _wcsicmp(pSuffixName, L".XLS"))
                    ||( 0 == _wcsicmp(pSuffixName, L".PPT") && 0 != _wcsicmp(strname.c_str(), wcsrchr(strTemp.c_str(), L'\\')+1)))
                {
                    std::wstring wstrDst(strTemp);
                    wstrDst = wstrDst.substr(0, wstrDst.rfind(L'\\') + 1).append(strname);
                    ::CopyFileW(strTemp.c_str(), wstrDst.c_str(), FALSE);
                    strFullName = wstrDst;
                }
                else
                {
                    strFullName = strTemp;
                }
            }
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////

    if(!strFullName.empty())
    {
        theNode.vecFiles.push_back(FilePair(strname,strFullName));
        CTransferInfo::put_FileInfo(theNode.strSubject.c_str(),theNode.uAttachmentCount,theNode.vecFiles);
        wchar_t buf[1024] = {0};
        wnsprintf(buf, 1024, L"CEOffice--put_FileInfo:Subject = %s, strname = %s, strFullName = %s\n", 
            theNode.strSubject.c_str(), strname.c_str(), strFullName.c_str());
        ::OutputDebugStringW(buf);
    }
    return g_OldSendMailW(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
}

ULONG _stdcall g_NewSendMail(LHANDLE lhSession,  ULONG ulUIParam,  lpMapiMessage lpMessage,  FLAGS flFlags,  ULONG ulReserved)
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
//////////////////////////////////////////////////////////////////////////

ULONG _stdcall g_NewSendMailW(LHANDLE lhSession,  ULONG ulUIParam,  lpMapiMessage lpMessage,  FLAGS flFlags,  ULONG ulReserved)
{
    __try
    {
        return g_NewSendMailCoreW(lhSession,ulUIParam,lpMessage,flFlags,ulReserved);
    }
    __except( NLEXCEPT_FILTER_EX(&exception_state) )
    {
        /* empty */
        ;
    }
    return 0; /* fail */
}

bool SetHook()
{
	InitializeMadCHook() ;
    tagDocType theDocType = SetType();
    // For Powerpoint 2013, different mini versions call different functions.
    // Just call one of MAPISendMailW and MAPISendMail to send email.
    if(IsOfficeVersion2013() && tagPPT == theDocType)
    {
        HookAPI("OLMAPI32.dll","MAPISendMailW",g_NewSendMailW,(PVOID*)&g_OldSendMailW);
    }

    HookAPI("MAPI32.dll","MAPISendMail",g_NewSendMail,(PVOID*)&g_OldSendMail);

	return true;
}

bool UnsetHook()
{
	if( g_OldSendMail!= NULL )
	{
		UnhookAPI((PVOID*)&g_OldSendMail);
	}
    
    if( g_OldSendMailW!= NULL )
    {
        UnhookAPI((PVOID*)&g_OldSendMailW);
    }
	FinalizeMadCHook() ;

  return true;
}
