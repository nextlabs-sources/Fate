#pragma once
#ifndef _NLOFFICEPEP_UTILS_
#define _NLOFFICEPEP_UTILS_

#pragma warning( push )
#pragma warning( disable: 4995 )
#include "celog.h"								
#pragma warning( pop )

#include <winhttp.h>

#include <assert.h>
#include <basetsd.h>

// useful macro
#define print_string(s)  s?s:" "
#define print_long_string(s) s?s:L" "
#define print_non_string(s) s?*s:0 

//////////////////////////////////////////////////////////////////////////
//	Global Variables
//////////////////////////////////////////////////////////////////////////
extern BOOL g_bDebugMode_L1;
extern BOOL g_bDebugMode_L2;
extern bool g_bFourceLogDefault;

extern std::wstring g_cacheFilePath;

extern HINSTANCE g_hInstance;
extern wstring	g_strModulePath;
extern BOOL g_bSavePressed;





//////////////////////////////////////////////////////////////////////////
//	NL_PEP_Log
//////////////////////////////////////////////////////////////////////////

//CELog2
#define CELOG_CUR_MODULE L"NLOfficePEP"  // the current module name
enum  // every file should has a file integer
{
	EMNLFILEINTEGER_DLLMAIN = 1,
	EMNLFILEINTEGER_UTILS,
	EMNLFILEINTEGER_NLACTION,
	EMNLFILEINTEGER_NLACTIONTYPE,
	EMNLFILEINTEGER_NLCOMADDIN,
	EMNLFILEINTEGER_NLDATA,
	EMNLFILEINTEGER_NLENCRYPT,
	EMNLFILEINTEGER_NLFILETAGGING,
	EMNLFILEINTEGER_NLFILETYPE,
	EMNLFILEINTEGER_NLGLOBALINFO,
	EMNLFILEINTEGER_NLHOOKAPI,
	EMNLFILEINTEGER_NLINSERTANDDATAACTION,
	EMNLFILEINTEGER_NLLOGICFLAG,
	EMNLFILEINTEGER_NLOBLIGATIONTYPE,
	EMNLFILEINTEGER_NLOBMGR,
	EMNLFILEINTEGER_NLOFFICEPEP,
	EMNLFILEINTEGER_NLOFFICE_Comm,
	EMNLFILEINTEGER_NLOFFICESINK,
	EMNLFILEINTEGER_OFFICELISTENER,
	EMNLFILEINTEGER_NLPROCESS,
	EMNLFILEINTEGER_NLSESYNCHROGOLDENTAGS,
	EMNLFILEINTEGER_NLTAG,
	EMNLFILEINTEGER_POLICY,
	EMNLFILEINTEGER_STDAFX,
	EMNLFILEINTEGER_NLOFFICEPEPCOMM,
	EMNLFILEINTEGER_NLSECONDARYTHREADFORPDFMAKER
};

//For CELog2 define

// Debug string
#define NLCELOG_LOG( ... ) CELOG_LOG( CELOG_DEBUG, __VA_ARGS__ )

//
// External macros for function entering and leaving.  These should be used
// for:
// - functions that do not handle any exceptions.
// - functions that handle C++ exceptions.
//

// Function entering
#define NLCELOG_ENTER CELOG_ENTER;

// Function returning void
#define NLCELOG_RETURN CELOG_RETURN;

// Function returning a value whose type is supported
#define NLCELOG_RETURN_VAL(val) CELOG_RETURN_VAL(val);

// Function returning a value whose type is not supported
#define NLCELOG_RETURN_VAL_NOPRINT(val) CELOG_RETURN_VAL_NOPRINT(val);


// for user define type: enum & pointer
// Entering (for functions returning an enum value)
#define NLCELOG_ENUM_ENTER(type) CELOG_ENUM_ENTER(type);

// Returning an enum value
#define NLCELOG_ENUM_RETURN_VAL(val) CELOG_ENUM_RETURN_VAL(val);

// Entering (for functions returning a pointer to non-void type)
#define NLCELOG_PTR_ENTER(type) CELOG_PTR_ENTER(type);

// Returning a pointer to non-void type
#define NLCELOG_PTR_RETURN_VAL(val) CELOG_PTR_RETURN_VAL(val);

//
// External macros for function entering and leaving.  These should be used
// for:
// - functions that handle Structured Exceptions (SEH).
//
// Function entering
#define NLCELOG_SEH_ENTER CELOG_SEH_ENTER;

// Function returning void
#define NLCELOG_SEH_RETURN CELOG_SEH_RETURN;

// Function returning a value whose type is supported
#define NLCELOG_SEH_RETURN_VAL(val) CELOG_SEH_RETURN_VAL(val);

// Function returning a value whose type is not supported
#define NLCELOG_SEH_RETURN_VAL_NOPRINT(val) CELOG_SEH_RETURN_VAL_NOPRINT(val);


