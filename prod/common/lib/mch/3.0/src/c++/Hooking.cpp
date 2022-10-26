// ***************************************************************
//  Hooking.cpp               version: 1.0.0   date: 2010-01-10
//  -------------------------------------------------------------
//  hooking groundwork
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _HOOKING_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

static HANDLE ghAutoUnhookMap = NULL;

static BOOL HookCodeInternal(HMODULE callingModule, HMODULE hModule, LPCSTR moduleName, LPCSTR procName,
                             LPVOID pCode, LPVOID pCallback, LPVOID *pNextHook, DWORD flags = 0);

static BOOL UnhookInternal(LPVOID *pNextHook, BOOL dontUnhookHelperHooks, BOOL wait);

void AutoUnhook(HMODULE hModule);

static HMODULE (WINAPI *pfnLoadLibraryExWNextHook)(LPCWSTR libraryName, HANDLE hFile, DWORD flags) = NULL;
static HMODULE WINAPI LoadLibraryExWCallbackProc(LPCWSTR libraryName, HANDLE hFile, DWORD flags);

static DWORD (WINAPI *pfnLdrLoadDllNextHook)(DWORD path, DWORD *flags, UNICODE_STRING *name, HANDLE *handle) = NULL;
static DWORD WINAPI LdrLoadDllCallbackProc(DWORD path, DWORD *flags, UNICODE_STRING *name, HANDLE *handle);

// ------------------ Library Initialization Routines ------------------------------------

BOOL OpenHooking(void)
{
  BOOL result = false;
  __try
  {
    InitializeCriticalSection(&gHookCriticalSection);
    g_pHookCollection = new CCollection<HOOK_ITEM, CStructureEqualHelper<HOOK_ITEM>>;
    gHookReady = true;
    result = true;
  }
  __except (ExceptionFilter(L"InitializeHooking", GetExceptionInformation()))
  {
    result = false;
  }
  return result;
}

SYSTEMS_API void WINAPI AutoUnhook2(HMODULE hModule, BOOL wait);

BOOL CloseHooking(void)
{
  BOOL result = false;
  __try
  {
    AutoUnhook2((HMODULE) -1, false);
    if (gHookReady && (g_pHookCollection->GetCount() == 0))
    {
      if (ghAutoUnhookMap != NULL)
      {
        CloseHandle(ghAutoUnhookMap);
        ghAutoUnhookMap = 0;
      }
      gHookReady = false;
      DeleteCriticalSection(&gHookCriticalSection);
      delete g_pHookCollection;
    }

    result = true;
  }
  __except (ExceptionFilter(L"CloseHooking", GetExceptionInformation()))
  {
    result = false;
  }
  return result;
}
// ------------------------------- EXPORTS -------------------------------------------------------

SYSTEMS_API BOOL WINAPI HookCode(LPVOID pCode, LPVOID pCallback, LPVOID *pNextHook, DWORD flags)
{
  TraceVerbose(L"HookCode(%p, %p, %p, %08x)", pCode, pCallback, pNextHook, flags);

  ASSERT(gHookReady);

  BOOL result = false;
  __try
  {
    if (pCode != NULL)
    {
      pCode = FindRealCode(pCode);
      HMODULE hModule = NULL;
      char moduleName[MAX_PATH];
      char apiName[MAX_PATH];
      *moduleName = L'\0';
      *apiName = '\0';
      if (FindModule(pCode, &hModule, moduleName, MAX_PATH))
      {
        if (!GetImageProcName(hModule, pCode, apiName, MAX_PATH))
        {
          hModule = NULL;
          *moduleName = L'\0';
        }
      }
      HMODULE hOwner = GetCallingModule();
      result = HookCodeInternal(hOwner, hModule, moduleName, apiName, pCode, pCallback, pNextHook, flags);
    }
    else
      SetLastError(ERROR_INVALID_PARAMETER);
  }
  __except (ExceptionFilter(L"HookCode", GetExceptionInformation()))
  {
    result = false;
  }
  return result;
}

