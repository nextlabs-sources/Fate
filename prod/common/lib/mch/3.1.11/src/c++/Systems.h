// ***************************************************************
//  Systems.h                 version: 1.0.1  ·  date: 2011-03-26
//  -------------------------------------------------------------
//  defines all exports
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2011 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2011-03-26 1.0.1 (1) added IsInjectionDriverInstalled/Running
//                  (2) added SetInjectionMethod API
//                  (3) added UninjectAllLibrariesA/W API
//                  (4) added IsWine API
// 2010-01-10 1.0.0 initial version

#define _CRT_SECURE_NO_DEPRECATE

#ifndef _SYSTEMS_H
#define _SYSTEMS_H

#ifndef _INC_WINDOWS
  #include <windows.h>
  #include <tchar.h>
#endif

// For building/importing the dynamic library version
// #define SYSTEMS_DLL

#ifndef SYSTEMS_API
  #ifdef _SYSTEMS_DLL
    #ifdef _BUILDING_SYSTEMS
      #define SYSTEMS_API __declspec ( dllexport )
    #else
      #define SYSTEMS_API __declspec ( dllimport )
    #endif
  #else
    #define SYSTEMS_API
    #define SYSTEMS_API2 extern "C"
  #endif
#endif

// -------------------- Variables shared with library consumers ------------------- 

#undef EXTERN
#ifdef _DLLMAIN_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN HMODULE gHModule
#ifdef _DLLMAIN_C
= NULL
#endif
;

// --------------------------------------------------------------------------------

// Exported classes
#include "CEnumProcesses.h"   // depends on CCollection
#include "CMemoryMap.h"       // depends on CCollection

// -------------------- Internals Types---------------------------------------------

#include "ProcessTypes.h"
#include "HookingTypes.h"

// -------------------- Initialization ---------------------------------------------

SYSTEMS_API2 BOOL WINAPI MADCHOOK_DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
SYSTEMS_API2 VOID WINAPI StaticLibHelper_Init (PVOID dllMain);
SYSTEMS_API2 VOID WINAPI StaticLibHelper_Final(PVOID dllMain);
SYSTEMS_API BOOL WINAPI VersionSupported(void);
SYSTEMS_API void WINAPI EnableAllPrivileges(void);

// -------------------- String Tools -----------------------------------------------

SYSTEMS_API2 void WINAPI AnsiToWide(LPCSTR pAnsi, LPWSTR pWide);
SYSTEMS_API2 void WINAPI WideToAnsi(LPCWSTR pWide, LPSTR pAnsi);

// -------------------- Atomic Move ------------------------------------------------

SYSTEMS_API BOOL WINAPI AtomicMove(LPVOID source, LPVOID destination, int length);

// -------------------- Stub Tools -------------------------------------------------

// For relative jmp, the assembler is expecting a label.
// We simulate the label by using '$', the current location counter
// and add 5 to get passed the jmp opcode and 4 byte address, and
// then add the token, which will be replaced by an actual memory address.
#define RELATIVE_OFFSET_TOKEN(a) $+5+a

#define TOKEN1 0x11111111
#define TOKEN2 0x22222222
#define TOKEN3 0x33333333

SYSTEMS_API BOOL WINAPI ReplaceDword(LPVOID buffer, DWORD bufferSize, DWORD token, DWORD value);
SYSTEMS_API BOOL WINAPI ReplaceQword(LPVOID buffer, DWORD bufferSize, ULONG_PTR token, ULONG_PTR value);
SYSTEMS_API BOOL WINAPI ReplaceRelativeOffset(LPVOID buffer, DWORD bufferSize, DWORD token, LPVOID address);

// -------------------- Windows Object Tools ---------------------------------------

