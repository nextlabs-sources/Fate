// ***************************************************************
//  FunctionTypes.h           version: 1.0.3  ·  date: 2016-03-16
//  -------------------------------------------------------------
//  function types for dynamic linking
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2016 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2016-03-16 1.0.3 (1) added NtDelayExecution
//                  (2) fixed: some undocumented APIs had incorrect types
// 2013-02-13 1.0.2 fixed RtlCreateUserThread definition
// 2010-05-25 1.0.1 (1) added RtlNtStatusToDosError
//                  (2) added Enter/LeaveCriticalSection
// 2010-01-10 1.0.0 initial version

#ifndef _FUNCTIONTYPES_H
#define _FUNCTIONTYPES_H

#include "Aclapi.h"

// -------------------------------- Internal function definitions --------------------------

typedef BOOL (WINAPI *PFN_CHECK_PROC_ADDRESS)(LPVOID *pAddress);

typedef LPVOID (WINAPI *PFN_NEED_MODULE_FILE_MAP_EX)(HMODULE hModule);

// -------------------------------- kernel32.dll functions ----------------------------------

// DllMain
typedef BOOL (WINAPI *PFN_ENTRY_POINT)(HINSTANCE hInstance, DWORD reason, LPVOID reserved);
// GetVersion
typedef DWORD (WINAPI *PFN_GET_VERSION)(void);
// GetLastError
typedef DWORD (WINAPI *PFN_GET_LAST_ERROR)(void);
// GetCurrentProcessId
typedef DWORD (WINAPI *PFN_GET_CURRENT_PROCESS_ID)(void);
// GetProcessProcess
typedef HANDLE (WINAPI *PFN_GET_CURRENT_PROCESS)(void);
// VirtualFree
typedef BOOL (WINAPI *PFN_VIRTUAL_FREE)(LPVOID pAddress, SIZE_T size, DWORD freeType);
// SetErrorMode
typedef UINT (WINAPI *PFN_SET_ERROR_MODE)(UINT mode);
// LoadLibraryW
typedef HMODULE (WINAPI *PFN_LOAD_LIBRARY_W)(LPCWSTR fileName);
// GetModuleHandleW
typedef HMODULE (WINAPI *PFN_GET_MODULE_HANDLE_W)(LPCWSTR moduleName);
// OpenFileMappingA
typedef HANDLE (WINAPI *PFN_OPEN_FILE_MAPPING_A)(DWORD desiredAccess, BOOL inheritHandle, LPCSTR name);
// OpenFileMappingW
typedef HANDLE (WINAPI *PFN_OPEN_FILE_MAPPING_W)(DWORD desiredAccess, BOOL inheritHandle, LPCWSTR name);
// MapViewOfFile
typedef LPVOID (WINAPI *PFN_MAP_VIEW_OF_FILE)(HANDLE fileMappingObject, DWORD desiredAccess, DWORD offsetHigh, DWORD offsetLow, SIZE_T numberBytesToMap);
// UnmapViewOfFile
typedef BOOL (WINAPI *PFN_UNMAP_VIEW_OF_FILE)(LPCVOID baseAddress);
// CloseHandle
typedef BOOL (WINAPI *PFN_CLOSE_HANDLE)(HANDLE handle);
// FreeLibrary
typedef BOOL (WINAPI *PFN_FREE_LIBRARY)(HMODULE module);
// ProcessIdToSessionId
typedef BOOL (WINAPI *PFN_PROCESS_ID_TO_SESSION_ID)(DWORD processId, DWORD *pSessionId);
// GetNativeSystemInfo
typedef void (WINAPI * PFN_GET_NATIVE_SYSTEM_INFO)(SYSTEM_INFO *SystemInfo);
// IsWow64Process
typedef BOOL (WINAPI * PFN_IS_WOW64_PROCESS)(HANDLE hProcess, BOOL *wow64Process);
// QueryFullProcessImageName
typedef BOOL (WINAPI * PFN_QUERY_FULL_PROCESS_IMAGE_NAME)(HANDLE hProcess, DWORD dwFlags, LPTSTR lpExeName, PDWORD lpdwSize);
// EnterCriticalSection
typedef void (WINAPI *PFN_ENTER_CRITICAL_SECTION)(LPCRITICAL_SECTION lpCriticalSection);
// LeaveCriticalSection
typedef void (WINAPI *PFN_LEAVE_CRITICAL_SECTION)(LPCRITICAL_SECTION lpCriticalSection);
// QueueUserAPC
typedef DWORD (WINAPI *PFN_QUEUE_USER_APC)(PAPCFUNC pfnAPC, HANDLE hThread, ULONG_PTR dwData);
// GetThreadId
typedef DWORD (WINAPI *PFN_GET_THREAD_ID)(HANDLE Thread);
// GetProcessId
typedef DWORD (WINAPI *PFN_GET_PROCESS_ID)(HANDLE Process);
// GetProcessIdOfThread
typedef DWORD (WINAPI *PDN_GET_PROCESS_ID_OF_THREAD)(HANDLE Thread);

