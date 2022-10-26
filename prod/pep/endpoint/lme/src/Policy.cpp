#include "stdafx.h"
#include <Sddl.h>
#include "policy.h"
#include <time.h>
#include <WinSock2.h>
#include "PartDB.h"
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")
#define TIMEOUT_QUERY_POLICY	30000


#define REG_KEY_INSTALL_PATH   "SOFTWARE\\NextLabs\\CommonLibraries"
#define REG_KEY_INSTALL        L"InstallDir"
#define CONTROL_MANAGER_UUID L"b67546e2-6dc7-4d07-aa8a-e1647d29d4d7"

extern CRITICAL_SECTION g_csPolicyInstance;

bool IsPolicyControllerUp(void)
{
	HANDLE hPIDFileMapping = OpenFileMappingW(FILE_MAP_WRITE,FALSE,CONTROL_MANAGER_UUID);
	bool result = false;
	if( hPIDFileMapping != NULL )
	{
		result = true;
		CloseHandle(hPIDFileMapping);
	}
	return result;
}

 void GetFQDN(LPCWSTR hostname, LPWSTR fqdn, int nSize)
{
    char szHostName[1001] = {0};
    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, hostname,(int) wcslen(hostname), szHostName, 1000, NULL, NULL);

    hostent* hostinfo;
    hostinfo = gethostbyname(szHostName);
    if(hostinfo && hostinfo->h_name)
    {
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, hostinfo->h_name,(int) strlen(hostinfo->h_name), fqdn, nSize);
        //DP((L"[GetFQDN] OK! %s\n", fqdn));
    }
    else
    {
        wcsncpy_s(fqdn,nSize, hostname, nSize);
        //DP((L"[GetFQDN] Fail! %s\n", hostname));
    }
}

static BOOL GetFQDNPath(LPCWSTR src, LPWSTR fqdnpath, int nSize)
{
//    BOOL	bGet = FALSE;
    WCHAR   wzHostName[MAX_PATH+1];
    WCHAR   wzFqdnName[MAX_PATH+1];
    int     nLeftSize = nSize;
    memset(wzHostName, 0, sizeof(wzHostName));
    memset(wzFqdnName, 0, sizeof(wzFqdnName));

    if(2 >= wcslen(src))
        return FALSE;
    if(L'\\'!=src[0] || L'\\'!=src[1])
        return FALSE;

    wcsncpy_s(wzHostName,MAX_PATH, (src+2), MAX_PATH);
    WCHAR* pHostEnd = wcsstr(wzHostName, L"\\");
    const WCHAR* pHostEnd2= wcsstr((src+2), L"\\");
    if(pHostEnd) *pHostEnd = 0;

    GetFQDN(wzHostName, wzFqdnName, MAX_PATH);
    wcsncpy_s(fqdnpath, nSize,L"\\\\", nLeftSize); nLeftSize -= 2;
    wcsncat_s(fqdnpath,nSize, wzFqdnName, nLeftSize); nLeftSize -= (int)wcslen(fqdnpath);
    if(pHostEnd2 && nLeftSize>0)
    {
        wcsncat_s(fqdnpath, nSize,pHostEnd2, nLeftSize);
    }

    return TRUE;
}

static BOOL IsLocalDrive(LPCWSTR wzPath)
{
    if(wcslen(wzPath) < 2)
        return FALSE;
    if(L':'!=wzPath[1] || wzPath[0] < L'A' ||  (wzPath[0] > L'Z' && wzPath[0] < L'a') || wzPath[0] > L'z')
        return FALSE;

    return TRUE;
}

#define NETMAP_HEAD	L"\\Device\\LanmanRedirector\\;"
static BOOL IsMapped(LPCWSTR wzPath, LPWSTR wzRealPath, int cch)
{
    BOOL bIsMapped = FALSE;
    if(IsLocalDrive(wzPath))
    {
        WCHAR wzDriver[3]; wzDriver[0] = wzPath[0]; wzDriver[1]=L':'; wzDriver[2] = 0;
        WCHAR wzRealDriver[MAX_PATH+1];	memset(wzRealDriver, 0, sizeof(wzRealDriver));
        int nLeft = 0;
        if(0 < QueryDosDeviceW(wzDriver, wzRealDriver, MAX_PATH))
        {
            if(0 == wcsncmp(wzRealDriver, L"\\??\\", 4))
            {
                bIsMapped = TRUE;
                wcsncpy_s(wzRealPath,cch, wzRealDriver+4, cch);
                nLeft = cch - (int)wcslen(wzRealPath);
                wcsncat_s(wzRealPath,cch, wzPath+2, nLeft);
            }
            else if(0 == _wcsnicmp(wzRealDriver, NETMAP_HEAD, (int)wcslen(NETMAP_HEAD)))
            {
                bIsMapped = TRUE;
                WCHAR* pStart = wcsstr(wzRealDriver+(int)wcslen(NETMAP_HEAD), L"\\");
                if(pStart)//((int)wcslen(wzRealDriver) >  (int)wcslen(NETMAP_HEAD)+18)
                {
                    wcsncpy_s(wzRealPath,cch, L"\\", cch);
                    wcsncpy_s(wzRealPath+1,cch-1, pStart, cch-1);//wzRealDriver+(int)wcslen(NETMAP_HEAD)+18, cch);

                    // Now we get the remote path here
                    nLeft = cch - (int)wcslen(wzRealPath);
                    wcsncat_s(wzRealPath,cch, wzPath+2, nLeft);
                }
                else
                {
                    wcsncpy_s(wzRealPath,MAX_PATH, wzPath, cch);
                }

            }
        }
    }

    return bIsMapped;
}

