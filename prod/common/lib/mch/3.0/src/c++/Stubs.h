// ***************************************************************
//  Stubs.h                   version: 1.0.0   date: 2010-01-10
//  -------------------------------------------------------------
//  dynamic code stub creation
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _STUBS_H
#define _STUBS_H

#ifdef _WIN64

  #pragma pack(1)
  typedef struct tagInUse
  {
    WORD push;             // 0x35ff
    DWORD pushParam;       // &returnAddress (RIP + 0xa)
    BYTE lock;             // 0xf0
    BYTE rex;              // 0x48
    WORD and;              // 0x2583
    DWORD lockParam;       // &returnAddress (RIP + 0x1)
    BYTE zero;             // 0
    BYTE ret;              // 0xC3
    LPVOID returnAddress;
  } IN_USE;
  #pragma pack()

  const IN_USE CInUse =
  {
    0x35ff,  // push
    0x000a,  // &returnAddress (RIP + 0xa)
    0xf0,    // lock
    0x48,    // rex
    0x2583,  // and
    0x0001,  // &returnAddress (RIP + 0x1)
    0x00,    // 0
    0xc3,    // ret
    0x0000   // returnAddress
  };

#else

  #pragma pack(1)
  typedef struct tagInUse
  // push returnAddress
  // lock and address, 0   ; for mp, assures address not changed during next instruction
  // ret
  {
    BYTE push;             // 0x68
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
    0x68,    // push
    0x0000,  // retAddr
    0xf0,    // lock
    0x2583,  // and
    0x0000,  // &retAddr
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

EXTERN void WINAPI GetIndirectAbsoluteJumpBuffer(LPVOID buffer, DWORD *bufferSize, LPVOID address);
EXTERN void WINAPI GetInUseStubBuffer(LPVOID buffer, DWORD *bufferSize, LPVOID pInUseArray, LPVOID pCallback);
EXTERN void WINAPI GetInUseBuffer(LPVOID buffer, DWORD *bufferSize, LPVOID returnAddress);
EXTERN void WINAPI GetLdrLoadCallbackBuffer(BYTE *buffer, DWORD bufferSize, LPVOID pLoadInjectDlls);
EXTERN void WINAPI GetLdrUnloadCallbackBuffer(BYTE *buffer, DWORD bufferSize, LPVOID pMayUnloadLibrary);

#endif