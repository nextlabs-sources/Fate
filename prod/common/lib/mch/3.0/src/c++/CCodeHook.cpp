// ***************************************************************
//  CCodeHook.cpp             version: 1.0.0   date: 2010-01-10
//  -------------------------------------------------------------
//  API hooking framework
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _CCODEHOOK_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

CCodeHook::CCodeHook(HMODULE hModule,
                     LPCSTR apiName,
                     LPVOID hookThisFunction,
                     LPVOID callbackFunction,
                     LPVOID *nextHook,
                     DWORD flags)
{
  DWORD error = 0;

  __try
  {
    CCollectCache::AddReference();
    __try
    {
      Initialize(hModule, apiName, hookThisFunction, callbackFunction, nextHook, flags);

      DWORD hookStubSize = sizeof(INDIRECT_ABSOLUTE_JUMP);
      DWORD inUseSize;
      GetInUseBuffer(NULL, &inUseSize, NULL);
      DWORD inUseArraySize = inUseSize * IN_USE_COUNT;
      DWORD inUseStubSize;
      GetInUseStubBuffer(NULL, &inUseStubSize, NULL, NULL);
      #ifdef _WIN64
        // (in case we're later using our MIXTURE_MODE)
        // we need a page which is located *before* mpHookStub
        // this is due to RIP relative addressing
        mpSparePage = VirtualAlloc2(2048, mpHookedFunction);
        __try
        {
          // Allocate contiguous buffer for hookStub and inUse array and inUseStub buffer
          mpHookStub = (INDIRECT_ABSOLUTE_JUMP *) VirtualAlloc2(hookStubSize + inUseStubSize + inUseArraySize, mpSparePage);
      #else
        // Allocate contiguous buffer for hookStub and inUse array and inUseStub buffer
        mpHookStub = (INDIRECT_ABSOLUTE_JUMP *) VirtualAlloc2(hookStubSize + inUseStubSize + inUseArraySize);
      #endif
        *mpHookStub = CIndirectAbsoluteJump;
        #ifdef _WIN64
          // caution: we need to use RIP relative addressing here!
          mpHookStub->Target = NULL;
        #else
          mpHookStub->Target = (DWORD) (ULONG_PTR) &mpHookStub->AbsoluteAddress;
        #endif

        LPVOID pa = mpHookedFunction;
        bool keepMapFile = false;
        mIsWinsock2 = false;

        char mutexName[MAX_PATH];
        ApiSpecialName(CMutex, CMAHPrefix, mpHookedFunction, FALSE, mutexName);

        HANDLE hMutex = CreateLocalMutex(mutexName);

        __try
        {
          // Gate: One thread can work with this function at a time
          if (hMutex != NULL)
            WaitForSingleObject(hMutex, INFINITE);
          __try
          {
            char mapName[MAX_PATH];
            ApiSpecialName(CNamedBuffer, CMAHPrefix, mpHookedFunction, FALSE, mapName);
            HANDLE hMap = OpenGlobalFileMapping(mapName, true);
            mNewHook = !CheckMap(&hMap);
            __try
            {
              if ((mNewHook) && (!DoNewHookPrep(flags, &pa, &error, &keepMapFile, &hMap)))
              {
                mValid = false;
                return;
              }

              LPVOID pBuffer = NULL;
              __try
              {
                InitializeQueue(&hMap, mapName, pa, keepMapFile);

                if ((flags & NO_SAFE_UNHOOKING) == 0)
                {
                  // Enable the save unhooking feature
                  mpInUseArray = (IN_USE *) ((ULONG_PTR) mpHookStub + hookStubSize + inUseStubSize);
                  for (int i = 0; i < IN_USE_COUNT; i++)
                    // Not sure about values
                    // But this should be called when used, right now probably empty
                    GetInUseBuffer((LPVOID) &mpInUseArray[i], NULL, NULL);

                  LPVOID p1 = (LPVOID) ((ULONG_PTR) mpHookStub + hookStubSize);
                  // Put callback function in the InUseStub, it will jump to callback when it ends
                  GetInUseStubBuffer(p1, NULL, mpInUseArray, mpCallbackFunction);
                  // Then have callback function call InUseStub
                  mpCallbackFunction = p1;
                }

                *mpNextHook = mpHookStub;

                // When Enqueue is called, mpHookStub->Target is NULL
                //  Enqueue fills it in so it knows where to jump to.
                //  mpCallbackFunction is the InUseStub (see above) and this is put
                //  at mpTramp+CTrampSize
                Enqueue(mpCallbackFunction, (LPVOID *) (ULONG_PTR) &mpHookStub->AbsoluteAddress);

                mValid = true;
                WritePatch();
              }
              __finally
              {
                if (pBuffer != NULL)
                  UnmapViewOfFile(pBuffer);
              }
            }
            __finally
            {
              if ((hMap != NULL) && (!mNewHook))
                CloseHandle(hMap);
            }
          }
          __finally
          {
            if (hMutex != NULL)
              ReleaseMutex(hMutex);
          }
        }
        __finally
        {
          if (hMutex != NULL)
            CloseHandle(hMutex);
        }
      #ifdef _WIN64
        }
        __finally
        {
          if (mpSparePage != NULL)
          {
            VirtualFree(mpSparePage, 0, MEM_RELEASE);
            mpSparePage = NULL;
          }
        }
      #endif
    }
    __finally
    {
      CCollectCache::ReleaseReference();
      SetLastError(error);
    }
  }
  __except (ExceptionFilter(L"CCodeHook Constructor", GetExceptionInformation()))
  {
    mValid = false;
  }
}

