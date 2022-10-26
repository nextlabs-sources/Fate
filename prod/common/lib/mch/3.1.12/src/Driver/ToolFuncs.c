// ***************************************************************
//  ToolFuncs.c               version: 1.0.8  ·  date: 2016-03-16
//  -------------------------------------------------------------
//  collection of tool functions
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2016 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2016-03-16 1.0.8 (1) fixed conflict where alloc collided with kernel32.dll
//                  (2) fixed: DriverVerifier made driver not load (win8 x64)
//                  (3) fixed: some undocumented APIs had incorrect types
// 2014-05-05 1.0.7 (1) IsWin8OrNewer added
//                  (2) Set/RemoveLoadImageNotifyRoutine added
// 2013-12-03 1.0.6 fixed: injection in Vista x64 sometimes failed
// 2013-10-01 1.0.5 added support for Windows 8.1
// 2013-02-13 1.0.4 (1) fixed: injection failure with MSVC++ 2012 hook dlls
//                  (2) added ProcessIdToSessionId
//                  (3) added CriticalSection helper functions
// 2012-07-05 1.0.3 fixed Windows 7 Driver Verifier complaint
// 2012-03-23 1.0.2 (1) added support for Windows 8
//                  (2) GetModuleHandle -> GetSystemModuleHandle
//                  (3) added AttachTo/DetachMemoryContext
//                  (4) added FindNtdll
//                  (5) fixed x64 crash in Get(System)ModuleHandle
// 2010-05-25 1.0.1 (1) added native API signatures used by XPSP0 and XPSP1
//                  (2) GetModuleHandle is no longer case sensitive (w2k fix)
//                  (3) GetRemoteProcAddress/AddressOfEntryPoint added
//                  (4) IsRemoteModule32bit added
//                  (5) IsXpOrNewer and IsVistaOrNewer added
// 2010-01-10 1.0.0 initial release

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif
#include <ntimage.h>
#include <Ntstrsafe.h>

#include "ToolFuncs.h"
#include "RipeMD.h"

// ********************************************************************
// exported APIs by ntoskrnl.exe

// undocumented API
// used by this driver to enumerate system dlls
NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation(
    ULONG SystemInformationClass_,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength);

// undocumented API
// get various bits of information about a specific process
NTSYSAPI NTSTATUS NTAPI ZwQueryInformationProcess(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength);

#ifdef _WIN64
  // undocumented API
  // used by this driver to get thread id and process id from a thread handle
  // in x64 this API is always exported by ntoskrnl.exe
  // in NT4 this API is not exported, unfortunately
  NTSYSAPI NTSTATUS NTAPI ZwQueryInformationThread(
      HANDLE ThreadHandle,
      THREADINFOCLASS ThreadInformationClass,
      PVOID ThreadInformation,
      ULONG ThreadInformationLength,
      PULONG ReturnLength);
#endif

// defined in ntifs.h
// used by this driver to find out whether a process is a system process or not
NTSYSAPI NTSTATUS NTAPI ZwQueryInformationToken(
    HANDLE TokenHandle,
    ULONG TokenInformationClass,
    PVOID TokenInformation,
    ULONG TokenInformationLength,
    PULONG ReturnLength);

// undocumented API
// used by this driver to find out whether a process is a system process or not
NTSYSAPI NTSTATUS NTAPI ZwOpenProcessToken(
    HANDLE ProcessHandle,
    ACCESS_MASK DesiredAccess,
    PHANDLE TokenHandle);

#ifdef _WIN64
  // defined in ntifs.h
  NTSTATUS ZwOpenProcessTokenEx(
      HANDLE ProcessHandle,
      ACCESS_MASK DesiredAccess,
      ULONG HandleAttributes,
      PHANDLE TokenHandle);
#endif

// defined in ntifs.h
// allocates virtual memory in the specified process
NTSYSAPI NTSTATUS NTAPI ZwAllocateVirtualMemory(
    HANDLE ProcessHandle,
    PVOID *BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect);

// defined in ntifs.h
// process id -> eprocess pointer
NTSYSAPI NTSTATUS NTAPI PsLookupProcessByProcessId(
    HANDLE ProcessId,
    PEPROCESS *Process);

// defined in ntifs.h
// thread id -> ethread pointer
NTSYSAPI NTSTATUS NTAPI PsLookupThreadByThreadId(
    HANDLE ThreadId,
    PETHREAD *Thread);

#ifdef _WIN64
  // maybe documented in ntifs.h or maybe undocumented
  // eprocess pointer -> session id
  NTSYSAPI HANDLE NTAPI PsGetProcessSessionId(
      PEPROCESS Process);
#endif

// defined in ntifs.h
// eprocess pointer -> process handle
NTSYSAPI NTSTATUS NTAPI ObOpenObjectByPointer(
    PVOID Object,
    ULONG Flags,
    PACCESS_STATE AccessState,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE ObjectType,
    KPROCESSOR_MODE AccessMode,
    PHANDLE Handle);

// defined in ntifs.h
// ethread pointer -> eprocess pointer
NTSYSAPI PEPROCESS NTAPI IoThreadToProcess(
    PETHREAD Thread);

// defined in ntifs.h
// query the dos name ("C:\...") of a file object
NTSYSAPI NTSTATUS NTAPI IoQueryFileDosDeviceName(
    PFILE_OBJECT FileObject,
    PUNICODE_STRING *ObjectNameInformation);

// defined in ntifs.h
// attaches the current thread to the memory context of the specified process
NTSYSAPI VOID NTAPI KeStackAttachProcess(
    PEPROCESS Process,
    PKAPC_STATE ApcState);
NTSYSAPI VOID NTAPI KeUnstackDetachProcess(
    PKAPC_STATE ApcState);

// ********************************************************************

