// ***************************************************************
//  RemoteTests.cpp           version: 1.0.1  ·  date: 2011-03-27
//  -------------------------------------------------------------
//  tests remote functions
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2011 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2011-03-27 1.0.1 fixed format ansi/wide mismatch
// 2010-01-10 1.0.0 initial version

#define _CRT_SECURE_NO_WARNINGS
#include "SystemsTest.h"
#include <Systems.h>
#include <stdio.h>

bool TestGetSmssHandle()
{
   bool result = false;
   wprintf(L"%S: Testing GetSmssProcessHandle()\n", __FUNCTION__);

   HANDLE hSmss = GetSmssProcessHandle();
   if (hSmss != NULL)
   {
      if (CloseHandle(hSmss))
      {
         wprintf(L"  SUCCESS\n\n");
         result = true;
      }
      else
         wprintf(L"  FAILURE - CloseHandle(smss): %x\n\n", GetLastError());
   }
   else
      wprintf(L"  FAILURE - GetSmssProcessHandle()\n\n");

   return result;
}

bool TestHandleLiveForeverWithProcess()
{
   bool result = false;
   wprintf(L"%S: Testing HandleLiveForever(), ProcessHandleToId()\n", __FUNCTION__);

   HANDLE hProcess = NULL;
   if (DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(), &hProcess, 0, false, DUPLICATE_SAME_ACCESS))
   {
      if (hProcess)
      {
         if (ProcessHandleToId(hProcess) == GetCurrentProcessId())
         {
            if (CloseHandle(hProcess))
            {
               hProcess = GetSmssProcessHandle();
               if (hProcess)
               {
                  if (ProcessHandleToId(hProcess) != GetCurrentProcessId())
                  {
                     if (HandleLiveForever(hProcess))
                     {
                        if (CloseHandle(hProcess))
                        {
                           wprintf(L"  SUCCESS\n\n");
                           result = true;
                        }
                        else
                           wprintf(L"  FAILURE - CloseHandle(smss): %x\n\n", GetLastError());
                     }
                     else
                        wprintf(L"  FAILURE - HandleLiveForever()\n\n");
                  }
                  else
                     wprintf(L"  FAILURE - ProcessHandleToId(smss) should not equal GetCurrentProcessId()");
               }
               else
                  wprintf(L"  FAILURE - GetSmssProcessHandle()\n\n");
            }
            else
               wprintf(L"  FAILURE - CloseHandle(hCurrentProcess): %x\n\n", GetLastError());
         }
         else
            wprintf(L"  FAILURE - ProcessHandleToId(hCurrentProcess) should equal GetCurrentProcessId()\n\n");
      }
      else
         wprintf(L"  FAILURE - DuplicateHandle() should return non-null hProcess\n\n");
   }
   else
      wprintf(L"  FAILURE - DuplicateHandle(): %x\n\n", GetLastError());

   return result;
}

bool TestHandleLiveForever()
{
   int testCount = 0;

   bool result = false;
   wprintf(L"%S: Testing HandleLiveForever()\n", __FUNCTION__);

   char mapName[MAX_PATH];

   do
   {
      sprintf_s(mapName, sizeof(mapName), "RemoteTestsMap%d", testCount++);
      HANDLE memoryMap = CreateGlobalFileMapping(mapName, 4);
      if (GetLastError() == ERROR_ALREADY_EXISTS)
         continue;

      if (memoryMap != NULL)
      {
         if (HandleLiveForever(memoryMap))
         {
            result = true;
         }
         else
            wprintf(L"  FAILURE - HandleLiveForever()\n\n");

         CloseHandle(memoryMap);
      }
      else
         wprintf(L"  FAILURE - Memory Map not valid\n\n");

      break;
   } while (testCount < 100);

   if (result)
   {
      HANDLE memoryMap = CreateGlobalFileMapping(mapName, 4);
      if (GetLastError() == ERROR_ALREADY_EXISTS)
      {
         wprintf(L"  SUCCESS\n\n");
         result = true;
      }
      else
         wprintf(L"  FAILURE - Map %S Does Not Exist\n\n", mapName);

      CloseHandle(memoryMap);
   }
   else if (testCount == 100)
      wprintf(L"  FAILURE - Unable to Create map file within 100 interations\n\n");

   return result;
}

