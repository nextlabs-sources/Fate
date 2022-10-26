// ***************************************************************
//  DllList.c                 version: 1.0.3  ·  date: 2013-03-13
//  -------------------------------------------------------------
//  this list contains all active injection requests
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2013 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2013-03-13 1.0.3 added critical section etc to improve thread safety
// 2012-10-02 1.0.2 fixed: Verifier blue screens when using ex/include lists
// 2010-03-25 1.0.1 fixed: upper case incl/excl lists made problems
// 2010-01-10 1.0.0 initial release

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

#include "ToolFuncs.h"
#include "Config.h"
#include "DllList.h"
#include "StrMatch.h"

// ********************************************************************

// dll list management
ULONG DllCount = 0;
ULONG DllCapacity = 0;
PDllItem *DllList = NULL;
CRIT_SECTION Section;

// ********************************************************************

BOOLEAN AddDll(PDllItem Dll)
// add a dll injection request to our list, if possible
{
  EnterCriticalSection(&Section);
  __try
  {

    int i1;
    UCHAR hash [20];
    ULONG fileSize;
    BOOLEAN result = FALSE;

    // first let's check whether this injection request is already in the list
    for (i1 = 0; i1 < (int) DllCount; i1++)
    {
      if ( (DllList[i1]->Size == Dll->Size) &&
           (DllList[i1]->Session == Dll->Session) &&
           ((DllList[i1]->Flags & 0x7fffffff) == (Dll->Flags & 0x7fffffff)) &&
           (DllList[i1]->IncludeLen == Dll->IncludeLen) &&
           (DllList[i1]->ExcludeLen == Dll->ExcludeLen) &&
           (!memcmp(DllList[i1]->OwnerHash, Dll->OwnerHash, 20)) &&
           (!memcmp(DllList[i1]->Name, Dll->Name, 260 * 2)) &&
           (!memcmp(DllList[i1]->Data, Dll->Data, Dll->IncludeLen * 2 + 2 + Dll->ExcludeLen * 2 + 2)) )
      {
        // it's already in the list, so we do nothing
        ExFreePool(Dll);
        return TRUE;
      }
    }
    Dll->FileHandle = (HANDLE) -1;

    // we have a new injection request

    // let's first check whether the requested dll is authorized for injection
    if (GetFileHash(Dll->Name, &hash, 0, &fileSize, &Dll->FileHandle))
    {
      int i1 = 0;
      ULONG authFileSize;
      PULONG authHash;

      while (EnumAuthorizedDlls(i1, &authFileSize, &authHash))
      {
        if ((fileSize == authFileSize) && (!memcmp(hash, authHash, 20)))
        {
          // this dll is known to the driver, so everything is fine
          result = TRUE;
          break;
        }
        i1++;
      }

      if (!result)
        // this dll is not known to the driver - sorry!
        ZwClose(Dll->FileHandle);
    }

    if (!result)
      return FALSE;

    memcpy(Dll->Hash, hash, 20);

    #ifdef _WIN64
      // check if the to-be-injected dll is a 32bit or 64bit dll
      if (Is64bitDll(Dll->FileHandle))
        Dll->Flags = Dll->Flags | 0x80000000;
      else
    #endif
      Dll->Flags = Dll->Flags & 0x7fffffff;

    if (DllCount == DllCapacity)
    {
      // the injection list is either full or not initialized yet
      // so we need to do some (re)allocation work
      PDllItem *newList;
      if (DllCapacity)
        DllCapacity = DllCapacity * 2;
      else
        DllCapacity = 16;
      newList = ExAllocatePool(PagedPool, DllCapacity * sizeof(PVOID));
      if (newList)
      {
        RtlZeroMemory(newList, DllCapacity * sizeof(PVOID));
        if (DllCount)
          memcpy(newList, DllList, DllCount * sizeof(PVOID));
      } else {
        DllCount = 0;
        DllCapacity = 0;
      }
      if (DllList)
        ExFreePool(DllList);
      DllList = newList;
    }

    // finally let's store the injection request into the list
    if (DllList)
    {

      // but first we need to parse and split out the includes/excludes
      if (Dll->IncludeLen)
        // lower case makes file matching easier
        _wcslwr((LPWSTR) &Dll->Data[0]);
      SplitStrArray((LPWSTR) &Dll->Data[0], Dll->IncludeLen, &Dll->IncPathBuf, &Dll->IncNameBuf, &Dll->IncPathLen, &Dll->IncNameLen, &Dll->IncCount);
      if (Dll->ExcludeLen)
        _wcslwr((LPWSTR) &Dll->Data[Dll->IncludeLen + 1]);
      SplitStrArray((LPWSTR) &Dll->Data[Dll->IncludeLen + 1], Dll->ExcludeLen, &Dll->ExcPathBuf, &Dll->ExcNameBuf, &Dll->ExcPathLen, &Dll->ExcNameLen, &Dll->ExcCount);

      DllList[DllCount++] = Dll;
      return TRUE;
    }
    else
    {
      ZwClose(Dll->FileHandle);
      return FALSE;
    }

  }
  __finally
  {
    LeaveCriticalSection(&Section);
  }
}

// ********************************************************************

