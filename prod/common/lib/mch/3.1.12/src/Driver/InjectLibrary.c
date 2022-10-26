// ***************************************************************
//  InjectLibrary.c           version: 1.0.6  ·  date: 2016-03-16
//  -------------------------------------------------------------
//  inject a dll into a specific 32bit or 64bit process
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2016 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2016-03-16 1.0.6 (1) dll inject is now always done in main thread (win10)
//                  (2) ntdll APIs are now located by parsing ntdll.dll file
//                  (3) fixed some PAGE_EXECUTE_READWRITE security issues
//                  (4) worked around Microsoft EMET EAF complaint
// 2013-10-01 1.0.5 (1) fixed injection problem caused by StormShield fix
//                  (2) revert aligned UNICODE_STRING (compatability problems)
// 2013-05-13 1.0.4 aligned UNICODE_STRING in internal structure
// 2013-02-28 1.0.3 (1) fixed: injecting multiple 32bit dlls in x64 OS crashed
//                  (2) fixed: incompatability with StormShield AV (x64)
// 2012-03-23 1.0.2 added support for Windows 8
// 2011-03-27 1.0.1 (1) injection into wow64 processes improved (Vista+)
//                  (2) SetInjectionMethod API added
//                  (3) defaults to old injection method for all OSs now
//                  (4) added some injection tweaks to improve stability
// 2010-01-10 1.0.0 initial release

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

#include "ToolFuncs.h"

// ********************************************************************

// sizeof(PVOID32) is always 32bit, even in the 64bit driver
// sizeof(PVOID) is 32bit in a 32bit driver and 64bit in a 64bit driver
typedef VOID * POINTER_32 PVOID32;

// ********************************************************************

BOOLEAN UseNewInjectionMethod = FALSE;

BOOLEAN InitDone = FALSE;
HANDLE Ndh32 = NULL;     // ntdll handle
PVOID32 Ndep32 = NULL;   // ntdll entry point
PVOID32 Npvm32 = NULL;   // ntdll.NtProtectVirtualMemory
PVOID32 Lld32 = NULL;    // ntdll.LdrLoadDll
PVOID32 Navm32 = NULL;   // ntdll.NtAllocateVirtualMemory
PVOID32 Nfvm32 = NULL;   // ntdll.NtFreeVirtualMemory
PVOID32 Nqsi32 = NULL;   // ntdll.NtQuerySystemInformation
PVOID32 Nde32 = NULL;    // ntdll.NtDelayExecution
PVOID32 Nta32 = NULL;    // ntdll.NtTestAlert
#ifdef _WIN64
  PVOID Nta64 = NULL;    // ntdll.NtTestAlert
  PVOID Npvm64 = NULL;   // ntdll.NtProtectVirtualMemory
  PVOID Lld64 = NULL;    // ntdll.LdrLoadDll
  PVOID Navm64 = NULL;   // ntdll.NtAllocateVirtualMemory
  PVOID Nfvm64 = NULL;   // ntdll.NtFreeVirtualMemory
  PVOID Nqsi64 = NULL;   // ntdll.NtQuerySystemInformation
  PVOID Nde64 = NULL;    // ntdll.NtDelayExecution
#endif

PVOID GetProcAddressRaw(HANDLE ModuleHandleVirtual, HANDLE ModuleHandleRaw, PCSTR ProcName)
{
  if (ModuleHandleRaw)
  {
    PVOID resultRaw = GetProcAddress(ModuleHandleRaw, ProcName, FALSE, TRUE);
    if (resultRaw)
    {
      ULONG_PTR offsetRaw = (ULONG_PTR) resultRaw - (ULONG_PTR) ModuleHandleRaw;
      return (PVOID) ((ULONG_PTR) ModuleHandleVirtual + offsetRaw);
    }
  }
  return GetProcAddress(ModuleHandleVirtual, ProcName, TRUE, TRUE);
}

BOOLEAN InitInjectLibrary(HANDLE ProcessId)
{
  if (InitDone)
    return TRUE;
  else
  {

    BOOLEAN result = FALSE;
    BOOLEAN attached = FALSE;
    PEPROCESS eprocess = NULL;
    KAPC_STATE apcstate;
    HANDLE ntdll;

    UseNewInjectionMethod = FALSE;

    // let's try to find ntdll in the dlls loaded by the system
    // this usually succeeds with all OSs until and including Windows 7
    ntdll = GetSystemModuleHandle("ntdll.dll");

    __try
    {

      if ((!ntdll) && (ProcessId))
      {
        // Windows 8 doesn't seem to have ntdll loaded in system, anymore
        // so we wait until the user application tries to inject
        // then we look for the ntdll which is loaded in the user process
        if (AttachToMemoryContext(ProcessId, &eprocess, &apcstate))
        {
          attached = TRUE;
          ntdll = FindNtdll();
        }
      }        
      if (ntdll)
      {
        HANDLE ntdllFile = NULL;
        HANDLE ntdllSection = NULL;
        HANDLE ntdllBuf = NULL;
        BOOLEAN needUnmap = FALSE;
        if ((attached) && (MapDllFile(L"\\SystemRoot\\System32\\ntdll.dll", &ntdllFile, &ntdllSection, &ntdllBuf)))
          needUnmap = TRUE;
        else
          ntdllBuf = NULL;
        #ifdef _WIN64
          Nta64  = GetProcAddressRaw(ntdll, ntdllBuf, "NtTestAlert"             );
          Npvm64 = GetProcAddressRaw(ntdll, ntdllBuf, "NtProtectVirtualMemory"  );
          Lld64  = GetProcAddressRaw(ntdll, ntdllBuf, "LdrLoadDll"              );
          Navm64 = GetProcAddressRaw(ntdll, ntdllBuf, "NtAllocateVirtualMemory" );
          Nfvm64 = GetProcAddressRaw(ntdll, ntdllBuf, "NtFreeVirtualMemory"     );
          Nqsi64 = GetProcAddressRaw(ntdll, ntdllBuf, "NtQuerySystemInformation");
          Nde64  = GetProcAddressRaw(ntdll, ntdllBuf, "NtDelayExecution"        );
          if ((Nta64) && (((ULONG*) Nta64)[0] != 0xb8d18b4c))
          {
            // We require NtTestAlert to follow the typical native API code pattern, which is:
            // asm
            //   mov r10, rcx   (binary: 4c 8b d1)
            //   mov eax, dw    (binary: b8 dw)
            // Because only this way we can uninstall our patch properly later
            // in certain situations (see comment about StormShield above).
            Nta64 = NULL;
          }
          result = ((Nta64) && (Npvm64) && (Lld64));
        #else
          Ndh32  = ntdll;
          Ndep32 = GetAddressOfEntryPoint(ntdll);
          Nta32  = GetProcAddressRaw(ntdll, ntdllBuf, "NtTestAlert"             );
          Npvm32 = GetProcAddressRaw(ntdll, ntdllBuf, "NtProtectVirtualMemory"  );
          Lld32  = GetProcAddressRaw(ntdll, ntdllBuf, "LdrLoadDll"              );
          if (IsWin8OrNewer())
          {
            Navm32 = GetProcAddressRaw(ntdll, ntdllBuf, "NtAllocateVirtualMemory" );
            Nfvm32 = GetProcAddressRaw(ntdll, ntdllBuf, "NtFreeVirtualMemory"     );
            Nqsi32 = GetProcAddressRaw(ntdll, ntdllBuf, "NtQuerySystemInformation");
            Nde32  = GetProcAddressRaw(ntdll, ntdllBuf, "NtDelayExecution"        );
          }
          result = ((Ndep32) && (Nta32) && (Npvm32) && (Lld32));
        #endif
        if (needUnmap)
          UnmapDllFile(ntdllFile, ntdllSection, ntdllBuf);
      }
    }
    __finally
    {
      if (attached)
        DetachMemoryContext(eprocess, &apcstate);
    }

    if (result)
      InitDone = TRUE;

    return ((result) || (!ProcessId));
  }
}

#ifdef _WIN64
  void Set32bitNtdllInfo(HANDLE ModuleHandle, PVOID AddressOfEntryPoint, PVOID NtProtectVirtualMemory, PVOID LdrLoadDll, PVOID NtAllocateVirtualMemory, PVOID NtFreeVirtualMemory, PVOID NtQuerySystemInformation, PVOID NtDelayExecution, PVOID NtTestAlert)
  {
    if ((!Ndh32) && (ModuleHandle) && (AddressOfEntryPoint) && (NtProtectVirtualMemory) && (LdrLoadDll) && (NtTestAlert))
    {
      // we are being informed about the 32bit ntdll

      Ndh32 = ModuleHandle;
      Ndep32 = (PVOID32) AddressOfEntryPoint;
      Npvm32 = (PVOID32) NtProtectVirtualMemory;
      Lld32 = (PVOID32) LdrLoadDll;
      Navm32 = (PVOID32) NtAllocateVirtualMemory;
      Nfvm32 = (PVOID32) NtFreeVirtualMemory;
      Nqsi32 = (PVOID32) NtQuerySystemInformation;
      Nde32 = (PVOID32) NtDelayExecution;
      Nta32 = (PVOID32) NtTestAlert;
    }
  }
#endif

void SetInjectionMethod(BOOLEAN NewInjectionMethod)
// set the injection method used by the kernel mode driver
{
  UseNewInjectionMethod = NewInjectionMethod;
}

// ********************************************************************

#pragma pack(push, 1)