SYSTEMS_API2 HANDLE WINAPI CreateGlobalMutex(LPCSTR name);
SYSTEMS_API2 HANDLE WINAPI OpenGlobalMutex(LPCSTR name);
SYSTEMS_API HANDLE WINAPI CreateLocalMutex(LPCSTR name);
SYSTEMS_API HANDLE WINAPI InternalCreateFileMapping(LPCSTR name, DWORD size, BOOL global);
SYSTEMS_API2 HANDLE WINAPI CreateGlobalFileMapping(LPCSTR name, DWORD size);
SYSTEMS_API2 HANDLE WINAPI OpenGlobalFileMapping(LPCSTR name, BOOL write);
SYSTEMS_API HANDLE WINAPI CreateLocalFileMapping(LPCSTR name, DWORD size);
SYSTEMS_API HANDLE WINAPI OpenLocalFileMapping(LPCSTR name, BOOL write);
SYSTEMS_API2 HANDLE WINAPI CreateGlobalEvent(LPCSTR name, BOOL manual, BOOL initialState);
SYSTEMS_API2 HANDLE WINAPI OpenGlobalEvent(LPCSTR name);

// -------------------- Module Tools -------------------------------------------------

SYSTEMS_API2 BOOL    WINAPI Is64bitModule(LPCWSTR FileName);
SYSTEMS_API LPVOID  WINAPI GetImageProcAddress(HMODULE hModule, LPCSTR name, BOOL doubleCheck = FALSE);
SYSTEMS_API BOOL    WINAPI GetImageProcAddressesRaw(LPCWSTR ModuleFileName, PVOID ImageBaseAddress, LPCSTR *ApiNames, PVOID *ProcAddresses, int ApiCount);
SYSTEMS_API LPVOID  WINAPI GetImageProcAddress(HMODULE hModule, int index);
SYSTEMS_API BOOL    WINAPI GetImageProcName(HMODULE hModule, LPVOID proc, LPSTR apiName, DWORD procNameSize);
SYSTEMS_API2 HMODULE WINAPI GetCallingModule(PVOID pReturnAddress = NULL);
SYSTEMS_API BOOL    WINAPI FindModule(LPVOID pCode, HMODULE *hModule, LPSTR moduleName, DWORD moduleNameSize);
SYSTEMS_API ULONG   WINAPI GetSizeOfImage(PIMAGE_NT_HEADERS pNtHeaders);
SYSTEMS_API BOOL    WINAPI WasCodeChanged(HMODULE hModule, LPVOID pCode, ULONGLONG *pOrgCode);
SYSTEMS_API LPVOID* WINAPI FindWs2InternalProcList(HMODULE hModule);
SYSTEMS_API PVOID   WINAPI NeedModuleFileMap(HMODULE hModule);

SYSTEMS_API USHORT                   WINAPI GetModuleLoadCount(HMODULE hModule);
SYSTEMS_API IMAGE_NT_HEADERS*        WINAPI GetImageNtHeaders(HMODULE hModule);
SYSTEMS_API LPVOID                   WINAPI ExportToFunc(HMODULE hModule, DWORD addr);
SYSTEMS_API DWORD                    WINAPI VirtualToRaw(IMAGE_NT_HEADERS *pNtHeaders, DWORD addr);
SYSTEMS_API LPVOID                   WINAPI GetImageDataDirectory(HMODULE hModule, DWORD directory);
SYSTEMS_API IMAGE_IMPORT_DESCRIPTOR* WINAPI GetImageImportDirectory(HMODULE hModule);
SYSTEMS_API IMAGE_EXPORT_DIRECTORY*  WINAPI GetImageExportDirectory(HMODULE hModule);
SYSTEMS_API BOOL                     WINAPI GetExportDirectory(LPVOID pCode, HMODULE *phModule, IMAGE_EXPORT_DIRECTORY **pImageExportDirectory);

SYSTEMS_API LPCSTR  WINAPI DecryptStr(LPCSTR string, LPSTR buffer, int bufferLength);
SYSTEMS_API HMODULE WINAPI GetKernel32Handle(void);
SYSTEMS_API HMODULE WINAPI GetNtDllHandle(void);
SYSTEMS_API LPVOID  WINAPI KernelProc(LPCSTR name, BOOL doubleCheck = false);
SYSTEMS_API LPVOID  WINAPI NtProc(LPCSTR name, BOOL doubleCheck = false);
SYSTEMS_API void    WINAPI InitKernelProcs(void);

