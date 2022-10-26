#ifndef __FTPE_EVALUATION_H__
#define __FTPE_EVALUATION_H__
#include <list>

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
	static CPolicy* CreateInstance();
	void Release();
public:
	FTPE_STATUS QuerySingleFilePolicy(  CEAction_t operation, FTP_EVAL_INFO &evalInfo, CEEnforcement_t& pEnforcement );
	FTPE_STATUS QuerySingleFilePolicy(  std::wstring  operation, FTP_EVAL_INFO& evalInfo, CEEnforcement_t& pEnforcement ) ;
	BOOL Connect2PolicyServer() ;
	void Disconnect2PolicyServer()   ;
protected:
	//	make it protected to ensure single pattern
	CPolicy() ;
	virtual ~CPolicy() ;

	static DWORD GetLocalIP() ;
	static FTPE_STATUS GetWindowUserInfo(  wchar_t *pszSid, INT iBufSize, wchar_t* pszUserName, INT inbufLen ) ;
	static void GetFQDN(wchar_t* hostname, wchar_t* fqdn, int nSize) ;
	FTPE_STATUS GetFileAttribute( std::wstring strFileName,  CEAttributes *pAttribute ) ;
private:
	static	VOID InitLocalInfo(void) ;
private:

	static BOOL                m_bFirstInit;
	static int                 m_nRef;
	static CPolicy* m_pThis;

	//	cs for verdictCache
	CRITICAL_SECTION m_csVerdictCache;

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
	static wchar_t m_hostopen[SID_LEN] ;
	static wchar_t m_networkAccess[SID_LEN] ;
	static BOOL				   m_bSDK;
};
#endif