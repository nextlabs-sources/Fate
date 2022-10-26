#include "stdafx.h"
#include <Sddl.h>
#include "criticalMngr.h"
#include "Eval.h"
#include "APIHook.h"


#pragma warning(push)
#pragma warning(disable:6334)
#include "eframework\verdict_cache\verdict_cache.hpp"
#pragma warning(pop)

#define  DNS_REVERSE_LOOKUP_ATTR		L"ce::get_equivalent_host_names"
#define TIMEOUT_QUERY_POLICY	30000

static nextlabs::verdict_cache cache;

#include "eframework/platform/cesdk_loader.hpp"
extern nextlabs::cesdk_loader cesdkLoader;

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
			_snwprintf_s(pszTimeBuf, iTimeLen, _TRUNCATE, L"%I64d", tmModify);
			bRet = TRUE;
		}
	}
	return bRet ;
}


int                 CPolicy::m_nRef = 0;
BOOL                CPolicy::m_bFirstInit = TRUE;
CPolicy* CPolicy::m_pThis = NULL;

wchar_t    CPolicy::m_wzSID[SID_LEN]          = {0};
wchar_t    CPolicy::m_wzUserName[SID_LEN]     = {0};
wchar_t    CPolicy::m_wzHostName[SID_LEN]     = {0};
wchar_t    CPolicy::m_wzAppName[MAX_PATH]        = {0};
wchar_t    CPolicy::m_wzAppPath[MAX_PATH] = {0};
CEHandle CPolicy::m_connectHandle       = NULL;
ULONG    CPolicy::m_ulIp                = 0;

BOOL	CPolicy::m_bSDK = FALSE;

CPolicy::CPolicy()
{
	InitializeCriticalSection(&m_csVerdictCache);
}
CPolicy::~CPolicy()
{
	DeleteCriticalSection(&m_csVerdictCache);
}

 CPolicy* CPolicy::CreateInstance()
{
    EnterCriticalSection(&CcriticalMngr::s_csPolicyInstance);

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
    }

    if (NULL != m_pThis)
    {
        m_nRef++;
    }

    CPolicy *temp = m_pThis;

    LeaveCriticalSection(&CcriticalMngr::s_csPolicyInstance);

    return temp;
}

void CPolicy::Release()
{
    EnterCriticalSection(&CcriticalMngr::s_csPolicyInstance);

    assert(0 < m_nRef);
    m_nRef--;

    if (0 == m_nRef)
    {
        assert(NULL != m_pThis);
        delete m_pThis;
        m_pThis = NULL;
    }

    LeaveCriticalSection(&CcriticalMngr::s_csPolicyInstance);
}
static DWORD NLGetLongPathName(const wstring& filename, LPWSTR lpszLongPath, DWORD cchBuffer)
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

	   if (hFile != INVALID_HANDLE_VALUE) 
	   {
		   dwLen = GetLongPathName(filename.c_str(), lpszLongPath, cchBuffer);			 
		   CloseHandle(hFile);  
		   DeleteFileW(filename.c_str());
	   }
	   
	}
	return dwLen;
}