// -------------------- Disassembly types and classes ---------------------------------

#include "Opcodes.h"
#include "CCodeParse.h"      // depends on SString 
#include "CFunctionParse.h"  // depends on SString and CCollection

// -------------------- Process Tools -------------------------------------------------

#ifdef _WIN64
  SYSTEMS_API2 LPVOID WINAPI AllocMemEx(DWORD size, HANDLE hProcess = NULL, LPVOID pPreferredAddress = NULL);
#else
  SYSTEMS_API2 LPVOID WINAPI AllocMemEx(DWORD size, HANDLE hProcess = NULL);
#endif
SYSTEMS_API2 BOOL   WINAPI FreeMemEx(LPVOID pMemory, HANDLE hProcess = NULL);
SYSTEMS_API BOOL   WINAPI IsBadReadPtr2(LPVOID src, SIZE_T count);
SYSTEMS_API BOOL   WINAPI IsBadWritePtr2(LPVOID dst, SIZE_T count);
SYSTEMS_API BOOL   WINAPI IsMemoryProtected(LPVOID address);
SYSTEMS_API PPEB   WINAPI GetPeb(HANDLE hProcess);
#ifndef _WIN64
  SYSTEMS_API ULONGLONG WINAPI GetPeb64(HANDLE hProcess);
  SYSTEMS_API BOOL WINAPI ReadProcessMemory64(HANDLE ProcessHandle, ULONG64 BaseAddress, PVOID Buffer, ULONG64 BufferSize, PULONG64 NumberOfBytesRead);
#endif
SYSTEMS_API2 DWORD  WINAPI ProcessHandleToId(HANDLE hProcess);
SYSTEMS_API2 DWORD  WINAPI ThreadHandleToId(HANDLE hThread);
SYSTEMS_API2 BOOL   WINAPI ProcessIdToFileNameW(DWORD processId, LPWSTR fileName, USHORT bufLenInChars);
SYSTEMS_API2 BOOL   WINAPI ProcessIdToFileNameA(DWORD processId, LPSTR  fileName, USHORT bufLenInChars);
SYSTEMS_API DWORD  WINAPI GetProcessSessionId(DWORD processId);
SYSTEMS_API2 DWORD  WINAPI GetCurrentSessionId(void);
SYSTEMS_API2 DWORD  WINAPI GetInputSessionId(void);
SYSTEMS_API2 BOOL   WINAPI AmUsingInputDesktop(void);
SYSTEMS_API BOOL   WINAPI IsSystemProcess(HANDLE hProcess, PSID sid = NULL);
SYSTEMS_API2 BOOL   WINAPI AmSystemProcess(void);
SYSTEMS_API2 BOOL   WINAPI Is64bitProcess(HANDLE hProcess);
SYSTEMS_API HANDLE WINAPI HandleLiveForever(HANDLE handle);
SYSTEMS_API HANDLE WINAPI GetSmssProcessHandle(void);
SYSTEMS_API BOOL   WINAPI IsAdminAndElevated(void);
SYSTEMS_API2 LPVOID WINAPI CopyFunction(LPVOID func,
                                        HANDLE hProcess = NULL,
                                        BOOL acceptUnknownTargets = FALSE,
                                        LPVOID *buffer = NULL,
                                        CFunctionParse **pFunctionParse = NULL,
                                        int extraSpace = 0);

SYSTEMS_API2 HANDLE WINAPI madCreateRemoteThread(HANDLE hProcess,
                                                 SECURITY_ATTRIBUTES *pThreadAttributes,
                                                 DWORD stackSize,
                                                 LPVOID pStartAddress,
                                                 LPVOID pParameters,
                                                 DWORD creationFlags,
                                                 DWORD *pThreadId);

typedef DWORD (WINAPI *PFN_REMOTE_EXECUTE_FUNCTION)(LPVOID params);
SYSTEMS_API2 BOOL WINAPI RemoteExecute(HANDLE hProcess,
                                       PFN_REMOTE_EXECUTE_FUNCTION function,
                                       DWORD *result,
                                       LPVOID parameters = NULL,
                                       DWORD size = 0);

