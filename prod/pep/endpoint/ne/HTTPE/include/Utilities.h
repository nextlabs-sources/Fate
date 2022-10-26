#pragma once
#include <string>
#include <map>
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

BOOL GetFileNameFromHandle(HANDLE hFile, std::wstring& strFilePath) ;


BOOL GetModuleName(LPWSTR lpszProcessName, int nLen, HMODULE hMod);
BOOL IsProcess(LPCWSTR lpszName);
BOOL IsSupportedProcess();

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


int GetIEVersionNum();
std::wstring GetIEVersionNum_str();

BOOL GetProcessSID(HANDLE hProcess, std::wstring& strSID);
bool NLIsWellKnownSid( const WCHAR* sid );

BOOL InitLog();

std::string MyWideCharToMultipleByte(const std::wstring & strValue);
std::wstring MyMultipleByteToWideChar(const std::string & strValue);
std::wstring MyMultipleByteToWideChar(const std::string & strValue, UINT codepage);

void HTTPE_Initialize();
void HTTPE_Release();

void TryStringAppend(LPCVOID lpBuffer,  DWORD nNumberOfBytesToWrite);
bool TestStringAppendError(LPCVOID lpBuffer,  DWORD nNumberOfBytesToWrite);

BOOL GetOSInfo(DWORD& dwMajor, DWORD& dwMinor);

void exception_cb( NLEXCEPT_CBINFO* cb_info );

std::string UrlDecode(const std::string& src);

wstring GetPeerIP(SOCKET s);

void ConvertUNCPath(wstring& strPath);

BOOL IsIgnoredByPolicy();

std::wstring GetCommonComponentsDir();

class CUtility
{
public:
	CUtility() ;
	~CUtility() ;
public:
    static DWORD GetVersionNumber(const std::wstring & szModuleName, const std::wstring & szKeyName) ;
	static bool IsSupportFileType(LPCWSTR lpFilePath)    ;
	static bool GetIgnoreFolderList(std::map<std::wstring, int>& mapIgnoreFolders);
	static bool IsOfficeFile( LPCSTR lpFilePath ) ;

} ;

class CEncoding
{
public:
	static BOOL UTF8ToGB2312(const string& strInput, string& strOut);
};