DWORD CPolicy::GetLocalIP() 
{
	return 0;//We can pass 0 to PC for client side PEP, this was confirmed by platform team.
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
	if(!GetTokenInformation(hTokenHandle, TokenUser, InfoBuffer, cbInfoBuffer, &cbInfoBuffer))
	{
		return FALSE;
	}

	if(ConvertSidToStringSid(((PTOKEN_USER)InfoBuffer)->User.Sid, &StringSid))
	{
		::wcsncpy_s(pszSid, iBufSize, StringSid, _TRUNCATE);
		if(StringSid) LocalFree(StringSid);
	}
	if(LookupAccountSid(NULL, ((PTOKEN_USER)InfoBuffer)->User.Sid, uname, &unamelen, dname, &dnamelen, &snu))   
	{
		char  szHostname[MAX_PATH+1]; memset(szHostname, 0, sizeof(szHostname));
		WCHAR wzHostname[MAX_PATH+1]; memset(wzHostname, 0, sizeof(wzHostname));
		gethostname(szHostname, MAX_PATH);
		if(0 != szHostname[0])
		{
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szHostname, -1, wzHostname, MAX_PATH);
			
			GetFQDN(wzHostname, fqdnname, MAX_PATH);
			wcsncat_s(pszUserName,inbufLen, fqdnname, _TRUNCATE);

			wcsncat_s(pszUserName,inbufLen, L"\\", _TRUNCATE);
			
			wcsncat_s( pszUserName,inbufLen, uname, _TRUNCATE);
			
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
		wcsncpy_s(fqdn, nSize, hostname, _TRUNCATE);
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
			wcsncpy_s( m_wzAppName, MAX_PATH, fileName, _TRUNCATE) ;
		}
	}

}
BOOL CPolicy::Connect2PolicyServer()
{
	if(!CPolicy::m_bSDK)
	{
		return FALSE;
	}

	CEApplication   app;
	CEUser          user;
	CEint32         timeout_in_millisec = TIMEOUT_QUERY_POLICY;
	BOOL            bRet = FALSE;

	// Evaluate
	memset(&app, 0, sizeof(CEApplication));
	memset(&user, 0, sizeof(CEUser));
	app.appName   = cesdkLoader.fns.CEM_AllocateString(CPolicy::m_wzAppName);
	app.appPath   = cesdkLoader.fns.CEM_AllocateString(CPolicy::m_wzAppPath);
	user.userID   = cesdkLoader.fns.CEM_AllocateString(CPolicy::m_wzSID);
	user.userName = cesdkLoader.fns.CEM_AllocateString(CPolicy::m_wzUserName);
	CEResult_t result = cesdkLoader.fns.CECONN_Initialize(app, user, NULL/**//*host*/, &CPolicy::m_connectHandle, timeout_in_millisec) ;
	if(CE_RESULT_SUCCESS ==result 
		&& NULL!=CPolicy::m_connectHandle)
	{
		g_log.Log(CELOG_DEBUG, L"Succeed on to initialize CECONN!\n");
		bRet = TRUE;
	}
	else
	{
		g_log.Log(CELOG_DEBUG, L"Failed to initialize CECONN!\n");
	}
	
	cesdkLoader.fns.CEM_FreeString(app.appName);
	cesdkLoader.fns.CEM_FreeString(app.appPath);
	cesdkLoader.fns.CEM_FreeString(user.userID);
	cesdkLoader.fns.CEM_FreeString(user.userName);

	return bRet;
}
void CPolicy::Disconnect2PolicyServer()
{
	if(!CPolicy::m_bSDK)
	{
		return;
	}

	if(NULL != CPolicy::m_connectHandle)
	{
		CEint32         timeout_in_millisec = TIMEOUT_QUERY_POLICY;
		cesdkLoader.fns.CECONN_Close(CPolicy::m_connectHandle, timeout_in_millisec);
		CPolicy::m_connectHandle = NULL;
	}
}

BOOL CPolicy::QueryPolicy( const std::wstring&  strAction, LPVOID pEval_data, CEEnforcement_t& pEnforcement )
{
	std::map<std::wstring, std::wstring> mapAttributes;
	mapAttributes.clear();
	return QueryPolicy(strAction, pEval_data, mapAttributes, pEnforcement);
}

