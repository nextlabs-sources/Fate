// ***************************************************************
//  ProcessTools.h            version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  functions that work against processes
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _PROCESSTOOLS_H
#define _PROCESSTOOLS_H

#ifndef _CCOLLECTION_H
  #include "CCollection.h"
#endif

#undef EXTERN
#ifdef _PROCESSTOOLS_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

#ifdef _WIN64
  EXTERN bool Init32bitKernelAPIs(HANDLE hProcess);
  EXTERN PVOID GetKernelAPI(ULONG index);
  EXTERN HMODULE GetRemoteModuleHandle64(HANDLE ProcessHandle, LPCWSTR DllName);
  EXTERN HMODULE GetRemoteModuleHandle32(HANDLE ProcessHandle, LPCWSTR DllName, LPWSTR DllFullPath);
  EXTERN bool IsProcessLargeAddressAware(HANDLE hProcess);
  EXTERN PVOID LargeAddressAwareApiTo2GB(HANDLE hProcess, PVOID largeApi);
#endif

EXTERN bool IsProcessDotNet(HANDLE hProcess);
EXTERN void UnpatchCreateRemoteThread(void);
EXTERN bool CheckDllName(LPCWSTR subStr, LPCWSTR str);
EXTERN BOOL GetThreadSecurityAttributes(SECURITY_ATTRIBUTES *pSecurityAttributes);
EXTERN void GetOtherThreadHandles(CCollection<HANDLE>& threads);
EXTERN HANDLE OpenFirstThread(DWORD pid);
// EXTERN BOOL GetProcessSid(HANDLE hProcess, SID_AND_ATTRIBUTES **ppSidAndAttributes);

#endif