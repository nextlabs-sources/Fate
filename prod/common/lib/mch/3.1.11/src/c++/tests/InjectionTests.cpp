// ***************************************************************
//  InjectionTests.cpp        version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  tests the DLL injection functionality
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#include "SystemsTest.h"
#include <Systems.h>
#include <windows.h>

static bool StartProcess(LPCWSTR processPath, LPCWSTR arguments, LPCWSTR workingDirectory, bool suspended, PROCESS_INFORMATION *pi);

bool StartProcess(LPCWSTR processPath, LPCWSTR arguments, LPCWSTR workingDirectory, bool suspended, PROCESS_INFORMATION *pi)
{
   STARTUPINFO startupInfo;

   // Create struct
   ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
   startupInfo.cb = sizeof(STARTUPINFO);
   ZeroMemory(pi, sizeof(PROCESS_INFORMATION));

   // Create command line
   wchar_t commandLine[MAX_PATH];
   if (arguments != NULL)
      swprintf_s(commandLine, L"%s %s", processPath, arguments);
   else
      swprintf_s(commandLine, L"%s", processPath);

   DWORD flags;
   if (suspended)
      flags = CREATE_SUSPENDED;
   else
      flags = 0;
   // Start process
   if (CreateProcessW(NULL,               // app name, null uses first token of command line
                     commandLine,        // command line 
                     NULL,               // process security attributes, null uses default
                     NULL,               // thread security attributes, null uses default
                     FALSE,              // inherit handles
                     flags,              // creation flags
                     NULL,               // environment, null uses parent
                     workingDirectory,   // specifies the drive and directory for the new process
                     &startupInfo,       // specifies main window appearance and device handles, we are ignoring
                     pi))               // returns the process identification and handles
      return true;

   return false;
}

typedef bool (WINAPI * PFNENUMPROCESSMODULES)(
    HANDLE hProcess,
    HMODULE *lphModule,
    DWORD cb,
    LPDWORD lpcbNeeded
  );

typedef DWORD (WINAPI * PFNGETMODULEFILENAMEEXW)(
    HANDLE hProcess,
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
  );

bool ProcessHasLibraryNt(HANDLE hProcess, LPCWSTR lib)
{
    static HMODULE hModPSAPI = 0;
    static PFNENUMPROCESSMODULES pfnEnumProcessModules = 0;
    static PFNGETMODULEFILENAMEEXW pfnGetModuleFileNameExW = 0;

    if (!hModPSAPI)
        hModPSAPI = LoadLibrary(L"PSAPI.DLL");

    if (!hModPSAPI)
        return FALSE;
        
    pfnEnumProcessModules = (PFNENUMPROCESSMODULES)
            GetProcAddress(hModPSAPI, "EnumProcessModules");

    pfnGetModuleFileNameExW = (PFNGETMODULEFILENAMEEXW)
            GetProcAddress(hModPSAPI, "GetModuleFileNameExW");

    if (  !pfnEnumProcessModules
        ||  !pfnGetModuleFileNameExW)
        return FALSE;
    
    // If we get to this point, we've successfully hooked up to the PSAPI APIs

    HMODULE hModuleArray[1024];
    DWORD nModules;
    DWORD cbNeeded;
        
    // EnumProcessModules returns an array of HMODULEs for the process
    if (!pfnEnumProcessModules(hProcess, hModuleArray, sizeof(hModuleArray), &cbNeeded))
        return FALSE;

    // Calculate number of modules in the process                                   
    nModules = cbNeeded / sizeof(hModuleArray[0]);

    // Iterate through each of the process's modules
    for (unsigned j = 0; j < nModules; j++)
    {
        HMODULE hModule = hModuleArray[j];
        WCHAR szModuleName[MAX_PATH];

        // GetModuleFileNameEx is like GetModuleFileName, but works
        // in other process address spaces
        pfnGetModuleFileNameExW(hProcess, hModule, szModuleName, sizeof(szModuleName));

        if (_wcsicmp(lib, &szModuleName[wcslen(szModuleName) - wcslen(lib)]) == 0)
            return TRUE;
    }

    return FALSE;
}

