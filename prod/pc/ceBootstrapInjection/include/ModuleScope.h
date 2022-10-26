/*
* ModuleScope.h 
* Author: Helen Friedland
* All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
* Redwood City CA, Ownership remains with Blue Jungle Inc, 
* All rights reserved worldwide. 
*/

#if !defined(_MODULESCOPE_H_)
#define _MODULESCOPE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <objbase.h>
#include "LockMgr.h"
#include "Detours.h"

#pragma warning(disable : 4244) /* Ignore warnings for conversion from '__w64 int' to 'long', possible loss of data */
#pragma warning(disable : 4267) /* Ignore warnings for conversion from 'size_t' to 'long', possible loss of data */
#pragma warning(disable : 4231) 
#pragma warning(disable : 4251)
#pragma warning(disable : 4996)

//---------------------------------------------------------------------------
//
// Global variables
// 
//---------------------------------------------------------------------------
//
// A global guard object used for protecting singelton's instantiation 
//
static CCSWrapper g_ModuleSingeltonLock;

//---------------------------------------------------------------------------
//
// class CModuleScope 
//
//---------------------------------------------------------------------------
class CModuleScope  
{
private:

    // Default constructor
    CModuleScope(void);

    // Avoid copyable object - no definitions will prevent link
    CModuleScope(const CModuleScope& rhs);
    CModuleScope& operator=(const CModuleScope& rhs);

public:
    //
    // Destructor - we must declare it as public in order to provide
    // enough visibility for the GetInstance().
    // However the destructor shouldn't be called directly by the 
    // Module's code.
    //
    virtual ~CModuleScope();
    //
    // Implements the "double-checking" locking pattern combined with 
    // Scott Meyers single instance
    // For more details see - 
    // 1. "Modern C++ Design" by Andrei Alexandrescu - 6.9 Living in a 
    //     Multithreaded World
    // 2. "More Effective C++" by Scott Meyers - Item 26
    //
    static CModuleScope* GetInstance(void);

    //
    // Accessor method
    //
    WCHAR* GetProcessName() const;
    //
    // Accessor method
    //
    DWORD GetProcessId() const;
    //
    // Called on DLL_PROCESS_ATTACH DLL notification
    //
    BOOL ManageModuleEnlistment();
    //
    // Called on DLL_PROCESS_DETACH notification
    //
    void ManageModuleDetachment();
    //
    // Activate/Deactivate hooking engine
    //
    BOOL InstallHookMethod( BOOL bActivate, 
			    BOOL bServer );

private:
    // the name of the process the loads this DLL
    WCHAR  m_szProcessName[MAX_PATH];
    
    // and its process id
    DWORD  m_dwProcessId;
   
    static HANDLE hookedAPIDetourMutex; //The mutex for hookedAPIDetours
    //In this process, hooked APIs and their detours
    static std::map<std::wstring, APIDetours*> hookedAPIDetours;

    //
    // Instance's pointer holder
    //
    static CModuleScope* sm_pInstance;

    //
    // Initialize hook engine
    //
    bool InitializeHookManagement(HMODULE injectLib, int numHookAPI, HookDetour *pHookDetours);

    //Get the address of original Win API
    PVOID *GetRealAPI(WCHAR *funcName);

    //Get the address of general detour function 
    PVOID GetGeneralHookedAPI(WCHAR *funcName);
   
    //Prototypes of General Detours of Win APIs
    static HANDLE WINAPI CE_BootstrapInjection_Detour_GetClipboardData (UINT uFormat);
    static HANDLE __stdcall CE_BootstrapInjection_Detour_CreateFileW(
								     LPCWSTR lpFileName,
								     DWORD dwDesiredAccess,
								     DWORD dwShareMode,
								     LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								     DWORD dwCreationDisposition,
								     DWORD dwFlagsAndAttributes,
								     HANDLE hTemplateFile);
    static HRESULT __stdcall CE_BootstrapInjection_Detour_CoCreateInstance(
									   REFCLSID rclsid,
									   LPUNKNOWN pUnkOuter,
									   DWORD dwClsContext,
									   REFIID riid,
									   LPVOID * ppv);
    static HANDLE WINAPI CE_BootstrapInjection_Detour_SetClipboardData(UINT uFormat, 
								       HANDLE hMem);
    
    /* CreateProcessW */
    static BOOL WINAPI CE_BootstrapInjection_Detour_CreateProcessW(LPCTSTR lpApplicationName,
								   LPTSTR lpCommandLine,
								   LPSECURITY_ATTRIBUTES lpProcessAttributes,
								   LPSECURITY_ATTRIBUTES lpThreadAttributes,
								   BOOL bInheritHandles,
								   DWORD dwCreationFlags,
								   LPVOID lpEnvironment,
								   LPCTSTR lpCurrentDirectory,
								   LPSTARTUPINFO lpStartupInfo,
								   LPPROCESS_INFORMATION lpProcessInformation );

};

#endif // !defined(_MODULESCOPE_H_)