static void GetRealPath(LPCWSTR wzPath, LPWSTR wzRealPath, int cch)
{
    WCHAR wzTempPath[MAX_PATH+1];	memset(wzTempPath, 0, sizeof(wzTempPath));

    wcsncpy_s(wzTempPath,MAX_PATH, wzPath, MAX_PATH);
    while(IsMapped(wzTempPath, wzRealPath, cch))
    {
        wcsncpy_s(wzTempPath,MAX_PATH, wzRealPath, MAX_PATH);
    }
    wcsncpy_s(wzRealPath,cch, wzTempPath, MAX_PATH);
}
time_t getSystemTime(PSYSTEMTIME pSysTime = NULL ) 
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
}

BOOL getFileLastModifiedTime( const wchar_t *pszFileName, wchar_t* pszTimeBuf, INT iTimeLen ) 
{
	if(!pszFileName || !pszTimeBuf)
	{
		return 0;
	}
	BOOL bRet = FALSE;
	memset(pszTimeBuf, 0, iTimeLen*sizeof(WCHAR));
	WIN32_FILE_ATTRIBUTE_DATA attributes ;
	bRet = GetFileAttributesExW(  pszFileName, GetFileExInfoStandard, &attributes ) ;
	if( bRet )
	{
		SYSTEMTIME stModifyTime;
		if(FileTimeToSystemTime(&attributes.ftLastWriteTime/*ftLocalLastModify*/, &stModifyTime))
		{
			time_t tmModify;

			tmModify = getSystemTime(&stModifyTime);
			swprintf_s(pszTimeBuf, iTimeLen, L"%I64d", tmModify);
			bRet = TRUE;
		}
	}
	return bRet ;
}
int                 CPolicy::m_nRef = 0;
BOOL                CPolicy::m_bFirstInit = TRUE;
CPolicy* CPolicy::m_pThis = 0;

wchar_t    CPolicy::m_wzSID[SID_LEN]          = {0};
wchar_t    CPolicy::m_wzUserName[SID_LEN]     = {0};
wchar_t    CPolicy::m_wzHostName[SID_LEN]     = {0};
wchar_t    CPolicy::m_wzAppName[MAX_PATH]        = {0};
wchar_t    CPolicy::m_wzAppPath[MAX_PATH] = {0};
CEHandle CPolicy::m_connectHandle       = NULL;
ULONG    CPolicy::m_ulIp                = 0;

//CESDK::Handle CPolicy::m_sdk;
//BOOL	CPolicy::m_bSDK = FALSE;

CPolicy::CPolicy()
{
	 //InitializeCriticalSection(&CPolicy::m_cs);
}
CPolicy::~CPolicy()
{}
bool CPolicy::LoadSDK(  )
{
	bool bRet = false ;
	LONG rstatus;
	HKEY hKey = NULL; 

	rstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
		REG_KEY_INSTALL_PATH,
		0,KEY_QUERY_VALUE,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		return false;
	}

	WCHAR enforcer_root[MAX_PATH] = {0};                 /* InstallDir */
	DWORD enforcer_root_size = sizeof(enforcer_root);

	rstatus = RegQueryValueExW(hKey,REG_KEY_INSTALL,NULL,NULL,(LPBYTE)enforcer_root,&enforcer_root_size);
	RegCloseKey(hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		return false;
	}
	if( wcsstr( enforcer_root,L"\\bin32" ) == NULL )
	{
		std::wstring strRoot  =	   enforcer_root ;
		strRoot.append(L"bin32\\")   ;
		bRet = m_cesdk.load(strRoot.c_str() );
	}
	else
	{
		bRet = m_cesdk.load(enforcer_root);
	}
	if( m_cesdk.is_loaded() == false ) 
	{
		bRet = false ;
	}

	return bRet ;

}
DWORD CPolicy::GetLocalIP() 
{
	DWORD dIP = 0 ;
	char   szHostname[100] = {0};   
	HOSTENT   *pHostEnt;   
	in_addr   inAddr;

	gethostname(szHostname, sizeof(szHostname));   
	pHostEnt = gethostbyname(szHostname);   
	if(!pHostEnt)
		return 0;
	memcpy(&inAddr.S_un,   pHostEnt->h_addr,   pHostEnt->h_length);
	dIP =  ntohl(inAddr.S_un.S_addr);
	return dIP ;
}

