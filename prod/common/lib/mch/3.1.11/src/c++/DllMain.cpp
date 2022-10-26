// ***************************************************************
//  DllMain.cpp               version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  madCodeHook initialization/finalization
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _DLLMAIN_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

#ifdef _WINDLL

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD reason, LPVOID pReserved)
{
  BOOL result = TRUE;

  if (reason == DLL_PROCESS_ATTACH)
  {
    gHModule = hModule;

    if (pReserved == NULL)
      gAttachType = dynamicLoad;
    else
      gAttachType = staticLoad;

    if (VersionSupported())
    {
      gSystemDll = true;
      CheckStructureOffsets();
      InitializeHooking();
      SetLastError(0);
    }
    else
    {
      result = FALSE;
    }

  }
  else if (reason == DLL_PROCESS_DETACH)
  {
    DETACH_TYPE detachType;

    if (pReserved == NULL)
      detachType = freeLibrary;
    else
      detachType = processTermination;

    result = CloseHooking();
  }
  else if (reason == DLL_THREAD_ATTACH)
  {
  }
  else if (reason == DLL_THREAD_DETACH)
  {
  }

  return result;
}

#endif

SYSTEMS_API BOOL WINAPI MADCHOOK_DllMain(HINSTANCE, DWORD, LPVOID)
{
  return true;
}

SYSTEMS_API VOID WINAPI StaticLibHelper_Init(PVOID)
{
  Trace(L"InitializeMadCHook()");
  DebugTrace((L"InitializeMadCHook() - DEBUG BUILD"));

  // GetModuleHandle(NULL) returns HMODULE of file that created calling process.
  // CheckLibPath uses this HMODULE to search for libraries
  char buffer[8];
  if (!FindModule(StaticLibHelper_Init, &gHModule, buffer, 8))
    gHModule = GetModuleHandle(NULL);

  if (VersionSupported())
  {
    gSystemDll = false;
    CheckStructureOffsets();
    CCollectCache::Initialize();
    OpenHooking();
    InitSas();
  }
}

SYSTEMS_API VOID WINAPI StaticLibHelper_Final(PVOID)
{
  UnpatchCreateRemoteThread();
  CloseHooking();
  CCollectCache::Finalize();
  FinalSas();

  Trace(L"FinalizeMadCHook()");
  DebugTrace((L"FinalizeMadCHook() - DEBUG BUILD"));
}

SYSTEMS_API BOOL WINAPI VersionSupported(void)
{
  return (GetVersion() & 0x80000000) == 0;
}

static bool PrivilegesEnabled = false;
SYSTEMS_API void WINAPI EnableAllPrivileges(void)
{
  HANDLE hToken;

  if (PrivilegesEnabled)
    return;

  if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
  {
    __try
    {
      DWORD returnLength;
      GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &returnLength);
      if (returnLength != 0)
      {
        TOKEN_PRIVILEGES *pTokenPrivileges = (TOKEN_PRIVILEGES *) LocalAlloc(LPTR, returnLength*2);
        if (GetTokenInformation(hToken, TokenPrivileges, pTokenPrivileges, returnLength * 2, &returnLength))
        {
          LUID backup;
          if (!LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &backup))
          {
            backup.HighPart = 0;
            backup.LowPart = 0;
          }
          LUID restore;
          if (!LookupPrivilegeValue(NULL, SE_RESTORE_NAME, &restore))
          {
            restore.HighPart = 0;
            restore.LowPart = 0;
          }
          LUID owner;
          if (!LookupPrivilegeValue(NULL, SE_TAKE_OWNERSHIP_NAME, &owner))
          {
            owner.HighPart = 0;
            owner.LowPart = 0;
          }
          // Update all the privileges to enable except backup and restore
          //      Enabling backup/restore privileges breaks Explorer's Samba support
          for (int i = 0; i < (int) pTokenPrivileges->PrivilegeCount - 1; i++)
          {
            if ( ( (pTokenPrivileges->Privileges[i].Luid.HighPart !=  backup.HighPart) ||
                   (pTokenPrivileges->Privileges[i].Luid.LowPart  !=  backup.LowPart)     ) &&
                 ( (pTokenPrivileges->Privileges[i].Luid.HighPart != restore.HighPart) ||
                   (pTokenPrivileges->Privileges[i].Luid.LowPart  != restore.LowPart)     ) &&
                 ( (pTokenPrivileges->Privileges[i].Luid.HighPart !=   owner.HighPart) ||
                   (pTokenPrivileges->Privileges[i].Luid.LowPart  !=   owner.LowPart)     )    )
            {
              pTokenPrivileges->Privileges[i].Attributes = pTokenPrivileges->Privileges[i].Attributes | SE_PRIVILEGE_ENABLED;
            }
          }
          VERIFY(AdjustTokenPrivileges(hToken, FALSE, pTokenPrivileges, returnLength, NULL, NULL));
        }
        LocalFree(pTokenPrivileges);
      }
    }
    __finally
    {
      VERIFY(CloseHandle(hToken));
    }
  }

  PrivilegesEnabled = true;
}