SYSTEMS_API BOOL WINAPI GetProcessSid(HANDLE hProcess, SID_AND_ATTRIBUTES **ppSidAndAttributes);

// -------------------- Patching Tools -------------------------------------------------

SYSTEMS_API BOOL   WINAPI PatchExportTable(HMODULE hModule, LPVOID pOld, LPVOID pNew, LPVOID *ws2procList);
SYSTEMS_API void   WINAPI PatchImportTable(HMODULE hModule, LPVOID pOld, LPVOID pNew);
SYSTEMS_API void   WINAPI PatchAllImportTables(LPVOID pOld, LPVOID pNew, BOOL shared);
SYSTEMS_API LPVOID WINAPI FindRealCode(LPVOID pCode);

// -------------------- Options --------------------------------------------------------

#include "Options.h"

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

// by default madCodeHook allocates at <= 0x71af0000
// you can tell madCodeHook to allocate at the address you prefer
// param: LPVOID (must be < 0x80000000)
// default: 0x71af0000
#define X86_ALLOCATION_ADDRESS (DWORD) 0x0000000c

// with this API you can configure some aspects of madCodeHook
// available options see constants above
SYSTEMS_API2 BOOL WINAPI SetMadCHookOption(DWORD option, LPCWSTR param);
SYSTEMS_API LPCWSTR WINAPI GetMadCHookOption(DWORD option);

// -------------------- Injection ------------------------------------------------------

// same as CreateProcess
// additionally the dll "loadLibrary" is injected into the newly created process
// the dll gets loaded before the entry point of the exe module is called
SYSTEMS_API2 BOOL WINAPI CreateProcessExW(LPCWSTR applicationName,
                                          LPWSTR commandLine,
                                          SECURITY_ATTRIBUTES *processAttr,
                                          SECURITY_ATTRIBUTES *threadAttr,
                                          BOOL inheritHandles,
                                          DWORD creationFlags,
                                          LPVOID environment,
                                          LPCWSTR currentDirectory,
                                          STARTUPINFOW *startupInfo,
                                          PROCESS_INFORMATION *processInfo,
                                          LPCWSTR loadLibrary);
SYSTEMS_API2 BOOL WINAPI CreateProcessExA(LPCSTR applicationName,
                                          LPSTR commandLine,
                                          SECURITY_ATTRIBUTES *processAttr,
                                          SECURITY_ATTRIBUTES *threadAttr,
                                          BOOL inheritHandles,
                                          DWORD creationFlags,
                                          LPVOID environment,
                                          LPCSTR currentDirectory,
                                          STARTUPINFOA *startupInfo,
                                          PROCESS_INFORMATION *processInfo,
                                          LPCSTR loadLibrary);

// (un)inject the specified dll into (from) all current and future processes
// these flags can be used for both UninjectLibrary + InjectLibrary in place of hProcess
const DWORD ALL_SESSIONS    = (DWORD) -1;  // apps of all sessions
const DWORD CURRENT_SESSION = (DWORD) -2;  // apps of current session

SYSTEMS_API2 BOOL WINAPI   InjectLibraryA(LPCSTR  pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut);
SYSTEMS_API2 BOOL WINAPI   InjectLibraryW(LPCWSTR pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut);
SYSTEMS_API2 BOOL WINAPI UninjectLibraryA(LPCSTR  pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut);
SYSTEMS_API2 BOOL WINAPI UninjectLibraryW(LPCWSTR pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut);

SYSTEMS_API2 BOOL WINAPI   InjectLibrarySystemWideA(LPCSTR  pDriverName, LPCSTR  pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCSTR  pIncludeMask, LPCSTR  pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut);
SYSTEMS_API2 BOOL WINAPI   InjectLibrarySystemWideW(LPCWSTR pDriverName, LPCWSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCWSTR pIncludeMask, LPCWSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut);
SYSTEMS_API2 BOOL WINAPI UninjectLibrarySystemWideA(LPCSTR  pDriverName, LPCSTR  pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCSTR  pIncludeMask, LPCSTR  pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut);
SYSTEMS_API2 BOOL WINAPI UninjectLibrarySystemWideW(LPCWSTR pDriverName, LPCWSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCWSTR pIncludeMask, LPCWSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut);