#ifdef _WIN64

  typedef struct _WOW64_FLOATING_SAVE_AREA_
  {
    DWORD   ControlWord;
    DWORD   StatusWord;
    DWORD   TagWord;
    DWORD   ErrorOffset;
    DWORD   ErrorSelector;
    DWORD   DataOffset;
    DWORD   DataSelector;
    BYTE    RegisterArea[80];
    DWORD   Cr0NpxState;
  } WOW64_FLOATING_SAVE_AREA_;

  typedef struct _WOW64_CONTEXT_
  {
    DWORD                       ContextFlags;
    DWORD                       Dr0;
    DWORD                       Dr1;
    DWORD                       Dr2;
    DWORD                       Dr3;
    DWORD                       Dr6;
    DWORD                       Dr7;
    WOW64_FLOATING_SAVE_AREA_   FloatSave;
    DWORD                       SegGs;
    DWORD                       SegFs;
    DWORD                       SegEs;
    DWORD                       SegDs;
    DWORD                       Edi;
    DWORD                       Esi;
    DWORD                       Ebx;
    DWORD                       Edx;
    DWORD                       Ecx;
    DWORD                       Eax;
    DWORD                       Ebp;
    DWORD                       Eip;
    DWORD                       SegCs;
    DWORD                       EFlags;
    DWORD                       Esp;
    DWORD                       SegSs;
    BYTE                        ExtendedRegisters[512];
  } WOW64_CONTEXT_, *PWOW64_CONTEXT_;

  // Wow64GetThreadContext
  typedef BOOL (WINAPI * PFN_WOW64_GET_THREAD_CONTEXT)(HANDLE hThread, PWOW64_CONTEXT_ lpContext);
  // Wow64GetThreadSelectorEntry
  typedef BOOL (WINAPI * PFN_WOW64_GET_THREAD_SELECTOR_ENTRY)(HANDLE hThread, DWORD dwSelector, PLDT_ENTRY lpSelectorEntry);

#endif

// ---------------------------- kernelbase.dll functions ------------------------------------------

// GetAppContainerNamedObjectPath
typedef BOOL (WINAPI *PFN_GET_APP_CONTAINER_NAMED_OBJECT_PATH)(HANDLE Token, PSID AppContainerSid, ULONG ObjectPathLength, LPWSTR ObjectPath, PULONG ReturnLength);

// ---------------------------- ntdll.dll functions -----------------------------------------------

// from ntddk.h
typedef enum _SECTION_INHERIT
{
  ViewShare = 1,
  ViewUnmap = 2
} SECTION_INHERIT;