// for user define type: enum & pointer
// Entering (for functions returning an enum value)
#define NLCELOG_SEH_ENUM_ENTER(type) CELOG_SEH_ENUM_ENTER(type);

// Returning an enum value
#define NLCELOG_SEH_ENUM_RETURN_VAL(val) CELOG_SEH_ENUM_RETURN_VAL(val);

// Entering (for functions returning a pointer to non-void type)
#define NLCELOG_SEH_PTR_ENTER(type) CELOG_SEH_PTR_ENTER(type);

// Returning a pointer to non-void type
#define NLCELOG_SEH_PTR_RETURN_VAL(val) CELOG_SEH_PTR_RETURN_VAL(val);


/*
���ڵ����õ�һЩ������
1. �����ڱ�־�꣺�����ڵ��ַ�����־�����ڼ�¼����������ɻ����ƵĲ���
2. ����ģ����Ե��ַ�����־ͷ
3. �����������ڵı�־��
4. �����ַ������������
5. ���̱���ʱ�Զ�dump�ļ��ռ���

������Ϣ��
1. ģ��Ľ��������������Ϣ
2. �ṹ���쳣����

��ȫ�ԣ�
1. �̰߳�ȫ����
2. ��Դ���亯�����쳣��ȫ
*/

// Flag
static const wchar_t* g_pwchToolStringFlag = L"*** NLOFFICEPEP_DEBUG";
static const wchar_t* g_pwchToolSepratorFlag = L"------------------------------------------------------";


// For compile flag
#define chSTR2(x) #x
#define chSTR(x)  chSTR2(x)

#if 1
#define chMSG(desc) message( "" )
#else
#define chMSG(desc) message( __FILE__ "(" chSTR(__LINE__) "):" #desc )
#endif


#define ONLY_DEBUG CFunTraceLog OnlyForDebugLog( __FUNCTIONW__, __LINE__ ); 
class CFunTraceLog
{
public:
	CFunTraceLog(_In_ const wstring& wstrFuncName, _In_ const unsigned long& unlLine);
	~CFunTraceLog();
private:
	wstring m_wstrFuncName;
	unsigned long m_unlStartLine;
};

// For debug string, Variable length
#define NLPRINT_DEBUGLOGEX( bFourceLog, ... ) /*NLCELOG_LOG( __VA_ARGS__ )*/ NLPrintLogW( bFourceLog, L"%s:[%s:%s:%d]>:", g_pwchToolStringFlag, __FUNCTIONW__, __FILEW__, __LINE__ ), NLPrintLogW( bFourceLog, __VA_ARGS__ )
#define NLPRINT_DEBUGLOG( ... )               /*NLCELOG_LOG( __VA_ARGS__ )*/ NLPrintLogW( g_bFourceLogDefault, L"%s:[%s:%s:%d]>:", g_pwchToolStringFlag, __FUNCTIONW__, __FILEW__, __LINE__ ), NLPrintLogW( g_bFourceLogDefault, __VA_ARGS__ )
void NLPrintLogW(_In_ const bool bFourceLog, _In_ const wchar_t* _Fmt, ...);

#define NLPRINT_TAGPAIRLOG /*NLCELogPrintTagPairW*/ NLPrintLogW( g_bFourceLogDefault, L"%s:[%s:%d]>:", g_pwchToolStringFlag, __FUNCTIONW__, __LINE__ ), NLPrintTagPairW
void NLPrintTagPairW(_In_ const vector<pair<wstring, wstring>>& vecTagPair, _In_opt_ const wchar_t* pwchBeginFlag = NULL, _In_opt_ const wchar_t* pwchEndFlag = NULL);
void NLCELogPrintTagPairW(_In_ const vector<pair<wstring, wstring>>& vecTagPair, _In_opt_ const wchar_t* pwchBeginFlag = NULL, _In_opt_ const wchar_t* pwchEndFlag = NULL);

int NLMessageBox(_In_opt_ const wchar_t* pwchText, _In_opt_ const wchar_t* pwchCaption, _In_ UINT uType);



// use boost locks
typedef boost::shared_mutex rwmutex;
typedef boost::shared_lock<rwmutex> readLock;
typedef boost::unique_lock<rwmutex> writeLock;



namespace utils{
	