// structure for ZwQueryInformationThread
typedef struct _THREAD_BASIC_INFORMATION {
    NTSTATUS  ExitStatus;
    PVOID     TebBaseAddress;
    CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    KPRIORITY Priority;
    KPRIORITY BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

// structure for ZwQuerySystemInformation(SystemModuleInformation)
typedef struct _SYSTEM_MODULE_INFORMATION {
    PVOID  Reserved;
    PVOID  MappedBase;
    PVOID  ImageBase;
    ULONG  Size;
    ULONG  Flags;
    USHORT Index;
    USHORT Unknown;
    USHORT LoadCount;
    USHORT ModuleNameOffset;
    CHAR   ImageName[256];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

// structure for ZwQuerySystemInformation(SystemProcessInformation)
typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG  NextEntryOffset;
    ULONG  NumberOfThreads;
    CHAR   Reserved1[48];
    PVOID  Reserved2[3];
    HANDLE UniqueProcessId;
    HANDLE ParentProcessId;
    ULONG  HandleCount;
    ULONG  SessionId;
    PVOID  Reserved5[11];
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER Reserved6[6];
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

// structure for ZwQueryVirtualMemory
typedef struct _MEMORY_BASIC_INFORMATION {
    PVOID  BaseAddress;
    PVOID  AllocationBase;
    ULONG  AllocationProtect;
    SIZE_T RegionSize;
    ULONG  State;
    ULONG  Protect;
    ULONG  Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

// structure for ZwQueryInformationToken
typedef struct _SID_AND_ATTRIBUTES
{
    PSID Sid;
    ULONG Attributes;
} SID_AND_ATTRIBUTES, *PSID_AND_ATTRIBUTES;

// ********************************************************************
// undocumented APIs, *NOT* directly exported in kernel land
// 32bit: so we have to call these APIs by their service id
// 64bit: so we have to link to these APIs by dynamically parsing ntdll.dll

#ifdef _WIN64

  NTSTATUS (*ZwReadVirtualMemory)(
      HANDLE ProcessHandle,
      PVOID BaseAddress,
      PVOID Buffer,
      SIZE_T NumberOfBytesToRead,
      PSIZE_T NumberOfBytesRead) = NULL;

  NTSTATUS (*ZwWriteVirtualMemory)(
      HANDLE ProcessHandle,
      PVOID BaseAddress,
      PVOID Buffer,
      SIZE_T NumberOfBytesToWrite,
      PSIZE_T NumberOfBytesWritten) = NULL;

  NTSTATUS (*ZwProtectVirtualMemory)(
      HANDLE ProcessHandle,
      PVOID *BaseAddress,
      PSIZE_T NumberOfBytesToProtect,
      ULONG NewAccessProtection,
      PULONG OldAccessProtection) = NULL;

  NTSTATUS (*ZwQueryVirtualMemory)(
      HANDLE ProcessHandle,
      PVOID BaseAddress,
      ULONG MemoryInformationClass,
      PVOID MemoryInformation,
      SIZE_T MemoryInformationLength,
      PSIZE_T ReturnLength) = NULL;

#else

  #pragma optimize( "", off )

  // ZwProtect/Read/WriteVirtualMemory
  NTSTATUS ZwXxxVirtualMemoryAsm(
      ULONG ServiceId,
      HANDLE ProcessHandle,
      PVOID Param1,
      PVOID Param2,
      SIZE_T Param3,
      PSIZE_T Param4)
  {
    _asm
    {
      // this is the same that ntdll.dll does in user land
      // when ZwProtect/Read/WriteVirtualMemory are called
      mov eax, ServiceId
      lea edx, ServiceId
      add edx, 4
      int 0x2e
    }
  }

  NTSTATUS ZwQueryVirtualMemoryAsm(
      ULONG ServiceId,
      HANDLE ProcessHandle,
      PVOID BaseAddress,
      ULONG MemoryInformationClass,
      PVOID MemoryInformation,
      SIZE_T MemoryInformationLength,
      PSIZE_T ReturnLength)
  {
    _asm
    {
      mov eax, ServiceId
      lea edx, ServiceId
      add edx, 4
      int 0x2e
    }
  }

  NTSTATUS ZwQueryInformationThreadAsm(
      ULONG ServiceId,
      HANDLE ThreadHandle,
      THREADINFOCLASS ThreadInformationClass,
      PVOID ThreadInformation,
      ULONG ThreadInformationLength,
      PULONG ReturnLength)
  {
    _asm
    {
      mov eax, ServiceId
      lea edx, ServiceId
      add edx, 4
      int 0x2e
    }
  }

  NTSTATUS ZwOpenProcessTokenExAsm(
      ULONG ServiceId,
      HANDLE ProcessHandle,
      ACCESS_MASK DesiredAccess,
      ULONG HandleAttributes,
      PHANDLE TokenHandle)
  {
    _asm
    {
      mov eax, ServiceId
      lea edx, ServiceId
      add edx, 4
      int 0x2e
    }
  }

  #pragma optimize( "", on )

  // service ids of a few needed undocumented kernel mode APIs
  ULONG ZwReadVirtualMemoryServiceNo = 0;
  ULONG ZwWriteVirtualMemoryServiceNo = 0;
  ULONG ZwQueryVirtualMemoryServiceNo = 0;
  ULONG ZwProtectVirtualMemoryServiceNo = 0;
  ULONG ZwQueryInformationThreadServiceNo = 0;
  ULONG ZwOpenProcessTokenExServiceNo = 0;

  NTSTATUS NTAPI ZwReadVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T NumberOfBytesToRead, PSIZE_T NumberOfBytesRead)
  {
    return ZwXxxVirtualMemoryAsm(ZwReadVirtualMemoryServiceNo, ProcessHandle, BaseAddress, Buffer, NumberOfBytesToRead, NumberOfBytesRead);
  }

  NTSTATUS NTAPI ZwWriteVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T NumberOfBytesToWrite, PSIZE_T NumberOfBytesWritten)
  {
    return ZwXxxVirtualMemoryAsm(ZwWriteVirtualMemoryServiceNo, ProcessHandle, BaseAddress, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten);
  }

  NTSTATUS NTAPI ZwProtectVirtualMemory(HANDLE ProcessHandle, PVOID *BaseAddress, PULONG NumberOfBytesToProtect, ULONG NewAccessProtection, PULONG OldAccessProtection)
  {
    return ZwXxxVirtualMemoryAsm(ZwProtectVirtualMemoryServiceNo, ProcessHandle, (PVOID) BaseAddress, (PVOID) NumberOfBytesToProtect, NewAccessProtection, OldAccessProtection);
  }

  NTSTATUS NTAPI ZwQueryVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, ULONG MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength)
  {
    return ZwQueryVirtualMemoryAsm(ZwQueryVirtualMemoryServiceNo, ProcessHandle, BaseAddress, MemoryInformationClass, MemoryInformation, MemoryInformationLength, ReturnLength);
  }

  NTSTATUS NTAPI ZwQueryInformationThread(HANDLE ThreadHandle, THREADINFOCLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength, PULONG ReturnLength)
  {
    return ZwQueryInformationThreadAsm(ZwQueryInformationThreadServiceNo, ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength, ReturnLength);
  }

  NTSTATUS NTAPI ZwOpenProcessTokenEx(HANDLE ProcessHandle, ACCESS_MASK DesiredAccess, ULONG HandleAttributes, PHANDLE TokenHandle)
  // we link to ZwOpenProcessTokenEx this way because we want to stay compatible to Windows 2000
  {
    if (ZwOpenProcessTokenExServiceNo)
      return ZwOpenProcessTokenExAsm(ZwOpenProcessTokenExServiceNo, ProcessHandle, DesiredAccess, HandleAttributes, TokenHandle);
    else
      return ZwOpenProcessToken(ProcessHandle, DesiredAccess, TokenHandle);
  }

#endif

// ********************************************************************

int NtdllApiToServiceNo(PVOID NtdllApi)
// tries to extract the service no from a native ntdll API
{
  #ifdef _WIN64

    // most native APIs in the 64bit "ntdll.dll" begin with the code:
    //
    // asm
    //   mov r10, rcx   (binary: 4c 8b d1)
    //   mov eax, dw    (binary: b8 dw)

    if ( (NtdllApi) && ( ((ULONG*) NtdllApi)[0] == 0xb8d18b4c ) )
      return ((ULONG*) NtdllApi)[1];
    else
      return (int) -1;

  #else

    // XPSP2 and higher:
    //   mov eax, dw           (binary: b8 dw)
    //   mov edx, dw           (binary: ba dw)
    //   call dword ptr [edx]  (binary: ff 12)
    //   retn w                (binary: c2 w)

    if ( (NtdllApi) &&
         ( *(UCHAR *) ((ULONG_PTR) NtdllApi +  0) == 0xb8   ) &&
         ( *(UCHAR *) ((ULONG_PTR) NtdllApi +  5) == 0xba   ) &&
         ( *(USHORT*) ((ULONG_PTR) NtdllApi + 10) == 0x12ff ) &&
         ( *(UCHAR *) ((ULONG_PTR) NtdllApi + 12) == 0xc2   )    )
      return *(int*) ((ULONG_PTR) NtdllApi + 1);

    else

      // Windows 8:
      //   mov eax, dw           (binary: b8 dw)
      //   call dw               (binary: e8 dw)
      //   retn w                (binary: c2 w)

      if ( (NtdllApi) &&
           ( *(UCHAR *) ((ULONG_PTR) NtdllApi +  0) == 0xb8   ) &&
           ( *(UCHAR *) ((ULONG_PTR) NtdllApi +  5) == 0xe8   ) &&
           ( *(UCHAR *) ((ULONG_PTR) NtdllApi + 10) == 0xc2   )    )
        return *(int*) ((ULONG_PTR) NtdllApi + 1);

      else

        // XPSP0 + XPSP1:
        //   mov eax, dw  (binary: b8 dw)
        //   mov edx, dw  (binary: ba dw)
        //   call edx     (binary: ff d2)
        //   retn w       (binary: c2 w)

        if ( (NtdllApi) &&
             ( *(UCHAR *) ((ULONG_PTR) NtdllApi +  0) == 0xb8   ) &&
             ( *(UCHAR *) ((ULONG_PTR) NtdllApi +  5) == 0xba   ) &&
             ( *(USHORT*) ((ULONG_PTR) NtdllApi + 10) == 0xd2ff ) &&
             ( *(UCHAR *) ((ULONG_PTR) NtdllApi + 12) == 0xc2   )    )
          return *(int*) ((ULONG_PTR) NtdllApi + 1);

        else

          // NT4 + 2000:
          //   mov eax, dw           (binary: b8 dw)
          //   lea edx, [esp+4]      (binary: 8d 54 24 04)
          //   int 0x2e              (binary: cd 2e)
          //   retn w                (binary: c2 w)

          if ( (NtdllApi) &&
               ( *(UCHAR *) ((ULONG_PTR) NtdllApi +  0) == 0xb8       ) &&
               ( *(ULONG *) ((ULONG_PTR) NtdllApi +  5) == 0x0424548d ) &&
               ( *(USHORT*) ((ULONG_PTR) NtdllApi +  9) == 0x2ecd     ) &&
               ( *(UCHAR *) ((ULONG_PTR) NtdllApi + 11) == 0xc2       )    )
            return *(int*) ((ULONG_PTR) NtdllApi + 1);

          else
            // sorry, no idea what service no this native API has

            return (ULONG) -1;

  #endif
}

BOOLEAN GetServiceNumberByName(HANDLE ntdllModule, BOOLEAN virt, PCSTR ServiceName, ULONG *ServiceNo)
// find out which service number a specific native API has
{
  PVOID ntdllApi = GetProcAddress(ntdllModule, ServiceName, virt, virt);
  *ServiceNo = NtdllApiToServiceNo(ntdllApi);

  return (*ServiceNo != (ULONG) -1);
}