#ifdef _WIN64
  const LPWSTR CHookDll = L"TestAutoUnhook64.dll";
#else
  const LPWSTR CHookDll = L"TestAutoUnhook32.dll";
#endif

bool TestInjectionProcess(HANDLE hProcess, DWORD dwProcessId, HANDLE hThread, bool isSuspended)
{
   bool result = false;

   if ((NotInitializedYet(NULL, NULL, hProcess) != 0) == isSuspended)
   {
      if (!ProcessHasLibraryNt(hProcess, CHookDll))
      {
         if (InjectLibraryW(CHookDll, hProcess, 7000))
         {
            if (!isSuspended || (ResumeThread(hThread) && (WaitForInputIdle(hProcess, INFINITE) == 0)))
            {
               if (ProcessHasLibraryNt(hProcess, CHookDll))
               {
                  if (UninjectLibraryW(CHookDll, hProcess, 7000))
                  {
                     if (!ProcessHasLibraryNt(hProcess, CHookDll))
                     {
                        wchar_t arrCh[30];
                        swprintf_s(arrCh, L"AutoUnhook$%x", dwProcessId);
                        HANDLE event = OpenEvent(SYNCHRONIZE, false, arrCh);
                        if (event != NULL)
                        {
                           result = true;
                           CloseHandle(event);
                           wprintf(L"  SUCCESS\n\n");
                        }
                        else
                           wprintf(L"  FAILURE - auto unhooking failed\n\n");
                     }
                     else
                        wprintf(L"  FAILURE - test dll still loaded after uninjection\n\n");
                  }
                  else
                     wprintf(L"  FAILURE - uninjection failed\n\n");
               }
               else
                  wprintf(L"  FAILURE - test dll not found after injection\n\n");
            }
            else
               wprintf(L"  FAILURE - WaitForInputIdle failed\n\n");
         }
         else
            wprintf(L"  FAILURE - injection failed\n\n");
      }
      else
         wprintf(L"  FAILURE - test dll already loaded\n\n");
   }
   else
      wprintf(L"  FAILURE - \"initialized\" state incorrect\n\n");

   return result;
}

bool TestInjection()
{
   wprintf(L"%S: Testing Injection (on our own process)\n", __FUNCTION__);
   bool result = TestInjectionProcess(GetCurrentProcess(), GetCurrentProcessId(), NULL, false);

   if (result)
   {
      wprintf(L"%S: Testing Injection (fully initialized calc)\n", __FUNCTION__);
      PROCESS_INFORMATION pi;
      if (StartProcess(L"notepad.exe", NULL, NULL, false, &pi))
      {
         if (WaitForInputIdle(pi.hProcess, INFINITE) == 0)
         {
            result = TestInjectionProcess(pi.hProcess, pi.dwProcessId, NULL, false);
            TerminateProcess(pi.hProcess, 0);
         }
         else
            wprintf(L"  FAILURE - WaitForInputIdle failed\n\n");
         CloseHandle(pi.hProcess);
         CloseHandle(pi.hThread);
      }
      else
         wprintf(L"  FAILURE - calc process creation failed\n\n");

      if (result)
      { 
         wprintf(L"%S: Testing Injection (not yet initialized calc)\n", __FUNCTION__);
         PROCESS_INFORMATION pi;
         if (StartProcess(L"notepad.exe", NULL, NULL, true, &pi))
         { 
            result = TestInjectionProcess(pi.hProcess, pi.dwProcessId, pi.hThread, true);
            TerminateProcess(pi.hProcess, 0);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
         }
         else
            wprintf(L"  FAILURE - calc process creation failed\n\n");
      }
   }

   return result;
}