SYSTEMS_API BOOL WINAPI HookAPI(LPCSTR moduleName, LPCSTR apiName, LPVOID pCallback, LPVOID *pNextHook, DWORD flags)
{
  TraceVerbose(L"%S(%S, %S, %p, %p, %08X)", __FUNCTION__, moduleName, apiName, pCallback, pNextHook, flags);

  ASSERT(gHookReady);

  BOOL result = false;

  __try
  {
    if ((moduleName != NULL) && (apiName != NULL))
    {
      LPVOID pCode = NULL;
      char procName[MAX_PATH];

      HMODULE hModule = GetModuleHandleA(moduleName);
      if (hModule != NULL)
      {
        pCode = GetImageProcAddress(hModule, apiName, TRUE);
        if (pCode != NULL)
        {
          pCode = FindRealCode(pCode);
          if (GetImageProcName(hModule, pCode, procName, MAX_PATH))
            apiName = procName;
        }
      }
      HMODULE hOwner = GetCallingModule();
      result = HookCodeInternal(hOwner, hModule, moduleName, apiName, pCode, pCallback, pNextHook, flags);
    }
    else
      SetLastError(ERROR_INVALID_PARAMETER);
  }
  __except (ExceptionFilter(L"HookCode", GetExceptionInformation()))
  {
    result = false;
  }
  return result;
}

SYSTEMS_API BOOL WINAPI Unhook(LPVOID *pNextHook)
{
  TraceVerbose(L"Unhook(%p)", pNextHook);

  ASSERT(gHookReady);

  return UnhookInternal(pNextHook, false, true);
}

SYSTEMS_API BOOL WINAPI UnhookCode(LPVOID *pNextHook)
{
  TraceVerbose(L"UnhookCode(%p)", pNextHook);

  ASSERT(gHookReady);

  return UnhookInternal(pNextHook, false, true);
}

SYSTEMS_API BOOL WINAPI UnhookAPI(LPVOID *pNextHook)
{
  TraceVerbose(L"UnhookAPI(%p)", pNextHook);

  ASSERT(gHookReady);

  return UnhookInternal(pNextHook, false, true);
}

inline CCodeHook* FindCodeHook(LPVOID* pNextHook)
{
  CCodeHook *pCodeHook = NULL;
  EnterCriticalSection(&gHookCriticalSection);
  __try
  {
    for (int i = g_pHookCollection->GetCount() - 1; i >= 0; i--)
    {
      if ((*g_pHookCollection)[i].pNextHook == pNextHook)
      {
        pCodeHook = (*g_pHookCollection)[i].pCodeHook;
        break;
      }
    }
  }
  __finally
  {
    LeaveCriticalSection(&gHookCriticalSection);
  }
  return pCodeHook;
}

SYSTEMS_API BOOL WINAPI RenewHook(LPVOID *pNextHook)
{
  TraceVerbose(L"RenewHook(%p)", pNextHook);

  ASSERT(gHookReady);

  BOOL result = false;
  DWORD lastError = GetLastError();

  __try
  {
    CCodeHook *pCodeHook = FindCodeHook(pNextHook);

    if (pCodeHook != NULL)
      pCodeHook->WritePatch();
    result = true;
  }
  __except (ExceptionFilter(L"RenewHook", GetExceptionInformation()))
  {
    result = false;
  }

  SetLastError(lastError);
  return result;
}

SYSTEMS_API int WINAPI IsHookInUse(LPVOID *pNextHook)
{
  TraceVerbose(L"IsHookInUse(%p)", pNextHook);

  ASSERT(gHookReady);

  int result = 0;
  DWORD lastError = GetLastError();

  __try
  {
    CCodeHook *pCodeHook = FindCodeHook(pNextHook);

    if (pCodeHook != NULL)
      result = pCodeHook->IsInUse();
  }
  __except (ExceptionFilter(L"IsHookInUse", GetExceptionInformation()))
  {
    result = 0;
  }

  SetLastError(lastError);
  return result;
}

// ----------------------------- INTERNAL FUNCTIONS --------------------------------------

static bool LoadLibraryHooked = false;
static bool LoadLibraryExWDone = false;
static PVOID LoadLibraryCallsNtDll = NULL;

void HookLoadLibrary(void);
void UnhookLoadLibrary(BOOL wait);

