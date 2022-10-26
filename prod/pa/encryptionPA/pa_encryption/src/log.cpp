

#pragma once

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "log.h"

#ifdef WIN32
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
	if (buffer != NULL)
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
	if (buffer != NULL)
	{
	    vswprintf_s( buffer, len, _Fmt, args ); // C4996
	    // Note: vsprintf is deprecated; consider using vsprintf_s instead
	    OutputDebugStringW( buffer );

	    free( buffer );
	}
	va_end(args);
}


void NL_Trace(const char *file, int line, const char *proc, const char *fmt, ...)  
{
	va_list marker;
	char s[1024];memset(s, 0, sizeof(s));
	char s2[1024];memset(s2, 0, sizeof(s2));
	const char *p = strrchr( file, '/' );

	if( p )  
		file = p+1;

	va_start( marker, fmt );     
	vsnprintf_s( s, 1024, _TRUNCATE, fmt, marker );
	va_end( marker ); 
	_snprintf_s( s2, 1024, _TRUNCATE, "\n**** %s (%4d): %s()%s %s\n", file, line, proc, *s?",":"", s );
#if WIN32
	OutputDebugStringA( s2 );
#else
	printf( "%s", s2 );	/* TODO: logging for other platforms */
#endif
}


#endif
