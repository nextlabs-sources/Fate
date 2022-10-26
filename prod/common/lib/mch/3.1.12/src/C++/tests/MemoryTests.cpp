// ***************************************************************
//  MemoryTests.cpp           version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  tests memory functions
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#include "SystemsTest.h"
#include <Systems.h>

bool TestAllocEx()
{
   bool result = false;
   wprintf(L"%S: Testing AllocMemEx(), FreeMemEx()\n", __FUNCTION__);

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
      wprintf(L"  FAILURE - OpenProcess(): %x\n\n", GetLastError());
      return false;
   }

   MEMORY_BASIC_INFORMATION mbi;
   SIZE_T size;
   LPVOID dummy = AllocMemEx(4, hProcess);
   if (dummy == NULL)
   {
      wprintf(L"  FAILURE - AllocMemEx()\n\n");
      CloseHandle(hProcess);
      return false;
   }

   LPVOID buf = AllocMemEx(4, hProcess);
   if (buf != NULL)
   {
      if ((DWORD) buf >= 0x50000000)
      {
         if (FreeMemEx(buf, hProcess))
         {
            if (VirtualQueryEx(hProcess, buf, &mbi, sizeof(mbi)) == sizeof(mbi))
            {
               if (mbi.State == MEM_FREE)
               {
                  if (AllocMemEx(4, hProcess) == buf)
                  {
                     if (WriteProcessMemory(hProcess, buf, &hProcess, 4, &size))
                     {
                        if (FreeMemEx(buf, hProcess))
                        {
                           wprintf(L"  SUCCESS\n\n");
                           result = true;
                        }
                        else
                           wprintf(L"  FAILURE - FreeMemEx()\n\n");
                     }
                     else
                        wprintf(L"    FAILURE - WriteProcessMemory(): %x\n\n", GetLastError());
                  }
                  else
                     wprintf(L"    FAILURE - AllocMemEx()\n\n");
               }
               else
                  wprintf(L"    FAILURE - mbi.State not MEM_FREE\n\n");
            }
            else
               wprintf(L"    FAILURE - VirtualQueryEx(): %x\n\n", GetLastError());
         }
         else
            wprintf(L"    FAILURE - FreeMemEx()\n\n");
      }
      else
         wprintf(L"  buf (%p) < 0x50000000\n\n", buf);
   }
   else
      wprintf(L"    FAILURE - AllocMemEx()\n\n");

   FreeMemEx(dummy, hProcess);
   CloseHandle(hProcess);
   return result;
}

bool TestProtect()
{
   bool result = false;
   wprintf(L"%S: Testing ProtectMemory(), UnprotectMemory(), IsMemoryProtected()\n", __FUNCTION__);

   LPVOID buf = AllocMemEx(4);
   if (buf == NULL)
   {
      wprintf(L"  FAILURE - AllocMemEx()\n\n");
      return false;
   }
   if (!IsMemoryProtected(buf))
   {
      DWORD op;
      if (VirtualProtect(buf, 4, PAGE_EXECUTE_READWRITE, &op))
      {
         if (IsMemoryProtected(buf))
         {
            if (VirtualProtect(buf, 4, op, &op))
            {
               if (!IsMemoryProtected(buf))
               {
                  wprintf(L"  SUCCESS\n\n");
                  result = true;
               }
               else
                  wprintf(L"    FAILURE - IsMemoryProtected() should have returned false\n\n");
            }
            else
               wprintf(L"    FAILURE - UnprotectMemory()\n\n");
         }
         else
            wprintf(L"    FAILURE - IsMemoryProtected() should have returned true\n\n");
      }
      else
         wprintf(L"    FAILURE - ProtectMemory()\n\n");
   }
   else
      wprintf(L"    FAILURE - IsMemoryProtected() should have return false\n\n");

   FreeMemEx(buf);
   return result;
}