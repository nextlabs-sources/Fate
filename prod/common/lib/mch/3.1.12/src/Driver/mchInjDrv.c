// ***************************************************************
//  mchInjDrv.c               version: 2.0.2  ·  date: 2014-05-05
//  -------------------------------------------------------------
//  DLL injection into newly created processes in the NT family
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2014 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2014-05-05 2.0.2 fixed: injection sometimes failed (win8.1)
// 2010-06-17 2.0.1 added callback for newly loaded images
// 2010-01-10 2.0.0 full rewrite, virtually everything has changed

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

#include "Config.h"
#include "DriverEvents.h"
#include "Ioctl.h"
#include "ToolFuncs.h"
#include "DelayedInjectList.h"

// ********************************************************************

BOOLEAN InitCommunication(PDRIVER_OBJECT DriverObject)
// initialize all the stuff we need for communication with user land
{
  UNICODE_STRING driverString;
  UNICODE_STRING deviceString;
  PDEVICE_OBJECT deviceObject;
  WCHAR wstr1 [60], wstr2 [60];

  wcscpy(wstr1, L"\\Device\\");
  AppendDriverName(wstr1);
  RtlInitUnicodeString(&driverString, wstr1);

  // create and initialize device object
  if (IoCreateDevice(DriverObject, 0, &driverString, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject))
    return FALSE;

  wcscpy(wstr2, L"\\DosDevices\\");
  AppendDriverName(wstr2);
  RtlInitUnicodeString(&deviceString, wstr2);

  // this makes the driver accessible from user land
  if (IoCreateSymbolicLink(&deviceString, &driverString))
  {
    IoDeleteDevice(deviceObject);
    return FALSE;
  }

  return TRUE;
}

void CloseCommunication(PDRIVER_OBJECT DriverObject)
// close all the stuff we needed for communication with user land
{
  UNICODE_STRING deviceString;
  WCHAR wstr [60];

  wcscpy(wstr, L"\\DosDevices\\");
  AppendDriverName(wstr);
  RtlInitUnicodeString(&deviceString, wstr);
  IoDeleteSymbolicLink(&deviceString);
  if (DriverObject->DeviceObject)
    IoDeleteDevice(DriverObject->DeviceObject);
}

// ********************************************************************

VOID ProcessCallback(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Created)
// this callback is called whenever a process is created or destroyed
{
  if (Created)
    DriverEvent_NewProcess(ProcessId);
  else
    DriverEvent_ProcessGone(ProcessId);
}

// ********************************************************************

PDRIVER_OBJECT GlobalDriverObject = NULL;

VOID ImageCallback(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
// this callback is called whenever a new image is loaded
{
  DriverEvent_NewImage(GlobalDriverObject->DeviceObject, ProcessId, FullImageName, ImageInfo);
}

// ********************************************************************

void UnloadDriver(PDRIVER_OBJECT DriverObject)
// called when our driver is about to be stopped
{
  // unregister the notification about new/closed processes and images
  #ifdef _WIN64
    if (IsVistaOrNewer())
  #else
    if (IsWin8OrNewer())
  #endif
  {
    RemoveLoadImageNotifyRoutine(ImageCallback);
    FreeDelayedInjectList();
  }
  PsSetCreateProcessNotifyRoutine(ProcessCallback, TRUE);

  // close all the stuff we needed for communication with user land
  CloseCommunication(DriverObject);

  // finalize everything
  DriverEvent_UnloadDriver(DriverObject);
}

// ********************************************************************

NTSTATUS DispatchIoctl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
// handling requests coming from application land
{
  NTSTATUS result = STATUS_UNSUCCESSFUL;
  PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
  BOOLEAN driverUnloadEnabled = (DeviceObject->DriverObject->DriverUnload != NULL);

  if ( (irpStack->Parameters.DeviceIoControl.InputBufferLength > 24) &&
       (HandleEncryptedIoctl(irpStack->Parameters.DeviceIoControl.IoControlCode,
                             Irp->AssociatedIrp.SystemBuffer,
                             irpStack->Parameters.DeviceIoControl.InputBufferLength,
                             irpStack->Parameters.DeviceIoControl.OutputBufferLength,
                             Irp->Tail.Overlay.Thread, &driverUnloadEnabled)) )
  {
    if ((driverUnloadEnabled) || IsUnsafeUnloadAllowed())
      DeviceObject->DriverObject->DriverUnload = UnloadDriver;
    else
      DeviceObject->DriverObject->DriverUnload = NULL;

    result = STATUS_SUCCESS;
  }

  Irp->IoStatus.Status = result;

  // set number of bytes to copy back to user-mode
  if (result == STATUS_SUCCESS)
    Irp->IoStatus.Information = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
  else
    Irp->IoStatus.Information = 0;

  IoCompleteRequest(Irp, IO_NO_INCREMENT);
  return result;
}

// ********************************************************************

NTSTATUS DummyIrpHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp)
// dummy handler, does nothing but return success
{
  Irp->IoStatus.Status = STATUS_SUCCESS;
  Irp->IoStatus.Information = 0;
  IoCompleteRequest(Irp, IO_NO_INCREMENT);

  return STATUS_SUCCESS;
}

// ********************************************************************

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
// the entry point of our driver
{
  if (!DriverEvent_InitDriver(DriverObject))
    return STATUS_UNSUCCESSFUL;

  // each driver has to handle IRP_MJ_CREATE and IRP_MJ_CLOSE
  DriverObject->MajorFunction[IRP_MJ_CREATE        ] = DummyIrpHandler;
  DriverObject->MajorFunction[IRP_MJ_CLOSE         ] = DummyIrpHandler;
  DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;

  if (IsUnsafeUnloadAllowed())
    // this driver is allowed to be unloaded by anyone at any time
    DriverObject->DriverUnload = UnloadDriver;

  // initialize all the stuff we need for communication with user land
  if (!InitCommunication(DriverObject))
    return STATUS_UNSUCCESSFUL;

  // initialize critical sections and stuff
  InitDllList();

  // finally register for notification about new/closed processes and images
  if (PsSetCreateProcessNotifyRoutine(ProcessCallback, FALSE))
  {
    DriverObject->DriverUnload = NULL;
    CloseCommunication(DriverObject);
    return STATUS_UNSUCCESSFUL;
  }

  #ifdef _WIN64
    if (IsVistaOrNewer())
  #else
    if (IsWin8OrNewer())
  #endif
  {
    // this is only needed for the delayed injection method
    // which is only needed in Vista x64 and newer OSs
    GlobalDriverObject = DriverObject;
    InitDelayedInjectList();
    SetLoadImageNotifyRoutine(ImageCallback);
  }

  return STATUS_SUCCESS;
}

// ********************************************************************