// LdrLoadDll
typedef NTSTATUS (NTAPI *PFN_LDR_LOAD_DLL)(PWCHAR path, LPVOID flags, PUNICODE_STRING moduleFileName, HMODULE *pHModule);
// LdrUnloadDll
typedef NTSTATUS (NTAPI *PFN_LDR_UNLOAD_DLL)(HMODULE hModule);
// LdrGetDllHandle
typedef NTSTATUS (NTAPI *PFN_LDR_GET_DLL_HANDLE)(PWORD path, PVOID unused, PUNICODE_STRING moduleFileName, HMODULE *pHModule);
// LdrGetProcedureAddress
typedef NTSTATUS (NTAPI *PFN_LDR_GET_PROCEDURE_ADDRESS)(HMODULE hModule, PANSI_STRING functionName, ULONG ordinal, LPVOID *pFunction);
// LdrRegisterDllNotification
typedef NTSTATUS (NTAPI *PFN_LDR_REGISTER_DLL_NOTIFICATION)(ULONG Flags, PVOID NotificationFunction, PVOID Context, PVOID *Cookie);
// LdrUnregisterDllNotification
typedef NTSTATUS (NTAPI *PFN_LDR_UNREGISTER_DLL_NOTIFICATION)(PVOID Cookie);
// NtClose
typedef NTSTATUS (NTAPI *PFN_NT_CLOSE)(HANDLE objectHandle);
// NtAllocateVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_ALLOCATE_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID *pBaseAddress, ULONG_PTR zeroBits, PSIZE_T pRegionSize, ULONG allocationType, ULONG protect);
// NtFreeVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_FREE_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID *pBaseAddress, PSIZE_T pRegionSize, ULONG freeType);
// NtQueryVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_QUERY_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID pBaseAddress, ULONG infoClass, MEMORY_BASIC_INFORMATION *pMbi, SIZE_T length, PSIZE_T pResultLength);
// NtQueryInformationProcess
typedef NTSTATUS (NTAPI *PFN_NT_QUERY_INFORMATION_PROCESS)(HANDLE hProcess, PROCESS_INFO_CLASS infoClass, LPVOID pBuffer, ULONG bufferLength, ULONG *pReturnLength);
// NtQueryInformationThread
typedef NTSTATUS (NTAPI *PFN_NT_QUERY_INFORMATION_THREAD)(HANDLE hProcess, THREAD_INFO_CLASS infoClass, LPVOID pBuffer, ULONG bufferLength, ULONG *pReturnLength);
// NtProtectVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_PROTECT_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID *pBaseAddress, PSIZE_T numberOfBytesToProtect, ULONG newAccessProtection, PULONG oldAccessProtection);
// NtReadVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_READ_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID pBaseAddress, LPVOID pBuffer, SIZE_T numberOfBytesToRead, PSIZE_T pNumberOfBytesRead);
// NtWriteVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_WRITE_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID pBaseAddress, LPVOID pBuffer, SIZE_T numberOfBytesToWrite, PSIZE_T pNumberOfBytesWritten);
// NtOpenSection
typedef NTSTATUS (NTAPI *PFN_NT_OPEN_SECTION)(HANDLE *phSectionHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES pObjectAttributes);
// NtMapViewOfSection
typedef NTSTATUS (NTAPI *PFN_NT_MAP_VIEW_OF_SECTION)(HANDLE hSection, HANDLE hProcess, LPVOID pBaseAddress, ULONG_PTR zeroBits, SIZE_T commitSize, PLARGE_INTEGER pSectionOffset, PSIZE_T pViewSize, SECTION_INHERIT inheritDisposition, ULONG allocationType, ULONG protect);
// NtUnmapViewOfSection
typedef NTSTATUS (NTAPI *PFN_NT_UNMAP_VIEW_OF_SECTION)(HANDLE hProcess, LPVOID pBaseAddress);
// NtTestAlert
typedef NTSTATUS (NTAPI *PFN_NT_TEST_ALERT)(void);
// NtOpenThread
typedef NTSTATUS (NTAPI *PFN_NT_OPEN_THREAD)(HANDLE *hThread, DWORD desiredAccess, OBJECT_ATTRIBUTES *objAttributes, CLIENT_ID *clientId);
// NtQuerySystemInformation
typedef NTSTATUS (NTAPI *PFN_NT_QUERY_SYSTEM_INFORMATION)(SYSTEM_INFORMATION_CLASS infoClass, LPVOID buffer, ULONG bufferLength, ULONG *pReturnLengt);
// NtQueryInformationToken
typedef NTSTATUS (NTAPI *PFN_NT_QUERY_INFORMATION_TOKEN)(HANDLE hToken, TOKEN_INFORMATION_CLASS infoClass, PVOID buffer, ULONG bufferLenth, PULONG pReturnLength);
// NtOpenProcessToken
typedef NTSTATUS (NTAPI *PFN_NT_OPEN_PROCESS_TOKEN)(HANDLE hProcess, ACCESS_MASK desiredAccess, HANDLE *phToken);
// RtlInitUnicodeString
typedef void (NTAPI *PFN_RTL_INIT_UNICODE_STRING)(PUNICODE_STRING destinationString, LPCWSTR sourceString);
// RtlInitAnsiString
typedef void (NTAPI *PFN_RTL_INIT_ANSI_STRING)(PANSI_STRING destinationString, LPCSTR sourceString);
// RtlCompareUnicodeString
typedef long (NTAPI *PFN_RTL_COMPARE_UNICODE_STRING)(PUNICODE_STRING string1, PUNICODE_STRING string2, BOOLEAN isCaseInsensitive);
// RtlCreateUserThread
typedef int (NTAPI *PFN_RTL_CREATE_USER_THREAD)(HANDLE hProcess, SECURITY_DESCRIPTOR *pSecurityDescriptor, ULONG_PTR createSuspended, ULONG_PTR stackZeroBits, SIZE_T *stackReserved, SIZE_T *stackCommit, LPVOID pStartAddress, LPVOID pParameters, HANDLE *threadHandle, __int64 *clientId);
// RtlValidSid
typedef BOOLEAN (NTAPI *PFN_RTL_VALID_SID)(PSID pSid);
// RtlEqualSid
typedef BOOL (NTAPI *PFN_RTL_EQUAL_SID)(PSID pSid1, PSID pSid2);
// NtLoadDriver
typedef NTSTATUS (NTAPI *PFN_NT_LOAD_DRIVER)(PUNICODE_STRING name);
// NtUnloadDriver
typedef NTSTATUS (NTAPI *PFN_NT_UNLOAD_DRIVER)(PUNICODE_STRING name);
// RtlNtStatusToDosError
typedef DWORD (NTAPI *PFN_RTL_NT_STATUS_TO_DOS_ERROR)(DWORD error);
// NtQueryObject
typedef DWORD (NTAPI *PFN_NT_QUERY_OBJECT)(HANDLE Handle, ULONG ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength);
// NtDelayExecution
typedef NTSTATUS (NTAPI *PFN_NT_DELAY_EXECUTION)(BOOLEAN Alertable, PLARGE_INTEGER DelayInterval);

