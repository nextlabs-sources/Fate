#include "stdafx.h"
#include "Eval.h"
#include "APIHook.h"
#pragma warning( push )
#pragma warning( disable : 6334 )
#include "eframework\verdict_cache\verdict_cache.hpp"
#pragma warning( pop )
#define  DNS_REVERSE_LOOKUP_ATTR		L"ce::get_equivalent_host_names"

#include "eframework/platform/cesdk_loader.hpp"
extern nextlabs::cesdk_loader cesdkLoader;
#include "criticalMngr.h"


//	add 2010, Feb 2
//	for URIConvert
#  include <boost/algorithm/string.hpp>
#include <map>
using namespace std;

pair<wstring, wstring> g_mapURI[] = {pair<wstring, wstring>(L"|", L"%7C")};
static void URIConvert(wstring& strURI)
{
	for(int i = 0; i < _countof(g_mapURI); i++)
	{
		boost::replace_all(strURI, g_mapURI[i].first, g_mapURI[i].second);
	}
}

static nextlabs::verdict_cache cache;

int                 CPolicy::m_nRef = 0;
BOOL                CPolicy::m_bFirstInit = TRUE;
CPolicy* CPolicy::m_pThis = NULL;

wchar_t    CPolicy::m_wzSID[SID_LEN]          = {0};
wchar_t    CPolicy::m_wzUserName[SID_LEN]     = {0};
wchar_t    CPolicy::m_wzHostName[SID_LEN]     = {0};
wchar_t    CPolicy::m_wzAppName[MAX_PATH]        = {0};
wchar_t    CPolicy::m_wzAppPath[MAX_PATH] = {0};
wchar_t CPolicy::m_hostopen[SID_LEN] = {L"HOST_CONNECT"};
wchar_t CPolicy::m_networkAccess[SID_LEN] = {L"NETWORK_ACCESS"} ;
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
		if(CAPIHook::real_CreateFileW && CAPIHook::real_CloseHandle)
		{
			HANDLE hFile = CAPIHook::real_CreateFileW(filename.c_str(),                // name of the write
				   GENERIC_WRITE,          // open for writing
				   0,                      // do not share
				   NULL,                   // default security
				   CREATE_NEW,          // overwrite existing
				   FILE_ATTRIBUTE_NORMAL,  // normal file
				   NULL);                  // no attr. template

		   if (hFile != INVALID_HANDLE_VALUE) 
		   {
			   dwLen = GetLongPathName(filename.c_str(), lpszLongPath, cchBuffer);
			   CAPIHook::real_CloseHandle(hFile);  
			   DeleteFileW(filename.c_str());
		   }
	   }
	}
	return dwLen;
}

DWORD CPolicy::GetLocalIP() 
{
	return 0;//We can pass 0 to PC for client side PEP, this was confirmed by platform team.
}

FTPE_STATUS CPolicy::GetWindowUserInfo(  wchar_t *pszSid, INT iBufSize, wchar_t* pszUserName, INT inbufLen )
{
	FTPE_STATUS status = FTPE_SUCCESS ;
	HANDLE hTokenHandle = 0;

	if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hTokenHandle))
	{
		if(GetLastError() == ERROR_NO_TOKEN)
		{
			if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTokenHandle ))
			{

				return FTPE_ERROR;
			}
		}
		else
		{

			return FTPE_ERROR;
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
		return FTPE_ERROR;
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
	return status ;
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
	GetCurrentProcessName(m_wzAppPath, MAX_PATH, NULL);
	if( wcslen( m_wzAppPath ) != 0 )
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
	CEString        host;
	CEint32         timeout_in_millisec = TIMEOUT_TIME;
	BOOL            bRet = FALSE;

	// Evaluate
	memset(&app, 0, sizeof(CEApplication));
	memset(&user, 0, sizeof(CEUser));
	app.appName   = cesdkLoader.fns.CEM_AllocateString(CPolicy::m_wzAppName);
	app.appPath   = cesdkLoader.fns.CEM_AllocateString(CPolicy::m_wzAppPath);
	user.userID   = cesdkLoader.fns.CEM_AllocateString(CPolicy::m_wzSID);
	user.userName = cesdkLoader.fns.CEM_AllocateString(CPolicy::m_wzUserName);
	host          = cesdkLoader.fns.CEM_AllocateString(L"10.187.2.111");//10.187.2.111;10.187.4.183(L"vagartst07.test.bluejungle.com");//10.187.6.136(L"10.187.4.161");//(L"10.187.4.176");
	CEResult_t result = cesdkLoader.fns.CECONN_Initialize(app, user, NULL/**//*host*/, &CPolicy::m_connectHandle, timeout_in_millisec) ;
	if(CE_RESULT_SUCCESS ==result 
		&& NULL!=CPolicy::m_connectHandle)
	{
		DP((L"Succeed on to initialize CECONN!\n"));
		bRet = TRUE;
	}
	else
	{

		DP((L"Failed to initialize CECONN!\n"));
	}
	cesdkLoader.fns.CEM_FreeString(host);
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
		CEint32         timeout_in_millisec = TIMEOUT_TIME;
		cesdkLoader.fns.CECONN_Close(CPolicy::m_connectHandle, timeout_in_millisec);
		CPolicy::m_connectHandle = NULL;
	}
}