// sizeof(PRelJump32) is always 32bit
// sizeof(PRelJump64) is 32bit in a 32bit driver and 64bit in a 64bit driver
typedef struct _RelJump {
    UCHAR jmp;     // 0xe9
    ULONG target;  // relative target
} RelJump, *PRelJump64, * POINTER_32 PRelJump32;

// sizeof(PInjectLib32New) is always 32bit
typedef struct _InjectLib32New * POINTER_32 PInjectLib32New;

// this structure has the same size in both the 32bit and 64bit driver
typedef struct _InjectLib32New
{
  PVOID32          pEntryPoint;
  ULONG            oldEntryPoint;
  PVOID32          oldEntryPointFunc;
  PVOID32          NtProtectVirtualMemory;
  PVOID32          LdrLoadDll;
  UNICODE_STRING32 dllStr;
  WCHAR            dllBuf[260];
  UCHAR            movEax;
  PVOID32          buf;
  UCHAR            jmp;
  int              target;
} InjectLib32New;

const UCHAR CInjectLib32FuncNew[155] =
  {0x55, 0x8b, 0xec, 0x83, 0xc4, 0xf4, 0x53, 0x56, 0x57, 0x8b, 0xd8, 0x8b, 0x75, 0x0c, 0x83, 0xfe,
   0x01, 0x75, 0x49, 0x8b, 0x03, 0x89, 0x45, 0xfc, 0xc7, 0x45, 0xf8, 0x04, 0x00, 0x00, 0x00, 0x8d,
   0x45, 0xf4, 0x50, 0x6a, 0x40, 0x8d, 0x45, 0xf8, 0x50, 0x8d, 0x45, 0xfc, 0x50, 0x6a, 0xff, 0xff,
   0x53, 0x0c, 0x85, 0xc0, 0x75, 0x26, 0x8b, 0x53, 0x04, 0x8b, 0x03, 0x89, 0x10, 0x89, 0x45, 0xfc,
   0xc7, 0x45, 0xf8, 0x04, 0x00, 0x00, 0x00, 0x8d, 0x45, 0xf4, 0x50, 0x8b, 0x45, 0xf4, 0x50, 0x8d,
   0x45, 0xf8, 0x50, 0x8d, 0x45, 0xfc, 0x50, 0x6a, 0xff, 0xff, 0x53, 0x0c, 0x83, 0x7b, 0x04, 0x00,
   0x74, 0x14, 0x8b, 0x45, 0x10, 0x50, 0x56, 0x8b, 0x45, 0x08, 0x50, 0xff, 0x53, 0x08, 0x85, 0xc0,
   0x75, 0x04, 0x33, 0xc0, 0xeb, 0x02, 0xb0, 0x01, 0xf6, 0xd8, 0x1b, 0xff, 0x83, 0xfe, 0x01, 0x75,
   0x0f, 0x8d, 0x45, 0xf8, 0x50, 0x8d, 0x43, 0x14, 0x50, 0x6a, 0x00, 0x6a, 0x00, 0xff, 0x53, 0x10,
   0x8b, 0xc7, 0x5f, 0x5e, 0x5b, 0x8b, 0xe5, 0x5d, 0xc2, 0x0c, 0x00};

// the CInjectLib32FuncNew data is a compilation of the following Delphi code
// this user land code is copied to newly created 32bit processes to execute the dll injection
// unfortunately the x64 editions of XP and 2003 don't like this solution

// function CInjectLib32FuncNew(var injectRec: TInjectRec; d1, d2, reserved, reason, hInstance: dword) : bool;
// var p1     : pointer;
//     c1, c2 : dword;
// begin
//   with injectRec do begin
//     if reason = DLL_PROCESS_ATTACH then begin
//       p1 := pEntryPoint;
//       c1 := 4;
//       if NtProtectVirtualMemory($ffffffff, p1, c1, PAGE_EXECUTE_READWRITE, c2) = 0 then begin
//         pEntryPoint^ := oldEntryPoint;
//         p1 := pEntryPoint;
//         c1 := 4;
//         NtProtectVirtualMemory($ffffffff, p1, c1, c2, c2);
//       end;
//     end;
//     result := (oldEntryPoint = 0) or oldEntryPointFunc(hInstance, reason, reserved);
//     if reason = DLL_PROCESS_ATTACH then
//       LdrLoadDll(0, nil, @dll, c1);
//   end;
// end;

// sizeof(PInjectLib32Old) is always 32bit
typedef struct _InjectLib32Old * POINTER_32 PInjectLib32Old;

// this structure has the same size in both the 32bit and 64bit driver
typedef struct _InjectLib32Old
{
  UCHAR            movEax;        // 0xb8
  PInjectLib32Old  param;         // pointer to "self"
  UCHAR            movEcx;        // 0xb9
  PVOID32          proc;          // pointer to "self.injectFunc"
  USHORT           callEcx;       // 0xd1ff
  ULONG            magic;         // "mcIL" / 0x4c49636d (old version: "mciL" / 0x4c69636d
  PInjectLib32Old  next;          // next dll (if any)
  PRelJump32       pOldApi;       // which code location in user land do we patch?
  RelJump          oldApi;        // patch backup buffer, contains overwritten code
//  UCHAR            align;
  UNICODE_STRING32 dll;           // dll path/name
  WCHAR            dllBuf [260];  // string buffer for the dll path/name
  PVOID32          npvm;          // ntdll.NtProtectVirtualMemory
  PVOID32          lld;           // ntdll.LdrLoadDll
  PVOID32          navm;          // ntdll.NtAllocateVirtualMemory
  PVOID32          nfvm;          // ntdll.NtFreeVirtualMemory
  PVOID32          nqsi;          // ntdll.NtQuerySystemInformation
  PVOID32          nde;           // ntdll.NtDelayExecution
  UCHAR            injectFunc;    // will be filled with CInjectLibFunc32 (see below)
} InjectLib32Old;

