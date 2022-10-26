// ***************************************************************
//  madCHook.h                version: 3.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  API hooking & DLL injection
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-01-10 3.0.0 initial version of header file for madCodeHook 3.0.0

// ***************************************************************

#if !defined(MADCHOOK_H)
#define MADCHOOK_H

#ifdef __cplusplus
  extern "C" {
#endif
#define madCHookApi(rt) rt WINAPI

// ***************************************************************
// 64bit specific functions

// is the current OS a native 64bit OS?
madCHookApi(BOOL) Is64bitOS();

// is the specified process a native 64bit process?
madCHookApi(BOOL) Is64bitProcess(
  HANDLE  hProcess
);

// is the specified dll file a native 64bit dll?
madCHookApi(BOOL) Is64bitModule(
  LPCWSTR pFileName
);

// ***************************************************************

// make all memory maps available only to current user + admins + system
// without this option all memory maps are open for Everyone
// this option is not fully tested yet
// so use it only after extensive testing and on your own danger
// param: unsused
#define SECURE_MEMORY_MAPS (DWORD) 0x00000002

// before installing an API hook madCodeHook does some security checks
// one check is verifying whether the to be hooked code was already modified
// in this case madCodeHook does not tempt to modify the code another time
// otherwise there would be a danger to run into stability issues
// with protected/compressed modules there may be false alarms, though
// so you can turn this check off
// param: unsused
#define DISABLE_CHANGED_CODE_CHECK (DWORD) 0x00000003

// madCodeHook has two different IPC solutions built in
// in Vista and in all 64 bit OSs the "old" IPC solution doesn't work
// so in these OSs the new IPC solution is always used
// in all other OSs the old IPC solution is used by default
// the new solution is based on undocumented internal Windows IPC APIs
// the old solution is based on pipes and memory mapped files
// you can optionally enable the new IPC solution for the older OSs, too
// the new IPC solution doesn't work in win9x and so cannot be enabled there
// param: unused
#define USE_NEW_IPC_LOGIC (DWORD) 0x00000004

// when calling SendIpcMessage you can specify a timeout value
// this value only applies to how long madCodeHook waits for the reply
// there's an additional internal timeout value which specifies how long
// madCodeHook waits for the IPC message to be accepted by the queue owner
// the default value is 2000ms
// param: internal timeout value in ms
// example: SetMadCHookOption(SET_INTERNAL_IPC_TIMEOUT, (LPCWSTR) 5000);
#define SET_INTERNAL_IPC_TIMEOUT (DWORD) 0x00000005

// VMware: when disabling acceleration dll injection sometimes is delayed
// to work around this issue you can activate this special option
// it will result in a slightly modified dll injection logic
// as a side effect injection into DotNet applications may not work properly
// param: unused
#define VMWARE_INJECTION_MODE (DWORD) 0x00000006

// system wide dll injection normally injects the hook dll into both:
// (1) currently running processes
// (2) newly created processes
// this flag disables injection into already running processes
// param: unused
#define DONT_TOUCH_RUNNING_PROCESSES (DWORD) 0x00000007

// normally madCodeHook renews hooks only when they were removed
// hooks that were overwritten with some other code aren't renewed by default
// this behaviour allows other hooking libraries to co-exist with madCodeHook
// use this flag to force madCodeHook to always renew hooks
// this may result in other hooking libraries stopping to work correctly
// param: unused
#define RENEW_OVERWRITTEN_HOOKS (DWORD) 0x00000009

// system wide dll injection normally injects the hook dll into both:
// (1) currently running processes
// (2) newly created processes
// this option enabled/disables injection into already running processes
// param: bool
// default: true
#define INJECT_INTO_RUNNING_PROCESSES (DWORD) 0x0000000a

// system wide dll uninjection normally does two things:
// (1) uninjecting from all running processes
// (2) stop automatic injection into newly created processes
// this option controls if uninjection from running processes is performed
// param: bool
// default: true
#define UNINJECT_FROM_RUNNING_PROCESSES (DWORD) 0x0000000b

// with this API you can configure some aspects of madCodeHook
// available options see constants above
madCHookApi(BOOL) SetMadCHookOption(
  DWORD   option,
  LPCWSTR param
);

// ***************************************************************

// you don't need to install madCodeHook, but you can
// what purpose does this have?
// (1) in the NT family madCodeHook uses a little kernel mode injection driver
//     to inject your hook dlls into all newly created processes
//     if you install madCodeHook, this driver is officially added to the OS
//     list of installed drivers with the description "madCodeHook DLL
//     injection driver"
//     if you don't install madCodeHook, the driver is temporarily extracted
//     to harddisk, which might look "suspicious"
// (2) some protection software (e.g. DeepFreeze) doesn't allow dynamic loading
//     of drivers, so we have to officially and permanently install our driver
//     to make madCodeHook work together with such protection software
// each (successful) call to "InstallMadCHook" needs its own call to
// "UninstallMadCHook", since the installation count is reference counted
madCHookApi(BOOL)   InstallMadCHook ();
madCHookApi(BOOL) UninstallMadCHook ();

// ***************************************************************

// by default madCodeHook counts how many times any thread is currently
// running inside of your callback function
// this way unhooking can be safely synchronized to that counter
// sometimes you don't need/want this counting to happen, e.g.
// (1) if you don't plan to ever unhook, anyway
// (2) if the counting performance drop is too high for your taste
// (3) if you want to unhook from inside the hook callback function
// in those cases you can set the flag "NO_SAFE_UNHOOKING"
#define DONT_COUNT (DWORD) 0x00000001         // old name - kept for compatability
#define NO_SAFE_UNHOOKING (DWORD) 0x00000001  // new name

// with 2.1f the "safe unhooking" functionality (see above) was improved
// most probably there's no problem with the improvement
// but to be sure you can disable the improvement
// the improved safe unhooking is currently only available in the NT family
#define NO_IMPROVED_SAFE_UNHOOKING (DWORD) 0x00000040

// optionally madCodeHook can use a special technique to make sure that
// hooking in multi threaded situations won't result in crashing threads
// this technique is not tested too well right now, so it's optional for now
// you can turn this feature on by setting the flag "SAFE_HOOKING"
// without this technique crashes can happen, if a thread is calling the API
// which we want to hook in exactly the moment when the hook is installed
// safe hooking is currently only available in the NT family
#define SAFE_HOOKING (DWORD) 0x00000020

// madCodeHook implements two different API hooking methods
// the mixture mode is the second best method, it's only used if the main
// hooking method doesn't work for whatever reason (e.g. API code structure)
// normally madCodeHook chooses automatically which mode to use
// you can force madCodeHook to use the mixture mode by specifying this flag:
#define MIXTURE_MODE (DWORD) 0x00000002

// if you don't want madCodeHook to use the mixture mode, you can say so
// however, if the main hooking mode can't be used, hooking then simply fails
#define NO_MIXTURE_MODE (DWORD) 0x00000010

// winsock2 normally doesn't like the mixture mode
// however, I've found a way to convince winsock2 to accept mixture hooks
// this is a somewhat experimental feature, though
// so it must be turned on explicitly
#define ALLOW_WINSOCK2_MIXTURE_MODE (DWORD) 0x00000080

// under win9x you can hook code system wide, if it begins > $80000000
// or if the code section of the to-be-hooked dll is shared
// the callback function is in this case automatically copied to shared memory
// use only kernel32 APIs in such a system wide hook callback function (!!)
// if you want an easier way and/or a NT family compatible way to hook code
// system wide, please use InjectLibrary(ALL_SESSIONS) instead of these flags:
#define SYSTEM_WIDE_9X            (DWORD) 0x00000004
#define ACCEPT_UNKNOWN_TARGETS_9X (DWORD) 0x00000008

// hook any code or a specific API
madCHookApi(BOOL) HookCode(
  PVOID  pCode,
  PVOID  pCallbackFunc,
  PVOID  *pNextHook,
  #ifdef __cplusplus
    DWORD  dwFlags = 0
  #else
    DWORD  dwFlags
  #endif
);
madCHookApi(BOOL) HookAPI(
  LPCSTR pszModule,
  LPCSTR pszFuncName,
  PVOID  pCallbackFunc,
  PVOID  *pNextHook,
  #ifdef __cplusplus
    DWORD  dwFlags = 0
  #else
    DWORD  dwFlags
  #endif
);

// some firewall/antivirus programs kill our hooks, so we need to renew them
madCHookApi(BOOL) RenewHook(
  PVOID  *pNextHook
);

// is the hook callback function of the specified hook currently in use?
// 0: the hook callback function is not in use
// x: the hook callback function is in use x times
madCHookApi(DWORD) IsHookInUse(
  PVOID  *pNextHook
);

// unhook again
madCHookApi(BOOL) UnhookCode( PVOID  *pNextHook );
madCHookApi(BOOL) UnhookAPI ( PVOID  *pNextHook );

// putting all your "HookCode/API" calls into a "CollectHooks".."FlushHooks"
// frame can eventually speed up the installation of the hooks
madCHookApi(VOID) CollectHooks ();
madCHookApi(VOID)   FlushHooks ();

// restores the original code of the API/function (only first 6 bytes)
// the original code is read from the dll file on harddisk
// you can use this function e.g. to remove the hook of another hook library
// don't use this to uninstall your own hooks, use UnhookCode for that purpose
madCHookApi(BOOL) RestoreCode ( PVOID code );

// ***************************************************************
// same as CreateProcess
// additionally the dll "loadLibrary" is injected into the newly created process
// the dll is loaded right before the entry point of the exe module is called

madCHookApi(BOOL) CreateProcessExA(
  LPCSTR                lpApplicationName,
  LPSTR                 lpCommandLine,
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL                  bInheritHandles,
  DWORD                 dwCreationFlags,
  LPVOID                lpEnvironment,
  LPCSTR                lpCurrentDirectory,
  LPSTARTUPINFOA        lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation,
  LPCSTR                lpLoadLibrary
);
madCHookApi(BOOL) CreateProcessExW(
  LPCWSTR               lpApplicationName,
  LPWSTR                lpCommandLine,
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL                  bInheritHandles,
  DWORD                 dwCreationFlags,
  LPVOID                lpEnvironment,
  LPCWSTR               lpCurrentDirectory,
  LPSTARTUPINFOW        lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation,
  LPCWSTR               lpLoadLibrary
);

// ***************************************************************
// memory allocation in the specified processes (shared memory in win9x)
// if the processHandle is 0, the memory is allocated or freed in the shared
// area (in win9x) or in our own process (in winNT)

madCHookApi(PVOID) AllocMemEx(
  DWORD  dwSize,
  #ifdef __cplusplus
    HANDLE hProcess = 0
  #else
    HANDLE hProcess
  #endif
  #ifdef _WIN64
    ,
    #ifdef __cplusplus
      LPVOID pPreferredAddress = NULL
    #else
      LPVOID pPreferredAddress
    #endif
  #endif
);

madCHookApi(BOOL) FreeMemEx(
  PVOID  pMem,
  #ifdef __cplusplus
    HANDLE hProcess = 0
  #else
    HANDLE hProcess
  #endif
);

// ***************************************************************
// copy (and relocate) any function to a new location in any process
// if the processHandle is 0, the function is copied to shared area (in win9x)
// or to another memory location in our own process (in winNT)
// don't forget to free the function with FreeMemEx, if you don't it anymore

madCHookApi(PVOID) CopyFunction(
  PVOID  pFunction,
  #ifdef __cplusplus
    HANDLE hProcess              = 0,
    BOOL   bAcceptUnknownTargets = FALSE,
    PVOID  *pBuffer              = NULL
  #else
    HANDLE hProcess,
    BOOL   bAcceptUnknownTargets,
    PVOID  *pBuffer
  #endif
);

// ***************************************************************
// like CreateRemoteThread, but 3 changes:
// (1) this one also works perfectly in win9x!!
// (2) this one also works on other sessions in winNt
// (3) the DACL of the current thread is copied in winNt (if threadAttr = nil)

madCHookApi(HANDLE) madCreateRemoteThread(
  HANDLE                 hProcess,
  LPSECURITY_ATTRIBUTES  lpThreadAttributes,
  DWORD                  dwStackSize,
  LPTHREAD_START_ROUTINE lpStartAddress,
  LPVOID                 lpParameter,
  DWORD                  dwCreationFlags,
  LPDWORD                lpThreadId
);

// ***************************************************************

// this is how your remote function must look like
typedef DWORD (WINAPI *PREMOTE_EXECUTE_ROUTINE)( LPVOID pParams );

// executes the specified function in the context of another process
// this works only if the function follows some specific rules
// e.g. it must not use global variables, nor C++ private functions
// only win32 APIs are allowed
// if "dwSize" > 0, the "pParams" block will be copied to the other process
// after the remote function is finished, the "pParams" block is copied back
// so you can use the "pParams" block for both "in" and "out" parameters
// if "dwSize" = 0, the "pParams" value is just given into the remote function
madCHookApi(BOOL) RemoteExecute(
  HANDLE                  hProcess,
  PREMOTE_EXECUTE_ROUTINE pFunc,
  DWORD                   *dwFuncResult,
  #ifdef __cplusplus
    PVOID                   pParams = NULL,
    DWORD                   dwSize  = 0
  #else
    PVOID                   pParams,
    DWORD                   dwSize        
  #endif
);

// ***************************************************************

// flags for injection/uninjection "session" parameter
#define ALL_SESSIONS    (DWORD) -1
#define CURRENT_SESSION (DWORD) -2

// same as Load/FreeLibrary, but is able to load/free the library in any process
#ifdef __cplusplus
  madCHookApi(BOOL)   InjectLibraryA(LPCSTR  pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut = 7000);
  madCHookApi(BOOL)   InjectLibraryW(LPCWSTR pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut = 7000);
  madCHookApi(BOOL) UninjectLibraryA(LPCSTR  pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut = 7000);
  madCHookApi(BOOL) UninjectLibraryW(LPCWSTR pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut = 7000);
#else
  madCHookApi(BOOL)   InjectLibraryA(LPCSTR  pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut);
  madCHookApi(BOOL)   InjectLibraryW(LPCWSTR pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut);
  madCHookApi(BOOL) UninjectLibraryA(LPCSTR  pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut);
  madCHookApi(BOOL) UninjectLibraryW(LPCWSTR pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut);
#endif

// (un)injects a library to/from all processes of the specified session(s)
// driverName:      name of the driver to use
// libFileName:     full file path/name of the hook dll
// session:         session id into which you want the dll to be injected
//                  "-1" or "ALL_SESSIONS" means all sessions
//                  "-2" or "CURRENT_SESSION" means the current session
// systemProcesses: shall the dll be injected into system processes?
// includeMask:     list of exe file name/path masks into which the hook dll shall be injected
//                  you can use multiple masks separated by a "|" char
//                  you can either use a full path, or a file name, only
//                  leaving this parameter empty means that all processes are "included"
//                  Example: "c:\program files\*.exe|calc.exe|*.scr"
// excludeMask:     list of exe file name/path masks which shall be excluded from dll injection
//                  the excludeMask has priority over the includeMask
//                  leaving this parameter empty means that no processes are "excluded"
// excludePIDs:     list of process IDs which shall not be touched
//                  when used, the list must be terminated with a "0" PID item
#ifdef __cplusplus
  }
  madCHookApi(BOOL) InjectLibraryA(
    LPCSTR  pDriverName, 
    LPCSTR  pLibFileName,
    DWORD   dwSession        = ALL_SESSIONS,
    BOOL    bSystemProcesses = TRUE,
    LPCSTR  pIncludeMask     = NULL,
    LPCSTR  pExcludeMask     = NULL,
    PULONG  pExcludePIDs     = NULL,
    DWORD   dwTimeOut        = 7000
  );
  madCHookApi(BOOL) InjectLibraryW(
    LPCWSTR pDriverName, 
    LPCWSTR pLibFileName,
    DWORD   dwSession        = ALL_SESSIONS,
    BOOL    bSystemProcesses = TRUE,
    LPCWSTR pIncludeMask     = NULL,
    LPCWSTR pExcludeMask     = NULL,
    PULONG  pExcludePIDs     = NULL,
    DWORD   dwTimeOut        = 7000
  );
  madCHookApi(BOOL) UninjectLibraryA(
    LPCSTR  pDriverName, 
    LPCSTR  pLibFileName,
    DWORD   dwSession        = ALL_SESSIONS,
    BOOL    bSystemProcesses = TRUE,
    LPCSTR  pIncludeMask     = NULL,
    LPCSTR  pExcludeMask     = NULL,
    PULONG  pExcludePIDs     = NULL,
    DWORD   dwTimeOut        = 7000
  );
  madCHookApi(BOOL) UninjectLibraryW(
    LPCWSTR pDriverName, 
    LPCWSTR pLibFileName,
    DWORD   dwSession        = ALL_SESSIONS,
    BOOL    bSystemProcesses = TRUE,
    LPCWSTR pIncludeMask     = NULL,
    LPCWSTR pExcludeMask     = NULL,
    PULONG  pExcludePIDs     = NULL,
    DWORD   dwTimeOut        = 7000
  );
  extern "C" {
#endif

madCHookApi(BOOL)   InjectLibrarySystemWideA(LPCSTR  pDriverName, LPCSTR  pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCSTR  pIncludeMask, LPCSTR  pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut);
madCHookApi(BOOL)   InjectLibrarySystemWideW(LPCWSTR pDriverName, LPCWSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCWSTR pIncludeMask, LPCWSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut);
madCHookApi(BOOL) UninjectLibrarySystemWideA(LPCSTR  pDriverName, LPCSTR  pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCSTR  pIncludeMask, LPCSTR  pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut);
madCHookApi(BOOL) UninjectLibrarySystemWideW(LPCWSTR pDriverName, LPCWSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCWSTR pIncludeMask, LPCWSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut);

// ***************************************************************

// you can use these APIs to permanently (un)install your injection driver
// it will survive reboots, it will stay installed until you uninstall it
madCHookApi(BOOL) InstallInjectionDriver(
  LPCWSTR  pDriverName,
  LPCWSTR  pFileName32bit,
  LPCWSTR  pFileName64bit,
  LPCWSTR  pDescription
);
madCHookApi(BOOL) UninstallInjectionDriver(
  LPCWSTR  pDriverName
);

// this will load and activate your injection driver
// it will stay active only until the next reboot
// you may prefer this technique if you want your product to not require any
// installation and uninstallation procedures
madCHookApi(BOOL) LoadInjectionDriver(
  LPCWSTR  pDriverName,
  LPCWSTR  pFileName32bit,
  LPCWSTR  pFileName64bit
);

// stopping the injection driver will only work:
// (1) if the driver was configured at build time to be stoppable
// (2) if you call StopInjectionDriver
// (3) if there are no dll injections currently running
// stopping the injection driver with the device manager or "sc" is blocked
// call Stop + Start to update an installed driver to a newer version
// call Stop + Load  to update a  loaded    driver to a newer version
madCHookApi(BOOL) StopInjectionDriver(
  LPCWSTR  pDriverName
);
madCHookApi(BOOL) StartInjectionDriver(
  LPCWSTR  pDriverName
);

// ***************************************************************

// which processId belongs to the specified process handle?
// undocumented function, works in all windows 32 bit systems
madCHookApi(DWORD) ProcessHandleToId(
  HANDLE hProcessHandle
);

// which threadId belongs to the specified thread handle?
// undocumented function, works in all windows 32 bit systems
madCHookApi(DWORD) ThreadHandleToId(
  HANDLE hThreadHandle
);

// find out which file the specified process was executed from
madCHookApi(BOOL) ProcessIdToFileNameA(
  DWORD  dwProcessId,
  LPSTR  pFileName,
  USHORT bufLenInChars
);
madCHookApi(BOOL) ProcessIdToFileNameW(
  DWORD  dwProcessId,
  LPWSTR pFileName,
  USHORT bufLenInChars
);

// ***************************************************************

// is the current process a service/system process?  (win9x -> always false)
madCHookApi(BOOL) AmSystemProcess (VOID);

// is the current thread's desktop the input desktop?  (win9x -> always true)
// only in that case you should show messages boxes or other GUI stuff
// but please note that in XP fast user switching AmUsingInputDesktop may
// return true, although the current session is currently not visible
// XP fast user switching is implemented by using terminal server logic
// so each fast user session has its own window station and input desktop
madCHookApi(BOOL) AmUsingInputDesktop (VOID);

// the following two functions can be used to get the session id of the
// current session and of the input session
// each terminal server (or XP fast user switching) session has its own id
// the "input session" is the one currently shown on the physical screen
madCHookApi(DWORD) GetCurrentSessionId (VOID);
madCHookApi(DWORD) GetInputSessionId   (VOID);

// ***************************************************************

// which module called me? works only if your function has a stack frame
// MSVC++ users can turn forced stackframe on/off by using:
// #pragma optimize( "y", off/on )
madCHookApi(HMODULE) GetCallingModule (VOID);

// ***************************************************************
// global  =  normal  +  "access for everyone"  +  "non session specific"

madCHookApi(HANDLE) CreateGlobalMutex(
  LPCSTR  pName
);
madCHookApi(HANDLE) OpenGlobalMutex(
  LPCSTR  pName
);
madCHookApi(HANDLE) CreateGlobalEvent(
  LPCSTR  pName,
  BOOL    bManual,
  BOOL    bInitialState
);
madCHookApi(HANDLE) OpenGlobalEvent(
  LPCSTR  pName
);
madCHookApi(HANDLE) CreateGlobalFileMapping(
  LPCSTR  pName,
  DWORD   dwSize
);
madCHookApi(HANDLE) OpenGlobalFileMapping(
  LPCSTR  pName,
  BOOL    bWrite
);

// ***************************************************************

// convert strings ansi <-> wide
// the result buffer must have a size of MAX_PATH characters (or more)
// please use these functions in nt wide API hook callback functions
// because the OS' own functions seem to confuse nt in hook callback functions
madCHookApi(VOID) AnsiToWide(
  LPCSTR  pAnsi,
  LPWSTR  pWide
);
madCHookApi(VOID) WideToAnsi(
  LPCWSTR pWide,
  LPSTR   pAnsi
);

// ***************************************************************
// ipc (inter process communication) message services
// in contrast to SendMessage the following functions don't crash NT services

// this is how you get notified about incoming ipc messages
// you have to write a function which fits to this type definition
// and then you give it into "CreateIpcQueue"
// your callback function will then be called for each incoming message
// CAUTION: each ipc message is handled by a seperate thread, as a result
//          your callback will be called by a different thread each time
typedef VOID (WINAPI *PIPC_CALLBACK_ROUTINE)(
  LPCSTR  pIpc,
  LPCVOID pMessageBuf,
  DWORD   dwMessageLen,
  LPVOID  pAnswerBuf,
  DWORD   dwAnswerLen
);

// create an ipc queue
// please choose a unique ipc name to avoid conflicts with other programs
// only one ipc queue with the same name can be open at the same time
// so if 2 programs try to create the same ipc queue, the second call will fail
// you can specify how many threads may be created to handle incoming messages
// if the order of the messages is crucial for you, set "maxThreadCount" to "1"
// in its current implementation "maxThreadCount" only supports "1" or unlimited
// the parameter "maxQueueLen" is not yet implemented at all
madCHookApi(BOOL) CreateIpcQueueEx(
  LPCSTR                pIpc,
  PIPC_CALLBACK_ROUTINE pCallback,
  #ifdef __cplusplus
    DWORD                 dwMaxThreadCount = 16,
    DWORD                 dwMaxQueueLen    = 0x1000
  #else
    DWORD                 dwMaxThreadCount,
    DWORD                 dwMaxQueueLen
  #endif
);
madCHookApi(BOOL) CreateIpcQueue(
  LPCSTR                pIpc,
  PIPC_CALLBACK_ROUTINE pCallback
);

// send an ipc message to whomever has created the ipc queue (doesn't matter)
// if you only fill the first 3 parameters, SendIpcMessage returns at once
// if you fill the next two parameters, too, SendIpcMessage will
// wait for an answer of the ipc queue owner
// you can further specify how long you're willing to wait for the answer
// and whether you want SendIpcMessage to handle messages while waiting
madCHookApi(BOOL) SendIpcMessage(
  LPCSTR  pIpc,
  PVOID   pMessageBuf,
  DWORD   dwMessageLen,
  #ifdef __cplusplus
    PVOID   pAnswerBuf      = NULL,
    DWORD   dwAnswerLen     = 0,
    DWORD   dwAnswerTimeOut = INFINITE,
    BOOL    bHandleMessage  = TRUE
  #else
    PVOID   pAnswerBuf,
    DWORD   dwAnswerLen,
    DWORD   dwAnswerTimeOut,
    BOOL    bHandleMessage
  #endif
);

// destroy the ipc queue again
// when the queue owning process quits, the ipc queue is automatically deleted
// only the queue owning process can destroy the queue
madCHookApi(BOOL) DestroyIpcQueue(
  LPCSTR  pIpc
);

// ***************************************************************
// this function adds some access rights to the specified target
// the target can either be a process handle or a service handle

madCHookApi(BOOL) AddAccessForEveryone(
  HANDLE  hProcessOrService,
  DWORD   dwAccess
);

// ***************************************************************
// please in your initialization call "InitializeMadCHook"
// and in your finalization please call "FinalizeMadCHook"
// do *not* call InitializeMadCHook again after having called FinalizeMadCHook!

#define InitializeMadCHook() StaticLibHelper_Init((PVOID) &MADCHOOK_DllMain)
#define FinalizeMadCHook() StaticLibHelper_Final((PVOID) &MADCHOOK_DllMain)

// ***************************************************************

#ifdef UNICODE
  #define CreateProcessEx CreateProcessExW
  #define   InjectLibrary   InjectLibraryW
  #define UninjectLibrary UninjectLibraryW
  #define ProcessIdToFileName ProcessIdToFileNameW
#else
  #define CreateProcessEx CreateProcessExA
  #define   InjectLibrary   InjectLibraryA
  #define UninjectLibrary UninjectLibraryA
  #define ProcessIdToFileName ProcessIdToFileNameA
#endif

// ***************************************************************
// internal stuff, please ignore

BOOL WINAPI MADCHOOK_DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
madCHookApi(VOID) StaticLibHelper_Init (PVOID dllMain);
madCHookApi(VOID) StaticLibHelper_Final(PVOID dllMain);
#ifdef __cplusplus
  }
  inline madCHookApi(BOOL) InjectLibraryA(LPCSTR pDriverName, LPCSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCSTR pIncludeMask, LPCSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut)
  { return InjectLibrarySystemWideA(pDriverName, pLibFileName, dwSession, bSystemProcesses, pIncludeMask, pExcludeMask, pExcludePIDs, dwTimeOut); }
  inline madCHookApi(BOOL) InjectLibraryW(LPCWSTR pDriverName, LPCWSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCWSTR pIncludeMask, LPCWSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut)
  { return InjectLibrarySystemWideW(pDriverName, pLibFileName, dwSession, bSystemProcesses, pIncludeMask, pExcludeMask, pExcludePIDs, dwTimeOut); }
  inline madCHookApi(BOOL) UninjectLibraryA(LPCSTR pDriverName, LPCSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCSTR pIncludeMask, LPCSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut)
  { return UninjectLibrarySystemWideA(pDriverName, pLibFileName, dwSession, bSystemProcesses, pIncludeMask, pExcludeMask, pExcludePIDs, dwTimeOut); }
  inline madCHookApi(BOOL) UninjectLibraryW(LPCWSTR pDriverName, LPCWSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCWSTR pIncludeMask, LPCWSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut)
  { return UninjectLibrarySystemWideW(pDriverName, pLibFileName, dwSession, bSystemProcesses, pIncludeMask, pExcludeMask, pExcludePIDs, dwTimeOut); }
#endif

#endif
