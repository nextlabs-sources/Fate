// ***************************************************************
//  ProcessList.h             version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  this list stores information about known processes
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-01-10 1.0.0 initial release

#ifndef _ProcessList_
#define _ProcessList_

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

// ********************************************************************

// add a process to the list of known processes
BOOLEAN AddKnownProcess (HANDLE ProcessId, UCHAR *Hash);

// remove a known process from the list
BOOLEAN DelKnownProcess (HANDLE ProcessId);

// find a known process and return the hash
BOOLEAN FindKnownProcess (HANDLE ProcessId, UCHAR *Hash);

// free the whole process list - used for driver shutdown
void FreeProcessList (void);

// ********************************************************************

#endif
