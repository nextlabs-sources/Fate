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

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

#include "DelayedInjectList.h"

// ********************************************************************

typedef struct _DelayedInjectProcess {
    HANDLE ProcessId;
    BOOLEAN IsWow64Process;
} DelayedInjectProcess, *PDelayedInjectProcess;

// list management
ULONG Count = 0;
ULONG Capacity = 0;
DelayedInjectProcess *List = NULL;
FAST_MUTEX Mutex;

// ********************************************************************

BOOLEAN AddDelayedInjectProcess(HANDLE ProcessId, BOOLEAN IsWow64Process)
// add a process to our list
{
  ExAcquireFastMutex(&Mutex);
  __try
  {

    // first let's check whether this process is already in the list
    int i1;
    for (i1 = 0; i1 < (int) Count; i1++)
      if (List[i1].ProcessId == ProcessId)
      {
        // it's already in the list, so we do nothing
        List[i1].IsWow64Process = IsWow64Process;
        return TRUE;
      }

    // we have a new process

    if (Count == Capacity)
    {
      // the process list is either full or not initialized yet
      // so we need to do some (re)allocation work
      DelayedInjectProcess *newList;
      if (Capacity)
        Capacity = Capacity * 2;
      else
        Capacity = 64;
      newList = ExAllocatePool(PagedPool, Capacity * sizeof(DelayedInjectProcess));
      if (newList)
      {
        RtlZeroMemory(newList, Capacity * sizeof(DelayedInjectProcess));
        if (Count)
          memcpy(newList, List, Count * sizeof(DelayedInjectProcess));
      }
      else
      {
        Count = 0;
        Capacity = 0;
      }
      if (List)
        ExFreePool(List);
      List = newList;
    }

    // finally let's store the process into the list
    if (List)
    {
      List[Count].ProcessId = ProcessId;
      List[Count].IsWow64Process = IsWow64Process;
      Count++;
      return TRUE;
    }

  }
  __finally
  {
    ExReleaseFastMutex(&Mutex);
  }

  return FALSE;
}

// ********************************************************************

BOOLEAN DelDelayedInjectProcess(HANDLE ProcessId)
// remove the specified process from our list
{
  ExAcquireFastMutex(&Mutex);
  __try
  {

    int i1;
    for (i1 = 0; i1 < (int) Count; i1++)
      if (List[i1].ProcessId == ProcessId)
      {
        // we found the record we're searching for, so let's remove it from the list
        int i2;
        Count--;
        for (i2 = i1; i2 < (int) Count; i2++)
        {
          List[i2].ProcessId = List[i2 + 1].ProcessId;
          List[i2].IsWow64Process = List[i2 + 1].IsWow64Process;
        }
        List[Count].ProcessId = NULL;
        List[Count].IsWow64Process = FALSE;
        return TRUE;
      }

  }
  __finally
  {
    ExReleaseFastMutex(&Mutex);
  }

  return FALSE;
}

// ********************************************************************

BOOLEAN IsDelayedInjectProcess(HANDLE ProcessId, BOOLEAN *IsWow64Process)
// is the specified process contained in our list?
{
  ExAcquireFastMutex(&Mutex);
  __try
  {

    int i1;
    for (i1 = 0; i1 < (int) Count; i1++)
      if (List[i1].ProcessId == ProcessId)
      {
        // yes - it is!
        if (IsWow64Process)
          *IsWow64Process = List[i1].IsWow64Process;
        return TRUE;
      }

  }
  __finally
  {
    ExReleaseFastMutex(&Mutex);
  }

  return FALSE;
}

// ********************************************************************

void InitDelayedInjectList(void)
// initialize the delayed inject list - called from driver initialization
{
  ExInitializeFastMutex(&Mutex);
}

void FreeDelayedInjectList(void)
// free the delayed inject list - used for driver shutdown
{
  if (List)
  {
    ExFreePool(List);
    List = NULL;
  }
  ExInitializeFastMutex(&Mutex);
}

// ********************************************************************
