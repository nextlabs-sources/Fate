//---------------------------------------------------------------------------
// Created on Nov 2, 2004 All sources, binaries and HTML pages (C) copyright
// 2004 by Blue Jungle Inc., Redwood City CA, Ownership remains with Blue Jungle
// Inc, All rights reserved worldwide.
//
// ApplicationScope.h
//
// DESCRIPTION: Declaration of the CApplicationScope class.
//              This class is designed to provide single interface for 
//              all hook related activities.
//
// AUTHOR:		Helen Friedland
// DATE:		December 7, 2004
//
//---------------------------------------------------------------------------


#if !defined(_APPLICATIONSCOPE_H_)
#define _APPLICATIONSCOPE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

//---------------------------------------------------------------------------
//
// Prototype of the main hook function
//
//---------------------------------------------------------------------------
typedef BOOL (WINAPI *PFN_INSTALLHOOK)(
	BOOL bActivate, 
	/*HWND hWndServer, */
    BOOL bServer
	);


//---------------------------------------------------------------------------
//
// class CApplicationScope 
//
//---------------------------------------------------------------------------
class CApplicationScope  
{
private:
	//
	// Intentionally hide the defualt constructor,
	// copy constructor and assignment operator 
	//

	//
	// Default constructor
	//
	CApplicationScope();
	//
	// Copy constructor
	//
	CApplicationScope(const CApplicationScope& rhs);
	//
	// Assignment operator
	//
	CApplicationScope& operator=(const CApplicationScope& rhs);
public:
	//
	// Destructor - we must declare it as public in order to provide
	// enough visibility for the GetInstance().
	// However the destructor shouldn't be called directly by the 
	// application's code.
	//
	virtual ~CApplicationScope();
	//
	// Implements the "double-checking" locking pattern combined with 
	// Scott Meyers single instance
	// For more details see - 
	// 1. "Modern C++ Design" by Andrei Alexandrescu - 6.9 Living in a 
	//     Multithreaded World
	// 2. "More Effective C++" by Scott Meyers - Item 26
	//
	static CApplicationScope& GetInstance();
	//
	// Delegates the call to the DLL InstallHook function
	//
	void InstallHook(BOOL bActivate, /*HWND hwndServer,*/ BOOL bServer);

private:
	//Get DE installation directory
	BOOL GetDEInstallDir ( TCHAR (&InstallDir)[MAX_PATH] );

private:
	//
	// Instance's pointer holder
	//
	static CApplicationScope* sm_pInstance;
	//
	// HookTool handle
	//
	HMODULE m_hmodHookTool;
	//
	//
	//
	PFN_INSTALLHOOK m_pfnInstallHook;
};

#endif // !defined(_APPLICATIONSCOPE_H_)

//----------------------------End of the file -------------------------------
