#ifndef COVERLAYTOOL_H
#define COVERLAYTOOL_H


#include "stdafx.h"
#include <sddl.h>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
using namespace std;

class COverlayTool
{
public:
	COverlayTool(){};
	~COverlayTool(){};
public:

	//static void SpliterStringPair(const wstring& strPair,wstring& strname,wstring& strvalue,const wchar_t* pChar);
	static void GetUserInfo(LPWSTR wzSid, int nSize, LPWSTR UserName, int UserNameLen);
	static bool GetGMTTime(wstring& strGmtTime);
	static bool GetLCTime(wstring& strCTTime);
	static wstring GetFormateTime(const tm &today,const wstring & strFormat);
	static wstring ReplaceTimeTxt(const tm &today,const wstring strFormat);
	static bool ReadTimeFromate(const wstring &strFormate,_Out_ wstring &strTimeFormate);
	static void ConvertTxT(__in const wstring &strText,__in const wstring &strFilePath,__in const wstring &strPolicyName,
		__in const wstring &strDateFormat,__in const wstring &strTimeFormat,__out wstring &strTextValueTemp);
	static DWORD GetFontColor(wstring strColor);
};


void COverlayTool::GetUserInfo(LPWSTR wzSid, int nSize, LPWSTR UserName, int UserNameLen)
{
	HANDLE hTokenHandle = 0;

	if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hTokenHandle))
	{
		if(GetLastError() == ERROR_NO_TOKEN)
		{
			if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTokenHandle ))
			{
				return;
			}
		}
		else
		{
			return;
		}
	}

	// Get SID
	UCHAR InfoBuffer[512];
	DWORD cbInfoBuffer = 512;
	LPTSTR StringSid = 0;
	WCHAR   uname[64] = {0}; DWORD unamelen = 63;
	WCHAR   dname[64] = {0}; DWORD dnamelen = 63;
	WCHAR   fqdnname[MAX_PATH+1]; memset(fqdnname, 0, sizeof(fqdnname));
	SID_NAME_USE snu;
	if(!GetTokenInformation(hTokenHandle, TokenUser, InfoBuffer, cbInfoBuffer, &cbInfoBuffer))
		return;
	if(ConvertSidToStringSid(((PTOKEN_USER)InfoBuffer)->User.Sid, &StringSid))
	{
		_tcsncpy_s(wzSid,nSize,StringSid, _TRUNCATE);
		if(StringSid) LocalFree(StringSid);
	}
	if(LookupAccountSid(NULL, ((PTOKEN_USER)InfoBuffer)->User.Sid, uname, &unamelen, dname, &dnamelen, &snu))   
	{
		wcsncat_s( UserName, UserNameLen,uname, _TRUNCATE);
	}
}

bool COverlayTool::GetGMTTime(wstring& strGmtTime)
{
	struct tm curtime;
	__int64 ltime;
	wchar_t buf[64]={0};
	errno_t err;

	_time64( &ltime );

	// Obtain coordinated universal time: 
	err = _gmtime64_s( &curtime, &ltime );
	if (err)	return false;
	// Convert to an ASCII representation 
	err = _wasctime_s(buf, 26, &curtime);
	if(err)	return false;
	wstring strTemp = buf;
	size_t nPos = strTemp.find('\n');
	if(nPos != wstring::npos)
	{
		strTemp = strTemp.substr(0,nPos);
	}
	strGmtTime = strTemp;
	strGmtTime += L" GMT";
	return true;
}
bool COverlayTool::GetLCTime(wstring& strCTTime)
{
	struct tm curtime;
	__int64 ltime;
	wchar_t buf[64]={0};
	errno_t err;

	_time64( &ltime );

	// Obtain coordinated universal time: 
	err = _localtime64_s( &curtime, &ltime );
	if (err)	return false;

	// Convert to an ASCII representation 
	err = _wasctime_s(buf, 26, &curtime);
	if(err)	return false;
	wstring strTemp = buf;
	size_t nPos = strTemp.find('\n');
	if(nPos != wstring::npos)
	{
		strTemp = strTemp.substr(0,nPos);
	}
	strCTTime = strTemp;
	return true;
}
wstring COverlayTool::GetFormateTime(const tm &today,const wstring & strFormat)
{
	wchar_t tmpbuf[256] = {0};
	wcsftime( tmpbuf,256,strFormat.c_str(), &today );
	return tmpbuf;
}