// ---------------------------- advapi.dll functions ----------------------------------------------

typedef DWORD (WINAPI *PFN_SET_ENTRIES_IN_ACL)(ULONG cCountOfExplicitEntries, PEXPLICIT_ACCESS pListOfExplicitEntries, PACL OldAcl, PACL *NewAcl);

#undef EXTERN
#ifdef _MODULETOOLS_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

#undef __NULL
#ifdef _MODULE_C
  #define __NULL = NULL
#else
  #define __NULL
#endif

EXTERN PFN_CHECK_PROC_ADDRESS pfnCheckProcAddress __NULL;
EXTERN PFN_NEED_MODULE_FILE_MAP_EX pfnNeedModuleFileMapEx __NULL;
EXTERN PFN_GET_VERSION pfnGetVersion __NULL;
EXTERN PFN_GET_LAST_ERROR pfnGetLastError __NULL;
EXTERN PFN_GET_CURRENT_PROCESS_ID pfnGetCurrentProcessId __NULL;
EXTERN PFN_VIRTUAL_FREE pfnVirtualFree __NULL;
EXTERN PFN_SET_ERROR_MODE pfnSetErrorMode __NULL;
EXTERN PFN_LOAD_LIBRARY_W pfnLoadLibraryW __NULL;
EXTERN PFN_GET_MODULE_HANDLE_W pfnGetModuleHandleW __NULL;
EXTERN PFN_OPEN_FILE_MAPPING_A pfnOpenFileMappingA __NULL;
EXTERN PFN_MAP_VIEW_OF_FILE pfnMapViewOfFile __NULL;
EXTERN PFN_UNMAP_VIEW_OF_FILE pfnUnmapViewOfFile __NULL;
EXTERN PFN_CLOSE_HANDLE pfnCloseHandle __NULL;
EXTERN PFN_FREE_LIBRARY pfnFreeLibrary __NULL;
EXTERN PFN_IS_WOW64_PROCESS pfnIsWow64Process __NULL;
EXTERN PFN_ENTER_CRITICAL_SECTION pfnEnterCriticalSection __NULL;
EXTERN PFN_LEAVE_CRITICAL_SECTION pfnLeaveCriticalSection __NULL;

#ifdef _WIN64
  EXTERN PFN_GET_VERSION pfnGetVersion2GB __NULL;
  EXTERN PFN_GET_LAST_ERROR pfnGetLastError2GB __NULL;
  EXTERN PFN_GET_CURRENT_PROCESS_ID pfnGetCurrentProcessId2GB __NULL;
  EXTERN PFN_VIRTUAL_FREE pfnVirtualFree2GB __NULL;
  EXTERN PFN_SET_ERROR_MODE pfnSetErrorMode2GB __NULL;
  EXTERN PFN_LOAD_LIBRARY_W pfnLoadLibraryW2GB __NULL;
  EXTERN PFN_GET_MODULE_HANDLE_W pfnGetModuleHandleW2GB __NULL;
  EXTERN PFN_OPEN_FILE_MAPPING_A pfnOpenFileMappingA2GB __NULL;
  EXTERN PFN_MAP_VIEW_OF_FILE pfnMapViewOfFile2GB __NULL;
  EXTERN PFN_UNMAP_VIEW_OF_FILE pfnUnmapViewOfFile2GB __NULL;
  EXTERN PFN_CLOSE_HANDLE pfnCloseHandle2GB __NULL;
  EXTERN PFN_FREE_LIBRARY pfnFreeLibrary2GB __NULL;
  EXTERN PFN_ENTER_CRITICAL_SECTION pfnEnterCriticalSection2GB __NULL;
  EXTERN PFN_LEAVE_CRITICAL_SECTION pfnLeaveCriticalSection2GB __NULL;
#endif

#endif