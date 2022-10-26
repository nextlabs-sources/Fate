////////////////////////////////////////////////////////////////
// 1998 Microsoft Systems Journal
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
// CModuleVersion provides an easy way to get version info
// for a module.(DLL or EXE).
//
#include "StdAfx.h"
#include "ModulVer.h"
#include "Utilities.h"

ModuleVer::DllVersion::CModuleVersion::CModuleVersion()
{
	m_pVersionInfo = NULL;				// raw version info data 
}

//////////////////
// Destroy: delete version info
//
ModuleVer::DllVersion::CModuleVersion::~CModuleVersion()
{
	delete [] m_pVersionInfo;
}

//////////////////
// Get file version info for a given module
// Allocates storage for all info, fills "this" with
// VS_FIXEDFILEINFO, and sets codepage.
//
BOOL ModuleVer::DllVersion::CModuleVersion::GetFileVersionInfo(LPCTSTR modulename)
{
	m_translation.charset = 1252;		// default = ANSI code page
	memset((VS_FIXEDFILEINFO*)this, 0, sizeof(VS_FIXEDFILEINFO));

	// get module handle
	TCHAR filename[_MAX_PATH + 1] = {0};
	HMODULE hModule = ::GetModuleHandle(modulename);
	if (hModule==NULL && modulename!=NULL)
		return FALSE;

	// get module file name
	GetCurrentProcessName(filename, MAX_PATH, hModule);
	if (wcslen(filename) == 0)
		return FALSE;

	DWORD len = 0;
	// read file version info
	DWORD dwDummyHandle; // will always be set to zero
	len = GetFileVersionInfoSize(filename, &dwDummyHandle);
	if (len <= 0)
		return FALSE;

	m_pVersionInfo = new BYTE[len]; // allocate version info
	if (!::GetFileVersionInfo(filename, 0, len, m_pVersionInfo))
		return FALSE;

	LPVOID lpvi;
	UINT iLen;
	if (!VerQueryValue(m_pVersionInfo, TEXT("\\"), &lpvi, &iLen))
		return FALSE;

	// copy fixed info to myself, which am derived from VS_FIXEDFILEINFO
	*(VS_FIXEDFILEINFO*)this = *(VS_FIXEDFILEINFO*)lpvi;

	// Get translation info
	if (VerQueryValue(m_pVersionInfo,
		TEXT("\\VarFileInfo\\Translation"), &lpvi, &iLen) && iLen >= 4) {
		m_translation = *(TRANSLATION*)lpvi;
	}

	return dwSignature == VS_FFI_SIGNATURE;
}

//////////////////
// Get string file info.
// Key name is something like "CompanyName".
// returns the value as a tString.
//
VOID ModuleVer::DllVersion::CModuleVersion::GetLanguageValue( UINT& _iCodePage,UINT& _iLanguage ) 
{
		LPVOID pVal = NULL ;
		UINT Len = 0 ;
		VerQueryValue(m_pVersionInfo,TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&pVal, &Len);
		_iLanguage =*((unsigned short int *)pVal);
		_iCodePage =*((unsigned short int *) &((CHAR*)pVal)[2]);
}
std::wstring ModuleVer::DllVersion::CModuleVersion::GetValue(LPCTSTR lpKeyName)
{
	std::wstring sVal;
	if (m_pVersionInfo) {

		// To get a string value must pass query in the form
		//
		//    "\StringFileInfo\<langID><codepage>\keyname"
		//
		// where <lang-codepage> is the languageID concatenated with the
		// code page, in hex. Wow.
		//

		TCHAR query[MAX_PATH]={0};

#ifdef UNICODE
		
		_snwprintf_s(
			query, 
			MAX_PATH, _TRUNCATE,
			TEXT("\\StringFileInfo\\%04x%04x\\%s"),
			m_translation.langID,
			m_translation.charset,
			lpKeyName
			);
#else
		_snprintf_s(
			query, 
			MAX_PATH, _TRUNCATE,
			TEXT("\\StringFileInfo\\%04x%04x\\%s"),
			m_translation.langID,
			m_translation.charset,
			lpKeyName
			);
#endif

		LPCTSTR pVal;
		UINT iLenVal;
		if (VerQueryValue(m_pVersionInfo, (LPTSTR)(LPCTSTR)query,
			(LPVOID*)&pVal, &iLenVal)) {

				sVal = pVal;
			}
	}
	return sVal;
}

// typedef for DllGetVersion proc
typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(ModuleVer::DllVersion::DLLVERSIONINFO *);

/////////////////
// Get DLL Version by calling DLL's DllGetVersion proc
//
BOOL ModuleVer::DllVersion::CModuleVersion::DllGetVersion(LPCTSTR modulename, DLLVERSIONINFO& dvi)
{
	HINSTANCE hinst = LoadLibrary(modulename);
	if (!hinst)
		return FALSE;

	// Must use GetProcAddress because the DLL might not implement 
	// DllGetVersion. Depending upon the DLL, the lack of implementation of the 
	// function may be a version marker in itself.
	//
	DLLGETVERSIONPROC pDllGetVersion =
		(DLLGETVERSIONPROC)GetProcAddress(hinst, ("DllGetVersion"));

	if (!pDllGetVersion)
		return FALSE;

	memset(&dvi, 0, sizeof(dvi));			 // clear
	dvi.cbSize = sizeof(dvi);				 // set size for Windows

	return SUCCEEDED((*pDllGetVersion)(&dvi));
}
