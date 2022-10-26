// ***************************************************************
//  HookingTests.cpp          version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  tests API hooking framework
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#include "SystemsTest.h"
#include <Systems.h>

#pragma optimize("", off)

// we need to test 9 hooks to check whether enlarging the hook queue works (initial allocation 8 entries)
int Test1 = 0;
int Test2 = 0;
int Test3 = 0;
int Test4 = 0;
int Test5 = 0;
int Test6 = 0;
int Test7 = 0;
int Test8 = 0;
int Test9 = 0;

ULONG (WINAPI *GetCurrentThreadNext1) ();
ULONG (WINAPI *GetCurrentThreadNext2) ();
ULONG (WINAPI *GetCurrentThreadNext3) ();
ULONG (WINAPI *GetCurrentThreadNext4) ();
ULONG (WINAPI *GetCurrentThreadNext5) ();
ULONG (WINAPI *GetCurrentThreadNext6) ();
ULONG (WINAPI *GetCurrentThreadNext7) ();
ULONG (WINAPI *GetCurrentThreadNext8) ();
ULONG (WINAPI *GetCurrentThreadNext9) ();

ULONG WINAPI GetCurrentThreadCallback1()
{ 
   Test1++;
   return GetCurrentThreadNext1();
}

ULONG WINAPI GetCurrentThreadCallback2() { Test2++; return GetCurrentThreadNext2(); }
ULONG WINAPI GetCurrentThreadCallback3() { Test3++; return GetCurrentThreadNext3(); }
ULONG WINAPI GetCurrentThreadCallback4() { Test4++; return GetCurrentThreadNext4(); }
ULONG WINAPI GetCurrentThreadCallback5() { Test5++; return GetCurrentThreadNext5(); }
ULONG WINAPI GetCurrentThreadCallback6() { Test6++; return GetCurrentThreadNext6(); }
ULONG WINAPI GetCurrentThreadCallback7() { Test7++; return GetCurrentThreadNext7(); }
ULONG WINAPI GetCurrentThreadCallback8() { Test8++; return GetCurrentThreadNext8(); }
ULONG WINAPI GetCurrentThreadCallback9() { Test9++; return GetCurrentThreadNext9(); }

typedef int (WINAPI *WSAStartupFunc)(WORD wVersionRequested, LPWSADATA lpWSAData);

int (WINAPI *WSAStartupNext)(WORD wVersionRequested, LPWSADATA lpWSAData);
int WINAPI WSAStartupCallback(WORD wVersionRequested, LPWSADATA lpWSAData)
{
  return WSAStartupNext(wVersionRequested, lpWSAData);
}