SYSTEMS_API2 BOOL WINAPI UninjectAllLibrariesA(LPCSTR  pDriverName, PULONG pExcludePIDs, DWORD dwTimeOutPerUninject);
SYSTEMS_API2 BOOL WINAPI UninjectAllLibrariesW(LPCWSTR pDriverName, PULONG pExcludePIDs, DWORD dwTimeOutPerUninject);

SYSTEMS_API BOOL WINAPI NotInitializedYet(ULONG_PTR pPeb32, ULONGLONG pPeb64, HANDLE hProcess);

SYSTEMS_API2 BOOL WINAPI InstallInjectionDriver(LPCWSTR driverName, LPCWSTR fileName32bit, LPCWSTR fileName64bit, LPCWSTR description);
SYSTEMS_API2 BOOL WINAPI UninstallInjectionDriver(LPCWSTR driverName);
SYSTEMS_API2 BOOL WINAPI LoadInjectionDriver(LPCWSTR driverName, LPCWSTR fileName32bit, LPCWSTR fileName64bit);
SYSTEMS_API2 BOOL WINAPI StopInjectionDriver(LPCWSTR driverName);
SYSTEMS_API2 BOOL WINAPI StartInjectionDriver(LPCWSTR driverName);
SYSTEMS_API2 BOOL WINAPI IsInjectionDriverInstalled(LPCWSTR driverName);
SYSTEMS_API2 BOOL WINAPI IsInjectionDriverRunning(LPCWSTR driverName);
SYSTEMS_API2 BOOL WINAPI StartDllInjection(LPCWSTR driverName, LPCWSTR dllFileName, DWORD session, BOOL systemProcesses, LPCWSTR includeMask, LPCWSTR excludeMask);
SYSTEMS_API2 BOOL WINAPI StopDllInjection(LPCWSTR driverName, LPCWSTR dllFileName, DWORD session, BOOL systemProcesses, LPCWSTR includeMask, LPCWSTR excludeMask);
SYSTEMS_API2 BOOL WINAPI SetInjectionMethod(LPCWSTR driverName, BOOL newInjectionMethod);

// -------------------- Hooking ---------------------------------------------------------

// by default madCodeHook counts how many times any thread is currently
// running inside of your callback function
// this way unhooking can be safely synchronized to that counter
// sometimes you don't need/want this counting to happen, e.g.
// (1) if you don't plan to ever unhook, anyway
// (2) if the counting performance drop is too high for your taste
// (3) if you want to unhook from inside the hook callback function
// in those cases you can set the flag "NO_SAFE_UNHOOKING"
const int DONT_COUNT = 0x00000001;         // old name - kept for compatability
const int NO_SAFE_UNHOOKING = 0x00000001;  // new name
// madCodeHook implements two different API hooking methods
// the mixture mode is the second best method, it's only used if the main
// hooking method doesn't work for whatever reason (e.g. API code structure)
// normally madCodeHook chooses automatically which mode to use
// you can force madCodeHook to use the mixture mode by specifying the flag:
const int MIXTURE_MODE = 0x00000002;
// if you don't want madCodeHook to use the mixture mode, you can say so
// however, if the main hooking mode can't be used, hooking then simply fails
const int NO_MIXTURE_MODE = 0x00000010;
// optionally madCodeHook can use a special technique to make sure that
// hooking in multi threaded situations won't result in crashing threads
// this technique is not tested too well right now, so it's optional for now
// you can turn this feature on by setting the flag "SAFE_HOOKING"
// without this technique crashes can happen, if a thread is calling the API
// which we want to hook in exactly the moment when the hook is installed
// safe hooking is currently only available in the NT family
const int SAFE_HOOKING = 0x00000020;
// with 2.1f the "safe unhooking" functionality (see above) was improved
// most probably there's no problem with the improvement
// but to be sure you can disable the improvement
// the improved safe unhooking is currently only available in the NT family
const int NO_IMPROVED_SAFE_UNHOOKING = 0x00000040;
// winsock2 normally doesn't like the mixture mode
// however, I've found a way to convince winsock2 to accept mixture hooks
// this is a somewhat experimental feature, though
// so it must be turned on explicitly
const int ALLOW_WINSOCK2_MIXTURE_MODE = 0x00000080;
// By default, if the target API was already hooked by other hook library,
// madCodeHook switches to mixture mode (if possible).
// If the other hook library used code overwriting with a simple JMP,
// using the flag FOLLOW_JMP will instead make madCodeHook hook the callback
// function of the other hook library. This should work just fine. However,
// your hook will stop working in this case in the moment when the other
// hook library uninstalls its hook.
const int FOLLOW_JMP = 0x00000200;