#ifdef _WIN64

  PVOID GetServiceAddressByNumber(HANDLE ntdllModule, BOOLEAN virt, int ServiceNo)
  // this function tries to locate the internal "Zw" API in the NtOsKrnl.exe
  {
    if ( ServiceNo != -1 )
    {

      // first we try to find out which service number "ZwAllocateVirtualMemory" has
      PVOID ZwAllocateNtdllApi = GetProcAddress(ntdllModule, "ZwAllocateVirtualMemory", virt, virt);
      int ZwAllocateServiceNo = NtdllApiToServiceNo(ZwAllocateNtdllApi);

      if ( ZwAllocateServiceNo != -1 )
      {
        // we successfully extracted the "ZwAllocateVirtualMemory" service no
        // the internal "ZwAllocateVirtualMemory" API in the NtOsKrnl is known
        // so we look through it's code and try to find a reference to the service number

        UCHAR *buf = GetProcAddress(GetSystemModuleHandle("ntoskrnl.exe"), "ZwAllocateVirtualMemory", TRUE, TRUE);
        int i1;
        if (!buf)
          buf = (PVOID) ZwAllocateVirtualMemory;

        for (i1 = 0; i1 < 100; i1++)
        {
          if ( (buf[i1] == 0xb8) && (*(int*)&(buf[i1 + 1]) == ZwAllocateServiceNo) )
          {
            // we found a "mov eax, ZwAllocateServiceNo" instruction
            // now we continue to search for a reference to the succeeding service number
            // we take advantage of the knowledge that NtOsKrnl internally has "Zw" stubs
            // for all services in running order, which are all nearly identical
            // so we try to track the stub code pattern down, shouldn't be too hard

            int i2;

            for (i2 = i1; i2 < i1 + 100; i2++)
            {
              if ( (buf[i2] == 0xb8) && (*(int*)&(buf[i2 + 1]) == ZwAllocateServiceNo + 1) )
              {
                // ok, now we have a reference to two service numbers in the "Zw" stubs
                // as a result we can easily find out how long each stub is

                int apiLength = i2 - i1;

                // finding the address of the wanted service is a simple calculation now
                UCHAR *result = buf + (ServiceNo - ZwAllocateServiceNo) * apiLength;

                // of course such kind of hacks can stop working some day
                // so we need to do some safety checks to make sure everything is as expected
                if ( (result[i1] == 0xb8) && (*(int*)&(result[i1 + 1]) == ServiceNo) )
                {
                  // the wanted service does reference its own service number at the expected location
                  // so our calculations obviously were correct and we can return the result
                  return result;

                }
                break;
              }
            }
            break;
          }
        }
      }
    }
    return NULL;
  }

  PVOID GetServiceAddressByName(HANDLE ntdllModule, BOOLEAN virt, PCSTR ServiceName)
  {
    PVOID ntdllApi = GetProcAddress(ntdllModule, ServiceName, virt, virt);
    int serviceNo = NtdllApiToServiceNo(ntdllApi);

    return GetServiceAddressByNumber(ntdllModule, virt, serviceNo);
  }

#endif

// ********************************************************************

void InitObjectAttributes(POBJECT_ATTRIBUTES ObjAttr, PUNICODE_STRING UniStr, LPWSTR Name)
// initialization of some structures for file access
{
  if (Name)
    RtlInitUnicodeString(UniStr, Name);
  ObjAttr->Length                   = sizeof(OBJECT_ATTRIBUTES);
  ObjAttr->RootDirectory            = 0;
  ObjAttr->ObjectName               = UniStr;
  ObjAttr->Attributes               = 0x240;  // OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE
  ObjAttr->SecurityDescriptor       = NULL;
  ObjAttr->SecurityQualityOfService = NULL;
}

BOOLEAN MapDllFile(LPWSTR ModulePath, HANDLE *file, HANDLE *section, HANDLE *moduleHandle)
{
  OBJECT_ATTRIBUTES oa;
  UNICODE_STRING    us;
  IO_STATUS_BLOCK   isb;
  BOOLEAN           result = FALSE;

  InitObjectAttributes(&oa, &us, ModulePath);

  // first let's open the file and map it into memory
  if (NT_SUCCESS(ZwCreateFile(file, FILE_READ_DATA, &oa, &isb, NULL, 0, FILE_SHARE_READ, FILE_OPEN, 0, NULL, 0)))
  {
    InitObjectAttributes(&oa, NULL, NULL);
    if (NT_SUCCESS(ZwCreateSection(section, STANDARD_RIGHTS_REQUIRED | SECTION_MAP_READ | SECTION_QUERY, &oa, NULL, PAGE_READONLY, 0x8000000 /*SEC_COMMIT*/, *file)))
    {
      SIZE_T viewSize = 0;
      PVOID buf = NULL;
      if ((NT_SUCCESS(ZwMapViewOfSection(*section, (HANDLE) -1, &buf, 0, 0, NULL, &viewSize, 1, 0, PAGE_READONLY))) && (buf))
      {
        // succeeded, now let's check whether it's a valid module image

        if (((PIMAGE_DOS_HEADER) buf)->e_magic == IMAGE_DOS_SIGNATURE)
        {
          // might be a valid image

          PIMAGE_NT_HEADERS nh = (PVOID) ((ULONG_PTR) buf + ((PIMAGE_DOS_HEADER) buf)->e_lfanew);

          if (nh->Signature == IMAGE_NT_SIGNATURE)
          {
            // yep, it's really a valid image

            *moduleHandle = (HANDLE) buf;
            return TRUE;
          }
        }

        ZwUnmapViewOfSection((HANDLE) -1, buf);
      }
      ZwClose(*section);
    }
    ZwClose(*file);
  }

  return result;
}

void UnmapDllFile(HANDLE file, HANDLE section, HANDLE moduleHandle)
{
  ZwUnmapViewOfSection((HANDLE) -1, (PVOID) moduleHandle);
  ZwClose(section);
  ZwClose(file);
}

// ********************************************************************

BOOLEAN InitToolFuncs(void)
// initialize all the undocumented stuff
{
  BOOLEAN result = FALSE;
  HANDLE ntdllFile = NULL;
  HANDLE ntdllSection = NULL;
  HANDLE ntdllModule = NULL;
  BOOLEAN virt;

  ntdllModule = GetSystemModuleHandle("ntdll.dll");
  virt = (ntdllModule != NULL);

  if ((virt) || (MapDllFile(L"\\SystemRoot\\System32\\ntdll.dll", &ntdllFile, &ntdllSection, &ntdllModule)))
  {
    #ifdef _WIN64
      result = ( (ZwReadVirtualMemory = GetServiceAddressByName(ntdllModule, virt, "ZwReadVirtualMemory")) &&
                 (ZwWriteVirtualMemory = GetServiceAddressByName(ntdllModule, virt, "ZwWriteVirtualMemory")) &&
                 (ZwQueryVirtualMemory = GetServiceAddressByName(ntdllModule, virt, "ZwQueryVirtualMemory")) &&
                 (ZwProtectVirtualMemory = GetServiceAddressByName(ntdllModule, virt, "ZwProtectVirtualMemory")) );
    #else
      result = ( (GetServiceNumberByName(ntdllModule, virt, "ZwReadVirtualMemory", &ZwReadVirtualMemoryServiceNo)) &&
                 (GetServiceNumberByName(ntdllModule, virt, "ZwWriteVirtualMemory", &ZwWriteVirtualMemoryServiceNo)) &&
                 (GetServiceNumberByName(ntdllModule, virt, "ZwQueryVirtualMemory", &ZwQueryVirtualMemoryServiceNo)) &&
                 (GetServiceNumberByName(ntdllModule, virt, "ZwProtectVirtualMemory", &ZwProtectVirtualMemoryServiceNo)) &&
                 (GetServiceNumberByName(ntdllModule, virt, "ZwQueryInformationThread", &ZwQueryInformationThreadServiceNo)) );
      GetServiceNumberByName(ntdllModule, virt, "ZwOpenProcessTokenEx", &ZwOpenProcessTokenExServiceNo);
    #endif

    if (!virt)
      UnmapDllFile(ntdllFile, ntdllSection, ntdllModule);
  }

  return result;
}

// ********************************************************************

void LogBin(LPWSTR FileName, PVOID Buf, ULONG Len)
{
  OBJECT_ATTRIBUTES oa;
  UNICODE_STRING    us;
  IO_STATUS_BLOCK   isb;
  HANDLE            fh;

  InitObjectAttributes(&oa, &us, FileName);
  if (NT_SUCCESS(ZwCreateFile(&fh, FILE_WRITE_DATA | SYNCHRONIZE, &oa, &isb, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_SUPERSEDE, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0))) {
    ZwWriteFile(fh, 0, NULL, NULL, &isb, Buf, Len, NULL, NULL);
    ZwClose(fh);
  }
}

void LogStr(LPWSTR FileName, LPWSTR Content)
{
  LogBin(FileName, Content, (ULONG) wcslen(Content) * 2);
}