#ifdef _WIN64

  const UCHAR CInjectLib32FuncOldDynamic[1171] =
    {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0x90, 0x53, 0x56, 0x57, 0x8B, 0xF0, 0x64, 0x8B, 0x05, 0x30, 0x00,
     0x00, 0x00, 0x89, 0x45, 0xFC, 0x64, 0x8B, 0x05, 0x18, 0x00, 0x00, 0x00, 0x8B, 0x40, 0x20, 0x89,
     0x45, 0xF8, 0x64, 0x8B, 0x05, 0x18, 0x00, 0x00, 0x00, 0x8B, 0x40, 0x24, 0x89, 0x45, 0xF4, 0x89,
     0x6D, 0xF0, 0x8B, 0x46, 0x14, 0x8B, 0x55, 0xF0, 0x83, 0xC2, 0x04, 0x89, 0x02, 0x33, 0xC0, 0x89,
     0x45, 0xCC, 0x33, 0xC0, 0x89, 0x45, 0xC8, 0x33, 0xC0, 0x89, 0x45, 0xC4, 0x33, 0xC0, 0x89, 0x45,
     0xC0, 0x33, 0xC0, 0x89, 0x45, 0xBC, 0x33, 0xC0, 0x89, 0x45, 0xB8, 0x8B, 0x45, 0xFC, 0x83, 0xC0,
     0x0C, 0x8B, 0x00, 0x83, 0xC0, 0x14, 0x89, 0x45, 0xDC, 0x8B, 0x45, 0xDC, 0x56, 0x8B, 0xF0, 0x8D,
     0x7D, 0x90, 0xB9, 0x09, 0x00, 0x00, 0x00, 0xF3, 0xA5, 0x5E, 0xE9, 0x01, 0x04, 0x00, 0x00, 0x56,
     0x8B, 0xF0, 0x8D, 0x7D, 0x90, 0xB9, 0x09, 0x00, 0x00, 0x00, 0xF3, 0xA5, 0x5E, 0x83, 0x7D, 0xB0,
     0x00, 0x0F, 0x84, 0xE9, 0x03, 0x00, 0x00, 0x33, 0xC9, 0xEB, 0x01, 0x41, 0x8B, 0x45, 0xB0, 0x66,
     0x83, 0x3C, 0x48, 0x00, 0x75, 0xF5, 0x8B, 0x45, 0xB0, 0x8D, 0x44, 0x48, 0xEE, 0x81, 0x78, 0x04,
     0x64, 0x00, 0x6C, 0x00, 0x0F, 0x85, 0xC6, 0x03, 0x00, 0x00, 0x81, 0x38, 0x6E, 0x00, 0x74, 0x00,
     0x0F, 0x85, 0xBA, 0x03, 0x00, 0x00, 0x8B, 0x45, 0xB0, 0x8D, 0x44, 0x48, 0xF6, 0x81, 0x78, 0x04,
     0x64, 0x00, 0x6C, 0x00, 0x0F, 0x85, 0xA6, 0x03, 0x00, 0x00, 0x81, 0x38, 0x6C, 0x00, 0x2E, 0x00,
     0x0F, 0x85, 0x9A, 0x03, 0x00, 0x00, 0x8B, 0x5D, 0xA0, 0x8D, 0x43, 0x3C, 0x8B, 0x00, 0x03, 0xC3,
     0x8B, 0x40, 0x78, 0x03, 0xC3, 0x89, 0xC7, 0x8B, 0x47, 0x18, 0x48, 0x85, 0xC0, 0x0F, 0x8C, 0x01,
     0x02, 0x00, 0x00, 0x40, 0x89, 0x45, 0xB4, 0xC7, 0x45, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x47,
     0x20, 0x03, 0xC3, 0x8B, 0x55, 0xD8, 0x8B, 0x0C, 0x90, 0x03, 0xCB, 0x85, 0xC9, 0x0F, 0x84, 0xD5,
     0x01, 0x00, 0x00, 0x81, 0x79, 0x04, 0x6F, 0x61, 0x64, 0x44, 0x75, 0x1D, 0x81, 0x39, 0x4C, 0x64,
     0x72, 0x4C, 0x75, 0x15, 0x8D, 0x41, 0x08, 0x8B, 0x00, 0x25, 0xFF, 0xFF, 0xFF, 0x00, 0x3D, 0x6C,
     0x6C, 0x00, 0x00, 0x0F, 0x84, 0x31, 0x01, 0x00, 0x00, 0x81, 0x79, 0x04, 0x6F, 0x74, 0x65, 0x63,
     0x75, 0x3D, 0x81, 0x39, 0x4E, 0x74, 0x50, 0x72, 0x75, 0x35, 0x8D, 0x41, 0x08, 0x81, 0x78, 0x04,
     0x74, 0x75, 0x61, 0x6C, 0x75, 0x29, 0x81, 0x38, 0x74, 0x56, 0x69, 0x72, 0x75, 0x21, 0x8D, 0x41,
     0x10, 0x8B, 0x50, 0x04, 0x8B, 0x00, 0x81, 0xE2, 0xFF, 0xFF, 0xFF, 0x00, 0x81, 0xFA, 0x72, 0x79,
     0x00, 0x00, 0x75, 0x05, 0x3D, 0x4D, 0x65, 0x6D, 0x6F, 0x0F, 0x84, 0xEB, 0x00, 0x00, 0x00, 0x81,
     0x79, 0x04, 0x6C, 0x6F, 0x63, 0x61, 0x75, 0x34, 0x81, 0x39, 0x4E, 0x74, 0x41, 0x6C, 0x75, 0x2C,
     0x8D, 0x41, 0x08, 0x81, 0x78, 0x04, 0x72, 0x74, 0x75, 0x61, 0x75, 0x20, 0x81, 0x38, 0x74, 0x65,
     0x56, 0x69, 0x75, 0x18, 0x8D, 0x41, 0x10, 0x81, 0x78, 0x04, 0x6F, 0x72, 0x79, 0x00, 0x75, 0x06,
     0x81, 0x38, 0x6C, 0x4D, 0x65, 0x6D, 0x0F, 0x84, 0xAE, 0x00, 0x00, 0x00, 0x81, 0x79, 0x04, 0x65,
     0x65, 0x56, 0x69, 0x75, 0x27, 0x81, 0x39, 0x4E, 0x74, 0x46, 0x72, 0x75, 0x1F, 0x8D, 0x41, 0x08,
     0x81, 0x78, 0x04, 0x6C, 0x4D, 0x65, 0x6D, 0x75, 0x13, 0x81, 0x38, 0x72, 0x74, 0x75, 0x61, 0x75,
     0x0B, 0x8D, 0x41, 0x10, 0x81, 0x38, 0x6F, 0x72, 0x79, 0x00, 0x74, 0x7E, 0x81, 0x79, 0x04, 0x65,
     0x72, 0x79, 0x53, 0x75, 0x38, 0x81, 0x39, 0x4E, 0x74, 0x51, 0x75, 0x75, 0x30, 0x8D, 0x41, 0x08,
     0x81, 0x78, 0x04, 0x6D, 0x49, 0x6E, 0x66, 0x75, 0x24, 0x81, 0x38, 0x79, 0x73, 0x74, 0x65, 0x75,
     0x1C, 0x8D, 0x41, 0x10, 0x81, 0x78, 0x04, 0x74, 0x69, 0x6F, 0x6E, 0x75, 0x10, 0x81, 0x38, 0x6F,
     0x72, 0x6D, 0x61, 0x75, 0x08, 0x8D, 0x41, 0x18, 0x80, 0x38, 0x00, 0x74, 0x3D, 0x81, 0x79, 0x04,
     0x6C, 0x61, 0x79, 0x45, 0x0F, 0x85, 0xAE, 0x00, 0x00, 0x00, 0x81, 0x39, 0x4E, 0x74, 0x44, 0x65,
     0x0F, 0x85, 0xA2, 0x00, 0x00, 0x00, 0x8D, 0x41, 0x08, 0x81, 0x78, 0x04, 0x74, 0x69, 0x6F, 0x6E,
     0x0F, 0x85, 0x92, 0x00, 0x00, 0x00, 0x81, 0x38, 0x78, 0x65, 0x63, 0x75, 0x0F, 0x85, 0x86, 0x00,
     0x00, 0x00, 0x8D, 0x41, 0x10, 0x80, 0x38, 0x00, 0x75, 0x7E, 0x8B, 0x47, 0x24, 0x03, 0xC3, 0x8B,
     0x55, 0xD8, 0x0F, 0xB7, 0x04, 0x50, 0x89, 0x45, 0xE8, 0x8B, 0x47, 0x1C, 0x03, 0xC3, 0x8B, 0x55,
     0xE8, 0x8B, 0x04, 0x90, 0x89, 0x45, 0xE8, 0x33, 0xC0, 0x8A, 0x41, 0x02, 0x83, 0xF8, 0x50, 0x7F,
     0x13, 0x74, 0x25, 0x83, 0xE8, 0x41, 0x74, 0x2A, 0x83, 0xE8, 0x03, 0x74, 0x43, 0x83, 0xE8, 0x02,
     0x74, 0x2A, 0xEB, 0x44, 0x83, 0xE8, 0x51, 0x74, 0x2D, 0x83, 0xE8, 0x21, 0x75, 0x3A, 0x8B, 0x45,
     0xE8, 0x03, 0xC3, 0x89, 0x45, 0xC8, 0xEB, 0x30, 0x8B, 0x45, 0xE8, 0x03, 0xC3, 0x89, 0x45, 0xCC,
     0xEB, 0x26, 0x8B, 0x45, 0xE8, 0x03, 0xC3, 0x89, 0x45, 0xC4, 0xEB, 0x1C, 0x8B, 0x45, 0xE8, 0x03,
     0xC3, 0x89, 0x45, 0xC0, 0xEB, 0x12, 0x8B, 0x45, 0xE8, 0x03, 0xC3, 0x89, 0x45, 0xBC, 0xEB, 0x08,
     0x8B, 0x45, 0xE8, 0x03, 0xC3, 0x89, 0x45, 0xB8, 0xFF, 0x45, 0xD8, 0xFF, 0x4D, 0xB4, 0x0F, 0x85,
     0x0A, 0xFE, 0xFF, 0xFF, 0x83, 0x7D, 0xC8, 0x00, 0x0F, 0x84, 0x7E, 0x01, 0x00, 0x00, 0x83, 0x7D,
     0xCC, 0x00, 0x0F, 0x84, 0x74, 0x01, 0x00, 0x00, 0x33, 0xDB, 0x83, 0x7D, 0xC4, 0x00, 0x0F, 0x84,
     0xA6, 0x00, 0x00, 0x00, 0x83, 0x7D, 0xC0, 0x00, 0x0F, 0x84, 0x9C, 0x00, 0x00, 0x00, 0x83, 0x7D,
     0xBC, 0x00, 0x0F, 0x84, 0x92, 0x00, 0x00, 0x00, 0x83, 0x7D, 0xB8, 0x00, 0x0F, 0x84, 0x88, 0x00,
     0x00, 0x00, 0x33, 0xC0, 0x89, 0x45, 0xE8, 0x8D, 0x45, 0xE8, 0x50, 0x6A, 0x00, 0x6A, 0x00, 0x6A,
     0x05, 0xFF, 0x55, 0xBC, 0x83, 0x7D, 0xE8, 0x00, 0x74, 0x70, 0x8B, 0x45, 0xE8, 0x03, 0xC0, 0x89,
     0x45, 0xEC, 0x33, 0xC0, 0x89, 0x45, 0xE0, 0x6A, 0x40, 0x68, 0x00, 0x10, 0x00, 0x00, 0x8D, 0x45,
     0xEC, 0x50, 0x6A, 0x00, 0x8D, 0x45, 0xE0, 0x50, 0x6A, 0xFF, 0xFF, 0x55, 0xC4, 0x85, 0xC0, 0x75,
     0x49, 0x6A, 0x00, 0x8B, 0x45, 0xE8, 0x50, 0x8B, 0x45, 0xE0, 0x50, 0x6A, 0x05, 0xFF, 0x55, 0xBC,
     0x85, 0xC0, 0x75, 0x1F, 0x8B, 0x45, 0xE0, 0x8B, 0x50, 0x44, 0x3B, 0x55, 0xF8, 0x75, 0x08, 0x8B,
     0x98, 0xDC, 0x00, 0x00, 0x00, 0xEB, 0x0C, 0x8B, 0x10, 0x85, 0xD2, 0x74, 0x06, 0x03, 0xD0, 0x8B,
     0xC2, 0xEB, 0xE4, 0x33, 0xC0, 0x89, 0x45, 0xEC, 0x68, 0x00, 0x80, 0x00, 0x00, 0x8D, 0x45, 0xEC,
     0x50, 0x8D, 0x45, 0xE0, 0x50, 0x6A, 0xFF, 0xFF, 0x55, 0xC0, 0x85, 0xDB, 0x74, 0x41, 0x3B, 0x5D,
     0xF4, 0x74, 0x3C, 0xC7, 0x45, 0xE8, 0x01, 0x00, 0x00, 0x00, 0x8B, 0x46, 0x14, 0x8A, 0x10, 0x3A,
     0x56, 0x18, 0x75, 0x08, 0x8B, 0x40, 0x01, 0x3B, 0x46, 0x19, 0x74, 0x23, 0xC7, 0x45, 0xD0, 0x60,
     0x79, 0xFE, 0xFF, 0xC7, 0x45, 0xD4, 0xFF, 0xFF, 0xFF, 0xFF, 0x8D, 0x45, 0xD0, 0x50, 0x6A, 0x00,
     0xFF, 0x55, 0xB8, 0xFF, 0x45, 0xE8, 0x81, 0x7D, 0xE8, 0xE9, 0x03, 0x00, 0x00, 0x75, 0xCB, 0x8B,
     0x46, 0x14, 0x8A, 0x10, 0x3A, 0x56, 0x18, 0x75, 0x08, 0x8B, 0x50, 0x01, 0x3B, 0x56, 0x19, 0x74,
     0x6B, 0x89, 0x45, 0xE0, 0xC7, 0x45, 0xE8, 0x05, 0x00, 0x00, 0x00, 0x8D, 0x45, 0xE4, 0x50, 0x6A,
     0x40, 0x8D, 0x45, 0xE8, 0x50, 0x8D, 0x45, 0xE0, 0x50, 0x6A, 0xFF, 0xFF, 0x55, 0xCC, 0x8B, 0x46,
     0x14, 0x8B, 0x56, 0x18, 0x89, 0x10, 0x8A, 0x56, 0x1C, 0x88, 0x50, 0x04, 0xC7, 0x45, 0xE8, 0x05,
     0x00, 0x00, 0x00, 0x8D, 0x45, 0xE4, 0x50, 0x8B, 0x45, 0xE4, 0x50, 0x8D, 0x45, 0xE8, 0x50, 0x8D,
     0x45, 0xE0, 0x50, 0x6A, 0xFF, 0xFF, 0x55, 0xCC, 0x8D, 0x45, 0xE8, 0x50, 0x8D, 0x46, 0x1D, 0x50,
     0x6A, 0x00, 0x6A, 0x00, 0xFF, 0x55, 0xC8, 0x8B, 0x76, 0x10, 0x85, 0xF6, 0x75, 0xEA, 0xEB, 0x0C,
     0x8B, 0x45, 0x90, 0x3B, 0x45, 0xDC, 0x0F, 0x85, 0xF3, 0xFB, 0xFF, 0xFF, 0x5F, 0x5E, 0x5B, 0x8B,
     0xE5, 0x5D, 0xC3};

  // the CInjectLib32FuncOldDynamic data is a compilation of the following Delphi code
  // this user land code is copied to newly created wow64 processes to execute the dll injection

  // this code is used by default for all 64bit OSs

  // the 32bit injection code is rather complicated in the 64bit driver
  // the reason for that is that the 64bit driver doesn''t know some 32bit ntdll APIs
  // so the 32bit code has to find out the address of these APIs at runtime

  // procedure InjectLibFunc32Old(buf: PInjectLib32);
  // var ctid, mtid : dword;  // current and main thread ids
  //     cpid       : dword;
  //     ebp_       : dword;
  //     size       : NativeInt;
  //     c1, c2     : dword;
  //     peb        : dword;
  //     p1         : pointer;
  //     loopEnd    : TPNtModuleInfo;
  //     mi         : TNtModuleInfo;
  //     len        : dword;
  //     ntdll      : dword;
  //     nh         : PImageNtHeaders;
  //     ed         : PImageExportDirectory;
  //     i1         : integer;
  //     api        : pchar;
  //     npi        : ^TNtProcessInfo;
  //     sleep      : int64;
  //     npvm       : function (process: THandle; var addr: pointer; var size: NativeUInt; newProt: dword; var oldProt: dword) : dword; stdcall;
  //     lld        : function (path, flags, name: pointer; var handle: HMODULE) : dword; stdcall;
  //     navm       : function (process: THandle; var addr: pointer; zeroBits: NativeUInt; var regionSize: NativeInt; allocationType, protect: dword) : dword; stdcall;
  //     nfvm       : function (process: THandle; var addr: pointer; var regionSize: NativeInt; freeType: dword) : dword; stdcall;
  //     nqsi       : function (infoClass: dword; buffer: pointer; bufSize: dword; returnSize: TPCardinal) : dword; stdcall;
  //     nde        : function (alertable: pointer; var delayInterval: int64) : dword; stdcall;
  // begin
  //   asm
  //     mov eax, fs:[$30]
  //     mov peb, eax
  //     mov eax, fs:[$18]
  //     mov eax, [eax+$20]
  //     mov cpid, eax
  //     mov eax, fs:[$18]
  //     mov eax, [eax+$24]
  //     mov ctid, eax
  //     mov ebp_, ebp
  //   end;
  //   TPPointer(ebp_ + 4)^ := buf.pOldApi;
  // 
  //   npvm := nil;
  //   lld := nil;
  //   navm := nil;
  //   nfvm := nil;
  //   nqsi := nil;
  //   nde := nil;
  // 
  //   // step 1: locate ntdll.dll
  //   loopEnd := pointer(dword(pointer(peb + $C)^) + $14);
  //   mi := loopEnd^;
  //   while mi.next <> loopEnd do begin
  //     mi := mi.next^;
  //     if mi.name <> nil then begin
  //       len := 0;
  //       while mi.name[len] <> #0 do
  //         inc(len);
  //       if (int64(pointer(@mi.name[len - 9])^) = $006c00640074006e) and         // ntdl
  //          (int64(pointer(@mi.name[len - 5])^) = $006c0064002e006c) then begin  // l.dl
  //         // found it!
  //         ntdll := mi.handle;
  // 
  //         // step 2: locate LdrLoadDll and NtProtectVirtualMemory
  //         nh := pointer(ntdll + dword(pointer(ntdll + $3C)^));
  //         dword(ed) := ntdll + nh^.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  //         for i1 := 0 to ed^.NumberOfNames - 1 do begin
  //           api := pointer(ntdll + TPACardinal(ntdll + ed^.AddressOfNames)^[i1]);
  //           if (api <> nil) and
  //              ( ( (int64(pointer(@api[ 0])^)                       = $4464616f4c72644c) and                    // LdrLoadD
  //                  (dword(pointer(@api[ 8])^) and $00ffffff         = $00006c6c        )     ) or               // ll
  //                ( (int64(pointer(@api[ 0])^)                       = $6365746f7250744e) and                    // NtProtec
  //                  (int64(pointer(@api[ 8])^)                       = $6c61757472695674) and                    // tVirtual
  //                  (int64(pointer(@api[16])^) and $00ffffffffffffff = $000079726f6d654d)     ) or               // Memory
  //                ( (int64(pointer(@api[ 0])^)                       = $61636F6C6C41744E) and                    // NtAlloca
  //                  (int64(pointer(@api[ 8])^)                       = $6175747269566574) and                    // teVirtua
  //                  (int64(pointer(@api[16])^)                       = $0079726F6D654D6C)     ) or               // lMemory
  //                ( (int64(pointer(@api[ 0])^)                       = $695665657246744E) and                    // NtFreeVi
  //                  (int64(pointer(@api[ 8])^)                       = $6D654D6C61757472) and                    // rtualMem
  //                  (dword(pointer(@api[16])^)                       = $0079726F        )     ) or               // ory
  //                ( (int64(pointer(@api[ 0])^)                       = $537972657551744E) and                    // NtQueryS
  //                  (int64(pointer(@api[ 8])^)                       = $666E496D65747379) and                    // ystemInf
  //                  (int64(pointer(@api[16])^)                       = $6E6F6974616D726F) and                    // ormation
  //                  (byte (pointer(@api[24])^)                       = $00              )     ) or               // ormation
  //                ( (int64(pointer(@api[ 0])^)                       = $4579616c6544744e) and                    // NtDelayE
  //                  (int64(pointer(@api[ 8])^)                       = $6e6f697475636578) and                    // xecution
  //                  (byte (pointer(@api[16])^)                       = $00              )     )    ) then begin
  //             c1 := TPAWord(ntdll + ed^.AddressOfNameOrdinals)^[i1];
  //             c1 := TPACardinal(ntdll + ed^.AddressOfFunctions)^[c1];
  //             case api[2] of
  //               'r': lld  := pointer(ntdll + c1);
  //               'P': npvm := pointer(ntdll + c1);
  //               'A': navm := pointer(ntdll + c1);
  //               'F': nfvm := pointer(ntdll + c1);
  //               'Q': nqsi := pointer(ntdll + c1);
  //               'D': nde  := pointer(ntdll + c1);
  //             end;
  //           end;
  //         end;
  //         if (@lld <> nil) and (@npvm <> nil) then begin
  //           // found both APIs!
  //           mtid := 0;
  //           if (@navm <> nil) and (@nfvm <> nil) and (@nqsi <> nil) and (@nde <> nil) then begin
  //             c1 := 0;
  //             nqsi(5, nil, 0, @c1);
  //             if c1 <> 0 then begin
  //               size := c1 * 2;
  //               p1 := nil;
  //               if navm(THandle(-1), p1, 0, size, MEM_COMMIT, PAGE_READWRITE) = 0 then begin
  //                 if nqsi(5, p1, size, nil) = 0 then begin
  //                   npi := p1;
  //                   while true do begin
  //                     if npi^.pid = cpid then begin
  //                       mtid := npi^.threads[0].tid_nt5;
  //                       break;
  //                     end;
  //                     if npi^.offset = 0 then
  //                       break;
  //                     npi := pointer(NativeUInt(npi) + npi^.offset);
  //                   end;
  //                 end;
  //                 size := 0;
  //                 nfvm(THandle(-1), p1, size, MEM_RELEASE);
  //               end;
  //             end;
  //           end;
  //           if (mtid <> 0) and (mtid <> ctid) then begin
  //             // This is not the main thread! This usually doesn't happen, except sometimes in win10.
  //             // We "solve" this by waiting until the main thread has completed executing our loader stub.
  //             // Max wait time 10 seconds, just to be safe.
  //             for c1 := 1 to 1000 do begin
  //               if (buf.pOldApi^.jmp = buf.oldApi.jmp) and (buf.pOldApi^.target = buf.oldApi.target) then
  //                 // Our loader stub patch was removed, so we assume that the main thread has completed running it.
  //                 break;
  //               sleep := -100000;  // 10 milliseconds
  //               nde(nil, sleep);
  //             end;
  //           end;
  //           if (buf.pOldApi^.jmp <> buf.oldApi.jmp) or (buf.pOldApi^.target <> buf.oldApi.target) then begin
  //             // step 3: finally load the to-be-injected dll
  //             p1 := buf.pOldApi;
  //             c1 := 5;
  //             npvm(dword(-1), p1, c1, PAGE_EXECUTE_READWRITE, c2);
  //             buf.pOldApi^ := buf.oldApi;
  //             c1 := 5;
  //             npvm(dword(-1), p1, c1, c2, c2);
  //             repeat
  //               lld(nil, nil, @buf.dll, c1);
  //               buf := buf.next;
  //             until buf = nil;
  //           end;
  //         end;
  //         break;
  //       end;
  //     end;
  //   end;
  // end;

  typedef struct _InjectLib64 *PInjectLib64;
  typedef struct _InjectLib64
  {
    USHORT         movRax;        // 0xb848
    PVOID          retAddr;       // patched API
    UCHAR          pushRax;       // 0x50
    UCHAR          pushRcx;       // 0x51
    UCHAR          pushRdx;       // 0x52
    USHORT         pushR8;        // 0x5041
    USHORT         pushR9;        // 0x5141
    ULONG          subRsp28;      // 0x28ec8348
    USHORT         movRcx;        // 0xb948
    PInjectLib64   param;         // pointer to "self"
    USHORT         movRdx;        // 0xba48
    PVOID          proc;          // pointer to "self.injectFunc"
    USHORT         jmpEdx;        // 0xe2ff
    ULONG          magic;         // "mcIL" / 0x4c49636d (old version: "mciL" / 0x4c69636d
    PInjectLib64   next;          // next dll (if any)
    PRelJump64     pOldApi;       // which code location in user land do we patch?
    RelJump        oldApi;        // patch backup buffer, contains overwritten code
    UNICODE_STRING dll;           // dll path/name    68
    WCHAR          dllBuf [260];  // string buffer for the dll path/name
    PVOID          npvm;          // ntdll.NtProtectVirtualMemory
    PVOID          lld;           // ntdll.LdrLoadDll
    PVOID          navm;          // ntdll.NtAllocateVirtualMemory
    PVOID          nfvm;          // ntdll.NtFreeVirtualMemory
    PVOID          nqsi;          // ntdll.NtQuerySystemInformation
    PVOID          nde;           // ntdll.NtDelayExecution
    UCHAR          injectFunc;    // will be filled with CInjectLibFunc64 (see below)
  } InjectLib64;

  const UCHAR CInjectLib64Func[562] =
   {0x4C, 0x8B, 0xDC, 0x53, 0x55, 0x56, 0x57, 0x41, 0x55, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x58, 0x65,
    0x48, 0x8B, 0x04, 0x25, 0x30, 0x00, 0x00, 0x00, 0x41, 0xBE, 0x05, 0x00, 0x00, 0x00, 0x33, 0xFF,
    0x8B, 0x70, 0x40, 0x8B, 0x68, 0x48, 0x48, 0x8B, 0x41, 0x37, 0x49, 0x89, 0x43, 0xB0, 0x48, 0x8B,
    0x81, 0x7C, 0x02, 0x00, 0x00, 0x49, 0x83, 0xCD, 0xFF, 0x48, 0x85, 0xC0, 0x48, 0x8B, 0xD9, 0x4D,
    0x89, 0x73, 0xA8, 0x0F, 0x84, 0x10, 0x01, 0x00, 0x00, 0x41, 0x21, 0x7B, 0x08, 0x4D, 0x8D, 0x4B,
    0x08, 0x45, 0x33, 0xC0, 0x33, 0xD2, 0x41, 0x8B, 0xCE, 0xFF, 0xD0, 0x8B, 0x84, 0x24, 0x90, 0x00,
    0x00, 0x00, 0x85, 0xC0, 0x0F, 0x84, 0xEF, 0x00, 0x00, 0x00, 0x48, 0x21, 0xBC, 0x24, 0xA8, 0x00,
    0x00, 0x00, 0x8D, 0x0C, 0x00, 0x4C, 0x8D, 0x8C, 0x24, 0xA0, 0x00, 0x00, 0x00, 0x48, 0x89, 0x8C,
    0x24, 0xA0, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x94, 0x24, 0xA8, 0x00, 0x00, 0x00, 0x45, 0x33, 0xC0,
    0x49, 0x8B, 0xCD, 0xC7, 0x44, 0x24, 0x28, 0x40, 0x00, 0x00, 0x00, 0xC7, 0x44, 0x24, 0x20, 0x00,
    0x10, 0x00, 0x00, 0xFF, 0x93, 0x6C, 0x02, 0x00, 0x00, 0x85, 0xC0, 0x0F, 0x85, 0xA8, 0x00, 0x00,
    0x00, 0x44, 0x8B, 0x84, 0x24, 0xA0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x94, 0x24, 0xA8, 0x00, 0x00,
    0x00, 0x45, 0x33, 0xC9, 0x41, 0x8B, 0xCE, 0xFF, 0x93, 0x7C, 0x02, 0x00, 0x00, 0x85, 0xC0, 0x75,
    0x1F, 0x48, 0x8B, 0x8C, 0x24, 0xA8, 0x00, 0x00, 0x00, 0xEB, 0x09, 0x39, 0x39, 0x74, 0x11, 0x8B,
    0x01, 0x48, 0x03, 0xC8, 0x48, 0x39, 0x71, 0x50, 0x75, 0xF1, 0x8B, 0xB9, 0x30, 0x01, 0x00, 0x00,
    0x48, 0x83, 0xA4, 0x24, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x8D, 0x84, 0x24, 0xA0, 0x00, 0x00,
    0x00, 0x48, 0x8D, 0x94, 0x24, 0xA8, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x80, 0x00, 0x00, 0x49,
    0x8B, 0xCD, 0xFF, 0x93, 0x74, 0x02, 0x00, 0x00, 0x85, 0xFF, 0x74, 0x3D, 0x3B, 0xFD, 0x74, 0x39,
    0xBF, 0x01, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x4B, 0x37, 0x8A, 0x43, 0x3F, 0x38, 0x01, 0x75, 0x08,
    0x8B, 0x43, 0x40, 0x39, 0x41, 0x01, 0x74, 0x21, 0x48, 0x8D, 0x54, 0x24, 0x40, 0x33, 0xC9, 0x48,
    0xC7, 0x44, 0x24, 0x40, 0x60, 0x79, 0xFE, 0xFF, 0xFF, 0x93, 0x84, 0x02, 0x00, 0x00, 0x83, 0xC7,
    0x01, 0x81, 0xFF, 0xE8, 0x03, 0x00, 0x00, 0x7C, 0xCC, 0x48, 0x8B, 0x4B, 0x37, 0x8A, 0x43, 0x3F,
    0x38, 0x01, 0x75, 0x0C, 0x8B, 0x43, 0x40, 0x39, 0x41, 0x01, 0x0F, 0x84, 0xAB, 0x00, 0x00, 0x00,
    0x48, 0x8D, 0x84, 0x24, 0x98, 0x00, 0x00, 0x00, 0x4C, 0x8D, 0x44, 0x24, 0x30, 0x48, 0x8D, 0x54,
    0x24, 0x38, 0x41, 0xB9, 0x40, 0x00, 0x00, 0x00, 0x49, 0x8B, 0xCD, 0x48, 0x89, 0x44, 0x24, 0x20,
    0xFF, 0x93, 0x5C, 0x02, 0x00, 0x00, 0x85, 0xC0, 0x75, 0x3E, 0x8B, 0x43, 0x3F, 0x48, 0x8B, 0x53,
    0x37, 0x4C, 0x8D, 0x44, 0x24, 0x30, 0x89, 0x02, 0x8A, 0x43, 0x43, 0x49, 0x8B, 0xCD, 0x88, 0x42,
    0x04, 0x44, 0x8B, 0x8C, 0x24, 0x98, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x84, 0x24, 0x98, 0x00, 0x00,
    0x00, 0x48, 0x8D, 0x54, 0x24, 0x38, 0x48, 0x89, 0x44, 0x24, 0x20, 0x4C, 0x89, 0x74, 0x24, 0x30,
    0xFF, 0x93, 0x5C, 0x02, 0x00, 0x00, 0xEB, 0x27, 0x48, 0x8B, 0x43, 0x37, 0x48, 0x8B, 0x08, 0x8B,
    0x43, 0x3F, 0x48, 0x89, 0x0B, 0x89, 0x03, 0x8A, 0x43, 0x43, 0x88, 0x43, 0x04, 0x48, 0x8D, 0x4B,
    0x08, 0xC6, 0x01, 0xE9, 0x8B, 0x43, 0x37, 0x2B, 0xC1, 0x83, 0xC0, 0x03, 0x89, 0x41, 0x01, 0x4C,
    0x8D, 0x43, 0x44, 0x4C, 0x8D, 0x4C, 0x24, 0x30, 0x33, 0xD2, 0x33, 0xC9, 0xFF, 0x93, 0x64, 0x02,
    0x00, 0x00, 0x48, 0x8B, 0x5B, 0x2F, 0x48, 0x85, 0xDB, 0x75, 0xE4, 0x48, 0x83, 0xC4, 0x58, 0x41,
    0x5E, 0x41, 0x5D, 0x5F, 0x5E, 0x5D, 0x5B, // 0xC3};
    0x48, 0x83, 0xC4, 0x28, 0x41, 0x59, 0x41, 0x58, 0x5A, 0x59, 0xC3};

  // the CInjectLib64Func data is a compilation of the following C++ code
  // it got a manually created extended "tail", though (last 11 bytes)
  // this user land code is copied to newly created 64bit processes to execute the dll injection

  // static void InjectLib64Func(InjectLib64 *buf)
  // {
  //   ULONG_PTR* ptib = (ULONG_PTR*) NtCurrentTeb();
  //   DWORD cpid = (DWORD) ptib[8];
  //   DWORD ctid = (DWORD) ptib[9];
  // 
  //   LPVOID p1 = (LPVOID)buf->pOldApi;
  //   ULONG_PTR c1 = 5;
  //   ULONG c2;
  // 
  //   DWORD mtid = 0;
  //   if (buf->nqsi)
  //   {
  //     ULONG c1 = 0;
  //     buf->nqsi(SystemProcessInformation, NULL, 0, &c1);
  //     if (c1)
  //     {
  //       SIZE_T size = c1 * 2;
  //       LPVOID p1 = NULL;
  //       if (buf->navm((HANDLE) -1, &p1, 0, &size, MEM_COMMIT, PAGE_READWRITE) == 0)
  //       {
  //         if (buf->nqsi(SystemProcessInformation, p1, (ULONG) size, NULL) == 0)
  //         {
  //           SYSTEM_PROCESS_INFORMATION* npi = (SYSTEM_PROCESS_INFORMATION*) p1;
  //           while (true)
  //           {
  //             if (npi->Process.UniqueProcessId == cpid)
  //             {
  //               mtid = (DWORD) npi->Process_NT5.Threads[0].ClientId.UniqueThread;
  //               break;
  //             }
  //             if (npi->Process.Next == 0)
  //               break;
  //             npi = (SYSTEM_PROCESS_INFORMATION*) (((ULONG_PTR) npi) + npi->Process.Next);
  //           }
  //         }
  //         size = 0;
  //         buf->nfvm((HANDLE) -1, &p1, &size, MEM_RELEASE);
  //       }
  //     }
  //   }
  //   if ((mtid) && (mtid != ctid))
  //   {
  //     // This is not the main thread! This usually doesn't happen, except sometimes in win10.
  //     // We "solve" this by waiting until the main thread has completed executing our loader stub.
  //     // Max wait time 10 seconds, just to be safe.
  //     for (int i1 = 1; i1 < 1000; i1++)
  //     {
  //       if ((buf->pOldApi->jmp == buf->oldApi.jmp) && (buf->pOldApi->target == buf->oldApi.target))
  //         // Our loader stub patch was removed, so we assume that the main thread has completed running it.
  //         break;
  //       LONGLONG sleep = -100000;  // 10 milliseconds
  //       buf->nde(NULL, (PLARGE_INTEGER) &sleep);
  //     }
  //   }
  //   if ((buf->pOldApi->jmp != buf->oldApi.jmp) || (buf->pOldApi->target != buf->oldApi.target))
  //   {
  //     if (!buf->npvm((HANDLE) -1, &p1, &c1, PAGE_EXECUTE_READWRITE, &c2))
  //     {
  //       *(buf->pOldApi) = buf->oldApi;
  //       c1 = 5;
  //       buf->npvm((HANDLE) -1, &p1, &c1, c2, &c2);
  //     }
  //     else
  //     {
  //       // For some reason we can't uninstall our patch correctly.
  //       // This is known to happen with StormShield AV installed.
  //       // So what we do is modify our hook callback into a simple passthrough trampoline.
  //       RelJump *rj = (RelJump*) buf;
  //       // (1) Copy first 8 bytes from original API to our hook callback.
  //       *((LPVOID*) buf) = *((LPVOID*) buf->pOldApi);
  //       // (2) Uninstall the JMP that is placed in the first 5 bytes.
  //       *rj = buf->oldApi;
  //       // (3) Finally, after the first 8 bytes, JMP back to the original API.
  //       rj = (RelJump*) (((ULONG_PTR) buf) + 8);
  //       rj->jmp = 0xe9;
  //       rj->target = (ULONG) ((((ULONG_PTR) buf->pOldApi) + 8) - (ULONG_PTR) rj - 5);
  //     }
  //     do
  //     {
  //       buf->lld(0, NULL, (PUNICODE_STRING) &(buf->dll), (HMODULE*) &c1);
  //       buf = (InjectLib64*) buf->next;
  //     } while (buf);
  //   }
  // }