BOOL CPolicy::QueryPolicy( const std::wstring& strAction, LPVOID pEval_data, std::map<std::wstring, std::wstring>& mapAttributes, CEEnforcement_t& pEnforcement )
{
	if(!CPolicy::m_bSDK || !pEval_data)
	{
		return FALSE;
	}

	LPEVALDATA pData = (LPEVALDATA)pEval_data;

	if(IsPolicyControllerUp() == false)//return CEAllow if PC is stopped.
	{
		pEnforcement.result = CEAllow;
		return TRUE;
	}

	time_t mod_time = 0;
	{
		bool cache_allow = false;
		EnterCriticalSection(&m_csVerdictCache);
		cache_allow = cache.query(strAction.c_str(), (pData->strSrc + pData->strDest).c_str(), mod_time);
		LeaveCriticalSection(&m_csVerdictCache);

		if( cache_allow == true )
		{
			pEnforcement.result = CEAllow;
			return TRUE;
		}
	}

	BOOL bRet = TRUE;
	CEApplication app ;
	CEUser user ;
	CEint32         timeout_in_millisec = TIMEOUT_QUERY_POLICY;

	::ZeroMemory( &app,sizeof( CEApplication ) ) ;
	::ZeroMemory( &user,sizeof( CEUser ) ) ;
	::ZeroMemory( &pEnforcement, sizeof( CEEnforcement_t ) ) ;

	app.appURL	= cesdkLoader.fns.CEM_AllocateString( L"" ) ;
	app.appPath = cesdkLoader.fns.CEM_AllocateString( m_wzAppPath ) ;
	app.appName = cesdkLoader.fns.CEM_AllocateString( m_wzAppName ) ;

	CEString action =	cesdkLoader.fns.CEM_AllocateString( strAction.c_str() ) ;
	CEAttributes psrcAttributes ={0};
	psrcAttributes.attrs = NULL ;

	std::wstring strLastModifiedTime;
	bRet = GetFileLastModifiedTime( pData->strSrc, strLastModifiedTime ) ;
	if(bRet)
	{
		mapAttributes[L"modified_date"] = strLastModifiedTime;
	}
	
	mapAttributes[L"ce::request_cache_hint"] = L"yes";

	if(strAction.compare(HTTP_OPEN) == 0)
	{
		mapAttributes[DNS_REVERSE_LOOKUP_ATTR] = L"no";
	}

	SetAttributes(mapAttributes, &psrcAttributes);
	
	CEResource *resource = NULL ;
	wchar_t szLongFileName[1024] = {0};
	if(pData->strSrc.length() > 3 && pData->strSrc[0] != '\\')//Try to get the long file path
	{
		 NLGetLongPathName(pData->strSrc, szLongFileName, sizeof(szLongFileName)/sizeof(wchar_t));
	}
	if( (*szLongFileName) != 0 )
	{
		resource = cesdkLoader.fns.CEM_CreateResourceW(szLongFileName, pData->strResourceType_Src.c_str()) ;
	}
	else
	{
		resource = cesdkLoader.fns.CEM_CreateResourceW(pData->strSrc.c_str(), pData->strResourceType_Src.c_str());
	}

	
	CEResource *destination =  cesdkLoader.fns.CEM_CreateResourceW(pData->strDest.c_str(), pData->strResourceType_Dest.c_str()) ;
	user.userID		=  cesdkLoader.fns.CEM_AllocateString( m_wzSID ) ;
	user.userName	=  cesdkLoader.fns.CEM_AllocateString( m_wzUserName ) ;
	CEResult_t result = CE_RESULT_GENERAL_FAILED;
	for( int i = 0 ; i<2 ; i++ )
	{
		if( m_connectHandle == NULL )
		{
			Connect2PolicyServer() ;
		}
		try{
			result = cesdkLoader.fns.CEEVALUATE_CheckResources(	m_connectHandle, 
				action,
				resource,
				&psrcAttributes,
				destination,
				NULL ,
				&user,
				NULL,  
				&app,
				NULL,
				NULL,
				0,
					(CEint32) (this->m_ulIp),
				CETrue,
				CE_NOISE_LEVEL_USER_ACTION,
				&pEnforcement,
				timeout_in_millisec ) ;
		}catch(...)
		{
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
		bRet = TRUE;
		
		BOOL bAllow = TRUE; /* default */

		// Enforcement is allow?
		bAllow = (pEnforcement.result == CEAllow);

		// Obligations exist (structure is populated)?
		bool obs_exist = (pEnforcement.obligation != NULL && pEnforcement.obligation->attrs != NULL);
		if( bAllow && obs_exist == true )
		{
			bool commit = false;
			std::size_t commit_ttl = 60;
			for( int i = 0 ; i < pEnforcement.obligation->count ; i++ )
			{
				const wchar_t* key   = cesdkLoader.fns.CEM_GetString(pEnforcement.obligation->attrs[i].key);
				const wchar_t* value = cesdkLoader.fns.CEM_GetString(pEnforcement.obligation->attrs[i].value);
				if( key == NULL || value == NULL )
				{
					continue;
				}
				if( _wcsicmp(key,L"CE_CACHE_HINT") == 0 )
				{
					commit = true;              // Commit to cache
					commit_ttl = _wtoi(value);  // Commit TTL
				}
			}
			if( commit == true )
			{
				EnterCriticalSection(&m_csVerdictCache);
				cache.commit(strAction.c_str(), (pData->strSrc + pData->strDest).c_str(), commit_ttl, mod_time);
				LeaveCriticalSection(&m_csVerdictCache);
			}
		}
	}
	cesdkLoader.fns.CEM_FreeString( app.appName ) ;
	cesdkLoader.fns.CEM_FreeString( app.appPath ) ;
	cesdkLoader.fns.CEM_FreeResource( resource ) ;
	cesdkLoader.fns.CEM_FreeResource( destination ) ;
	cesdkLoader.fns.CEM_FreeString( app.appURL	)  ;
	cesdkLoader.fns.CEM_FreeString( user.userID ) ;
	cesdkLoader.fns.CEM_FreeString( user.userName ) ;
	cesdkLoader.fns.CEM_FreeString( action ) ;
	if( psrcAttributes.attrs )
	{
		for( CEint32 i=0 ;   i<psrcAttributes.count ;  ++i )
		{
			cesdkLoader.fns.CEM_FreeString( psrcAttributes.attrs[i].key ) ;
			cesdkLoader.fns.CEM_FreeString( psrcAttributes.attrs[i].value ) ;
		}
		delete[] psrcAttributes.attrs ;
	}
	
	return	bRet ;
}

BOOL CPolicy::GetFileLastModifiedTime( const std::wstring & strFileName, std::wstring& strLastModifiedTime) 
{
	if(!CPolicy::m_bSDK)
	{
		return FALSE;
	}

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
		_snwprintf_s(pszLastModifyTime, MAX_PATH, _TRUNCATE, L"%I64d", systm);
	}

	strLastModifiedTime = std::wstring(pszLastModifyTime);

	return TRUE; 
}

