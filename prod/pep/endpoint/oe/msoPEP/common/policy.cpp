


#pragma once
#include "stdafx.h"
#include <Winsock2.h>
#include <Windows.h>
#include <time.h>
#include "policy.h"
#include "..\outlook\outlookUtilities.h"
#include "msoPEP.h"
#include "strsafe.h"
#include <string>
#include "log.h"
#include <Iphlpapi.h>


#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib") 

#include "eframework/auto_disable/auto_disable.hpp"
#include "eframework/verdict_cache/verdict_cache.hpp"
#include "eframework/timer/timer_high_resolution.hpp"
#include "eframework/platform/cesdk_loader.hpp"
#include "eframework/platform/cesdk_attributes.hpp"
using namespace std;

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

nextlabs::cesdk_loader cesdk;
int                 PolicyCommunicator::m_nRef = 0;
BOOL                PolicyCommunicator::m_bFirstInit = TRUE;
PolicyCommunicator* PolicyCommunicator::m_pThis = NULL;
extern CRITICAL_SECTION g_csPolicyInstance;
CEHandle	    PolicyCommunicator::m_sdk = {0};
BOOL				PolicyCommunicator::m_bSDK= FALSE;
CELogging::Handle	   PolicyCommunicator::m_ceLogging  = {0} ;
extern PolicyCommunicator* g_spPolicyCommunicator;

WCHAR    PolicyCommunicator::m_wzSID[128]          = {0};
WCHAR    PolicyCommunicator::m_wzUserName[128]     = {0};
WCHAR    PolicyCommunicator::m_wzHostName[128]     = {0};
WCHAR    PolicyCommunicator::m_wzAppName[]         = L"OUTLOOK";
WCHAR    PolicyCommunicator::m_wzAppPath[MAX_PATH] = {0};
CEHandle PolicyCommunicator::m_connectHandle       = NULL;
ULONG    PolicyCommunicator::m_ulIp                = 0;
WCHAR    PolicyCommunicator::m_wzSenderName[128]   = {0};
string   PolicyCommunicator::m_strIp               = "";
wstring  PolicyCommunicator::m_strID;

STRINGLIST      PolicyCommunicator::m_listRecipients;
ATTACHMENTLIST  PolicyCommunicator::m_listAttachments;
int PolicyCommunicator::m_nQueryPCTimeout = 60000;

BOOL PolicyCommunicator::InitSDK()
{NLONLY_DEBUG
	if( cesdk.is_loaded() == false )
	{
		std::wstring strComDir = OLUtilities::GetCommonComponentsDir();
		DPW((L"CESDK::SetupConnection: loading SDK, common components directory: %s\n", strComDir.c_str()));
		if( cesdk.load(strComDir.c_str()) == false )
		{
			DPW((L"CESDK::SetupConnection: CESDK::Load failed\n"));
			PolicyCommunicator::m_bSDK = FALSE;
		}
		else
		{
			PolicyCommunicator::m_bSDK = TRUE;
		}
		CELogging::Load( strComDir, &PolicyCommunicator::m_ceLogging ) ;
	}

	return PolicyCommunicator::m_bSDK;
}

VOID PolicyCommunicator::DeinitSDK()
{
	if(PolicyCommunicator::m_bSDK)
	{
		PolicyCommunicator::m_bSDK = FALSE;
		
	}
	CELogging::Unload(   &PolicyCommunicator::m_ceLogging ) ;
	if (g_spPolicyCommunicator != NULL)
	{
		g_spPolicyCommunicator->Release();
		g_spPolicyCommunicator = NULL;
	}
	
}

PolicyCommunicator::PolicyCommunicator()
{
	m_strTickTime = L"0";
}

PolicyCommunicator::~PolicyCommunicator()
{
}

PolicyCommunicator* PolicyCommunicator::CreateInstance()
{
	EnterCriticalSection(&g_csPolicyInstance);

    if(PolicyCommunicator::m_bFirstInit)
    {
        PolicyCommunicator::m_bFirstInit = FALSE;
        PolicyCommunicator::InitData();
    }

    assert(0 <= m_nRef && INT_MAX > m_nRef);
    if (0 == m_nRef)
    {
        assert(NULL == m_pThis);
        m_pThis = new(std::nothrow) PolicyCommunicator();
    }

    if (NULL != m_pThis)
    {
        m_nRef++;
    }

    PolicyCommunicator *temp = m_pThis;

    LeaveCriticalSection(&g_csPolicyInstance);

    return temp;
}

void PolicyCommunicator::Release()
{
	EnterCriticalSection(&g_csPolicyInstance);

    assert(0 < m_nRef);
    m_nRef--;

    if (0 == m_nRef)
    {
        assert(NULL != m_pThis);
        delete m_pThis;
        m_pThis = NULL;
    }
	Disconnect2PolicyServer();

    LeaveCriticalSection(&g_csPolicyInstance);
}


int PolicyCommunicator::IPToValue(const string& strIP)
{
	
	int a[4];
	string IP = strIP;
	string strTemp;
	size_t pos;
	size_t i=3;

	do
	{
		pos = IP.find(".");

		if(pos != string::npos)
		{
			strTemp = IP.substr(0,pos);
			a[i] = atoi(strTemp.c_str());
			i--;
			IP.erase(0,pos+1);
		}
		else
		{
			strTemp = IP;
			a[i] = atoi(strTemp.c_str());
			break;
		}

	}while(1);

	int nResult = (a[3]<<24) + (a[2]<<16)+ (a[1]<<8) + a[0];
	return nResult;
}


