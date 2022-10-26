// ***************************************************************
//  Hooking.cpp               version: 1.0.8  ·  date: 2016-03-16
//  -------------------------------------------------------------
//  hooking groundwork
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2016 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2016-03-16 1.0.8 (1) added hook to detect delay loaded dlls
//                  (2) fixed some PAGE_EXECUTE_READWRITE security issues
//                  (3) fixed rare crash when calling HookAPI
// 2015-09-10 1.0.7 fixed: threading issue when to-be-hooked dll is loaded
// 2015-04-20 1.0.6 fixed: rare injection/hook instability bug
// 2014-05-05 1.0.5 fixed: 32bit injection problems when compiled as 32bit
// 2013-02-13 1.0.4 fixed: uninjecting DLL twice at the same time crashed
// 2012-07-03 1.0.3 improved internal LoadLibrary hook reliability
// 2012-04-03 1.0.2 fixed uninjection memory leak
// 2010-07-28 1.0.1 (1) got rid of some superfluous initialization processing
//                  (2) modified internal LoadLibrary hook (win7) once again
// 2010-01-10 1.0.0 initial version

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

static DWORD (WINAPI *pfnLdrLoadDllNextHook)(PVOID path, DWORD *flags, UNICODE_STRING *name, HANDLE *handle) = NULL;
static DWORD WINAPI LdrLoadDllCallbackProc(PVOID path, DWORD *flags, UNICODE_STRING *name, HANDLE *handle);

typedef struct _IMAGE_DELAYLOAD_DESCRIPTOR
{
    DWORD AllAttributes;
    DWORD DllNameRVA;
    DWORD ModuleHandleRVA;
    DWORD ImportAddressTableRVA;
    DWORD ImportNameTableRVA;
    DWORD BoundImportAddressTableRVA;
    DWORD UnloadInformationTableRVA;
    DWORD TimeDateStamp;
} IMAGE_DELAYLOAD_DESCRIPTOR, *PIMAGE_DELAYLOAD_DESCRIPTOR;
static void* (WINAPI *pfnLdrResolveDelayLoadedAPINextHook)(void* base, const IMAGE_DELAYLOAD_DESCRIPTOR* desc, void* dllhook, void* syshook, void* addr, ULONG flags) = NULL;
static void* WINAPI LdrResolveDelayLoadedAPICallbackProc(void* base, const IMAGE_DELAYLOAD_DESCRIPTOR* desc, void* dllhook, void* syshook, void* addr, ULONG flags);

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
      HMODULE hOwner = GetCallingModule(_ReturnAddress());
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
      HMODULE hOwner = GetCallingModule(_ReturnAddress());
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
static PFN_LDR_GET_DLL_HANDLE pfnLdrGetDllHandle = NULL;
static PFN_LDR_REGISTER_DLL_NOTIFICATION pfnLdrRegisterDllNotification = NULL;
static PFN_LDR_UNREGISTER_DLL_NOTIFICATION pfnLdrUnregisterDllNotification = NULL;
static PVOID LdrDllNotificationCookie = NULL;

void HookLoadLibrary(void);
void UnhookLoadLibrary(BOOL wait);

static LONG AutoUnhookCounter = -1;
void InternalAutoUnhookUninject(HMODULE hModule)
// this gets called from the DLL uninjection thread
{
  if (InterlockedIncrement(&AutoUnhookCounter) != 0)
  {
    // only the first uninjection thread gets to actually perform unhooking
    // the 2nd (and 3rd etc) thread is stopped right here to avoid conflicts
    ExitThread(0);
  }
  AutoUnhook(hModule);
}

#ifdef _WIN64

void AutoUnhookUninject(HMODULE hModule)
{
  InternalAutoUnhookUninject(hModule);
}

#else

void __declspec(naked) AutoUnhookUninject()
{
  __asm
  {
    push eax
    call InternalAutoUnhookUninject
    pop eax
    ret
  }
}

#endif