void LogUlonglong(LPWSTR FileName, ULONGLONG Value)
{
  CHAR str[16];
  int i1;

  for (i1 = 0; i1 < 16; i1++) {
    DWORD32 k = (DWORD32)((Value >> 60) & 0xf);
    if (k < 0xa)
      str[i1] = L'0' + k;
    else
      str[i1] = L'a' + (k - 0xa);
    Value = Value << 4;
  }

  LogBin(FileName, str, 16);
}

// ********************************************************************

BOOLEAN IsXpOrNewer(void)
// is this XP or 2003 or any newer OS?
{
  #ifdef _WIN64
    // all 64bit OSs are XP/2003 or newer
    return TRUE;
  #else
    // we can't use RtlGetVersion() here because that wouldn't be NT4 compatible
    ULONG major, minor;
    PsGetVersion(&major, &minor, NULL, NULL);
    return ((major > 5) || ((major == 5) && (minor > 0)));
  #endif
}

BOOLEAN IsVistaOrNewer(void)
// is this Vista or 2008 or any newer OS?
{
  #ifdef _WIN64
    // all 64bit OSs support the RtlGetVersion() API, so we can use it here
    RTL_OSVERSIONINFOW version;
    version.dwOSVersionInfoSize = sizeof(version);
    return ((NT_SUCCESS(RtlGetVersion(&version))) && (version.dwMajorVersion > 5));
  #else
    // we can't use RtlGetVersion() here because that wouldn't be NT4 compatible
    ULONG major, minor;
    PsGetVersion(&major, &minor, NULL, NULL);
    return (major > 5);
  #endif
}

BOOLEAN IsWin8OrNewer(void)
// is this Windows 8 or any newer OS?
{
  #ifdef _WIN64
    // all 64bit OSs support the RtlGetVersion() API, so we can use it here
    RTL_OSVERSIONINFOW version;
    version.dwOSVersionInfoSize = sizeof(version);
    return ((NT_SUCCESS(RtlGetVersion(&version))) && ((version.dwMajorVersion > 6) || ((version.dwMajorVersion == 6) && (version.dwMinorVersion > 1))));
  #else
    // we can't use RtlGetVersion() here because that wouldn't be NT4 compatible
    ULONG major, minor;
    PsGetVersion(&major, &minor, NULL, NULL);
    return ((major > 6) || ((major == 6) && (minor > 1)));
  #endif
}

// ********************************************************************

HANDLE GetSystemModuleHandle(PCSTR ModuleName)
// get the handle of a system dll
// currently used by this driver to locate "ntdll.dll"
{
  ULONG_PTR result = 0;
  ULONG size = 0;
  PULONG_PTR buf;

  ZwQuerySystemInformation(11 /*SystemModuleInformation*/, &buf, 4, &size);
  if (size)
  {
    buf = ExAllocatePool(PagedPool, size * 2);

    if (buf)
    {

      if (!ZwQuerySystemInformation(11 /*SystemModuleInformation*/, buf, size * 2, NULL))
      {
        // we got information about the loaded system modules

        PSYSTEM_MODULE_INFORMATION modules = (PVOID) (buf + 1);
        ULONG i1;

        for (i1 = 0; i1 < *((ULONG*) buf); i1++)
        {
          // now we loop through all modules to find 'ntdll.dll'

          if (!_stricmp(ModuleName, &modules[i1].ImageName[modules[i1].ModuleNameOffset]))
          {
            // found it!

            result = (ULONG_PTR) modules[i1].ImageBase;

            break;
          }
        }
      }
      ExFreePool(buf);
    }
  }

  return (HANDLE) result;
}

// ********************************************************************

BOOLEAN AttachToMemoryContext(HANDLE ProcessId, PEPROCESS *EProcess, KAPC_STATE *ApcState)
// attaches the current thread to the memory context of the specific process
{
  if (NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, EProcess)))
  {
    KeStackAttachProcess(*EProcess, ApcState);
    return TRUE;
  }
  return FALSE;
}

HANDLE FindNtdll(void)
// tries to locate ntdll, loaded in the current memory context
{
  HANDLE result = NULL;

  // kind of hacky, but I don't see any other way:
  // (1) get PEB loader data
  // (2) enumerate through InLoadOrderModuleList
  PROCESS_BASIC_INFORMATION pbi;
  if (NT_SUCCESS(ZwQueryInformationProcess(NtCurrentProcess(), ProcessBasicInformation, &pbi, sizeof(pbi), NULL)))
  {
    __try
    {
      #ifdef _WIN64
        int peb_ldr_offset = 0x18;
        int ldr_loadOrderList_offset = 0x10;
        int dll_baseName_offset = 0x58;
        int dll_baseAddr_offset = 0x30;
      #else
        int peb_ldr_offset = 0x0c;
        int ldr_loadOrderList_offset = 0x0c;
        int dll_baseName_offset = 0x2c;
        int dll_baseAddr_offset = 0x18;
      #endif

      // first let's get the loader data block
      PVOID ldr = *((PVOID*) ((ULONG_PTR) pbi.PebBaseAddress + peb_ldr_offset));

      // now let's get the first loaded module
      PVOID dll = *((PVOID*) ((ULONG_PTR) ldr + ldr_loadOrderList_offset));

      // now let's enumerate the modules, ntdll should be one of the first
      int i1;
      for (i1 = 0; i1 < 8; i1++)
      {
        // let's check the module name
        UNICODE_STRING* dllName = (UNICODE_STRING*) ((ULONG_PTR) dll + dll_baseName_offset);
        if ((dllName->Buffer) && (dllName->Length == 18) && (!_wcsicmp(dllName->Buffer, L"ntdll.dll")))
        {
          // we found it!
          result = (HANDLE) *((PVOID*) ((ULONG_PTR) dll + dll_baseAddr_offset));
          break;
        }
        dll = *((PVOID*) dll);
        if (!dll)
          break;
      }
    } __except (1) {}
  }

  return result;
}

void DetachMemoryContext(PEPROCESS EProcess, KAPC_STATE *ApcState)
// undoes "AttachToMemoryContext", see above
{
  KeUnstackDetachProcess(ApcState);
  ObDereferenceObject(EProcess);
}

// ********************************************************************

BOOLEAN IsRemoteModule32bit(HANDLE Process, HANDLE ModuleHandle)
// find out whether the specified module handle is a 32bit module
// currently used by this driver to check whether a given "ntdll.dll" is wow64
{
  IMAGE_DOS_HEADER dh;
  if ((ReadProcessMemory(Process, ModuleHandle, &dh, sizeof(dh))) && (dh.e_magic == IMAGE_DOS_SIGNATURE))
  {
    // might be a valid image

    IMAGE_NT_HEADERS32 nh32;
    if ( (ReadProcessMemory(Process, (PVOID) ((ULONG_PTR) ModuleHandle + dh.e_lfanew), &nh32, sizeof(nh32))) && 
         (nh32.Signature == IMAGE_NT_SIGNATURE) &&
         (nh32.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) )
    {
      // yeah, it's really a valid 32bit image

      return TRUE;
    }
  }

  return FALSE;
}

// ********************************************************************