#endif

const UCHAR CInjectLib32FuncOldStatic[411] =
  {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xD8, 0x53, 0x56, 0x8B, 0xD8, 0x64, 0x8B, 0x05, 0x18, 0x00, 0x00,
   0x00, 0x8B, 0x40, 0x20, 0x89, 0x45, 0xFC, 0x64, 0x8B, 0x05, 0x18, 0x00, 0x00, 0x00, 0x8B, 0x40,
   0x24, 0x89, 0x45, 0xF8, 0x89, 0x6D, 0xF4, 0x8B, 0x45, 0xF4, 0x83, 0xC0, 0x04, 0x8B, 0x53, 0x14,
   0x89, 0x10, 0x33, 0xF6, 0x83, 0xBB, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x0F, 0x84, 0x94, 0x00, 0x00,
   0x00, 0x33, 0xC0, 0x89, 0x45, 0xF0, 0x8D, 0x45, 0xF0, 0x50, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x05,
   0xFF, 0x93, 0x3D, 0x02, 0x00, 0x00, 0x83, 0x7D, 0xF0, 0x00, 0x74, 0x79, 0x8B, 0x45, 0xF0, 0x03,
   0xC0, 0x89, 0x45, 0xE8, 0x33, 0xC0, 0x89, 0x45, 0xE4, 0x6A, 0x40, 0x68, 0x00, 0x10, 0x00, 0x00,
   0x8D, 0x45, 0xE8, 0x50, 0x6A, 0x00, 0x8D, 0x45, 0xE4, 0x50, 0x6A, 0xFF, 0xFF, 0x93, 0x35, 0x02,
   0x00, 0x00, 0x85, 0xC0, 0x75, 0x4F, 0x6A, 0x00, 0x8B, 0x45, 0xF0, 0x50, 0x8B, 0x45, 0xE4, 0x50,
   0x6A, 0x05, 0xFF, 0x93, 0x3D, 0x02, 0x00, 0x00, 0x85, 0xC0, 0x75, 0x1F, 0x8B, 0x45, 0xE4, 0x8B,
   0x50, 0x44, 0x3B, 0x55, 0xFC, 0x75, 0x08, 0x8B, 0xB0, 0xDC, 0x00, 0x00, 0x00, 0xEB, 0x0C, 0x8B,
   0x10, 0x85, 0xD2, 0x74, 0x06, 0x03, 0xD0, 0x8B, 0xC2, 0xEB, 0xE4, 0x33, 0xC0, 0x89, 0x45, 0xE8,
   0x68, 0x00, 0x80, 0x00, 0x00, 0x8D, 0x45, 0xE8, 0x50, 0x8D, 0x45, 0xE4, 0x50, 0x6A, 0xFF, 0xFF,
   0x93, 0x39, 0x02, 0x00, 0x00, 0x85, 0xF6, 0x74, 0x44, 0x3B, 0x75, 0xF8, 0x74, 0x3F, 0xC7, 0x45,
   0xF0, 0x01, 0x00, 0x00, 0x00, 0x8B, 0x43, 0x14, 0x8A, 0x10, 0x3A, 0x53, 0x18, 0x75, 0x08, 0x8B,
   0x40, 0x01, 0x3B, 0x43, 0x19, 0x74, 0x26, 0xC7, 0x45, 0xD8, 0x60, 0x79, 0xFE, 0xFF, 0xC7, 0x45,
   0xDC, 0xFF, 0xFF, 0xFF, 0xFF, 0x8D, 0x45, 0xD8, 0x50, 0x6A, 0x00, 0xFF, 0x93, 0x41, 0x02, 0x00,
   0x00, 0xFF, 0x45, 0xF0, 0x81, 0x7D, 0xF0, 0xE9, 0x03, 0x00, 0x00, 0x75, 0xC8, 0x8B, 0x43, 0x14,
   0x8A, 0x10, 0x3A, 0x53, 0x18, 0x75, 0x08, 0x8B, 0x50, 0x01, 0x3B, 0x53, 0x19, 0x74, 0x66, 0x89,
   0x45, 0xE4, 0xC7, 0x45, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x8D, 0x45, 0xEC, 0x50, 0x6A, 0x40, 0x8D,
   0x45, 0xF0, 0x50, 0x8D, 0x45, 0xE4, 0x50, 0x6A, 0xFF, 0xFF, 0x93, 0x2D, 0x02, 0x00, 0x00, 0x8B,
   0x43, 0x14, 0x8B, 0x53, 0x18, 0x89, 0x10, 0x8A, 0x53, 0x1C, 0x88, 0x50, 0x04, 0xC7, 0x45, 0xF0,
   0x05, 0x00, 0x00, 0x00, 0x8D, 0x45, 0xEC, 0x50, 0x8B, 0x45, 0xEC, 0x50, 0x8D, 0x45, 0xF0, 0x50,
   0x8D, 0x45, 0xE4, 0x50, 0x6A, 0xFF, 0xFF, 0x93, 0x2D, 0x02, 0x00, 0x00, 0x8D, 0x45, 0xF0, 0x50,
   0x8D, 0x43, 0x1D, 0x50, 0x6A, 0x00, 0x6A, 0x00, 0xFF, 0x93, 0x31, 0x02, 0x00, 0x00, 0x8B, 0x5B,
   0x10, 0x85, 0xDB, 0x75, 0xE7, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC3};

