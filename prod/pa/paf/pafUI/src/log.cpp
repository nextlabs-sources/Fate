

#pragma once
#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "log.h"

CCommonLog::CCommonLog()
{
}

CCommonLog::~CCommonLog()
{
}

void CCommonLog::PrintLogA(const char* _Fmt, ...)
{
    va_list args;
    int     len;
    char    *buffer;

    // retrieve the variable arguments
    va_start( args, _Fmt );

    len = _vscprintf_l( _Fmt, 0, args ) // _vscprintf doesn't count
        + 1; // terminating '\0'

    buffer = (char*)malloc( len * sizeof(char) );

	//for warning C6387
	if (buffer)
	{
    vsprintf_s( buffer, len, _Fmt, args ); // C4996
    // Note: vsprintf is deprecated; consider using vsprintf_s instead
    OutputDebugStringA( buffer );

    free( buffer );
	}

	va_end(args);
}

void CCommonLog::PrintLogW(const WCHAR* _Fmt, ...)
{
    va_list args;
    int     len;
    WCHAR   *buffer;

    // retrieve the variable arguments
    va_start( args, _Fmt );

    len = _vscwprintf_l( _Fmt, 0, args ) // _vscprintf doesn't count
        + 1; // terminating '\0'

    buffer = (WCHAR*)malloc( len * sizeof(WCHAR) );

	//for warning C6387
	if (buffer)
	{
    vswprintf_s( buffer, len, _Fmt, args ); // C4996
    // Note: vsprintf is deprecated; consider using vsprintf_s instead
    OutputDebugStringW( buffer );

    free( buffer );
	}
	va_end(args);
}
