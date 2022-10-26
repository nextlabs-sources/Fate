// ***************************************************************
//  FunctionTypes.h           version: 1.0.0   date: 2010-01-10
//  -------------------------------------------------------------
//  function types for dynamic linking
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

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
// WaitForMultipleObjects
typedef DWORD (WINAPI *PFN_WAIT_FOR_MULTIPLE_OBJECTS)(DWORD count, const HANDLE *pHandles, BOOL waitALl, DWORD milliSeconds);
// WaitForSingleObject
typedef DWORD (WINAPI *PFN_WAIT_FOR_SINGLE_OBJECT)(HANDLE hHandle, DWORD milliSeconds);
// Sleep
typedef void (WINAPI *PFN_SLEEP)(DWORD milliSeconds);
// VirtualQuery
typedef DWORD (WINAPI *PFN_VIRTUAL_QUERY)(LPCVOID pAddress, PMEMORY_BASIC_INFORMATION pBuffer, SIZE_T length);
// LocalAlloc
typedef HLOCAL (WINAPI *PFN_LOCAL_ALLOC)(UINT flags, SIZE_T bytes);
// LocalFree
typedef HLOCAL (WINAPI *PFN_LOCAL_FREE)(HLOCAL hMem);
// SetLastError
typedef void (WINAPI *PFN_SET_LAST_ERROR)(DWORD errorCode);
// ReleaseMutex
typedef BOOL (WINAPI *PFN_RELEASE_MUTEX)(HANDLE hMutex);
// ProcessIdToSessionId
typedef BOOL (WINAPI *PFN_PROCESS_ID_TO_SESSION_ID)(DWORD processId, DWORD *pSessionId);
// GetNativeSystemInfo
typedef void (WINAPI * PFN_GET_NATIVE_SYSTEM_INFO)(SYSTEM_INFO *SystemInfo);
// IsWow64Process
typedef BOOL (WINAPI * PFN_IS_WOW64_PROCESS)(HANDLE hProcess, BOOL *wow64Process);
// QueryFullProcessImageName
typedef BOOL (WINAPI * PFN_QUERY_FULL_PROCESS_IMAGE_NAME)(HANDLE hProcess, DWORD dwFlags, LPTSTR lpExeName, PDWORD lpdwSize);

#ifdef _WIN64

  //typedef struct _WOW64_FLOATING_SAVE_AREA
  //{
  //  DWORD   ControlWord;
  //  DWORD   StatusWord;
  //  DWORD   TagWord;
  //  DWORD   ErrorOffset;
  //  DWORD   ErrorSelector;
  //  DWORD   DataOffset;
  //  DWORD   DataSelector;
  //  BYTE    RegisterArea[80];
  //  DWORD   Cr0NpxState;
  //} WOW64_FLOATING_SAVE_AREA;

  //typedef struct _WOW64_CONTEXT
  //{
  //  DWORD                       ContextFlags;
  //  DWORD                       Dr0;
  //  DWORD                       Dr1;
  //  DWORD                       Dr2;
  //  DWORD                       Dr3;
  //  DWORD                       Dr6;
  //  DWORD                       Dr7;
  //  WOW64_FLOATING_SAVE_AREA    FloatSave;
  //  DWORD                       SegGs;
  //  DWORD                       SegFs;
  //  DWORD                       SegEs;
  //  DWORD                       SegDs;
  //  DWORD                       Edi;
  //  DWORD                       Esi;
  //  DWORD                       Ebx;
  //  DWORD                       Edx;
  //  DWORD                       Ecx;
  //  DWORD                       Eax;
  //  DWORD                       Ebp;
  //  DWORD                       Eip;
  //  DWORD                       SegCs;
  //  DWORD                       EFlags;
  //  DWORD                       Esp;
  //  DWORD                       SegSs;
  //  BYTE                        ExtendedRegisters[512];
  //} WOW64_CONTEXT, *PWOW64_CONTEXT;

  // Wow64GetThreadContext
  typedef BOOL (WINAPI * PFN_WOW64_GET_THREAD_CONTEXT)(HANDLE hThread, PWOW64_CONTEXT lpContext);
  // Wow64GetThreadSelectorEntry
  typedef BOOL (WINAPI * PFN_WOW64_GET_THREAD_SELECTOR_ENTRY)(HANDLE hThread, DWORD dwSelector, PLDT_ENTRY lpSelectorEntry);

#endif

// ---------------------------- ntdll.dll functions -----------------------------------------------

// from ntddk.h
typedef enum _SECTION_INHERIT
{
  ViewShare = 1,
  ViewUnmap = 2
} SECTION_INHERIT;

