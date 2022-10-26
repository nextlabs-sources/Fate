/**
*  @file debug tools implement.
*
*  @author NextLabs::kim
*/

#include "stdAfx.h"
#include "nlofficerep_only_debug.h"



////////////////////////////////Debug log control flag//////////////////////////////////////////
const wchar_t* g_pKwchDebugLogHeader    = L"*** NLOFFICREP_DEBUG";
const wchar_t* g_pkwchDebugLogSeperator = L" ----------------- ";

const bool g_kbDebugMessageBox = true;     /** Control the debug message box pop up */
const bool g_kbDebugTraceLog   = true;     /** Control the debug trace log out put */
const bool g_kbOutputDebugLog  = true;     /** Control the base debug log out put */
const bool g_kbOutputEventViewLog = false;  /** Control the base debug log out put */

////////////////////////////for debug function life//////////////////////////////////////////////
CFunTraceLog::CFunTraceLog(_In_ const wchar_t* const pkwchFuncName, _In_ const unsigned int unlLine) : m_wstrFuncName(L""), m_unlStartLine(0)
{
    if ( g_kbDebugTraceLog )
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
    if ( g_kbDebugTraceLog )
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
    if ( bOutputDebugLog )
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
    if ( g_kbDebugMessageBox )
    {
        nRet = ::MessageBoxW( GetActiveWindow(), pkwchText, pkwchCaption, uType );
    }
    return nRet;
}
