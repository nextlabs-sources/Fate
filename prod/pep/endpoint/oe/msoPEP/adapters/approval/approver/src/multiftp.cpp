#include "stdafx.h"
#include "multiftp.h"
#include "../import/vbscript.tlh"
#include "log.h"
bool CFtpSite::Matched(const WCHAR* wzEmailAddress)
{
	try
	{
		VBScriptRegEx::IRegExpPtr regExp(__uuidof(VBScriptRegEx::RegExp) );
		regExp->Pattern=_bstr_t(this->m_strEmailAddressPattern.c_str());
		if(regExp->Test(wzEmailAddress))
		{
			VBScriptRegEx::IMatchCollectionPtr matches=regExp->Execute(wzEmailAddress);
			if(matches->Count ==1)
			{
				return true;
			}
		}
	}
	catch (_com_error &e)
	{
		DP((L"exception happen regular expression matching. Error=%s",e.ErrorMessage()));
	}
	return false;
}
const WCHAR CMultiFTPManager::MULTIFTP_CONFIGFILE[]=L"multiftp.cfg";
const WCHAR CMultiFTPManager::MULTIFTP_CFG_KEY_EMAILADDRESSPATTERN[]=L"EmailAddressPattern";
const WCHAR CMultiFTPManager::MULTIFTP_CFG_KEY_FTPSERVER[]=L"FTPServer";
const WCHAR CMultiFTPManager::MULTIFTP_CFG_KEY_FTPUSER[]=L"FTPUser";
const WCHAR CMultiFTPManager::MULTIFTP_CFG_KEY_FTPPASSWORD[]=L"FTPPassword";
std::wstring CMultiFTPManager::m_strConfigFile=L"";
std::vector<CFtpSite>	CMultiFTPManager::m_vecFtpSites;
bool					CMultiFTPManager::m_bMalFormed;
void CMultiFTPManager::Init()
{
	if(m_strConfigFile.length()==0)
	{
		//get the full path of multiple ftp config file
		WCHAR wzModuleName[MAX_PATH+1];memset(wzModuleName,0,sizeof(wzModuleName));
		DWORD dwRet=::GetModuleFileName(m_hInstance,wzModuleName,MAX_PATH);
		
		if(dwRet==0)
		{
			m_strConfigFile = L".\\";
			m_strConfigFile +=MULTIFTP_CONFIGFILE;
		}
		else
		{
			m_strConfigFile=wzModuleName;
			std::wstring::size_type pos=m_strConfigFile.rfind(L"\\");
			if(pos!=std::wstring::npos)
			{
				std::wstring strTemp=m_strConfigFile.substr(0,pos);
				m_strConfigFile=strTemp;
				m_strConfigFile+=L"\\";
				m_strConfigFile+=MULTIFTP_CONFIGFILE;
			}
			else
			{
				m_strConfigFile+=L"\\";
				m_strConfigFile+=MULTIFTP_CONFIGFILE;
			}
		}
		//read all ftp sites information from the config file
		WCHAR wzSiteNames[1024*4+1];memset(wzSiteNames,0,sizeof(wzSiteNames));
		dwRet=GetPrivateProfileString(NULL,NULL,NULL,wzSiteNames,sizeof(wzSiteNames)/sizeof(WCHAR)-1,m_strConfigFile.c_str());
		std::wstring strErrorMsg;
		if(dwRet)
		{
			WCHAR * pSiteName=wzSiteNames;
			WCHAR wzAddressPattern[1024+1];memset(wzAddressPattern,0,sizeof(wzAddressPattern));
			WCHAR wzFtpServer[MAX_PATH+1];memset(wzFtpServer,0,sizeof(wzFtpServer));
			WCHAR wzFtpUser[MAX_PATH+1];memset(wzFtpUser,0,sizeof(wzFtpUser));
			WCHAR wzFtpPasswd[MAX_PATH+1];memset(wzFtpPasswd,0,sizeof(wzFtpPasswd));
			while(pSiteName[0]!=L'\0')
			{
				WCHAR * pTempSiteName=pSiteName;
				pSiteName=pSiteName+wcslen(pSiteName)+1;
				//EmailAddressPattern
				dwRet=GetPrivateProfileString(pTempSiteName,MULTIFTP_CFG_KEY_EMAILADDRESSPATTERN,NULL,
												wzAddressPattern,sizeof(wzAddressPattern)/sizeof(WCHAR)-1,m_strConfigFile.c_str());
				if(dwRet==0)
				{
					m_bMalFormed=true;
					strErrorMsg=L"no definition key EmailAddressPattern in section ";
					strErrorMsg+=pTempSiteName;
					CLog::WriteLog(L"multiftp.cfg error",strErrorMsg.c_str());
					return;
				}
				//FTPServer
				dwRet=GetPrivateProfileString(pTempSiteName,MULTIFTP_CFG_KEY_FTPSERVER,NULL,
												wzFtpServer,sizeof(wzFtpServer)/sizeof(WCHAR)-1,m_strConfigFile.c_str());
				if(dwRet==0)
				{
					m_bMalFormed=true;
					strErrorMsg=L"no definition key FTPServer in section ";
					strErrorMsg+=pTempSiteName;
					CLog::WriteLog(L"multiftp.cfg error",strErrorMsg.c_str());
					return;
				}
				//FTPUser
				dwRet=GetPrivateProfileString(pTempSiteName,MULTIFTP_CFG_KEY_FTPUSER,NULL,
												wzFtpUser,sizeof(wzFtpUser)/sizeof(WCHAR)-1,m_strConfigFile.c_str());
				if(dwRet==0)
				{
					m_bMalFormed=true;
					strErrorMsg=L"no definition key FTPUser in section ";
					strErrorMsg+=pTempSiteName;
					CLog::WriteLog(L"multiftp.cfg error",strErrorMsg.c_str());
					return;
				}
				//FTPPassword
				dwRet=GetPrivateProfileString(pTempSiteName,MULTIFTP_CFG_KEY_FTPPASSWORD,NULL,
												wzFtpPasswd,sizeof(wzFtpPasswd)/sizeof(WCHAR)-1,m_strConfigFile.c_str());
				if(dwRet==0)
				{
					m_bMalFormed=true;
					strErrorMsg=L"no definition key FTPPassword in section ";
					strErrorMsg+=pTempSiteName;
					CLog::WriteLog(L"multiftp.cfg error",strErrorMsg.c_str());
					return;
				}
				
				CFtpSite ftpSite(pTempSiteName,wzAddressPattern,wzFtpServer,wzFtpUser,wzFtpPasswd);
				AddSite(ftpSite);
			}
			m_bMalFormed=false;
		}
		else
		{
			m_bMalFormed=true;
			CLog::WriteLog(L"multiftp.cfg error",L"failed to get the colleciton of section name!");
			return;
		}
	}
};