// LdrLoadDll
typedef NTSTATUS (NTAPI *PFN_LDR_LOAD_DLL)(PWCHAR path, ULONG flags, PUNICODE_STRING moduleFileName, HMODULE *pHModule);
// LdrUnloadDll
typedef NTSTATUS (NTAPI *PFN_LDR_UNLOAD_DLL)(HMODULE hModule);
// LdrGetDllHandle
typedef NTSTATUS (NTAPI *PFN_LDR_GET_DLL_HANDLE)(PWORD path, PVOID unused, PUNICODE_STRING moduleFileName, HMODULE *pHModule);
// LdrGetProcedureAddress
typedef NTSTATUS (NTAPI *PFN_LDR_GET_PROCEDURE_ADDRESS)(HMODULE hModule, PANSI_STRING functionName, WORD ordinal, LPVOID *pFunction);
// NtClose
typedef NTSTATUS (NTAPI *PFN_NT_CLOSE)(HANDLE objectHandle);
// NtAllocateVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_ALLOCATE_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID *pBaseAddress, ULONG zeroBits, PULONG pRegionSize, ULONG allocationType, ULONG protect);
// NtFreeVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_FREE_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID *pBaseAddress, ULONG *regionSize, ULONG freeType);
// NtQueryVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_QUERY_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID pBaseAddress, ULONG infoClass, MEMORY_BASIC_INFORMATION *pMbi, ULONG length, PULONG pResultLength);
// NtQueryInformationProcess
typedef NTSTATUS (NTAPI *PFN_NT_QUERY_INFORMATION_PROCESS)(HANDLE hProcess, PROCESS_INFO_CLASS infoClass, LPVOID pBuffer, ULONG bufferLength, ULONG *pReturnLength);
// NtQueryInformationThread
typedef NTSTATUS (NTAPI *PFN_NT_QUERY_INFORMATION_THREAD)(HANDLE hProcess, THREAD_INFO_CLASS infoClass, LPVOID pBuffer, ULONG bufferLength, ULONG *pReturnLength);
// NtProtectVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_PROTECT_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID *pBaseAddress, PULONG_PTR numberOfBytesToProtect, ULONG newAccessProtection, PULONG oldAccessProtection);
// NtReadVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_READ_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID pBaseAddress, LPVOID pBuffer, ULONG numberOfBytesToRead, PULONG pNumberOfBytesRead);
// NtWriteVirtualMemory
typedef NTSTATUS (NTAPI *PFN_NT_WRITE_VIRTUAL_MEMORY)(HANDLE hProcess, LPVOID pBaseAddress, LPVOID pBuffer, ULONG numberOfBytesToWrite, PULONG pNumberOfBytesWritten);
// NtOpenSection
typedef NTSTATUS (NTAPI *PFN_NT_OPEN_SECTION)(HANDLE *phSectionHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES pObjectAttributes);
// NtMapViewOfSection
typedef NTSTATUS (NTAPI *PFN_NT_MAP_VIEW_OF_SECTION)(HANDLE hSection, HANDLE hProcess, LPVOID pBaseAddress, ULONG zeroBits, ULONG commitSize, PLARGE_INTEGER pSectionOffset, PULONG pViewSize, SECTION_INHERIT inheritDisposition, ULONG allocationType, ULONG protect);
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
typedef int (NTAPI *PFN_RTL_CREATE_USER_THREAD)(HANDLE hProcess, SECURITY_DESCRIPTOR *pSecurityDescriptor, BOOL createSuspended, DWORD stackZeroBits, DWORD stackReserved, DWORD stackCommit, LPVOID pStartAddress, LPVOID pParameters, HANDLE *threadHandle, __int64 *clientId);
// RtlValidSid
typedef BOOLEAN (NTAPI *PFN_RTL_VALID_SID)(PSID pSid);
// RtlEqualSid
typedef BOOL (NTAPI *PFN_RTL_EQUAL_SID)(PSID pSid1, PSID pSid2);
// NtLoadDriver
typedef NTSTATUS (NTAPI *PFN_NT_LOAD_DRIVER)(PUNICODE_STRING name);
// NtUnloadDriver
typedef NTSTATUS (NTAPI *PFN_NT_UNLOAD_DRIVER)(PUNICODE_STRING name);

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
EXTERN PFN_WAIT_FOR_MULTIPLE_OBJECTS pfnWaitForMultipleObjects __NULL;
EXTERN PFN_WAIT_FOR_SINGLE_OBJECT pfnWaitForSingleObject __NULL;
EXTERN PFN_SLEEP pfnSleep __NULL;
EXTERN PFN_VIRTUAL_QUERY pfnVirtualQuery __NULL;
EXTERN PFN_LOCAL_ALLOC pfnLocalAlloc __NULL;
EXTERN PFN_LOCAL_FREE pfnLocalFree __NULL;
EXTERN PFN_SET_LAST_ERROR pfnSetLastError __NULL;
EXTERN PFN_RELEASE_MUTEX pfnReleaseMutex __NULL;
EXTERN PFN_IS_WOW64_PROCESS pfnIsWow64Process __NULL;

#endif