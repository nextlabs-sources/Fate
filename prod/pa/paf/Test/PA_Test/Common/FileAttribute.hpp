#ifndef __NEXTLABS_FILE_ATTRIBUTE_H__ 
#define __NEXTLABS_FILE_ATTRIBUTE_H__ 

///////////////////////////////////////////////////////////////////
////// get the special file detail attributes.
////// put the file name &path, 
///////////////////////////////////////////////////////////////////

#include <time.h>

time_t WinTime2JavaTime(SYSTEMTIME* pSysTime)
{
    time_t rtTime = 0;
    tm     rtTM;

    rtTM.tm_year = pSysTime->wYear - 1900;
    rtTM.tm_mon  = pSysTime->wMonth - 1;
    rtTM.tm_mday = pSysTime->wDay;
    rtTM.tm_hour = pSysTime->wHour;
    rtTM.tm_min  = pSysTime->wMinute;
    rtTM.tm_sec  = pSysTime->wSecond;
    rtTM.tm_wday = pSysTime->wDayOfWeek;
    rtTM.tm_isdst = -1;     // Let CRT Lib compute whether DST is in effect,
                            // assuming US rules for DST.
    rtTime = mktime(&rtTM); // get the second from Jan. 1, 1970

    if (rtTime == (time_t) -1)
    {
        if (pSysTime->wYear <= 1970)
        {
            // Underflow.  Return the lowest number possible.
            rtTime = (time_t) 0;
        }
        else
        {
            // Overflow.  Return the highest number possible.
            rtTime = (time_t) _I64_MAX;
        }
    }
    else
    {
        rtTime*= 1000;          // get millisecond
    }

    return rtTime;
};

BOOL GetFileLastModifyTime( const TCHAR* i_pszFileName,TCHAR *i_pszTime, INT i_iBuffsize ) 
{
	BOOL bRet = FALSE ;
	HANDLE hFile = INVALID_HANDLE_VALUE ;

	ZeroMemory( i_pszTime, i_iBuffsize*sizeof(TCHAR ) ) ;
	if( i_pszFileName == NULL )
	{
		return bRet ;
	}

	hFile = ::CreateFile( i_pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dError= 0 ;
	dError = ::GetLastError() ;

	if( hFile == INVALID_HANDLE_VALUE ) 
	{
		return bRet ;
	}
	FILETIME fLastModify ;
	::ZeroMemory( &fLastModify,sizeof( FILETIME ) ) ;
	if( GetFileTime( hFile,NULL, NULL, &fLastModify )  )
	{
		SYSTEMTIME sysTime ;
		::ZeroMemory( &sysTime, sizeof( SYSTEMTIME ) ) ;
		if( FileTimeToSystemTime( &fLastModify,&sysTime ) ) 
		{
			time_t timeModify ;
			timeModify = WinTime2JavaTime( &sysTime ) ;
			_snwprintf_s(i_pszTime, i_iBuffsize, _TRUNCATE, L"%i64d", timeModify) ;
			bRet = TRUE ;
		}
	}
	::CloseHandle( hFile ) ;
	return bRet ;
} ;
#endif