static BOOL HookCodeInternal(HMODULE hOwner, HMODULE hModule, LPCSTR moduleName, LPCSTR procName,
                             LPVOID pCode, LPVOID pCallback, LPVOID *pNextHook, DWORD flags)
{
  TraceVerbose(L"%S(%p, %p, %S, %S, %p, %p, %p, %p)", __FUNCTION__, hOwner, hModule, moduleName, procName, pCode, pCallback, pNextHook, flags);

  BOOL result = false;

  __try
  {
    InitKernelProcs();
    SetLastError(0);

    CCodeHook *pCodeHook = NULL;
    if (pCode != NULL)
    {
      __try
      {
        if (hModule == NULL)
        {
          char dummyModuleName[MAX_PATH + 1];
          HMODULE hDummyModule;
          if (!FindModule(pCode, &hDummyModule, dummyModuleName, MAX_PATH))
            hDummyModule = NULL;
          pCodeHook = new CCodeHook(hDummyModule, procName, pCode, pCallback, pNextHook, flags);
        }
        else
        {
          pCodeHook = new CCodeHook(hModule, procName, pCode, pCallback, pNextHook, flags);
        }
        if ((pCodeHook != NULL) && (!pCodeHook->IsValid))
        {
          DWORD error = GetLastError();
          delete pCodeHook;
          SetLastError(error);
          pCodeHook = NULL;
          hModule = NULL;
        }
      }
      __except (ExceptionFilter(L"HookCodeInternal - CCodeHook construction", GetExceptionInformation()))
      {
        pCodeHook = NULL;
      }
    }

    if ((pCodeHook != NULL) || ((pCode == NULL) && (moduleName != NULL) && (*moduleName != '\0')))
    {
      result = true;

      bool b = false;
      EnterCriticalSection(&gHookCriticalSection);
      __try
      {
        HOOK_ITEM item;
        item.hOwner = hOwner;
        item.hModule = hModule;
        strcpy_s(item.ModuleName, MAX_PATH, moduleName);
        strcpy_s(item.ProcName, MAX_PATH, procName);
        item.pCode = pCode;
        item.pCodeHook = pCodeHook;
        item.pNextHook = pNextHook;
        item.pCallback = pCallback;
        item.Flags = flags;

        if (g_pHookCollection != NULL)
          g_pHookCollection->Add(item);

        b = (hOwner) && (!LoadLibraryHooked);
      }
      __finally
      {
        LeaveCriticalSection(&gHookCriticalSection);
      }

      if (b)
      {
        // First time through, hook LoadLibrary and store AutoUnhook flag for this process
        HookLoadLibrary();
        char buffer[64];
        char mapName[MAX_PATH];
        sprintf_s(mapName, MAX_PATH, "%s$%08x$%08x", DecryptStr(CAutoUnhookMap, buffer, 64), GetCurrentProcessId(), (ULONG_PTR) hOwner);
        HANDLE hMap = CreateLocalFileMapping(mapName, sizeof(LPVOID));
        if (hMap != NULL)
        {
          LPVOID *pBuffer = (LPVOID *) MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
          if (pBuffer != NULL)
          {
            *pBuffer = AutoUnhook;
            UnmapViewOfFile(pBuffer);
            ghAutoUnhookMap = hMap;
          }
          else
            CloseHandle(hMap);
        }
      }
    }
  }
  __except (ExceptionFilter(L"HookCodeInternal", GetExceptionInformation()))
  {
    result = false;
  }

  if (result)
    SetLastError(0);
  return result;
}

static BOOL UnhookInternal(LPVOID *pNextHook, BOOL dontUnhookHelperHooks, BOOL wait)
{
  TraceVerbose(L"UnhookInternal(%p, %d, %d)", pNextHook, dontUnhookHelperHooks, wait);

  BOOL result = false;

  __try
  {
    CCodeHook *pCodeHook = NULL;
    bool unhookHelpers = false;
    EnterCriticalSection(&gHookCriticalSection);
    __try
    {
      for (int i = g_pHookCollection->GetCount() - 1; i >= 0; i--)
      {
        if ((*g_pHookCollection)[i].pNextHook == pNextHook)
        {
          pCodeHook = (*g_pHookCollection)[i].pCodeHook;
          g_pHookCollection->RemoveAt(i);
          result = true;
          break;
        }
      }
      unhookHelpers = result && (!dontUnhookHelperHooks);
      if (unhookHelpers)
      {
        for (int i = 0; i < g_pHookCollection->GetCount(); i++)
        {
          if ((*g_pHookCollection)[i].hOwner != NULL)
          {
            unhookHelpers = false;
            break;
          }
        }
      }
    }
    __finally
    {
      LeaveCriticalSection(&gHookCriticalSection);
    }
    if (result)
    {
      if (pCodeHook != NULL)
      {
        pCodeHook->mLeakUnhook = !wait;
        delete pCodeHook;
      }
    }
    if (unhookHelpers)
      UnhookLoadLibrary(wait);
  }
  __except (ExceptionFilter(L"UnhookInternal", GetExceptionInformation()))
  {
    result = false;
  }
  return result;
}