DWORD PolicyCommunicator::GetIp(string& strIP)
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;


	ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) 
	{
		return 1;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
	{
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) 
		{
			return 1;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) 
	{
		pAdapter = pAdapterInfo;
		while (pAdapter) 
		{

			if (strcmp(pAdapter->GatewayList.IpAddress.String,"0.0.0.0") == 0)
			{
				if (strIP.empty())
				{
					strIP = pAdapter->IpAddressList.IpAddress.String;
				}
			}
			else
			{
				strIP =pAdapter->IpAddressList.IpAddress.String;

				break;
			}
			pAdapter = pAdapter->Next;

		}
	}

	if (pAdapterInfo)
		FREE(pAdapterInfo);

	return 0;

}


void PolicyCommunicator::GetUserInfo(LPWSTR wzSid, int nSize, LPWSTR UserName, int UserNameLen)
{
    HANDLE hTokenHandle = 0;

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hTokenHandle))
    {
        if(GetLastError() == ERROR_NO_TOKEN)
        {
            if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTokenHandle ))
            {
            	DP((L"[GetAndPrintSid] Fail to get process token."));
                return;
            }
        }
        else
        {
        	DP((L"[GetAndPrintSid] Fail to get thread token."));
            return;
        }
    }

    // Get SID
    UCHAR InfoBuffer[512];
    DWORD cbInfoBuffer = 512;
    LPTSTR StringSid = 0;
    WCHAR   uname[64] = {0}; DWORD unamelen = 63;
    WCHAR   dname[64] = {0}; DWORD dnamelen = 63;
	WCHAR   fqdnname[MAX_PATH+1]; memset(fqdnname, 0, sizeof(fqdnname));
    SID_NAME_USE snu;
    if(!GetTokenInformation(hTokenHandle, TokenUser, InfoBuffer, cbInfoBuffer, &cbInfoBuffer))
        return;
    if(ConvertSidToStringSid(((PTOKEN_USER)InfoBuffer)->User.Sid, &StringSid))
    {
        _tcsncpy_s(wzSid, nSize, StringSid, _TRUNCATE);
        if(StringSid) LocalFree(StringSid);
    }
    if(LookupAccountSid(NULL, ((PTOKEN_USER)InfoBuffer)->User.Sid, uname, &unamelen, dname, &dnamelen, &snu))   
    {
		wcsncpy_s( UserName, UserNameLen, uname, _TRUNCATE);

		char  szHostname[MAX_PATH+1]; memset(szHostname, 0, sizeof(szHostname));
		WCHAR wzHostname[MAX_PATH+1]; memset(wzHostname, 0, sizeof(wzHostname));
		gethostname(szHostname, MAX_PATH);
		if(0 != szHostname[0])
		{
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szHostname, -1, wzHostname, MAX_PATH);
			DP((L"HOSTNAME == %s\n", wzHostname));
			wcsncat_s(UserName, UserNameLen, L"@", _TRUNCATE); 
			OLUtilities::GetFQDN(wzHostname, fqdnname, MAX_PATH);


			wstring strFullHost(CharLowerW(fqdnname));
			wstring strHostName(CharLowerW(wzHostname));
	
			size_t nPos = strFullHost.find(strHostName);
			if(nPos != wstring::npos)
			{
				if (strHostName.length() < strFullHost.length())
				{
					strFullHost = strFullHost.substr(strHostName.length() + 1);
				}
			}
			wcsncpy_s(m_wzSenderName, 127, UserName, _TRUNCATE);
			wcsncat_s(m_wzSenderName, 127, strFullHost.c_str(), _TRUNCATE);
			wcsncat_s(UserName, UserNameLen, fqdnname, _TRUNCATE);
		}
    }

	DP((L"We get host UserName with FQDN: %s\n", UserName));
}

void PolicyCommunicator::InitData()
{
    GetUserInfo(m_wzSID, 128, m_wzUserName, 128);
    GetModuleFileName(NULL, m_wzAppPath, MAX_PATH);
	GetIp(PolicyCommunicator::m_strIp);
	PolicyCommunicator::m_ulIp = IPToValue(PolicyCommunicator::m_strIp);
}

