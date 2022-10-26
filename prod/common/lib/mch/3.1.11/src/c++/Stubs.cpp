// ***************************************************************
//  Stubs.cpp                 version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  dynamic code stub creation
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _STUBS_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

void WINAPI GetIndirectAbsoluteJumpBuffer(LPVOID buffer, LPVOID address)
{
  if (buffer == NULL)
    return;

  *(WORD*) buffer = 0x25ff;
  buffer = (LPVOID) ((ULONG_PTR) buffer + 2);

  #ifdef _WIN64
    *(DWORD*) buffer = 0;
  #else
    *(LPVOID*) buffer = (LPVOID) ((ULONG_PTR) buffer + 4);
  #endif
   buffer = (LPVOID) ((ULONG_PTR) buffer + 4);

   *(LPVOID*) buffer = address;

   // Disassemble for validation
   //   CFunctionParse c((LPVOID) (ULONG_PTR) buffer - 6));
   //   wprintf(L"%s\n", c.ToString(NULL, 0, TRUE));
}

// Disable truncation for entire file because 64 bit support for this material needs to be addressed
#pragma warning(disable: 4311)

// ----------------- InUseStub ----------------------------
#ifdef _WIN64
  extern "C" void InUseStub();
#endif

#pragma warning(disable: 4748) // Disable warning about /GS being disabled due to optimizations being disabled
#pragma optimize("", off)

// Populate InUseStub buffer with copy of InUseStub code and replace tokens
void WINAPI GetInUseStubBuffer(LPVOID buffer, LPVOID pInUseCodeArray, LPVOID pInUseTargetArray, LPVOID pCallback)
{
  LPVOID   pCode;
  DWORD codeSize;

  #ifdef _WIN64
    CFunctionParse func(InUseStub);
    pCode = InUseStub;
    codeSize = func.mCodeLength + 8;
  #else
    _asm
    {
      mov   eax, offset BEGIN
      mov   [pCode], eax
      mov   edx, offset END
      sub   edx, eax
      mov   [codeSize], edx
    }
  #endif

  if (buffer == NULL)
    return;

  memcpy(buffer, pCode, codeSize);
  #ifdef _WIN64
    ReplaceQword(buffer, codeSize, 0x1111111111111111, (ULONG_PTR) pInUseCodeArray);
    ReplaceQword(buffer, codeSize, 0x2222222222222222, (ULONG_PTR) pInUseTargetArray);
    *(LPVOID*) ((ULONG_PTR) buffer + codeSize - 8) = pCallback;
  #else
    ReplaceDword(buffer, codeSize, TOKEN1, (DWORD) pInUseCodeArray);
    ReplaceDword(buffer, codeSize, TOKEN2, (DWORD) pInUseTargetArray);
    ReplaceRelativeOffset(buffer, codeSize, TOKEN3, pCallback);
  #endif
  // Disassemble for validation
  //   CFunctionParse c(buffer);
  //   wprintf(L"%s\n", c.ToString(NULL, 0, TRUE));

  return;

  #ifndef _WIN64
    _asm
    {
      BEGIN:
        // stack
        // 10 return address
        // 0c edi
        // 08 edx
        // 04 ecx
        // 00 eax   esp__
        push  edi
        push  edx
        push  ecx
        push  eax
        push  ebx

        mov   ebx, TOKEN1                    // replaced by addresss of InUseCodeArray
        mov   edi, TOKEN2                    // replaced by addresss of InUseTargetArray
        mov   edx, [esp + 0x14]              // return address
        mov   ecx, IN_USE_COUNT              // 280 iterations
        xor   eax, eax
      START:
        lock cmpxchg [edi], edx              // compares InUseTargetArray[index] to 0 (eax)
                                             // if its 0 as well, its available, and it copies edx
                                             // (which is the return address from the stack) into InUseTargetArray[index]
        jz    SUCCESS
        add   ebx, IN_USE_SIZE               // size of IN_USE record
        add   edi, 4                         // size of LPVOID
        xor   eax, eax
        loop  START
        jmp   QUIT
      SUCCESS:                               // found empty record
        mov   [esp + 0x14], ebx              // overwrite return address on stack with IN_USE record address
      QUIT:
        pop   ebx
        pop   eax
        pop   ecx
        pop   edx
        pop   edi
        jmp   RELATIVE_OFFSET_TOKEN(TOKEN3)  // replaced by CallbackFunction
      END:
    }
  #endif
}

#pragma optimize("", on)
#pragma warning(default: 4748)

// ----------------- InUseBuffer  --------------------------------
// Populate InUseEntry buffer with copy of InUseEntry code and replace tokens
void WINAPI GetInUseBuffer(LPVOID buffer, LPVOID *pReturnAddress)
{
  if (buffer == NULL)
    return;
  IN_USE *p = (IN_USE *) buffer;

  *p = CInUse;
  #ifdef _WIN64
    p->pushParam = (DWORD) (((ULONG_PTR) pReturnAddress) - ((ULONG_PTR) &(p->pushParam)) - 4);
    p->lockParam = (DWORD) (((ULONG_PTR) pReturnAddress) - ((ULONG_PTR) &(p->lockParam)) - 5);
  #else
    p->returnAddress = pReturnAddress;
    p->andAddress = pReturnAddress;
  #endif
  *pReturnAddress = NULL;
}