ULONG VirtualToRaw(PIMAGE_NT_HEADERS nh, ULONG address)
// fully loaded dlls are mapped into memory section by section
// a dll file on harddisk is stored in a more packed format
// this function converts a mapped/virtual address to the packed/raw address
{
  PIMAGE_SECTION_HEADER sections;
  int i1;

  if (nh->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    sections = (PIMAGE_SECTION_HEADER) (((ULONG_PTR) &nh->OptionalHeader) + sizeof(IMAGE_OPTIONAL_HEADER64));
  else
    sections = (PIMAGE_SECTION_HEADER) (((ULONG_PTR) &nh->OptionalHeader) + sizeof(IMAGE_OPTIONAL_HEADER32));
  for (i1 = 0; i1 < nh->FileHeader.NumberOfSections; i1++)
    if ( (address >= sections[i1].VirtualAddress) &&
         ((i1 == nh->FileHeader.NumberOfSections - 1) || (address < sections[i1 + 1].VirtualAddress)) )
    {
      // found the image section header which this virtual address is in
      return address - sections[i1].VirtualAddress + sections[i1].PointerToRawData;
      break;
    }

  return address;
}

PVOID GetProcAddress(HANDLE ModuleHandle, PCSTR ProcName, BOOLEAN Virtual, BOOLEAN VirtualResult)
// see win32 API "GetProcAddress"
// used by this driver to find the address of some ntdll.dll exports
// this code requires the dll to have the same bitdepth as the driver/OS
{
  if ((ModuleHandle) && (((PIMAGE_DOS_HEADER) ModuleHandle)->e_magic == IMAGE_DOS_SIGNATURE))
  {
    // might be a valid image

    PIMAGE_NT_HEADERS nh = (PVOID) ((ULONG_PTR) ModuleHandle + ((PIMAGE_DOS_HEADER) ModuleHandle)->e_lfanew);

    if (nh->Signature == IMAGE_NT_SIGNATURE)
    {
      // yep, it's really a valid image

      ULONG                   size      = ((PIMAGE_DATA_DIRECTORY) (nh->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT))->Size;
      ULONG                   addr      = ((PIMAGE_DATA_DIRECTORY) (nh->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT))->VirtualAddress;
      PIMAGE_EXPORT_DIRECTORY exports   = (PVOID)   ((ULONG_PTR) ModuleHandle + ((Virtual) ? (addr                          ) : VirtualToRaw(nh, addr                          )));
      PULONG                  functions = (PULONG)  ((ULONG_PTR) ModuleHandle + ((Virtual) ? (exports->AddressOfFunctions   ) : VirtualToRaw(nh, exports->AddressOfFunctions   )));
      PUSHORT                 ordinals  = (PUSHORT) ((ULONG_PTR) ModuleHandle + ((Virtual) ? (exports->AddressOfNameOrdinals) : VirtualToRaw(nh, exports->AddressOfNameOrdinals)));
      PULONG                  names     = (PULONG)  ((ULONG_PTR) ModuleHandle + ((Virtual) ? (exports->AddressOfNames       ) : VirtualToRaw(nh, exports->AddressOfNames       )));
      ULONG_PTR               nameDif   = 0;
      BOOLEAN                 first     = TRUE;
      ULONG                   i1;

      for (i1 = 0; i1 < exports->NumberOfNames; i1++)
      {
        // let's loop through all APIs until we find the right one

        if ((functions[ordinals[i1]] < addr) || (functions[ordinals[i1]] >= addr + size))
        {
          if (first)
          {
            if (!Virtual)
              nameDif = (ULONG_PTR) VirtualToRaw(nh, names[i1]) - (ULONG_PTR) names[i1];
            first = FALSE;
          }
          if (!strcmp(ProcName, (PSTR) ((ULONG_PTR) ModuleHandle + names[i1] + nameDif)))
          {
            // found it!

            return (PVOID) ((ULONG_PTR) ModuleHandle + ((VirtualResult) ? (functions[ordinals[i1]]) : VirtualToRaw(nh, functions[ordinals[i1]])));
          }
        }
      }
    }
  }

  return NULL;
}

PVOID GetRemoteProcAddress(HANDLE Process, HANDLE ModuleHandle, PCSTR ProcName)
// similar to GetProcAddress, but for a remote process / memory context
// used by this driver to find the address of some wow64 ntdll.dll exports
{
  IMAGE_DOS_HEADER dh;
  if ((ReadProcessMemory(Process, ModuleHandle, &dh, sizeof(dh))) && (dh.e_magic == IMAGE_DOS_SIGNATURE))
  {
    // might be a valid image

    IMAGE_NT_HEADERS32 nh32;
    IMAGE_NT_HEADERS64 nh64;
    if ((ReadProcessMemory(Process, (PVOID) ((ULONG_PTR) ModuleHandle + dh.e_lfanew), &nh32, sizeof(nh32))) && (nh32.Signature == IMAGE_NT_SIGNATURE))
    {
      // yep, it's really a valid image

      ULONG addr = 0;
      ULONG size = 0;

      if (nh32.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
      {
        // it's a 32bit module

        addr = nh32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        size = nh32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
      }
      else
        if (nh32.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        {
          // it's a 64bit module

          if (ReadProcessMemory(Process, (PVOID) ((ULONG_PTR) ModuleHandle + dh.e_lfanew), &nh64, sizeof(nh64)))
          {
            addr = nh64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
            size = nh64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
          }
        }

      if ((addr) && (size))
      {
        PIMAGE_EXPORT_DIRECTORY exports = ExAllocatePool(PagedPool, size);
        if (exports)
        {
          PVOID result = NULL;

          if ( (ReadProcessMemory(Process, (PVOID) ((ULONG_PTR) ModuleHandle + addr), exports, size)) &&
               (exports->AddressOfFunctions    > addr) && (exports->AddressOfFunctions    < addr + size) &&
               (exports->AddressOfNameOrdinals > addr) && (exports->AddressOfNameOrdinals < addr + size) &&
               (exports->AddressOfNames        > addr) && (exports->AddressOfNames        < addr + size)    )
          {
            // we loaded the complete export section into memory
            // and all needed pointers are valid

            PULONG  functions = (PULONG)  ((ULONG_PTR) exports - addr + exports->AddressOfFunctions   );
            PUSHORT ordinals  = (PUSHORT) ((ULONG_PTR) exports - addr + exports->AddressOfNameOrdinals);
            PULONG  names     = (PULONG)  ((ULONG_PTR) exports - addr + exports->AddressOfNames       );
            ULONG   i1;

            for (i1 = 0; i1 < exports->NumberOfNames; i1++)
            {
              // let's loop through all APIs until we find the right one

              if ( ((functions[ordinals[i1]] < addr) || (functions[ordinals[i1]] >= addr + size)) &&
                   (names[i1] > addr) && (names[i1] < addr + size) &&
                   (!strcmp((PSTR) exports - addr + names[i1], ProcName)) )
              {
                // found it!

                result = (PVOID) ((ULONG_PTR) ModuleHandle + functions[ordinals[i1]]);
                break;
              }
            }
          }

          ExFreePool(exports);

          return result;

        }
      }
    }
  }

  return NULL;
}

// ********************************************************************

PVOID GetAddressOfEntryPoint(HANDLE ModuleHandle)
// return the address where the entry point of the specified dll is stored
// used by this driver to find the entry point of ntdll.dll
{
  if ((ModuleHandle) && (((PIMAGE_DOS_HEADER) ModuleHandle)->e_magic == IMAGE_DOS_SIGNATURE))
  {
    // might be a valid image

    PIMAGE_NT_HEADERS nh = (PVOID) ((ULONG_PTR) ModuleHandle + ((PIMAGE_DOS_HEADER) ModuleHandle)->e_lfanew);

    if (nh->Signature == IMAGE_NT_SIGNATURE)
    {
      // yep, it's really a valid image

      return &nh->OptionalHeader.AddressOfEntryPoint;
    }
  }

  return NULL;
}

PVOID GetRemoteAddressOfEntryPoint(HANDLE Process, HANDLE ModuleHandle)
// similar to GetAddressOfEntryPoint, but for a remote process / memory context
// used by this driver to find the entry point of wow64 ntdll.dll
{
  IMAGE_DOS_HEADER dh;
  if ((ReadProcessMemory(Process, ModuleHandle, &dh, sizeof(dh))) && (dh.e_magic == IMAGE_DOS_SIGNATURE))
  {
    // might be a valid image

    IMAGE_NT_HEADERS nh;
    if ((ReadProcessMemory(Process, (PVOID) ((ULONG_PTR) ModuleHandle + dh.e_lfanew), &nh, sizeof(nh))) && (nh.Signature == IMAGE_NT_SIGNATURE))
    {
      // yep, it's really a valid image

      return (PVOID) ((ULONG_PTR) ModuleHandle + dh.e_lfanew + (ULONG_PTR) &nh.OptionalHeader.AddressOfEntryPoint - (ULONG_PTR) &nh);
    }
  }

  return NULL;
}

// ********************************************************************

#ifndef _WIN64
  NTSTATUS (*PsSetLoadImageNotifyRoutine_)(PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine);
  NTSTATUS (*PsRemoveLoadImageNotifyRoutine_)(PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine);
#endif

NTSTATUS SetLoadImageNotifyRoutine(PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine)
{
  #ifdef _WIN64
    return PsSetLoadImageNotifyRoutine(NotifyRoutine);
  #else
    if (!PsSetLoadImageNotifyRoutine_)
    {
      UNICODE_STRING funcName;
      RtlInitUnicodeString(&funcName, L"PsSetLoadImageNotifyRoutine");
      PsSetLoadImageNotifyRoutine_ = MmGetSystemRoutineAddress(&funcName);
    }
    if (PsSetLoadImageNotifyRoutine_)
      return PsSetLoadImageNotifyRoutine_(NotifyRoutine);
    else
      return STATUS_PROCEDURE_NOT_FOUND;
  #endif
}

NTSTATUS RemoveLoadImageNotifyRoutine(PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine)
{
  #ifdef _WIN64
    return PsRemoveLoadImageNotifyRoutine(NotifyRoutine);
  #else
    if (!PsRemoveLoadImageNotifyRoutine_)
    {
      UNICODE_STRING funcName;
      RtlInitUnicodeString(&funcName, L"PsRemoveLoadImageNotifyRoutine");
      PsRemoveLoadImageNotifyRoutine_ = MmGetSystemRoutineAddress(&funcName);
    }
    if (PsRemoveLoadImageNotifyRoutine_)
      return PsRemoveLoadImageNotifyRoutine_(NotifyRoutine);
    else
      return STATUS_PROCEDURE_NOT_FOUND;
  #endif
}

// ********************************************************************

BOOLEAN ThreadHandleToThreadProcessId(HANDLE ThreadHandle, HANDLE *ThreadId, HANDLE *ProcessId)
// which thread id does this thread handle represent?
// which process does this thread belong to?
{
  if (ThreadId)
    *ThreadId = NULL;
  if (ProcessId)
    *ProcessId = NULL;

  if (ThreadHandle)
  {
    THREAD_BASIC_INFORMATION tbi;
    if (NT_SUCCESS(ZwQueryInformationThread(ThreadHandle, ThreadBasicInformation, &tbi, sizeof(tbi), NULL)))
    {
      if (ThreadId)
        *ThreadId = tbi.ClientId.UniqueThread;
      if (ProcessId)
        *ProcessId = tbi.ClientId.UniqueProcess;

      return TRUE;
    }
  }

  return FALSE;
}

BOOLEAN ProcessHandleToId(HANDLE ProcessHandle, HANDLE *ProcessId)
// which process id does this process handle represent?
{
  if (ProcessId)
    *ProcessId = NULL;

  if (ProcessHandle)
  {
    PROCESS_BASIC_INFORMATION pbi;
    if (NT_SUCCESS(ZwQueryInformationProcess(ProcessHandle, ProcessBasicInformation, &pbi, sizeof(pbi), NULL)))
    {
      if (ProcessId)
        *ProcessId = (HANDLE) pbi.UniqueProcessId;

      return TRUE;
    }
  }

  return FALSE;
}

HANDLE OpenEProcess(PEPROCESS Process)
{
  HANDLE result = NULL;
  if (NT_SUCCESS(ObOpenObjectByPointer(Process, OBJ_KERNEL_HANDLE, NULL, PROCESS_ALL_ACCESS, NULL, KernelMode, &result)))
    return result;
  else
    return NULL;
}

HANDLE OpenEThread(PETHREAD Thread)
{
  HANDLE result = NULL;
  if (NT_SUCCESS(ObOpenObjectByPointer(Thread, OBJ_KERNEL_HANDLE, NULL, THREAD_ALL_ACCESS, NULL, KernelMode, &result)))
    return result;
  else
    return NULL;
}

HANDLE OpenProcess(HANDLE ProcessId)
{
  HANDLE result = NULL;
  PEPROCESS pp;

  if (NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &pp)))
  {
    result = OpenEProcess(pp);
    ObDereferenceObject(pp);
  }

  return result;
}

HANDLE OpenCurrentProcess()
{
  return OpenEProcess(PsGetCurrentProcess());
}

HANDLE OpenThread(HANDLE ThreadId)
{
  HANDLE result = NULL;
  PETHREAD pt;

  if (NT_SUCCESS(PsLookupThreadByThreadId(ThreadId, &pt)))
  {
    result = OpenEThread(pt);
    ObDereferenceObject(pt);
  }

  return result;
}

BOOLEAN ProcessIdToSessionId(HANDLE ProcessId, ULONG *Session)
// get the session id of the specified process
{
  BOOLEAN result = FALSE;
  #ifdef _WIN64
    PEPROCESS pp;
    if (NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &pp)))
    {
      *Session = (ULONG) PsGetProcessSessionId(pp);
      ObDereferenceObject(pp);
      result = TRUE;
    }
  #else
    HANDLE ph = OpenProcess(ProcessId);
    if (ph)
    {
      *Session = GetProcessSessionId(ph);
      ZwClose(ph);
      result = TRUE;
    }
  #endif
  return result;
}

