// ***************************************************************
//  ToolFuncs.h               version: 1.0.4  ·  date: 2014-05-05
//  -------------------------------------------------------------
//  collection of tool functions
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2014 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2014-05-05 1.0.4 (1) IsWin8OrNewer added
//                  (2) Set/RemoveLoadImageNotifyRoutine added
// 2013-01-09 1.0.3 (1) added CriticalSection helper functions
//                  (2) added ProcessIdToSessionId
// 2011-11-19 1.0.2 (1) GetModuleHandle -> GetSystemModuleHandle
//                  (2) added AttachTo/DetachMemoryContext
//                  (3) added FindNtdll
// 2010-03-28 1.0.1 (1) GetRemoteProcAddress/AddressOfEntryPoint added
//                  (2) IsRemoteModule32bit added
//                  (3) IsXpOrNewer and IsVistaOrNewer added
// 2010-01-10 1.0.0 initial release

#ifndef _ToolFuncs_
#define _ToolFuncs_

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

// ********************************************************************
// initialize all the undocumented stuff

BOOLEAN InitToolFuncs (void);

// ********************************************************************
// file creation functions to do simple logging

void LogBin       (LPWSTR FileName, PVOID Buf, ULONG Len);
void LogStr       (LPWSTR FileName, LPWSTR Content);
void LogUlonglong (LPWSTR FileName, ULONGLONG Value);

// ********************************************************************
// OS version helpers

// is this XP or 2003 or any newer OS?
BOOLEAN IsXpOrNewer (void);

// is this Vista or 2008 or any newer OS?
BOOLEAN IsVistaOrNewer (void);

// is this Windows 8 or any newer OS?
BOOLEAN IsWin8OrNewer (void);

// ********************************************************************
// memory context grabbing

// structure for AttachToMemoryContext
typedef struct _KAPC_STATE {
     LIST_ENTRY ApcListHead[2];
     PEPROCESS  Process;
     BOOLEAN    KernelApcInProgress;
     BOOLEAN    KernelApcPending;
     BOOLEAN    UserApcPending;
     PVOID      Padding[16];
} KAPC_STATE, *PKAPC_STATE;

BOOLEAN AttachToMemoryContext (HANDLE ProcessId, PEPROCESS *EProcess, KAPC_STATE *ApcState);
// attaches the current thread to the memory context of the specific process

void DetachMemoryContext (PEPROCESS EProcess, KAPC_STATE *ApcState);
// undoes "AttachToMemoryContext", see above

// ********************************************************************
// dynamic linking helpers (for the current process / memory context)

// get the handle of a system dll
HANDLE GetSystemModuleHandle (PCSTR ModuleName);

// tries to locate ntdll, loaded in the current memory context
HANDLE FindNtdll (void);

// see win32 API "GetProcAddress"
PVOID GetProcAddress (HANDLE ModuleHandle, PCSTR ProcName, BOOLEAN Virtual, BOOLEAN VirtualResult);

// return the address where the entry point of the specified dll is stored
PVOID GetAddressOfEntryPoint (HANDLE ModuleHandle);

// ********************************************************************
// dynamic linking helpers (for a specific remote process / memory context)

// similar to GetProcAddress, but for a remote process / memory context
PVOID GetRemoteProcAddress (HANDLE Process, HANDLE ModuleHandle, PCSTR ProcName);

// similar to GetAddressOfEntryPoint, but for a remote process / memory context
PVOID GetRemoteAddressOfEntryPoint (HANDLE Process, HANDLE ModuleHandle);

// find out whether the specified module handle is a 32bit module
BOOLEAN IsRemoteModule32bit (HANDLE Process, HANDLE ModuleHandle);

// ********************************************************************
// maps a dll file from harddisk into RAM, so we can parse it

BOOLEAN MapDllFile (LPWSTR ModulePath, HANDLE *file, HANDLE *section, HANDLE *moduleHandle);
void UnmapDllFile (HANDLE file, HANDLE section, HANDLE moduleHandle);

// ********************************************************************
// image load notification

// see PsSet/RemoveImageNotifyRoutine
NTSTATUS    SetLoadImageNotifyRoutine (PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine);
NTSTATUS RemoveLoadImageNotifyRoutine (PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine);

// ********************************************************************
// some handle/object conversion helpers

// you need to close these handles with "ZwClose"
HANDLE OpenProcess (HANDLE ProcessId);
HANDLE OpenThread (HANDLE ThreadId);
HANDLE OpenEProcess (PEPROCESS Process);
HANDLE OpenEThread (PETHREAD Thread);

// which thread id does this thread handle represent?
// which process does this thread belong to?
BOOLEAN ThreadHandleToThreadProcessId (HANDLE ThreadHandle, HANDLE *ThreadId, HANDLE *TrocessId);

// get the session id of the specified process
BOOLEAN ProcessIdToSessionId (HANDLE ProcessId, ULONG *Session);

// ********************************************************************

// count running processes of a specific session
ULONG CountProcesses (ULONG Session);

// ********************************************************************
// query information about a specific process

// find the full exe file path/name
BOOLEAN GetExeFileName (HANDLE ProcessHandle, WCHAR *Buf, ULONG BufLen);

// is the specified process a system process?
BOOLEAN IsSystemProcess (HANDLE ProcessHandle);

// is the specified process a 64bit process?
BOOLEAN Is64bitProcess (HANDLE ProcessHandle);

// is the specified process a native process?
BOOLEAN IsNativeProcess (HANDLE ProcessHandle);

// get the session id of the specified process
ULONG GetProcessSessionId (HANDLE ProcessHandle);

// get the entry point of the exe file of the specified process
PVOID GetProcessEntryPoint (HANDLE ProcessHandle);

// ********************************************************************
// do some manipulations to a specific process

// change access rights
BOOLEAN VirtualProtectEx (HANDLE ProcessHandle, PVOID BaseAddress, SIZE_T Size, ULONG NewAccess, PULONG OldAccess);

// read from (write to) the process' address range
BOOLEAN  ReadProcessMemory (HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T Size);
BOOLEAN WriteProcessMemory (HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T Size);

// by default madCodeHook allocates at <= 0x71b00000
// this function changes this address
void SetX86AllocAddr (PVOID PreferredAddress);

// allocate memory at the preferred address (or slightly above that)
// if no preferred address is specified:
// - for 32bit processes, allocate at 0x71b00000    (or slightly below that)
// - for 64bit processes, allocate at 0x7ff00000000 (or slightly below that)
PVOID AllocMemEx (HANDLE ProcessHandle, SIZE_T Size, PVOID PreferredAddress);

// ********************************************************************
// some file related helper functions

// find out and return whether the dll file is a 64bit dll
BOOLEAN Is64bitDll (HANDLE FileHandle);

// open a file and calculate a hash of the file contents
// maxFileSize: can be used to limit the hash to a part of the file, only
// fileSize:    size of the file is returned; can be NULL
// FileHandle:  handle of the file is returned; can be NULL (in that case the file handle is closed)
BOOLEAN GetFileHash (PWCHAR FileName, PVOID FileHash, ULONG MaxFileSize, ULONG *FileSize, HANDLE *FileHandle);

// ********************************************************************
// critical section helpers

#define CRIT_SECTION KMUTEX

void InitializeCriticalSection (CRIT_SECTION *section);
void EnterCriticalSection (CRIT_SECTION *section);
void LeaveCriticalSection (CRIT_SECTION *section);

// ********************************************************************

#endif