// the CInjectLibFunc32OldStatic data is a compilation of the following Delphi code
// this user land code is copied to newly created 32bit processes to execute the dll injection
// this solution is used by default in all 32bit OSs

// procedure InjectLibFunc32Old(buf: PInjectLib32);
// var ctid, mtid : dword;// current and main thread ids
//     cpid       : dword;
//     ebp_       : dword;
//     c1, c2     : dword;
//     size       : NativeInt;
//     p1         : pointer;
//     sleep      : int64;
//     npi        : ^TNtProcessInfo;
// begin
//   asm
//     mov eax, fs:[$18]
//     mov eax, [eax+$20]
//     mov cpid, eax
//     mov eax, fs:[$18]
//     mov eax, [eax+$24]
//     mov ctid, eax
//     mov ebp_, ebp
//   end;
//   TPPointer(ebp_ + 4)^ := buf.pOldApi;
//   mtid := 0;
//   if @buf.nqsi <> nil then begin
//     c1 := 0;
//     buf.nqsi(5, nil, 0, @c1);
//     if c1 <> 0 then begin
//       size := c1 * 2;
//       p1 := nil;
//       if buf.navm(THandle(-1), p1, 0, size, MEM_COMMIT, PAGE_READWRITE) = 0 then begin
//         if buf.nqsi(5, p1, c1, nil) = 0 then begin
//           npi := p1;
//           while true do begin
//             if npi^.pid = cpid then begin
//               mtid := npi^.threads[0].tid_nt5;
//               break;
//             end;
//             if npi^.offset = 0 then
//               break;
//             npi := pointer(NativeUInt(npi) + npi^.offset);
//           end;
//         end;
//         size := 0;
//         buf.nfvm(THandle(-1), p1, size, MEM_RELEASE);
//       end;
//     end;
//   end;
//   if (mtid <> 0) and (mtid <> ctid) then begin
// // This is not the main thread! This usually doesn't happen, except sometimes in win10.
// // We "solve" this by waiting until the main thread has completed executing our loader stub.
// // Max wait time 10 seconds, just to be safe.
//     for c1 := 1 to 1000 do begin
//       if (buf.pOldApi^.jmp = buf.oldApi.jmp) and (buf.pOldApi^.target = buf.oldApi.target) then
//     // Our loader stub patch was removed, so we assume that the main thread has completed running it.
//         break;
//       sleep := -100000;// 10 milliseconds
//       buf.nde(nil, sleep);
//     end;
//   end;
//   if (buf.pOldApi^.jmp <> buf.oldApi.jmp) or (buf.pOldApi^.target <> buf.oldApi.target) then begin
// // step 3: finally load the to-be-injected dll
//     p1 := buf.pOldApi;
//     c1 := 5;
//     buf.npvm(dword(-1), p1, c1, PAGE_EXECUTE_READWRITE, c2);
//     buf.pOldApi^ := buf.oldApi;
//     c1 := 5;
//     buf.npvm(dword(-1), p1, c1, c2, c2);
//     repeat
//       buf.lld(nil, nil, @buf.dll, c1);
//       buf := buf.next;
//     until buf = nil;
//   end;
// end;

