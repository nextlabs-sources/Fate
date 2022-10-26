// ***************************************************************
//  DelayedInjectList.h       version: 1.1.0  ·  date: 2014-03-21
//  -------------------------------------------------------------
//  list of processes which still need injection
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2014 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2014-03-21 1.1.0 (1) renamed to "DelayedInjectList.h/cpp"
//                  (2) now used for 64bit processes, too
// 2010-06-17 1.0.0 initial release

#ifndef _DelayedInjectList_
#define _DelayedInjectList_

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

// ********************************************************************

// add a process to our list
BOOLEAN AddDelayedInjectProcess (HANDLE ProcessId, BOOLEAN IsWow64Process);

// remove the specified process from our list
BOOLEAN DelDelayedInjectProcess (HANDLE ProcessId);

// is the specified process contained in our list?
BOOLEAN IsDelayedInjectProcess (HANDLE ProcessId, BOOLEAN *IsWow64Process);

// initialize the delayed inject list - called from driver initialization
void InitDelayedInjectList (void);

// free the delayed inject list - used for driver shutdown
void FreeDelayedInjectList (void);

// ********************************************************************

#endif