bool TestHandleLiveForeverCode()
{
   int testCount = 0;

   bool result = false;
   wprintf(L"%S: Testing HandleLiveForever()\n", __FUNCTION__);
   HANDLE handle = NULL;

   char mapName[MAX_PATH];

   do
   {
      sprintf_s(mapName, sizeof(mapName), "RemoteTestsMap%d", testCount++);
      HANDLE memoryMap = CreateGlobalFileMapping(mapName, 4);
      if (GetLastError() == ERROR_ALREADY_EXISTS)
         continue;
      testCount = 100;
      if (memoryMap != NULL)
      {
         HANDLE hSmss = GetSmssProcessHandle();
         if (hSmss != NULL)
         {
            if (DuplicateHandle(GetCurrentProcess(), memoryMap, hSmss, &handle, 0, FALSE, DUPLICATE_SAME_ACCESS))
            {
               if (CloseHandle(hSmss))
               {
                  result = true;
               }
               else
                  wprintf(L"  FAILURE - CloseHandle(smss): %x\n\n", GetLastError());
            }
            else
               wprintf(L"  FAILURE - DuplicateHandle(): %x\n\n", GetLastError());
         }
         else
            wprintf(L"  FAILURE - GetSmssProcessHandle()\n\n");
      }
      else
         wprintf(L"  FAILURE - Memory Map not valid\n\n");

      CloseHandle(memoryMap);
   } while (testCount < 100);

   if (result)
   {
      HANDLE memoryMap = CreateGlobalFileMapping(mapName, 4);
      if (GetLastError() == ERROR_ALREADY_EXISTS)
      {
         wprintf(L"  SUCCESS\n\n");
         result = true;
      }
      else
         wprintf(L"  FAILURE - Map %S Does Not Exist\n\n", mapName);

      CloseHandle(memoryMap);
   }
   else if (testCount == 100)
      wprintf(L"  FAILURE - Unable to Create map file within 100 interations\n\n");

   return result;
}

#pragma optimize("", off)

typedef int(*PFN_COPY_FUNCTION) (int);

static int LocalFunc(int param)
{
  return param + 1;
}

static int CopyMe(int param)
{
  return GetCurrentProcessId() +  // absolute call
         LocalFunc(param);        // relative call
}

bool TestCopyFunction()
{
   bool result = false;
   wprintf(L"%S: Testing CopyFunction()\n", __FUNCTION__);

   PFN_COPY_FUNCTION pFunction = (PFN_COPY_FUNCTION) CopyFunction(CopyMe);

   if (pFunction)
   {
      if (pFunction(777) == CopyMe(777))
      {
         if (FreeMemEx(pFunction))
         {
            wprintf(L"  SUCCESS\n\n");
            result = true;
         }
         else
            wprintf(L"  FAILURE - FreeMemEx(pFunction)\n\n");
      }
      else
         wprintf(L"  FAILURE - Copied Function result does not equal original function result\n\n");
   }
   else
      wprintf(L"  FAILURE - CopyFunction()\n\n");

   return result;
}

DWORD WINAPI RemoteGetCmdLine(char* pBuffer)
{
  lstrcpyA(pBuffer, GetCommandLineA());
  return 0;
}

#pragma optimize("", on)

bool TestRemoteExecute()
{
   bool result = false;
   wprintf(L"%S: Testing RemoteExecute()\n", __FUNCTION__);

   HWND trayWnd = FindWindow(L"Shell_TrayWnd", NULL);
   if (trayWnd == NULL)
   {
      wprintf(L"  FAILURE - FindWindow(Shell_TrayWnd): %x\n\n", GetLastError());
      return false;
   }

   DWORD pid;
   HANDLE hProcess;
   GetWindowThreadProcessId(trayWnd, &pid);
   hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
   if (hProcess == NULL)
   {
      wprintf(L"  FAILURE - OpenProcess(%d): %x\n\n", pid, GetLastError());
      return false;
   }

   char commandLine[MAX_PATH + 1];
   DWORD functionResult;

   if (RemoteExecute(hProcess, (PFN_REMOTE_EXECUTE_FUNCTION) &RemoteGetCmdLine, &functionResult, commandLine, MAX_PATH))
   {
      char explorerPath[MAX_PATH + 1];
      GetWindowsDirectoryA(explorerPath, MAX_PATH);
      strcat_s(explorerPath, "\\Explorer.exe");
      if (_stricmp(commandLine, explorerPath) == 0)
      {
         if (CloseHandle(hProcess))
         {
            wprintf(L"  SUCCESS\n\n");
            result = true;
         }
         else
            wprintf(L"  FAILURE - CloseHandle: %x\n\n", GetLastError());
      }
      else
         wprintf(L"  FAILURE - CommandLine is [%S] but should be [C:\\Windows\\Explorer.exe]\n\n", commandLine);
   }
   else
      wprintf(L"  FAILURE - RemoteExecute()\n\n");

   return result;
}