BOOL PolicyCommunicator::QueryOutlookPolicySingle(CEAction_t operation, CEString Attachment, CEAttributes* psourceAttributes, LPCWSTR wzRecipient)
{
	if( cesdk.is_loaded() == false )
	{
		std::wstring strComDir = OLUtilities::GetCommonComponentsDir();
		DPW((L"CESDK::SetupConnection: loading SDK, common components directory: %s\n", strComDir.c_str()));
		if( cesdk.load(strComDir.c_str()) == false )
		{
			DPW((L"CESDK::SetupConnection: CESDK::Load failed\n"));
			return TRUE;
		}
	}

	CEApplication   app;
	CEUser          user;
	CEEnforcement_t enforcement;    memset(&enforcement, 0, sizeof(enforcement));
	CEint32         timeout_in_millisec = m_nQueryPCTimeout;
	//CEAttributes    sourceAttributes;   memset(&sourceAttributes, 0,sizeof(sourceAttributes));
	CEString        recipients    = cesdk.fns.CEM_AllocateString(wzRecipient);

	BOOL            bAllow = TRUE;

	app.appURL    = cesdk.fns.CEM_AllocateString(L"");
	app.appName   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppName);
	app.appPath   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppPath);
	user.userID   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzSID);
	user.userName = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzUserName);
	try
	{
		if(CE_RESULT_SUCCESS
			==
			cesdk.fns.CEEVALUATE_CheckMessageAttachment(PolicyCommunicator::m_connectHandle, 
			operation, 
			Attachment, 
			psourceAttributes,//&sourceAttributes,
			1,
			&recipients,
			(CEint32) PolicyCommunicator::m_ulIp,
			&user,
			NULL,
			&app,
			NULL,
			CEFalse,
			CE_NOISE_LEVEL_MIN,
			&enforcement,
			timeout_in_millisec)
			)
		{
			bAllow = (CEAllow == enforcement.result);
		}
	}
   catch( ... )
   {
	   DPW((L"CESDK::QueryOutlookPolicySingle: CEEVALUATE_CheckMessageAttachment failed\n"));
   }

   cesdk.fns.CEM_FreeString(recipients);
   cesdk.fns.CEM_FreeString(app.appURL);
   cesdk.fns.CEM_FreeString(app.appName);
   cesdk.fns.CEM_FreeString(app.appPath);
   cesdk.fns.CEM_FreeString(user.userID);
   cesdk.fns.CEM_FreeString(user.userName);


	return bAllow;
}
BOOL PolicyCommunicator::QueryOutlookPolicyEx(MAILTYPE mtMailType,
											  LPCWSTR wzAttachment,
											  LPCWSTR wzResolvedPath,
											  STRINGLIST& pwzRecipients,
											  STRINGLIST& pwzDenyRecipients,
											  CEEnforcement_t* pEnforcement,
											  std::vector<std::pair<std::wstring,std::wstring>>& attrs)
{
    UNREFERENCED_PARAMETER(mtMailType);

	BOOL            bRet = TRUE;
	CEAction_t      operation;
	CEApplication   app;
	CEUser          user;
	CEint32         timeout_in_millisec;
	CEString        srcFile = cesdk.fns.CEM_AllocateString(wzAttachment);
	CEAttributes    sourceAttributes;   memset(&sourceAttributes, 0,sizeof(sourceAttributes));
	CEint32         numRecipients = (CEint32)pwzRecipients.size();
	CEint32			numAttrs=(CEint32)attrs.size(),numAttrsIndex=0;
	CEString        *recipients   = NULL;
	int             i = 0;
	BOOL            bAllow = TRUE;
	WCHAR wzLastModifyDate[MAX_PATH+1]; memset(wzLastModifyDate, 0, sizeof(wzLastModifyDate));
	DP((L"Here is QueryOutlookPolicyEx."));
	OLUtilities::GetFileLastModifyTime(wzAttachment, wzLastModifyDate, MAX_PATH);
	if(0 == wzLastModifyDate[0]) wcsncpy_s(wzLastModifyDate, MAX_PATH, L"123456789", _TRUNCATE);
	DP((L"[Attachment:LastModifiedDate] %s\n", wzLastModifyDate));
	DP((L"Evaluate attachment: %s\n", wzAttachment));
	DP((L"Evaluate attachment(resolved): %s\n", wzResolvedPath));

	if(NULL == PolicyCommunicator::m_connectHandle) Connect2PolicyServer();
	if(NULL == PolicyCommunicator::m_connectHandle)
	{
		cesdk.fns.CEM_FreeString(srcFile);
		pEnforcement->result = CEAllow;
		DP((L"Fail to connect to PDP. Return False.\n"));
		return FALSE;
	}

	pwzDenyRecipients.clear();
	app.appURL    = cesdk.fns.CEM_AllocateString(L"");
	app.appName   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppName);
	app.appPath   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppPath);
	user.userID   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzSID);
	user.userName = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzUserName);
	///operation = (OPEN_FORWARD_MAIL==MailType)?CE_ACTION_FORWARD : ((OPEN_REPLY_MAIL==MailType)?CE_ACTION_REPLY : CE_ACTION_NEW_EMAIL);
	// For now, we always use CE_ACTION_EMAIL_FILE
	operation = CE_ACTION_EMAIL_FILE;
	timeout_in_millisec = m_nQueryPCTimeout;

	recipients    = new CEString[numRecipients];
    if(NULL == recipients) goto _CLEAN_EXIT;
    for (i=0; i<numRecipients; i++)
    {
        DP((L"[QueryOutlookPolicy] Recipient: %s\n", pwzRecipients[i].c_str()));
        recipients[i] = cesdk.fns.CEM_AllocateString(pwzRecipients[i].c_str());
    }
    try
    {	
        sourceAttributes.count = 2+numAttrs;
        sourceAttributes.attrs = new CEAttribute[sourceAttributes.count];
        if(NULL != sourceAttributes.attrs)
        {
            for(numAttrsIndex=0;numAttrsIndex<numAttrs;numAttrsIndex++)
            {
                sourceAttributes.attrs[numAttrsIndex].key	=cesdk.fns.CEM_AllocateString(attrs[numAttrsIndex].first.c_str());
                sourceAttributes.attrs[numAttrsIndex].value	=cesdk.fns.CEM_AllocateString(attrs[numAttrsIndex].second.c_str());
            }
            sourceAttributes.attrs[numAttrs+0].key   = cesdk.fns.CEM_AllocateString(L"modified_date");
            sourceAttributes.attrs[numAttrs+0].value = cesdk.fns.CEM_AllocateString(wzLastModifyDate);
            sourceAttributes.attrs[numAttrs+1].key   = cesdk.fns.CEM_AllocateString(L"resolved_name");
            sourceAttributes.attrs[numAttrs+1].value = cesdk.fns.CEM_AllocateString(wzResolvedPath);

            INT result = 0;
            result =
                cesdk.fns.CEEVALUATE_CheckMessageAttachment(PolicyCommunicator::m_connectHandle, 
                operation, 
                srcFile, 
                &sourceAttributes,
                numRecipients,
                recipients,
                (CEint32) PolicyCommunicator::m_ulIp,
                &user,
                NULL,
                &app,
                NULL,											  
                CETrue,
                CE_NOISE_LEVEL_USER_ACTION,
                pEnforcement,
                timeout_in_millisec) ;
            if(CE_RESULT_SUCCESS == result )          
            {
                bAllow = (CEAllow == pEnforcement->result);
                if(!bAllow)
                {
                    DP((L"It is deny!\n"));
                }
                else
                {
                    DP((L"It is allow!\n"));
                }
            }
            else
            {
                bRet = FALSE;
                DP((L"Fail to query policy ErrorCode[%d]!\n",result));
            }

            for (i=0; i<sourceAttributes.count; i++)
            {
                cesdk.fns.CEM_FreeString(sourceAttributes.attrs[i].key);
                cesdk.fns.CEM_FreeString(sourceAttributes.attrs[i].value);
            }
            if(sourceAttributes.attrs) delete [](sourceAttributes.attrs); sourceAttributes.attrs=NULL;
        }
    }
    catch( ... )
    {
        if(sourceAttributes.attrs) delete [](sourceAttributes.attrs); sourceAttributes.attrs=NULL;
        if(recipients)
        {
            for (i=0; i<numRecipients; i++)
            {
                cesdk.fns.CEM_FreeString(recipients[i]);
            }
            delete []recipients;
            recipients = NULL;
        }
        goto _CLEAN_EXIT;
    }

