#ifndef __MULTIFTP_H__
#define __MULTIFTP_H__
#include <string>
#include <vector>
class CMultiFTPSession
{

};

class CFtpSite
{
public:
	CFtpSite():m_strSiteName(L""){};
	CFtpSite(WCHAR* wzSiteName,WCHAR*wzEmailAddressPattern,WCHAR* wzFtpServer,WCHAR* wzFtpUser,WCHAR* wzFtpPasswd):
	  m_strSiteName(wzSiteName),m_strEmailAddressPattern(wzEmailAddressPattern),m_strFtpServer(wzFtpServer),
		  m_strFtpUser(wzFtpUser),m_strFtpPasswd(wzFtpPasswd){};
	bool Matched(const WCHAR* wzEmailAddress);
	std::wstring GetSiteName(){return m_strSiteName;};
	std::wstring GetFtpServer(){return m_strFtpServer;};
	std::wstring GetFtpUser(){return m_strFtpUser;};
	std::wstring GetFtpPasswd(){return m_strFtpPasswd;};
	bool IsSameSite(CFtpSite& anotherSite);
private:
	std::wstring m_strSiteName;
	std::wstring m_strEmailAddressPattern;
	std::wstring m_strFtpServer;
	std::wstring m_strFtpUser;
	std::wstring m_strFtpPasswd;
};
class CMultiFTPManager
{
private:
	static const WCHAR MULTIFTP_CONFIGFILE[];

	static const WCHAR MULTIFTP_CFG_KEY_EMAILADDRESSPATTERN[];
	static const WCHAR MULTIFTP_CFG_KEY_FTPSERVER[];
	static const WCHAR MULTIFTP_CFG_KEY_FTPUSER[];
	static const WCHAR MULTIFTP_CFG_KEY_FTPPASSWORD[];
public:
	enum MATCH_TYPE{NONE=0,ONE,MISMATCH};
	CMultiFTPManager(HINSTANCE hInstance):m_hInstance(hInstance){Init();};
	
	void AddSite(CFtpSite& ftpSite){m_vecFtpSites.push_back(ftpSite);};
	
	MATCH_TYPE Match(std::vector<std::wstring>& vecRecipients,CFtpSite&site);
	bool IsMalFormed(){return m_bMalFormed;};
private:
	void Init();
	bool CMultiFTPManager::Match(std::wstring& strRecipient,CFtpSite& site);
	HINSTANCE				m_hInstance;
	static std::vector<CFtpSite>	m_vecFtpSites;
	static std::wstring			m_strConfigFile;
	static bool					m_bMalFormed;
};


#endif //__MULTIFTP_H__