#pragma pack(pop)

// ********************************************************************

static BOOLEAN InjectLibraryNew32(HANDLE ProcessHandle, PWSTR LibraryFileName)
// inject a dll into a newly started 32bit process
// injection is done by providing/patching the ntdll.dll entry point
// unfortunately NT4 and Windows 2000 simply ignore an ntdll.dll entry point
// this solution is currently not used in any OSs by default
// the dll is injected directly after ntdll.dll is initialized
// it performs like calling LoadLibrary at the end of ntdll's init source code
{
  BOOLEAN result = FALSE;
  InjectLib32New il;
  PInjectLib32New buf = (PInjectLib32New) AllocMemEx(ProcessHandle, sizeof(InjectLib32New) + sizeof(CInjectLib32FuncNew), NULL);

  il.pEntryPoint = Ndep32;

  if ( (buf) &&
       (ReadProcessMemory(ProcessHandle, il.pEntryPoint, &il.oldEntryPoint, 4)) )
  {
    // we were able to allocate a buffer in the newly created process

    ULONG newEntryPoint;
    ULONG op;

    if (!il.oldEntryPoint)
      il.oldEntryPointFunc = NULL;
    else
      il.oldEntryPointFunc = (PVOID32) ((ULONG_PTR) Ndh32 + il.oldEntryPoint);
    il.NtProtectVirtualMemory = Npvm32;
    il.LdrLoadDll             = Lld32;
    il.dllStr.Length          = (USHORT) (wcslen(LibraryFileName) * 2);
    il.dllStr.MaximumLength   = il.dllStr.Length + 2;
    il.dllStr.Buffer          = (ULONG) ((ULONG_PTR) buf + sizeof(CInjectLib32FuncNew) + ((ULONG_PTR) (&il.dllBuf[0]) - (ULONG_PTR) (&il)));
    memcpy(il.dllBuf, LibraryFileName, il.dllStr.MaximumLength);
    il.movEax = 0xb8;  // mov eax, dw
    il.buf    = (PVOID32) ((ULONG_PTR) buf + sizeof(CInjectLib32FuncNew));
    il.jmp    = 0xe9;  // jmp dw
    il.target = - (int) sizeof(CInjectLib32FuncNew) - (int) sizeof(il);
    newEntryPoint = (ULONG) ((ULONG_PTR) buf + sizeof(CInjectLib32FuncNew) + sizeof(il) - 10 - (ULONG_PTR) Ndh32);
    if ( WriteProcessMemory(ProcessHandle, buf, (PVOID) CInjectLib32FuncNew, sizeof(CInjectLib32FuncNew)) &&
         WriteProcessMemory(ProcessHandle, (PVOID32) ((ULONG_PTR) buf + sizeof(CInjectLib32FuncNew)), &il, sizeof(il)) &&
         VirtualProtectEx(ProcessHandle, il.pEntryPoint, 4, PAGE_EXECUTE_READWRITE, &op) &&
         WriteProcessMemory(ProcessHandle, il.pEntryPoint, &newEntryPoint, 4) )
    {
      VirtualProtectEx(ProcessHandle, il.pEntryPoint, 4, op, &op);
      result = TRUE;
    }

    VirtualProtectEx(ProcessHandle, buf, sizeof(InjectLib32New) + sizeof(CInjectLib32FuncNew), PAGE_EXECUTE_READ, &op);
  }

  return result;
}