_CLEAN_EXIT:
	cesdk.fns.CEM_FreeString(srcFile);
	cesdk.fns.CEM_FreeString(app.appURL);
	cesdk.fns.CEM_FreeString(app.appName);
	cesdk.fns.CEM_FreeString(app.appPath);
	cesdk.fns.CEM_FreeString(user.userID);
	cesdk.fns.CEM_FreeString(user.userName);
	if(recipients)
	{
		for (i=0; i<numRecipients; i++)
		{
			cesdk.fns.CEM_FreeString(recipients[i]);
		}
		delete []recipients;
		recipients = NULL;
	}

	return bRet;
}




BOOL PolicyCommunicator::QueryOutlookPreviewPolicy(LPCWSTR AttachmentPath)
{


	BOOL BAllow = TRUE;

	CEString ce_string_action_name = cesdk.fns.CEM_AllocateString(L"OPEN");
	CEResource* res_source = cesdk.fns.CEM_CreateResourceW(AttachmentPath,L"fso");
	

	CEApplication   app;
	app.appURL    = cesdk.fns.CEM_AllocateString(L"");
	app.appName   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppName);
	app.appPath   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppPath);


	CEEnforcement_t Enforcement;
	CEint32         timeout_in_millisec = m_nQueryPCTimeout;


	if(NULL == PolicyCommunicator::m_connectHandle) Connect2PolicyServer();
	if(NULL == PolicyCommunicator::m_connectHandle)
	{
		Enforcement.result = CEAllow;
		return FALSE;
	}

	WCHAR wzLastModifyDate[MAX_PATH+1]; memset(wzLastModifyDate, 0, sizeof(wzLastModifyDate));
	OLUtilities::GetFileLastModifyTime(AttachmentPath, wzLastModifyDate, MAX_PATH);
	if(0 == wzLastModifyDate[0]) wcsncpy_s(wzLastModifyDate, MAX_PATH, L"123456789", _TRUNCATE);

	CEAttributes   sourceAttributes;   
	memset(&sourceAttributes, 0,sizeof(sourceAttributes));
	sourceAttributes.count = 3;

	sourceAttributes.attrs = new CEAttribute[sourceAttributes.count];
	if(NULL != sourceAttributes.attrs)
	{
		sourceAttributes.attrs[0].key   = cesdk.fns.CEM_AllocateString(L"modified_date");
		sourceAttributes.attrs[0].value = cesdk.fns.CEM_AllocateString(wzLastModifyDate);
		sourceAttributes.attrs[1].key   = cesdk.fns.CEM_AllocateString(L"resolved_name");
		sourceAttributes.attrs[1].value = cesdk.fns.CEM_AllocateString(AttachmentPath);
		sourceAttributes.attrs[2].key   = cesdk.fns.CEM_AllocateString(L"CE::nocache");
		sourceAttributes.attrs[2].value = cesdk.fns.CEM_AllocateString(L"yes");
	}


	CEResult_t result = cesdk.fns.CEEVALUATE_CheckResources(PolicyCommunicator::m_connectHandle,          /* connection handle */
		ce_string_action_name,   /* action name */
		res_source,              /* source */
		&sourceAttributes,            /* source - attributes */
		NULL,              /* target */
		NULL,            /* target - attributes */
		NULL,                    /* user */
		NULL,                    /* user - attributes */
		&app,                     /* application */
		NULL,                    /* application attributes */
		NULL,                    /* receipients */
		0,                       /* number of receipients */
		0,                       /* IP */
		CEFalse,                /* perform obligations? */
		CE_NOISE_LEVEL_USER_ACTION,             /* noise level */
		&Enforcement,            /* enforcement */
		timeout_in_millisec);             /* timeout in milliseconds */


	if( result == CE_RESULT_CONN_FAILED || result == CE_RESULT_NULL_CEHANDLE )
	{
		Disconnect2PolicyServer() ;
		Connect2PolicyServer();
		result = cesdk.fns.CEEVALUATE_CheckResources(PolicyCommunicator::m_connectHandle,          /* connection handle */
			ce_string_action_name,   /* action name */
			res_source,              /* source */
			&sourceAttributes,            /* source - attributes */
			NULL,              /* target */
			NULL,            /* target - attributes */
			NULL,                    /* user */
			NULL,                    /* user - attributes */
			&app,                     /* application */
			NULL,                    /* application attributes */
			NULL,                    /* receipients */
			0,                       /* number of receipients */
			0,                       /* IP */
			CEFalse,                /* perform obligations? */
			CE_NOISE_LEVEL_USER_ACTION,             /* noise level */
			&Enforcement,            /* enforcement */
			timeout_in_millisec);             /* timeout in milliseconds */
	}


	if(CE_RESULT_SUCCESS == result )
	{
		if(CEDeny == Enforcement.result)
		{
			BAllow = FALSE;
		}
	}
	else
	{
		BAllow = FALSE;
	}


	cesdk.fns.CEM_FreeString(ce_string_action_name);
	cesdk.fns.CEM_FreeString(app.appName);
	cesdk.fns.CEM_FreeString(app.appPath);
	cesdk.fns.CEM_FreeString(app.appURL);
	cesdk.fns.CEM_FreeString(sourceAttributes.attrs[0].key);
	cesdk.fns.CEM_FreeString(sourceAttributes.attrs[0].value);
	cesdk.fns.CEM_FreeString(sourceAttributes.attrs[1].key );
	cesdk.fns.CEM_FreeString(sourceAttributes.attrs[1].value);
	cesdk.fns.CEM_FreeString(sourceAttributes.attrs[2].key );
	cesdk.fns.CEM_FreeString(sourceAttributes.attrs[2].value);
	delete []sourceAttributes.attrs;
	cesdk.fns.CEM_FreeResource(res_source);

	return BAllow;
}


