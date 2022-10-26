#ifndef __FILTER_RES_H__
#define __FILTER_RES_H__	   
/*
Implement the interface WSPStartup(...)
*/
#include "Utilities.h"
namespace FILTER
{
	typedef struct _tagPROCESSINFO
	{
		wchar_t *pProcName ;
		UINT	iProcID ;
	}  *PPROCESSINFO ,PROCESSINFO ;
};

class CFilterRes
{
public:
	CFilterRes(){} ;
	~CFilterRes(){} ;
public:
	static BOOL IsSupportedProcess()  ;
	static DWORD GetCurrentMudleFileName( wchar_t* pszFileName)
	{
		DWORD dRet = 0 ;
		if(	 m_hInst )
		{
			wchar_t szTemp[MAX_PATH+1] = {0} ;
			
			GetCurrentProcessName(szTemp, MAX_PATH, m_hInst);
			if( wcslen(szTemp) == 0 )
			{
				return dRet ;
			}
			LPCWSTR fileName = wcsrchr( szTemp, L'\\' ) ;
			if( fileName )
			{
				fileName = fileName+1 ;
				wcsncpy_s( pszFileName, MAX_PATH, fileName, _TRUNCATE) ;
				dRet = (DWORD)wcslen( fileName ) ;
			}

		}
		return dRet ;
	};
	static DWORD GetCurrentFilePath(  std::wstring &strFilePath ) 
	{
		DWORD dRet = 0 ;
		if(	 m_hInst )
		{
			wchar_t szTemp[MAX_PATH+1] = {0} ;
			
			GetCurrentProcessName(szTemp, MAX_PATH, m_hInst);
			if( wcslen(szTemp) == 0 )
			{
				return dRet ;
			}
			LPCWSTR fileName = wcsrchr( szTemp, L'\\' ) ;
			if( fileName )
			{		
				fileName = fileName+1 ;
				::ZeroMemory( szTemp+(wcslen( szTemp) -wcslen( fileName )),wcslen( fileName ) ) ;
			}
			strFilePath =	szTemp ;

		}
		return dRet ;
	}
	static VOID SetCurrentInstance(HMODULE hInstance) 
	{
		if(hInstance)
		{
		   m_hInst = hInstance ;
		}

	};
private:
	static HMODULE m_hInst ;
};
#endif
