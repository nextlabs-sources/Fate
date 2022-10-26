#include "stdafx.h"
#include "FTPEEval.h"
#include "utilities.h"
#include "APIHook.h"
#include "criticalMngr.h"

#pragma warning( push )
#pragma warning( disable : 6334 )
#include "eframework\verdict_cache\verdict_cache.hpp"
#pragma warning( pop )
extern nextlabs::cesdk_loader cesdkLoader;

static nextlabs::verdict_cache cache;


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


int                 CPolicy::m_nRef = 0;
BOOL                CPolicy::m_bFirstInit = TRUE;
CPolicy* CPolicy::m_pThis = NULL;

wchar_t    CPolicy::m_wzSID[SID_LEN]          = {0};
wchar_t    CPolicy::m_wzUserName[SID_LEN]     = {0};
wchar_t    CPolicy::m_wzHostName[SID_LEN]     = {0};
wchar_t    CPolicy::m_wzAppName[MAX_PATH]        = {0};
wchar_t    CPolicy::m_wzAppPath[MAX_PATH] = {0};
wchar_t	   CPolicy::m_upload[SID_LEN]   = {L"upload"} ;
wchar_t    CPolicy::m_download[SID_LEN]	= {L"download"} ;
wchar_t CPolicy::m_ftp[SID_LEN] = {L"FTP"} ;
wchar_t CPolicy::m_sftp[SID_LEN] ={L"SFTP"}	;
wchar_t CPolicy::m_ftps[SID_LEN] ={L"FTPS"}	;
CEHandle CPolicy::m_connectHandle       = NULL;
ULONG    CPolicy::m_ulIp                = 0;

BOOL CPolicy::m_bSDK = FALSE;

CPolicy::CPolicy()
{}
CPolicy::~CPolicy()
{}

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

