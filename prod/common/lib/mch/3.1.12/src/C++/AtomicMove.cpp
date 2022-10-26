// ***************************************************************
//  AtomicMove.cpp            version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  move up to 8 bytes atomically (multi cpu safe)
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _ATOMICMOVE_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"
#include <intrin.h>

static BOOL IsCmpXchg8bSupported(void);

#ifdef _WIN64
  extern "C" void WINAPI _CmpXchg8b(LPVOID pSource, LPVOID pDestination, SIZE_T byteCount);
#else
  void WINAPI _CmpXchg8b(LPVOID pSource, LPVOID pDestination, SIZE_T byteCount);
#endif

SYSTEMS_API BOOL WINAPI AtomicMove(LPVOID source, LPVOID destination, int length)
{
  BOOL result = false;

  if ((length > 0) && (length <= 8) && IsCmpXchg8bSupported())
  {
    _CmpXchg8b(source, destination, length);
    result = true;
  }

  return result;
}

// Ask the CPU whether it supports the CmpXchg8b assembler instruction
static BOOL IsCmpXchg8bSupported(void)
{
  #ifdef _WIN64
    // all win64 capable CPUs support cmpxchg8b
    return true;
  #else
    int CPUInfo[4] = {-1};
    __cpuid(CPUInfo, 1);
    return (CPUInfo[3] & 0x100) == 0x100;
  #endif
}

#ifndef _WIN64
// __stdcall (LPVOID pSource, LPVOID pDestination, int byteCount)
__declspec(naked) void WINAPI _CmpXchg8b(LPVOID, LPVOID, SIZE_T)
{
  _asm
  {
                               // stack
                               // 24 36 length
                               // 20 32 destination
                               // 1c 28 source
                               // 18 24 return address
    add   esp, -8              // 14 20 local1, HIGH DWORD of destination value
                               // 10 16 local2, LOW DWORD of destination value
    push  esi                  // 0c 12 esi
    push  edi                  // 08 8  edi
    push  ebx                  // 04 4  ebx
    push  ebp                  // 00 0  ebp

    mov   ebp, [esp+0x20]      // move address of destination to ebp
    mov   eax, ebp             // eax gets address of destination
    mov   edx, [eax+0x04]      // edx gets value at destination+4, HIGH DWORD
    mov   eax, [eax]           // eax gets value at destination, LOW DWORD
                               // EDX:EAX is 8 bytes that destination points to

  TRYAGAIN:
    mov   [esp+0x10], eax      // local2 gets EAX, LOW DWORD
    mov   [esp+0x14], edx      // local1 gets EDX, HIGH DWORD

    mov   esi, [esp+0x1c]      // esi gets source address parameter
    lea   edi, [esp+0x10]      // edi gets address of first byte of local 2
    mov   ecx, [esp+0x24]      // ecx gets length
    cld                        // clear the direction flag
                               // thus, string instructions increment esi/edi
    rep   movsb                // repeat cx times a move of a byte from [esi] to [edi] (cx will be 1-8)
                               // thus, the locals are populated with SOURCE

// we want to write to the destination by using "cmpxchg8b"; this asm instruction always writes 8 bytes
// but in reality we don't want to change 8 bytes, but somewhat less
// so we use a temp buffer of 8 bytes, which are prefilled with the current first 8 bytes of the destination
// if we would write the buffer in this form back to the destination with cmpxchg8b, nothing would change
// but we do want to change something, so we overwrite the first "length" bytes of our temp buffer with the source content
// after this our temp buffer contains exactly what we want to have in the destination later
// the first "length" bytes shall be changed, the remaining bytes (8 - length) shall remain unchanged

    mov ebx, [esp+0x10]        // ebx = LOW DWORD of Source value
    mov ecx, [esp+0x14]        // ecx = HIGH DWORD of Source value

    lock cmpxchg8b [ebp]
    // if (edx:eax == dest)    if DESTINATION 8 bytes in EDX:EAX == original DESTINATION 8 bytes from parameter
    //   dest = ecx:ebx          then [destination] = SOURCE 8 bytes in ECX:EBX atomically
    // else
    //   edx:eax = dest        EDX:EAX is re-set to [destination] 8 bytes atomically

    jnz TRYAGAIN

// cmpxchg8b works this way: we must tell it what value we believe is in "*dest";
// if what we believe is right, cmpxchg8b executes our write request;
// if what we believe is wrong, cmpxchg8b does not execute our write request;
// this sometimes is a useful logic for multi thread purposes
// in our specific case we always want to write, no matter what, so we have to loop until we're done
// basically most of the time cmpxchg8b will succeed the first time it is called here
// however, if two threads try to write to the destination at the same time, the 2nd thread's call to cmpxchg8b
// will fail and must be repeated

// if there was a "lock mov [destination], something" which supported writing of 8 bytes
// simultanously, I wouldn't use cmpxchg8b here; but as things are, cmpxchg8b is the only assembler
// instruction which can write more than 4 bytes atomatically; so I'm misusing it

    pop ebp
    pop ebx
    pop edi
    pop esi

    add esp, 0x08        // reclaim local variables, note ESP never changed, so stack will be fine
    ret 0x0c             // stdcall return, restore stack, 3 4-byte parameters
  }
}
#endif