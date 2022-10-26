

#pragma once
#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#pragma warning(push)
#pragma warning(disable: 6334)
#include <boost/algorithm/string.hpp>
#pragma warning(pop)
#include "log.h"
#pragma warning(push)
#pragma warning(disable: 4267)
#include "nlconfig.hpp"
#pragma warning(pop)
#include "celog.h"
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"
#include "eframework/auto_disable/auto_disable.hpp"

#include "Shlwapi.h"
#pragma comment(lib,"Shlwapi.lib")
#include "strsafe.h"
#include <string>
using namespace std;
#define OE_LOG_SUBDIR "Nextlabs\\OutlookEnforcer\\diags\\logs" 



CELog g_log_OE;

HANDLE CEventLog::m_hEventLog = NULL;
bool   CEventLog::m_bSetlog = false;
DWORD  CEventLog::m_dwTime = 0;

extern nextlabs::recursion_control mso_hook_control;
extern HINSTANCE g_hInstance;

CCommonLog::CCommonLog()
{
}

CCommonLog::~CCommonLog()
{
}

void CCommonLog::Initialize(void)
{
    nextlabs::recursion_control_auto auto_disable(mso_hook_control);

	// %LocalAppData%\Nextlabs\OutlookEnforcer\diags\logs\OutlookEnforcer.log
	char szLogPath[MAX_PATH] = "";
	HRESULT hr = SHGetFolderPathAndSubDirA(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, OE_LOG_SUBDIR, szLogPath);
	if (SUCCEEDED(hr))
	{
		PathAppendA(szLogPath, "OutlookEnforcer.log");
		g_log_OE.SetPolicy( new CELogPolicy_File(szLogPath));
	}

    g_log_OE.SetPolicy( new CELogPolicy_WinDbg() ); // output to DebugView
    g_log_OE.Enable();                              // enable log
    g_log_OE.SetLevel(CELOG_DEBUG);                 // log threshold to debug level
}/* Initialize */

void CCommonLog::PrintLogA(const char* _Fmt, ...)
{
    nextlabs::recursion_control_auto auto_disable(mso_hook_control);
    va_list args;

    if( NLConfig::IsDebugMode() == false )
    {
        return;
    }

    va_start( args, _Fmt );// retrieve the variable arguments
    VPrintLogA(CELOG_DEBUG, _Fmt, args);
    va_end(args);
}

void CCommonLog::PrintLogA(int lvl, const char* _Fmt, ...)
{
	va_list args;
	va_start( args, _Fmt ); // retrieve the variable arguments
	VPrintLogA(lvl, _Fmt, args);
	va_end(args);
}

void CCommonLog::VPrintLogA(int lvl, const char* _Fmt, va_list _ArgList)
{
	char sline[CELOG_MAX_MESSAGE_SIZE_CHARS] = "";
	vsprintf_s(sline,_countof(sline), _Fmt,_ArgList);
	g_log_OE.Log(lvl, "%s", sline);
}

void CCommonLog::PrintLogW(const WCHAR* _Fmt, ...)
{
    nextlabs::recursion_control_auto auto_disable(mso_hook_control);
    va_list args;

    if( NLConfig::IsDebugMode() == false )
    {
        return;
    }

	// retrieve the variable arguments
	va_start( args, _Fmt );
	VPrintLogW(CELOG_DEBUG, _Fmt,args);
	va_end(args);
}


void CCommonLog::PrintLogW(int lvl, const wchar_t* _Fmt, ...)
{
	va_list args;
	va_start( args, _Fmt ); // retrieve the variable arguments
	VPrintLogW(lvl, _Fmt, args);
	va_end(args);
}
//
//void CCommonLog::VPrintLogW(int lvl, const wchar_t* _Fmt, va_list _ArgList)
//{
//	wchar_t sline[CELOG_MAX_MESSAGE_SIZE_CHARS] = L"";
//	vswprintf_s(sline, _countof(sline), _Fmt, _ArgList);
//	g_log_OE.Log(lvl, L"%s", sline);
//}

void CCommonLog::VPrintLogW(int lvl, const wchar_t* _Fmt, va_list _ArgList)
{
	// vwprintf(_Fmt, _ArgList);
	g_log_OE.Log(lvl, NULL, 0, _Fmt, _ArgList);
}

