// ***************************************************************
//  ProcessTools.h            version: 1.0.0   date: 2010-01-10
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
#endif

EXTERN void UnpatchCreateRemoteThread(void);
EXTERN bool CheckDllName(LPCWSTR subStr, LPCWSTR str);
EXTERN BOOL GetThreadSecurityAttributes(SECURITY_ATTRIBUTES *pSecurityAttributes);
EXTERN void GetOtherThreadHandles(CCollection<HANDLE>& threads);
// EXTERN BOOL GetProcessSid(HANDLE hProcess, SID_AND_ATTRIBUTES **ppSidAndAttributes);

#endif