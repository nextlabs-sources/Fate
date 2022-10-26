#pragma once

#include <fstream>
#include <comdef.h>

#include <activeds.h>
#include <sddl.h>

#include <shlwapi.h>
#pragma comment( lib , "activeds.lib" )
#pragma comment( lib , "adsiid.lib" )
#pragma comment(lib,"shlwapi.lib")


class CWinAD
{
public:
	CWinAD(void);
public:
	~CWinAD(void);
private:
	typedef struct _User_Info
	{
		std::wstring strSID;
		std::wstring strEmail;
	}User_Info;

	std::wstring m_strFileterKeyWords;
	std::wstring m_strSidName;
	std::wstring m_strMailName;
	User_Info m_theUserInfo;
	bool	m_bGetSuccess;

private:
	bool CWinAD::ReadScheme()
	{
		char strFileName[256]="\0";
		DWORD dwLen = ::GetModuleFileNameA(NULL,strFileName,256);
		int i=0;
#pragma warning(push)
#pragma warning(disable: 6385)
		while(strFileName[dwLen-i++] != '\\');
#pragma warning(pop)
		strFileName[dwLen-i+2]='\0';

		strncat_s(strFileName,256,"Obligation.cfg", _TRUNCATE);

		//wcscat_s(wstrFileName,256,L"obligation.cfg");
		std::ifstream inf(strFileName);
		if(!inf.is_open())
		{
			//::MessageBoxW(NULL,L"Can't Open the Cfg File!",L"Error",MB_OK);
			//CLog::WriteLog(L"Error ",L"Can't Open the Cfg File! at ReadScheme!");
			m_strFileterKeyWords = L"";
			m_strSidName = L"objectSid";
			m_strMailName = L"mail";
			return true;
		}
		std::string strline;
//		bool bStart = false;
		while(!inf.eof())
		{
			std::getline(inf,strline);
			if(strline.find("<KeyWord>") != std::string::npos)
			{
				std::getline(inf,strline);
				strline.erase(strline.find_last_not_of(' ')+1);
				strline.erase(0,strline.find_first_not_of(' '));
				BSTR bstrText = _com_util::ConvertStringToBSTR(strline.c_str()); 
				m_strFileterKeyWords = bstrText;
				::SysFreeString(bstrText);
				continue;
			}
			if(strline.find("<ObjectSID>") != std::string::npos)
			{
				std::getline(inf,strline);
				strline.erase(strline.find_last_not_of(' ')+1);
				strline.erase(0,strline.find_first_not_of(' '));
				BSTR bstrText = _com_util::ConvertStringToBSTR(strline.c_str()); 
				m_strSidName = bstrText;
				if(m_strSidName.empty() || _wcsicmp(m_strSidName.c_str(),L"")==0)
				{
					m_strSidName = L"objectSid";
				}
				::SysFreeString(bstrText);	
				continue;
			}
			if(strline.find("<MailAddress>") != std::string::npos)
			{
				std::getline(inf,strline);
				strline.erase(strline.find_last_not_of(' ')+1);
				strline.erase(0,strline.find_first_not_of(' '));
				BSTR bstrText = _com_util::ConvertStringToBSTR(strline.c_str()); 
				m_strMailName = bstrText;
				if(m_strMailName.empty() || _wcsicmp(m_strMailName.c_str(),L"")==0)
				{
					m_strMailName = L"mail";
				}
				::SysFreeString(bstrText);	
				break;				
			}
		}
		inf.close();

		if(m_strSidName.empty() || m_strMailName.empty() ||
			m_strFileterKeyWords.empty())
		{
			::MessageBoxW(NULL,L"Read cfg file failed!",L"Error",MB_OK);
			return false;
		}
		return true;
	}
	HRESULT CWinAD::FindUsers(CComPtr<IDirectorySearch> pContainerToSearch,  // IDirectorySearch pointer to the container to search.
		LPOLESTR szFilter, // Filter for finding specific users.
		// NULL returns all user objects.
		LPOLESTR *pszPropertiesToReturn, // Properties to return for user objects found.
		// NULL returns all set properties.
		BOOL bIsVerbose   // TRUE indicates that all properties for the found objects are displayed.
		// FALSE indicates only the RDN.
		);
private:
	std::wstring m_strManagerKey;
public:
	bool GetManagerInfo(std::wstring& strEmail,std::wstring& strSid)
	{
		std::wstring strKey = L"(distinguishedName=";
		strKey += m_strManagerKey;
		strKey += L")";
		return SearchUserInfo(strEmail,strSid,strKey.c_str());
	}
public:
	bool CWinAD::SearchUserInfo(std::wstring& strEMail,std::wstring& strSID,const wchar_t* wstrkeyWord=NULL);
};
