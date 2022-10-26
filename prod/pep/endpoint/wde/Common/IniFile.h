/*
 * IniFile.h 
 * Author: Helen Friedland
 * All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
 * Redwood City CA, Ownership remains with Blue Jungle Inc, 
 * All rights reserved worldwide. 
 */


#if !defined(_INIFILE_H_)
#define _INIFILE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//---------------------------------------------------------------------------
//
// class CIniFile   
//
//---------------------------------------------------------------------------
class CIniFile  
{
public:
	CIniFile(char* pszFileName);
	virtual ~CIniFile();
	//
	// Retrieve a string value from an INI file
	//
	void ReadString(
		const char* pszSection, 
		const char* pszIdent, 
		const char* pszDefault,
		char*       pszResult
		); 
	//
	// Retrieve a boolean value from an INI file
	//
	BOOL ReadBool(
		const char* pszSection, 
		const char* pszIdent, 
		BOOL        bDefault
		); 
private:
	char m_szFileName[MAX_PATH];
};

#endif // !defined(_INIFILE_H_)

//--------------------- End of the file -------------------------------------