bool CFtpSite::IsSameSite(CFtpSite& anotherSite)
{
	if(::CompareString(LOCALE_USER_DEFAULT,
					   NORM_IGNORECASE,
					   this->GetSiteName().c_str(),-1,
					   anotherSite.GetSiteName().c_str(),-1)==CSTR_EQUAL)
		return true;

	if(::CompareString(LOCALE_USER_DEFAULT,
					   NORM_IGNORECASE,
					   this->GetFtpServer().c_str(),-1,
					   anotherSite.GetFtpServer().c_str(),-1)==CSTR_EQUAL
		&&
		::CompareString(LOCALE_USER_DEFAULT,
					   NORM_IGNORECASE,
					   this->GetFtpUser().c_str(),-1,
					   anotherSite.GetFtpUser().c_str(),-1)==CSTR_EQUAL)
	   return true;
	return false;
}


CMultiFTPManager::MATCH_TYPE CMultiFTPManager::Match(std::vector<std::wstring>& vecRecipients,CFtpSite&site)
{
	//std::vector<CFtpSite>::iterator itSite;
	bool bRetOneMatch=false,bMatched=false,bNoMached=false;
	CFtpSite matchedSite,returnedSite;
	std::vector<std::wstring>::iterator itRec;
	//int aiResult[2]={0,0};
	for(itRec=vecRecipients.begin();itRec!=vecRecipients.end();itRec++)
	{
		bRetOneMatch=Match((*itRec),returnedSite);
		if(bRetOneMatch==true)
		{
			if(bNoMached==true)
				return MISMATCH;
			if(matchedSite.GetSiteName().length()==0)
			{
				matchedSite=returnedSite;
			}
			else
			{
				if(matchedSite.IsSameSite(returnedSite)==false)
					return MISMATCH;
			}
			bMatched=true;
		}
		else
		{
			bNoMached=true;
			if(bMatched==true)
				return MISMATCH;
		}
	}
	if(bMatched==true)
	{
		site=matchedSite;
		return ONE;
	}
	return NONE;
}
bool CMultiFTPManager::Match(std::wstring& strRecipient,CFtpSite& site)
{
	std::vector<CFtpSite>::iterator itSite;
	for(itSite=this->m_vecFtpSites.begin();itSite!=this->m_vecFtpSites.end();itSite++)
	{
		if(itSite->Matched(strRecipient.c_str())==true)
		{
			site=(*itSite);
			return true;
		}
	}
	return false;
}