SYSTEMS_API void WINAPI CollectHooks(void)
{
  TraceVerbose(L"%S(void)", __FUNCTION__);

  CCollectCache::AddReference();
}

SYSTEMS_API void WINAPI FlushHooks(void)
{
  TraceVerbose(L"%S(void)", __FUNCTION__);

  CCollectCache::ReleaseReference();
}

SYSTEMS_API BOOL WINAPI RestoreCode(LPVOID pCode)
{
  TraceVerbose(L"RestoreCode(%p)", pCode);

  BOOL result = false;
  CCollectCache::AddReference();
  __try
  {
    HMODULE module;
    CHAR arrCh[MAX_PATH];
    ABSOLUTE_JUMP orgCode;
    if (FindModule(pCode, &module, arrCh, MAX_PATH) && (WasCodeChanged(module, pCode, &orgCode)))
    {
      BOOL b1 = IsMemoryProtected(pCode);
      if ((!b1) || UnprotectMemory(pCode, 8))
      {
        result = true;
        if (!AtomicMove(&orgCode, pCode, sizeof(orgCode)))
          memcpy(pCode, &orgCode, sizeof(orgCode));
        FlushInstructionCache(GetCurrentProcess(), pCode, sizeof(orgCode));
        if (b1)
          ProtectMemory(pCode, 8);
      }
    }
  }
  __finally
  {
    CCollectCache::ReleaseReference();
  }

  return result;
}

void AutoUnhook(HMODULE hModule)
{
  TraceVerbose(L"AutoUnhook(%p)", hModule);

  AutoUnhook2(hModule, true);
}

SYSTEMS_API void WINAPI AutoUnhook2(HMODULE hModule, BOOL wait)
{
  TraceVerbose(L"AutoUnhook2(%p, %d)", hModule, wait);

  ASSERT(gHookReady);

  if (hModule != NULL)
  {

    CCollection<HOOK_ITEM, CStructureEqualHelper<HOOK_ITEM>> ach;
    EnterCriticalSection(&gHookCriticalSection);
    __try
    {
      for (int i = 0; i < g_pHookCollection->GetCount(); i++)
      {
        if (((hModule == (HMODULE) -1) && (!wait)) || ((*g_pHookCollection)[i].hOwner == hModule))
        {
          ach.Add((*g_pHookCollection)[i]);
        }
      }
    }
    __finally
    {
      LeaveCriticalSection(&gHookCriticalSection);
    }

    for (int i = 0; i < ach.GetCount(); i++)
      UnhookInternal(ach[i].pNextHook, false, wait);
  }
}

// --------------------------------- Shared ----------------------------------------------

// starts allocation search at 0x5f040000
#ifdef _WIN64
  LPVOID VirtualAlloc2(int size, LPVOID preferredAddress)
#else
  LPVOID VirtualAlloc2(int size, LPVOID)
#endif
{
  #ifdef _WIN64
    if ((preferredAddress >= (LPVOID) 0x70000000) && (preferredAddress <= (LPVOID) 0x7fffffff))
      // let's skip the area which is usually used by windows system dlls
      // in 64 bit we have enough memory address range available above of 0x80000000
      // so there's no need to allocate where we could collide with the system dlls
      preferredAddress = (LPVOID) (ULONG_PTR) 0x80000000;
    return AllocMemEx(size, 0, preferredAddress);
  #else
    return AllocMemEx(size);
  #endif
}
// -------------------------------- CheckHooks support -------------------------------------