BOOL CPolicy::GetWindowUserInfo(  wchar_t *pszSid, INT iBufSize, wchar_t* pszUserName, INT inbufLen )
{
	BOOL bRet = TRUE;
	HANDLE hTokenHandle = 0;

	if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hTokenHandle))
	{
		if(GetLastError() == ERROR_NO_TOKEN)
		{
			if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTokenHandle ))
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}
	// Get SID
	UCHAR InfoBuffer[512] = {0};
	DWORD cbInfoBuffer = 512;
	wchar_t* StringSid = 0;
	wchar_t   uname[64] = {0}; DWORD unamelen = 63;
	wchar_t   dname[64] = {0}; DWORD dnamelen = 63;
	wchar_t   fqdnname[MAX_PATH+1]; memset(fqdnname, 0, sizeof(fqdnname));
	SID_NAME_USE snu;
	int     nLeftSize = inbufLen;
	if(!GetTokenInformation(hTokenHandle, TokenUser, InfoBuffer, cbInfoBuffer, &cbInfoBuffer))
	{
		return FALSE;
	}

	if(ConvertSidToStringSid(((PTOKEN_USER)InfoBuffer)->User.Sid, &StringSid))
	{
		::wcsncpy_s(pszSid, iBufSize, StringSid, wcslen(StringSid));
		if(StringSid) LocalFree(StringSid);
	}
	if(LookupAccountSid(NULL, ((PTOKEN_USER)InfoBuffer)->User.Sid, uname, &unamelen, dname, &dnamelen, &snu))   
	{
		wcsncat_s( pszUserName,nLeftSize, uname, nLeftSize);
		nLeftSize -= (int)wcslen(pszUserName);

		char  szHostname[MAX_PATH+1]; memset(szHostname, 0, sizeof(szHostname));
		WCHAR wzHostname[MAX_PATH+1]; memset(wzHostname, 0, sizeof(wzHostname));
		gethostname(szHostname, MAX_PATH);
		if(0 != szHostname[0])
		{
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szHostname, -1, wzHostname, MAX_PATH);
			wcsncat_s(pszUserName,nLeftSize, L"@", nLeftSize); nLeftSize--;
			GetFQDN(wzHostname, fqdnname, MAX_PATH);
			wcsncat_s(pszUserName,nLeftSize, fqdnname, nLeftSize);
		}
	}
	return bRet ;
}

void CPolicy::GetFQDN(wchar_t* hostname, wchar_t* fqdn, int nSize)
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
		wcsncpy_s(fqdn, nSize, hostname, wcslen(hostname));
	}
}

VOID CPolicy::InitLocalInfo(void)
{
	GetWindowUserInfo( m_wzSID,SID_LEN,  m_wzUserName,  SID_LEN ) ;
	m_ulIp = (ULONG)GetLocalIP() ;
	GetModuleFileName(NULL, m_wzAppPath, MAX_PATH);
	if( ( *m_wzAppPath ) != 0 )
	{
		LPCWSTR fileName = wcsrchr( m_wzAppPath, L'\\' ) ;
		if( fileName )
		{
			fileName = fileName+1 ;
			wcsncpy_s( m_wzAppName, MAX_PATH, fileName, wcslen(fileName) ) ;
		}
	}

}


CPolicy* CPolicy::CreateInstance()
{
	EnterCriticalSection(&g_csPolicyInstance);

    if(CPolicy::m_bFirstInit)
    {
        CPolicy::m_bFirstInit = FALSE;
        CPolicy::InitLocalInfo();
    }

    assert(0 <= m_nRef && INT_MAX > m_nRef);
    if (0 == m_nRef)
    {
        assert(NULL == m_pThis);
        m_pThis = new(std::nothrow) CPolicy();
		m_pThis->LoadSDK() ;
    }

    if (NULL != m_pThis)
    {
        m_nRef++;
    }

    CPolicy *temp = m_pThis;

    LeaveCriticalSection(&g_csPolicyInstance);

    return temp;
}

void CPolicy::Release()
{
	EnterCriticalSection(&g_csPolicyInstance);

    assert(0 < m_nRef);
    m_nRef--;

    if (0 == m_nRef)
    {
	//	Disconnect2PolicyServer();
        assert(NULL != m_pThis);
        delete m_pThis;
        m_pThis = NULL;
    }
	
    LeaveCriticalSection(&g_csPolicyInstance);
}
static DWORD NLGetLongPathName(const std::wstring& filename, LPWSTR lpszLongPath, DWORD cchBuffer)
{
	DWORD dwLen = GetLongPathName(filename.c_str(), lpszLongPath, cchBuffer);
	DWORD dwErr = GetLastError();
	if(dwLen == 0 && dwErr == ERROR_FILE_NOT_FOUND)
	{		
		HANDLE hFile = CreateFileW(filename.c_str(),                // name of the write
								   GENERIC_WRITE,          // open for writing
								   0,                      // do not share
								   NULL,                   // default security
								   CREATE_NEW,          // overwrite existing
								   FILE_ATTRIBUTE_NORMAL,  // normal file
								   NULL);                  // no attr. template

		if ( hFile ==(HANDLE) -1 ) 
	   {
		   dwLen = GetLongPathName(filename.c_str(), lpszLongPath, cchBuffer);			 
		   CloseHandle(hFile);  
		   DeleteFileW(filename.c_str());
	   }
	   
	}
	return dwLen;
}