void CCodeHook::Initialize(HMODULE hModule,
                           LPCSTR,  // apiName
                           LPVOID hookThisFunction,
                           LPVOID callbackFunction,
                           LPVOID *nextHook,
                           DWORD flags)
{
  mValid = false;

  *nextHook = hookThisFunction;
  mHModule = hModule;
  mpHookedFunction = hookThisFunction;
  mpCallbackFunction = callbackFunction;
  mpNextHook = nextHook;
  mIsWinsock2 = false;

  mSafeHooking = ((flags & SAFE_HOOKING) != 0);
  mNoImproveUnhook = ((flags & NO_IMPROVED_SAFE_UNHOOKING) != 0);

  mpHookStub = NULL;
  mpInUseArray = NULL;
  mpHookQueue = NULL;

  mPatchAddr = NULL;
}

BOOL CCodeHook::DoNewHookPrep(DWORD flags, LPVOID *pa, DWORD *error, bool *keepMapFile, HANDLE *hMap)
{
  *hMap = NULL;

  if ((flags & MIXTURE_MODE) == 0)
  {
    CFunctionParse c(mpHookedFunction);
    if ((!c.mIsValid) || (!c.IsInterceptable()))
    {
      // Not interceptable w/code overwrite
      flags |= MIXTURE_MODE;
    }
    else
    {
      ABSOLUTE_JUMP orgCode;
      if ((!GetMadCHookOption(DISABLE_CHANGED_CODE_CHECK)) && (WasCodeChanged(mHModule, mpHookedFunction, &orgCode)))
      {
        // Already hooked?
        flags |= MIXTURE_MODE;
        *error = DoubleHookError;
      }
    }
  }
  if ((flags & MIXTURE_MODE) == 0)
  {
  }
  else  // Mixture Mode is TRUE
  {
    if ((flags & NO_MIXTURE_MODE) == 0) // And is ok to use
    {
      if (mHModule != NULL)
      {
        char buffer[16];
        SString ws2(DecryptStr(CWs2_32, buffer, 16));
        mIsWinsock2 = (mHModule == GetModuleHandle(ws2.GetBuffer()));
      }
      LPVOID *ws2procList = NULL;
      if ((mIsWinsock2) && (flags & ALLOW_WINSOCK2_MIXTURE_MODE))
        ws2procList = FindWs2InternalProcList(mHModule);
      if ((!mIsWinsock2) || (ws2procList))
      {
        #ifdef _WIN64
          LPVOID tableStub = mpSparePage;
          mpSparePage = NULL;
        #else
          DWORD hookStubSize = 0;
          GetIndirectAbsoluteJumpBuffer(NULL, &hookStubSize, NULL);
          LPVOID tableStub = VirtualAlloc2(hookStubSize);
        #endif

        GetIndirectAbsoluteJumpBuffer(tableStub, NULL, mpHookedFunction);

        pfnCheckProcAddress = ResolveMixtureMode;

        char stubName[MAX_PATH];
        ApiSpecialName(CNamedBuffer, CMixPrefix, tableStub, FALSE, stubName);

        *hMap = CreateLocalFileMapping(stubName, sizeof(LPVOID));

        if (*hMap != NULL)
        {
          LPVOID *buf = (LPVOID *) MapViewOfFile(*hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
          if (buf != NULL)
          {
            *buf = mpHookedFunction;
            UnmapViewOfFile((LPCVOID) buf);
          }
          else
          {
            CloseHandle(*hMap);
            *hMap = NULL;
          }
        }

        if ((*hMap != NULL) && (PatchExportTable(mHModule, mpHookedFunction, tableStub, ws2procList)))
        {
          if (mIsWinsock2)
          {
            // we've hooked Winsock2 by using the mixture mode
            // in order to improve the efficieny of our manipulations
            // we're increasing the load count of the Winsock2 dll
            // this way we won't have to redo the hook all the time
            // if the application unloads and reloads the dll
            char buffer[16];
            SString ws2(DecryptStr(CWs2_32, buffer, 16));
            LoadLibraryW(ws2.GetBuffer());
          }

          __try
          {
            PatchMyImportTables(mpHookedFunction, tableStub);
          }
          __except (ExceptionFilter(L"PatchMyImportTables", GetExceptionInformation()))
          {
            // eat it
          }
          *pa = tableStub;
          *keepMapFile = true;
          *error = 0;
        }
        else
        {
          if (*hMap != NULL)
          {
            CloseHandle(*hMap);
            *hMap = NULL;
          }
          VirtualFree(tableStub, 0, MEM_RELEASE);
          if (!*error)
            *error = CodeNotInterceptableError;
          return false;
        }
      }
      else
      {
        // WS2_32.dll cannot be hooked in mixture mode
        *error = CodeNotInterceptableError;
        return false;
      }
    }
    else
    {
      // Mixture mode required by NO_MIXTURE_MODE flag set
      *error = CodeNotInterceptableError;
      return false;
    }
  }
  return true;
}

void CCodeHook::InitializeQueue(HANDLE *phMap, LPCSTR mapName, LPVOID pa, bool keepMapFile)
{
  mpHookQueue = new CHookQueue();
  if (mNewHook)
  {
    // Copy at least the first 6 bytes of the function to be hooked
    mpTramp = VirtualAlloc2(CTrampSize + sizeof(LPVOID), pa);
    LPVOID cc1 = pa;
    LPVOID cc2 = mpTramp;
    do
    {
      CCodeParse c(cc1);
      if (c.mIsRelTarget)
      {
        if (c.mTargetSize == 1)
        {
          if (c.mOpCode != 0xeb)
          {
            // (0x70-0x7f) 1 byte jcc calls -> 4 byte jcc calls
            ((LPBYTE) cc2)[0] = 0x0f;
            ((LPBYTE) cc2)[1] = (BYTE) 0x80 + (BYTE) (c.mOpCode & 0x0f);
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 6);
          }
          else
          {
            *(LPBYTE) cc2 = 0xe9;
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 5);
          }
        }
        else if (c.mTargetSize == 2)
        {
          // kill the 16 bite prefix
          DWORD c1 = (DWORD) ((ULONG_PTR) c.mpNext - (ULONG_PTR) cc1);
          memmove(cc2, (LPVOID) ((ULONG_PTR) cc1 + 1), c1 - 3);
          cc2 = (LPVOID) ((ULONG_PTR) cc2 + c1 + 1);
        }
        else
        {
          DWORD c1 = (DWORD) ((ULONG_PTR) c.mpNext - (ULONG_PTR) cc1);
          memmove(cc2, (LPVOID) ((ULONG_PTR) cc1), c1);
          cc2 = (LPVOID) ((ULONG_PTR) cc2 + c1);
        }
        *((int *) ((ULONG_PTR) cc2 - 4)) = (int) ((ULONG_PTR) c.mTarget - (ULONG_PTR) cc2);
      }
      else
      {
        #ifdef _WIN64
          if ((c.mIsCall || c.mIsJmp) && c.mIsRipRelative)
          {
            // ouch, this is difficult, it's a "jmp/call [RIP + someConstant]"
            // when copying this instruction to another code location, we run into trouble
            // we need to translate this to a chunk of asm code, which will work anywhere
            // this is ugly, but the only general solution to this problem I can think of
            // the underlying problem is that the x64 instruction set doesn't know
            // a way to to encode "jmp/call [64bitConstant]", that's simply not possible

            if (c.mIsCall)
            {
              *(BYTE*) cc2 = 0xe8;                      // call +0
              cc2 = (LPVOID) ((ULONG_PTR) cc2 + 1);
              *(DWORD*) cc2 = 0;
              cc2 = (LPVOID) ((ULONG_PTR) cc2 + 4);

              *(DWORD*) cc2 = 0x24048348;               // add qword ptr [rsp], 18h
              cc2 = (LPVOID) ((ULONG_PTR) cc2 + 4);
              *(BYTE*) cc2 = 0x18;
              cc2 = (LPVOID) ((ULONG_PTR) cc2 + 1);
            }

            *(BYTE*) cc2 = 0x50;                        // push rax
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 1);

            *(WORD*) cc2 = 0xb848;                      // mov rax, qw
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 2);
            *(ULONG_PTR*) cc2 = c.mDisplacementDword;
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 8);

            *(BYTE*) cc2 = 0x48;
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 1);
            *(WORD*) cc2 = 0x008b;                      // mov rax, [rax]
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 2);

            *(DWORD*) cc2 = 0x24048748;                 // xchg rax, [rsp]
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 4);

            *(BYTE*) cc2 = 0xc3;                        // ret
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 1);
          }
          else
        #endif
        {
          DWORD c1 = (DWORD) ((ULONG_PTR) c.mpNext - (ULONG_PTR) cc1);
          memmove(cc2, cc1, c1);
          #ifdef _WIN64
            if (c.mIsRipRelative)
            {
              int* pDisplacement = (int*) ((ULONG_PTR) cc2 + ((ULONG_PTR) c.mpDisplacement - (ULONG_PTR) cc1));
              *pDisplacement = (int) ((LONG_PTR) c.mDisplacementDword - (LONG_PTR) pDisplacement - 4);
            }
          #endif
          cc2 = (LPVOID) ((ULONG_PTR) cc2 + c1);
        }
      }
      cc1 = c.mpNext;
    } while (((ULONG_PTR) cc1 - (ULONG_PTR) pa) < 6);
    // Now in the copied buffer, put the jump back to the original code
    *(LPBYTE) cc2 = 0xe9;
    cc2 = (LPVOID) ((ULONG_PTR) cc2 + 5);
    *(int *) ((ULONG_PTR) cc2 - 4) = (int) ((ULONG_PTR) cc1 - (ULONG_PTR) cc2);

    *phMap = mpHookQueue->Initialize(mapName, mpHookedFunction, pa, mpTramp, keepMapFile);
  }
  else
  {
    mpHookQueue->Initialize(*phMap, &mpTramp);
  }
}