bool TestHookQueueEx(DWORD flags)
{
   bool result = false;

   if (!HookAPI("kernel32.dll", "GetCurrentThread", GetCurrentThreadCallback1, (PVOID*) &GetCurrentThreadNext1, flags))
   {
      wprintf(L"  FAILURE - HookAPI(1)\n\n");
      return false;
   }
   if (!HookAPI("kernel32.dll", "GetCurrentThread", GetCurrentThreadCallback2, (PVOID*) &GetCurrentThreadNext2, flags))
   {
      wprintf(L"  FAILURE - HookAPI(2)\n\n");
      return false;
   }
   if (!HookAPI("kernel32.dll", "GetCurrentThread", GetCurrentThreadCallback3, (PVOID*) &GetCurrentThreadNext3, flags))
   {
      wprintf(L"  FAILURE - HookAPI(3)\n\n");
      return false;
   }
   if (!HookAPI("kernel32.dll", "GetCurrentThread", GetCurrentThreadCallback4, (PVOID*) &GetCurrentThreadNext4, flags))
   {
      wprintf(L"  FAILURE - HookAPI(4)\n\n");
      return false;
   }
   if (!HookAPI("kernel32.dll", "GetCurrentThread", GetCurrentThreadCallback5, (PVOID*) &GetCurrentThreadNext5, flags))
   {
      wprintf(L"  FAILURE - HookAPI(5)\n\n");
      return false;
   }
   if (!HookAPI("kernel32.dll", "GetCurrentThread", GetCurrentThreadCallback6, (PVOID*) &GetCurrentThreadNext6, flags))
   {
      wprintf(L"  FAILURE - HookAPI(6)\n\n");
      return false;
   }
   if (!HookAPI("kernel32.dll", "GetCurrentThread", GetCurrentThreadCallback7, (PVOID*) &GetCurrentThreadNext7, flags))
   {
      wprintf(L"  FAILURE - HookAPI(7)\n\n");
      return false;
   }
   if (!HookAPI("kernel32.dll", "GetCurrentThread", GetCurrentThreadCallback8, (PVOID*) &GetCurrentThreadNext8, flags))
   {
      wprintf(L"  FAILURE - HookAPI(8)\n\n");
      return false;
   }
   if (!HookAPI("kernel32.dll", "GetCurrentThread", GetCurrentThreadCallback9, (PVOID*) &GetCurrentThreadNext9, flags))
   {
      wprintf(L"  FAILURE - HookAPI(9)\n\n");
      return false;
   }

   GetCurrentThread();

   if ((Test1 == 1) && (Test2 == 1) && (Test3 == 1) && (Test4 == 1) && (Test5 == 1) && (Test6 == 1) && (Test7 == 1) && (Test8 == 1) && (Test9 == 1))
   {
      if (!Unhook((LPVOID *) &GetCurrentThreadNext9))
      {
         wprintf(L"  FAILURE - Unhook(9)\n\n");
         return false;
      }
      if (!Unhook((LPVOID *) &GetCurrentThreadNext1))
      {
         wprintf(L"  FAILURE - Unhook(1)\n\n");
         return false;
      }
      if (!Unhook((LPVOID *) &GetCurrentThreadNext5))
      {
         wprintf(L"  FAILURE - Unhook(5)\n\n");
         return false;
      }
      Test1 = 0; Test2 = 0; Test3 = 0; Test4 = 0; Test5 = 0; Test6 = 0; Test7 = 0; Test8 = 0; Test9 = 0;

      GetCurrentThread();

      if ((Test1 == 0) && (Test2 == 1) && (Test3 == 1) && (Test4 == 1) && (Test5 == 0) && (Test6 == 1) && (Test7 == 1) && (Test8 == 1) && (Test9 == 0))
      {
         if (!Unhook((LPVOID *) &GetCurrentThreadNext2))
         {
            wprintf(L"  FAILURE - Unhook(2)\n\n");
            return false;
         }
         if (!Unhook((LPVOID *) &GetCurrentThreadNext3))
         {
            wprintf(L"  FAILURE - Unhook(3)\n\n");
            return false;
         }
         if (!Unhook((LPVOID *) &GetCurrentThreadNext4))
         {
            wprintf(L"  FAILURE - Unhook(4)\n\n");
            return false;
         }
         if (!Unhook((LPVOID *) &GetCurrentThreadNext6))
         {
            wprintf(L"  FAILURE - Unhook(6)\n\n");
            return false;
         }
         if (!Unhook((LPVOID *) &GetCurrentThreadNext7))
         {
            wprintf(L"  FAILURE - Unhook(7)\n\n");
            return false;
         }
         if (!Unhook((LPVOID *) &GetCurrentThreadNext8))
         {
            wprintf(L"  FAILURE - Unhook(8)\n\n");
            return false;
         }

         Test1 = 0; Test2 = 0; Test3 = 0; Test4 = 0; Test5 = 0; Test6 = 0; Test7 = 0; Test8 = 0; Test9 = 0;

         GetCurrentThread();

         if ((Test1 == 0) && (Test2 == 0) && (Test3 == 0) && (Test4 == 0) && (Test5 == 0) && (Test6 == 0) && (Test7 == 0) && (Test8 == 0) && (Test9 == 0))
         {
            wprintf(L"  SUCCESS\n\n");
            result = true;
         }
         else
            wprintf(L"  FAILURE - Third pass\n\n");
      }
      else
         wprintf(L"  FAILURE - Second pass\n\n");
   }
   else
      wprintf(L"  FAILURE - First pass\n\n");

   return result;
}

bool TestHooking()
{
   wprintf(L"%S: Testing HookAPI(0) and Unhook()\n", __FUNCTION__);
   return TestHookQueueEx(0);
}

bool TestSafeHooking()
{
   wprintf(L"%S: Testing HookAPI(SAFE_HOOKING) and Unhook()\n", __FUNCTION__);
   return TestHookQueueEx(SAFE_HOOKING);
}

bool TestNoSafeUnhooking()
{
   wprintf(L"%S: Testing HookAPI(NO_SAFE_UNHOOKING) and Unhook()\n", __FUNCTION__);
   return TestHookQueueEx(NO_SAFE_UNHOOKING);
}

bool TestMixtureModeHooking()
{
   wprintf(L"%S: Testing HookAPI(MIXTURE_MODE) and Unhook()\n", __FUNCTION__);
   return TestHookQueueEx(MIXTURE_MODE);
}

void (WINAPI *SleepNext)(DWORD delay);
void WINAPI SleepCallback(DWORD delay)
{
  SleepEx(delay, false);
}

bool InUseTestExcept  = false;
bool InUseTestSuccess = false;

#ifdef _WIN64
   LONG NTAPI MyExceptionHandler(struct _EXCEPTION_POINTERS*)
   {
     InUseTestExcept = true;
     ExitThread(0);
   }
#endif

DWORD WINAPI InUseTestThread(LPVOID)
{
  __try
  {
    Sleep(100);
    InUseTestSuccess = true;
  } __except (1)
  {
    InUseTestExcept = true;
  }
  return 0;
}