// ********************************************************************

ULONG CountProcesses(ULONG Session)
// count running processes of a specific session
{
  ULONG result = 0;
  ULONG size = 0;
  PVOID buf = NULL;

  ZwQuerySystemInformation(5 /*SystemProcessInformation*/, 0, 0, &size);

  if (!size)
  {
    // some older OSs don't report the needed size, sadly
    size = 0x10000;
    do
    {
      size = size * 4;
      buf = ExAllocatePool(PagedPool, size);
      if (!buf)
        break;
      if (!NT_SUCCESS(ZwQuerySystemInformation(5 /*SystemProcessInformation*/, buf, size, 0)))
      {
        ExFreePool(buf);
        buf = NULL;
      }
    } while ((!buf) && (size < 0x400000));
  } else
    // we got the needed size reported
    if ( (buf = ExAllocatePool(PagedPool, size * 2)) && 
         (!NT_SUCCESS(ZwQuerySystemInformation(5 /*SystemProcessInformation*/, buf, size * 2, 0))) )
    {
      ExFreePool(buf);
      buf = NULL;
    }

  if ( buf )
  {
    PSYSTEM_PROCESS_INFORMATION processes = buf;

    while (TRUE)
    {
      if (processes->SessionId == Session)
        result++;
      if (!processes->NextEntryOffset)
        break;
      processes = (PVOID) ((ULONG_PTR) processes + processes->NextEntryOffset);
    }
    ExFreePool(buf);
  }

  return result;
}

// ********************************************************************

WCHAR UpChar(WCHAR chr)
{
  if ((chr >= L'a') && (chr <= L'z'))
    chr = chr - L'a' + L'A';
  return chr;
}

void DeviceNameToDosName(WCHAR *Buf, ULONG BufLen)
// "\Device\HarddiskVolume1\Some Folder\Hook.dll" -> "C:\Some Folder\Some.dll"
{
  OBJECT_ATTRIBUTES oa;
  UNICODE_STRING    us;
  IO_STATUS_BLOCK   isb;
  HANDLE            fh;
  PFILE_OBJECT      fo;

  InitObjectAttributes(&oa, &us, Buf);
  if (NT_SUCCESS(ZwCreateFile(&fh, FILE_READ_DATA, &oa, &isb, NULL, 0, FILE_SHARE_READ, FILE_OPEN, 0, NULL, 0)))
  {
    if (NT_SUCCESS(ObReferenceObjectByHandle(fh, FILE_ALL_ACCESS, 0, KernelMode, &fo, 0)))
    {
      PUNICODE_STRING dosstr;
      if (NT_SUCCESS(IoQueryFileDosDeviceName(fo, &dosstr)))
      {
        if ((ULONG) dosstr->Length >= BufLen * 2)
          dosstr->Length = (USHORT) BufLen * 2 - 2;
        wcsncpy(Buf, dosstr->Buffer, dosstr->Length / 2);
        Buf[dosstr->Length / 2] = 0;

        ExFreePool(dosstr);
      }
      ObDereferenceObject(fo);
    }
    ZwClose(fh);
  }
/*
  // this should in theory help with IP UNC paths
  // but I've not found a way to test this, so it's commented out for now
  if ( (Buf) &&
       (Buf[0] == L'\\') &&
       (UpChar(Buf[1]) == L'D') && (UpChar(Buf[2]) == L'E') && (UpChar(Buf[3]) == L'V') && (UpChar(Buf[4]) == L'I') && (UpChar(Buf[5]) == L'C') && (UpChar(Buf[6]) == L'E') &&
       (Buf[7] == L'\\') &&
       (UpChar(Buf[8]) == L'M') && (UpChar(Buf[9]) == L'U') && (UpChar(Buf[10]) == L'P') &&
       (Buf[11] == L'\\') )
    // "\device\mup\192.168.1.1\bla" -> "\\192.168.1.1\bla"
    memmove(Buf + 1, Buf + 11, wcslen(Buf) * 2 - 20); */
}

