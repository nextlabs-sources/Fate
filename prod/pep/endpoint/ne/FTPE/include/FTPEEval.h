#ifndef __FTPE_EVALUATION_H__
#define __FTPE_EVALUATION_H__
#include <list>
#include "CEsdk.h"
/*
define the structure for the FTP type
*/
struct FTP_EVAL_INFO
{
	bool IsValid() const
	{
		return pszServerIP.length() > 0 && pszServerPort.length() > 0 && pszDestFileName.length() > 0 && pszSrcFileName.length() > 0;
	}
	std::wstring pszServerIP ;
	std::wstring pszServerPort ;
	std::wstring pszFTPUserName ;
	std::wstring pszDestFileName ;
	std::wstring pszSrcFileName ;
	UINT		 iProtocolType ;
};


class CPolicy :public CTime, public FTPE::CLock
{
public:
	CPolicy() ;
	virtual ~CPolicy() ;
	static CPolicy* CreateInstance();
	void Release();
public:
	FTPE_STATUS QuerySingleFilePolicy(  CEAction_t operation, FTP_EVAL_INFO &evalInfo, CEEnforcement_t& pEnforcement );
	FTPE_STATUS QuerySingleFilePolicy(  std::wstring  operation, FTP_EVAL_INFO& evalInfo, CEEnforcement_t& pEnforcement ) ;
	BOOL Connect2PolicyServer() ;
	void Disconnect2PolicyServer()   ;
protected:
	static DWORD GetLocalIP() ;
	static FTPE_STATUS GetWindowUserInfo(  wchar_t *pszSid, INT iBufSize, wchar_t* pszUserName, INT inbufLen ) ;
	static void GetFQDN(wchar_t* hostname, wchar_t* fqdn, int nSize) ;
	FTPE_STATUS GetFileAttribute( std::wstring strFileName,  CEAttributes *pAttribute ) ;
private:
	static	VOID InitLocalInfo(void) ;

public:
	static BOOL				   m_bSDK;
private:
	static BOOL                m_bFirstInit;
	static int                 m_nRef;
	static CPolicy* m_pThis;

#ifndef SID_LEN
#define	 SID_LEN  128
#endif
	static wchar_t    m_wzSID[SID_LEN];
	static wchar_t    m_wzUserName[SID_LEN];
	static wchar_t    m_wzHostName[SID_LEN];
	static wchar_t    m_wzAppName[MAX_PATH];
	static wchar_t    m_wzAppPath[MAX_PATH];
	static CEHandle m_connectHandle;
	static ULONG    m_ulIp;
public:
	static wchar_t m_upload[SID_LEN] ;
	static wchar_t m_download[SID_LEN]	;

	static wchar_t m_ftp[SID_LEN] ;
	static wchar_t m_sftp[SID_LEN]	;
	static wchar_t m_ftps[SID_LEN]	;
};
#endif