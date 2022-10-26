// ***************************************************************
//  ProcessTools.cpp          version: 1.0.9  ·  date: 2016-05-17
//  -------------------------------------------------------------
//  functions that work against processes
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2016 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2016-05-17 1.0.9 fixed: preferred allocation address was sometimes ignored
// 2016-03-16 1.0.8 fixed some PAGE_EXECUTE_READWRITE security issues
// 2015-09-10 1.0.7 using official GetThread/ProcessId APIs now if available
// 2015-04-20 1.0.6 AllocMemEx performance improvement
// 2013-10-01 1.0.5 added IsAdminAndElevated function
// 2013-03-13 1.0.4 (1) fixed: IPC messages sometimes contained wrong session id
//                  (2) added support for csrss injection in Windows 8
//                  (3) fixed crash when hooking system APIs in x64 MSSQL
// 2012-07-18 1.0.3 fixed: x64 RtlCreateUserThread solution crashed in win7
// 2012-05-21 1.0.2 (1) improved 64bit injection into already running processes
//                  (2) added support for non-large-address-aware x64 processes
//                  (3) improved 64bit injection thread safety
//                  (4) fixed bug in CopyFunction
//                  (5) fixed win8 crash when 64bit exe injects 32bit dlls
// 2011-03-26 1.0.1 (1) some changes for improved wow64 injection
//                  (2) added workaround for XP/2003 wow64 bug
//                  (3) fixed: crash when injecting wow64 from 32bit + 64bit exe
//                  (4) added detection for Linux' Windows emulation "wine"
// 2010-01-10 1.0.0 initial version

#define _PROCESSTOOLS_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

#ifdef _WIN64
  #include <tlhelp32.h>
#endif

//----------------------------------------------------------------------
// Memory allocation in the specified process.
// If hProcess is NULL, memory is allocated in calling process.
//----------------------------------------------------------------------
LPVOID PreviousAllocationAddress = NULL;
#ifdef _WIN64
  SYSTEMS_API LPVOID WINAPI AllocMemEx(DWORD size, HANDLE hProcess, LPVOID pPreferredAddress)
#else
  SYSTEMS_API LPVOID WINAPI AllocMemEx(DWORD size, HANDLE hProcess)
