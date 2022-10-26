#pragma once
#include <string>
#include <algorithm>
#include "eframework/auto_disable/auto_disable.hpp"
using namespace std;

#define DUMMY_DESTINATION L" "

typedef struct struEvalCache
{
	std::wstring strAction;
	std::wstring strSrc;
	std::wstring strDest;
	BOOL		 bAllow;
	DWORD		 dwTimeEntry;
}EVALCACHE;

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
BOOL IsProcess(LPCWSTR lpszName);

void SetDetachFlag(BOOL bFlag);
BOOL GetDetachFlag();

/*****************************************************************
function: PushEvalCache
feature: this function is used to cache an evaluation, including
action, source path, destination path, result.
Note: don't use NULL for the parameters. use L"" if the string is
empty.
******************************************************************/
void PushEvalCache(LPCWSTR lpszAction, LPCWSTR lpszSrc, LPCWSTR lpszDest, BOOL bAllow);

BOOL GetEvalCache(LPCWSTR lpszAction, LPCWSTR lpszSrc, LPCWSTR lpszDest, OUT BOOL& bAllow);

/********************************
function name: IsPolicyControllerUp
function feature: this function was 
used to determine if PC is stopped. 
*********************************/
bool IsPolicyControllerUp(void);

std::string MyWideCharToMultipleByte(const std::wstring & strValue);
std::wstring MyMultipleByteToWideChar(const std::string & strValue);

void exception_cb( NLEXCEPT_CBINFO* cb_info );

void ConvertUNCPath(wstring& strPath);

void FTPE_Init();
void FTPE_Finalize();
BOOL GetOSInfo(DWORD& dwMajor, DWORD& dwMinor) ;