BOOL CPolicy::Connect2PolicyServer()
{
	if( !this->m_cesdk.is_loaded() )
	{
		return FALSE;
	}

	CEApplication   app;
	CEUser          user;
	//CEString        host;
	CEint32         timeout_in_millisec = TIMEOUT_QUERY_POLICY;
	BOOL            bRet = FALSE;

	// Evaluate
	memset(&app, 0, sizeof(CEApplication));
	memset(&user, 0, sizeof(CEUser));
	app.appName   = m_cesdk.fns.CEM_AllocateString(CPolicy::m_wzAppName);
	app.appPath   = m_cesdk.fns.CEM_AllocateString(CPolicy::m_wzAppPath);
	user.userID   = m_cesdk.fns.CEM_AllocateString(CPolicy::m_wzSID);
	user.userName = m_cesdk.fns.CEM_AllocateString(CPolicy::m_wzUserName);
	//host          = m_cesdk.fns.CEM_AllocateString(L"10.187.2.111");//10.187.2.111;10.187.4.183(L"vagartst07.test.bluejungle.com");//10.187.6.136(L"10.187.4.161");//(L"10.187.4.176");
	CEResult_t result = m_cesdk.fns.CECONN_Initialize(app, user, NULL/**//*host*/, &CPolicy::m_connectHandle, timeout_in_millisec) ;
	if(CE_RESULT_SUCCESS ==result 
		&& NULL!=CPolicy::m_connectHandle)
	{
		DPW((L"Succeed on to initialize CECONN!\n"));
		bRet = TRUE;
	}
	else
	{
		DPW((L"Failed to initialize CECONN!\n"));
	}
	//m_cesdk.fns.CEM_FreeString(host);
	m_cesdk.fns.CEM_FreeString(app.appName);
	m_cesdk.fns.CEM_FreeString(app.appPath);
	m_cesdk.fns.CEM_FreeString(user.userID);
	m_cesdk.fns.CEM_FreeString(user.userName);

	return bRet;
}
void CPolicy::Disconnect2PolicyServer()
{
	if( !this->m_cesdk.is_loaded() )
	{
		return ;
	}

	if(NULL != CPolicy::m_connectHandle)
	{
		CEint32         timeout_in_millisec = TIMEOUT_QUERY_POLICY;
		m_cesdk.fns.CECONN_Close(CPolicy::m_connectHandle, timeout_in_millisec);
		CPolicy::m_connectHandle = NULL;
	}
}
CEAction_t getActionTypeByName( const wchar_t *szAction )
{
	CEAction_t strAction = CE_ACTION_WM_SHARE ;
	if( _wcsnicmp( szAction,L"IM",wcslen(szAction)) == 0 )
	{
		strAction = CE_ACTION_IM_FILE ;
	}else if( _wcsnicmp( szAction,L"RECORD",wcslen(szAction)) == 0 ){
		strAction = CE_ACTION_WM_RECORD ;
	}
	else if( _wcsnicmp( szAction,L"QUESTION",wcslen(szAction)) == 0 ){
		strAction = CE_ACTION_WM_QUESTION ;
	}
	else if( _wcsnicmp( szAction,L"VOICE",wcslen(szAction)) == 0 ){
		strAction = CE_ACTION_WM_VOICE ;
	}	else if( _wcsnicmp( szAction,L"VIDEO",wcslen(szAction)) == 0 ){
		strAction = CE_ACTION_WM_VIDEO ;
	}	else	if( _wcsnicmp( szAction,L"JOIN",wcslen(szAction)) == 0 ){
		strAction = CE_ACTION_WM_JOIN ;
	}	else	if( _wcsnicmp( szAction,L"MEETING",wcslen(szAction)) == 0 ){
		strAction = CE_ACTION_MEETING ;
	}
	return strAction ;
}

