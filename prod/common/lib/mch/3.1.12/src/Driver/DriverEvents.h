// ***************************************************************
//  DriverEvents.h            version: 1.0.1  ·  date: 2010-08-02
//  -------------------------------------------------------------
//  handles all the various driver events
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-08-02 1.0.1 (1) DriverEvent_NewImage added
//                  (2) DriverEvent_InjectMethodChange added
//                  (3) DriverEvent_EnumInjectRequest added
// 2010-01-10 1.0.0 initial release

#ifndef _DriverEvents_
#define _DriverEvents_

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

#include "DllList.h"

// ********************************************************************

// driver got loaded - or is about to be unloaded
BOOLEAN DriverEvent_InitDriver   (PDRIVER_OBJECT DriverObject);
void    DriverEvent_UnloadDriver (PDRIVER_OBJECT DriverObject);

// ********************************************************************

// user land asks us to allow/disallow unloading of the driver
BOOLEAN DriverEvent_AllowUnloadRequest (BOOLEAN Allow, BOOLEAN *DriverUnloadEnabled);

// ********************************************************************

// user land tells us to use a specific injection method
BOOLEAN DriverEvent_InjectMethodChange (BOOLEAN NewInjectionMethod);

// ********************************************************************

// user land asks us to start/stop injection of a specific dll
BOOLEAN DriverEvent_InjectionRequest   (PDllItem Dll, HANDLE CallingProcessId, BOOLEAN *DriverUnloadEnabled);
BOOLEAN DriverEvent_UninjectionRequest (PDllItem Dll, HANDLE CallingProcessId, BOOLEAN *DriverUnloadEnabled);

// ********************************************************************

// user land wants to enumerate the active dll injection requests
BOOLEAN DriverEvent_EnumInjectRequest (ULONG Index, PDllItem Dll, ULONG BufSize);

// ********************************************************************

// a new process was created - or has just closed down
void DriverEvent_NewProcess  (HANDLE ProcessId);
void DriverEvent_ProcessGone (HANDLE ProcessId);

// ********************************************************************

// a new image was just loaded into a process
void DriverEvent_NewImage (PDEVICE_OBJECT DeviceObject, HANDLE ProcessId, PUNICODE_STRING FullImageName, PIMAGE_INFO ImageInfo);

// ********************************************************************

#endif
