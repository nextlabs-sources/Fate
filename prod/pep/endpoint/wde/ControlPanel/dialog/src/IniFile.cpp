/*
 * IniFile.cpp 
 * Author: Helen Friedland
 * All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
 * Redwood City CA, Ownership remains with Blue Jungle Inc, 
 * All rights reserved worldwide. 
 */
#include "stdafx.h"
#include <windows.h>
#include "SysUtils.h"
#include "IniFile.h"

//---------------------------------------------------------------------------
//
// class CIniFile   
//
//---------------------------------------------------------------------------
CIniFile::CIniFile(const char* pszFileName)
{
	if(NULL != pszFileName)
	{
		strncpy_s(m_szFileName, MAX_PATH, pszFileName, _TRUNCATE);
	}
}

CIniFile::~CIniFile()
{

}

//
// Retrieve a string value from an INI file
//
void CIniFile::ReadString(
	const char* pszSection, 
	const char* pszIdent, 
	const char* pszDefault,
	char*       pszResult,
	int			nLen
	)
{
    DWORD dwResult = ::GetPrivateProfileStringA(
		pszSection,        // section name
		pszIdent,          // key name
		NULL,              // default string
		pszResult,         // destination buffer  
		nLen,          // size of destination buffer
		m_szFileName       // initialization file name
		);

	if (!dwResult)
		strncpy_s(pszResult, nLen, pszDefault, _TRUNCATE);
}

//
// Retrieve a boolean value from an INI file
//
BOOL CIniFile::ReadBool(
	const char* pszSection, 
	const char* pszIdent, 
	BOOL        bDefault
	)
{
	char    szResult[MAX_PATH];
	BOOL    bResult   = bDefault;
	char    szDefault[MAX_PATH];
		
	BoolToStr(bDefault, szDefault);

	ReadString(
		pszSection, 
		pszIdent,
		szDefault,
		szResult,
		MAX_PATH
		);

	bResult = StrToBool(szResult);

	return bResult;
}

//--------------------- End of the file -------------------------------------