bool TestInUse()
{
   bool result = false;
   wprintf(L"%S: Testing Hooking and InUse protection\n", __FUNCTION__);

   #ifdef _WIN64
      // unfortunately the 64bit exception handling logic has changed completely
      // honestly, the new logic is *BAD*, so have to use this hack to make our test work
      HANDLE exceptHandler = AddVectoredExceptionHandler(0, MyExceptionHandler);
   #endif

   // we simulate dll loading/unloading by copying/releasing the callback function
   LPVOID callbackCopy = CopyFunction(SleepCallback);

   if (HookAPI("kernel32.dll", "Sleep", callbackCopy, (LPVOID *) &SleepNext, NO_SAFE_UNHOOKING))
   {
      DWORD tid;
      HANDLE th = CreateThread(NULL, 0, InUseTestThread, NULL, 0, &tid);
      SleepEx(10, false);
      Unhook((LPVOID *) &SleepNext);
      VirtualFree(callbackCopy, 0, MEM_RELEASE);
      WaitForSingleObject(th, INFINITE);
   }

   if (InUseTestExcept)
   {
      InUseTestExcept = false;
      InUseTestSuccess = false;

      LPVOID callbackCopy = CopyFunction(SleepCallback);

      if (HookAPI("kernel32.dll", "Sleep", callbackCopy, (LPVOID *) &SleepNext, 0))
      {
         DWORD tid;
         HANDLE th = CreateThread(NULL, 0, InUseTestThread, NULL, 0, &tid);
         SleepEx(10, false);
         Unhook((LPVOID *) &SleepNext);
         VirtualFree(callbackCopy, 0, MEM_RELEASE);
         WaitForSingleObject(th, INFINITE);
         result = InUseTestSuccess && (!InUseTestExcept);
         if (result)
            wprintf(L"  SUCCESS\n\n");
      }
   }  
   else
      wprintf(L"  FAILURE - Was not InUse during first pass\n\n");

   #ifdef _WIN64
      RemoveVectoredExceptionHandler(exceptHandler);
   #endif

  return result;
}

bool TestImportExportTablePatching2(HMODULE hKernel, LPVOID api)
// MSVC++ is really really clever sometimes
// if we use "GetCurrentProcess" multiple times in the same function
// MSVC++ is not actually asking the address multiple times
// instead the result is stored in a register and reused
// this breaks our test
// so we have to split the import/export table test into 2 pieces
{
   bool result = false;

   if (api != FindRealCode(GetCurrentProcess))
   {
      if (FindRealCode(GetCurrentProcess) == GetProcAddress(hKernel, "GetCurrentProcess"))
      {
         wprintf(L"  SUCCESS\n\n");
         result = true;
      }
      else
         wprintf(L"  FAILURE - FindRealCode(GetCurrentProcess) != GetProcAddress(hKernel, GetCurrentProcess)\n\n");
   }
   else
      wprintf(L"  FAILURE - Api address not changed\n\n");

   return result;
}

bool TestImportExportTablePatching()
{
   bool result = false;
   wprintf(L"%S: Testing Import/Export Table Patching\n", __FUNCTION__);

   HMODULE hKernel = GetModuleHandle(L"kernel32.dll");
   if (hKernel)
   {
      LPVOID api = FindRealCode(GetCurrentProcess);
      if (api == GetProcAddress(hKernel, "GetCurrentProcess"))
      {
         if (HookCode(GetImageProcAddress(hKernel, "GetCurrentProcess"), GetCurrentThreadCallback1, (PVOID *) &GetCurrentThreadNext1, MIXTURE_MODE))
         {
            if (Unhook((PVOID *) &GetCurrentThreadNext1))
            {
               // if the MIXTURE_MODE hook worked alright, the address of the API should have officially changed
               // FindRealCode should return the new address if the import table got correctly patched
               // GetProcAddress should return the new address if the export table got correctly patched
               // result = TestImportExportTablePatching2(hKernel, api);
               if (api != FindRealCode(GetCurrentProcess))
               {
                  if (FindRealCode(GetCurrentProcess) == GetProcAddress(hKernel, "GetCurrentProcess"))
                  {
                     wprintf(L"  SUCCESS\n\n");
                     result = true;
                  }
                  else
                     wprintf(L"  FAILURE - FindRealCode(GetCurrentProcess) != GetProcAddress(hKernel, GetCurrentProcess)\n\n");
               }
               else
                  wprintf(L"  FAILURE - Api address not changed\n\n");
            }
            else
               wprintf(L"  FAILURE - Unhook\n\n");
         }
         else
            wprintf(L"  FAILURE - HookCode\n\n");
      }
      else
         wprintf(L"  FAILURE - Api address != GetProcAddress(hKernel, GetCurrentProcess)\n\n");
   }
   else
      wprintf(L"  FAILURE - GetModuleHandle(kernel32.dll): %x\n\n", GetLastError());

  return result;
}

#pragma optimize("", on)
