/*
 * LogFile.h 
 * Author: Helen Friedland
 * All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
 * Redwood City CA, Ownership remains with Blue Jungle Inc, 
 * All rights reserved worldwide. 
 */


#if !defined(_LOGFILE_H_)
#define _LOGFILE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <share.h>
#include <stdio.h>
//---------------------------------------------------------------------------
//
// class CLogFile 
//
//---------------------------------------------------------------------------
class CLogFile
{
public:
	CLogFile(BOOL bTraceEnabled):
	  m_bTraceEnabled(bTraceEnabled)
	{
		m_hMutex = ::CreateMutexA(
			NULL, 
			FALSE, 
			"{45428D53-A5DB-4168-BD3C-658419B60279}"
			); 
	}
	virtual ~CLogFile()
	{
		::CloseHandle(m_hMutex);
	}

	void InitializeFileName(char* pszFileName)
	{
		lstrcpyA(m_szFileName, pszFileName);
	}

	void DoLogMessage(char* pszMessage)
	{
		::WaitForSingleObject(m_hMutex, INFINITE);
		__try
		{
			if (m_bTraceEnabled)
			{
				FILE* pOutFile;
				char  szFlags[2];
				strncpy_s(szFlags, _countof(szFlags), "a", _TRUNCATE);

				pOutFile = _fsopen(m_szFileName, szFlags, _SH_DENYNO);

				if (pOutFile != NULL)
				{
					char szPrintMessage[MAX_PATH*2];
					fseek(pOutFile, 0L, SEEK_END);
					_snprintf_s(szPrintMessage, _countof(szPrintMessage), _TRUNCATE, "%s\n", pszMessage);
					fputs(szPrintMessage, pOutFile);            
					fflush(pOutFile);

					fclose(pOutFile);
				} // if
			} // if
		}
		__finally
		{
			::ReleaseMutex(m_hMutex);
		}
	}
	
private:
	//
	// Determines whether to use a log file management
	//
	BOOL             m_bTraceEnabled;
	//
	// Name of the log file
	//
	char             m_szFileName[MAX_PATH*2];
	//
	// Handle to the mutex used as guard
	//
	HANDLE           m_hMutex;
};

#endif // !defined(_LOGFILE_H_)

//--------------------- End of the file -------------------------------------