FTPE_STATUS CPolicy::QuerySingleFilePolicy(  std::wstring  operation, FTP_EVAL_INFO& evalInfo, CEEnforcement_t& pEnforcement )
{
	if(!CPolicy::m_bSDK)
	{
		return FTPE_ERROR;
	}

	if(IsPolicyControllerUp() == false)//return CEAllow if PC is stopped.
	{
		pEnforcement.result = CEAllow;
		return FTPE_SUCCESS;
	}

	//	add by Ben, 2010, Feb 2
	//	convert url ( convert "|" to "%7C" ) before do anything
	if(operation == CPolicy::m_networkAccess)
	{
		//	the operation is network access, url is pszDestFileName
		URIConvert(evalInfo.pszDestFileName);
		ConvertUNCPath(evalInfo.pszSrcFileName);//added by kevin 2010-4-23, convert "\device\mup\hz-ts02..." to "\\hz-ts02..."
	}
	else if (operation == CPolicy::m_hostopen)
	{
		//	the operation is host open, url is pszSrcFileName
		URIConvert(evalInfo.pszSrcFileName);
	}

	time_t mod_time = 0;
	{
		bool cache_allow = false;
		EnterCriticalSection(&m_csVerdictCache);
		cache_allow = cache.query(operation.c_str(), (evalInfo.pszSrcFileName + evalInfo.pszDestFileName).c_str(), mod_time);
		LeaveCriticalSection(&m_csVerdictCache);

		if( cache_allow == true )
		{
			pEnforcement.result = CEAllow;
			return FTPE_SUCCESS;
		}
	}

	FTPE_STATUS status =  FTPE_SUCCESS ;
	CEApplication app ;
	CEUser user ;
	CEint32         timeout_in_millisec = TIMEOUT_TIME;

	::ZeroMemory( &app,sizeof( CEApplication ) ) ;
	::ZeroMemory( &user,sizeof( CEUser ) ) ;
	::ZeroMemory( &pEnforcement, sizeof( CEEnforcement_t ) ) ;

	app.appURL	= cesdkLoader.fns.CEM_AllocateString( L"" ) ;
	app.appPath = cesdkLoader.fns.CEM_AllocateString( m_wzAppPath ) ;
	app.appName = cesdkLoader.fns.CEM_AllocateString( m_wzAppName ) ;

	CEString strAction =cesdkLoader.fns.CEM_AllocateString( operation.c_str() ) ;
	CEAttributes psrcAttributes ;
	psrcAttributes.attrs = NULL ;
	status = GetFileAttribute( evalInfo.pszSrcFileName , &psrcAttributes ) ;
	
	
	CEAttributes pDestAttributes;
	
	CEResource *resource = NULL ;
	if( operation.compare( CPolicy::m_hostopen ) == 0 )
	{
		resource = cesdkLoader.fns.CEM_CreateResourceW(evalInfo.pszSrcFileName.c_str(), L"server") ;

		pDestAttributes.attrs = NULL;
		pDestAttributes.count = 0;
	}
	else
	{
		wchar_t szLongFileName[1024] = {0};
		if(evalInfo.pszSrcFileName.length() > 3 && evalInfo.pszSrcFileName[0] != '\\' && NLGetLongPathName(evalInfo.pszSrcFileName, szLongFileName, sizeof(szLongFileName)/sizeof(wchar_t)) > 0)
		{
			resource = cesdkLoader.fns.CEM_CreateResourceW(szLongFileName, L"fso") ;
		}else
		{
		resource =  cesdkLoader.fns.CEM_CreateResourceW(evalInfo.pszSrcFileName.c_str(), L"fso") ;
	}

		pDestAttributes.attrs = new  CEAttribute[1]  ;
		pDestAttributes.count = 1 ;
		pDestAttributes.attrs[0].key		= cesdkLoader.fns.CEM_AllocateString(DNS_REVERSE_LOOKUP_ATTR);//Disable DNS reserve look up for NETWORK_ACCESS. Kevin Zhou 3/12 2010
		pDestAttributes.attrs[0].value		= cesdkLoader.fns.CEM_AllocateString(L"no");
	}
	CEResource *destination =  cesdkLoader.fns.CEM_CreateResourceW(evalInfo.pszDestFileName.c_str(), L"server") ;
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
			strAction,
			resource,
			&psrcAttributes,
			destination,
			&pDestAttributes ,
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
		status =   FTPE_ERROR ;
	}
		if( result !=   CE_RESULT_CONN_FAILED )
		{
			break ;
		}
		m_connectHandle = NULL ;
	}
	if( result == CE_RESULT_SUCCESS ) 
	{
		status =  FTPE_SUCCESS ;

		
		BOOL bAllow = TRUE; /* default */

		// Enforcement is allow?
		bAllow = (pEnforcement.result == CEAllow);

		DPW((L"HPE::Evaluation result: \n action: %s \n source file: %s \n destination file: %s \n result (allow): %d", operation.c_str(), evalInfo.pszSrcFileName.c_str(), evalInfo.pszDestFileName.c_str(), bAllow));

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
				cache.commit(operation.c_str(), (evalInfo.pszSrcFileName + evalInfo.pszDestFileName).c_str(), commit_ttl, mod_time);
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
	cesdkLoader.fns.CEM_FreeString( strAction ) ;
	if( psrcAttributes.attrs )
	{
		for( CEint32 i=0 ;   i<psrcAttributes.count ;  ++i )
		{
			cesdkLoader.fns.CEM_FreeString( psrcAttributes.attrs[i].key ) ;
			cesdkLoader.fns.CEM_FreeString( psrcAttributes.attrs[i].value ) ;
		}

		delete[] psrcAttributes.attrs ;
	}
	
	if( pDestAttributes.attrs)
	{
		for( CEint32 i=0 ;   i<pDestAttributes.count ;  ++i )
		{
			cesdkLoader.fns.CEM_FreeString( pDestAttributes.attrs[i].key ) ;
			cesdkLoader.fns.CEM_FreeString( pDestAttributes.attrs[i].value ) ;
		}

		delete[] pDestAttributes.attrs ;
	}

	return	status ;
}
FTPE_STATUS CPolicy::GetFileAttribute( std::wstring strFileName,  CEAttributes *pAttributes ) 
{
	if(!CPolicy::m_bSDK)
	{
		return FTPE_ERROR;
	}

	FTPE_STATUS	  status =  FTPE_SUCCESS  ;
	if(	 strFileName.length() == 0 )
	{
		status = FTPE_ERROR ;
		return	status ;
	}
	wchar_t pszLastModifyTime[MAX_PATH] = {0} ;
	if(strFileName.length() > 0)
	{
	getFileLastModifiedTime( strFileName.c_str(), pszLastModifyTime,MAX_PATH )	;
	}
	
	if( wcslen( pszLastModifyTime ) == 0 )
	{
		time_t systm =getSystemTime() ;
		_snwprintf_s(pszLastModifyTime, MAX_PATH, _TRUNCATE, L"%I64d", systm);
	}
	pAttributes->attrs = new  CEAttribute[3]  ;
	pAttributes->count = 3 ;
	pAttributes->attrs[0].key		= cesdkLoader.fns.CEM_AllocateString(L"modified_date")	; 
	pAttributes->attrs[0].value		= cesdkLoader.fns.CEM_AllocateString(pszLastModifyTime) ; 
 	pAttributes->attrs[1].key		= cesdkLoader.fns.CEM_AllocateString(L"ce::request_cache_hint");
 	pAttributes->attrs[1].value		= cesdkLoader.fns.CEM_AllocateString(L"yes");
	pAttributes->attrs[2].key		= cesdkLoader.fns.CEM_AllocateString(DNS_REVERSE_LOOKUP_ATTR);
	pAttributes->attrs[2].value		= cesdkLoader.fns.CEM_AllocateString(L"no");

	return status ; 
}