BOOL CPolicy::QueryPolicy( wchar_t* action, LPCWSTR wzAttachment, std::map<std::wstring, std::wstring>& mapAttributes, CEEnforcement_t& pEnforcement,STRINGLIST& pwzRecipients )
{
	/*if( !pEval_data)
	{
		return FALSE;
	}*/
	BOOL bRet =  false ;

//	LPEVALDATA pData = (LPEVALDATA)pEval_data;

	if(nextlabs::policy_controller::is_up() == false)//return CEAllow if PC is stopped.
	{
		pEnforcement.result = CEAllow;
		return TRUE;
	}

	CEApplication app ;
	CEUser user ;
	CEint32         timeout_in_millisec = TIMEOUT_QUERY_POLICY;
	CEint32         numRecipients = (CEint32)pwzRecipients.size();

	CEString        *recipients   = NULL;
	if( m_cesdk.fns.CEM_AllocateString == NULL )
	{	  
		return TRUE ;
		
	}
	CEAction_t strAction = getActionTypeByName( action) ;
	//CEString        strAction = m_cesdk.fns.CEM_AllocateString(action);
	//CEString        srcFile = m_cesdk.fns.CEM_AllocateString(wzAttachment);
	::ZeroMemory( &app,sizeof( CEApplication ) ) ;
	::ZeroMemory( &user,sizeof( CEUser ) ) ;
	::ZeroMemory( &pEnforcement, sizeof( CEEnforcement_t ) ) ;

	app.appURL	= m_cesdk.fns.CEM_AllocateString( L"" ) ;
	app.appPath = m_cesdk.fns.CEM_AllocateString( m_wzAppPath ) ;
	app.appName = m_cesdk.fns.CEM_AllocateString( m_wzAppName ) ;

	CEString resource =m_cesdk.fns.CEM_AllocateString(wzAttachment ) ; ;
	//CEString action =	m_cesdk.fns.CEM_AllocateString( strAction.c_str() ) ;
	CEAttributes psrcAttributes ={0};
	psrcAttributes.attrs = NULL ;

	CEAttributes pdestAttributes ;
	pdestAttributes.attrs = NULL ;


	std::wstring strLastModifiedTime;
	bRet = GetFileLastModifiedTime( wzAttachment, strLastModifiedTime ) ;
	if(bRet)
	{
		mapAttributes[L"modified_date"] = strLastModifiedTime;
	}
	mapAttributes[L"ce::request_cache_hint"] = L"yes";

	SetAttributes(mapAttributes, &psrcAttributes);
std::wstring strID =  CPartDB::GetInstance()->GetLocalPart().GetSIP() ;
	std::transform( strID.begin(), strID.end(), strID.begin(), tolower ) ;

	user.userID		=  m_cesdk.fns.CEM_AllocateString( strID.c_str()  ) ;
	user.userName	=  m_cesdk.fns.CEM_AllocateString( CPartDB::GetInstance()->GetLocalPart().GetSIP().c_str()  ) ;
	CEResult_t result = CE_RESULT_GENERAL_FAILED;
	recipients    = new CEString[numRecipients];
	if(NULL == recipients) goto _CLEAN_EXIT;
	for (int i=0; i<numRecipients; i++)
	{
		DPW((pwzRecipients[i].c_str()));
		recipients[i] = m_cesdk.fns.CEM_AllocateString(pwzRecipients[i].c_str());
	}
	for( int i = 0 ; i<2 ; i++ )
	{
		if( m_connectHandle == NULL )
		{
			Connect2PolicyServer() ;
		}
		try{
			result =  m_cesdk.fns.CEEVALUATE_CheckMessageAttachment(m_connectHandle, 
				strAction,
				resource,
				&psrcAttributes,
				numRecipients,
				recipients ,	
				(CEint32)0,
				&user,
				NULL,
				&app,  
				NULL,	
				CETrue,
				CE_NOISE_LEVEL_USER_ACTION,
				&pEnforcement,
				timeout_in_millisec ) ;
		}catch(...)
		{
			wchar_t szBuf[50] = {0} ;
			swprintf( szBuf, 50,L"Error Code:%d",   result ) ;
			::DPW(( szBuf) );
			bRet = FALSE;
		}
		if( result !=   CE_RESULT_CONN_FAILED )
		{
			break ;
		}
		m_connectHandle = NULL ;
	}
	if( result == CE_RESULT_SUCCESS ) 
	{
		if(pEnforcement.result == CEAllow)
		{
			::DPW(( L"Evaluation allow" )) ;
			bRet = TRUE ;
		}
		else
		{
			::DPW(( L"Evaluation Deny" )) ;
			bRet = FALSE ;
		}
	}
//	if( result == CE_RESULT_SUCCESS ) 
//	{
//		bRet = TRUE;
//		
//		BOOL bAllow = TRUE; /* default */
//
//		// Enforcement is allow?
//		bAllow = (pEnforcement.result == CEAllow);
//
////		DPW((L"HPE::Evaluation result: \n action: %s \n source file: %s \n destination file: %s \n result (allow): %d", operation.c_str(), evalInfo.pszSrcFileName.c_str(), evalInfo.pszDestFileName.c_str(), bAllow));
//
//		// Obligations exist (structure is populated)?
//		bool obs_exist = (pEnforcement.obligation != NULL && pEnforcement.obligation->attrs != NULL);
//		if( bAllow && obs_exist == true )
//		{
//			bool commit = false;
//			std::size_t commit_ttl = 60;
//			for( int i = 0 ; i < pEnforcement.obligation->count ; i++ )
//			{
//				const wchar_t* key   = m_cesdk.fns.CEM_GetString(pEnforcement.obligation->attrs[i].key);
//				const wchar_t* value = m_cesdk.fns.CEM_GetString(pEnforcement.obligation->attrs[i].value);
//				if( key == NULL || value == NULL )
//				{
//					continue;
//				}
//				if( _wcsicmp(key,L"CE_CACHE_HINT") == 0 )
//				{
//					commit = true;              // Commit to cache
//					commit_ttl = _wtoi(value);  // Commit TTL
//				}
//			}
//			if( commit == true )
//			{
//				EnterCriticalSection(&m_cesdk.fns.cs);
//	//			cache.commit(strAction.c_str(), (pData->strSrc + pData->strDest).c_str(), commit_ttl, mod_time);
//	//			DPW((L"has commit allow cache %s, %s", operation.c_str(), (evalInfo.pszSrcFileName + evalInfo.pszDestFileName).c_str()));
//				LeaveCriticalSection(&m_cesdk.fns.cs);
//			}
//		}
//	}
_CLEAN_EXIT:
	m_cesdk.fns.CEM_FreeString( app.appName ) ;
	m_cesdk.fns.CEM_FreeString( app.appPath ) ;
	//m_cesdk.fns.CEM_FreeString( srcFile ) ;
	m_cesdk.fns.CEM_FreeString( app.appURL	)  ;
	m_cesdk.fns.CEM_FreeString( user.userID ) ;
	m_cesdk.fns.CEM_FreeString( user.userName ) ;
	m_cesdk.fns.CEM_FreeString( resource ) ;
	//m_cesdk.fns.CEM_FreeString( action ) ;
	if( psrcAttributes.attrs )
	{
		for( CEint32 i=0 ;   i<psrcAttributes.count ;  ++i )
		{
			m_cesdk.fns.CEM_FreeString( psrcAttributes.attrs[i].key ) ;
			m_cesdk.fns.CEM_FreeString( psrcAttributes.attrs[i].value ) ;
		}
		delete[] psrcAttributes.attrs ;
	}
	
	return	bRet ;
}

