// ***************************************************************
//  Ioctl.h                   version: 1.0.1  ·  date: 2010-08-02
//  -------------------------------------------------------------
//  handling of encrypted IOCTL requests from user land
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-08-02 1.1.1 added buf size params to HandleEncryptedIoctl
// 2010-01-10 1.0.0 initial release

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

#ifndef _Ioctl_
#define _Ioctl_

// ********************************************************************

// this function will decrypt and parse the IOCL command
BOOLEAN HandleEncryptedIoctl (ULONG Ioctl, PVOID Buf, ULONG InSize, ULONG OutSize, PETHREAD Caller, BOOLEAN *DriverUnloadEnabled);

// ********************************************************************

#endif