void CheckHooks(HMODULE hModule)
{
  Trace(L"CheckHooks(%p)", hModule);
  __try
  {
    DWORD lastError = GetLastError();
    if (hModule != NULL)
    {
      CCollection<HOOK_ITEM, CStructureEqualHelper<HOOK_ITEM>> ach;
      EnterCriticalSection(&gHookCriticalSection);
      __try
      {
        for (int i = 0; i < g_pHookCollection->GetCount(); i++)
        {
          if (strcmp((*g_pHookCollection)[i].ModuleName, ""))
          {
            ach.Add((*g_pHookCollection)[i]);
          }
        }
      }
      __finally
      {
        LeaveCriticalSection(&gHookCriticalSection);
      }

      bool b1 = false;
      __try
      {
        for (int i = 0; i < ach.GetCount(); i++)
        {
          if ( (strcmp(ach[i].ModuleName, "")) &&
               ( (!ach[i].pCodeHook) ||
                 (IsBadReadPtr(ach[i].pCodeHook->mPatchAddr, sizeof(ABSOLUTE_JUMP))) ||
                 (*(DWORD *) ((ULONG_PTR) ach[i].pCodeHook->mPatchAddr    ) != *(DWORD *)((ULONG_PTR) &ach[i].pCodeHook->mNewCode    )) ||
                 (*( WORD *) ((ULONG_PTR) ach[i].pCodeHook->mPatchAddr + 4) != *( WORD *)((ULONG_PTR) &ach[i].pCodeHook->mNewCode + 4))    ) )
          {
            if (!b1)
            {
              b1 = true;
              CCollectCache::AddReference();
            }

            HMODULE hModule = GetModuleHandleA(ach[i].ModuleName);
            LPVOID p = FindRealCode(GetImageProcAddress(hModule, ach[i].ProcName, true));
            if ((!ach[i].pCodeHook) || (ach[i].pCode != p))
            {
              UnhookInternal(ach[i].pNextHook, true, true);
              HookCodeInternal(ach[i].hOwner,
                               hModule,
                               ach[i].ModuleName,
                               ach[i].ProcName,
                               p,
                               ach[i].pCallback,
                               ach[i].pNextHook,
                               ach[i].Flags);
            }
            else
            {
              if ( (GetMadCHookOption(RENEW_OVERWRITTEN_HOOKS)) ||
                   ( (!IsBadReadPtr(ach[i].pCodeHook->mPatchAddr, sizeof(ABSOLUTE_JUMP))) &&
                     (*(DWORD *) ((ULONG_PTR) ach[i].pCodeHook->mPatchAddr    ) == *(DWORD *)((ULONG_PTR) &ach[i].pCodeHook->mOldCode    )) &&
                     (*( WORD *) ((ULONG_PTR) ach[i].pCodeHook->mPatchAddr + 4) == *( WORD *)((ULONG_PTR) &ach[i].pCodeHook->mOldCode + 4))    ) )
                RenewHook(ach[i].pNextHook);
            }
          }
        }
      }
      __finally
      {
        if (b1)
          CCollectCache::ReleaseReference();
      }
    }
    SetLastError(lastError);
  }
  __except (ExceptionFilter(L"CheckHooks", GetExceptionInformation()))
  {
  }
  Trace(L"CheckHooks(%p) Complete", hModule);
}

HMODULE WINAPI LoadLibraryExWCallbackProc(LPCWSTR libraryName, HANDLE hFile, DWORD flags)
{
  HMODULE result;
  HMODULE hModule = NULL;
  DWORD lastError = GetLastError();
  __try
  {
    hModule = GetModuleHandleW(libraryName);
  }
  __except (1)
  {
    hModule = NULL;
  }
  SetLastError(lastError);
  result = pfnLoadLibraryExWNextHook(libraryName, hFile, flags);
  if ((result) && (result != hModule) && ((flags & LOAD_LIBRARY_AS_DATAFILE) == 0))
    CheckHooks(result);

  return result;
}

DWORD WINAPI LdrLoadDllCallbackProc(DWORD path, DWORD *flags, UNICODE_STRING *name, HANDLE *handle)
{
  HMODULE hModule = NULL;
  DWORD lastError = GetLastError();
  __try
  {
    hModule = GetModuleHandleW(name->Buffer);
  }
  __except (1)
  {
    hModule = NULL;
  }
  SetLastError(lastError);
  DWORD result = pfnLdrLoadDllNextHook(path, flags, name, handle);
  if ((*handle) && (*handle != hModule))
    CheckHooks((HMODULE) *handle);

  return result;
}

typedef struct tagMchLLEW
{
  LONGLONG name;
  PVOID    jmp;
} TMchLLEW;