BOOL PolicyCommunicator::QueryOutlookPolicy(MAILTYPE mtMailType,
											LPCWSTR wzAttachment,
											LPCWSTR wzResolvedPath,
                                            LPCWSTR wzClientIdPropertyName,
                                            LPCWSTR wzClientId,
                                            STRINGLIST& pwzRecipients,
                                            STRINGLIST& pwzDenyRecipients,
                                            CEEnforcement_t* pEnforcement)
{
    UNREFERENCED_PARAMETER(mtMailType);
    BOOL            bRet = TRUE;
    CEAction_t      operation;
    CEApplication   app;
    CEUser          user;
    CEint32         timeout_in_millisec;
    CEString        srcFile = cesdk.fns.CEM_AllocateString(wzAttachment);
    CEAttributes    sourceAttributes;   memset(&sourceAttributes, 0,sizeof(sourceAttributes));
    CEint32         numRecipients = (CEint32)pwzRecipients.size();
    CEString        *recipients   = NULL;
    int             i = 0;
    BOOL            bAllow = TRUE;
    WCHAR wzLastModifyDate[MAX_PATH+1]; memset(wzLastModifyDate, 0, sizeof(wzLastModifyDate));
    OLUtilities::GetFileLastModifyTime(wzAttachment, wzLastModifyDate, MAX_PATH);
    if(0 == wzLastModifyDate[0]) wcsncpy_s(wzLastModifyDate, MAX_PATH, L"123456789", _TRUNCATE);
    DP((L"[Attachment:LastModifiedDate] %s\n", wzLastModifyDate));

	DP((L"Evaluate attachment: %s\n", wzAttachment));
	DP((L"Evaluate attachment(resolved): %s\n", wzResolvedPath));

    if(NULL == PolicyCommunicator::m_connectHandle) Connect2PolicyServer();
    if(NULL == PolicyCommunicator::m_connectHandle)
    {
        cesdk.fns.CEM_FreeString(srcFile);
        pEnforcement->result = CEAllow;
        DP((L"Fail to connect to PDP. Return False.\n"));
        return FALSE;
    }

    pwzDenyRecipients.clear();
    app.appURL    = cesdk.fns.CEM_AllocateString(L"");
    app.appName   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppName);
    app.appPath   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppPath);
    user.userID   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzSID);
    user.userName = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzUserName);
    ///operation = (OPEN_FORWARD_MAIL==MailType)?CE_ACTION_FORWARD : ((OPEN_REPLY_MAIL==MailType)?CE_ACTION_REPLY : CE_ACTION_NEW_EMAIL);
    // For now, we always use CE_ACTION_EMAIL_FILE
    operation = CE_ACTION_EMAIL_FILE;
    timeout_in_millisec = m_nQueryPCTimeout;

    recipients    = new CEString[numRecipients];
    if(NULL == recipients) goto _CLEAN_EXIT;
    for (i=0; i<numRecipients; i++)
    {
        DP((L"[QueryOutlookPolicy] Recipient: %s\n", pwzRecipients[i].c_str()));
        recipients[i] = cesdk.fns.CEM_AllocateString(pwzRecipients[i].c_str());
    }

    try
    {
        if (wcslen(wzClientId) > 0)
        {
            sourceAttributes.count = 4;
        }
        else
        {
            sourceAttributes.count = 2;
        }

        sourceAttributes.attrs = new CEAttribute[sourceAttributes.count];
        if(NULL != sourceAttributes.attrs)
        {
            sourceAttributes.attrs[0].key   = cesdk.fns.CEM_AllocateString(L"modified_date");
            sourceAttributes.attrs[0].value = cesdk.fns.CEM_AllocateString(wzLastModifyDate);
            sourceAttributes.attrs[1].key   = cesdk.fns.CEM_AllocateString(L"resolved_name");
            sourceAttributes.attrs[1].value = cesdk.fns.CEM_AllocateString(wzResolvedPath);

            if (sourceAttributes.count == 4)
            {
                sourceAttributes.attrs[2].key   =
                    cesdk.fns.CEM_AllocateString(wzClientIdPropertyName);
                sourceAttributes.attrs[2].value = cesdk.fns.CEM_AllocateString(wzClientId);
                // Tell PDP that it should use the attributes that we are passing
                // now instead of the cached ones from earlier EVALs.
                sourceAttributes.attrs[3].key   = cesdk.fns.CEM_AllocateString(L"CE::nocache");
                sourceAttributes.attrs[3].value = cesdk.fns.CEM_AllocateString(L"yes");
            }
            INT result = 0;
            result =
                cesdk.fns.CEEVALUATE_CheckMessageAttachment(PolicyCommunicator::m_connectHandle, 
                operation, 
                srcFile, 
                &sourceAttributes,
                numRecipients,
                recipients,
                (CEint32) PolicyCommunicator::m_ulIp,
                &user,
                NULL,
                &app,
                NULL,
                CETrue,
                CE_NOISE_LEVEL_USER_ACTION,
                pEnforcement,
                timeout_in_millisec) ;
			if( result == CE_RESULT_CONN_FAILED || result == CE_RESULT_NULL_CEHANDLE )
			{
				Disconnect2PolicyServer() ;
				Connect2PolicyServer();
				 result = cesdk.fns.CEEVALUATE_CheckMessageAttachment(PolicyCommunicator::m_connectHandle, 
                operation, 
                srcFile, 
                &sourceAttributes,
                numRecipients,
                recipients,
                (CEint32) PolicyCommunicator::m_ulIp,
                &user,
                NULL,
                &app,
                NULL,
                CETrue,
                CE_NOISE_LEVEL_USER_ACTION,
                pEnforcement,
                timeout_in_millisec) ;
			}
            if(CE_RESULT_SUCCESS == result )          
            {
                bAllow = (CEAllow == pEnforcement->result);
                if(!bAllow)
                {
                    DP((L"It is deny!\n"));
                }
                else
                {
                    DP((L"It is allow!\n"));
                }
            }
            else
            {
                bRet = FALSE;
                DP((L"Fail to query policy ErrorCode[%d]!\n",result));
            }

            for (i=0; i<sourceAttributes.count; i++)
            {
                cesdk.fns.CEM_FreeString(sourceAttributes.attrs[i].key);
                cesdk.fns.CEM_FreeString(sourceAttributes.attrs[i].value);
            }
            if(NULL != sourceAttributes.attrs) delete [](sourceAttributes.attrs); sourceAttributes.attrs = NULL;
        }
    }
    catch(...)
    {
        if(NULL != sourceAttributes.attrs) delete [](sourceAttributes.attrs); sourceAttributes.attrs = NULL;
        if(recipients)
        {
            for (i=0; i<numRecipients; i++)
            {
                cesdk.fns.CEM_FreeString(recipients[i]);
            }
            delete []recipients;
            recipients = NULL;
        }
    }