SYSTEMS_API2 BOOL WINAPI HookCode(LPVOID pCode, LPVOID pCallback, LPVOID *pNextHook, DWORD flags = 0);
SYSTEMS_API2 BOOL WINAPI HookAPI(LPCSTR moduleName, LPCSTR apiName, LPVOID pCallback, LPVOID *pNextHook, DWORD flags = 0);
SYSTEMS_API  BOOL WINAPI Unhook(LPVOID *pNextHook);
SYSTEMS_API2 BOOL WINAPI UnhookCode(LPVOID *pNextHook);
SYSTEMS_API2 BOOL WINAPI UnhookAPI(LPVOID *pNextHook);
SYSTEMS_API2 BOOL WINAPI RenewHook(LPVOID *pNextHook);
SYSTEMS_API2 int  WINAPI IsHookInUse(LPVOID *pNextHook);
SYSTEMS_API2 void WINAPI CollectHooks(void);
SYSTEMS_API2 void WINAPI FlushHooks(void);
SYSTEMS_API2 BOOL WINAPI RestoreCode(LPVOID pCode);

// ------------------------ IPC -----------------------------------------------------

typedef void (WINAPI *PFN_IPC_CALLBACK)(LPCSTR name, LPCVOID messageBuffer, DWORD messageLength, LPCVOID answerBuffer, DWORD answerLength);

typedef struct tagIpcMessage
{
  char *Buffer;
  DWORD Length;
} IPC_MESSAGE;

typedef struct tagIpcAnswer
{
  HANDLE Map;
  LPCVOID Buffer;
  DWORD Length;
  HANDLE Event1;
  HANDLE Event2;
} IPC_ANSWER;

typedef struct tagPipedIpcRec
{
  HANDLE Map;
  DWORD ProcessId;
  HANDLE ReadPipeHandle;
  HANDLE WritePipeHandle;
  PFN_IPC_CALLBACK Callback;
  DWORD MaxThreads;
  DWORD MaxQueue;
  DWORD Flags;
  HANDLE ThreadHandle;
  char Name[MAX_PATH];
  DWORD Counter;
  IPC_MESSAGE Message;
  IPC_ANSWER Answer;
} PIPED_IPC_REC;

SYSTEMS_API2 BOOL WINAPI Is64bitOS(void);
SYSTEMS_API2 BOOL WINAPI IsWine(void);

SYSTEMS_API2 BOOL WINAPI AddAccessForEveryone(HANDLE processOrService, DWORD access);
SYSTEMS_API2 BOOL WINAPI CreateIpcQueueEx(LPCSTR ipcName, PFN_IPC_CALLBACK callback, DWORD maxThreadCount = 16, DWORD maxQueueLen = 0x1000);
SYSTEMS_API2 BOOL WINAPI CreateIpcQueue(LPCSTR ipcName, PFN_IPC_CALLBACK callback);
SYSTEMS_API2 BOOL WINAPI DestroyIpcQueue(LPCSTR ipcName);
SYSTEMS_API2 BOOL WINAPI SendIpcMessage(LPCSTR ipcName, void *messageBuf, DWORD messageLen, void *answerBuf = NULL, DWORD answerLen = 0, DWORD answerTimeout = INFINITE, BOOL handleMessages = true);

#endif