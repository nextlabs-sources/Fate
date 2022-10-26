// ***************************************************************
//  Inject.h                  version: 1.0.1  ·  date: 2010-03-26
//  -------------------------------------------------------------
//  injection into already running processes
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-03-26 1.0.1 small change to make (un)injection slightly more stable
// 2010-01-10 1.0.0 initial version

#ifndef _INJECT_H
#define _INJECT_H

// ----------------------- TYPES ------------------------------------------------------------------

const __int64 CInjectMagic = 0x77712345abcde777;  // magic number for our buffer

typedef void (__cdecl *PFN_AUTO_UNHOOK)(HMODULE module);

#pragma pack(1)

typedef wchar_t DLL_DRIVER_INJECTION[MAX_PATH];

typedef struct tagInjectDll
{
  wchar_t Name[MAX_PATH];
  wchar_t Owner[MAX_PATH];
  BYTE Sid[91];
  BYTE System;
  __int64 Reserved;
  DWORD Session;
} INJECT_DLL;

typedef struct tagInjectDllsRec
{
  char Name[8];
  int Count;
  INJECT_DLL Items[0x100000];
} INJECT_DLLS_REC;

#ifdef _WIN64
  typedef struct tagInjectRec32
  {
    char Load;
    char LibraryNameA[MAX_PATH + 11];
    WCHAR LibraryNameW[MAX_PATH + 11];
    PVOID32 pfnGetVersion;
    PVOID32 pfnSharedMem9x_Free;
    PVOID32 pfnVirtualFree;
    PVOID32 pfnSetErrorMode;
    PVOID32 pfnLoadLibraryA;
    PVOID32 pfnLoadLibraryW;
    PVOID32 pfnGetLastError;
    PVOID32 pfnGetModuleHandleA;
    PVOID32 pfnGetModuleHandleW;
    PVOID32 pfnGetCurrentProcessId;
    PVOID32 pfnOpenFileMappingA;
    PVOID32 pfnMapViewOfFile;
    PVOID32 pfnUnmapViewOfFile;
    PVOID32 pfnCloseHandle;
    PVOID32 pfnFreeLibrary;
    PVOID32 pfnEnterCriticalSection;
    PVOID32 pfnLeaveCriticalSection;
  } INJECT_REC32;
#endif

typedef struct tagInjectRec
{
  char Load;
  char LibraryNameA[MAX_PATH + 11];
  WCHAR LibraryNameW[MAX_PATH + 11];
  PFN_GET_VERSION pfnGetVersion;
  PVOID pfnSharedMem9x_Free;
  PFN_VIRTUAL_FREE pfnVirtualFree;
  PFN_SET_ERROR_MODE pfnSetErrorMode;
  PVOID pfnLoadLibraryA;
  PFN_LOAD_LIBRARY_W pfnLoadLibraryW;
  PFN_GET_LAST_ERROR pfnGetLastError;
  PVOID pfnGetModuleHandleA;
  PFN_GET_MODULE_HANDLE_W pfnGetModuleHandleW;
  PFN_GET_CURRENT_PROCESS_ID pfnGetCurrentProcessId;
  PFN_OPEN_FILE_MAPPING_A pfnOpenFileMappingA;
  PFN_MAP_VIEW_OF_FILE pfnMapViewOfFile;
  PFN_UNMAP_VIEW_OF_FILE pfnUnmapViewOfFile;
  PFN_CLOSE_HANDLE pfnCloseHandle;
  PFN_FREE_LIBRARY pfnFreeLibrary;
  PFN_ENTER_CRITICAL_SECTION pfnEnterCriticalSection;
  PFN_LEAVE_CRITICAL_SECTION pfnLeaveCriticalSection;
} INJECT_REC;

#ifdef _WIN64
  typedef struct tagMchIInjT32
  {
    char Name[8];
    DWORD ProcessId;
    PVOID32 InjThread;
  } MCH_I_INJ_T32;
#endif

typedef struct tagMchIInjT
{
  char Name[8];
  DWORD ProcessId;
  LPVOID InjThread;
} MCH_I_INJ_T;

typedef wchar_t LOAD_LIBRARY_ITEM[MAX_PATH];

struct tagLoadLibraries;
typedef tagLoadLibraries *PLOAD_LIBRARIES;

#ifdef _WIN64
typedef struct tagLoadLibrariesHeader
{
  WORD movRax;
  LPVOID retAddr;
  BYTE pushRax;
  BYTE pushRcx;
  BYTE pushRdx;
  WORD pushR8;
  WORD pushR9;
  DWORD subRsp28;
  WORD movRcx;
  PLOAD_LIBRARIES param;
  WORD movRdx;
  LPVOID proc;
  WORD jmpEdx;
  __int64 magic;
  int capacity;
  int count;
  OPCODE_DWORD *pOldApi;
  OPCODE_DWORD oldApi;
} LOAD_LIBRARIES_HEADER;
#else
typedef struct tagLoadLibrariesHeader
{
  BYTE movEax;
  PLOAD_LIBRARIES param;
  BYTE movEcx;
  LPVOID proc;
  WORD callEcx;
  __int64 magic;
  int capacity;
  int count;
  OPCODE_DWORD *pOldApi;
  OPCODE_DWORD oldApi;
} LOAD_LIBRARIES_HEADER;
#endif

typedef struct tagLoadLibraries
{
  LOAD_LIBRARIES_HEADER header;
  LOAD_LIBRARY_ITEM items[0x100000];
} LOAD_LIBRARIES, *PLOAD_LIBRARIES;

#pragma pack()

#undef EXTERN
#ifdef _INJECT_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

#undef __NULL
#ifdef _INJECT_C
  #define __NULL = NULL
#else
  #define __NULL
#endif

EXTERN PFN_LDR_LOAD_DLL pfnLdrLoadDll __NULL;
EXTERN PFN_NT_PROTECT_VIRTUAL_MEMORY pfnNtProtectVirtualMemory __NULL;

SYSTEMS_API BOOL WINAPI CheckLibFilePath(LPCWSTR libraryName, SString& libraryPath);

SYSTEMS_API BOOL InternalUninjectLibrary(HMODULE hOwner, LPCWSTR libraryName, bool multiInject, HANDLE hProcess, LPCWSTR driverName, DWORD session, BOOL system, LPCWSTR includes, LPCWSTR excludes, PULONG excludePIDs, DWORD timeOut);

#endif