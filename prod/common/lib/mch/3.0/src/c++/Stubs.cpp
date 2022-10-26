// ***************************************************************
//  Stubs.cpp                 version: 1.0.0   date: 2010-01-10
//  -------------------------------------------------------------
//  dynamic code stub creation
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _STUBS_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

void WINAPI GetIndirectAbsoluteJumpBuffer(LPVOID buffer, DWORD *bufferSize, LPVOID address)
{
  if (bufferSize != NULL)
    *bufferSize = 6 + sizeof(LPVOID);
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
#else
  #define INUSE_SIZE 14 // sizeof(IN_USE)
#endif

#pragma warning(disable: 4748) // Disable warning about /GS being disabled due to optimizations being disabled
#pragma optimize("", off)

// Populate InUseStub buffer with copy of InUseStub code and replace tokens
void WINAPI GetInUseStubBuffer(LPVOID buffer, DWORD *bufferSize, LPVOID pInUseArray, LPVOID pCallback)
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

  if (bufferSize != NULL)
    *bufferSize = codeSize;
  if (buffer == NULL)
    return;

  memcpy(buffer, pCode, codeSize);
  ReplaceDword(buffer, codeSize, TOKEN1, (DWORD) pInUseArray);
  #ifdef _WIN64
    *(LPVOID*) ((ULONG_PTR) buffer + codeSize - 8) = pCallback;
  #else
    ReplaceRelativeOffset(buffer, codeSize, TOKEN2, pCallback);
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

        mov   edi, TOKEN1                    // replaced by addresss of InUseArray
        mov   edx, [esp + 0x10]              // return address
        mov   ecx, IN_USE_COUNT              // 280 iterations
        xor   eax, eax
      START:
        lock cmpxchg [edi + 1], edx          // compares InUse.ReturnAddress to 0 (eax)
                                             // if its 0 as well, its available, and it copies edx,
                                             // the return address from the stack, into InUse.ReturnAddress
        jz    SUCCESS
        add   edi, INUSE_SIZE                // size of IN_USE record
        xor   eax, eax
        loop  START
        jmp   QUIT
      SUCCESS:                               // found empty record
        mov   [esp + 0x10], edi              // overwrite return address on stack with IN_USE record address
      QUIT:
        pop   eax
        pop   ecx
        pop   edx
        pop   edi
        jmp   RELATIVE_OFFSET_TOKEN(TOKEN2)  // replaced by CallbackFunction 
      END:
    }
  #endif
}

#pragma optimize("", on)
#pragma warning(default: 4748)

// ----------------- InUseBuffer  --------------------------------
// Populate InUseEntry buffer with copy of InUseEntry code and replace tokens
void WINAPI GetInUseBuffer(LPVOID buffer, DWORD *bufferSize, LPVOID returnAddress)
{
  if (buffer == NULL)
  {
    *bufferSize = sizeof(IN_USE);
    return;
  }
  IN_USE *p = (IN_USE *) buffer;

  *p = CInUse;
  #ifndef _WIN64
    p->andAddress = &p->returnAddress;
  #endif
  p->returnAddress = returnAddress;
}
