#ifndef EMGRDLG_UTILS_H_
#define EMGRDLG_UTILS_H_

static BOOL GetCurrentDirectory(char* pszFullFileName, int nLen)
{
	if(!pszFullFileName)
	{
		return FALSE;
	}

	static char szCurDirectory[MAX_PATH + 1] = {0};

	if(strlen(szCurDirectory) == 0)
	{
		::GetModuleFileNameA(
			NULL,                   // handle to module
			szCurDirectory,        // file name of module
			MAX_PATH                // size of buffer
			);

		char* pTemp = strrchr(szCurDirectory, '\\');
		if(pTemp)
		{
			*(++pTemp) = 0;
		}
	}

	strncpy_s(pszFullFileName, nLen, szCurDirectory, _TRUNCATE);

	return strlen(pszFullFileName) > 0? TRUE: FALSE;
}

static BOOL getFileLastModifiedTime( const wchar_t *pszFileName, LPSYSTEMTIME lpLastModifiedTime /*UTC time*/) 
{
	if( !pszFileName )
	{
		return FALSE;
	}

	BOOL bRet = FALSE;

	WIN32_FIND_DATA ffd ;
	HANDLE hFind = FindFirstFile(pszFileName, &ffd);

	if(hFind != INVALID_HANDLE_VALUE)
	{
		FILETIME ftLastModify; 
		ftLastModify = ffd.ftLastWriteTime;

		FILETIME localTime;
		if(FileTimeToLocalFileTime(&ftLastModify, &localTime))
		{
			if(FileTimeToSystemTime(&localTime, lpLastModifiedTime))
			{
				bRet = TRUE;

			}
		}
		FindClose(hFind);
	}

	return bRet ;
};

static BOOL getFileLastModifiedTime( const wchar_t *pszFileName, LPWSTR pszLastModifiedTime, int nLen )
{
	if(!pszFileName || !pszLastModifiedTime)
	{
		return FALSE;
	}

	SYSTEMTIME lastModifiedTime;
	if(getFileLastModifiedTime(pszFileName, &lastModifiedTime))
	{
		int nHour;
		wchar_t buffer[10] = {0};
		wchar_t minute[10] = {0};
		wchar_t year[10] = {0};

		nHour = lastModifiedTime.wHour;

		if(lastModifiedTime.wHour >= 0 && lastModifiedTime.wHour <= 11)
		{
			wcsncpy_s(buffer, 10, L"AM", _TRUNCATE);

			if(lastModifiedTime.wHour == 0)
			{
				nHour = 12;
			}
		}
		else
		{
			wcsncpy_s(buffer, 10, L"PM", _TRUNCATE);
			if(lastModifiedTime.wHour >= 13)
			{
				nHour = lastModifiedTime.wHour - 12;
			}
		}

		if(lastModifiedTime.wMinute <= 9)
		{
			_snwprintf_s(minute, 10, _TRUNCATE, L"0%d", lastModifiedTime.wMinute);
		}
		else
		{
			_snwprintf_s(minute, 10, _TRUNCATE, L"%d", lastModifiedTime.wMinute);
		}

		_snwprintf_s(year, 10, _TRUNCATE, L"%d", lastModifiedTime.wYear);
		if(wcslen(year) == 4)
		{
			year[0] = year[2];
			year[1] = year[3];
			year[2] = '\0';
		}

		wchar_t szLastModifiedTime[100] = {0};
		_snwprintf_s(szLastModifiedTime, 100, _TRUNCATE, L"%d/%d/%s %d:%s%s", lastModifiedTime.wMonth, lastModifiedTime.wDay, year, nHour, minute, buffer);

		wcsncpy_s(pszLastModifiedTime, nLen, szLastModifiedTime, _TRUNCATE);

		return TRUE;
	}

	return FALSE;
}

#endif