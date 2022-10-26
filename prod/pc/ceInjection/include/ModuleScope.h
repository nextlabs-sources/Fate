#if !defined(_MODULESCOPE_H_)
#define _MODULESCOPE_H_

#include <vector>
#include <windows.h>
#include <objbase.h>
#include "Detours.h"

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
    ~CModuleScope();

    static CModuleScope* GetInstance(void);

    // Called on DLL_PROCESS_ATTACH DLL notification
    BOOL ManageModuleEnlistment(void);

    // Activate/Deactivate hooking engine
    BOOL InstallHookMethod( BOOL bActivate, 
			    BOOL bServer );

    // Initialize hook engine
    void InitializeHookManagement( std::vector<HookDetour*>* hlist );

private:
    static HANDLE hookedAPIDetourMutex; //The mutex for hookedAPIDetours
    //In this process, hooked APIs and their detours
    static std::map<std::string, APIDetours*> hookedAPIDetours;

    // Instance's pointer holder
    static CModuleScope* sm_pInstance;

    //Get the address of original Win API
    PVOID *GetRealAPI(const char *funcName);

    //Get the address of general detour function 
    PVOID GetGeneralHookedAPI(const char *funcName);
   
	std::wstring GetConfigFilePath(WCHAR* pDllPath);

	BOOL isHookAll(const std::wstring& wstrConfigFilePath);

	std::wstring GetConfigInfo(BOOL bIsHookAll, const std::wstring& wstrConfigFilePath);

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