BOOLEAN GetExeFileName(HANDLE ProcessHandle, WCHAR *Buf, ULONG BufLen)
// find the full exe file path/name of the specified process
// ZwQueryInformationProcess(ProcessImageFileName) doesn't work in NT/2000
// 32bit: so we can't use it cause our driver needs to support NT/2000, too
// 64bit: but that doesn't bother us cause there's no 64bit version of NT/2000
{
  BOOLEAN result = FALSE;

  RtlZeroMemory(Buf, BufLen * 2);
  if (ProcessHandle)
  {

    #ifdef _WIN64

      ULONG c1;

      if (ZwQueryInformationProcess(ProcessHandle, ProcessImageFileName, NULL, 0, &c1) == STATUS_INFO_LENGTH_MISMATCH)
      {
        PUNICODE_STRING ustr = ExAllocatePool(PagedPool, c1);
        if (ustr)
        {
          if (NT_SUCCESS(ZwQueryInformationProcess(ProcessHandle, ProcessImageFileName, ustr, c1, NULL)))
          {
            if ((ULONG) ustr->Length >= BufLen * 2)
              ustr->Length = (USHORT) BufLen * 2 - 2;
            wcsncpy(Buf, ustr->Buffer, ustr->Length / 2);
            Buf[ustr->Length / 2] = 0;

            if ((Buf[0] == L'\\') && (Buf[1] == L'D'))
              // we usually receive a "\Device\HarddiskVolumeX\" path here
              // but we prefer a dos path ("C:\") path for two reasons:
              // (1) not having to deal with different types of paths
              // (2) user land white/black lists have dos paths in mind
              DeviceNameToDosName(Buf, BufLen);

            result = TRUE;
          }
          ExFreePool(ustr);
        }
      }

    #else

      PROCESS_BASIC_INFORMATION pbi;
      PVOID pp;
      UNICODE_STRING ustr;
      ULONG flags;

      // kind of hacky, but the only way to make it work in older OSs, too:
      // (1) get PEB (process environment block) address
      // (2) get PEB->ProcessParameters structure
      // (3) get PEB->ProcessParameters->ImagePathName string
      if ( (NT_SUCCESS(ZwQueryInformationProcess(ProcessHandle, ProcessBasicInformation, &pbi, sizeof(pbi), NULL))) &&
           (ReadProcessMemory(ProcessHandle, (PVOID) ((ULONG_PTR) pbi.PebBaseAddress + 0x10), &pp,    4)) &&
           (ReadProcessMemory(ProcessHandle, (PVOID) ((ULONG_PTR) pp                 + 0x08), &flags, 4)) &&
           (ReadProcessMemory(ProcessHandle, (PVOID) ((ULONG_PTR) pp                 + 0x38), &ustr,  8))               )
      {
        if ((ULONG) ustr.Length >= BufLen * 2)
          ustr.Length = (USHORT) (BufLen * 2 - 2);
        if (!(flags & 1))  // PROCESS_PARAMETERS_NORMALIZED
          ustr.Buffer = (LPWSTR) ((ULONG_PTR) ustr.Buffer + (ULONG_PTR) pp);
        result = (ReadProcessMemory(ProcessHandle, (PVOID) ustr.Buffer, Buf, ustr.Length));
        if (result)
        {
          WCHAR helperChar;
          int compare;
          Buf[ustr.Length / 2] = 0;
          helperChar = Buf[4];
          Buf[4] = 0;
          compare = _wcsicmp(L"\\??\\", Buf);
          Buf[4] = helperChar;
          if (!compare)
            memmove(Buf, &Buf[4], ustr.Length + 2 - 4 * 2);

/*

here I'd like to replace "\SystemRoot\" with "C:\Windows\"
unfortunately there's no "GetWindowsDirectoryW" API in the DDK
so we have to live with that sometimes paths begin with "\SystemRoot\"
not such a big problem, cause it's rare for processes we're interested in

          else
          {
            helperChar = Buf[12];
            Buf[12] = 0;
            compare = _wcsicmp(L"\\SystemRoot\\", Buf);
            Buf[12] = helperChar;
            if (!compare)
            {
              WCHAR arrCh[260];
              int len = GetWindowsDirectoryW(arrCh, 260);
              if ((ULONG) ustr.Length / 2 + 1 - 11 + len >= BufLen)
              {
                ustr.Length = (USHORT) ((BufLen - 1 + 11 - len) * 2);
                Buf[ustr.Length / 2] = 0;
              }
              memmove(&Buf[len], &Buf[11], ustr.Length + 2 - 11 * 2);
              memmove(Buf, arrCh, len * 2);
            }
          }
*/

        }
      }

    #endif

  }
  return result;
}

BOOLEAN IsSystemProcess(HANDLE ProcessHandle)
// is the specified process a system process?
{
  BOOLEAN result = TRUE;
  PSID_AND_ATTRIBUTES saa = NULL;
  PVOID sid = NULL;
  HANDLE token;

  if (NT_SUCCESS(ZwOpenProcessTokenEx(ProcessHandle, 8 /*TOKEN_QUERY*/, OBJ_KERNEL_HANDLE, &token)))
  {
    ULONG c1 = 0;
    ZwQueryInformationToken(token, 1 /*TokenUser*/, NULL, 0, &c1);
    c1 = c1 * 2;
    saa = ExAllocatePool(PagedPool, c1);
    if (saa)
    {
      if (NT_SUCCESS(ZwQueryInformationToken(token, 1, saa, c1, &c1)))
        result = ((saa->Sid == NULL) || (((UCHAR*) saa->Sid)[1] <= 1));
      ExFreePool(saa);
    }
    ZwClose(token);
  }

  return result;
}

BOOLEAN Is64bitProcess(HANDLE ProcessHandle)
// is the specified process a 64bit process?
{
  #ifdef _WIN64
    ULONGLONG wow64;
    return (!NT_SUCCESS(ZwQueryInformationProcess(ProcessHandle, ProcessWow64Information, &wow64, 8, 0)) || (!wow64));
  #else
    return FALSE;
  #endif
}

BOOLEAN IsNativeProcess(HANDLE ProcessHandle)
// is the specified process a native process?
{
  PROCESS_BASIC_INFORMATION pbi;
  ULONG_PTR subsIndex;
  ULONG subs;

  #ifdef _WIN64
    if (!Is64bitProcess(ProcessHandle))
      return FALSE;
    subsIndex = 0x128;
  #else
    subsIndex = 0x0b4;
  #endif

  // slightly ugly, but I don't know any "official" way to do this
  // (1) get PEB (process environment block) address
  // (2) get PEB->SubSystem
  return ( (NT_SUCCESS(ZwQueryInformationProcess(ProcessHandle, ProcessBasicInformation, &pbi, sizeof(pbi), NULL))) &&
           (ReadProcessMemory(ProcessHandle, (PVOID) ((ULONG_PTR) pbi.PebBaseAddress + subsIndex), &subs, 4)) &&
           (subs == IMAGE_SUBSYSTEM_NATIVE) );
}

ULONG GetProcessSessionId(HANDLE ProcessHandle)
// get the session id of the specified process
{
  ULONG session;
  if (NT_SUCCESS(ZwQueryInformationProcess(ProcessHandle, ProcessSessionInformation, &session, sizeof(session), NULL)))
    return session;
  else
    return 0;
}

PVOID GetProcessEntryPoint(HANDLE ProcessHandle)
// get the entry point of the exe file of the specified process
{
  #ifdef _WIN64
    PVOID buf[8];
  #else
    PVOID buf[12];
  #endif

  if (!NT_SUCCESS(ZwQueryInformationProcess(ProcessHandle, ProcessImageInformation, buf, sizeof(buf), 0)))
    buf[0] = NULL;

  if (buf[0] == NULL)
  {
    // Vista should never reach this
    // in XP and 2003 "ProcessImageInformation" doesn't work the normal way
    // it works only if you attach to the target process first

    HANDLE processId;
    if (ProcessHandleToId(ProcessHandle, &processId))
    {
      PEPROCESS pp;
      if (NT_SUCCESS(PsLookupProcessByProcessId(processId, &pp)))
      {
        KAPC_STATE apcState;
        KeStackAttachProcess(pp, &apcState);
        if (!NT_SUCCESS(ZwQueryInformationProcess(NtCurrentProcess(), ProcessImageInformation, buf, sizeof(buf), 0)))
          buf[0] = NULL;
        KeUnstackDetachProcess(&apcState);
        ObDereferenceObject(pp);
      }
    }
  }

  return buf[0];
}

// ********************************************************************

BOOLEAN VirtualProtectEx(HANDLE ProcessHandle, PVOID BaseAddress, SIZE_T Size, ULONG NewAccess, PULONG OldAccess)
{
  return NT_SUCCESS(ZwProtectVirtualMemory(ProcessHandle, &BaseAddress, &Size, NewAccess, OldAccess));
}

BOOLEAN ReadProcessMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T Size)
{
  SIZE_T bytesRead;
  return (NT_SUCCESS(ZwReadVirtualMemory(ProcessHandle, BaseAddress, Buffer, Size, &bytesRead)) && (bytesRead == Size));
}

BOOLEAN WriteProcessMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T Size)
{
  SIZE_T bytesWritten;
  return (NT_SUCCESS(ZwWriteVirtualMemory(ProcessHandle, BaseAddress, Buffer, Size, &bytesWritten)) && (bytesWritten == Size));
}

static PVOID X86AllocAddr = (PVOID) 0x71b00000;
void SetX86AllocAddr(PVOID PreferredAddress)
{
  X86AllocAddr = PreferredAddress;
}