BOOL CPolicy::QueryPolicyWithApp( wchar_t* action, LPCWSTR wzAttachment, std::map<std::wstring, std::wstring>& mapAttributes,LPCWSTR wzApplication, std::map<std::wstring, std::wstring>& appAttributes, CEEnforcement_t& pEnforcement,STRINGLIST& pwzRecipients,const wchar_t* pszDest )
{
	/*if( !pEval_data)
	{
		return FALSE;
	}*/
	BOOL bRet =  false ;

//	LPEVALDATA pData = (LPEVALDATA)pEval_data;
	if(nextlabs::policy_controller::is_up() == false)//return CEAllow if PC is stopped.
	{
		pEnforcement.result = CEAllow;
		return TRUE;
	}
	CEApplication app ;
	CEUser user ;
	CEint32         timeout_in_millisec = TIMEOUT_QUERY_POLICY;
	CEint32         numRecipients = (CEint32)pwzRecipients.size();
	CEString        *recipients   = NULL;
	if( m_cesdk.fns.CEM_AllocateString == NULL )
	{	  
		::DPW(( L"==================SDK load failure!=============================")) ;
		return TRUE ;
		
	}
	
	
	//CEString        strAction = m_cesdk.fns.CEM_AllocateString(action);
	//CEString        srcFile = m_cesdk.fns.CEM_AllocateString(wzAttachment);
	::ZeroMemory( &app,sizeof( CEApplication ) ) ;
	::ZeroMemory( &user,sizeof( CEUser ) ) ;
	::ZeroMemory( &pEnforcement, sizeof( CEEnforcement_t ) ) ;

	app.appURL	= m_cesdk.fns.CEM_AllocateString( L"" ) ;
	app.appPath = m_cesdk.fns.CEM_AllocateString( wzApplication ) ;
	app.appName = m_cesdk.fns.CEM_AllocateString( m_wzAppName ) ;
	CEAction_t strAction = getActionTypeByName( action) ;
	CEString resource =m_cesdk.fns.CEM_AllocateString(wzAttachment ) ; 
	CEResource* destination = NULL ;
	if(pszDest!=NULL)
	{ 
		destination = m_cesdk.fns.CEM_CreateResourceW(pszDest, L"fso" ) ; ;
	}
	//CEString action =	m_cesdk.fns.CEM_AllocateString( strAction.c_str() ) ;
	CEAttributes pAppAttributes ={0};

	pAppAttributes.attrs = NULL ;
	 
	CEAttributes pSrcAttributes ={0};
	pSrcAttributes.attrs = NULL ;
	CEAttributes pdestAttributes ;
	pdestAttributes.attrs = NULL ;


	std::wstring strLastModifiedTime;
	bRet = GetFileLastModifiedTime( wzAttachment, strLastModifiedTime ) ;
	if(bRet)
	{
		mapAttributes[L"modified_date"] = strLastModifiedTime;
	}
   ::DPW(( L"==================2=============================") );
	mapAttributes[L"ce::request_cache_hint"] = L"yes";
	appAttributes[L"Tool"]	    = wzApplication ;
	SetAttributes(appAttributes, &pAppAttributes);
	SetAttributes(mapAttributes, &pSrcAttributes);

	std::wstring strID =  CPartDB::GetInstance()->GetLocalPart().GetSIP() ;
	std::transform( strID.begin(), strID.end(), strID.begin(), tolower ) ;
	user.userID		=  m_cesdk.fns.CEM_AllocateString(strID.c_str()  ) ;
	user.userName	=  m_cesdk.fns.CEM_AllocateString( CPartDB::GetInstance()->GetLocalPart().GetSIP().c_str()  ) ;
	CEResult_t result = CE_RESULT_GENERAL_FAILED;
	recipients    = new CEString[numRecipients];
	if(NULL == recipients) goto _CLEAN_EXIT;
	for (int i=0; i<numRecipients; i++)
	{
		DPW((pwzRecipients[i].c_str()));
		recipients[i] = m_cesdk.fns.CEM_AllocateString(pwzRecipients[i].c_str());
	}
	::DPW(( L"==================3=============================")) ;
	for( int i = 0 ; i<2 ; i++ )
	{
		if( m_connectHandle == NULL )
		{
			Connect2PolicyServer() ;
		}
		try{
			result =  m_cesdk.fns.CEEVALUATE_CheckMessageAttachment(m_connectHandle, 
				strAction,
				resource,
				&pSrcAttributes,
				numRecipients,
				recipients ,			
				(CEint32)0,
				&user,
				NULL,
				&app,  
				&pAppAttributes,	
				CETrue,
				CE_NOISE_LEVEL_USER_ACTION,
				&pEnforcement,
				timeout_in_millisec ) ;
			DPW( (L"Evaluation result:%d", result ) );
		}catch(...)
		{
			wchar_t szBuf[50] = {0} ;
			swprintf( szBuf, 50,L"Error Code:%d",   result ) ;
			::DPW(( szBuf) );
			bRet = FALSE;
		}
		if( result !=   CE_RESULT_CONN_FAILED )
		{
			break ;
		}
		m_connectHandle = NULL ;
	}
	if( result == CE_RESULT_SUCCESS ) 
	{
		if(pEnforcement.result == CEAllow)
		{
			::DPW(( L"Evaluation allow" )) ;
			bRet = TRUE ;
		}
		else
		{
			::DPW(( L"Evaluation Deny") ) ;
			bRet = FALSE ;
		}
	}
_CLEAN_EXIT:
	m_cesdk.fns.CEM_FreeString( app.appName ) ;
	m_cesdk.fns.CEM_FreeString( app.appPath ) ;
	m_cesdk.fns.CEM_FreeString( app.appURL	)  ;
	m_cesdk.fns.CEM_FreeString( user.userID ) ;
	m_cesdk.fns.CEM_FreeString( user.userName ) ;
	m_cesdk.fns.CEM_FreeString( resource ) ;
	if( pAppAttributes.attrs )
	{
		for( CEint32 i=0 ;   i<pAppAttributes.count ;  ++i )
		{
			m_cesdk.fns.CEM_FreeString( pAppAttributes.attrs[i].key ) ;
			m_cesdk.fns.CEM_FreeString( pAppAttributes.attrs[i].value ) ;
		}
		delete[] pAppAttributes.attrs ;
	}
	if( pSrcAttributes.attrs )
	{
		for( CEint32 i=0 ;   i<pSrcAttributes.count ;  ++i )
		{
			m_cesdk.fns.CEM_FreeString( pSrcAttributes.attrs[i].key ) ;
			m_cesdk.fns.CEM_FreeString( pSrcAttributes.attrs[i].value ) ;
		}
		delete[] pSrcAttributes.attrs ;
	}
	
	return	bRet ;
}