_CLEAN_EXIT:
    cesdk.fns.CEM_FreeString(srcFile);
    cesdk.fns.CEM_FreeString(app.appURL);
    cesdk.fns.CEM_FreeString(app.appName);
    cesdk.fns.CEM_FreeString(app.appPath);
    cesdk.fns.CEM_FreeString(user.userID);
    cesdk.fns.CEM_FreeString(user.userName);
    if(recipients)
    {
        for (i=0; i<numRecipients; i++)
        {
            cesdk.fns.CEM_FreeString(recipients[i]);
        }
        delete []recipients;
        recipients = NULL;
    }

    return bRet;
}





BOOL PolicyCommunicator::LogDecision(LPCWSTR wzCookie,
                                     CEResponse_t userResponse, 
                                     CEAttributes* optAttributes)
{
    CEString cookie = cesdk.fns.CEM_AllocateString(wzCookie);
    BOOL bRet = FALSE;

    if(NULL == PolicyCommunicator::m_connectHandle) Connect2PolicyServer();
    if(NULL == PolicyCommunicator::m_connectHandle)
    {
        DP((L"Fail to connect to PDP. Return False.\n"));
        goto _CLEAN_EXIT;
    }

    if (NULL == cookie) goto _CLEAN_EXIT;

    if (CE_RESULT_SUCCESS
        ==
		PolicyCommunicator::m_ceLogging.CELOGGING_LogDecision(m_connectHandle, cookie, userResponse,
                              optAttributes))
    {
        bRet = TRUE;
    }
    else
    {
        DP((L"Fail to log decision!\n"));
    }

_CLEAN_EXIT:
    cesdk.fns.CEM_FreeString(cookie);
    return bRet;
}

