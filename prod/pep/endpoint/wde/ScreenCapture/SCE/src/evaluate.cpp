#include "stdafx.h"
#include "evaluate.h"
#include "eframework/platform/cesdk_loader.hpp"
#include "nlconfig.hpp"

#include <Sddl.h>

nextlabs::cesdk_loader cesdk;

#pragma warning(push)
#pragma warning(disable: 4995) 
#include "celog.h"
#pragma warning(pop)

extern CELog  g_log;

static std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
		return szDir;
	}

	return L"";
}

void GetFQDN(LPCWSTR hostname, LPWSTR fqdn, int nSize)
{
    char szHostName[1001] = {0};
    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, hostname, (int)wcslen(hostname), szHostName, 1000, NULL, NULL);

    hostent* hostinfo;
    hostinfo = gethostbyname(szHostName);
    if(hostinfo && hostinfo->h_name)
    {
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, hostinfo->h_name, (int)strlen(hostinfo->h_name), fqdn, nSize);
    }
    else
    {
        wcsncpy_s(fqdn, nSize, hostname, _TRUNCATE);
    }
}

#define NETMAP_HEAD	L"\\Device\\LanmanRedirector\\;"

#ifdef LONG_PTR
#undef LONG_PTR
#endif
#define LONG_PTR LONG

PolicyCommunicator::PolicyCommunicator()
{
    memset( m_wzSID, 0, sizeof( m_wzSID ) );
    memset( m_wzUserName, 0, sizeof( m_wzUserName ) );
    memset( m_wzHostName, 0, sizeof( m_wzHostName ) );
    memset( m_wzAppName, 0, sizeof( m_wzAppName ) );
    memset( m_wzAppPath, 0, sizeof( m_wzAppPath ) );
    m_connectHandle = NULL;
    m_ulIp = 0;

    InitData();

	if( cesdk.is_loaded() == false )
	{
		g_log.Log(CELOG_DEBUG,"load sdk: loading!\n");

		std::wstring wstrDir = GetCommonComponentsDir();

		if( cesdk.load(wstrDir.c_str()) == false )
		{
			g_log.Log(CELOG_DEBUG,"load sdk failed!\n");
		}
		else
		{
			g_log.Log(CELOG_DEBUG,"load sdk successfully!\n");
		}
	}
}

PolicyCommunicator::~PolicyCommunicator()
{
	if( cesdk.is_loaded() == true )
	{
		cesdk.unload();
	}
}


DWORD PolicyCommunicator::GetIp()
{
    return 0;
}


void PolicyCommunicator::GetUserInfo(LPWSTR wzSid, int nSize, LPWSTR UserName, int UserNameLen)
{
    HANDLE hTokenHandle = 0;

    memset( wzSid, 0, nSize );
    memset( UserName, 0, UserNameLen );

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hTokenHandle))
    {
        if(GetLastError() == ERROR_NO_TOKEN)
        {
            if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTokenHandle ))
            {
				g_log.Log(CELOG_DEBUG,L"[GetAndPrintSid] Fail to get process token.");
				return;
            }
        }
        else
        {
			g_log.Log(CELOG_DEBUG,L"[GetAndPrintSid] Fail to get thread token.");
			return;
        }
    }

    // Get SID
	UCHAR InfoBuffer[512] = {0};
    DWORD cbInfoBuffer = 512;
    LPTSTR StringSid = 0;
    WCHAR   uname[64] = {0}; DWORD unamelen = 63;
    WCHAR   dname[64] = {0}; DWORD dnamelen = 63;
    WCHAR   fqdnname[MAX_PATH+1]; 
	memset(fqdnname, 0, sizeof(fqdnname));
    SID_NAME_USE snu;
    if(!GetTokenInformation(hTokenHandle, TokenUser, InfoBuffer, cbInfoBuffer, &cbInfoBuffer))
        return;
    if(ConvertSidToStringSid(((PTOKEN_USER)InfoBuffer)->User.Sid, &StringSid))
    {
        wcsncpy_s(wzSid, nSize, StringSid, _TRUNCATE);
        if(StringSid) LocalFree(StringSid);
    }
    if(LookupAccountSid(NULL, ((PTOKEN_USER)InfoBuffer)->User.Sid, uname, &unamelen, dname, &dnamelen, &snu))   
    {
        wcsncat_s( UserName, nSize, uname, _TRUNCATE);

        char  szHostname[MAX_PATH+1]; memset(szHostname, 0, sizeof(szHostname));
        WCHAR wzHostname[MAX_PATH+1]; memset(wzHostname, 0, sizeof(wzHostname));
        gethostname(szHostname, MAX_PATH);
        if(0 != szHostname[0])
        {
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szHostname, -1, wzHostname, MAX_PATH);
            wcsncat_s(UserName, nSize, L"@", _TRUNCATE);
            GetFQDN(wzHostname, fqdnname, MAX_PATH);
            wcsncat_s(UserName, nSize, fqdnname, _TRUNCATE);
        }
    }
	if(hTokenHandle)//Close the handle, added by kevin 2009-7-31
	{
		CloseHandle(hTokenHandle);
		hTokenHandle = 0;
	}
}

void PolicyCommunicator::InitData()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 0);
	err = WSAStartup(wVersionRequested, &wsaData);

    GetUserInfo(m_wzSID, 128, m_wzUserName, 128);
    GetModuleFileName(NULL, m_wzAppPath, MAX_PATH);
  
    if( err != 0 )
    {
        char strError[256] = {0};
        _snprintf_s( strError, 256, _TRUNCATE, "WSA socket error: %d", err );
	    g_log.Log(CELOG_DEBUG,"%s",strError);
        exit(0);
    }
    m_ulIp = GetIp();
}

