// ***************************************************************
//  MainTest.cpp              version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  tool to test various functionality areas
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#pragma warning(disable: 4200 4100 4127) // Zero length array and conditional expression is constant
#include "SystemsTest.h"
#include <Systems.h>

void TestFunction(LPCWSTR module, LPCSTR function);
void WriteApi(HANDLE fh, HMODULE dll, LPCSTR api);
void RegressionTest(wchar_t * dllName);

ULONG WINAPI RemoteFunc(PVOID)
{
  return 777;
}

#ifndef _WIN64

  typedef struct _PEB64 {
      BOOLEAN InheritedAddressSpace;
      PVOID64 Mutant;
      PVOID64 ImageBaseAddress;
      PVOID64 LdrData;
  } PEB64, * __ptr64 PPEB64;

  SYSTEMS_API BOOL WINAPI NotInitializedYet64(PPEB64 pPeb, HANDLE hProcess)
  {
    BOOL result = false;

    if (pPeb == NULL)
      pPeb = (PPEB64) GetPeb64(hProcess);

    if (pPeb != NULL)
    {
      ULONG64 pLdrData;
      PEB64 pebofs;
      if (!ReadProcessMemory64(hProcess, (ULONG64) pPeb + ((ULONG) &(pebofs.LdrData) - (ULONG) (&pebofs)), &pLdrData, sizeof(pLdrData), NULL))
      {
        if (pLdrData == 0)
          result = true;
      }
    }

    return result;
  }

#endif

DWORD WINAPI TestFunc(LPVOID params)
{
  return 0;
}

int wmain(int argc, wchar_t* argv[])
{

/*
STARTUPINFO si;
PROCESS_INFORMATION pi;
WCHAR str[MAX_PATH];
memset(&si, 0, sizeof(si));
wcscpy(str, L"calc.exe");

CreateProcessW(NULL, str, NULL, NULL, false, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
*/

    BOOL success = false;
    BOOL bx[50];
    int index = 0;
    StaticLibHelper_Init(NULL);

    EnableAllPrivileges();

    bx[index++] = TestAllocEx();
    bx[index++] = TestProtect();
    bx[index++] = TestCopyFunction();
    bx[index++] = TestHandleLiveForever();
    bx[index++] = TestHandleLiveForeverCode();
    bx[index++] = TestHandleLiveForeverWithProcess();
    bx[index++] = TestRemoteExecute();
    bx[index++] = TestHooking();
    bx[index++] = TestSafeHooking();
    bx[index++] = TestNoSafeUnhooking();
    bx[index++] = TestMixtureModeHooking();
    bx[index++] = TestImportExportTablePatching();
    bx[index++] = TestInUse();
    bx[index++] = TestInjection();
//    bx[index++] = TestDriverInjection();
    bx[index++] = TestIpc();
    StaticLibHelper_Final(NULL);

    success = true;
    for (int i = 0; i < index; i++)
    {
       if (!bx[i])
       {
          success = false;
          break;
       }
    }

    if (!success)
       return -1;
    else
       return 0;

/*  
  for(int i = 0; i < 22; i++)
  {
    CCodeParse a(regProc);
    wprintf(L"%s\n", a.ToString());
    regProc = a.mpNext;
  }

//  LPWSTR procName = GetImageProcName(hModule, proc);
//  FreeImageProcName(procName);

//  LPVOID procO = GetImageProcAddress(hModule, 6);
//  LPVOID regProcO = GetProcAddress(hModule, (LPCSTR) 6);
   wprintf(L"Press 'd' for regression test of \"disAsm32.dll\".\r\n");
   wprintf(L"Press 'k' for regression test of \"kernel32.dll\".\r\n");
   wprintf(L"Press 'n' for regression test of \"ntdll.dll\".\r\n");
   wprintf(L"Press 'u' for regression test of \"user32.dll\".\r\n");
   wprintf(L"Press 'g' for regression test of \"gdi32.dll\".\r\n");
  wint_t key = _getwch();
  if (key == L'd'))
  {
    wprintf(L"\n\n");
    #ifdef _WIN64
      RegressionTest(L"disAsm64.dll");
    #else
      RegressionTest(L"disAsm32.dll");
    #endif
  }
  else
  {
    if (key == L'k'))
    {
      wprintf(L"\n\n");
      RegressionTest(L"kernel32.dll");
    }
    else
    {
      if (key == L'n'))
      {
        wprintf(L"\n\n");
        RegressionTest(L"ntdll.dll");
      }
      else
      {
        if (key == L'u'))
        {
          wprintf(L"\n\n");
          RegressionTest(L"user32.dll");
        }
        else
        {
          if (key == L'g'))
          {
            wprintf(L"\n\n");
            RegressionTest(L"gdi32.dll");
          }
        }
      }
    }
  }

  return 0;
  */
}

