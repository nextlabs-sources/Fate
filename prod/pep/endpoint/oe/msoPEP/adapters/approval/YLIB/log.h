

#ifndef _YLIB_COMMON_LOG_H_ 
#define _YLIB_COMMON_LOG_H_
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h> 

class CCommonLog
{
public:
    CCommonLog(){}
    virtual ~CCommonLog(){}
    static ULONG    m_flag;

public:
    static void PrintLogA(const char* _Fmt, ...)
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
	        //OutputDebugStringA( buffer );

	        free( buffer );
		}
		va_end(args);
    }
    static void PrintLogW(const wchar_t* _Fmt, ...)
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
	        //OutputDebugStringW( buffer );

	        free( buffer );
		}
		va_end(args);
    }
};

//ULONG CCommonLog::m_flag = 0;

// Show debug
//#ifdef _DEBUG
#ifdef _UNICODE
#define DP(x)   CCommonLog::PrintLogW x
#else
#define DP(x)   CCommonLog::PrintLogA x
#endif
//#else
//#define DP(x)
//#endif

#endif
