// ***************************************************************
//  InjectLibrary.h           version: 1.0.1  ·  date: 2010-11-18
//  -------------------------------------------------------------
//  inject a dll into a specific 32bit or 64bit process
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-11-18 1.0.1 (1) exported function "Set32bitNtdllInfo" added
//                  (2) SetInjectionMethod API added
// 2010-01-10 1.0.0 initial release

#ifndef _InjectLibrary_
#define _InjectLibrary_

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

// ********************************************************************

// initialize some undocumented stuff
BOOLEAN InitInjectLibrary (HANDLE ProcessId);

#ifdef _WIN64
  // initialize 32bit ntdll information
  void Set32bitNtdllInfo (HANDLE ModuleHandle, PVOID AddressOfEntryPoint, PVOID NtProtectVirtualMemory, PVOID LdrLoadDll, PVOID NtAllocateVirtualMemory, PVOID NtFreeVirtualMemory, PVOID NtQuerySystemInformation, PVOID NtDelayExecution, PVOID NtTestAlert);
#endif

// ********************************************************************

// set the injection method used by the kernel mode driver
void SetInjectionMethod (BOOLEAN NewInjectionMethod);

// ********************************************************************

// inject a dll into a newly started 32bit or 64bit process
BOOLEAN InjectLibrary (HANDLE ProcessHandle, PWCHAR LibraryFileName);

// ********************************************************************

#endif
