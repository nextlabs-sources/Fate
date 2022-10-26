// ***************************************************************
//  CCodeHook.cpp            version: 1.0.10  ·  date: 2016-05-19
//  -------------------------------------------------------------
//  API hooking framework
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2016 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2016-05-19 1.0.10 fixed: regression could cause finalization crashes
// 2016-05-17 1.0.9  (1) fixed: some chrome shutdown crashes (when debugging)
//                   (2) fixed: hook uninstall could crash (when debugging)
//                   (3) fixed: SAFE_HOOKING could crash after uninjection
//                   (4) fixed: hook stub was allocated at wrong address (x64)
// 2016-03-16 1.0.8  (1) fixed: x64 jmp/call relocation miscalculation
//                   (2) fixed some PAGE_EXECUTE_READWRITE security issues
// 2015-09-10 1.0.7  (1) fixed: threading issue when to-be-hooked dll is loaded
//                   (2) fixed: some conflicts with other hook libraries (x64)
// 2014-10-26 1.0.6  (1) fixed: RestoreCode sometimes produced incorrect code
//                   (2) fixed: hooking ntdll in non-large-address-aware x64
//                              processes crashed
//                   (3) FOLLOW_JMP now follows up to 10 JMPs in a row
// 2013-10-01 1.0.5  improved FOLLOW_JMP implementation
// 2013-04-26 1.0.4  fixed: FOLLOW_JMP still eventually modified export tables
// 2013-03-13 1.0.3  added new FOLLOW_JMP flag for HookAPI/Code
// 2012-05-21 1.0.2  (1) fixed unhook memory leak
//                   (2) RENEW_OVERWRITTEN_HOOKS can now also affect new hooks
//                   (3) fixed chrome sandbox uninjection problems
// 2010-04-20 1.0.1  (1) fixed bug in rip relative code copy
//                   (2) fixed format ansi/wide mismatch
// 2010-01-10 1.0.0  initial version

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

      #ifdef _WIN64
        // (in case we're later using our MIXTURE_MODE)
        // we need a page which is located *before* mpHookStub
        // this is due to RIP relative addressing
        mpSparePage = VirtualAlloc2(PAGE_SIZE, mpHookedFunction);
        if ( (((ULONG_PTR) mpSparePage > (ULONG_PTR) mpHookedFunction) && ((ULONG_PTR) mpSparePage - (ULONG_PTR) mpHookedFunction >= 0x7fff0000)) ||
             (((ULONG_PTR) mpSparePage < (ULONG_PTR) mpHookedFunction) && ((ULONG_PTR) mpHookedFunction - (ULONG_PTR) mpSparePage >= 0x7fff0000))    )
        {
          // This is probably a 64bit process which is not large address aware.
          // Furthermore the to-be-hooked API is probably an ntdll.dll API.
          // This situation is currently not supported by madCodeHook because the
          // OS doesn't allow madCodeHook to allocate memory near ntdll.dll in this situation.
          VirtualFree(mpSparePage, 0, MEM_RELEASE);
          mValid = false;
          error = CodeNotInterceptableError;
          return;
        }
        __try
        {
      #endif
        // Allocate contiguous buffer for hookStub and inUse array and inUseStub buffer
        // page  1:   PAGE_EXECUTE_READ, ["NextHook" calls this, it's a 6-byte JMP, JMP address stored in page 4][inUseStub]
        // pages 2+3: PAGE_EXECUTE_READ, [inUseCodeArray[IN_USE_COUNT]]
        // page  4:   PAGE_READWRITE,    [JMP address for JMP from page 1][inUseTargetArray[IN_USE_COUNT]]
        #ifdef _WIN64
          mpHookStub = (INDIRECT_ABSOLUTE_JUMP *) VirtualAlloc2(PAGE_SIZE * 4, mpSparePage);
        #else
          mpHookStub = (INDIRECT_ABSOLUTE_JUMP *) VirtualAlloc2(PAGE_SIZE * 4);
        #endif
        mpHookStubTarget = (LPVOID*) (((ULONG_PTR) mpHookStub) + PAGE_SIZE * 3);
        *mpHookStub = CIndirectAbsoluteJump;
        #ifdef _WIN64
          // caution: we need to use RIP relative addressing here!
          mpHookStub->Target = PAGE_SIZE * 3 - 6;
        #else
          mpHookStub->Target = (DWORD) (ULONG_PTR) mpHookStubTarget;
        #endif

        LPVOID pa = mpHookedFunction;
        bool keepMapFile = false;
        mIsWinsock2 = false;
        mPatchExportTable = false;

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
              if ((mNewHook) && (!DoNewHookPrep(flags, &pa, &error, &keepMapFile)))
              {
                mValid = false;
                VirtualFree(mpHookStub, 0, MEM_RELEASE);
                mpHookStub = NULL;
                mpHookStubTarget = NULL;
                return;
              }

              mMapHandle = InitializeQueue(hMap, mapName, pa, keepMapFile);

              if ((flags & NO_SAFE_UNHOOKING) == 0)
              {
                // Enable the save unhooking feature
                mpInUseCodeArray = (IN_USE*) (((ULONG_PTR) mpHookStub) + PAGE_SIZE);
                mpInUseTargetArray = (LPVOID*) (((ULONG_PTR) mpHookStub) + PAGE_SIZE * 3 + sizeof(LPVOID));
                for (int i = 0; i < IN_USE_COUNT; i++)
                  // Not sure about values
                  // But this should be called when used, right now probably empty
                  GetInUseBuffer(&(mpInUseCodeArray[i]), &(mpInUseTargetArray[i]));

                LPVOID p1 = (LPVOID) ((ULONG_PTR) mpHookStub + sizeof(INDIRECT_ABSOLUTE_JUMP));
                // Put callback function in the InUseStub, it will jump to callback when it ends
                GetInUseStubBuffer(p1, mpInUseCodeArray, mpInUseTargetArray, mpCallbackFunction);
                // Then have callback function call InUseStub
                mpCallbackFunction = p1;
              }

              *mpNextHook = mpHookStub;

              // When Enqueue is called, mpHookStubTarget is NULL
              //  Enqueue fills it in so it knows where to jump to.
              //  mpCallbackFunction is the InUseStub (see above) and this is put
              //  at mpTramp+PAGE_SIZE
              Enqueue(mpCallbackFunction, mpHookStubTarget);

              mValid = true;
              if ( (mNewHook) || (GetMadCHookOption(RENEW_OVERWRITTEN_HOOKS)) ||
                   ( (!IsBadReadPtr2(mPatchAddr, sizeof(ABSOLUTE_JUMP))) &&
                     (*(DWORD *) ((ULONG_PTR) mPatchAddr    ) == *(DWORD *)((ULONG_PTR) &mOldCode    )) &&
                     (*( WORD *) ((ULONG_PTR) mPatchAddr + 4) == *( WORD *)((ULONG_PTR) &mOldCode + 4))    ) )
                WritePatch();
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
        DWORD op;
        VirtualProtect((LPVOID) (((ULONG_PTR) mpHookStub) + PAGE_SIZE * 0), PAGE_SIZE * 3, PAGE_EXECUTE_READ, &op);
        VirtualProtect((LPVOID) (((ULONG_PTR) mpHookStub) + PAGE_SIZE * 3), PAGE_SIZE,     PAGE_READWRITE,    &op);
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
  mPatchExportTable = false;

  mSafeHooking = ((flags & SAFE_HOOKING) != 0);
  mNoImproveUnhook = ((flags & NO_IMPROVED_SAFE_UNHOOKING) != 0);

  mpHookStub = NULL;
  mpHookStubTarget = NULL;
  mpInUseCodeArray = NULL;
  mpInUseTargetArray = NULL;
  mpHookQueue = NULL;
  mMapHandle = NULL;

  mPatchAddr = NULL;

  mLeakUnhook = false;
  mDoubleHook = false;
}