wstring COverlayTool::ReplaceTimeTxt(const tm &today,const wstring strFormat)
{
	wstring strTemp = strFormat;
	boost::replace_all(strTemp,L"%a",GetFormateTime(today,L"%a"));
	boost::replace_all(strTemp,L"%A",GetFormateTime(today,L"%A"));
	boost::replace_all(strTemp,L"%B",GetFormateTime(today,L"%B"));
	boost::replace_all(strTemp,L"%b",GetFormateTime(today,L"%b"));
	boost::replace_all(strTemp,L"%c",GetFormateTime(today,L"%c"));
	boost::replace_all(strTemp,L"%d",GetFormateTime(today,L"%d"));
	boost::replace_all(strTemp,L"%H",GetFormateTime(today,L"%H"));
	boost::replace_all(strTemp,L"%I",GetFormateTime(today,L"%I"));

	boost::replace_all(strTemp,L"%j",GetFormateTime(today,L"%j"));
	boost::replace_all(strTemp,L"%m",GetFormateTime(today,L"%m"));
	boost::replace_all(strTemp,L"%M",GetFormateTime(today,L"%M"));
	boost::replace_all(strTemp,L"%p",GetFormateTime(today,L"%p"));
	boost::replace_all(strTemp,L"%S",GetFormateTime(today,L"%S"));
	boost::replace_all(strTemp,L"%U",GetFormateTime(today,L"%U"));
	boost::replace_all(strTemp,L"%w",GetFormateTime(today,L"%w"));
	boost::replace_all(strTemp,L"%W",GetFormateTime(today,L"%W"));
	boost::replace_all(strTemp,L"%X",GetFormateTime(today,L"%X"));
	boost::replace_all(strTemp,L"%x",GetFormateTime(today,L"%x"));

	boost::replace_all(strTemp,L"%y",GetFormateTime(today,L"%y"));
	boost::replace_all(strTemp,L"%Y",GetFormateTime(today,L"%Y"));
	boost::replace_all(strTemp,L"%Z",GetFormateTime(today,L"%Z"));
	boost::replace_all(strTemp,L"%z",GetFormateTime(today,L"%z"));
	boost::replace_all(strTemp,L"%%",GetFormateTime(today,L"%%"));

	boost::replace_all(strTemp,L"%#a",GetFormateTime(today,L"%#a"));
	boost::replace_all(strTemp,L"%#A",GetFormateTime(today,L"%#A"));
	boost::replace_all(strTemp,L"%#b",GetFormateTime(today,L"%#b"));
	boost::replace_all(strTemp,L"%#B",GetFormateTime(today,L"%#B"));
	boost::replace_all(strTemp,L"%#p",GetFormateTime(today,L"%#p"));
	boost::replace_all(strTemp,L"%#X",GetFormateTime(today,L"%#X"));
	boost::replace_all(strTemp,L"%#z",GetFormateTime(today,L"%#z"));
	boost::replace_all(strTemp,L"%#Z",GetFormateTime(today,L"%#Z"));
	boost::replace_all(strTemp,L"%#%",GetFormateTime(today,L"%#%"));

	boost::replace_all(strTemp,L"%#c",GetFormateTime(today,L"%#c"));
	boost::replace_all(strTemp,L"%#x",GetFormateTime(today,L"%#x"));
	boost::replace_all(strTemp,L"%#d",GetFormateTime(today,L"%#d"));
	boost::replace_all(strTemp,L"%#H",GetFormateTime(today,L"%#H"));
	boost::replace_all(strTemp,L"%#I",GetFormateTime(today,L"%#I"));
	boost::replace_all(strTemp,L"%#j",GetFormateTime(today,L"%#j"));
	boost::replace_all(strTemp,L"%#m",GetFormateTime(today,L"%#m"));
	boost::replace_all(strTemp,L"%#M",GetFormateTime(today,L"%#M"));
	boost::replace_all(strTemp,L"%#S",GetFormateTime(today,L"%#S"));
	boost::replace_all(strTemp,L"%#U",GetFormateTime(today,L"%#U"));
	boost::replace_all(strTemp,L"%#w",GetFormateTime(today,L"%#w"));
	boost::replace_all(strTemp,L"%#W",GetFormateTime(today,L"%#W"));
	boost::replace_all(strTemp,L"%#y",GetFormateTime(today,L"%#y"));
	boost::replace_all(strTemp,L"%#Y",GetFormateTime(today,L"%#Y"));

	return strTemp;

}
bool COverlayTool::ReadTimeFromate(const wstring &strFormate,_Out_ wstring &strTimeFormate)
{
	time_t ltime;
	struct tm today;
	errno_t err;
	_time64( &ltime );

	if (strFormate.empty())
	{
		return false;
	}

	wstring strTime = strFormate;
	if (boost::algorithm::icontains(strFormate,L"gmt "))
	{
		boost::replace_all(strTime,L"gmt ",L"");
		err = _gmtime64_s( &today, &ltime );
		if (err)	return false;
	}
	else
	{
		err = _localtime64_s( &today, &ltime );
		if (err)	return false;
	}


	strTimeFormate = ReplaceTimeTxt(today,strTime);
	return true;
}

