

#pragma once
#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "nlconfig.hpp"
#include "celog.h"
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"
#include "eframework/auto_disable/auto_disable.hpp"
//#include "Utilities.h"

static CELog log;
extern nextlabs::recursion_control hook_control;
CCommonLog::CCommonLog()
{
}

CCommonLog::~CCommonLog()
{
}

void CCommonLog::Initialize(void)
{
  nextlabs::recursion_control_auto auto_disable(hook_control);

  // If debug mode is enabled write to log file as well
  if( NLConfig::IsDebugMode() == true )
  {
    /* Generate a path using the image name of the current process.  Set log policy for DebugView
     * and file on log instance.  Path will be [NextLabs]/Network Enforcer/diags/logs/.
     */
    char image_name[MAX_PATH] = {0};
	
    if( !GetCurrentProcessName2(image_name, MAX_PATH, NULL) )
    {
      return;
    }
    char* image_name_ptr = strrchr(image_name,'\\'); // get just image name w/o full path
    if( image_name_ptr == NULL )
    {
      image_name_ptr = image_name;                   // when failed use default image name
    }
    else
    {
      image_name_ptr++;                              // move past '\\'
    }

    wchar_t ne_install_path[MAX_PATH] = {0};
    if( NLConfig::GetComponentInstallPath(L"Enterprise DLP\\Network Enforcer",ne_install_path,_countof(ne_install_path)) == true )
    {
      char log_file[MAX_PATH] = {0};
      sprintf_s(log_file,_countof(log_file),"%ws\\diags\\logs\\hpe_%s.txt",ne_install_path,image_name_ptr);
      log.SetPolicy( new CELogPolicy_File(log_file) );
    }
  }

  log.SetPolicy( new CELogPolicy_WinDbg() ); // output to DebugView
  log.Enable();                              // enable log
  log.SetLevel(CELOG_DEBUG);                 // log threshold to debug level

}/* Initialize */

void CCommonLog::PrintLogA(const char* _Fmt, ...)
{
  nextlabs::recursion_control_auto auto_disable(hook_control);
  if( NLConfig::IsDebugMode() == false )
  {
    return;
  }
    va_list args;
    int     len;
    char    *buffer;

    // retrieve the variable arguments
    va_start( args, _Fmt );

    len = _vscprintf_l( _Fmt, 0, args ) // _vscprintf doesn't count
        + 1; // terminating '\0'

    buffer = (char*)malloc( len * sizeof(char) );
	if( buffer)
	{
		vsprintf_s( buffer, len, _Fmt, args );
		log.Log(CELOG_DEBUG,"%s",buffer);
		free( buffer );
	}
   va_end(args);
}

void CCommonLog::PrintLogW(const WCHAR* _Fmt, ...)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    if( NLConfig::IsDebugMode() == false )
    {
      return;
    }

    va_list args;
    int     len;
    WCHAR   *buffer;

    // retrieve the variable arguments
    va_start( args, _Fmt );

    len = _vscwprintf_l( _Fmt, 0, args ) // _vscprintf doesn't count
        + 1; // terminating '\0'

    buffer = (WCHAR*)malloc( len * sizeof(WCHAR) );
	if( buffer )
	{
		vswprintf_s( buffer, len, _Fmt, args );
		log.Log(CELOG_DEBUG,L"%s",buffer);
		free( buffer );
	}
   va_end(args);
}

BOOL GetCurrentProcessName2(LPSTR lpszProcessName, int nLen, HMODULE hMod)
{
	if(!lpszProcessName || nLen > 1024)
		return FALSE;

	if(hMod)
	{
		DWORD dwCount = GetModuleFileNameA(hMod, lpszProcessName, nLen);
		return dwCount > 0 ? TRUE: FALSE;
	}

	static char filename[1025] = {0};
	if( strlen(filename) == 0)//only call GetModuleFileNameW at the first time. Kevin 20089-8-20
	{
		GetModuleFileNameA(hMod, filename, 1024);
	}

	if(strlen(filename) > 0)
	{
		memcpy(lpszProcessName, filename, nLen * sizeof(char));
		return TRUE;
	}

	return FALSE;
}