PVOID AllocMemEx(HANDLE ProcessHandle, SIZE_T Size, PVOID PreferredAddress)
// allocate memory in the specified process at 0x71b00000 (or slightly below that)
{
  PVOID result = NULL;
  MEMORY_BASIC_INFORMATION mbi;
  PVOID p1;

  if (((ULONG_PTR) PreferredAddress >= 0x80000000) || ((ULONG_PTR) PreferredAddress < 0x70000000))
  {
    p1 = PreferredAddress;

    // we loop through the memory area, looking for a free spot near to the preferred address
    while (NT_SUCCESS(ZwQueryVirtualMemory(ProcessHandle, p1, 0, &mbi, sizeof(mbi), NULL)))
    {
      if (mbi.State == MEM_FREE)
      {
        // found a free page, so let's reserve it and stop searching
        if ((ULONG_PTR) mbi.BaseAddress - (ULONG_PTR) PreferredAddress >= 0x80000000)
          // oooops, too far away for 32bit addressing
          break;
        if (NT_SUCCESS(ZwAllocateVirtualMemory(ProcessHandle, &mbi.BaseAddress, 0, &Size, MEM_RESERVE, PAGE_EXECUTE_READWRITE)))
        {
          result = mbi.BaseAddress;
          break;
        }
      }
      // we didn't find a free page yet, so let's continue to search after the current allocation
      if (mbi.RegionSize < 0x10000)
        mbi.RegionSize = 0x10000;
      else
        mbi.RegionSize = mbi.RegionSize & 0xffffffffffff0000;
      p1 = (PVOID) ((ULONG_PTR) p1 + mbi.RegionSize);
    }
    if (!result)
    {
      // something went wrong - this can happen especially in Windows 8.1
      // we didn't get any memory *after* the desired address, so we look *before*...
      if (NT_SUCCESS(ZwQueryVirtualMemory(ProcessHandle, PreferredAddress, 0, &mbi, sizeof(mbi), NULL)))
      {
        #ifdef _WIN64
          p1 = (PVOID) (((ULONG_PTR) mbi.BaseAddress) & 0xffffffffffff0000);
          if (p1 > (PVOID) 0x700000000000)
            // we need to avoid conflicts with system dlls, so we aim about 0x20000000 lower than the preferred address
            p1 = (PVOID) (((((ULONG_PTR) p1) & 0xfffffffff0000000) - 0x20000000 - Size) & 0xffffffffffff0000);
        #else
          p1 = (PVOID) (((ULONG_PTR) mbi.BaseAddress) & 0xffff0000);
        #endif
        // we loop through the memory area, looking for a free spot
        while (NT_SUCCESS(ZwQueryVirtualMemory(ProcessHandle, p1, 0, &mbi, sizeof(mbi), NULL)))
        {
          if (mbi.State == MEM_FREE)
          {
            // found a free page, so let's reserve it and stop searching
            if ((ULONG_PTR) PreferredAddress - (ULONG_PTR) mbi.BaseAddress >= 0x80000000)
              // oooops, too far away for 32bit addressing
              break;
            if (NT_SUCCESS(ZwAllocateVirtualMemory(ProcessHandle, &mbi.BaseAddress, 0, &Size, MEM_RESERVE, PAGE_EXECUTE_READWRITE)))
            {
              result = mbi.BaseAddress;
              break;
            }
          }
          // we didn't find a free page yet, so let's go one page back
          if (p1 == mbi.AllocationBase)
            p1 = (PVOID) ((ULONG_PTR) p1 - 0x10000);
          else
            p1 = mbi.AllocationBase;
        }
      }
    }
  }
  else
  {
    // allocate <= 0x71b00000 to avoid fragmentation
    p1 = X86AllocAddr;

    #ifdef _WIN64
      if ((!PreferredAddress) && (Is64bitProcess(ProcessHandle)))
        // allocate <= 0x7ed00000000 to avoid fragmentation
        p1 = (PVOID) 0x7ed00000000;
    #endif

    // we loop through the memory area, looking for a free spot
    while (NT_SUCCESS(ZwQueryVirtualMemory(ProcessHandle, p1, 0, &mbi, sizeof(mbi), NULL)))
    {
      if (mbi.State == MEM_FREE)
      {
        // found a free page, so let's reserve it and stop searching
        if (NT_SUCCESS(ZwAllocateVirtualMemory(ProcessHandle, &mbi.BaseAddress, 0, &Size, MEM_RESERVE, PAGE_EXECUTE_READWRITE)))
        {
          result = mbi.BaseAddress;
          break;
        }
      }
      // we didn't find a free page yet, so let's go one page back
      if (p1 == mbi.AllocationBase)
        p1 = (PVOID) ((ULONG_PTR) p1 - 0x10000);
      else
        p1 = mbi.AllocationBase;
    }
  }

  // finally let's commit the reserved page
  // we have to do this in two steps (reserve + commit)
  // or else Windows won't allow us to choose the allocation address
  if ((result) && (NT_SUCCESS(ZwAllocateVirtualMemory(ProcessHandle, &result, 0, &Size, MEM_COMMIT, PAGE_EXECUTE_READWRITE))))
    return result;

  return NULL;
}

// ********************************************************************

BOOLEAN Is64bitDll(HANDLE FileHandle)
// find out and return whether the dll file is a 64bit dll
{
  BOOLEAN result = FALSE;
  HANDLE section = 0;
  OBJECT_ATTRIBUTES oa;

  // for easier handling we map the file into RAM
  InitObjectAttributes(&oa, NULL, NULL);
  if (NT_SUCCESS(ZwCreateSection(&section, STANDARD_RIGHTS_REQUIRED | SECTION_MAP_READ | SECTION_QUERY, &oa, NULL, PAGE_READONLY, 0x8000000 /*SEC_COMMIT*/, FileHandle)))
  {

    PVOID buf = NULL;
    SIZE_T viewSize = 0;

    if (NT_SUCCESS(ZwMapViewOfSection(section, (HANDLE) -1, &buf, 0, 0, NULL, &viewSize, 1, 0, PAGE_READONLY)) && (buf))
    {

      ULONG_PTR moduleHandle = (ULONG_PTR) buf;

      if (((PIMAGE_DOS_HEADER) moduleHandle)->e_magic == IMAGE_DOS_SIGNATURE)
      {
        // might be a valid image

        PIMAGE_NT_HEADERS nh = (PVOID) (moduleHandle + ((PIMAGE_DOS_HEADER) moduleHandle)->e_lfanew);

        if (nh->Signature == IMAGE_NT_SIGNATURE)
        {
          // yep, it's really a valid image

          if (nh->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
            // and it's a 64bit dll
            result = TRUE;
        }
      }
      ZwUnmapViewOfSection((HANDLE) -1, buf);
    }
    ZwClose(section);
  }

  return result;
}

BOOLEAN GetFileHash(PWCHAR FileName, PVOID FileHash, ULONG MaxFileSize, ULONG *FileSize, HANDLE *FileHandle)
// open a file and calculate a hash of the file contents
// MaxFileSize: can be used to limit the hash to a part of the file, only
// FileSize:    size of the file is returned; can be NULL
// FileHandle:  handle of the file is returned; can be NULL (in that case the file handle is closed)
{
  OBJECT_ATTRIBUTES oa;
  UNICODE_STRING    us;
  IO_STATUS_BLOCK   isb;
  HANDLE            fh;
  WCHAR             newFile [260 + 4];
  BOOLEAN           result = FALSE;

  // we use dos type paths ("C:\"), so we have to prepend "\??\" to make ZwCreateFile work
#pragma warning(disable: 4995)
  wcscpy(newFile, L"\\??\\");
  RtlStringCbCatW(newFile, (260 + 4) * sizeof(WCHAR), FileName);
  memset(FileHash, 0, 20);
  InitObjectAttributes(&oa, &us, newFile);

  // open the file with such attributes that users can't rename/delete the file, anymore
  if (NT_SUCCESS(ZwCreateFile(&fh, FILE_READ_DATA, &oa, &isb, NULL, 0, FILE_SHARE_READ, FILE_OPEN, 0, NULL, 0)))
  {

    HANDLE section = 0;

    // for easier hash calculation we map the file into RAM
    InitObjectAttributes(&oa, NULL, NULL);
    if (NT_SUCCESS(ZwCreateSection(&section, STANDARD_RIGHTS_REQUIRED | SECTION_MAP_READ | SECTION_QUERY, &oa, NULL, PAGE_READONLY, 0x8000000 /*SEC_COMMIT*/, fh)))
    {

      PVOID buf = NULL;
      SIZE_T viewSize = 0;

      if (NT_SUCCESS(ZwMapViewOfSection(section, (HANDLE) -1, &buf, 0, 0, NULL, &viewSize, 1, 0, PAGE_READONLY)) && (buf))
      {

        FILE_STANDARD_INFORMATION fsi;
        IO_STATUS_BLOCK isb;

        if (NT_SUCCESS(ZwQueryInformationFile(fh, &isb, &fsi, sizeof(fsi), 5)))
        {
          if (FileSize)
            *FileSize = fsi.EndOfFile.LowPart;
          if ((MaxFileSize == 0) || (MaxFileSize > fsi.EndOfFile.LowPart))
            MaxFileSize = fsi.EndOfFile.LowPart;

          // finally let's calculate the hash
          Hash((PVOID)buf, MaxFileSize, FileHash);

          result = TRUE;
        }
        ZwUnmapViewOfSection((HANDLE) -1, buf);
      }
      ZwClose(section);
    }
    if (FileHandle)
      *FileHandle = fh;
    else
      ZwClose(fh);
  }

  return result;
}

// ********************************************************************

void InitializeCriticalSection(CRIT_SECTION *section)
{
  KeInitializeMutex(section, 0);
}

void EnterCriticalSection(CRIT_SECTION *section)
{
  KeWaitForMutexObject(section, Executive, KernelMode, FALSE, NULL);
}

void LeaveCriticalSection(CRIT_SECTION *section)
{
  KeReleaseMutex(section, FALSE);
}


// ********************************************************************