BOOL CPolicy::GetFileLastModifiedTime( const std::wstring & strFileName, std::wstring& strLastModifiedTime) 
{
	//if(!CPolicy::m_bSDK)
	//{
	//	return FALSE;
	//}

	if(	 strFileName.length() == 0 )
	{
		return	FALSE ;
	}
	wchar_t pszLastModifyTime[MAX_PATH] = {0} ;
	if(strFileName.length() > 0)
	{
		getFileLastModifiedTime( strFileName.c_str(), pszLastModifyTime,MAX_PATH )	;
	}
	
	if( ( *pszLastModifyTime ) == 0 )
	{
		time_t systm =getSystemTime() ;
		swprintf_s(pszLastModifyTime, MAX_PATH, L"%I64d", systm);
	}

	strLastModifiedTime = std::wstring(pszLastModifyTime);

// 	pAttributes->attrs = new  CEAttribute[2]  ;
// 	pAttributes->count = 2 ;
// 	pAttributes->attrs[0].key		= m_cesdk.fns.CEM_AllocateString(L"modified_date")	; 
// 	pAttributes->attrs[0].value		= m_cesdk.fns.CEM_AllocateString(pszLastModifyTime) ; 
//  	pAttributes->attrs[1].key		= m_cesdk.fns.CEM_AllocateString(L"ce::request_cache_hint");
//  	pAttributes->attrs[1].value		= m_cesdk.fns.CEM_AllocateString(L"yes");

	return TRUE; 
}

BOOL CPolicy::SetAttributes(const std::map<std::wstring, std::wstring>& mapAttributes, CEAttributes *pAttribute)
{
	if( !pAttribute)
	{
		return FALSE;
	}

	pAttribute->count = (CEint32)mapAttributes.size();
	pAttribute->attrs = new CEAttribute[pAttribute->count];

	if (pAttribute->attrs)
	{
		int i = 0;
		for(std::map<std::wstring, std::wstring>::const_iterator itr = mapAttributes.begin(); itr != mapAttributes.end(); itr++)
		{
			pAttribute->attrs[i].key = m_cesdk.fns.CEM_AllocateString((*itr).first.c_str());
			pAttribute->attrs[i].value = m_cesdk.fns.CEM_AllocateString((*itr).second.c_str());
			i++;
		}
		return TRUE;
	}
	
	return FALSE;
}
BOOL CPolicy::QueryLiveMeetingPolicy( const wchar_t* pstrAttachment ,wchar_t* action)
{
	
    return QueryLiveMeetingPolicy( pstrAttachment, action,0 );
}

BOOL CPolicy::QueryLiveMeetingPolicy( const wchar_t* pstrAttachment,wchar_t* action, STRINGLIST* pAttendees  )
{
	BOOL bRet = FALSE;
    wchar_t wzSrc[MAX_SRC_PATH_LENGTH+1] = {0}; 
    wchar_t wzResolved[MAX_SRC_PATH_LENGTH+1] = {0}; 
    wchar_t wzFqdnPath[MAX_SRC_PATH_LENGTH+1] = {0}; 

    if( pstrAttachment == 0 || wcslen( pstrAttachment ) == 0 )
    {
        if( GetFQDNPath( L"C:\\No_attachment.ice", wzFqdnPath, MAX_SRC_PATH_LENGTH))
        {      
            wcsncpy_s( wzSrc,MAX_SRC_PATH_LENGTH, wzFqdnPath, MAX_SRC_PATH_LENGTH);
        }
        else
        {
            wcscpy_s( wzSrc,MAX_SRC_PATH_LENGTH, L"C:\\No_attachment.ice" );
        }
    }
    else
    {
        wcscpy_s( wzSrc, MAX_SRC_PATH_LENGTH,pstrAttachment );
    }

    GetRealPath( wzSrc, wzResolved, MAX_SRC_PATH_LENGTH);
    if( GetFQDNPath( wzResolved, wzFqdnPath, MAX_SRC_PATH_LENGTH))
    {
        memset( wzResolved, 0, sizeof(WCHAR)*MAX_SRC_PATH_LENGTH);
        wcsncpy_s( wzResolved, MAX_SRC_PATH_LENGTH,wzFqdnPath, MAX_SRC_PATH_LENGTH);
    }

    CEEnforcement_t anEnforcement;
    STRINGLIST DenyList;

    

    if( !pAttendees || pAttendees->empty() )
    {        
        STRINGLIST Attendee;
        
        CPartDB::GetInstance()->GetSIPList( Attendee );

		::DPW(( L"attendee(s):" ));
        STRINGLIST::iterator it = Attendee.begin();
        for( ; it != Attendee.end(); ++it )
        {
            DPW(( (*it).c_str() ));
        }

       // DPW((TEXT( "Query Policy Server......" ));
        std::map<std::wstring, std::wstring> mapAttributes ;
		 bRet = QueryPolicy(action, wzSrc, /*attachlist*/mapAttributes,  anEnforcement , Attendee);  
     
    }
    else
    {
        DPW(( TEXT("Attendee(s):") ));
        STRINGLIST::iterator it = pAttendees->begin();
        for( ; it != pAttendees->end(); ++it )
        {
            DPW(( (*it).c_str()) );
        }

      //  DPW((TEXT( "Query Policy Server......" ));
		std::map<std::wstring, std::wstring> mapAttributes ;
       bRet = QueryPolicy(action, wzSrc, /*attachlist*/mapAttributes,  anEnforcement , *pAttendees);      
    }

    //static DWORD dwTime = GetTickCount();
    //static std::wstring strLastRes = L"";
    //if( bRet == false && CPolicyDialog::IsActive() && 
    //    ( strLastRes != pstrAttachment || GetTickCount() - dwTime > 5000 ) )  
    //{            
    //    CPolicyDialog::EDenyReason eReason = CPolicyDialog::eReasonParticipant;
    //    if( pstrAttachment[0] != TEXT('[') 
    //        && !wcsstr( pstrAttachment, L"exe" )
    //        && !wcsstr( pstrAttachment, L"EXE" )  )
    //    {
    //        eReason = CPolicyDialog::eReasonFile;
    //        DPA(( "Reason is file." ));
    //    }
    //    //DPA(( "Reason is Part." );

    //    CPolicyDialog *pDialog = 0;
    //    {
    //        CResSwitch aResSwitch;
    //        pDialog = new CPolicyDialog();
    //    }
    //    pDialog->SetReasonContent( eReason, DenyList, pstrAttachment );        

    //    //pDialog->Create( CPolicyDialog::IDD );
    //    //pDialog->ShowWindow(SW_SHOW);  
    //    _beginthread( DiagThread, 0, pDialog );

    //    dwTime = GetTickCount();
    //    strLastRes = pstrAttachment;
    //}    

    //dwTime = GetTickCount();

    return bRet;
}