FTPE_STATUS CPolicy::QuerySingleFilePolicy(  CEAction_t operation, FTP_EVAL_INFO &evalInfo, CEEnforcement_t& pEnforcement )
{
	if(!CPolicy::m_bSDK)
	{
		return FTPE_ERROR;
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

	CEString srcFile =	cesdkLoader.fns.CEM_AllocateString( evalInfo.pszSrcFileName.c_str() ) ;
	CEString destFile  = cesdkLoader.fns.CEM_AllocateString( evalInfo.pszDestFileName.c_str() ) ;
	CEAttributes psrcAttributes ;
	psrcAttributes.attrs = NULL ;
	status = GetFileAttribute( evalInfo.pszSrcFileName , &psrcAttributes ) ;
	CEAttributes pdestAttributes ;
	pdestAttributes.attrs = NULL ;
	status = GetFileAttribute( evalInfo.pszDestFileName , &pdestAttributes ) ;

	user.userID		= cesdkLoader.fns.CEM_AllocateString( m_wzSID ) ;
	user.userName	= cesdkLoader.fns.CEM_AllocateString( m_wzUserName ) ;
	CEResult_t result = CE_RESULT_GENERAL_FAILED;
	for( int i = 0 ; i<2 ; i++ )
	{
	if( m_connectHandle == NULL )
	{
		Connect2PolicyServer() ;
	}
	try{
		result = cesdkLoader.fns.CEEVALUATE_CheckFile(	m_connectHandle, 
			operation,
			srcFile,
			&psrcAttributes,
			destFile,
			&pdestAttributes ,
			(CEint32) (this->m_ulIp),
			&user,
			&app,
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
		status = (CEAllow == pEnforcement.result ) ;
	}
	cesdkLoader.fns.CEM_FreeString( app.appName ) ;
	cesdkLoader.fns.CEM_FreeString( app.appPath ) ;
	cesdkLoader.fns.CEM_FreeString( srcFile ) ;
	cesdkLoader.fns.CEM_FreeString(	destFile ) ;
	cesdkLoader.fns.CEM_FreeString( app.appURL	)  ;
	cesdkLoader.fns.CEM_FreeString( user.userID ) ;
	cesdkLoader.fns.CEM_FreeString( user.userName ) ;
	if( psrcAttributes.attrs )
	{
		delete[] psrcAttributes.attrs ;
	}
	if(	 pdestAttributes.attrs )
	{
		delete[] pdestAttributes.attrs ;
	}
	return	status ;
}

static DWORD NLGetLongPathName(const wstring& filename, LPWSTR lpszLongPath, DWORD cchBuffer)
{
	if(_wcsicmp(filename.c_str(), DUMMY_DESTINATION) == 0)
	{
		return 0;
	}

	if(StringFindNoCase(filename, L"ftp://") != 0)
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
	return 0;
}

FTPE_STATUS CPolicy::QuerySingleFilePolicy(  std::wstring  operation, FTP_EVAL_INFO &evalInfo, CEEnforcement_t& pEnforcement )
{
	if(!CPolicy::m_bSDK)
	{
		return FTPE_ERROR;
	}

	DPW((L"FTPE::Try to do query: %s, %s, %s\r\n", operation.c_str(), evalInfo.pszSrcFileName.c_str(), evalInfo.pszDestFileName.c_str()));

	if(IsPolicyControllerUp() == false)//return CEAllow if PC is stopped.
	{
		pEnforcement.result = CEAllow;
		return FTPE_SUCCESS;
	}

	//	add by Ben, 2010, Feb 2
	//	convert url ( convert "|" to "%7C" ) before do anything
	URIConvert(evalInfo.pszDestFileName);
	URIConvert(evalInfo.pszSrcFileName);

	//added by kevin 2010-4-23
	ConvertUNCPath(evalInfo.pszDestFileName);
	ConvertUNCPath(evalInfo.pszSrcFileName);

	time_t mod_time = 0;
	{
		bool cache_allow = false;
		EnterCriticalSection(&CcriticalMngr::s_csVerdictCache);
		cache_allow = cache.query(operation.c_str(), (evalInfo.pszSrcFileName + evalInfo.pszDestFileName).c_str(), mod_time);
		LeaveCriticalSection(&CcriticalMngr::s_csVerdictCache);

		if( cache_allow == true )
		{
			DPW((L"query cache got allow decision %s, %s, %s", operation.c_str(), evalInfo.pszSrcFileName.c_str(), evalInfo.pszDestFileName.c_str()));

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

	CEString strAction =	cesdkLoader.fns.CEM_AllocateString( operation.c_str() ) ;
	CEAttributes psrcAttributes ;
	psrcAttributes.attrs = NULL ;
	status = GetFileAttribute( evalInfo.pszSrcFileName , &psrcAttributes ) ;
	CEAttributes pdestAttributes ;
	pdestAttributes.attrs = NULL ;
	status = GetFileAttribute( evalInfo.pszDestFileName , &pdestAttributes ) ;

	CEResource *resource = NULL;
	CEResource *destination = NULL;
	wchar_t szLongFileName[1024] = {0};
	if(NLGetLongPathName(evalInfo.pszSrcFileName, szLongFileName, sizeof(szLongFileName)/sizeof(wchar_t)) > 0)
	{
		resource = cesdkLoader.fns.CEM_CreateResourceW(szLongFileName, L"fso") ;
		DPW((L"FTPE, GetLongPathName succeeds (src file), %s", szLongFileName));
	}
	else
	{
		resource = cesdkLoader.fns.CEM_CreateResourceW(evalInfo.pszSrcFileName.c_str(), L"fso") ;
	}

	memset(szLongFileName, 0, sizeof(szLongFileName));
	if(NLGetLongPathName(evalInfo.pszDestFileName, szLongFileName, sizeof(szLongFileName)/sizeof(wchar_t)) > 0)
	{
		destination = cesdkLoader.fns.CEM_CreateResourceW(szLongFileName, L"fso") ;
		DPW((L"FTPE, GetLongPathName succeeds (dest file), %s", szLongFileName));
	}
	else
	{
		destination = cesdkLoader.fns.CEM_CreateResourceW(evalInfo.pszDestFileName.c_str(), L"fso") ;
	}
	
	user.userID		= cesdkLoader.fns.CEM_AllocateString( m_wzSID ) ;
	user.userName	= cesdkLoader.fns.CEM_AllocateString( m_wzUserName ) ;
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
			&pdestAttributes ,
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

		DPW((L"FTPE::Evaluation result: \n action: %s \n source file: %s \n destination file: %s \n result (allow): %d", operation.c_str(), evalInfo.pszSrcFileName.c_str(), evalInfo.pszDestFileName.c_str(), bAllow));

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
				EnterCriticalSection(&CcriticalMngr::s_csVerdictCache);
				cache.commit(operation.c_str(), (evalInfo.pszSrcFileName + evalInfo.pszDestFileName).c_str(), commit_ttl,mod_time);
				LeaveCriticalSection(&CcriticalMngr::s_csVerdictCache);
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
	if(	 pdestAttributes.attrs )
	{
		for( CEint32 i=0 ;   i<pdestAttributes.count ;  ++i )
		{
			cesdkLoader.fns.CEM_FreeString( pdestAttributes.attrs[i].key ) ;
			cesdkLoader.fns.CEM_FreeString( pdestAttributes.attrs[i].value ) ;
		}

		delete[] pdestAttributes.attrs ;
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
	if(_wcsicmp(strFileName.c_str(), DUMMY_DESTINATION) != 0)
	{
	getFileLastModifiedTime( strFileName.c_str(), pszLastModifyTime,MAX_PATH )	;
	}
	if( wcslen( pszLastModifyTime ) == 0 )
	{
		time_t systm =getSystemTime() ;
		_snwprintf_s(pszLastModifyTime, MAX_PATH, _TRUNCATE, L"%I64d", systm);
	}
	pAttributes->attrs = new  CEAttribute[2]  ;
	pAttributes->count = 2 ;
	pAttributes->attrs[0].key		= cesdkLoader.fns.CEM_AllocateString(L"modified_date")	; 
	pAttributes->attrs[0].value		= cesdkLoader.fns.CEM_AllocateString(pszLastModifyTime) ; 
	pAttributes->attrs[1].key		= cesdkLoader.fns.CEM_AllocateString(L"ce::request_cache_hint");
	pAttributes->attrs[1].value		= cesdkLoader.fns.CEM_AllocateString(L"yes");
	return status ; 
}