BOOL PolicyCommunicator::Connect2PolicyServer()
{
    CEApplication   app;
    CEUser          user;
    CEString        host;
    CEint32         timeout_in_millisec = m_nQueryPCTimeout;
    BOOL            bRet = FALSE;

    // Evaluate
    memset(&app, 0, sizeof(CEApplication));
    memset(&user, 0, sizeof(CEUser));
    app.appName   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppName);
    app.appPath   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzAppPath);
    user.userID   = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzSID);
    user.userName = cesdk.fns.CEM_AllocateString(PolicyCommunicator::m_wzUserName);
    host          = cesdk.fns.CEM_AllocateString(L"10.187.2.111");//10.187.4.183(L"vagartst07.test.bluejungle.com");//10.187.6.136(L"10.187.4.161");//(L"10.187.4.176");

	wchar_t strTimeLog[MAX_PATH] = {0};
	DWORD dwStart = GetTickCount();
	CEventLog::WriteEventLog(L"Call CECONN_Initialize --------------------------",EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);
	CEResult_t res = cesdk.fns.CECONN_Initialize(app, user, NULL/**//*host*/, &PolicyCommunicator::m_connectHandle, timeout_in_millisec);
    if(CE_RESULT_SUCCESS == res
        && NULL!=PolicyCommunicator::m_connectHandle)
    {
        DP((L"Succeed on to initialize CECONN!\n"));
        bRet = TRUE;
    }
    else
    {
//##
		wchar_t strlog[MAX_PATH] = {0};
		StringCchPrintf(strlog,MAX_PATH,L"OutlookEnforcer::Failed to initialize CECONN! the err is [%d]",res);
		CEventLog::WriteEventLog(strlog,EVENTLOG_ERROR_TYPE,EVENTLOG_ERROR_INIT_ID);
        DP((L"Failed to initialize CECONN!\n"));
    }

	StringCchPrintf(strTimeLog,MAX_PATH,L"CECONN_Initialize Spend Time [%d] @@@",GetTickCount() - dwStart);

	CEventLog::WriteEventLog(strTimeLog,EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);

    cesdk.fns.CEM_FreeString(host);
    cesdk.fns.CEM_FreeString(app.appName);
    cesdk.fns.CEM_FreeString(app.appPath);
    cesdk.fns.CEM_FreeString(user.userID);
    cesdk.fns.CEM_FreeString(user.userName);

    return bRet;
}

void PolicyCommunicator::Disconnect2PolicyServer()
{
    if(NULL != PolicyCommunicator::m_connectHandle)
    {
        CEint32         timeout_in_millisec = m_nQueryPCTimeout;
		CEventLog::WriteEventLog(L"Enter Disconnect2PolicyServer cesdk.fns.CECONN_Close ------------------",EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);
		cesdk.fns.CECONN_Close(PolicyCommunicator::m_connectHandle, timeout_in_millisec);
		CEventLog::WriteEventLog(L"Leave Disconnect2PolicyServer cesdk.fns.CECONN_Close ------------------",EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);
        PolicyCommunicator::m_connectHandle = NULL;
    }
}


BOOL PolicyCommunicator::GetAccountSid(LPCWSTR AccountName,WCHAR *wzSid, size_t nSize)
{
	PSID pSID = NULL;
	DWORD cbSid = 0;
	LPTSTR DomainName = NULL;
	DWORD cbDomainName = 0;
	SID_NAME_USE SIDNameUse;
	BOOL  bDone = FALSE;

	__try
	{
		if(!LookupAccountName(NULL,  // we only concertrate local machine.
			AccountName,
			pSID,
			&cbSid,
			DomainName,
			&cbDomainName,
			&SIDNameUse))
		{
			pSID = (PSID)malloc(cbSid);
			DomainName = (LPTSTR)malloc(cbDomainName * sizeof(TCHAR));
			if(!pSID || !DomainName)
			{
				__leave;
			}
			if(!LookupAccountName(NULL,
				AccountName,
				pSID,
				&cbSid,
				DomainName,
				&cbDomainName,
				&SIDNameUse))
			{
				__leave;
			}
			bDone = TRUE;
		}
	}
	__finally
	{
		if(DomainName)
			free(DomainName);
		if(!bDone && pSID)
			free(pSID);
	}
	

	if (bDone)
	{
		LPTSTR StringSid = 0;
		ConvertSidToStringSid(pSID, &StringSid);
		if (wzSid != NULL)
		{
			_tcsncpy_s(wzSid, nSize, StringSid, _TRUNCATE);
			if(StringSid) LocalFree(StringSid);
		}
		else
		{
			bDone = FALSE;
		}

		if(pSID)
			free(pSID);
	}
	return bDone;
}


void PolicyCommunicator::SetMultiplyFlag()
{
	DWORD dwTickTime = ::GetTickCount();
	wchar_t strTime[MAX_PATH] = {0};
	StringCchPrintf(strTime,MAX_PATH,L"%d",dwTickTime);
	m_strTickTime = strTime;
}



void PolicyCommunicator::SetRecipients(STRINGLIST &listRecipients)
{
	m_listRecipients.clear();
	m_listRecipients = listRecipients;
}
wstring PolicyCommunicator::GetRecipientsInfo()
{
	wstring wstrRetbuf = L"";
	STRINGLIST::iterator it;
	for (it = m_listRecipients.begin(); it != m_listRecipients.end(); it++)
	{
		wstrRetbuf += *it;
		wstrRetbuf += L";";
	}
	return wstrRetbuf;
	
}
void PolicyCommunicator::SetAttachmentSrcPath(ATTACHMENTLIST &listAttachments)
{
	m_listAttachments.clear();
	m_listAttachments = listAttachments;
}
wstring PolicyCommunicator::GetAttachmentSrcPath()
{
	wstring wstrRetbuf = L"";
	wstring wstrAttachmentPath;
	ATTACHMENTLIST::iterator it;
	for (it = m_listAttachments.begin(); it != m_listAttachments.end(); it++)
	{
		wstrAttachmentPath = (*it)->src;
		wstrAttachmentPath += L";";
		wstrRetbuf += wstrAttachmentPath;
	}
	return wstrRetbuf;
}



