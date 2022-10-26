// ***************************************************************
//  Config.c                  version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  basic driver configuration (initialized at build time)
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

#pragma pack(push, 1)

// this structure is initialized by the command line tool "madConfigDrv.exe"
typedef struct _CONFIG {
    ULONG Magic [4];          // used by "madConfigDrv.exe" to locate this structure
    ULONG Flags;              // 0x01: unloading the driver is generally supported
    ULONG Reserved;           // not used yet
    CHAR  Name [40];          // name of the driver, used for communication with user land
    ULONG DllCount;           // how many dlls are known to the driver?
    ULONG DllData [40 * 6];   // storage space for max 40 different hook dlls
} CONFIG;

#pragma pack(pop)

const CONFIG Config = {
    0x77777777, 0x88888888, 0x99999999, 0x77777777,
    0,
    0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,
    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,
    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,
    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,
    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,
    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,
    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,
    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,
    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,
    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,
    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0,    0, 0,0,0,0,0
};

#pragma optimize( "", off )

// ********************************************************************

BOOLEAN IsValidConfig(void)
// was the driver correctly configured?
{
  return (Config.Name[0] != 0);
}

// ********************************************************************

void AppendDriverName(LPWSTR Str)
// append the driver name to the specified string
// this is the name used for communication with user land
{
  int i1;
  int i2 = (int) wcslen(Str);

  for (i1 = 0; i1 < sizeof(Config.Name); i1++)
    Str[i2 + i1] = Config.Name[i1];
}

// ********************************************************************

ULONG EnumAuthorizedDlls(int Index, PULONG FileSize, PULONG *Hash)
// which dlls were made known to the driver at build time?
{
  if ((Index >=0) && ((ULONG) Index < Config.DllCount))
  {
    if (FileSize)
      *FileSize = Config.DllData[Index * 6];
    if (Hash)
      *Hash = (PULONG) &(Config.DllData[Index * 6 + 1]);
    return TRUE;
  }

  return FALSE;
}

// ********************************************************************

BOOLEAN IsSafeUnloadAllowed(void)
// is this driver allowed to unload safely?
{
  return ((Config.Flags & 0x03) != 0);
}

BOOLEAN IsUnsafeUnloadAllowed(void)
// is this driver allowed to unload unsafely?
{
  return ((Config.Flags & 0x02) != 0);
}

// ********************************************************************

#pragma optimize( "", on )
