// ***************************************************************
//  Config.h                  version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  basic driver configuration (initialized at build time)
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-01-10 1.0.0 initial release

#ifndef _DrvConfig_
#define _DrvConfig_

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

// ********************************************************************

// was the driver correctly configured?
BOOLEAN IsValidConfig (void);

// append the driver name to the specified string
// this is the name used for communication with user land
void AppendDriverName (LPWSTR Str);

// which dlls were made known to the driver at build time?
ULONG EnumAuthorizedDlls (int Index, PULONG FileSize, PULONG *Hash);

// is this driver allowed to unload safely, or even unsafely?
BOOLEAN IsSafeUnloadAllowed (void);
BOOLEAN IsUnsafeUnloadAllowed (void);

#endif