bool PolicyCommunicator::WriteReportLog(const wchar_t* ObName,const std::vector<std::wstring>& value)
{
	if( value.size() < 1)	return false;

	if(m_strID.empty())	return false;

	if (m_connectHandle == NULL)
	{
		return false;
	}

	CEString theLogID = cesdk.fns.CEM_AllocateString(m_strID.c_str());
	CEString theObligationName = cesdk.fns.CEM_AllocateString(ObName);
	CEAttributes theLogAttr;
	theLogAttr.attrs = new CEAttribute[value.size()];
	if (theLogAttr.attrs != NULL)
	{
		NLPRINT_DEBUGVIEWLOG(L"Write audit log info is following!\n");
		theLogAttr.count = (int)value.size();
		for(size_t i=0;i<value.size();i++)
		{
			theLogAttr.attrs[i].key = cesdk.fns.CEM_AllocateString(L"Description");
			NLPRINT_DEBUGVIEWLOG(L"%s", value[i].c_str());
			theLogAttr.attrs[i].value = cesdk.fns.CEM_AllocateString(value[i].c_str());

		}
	}
	

	CEResult_t ret = cesdk.fns.CELOGGING_LogObligationData(m_connectHandle,theLogID,theObligationName,&theLogAttr);

	if(ret == CE_RESULT_SUCCESS)
	{
		NLPRINT_DEBUGVIEWLOG(L"Write the obligation into log reporter successfully!!!!!!!!!!!");
	}
	else
	{
		NLPRINT_DEBUGVIEWLOG(L"Write the obligation into log reporter failed!!!!!!!!!!!");
	}

	cesdk.fns.CEM_FreeString(theLogID);
	cesdk.fns.CEM_FreeString(theObligationName);
	if(theLogAttr.attrs != NULL)
	{
		for(int i=0;i<theLogAttr.count;i++)
		{
			cesdk.fns.CEM_FreeString(theLogAttr.attrs[i].key);
			cesdk.fns.CEM_FreeString(theLogAttr.attrs[i].value);
		}
		delete [] theLogAttr.attrs;
		theLogAttr.attrs=NULL;
	}

	if (ret == CE_RESULT_SUCCESS)
	{
		return true;
	}
	
	return false;
}

void PolicyCommunicator::SetLogID(const wchar_t* strID)
{
	assert(strID != NULL && wcslen(strID) > 0);
	m_strID = strID;
}

BOOL PolicyCommunicator::QueryPolicy(_In_ CQueryPCInfo& queryInfo, _Out_ BOOL &bConnectSuccess)
{

	CEResult_t res;
	CEint32 timeout_in_millisec  = m_nQueryPCTimeout;

	//connect PC
	if(NULL == PolicyCommunicator::m_connectHandle)
	{
		Connect2PolicyServer();
	}
	if(NULL == PolicyCommunicator::m_connectHandle)
	{
		bConnectSuccess = false;
		loge(L"Fail to connect to PDP. Return False.\n");
		CEventLog::WriteEventLog(L"OutlookEnforcer::Fail to connect to PC,Return False.",EVENTLOG_ERROR_TYPE,EVENTLOG_ERROR_INIT_ID);
		return FALSE;
	}
	bConnectSuccess = true;
	logd(L"[QueryPolicy]bConnectSuccess=%d\n", bConnectSuccess);
	//Query pc
	res = cesdk.fns.CEEVALUATE_CheckResourcesEx(PolicyCommunicator::m_connectHandle,
        queryInfo.GetRequest(), (CEint32)queryInfo.GetRequestNumber(),
		NULL, CEFalse,
		(CEint32) PolicyCommunicator::m_ulIp,
		 queryInfo.GetEnforcement(),
         timeout_in_millisec);
	logd(L"[QueryPolicy]CEEVALUATE_CheckResourcesEx return %d\n", res);
	CEventLog::WriteEventLog(L"Leave CEEVALUATE_CheckResourcesEx --------------------------",EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);
	if( res == CE_RESULT_CONN_FAILED || res == CE_RESULT_NULL_CEHANDLE ||CE_RESULT_THREAD_NOT_INITIALIZED )
	{
		Disconnect2PolicyServer() ;

		Connect2PolicyServer();
		CEventLog::WriteEventLog(L"Call CEEVALUATE_CheckResourcesEx again--------------------------",EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);
		res = cesdk.fns.CEEVALUATE_CheckResourcesEx(PolicyCommunicator::m_connectHandle,
			queryInfo.GetRequest(),(CEint32)queryInfo.GetRequestNumber(),
			NULL, CEFalse,
			(CEint32) PolicyCommunicator::m_ulIp,
			queryInfo.GetEnforcement(),
            timeout_in_millisec);
		CEventLog::WriteEventLog(L"Leave CEEVALUATE_CheckResourcesEx again--------------------------",EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);
	}
    return CE_RESULT_SUCCESS == res;
}