void RegressionTest(wchar_t * dllName)
{
   HMODULE dll = LoadLibrary(dllName);
   
   HANDLE fh = CreateFile(L"./msvc++.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
   DWORD tick = GetTickCount();
   CHAR api[10];

   ZeroMemory(api, 10);

   for (int i = 0; i < 0xffff; i++)
   {
      WriteApi(fh, dll, (LPCSTR) i);
   } 

   tick = GetTickCount() - tick;
   SString tickStr;
   tickStr.Format(L"%d", tick);
   MessageBox(0, tickStr.GetBuffer(), L"Time", 0);

   CloseHandle(fh);
   FreeLibrary(dll);
}

void WriteApi(HANDLE fh, HMODULE dll, LPCSTR api)
{
   LPVOID apiAddr = GetProcAddress(dll, api);
   if (apiAddr)
   {
      if ((DWORD) api < 0x10000)
        wprintf(L"%d\n", (DWORD) api);
      else
        wprintf(L"%S\n", api);

      CFunctionParse func(apiAddr);

      if (func.mIsValid)
      {
        SString funcStr;
        DWORD len;

        funcStr = func.ToString(NULL, 0, TRUE);
        funcStr += L"\r\n";
        WriteFile(fh, funcStr.GetBuffer(), (DWORD) funcStr.Length() * sizeof(wchar_t), &len, NULL);
      }
   }
}

void TestFunction(LPCWSTR module, LPCSTR function)
{
  static wchar_t *yes = L"Interceptable";
  static wchar_t *no = L"CANNOT BE INTERCEPTED";
  wchar_t *answer = NULL;

  HMODULE hModule = LoadLibrary(module);
  
  if (hModule == NULL)
  {
    wprintf(L"Unable to load: %s\n\n", module);
    return;
  }

  LPVOID regProc = GetProcAddress(hModule, function);

  if (regProc == NULL)
  {
    wprintf(L"Unable to find %S in %s.\n\n", function, module);
  }
  else
  {
    CFunctionParse a(regProc);
    if (a.IsInterceptable())
      answer = yes;
    else
      answer = no;

    wprintf(L"%s-%S: %s\n\n", module, function, answer);
    wprintf(L"%s\n", a.ToString(NULL, 0, TRUE).GetBuffer());
  }
  FreeLibrary(hModule);
  
}
// *************************** Test Injection Driver *******************

#ifndef _WIN64

// ZwProtect/Read/WriteVirtualMemory
// undocumented API, *NOT* exported in kernel land
// so we have to call these APIs by their service id
NTSTATUS ZwXxxVirtualMemory(
    IN ULONG ServiceId,
    IN HANDLE ProcessHandle,
    IN PVOID Param1,
    IN PVOID Param2,
    IN ULONG Param3,
    OUT PULONG Param4)
{
  _asm
  {
    // this is the same what ntdll.dll does in user land
    // when ZwProtect/Read/WriteVirtualMemory are called
    mov eax, ServiceId
    lea edx, ServiceId
    add edx, 4
    int 0x2e
  }
}

#endif

// ********************************************************************

#pragma pack(1)
typedef struct tagLdrLoadStub
{
   BYTE  jmp;
   DWORD target;
} LDR_LOAD_STUB;

// our DLL injection buffer structure
// only the most important fields are declared here
// the rest is not important for our driver
typedef struct tagNtHookRecord
{
   DWORD    Size;
   DWORD    zwProtectVm;
   DWORD    zwReadVm;
   DWORD    zwWriteVm;
   DWORD    LdrLoadCallbackOffset;
   LPVOID   pEntryPoint;
   DWORD    OldEntryPoint;
   HMODULE  hNtDll;
   LDR_LOAD_STUB *pLdrLoadStub;
   LDR_LOAD_STUB LdrLoadStub;
} NT_HOOK_RECORD;
#pragma pack()

// we store the buffer during the whole life of our driver
NT_HOOK_RECORD *pDllInjectionBuffer = NULL;



BOOL CheckDllInjectBuf()
// this function opens the dll injection buffer memory map
// the content is then copied to our internal buffer
{
   if (pDllInjectionBuffer == NULL)
   {
      HANDLE hMap = OpenGlobalFileMapping("mchInjDrvMap", true);
      if (hMap != NULL)
      {
         NT_HOOK_RECORD *p = (NT_HOOK_RECORD *) MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
         if (p!= NULL)
         {
            pDllInjectionBuffer = (NT_HOOK_RECORD *) LocalAlloc(LPTR, p->Size);
            CopyMemory(pDllInjectionBuffer, p, p->Size);
            UnmapViewOfFile(p);
         }
         CloseHandle(hMap);
      }
   }
   return pDllInjectionBuffer != NULL;
}
