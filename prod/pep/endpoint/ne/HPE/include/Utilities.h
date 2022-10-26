#pragma once
#include <string>
#include <algorithm>
#include "nlexcept.h"
#include "eframework/auto_disable/auto_disable.hpp"
using namespace std;

BOOL GetFileNameFromHandle(HANDLE hFile, wstring& sFileName); 

template<class T1, class T2>
basic_string<T2> StringT1toT2(const basic_string<T1>& strt1)
{
	basic_string<T2> strt2;
	copy(strt1.begin(), strt1.end(), back_inserter(strt2));
	return strt2;
}

template<class T>
bool TStringicmp(const basic_string<T>& str1, const basic_string<T>& str2)
{
	basic_string<T>::size_type len1 = str1.length();
	basic_string<T>::size_type len2 = str2.length();
	if(len1 != len2)
		return false;
	for(basic_string<T>::size_type idx = 0; idx < len1; ++idx)
	{
		if((T)tolower(str1[idx]) != (T)tolower(str2[idx]))
			return false;
	}
	return true;
}

template<class T>
typename basic_string<T>::size_type StringFindNoCase(const basic_string<T>& str, const typename basic_string<T>::traits_type::char_type* pstrSub)
{
	if(pstrSub == NULL)
		return basic_string<T>::npos;
	basic_string<T> tmpstr(str);
	basic_string<T> tmpsubstr(pstrSub);
	transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), tolower);
	transform(tmpsubstr.begin(), tmpsubstr.end(), tmpsubstr.begin(), tolower);
	return tmpstr.find(tmpsubstr);
}

BOOL GetCurrentProcessName(LPWSTR lpszProcessName, int nLen, HMODULE hMod);
BOOL GetCurrentProcessName2(LPSTR lpszProcessName, int nLen, HMODULE hMod);

BOOL IsProcess(LPCWSTR lpProcessName);

int GetIEVersionNum();
std::wstring GetIEVersionNum_str();

void InitDetachCritical();
void SetDetachFlag(BOOL bFlag);
BOOL GetDetachFlag();

/********************************
function name: IsPolicyControllerUp
function feature: this function was 
used to determine if PC is stopped. 
*********************************/
bool IsPolicyControllerUp(void);

std::string MyWideCharToMultipleByte(const std::wstring & strValue);

void exception_cb( NLEXCEPT_CBINFO* cb_info );

void ConvertUNCPath(wstring& strPath);

bool IsIgnoredIP(LPCWSTR lpIP);
string AddressToString(const struct sockaddr* addr, int addr_len, bool with_port);
BOOL CheckNetworkAccess( SOCKET s,const wchar_t* lpIP,const  wchar_t* lpPort) ;

void HPE_Init();
void HPE_Finalize();

std::wstring MyMultipleByteToWideChar(const std::string & strValue);