void COverlayTool::ConvertTxT(__in const wstring &strText,__in const wstring &strFilePath,__in const wstring &strPolicyName,
							  __in const wstring &strDateFormat,__in const wstring &strTimeFormat,__out wstring &strTextValueTemp)
{
	if (strText.empty())
	{
		return;
	}
	

	WCHAR wzName[MAX_PATH+1] = {0};
	WCHAR wzSid[MAX_PATH+1]  = {0};
	GetUserInfo(wzSid,MAX_PATH,wzName,MAX_PATH);

	strTextValueTemp = strText;
	boost::algorithm::replace_all(strTextValueTemp,wstring(L"%userId"),wstring(wzSid));
	boost::algorithm::replace_all(strTextValueTemp,wstring(L"%userName"),wstring(wzName));

	wstring strlocaltime,strgmttime;
	GetLCTime(strlocaltime);
	GetGMTTime(strgmttime);

	boost::algorithm::replace_all(strTextValueTemp,wstring(L"%gmtTime"),strgmttime);
	boost::algorithm::replace_all(strTextValueTemp,wstring(L"%localTime"),strlocaltime);

	wstring strTemp(strFilePath);

	wstring::size_type npos = strTemp.rfind(L"\\");
	if(npos == wstring::npos)	npos = strTemp.rfind(L"/");
	if(npos == wstring::npos) boost::algorithm::replace_all(strTextValueTemp,L"%fileName",strTemp);
	else boost::algorithm::replace_all(strTextValueTemp,L"%fileName",strTemp.substr(npos + 1));

	boost::algorithm::replace_all(strTextValueTemp,L"%filePath",strTemp);

	char szHostname[MAX_PATH+1]={0};
	wchar_t wzHostname[MAX_PATH]={0};
	gethostname(szHostname, MAX_PATH);
	if(0 != szHostname[0])
	{
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szHostname, -1, wzHostname, MAX_PATH);
	}
	boost::algorithm::replace_all(strTextValueTemp,L"%hostName",wzHostname);
	boost::algorithm::replace_all(strTextValueTemp,L"%policyName",strPolicyName);
	boost::algorithm::replace_all(strTextValueTemp,L"%%",L"%");

	wstring strTempTime = L"";
	ReadTimeFromate(strDateFormat,strTempTime);
	boost::algorithm::replace_all(strTextValueTemp,L"%date",strTempTime);

	strTempTime = L"";
	ReadTimeFromate(strTimeFormat,strTempTime);
	boost::algorithm::replace_all(strTextValueTemp,L"%time",strTempTime);
	
}

DWORD COverlayTool::GetFontColor(wstring strColor)
{
	DWORD dwTemp = 0;
	if(strColor.empty())
	{
		return 0;
	}
	strColor = strColor.substr(0,6);

	int i = 0;
	while(i < 6)
	{
		dwTemp  = dwTemp * 16;
		wchar_t ch = strColor[i];

		if (ch == 'a'||ch == 'A')
		{
			dwTemp += 10;
			i++;
			continue;
		}
		if (ch == 'b'||ch == 'B')
		{
			dwTemp += 11;
			i++;
			continue;
		}
		if (ch == 'c'||ch == 'C')
		{
			dwTemp += 12;
			i++;
			continue;
		}
		if (ch == 'd'||ch == 'D')
		{
			dwTemp += 13;
			i++;
			continue;
		}
		if (ch == 'e'||ch == 'E')
		{
			dwTemp += 14;
			i++;
			continue;
		}
		if (ch == 'f'||ch == 'F')
		{
			dwTemp += 15;
			i++;
			continue;
		}
		dwTemp += _wtol(&ch);
		i++;
	}
	return dwTemp;
}




#endif