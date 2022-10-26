// ***************************************************************
//  SessionList.h             version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  this list stores information about known sessions
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-01-10 1.0.0 initial release

#ifndef _SessionList_
#define _SessionList_

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

// ********************************************************************

// initialize the session count information of a specific session
void InitSessionCount (ULONG Session);

// update the session count when a new process is created
void IncSessionCount (ULONG Session);

// update the session count when a process is destroyed
// "true" is returned if the session has closed down
BOOLEAN DecSessionCount (ULONG Session);

// free the whole session list - used for driver shutdown
void FreeSessionList (void);

// ********************************************************************

#endif
