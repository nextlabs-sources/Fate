#include "StdAfx.h"
#include <Sddl.h>
#include <WinSock2.h>
#include "LocalInfo.h"

#pragma comment(lib, "Ws2_32.lib")

wchar_t    CLocalInfo::m_wzSID[SID_LEN]          = {0};

wchar_t    CLocalInfo::m_wzUserName[SID_LEN]     = {0};

wchar_t    CLocalInfo::m_wzAppName[MAX_PATH]        = {0};

wchar_t    CLocalInfo::m_wzAppPath[MAX_PATH] = {0};

ULONG    CLocalInfo::m_ulIp                = 0;

BOOL CLocalInfo::m_bInit = FALSE;


CLocalInfo::CLocalInfo(void)
{
}

CLocalInfo::~CLocalInfo(void)
{
}

CLocalInfo& CLocalInfo::GetInstance()
{
	if(!m_bInit)
	{
		InitLocalInfo();
		m_bInit = TRUE;
	}
	static CLocalInfo localInfo;
	return localInfo;
}

VOID CLocalInfo::InitLocalInfo(void)
{

	GetWindowUserInfo( m_wzSID,SID_LEN,  m_wzUserName,  SID_LEN ) ;

	g_log.Log(CELOG_DEBUG, L"user sid %s, user name %s\n", m_wzSID, m_wzUserName);

	m_ulIp = (ULONG)GetLocalIP() ;

	g_log.Log(CELOG_DEBUG, L"local ip %d\n", m_ulIp);

	GetModuleFileName(NULL, m_wzAppPath, MAX_PATH);

	if( wcslen( m_wzAppPath ) != 0 )

	{

		LPCWSTR fileName = wcsrchr( m_wzAppPath, L'\\' ) ;

		if( fileName )

		{

			fileName = fileName+1 ;

			wcsncpy_s( m_wzAppName, MAX_PATH, fileName, _TRUNCATE) ;

		}

	}

	g_log.Log(CELOG_DEBUG, L"app path %s\n", m_wzAppPath);

}

BOOL CLocalInfo::GetWindowUserInfo(  wchar_t *pszSid, INT iBufSize, wchar_t* pszUserName, INT inbufLen )

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

		wcsncat_s( pszUserName,inbufLen, uname, _TRUNCATE);

		char  szHostname[MAX_PATH+1]; memset(szHostname, 0, sizeof(szHostname));

		WCHAR wzHostname[MAX_PATH+1]; memset(wzHostname, 0, sizeof(wzHostname));

		gethostname(szHostname, MAX_PATH);

		if(0 != szHostname[0])

		{

			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szHostname, -1, wzHostname, MAX_PATH);

			wcsncat_s(pszUserName,inbufLen, L"@", _TRUNCATE); 

			GetFQDN(wzHostname, fqdnname, MAX_PATH);

			wcsncat_s(pszUserName,inbufLen, fqdnname, _TRUNCATE);

		}

	}

	return bRet ;

}



void CLocalInfo::GetFQDN(wchar_t* hostname, wchar_t* fqdn, int nSize)

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


DWORD CLocalInfo::GetLocalIP() 

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

BOOL CLocalInfo::GetUserInfo(wstring& sid, wstring& userName)
{
	sid = m_wzSID;
	userName = m_wzUserName;
	return TRUE;
}

BOOL CLocalInfo::GetAppInfo(wstring& appName, wstring& appPath)
{
	appName = m_wzAppName;
	appPath = m_wzAppPath;
	return TRUE;
}
