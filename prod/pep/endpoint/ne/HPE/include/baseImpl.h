#ifndef __BASE_IMPLEMENT_H__
#define __BASE_IMPLEMENT_H__
#include <time.h>
#include <limits.h>
#include <stdio.h> 

typedef HANDLE (WINAPI* CreateFileWType2)(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);
typedef BOOL (WINAPI* CloseHandleType2)(HANDLE hObject);

extern CreateFileWType2 g_realCreateFileW;
extern CloseHandleType2 g_realCloseHandle;

class CTime
{
public:
	CTime() {} ;
	virtual ~CTime(){} ;
public:
	/*
	returns the java time
	*/
	virtual time_t getSystemTime(PSYSTEMTIME pSysTime = NULL ) 
	{
		time_t rtTime = 0;
		tm     rtTM;
		BOOL bFlag = FALSE ;
		if( pSysTime == NULL )
		{
			pSysTime = new SYSTEMTIME() ;
			GetSystemTime( pSysTime ) ;
			bFlag = TRUE ;
		}
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
		if( bFlag == TRUE )
		{
			delete pSysTime ;
		}
		return rtTime;
	};
	virtual DWORD getTickCout()
	{
		return GetTickCount() ;
	};
	virtual DWORD getFileLastModifiedTime( const wchar_t *pszFileName, wchar_t* pszTimeBuf, INT iTimeLen ) 
	{
		if(!g_realCreateFileW || !g_realCloseHandle || !pszFileName || !pszTimeBuf)
		{
			DPW((L"HPE::getFileLastModifiedTime g_realCreateFileW OR g_realCloseHandle is NULL"));
			return 0;
		}

		HANDLE hFile = INVALID_HANDLE_VALUE;
		DWORD dRet = (DWORD)FTPE_ERROR ;
		memset(pszTimeBuf, 0, iTimeLen*sizeof(WCHAR));
		hFile = g_realCreateFileW(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(INVALID_HANDLE_VALUE != hFile)
		{
			FILETIME ftLastModify;  memset(&ftLastModify, 0, sizeof(FILETIME));
			if(GetFileTime(hFile, NULL, NULL, &ftLastModify))
			{
				FILETIME ftLocalLastModify;  memset(&ftLocalLastModify, 0, sizeof(FILETIME));
				SYSTEMTIME stModifyTime;    memset(&stModifyTime, 0, sizeof(SYSTEMTIME));
				// We should pass UTC time, so don't convert it to local time
				if(FileTimeToSystemTime(&ftLastModify, &stModifyTime))
				{
					time_t tmModify;

					tmModify = getSystemTime(&stModifyTime);
					_snwprintf_s(pszTimeBuf, iTimeLen, _TRUNCATE, L"%I64d", tmModify);
					dRet = (DWORD)FTPE_SUCCESS ;
				}
			}
			g_realCloseHandle(hFile);
		}
		return dRet ;
	};
protected:
private:
};
class CMemory
{
public:
	CMemory(){} ;
	virtual ~CMemory() {} ;
public:
protected:
private:

};

#endif