void CCommonLog::PrintW(const WCHAR* _Fmt, ...)
{
    nextlabs::recursion_control_auto auto_disable(mso_hook_control);
    va_list args;
    int     len = 0;
    WCHAR   *buffer = 0;

    // retrieve the variable arguments
    va_start( args, _Fmt );

    len = _vscwprintf_l( _Fmt, 0, args ) // _vscprintf doesn't count
        + 1; // terminating '\0'

    buffer = (WCHAR*)malloc( len * sizeof(WCHAR) );

    if(NULL != buffer)
    {
        vswprintf_s( buffer, len, _Fmt, args ); // C4996
        // Note: vsprintf is deprecated; consider using vsprintf_s instead
        g_log_OE.Log(CELOG_DEBUG,L"%s",buffer);
        free( buffer );
    }
    va_end(args);
}





CEventLog::CEventLog()
{

}
CEventLog::~CEventLog()
{

}
void CEventLog::RegEventSource()
{
	m_hEventLog =  RegisterEventSource(NULL,L"Nextlabs Outlook Enforcer");
	if (m_hEventLog == NULL)
	{
		DP((L"Call RegisterEventSource fail!\n"));
	}

}
void CEventLog::DelEventSource()
{
	if (m_hEventLog != NULL)
	{	
		DeregisterEventSource(m_hEventLog);
		m_hEventLog = NULL;
	}
}
void CEventLog::WriteEventLog(const wchar_t *strlog, WORD LogType,DWORD LgID)
{
    if (strlog == NULL || wcslen(strlog) < 1)	return;

	size_t Len = wcslen(strlog) + MAX_PATH;
	wchar_t *strEventLog = new wchar_t[Len];
    strEventLog[0] = L'\0';
	if ((LogType == EVENTLOG_ERROR_TYPE) || (LogType==EVENTLOG_SUCCESS))
	{
		StringCbPrintf(strEventLog,Len,L"[%s]\n", strlog);
	}
	else
	{
		DWORD dwTime = m_dwTime;
		m_dwTime = GetTickCount();
		DWORD dwTimeSpan = m_dwTime - dwTime;
		if (dwTimeSpan > 1000)
		{
			StringCbPrintf(strEventLog,Len,L"[%s]. The TimeSpan is [%d] !\n", strlog, dwTimeSpan);
		}
		else
		{
			if (strEventLog != NULL)
			{
				delete []strEventLog;
			}
			return;
		}
	}
	NLPRINT_DEBUGVIEWLOG(L"%s", strEventLog);
	if (m_hEventLog != NULL)
	{
		BOOL bRet = TRUE;
		bRet=::ReportEventW(m_hEventLog,
			LogType,
			0,
			LgID,
			NULL, 
			1,
			0,
			const_cast<LPCWSTR*>(&strEventLog),
			NULL);
		if (!bRet)
		{
			DP((L"ReportEvent GetLastError is %d \n", GetLastError()));
		}
	}
	if (strEventLog != NULL)
	{
		delete []strEventLog;
	}	
}


void CEventLog::SetInitTime()
{
	m_dwTime = GetTickCount();
}


////////////////////////////Only debug begin//////////////////////////////////////////////
namespace nldebug
{

    ////////////////////////////////Debug log control flag//////////////////////////////////////////
    const wchar_t* g_pKwchDebugLogHeader    = L"*** NLOFFICREP_DEBUG";
    const wchar_t* g_pkwchDebugLogSeperator = L" ----------------- ";

    const bool g_kbDebugMessageBox = true;     /** Control the debug message box pop up */
    const bool g_kbDebugTraceLog = true;       /** Control the debug trace log out put */
    const bool g_kbOutputDebugLog = true;      /** Control the base debug log out put */
    const bool g_kbOutputEventViewLog = true;  /** Control the base debug log out put */
    const bool g_kbForceOutputDebugLog = true; /** Control the force debug log out put */

    ////////////////////////////for debug function life//////////////////////////////////////////////
    CFunTraceLog::CFunTraceLog(_In_ const wchar_t* const pkwchFuncName, _In_ const unsigned int unlLine) : m_wstrFuncName(L""), m_unlStartLine(0)
    {
#pragma warning(push)
#pragma warning(disable: 4127)
        if ( g_kbDebugTraceLog )
#pragma warning(pop)
        {
            if ( NULL != pkwchFuncName )
            {
                m_wstrFuncName = pkwchFuncName;
                m_unlStartLine = unlLine;
                NLPrintLogW(g_kbDebugTraceLog, L"\n->>-->>-->>-->>-->>-->>------->>>>>>>>>>>>>IN::, %s,start line:[%u]\n", m_wstrFuncName.c_str(), m_unlStartLine);
            }        
        }    
    }