#endif
{
  TraceVerbose(L"%S(%d, %X)", __FUNCTION__, size, hProcess);

  ASSERT(size > 0);

  LPVOID address = NULL;

  __try
  {
    if (size > 0)
    {
      if (hProcess == NULL)
        hProcess = GetCurrentProcess();

      MEMORY_BASIC_INFORMATION mbi;
      LPVOID p = NULL;

      #ifdef _WIN64
        if (pPreferredAddress != NULL)
        {
          p = (LPVOID) pPreferredAddress;
          while ((VirtualQueryEx(hProcess, p, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION)) && ((LONG_PTR) mbi.BaseAddress - (LONG_PTR) pPreferredAddress < 0x7fff0000))
          {
            if (mbi.State == MEM_FREE)
            {
              address = VirtualAllocEx(hProcess, mbi.BaseAddress, size, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
              if (address != NULL)
                break;
              mbi.RegionSize = 0x10000;
            }
            if (mbi.RegionSize < 0x10000)
              mbi.RegionSize = 0x10000;
            else
              mbi.RegionSize = mbi.RegionSize & 0xFFFFFFFFFFFF0000;
            p = (LPVOID) ((ULONG_PTR) p + mbi.RegionSize);
          }
        }
        if (!address)
        {
          if ((pPreferredAddress) && (((ULONG_PTR) pPreferredAddress < 0x70000000) || ((ULONG_PTR) pPreferredAddress > 0x80000000)))
            p = pPreferredAddress;
          else
            if ((!pPreferredAddress) && ((hProcess == GetCurrentProcess()) || Is64bitProcess(hProcess)))
              // allocate <= 0x7feffff0000 to avoid fragmentation
              p = (LPVOID) 0x7feffff0000;
            else
      #endif

              // allocate <= 0x71af0000 to avoid fragmentation
              p = (LPVOID) GetMadCHookOption(X86_ALLOCATION_ADDRESS);

          #ifdef _WIN64
            if ((!pPreferredAddress) && (PreviousAllocationAddress))
          #else
            if (PreviousAllocationAddress)
          #endif
            p = PreviousAllocationAddress;
          while (VirtualQueryEx(hProcess, p, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
          {
            if (mbi.State == MEM_FREE)
            {
              address = VirtualAllocEx(hProcess, mbi.BaseAddress, size, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
              break;
            }
            if (p == mbi.AllocationBase)
              p = (LPVOID) ((ULONG_PTR) p - 0x10000);
            else
              p = mbi.AllocationBase;
          }
          #ifdef _WIN64
            if ((!pPreferredAddress) && (address))
          #else
            if (address)
          #endif
            PreviousAllocationAddress = address;

      #ifdef _WIN64
        }
      #endif

      address = VirtualAllocEx(hProcess, address, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    }
  }
  __except (ExceptionFilter(L"AllocMemEx", GetExceptionInformation()))
  {
    address = NULL;
  }
  return address;
}

//----------------------------------------------------------------------
// Memory is freed from the specified process.
// If hProcess is NULL, memory is freed from calling process.
//----------------------------------------------------------------------
SYSTEMS_API BOOL WINAPI FreeMemEx(LPVOID pMemory, HANDLE hProcess)
{
  TraceVerbose(L"%S(%p, %X)", __FUNCTION__, pMemory, hProcess);

  ASSERT(pMemory!= NULL);

  BOOL result = false;

  if (hProcess == NULL)
    hProcess = GetCurrentProcess();

  result = VirtualFreeEx(hProcess, pMemory, 0, MEM_RELEASE);

  return result;
}

//----------------------------------------------------------------------
// Is memory at address protected?
//----------------------------------------------------------------------
SYSTEMS_API BOOL WINAPI IsMemoryProtected(LPVOID address)
{
  TraceVerbose(L"%S(%p)", __FUNCTION__, address);

  ASSERT(address != NULL);

  BOOL result = true;

  MEMORY_BASIC_INFORMATION mbi;
  if (VirtualQuery(address, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
  {
    if ( ((mbi.Protect & PAGE_EXECUTE_READWRITE) != 0) ||
         ((mbi.Protect & PAGE_EXECUTE_WRITECOPY) != 0) ||
         ((mbi.Protect & PAGE_READWRITE        ) != 0) ||
         ((mbi.Protect & PAGE_WRITECOPY        ) != 0)    )
      result = false;
  }
  else
    Trace(L"%S Failure - VirtualQuery: %X", __FUNCTION__, GetLastError());

  return result;
}

//----------------------------------------------------------------------
// Returns the pointer to the PEB of a process identified by its handle
//----------------------------------------------------------------------

static PFN_NT_QUERY_INFORMATION_PROCESS pfnNtQueryInformationProcess = NULL;

SYSTEMS_API PPEB WINAPI GetPeb(HANDLE hProcess)
{
  TraceVerbose(L"%S(%X)", __FUNCTION__, hProcess);

  ASSERT(hProcess != NULL);

  PPEB result = NULL;

  PROCESS_BASIC_INFORMATION pbi;
  memset(&pbi, 0, sizeof(PROCESS_BASIC_INFORMATION));

  if (!pfnNtQueryInformationProcess)
    pfnNtQueryInformationProcess = (PFN_NT_QUERY_INFORMATION_PROCESS) NtProc(CNtQueryInformationProcess);
  if (pfnNtQueryInformationProcess)
  {
    NTSTATUS status = pfnNtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);
    if (status == 0)
      result = pbi.PebBaseAddress;
    else
    {
      Trace(L"%S Failure - NtQueryInformationProcess: %X", __FUNCTION__, status);
      SetLastError(status);
    }
  }
  else
  {
    Trace(L"%S Failure - pfnNtQueryInformationProcess is NULL", __FUNCTION__);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  }
  return result;
}

#ifndef _WIN64

  typedef struct _PROCESS_BASIC_INFORMATION64 {
    NTSTATUS ExitStatus;
    ULONG64 PebBaseAddress;
    ULONG64 AffinityMask;
    KPRIORITY BasePriority;
    ULONG64 UniqueProcessId;
    ULONG64 InheritedFromUniqueProcessId;
  } PROCESS_BASIC_INFORMATION64;
  typedef PROCESS_BASIC_INFORMATION64 *PPROCESS_BASIC_INFORMATION64;

  typedef NTSTATUS (__stdcall *NTWOW64QUERYINFORMATIONPROCESS64)(HANDLE ProcessHandle, ULONG ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PVOID ReturnLength);

  NTWOW64QUERYINFORMATIONPROCESS64 NtQueryInformationProcess64 = NULL;

  SYSTEMS_API ULONGLONG WINAPI GetPeb64(HANDLE hProcess)
  {
    if (!NtQueryInformationProcess64)
      NtQueryInformationProcess64 = (NTWOW64QUERYINFORMATIONPROCESS64) NtProc(CNtWow64QueryInfoProcess64);

    if (NtQueryInformationProcess64)
    {
      PROCESS_BASIC_INFORMATION64 pbi64;

      if (!NtQueryInformationProcess64(hProcess, ProcessBasicInformation, &pbi64, sizeof(pbi64), NULL))
        return pbi64.PebBaseAddress;
    }

    return NULL;
  }

  typedef NTSTATUS (__stdcall *NTWOW64READVIRTUALMEMORY64)(HANDLE ProcessHandle, ULONG64 BaseAddress, PVOID Buffer, ULONG64 BufferSize, PULONG64 NumberOfBytesRead);

  NTWOW64READVIRTUALMEMORY64 NtReadVirtualMemory64 = NULL;

  SYSTEMS_API BOOL WINAPI ReadProcessMemory64(HANDLE ProcessHandle, ULONG64 BaseAddress, PVOID Buffer, ULONG64 BufferSize, PULONG64 NumberOfBytesRead)
  {
    if (!NtReadVirtualMemory64)
      NtReadVirtualMemory64 = (NTWOW64READVIRTUALMEMORY64) NtProc(CNtWow64ReadVirtualMemory64);

    if (NtReadVirtualMemory64)
      return !NtReadVirtualMemory64(ProcessHandle, BaseAddress, Buffer, BufferSize, NumberOfBytesRead);

    return false;
  }

#endif

bool CheckDllName(LPCWSTR subStr, LPCWSTR str)
{
  int subLen = (int) wcslen(subStr);
  int strLen = (int) wcslen(str);
  return (subLen <= strLen) && (!_wcsicmp(subStr, &str[strLen - subLen]));
}

#ifdef _WIN64

  PVOID pfn32bitKernelAPIs[15] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  bool kernelApisInitialized = false;

  PVOID GetKernelAPI(ULONG index)
  {
    return pfn32bitKernelAPIs[index];
  }

  #define WOW64_CONTEXT_SEGMENTS_ 0x00010004L

  static PFN_WOW64_GET_THREAD_SELECTOR_ENTRY Wow64GetThreadSelectorEntry_ = NULL;
  static PFN_WOW64_GET_THREAD_CONTEXT Wow64GetThreadContext_ = NULL;

  ULONG_PTR GetWow64Teb(HANDLE threadHandle)
  {
    // first we try the official way, works only in Windows 7 (and newer)
    WOW64_CONTEXT_ context;
    context.SegFs = 0;
    context.ContextFlags = WOW64_CONTEXT_SEGMENTS_;
    if (!Wow64GetThreadContext_)
      Wow64GetThreadContext_ = (PFN_WOW64_GET_THREAD_CONTEXT) KernelProc(CWow64GetThreadContext);
    if (!Wow64GetThreadSelectorEntry_)
      Wow64GetThreadSelectorEntry_ = (PFN_WOW64_GET_THREAD_SELECTOR_ENTRY) KernelProc(CWow64GetThreadSelectorEntry);
    LDT_ENTRY ldt;
    if ( (Wow64GetThreadContext_) && (Wow64GetThreadSelectorEntry_) && 
         (Wow64GetThreadContext_(threadHandle, &context)) &&
         (Wow64GetThreadSelectorEntry_(threadHandle, context.SegFs, &ldt)) )
    {
      // the FS segment register points to the TEB (thread environment block)
      return ((ULONG_PTR) ldt.HighWord.Bytes.BaseHi << 24) + ((ULONG_PTR) ldt.HighWord.Bytes.BaseMid << 16) + (ULONG_PTR) ldt.BaseLow;
    }

    // the official way failed, so we're probably on Vista or XP
    // let's hack around the problem...  :-(

    THREAD_BASIC_INFORMATION tbi;
    PFN_NT_QUERY_INFORMATION_THREAD pNtQuery = (PFN_NT_QUERY_INFORMATION_THREAD) NtProc(CNtQueryInformationThread);
    if ( (pNtQuery != NULL) && (!pNtQuery(threadHandle, ThreadBasicInformation, &tbi, sizeof(tbi), NULL)) )
      // the "WOW64 TEB" seems to always be at "native TEB + 0x2000"
      return (ULONG_PTR) tbi.TebBaseAddress + 0x2000;

    return 0;
  }

  PVOID32 GetPeb32(HANDLE processHandle)
  // retrieve the 32bit PEB of some 32bit process from within our 64bit process
  {
    PVOID32 result = NULL;

    // first we enumerate all threads
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (h != INVALID_HANDLE_VALUE)
    {
      THREADENTRY32 te;
      DWORD pid = ProcessHandleToId(processHandle);
      te.dwSize = sizeof(te);
      if ((pid) && (Thread32First(h, &te)))
      {
        do
        {
          if (te.th32OwnerProcessID == pid)
          {
            // we found a thread which belongs to the target 32bit process
            HANDLE th = OpenThread(THREAD_QUERY_INFORMATION | THREAD_GET_CONTEXT, false, te.th32ThreadID);
            if (th)
            {
              // we were able to open the thread, now let's get the wow64 thread environment block
              ULONG_PTR teb = GetWow64Teb(th);
              CloseHandle(th);
              if (teb)
              {
                PVOID32 peb;
                SIZE_T read;
                if (ReadProcessMemory(processHandle, (LPVOID) (teb + 0x30), &peb, sizeof(peb), &read))
                  // at TEB + 0x30 we finally find the PEB (process environment block)
                  result = peb;
              }
            }
            break;
          }
        } while (Thread32Next(h, &te));
      }
      CloseHandle(h);
    }

    return result;
  }

  HMODULE GetRemoteModuleHandle32(HANDLE ProcessHandle, LPCWSTR DllName, LPWSTR DllFullPath)
  // retrieve the module handle and full file path of a 32bit module in a remote 32bit process
  {
    HMODULE result = NULL;

    __try
    {
      // first let's get the 32bit PEB (Process Environment Block)
      DWORD peb = (DWORD) GetPeb32(ProcessHandle);
      if (peb)
      {
        // we got it, now let's get the loader data
        ULONG ldr;
        ULONG loopEnd;
        ULONG mi[9];
        SIZE_T c1;
        if ( (ReadProcessMemory(ProcessHandle, (PVOID) (ULONGLONG) (peb + 0x0c), &ldr,      4, &c1)) && (c1 ==  4) &&
             (ReadProcessMemory(ProcessHandle, (PVOID) (ULONGLONG) (ldr + 0x14), &loopEnd,  4, &c1)) && (c1 ==  4) &&
             (ReadProcessMemory(ProcessHandle, (PVOID) (ULONGLONG) loopEnd,      &mi,      36, &c1)) && (c1 == 36)    )
        {
          while ( (mi[0] != loopEnd) &&
                  (ReadProcessMemory(ProcessHandle, (PVOID) (ULONGLONG) mi[0], &mi, 36, &c1)) && (c1 == 36) )
          {
            // mi[0] = pointer to next dll
            // mi[4] = dll handle
            // mi[8] = full dll file name
            if ( (mi[8] != NULL) &&
                 (ReadProcessMemory(ProcessHandle, (PVOID) (ULONGLONG) mi[8], (PVOID) DllFullPath, MAX_PATH * 2, &c1)) && (c1 == MAX_PATH * 2) )
            {
              if (CheckDllName(DllName, DllFullPath))
              {
                // found the dll we're looking for
                UINT (WINAPI *GetSystemWow64DirectoryW)(LPTSTR lpBuffer, UINT uSize) = (UINT (WINAPI *)(LPTSTR, UINT)) KernelProc(CGetSystemWow64DirectoryW);
                if (GetSystemWow64DirectoryW)
                {
                  GetSystemWow64DirectoryW(DllFullPath, MAX_PATH);
                  wcscat_s(DllFullPath, MAX_PATH, L"\\");
                  wcscat_s(DllFullPath, MAX_PATH, DllName);
                }
                result = (HMODULE) (ULONGLONG) mi[4];
                break;
              }
            }
          }
        }
      }
    } __except (1) {}

    return result;
  }

  HMODULE GetRemoteModuleHandle64(HANDLE ProcessHandle, LPCWSTR DllName)
  // retrieve the module handle of a 64bit module
  {
    HMODULE result = NULL;

    __try
    {
      // first let's get the 64bit PEB (Process Environment Block)
      ULONG_PTR peb = (ULONG_PTR) GetPeb(ProcessHandle);
      if (peb)
      {
        // we got it, now let's get the loader data
        ULONG_PTR ldr;
        ULONG_PTR loopEnd;
        ULONG_PTR mi[9];
        SIZE_T c1;
        if ( (ReadProcessMemory(ProcessHandle, (PVOID) (peb + 0x18), &ldr,      8, &c1)) && (c1 ==  8) &&
             (ReadProcessMemory(ProcessHandle, (PVOID) (ldr + 0x20), &loopEnd,  8, &c1)) && (c1 ==  8) &&
             (ReadProcessMemory(ProcessHandle, (PVOID) (loopEnd   ), &mi,      72, &c1)) && (c1 == 72)    )
        {
          while ( (mi[0] != loopEnd) &&
                  (ReadProcessMemory(ProcessHandle, (PVOID) mi[0], &mi, 72, &c1)) && (c1 == 72) )
          {
            // mi[0] = pointer to next dll
            // mi[4] = dll handle
            // mi[8] = full dll file name
            WCHAR dllFullPath[MAX_PATH];
            if ( (mi[8] != NULL) &&
                 (ReadProcessMemory(ProcessHandle, (PVOID) mi[8], dllFullPath, MAX_PATH * 2, &c1)) && (c1 == MAX_PATH * 2) &&
                 (CheckDllName(DllName, dllFullPath)) )
            {
              // found the dll we're looking for
              result = (HMODULE) mi[4];
              break;
            }
          }
        }
      }
    } __except (1) {}

    return result;
  }

  bool Init32bitKernelAPIs(HANDLE hProcess)
  {
    if (!kernelApisInitialized)
    {
      WCHAR fileName[260];
      PVOID imageBaseAddress = GetRemoteModuleHandle32(hProcess, L"kernel32.dll", fileName);
      if (imageBaseAddress != NULL)
      {
        LPSTR apiNames[12];
        for (int i1 = 0; i1 < 12; i1++)
          apiNames[i1] = (LPSTR) LocalAlloc(LPTR, 25);
        DecryptStr(CGetVersion,           apiNames[ 0], 25);
        DecryptStr(CVirtualFree,          apiNames[ 1], 25);
        DecryptStr(CSetErrorMode,         apiNames[ 2], 25);
        DecryptStr(CLoadLibraryW,         apiNames[ 3], 25);
        DecryptStr(CGetLastError,         apiNames[ 4], 25);
        DecryptStr(CGetModuleHandleW,     apiNames[ 5], 25);
        DecryptStr(CGetCurrentProcessId,  apiNames[ 6], 25);
        DecryptStr(COpenFileMappingA,     apiNames[ 7], 25);
        DecryptStr(CMapViewOfFile,        apiNames[ 8], 25);
        DecryptStr(CUnmapViewOfFile,      apiNames[ 9], 25);
        DecryptStr(CCloseHandle,          apiNames[10], 25);
        DecryptStr(CFreeLibrary,          apiNames[11], 25);
        if (GetImageProcAddressesRaw(fileName, imageBaseAddress, (LPCSTR*) apiNames, pfn32bitKernelAPIs, 12))
        {
          imageBaseAddress = GetRemoteModuleHandle32(hProcess, L"ntdll.dll", fileName);
          if (imageBaseAddress != NULL)
          {
            DecryptStr(CRtlEnterCriticalSection, apiNames[0], 25);
            DecryptStr(CRtlLeaveCriticalSection, apiNames[1], 25);
            DecryptStr(CRtlExitUserThread,       apiNames[2], 25);
            if (!GetImageProcAddressesRaw(fileName, imageBaseAddress, (LPCSTR*) apiNames, &pfn32bitKernelAPIs[12], 3))
              pfn32bitKernelAPIs[0] = NULL;
          } else
            pfn32bitKernelAPIs[0] = NULL;
        } else
          pfn32bitKernelAPIs[0] = NULL;
        for (int i1 = 0; i1 < 12; i1++)
          LocalFree(apiNames[i1]);
      }
      kernelApisInitialized = (pfn32bitKernelAPIs[0] != NULL) && (pfn32bitKernelAPIs[12] != NULL);
    }

    return kernelApisInitialized;
  }

  bool IsProcessLargeAddressAware(HANDLE hProcess)
  // find out if the specified 64bit process is aware of large addresses
  // if it is not, system dlls are loaded in the lower 4GB of the address range
  {
    ULONG_PTR exeImage;
    IMAGE_NT_HEADERS64 nh;
    ULONG nhOffset;
    SIZE_T read;
    if ( (ReadProcessMemory(hProcess, (LPVOID) ((ULONG_PTR) GetPeb(hProcess) + 0x10), &exeImage, sizeof(exeImage), &read)) &&
         (ReadProcessMemory(hProcess, (LPVOID) (exeImage + NT_HEADERS_OFFSET       ), &nhOffset, sizeof(nhOffset), &read)) &&
         (ReadProcessMemory(hProcess, (LPVOID) (exeImage + nhOffset                ), &nh,       sizeof(nh      ), &read)) &&
         ((nh.FileHeader.Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE) == 0) )
      return false;
    return true;
  }

  PVOID LargeAddressAwareApiTo2GB(HANDLE hProcess, PVOID largeApi)
  {
    MEMORY_BASIC_INFORMATION mbi;
    WCHAR arrCh [MAX_PATH];
    PVOID remoteModule;
    if ( (VirtualQuery(largeApi, &mbi, sizeof(mbi)) == sizeof(mbi)) &&
         (mbi.State == MEM_COMMIT) &&
         (mbi.AllocationBase) &&
         (GetModuleFileNameW((HMODULE) mbi.AllocationBase, arrCh, MAX_PATH)) )
    {
      LPWSTR dllName = arrCh;
      for (int i1 = lstrlenW(arrCh) - 1; i1 > 0; i1--)
        if (arrCh[i1] == L'\\')
        {
          dllName = &(arrCh[i1 + 1]);
          break;
        }
      remoteModule = GetRemoteModuleHandle64(hProcess, dllName);
      if (remoteModule)
        largeApi = (PVOID) ((ULONG_PTR) largeApi - (ULONG_PTR) mbi.AllocationBase + (ULONG_PTR) remoteModule);
    }
    return largeApi;
  }

#endif

bool IsProcessDotNet64(HANDLE hProcess)
// find out if the specified 64bit process is a .NET process
{
  #ifdef _WIN64
    ULONG_PTR exeImage;
    IMAGE_NT_HEADERS64 nh;
    ULONG nhOffset;
    SIZE_T read;
    if ( (ReadProcessMemory(hProcess, (LPVOID) ((ULONG_PTR) GetPeb(hProcess) + 0x10), &exeImage, sizeof(exeImage), &read)) &&
         (ReadProcessMemory(hProcess, (LPVOID) (exeImage + NT_HEADERS_OFFSET       ), &nhOffset, sizeof(nhOffset), &read)) &&
         (ReadProcessMemory(hProcess, (LPVOID) (exeImage + nhOffset                ), &nh,       sizeof(nh      ), &read)) &&
         (nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress) &&
         (nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size          )    )
      return true;
  #else
    ULONGLONG exeImage;
    IMAGE_NT_HEADERS64 nh;
    ULONG nhOffset;
    ULONG64 read;
    if ( (ReadProcessMemory64(hProcess, GetPeb64(hProcess) + 0x10,    &exeImage, sizeof(exeImage), &read)) &&
         (ReadProcessMemory64(hProcess, exeImage + NT_HEADERS_OFFSET, &nhOffset, sizeof(nhOffset), &read)) &&
         (ReadProcessMemory64(hProcess, exeImage + nhOffset,          &nh,       sizeof(nh      ), &read)) &&
         (nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress) &&
         (nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size          )    )
      return true;
  #endif
  return false;
}

bool IsProcessDotNet32(HANDLE hProcess)
// find out if the specified 32bit process is a .NET process
{
  ULONG_PTR exeImage;
  IMAGE_NT_HEADERS32 nh;
  ULONG nhOffset;
  SIZE_T read;
  ULONG_PTR peb32;
  #ifdef _WIN64
    peb32 = (ULONG_PTR) GetPeb32(hProcess);
  #else
    peb32 = (ULONG_PTR) GetPeb(hProcess);
  #endif
  if ( (ReadProcessMemory(hProcess, (LPVOID) (peb32 +    0x08             ), &exeImage, sizeof(exeImage), &read)) &&
       (ReadProcessMemory(hProcess, (LPVOID) (exeImage + NT_HEADERS_OFFSET), &nhOffset, sizeof(nhOffset), &read)) &&
       (ReadProcessMemory(hProcess, (LPVOID) (exeImage + nhOffset         ), &nh,       sizeof(nh      ), &read)) &&
       (nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress) &&
       (nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size          )    )
    return true;
  return false;
}

bool IsProcessDotNet(HANDLE hProcess)
{
  if (Is64bitProcess(hProcess))
    return IsProcessDotNet64(hProcess);
  else
    return IsProcessDotNet32(hProcess);
}

SYSTEMS_API DWORD WINAPI GetProcessSessionId(DWORD dwProcessId)
{
  TraceVerbose(L"%S(%d)", __FUNCTION__, dwProcessId);

  ASSERT(dwProcessId != 0);

  DWORD dwSessionId = 0;

  DWORD dwError = GetLastError();

  PFN_PROCESSIDTOSESSIONID pfnProcToSession = (PFN_PROCESSIDTOSESSIONID) KernelProc(CProcessIdToSessionId);
  if (pfnProcToSession != NULL)
  {
    if (pfnProcToSession(dwProcessId, &dwSessionId) == 0)
    {
      if (!pfnNtQueryInformationProcess)
        pfnNtQueryInformationProcess = (PFN_NT_QUERY_INFORMATION_PROCESS) NtProc(CNtQueryInformationProcess);
      if ((pfnNtQueryInformationProcess) && (dwProcessId == GetCurrentProcessId()))
      {
        NTSTATUS status = pfnNtQueryInformationProcess(GetCurrentProcess(), ProcessSessionInformation, &dwSessionId, sizeof(dwSessionId), NULL);
        if (status != 0)
        {
          Trace(L"%S Failure - NtQueryInformationProcess: %X", __FUNCTION__, status);
          dwSessionId = 0;
        }
      }
      else
      {
        Trace(L"%S Failure - ProcessIdToSessionId", __FUNCTION__);
        dwSessionId = 0;
      }
    }
  }
  else
    Trace(L"%S Failure - pfnProcToSession is NULL", __FUNCTION__);

  SetLastError(dwError);
  return dwSessionId;
}

SYSTEMS_API DWORD WINAPI GetCurrentSessionId(void)
{
  TraceVerbose(L"%S(void)", __FUNCTION__);

  return GetProcessSessionId(GetCurrentProcessId());
}

SYSTEMS_API DWORD WINAPI GetInputSessionId(void)
{
  TraceVerbose(L"%S(void)", __FUNCTION__);

  DWORD dwSessionId = 0;

  DWORD dwError = GetLastError();

  PFN_GETINPUTSESSIONID pfnGetInputSession = (PFN_GETINPUTSESSIONID) KernelProc(CGetInputSessionId);
  if (pfnGetInputSession != NULL)
    dwSessionId = pfnGetInputSession();
  else
    Trace(L"%S Failure - pfnGetInputSession is NULL", __FUNCTION__);

  SetLastError(dwError);
  return dwSessionId;
}

SYSTEMS_API BOOL WINAPI AmUsingInputDesktop(void)
{
  TraceVerbose(L"%S(void)", __FUNCTION__);

  BOOL result = false;

  DWORD dwError = GetLastError();

  char inputDesktop[MAX_PATH];
  char currentDesktop[MAX_PATH];

  inputDesktop[0] = '\0';
  currentDesktop[0] = '\0';

  DWORD len = 0;
  HDESK desk = OpenInputDesktop(0, false, READ_CONTROL | GENERIC_READ);
  GetUserObjectInformationA(desk, UOI_NAME, inputDesktop, MAX_PATH, &len);
  CloseDesktop(desk);

  desk = GetThreadDesktop(GetCurrentThreadId());
  GetUserObjectInformationA(desk, UOI_NAME, currentDesktop, MAX_PATH, &len);
  CloseDesktop(desk);

  result = (inputDesktop[0] != '\0') && (strcmp(inputDesktop, currentDesktop) == 0);

  SetLastError(dwError);
  return result;
}

SYSTEMS_API BOOL WINAPI IsSystemProcess(HANDLE hProcess, PSID sid)
{
  BOOL result = true;

  DWORD dwError = GetLastError();

  if ((sid != NULL) && (*((BYTE*) sid) != 0))
  {
     result = ((LPBYTE) sid)[1] <= 1;
  }
  else
  {
    PSID_AND_ATTRIBUTES saa = NULL;
    if (GetProcessSid(hProcess, &saa) && (saa != NULL))
    {
      result = (saa->Sid == NULL) || (((LPBYTE) saa->Sid)[1] <= 1);
      LocalFree((HLOCAL) saa);
    }
    else
    {
      // asking the Sid of a process in another session sometimes doesn't work
      // processes in other sessions are usually not system processes
      DWORD sessionId = GetProcessSessionId(ProcessHandleToId(hProcess));
      if ((sessionId != 0) && (sessionId != GetProcessSessionId(GetCurrentProcessId())))
        result = FALSE;
    }
  }

  SetLastError(dwError);
  return result;
}

SYSTEMS_API BOOL WINAPI AmSystemProcess(void)
{
  TraceVerbose(L"%S(void)", __FUNCTION__);

  return IsSystemProcess(GetCurrentProcess(), NULL);
}

//----------------------------------------------------------------------
// Returns the PID of a process identified by its handle
//----------------------------------------------------------------------
SYSTEMS_API DWORD WINAPI ProcessHandleToId(HANDLE hProcess)
{
  TraceVerbose(L"%S(%X)", __FUNCTION__, hProcess);

  ASSERT(hProcess != NULL);

  if (hProcess == GetCurrentProcess())
    return GetCurrentProcessId();

  DWORD pid = NULL;

  HANDLE hProcessDuplicate;
  if ( (!DuplicateHandle(GetCurrentProcess(), hProcess, GetCurrentProcess(), &hProcessDuplicate, PROCESS_QUERY_INFORMATION,         FALSE, 0)) &&
       (!DuplicateHandle(GetCurrentProcess(), hProcess, GetCurrentProcess(), &hProcessDuplicate, PROCESS_QUERY_LIMITED_INFORMATION, FALSE, 0))    )  // vista specific
    hProcessDuplicate = hProcess;

  PFN_GET_PROCESS_ID pGetProcessId = (PFN_GET_PROCESS_ID) KernelProc(CGetProcessId);
  if (pGetProcessId != NULL)
  {
    pid = pGetProcessId(hProcessDuplicate);
    if (!pid)
      pid = pGetProcessId(hProcess);
  }
  else
  {
    PROCESS_BASIC_INFORMATION pbi;
    memset(&pbi, 0, sizeof(PROCESS_BASIC_INFORMATION));
    PFN_NT_QUERY_INFORMATION_PROCESS pNtQuery = (PFN_NT_QUERY_INFORMATION_PROCESS) NtProc(CNtQueryInformationProcess);
    if (pNtQuery != NULL)
    {
      NTSTATUS status = pNtQuery(hProcessDuplicate, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);
      if (status != 0)
        status = pNtQuery(hProcess, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);
      if (status == 0)
        pid = (DWORD) pbi.UniqueProcessId;
      else
      {
        Trace(L"%S Failure - pfnNtQueryInformationProcess: %X", __FUNCTION__, status);
        SetLastError(status);
      }
    }
    else
    {
      Trace(L"%S Failure - pfnNtQueryInformationProcess is NULL", __FUNCTION__);
      SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    }
  }

  if (hProcessDuplicate != hProcess)
    VERIFY(CloseHandle(hProcessDuplicate));

  return pid;
}

//----------------------------------------------------------------------
// Returns the TID of a thread identified by its handle
//----------------------------------------------------------------------
SYSTEMS_API DWORD WINAPI ThreadHandleToId(HANDLE hThread)
{
  TraceVerbose(L"%S(%X)", __FUNCTION__, hThread);

  ASSERT(hThread != NULL);

  if (hThread == GetCurrentThread())
    return GetCurrentThreadId();

  DWORD tid = NULL;

  HANDLE hThreadDuplicate;
  if ( (!DuplicateHandle(GetCurrentProcess(), hThread, GetCurrentProcess(), &hThreadDuplicate, THREAD_QUERY_INFORMATION,         FALSE, 0)) &&
       (!DuplicateHandle(GetCurrentProcess(), hThread, GetCurrentProcess(), &hThreadDuplicate, THREAD_QUERY_LIMITED_INFORMATION, FALSE, 0))    )  // vista specific
    hThreadDuplicate = hThread;

  PFN_GET_THREAD_ID pGetThreadId = (PFN_GET_THREAD_ID) KernelProc(CGetThreadId);
  if (pGetThreadId != NULL)
  {
    tid = pGetThreadId(hThreadDuplicate);
    if (!tid)
      tid = pGetThreadId(hThread);
  }
  else
  {
    THREAD_BASIC_INFORMATION tbi;
    memset(&tbi, 0, sizeof(tbi));
    PFN_NT_QUERY_INFORMATION_THREAD pNtQuery = (PFN_NT_QUERY_INFORMATION_THREAD) NtProc(CNtQueryInformationThread);
    if (pNtQuery != NULL)
    {
      NTSTATUS status = pNtQuery(hThreadDuplicate, ThreadBasicInformation, &tbi, sizeof(tbi), NULL);
      if (status != 0)
        status = pNtQuery(hThread, ThreadBasicInformation, &tbi, sizeof(tbi), NULL);
      if (status == 0)
        tid = tbi.UniqueThreadId;
      else
      {
        Trace(L"%S Failure - pfnNtQueryInformationThread: %X", __FUNCTION__, status);
        SetLastError(status);
      }
    }
    else
    {
      Trace(L"%S Failure - pfnNtQueryInformationThread is NULL", __FUNCTION__);
      SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    }
  }

  if (hThreadDuplicate != hThread)
    VERIFY(CloseHandle(hThreadDuplicate));

  return tid;
}

SYSTEMS_API HANDLE WINAPI HandleLiveForever(HANDLE handle)
{
  TraceVerbose(L"%S(%X)", __FUNCTION__, handle);

  ASSERT(handle != NULL);

  HANDLE result = NULL;

  HANDLE hSmss = GetSmssProcessHandle();
  if (hSmss != NULL)
  {
    VERIFY(DuplicateHandle(GetCurrentProcess(), handle, hSmss, &result, 0, FALSE, DUPLICATE_SAME_ACCESS));
    VERIFY(CloseHandle(hSmss));
  }
  else
    Trace(L"%S Failure - GetSmssProcessHandle returns NULL", __FUNCTION__);

  return result;
}

//----------------------------------------------------------------------
// Handle to Session Manager SubSystem
// Part of Windows responsible for starting the User session.
//----------------------------------------------------------------------
SYSTEMS_API HANDLE WINAPI GetSmssProcessHandle(void)
{
  TraceVerbose(L"%S(void)", __FUNCTION__);

  static DWORD smssPid = 0;

  HANDLE result = NULL;

  if (smssPid == 0)
  {
    CEnumProcesses enumProcesses(false);
    char buffer[10];
    DecryptStr(CSmss, buffer, 10);
    SString smssName(buffer);
    for (int i = 0; i < enumProcesses.GetCount(); i++)
    {
      if (_wcsicmp(smssName.GetBuffer(), enumProcesses[i].ExeFile) == 0)
      {
        smssPid = enumProcesses[i].Id;
        break;
      }
    }
  }
  if (smssPid != 0)
    result = OpenProcess(PROCESS_DUP_HANDLE, FALSE, smssPid);
  else
    Trace(L"%S Failure - smss.exe PID not found", __FUNCTION__);

  return result;
}

typedef BOOL (WINAPI *CheckTokenMembershipProc)(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember);

SYSTEMS_API BOOL WINAPI IsAdminAndElevated()
{
  BOOL result = false;
  if ((GetVersion() & 0xff) < 6)
  {
    HANDLE smss = GetSmssProcessHandle();
    result = (smss != NULL);
    if (result)
      CloseHandle(smss);
  }
  else
  {
    CheckTokenMembershipProc ctm = (CheckTokenMembershipProc) GetProcAddress(LoadLibraryW(L"advapi32.dll"), "CheckTokenMembership");
    SID_IDENTIFIER_AUTHORITY adminSia = SECURITY_NT_AUTHORITY;
    PSID sid;
    if ((ctm) && (AllocateAndInitializeSid(&adminSia, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &sid)))
    {
      BOOL b1;
      result = (ctm(NULL, sid, &b1)) && (b1);
      FreeSid(sid);
      if (result)
      {
        HANDLE token;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
        {
          DWORD size = 4;
          DWORD elevated;
          result = (GetTokenInformation(token, (TOKEN_INFORMATION_CLASS) 20 /*TokenElevation*/, &elevated, 4, &size)) && (elevated);
          CloseHandle(token);
        }
      }
    }
  }
  return result;
}

//----------------------------------------------------------------------
// Copy (and relocate) any function to a new location in any process.
// If the processHandle is 0, the function is copied to another memory
// location in our own process.
//----------------------------------------------------------------------
SYSTEMS_API LPVOID WINAPI CopyFunction(LPVOID pFunction, HANDLE hProcess, BOOL acceptUnknownTargets, LPVOID *buffer,
                                       CFunctionParse **ppFunctionParse, int extraSpace)
{
  TraceVerbose(L"%S(%p, %X, %d, %p, %p)", __FUNCTION__, pFunction, hProcess, acceptUnknownTargets, buffer, ppFunctionParse);

  ASSERT(pFunction != NULL);

  LPVOID result = NULL;

  CFunctionParse *function = new CFunctionParse(pFunction);

  if (!function->mIsValid)
  {
    Trace(L"%S Failure - Invalid function", __FUNCTION__);
    SetLastError(function->mLastErrorNo);
    return NULL;
  }
  if (!function->mCopy.IsValid)
  {
    Trace(L"%S Failure - Invailid Copy", __FUNCTION__);
    SetLastError(function->mCopy.LastErrorNo);
    return NULL;
  }
  if ((!acceptUnknownTargets) && (function->mUnknownTargets.GetCount() != 0))
  {
    Trace(L"%S Failure - not accepting unknown targets", __FUNCTION__);
    SetLastError(UnknownTargetError);
    return NULL;
  }

  if (hProcess == NULL)
    hProcess = GetCurrentProcess();

  LPVOID p1 = AllocMemEx(function->mCopy.BufferLength + extraSpace, hProcess);
  if (p1 == NULL)
  {
    Trace(L"%S Failure - AllocMemEx", __FUNCTION__);
    return NULL;
  }

  LPVOID buf = (LPVOID) LocalAlloc(LPTR, function->mCopy.BufferLength + extraSpace);
  if (buf == NULL)
  {
    Trace(L"%S Failure - LocalAlloc", __FUNCTION__);
    return NULL;
  }

  LPVOID *pp1 = (LPVOID *) ((ULONG_PTR) buf + function->mCodeLength + extraSpace + sizeof(LPVOID));
  LPVOID *pp2 = (LPVOID *) ((ULONG_PTR) p1  + function->mCodeLength + extraSpace + sizeof(LPVOID));

  LONG_PTR i64 = (LONG_PTR) function->mpCodeBegin - (LONG_PTR) buf;
  LONG_PTR i65 = (LONG_PTR) function->mpCodeBegin - (LONG_PTR) p1;

  memmove(buf, function->mpCodeBegin, function->mCodeLength);

  for (int i = 0; i < function->mFarCalls.GetCount(); i++)
  {
    if ((function->mFarCalls[i].Target != NULL) && ((function->mFarCalls[i].pTarget != NULL) || (function->mFarCalls[i].ppTarget != NULL)))
    {
      if (function->mFarCalls[i].RelTarget)
      {
        LONG_PTR pTarget64 = (LONG_PTR) function->mFarCalls[i].pTarget;
        #ifdef _WIN64
          int TargetDif = (int) ((LONG_PTR) pp2 - (pTarget64 + 4 - i65));
          *(WORD*) pp1 = 0x25ff;
          pp1 = (LPVOID*) ((ULONG_PTR) pp1 + 2);
          pp2 = (LPVOID*) ((ULONG_PTR) pp2 + 2);
          *(DWORD*) pp1 = 0;
          pp1 = (LPVOID*) ((ULONG_PTR) pp1 + 4);
          pp2 = (LPVOID*) ((ULONG_PTR) pp2 + 4);
          *pp1 = function->mFarCalls[i].Target;
          pp1++;
          pp2++;
        #else
          int TargetDif = (int) ((LONG_PTR) function->mFarCalls[i].Target - (pTarget64 + 4 - i65));
        #endif
        *((int *) (pTarget64 - i64)) = TargetDif;
      }
      else
      {
        if (function->mFarCalls[i].ppTarget != NULL)
        {
          *pp1 = function->mFarCalls[i].Target;
          #ifdef _WIN64
            if (function->mFarCalls[i].mIsRipRelative)
            {
              LONG_PTR ppTarget64 = (LONG_PTR) function->mFarCalls[i].ppTarget;
              *(DWORD*) ((LONG_PTR) function->mFarCalls[i].ppTarget - i64) = (DWORD) ((ULONG_PTR) pp2 - (ULONG_PTR) (ppTarget64 + 4 - i65));
            }
            else
          #endif
            *((LPVOID **) ((LONG_PTR) function->mFarCalls[i].ppTarget - i64)) = pp2;
          pp1++;
          pp2++;
        }
        else
        {
          *(LPVOID *) ((LONG_PTR) function->mFarCalls[i].pTarget - i64) = function->mFarCalls[i].Target;
        }
      }
    }
  }

  SIZE_T bytesWritten;
  if (WriteProcessMemory(hProcess, p1, buf, function->mCopy.BufferLength + extraSpace, &bytesWritten))
  {
    if (bytesWritten == (DWORD) function->mCopy.BufferLength + extraSpace)
    {
      result = (LPVOID) ((ULONG_PTR) p1 + ((ULONG_PTR) function->mpEntryPoint - (ULONG_PTR) function->mpCodeBegin));
    }
    else
      Trace(L"%S Failure - WriteProcessMemory, bufferLength: %d != bytesWritten %d", __FUNCTION__, bytesWritten, function->mCopy.BufferLength + extraSpace);
  }
  else
    Trace(L"%S Failure - WriteProcessMemory: %X", __FUNCTION__, GetLastError());

  LocalFree(buf);

  DWORD dummy;
  if (!VirtualProtectEx(hProcess, p1, bytesWritten, PAGE_EXECUTE_READ, &dummy))
    Trace(L"%S Warning - VirtualProtectEx: %X", __FUNCTION__, GetLastError());

  if (buffer != NULL)
    *buffer = p1;

  if (ppFunctionParse != NULL)
    *ppFunctionParse = function;
  else
    delete function;

  return result;
}

//----------------------------------------------------------------------
// Create Remote Thread Ex
//----------------------------------------------------------------------
#pragma pack(1)
typedef struct tagExitThreadFrame
// push   dword ptr [esp+4]
// mov    eax, threadProc
// call   eax
// push   eax
// push   eax
// push   MEM_RELEASE
// push   0
// push   frameAddr
// push   @ExitThread
// mov    eax, @VirtualFree
// jmp    eax
{
  DWORD pushParam;                     // 04 24 74 ff
                                       // ff = push ea (ModRm, SIB, displacement)
                                       // 74 = ModRm   01 110 100 == [-][-] w/SIB + 8bit displacement
                                       // 24 = SIB      00 100 100 no scaled index, esp
                                       // 04   = displacement  so push [esp+4]
  OPCODE_POINTER32 movEaxThreadProc;   // b8 xxxxxxxx
  WORD callThreadProc;                 // d0ff : ff=call ea, d0 = 11 010 000 means ea is eax
  BYTE pushResult;                     // 50
  BYTE pushDummy;                      // 50
  OPCODE_DWORD pushMemRelease;         // 68 xxxxxxxx
  WORD pushSize;                       // 006a 6a is push 8bits, so push 0
  OPCODE_POINTER32 pushAddr;           // 68 xxxxxxxx
  OPCODE_POINTER32 pushExitThread;     // 68 xxxxxxxx
  OPCODE_POINTER32 movEaxVirtualFree;  // b8 xxxxxxxx
  WORD jmpVirtualFree;                 // e0ff : ff = jmp ea, e0 = 11 100 000 means ea is eax
} EXIT_THREAD_FRAME;
#pragma pack()

const EXIT_THREAD_FRAME CExitThreadFrame =
{
  0x042474ff,
  {0xb8, 0x00000000},
  0xd0ff,
  0x50,
  0x50,
  {0x68, MEM_RELEASE},
  0x006a,
  {0x68, 0x00000000},
  {0x68, 0x00000000},
  {0xb8, 0x00000000},
  0xe0ff
};

BOOL GetThreadSecurityAttributes(SECURITY_ATTRIBUTES *pSecurityAttributes)
{
  TraceVerbose(L"%S(%p)", __FUNCTION__, pSecurityAttributes);

  ASSERT(pSecurityAttributes);

  BOOL result = FALSE;

  HANDLE hTarget;
  if (DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hTarget, 0, FALSE, DUPLICATE_SAME_ACCESS))
  {
    DWORD lengthNeeded = 0;
    GetKernelObjectSecurity(hTarget, 4, NULL, 0, &lengthNeeded);
    if (lengthNeeded > 0)
    {
      SECURITY_DESCRIPTOR * pSecurityDescriptor = (SECURITY_DESCRIPTOR *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lengthNeeded * 2);
      if (pSecurityDescriptor != NULL)
      {
        if (GetKernelObjectSecurity(hTarget, 4, pSecurityDescriptor, lengthNeeded*2, &lengthNeeded))
        {
          pSecurityAttributes->nLength = sizeof(SECURITY_ATTRIBUTES);
          pSecurityAttributes->lpSecurityDescriptor = pSecurityDescriptor;
          pSecurityAttributes->bInheritHandle = FALSE;
          result = TRUE;
        }
        else
          Trace(L"%S Failure - GetKernelObjectSecurity: %X", __FUNCTION__, GetLastError());
      }
      else
        Trace(L"%S Failure - HeapAlloc", __FUNCTION__);
    }
    else
      Trace(L"%S Failure - GetKernelObjectSecurity(NULL): %X", __FUNCTION__, GetLastError());

    VERIFY(CloseHandle(hTarget));
  }
  else
    Trace(L"%S Failure - DuplicateHandle: %X", __FUNCTION__, GetLastError());

  return result;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

typedef ULONG (WINAPI *PFN_CsrClientCallServer)(PVOID, PVOID, ULONG, ULONG);

LPVOID PCrtHookBuf     = NULL;  // trampoline
int   *PCrtIntTarget   = NULL;  // patch address (for "int" size patch)
int    PCrtOldInt      = 0;     // original patch content (for "int" size patch)
PVOID *PCrtPVoidTarget = NULL;  // patch address (for "pvoid" size patch)
LPVOID PCrtOldPVoid    = NULL;  // original patch content (for "pvoid" size patch)

PFN_CsrClientCallServer CsrClientCallServerNext = NULL;

ULONG WINAPI CsrClientCallServerCallback(PVOID Message, PVOID Reply, ULONG Opcode, ULONG Size)
// hooks CreateRemoteThread's call to CsrClientCallServer
// the call to CsrClientCallServer is passed on unchanged
// however, CsrClientCallServer is forced to always report success
{
  ULONG error = CsrClientCallServerNext(Message, Reply, Opcode, Size);
  if (error)
    for (int i = 7; i < 15; i++)
      if (((ULONG*) Message)[i] == error)
      {
        ((ULONG*) Message)[i] = 0;
        break;
      }
  return 0;
}

bool PatchCreateRemoteThread(void)
// patches CreateRemoteThread to not fail for other TS client sessions
// this patching works in most cases, except for win32 processes in a 64bit OS
{
  bool result = FALSE;

  if (CsrClientCallServerNext)  // already patched?
    return TRUE;

  CsrClientCallServerNext = (PFN_CsrClientCallServer) NtProc(CCsrClientCallServer);
  if (CsrClientCallServerNext)
  {
    LPVOID crt = KernelProc(CCreateRemoteThreadEx);
    if (!crt)
      crt = KernelProc(CCreateRemoteThread);
    CFunctionParse func(crt);

    for (int i = 0; i < func.mFarCalls.GetCount(); i++)
      if ( ((func.mFarCalls[i].pTarget != NULL) || (func.mFarCalls[i].ppTarget != NULL)) &&
           (func.mFarCalls[i].Target == CsrClientCallServerNext) )
      {
        if (func.mFarCalls[i].RelTarget)
        {
          DWORD op;
          #ifdef _WIN64
            LPVOID PCrtHookBuf = VirtualAlloc2(6 + sizeof(LPVOID), func.mFarCalls[i].CodeAddress2);
            LPVOID *pp1 = (LPVOID *) PCrtHookBuf;
            *(WORD*) pp1 = 0x25ff;
            pp1 = (LPVOID*)((ULONG_PTR) pp1 + 2);
            *(DWORD*) pp1 = 0;
            pp1 = (LPVOID*)((ULONG_PTR) pp1 + 4);
            *pp1 = func.mFarCalls[i].Target;
            int TargetDif = (int) ((LONG_PTR) PCrtHookBuf - (LONG_PTR) func.mFarCalls[i].pTarget - 4);
            VirtualProtect(PCrtHookBuf, 6 + sizeof(LPVOID), PAGE_EXECUTE_READ, &op);
          #else
            int TargetDif = (int) ((LONG_PTR) func.mFarCalls[i].Target - (LONG_PTR) func.mFarCalls[i].pTarget - 4);
          #endif
          if (VirtualProtect(func.mFarCalls[i].pTarget, 4, PAGE_EXECUTE_READWRITE, &op))
          {
            PCrtIntTarget = (int *) func.mFarCalls[i].pTarget;
            PCrtOldInt = *PCrtIntTarget;
            *PCrtIntTarget = TargetDif;
            VirtualProtect(func.mFarCalls[i].pTarget, 4, op, &op);
          }
        }
        else
          if (func.mFarCalls[i].ppTarget != NULL)
          {
            #ifdef _WIN64
              LPVOID PCrtHookBuf = VirtualAlloc2(sizeof(LPVOID), func.mFarCalls[i].CodeAddress2);
            #else
              LPVOID PCrtHookBuf = VirtualAlloc2(sizeof(LPVOID));
            #endif
            *((LPVOID *) PCrtHookBuf) = CsrClientCallServerCallback;
            #ifdef _WIN64
              if (func.mFarCalls[i].mIsRipRelative)
              {
                DWORD op;
                if (VirtualProtect(func.mFarCalls[i].ppTarget, 4, PAGE_EXECUTE_READWRITE, &op))
                {
                  PCrtIntTarget = (int *) func.mFarCalls[i].ppTarget;
                  PCrtOldInt = *PCrtIntTarget;
                  *PCrtIntTarget = (int) ((DWORD) ((ULONG_PTR) PCrtHookBuf - (ULONG_PTR) func.mFarCalls[i].ppTarget - 4));
                  VirtualProtect(func.mFarCalls[i].ppTarget, 4, op, &op);
                }
              }
              else
            #endif
              {
                DWORD op;
                if (VirtualProtect(func.mFarCalls[i].ppTarget, sizeof(LPVOID), PAGE_EXECUTE_READWRITE, &op))
                {
                  PCrtPVoidTarget = (LPVOID *) func.mFarCalls[i].ppTarget;
                  PCrtOldPVoid = *PCrtPVoidTarget;
                  *PCrtPVoidTarget = PCrtHookBuf;
                  VirtualProtect(func.mFarCalls[i].ppTarget, sizeof(LPVOID), op, &op);
                }
              }
            DWORD op;
            VirtualProtect(PCrtHookBuf, sizeof(LPVOID), PAGE_EXECUTE_READ, &op);
          }
          else
          {
            DWORD op;
            if (VirtualProtect(func.mFarCalls[i].pTarget, sizeof(LPVOID), PAGE_EXECUTE_READWRITE, &op))
            {
              PCrtPVoidTarget = (LPVOID *) func.mFarCalls[i].pTarget;
              PCrtOldPVoid = *PCrtPVoidTarget;
              *PCrtPVoidTarget = func.mFarCalls[i].Target;
              VirtualProtect(func.mFarCalls[i].pTarget, sizeof(LPVOID), op, &op);
            }
          }
        result = ((PCrtPVoidTarget) || (PCrtIntTarget));
        break;
      }
    if (!result)
      CsrClientCallServerNext = NULL;
  }

  return result;
}

void UnpatchCreateRemoteThread(void)
{
  DWORD op;
  if ((PCrtIntTarget) && (VirtualProtect(PCrtIntTarget, sizeof(int), PAGE_EXECUTE_READWRITE, &op)))
  {
    *PCrtIntTarget = PCrtOldInt;
    VirtualProtect(PCrtIntTarget, sizeof(int), op, &op);
    PCrtIntTarget = NULL;
  }
  if ((PCrtPVoidTarget) && (VirtualProtect(PCrtPVoidTarget, sizeof(PVOID), PAGE_EXECUTE_READWRITE, &op)))
  {
    *PCrtPVoidTarget = PCrtOldPVoid;
    VirtualProtect(PCrtPVoidTarget, sizeof(PVOID), op, &op);
    PCrtPVoidTarget = NULL;
  }
}

HANDLE InternalCreateRemoteThread(HANDLE hProcess,
                                  SECURITY_ATTRIBUTES *pThreadAttributes,
                                  DWORD  stackSize,
                                  LPVOID pStartAddress,
                                  LPVOID pParameters,
                                  DWORD  creationFlags,
                                  DWORD *pThreadId,
                                  bool   dontUseRtlCreateUserThread)
{
  TraceVerbose(L"%S(%X, %p, %d, %p, %p, %d, %p)", __FUNCTION__, hProcess, pThreadAttributes, stackSize, pStartAddress, pParameters, creationFlags, pThreadId);

  ASSERT(hProcess != NULL);

  HANDLE hThread = NULL;

  SECURITY_ATTRIBUTES securityAttributes;
  memset(&securityAttributes, 0, sizeof(SECURITY_ATTRIBUTES));

  if (pThreadAttributes == NULL)
  {
    if (GetThreadSecurityAttributes(&securityAttributes))
      pThreadAttributes = &securityAttributes;
  }

  PatchCreateRemoteThread();

  DebugTrace((L"%S - Trying CreateRemoteThread", __FUNCTION__));
  hThread = CreateRemoteThread(hProcess, pThreadAttributes, stackSize, (LPTHREAD_START_ROUTINE) pStartAddress, pParameters, creationFlags, pThreadId);

  if ((hThread == NULL) && (!dontUseRtlCreateUserThread))
  {
    // CreateRemoteThread failed; this can happen in two situations:
    // (1) for processes in other sessions
    // (2) for CSRSS processes in Windows 8
    // In both those situations RtlCreateUserThread works.
    DebugTrace((L"%S - Trying RtlCreateUserThread", __FUNCTION__));
    // CreateRemoteThread won't work in this situation
    //  but RtlCreateUserThread might succeed.  
    PFN_RTL_CREATE_USER_THREAD pRCUT = (PFN_RTL_CREATE_USER_THREAD) NtProc(CRtlCreateUserThread);
    if (pRCUT != NULL)
    {
      SECURITY_DESCRIPTOR *pSecurityDescriptor = NULL;
      if (pThreadAttributes != NULL)
        pSecurityDescriptor = (SECURITY_DESCRIPTOR *) pThreadAttributes->lpSecurityDescriptor;
      SIZE_T stackReserved = 0;
      SIZE_T stackCommit = 0;
      if (stackSize != 0)
      {
        if ((creationFlags & STACK_SIZE_PARAM_IS_A_RESERVATION) != 0)
          stackReserved = stackSize;
        else
          stackCommit = stackSize;
      }

      #ifdef _WIN64
        if (Is64bitProcess(hProcess))
        {
          DWORD_PTR client[2];
          NTSTATUS status;
          LPVOID baseThreadStart = KernelProc(CBaseThreadStart);
          if (baseThreadStart)
          {
            // in older x64 OSs we have to use BaseThreadStart, see below
            status = pRCUT(hProcess, pSecurityDescriptor, TRUE, 0, &stackReserved, &stackCommit, baseThreadStart, pParameters, &hThread, (__int64 *) &client);
            if (status >= 0)
            {
              CONTEXT context;
              context.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
              if (GetThreadContext(hThread, &context))
              {
                // we have to end threads created by RtlCreateUserThread with an ExitThread call
                // in 64bit Windows we're using kernel32.BaseThreadStart for this purpose
                // this is a wrapper function which calls a thread function and afterwards ExitThread
                // BaseThreadStart is internally used by Create(Remote)Thread for the same purpose
                // fortunately BaseThreadStart is exported in 64bit Windows (unlike 32bit Windows)
                // BaseThreadStart wants two parameters, namely the thread function and the parameter
                context.Rsp -= 0x30;
                context.Rcx = (ULONG_PTR) pStartAddress;
                context.Rdx = (ULONG_PTR) pParameters;
                if (SetThreadContext(hThread, &context))
                {
                  if ((creationFlags & CREATE_SUSPENDED) == NULL)
                    ResumeThread(hThread);
                }
                else
                {
                  Trace(L"%S Failure - SetThreadContext: %X", __FUNCTION__, GetLastError());
                  TerminateThread(hThread, 0);
                  CloseHandle(hThread);
                  hThread = NULL;
                }
              }
              else
              {
                Trace(L"%S Failure - GetThreadContext: %X", __FUNCTION__, GetLastError());
                TerminateThread(hThread, 0);
                CloseHandle(hThread);
                hThread = NULL;
              }
              *pThreadId = (DWORD) client[1];
            }
            else
              Trace(L"%S Failure - pRCUT: %X", __FUNCTION__, status);
          }
          else
          {
            // In newer x64 OSs BaseThreadStart does not exist, anymore, or at
            // at least it's not exported. On a positive note, it seems we don't
            // have to end threads with a manual ExitThread call, anymore, either.
            // So the code is much simpler.
            status = pRCUT(hProcess, pSecurityDescriptor, FALSE, 0, &stackReserved, &stackCommit, pStartAddress, pParameters, &hThread, (__int64 *) &client);
            if (status >= 0)
              *pThreadId = (DWORD) client[1];
            else
              Trace(L"%S Failure - pRCUT: %X", __FUNCTION__, status);
          }
        }
        else
      #endif
      {
        EXIT_THREAD_FRAME exitThreadFrame;
        #ifdef _WIN64
          EXIT_THREAD_FRAME *pExitThreadFrame = (EXIT_THREAD_FRAME *) AllocMemEx(sizeof(EXIT_THREAD_FRAME), hProcess, (LPVOID) GetMadCHookOption(X86_ALLOCATION_ADDRESS));
        #else
          EXIT_THREAD_FRAME *pExitThreadFrame = (EXIT_THREAD_FRAME *) AllocMemEx(sizeof(EXIT_THREAD_FRAME), hProcess);
        #endif
        if (pExitThreadFrame)
        {
          // unfortunately RtlCreateUserThread threads must end with ExitThread
          // so we write a little frame to the target process
          // this frame internally calls the original thread procedure
          // afterwards it frees itself and then calls ExitThread
          // CAUTION: if the original thread procedure calls ExitThread then we get a memory leak
          exitThreadFrame = CExitThreadFrame;
          #if _WIN64
            exitThreadFrame.movEaxThreadProc.pointer = (PVOID32) (ULONG) (ULONGLONG) pStartAddress;
            exitThreadFrame.pushAddr.pointer = (PVOID32) (ULONG) (ULONGLONG) pExitThreadFrame;
            if (Init32bitKernelAPIs(hProcess))
            {
              exitThreadFrame.pushExitThread.pointer = (PVOID32) (ULONG) (ULONGLONG) pfn32bitKernelAPIs[14];
              exitThreadFrame.movEaxVirtualFree.pointer = (PVOID32) (ULONG) (ULONGLONG) pfn32bitKernelAPIs[1];
            }
            else
            {
              FreeMemEx(pExitThreadFrame, hProcess);
              pExitThreadFrame = NULL;
              Trace(L"%S Failure - Init32bitKernelAPIs for pExitThreadFrame", __FUNCTION__);
            }
          #else
            exitThreadFrame.movEaxThreadProc.pointer = pStartAddress;
            exitThreadFrame.pushAddr.pointer = pExitThreadFrame;
            exitThreadFrame.pushExitThread.pointer = KernelProc(CExitThread, true);
            exitThreadFrame.movEaxVirtualFree.pointer = KernelProc(CVirtualFree, true);
          #endif
        }
        else
          Trace(L"%S Failure - AllocMemEx for pExitThreadFrame", __FUNCTION__);

        if (pExitThreadFrame)
        {
          SIZE_T bytesWritten;

          if (WriteProcessMemory(hProcess, pExitThreadFrame, &exitThreadFrame, sizeof(EXIT_THREAD_FRAME), &bytesWritten))
          {
            DWORD_PTR client[2];
            NTSTATUS status;
            status = pRCUT(hProcess, pSecurityDescriptor, creationFlags & CREATE_SUSPENDED, 0, &stackReserved, &stackCommit, pExitThreadFrame, pParameters, &hThread, (__int64 *) &client);
            if (status >= 0)
            {
              *pThreadId = (DWORD) client[1];
              DWORD op;
              VirtualProtectEx(hProcess, pExitThreadFrame, sizeof(EXIT_THREAD_FRAME), PAGE_EXECUTE_READ, &op);
            }
            else
            {
              Trace(L"%S Failure - pRCUT: %X", __FUNCTION__, status);
              VERIFY(FreeMemEx(pExitThreadFrame, hProcess));
            }
          }
          else
            Trace(L"%S Failure - WriteProcessMemory: %X", __FUNCTION__, GetLastError());
        }
      }
    } // end if pRCUT != NULL
    else
      Trace(L"%S Failure - pRCUT is NULL", __FUNCTION__);
  }

  if (securityAttributes.lpSecurityDescriptor != NULL)
  {
    // If we allocated it, we must free it
    VERIFY(HeapFree(GetProcessHeap(), 0, securityAttributes.lpSecurityDescriptor));
  }
  return hThread;
}

SYSTEMS_API HANDLE WINAPI madCreateRemoteThread(HANDLE hProcess,
                                                SECURITY_ATTRIBUTES *pThreadAttributes,
                                                DWORD  stackSize,
                                                LPVOID pStartAddress,
                                                LPVOID pParameters,
                                                DWORD  creationFlags,
                                                DWORD *pThreadId)
{
  return InternalCreateRemoteThread(hProcess, pThreadAttributes, stackSize, pStartAddress, pParameters, creationFlags, pThreadId, false);
}

//----------------------------------------------------------------------
// Remote Execute
//----------------------------------------------------------------------
SYSTEMS_API BOOL WINAPI RemoteExecute(HANDLE hProcess,
                                      PFN_REMOTE_EXECUTE_FUNCTION pFunction,
                                      DWORD *pFunctionResult,
                                      LPVOID pParameters,
                                      DWORD size)
{
  TraceVerbose(L"%S(%X, %p, %p, %p, %d)", __FUNCTION__, hProcess, pFunction, pFunctionResult, pParameters, size);

  ASSERT(pFunction != NULL);
  ASSERT(pFunctionResult != NULL);

  BOOL result = false;
  LPVOID pBuffer;
  LPVOID pProc = CopyFunction(pFunction, hProcess, false, &pBuffer);
  if (pProc != NULL)
  {
    __try
    {
      LPVOID pMem = AllocMemEx(size, hProcess);
      DWORD op;
      VirtualProtectEx(hProcess, pMem, size, PAGE_READWRITE, &op);
      __try
      {
        SIZE_T bytesWritten;
        if ((size == 0) || WriteProcessMemory(hProcess, pMem, pParameters, size, &bytesWritten))
        {
          LPVOID p;
          if (pMem != NULL)
            p = pMem;
          else
            p = pParameters;
          DWORD threadId;
          HANDLE hThread = madCreateRemoteThread(hProcess, NULL, 0, pProc, p, 0, &threadId);
          if (hThread != NULL)
          {
            __try
            {
              if (WaitForSingleObject(hThread, INFINITE) == WAIT_FAILED)
                Trace(L"%S Failure - WaitForSingleObject: %X", __FUNCTION__, GetLastError());
              VERIFY(GetExitCodeThread(hThread, pFunctionResult));
            }
            __finally
            {
              VERIFY(CloseHandle(hThread));
            }
            SIZE_T bytesRead;
            if (size == 0)
              result = true;
            else
            {
              if (!IsBadWritePtr2(pParameters, size))
              {
                if (ReadProcessMemory(hProcess, pMem, pParameters, size, &bytesRead))
                  result = true;
                else
                  Trace(L"%S Failure - ReadProcessMemory: %X", __FUNCTION__, GetLastError());
              }
              else
              {
                // this might seem strange, but theoretically it could make sense
                // to call RemoteExecute with a pointer to a constant data block
                // in this case the data block would be transported to the target process
                // but not read back, because it's a const block, and thus IsBadWritePtr returns true
                // the documentation does not explicitly allow this, but I think it doesn't harm to support it
                result = true;
              }
            }
          }
          else
            Trace(L"%S Failure - madCreateRemoteThread: %X", __FUNCTION__, GetLastError());
        }
        else
          Trace(L"%S Failure - WriteProcessMemory: %X", __FUNCTION__, GetLastError());
      }
      __finally
      {
        VERIFY(FreeMemEx(pMem, hProcess));
      }
    }
    __finally
    {
      VERIFY(FreeMemEx(pBuffer, hProcess));
    }
  }
  else
    Trace(L"%S Failure - CopyFunction", __FUNCTION__);

  return result;
}

const OBJECT_ATTRIBUTES CNtObjAttr = {sizeof(OBJECT_ATTRIBUTES), 0, 0, 0, 0, 0};

void GetOtherThreadHandles(CCollection<HANDLE>& threads)
{
  TraceVerbose(L"%S()", __FUNCTION__);

  threads.RemoveAll();

  PFN_NT_QUERY_SYSTEM_INFORMATION pfnNtQuerySi = (PFN_NT_QUERY_SYSTEM_INFORMATION) NtProc(CNtQuerySystemInformation);
  PFN_NT_OPEN_THREAD pfnNtOpenThread = (PFN_NT_OPEN_THREAD) NtProc(CNtOpenThread);

  if ((pfnNtQuerySi != NULL) && (pfnNtOpenThread != NULL))
  {
    ULONG length = 0;
    pfnNtQuerySi(SystemProcessInformation, NULL, 0, &length);
    HLOCAL p = NULL;
    NTSTATUS status = -1;
    __try
    {
      if (length == 0)
      {
        DebugTrace((L"%S - SystemProcessInformation length failed, determining", __FUNCTION__));
        length = 0x10000;
        while (length < 0x400000)
        {
          length = length * 2;
          if (p != NULL)
            LocalFree(p);
          p = LocalAlloc(LPTR, length);
          status = pfnNtQuerySi(SystemProcessInformation, p, length, NULL);
          if (status == 0)
            break;
        }
      }
      else
      {
        length = length * 2;
        p = LocalAlloc(LPTR, length);
        status = pfnNtQuerySi(SystemProcessInformation, p, length, NULL);
      }

      if (status == 0)
      {
        SYSTEM_PROCESS_INFORMATION *pSpi = (SYSTEM_PROCESS_INFORMATION *) p;
        while (TRUE)
        {
          if (pSpi->Process.UniqueProcessId == GetCurrentProcessId())
          {
            for (DWORD i = 0; i < pSpi->Process.ThreadCount; i++)
            {
              HANDLE tid;
              if (((BYTE) GetVersion()) > 4)
                   tid = pSpi->Process_NT5.Threads[i].ClientId.UniqueThread;
              else tid = pSpi->Process_NT4.Threads[i].ClientId.UniqueThread;
#pragma warning(disable: 4312)
              if (tid != (HANDLE) GetCurrentThreadId())
#pragma warning(default: 4312)
              {
                OBJECT_ATTRIBUTES oa = CNtObjAttr;
                CLIENT_ID cid;
                cid.UniqueProcess = NULL;
                cid.UniqueThread = tid;
                HANDLE hThread;
                if (pfnNtOpenThread(&hThread, THREAD_ALL_ACCESS, &oa, &cid) == 0)
                {
                  threads.Add(hThread);
                }
              }
            }
          }
          if (pSpi->Process.Next == 0)
            break;
          pSpi = (SYSTEM_PROCESS_INFORMATION *) ((ULONG_PTR) pSpi + pSpi->Process.Next);
        }
      }
      else
        Trace(L"%S Failure - Query SystemProcessInformation: %X", __FUNCTION__, status);
    }
    __finally
    {
      if (LocalFree(p) != NULL)
        Trace(L"%S Failure - LocalFree: %X", __FUNCTION__, GetLastError());
    }
  }
  else
    Trace(L"%S Failure - Unable to query procs", __FUNCTION__);
}

HANDLE OpenFirstThread(DWORD pid)
{
  TraceVerbose(L"%S()", __FUNCTION__);

  PFN_NT_QUERY_SYSTEM_INFORMATION pfnNtQuerySi = (PFN_NT_QUERY_SYSTEM_INFORMATION) NtProc(CNtQuerySystemInformation);
  PFN_NT_OPEN_THREAD pfnNtOpenThread = (PFN_NT_OPEN_THREAD) NtProc(CNtOpenThread);

  HANDLE result = NULL;

  if ((pfnNtQuerySi != NULL) && (pfnNtOpenThread != NULL))
  {
    ULONG length = 0;
    pfnNtQuerySi(SystemProcessInformation, NULL, 0, &length);
    HLOCAL p = NULL;
    NTSTATUS status = -1;
    __try
    {
      if (length == 0)
      {
        DebugTrace((L"%S - SystemProcessInformation length failed, determining", __FUNCTION__));
        length = 0x10000;
        while (length < 0x400000)
        {
          length = length * 2;
          if (p != NULL)
            LocalFree(p);
          p = LocalAlloc(LPTR, length);
          status = pfnNtQuerySi(SystemProcessInformation, p, length, NULL);
          if (status == 0)
            break;
        }
      }
      else
      {
        length = length * 2;
        p = LocalAlloc(LPTR, length);
        status = pfnNtQuerySi(SystemProcessInformation, p, length, NULL);
      }

      if (status == 0)
      {
        SYSTEM_PROCESS_INFORMATION *pSpi = (SYSTEM_PROCESS_INFORMATION *) p;
        while (TRUE)
        {
          if (pSpi->Process.UniqueProcessId == pid)
          {
            if (pSpi->Process.ThreadCount > 0)
            {
              HANDLE tid;
              if (((BYTE) GetVersion()) > 4)
                   tid = pSpi->Process_NT5.Threads[0].ClientId.UniqueThread;
              else tid = pSpi->Process_NT4.Threads[0].ClientId.UniqueThread;
              OBJECT_ATTRIBUTES oa = CNtObjAttr;
              CLIENT_ID cid;
              cid.UniqueProcess = NULL;
              cid.UniqueThread = tid;
              HANDLE hThread;
              if (pfnNtOpenThread(&hThread, THREAD_ALL_ACCESS, &oa, &cid) == 0)
                result = hThread;
            }
            break;
          }
          if (pSpi->Process.Next == 0)
            break;
          pSpi = (SYSTEM_PROCESS_INFORMATION *) ((ULONG_PTR) pSpi + pSpi->Process.Next);
        }
      }
      else
        Trace(L"%S Failure - Query SystemProcessInformation: %X", __FUNCTION__, status);
    }
    __finally
    {
      if (LocalFree(p) != NULL)
        Trace(L"%S Failure - LocalFree: %X", __FUNCTION__, GetLastError());
    }
  }
  else
    Trace(L"%S Failure - Unable to query procs", __FUNCTION__);

  return result;
}

SYSTEMS_API BOOL WINAPI GetProcessSid(HANDLE hProcess, SID_AND_ATTRIBUTES **ppSidAndAttributes)
{
  TraceVerbose(L"%S(%X, %p)", __FUNCTION__, hProcess, ppSidAndAttributes);

  ASSERT(hProcess != NULL);

  BOOL result = false;

  HANDLE hToken;
  if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
  {
    DWORD size = 0;
    VERIFY((!GetTokenInformation(hToken, TokenUser, NULL, 0, &size) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)));
    *ppSidAndAttributes = (SID_AND_ATTRIBUTES *) LocalAlloc(LPTR, size * 2);
    if (*ppSidAndAttributes != NULL)
    {
      if (GetTokenInformation(hToken, TokenUser, *ppSidAndAttributes, size * 2, &size))
      {
        result = true;
      }
      else
      {
        Trace(L"%S Failure - GetTokenInformation: %X", __FUNCTION__, GetLastError());
        if (LocalFree((HLOCAL) *ppSidAndAttributes) != NULL)
          Trace(L"%S Failure - LocalFree: %X", __FUNCTION__, GetLastError());
      }
    }
    else
      Trace(L"%S Failure - LocalAlloc: %X", __FUNCTION__, GetLastError());

    VERIFY(CloseHandle(hToken));
  }
  else
    Trace(L"%S Failure - OpenProcessToken: %X", __FUNCTION__, GetLastError());

  return result;
}

static int Is64bitOSFlag = -1;
static BOOL IsWineFlag = false;

SYSTEMS_API BOOL WINAPI IsWine(void)
{
  Is64bitOS();
  return IsWineFlag;
}

SYSTEMS_API BOOL WINAPI Is64bitOS(void)
{
  if (Is64bitOSFlag == -1)
  {
    PFN_GET_NATIVE_SYSTEM_INFO pGetNativeSystemInfo = (PFN_GET_NATIVE_SYSTEM_INFO) KernelProc(CGetNativeSystemInfo);
    if (pGetNativeSystemInfo != NULL)
    {
      SYSTEM_INFO si;
      ZeroMemory(&si, sizeof(si));
      pGetNativeSystemInfo(&si);
      Is64bitOSFlag = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
    }
    else
      Is64bitOSFlag = 0;
    IsWineFlag = NtProc(CWineGetVersion) != NULL;
  }
  return Is64bitOSFlag == 1;
}

static BOOL IsWow64Ready = false;

SYSTEMS_API BOOL WINAPI Is64bitProcess(HANDLE hProcess)
{
  if (!Is64bitOS())
    return false;

  if (!IsWow64Ready)
  {
    pfnIsWow64Process = (PFN_IS_WOW64_PROCESS) KernelProc(CIsWow64Process);
    IsWow64Ready = true;
  }

  BOOL b1;
  return (pfnIsWow64Process != NULL) && ((!pfnIsWow64Process(hProcess, &b1)) || (!b1));
}

SYSTEMS_API BOOL WINAPI IsBadReadPtr2(LPVOID src, SIZE_T count)
{
  MEMORY_BASIC_INFORMATION mbi;
  return (VirtualQuery(src, &mbi, sizeof(mbi)) != sizeof(mbi)) || (mbi.State != MEM_COMMIT) || ((mbi.Protect & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY)) == 0) || ((mbi.Protect & PAGE_GUARD) != 0) ||
         (((ULONG_PTR) src) + count > ((ULONG_PTR) mbi.BaseAddress) + mbi.RegionSize);
}

SYSTEMS_API BOOL WINAPI IsBadWritePtr2(LPVOID dst, SIZE_T count)
{
  MEMORY_BASIC_INFORMATION mbi;
  return (VirtualQuery(dst, &mbi, sizeof(mbi)) != sizeof(mbi)) || (mbi.State != MEM_COMMIT) || ((mbi.Protect & (PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_READWRITE | PAGE_WRITECOPY)) == 0) || ((mbi.Protect & PAGE_GUARD) != 0) ||
         (((ULONG_PTR) dst) + count > ((ULONG_PTR) mbi.BaseAddress) + mbi.RegionSize);
}
