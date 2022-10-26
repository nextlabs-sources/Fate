// ***************************************************************
//  Stubs.h                   version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  dynamic code stub creation
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _STUBS_H
#define _STUBS_H

#ifdef _WIN64
  #define IN_USE_SIZE 16    // sizeof(IN_USE)
  #define IN_USE_COUNT 511  // PAGE_SIZE * 2 / IN_USE_SIZE - 1
#else
  #define IN_USE_SIZE 15    // sizeof(IN_USE)
  #define IN_USE_COUNT 546  // PAGE_SIZE * 2 / IN_USE_SIZE
#endif

#ifdef _WIN64

  #pragma pack(1)
  typedef struct tagInUse
  {
    WORD push;             // 0x35ff
    DWORD pushParam;       // &returnAddress (RIP relative)
    BYTE lock;             // 0xf0
    BYTE rex;              // 0x48
    WORD and;              // 0x2583
    DWORD lockParam;       // &returnAddress (RIP relative)
    BYTE zero;             // 0
    BYTE ret;              // 0xC3
  } IN_USE;
  #pragma pack()

  const IN_USE CInUse =
  {
    0x35ff,  // push
    0x00,    // &returnAddress (RIP relative)
    0xf0,    // lock
    0x48,    // rex
    0x2583,  // and
    0x00,    // &returnAddress (RIP relative)
    0x00,    // 0
    0xc3,    // ret
  };

#else

  #pragma pack(1)
  typedef struct tagInUse
  // push dword ptr [returnAddress]
  // lock and dword ptr [andAddress], 0
  // ret
  {
    WORD push;             // 0x35ff
    LPVOID returnAddress;
    BYTE lock;             // 0xf0
    WORD and;              // 0x2583
    LPVOID andAddress;
    BYTE zero;             // 0
    BYTE ret;              // 0xc3
  } IN_USE;
  #pragma pack()

  const IN_USE CInUse =
  {
    0x35ff,  // push
    0x00,    // retAddr
    0xf0,    // lock
    0x2583,  // and
    0x00,    // andAddr
    0x00,    // 0
    0xc3     // ret
  };

#endif

#undef EXTERN
#ifdef _STUBS_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN void WINAPI GetIndirectAbsoluteJumpBuffer(LPVOID buffer, LPVOID address);
EXTERN void WINAPI GetInUseStubBuffer(LPVOID buffer, LPVOID pInUseCodeArray, LPVOID pInUseTargetArray, LPVOID pCallback);
EXTERN void WINAPI GetInUseBuffer(LPVOID buffer, LPVOID *pReturnAddress);
EXTERN void WINAPI GetLdrLoadCallbackBuffer(BYTE *buffer, DWORD bufferSize, LPVOID pLoadInjectDlls);
EXTERN void WINAPI GetLdrUnloadCallbackBuffer(BYTE *buffer, DWORD bufferSize, LPVOID pMayUnloadLibrary);

#endif