BOOL CPolicy::QueryLiveMeetingPolicyWithAppProp(   const wchar_t* pstrAttachment,const wchar_t* pstrApp, wchar_t* action, STRINGLIST* pAttendees ,const wchar_t* pszDest )
{
	BOOL bRet = FALSE;
    wchar_t wzSrc[MAX_SRC_PATH_LENGTH+1] = {0}; 
    wchar_t wzResolved[MAX_SRC_PATH_LENGTH+1] = {0}; 
    wchar_t wzFqdnPath[MAX_SRC_PATH_LENGTH+1] = {0}; 

    if( pstrAttachment == 0 || wcslen( pstrAttachment ) == 0 )
    {
        if( GetFQDNPath( L"C:\\No_attachment.ice", wzFqdnPath, MAX_SRC_PATH_LENGTH))
        {      
            wcsncpy_s( wzSrc,MAX_SRC_PATH_LENGTH, wzFqdnPath, MAX_SRC_PATH_LENGTH);
        }
        else
        {
            wcscpy_s( wzSrc,MAX_SRC_PATH_LENGTH, L"C:\\No_attachment.ice" );
        }
    }
    else
    {
        wcscpy_s( wzSrc, MAX_SRC_PATH_LENGTH,pstrAttachment );
    }

    GetRealPath( wzSrc, wzResolved, MAX_SRC_PATH_LENGTH);
    if( GetFQDNPath( wzResolved, wzFqdnPath, MAX_SRC_PATH_LENGTH))
    {
        memset( wzResolved, 0, sizeof(WCHAR)*MAX_SRC_PATH_LENGTH);
        wcsncpy_s( wzResolved, MAX_SRC_PATH_LENGTH,wzFqdnPath, MAX_SRC_PATH_LENGTH);
    }

    CEEnforcement_t anEnforcement;
    STRINGLIST DenyList;

    

    if( !pAttendees || pAttendees->empty() )
    {        
        STRINGLIST Attendee;
        
        CPartDB::GetInstance()->GetSIPList( Attendee );

		::DPW(( L"attendee(s):") );
        STRINGLIST::iterator it = Attendee.begin();
        for( ; it != Attendee.end(); ++it )
        {
            DPW(( (*it).c_str() ));
        }

       // DPW((TEXT( "Query Policy Server......" ));
        std::map<std::wstring, std::wstring> mapAttributes ;
		std::map<std::wstring, std::wstring> mapAppAttributes ;
		::DPW(( L"==========Enter==========================") );
		 bRet = QueryPolicyWithApp(action, wzSrc, /*attachlist*/mapAttributes, pstrApp,mapAppAttributes,  anEnforcement , Attendee);  
     
    }
    else
    {
        DPW(( TEXT("Attendee(s):")) );
        STRINGLIST::iterator it = pAttendees->begin();
        for( ; it != pAttendees->end(); ++it )
        {
            DPW(( (*it).c_str() ));
        }

      //  DPW((TEXT( "Query Policy Server......" ));
		std::map<std::wstring, std::wstring> mapAttributes ;
		::DPW(( L"==========Enter=======3333===================") );
		std::map<std::wstring, std::wstring> mapAppAttributes ;
       bRet = QueryPolicyWithApp(action, wzSrc, /*attachlist*/mapAttributes, pstrApp,mapAppAttributes,  anEnforcement , *pAttendees); 
    }
    return bRet;
}




