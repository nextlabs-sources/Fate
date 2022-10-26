//---------------------------------------------------------------------------
// Created on Nov 2, 2004 All sources, binaries and HTML pages (C) copyright
// 2004 by Blue Jungle Inc., Redwood City CA, Ownership remains with Blue Jungle
// Inc, All rights reserved worldwide.
//
// ApplicationScope.cpp
//
// DESCRIPTION: Implementation of the CApplicationScope class.
//              This class is designed to provide single interface for 
//              all hook related activities.
//
// AUTHOR:		Helen Friedland
// DATE:		December 7, 2004
//
//---------------------------------------------------------------------------

#include <tchar.h>
#include "LockMgr.h"
#include "ApplicationScope.h"
#include <eframework/platform/policy_controller.hpp>

//---------------------------------------------------------------------------
//
// Global variables
// 
//---------------------------------------------------------------------------
//
// A global guard object used for protecting singelton's instantiation 
//
static CCSWrapper g_AppSingeltonLock;

//---------------------------------------------------------------------------
//
// class CApplicationScope 
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//
// Static memeber declarations
//
//---------------------------------------------------------------------------
CApplicationScope* CApplicationScope::sm_pInstance = NULL;

//---------------------------------------------------------------------------
//
// Constructor
//
//---------------------------------------------------------------------------
CApplicationScope::CApplicationScope():
	m_hmodHookTool(NULL),
	m_pfnInstallHook(NULL)
{

}

//---------------------------------------------------------------------------
//
// Destructor 
//
//---------------------------------------------------------------------------
CApplicationScope::~CApplicationScope()
{
	if (m_hmodHookTool)
		::FreeLibrary( m_hmodHookTool );
}

//---------------------------------------------------------------------------
//
// Copy constructor
//
//---------------------------------------------------------------------------
CApplicationScope::CApplicationScope(const CApplicationScope& rhs)
{

}

//---------------------------------------------------------------------------
//
// Assignment operator
//
//---------------------------------------------------------------------------
CApplicationScope& CApplicationScope::operator=(const CApplicationScope& rhs)
{
	if (this == &rhs) 
		return *this;

	return *this; // return reference to left-hand object
}


//---------------------------------------------------------------------------
// GetInstance
//
// Implements the "double-checking" locking pattern combined with 
// Scott Meyers single instance
// For more details see - 
// 1. "Modern C++ Design" by Andrei Alexandrescu - 6.9 Living in a 
//     Multithreaded World
// 2. "More Effective C++" by Scott Meyers - Item 26
//---------------------------------------------------------------------------
CApplicationScope& CApplicationScope::GetInstance()
{
	if (!sm_pInstance)
	{
		CLockMgr<CCSWrapper> guard(g_AppSingeltonLock, TRUE);
		if (!sm_pInstance)
		{
			static CApplicationScope instance;
			sm_pInstance = &instance;
		}
	} // if

	return *sm_pInstance;
}

//---------------------------------------------------------------------------
// InstallHook
//
// Delegates the call to the DLL InstallHook function
//---------------------------------------------------------------------------
void CApplicationScope::InstallHook(BOOL bActivate, /*HWND hwndServer,*/ BOOL bServer)
{
    if (NULL == m_hmodHookTool)
    {
      TCHAR buffer[MAX_PATH];
      buffer[0] = _T('\0');

	  if ( !GetDEInstallDir ( buffer ) )
	  {
		  _tprintf(_T("Cann't get DE installation directory\n"));
		  return;
	  }
		
#if defined(_WIN64)
	  wcsncat_s ( buffer, MAX_PATH, _T("bin\\basepep.Dll"), _TRUNCATE );
#else
	  wcsncat_s ( buffer, MAX_PATH, _T("bin\\basepep32.Dll"), _TRUNCATE );
#endif
		

        m_hmodHookTool = ::LoadLibrary(buffer);

        if (NULL != m_hmodHookTool)
        {
            _tprintf(_T("*** %s loaded ***\n"), buffer);
            m_pfnInstallHook = (PFN_INSTALLHOOK)GetProcAddress(m_hmodHookTool, 
							       "InstallHook"
							       );
        }
        else
        {
            _tprintf(_T("*** %s not loaded ***\n"), buffer);

        }
    } // if
    else
    {
      _tprintf(_T("Hook dll is not null...\n"));
    }
    if (m_pfnInstallHook)
        m_pfnInstallHook(bActivate, /*hwndServer,*/ bServer);
}

//Get DE installation directory
BOOL CApplicationScope::GetDEInstallDir ( TCHAR (&InstallDir)[MAX_PATH] )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		return FALSE;
	}

	DWORD BufferSize = sizeof InstallDir;

	if ( ERROR_SUCCESS != RegQueryValueEx ( hKey, _T("InstallDir"), NULL, NULL, (LPBYTE)InstallDir, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		return FALSE;
	}

	RegCloseKey ( hKey );
	return TRUE;
}
//----------------------------End of the file -------------------------------