BOOL PolicyCommunicator::EvaluateApplication(LPCWSTR srcApp)
{
	if (!cesdk.is_loaded())
	{
		return TRUE;
	}

    BOOL            bAllow    = TRUE;
	CEString cesOperation = cesdk.fns.CEM_AllocateString(L"SCREEN_CAPTURE");
    CEint32         timeout_in_millisec;
    CEUser          user;   
    memset(&user, 0, sizeof(CEUser));
    CEApplication   appRun; 
    memset(&appRun, 0, sizeof(CEApplication));   

    CEEnforcement_t enforcement;

	CEResource resource;
	resource.resourceName = cesdk.fns.CEM_AllocateString(L"display");
	resource.resourceType = cesdk.fns.CEM_AllocateString(L"display");

    if(NULL == PolicyCommunicator::m_connectHandle) Connect2PolicyServer();
    if(NULL == PolicyCommunicator::m_connectHandle)
    {
        goto _CLEAN_EXIT;
    }

    GetUserInfo(m_wzSID, 128, m_wzUserName, 128);

    appRun.appName = cesdk.fns.CEM_AllocateString( srcApp );
    appRun.appPath = cesdk.fns.CEM_AllocateString( srcApp );
    appRun.appURL  = cesdk.fns.CEM_AllocateString(L"");
    user.userID    = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzSID);
    user.userName  = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzUserName);
    timeout_in_millisec = TIMEOUT_TIME;

    {
        BOOL bReconnect = TRUE;
_EVALUATE:
        
		int nErrorCode = cesdk.fns.CEEVALUATE_CheckResources(PolicyCommunicator::m_connectHandle, //CEHandle handle, 
												   cesOperation, //const CEString operation,                 
												   &resource, //const CEResource* source,           
												   NULL, //const CEAttributes * sourceAttributes,
												   NULL, //const CEResource* target,           
												   NULL, //const CEAttributes * targetAttributes,
												   &user,//const CEUser  *user,
												   NULL, //CEAttributes * userAttributes,
												   &appRun, //CEApplication *app,
												   NULL, //CEAttributes * appAttributes,
												   NULL, //CEString *recipients,
												   (CEint32)0,    //CEint32 numRecipients,
												   (CEint32)0, //const CEint32 ipNumber,
												   CETrue, //const CEBoolean performObligation,
												   CE_NOISE_LEVEL_USER_ACTION, //const CENoiseLevel_t noiseLevel,
												   &enforcement, //CEEnforcement_t * enforcement,
												   timeout_in_millisec //const CEint32 timeout_in_millisec
												   );
        
        if(CE_RESULT_SUCCESS == nErrorCode )            
        {
            bAllow = (CEAllow == enforcement.result);
        }
        else
        {
            if(bReconnect)
            {
                bReconnect = FALSE;
                Disconnect2PolicyServer();
                if(NULL == PolicyCommunicator::m_connectHandle) Connect2PolicyServer();
                if(NULL == PolicyCommunicator::m_connectHandle)
                {
                }
                else
                {
                    goto _EVALUATE;
                }
            }
        }
		cesdk.fns.CEEVALUATE_FreeEnforcement(enforcement);
    }

_CLEAN_EXIT:
    cesdk.fns.CEM_FreeString(resource.resourceName);
    cesdk.fns.CEM_FreeString(resource.resourceType);
    cesdk.fns.CEM_FreeString(appRun.appName);
    cesdk.fns.CEM_FreeString(appRun.appPath);
    cesdk.fns.CEM_FreeString(appRun.appURL);
    cesdk.fns.CEM_FreeString(user.userID);
    cesdk.fns.CEM_FreeString(user.userName);
	cesdk.fns.CEM_FreeString(cesOperation);
	

    return bAllow;
}

BOOL PolicyCommunicator::Connect2PolicyServer()
{
	if(!cesdk.is_loaded())
		return FALSE;

    CEApplication   app;
    CEUser          user;
    CEint32         timeout_in_millisec = TIMEOUT_TIME;
    BOOL            bRet = FALSE;

    // Evaluate
    memset(&app, 0, sizeof(CEApplication));
    memset(&user, 0, sizeof(CEUser));
    app.appName   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppName);
    app.appPath   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppPath);
    user.userID   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzSID);
    user.userName = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzUserName);
    int nErrorCode = cesdk.fns.CECONN_Initialize(app, user, 0, &m_connectHandle, timeout_in_millisec);
    if(CE_RESULT_SUCCESS == nErrorCode && NULL != m_connectHandle )
    {
        bRet = TRUE;
    }

    cesdk.fns.CEM_FreeString(app.appName);
    cesdk.fns.CEM_FreeString(app.appPath);
    cesdk.fns.CEM_FreeString(user.userID);
    cesdk.fns.CEM_FreeString(user.userName);

    return bRet;
}

void PolicyCommunicator::Disconnect2PolicyServer()
{
	if(!cesdk.is_loaded())
		return;

    if(NULL != PolicyCommunicator::m_connectHandle)
    {
        CEint32         timeout_in_millisec = TIMEOUT_TIME;
        cesdk.fns.CECONN_Close(PolicyCommunicator::m_connectHandle, timeout_in_millisec);
        PolicyCommunicator::m_connectHandle = NULL;
    }
}

PolicyCommunicator g_PolicyCommunicator;

bool EvaluateApp( LPCWSTR srcApp )
{
    if( srcApp && wcslen( srcApp )  )
    {
        bool bAllow = (TRUE == g_PolicyCommunicator.EvaluateApplication( srcApp ) );        
        return bAllow;
    }
    return false;
}