BOOL CCodeHook::DoNewHookPrep(DWORD flags, LPVOID *pa, DWORD *error, bool *keepMapFile)
{
  HANDLE map = NULL;

  if ((flags & MIXTURE_MODE) == 0)
  {
    ULONGLONG orgCode;
    CFunctionParse c(mpHookedFunction);
    if ((!c.mIsValid) || (!c.IsInterceptable()) || ((!GetMadCHookOption(DISABLE_CHANGED_CODE_CHECK)) && (WasCodeChanged(mHModule, mpHookedFunction, &orgCode))))
    {
      flags |= MIXTURE_MODE;
      if ((!c.mIsValid) || (!c.IsInterceptable()))
        ; // Not interceptable w/code overwrite
      else
        // Already hooked?
        *error = DoubleHookError;
      if (flags & FOLLOW_JMP)
      {
        BOOL isJmp;
        LPVOID target;
        #ifdef _WIN64
          // check for FRAPS' weird hooking method
          if ( (((PBYTE) mpHookedFunction)[0] == 0x48) && (((PBYTE) mpHookedFunction)[1] == 0xC7) && (((PBYTE) mpHookedFunction)[2] == 0xC0) &&
               (((PBYTE) mpHookedFunction)[7] == 0xff) && (((PBYTE) mpHookedFunction)[8] == 0xe0) )
          {
            isJmp = true;
            target = (LPVOID) (ULONG_PTR) (*((ULONG*) ((ULONG_PTR) mpHookedFunction + 3)));
          }
          else
        #endif
        {
          CCodeParse ci(mpHookedFunction);
          isJmp = ci.mIsJmp;
          target = ci.mTarget;
        }
        if ((isJmp) && (target))
        {
          BOOL isValid;
          BOOL isInterceptable;
          {
            CFunctionParse fi(target);
            isValid = fi.mIsValid;
            isInterceptable = fi.IsInterceptable();
          }
          for (int i1 = 0; i1 < 10; i1++)
            if ((isValid) && (!isInterceptable))
            {
              {
                CCodeParse ci(target);
                isJmp = ci.mIsJmp;
                target = ci.mTarget;
              }
              if ((isJmp) && (target))
              {
                CFunctionParse fi(target);
                isValid = fi.mIsValid;
                isInterceptable = fi.IsInterceptable();
              }
              else
                break;
            }
            else
              break;
          if ((isValid) && (isInterceptable))
          {
            *pa = target;
            flags &= ~MIXTURE_MODE;
            #ifdef _WIN64
              VirtualFree(mpSparePage, 0, MEM_RELEASE);
              VirtualFree(mpHookStub, 0, MEM_RELEASE);
              mpSparePage = VirtualAlloc2(PAGE_SIZE, *pa);
              mpHookStub = (INDIRECT_ABSOLUTE_JUMP *) VirtualAlloc2(PAGE_SIZE * 4, mpSparePage);
              mpHookStubTarget = (LPVOID*) (((ULONG_PTR) mpHookStub) + PAGE_SIZE * 3);
              *mpHookStub = CIndirectAbsoluteJump;
              mpHookStub->Target = PAGE_SIZE * 3 - 6;
            #endif
          }
        }
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
          LPVOID tableStub = VirtualAlloc2(PAGE_SIZE);
        #endif
        GetIndirectAbsoluteJumpBuffer(tableStub, mpHookedFunction);
        DWORD op;
        VirtualProtect(tableStub, PAGE_SIZE, PAGE_EXECUTE_READ, &op);

        pfnCheckProcAddress = ResolveMixtureMode;

        char stubName[MAX_PATH];
        ApiSpecialName(CNamedBuffer, CMixPrefix, tableStub, FALSE, stubName);

        map = CreateLocalFileMapping(stubName, sizeof(LPVOID));

        if (map != NULL)
        {
          LPVOID *buf = (LPVOID *) MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
          if (buf != NULL)
          {
            *buf = mpHookedFunction;
            UnmapViewOfFile((LPCVOID) buf);
          }
          else
          {
            CloseHandle(map);
            map = NULL;
          }
        }

        if ((map != NULL) && (PatchExportTable(mHModule, mpHookedFunction, tableStub, ws2procList)))
        {
          mPatchExportTable = true;

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
          if (map != NULL)
          {
            CloseHandle(map);
            map = NULL;
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

HANDLE CCodeHook::InitializeQueue(HANDLE map, LPCSTR mapName, LPVOID pa, bool keepMapFile)
{
  HANDLE result = NULL;
  mpHookQueue = new CHookQueue();
  if (mNewHook)
  {
    // Copy at least the first 6 bytes of the function to be hooked
    mpTramp = VirtualAlloc2(PAGE_SIZE * 2, pa);
    LPVOID cc1 = pa;
    LPVOID cc2 = mpTramp;
    do
    {
      CCodeParse c(cc1);
      if (c.mIsRelTarget)
      {
        #ifdef _WIN64
          if ((c.mTarget) && (c.mTargetSize == 4) && ((c.mOpCode == 0xe8) || (c.mOpCode == 0xe9)))
          {
            // Relative jumps can only jump 32bit far. When relocating code, the final
            // address could be further away. So we replace relative jmp/call calls with
            // a small assembler stub which supports any 64bit target address without
            // changing any registers.

            if (c.mIsCall)
            {
              *(BYTE*) cc2 = 0xe8;                      // call +0
              cc2 = (LPVOID) ((ULONG_PTR) cc2 + 1);
              *(DWORD*) cc2 = 0;
              cc2 = (LPVOID) ((ULONG_PTR) cc2 + 4);

              *(DWORD*) cc2 = 0x24048348;               // add qword ptr [rsp], 15h
              cc2 = (LPVOID) ((ULONG_PTR) cc2 + 4);
              *(BYTE*) cc2 = 0x15;
              cc2 = (LPVOID) ((ULONG_PTR) cc2 + 1);
            }

            *(BYTE*) cc2 = 0x50;                        // push rax
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 1);

            *(WORD*) cc2 = 0xb848;                      // mov rax, qw
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 2);
            *(ULONG_PTR*) cc2 = (ULONG_PTR) c.mTarget;
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 8);

            *(DWORD*) cc2 = 0x24048748;                 // xchg rax, [rsp]
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 4);

            *(BYTE*) cc2 = 0xc3;                        // ret
            cc2 = (LPVOID) ((ULONG_PTR) cc2 + 1);
          }
          else
        #endif
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
            // kill 16 bit prefix
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
      }
      else
      {
        #ifdef _WIN64
          if ((c.mIsCall || c.mIsJmp) && c.mIsRipRelative)
          {
            // "jmp/call [RIP + someConstant]"

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
              int* pNext = (int*) ((ULONG_PTR) cc2 + ((ULONG_PTR) c.mpNext - (ULONG_PTR) cc1));
              *pDisplacement = (int) ((LONG_PTR) c.mDisplacementDword - (LONG_PTR) pNext);
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
    DWORD op;
    VirtualProtect(mpTramp, PAGE_SIZE, PAGE_EXECUTE_READ, &op);
    VirtualProtect((LPVOID) (((ULONG_PTR) mpTramp) + PAGE_SIZE), PAGE_SIZE, PAGE_READWRITE, &op);

    result = mpHookQueue->Initialize(mapName, mpHookedFunction, pa, mpTramp, keepMapFile);
  }
  else
  {
    result = mpHookQueue->Initialize(map, &mpTramp);
  }
  return result;
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

SYSTEMS_API BOOL WINAPI WasCodeChanged(HMODULE hModule, LPVOID pCode, ULONGLONG *pOrgCode)
{
  BOOL result = FALSE;
  *pOrgCode = 0ULL;
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
              CHAR arr[20];
              memcpy(arr, (LPVOID) ((ULONG_PTR) buf + address - 4), sizeof(arr));
              ApplyRelocation(nh2, hModule, (LPVOID) ((ULONG_PTR) pCode - 4), arr, sizeof(arr) - 4);
              result = (*((DWORD *) ((ULONG_PTR) &arr[4] + 0)) != *((DWORD *) ((ULONG_PTR) pCode + 0))) ||
                       (*((WORD  *) ((ULONG_PTR) &arr[4] + 4)) != *((WORD  *) ((ULONG_PTR) pCode + 4)));
              if ((result) && (*((DWORD *) ((ULONG_PTR) &arr[4] + 8)) == *((DWORD *) ((ULONG_PTR) pCode + 8))))
                memcpy(pOrgCode, &arr[4], sizeof(ULONGLONG));
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
  if ((mValid) && (mpInUseTargetArray != NULL))
    for (int i = 0; i < IN_USE_COUNT; i++)
      if (mpInUseTargetArray[i] != NULL)
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

BOOL CCodeHook::InitRtlDispatchException(HANDLE& hMutex, PVOID& vectoredHandle, LPVOID oldEip, LPVOID newEip)
{
  BOOL result = FALSE;

  vectoredHandle = NULL;
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
  buffer[12] = 0;
  mutexName.Format(L"%S$%x", buffer, GetCurrentProcessId());
  hMutex = CreateMutex(NULL, FALSE, mutexName.GetBuffer());
  if (hMutex != NULL)
  {
    WaitForSingleObject(hMutex, INFINITE);
    gOldEip = (ULONG_PTR) oldEip;
    gNewEip = (ULONG_PTR) newEip;
    if ((pfnAddVectoredExceptionHandler == NULL) || (pfnRemoveVectoredExceptionHandler == NULL))
    {
      #ifndef _WIN64
        LPVOID p = NtProc(CKiUserExceptionDispatcher, true);
        ULONG_PTR pd = (ULONG_PTR) p;
        DWORD p0 = *((DWORD *) (pd + 0));               // mov ecx, [esp+4] ; pContext
        DWORD p4 = *((DWORD *) (pd + 4)) & 0x00ffffff;  // mov ebx, [esp+0] ; pExceptionRecord
        BYTE  p7 = *((BYTE  *) (pd + 7));               // push ecx
        BYTE  p8 = *((BYTE  *) (pd + 8));               // push ebx
        BYTE  p9 = *((BYTE  *) (pd + 9));               // call RtlDispatchException

        DWORD oldProtect;
        if ( (p0 == 0x04244c8b) && (p4 == 0x241c8b) && (p7 == 0x51) && (p8 == 0x53) && (p9 == 0xe8) &&
             (VirtualProtect((LPVOID) (pd + 10), 4, PAGE_EXECUTE_READWRITE, &oldProtect)) )
        {
          pfnRtlDispatchExceptionNext = (PFN_RTL_DISPATCH_EXCEPTION_NEXT) ((LPVOID) (pd + 14 + *((DWORD *) (pd + 10))));
          *((DWORD *) (pd + 10)) = (DWORD) ((ULONG_PTR) RtlDispatchExceptionCallback - pd - 14);
          VirtualProtect((LPVOID) (pd + 10), 4, oldProtect, &oldProtect);
          result = TRUE;
        }
      #endif
    }
    else
    {
      vectoredHandle = pfnAddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER) VectoredExceptionHandler);
      result = (vectoredHandle != NULL);
    }
    if (!result)
    {
      ReleaseMutex(hMutex);
      CloseHandle(hMutex);
    }
  }
  return result;
}

void CCodeHook::CloseRtlDispatchException(HANDLE& hMutex, PVOID vectoredHandle)
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
  }
  else
  {
    pfnRemoveVectoredExceptionHandler(vectoredHandle);
  }
  ReleaseMutex(hMutex);
  CloseHandle(hMutex);
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
    DWORD op;
    if (VirtualProtect(pQueueHeader->pPatchAddress, 8, PAGE_EXECUTE_READWRITE, &op))
    {
      CCodeParse c(pQueueHeader->pPatchAddress);
      PVOID vectoredHandle = NULL;
      BOOL b2 = mSafeHooking && ((ULONG_PTR) c.mpNext - (ULONG_PTR) c.mpCodeAddress < 6) &&
                InitRtlDispatchException(hRdecMutex, vectoredHandle, pQueueHeader->pPatchAddress, mpTramp);
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
                     ((ULONG_PTR) (context.Rip & 0xffffffffffff0000) != (ULONG_PTR) (c2 & 0xffffffffffff0000)) ) )
                break;
            #else
              if ( (!GetThreadContext(threads[i], &context)) ||
                   ( (context.Eip != (ULONG_PTR) pQueueHeader->pPatchAddress) &&
                     ( (context.Eip < p) || (context.Eip > p + 10)) &&
                     ( (ULONG_PTR) (context.Eip & 0xffff0000) != (ULONG_PTR) (c2 & 0xffff0000)) ) )
                break;
            #endif
            Sleep(10);
          }
          CloseHandle(threads[i]);
        }
      }

      VirtualProtect(pQueueHeader->pPatchAddress, 8, op, &op);
      if (b2)
        CloseRtlDispatchException(hRdecMutex, vectoredHandle);
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
            if ((pa) && (mPatchExportTable))
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
      WaitForSingleObject(mutex, INFINITE);

    char mapName[MAX_PATH];
    ApiSpecialName(CNamedBuffer, CMAHPrefix, mpHookedFunction, FALSE, mapName);

    HANDLE memoryMap = (mMapHandle) ? (mMapHandle) : (OpenGlobalFileMapping(mapName, true));
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
               (pHookRecords[i].ppNextHook == mpHookStubTarget) )
          {
            for (int j = i; j <= pHookQueueHeader->ItemCount - 2; j++)
              pHookRecords[j] = pHookRecords[j + 1];
            *pHookRecords[i - 1].ppNextHook = pHookRecords[i].pHookProc;
            pHookQueueHeader->ItemCount--;

            if ((pHookQueueHeader->ItemCount == 2) && (pHookQueueHeader->hMap != NULL))
            {
              if ((!IsBadReadPtr2(pHookQueueHeader->pPatchAddress, sizeof(ABSOLUTE_JUMP))) &&
                  (pHookQueueHeader->pPatchAddress->Opcode == pHookQueueHeader->NewCode.Opcode) &&
                  (pHookQueueHeader->pPatchAddress->modRm == pHookQueueHeader->NewCode.modRm) &&
                  (pHookQueueHeader->pPatchAddress->Target == pHookQueueHeader->NewCode.Target))
              {
                DWORD op;
                if (VirtualProtect(pHookQueueHeader->pPatchAddress, 8, PAGE_EXECUTE_READWRITE, &op))
                {
                  if (!AtomicMove(&pHookQueueHeader->OldCode, pHookQueueHeader->pPatchAddress, sizeof(ABSOLUTE_JUMP)))
                      *(pHookQueueHeader->pPatchAddress) = pHookQueueHeader->OldCode;
                    FlushInstructionCache(GetCurrentProcess(), pHookQueueHeader->pPatchAddress, sizeof(ABSOLUTE_JUMP));
                  VirtualProtect(pHookQueueHeader->pPatchAddress, 8, op, &op);
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
      if (!mMapHandle)
        CloseHandle(memoryMap);
    }

    if (mutex != NULL)
    {
      ReleaseMutex(mutex);
      CloseHandle(mutex);
    }

    if ((!mDoubleHook) && (this->mpNextHook != NULL))
      *mpNextHook = mpHookedFunction;
    if (this->mpInUseCodeArray != NULL)
    {
      CCollection<HANDLE> threads;
      if (!mNoImproveUnhook)
        GetOtherThreadHandles(threads);
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
                   (context.Rip >= (ULONG_PTR) mpInUseCodeArray) &&
                   (context.Rip < ((ULONG_PTR) mpInUseCodeArray + IN_USE_COUNT * IN_USE_SIZE)) )
            #else
              if ( GetThreadContext(threads[i], &context) &&
                   (context.Eip >= (ULONG_PTR) mpInUseCodeArray) &&
                   (context.Eip < ((ULONG_PTR) mpInUseCodeArray + IN_USE_COUNT * IN_USE_SIZE)) )
            #endif
            {
              b2 = FALSE;
              break;
            }
          }
          for (int i = 0; i < IN_USE_COUNT; i++)
            if (mpInUseTargetArray[i] != NULL)
            {
              b2 = FALSE;
              break;
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
      if ((VirtualQuery(pTramp, &mbi, sizeof(mbi)) != sizeof(mbi)) || ((mbi.Protect != PAGE_EXECUTE_READWRITE) && (mbi.Protect != PAGE_EXECUTE_READ)))
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
  this->mpInUseCodeArray = NULL;
  this->mpInUseTargetArray = NULL;
  this->mpHookStub = NULL;
  this->mpHookStubTarget = NULL;
  if (this->mpHookQueue)
  {
    delete this->mpHookQueue;
    mpHookQueue = NULL;
  }
}