void CCodeHook::Enqueue(LPVOID pHookProc, LPVOID *ppNextHook)
{
  mpHookQueue->AddEntry(pHookProc, ppNextHook);

  mPatchAddr = (ABSOLUTE_JUMP*) mpHookQueue->PatchAddress();
  mNewCode = mpHookQueue->NewCode();
  mOldCode = mpHookQueue->OldCode();
}

BOOL CCodeHook::CheckMap(HANDLE *hMap)
{
  BOOL result = FALSE;
  if ((hMap != NULL) && (*hMap != NULL))
  {
    LPVOID pBuffer = MapViewOfFile(*hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (pBuffer != NULL)
    {
      if (*((LPVOID *) pBuffer) == NULL)
      {
        VERIFY(CloseHandle(*hMap));
        *hMap = NULL;
      }
      VERIFY(UnmapViewOfFile(pBuffer));
    }
    else
    {
      VERIFY(CloseHandle(*hMap));
      *hMap = NULL;
    }
  }
  if (*hMap != NULL)
    result = TRUE;
  return result;
}

void ApplyRelocation(PIMAGE_NT_HEADERS nh, HMODULE hModule, LPVOID pCode, LPVOID pArr, DWORD size)
{
  IMAGE_BASE_RELOCATION *rel1 = (IMAGE_BASE_RELOCATION *) ((ULONG_PTR) hModule + nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
  ULONG_PTR rel2 = (ULONG_PTR) rel1 + nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
  while (((ULONG_PTR) rel1 < rel2) && (rel1->VirtualAddress))
  {
    WORD *item = (WORD *) ((ULONG_PTR) rel1 + sizeof(IMAGE_BASE_RELOCATION));
    for (int i1 = 0; i1 < (int) (rel1->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2; i1++)
    {
      if ((*item & 0xf000) == 0x3000)  // IMAGE_REL_BASED_HIGHLOW - x86
      {
        DWORD *data = (DWORD *) ((ULONG_PTR) hModule + rel1->VirtualAddress + (*item & 0x0fff));
        if ((data >= pCode) && ((ULONG_PTR) data < (ULONG_PTR) pCode + size))
        {
          data = (DWORD *) ((ULONG_PTR) pArr + ((ULONG_PTR) data - (ULONG_PTR) pCode));
          *data = (DWORD) ((ULONG_PTR) *data - nh->OptionalHeader.ImageBase + (ULONG_PTR) hModule);
        }
      }
      else
        if ((*item & 0xf000) == 0xa000)  // IMAGE_REL_BASED_DIR64 - x64
        {
          LONGLONG *data = (LONGLONG *) ((ULONG_PTR) hModule + rel1->VirtualAddress + (*item & 0x0fff));
          if ((data >= pCode) && ((ULONG_PTR) data < (ULONG_PTR) pCode + size))
          {
            data = (LONGLONG *) ((ULONG_PTR) pArr + ((ULONG_PTR) data - (ULONG_PTR) pCode));
            *data = (LONGLONG) ((ULONG_PTR) *data - nh->OptionalHeader.ImageBase + (ULONG_PTR) hModule);
          }
        }
      item++;
    }
    rel1 = (IMAGE_BASE_RELOCATION*) ((ULONG_PTR) rel1 + rel1->SizeOfBlock);
  }
}

BOOL IsCompressedModule(PIMAGE_NT_HEADERS nh1, PIMAGE_NT_HEADERS nh2)
{
  PIMAGE_SECTION_HEADER sh1 = (PIMAGE_SECTION_HEADER) ((ULONG_PTR) nh1 + sizeof(IMAGE_NT_HEADERS));
  return (nh1->OptionalHeader.BaseOfCode   != nh2->OptionalHeader.BaseOfCode  ) ||
         #ifndef _WIN64
           (nh1->OptionalHeader.BaseOfData   != nh2->OptionalHeader.BaseOfData  ) ||
         #endif
         (nh1->FileHeader.NumberOfSections != nh2->FileHeader.NumberOfSections) ||
         ((sh1->Name[0] == 'U') && (sh1->Name[1] == 'P') && (sh1->Name[2] == 'X'));
}

SYSTEMS_API BOOL WINAPI WasCodeChanged(HMODULE hModule, LPVOID pCode, ABSOLUTE_JUMP *pOrgCode)
{
  BOOL result = FALSE;
  memset(pOrgCode, 0, sizeof(ABSOLUTE_JUMP));
  if (hModule != NULL)
  {
    IMAGE_NT_HEADERS *nh1 = GetImageNtHeaders(hModule);
    if ( (nh1) && (pCode > hModule) && ((ULONG_PTR) pCode < ((ULONG_PTR) hModule + GetSizeOfImage(nh1))) )
    {
      LPVOID buf = CCollectCache::CacheModuleFileMap(hModule);
      if (buf)
      {
        DWORD address = VirtualToRaw(nh1, (DWORD) ((ULONG_PTR) pCode - (ULONG_PTR) hModule));
        PIMAGE_NT_HEADERS nh2 = GetImageNtHeaders((HMODULE) buf);
        if ((nh2) && (address < GetSizeOfImage(nh2)))
        {
          __try
          {
            result = (!IsCompressedModule(nh1, nh2)) &&
                     ( (*((DWORD *) ((ULONG_PTR) pCode + 0)) != *((DWORD *) ((ULONG_PTR) buf + address + 0))) ||
                       (*((WORD  *) ((ULONG_PTR) pCode + 4)) != *((WORD  *) ((ULONG_PTR) buf + address + 4)))    );
            if (result)
            {
              CHAR arr[14];
              memcpy(arr, (LPVOID) ((ULONG_PTR) buf + address - 4), sizeof(arr));
              ApplyRelocation(nh2, hModule, (LPVOID) ((ULONG_PTR) pCode - 4), arr, sizeof(arr) - 4);
              memcpy(pOrgCode, &arr[4], sizeof(ABSOLUTE_JUMP));
              result = (*((DWORD *) ((ULONG_PTR) &arr[4] + 0)) != *((DWORD *) ((ULONG_PTR) pCode + 0))) ||
                       (*((WORD  *) ((ULONG_PTR) &arr[4] + 4)) != *((WORD  *) ((ULONG_PTR) pCode + 4)));
            }
          } __except (1) {}
        }
      }
    }
  }
  return result;
}

int CCodeHook::IsInUse()
{
  int result = 0;
  if ((mValid) && (mpInUseArray != NULL))
    for (int i = 0; i < IN_USE_COUNT; i++)
      if (mpInUseArray[i].returnAddress != NULL)
        result++;
  return result;
}

int WINAPI VectoredExceptionHandler(EXCEPTION_POINTERS *pExceptionInfo)
{
  #ifdef _WIN64
    if ( (pExceptionInfo->ContextRecord->Rip == gOldEip) &&
         ( (pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_PRIVILEGED_INSTRUCTION) ||
           ( (pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION) &&
             (pExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 0) &&
             (pExceptionInfo->ExceptionRecord->ExceptionInformation[1] == (ULONG_PTR) -1) ) ) )
    {
      pExceptionInfo->ContextRecord->Rip = gNewEip;
      return -1;
    }
  #else
    if ( (pExceptionInfo->ContextRecord->Eip == gOldEip) &&
         ( (pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_PRIVILEGED_INSTRUCTION) ||
           ( (pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION) &&
             (pExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 0) &&
             (pExceptionInfo->ExceptionRecord->ExceptionInformation[1] == (ULONG_PTR) -1) ) ) )
    {
// disable this warning because ULONG_PTR is a DWORD in 32 bit case, and this code is not compiled for 64bit
#pragma warning(disable: 4244)
      pExceptionInfo->ContextRecord->Eip = gNewEip;
#pragma warning(default: 4244)
      return -1;
    }
  #endif
  return 0;
}

int WINAPI RtlDispatchExceptionCallback(EXCEPTION_RECORD *pExceptionRecord, CONTEXT *pExceptionContext)
{
  int result;
  #ifdef _WIN64
    if ( (pExceptionContext->Rip == gOldEip) &&
         ( (pExceptionRecord->ExceptionCode == STATUS_PRIVILEGED_INSTRUCTION) ||
           ( (pExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION) &&
             (pExceptionRecord->ExceptionInformation[0] == 0) &&
             (pExceptionRecord->ExceptionInformation[1] == (ULONG_PTR) -1) ) ) )
    {
      pExceptionContext->Rip = gNewEip;
      result = -1;
    }
  #else
    if ( (pExceptionContext->Eip == gOldEip) &&
         ( (pExceptionRecord->ExceptionCode == STATUS_PRIVILEGED_INSTRUCTION) ||
           ( (pExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION) &&
             (pExceptionRecord->ExceptionInformation[0] == 0) &&
             (pExceptionRecord->ExceptionInformation[1] == (ULONG_PTR) -1) ) ) )
    {
// disable this warning because ULONG_PTR is a DWORD in 32 bit case, and this code is not compiled for 64bit
#pragma warning(disable: 4244)
      pExceptionContext->Eip = gNewEip;
#pragma warning(default: 4244)
      result = -1;
    }
  #endif
  else
  {
    result = pfnRtlDispatchExceptionNext(pExceptionRecord, pExceptionContext);
  }
  return result;
}

BOOL CCodeHook::InitRtlDispatchException(HANDLE& hMutex, LPVOID oldEip, LPVOID newEip)
{
  BOOL result = FALSE;

  pfnAddVectoredExceptionHandler = (PFN_ADD_VECTORED_EXCEPTION_HANDLER) KernelProc(CAddVecExceptHandler);
  pfnRemoveVectoredExceptionHandler = (PFN_REMOVE_VECTORED_EXCEPTION_HANDLER) KernelProc(CRemoveVecExceptHandler);
  SString mutexName;
  char buffer[32];
  DecryptStr(CMutex, buffer, 32);
  lstrcatA(buffer, ", ");
  buffer[ 7] = 'r';
  buffer[ 8] = 'd';
  buffer[ 9] = 'e';
  buffer[10] = 'c';
  buffer[11] = ' ';
  mutexName.Format(L"%s$%x", buffer, GetCurrentProcessId());
  hMutex = CreateMutex(NULL, FALSE, mutexName);
  if (hMutex != NULL)
  {
    WaitForSingleObject(hMutex, INFINITE);
    gOldEip = (ULONG_PTR) oldEip;
    gNewEip = (ULONG_PTR) newEip;
    if ((pfnAddVectoredExceptionHandler == NULL) || (pfnRemoveVectoredExceptionHandler == NULL))
    {
      // When would the above not be found?
      // Can this section be explained:
      LPVOID p = NtProc(CKiUserExceptionDispatcher, true);
      ULONG_PTR pd = (ULONG_PTR) p;
      DWORD p0 = *((DWORD *) (pd + 0));               // mov ecx, [esp+4] ; pContext
      DWORD p4 = *((DWORD *) (pd + 4)) & 0x00ffffff;  // mov ebx, [esp+0] ; pExceptionRecord
      BYTE  p7 = *((BYTE  *) (pd + 7));               // push ecx
      BYTE  p8 = *((BYTE  *) (pd + 8));               // push ebx
      BYTE  p9 = *((BYTE  *) (pd + 9));               // call RtlDispatchException

      DWORD oldProtect;
      if ( (p0 == 0x04244c8b) && (p4 == 0x241c8b) && (p7 == 0x51) && (p8 == 0x53) && (p9 == 0xe8) &&
           VirtualProtect((LPVOID) (pd + 10), 4, PAGE_EXECUTE_READWRITE, &oldProtect) )
      {
        pfnRtlDispatchExceptionNext = (PFN_RTL_DISPATCH_EXCEPTION_NEXT) ((LPVOID) (pd + 14 + *((DWORD *) (pd + 10))));
        *((DWORD *) (pd + 10)) = (DWORD) ((ULONG_PTR) RtlDispatchExceptionCallback - pd - 14);
        VirtualProtect((LPVOID) (pd + 10), 4, oldProtect, &oldProtect);
        result = TRUE;
      }
    }
    else
    {
      LPVOID pExceptionHandler = pfnAddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER) VectoredExceptionHandler);
      result = (pExceptionHandler != NULL);
    }
  }
  if (!result)
  {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
  }
  return result;
}

void CCodeHook::CloseRtlDispatchException(HANDLE& hMutex)
{
  if ((pfnAddVectoredExceptionHandler == NULL) || (pfnRemoveVectoredExceptionHandler == NULL))
  {
    ULONG_PTR p = (ULONG_PTR) NtProc(CKiUserExceptionDispatcher, true);
    DWORD oldProtect;
    if (VirtualProtect((LPVOID) (p + 10), 4, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
      *((DWORD *) (p + 10)) = (DWORD) ((ULONG_PTR) pfnRtlDispatchExceptionNext - p - 14);
      VirtualProtect((LPVOID) (p + 10), 4, oldProtect, &oldProtect);
    }
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
  }
  else
  {
    pfnRemoveVectoredExceptionHandler(VectoredExceptionHandler);
  }
}

LPVOID CCodeHook::DoWritePatch(LPVOID pBuffer)
{
  HANDLE hRdecMutex = NULL;
  LPVOID pa = NULL;

  LPVOID pData = *((LPVOID *) pBuffer);  // pBuffer points to an address
  HOOK_QUEUE_HEADER *pQueueHeader = (HOOK_QUEUE_HEADER *) pData;
  if ( (*(DWORD *) pQueueHeader->pPatchAddress !=  *(DWORD *) &pQueueHeader->NewCode) ||
       (*(WORD  *) ((ULONG_PTR) pQueueHeader->pPatchAddress + 4) != *(WORD *) ((ULONG_PTR) &pQueueHeader->NewCode + 4)) )
  {
    pa = pQueueHeader->pPatchAddress;
    BOOL isMemoryProtected = IsMemoryProtected(pQueueHeader->pPatchAddress);
    if ((!isMemoryProtected) || (UnprotectMemory(pQueueHeader->pPatchAddress, 8)))
    {
      CCodeParse c(pQueueHeader->pPatchAddress);
      BOOL b2 = mSafeHooking && ((ULONG_PTR) c.mpNext - (ULONG_PTR) c.mpCodeAddress < 6) &&
                InitRtlDispatchException(hRdecMutex, pQueueHeader->pPatchAddress, mpTramp);
      if (b2)
      {
        BYTE halt = 0xf4;
        if (!AtomicMove(&halt, pQueueHeader->pPatchAddress, 1))
          *(BYTE *) pQueueHeader->pPatchAddress = halt;
        FlushInstructionCache(GetCurrentProcess(), pQueueHeader->pPatchAddress, 1);

        // Make sure no threads are within the code we are about to patch
        CCollection<HANDLE> threads;
        GetOtherThreadHandles(threads);
        for (int i = 0; i < threads.GetCount(); i++)
        {
          while (TRUE)
          {
            CONTEXT context;
            context.ContextFlags = CONTEXT_CONTROL;
            #ifdef _WIN64
              if ( (!GetThreadContext(threads[i], &context)) ||
                   (context.Rip <= (ULONG_PTR) pQueueHeader->pPatchAddress) ||
                   (context.Rip >= ((ULONG_PTR) pQueueHeader->pPatchAddress + 6)) )
                break;
            #else
              if ( (!GetThreadContext(threads[i], &context)) ||
                   (context.Eip <= (ULONG_PTR) pQueueHeader->pPatchAddress) ||
                   (context.Eip >= ((ULONG_PTR) pQueueHeader->pPatchAddress + 6)) )
                break;
            #endif
            Sleep(10);
          }
          CloseHandle(threads[i]);
        }
      }

      if (!AtomicMove((LPVOID) &pQueueHeader->NewCode, pQueueHeader->pPatchAddress, sizeof(ABSOLUTE_JUMP)))
        *pQueueHeader->pPatchAddress = pQueueHeader->NewCode;
      FlushInstructionCache(GetCurrentProcess(), pQueueHeader->pPatchAddress, sizeof(ABSOLUTE_JUMP));

      if (b2)
      {
        ULONG_PTR p = (ULONG_PTR) NtProc(CKiUserExceptionDispatcher);
        ULONG_PTR c2 = p + 14 + *(DWORD *) (p + 10);

        // What does this do?
        CCollection<HANDLE> threads;
        GetOtherThreadHandles(threads);
        for (int i = 0; i < threads.GetCount(); i++)
        {
          while (TRUE)
          {
            CONTEXT context;
            context.ContextFlags = CONTEXT_CONTROL;
            #ifdef _WIN64
              if ( (!GetThreadContext(threads[i], &context)) ||
                   ( (context.Rip != (ULONG_PTR) pQueueHeader->pPatchAddress) &&
                     ((context.Rip < p) || (context.Rip > p + 10)) &&
                     ((ULONG_PTR) (context.Rip && 0xffffffffffff0000) != (ULONG_PTR) (c2 & 0xffffffffffff0000)) ) )
                break;
            #else
              if ( (!GetThreadContext(threads[i], &context)) ||
                   ( (context.Eip != (ULONG_PTR) pQueueHeader->pPatchAddress) &&
                     ( (context.Eip < p) || (context.Eip > p + 10)) &&
                     ( (ULONG_PTR) (context.Eip && 0xffff0000) != (ULONG_PTR) (c2 & 0xffff0000)) ) )
                break;
            #endif
            Sleep(10);
          }
          CloseHandle(threads[i]);
        }
      }

      if (isMemoryProtected)
        ProtectMemory(pQueueHeader->pPatchAddress, 8);
      if (b2)
        CloseRtlDispatchException(hRdecMutex);
    }
  }
  return pa;
}

void CCodeHook::WritePatch()
{
  LPVOID pa = NULL;

  if (mValid)
  {
    char mutexName[MAX_PATH];
    ApiSpecialName(CMutex, CMAHPrefix, mpHookedFunction, FALSE, mutexName);
    HANDLE hMutex = CreateLocalMutex(mutexName);
    if (hMutex != NULL)
    {
      __try
      {
        WaitForSingleObject(hMutex, INFINITE);
        __try
        {
          char mapName[MAX_PATH];
          ApiSpecialName(CNamedBuffer, CMAHPrefix, mpHookedFunction, FALSE, mapName);
          HANDLE hMap = OpenGlobalFileMapping(mapName, true);
          if (hMap != NULL)
          {
            __try
            {
              LPVOID pBuffer = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
              if (pBuffer != NULL)
              {
                __try
                {
                  pa = DoWritePatch(pBuffer);
                }
                __finally
                {
                  UnmapViewOfFile(pBuffer);
                }
              }
            }
            __finally
            {
              CloseHandle(hMap);
            }
            if ((pa) && (mpHookedFunction != pa))
            {
              LPVOID *ws2procList = NULL;
              if (mIsWinsock2)
                ws2procList = FindWs2InternalProcList(mHModule);
              if ((!mIsWinsock2) || (ws2procList))
                PatchExportTable(mHModule, mpHookedFunction, pa, ws2procList);
            }
          }
        }
        __finally
        {
          ReleaseMutex(hMutex);
        }
      }
      __finally
      {
        CloseHandle(hMutex);
      }
    }
  }
}

CCodeHook::~CCodeHook()
{
  mDestroying = true;
  if (mValid)
  {
    LPVOID pTramp = NULL;
    LPVOID pQueue = NULL;

    char mutexName[MAX_PATH];
    ApiSpecialName(CMutex, CMAHPrefix, mpHookedFunction, FALSE, mutexName);

    HANDLE mutex = CreateLocalMutex(mutexName);

    if (mutex != NULL)
    {
      WaitForSingleObject(mutex, INFINITE);

      char mapName[MAX_PATH];
      ApiSpecialName(CNamedBuffer, CMAHPrefix, mpHookedFunction, FALSE, mapName);

      HANDLE memoryMap = OpenGlobalFileMapping(mapName, true);
      if (memoryMap != NULL)
      {
        LPVOID *pBuffer = (LPVOID *) MapViewOfFile(memoryMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (pBuffer != NULL)
        {
          HOOK_QUEUE_HEADER *pHookQueueHeader = (HOOK_QUEUE_HEADER *) *pBuffer;
          HOOK_QUEUE_RECORD *pHookRecords = (HOOK_QUEUE_RECORD *) ((ULONG_PTR) *pBuffer + sizeof(HOOK_QUEUE_HEADER));

          for (int i = 1; i <= pHookQueueHeader->ItemCount - 2; i++)
          {
            if ( (pHookRecords[i].pHookProc == mpCallbackFunction) &&
                 (pHookRecords[i].ppNextHook == &mpHookStub->AbsoluteAddress) )
            {
              for (int j = i; j <= pHookQueueHeader->ItemCount - 2; j++)
                pHookRecords[j] = pHookRecords[j + 1];
              *pHookRecords[i - 1].ppNextHook = pHookRecords[i].pHookProc;
              pHookQueueHeader->ItemCount--;

              if ((pHookQueueHeader->ItemCount == 2) && (pHookQueueHeader->hMap != NULL))
              {
                if ((!IsBadReadPtr(pHookQueueHeader->pPatchAddress, sizeof(ABSOLUTE_JUMP))) &&
                    (pHookQueueHeader->pPatchAddress->Opcode == pHookQueueHeader->NewCode.Opcode) &&
                    (pHookQueueHeader->pPatchAddress->modRm == pHookQueueHeader->NewCode.modRm) &&
                    (pHookQueueHeader->pPatchAddress->Target == pHookQueueHeader->NewCode.Target))
                {
                  BOOL b = IsMemoryProtected(pHookQueueHeader->pPatchAddress);
                  if (UnprotectMemory(pHookQueueHeader->pPatchAddress, 8))
                  {
                    if (!AtomicMove(&pHookQueueHeader->OldCode, pHookQueueHeader->pPatchAddress, sizeof(ABSOLUTE_JUMP)))
                        *(pHookQueueHeader->pPatchAddress) = pHookQueueHeader->OldCode;
                      FlushInstructionCache(GetCurrentProcess(), pHookQueueHeader->pPatchAddress, sizeof(ABSOLUTE_JUMP));
                    if (b)
                      ProtectMemory(pHookQueueHeader->pPatchAddress, 8);
                    pTramp = pHookRecords[1].pHookProc;
                  }
                }
                pQueue = *pBuffer;
                *pBuffer = NULL;
                CloseHandle(pHookQueueHeader->hMap);
              }
              break;
            }
          }
          UnmapViewOfFile(pBuffer);
        }
        CloseHandle(memoryMap);
      }
      ReleaseMutex(mutex);
      CloseHandle(mutex);
    }

    if (this->mpNextHook != NULL)
      *mpNextHook = mpHookedFunction;
    if (this->mpInUseArray != NULL)
    {
      DWORD inUseSize;
      CCollection<HANDLE> threads;
      if (!mNoImproveUnhook)
      {
        GetOtherThreadHandles(threads);
        GetInUseBuffer(NULL, &inUseSize, NULL);
        DWORD inUseStubSize;
        GetInUseStubBuffer(NULL, &inUseStubSize, NULL, NULL);
        inUseSize = inUseStubSize + inUseSize * IN_USE_COUNT;
      }
      else
      {
        inUseSize = 0;
      }
      BOOL b1 = FALSE;
      __try
      {
        while (TRUE)
        {
          BOOL b2 = TRUE;
          for (int i = 0; i < threads.GetCount(); i++)
          {
            CONTEXT context;
            context.ContextFlags = CONTEXT_CONTROL;
            #ifdef _WIN64
              if ( GetThreadContext(threads[i], &context) &&
                   (context.Rip >= (ULONG_PTR) mpInUseArray) &&
                   (context.Rip < ((ULONG_PTR) mpInUseArray + inUseSize)) )
            #else
              if ( GetThreadContext(threads[i], &context) &&
                   (context.Eip >= (ULONG_PTR) mpInUseArray) &&
                   (context.Eip < ((ULONG_PTR) mpInUseArray + inUseSize)) )
            #endif
            {
              b2 = FALSE;
              break;
            }
          }
          for (int i = 0; i < IN_USE_COUNT; i++)
          {
            if (mpInUseArray[i].returnAddress != NULL)
            {
              b2 = FALSE;
              break;
            }
          }
          if (b2)
            break;
          b1 = TRUE;
          if (mLeakUnhook)
            return;
          Sleep(500);
        }
      }
      __finally
      {
        for (int i = 0; i < threads.GetCount(); i++)
          CloseHandle(threads[i]);
      }
      if (b1)
        Sleep(500);
    }
    if (pTramp != NULL)
    {
      MEMORY_BASIC_INFORMATION mbi;
      if ((VirtualQuery(pTramp, &mbi, sizeof(mbi)) != sizeof(mbi)) || (mbi.Protect != PAGE_EXECUTE_READWRITE))
        LocalFree((HLOCAL) pTramp);
      else
        VirtualFree(pTramp, 0, MEM_RELEASE);
    }
    if (pQueue != NULL)
    {
      VirtualFree(pQueue, 0, MEM_RELEASE);
    }
  }
  VirtualFree(this->mpHookStub, 0, MEM_RELEASE);
  this->mpInUseArray = NULL;
  this->mpHookStub = NULL;
}