BOOLEAN InjectLibraryOld32(HANDLE ProcessHandle, PWCHAR LibraryFileName)
// inject a dll into a newly started 32bit process
// this solution is used by default for all 32bit and 64bit OSs
// the dll is injected after all statically linked dlls are initialized
// it performs like calling LoadLibrary in the 1st source code line of the exe
{
  BOOLEAN result = FALSE;
  #ifdef _WIN64
    BOOLEAN apisKnown = (Npvm32) && (Lld32) && (Navm32) && (Nfvm32) && (Nqsi32) && (Nde32);
    PVOID code = (apisKnown) ? ((PVOID) CInjectLib32FuncOldStatic) : ((PVOID) CInjectLib32FuncOldDynamic);
    int codeSize = (apisKnown) ? (sizeof(CInjectLib32FuncOldStatic)) : (sizeof(CInjectLib32FuncOldDynamic));
  #else
    PVOID code = (PVOID) CInjectLib32FuncOldStatic;
    int codeSize = sizeof(CInjectLib32FuncOldStatic);
  #endif
  PInjectLib32Old buf = (PInjectLib32Old) AllocMemEx(ProcessHandle, sizeof(InjectLib32Old) + codeSize, NULL);

  if (buf)
  {
    // we were able to allocate a buffer in the newly created process

    InjectLib32Old il;
    RelJump rj;
    BOOLEAN secureMem = TRUE;

    il.movEax            = 0xb8;
    il.param             = buf;
    il.movEcx            = 0xb9;
    il.proc              = (PVOID32) &buf->injectFunc;
    il.callEcx           = 0xd1ff;
    il.magic             = 0x4c49636d;  // "mcIL"
    il.next              = NULL;
    if (Nta32)
      il.pOldApi           = Nta32;
    else
      il.pOldApi           = (PVOID32) GetProcessEntryPoint(ProcessHandle);
    il.dll.Length        = (USHORT) wcslen(LibraryFileName) * 2;
    il.dll.MaximumLength = il.dll.Length + 2;
    il.dll.Buffer        = (ULONG) (&buf->dllBuf[0]);
    wcscpy(il.dllBuf, LibraryFileName);
    il.npvm              = Npvm32;
    il.lld               = Lld32;
    il.navm              = Navm32;
    il.nfvm              = Nfvm32;
    il.nqsi              = Nqsi32;
    il.nde               = Nde32;

    if ( (il.pOldApi) &&
         WriteProcessMemory(ProcessHandle, buf, &il, sizeof(InjectLib32Old)) &&
         WriteProcessMemory(ProcessHandle, &(buf->injectFunc), code, codeSize) &&
         ReadProcessMemory (ProcessHandle, il.pOldApi, &rj, 5) &&
         WriteProcessMemory(ProcessHandle, &(buf->oldApi), &rj, 5) )
    {
      // we've successfully initialized the buffer

      if (rj.jmp == 0xe9)
      {
        // there's already a jump instruction
        // maybe another dll injection request is already installed?

        InjectLib32Old buf2;
        PInjectLib32Old bufAddr = (PVOID32) ((ULONG) il.pOldApi + rj.target + 5);
        if ( ReadProcessMemory(ProcessHandle, bufAddr, &buf2, sizeof(buf2)) &&
             ((buf2.magic == 0x4c49636d) || (buf2.magic == 0x4c69636d)) )
        {
          // oh yes, there is and it's compatible to us
          // so instead of installing another patch we just append our dll
          // injection request to the already existing one

          while (buf2.next)
          {
            // there's already more than one dll injection request
            // let's loop through them all to find the end of the queue
            bufAddr = buf2.next;
            if ( (!ReadProcessMemory(ProcessHandle, bufAddr, &buf2, sizeof(buf2))) ||
                 ((buf2.magic != 0x4c49636d) && (buf2.magic != 0x4c69636d)) )
            {
              // the queue is broken somewhere, so we abort
              bufAddr = NULL;
              break;
            }
          }

          if (bufAddr)
          {
            // we found the end of the queue
            ULONG op;
            if (VirtualProtectEx(ProcessHandle, bufAddr, sizeof(InjectLib32Old), PAGE_EXECUTE_READWRITE, &op))
            {
              buf2.next = buf;
              result = WriteProcessMemory(ProcessHandle, bufAddr, &buf2, sizeof(buf2));
              VirtualProtectEx(ProcessHandle, bufAddr, sizeof(InjectLib32Old), op, &op);
              if (buf2.magic == 0x4c69636d)
              {
                // this is the old version, which always has PAGE_EXECUTE_READWRITE access
                // we need to maintain compatability, so we dumb down to the old logic
                secureMem = FALSE;
                il.magic = 0x4c69636d;  // "mciL"
                WriteProcessMemory(ProcessHandle, buf, &il, sizeof(InjectLib32Old));
              }
            }
          }
        }
      }

      if (!result)
      {
        // there's no compatible patch installed yet, so let's install one

        ULONG op;
        if (VirtualProtectEx(ProcessHandle, il.pOldApi, 5, PAGE_EXECUTE_READWRITE, &op))
        {
          // we successfully unprotected the to-be-patched code, so now we can patch it

          rj.jmp    = 0xe9;
          rj.target = (ULONG) buf - (ULONG) il.pOldApi - 5;
          WriteProcessMemory(ProcessHandle, il.pOldApi, &rj, 5);

          // now restore the original page protection

          VirtualProtectEx(ProcessHandle, il.pOldApi, 5, op, &op);

          result = TRUE;
        }
      }
    }

    if (secureMem)
    {
      ULONG op;
      VirtualProtectEx(ProcessHandle, buf, sizeof(InjectLib32Old) + codeSize, PAGE_EXECUTE_READ, &op);
    }
  }

  return result;
}

