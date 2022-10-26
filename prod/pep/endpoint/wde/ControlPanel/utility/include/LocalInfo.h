#pragma once

#include <string>
using namespace std;

class CLocalInfo
{
protected:
	CLocalInfo(void);
	~CLocalInfo(void);

public:

	static CLocalInfo& GetInstance();

	BOOL GetUserInfo(wstring& sid, wstring& userName);

	BOOL GetAppInfo(wstring& appName, wstring& appPath);

private:
	static VOID CLocalInfo::InitLocalInfo(void);

	static BOOL GetWindowUserInfo(  wchar_t *pszSid, INT iBufSize, wchar_t* pszUserName, INT inbufLen ) ;

	static DWORD GetLocalIP() ;

	static void GetFQDN(wchar_t* hostname, wchar_t* fqdn, int nSize) ;

#ifndef SID_LEN
#define	 SID_LEN  128
#endif

	static wchar_t    m_wzSID[SID_LEN];
	static wchar_t    m_wzUserName[SID_LEN];

	static wchar_t    m_wzAppName[MAX_PATH];
	static wchar_t    m_wzAppPath[MAX_PATH];

	static ULONG      m_ulIp;

	static BOOL                m_bInit;
};