static BOOL HookCodeInternal(HMODULE hOwner, HMODULE hModule, LPCSTR moduleName, LPCSTR procName,
                             LPVOID pCode, LPVOID pCallback, LPVOID *pNextHook, DWORD flags)
{
  TraceVerbose(L"%S(%p, %p, %S, %S, %p, %p, %p, %p)", __FUNCTION__, hOwner, hModule, moduleName, procName, pCode, pCallback, pNextHook, flags);

  BOOL result = false;

  __try
  {
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
      bool doubleInstalled = false;
      EnterCriticalSection(&gHookCriticalSection);
      __try
      {
        for (int i = g_pHookCollection->GetCount() - 1; i >= 0; i--)
        {
          if ((*g_pHookCollection)[i].pNextHook == pNextHook)
          {
            // There's already a hook for this "nextHook" variable installed.
            // This can happen if multiple threads are loading dlls at the same
            // time, and if madCodeHook detects a new DLL it's supposed to hook.
            // In that situation madCodeHook might try to install an API hook for
            // the new DLL from multiple threads simultaneously.
            // Our new hook is superfluous, so we will delete it right away.
            // But it has already modified the "nextHook" shared variable of the
            // already existing hook. So we need to restore the original value.
            if ((*g_pHookCollection)[i].pCodeHook)
              *(*g_pHookCollection)[i].pCodeHook->mpNextHook = (*g_pHookCollection)[i].pCodeHook->mpHookStub;
            doubleInstalled = true;
            break;
          }
        }

        if (!doubleInstalled)
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
        }

        b = (hOwner) && (!LoadLibraryHooked);
      }
      __finally
      {
        LeaveCriticalSection(&gHookCriticalSection);
      }

      if ((doubleInstalled) && (pCodeHook))
      {
        pCodeHook->mDoubleHook = true;
        delete pCodeHook;
      }

      if (b)
      {
        // First time through, hook LoadLibrary and store AutoUnhook flag for this process
        HookLoadLibrary();
        if (!ghAutoUnhookMap)
        {
          char buffer[64];
          char mapName[MAX_PATH];
          sprintf_s(mapName, MAX_PATH, "%s$%08x$%08x", DecryptStr(CAutoUnhookMap, buffer, 64), GetCurrentProcessId(), (ULONG_PTR) hOwner);
          HANDLE hMap = CreateLocalFileMapping(mapName, sizeof(LPVOID));
          if (hMap != NULL)
          {
            LPVOID *pBuffer = (LPVOID *) MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if (pBuffer != NULL)
            {
              *pBuffer = AutoUnhookUninject;
              UnmapViewOfFile(pBuffer);
              ghAutoUnhookMap = hMap;
            }
            else
              CloseHandle(hMap);
          }
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
    ULONGLONG orgCode;
    if (FindModule(pCode, &module, arrCh, MAX_PATH) && (WasCodeChanged(module, pCode, &orgCode)) && (orgCode != 0ULL))
    {
      DWORD op;
      if (VirtualProtect(pCode, 8, PAGE_EXECUTE_READWRITE, &op))
      {
        result = true;
        if (!AtomicMove(&orgCode, pCode, sizeof(orgCode)))
          memcpy(pCode, &orgCode, sizeof(orgCode));
        FlushInstructionCache(GetCurrentProcess(), pCode, sizeof(orgCode));
        VirtualProtect(pCode, 8, op, &op);
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

// starts allocation search at 0x71af0000
#ifdef _WIN64
  LPVOID VirtualAlloc2(int size, LPVOID preferredAddress)
#else
  LPVOID VirtualAlloc2(int size, LPVOID)
#endif
{
  #ifdef _WIN64
    if (((ULONG_PTR) preferredAddress >= 0x70000000) && ((ULONG_PTR) preferredAddress < 0x80000000))
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
                 (IsBadReadPtr2(ach[i].pCodeHook->mPatchAddr, sizeof(ABSOLUTE_JUMP))) ||
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
            if (((!ach[i].pCodeHook) && (hModule)) || ((ach[i].pCodeHook) && (ach[i].pCode != p)))
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
              if ( (ach[i].pCodeHook) &&
                   ( (GetMadCHookOption(RENEW_OVERWRITTEN_HOOKS)) ||
                     ( (!IsBadReadPtr2(ach[i].pCodeHook->mPatchAddr, sizeof(ABSOLUTE_JUMP))) &&
                       (*(DWORD *) ((ULONG_PTR) ach[i].pCodeHook->mPatchAddr    ) == *(DWORD *)((ULONG_PTR) &ach[i].pCodeHook->mOldCode    )) &&
                       (*( WORD *) ((ULONG_PTR) ach[i].pCodeHook->mPatchAddr + 4) == *( WORD *)((ULONG_PTR) &ach[i].pCodeHook->mOldCode + 4))    ) ) )
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

typedef struct _LDR_DLL_NOTIFICATION_DATA
{
  ULONG Flags;                   // Reserved.
  PUNICODE_STRING FullDllName;   // The full path name of the DLL module.
  PUNICODE_STRING BaseDllName;   // The base file name of the DLL module.
  PVOID DllBase;                 // A pointer to the base address for the DLL in memory.
  ULONG SizeOfImage;             // The size of the DLL image, in bytes.
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;

VOID CALLBACK LdrDllNotificationFunction(ULONG reason, PLDR_DLL_NOTIFICATION_DATA data, PVOID context)
{
  reason;
  context;
  CheckHooks(HMODULE(data->DllBase));
}

HMODULE GetModuleHandleEx(UNICODE_STRING *dll)
{
  HMODULE result = NULL;
  if (pfnLdrGetDllHandle)
  {
    // Calling GetModuleHandleW in a LoadLibrary hook callback can sometimes
    // screw up strings (don't ask me why). We work around this by calling
    // LdrGetDllHandle instead, which doesn't have this problem.
    if (pfnLdrGetDllHandle(NULL, NULL, dll, &result))
      result = NULL;
  }
  else
    result = GetModuleHandleW(dll->Buffer);
  return result;
}

HMODULE WINAPI LoadLibraryExWCallbackProc(LPCWSTR libraryName, HANDLE hFile, DWORD flags)
{
  HMODULE result;
  HMODULE hModule = NULL;
  DWORD lastError = GetLastError();
  __try
  {
    UNICODE_STRING us;
    us.Buffer = (PWCHAR) libraryName;
    us.Length = (WORD) lstrlenW(libraryName) * 2;
    us.MaximumLength = us.Length + 2;
    hModule = GetModuleHandleEx(&us);
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

DWORD WINAPI LdrLoadDllCallbackProc(PVOID path, DWORD *flags, UNICODE_STRING *name, HANDLE *handle)
{
  HMODULE hModule = NULL;
  DWORD lastError = GetLastError();
  __try
  {
    hModule = GetModuleHandleEx(name);
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

void* WINAPI LdrResolveDelayLoadedAPICallbackProc(void* base, const IMAGE_DELAYLOAD_DESCRIPTOR* desc, void* dllhook, void* syshook, void* addr, ULONG flags)
{
  DWORD lastError = GetLastError();
  HMODULE hModule = NULL;
  __try
  {
    CHAR* name = (CHAR*) base + desc->DllNameRVA;
    if (name)
    {
      int len = (int) strlen(name);
      if (len)
      {
        UNICODE_STRING us;
        us.Buffer = (PWCHAR) VirtualAlloc(NULL, len * 2 + 2, MEM_COMMIT, PAGE_READWRITE);
        for (int i1 = 0; i1 <= len; i1++)
          us.Buffer[i1] = name[i1];
        us.Length = (WORD) (len * 2);
        us.MaximumLength = us.Length + 2;
        hModule = GetModuleHandleEx(&us);
        VirtualFree(us.Buffer, 0, MEM_RELEASE);
      }
    }
  }
  __except (1)
  {
    hModule = NULL;
  }
  void* result = pfnLdrResolveDelayLoadedAPINextHook(base, desc, dllhook, syshook, addr, flags);
  if ((result) && (!hModule))
  {
    MEMORY_BASIC_INFORMATION mbi;
    if ((VirtualQuery(result, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION)) && (mbi.State == MEM_COMMIT) && (mbi.AllocationBase))
      CheckHooks((HMODULE) mbi.AllocationBase);
  }
  SetLastError(lastError);
  return result;
}

#pragma pack(1)

typedef struct tagMchLLEW
{
  LONGLONG name;
  PVOID    jmp;
} TMchLLEW;

typedef struct tagHookLdrLoadDllStruct
// jmp [target]
// AbsoluteAddress PVOID ??
{
  BYTE Opcode;   // 0xff
  BYTE modRm;    // 0x25, 00101001, Mod = 00, Reg = 100, R/M = 101 : 32 bit displacement follows
  DWORD Target;  // &(AbsoluteAddress) which is pointer to an address (32 bits)
  LPVOID AbsoluteAddress;
  LPVOID Self;
} HookLdrLoadDllStruct;

#pragma pack()

void HookLoadLibrary(void)
{
  if (!LoadLibraryHooked)
  {
    // I had high hopes for the "LdrRegisterDllNotification" API to replace my old self-made LoadLibrary API hook.
    // However, plays out "LdrRegisterDllNotification" is rather sensitive/unstable. Several things don't work,
    // inside of the callback function. Furthermore, in Windows 7 x64 (and only there) CorelDraw X6 crashes inside
    // of the callback, when hooking a WinSock API. The crash seems to be caused by trying to disassemble the
    // WinSock API code. Even a "__try __except(1)" block isn't able to solve the crash. So right now I see no
    // other option than to continue using my self-mode hook instead of the Ldr* API.
    pfnLdrRegisterDllNotification = NULL; //(PFN_LDR_REGISTER_DLL_NOTIFICATION) NtProc(CLdrRegisterDllNotification);
    pfnLdrUnregisterDllNotification = NULL; // (PFN_LDR_UNREGISTER_DLL_NOTIFICATION) NtProc(CLdrUnregisterDllNotification);
    if ((!pfnLdrRegisterDllNotification) || (!pfnLdrUnregisterDllNotification) || (pfnLdrRegisterDllNotification(0, LdrDllNotificationFunction, NULL, &LdrDllNotificationCookie)))
    {
      LdrDllNotificationCookie = NULL;
      if (!pfnLdrGetDllHandle)
        pfnLdrGetDllHandle = (PFN_LDR_GET_DLL_HANDLE) NtProc(CLdrGetDllHandle);
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
              PVOID api = KernelProc(CLoadLibraryExW, true);
              ULONGLONG orgCode = *((ULONGLONG*) api);
              if (!RestoreCode(api))
                orgCode = 0;
              CFunctionParse fi(api);
              PVOID p1 = NtProc(CLdrLoadDll, true);
              HookLdrLoadDllStruct *buf2 = (HookLdrLoadDllStruct*) VirtualAlloc2(sizeof(HookLdrLoadDllStruct), fi.mpEntryPoint);
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
                      buf2->Target = 0;
                    #else
                      buf2->Target = (DWORD) (ULONG_PTR) &(buf2->AbsoluteAddress);
                    #endif
                    buf2->AbsoluteAddress = p1;
                    buf2->Self = buf2;
                    if (fi.mFarCalls[i1].RelTarget)
                    {
                      DWORD op;
                      if (VirtualProtect(fi.mFarCalls[i1].pTarget, 4, PAGE_EXECUTE_READWRITE, &op))
                      {
                        DWORD c1 = (DWORD) ((ULONG_PTR) buf2 - (ULONG_PTR) fi.mFarCalls[i1].CodeAddress1 - 5);
                        if (!AtomicMove(&c1, fi.mFarCalls[i1].pTarget, 4))
                          *(DWORD*) fi.mFarCalls[i1].pTarget = c1;
                        VirtualProtect(fi.mFarCalls[i1].pTarget, 4, op, &op);
                        LoadLibraryCallsNtDll = (PVOID) buf2;
                        map = 0;
                      }
                    }
                    else
                    {
                      if (fi.mFarCalls[i1].pTarget)
                      {
                        DWORD op;
                        if (VirtualProtect(fi.mFarCalls[i1].pTarget, sizeof(PVOID), PAGE_EXECUTE_READWRITE, &op))
                        {
                          if (!AtomicMove(&buf2, fi.mFarCalls[i1].pTarget, sizeof(PVOID)))
                            *(PVOID*) fi.mFarCalls[i1].pTarget = buf2;
                          VirtualProtect(fi.mFarCalls[i1].pTarget, sizeof(PVOID), op, &op);
                          LoadLibraryCallsNtDll = (PVOID) buf2;
                          map = 0;
                        }
                      }
                      else
                      {
                        DWORD op;
                        if (VirtualProtect(fi.mFarCalls[i1].ppTarget, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &op))
                        {
                          #ifdef _WIN64
                            // RIP-relative addressing
                            DWORD newTarget = (DWORD) ((ULONG_PTR) &(buf2->Self) - (ULONG_PTR) fi.mFarCalls[i1].CodeAddress2);
                          #else
                            DWORD newTarget = (DWORD) (ULONG_PTR) &(buf2->Self);
                          #endif
                          if (!AtomicMove(&newTarget, fi.mFarCalls[i1].ppTarget, sizeof(DWORD)))
                            *(DWORD*) fi.mFarCalls[i1].ppTarget = newTarget;
                          VirtualProtect(fi.mFarCalls[i1].ppTarget, sizeof(DWORD), op, &op);
                          LoadLibraryCallsNtDll = (PVOID) buf2;
                          map = 0;
                        }
                      }
                    }
                  }
                  break;
                }
              if (orgCode)
              {
                DWORD op;
                if (VirtualProtect(api, 8, PAGE_EXECUTE_READWRITE, &op))
                {
                  if (!AtomicMove(&orgCode, api, 8))
                    *((ULONGLONG*) api) = orgCode;
                  FlushInstructionCache(GetCurrentProcess(), api, 8);
                  VirtualProtect(api, 8, op, &op);
                }
              }
              if (!LoadLibraryCallsNtDll)
              {
                UnmapViewOfFile(buf1);
                VirtualFree(buf2, 0, MEM_RELEASE);
              }
              else
              {
                buf1->name = *nameAsInt64;
                DWORD op;
                VirtualProtect(buf2, sizeof(HookLdrLoadDllStruct), PAGE_EXECUTE_READ, &op);
              }
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
        HookCodeInternal(NULL, NULL, "", "", LoadLibraryCallsNtDll, LdrLoadDllCallbackProc, (LPVOID *) &pfnLdrLoadDllNextHook, 0);
      else
        HookCodeInternal(NULL, NULL, "", "", FindRealCode(KernelProc(CLoadLibraryExW, true)), LoadLibraryExWCallbackProc, (LPVOID *) &pfnLoadLibraryExWNextHook, 0);
      HookCodeInternal(NULL, NULL, "", "", FindRealCode(NtProc(CLdrResolveDelayLoadedAPI, true)), LdrResolveDelayLoadedAPICallbackProc, (LPVOID *) &pfnLdrResolveDelayLoadedAPINextHook, 0);
    }
    LoadLibraryHooked = true;
  }
}

void UnhookLoadLibrary(BOOL wait)
{
  if (LoadLibraryHooked)
  {
    if (LdrDllNotificationCookie)
    {
      pfnLdrUnregisterDllNotification(LdrDllNotificationCookie);
      LdrDllNotificationCookie = NULL;
    }
    if (pfnLoadLibraryExWNextHook) UnhookInternal((LPVOID *) &pfnLoadLibraryExWNextHook, true, wait);
    if (    pfnLdrLoadDllNextHook) UnhookInternal((LPVOID *) &pfnLdrLoadDllNextHook,     true, wait);
    LoadLibraryHooked = false;
  }
}
