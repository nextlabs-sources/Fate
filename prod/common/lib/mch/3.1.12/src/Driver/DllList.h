// ***************************************************************
//  DllList.h                 version: 1.0.1  ·  date: 2013-01-09
//  -------------------------------------------------------------
//  this list contains all active injection requests
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2013 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2013-01-09 1.0.1 added InitDllList()
// 2010-01-10 1.0.0 initial release

#ifndef _DllList_
#define _DllList_

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

// ********************************************************************

#pragma pack(push, 1)

// for IOCTL_INJECT_DLL and IOCTL_UNINJECT_DLL
typedef struct _DllItem {
    ULONG   Size;            // size of the data structure
    UCHAR   Hash [20];       // hash of the data following this field
    UCHAR   OwnerHash [20];  // hash of the exe file which called the driver
    ULONG   Session;         // in which session shall the dll be injected? ("-1" = all sessions)
    ULONG   Flags;           // 0x01 = inject into system processes, too
    ULONG   X86AllocAddr;    // at which user mode address to allocate memory for x86 processes?

      // fields for temporary use in driver only:
      HANDLE  FileHandle;       // dll file handle is kept open to block renaming/deleting
      LPWSTR  *IncPathBuf;      //                 -
      LPWSTR  *IncNameBuf;      //                 |
      LPWSTR  *ExcPathBuf;      //                 |
      LPWSTR  *ExcNameBuf;      //                 |
      int     *IncPathLen;      // helpers to make file path matching quick
      int     *IncNameLen;      //                 |
      int     *ExcPathLen;      //                 |
      int     *ExcNameLen;      //                 |
      int     IncCount;         //                 |
      int     ExcCount;         //                 -

      #ifndef _WIN64
        ULONG   Dummy[9];       // this makes sizeof(DllItem) constant for 32bit and 64bit
      #endif

    WCHAR   Name [260];      // full dll file path/name
    ULONG   IncludeLen;      // file mask: into which processes shall the dll be injected?
    ULONG   ExcludeLen;      // file mask: which processes shall be excluded from injection?
    WCHAR   Data [1];
} DllItem, *PDllItem;

#pragma pack(pop)

// ********************************************************************

// add a dll injection request to our list, if possible
BOOLEAN AddDll (PDllItem Dll);

// remove the specified dll injection request from our list, if it's in there
BOOLEAN DelDll (PDllItem Dll);

// remove all dlls which belong to the specified session
BOOLEAN DelSessionDlls (ULONG Session);

// enumerate the items of the list
BOOLEAN EnumDlls (int Index, PDllItem *Dll);

// check whether the specified dll should be injected into the specified process
BOOLEAN CheckDll (DllItem *Dll, LPWSTR PathBuf, LPWSTR NameBuf, int PathLen, int NameLen);

// initialize the dll list - called from driver initialization
void InitDllList (void);

// free the dll list - used for driver shutdown
void FreeDllList (void);

// ********************************************************************

#endif
