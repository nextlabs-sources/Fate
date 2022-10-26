

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


static CELog g_log_OE;

extern nextlabs::recursion_control mso_hook_control;

CCommonLog::CCommonLog()
{
}

CCommonLog::~CCommonLog()
{
}

void CCommonLog::Initialize(void)
{
    nextlabs::recursion_control_auto auto_disable(mso_hook_control);
    // If debug mode is enabled write to log file as well
    if( NLConfig::IsDebugMode() == true )
    {
        WCHAR CEInstallPath[MAX_PATH+1] = {0};
        if( NLConfig::GetComponentInstallPath( L"Compliant Enterprise\\Policy Controller", CEInstallPath, _countof(CEInstallPath)) == true )
        {
            //OutputDebugStringW(L"*** msoPEP.LogFile ***\n");
            std::wstring wstrInstallPath(CEInstallPath);
            //OutputDebugStringW(wstrInstallPath.c_str());
            std::string strInstallPath(wstrInstallPath.begin(), wstrInstallPath.end());
            //OutputDebugStringA(strInstallPath.c_str());
            if(!boost::algorithm::iends_with(strInstallPath, "\\"))
                strInstallPath += "\\";
            strInstallPath += "Outlook Enforcer\\diags\\logs\\OutlookEnforcer.log";
            //OutputDebugStringA(strInstallPath.c_str());
            g_log_OE.SetPolicy( new CELogPolicy_File(strInstallPath.c_str()) );
        }
        /*else
        {
            OutputDebugStringW(L"** msoPEP **! Fail to get install path.\n");
        }*/
    }
    /*else
    {
        OutputDebugStringW(L"** msoPEP **! Not in debug mode.\n");
    }*/

    g_log_OE.SetPolicy( new CELogPolicy_WinDbg() ); // output to DebugView
    g_log_OE.Enable();                              // enable log
    g_log_OE.SetLevel(CELOG_DEBUG);                 // log threshold to debug level
}/* Initialize */

void CCommonLog::PrintLogA(const char* _Fmt, ...)
{
    nextlabs::recursion_control_auto auto_disable(mso_hook_control);
    va_list args;
    int     len = 0;
    char    *buffer = 0;

    if( NLConfig::IsDebugMode() == false )
    {
        return;
    }

    // retrieve the variable arguments
    va_start( args, _Fmt );

    len = _vscprintf_l( _Fmt, 0, args ) // _vscprintf doesn't count
        + 1; // terminating '\0'

    buffer = (char*)malloc( len * sizeof(char) );
    if(NULL != buffer)
    {
        vsprintf_s( buffer, len, _Fmt, args ); // C4996
        // Note: vsprintf is deprecated; consider using vsprintf_s instead
        g_log_OE.Log(CELOG_DEBUG,"%s",buffer);
        free( buffer );
    }
    va_end(args);
}

void CCommonLog::PrintLogW(const WCHAR* _Fmt, ...)
{
    nextlabs::recursion_control_auto auto_disable(mso_hook_control);
    va_list args;
    int     len = 0;
    WCHAR   *buffer = 0;

    if( NLConfig::IsDebugMode() == false )
    {
        return;
    }

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
