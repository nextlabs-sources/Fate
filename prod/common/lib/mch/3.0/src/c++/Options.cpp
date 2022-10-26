// ***************************************************************
//  Options.cpp               version: 1.0.0   date: 2010-01-10
//  -------------------------------------------------------------
//  inter process communication functionality
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _OPTIONS_C

#define _CRT_SECURE_NO_DEPRECATE

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

static bool  SecureMemoryMaps             = false;
static bool  DisableChangedCodeCheck      = false;
static bool  UseNewIpcLogic               = false;
static DWORD InternalIpcTimeout           = 2000;
static bool  VmWareInjectionMode          = false;
static bool  RenewOverwrittenHooks        = false;
static bool  InjectIntoRunningProcesses   = true;
static bool  UninjectFromRunningProcesses = true;

// with this API you can configure some aspects of madCodeHook
// available options see constants above
SYSTEMS_API BOOL WINAPI SetMadCHookOption(DWORD option, LPCWSTR param)
{
  switch (option)
  {
    case SECURE_MEMORY_MAPS              : SecureMemoryMaps = true;
                                           break;
    case DISABLE_CHANGED_CODE_CHECK      : DisableChangedCodeCheck = true;
                                           break;
    case USE_NEW_IPC_LOGIC               : UseNewIpcLogic = true;
                                           break;
    case SET_INTERNAL_IPC_TIMEOUT        : InternalIpcTimeout = (DWORD) (ULONG_PTR) param;
                                           break;
    case VMWARE_INJECTION_MODE           : VmWareInjectionMode = true;
                                           break;
    case DONT_TOUCH_RUNNING_PROCESSES    : InjectIntoRunningProcesses = false;
                                           break;
    case RENEW_OVERWRITTEN_HOOKS         : RenewOverwrittenHooks = true;
                                           break;
    case INJECT_INTO_RUNNING_PROCESSES   : InjectIntoRunningProcesses = (param != NULL);
                                           break;
    case UNINJECT_FROM_RUNNING_PROCESSES : UninjectFromRunningProcesses = (param != NULL);
                                           break;
    default                              : return false;
  }
  return true;
}

SYSTEMS_API LPCWSTR WINAPI GetMadCHookOption(DWORD option)
{
  switch (option)
  {
    case SECURE_MEMORY_MAPS              : return (LPCWSTR) SecureMemoryMaps;
    case DISABLE_CHANGED_CODE_CHECK      : return (LPCWSTR) DisableChangedCodeCheck;
    case USE_NEW_IPC_LOGIC               : return (LPCWSTR) UseNewIpcLogic;
    case SET_INTERNAL_IPC_TIMEOUT        : return (LPCWSTR) (ULONG_PTR) InternalIpcTimeout;
    case VMWARE_INJECTION_MODE           : return (LPCWSTR) VmWareInjectionMode;
    case DONT_TOUCH_RUNNING_PROCESSES    : return (LPCWSTR) (!InjectIntoRunningProcesses);
    case RENEW_OVERWRITTEN_HOOKS         : return (LPCWSTR) RenewOverwrittenHooks;
    case INJECT_INTO_RUNNING_PROCESSES   : return (LPCWSTR) InjectIntoRunningProcesses;
    case UNINJECT_FROM_RUNNING_PROCESSES : return (LPCWSTR) UninjectFromRunningProcesses;
    default                              : return NULL;
  }
}