BOOL CPolicy::SetAttributes(const std::map<std::wstring, std::wstring>& mapAttributes, CEAttributes *pAttribute)
{
	if(!CPolicy::m_bSDK || !pAttribute)
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
			pAttribute->attrs[i].key = cesdkLoader.fns.CEM_AllocateString((*itr).first.c_str());
			pAttribute->attrs[i].value = cesdkLoader.fns.CEM_AllocateString((*itr).second.c_str());
			i++;
		}
		return TRUE;
	}
	
	return FALSE;
}

BOOL CPolicy::LogDecision(LPCWSTR pszCookie,
						  CEResponse_t userResponse, 
						  CEAttributes* optAttributes)
{
	if(!CPolicy::m_bSDK || !pszCookie || !optAttributes)
	{
		return FALSE;
	}

	CEString cookie = cesdkLoader.fns.CEM_AllocateString(pszCookie);
	BOOL bRet = FALSE;

	if(NULL == CPolicy::m_connectHandle) Connect2PolicyServer();
	if(NULL == CPolicy::m_connectHandle)
	{
		
		goto _CLEAN_EXIT;
	}

	if (NULL == cookie) goto _CLEAN_EXIT;

	if (CE_RESULT_SUCCESS
		==
		cesdkLoader.fns.CELOGGING_LogDecision(m_connectHandle, cookie, userResponse,
		optAttributes))
	{
		bRet = TRUE;
	}

_CLEAN_EXIT:
	cesdkLoader.fns.CEM_FreeString(cookie);
	return bRet;
}

BOOL CPolicy::LogObligationData(LPCWSTR pszLogID, LPCWSTR pszObligationName, CEAttributes *attributes)
{
	if(!CPolicy::m_bSDK || !pszLogID || !pszObligationName)
	{
		return FALSE;
	}

	if(NULL == CPolicy::m_connectHandle) Connect2PolicyServer();
	if(NULL == CPolicy::m_connectHandle)
	{
		return FALSE;
	}

	CEString logID = cesdkLoader.fns.CEM_AllocateString(pszLogID);
	CEString obName = cesdkLoader.fns.CEM_AllocateString(pszObligationName);
	BOOL bRet = FALSE;

	if(CE_RESULT_SUCCESS == cesdkLoader.fns.CELOGGING_LogObligationData(m_connectHandle, logID, obName, attributes))
	{
		bRet = TRUE;
	}

	cesdkLoader.fns.CEM_FreeString(logID);
	cesdkLoader.fns.CEM_FreeString(obName);

	return bRet;
}

