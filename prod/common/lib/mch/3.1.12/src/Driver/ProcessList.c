// ***************************************************************
//  ProcessList.c             version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  this list stores information about known processes
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-01-10 1.0.0 initial release

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

// ********************************************************************
// we store a hash of the exe file of every process which has contacted the driver
// this is supposed to speed up performance

typedef struct _KnownProcess {
    HANDLE ProcessId;
    UCHAR  Hash [20];
} KnownProcess, *PKnownProcess;

// process list management
ULONG KPCount = 0;
ULONG KPCapacity = 0;
KnownProcess *KPs = NULL;

// ********************************************************************

BOOLEAN AddKnownProcess(HANDLE ProcessId, UCHAR *Hash)
// add a process to the list of known processes
{
  int i1;
  BOOLEAN result = FALSE;

  for (i1 = 0; i1 < (int) KPCount; i1++)
  {
    if (KPs[i1].ProcessId == ProcessId)
    {
      // this item is already in the list
      // the hash should match, but we refresh it nevertheless, just in case
      memcpy(KPs[i1].Hash, Hash, 20);
      return TRUE;
    }
  }

  if (KPCount == KPCapacity)
  {
    // the list is either not initialized yet or full
    // so we need to do some (re)allocation work
    KnownProcess *newList;
    if (KPCapacity)
      KPCapacity = KPCapacity * 2;
    else
      KPCapacity = 16;
    newList = ExAllocatePool(PagedPool, KPCapacity * sizeof(KnownProcess));
    if (newList)
    {
      RtlZeroMemory(newList, KPCapacity * sizeof(KnownProcess));
      if (KPCount)
        memcpy(newList, KPs, KPCount * sizeof(KnownProcess));
    } else {
      KPCount = 0;
      KPCapacity = 0;
    }
    if (KPs)
      ExFreePool(KPs);
    KPs = newList;
  }

  // finally let's add the known process to the list
  if (KPs)
  {
    KPs[KPCount].ProcessId = ProcessId;
    memcpy(KPs[KPCount].Hash, Hash, 20);
    KPCount++;

    return TRUE;
  }

  return FALSE;
}

// ********************************************************************

BOOLEAN DelKnownProcess(HANDLE ProcessId)
// remove a known process from the list
// this is only ever done when a process has closed down
{
  int i1;

  for (i1 = 0; i1 < (int) KPCount; i1++)
  {
    if (KPs[i1].ProcessId == ProcessId)
    {
      // we found the item we're searching for, let's remove it from the list
      int i2;
      KPCount--;
      for (i2 = i1; i2 < (int) KPCount; i2++)
        KPs[i2] = KPs[i2 + 1];
      RtlZeroMemory(&KPs[KPCount], sizeof(KnownProcess));
      return TRUE;
    }
  }

  return FALSE;
}

// ********************************************************************

BOOLEAN FindKnownProcess(HANDLE ProcessId, UCHAR *Hash)
// find a known process and return the hash
{
  int i1;

  for (i1 = 0; i1 < (int) KPCount; i1++)
  {
    if (KPs[i1].ProcessId == ProcessId)
    {
      // we found the item we're searching for
      memcpy(Hash, KPs[i1].Hash, 20);
      return TRUE;
    }
  }
  return FALSE;
}

// ********************************************************************

void FreeProcessList(void)
// free the whole process list - used for driver shutdown
{
  if (KPs)
  {
    ExFreePool(KPs);
    KPs = NULL;
  }
}

// ********************************************************************