	class CIniFile
	{
	public:
		inline CIniFile(wchar_t* filename)
		{
			if (NULL != filename)
			{
				wcsncpy_s(filename_, MAX_PATH, filename, _TRUNCATE);
			}
		}
		~CIniFile(){}
		inline bool FileExist(){
			return ::PathFileExistsW(filename_);
		}
		//
		// Retrieve a string value from an INI file
		//
		inline void ReadString(const wchar_t* pszSection, const wchar_t* pszIdent, const wchar_t* pszDefault, wchar_t* pszResult)
		{
			DWORD dwResult = ::GetPrivateProfileStringW(
				pszSection,        // section name
				pszIdent,          // key name
				NULL,              // default string
				pszResult,         // destination buffer  
				MAX_PATH,          // size of destination buffer
				filename_       // initialization file name
				);

			if (!dwResult)
				wcsncpy_s(pszResult, MAX_PATH, pszDefault, _TRUNCATE);
		}
		//
		// Retrieve a boolean value from an INI file
		//
		inline BOOL ReadBool(const wchar_t* pszSection, const wchar_t* pszIdent, BOOL bDefault)
		{
			wchar_t    szResult[MAX_PATH];
			BOOL    bResult = bDefault;
			wchar_t    szDefault[MAX_PATH];

			BoolToStr(bDefault, szDefault);

			ReadString(
				pszSection,
				pszIdent,
				szDefault,
				szResult
				);

			bResult = StrToBool(szResult);

			return bResult;
		}
	public:
		// Converts a string to a boolean value
		inline BOOL StrToBool(const wchar_t* pszValue)
		{
			return ((0 == _wcsicmp(L"YES", pszValue)) ||
				(0 == _wcsicmp(L"Y", pszValue)) ||
				(0 == _wcsicmp(L"TRUE", pszValue)) ||
				(0 == _wcsicmp(L"T", pszValue)));
		}

		// Converts a boolean value to a string
		inline void BoolToStr(BOOL bValue, wchar_t* pszResult)
		{
			wcsncpy_s(pszResult, MAX_PATH, bValue ? L"Yes" : L"No", _TRUNCATE);
		}
	private:
		wchar_t filename_[MAX_PATH];
	};

	namespace timedate{
		inline time_t WinTime2JavaTime(SYSTEMTIME* pSysTime)
		{
			time_t rtTime = 0;
			tm       rtTM = { 0 };

			rtTM.tm_year = pSysTime->wYear - 1900;
			rtTM.tm_mon = pSysTime->wMonth - 1;
			rtTM.tm_mday = pSysTime->wDay;
			rtTM.tm_hour = pSysTime->wHour;
			rtTM.tm_min = pSysTime->wMinute;
			rtTM.tm_sec = pSysTime->wSecond;
			rtTM.tm_wday = pSysTime->wDayOfWeek;
			rtTM.tm_isdst = -1;     // Let CRT Lib compute whether DST is in effect,

			// assuming US rules for DST.
			rtTime = mktime(&rtTM); // get the second from Jan. 1, 1970

			if (rtTime == (time_t)-1)
			{
				if (pSysTime->wYear <= 1970)
				{
					rtTime = (time_t)0;		// Underflow.  Return the lowest number possible.
				}
				else
				{
					rtTime = (time_t)_I64_MAX;	// Overflow.  Return the highest number possible.
				}
			}
			else
			{
				rtTime *= 1000;          // get millisecond
			}
			return rtTime;
		}

		inline std::wstring GetCurrentStringTime()
		{
			WCHAR wzTm[MAX_PATH + 1] = { 0 };
			SYSTEMTIME sysTm = { 0 };
			GetSystemTime(&sysTm);
			time_t javaTime = WinTime2JavaTime(&sysTm);
			StringCchPrintfW(wzTm, MAX_PATH, L"%I64d", javaTime);
			return wzTm;
		}
	}
}


//
//
//

wstring GetCommonComponentsDir();

bool IsLocalDriver(const wstring& strPath);

BOOL GetOSInfo(OSVERSIONINFOEX* os);

BOOL IsWin10();

BOOL IsWin7(void /*include Vista*/);

BOOL IsXp(void); // versions: 5.1 XP

void ConvertURLCharacterW(_Inout_ wstring& strUrl);

wstring GetUpperFolder(const wstring& strFilePath);


bool GetCrackURL(HINTERNET hIns, wstring& strUrlPath);

DWORD GetParentProcessID(DWORD  dwPID);

void NLCloseHandle(_Inout_ HANDLE& hHandle);



wstring GetSuffixFromFileName(_In_ const wstring& wstrFileName);

wstring GetFilePath(_In_ const wstring& wstrFilePath);

wstring GetFileName(_In_ const wstring& wstrFilePath);

wstring GetNetFileNameByFilePath(_In_ const wstring& wstrFilePath);

wstring GetUserAppDataPath();

BOOL IsInOKRect(HWND hwnd, POINT point);

BOOL GetOleContentClipboardDataSource(IDataObject *pDataObject, const std::wstring& clipboardInfo, __out std::wstring& srcFilePath);

BOOL isContentData(IDataObject *pDataObject);

std::wstring GetCurProcessFolderPath();

#endif //_NLOFFICEPEP_UTILS_