BOOLEAN InjectLibrary32(HANDLE ProcessHandle, PWCHAR LibraryFileName)
// inject a dll into a newly started 32bit process
// chooses between 2 different methods, depending on the OS
{
  if (UseNewInjectionMethod)
    return InjectLibraryNew32(ProcessHandle, LibraryFileName);
  else
    return InjectLibraryOld32(ProcessHandle, LibraryFileName);
}

// ********************************************************************

BOOLEAN InjectLibrary64(HANDLE ProcessHandle, PWCHAR LibraryFileName)
// inject a dll into a newly started 64bit process
// injection is done by patching "NtTestAlert"
// the dll is injected after all statically linked dlls are initialized
// it performs like calling LoadLibrary in the 1st source code line of the exe
{
  BOOLEAN result = FALSE;

  #ifdef _WIN64

    PInjectLib64 buf = NULL;
    buf = AllocMemEx(ProcessHandle, sizeof(InjectLib64) + sizeof(CInjectLib64Func), Nta64);

    if (buf)
    {
      // we were able to allocate a buffer in the newly created process

      InjectLib64 il;
      RelJump rj;
      BOOLEAN secureMem = TRUE;

      il.movRax            = 0xb848;
      il.retAddr           = Nta64;
      il.pushRax           = 0x50;
      il.pushRcx           = 0x51;
      il.pushRdx           = 0x52;
      il.pushR8            = 0x5041;
      il.pushR9            = 0x5141;
      il.subRsp28          = 0x28ec8348;
      il.movRcx            = 0xb948;
      il.param             = buf;
      il.movRdx            = 0xba48;
      il.proc              = &buf->injectFunc;
      il.jmpEdx            = 0xe2ff;
      il.magic             = 0x4c49636d;  // "mcIL"
      il.next              = NULL;
      il.pOldApi           = il.retAddr;
      il.dll.Length        = (USHORT) wcslen(LibraryFileName) * 2;
      il.dll.MaximumLength = il.dll.Length + 2;
      il.dll.Buffer        = buf->dllBuf;
      wcscpy(il.dllBuf, LibraryFileName);
      il.npvm              = Npvm64;
      il.lld               = Lld64;
      il.navm              = Navm64;
      il.nfvm              = Nfvm64;
      il.nqsi              = Nqsi64;
      il.nde               = Nde64;

      if ( WriteProcessMemory(ProcessHandle, buf, &il, sizeof(InjectLib64)) &&
           WriteProcessMemory(ProcessHandle, &(buf->injectFunc), (PVOID) CInjectLib64Func, sizeof(CInjectLib64Func)) &&
           ReadProcessMemory (ProcessHandle, il.pOldApi, &rj, 5) &&
           WriteProcessMemory(ProcessHandle, &(buf->oldApi), &rj, 5) )
      {
        // we've successfully initialized the buffer

        if (rj.jmp == 0xe9)
        {
          // there's already a jump instruction
          // maybe another dll injection request is already installed?

          InjectLib64 buf2;
          PInjectLib64 bufAddr = (PVOID) ((ULONG_PTR) il.pOldApi + (*((LONG*) (&(rj.target)))) + 5);
          if ( ReadProcessMemory(ProcessHandle, bufAddr, &buf2, sizeof(buf2)) &&
               ((buf2.magic == 0x4c49636d) || (buf2.magic == 0x4c69636d)) )
          {
            // oh yes, there is and it's compatible to us
            // so instead of installing another patch we just append our dll
            // injectioh request to the already existing one

            while (buf2.next)
            {
              // there's already more than one dll injection request
              // let's loop through them all to find the end of the queue
              bufAddr = buf2.next;
              if ( (!ReadProcessMemory(ProcessHandle, bufAddr, &buf2, sizeof(buf2))) ||
                   ((buf2.magic != 0x4c49636d) && (buf2.magic != 0x4c69636d)) )
              {
                // the queue is broken somewhere, so we abort
                bufAddr = NULL;
                break;
              }
            }

            if (bufAddr)
            {
              // we found the end of the queue
              ULONG op;
              if (VirtualProtectEx(ProcessHandle, bufAddr, sizeof(InjectLib64), PAGE_EXECUTE_READWRITE, &op))
              {
                buf2.next = buf;
                result = WriteProcessMemory(ProcessHandle, bufAddr, &buf2, sizeof(buf2));
                VirtualProtectEx(ProcessHandle, bufAddr, sizeof(InjectLib64), op, &op);
                if (buf2.magic == 0x4c69636d)
                {
                  // this is the old version, which always has PAGE_EXECUTE_READWRITE access
                  // we need to maintain compatability, so we dumb down to the old logic
                  secureMem = FALSE;
                  il.magic = 0x4c69636d;  // "mciL"
                  WriteProcessMemory(ProcessHandle, buf, &il, sizeof(InjectLib64));
                }
              }
            }
          }
        }

        if (!result)
        {
          // there's no compatible patch installed yet, so let's install one

          ULONG op;
          if ( VirtualProtectEx(ProcessHandle, il.pOldApi, 5, PAGE_EXECUTE_READWRITE, &op) )
          {
            // we successfully unprotected the to-be-patched code, so now we can patch it

            rj.jmp    = 0xe9;
            rj.target = (ULONG) ((ULONG_PTR) buf - (ULONG_PTR) il.pOldApi - 5);
            WriteProcessMemory(ProcessHandle, il.pOldApi, &rj, 5);

            // now restore the original page protection

            VirtualProtectEx(ProcessHandle, il.pOldApi, 5, op, &op);

            result = TRUE;
          }
        }
      }

      if (secureMem)
      {
        ULONG op;
        VirtualProtectEx(ProcessHandle, buf, sizeof(InjectLib64) + sizeof(CInjectLib64Func), PAGE_EXECUTE_READ, &op);
      }
    }

  #endif

  return result;
}

// ********************************************************************

BOOLEAN InjectLibrary(HANDLE ProcessHandle, PWCHAR LibraryFileName)
// inject a dll into a newly started 32bit or 64bit process
{
  if (Is64bitProcess(ProcessHandle))
    return InjectLibrary64(ProcessHandle, LibraryFileName);
  else
    return InjectLibrary32(ProcessHandle, LibraryFileName);
}

// ********************************************************************
