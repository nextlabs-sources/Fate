// ***************************************************************
//  SessionList.c             version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  this list stores information about known sessions
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-01-10 1.0.0 initial release

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

#include "ToolFuncs.h"

// ********************************************************************
// we store the number of processes that are running in each session
// this helps us detecting when a session goes down

// session list management
ULONG SessionCapacity = 0;
LONG *Sessions = NULL;

// ********************************************************************

void InitSessionCount(ULONG Session)
// initialize the session count information of a specific session
{
  if (Session >= SessionCapacity)
  {
    // the session list array is either not initialized yet or too small
    // so we need to do some (re)allocation work
    ULONG oldCapacity = SessionCapacity;
    LONG *newList;
    if (SessionCapacity)
    {
      while (Session >= SessionCapacity)
        SessionCapacity = SessionCapacity * 2;
    } else
      SessionCapacity = 512;
    newList = ExAllocatePool(PagedPool, SessionCapacity * sizeof(LONG));
    if (newList)
    {
      RtlZeroMemory(newList, SessionCapacity * sizeof(LONG));
      if (oldCapacity)
        memcpy(newList, Sessions, oldCapacity * sizeof(LONG));
    } else
      SessionCapacity = 0;
    if (Sessions)
      ExFreePool(Sessions);
    Sessions = newList;
  }

  if ((Sessions) && (!Sessions[Session]))
    Sessions[Session] = CountProcesses(Session);
}

// ********************************************************************

void IncSessionCount(ULONG Session)
// update the session count when a new process is created
{
  if ((Sessions) && (Session < SessionCapacity) && (Sessions[Session]))
    InterlockedIncrement(&Sessions[Session]);
}

// ********************************************************************

BOOLEAN DecSessionCount(ULONG Session)
// update the session count when a process is destroyed
// "true" is returned if the session the process belonged to has closed down
{
  if ( (Sessions) && (Session < SessionCapacity) && (Sessions[Session]) &&
       (InterlockedDecrement(&Sessions[Session]) == 0) )
  {
    ULONG count = CountProcesses(Session);
    if (count <= 1)
      count = 0;
    Sessions[Session] = count;
    return (count == 0);
  }

  return FALSE;
}

// ********************************************************************

void FreeSessionList(void)
// free the whole session list - used for driver shutdown
{
  if (Sessions)
  {
    ExFreePool(Sessions);
    Sessions = NULL;
  }
}

// ********************************************************************