BOOLEAN DelDll(PDllItem Dll)
// remove the specified dll injection request from our list, if it's in there
{
  EnterCriticalSection(&Section);
  __try
  {

    int i1;

    // we lower cased the include/exclude lists in AddDll, so we do that here, too
    if (Dll->IncludeLen)
      _wcslwr((LPWSTR) &Dll->Data[0]);
    if (Dll->ExcludeLen)
      _wcslwr((LPWSTR) &Dll->Data[Dll->IncludeLen + 1]);

    for (i1 = 0; i1 < (int) DllCount; i1++)
    {
      if ( (DllList[i1]->Size == Dll->Size) &&
           (DllList[i1]->Session == Dll->Session) &&
           ((DllList[i1]->Flags & 0x7fffffff) == (Dll->Flags & 0x7fffffff)) &&
           (DllList[i1]->IncludeLen == Dll->IncludeLen) &&
           (DllList[i1]->ExcludeLen == Dll->ExcludeLen) &&
           (!memcmp(DllList[i1]->OwnerHash, Dll->OwnerHash, 20)) &&
           (!memcmp(DllList[i1]->Name, Dll->Name, 260 * 2)) &&
           (!memcmp(DllList[i1]->Data, Dll->Data, Dll->IncludeLen * 2 + 2 + Dll->ExcludeLen * 2 + 2)) )
      {
        // we found the record we're searching for, so let's remove it from the list
        int i2;
        ZwClose(DllList[i1]->FileHandle);
        if (DllList[i1]->IncPathBuf) ExFreePool(DllList[i1]->IncPathBuf);
        if (DllList[i1]->IncNameBuf) ExFreePool(DllList[i1]->IncNameBuf);
        if (DllList[i1]->IncPathLen) ExFreePool(DllList[i1]->IncPathLen);
        if (DllList[i1]->IncNameLen) ExFreePool(DllList[i1]->IncNameLen);
        if (DllList[i1]->ExcPathBuf) ExFreePool(DllList[i1]->ExcPathBuf);
        if (DllList[i1]->ExcNameBuf) ExFreePool(DllList[i1]->ExcNameBuf);
        if (DllList[i1]->ExcPathLen) ExFreePool(DllList[i1]->ExcPathLen);
        if (DllList[i1]->ExcNameLen) ExFreePool(DllList[i1]->ExcNameLen);
        ExFreePool(DllList[i1]);
        DllCount--;
        for (i2 = i1; i2 < (int) DllCount; i2++)
          DllList[i2] = DllList[i2 + 1];
        DllList[DllCount] = NULL;
        return TRUE;
      }
    }

  }
  __finally
  {
    LeaveCriticalSection(&Section);
  }

  return FALSE;
}

// ********************************************************************

BOOLEAN DelSessionDlls(ULONG Session)
// remove all dlls which belong to the specified session
{
  BOOLEAN result = FALSE;

  EnterCriticalSection(&Section);
  __try
  {

    int i1;
    for (i1 = DllCount - 1; i1 >= 0; i1--)
      if ((DllList[i1]->Session == Session) && (DelDll(DllList[i1])))
        result = TRUE;

  }
  __finally
  {
    LeaveCriticalSection(&Section);
  }

  return result;
}

// ********************************************************************

BOOLEAN EnumDlls(int Index, PDllItem *Dll)
// enumerate the items of the list
{
  EnterCriticalSection(&Section);
  __try
  {

    if ((Index >= 0) && ((ULONG) Index < DllCount))
    {
      if (Dll)
      {
        *Dll = ExAllocatePool(PagedPool, DllList[Index]->Size);
        memcpy(*Dll, DllList[Index], DllList[Index]->Size);
      }
      return TRUE;
    }

  }
  __finally
  {
    LeaveCriticalSection(&Section);
  }

  return FALSE;
}

// ********************************************************************

BOOLEAN CheckDll(DllItem *Dll, LPWSTR PathBuf, LPWSTR NameBuf, int PathLen, int NameLen)
{
  EnterCriticalSection(&Section);
  __try
  {
    int i1;
    for (i1 = 0; i1 < (int) DllCount; i1++)
      if ((Dll->Size == DllList[i1]->Size) && (!memcmp(Dll, DllList[i1], Dll->Size)))
      {
        return ((!Dll->IncCount) || ( MatchStrArray(PathBuf, NameBuf, PathLen, NameLen, Dll->IncCount, Dll->IncPathBuf, Dll->IncNameBuf, Dll->IncPathLen, Dll->IncNameLen))) &&
               ((!Dll->ExcCount) || (!MatchStrArray(PathBuf, NameBuf, PathLen, NameLen, Dll->ExcCount, Dll->ExcPathBuf, Dll->ExcNameBuf, Dll->ExcPathLen, Dll->ExcNameLen)));
        break;
      }
  }
  __finally
  {
    LeaveCriticalSection(&Section);
  }
  return FALSE;
}

// ********************************************************************

void InitDllList(void)
// initialize the dll list - called from driver initialization
{
  InitializeCriticalSection(&Section);
}

void FreeDllList(void)
// free the dll list - used for driver shutdown
{
  EnterCriticalSection(&Section);
  __try
  {

    while (DllCount)
      DelDll(DllList[0]);
    if (DllList)
    {
      ExFreePool(DllList);
      DllList = NULL;
    }

  }
  __finally
  {
    LeaveCriticalSection(&Section);
  }
}

// ********************************************************************
