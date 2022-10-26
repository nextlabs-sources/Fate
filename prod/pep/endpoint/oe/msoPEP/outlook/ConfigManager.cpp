#include "stdafx.h"
#include "ConfigManager.h"
#include "nlconfig.hpp"

CConfigManager theCfg;

CConfigManager::CConfigManager()
{
}


CConfigManager::~CConfigManager()
{
}


BOOL CConfigManager::LoadConfig()
{
	//get config file name
	WCHAR wszOEInstallPath[MAX_PATH+1] = {0};
	NLConfig::GetComponentInstallPath( L"Compliant Enterprise\\Outlook Enforcer", wszOEInstallPath, _countof(wszOEInstallPath));
	std::wstring wstrCfgFile = wszOEInstallPath;
	wstrCfgFile += L"Outlook Enforcer\\config\\OutlookEnforcer.ini";
	DP((L"CConfigManager::LoadConfig path=%s", wstrCfgFile.c_str()));

	//get all section
	DWORD dwBufLen = 1024;
	wchar_t* wszBuffer = NULL;
	do 
	{
		wszBuffer = new wchar_t[dwBufLen + 1];
		DWORD dwReadLen = GetPrivateProfileString(NULL, NULL, L"", wszBuffer, dwBufLen, wstrCfgFile.c_str());
		if (dwReadLen==dwBufLen-2)//buffer to small
		{
			delete[] wszBuffer;
			wszBuffer = NULL;
			dwBufLen += 1024;
		}
		else
		{
			break;
		}

	} while (TRUE);

	//read each section
	wchar_t* pSection = wszBuffer;
	while (wcslen(pSection)>0)
	{
		m_mapCfgSections[pSection].LoadConfigSection(wstrCfgFile.c_str(), pSection);
		pSection += wcslen(pSection) + 1; //next section
	}

	//release buffer
	delete[] wszBuffer;
	wszBuffer = NULL;

	return TRUE;
}

int CConfigManager::QueryPCTimeout()
{
	std::wstring wstrTimeOut = theCfg[L"Timeout"][L"QueryPCTimeout"];
	int nTimeout = _wtoi(wstrTimeOut.c_str());
	if (nTimeout>0)
	{
		return nTimeout;
	}
	else
	{
       return 60000;
	}

}

std::wstring CConfigManager::GetXHeaderKeyPrefix()
{
	std::wstring wstrPrefix = theCfg[L"XHeader"][L"KeyPrefix"];
	if (wstrPrefix.empty())
	{
		wstrPrefix = L"XHeader-";
	}
	return wstrPrefix;
}

const CConfigSection& CConfigManager::operator[](const wchar_t* wszSectionName) const
{
	std::map<std::wstring,CConfigSection>::const_iterator itSectoin = m_mapCfgSections.find(wszSectionName);
	if (itSectoin!=m_mapCfgSections.end())
	{
		return itSectoin->second;
	}
	else
	{
		static CConfigSection emptySection;
		return emptySection;
	}
}


void CConfigSection::LoadConfigSection(const wchar_t* wszFileName, const wchar_t* wszSectionName)
{
	//get all key
	DWORD dwBufLen = 1024;
	wchar_t* wszBuffer = NULL;
	do
	{
		wszBuffer = new wchar_t[dwBufLen + 1];
		DWORD dwReadLen = GetPrivateProfileString(wszSectionName, NULL, L"", wszBuffer, dwBufLen, wszFileName);
		if (dwReadLen == dwBufLen - 2)//buffer to small
		{
			delete[] wszBuffer;
			wszBuffer = NULL;
			dwBufLen += 1024;
		}
		else
		{
			break;
		}

	} while (TRUE);

	//read each key
	const wchar_t* pKeyName = wszBuffer;
	while (wcslen(pKeyName)>0)
	{
		std::wstring wstrValue = GetIniString(wszFileName, wszSectionName, pKeyName);
		m_mapCfgValue[pKeyName] = wstrValue;

		pKeyName += wcslen(pKeyName) + 1;  //next section
	}

	//release buffer
	delete[] wszBuffer;
	wszBuffer = NULL;
}

const std::wstring& CConfigSection::operator[](const wchar_t* wszKeyName)  const
{
	std::map<std::wstring, std::wstring>::const_iterator itKeyValue = m_mapCfgValue.find(wszKeyName);
	if (itKeyValue != m_mapCfgValue.end())
	{
		return itKeyValue->second;
	}
	else
	{
		static std::wstring wstrEmpty;
		return wstrEmpty;
	}
}

std::wstring CConfigSection::GetIniString(const wchar_t* wszFileName, const wchar_t* wszSectionName, const wchar_t* wszKeyName)
{
	if ((wszSectionName==NULL) || (wszKeyName==NULL))
	{
		return L"";
	}

	//get value
	DWORD dwBufLen = 1024;
	wchar_t* wszBuffer = NULL;
	do
	{
		wszBuffer = new wchar_t[dwBufLen + 1];
		DWORD dwReadLen = GetPrivateProfileString(wszSectionName, wszKeyName, L"", wszBuffer, dwBufLen, wszFileName);
		if (dwReadLen == dwBufLen - 1)//buffer to small
		{
			delete[] wszBuffer;
			wszBuffer = NULL;
			dwBufLen += 1024;
		}
		else
		{
			break;
		}

	} while (TRUE);

	//get value
	std::wstring wstrValue = wszBuffer;
	delete[] wszBuffer;
	wszBuffer = NULL;

	return wstrValue;
}