void HookLoadLibrary(void)
{
  if (!LoadLibraryHooked)
  {
    if (!LoadLibraryExWDone)
    {
      LoadLibraryExWDone = true;

      char buffer1[32], buffer2[32];
      LONGLONG* nameAsInt64 = (LONGLONG*) buffer2;
      sprintf_s(buffer1, 32, "%s$%x", DecryptStr(CMchLLEW2, buffer2, 32), GetCurrentProcessId());
      HANDLE map = CreateFileMappingA((HANDLE) -1, NULL, PAGE_READWRITE, 0, sizeof(TMchLLEW), buffer1);
      if (map)
      {
        bool newMap = (GetLastError() != ERROR_ALREADY_EXISTS);
        TMchLLEW *buf1 = (TMchLLEW*) MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (buf1)
        {
          for (int i1 = 0; i1 < 50; i1++)
            if ((newMap) || (buf1->name == *nameAsInt64))
              break;
            else
              Sleep(50);
          if (newMap || (buf1->name != *nameAsInt64))
          {
            CFunctionParse fi(KernelProc(CLoadLibraryExW, true));
            PVOID p1 = NtProc(CLdrLoadDll, true);
            INDIRECT_ABSOLUTE_JUMP *buf2 = (INDIRECT_ABSOLUTE_JUMP*) VirtualAlloc2(sizeof(INDIRECT_ABSOLUTE_JUMP), fi.mpEntryPoint);
            for (int i1 = 0; i1 < fi.mFarCalls.GetCount(); i1++)
              if (fi.mFarCalls[i1].Target == p1)
              {
                if ((fi.mFarCalls[i1].pTarget) || (fi.mFarCalls[i1].ppTarget))
                {
                  buf1->jmp = (LPVOID) buf2;
                  buf2->Opcode = 0xff;
                  buf2->modRm = 0x25;
                  #ifdef _WIN64
                   // RIP-relative addressing
                    buf2->Target = (DWORD) ((ULONG_PTR) &(buf2->AbsoluteAddress) - (ULONG_PTR) buf2 - 6);
                  #else
                    buf2->Target = (DWORD) (ULONG_PTR) &(buf2->AbsoluteAddress);
                  #endif
                  buf2->AbsoluteAddress = p1;
                  if (fi.mFarCalls[i1].RelTarget)
                  {
                    BOOL b1 = IsMemoryProtected(fi.mFarCalls[i1].pTarget);
                    if ((!b1) || UnprotectMemory(fi.mFarCalls[i1].pTarget, 4))
                    {
                      DWORD c1 = (DWORD) ((ULONG_PTR) buf2 - (ULONG_PTR) fi.mFarCalls[i1].CodeAddress1 - 5);
                      if (!AtomicMove(&c1, fi.mFarCalls[i1].pTarget, 4))
                        *(DWORD*) fi.mFarCalls[i1].pTarget = c1;
                      if (b1)
                        ProtectMemory(fi.mFarCalls[i1].pTarget, 4);
                      LoadLibraryCallsNtDll = (PVOID) buf2;
                      map = 0;
                    }
                  }
                  else
                  {
                    LPVOID pTarget = fi.mFarCalls[i1].pTarget;
                    if (!pTarget)
                      pTarget = *(PVOID*) fi.mFarCalls[i1].ppTarget;
                    BOOL b1 = IsMemoryProtected(pTarget);
                    if ((!b1) || UnprotectMemory(pTarget, sizeof(PVOID)))
                    {
                      if (!AtomicMove(&buf2, pTarget, sizeof(PVOID)))
                        *(PVOID*) pTarget = buf2;
                      if (b1)
                        ProtectMemory(pTarget, sizeof(PVOID));
                      LoadLibraryCallsNtDll = (PVOID) buf2;
                      map = 0;
                    }
                  }
                }
                break;
              }
            if (!LoadLibraryCallsNtDll)
            {
              UnmapViewOfFile(buf1);
              VirtualFree(buf2, 0, MEM_RELEASE);
            }
            else
              buf1->name = *nameAsInt64;
          }
          else
          {
            LoadLibraryCallsNtDll = buf1->jmp;
            UnmapViewOfFile(buf1);
          }
        }
        if (map)
          CloseHandle(map);
      }
    }
    if (LoadLibraryCallsNtDll)
      HookCodeInternal(NULL, NULL, "", "", LoadLibraryCallsNtDll, LdrLoadDllCallbackProc, (LPVOID *) &pfnLdrLoadDllNextHook);
    else
      HookCodeInternal(NULL, NULL, "", "", FindRealCode(KernelProc(CLoadLibraryExW, true)), LoadLibraryExWCallbackProc, (LPVOID *) &pfnLoadLibraryExWNextHook);
    LoadLibraryHooked = true;
  }
}

void UnhookLoadLibrary(BOOL wait)
{
  if (LoadLibraryHooked)
  {
    if (pfnLoadLibraryExWNextHook) UnhookInternal((LPVOID *) &pfnLoadLibraryExWNextHook, true, wait);
    if (    pfnLdrLoadDllNextHook) UnhookInternal((LPVOID *) &pfnLdrLoadDllNextHook,     true, wait);
    LoadLibraryHooked = false;
  }
}