CObligation::CObligation(void) 
{}
CObligation::~CObligation(void) 
{}
void CObligation::PushObligationItem(const OBLIGATION& obligation)  
{
	m_listOb.push_back( obligation ) ;
}
std::wstring CObligation::GetAttrValueByName( const ATTRIBUTELIST& listAttr, const std::wstring & strAttrName ) 
{
	for (ATTRIBUTELIST::const_iterator itor = listAttr.begin(); itor!=listAttr.end(); ++itor)
    {
        if(boost::algorithm::istarts_with((*itor).strValue, strAttrName))
            return (listAttr.end()!=++itor)?(*itor).strValue:L"";
    }
    return L"";
}
ATTRIBUTELIST  CObligation::GetAttributeListByName( const std::wstring & strOBName )
{
	ATTRIBUTELIST listAttr ;
	if( strOBName.empty() )
	{
		return	listAttr ;
	}
	OBLIGATIONLIST::iterator itor = m_listOb.begin() ;
	for( itor ; itor!= m_listOb.end() ; itor++ )
	{
		if ( boost::algorithm::istarts_with( (*itor).strOBName, strOBName ) )
		{
			return (*itor).attrList ;
		}
	}
	return listAttr ;
}

BOOL  CObligation::GetAttributeListByName2( const std::wstring & strOBName,  vector<ATTRIBUTELIST* >& vAttrs)
{
	if( strOBName.empty() )
	{
		return	FALSE;
	}

	BOOL bRet = FALSE;
	OBLIGATIONLIST::iterator itor = m_listOb.begin();
	for( itor; itor != m_listOb.end(); itor++ )
	{
		if ( boost::algorithm::istarts_with( (*itor).strOBName, strOBName ) )
		{
			vAttrs.push_back(&((*itor).attrList));
			bRet = TRUE;
		}
	}
	return bRet;
}

INT CObligation::GetObligationCount(VOID)
{
	return (INT)  m_listOb.size() ;
}
std::wstring CObligation::GetRedirectURL(VOID)
{
	std::wstring url ;
	const ATTRIBUTELIST & listAttr = GetAttributeListByName(REDIRECT_OBLIGATION_NAME)  ;
	if( !listAttr.empty() )
	{
	   return	GetAttrValueByName(	 listAttr, REDIRECT_OBLIGATION_URL ) ;
	}
	return url ;
}
BOOL CObligation::GetHeaderInjectionData( vector<wstring> & vHeaderData  ) 
{
	BOOL bRet=  FALSE ;
	
	vector<ATTRIBUTELIST*> vAttrs;
	if(GetAttributeListByName2(HTTP_HEADER_INJECTION_NAME, vAttrs))
	{
		vector<ATTRIBUTELIST*>::iterator itr;
		for(itr = vAttrs.begin(); itr != vAttrs.end(); itr++)
		{
			if(*itr && !(*itr)->empty())
			{
				wstring headerKey = GetAttrValueByName(	 *(*itr), HTTP_HEADER_INJECTION_KEY ) ;
				if( headerKey.empty() )
				{//empty key is not allowed.
					continue;
				}
				const wstring inter = L": " ;
				vHeaderData.push_back(headerKey + inter + GetAttrValueByName(*(*itr), HTTP_HEADER_INJECTION_VALUE ));
				bRet = TRUE;
			}
		}
	}
	
	return bRet ;
}

BOOL CObligation::GetOBValue(const wstring& strOBName, const wstring& strKey, std::list<wstring>& strListValue)
{
	BOOL bRet = FALSE;
	vector<ATTRIBUTELIST* > listAttr ;
	if(GetAttributeListByName2(strOBName, listAttr))
{
	
		for(vector<ATTRIBUTELIST* >::iterator itr = listAttr.begin(); itr != listAttr.end(); itr++)
		{
			if(*itr && !(*itr)->empty())
			{
				wstring headerKey = GetAttrValueByName(	 *(*itr), strKey ) ;
				/*
				added if the key is NULL, set a defult
				*/
				if( !headerKey.empty() )
	{
					strListValue.push_back( headerKey ) ;
	  bRet = TRUE ;
	}
			}

		}
	}
	return bRet ;
}