    CFunTraceLog::~CFunTraceLog()
    {
#pragma warning(push)
#pragma warning(disable: 4127)
        if ( g_kbDebugTraceLog )
#pragma warning(pop)
        {
            NLPrintLogW(g_kbDebugTraceLog, L"\n-<<--<<--<<--<<--<<--<<-------<<<<<<<<<<<<<OUT::,%s,start line:[%u]\n", m_wstrFuncName.c_str(), m_unlStartLine);
        }    
    }
    ///////////////////////////////////end///////////////////////////////////////

    wchar_t* NLConvertBoolToString(_In_ const bool bIsTtrue)
    {
        return bIsTtrue ? L"true" : L"false" ;
    }

    void NLPrintLogW(_In_ const bool bOutputDebugLog, _In_ const wchar_t* _Fmt, ...)
    {
#pragma warning(push)
#pragma warning(disable: 4127)
        if ( bOutputDebugLog )
#pragma warning(pop)
        {
            // To ensure that this function is safe, even if an exception occurs.
            __try
            {
                va_list  args;
                int      len = 0;
                wchar_t* pwchBuffer = NULL;

                // retrieve the variable arguments
                va_start(args, _Fmt);

                len = _vscwprintf_l(_Fmt, 0, args) + 1; // _vscprintf doesn't count, terminating '\0' 
                pwchBuffer = (wchar_t*)malloc(len * sizeof(wchar_t));

                if (NULL != pwchBuffer)
                {
                    __try
                    {
                        vswprintf_s(pwchBuffer, len, _Fmt, args);
                        NLOutputDebugStringW(pwchBuffer);
                    }
                    __finally
                    {
                        free(pwchBuffer); // To make sure that this code should be execute,
                    }
                }
                va_end(args);
            }
            __except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) // To ensure that the global expansion 
            {
                // nothing to do, maybe a serious error happened
                NLOutputDebugStringW(L"PrintlogW function, an exception happened \n");
            }
        }
    }

    void NLPrintTagPairW(_In_ const std::vector<std::pair<std::wstring, std::wstring>>& kvecTagPair, _In_opt_ const wchar_t* pkwchBeginFlag, _In_opt_ const wchar_t* pkwchEndFlag)
    {
        NLPrintLogW( g_kbOutputDebugLog, L"[%s] [%s] [%s]\n", g_pkwchDebugLogSeperator, NULL == pkwchBeginFlag ?  L"Begin" : pkwchBeginFlag, g_pkwchDebugLogSeperator );
        for ( std::vector<std::pair<std::wstring, std::wstring>>::const_iterator cItr = kvecTagPair.begin(); cItr != kvecTagPair.end(); cItr++ )
        {
            NLPrintLogW( g_kbOutputDebugLog, L"tag name:[%s], tag value:[%s]\n", cItr->first.c_str(), cItr->second.c_str() );
        }
        NLPrintLogW( g_kbOutputDebugLog, L"[%s] [%s] [%s]\n", g_pkwchDebugLogSeperator, NULL == pkwchEndFlag ?  L"End" : pkwchEndFlag, g_pkwchDebugLogSeperator );
    }

    void NLOutputDebugStringW(_In_ wchar_t* pwchDebugLog)
    {
        ::OutputDebugStringW(pwchDebugLog);
    }

    int NLMessageBoxW(_In_opt_ const wchar_t* pkwchText, _In_opt_ const wchar_t* pkwchCaption, _In_ unsigned int uType)
    {
        int nRet = IDCANCEL;
#pragma warning(push)
#pragma warning(disable: 4127)
        if ( g_kbDebugMessageBox )
#pragma warning(pop)
        {
            nRet = ::MessageBoxW( GetActiveWindow(), pkwchText, pkwchCaption, uType );
        }
        return nRet;
    }

} /* namespace nlofficerep */
////////////////////////////Only debug end//////////////////////////////////////////////