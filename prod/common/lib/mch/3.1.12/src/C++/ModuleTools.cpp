// ***************************************************************
//  ModuleTools.cpp           version: 1.0.2  ·  date: 2012-04-03
//  -------------------------------------------------------------
//  module/dll related tools
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2012 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2012-04-03 1.0.2 improved 64bit injection thread safety
// 2011-03-26 1.0.1 (1) small change to make (un)injection slightly more stable
//                  (2) added special handling for "wine"
// 2010-01-10 1.0.0 initial version

#define _MODULETOOLS_C

#include <intrin.h>
#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

SYSTEMS_API ULONG WINAPI GetSizeOfImage(PIMAGE_NT_HEADERS pNtHeaders)
{
  if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    return ((PIMAGE_NT_HEADERS64) pNtHeaders)->OptionalHeader.SizeOfImage;
  else
    return ((PIMAGE_NT_HEADERS32) pNtHeaders)->OptionalHeader.SizeOfImage;
}

SYSTEMS_API PVOID WINAPI NeedModuleFileMap(HMODULE hModule)
{
  if (IsWine())
    return NULL;

  PVOID result = NULL;
  wchar_t *moduleName = (wchar_t*) LocalAlloc(LPTR, 32 * 1024 * 2);
  GetModuleFileNameW(hModule, moduleName, 32 * 1024);
  HANDLE fh = CreateFileW(moduleName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
  LocalFree(moduleName);
  if (fh != INVALID_HANDLE_VALUE)
  {
    HANDLE map = CreateFileMappingW(fh, NULL, PAGE_READONLY, 0, 0, NULL);
    if (map)
    {
      result = MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);
      CloseHandle(map);
    }
    CloseHandle(fh);
  }
  return result;
}

SYSTEMS_API LPVOID WINAPI GetImageProcAddress(HMODULE hModule, LPCSTR name, BOOL doubleCheck)
{
  TraceVerbose(L"%S(%p, %S, %d)", __FUNCTION__, hModule, name, doubleCheck);

  LPVOID result = NULL;

  __try
  {
    if (hModule != NULL)
    {
      IMAGE_NT_HEADERS *pNtHeaders;
      pNtHeaders = GetImageNtHeaders(hModule);
      if (pNtHeaders != NULL)
      {
        IMAGE_EXPORT_DIRECTORY *pExportDirectory;
        DWORD virtualAddress;
        DWORD sizeOfImage = GetSizeOfImage(pNtHeaders);
        if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
          virtualAddress = ((PIMAGE_NT_HEADERS64) pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        else
          virtualAddress = ((PIMAGE_NT_HEADERS32) pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        pExportDirectory = (IMAGE_EXPORT_DIRECTORY *) ((ULONG_PTR) hModule + virtualAddress);
        if (pExportDirectory != NULL)
        {
          for (DWORD i = 0; i < pExportDirectory->NumberOfNames; i++)
          {
            DWORD nameOffset = ((DWORD *) ((ULONG_PTR) hModule + pExportDirectory->AddressOfNames))[i];
            LPCSTR pName = (LPCSTR) ((ULONG_PTR) hModule + nameOffset);
            if (!strcmp(pName, name))
            {
              WORD ordinal = ((WORD *) ((ULONG_PTR) hModule + pExportDirectory->AddressOfNameOrdinals))[i];
              DWORD functionOffset = ((DWORD *) ((ULONG_PTR) hModule + pExportDirectory->AddressOfFunctions))[ordinal];
              if ((doubleCheck) || (functionOffset > sizeOfImage))
              {
                LPVOID functionAddress = (LPVOID) ((ULONG_PTR) hModule + functionOffset);
                if ((pfnCheckProcAddress) && (pfnCheckProcAddress(&functionAddress)))
                {
                  result = functionAddress;
                  break;
                }
                LPVOID buffer = NULL;
                if (pfnNeedModuleFileMapEx)
                  buffer = pfnNeedModuleFileMapEx(hModule);
                bool freeBuf = !buffer;
                if (!buffer)
                  buffer = NeedModuleFileMap(hModule);
                if (buffer)
                {
                  __try
                  {
                    DWORD aon = pExportDirectory->AddressOfNames;
                    DWORD aof = pExportDirectory->AddressOfFunctions;
                    pExportDirectory = (IMAGE_EXPORT_DIRECTORY *) ((ULONG_PTR) buffer + VirtualToRaw(pNtHeaders, virtualAddress));
                    if ((aon == pExportDirectory->AddressOfNames) && (aof == pExportDirectory->AddressOfFunctions))
                      functionOffset = ((DWORD *) ((ULONG_PTR) buffer + VirtualToRaw(pNtHeaders, pExportDirectory->AddressOfFunctions)))[ordinal];
                  } __except (1) { }
                  if (freeBuf)
                    UnmapViewOfFile(buffer);
                }
                else
                  Trace(L"%S Failure - NeedModuleFileMap failed", __FUNCTION__);
              } // if double check
              result = ExportToFunc(hModule, functionOffset);
              break;
            } // if name match
          } // for loop
        } // pExportDirectory != NULL
      } // pNtHeaders != NULL
      // If above fails, fall back to default API
      if (result == NULL)
      {
        Trace(L"%S Failure - Using GetProcAddress", __FUNCTION__);
        result = (LPVOID) GetProcAddress(hModule, name);
      }
    } // hModule != NULL
  }
  __except (ExceptionFilter(L"GetImageProcAddress - name", GetExceptionInformation()))
  {
    result = NULL;
  }
  return result;
}

SYSTEMS_API LPVOID WINAPI GetImageProcAddress(HMODULE hModule, int index)
{
  TraceVerbose(L"%S(%p, %d)", __FUNCTION__, hModule, index);

  ASSERT(hModule != NULL);

  LPVOID result = NULL;

  __try
  {
    IMAGE_EXPORT_DIRECTORY *pImageExportDirectory;
    pImageExportDirectory = GetImageExportDirectory(hModule);
    if (pImageExportDirectory != NULL)
    {
      index -= pImageExportDirectory->Base;
      if ((index >= 0) && (index < (int) pImageExportDirectory->NumberOfFunctions))
      {
        DWORD addr = ((DWORD *) ((ULONG_PTR) hModule + pImageExportDirectory->AddressOfFunctions))[index];
        if (addr > 0)
          result = ExportToFunc(hModule, addr);
      }
    }
    if (result == NULL)
    {
      Trace(L"%S Failure - Using GetProcAddress", __FUNCTION__);
#pragma warning(disable: 4312)
      result = GetProcAddress(hModule, (LPCSTR) index);
#pragma warning(default: 4312)
    }
  }
  __except (ExceptionFilter(L"GetImageProcAddress - ordinal", GetExceptionInformation()))
  {
    result = NULL;
  }
  return result;
}

SYSTEMS_API BOOL WINAPI GetImageProcAddressesRaw(LPCWSTR ModuleFileName, PVOID ImageBaseAddress, LPCSTR *ApiNames, PVOID *ProcAddresses, int ApiCount)
{
  BOOL result = FALSE;
  HANDLE fh = CreateFile(ModuleFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
  if (fh != INVALID_HANDLE_VALUE)
  {
    HANDLE hMap = CreateFileMapping(fh, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMap != NULL)
    {
      LPVOID buffer = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
      if (buffer != NULL)
      {
        IMAGE_NT_HEADERS *pNtHeaders;
        pNtHeaders = GetImageNtHeaders((HMODULE) buffer);
        if (pNtHeaders != NULL)
        {
          IMAGE_EXPORT_DIRECTORY *pExportDirectory;
          DWORD virtualAddress;
          if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
            virtualAddress = ((PIMAGE_NT_HEADERS64) pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
          else
            virtualAddress = ((PIMAGE_NT_HEADERS32) pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
          pExportDirectory = (IMAGE_EXPORT_DIRECTORY *) ((ULONG_PTR) buffer + VirtualToRaw(pNtHeaders, virtualAddress));
          if (pExportDirectory != NULL)
          {
            DWORD *addressOfNames = (DWORD *) ((ULONG_PTR) buffer + VirtualToRaw(pNtHeaders, pExportDirectory->AddressOfNames));
            DWORD i;
            for (i = 0; i < pExportDirectory->NumberOfNames; i++)
            {
              DWORD nameOffset = addressOfNames[i];
              LPCSTR pName = (LPCSTR) ((ULONG_PTR) buffer + VirtualToRaw(pNtHeaders, nameOffset));
              for (DWORD i2 = 0; i2 < (DWORD) ApiCount; i2++)
                if (strcmp(pName, ApiNames[i2]) == 0)
                {
                  WORD ordinal = ((WORD *) ((ULONG_PTR) buffer + VirtualToRaw(pNtHeaders, pExportDirectory->AddressOfNameOrdinals)))[i];
                  DWORD functionOffset = ((DWORD *) ((ULONG_PTR) buffer + VirtualToRaw(pNtHeaders, pExportDirectory->AddressOfFunctions)))[ordinal];
                  ProcAddresses[i2] = (PVOID) ((ULONG_PTR) ImageBaseAddress + functionOffset);
                  result = TRUE;
                  break;
                }
            }
            if (result)
              for (i = 0; i < (DWORD) ApiCount; i++)
                if (!ProcAddresses[i])
                {
                  result = FALSE;
                  break;
                }
          }
        }
        UnmapViewOfFile(buffer);
      }
      CloseHandle(hMap);
    }
    CloseHandle(fh);
  }
  return result;
}

SYSTEMS_API BOOL WINAPI Is64bitModule(LPCWSTR FileName)
// find out and return whether the dll file is a 64bit dll
{
  BOOL result = FALSE;
  HANDLE fh = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);

  if (fh != INVALID_HANDLE_VALUE)
  {
    HANDLE section = CreateFileMapping(fh, NULL, PAGE_READONLY, 0, 0, NULL);

    if (section)
    {
      PVOID buf = MapViewOfFile(section, FILE_MAP_READ, 0, 0, 0);

      if (buf)
      {
        HANDLE moduleHandle = (HANDLE) buf;

        if (((PIMAGE_DOS_HEADER) moduleHandle)->e_magic == IMAGE_DOS_SIGNATURE)
        {
          // might be a valid image

          PIMAGE_NT_HEADERS nh = (PIMAGE_NT_HEADERS) ((ULONG_PTR) moduleHandle + ((PIMAGE_DOS_HEADER) moduleHandle)->e_lfanew);

          if (nh->Signature == IMAGE_NT_SIGNATURE)
          {
            // yep, it's really a valid image

            if (nh->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
              // and it's a 64bit dll
              result = TRUE;
          }
        }
        UnmapViewOfFile(buf);
      }
      CloseHandle(section);
    }
    CloseHandle(fh);
  }

  return result;
}

SYSTEMS_API BOOL WINAPI GetImageProcName(HMODULE hModule, LPVOID proc, LPSTR procName, DWORD procNameSize)
{
  TraceVerbose(L"%S(%p, %p, %p, %d)", __FUNCTION__, hModule, proc, procName, procNameSize);

  ASSERT(hModule != NULL);
  ASSERT(proc != NULL);

  BOOL result = false;

  __try
  {
    IMAGE_EXPORT_DIRECTORY *pImageExportDirectory;
    pImageExportDirectory = GetImageExportDirectory(hModule);
    if (pImageExportDirectory != NULL)
    {
      for (WORD i = 0; i < pImageExportDirectory->NumberOfFunctions; i++)
      {
        DWORD offset = ((DWORD *) ((ULONG_PTR) hModule + pImageExportDirectory->AddressOfFunctions))[i];
        LPVOID address = (LPVOID) ((ULONG_PTR) hModule + offset);
        if (address == proc)
        {
          for (DWORD j = 0; j < pImageExportDirectory->NumberOfNames; j++)
          {
            WORD ordinal = ((WORD *) ((ULONG_PTR) hModule + pImageExportDirectory->AddressOfNameOrdinals))[j];
            if (ordinal == i)
            {
              DWORD nameAddress = ((DWORD *) ((ULONG_PTR) hModule + pImageExportDirectory->AddressOfNames))[j];
              HRESULT hr = strcpy_s(procName, procNameSize, (LPCSTR) ((ULONG_PTR) hModule + nameAddress));
              result = hr == 0;
              break;
            }
          }
          if (!result)
          {
            sprintf_s(procName, procNameSize, "#%d", pImageExportDirectory->Base + i);
            result = true;
          }
          break;
        }
      }
    }
  }
  __except (ExceptionFilter(L"GetImageProcName", GetExceptionInformation()))
  {
    result = false;
  }
  return result;
}

SYSTEMS_API HMODULE WINAPI GetCallingModule(PVOID pReturnAddress)
{
  TraceVerbose(L"%S(void)", __FUNCTION__);

  HMODULE result = NULL;

  DWORD lastError;
  lastError = GetLastError();

  __try
  {
    MEMORY_BASIC_INFORMATION mbi;
    WORD w;
    if (!pReturnAddress)
      pReturnAddress = _ReturnAddress();
    if ( (VirtualQuery(pReturnAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION)) && 
         CMemoryMap::TryRead(mbi.AllocationBase, &w, 2) && (w == IMAGE_DOS_SIGNATURE) )
    {
      // the allocation base is the beginning of a PE file 
      result = (HMODULE) mbi.AllocationBase;
    }
    else
    {
      // The call may have come from a hook that we have allocated, and therefore won't be a PE file
      EnterCriticalSection(&gHookCriticalSection);
      __try
      {
        for (int i = g_pHookCollection->GetCount() - 1; i >= 0; i--)
        {
          if ( ((*g_pHookCollection)[i].pCodeHook != NULL) &&
               ((*g_pHookCollection)[i].pCodeHook->mpInUseCodeArray != NULL) &&
               (pReturnAddress >= (*g_pHookCollection)[i].pCodeHook->mpInUseCodeArray) &&
               (pReturnAddress <  (*g_pHookCollection)[i].pCodeHook->mpInUseCodeArray + IN_USE_COUNT * IN_USE_SIZE) )
          {
            // if returnAddress is inside the inUseArray of a hook, it points to an IN_USE code buffer
            //  this buffer contains the real return address
            DWORD index = (DWORD) (((ULONG_PTR) pReturnAddress - (ULONG_PTR) (*g_pHookCollection)[i].pCodeHook->mpInUseCodeArray) / IN_USE_SIZE);
            pReturnAddress = (*g_pHookCollection)[i].pCodeHook->mpInUseTargetArray[index];

            if ( (VirtualQuery(pReturnAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION)) &&
                 CMemoryMap::TryRead(mbi.AllocationBase, &w, 2) && (w == IMAGE_DOS_SIGNATURE) )
            {
              // the allocation base is the beginning of a PE file 
              result = (HMODULE) mbi.AllocationBase;
              break;
            }
          }
        }
      }
      __finally
      {
        LeaveCriticalSection(&gHookCriticalSection);
      }
    }
  }
  __except (ExceptionFilter(L"GetCallingModule", GetExceptionInformation()))
  {
    result = NULL;
  }
  SetLastError(lastError);
  return result;
}

SYSTEMS_API BOOL WINAPI FindModule(LPVOID pCode, HMODULE *phModule, LPSTR moduleName, DWORD moduleNameSize)
{
  TraceVerbose(L"%S(%p, %p, %p, %d)", __FUNCTION__, pCode, phModule, moduleName, moduleNameSize);

  ASSERT(pCode != NULL);
  ASSERT(phModule != NULL);
  ASSERT(moduleName != NULL);

  BOOL result = false;

  __try
  {
    MEMORY_BASIC_INFORMATION mbi;
    result = (VirtualQuery(pCode, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION)) &&
             (mbi.State == MEM_COMMIT) && (mbi.AllocationBase != NULL);
    if (result)
    {
      result = (GetModuleFileNameA((HMODULE) mbi.AllocationBase, moduleName, moduleNameSize) != 0);
      if (result)
      {
        *phModule = (HMODULE) mbi.AllocationBase;
      }
      else
        Trace(L"%S Failure - GetModuleFileName: %X", __FUNCTION__, GetLastError());
    }
    else
      Trace(L"%S Failure - VirtualQuery", __FUNCTION__);
  }
  __except (ExceptionFilter(L"FindModule", GetExceptionInformation()))
  {
    result = false;
  }
  return result;
}

typedef struct tagLdrModule
{
  LIST_ENTRY InLoadOrderModuleList;
  LIST_ENTRY InMemoryOrderModuleList;
  LIST_ENTRY InInitializationOrderModuleList;
  PVOID BaseAddress;
  PVOID EntryPoint;
  ULONG SizeOfImage;
  UNICODE_STRING FullDllName;
  UNICODE_STRING BaseDllName;
  ULONG Flags;
  USHORT LoadCount;
  USHORT TlsIndex;
  LIST_ENTRY HashTableEntry;
  ULONG TimeDateStamp;
} LDR_MODULE, *PLDR_MODULE;

SYSTEMS_API USHORT WINAPI GetModuleLoadCount(HMODULE hModule)
{
  HANDLE hProcess = GetCurrentProcess();
  PPEB pPeb = GetPeb(hProcess);

  if (pPeb != NULL)
  {
    SIZE_T bytesRead;

    PEB peb;
    if (ReadProcessMemory(hProcess, pPeb, &peb, sizeof(PEB), &bytesRead))
    {
      PPEB_NT pPebNt = (PPEB_NT) &peb;
      PEB_LDR_DATA ldrData;

      if (ReadProcessMemory(hProcess, (LPCVOID) pPebNt->LdrData, &ldrData, sizeof(ldrData), &bytesRead))
      {
        LDR_MODULE pebLdrModule;
        const void *pebLdrModuleAddress = (const void *) ldrData.InLoadOrderModuleList.Flink;
        while (ReadProcessMemory(hProcess, pebLdrModuleAddress, &pebLdrModule, sizeof(LDR_MODULE), &bytesRead))
        {
          if (pebLdrModule.BaseAddress == hModule)
            return pebLdrModule.LoadCount;
          pebLdrModuleAddress = (const void *) pebLdrModule.InLoadOrderModuleList.Flink;
        }
        return 0;
      }
    }
    else
      Trace(L"%S Failure - Read", __FUNCTION__);
  }
  else
    Trace(L"%S Failure - GetPeb", __FUNCTION__);
  return 0;
}

SYSTEMS_API IMAGE_NT_HEADERS* WINAPI GetImageNtHeaders(HMODULE hModule)
{
  TraceVerbose(L"%S(%p)", __FUNCTION__, hModule);

  ASSERT(hModule != NULL);

  IMAGE_NT_HEADERS *result = NULL;
  __try
  {
    if (!IsBadReadPtr2((LPVOID) hModule, 2))
    {
      if (*((WORD *) hModule) == IMAGE_DOS_SIGNATURE)
      {
        DWORD offset = *(DWORD *) ((ULONG_PTR) hModule + NT_HEADERS_OFFSET);
        result = (IMAGE_NT_HEADERS *) ((ULONG_PTR) hModule + offset);
        if (result->Signature != IMAGE_NT_SIGNATURE)
        {
          Trace(L"%S Failure - Signature not IMAGE_NT_SIGNATURE: %X", __FUNCTION__, result->Signature);
          result = NULL;
        }
      }
      else
        Trace(L"%S Failure - First 2 bytes not IMAGE_DOS_SIGNATURE: %X", __FUNCTION__, *((WORD *) hModule));
    }
    else
      Trace(L"%S Failure - Unable to read pointer %p", __FUNCTION__, hModule);
  }
  __except (ExceptionFilter(L"GetImageNtHeaders", GetExceptionInformation()))
  {
    result = NULL;
  }

  return result;
}

SYSTEMS_API LPVOID WINAPI ExportToFunc(HMODULE hModule, DWORD addr)
{
  TraceVerbose(L"%S(%p, %08X)", __FUNCTION__, hModule, addr);

  ASSERT(hModule != NULL);

  LPVOID result = NULL;

  IMAGE_NT_HEADERS *pNtHeaders = GetImageNtHeaders(hModule);
  if (pNtHeaders != NULL)
  {
    IMAGE_DATA_DIRECTORY imageDataDirectory;
    if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
      imageDataDirectory = ((PIMAGE_NT_HEADERS64) pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    else
      imageDataDirectory = ((PIMAGE_NT_HEADERS32) pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

    if ( (addr >= imageDataDirectory.VirtualAddress) && 
         (addr <  imageDataDirectory.VirtualAddress + imageDataDirectory.Size) )
    {
      // addr is a "forward" API thunk to another dll
      //   forwarded APIs don't point to the code section, but to the image data section instead
      //   they don't point to actual code, but to a name, e.g. "ntdll.ProtectVirtualMemory"
      char thunkName[MAX_PATH];
      HRESULT hr = strcpy_s(thunkName, MAX_PATH, (LPCSTR) ((ULONG_PTR) hModule + addr));
      if (hr == 0)
      {
        CHAR *moduleNameA = thunkName;
        CHAR *procNameA = thunkName;
        while (*procNameA != '.')
          procNameA++;
        *procNameA = '\0';
        procNameA++;

        wchar_t moduleName[MAX_PATH];
        SString::MultiByteToWideChar(moduleNameA, (int) strlen(moduleNameA) + 1, moduleName, MAX_PATH);
        result = GetImageProcAddress(GetModuleHandle(moduleName), procNameA);
      }
    }
    else
    {
      result = (LPVOID) ((ULONG_PTR) hModule + addr);
    }
  }
  else
    Trace(L"%S Failure - GetImageNtHeaders returned NULL", __FUNCTION__);

  return result;
}

SYSTEMS_API DWORD WINAPI VirtualToRaw(IMAGE_NT_HEADERS *pNtHeaders, DWORD addr)
{
  TraceVerbose(L"%S(%p, %08X)", __FUNCTION__, pNtHeaders, addr);

  ASSERT(pNtHeaders != NULL);

  DWORD result = addr;

  IMAGE_SECTION_HEADER *pImageSectionHeader;

  if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    pImageSectionHeader = (IMAGE_SECTION_HEADER *) ((ULONG_PTR) pNtHeaders + sizeof(IMAGE_NT_HEADERS64));
  else
    pImageSectionHeader = (IMAGE_SECTION_HEADER *) ((ULONG_PTR) pNtHeaders + sizeof(IMAGE_NT_HEADERS32));
  for (int i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
  {
    if ( (addr >= pImageSectionHeader[i].VirtualAddress) &&
         ((i == pNtHeaders->FileHeader.NumberOfSections - 1) || (addr < pImageSectionHeader[i + 1].VirtualAddress)) )
    {
      result = addr - pImageSectionHeader[i].VirtualAddress + pImageSectionHeader[i].PointerToRawData;
      break;
    }
  }
  return result;
}

SYSTEMS_API LPVOID WINAPI GetImageDataDirectory(HMODULE hModule, DWORD directory)
{
  TraceVerbose(L"%S(%p, %08x)", __FUNCTION__, hModule, directory);

  ASSERT(hModule != NULL);

  LPVOID pointer = NULL;
  IMAGE_NT_HEADERS *pNtHeaders;
  pNtHeaders = GetImageNtHeaders(hModule);
  if (pNtHeaders != NULL)
  {
    if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
      pointer = (LPVOID) ((ULONG_PTR) hModule + ((PIMAGE_NT_HEADERS64) pNtHeaders)->OptionalHeader.DataDirectory[directory].VirtualAddress);
    else
      pointer = (LPVOID) ((ULONG_PTR) hModule + ((PIMAGE_NT_HEADERS32) pNtHeaders)->OptionalHeader.DataDirectory[directory].VirtualAddress);
  }
  else 
    Trace(L"%S Failure - GetImageNtHeaders returned NULL", __FUNCTION__);

  return pointer;
}

SYSTEMS_API IMAGE_IMPORT_DESCRIPTOR* WINAPI GetImageImportDirectory(HMODULE hModule)
{
  TraceVerbose(L"%S(%p)", __FUNCTION__, hModule);

  ASSERT(hModule != NULL);

  return (IMAGE_IMPORT_DESCRIPTOR *) GetImageDataDirectory(hModule, IMAGE_DIRECTORY_ENTRY_IMPORT);
}

SYSTEMS_API IMAGE_EXPORT_DIRECTORY* WINAPI GetImageExportDirectory(HMODULE hModule)
{
  TraceVerbose(L"%S(%p)", __FUNCTION__, hModule);

  ASSERT(hModule != NULL);

  return (IMAGE_EXPORT_DIRECTORY *) GetImageDataDirectory(hModule, IMAGE_DIRECTORY_ENTRY_EXPORT);
}

SYSTEMS_API BOOL WINAPI GetExportDirectory(LPVOID pCode, HMODULE *phModule, IMAGE_EXPORT_DIRECTORY **pImageExportDirectory)
{
  TraceVerbose(L"%S(%p, %p, %p)", __FUNCTION__, pCode, phModule, pImageExportDirectory);

  ASSERT(pCode != NULL);
  ASSERT(phModule != NULL);
  ASSERT(pImageExportDirectory != NULL);

  BOOL result = false;

  MEMORY_BASIC_INFORMATION mbi;
  if (VirtualQuery(pCode, &mbi, sizeof(MEMORY_BASIC_INFORMATION)))
  {
    if ((mbi.State == MEM_COMMIT) && (mbi.AllocationBase != NULL))
    {
      wchar_t moduleFileName[8];
      if (GetModuleFileName((HMODULE) mbi.AllocationBase, moduleFileName, 2) != 0)
      {
        *phModule = (HMODULE) mbi.AllocationBase;
        IMAGE_NT_HEADERS *pNtHeaders = GetImageNtHeaders(*phModule);
        if (pNtHeaders != NULL)
        {
          ULONG virtualAddress;
          if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
            virtualAddress = ((PIMAGE_NT_HEADERS64) pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
          else
            virtualAddress = ((PIMAGE_NT_HEADERS32) pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
          *pImageExportDirectory = (IMAGE_EXPORT_DIRECTORY *) ((ULONG_PTR) *phModule + virtualAddress);
          result = true;
        }
        else
          Trace(L"%S Failure - GetImageNtHeaders returned NULL", __FUNCTION__);
      }
      else
        Trace(L"%S Failure - GetModuleFileName: %X", __FUNCTION__, GetLastError());
    }
    else
      Trace(L"%S Failure - mbi incorrect", __FUNCTION__);
  }
  else
    Trace(L"%S Failure - VirtualQuery: %X", __FUNCTION__, GetLastError());

  return result;
}


// Two different API hooking methods; the seconndary one is called "mixture mode"
// the mixture mode permanently changes the entry point of the to-be-hooked dll
// so ResolveMixtureMode makes sure that the changed API entry point is returned
// and not the old one; this is done by checking up the mixture mode memory
// mapped file for this API (if one exists)
BOOL WINAPI ResolveMixtureMode(LPVOID *pCode)
{
  TraceVerbose(L"%S(%p)", __FUNCTION__, pCode);

  ASSERT(pCode != NULL);

  BOOL result = FALSE;

  HANDLE hMap;

  char mapName[MAX_PATH];
  ApiSpecialName(CNamedBuffer, CMixPrefix, (LPVOID) *pCode, FALSE, mapName);

  hMap = OpenGlobalFileMapping(mapName, true);
  if (hMap != NULL)
  {
    LPVOID buffer = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (buffer != NULL)
    {
      *pCode = *(LPVOID *) buffer;
      VERIFY(UnmapViewOfFile(buffer));
      result = TRUE;
    }
    else
      Trace(L"%S Failure - MapViewOfFile: %X", __FUNCTION__, GetLastError());

    VERIFY(CloseHandle(hMap));
  }

  return result;
}

//---------------------------------------------------------------------------------------------

// Strings must be terminated with a 0x55 (encrypted NULL)
SYSTEMS_API LPCSTR WINAPI DecryptStr(LPCSTR string, LPSTR buffer, int bufferLength)
{
  ASSERT(buffer != NULL);
  ASSERT(string != NULL);

  if ((string != NULL) && (buffer != NULL))
  {
    for (int i = 0; i < bufferLength; i++)
    {
      buffer[i] = string[i] ^ 0x55;
      if (buffer[i] == 0x00)
        return buffer;
    }
  }

  Trace(L"%S Failure", __FUNCTION__);
  return NULL;
}

SYSTEMS_API HMODULE WINAPI GetKernel32Handle(void)
{
  static HMODULE hKernel32 = NULL;
  if (hKernel32 == NULL)
  {
    char buffer[32];
    wchar_t moduleName[32];
    if (SString::MultiByteToWideChar(DecryptStr(CKernel32, buffer, 32), -1, moduleName, 32) > 0)
      hKernel32 = GetModuleHandle(moduleName);
  }
  return hKernel32;
}

SYSTEMS_API HMODULE WINAPI GetKernelbaseHandle(void)
// in Windows 7 many kernel32.dll APIs are actually in kernelbase.dll
// for hooking purposes it's better to hook the kernelbase.dll APIs directly
{
  static HMODULE hKernelbase = NULL;
  if (hKernelbase == NULL)
  {
    char buffer[32];
    wchar_t moduleName[32];
    if (SString::MultiByteToWideChar(DecryptStr(CKernelbase, buffer, 32), -1, moduleName, 32) > 0)
      hKernelbase = GetModuleHandle(moduleName);
    if (hKernelbase == NULL)
      hKernelbase = (HMODULE) 1;
  }
  return hKernelbase;
}

SYSTEMS_API HMODULE WINAPI GetNtDllHandle(void)
{
  static HMODULE hNtDll = NULL;
  if (hNtDll == NULL)
  {
    hNtDll = GetModuleHandleW(L"ntdll.dll");
  }
  return hNtDll;
}

SYSTEMS_API LPVOID WINAPI KernelProc(LPCSTR name, BOOL doubleCheck)
{
  ASSERT(name != NULL);

  char buffer[64];
  LPVOID result = NULL;
  if ((ULONG_PTR) GetKernelbaseHandle() > 1)
    result = GetImageProcAddress(GetKernelbaseHandle(), DecryptStr(name, buffer, 64), doubleCheck);
  if (!result)
    result = GetImageProcAddress(GetKernel32Handle(), DecryptStr(name, buffer, 64), doubleCheck);

  return result;
}

SYSTEMS_API LPVOID WINAPI NtProc(LPCSTR name, BOOL doubleCheck)
{
  if (name == NULL)
  {
    IMAGE_NT_HEADERS *pImageNtHeaders;
    pImageNtHeaders = GetImageNtHeaders(GetNtDllHandle());
    if (pImageNtHeaders != NULL)
      return (LPVOID) (&(pImageNtHeaders->OptionalHeader.AddressOfEntryPoint));
    else
      return NULL;
  }
  else
  {
    char buffer[64];
    return GetImageProcAddress(GetNtDllHandle(), DecryptStr(name, buffer, 64), doubleCheck);
  }
}

SYSTEMS_API void WINAPI InitKernelProcs(void)
{
  static bool initialized = false;
  if (!initialized)
  {
    // Because I'm decrypting to a single static buffer, we must call GetKernel32Handle first and be sure
    // that the HMODULE has been resolved.  This way, when calling DecryptStr in KernelProc, the buffer will be
    // the function name, and not be overwritten with "kernel32.dll" on the first call.
    HMODULE hKernel = GetKernel32Handle();
    ASSERT(hKernel != NULL);
    if (hKernel != NULL)
    {
      pfnGetVersion = (PFN_GET_VERSION) KernelProc(CGetVersion, true);
      pfnGetLastError = (PFN_GET_LAST_ERROR) KernelProc(CGetLastError, true);
      pfnSetErrorMode = (PFN_SET_ERROR_MODE) KernelProc(CSetErrorMode, true);
      pfnGetCurrentProcessId = (PFN_GET_CURRENT_PROCESS_ID) KernelProc(CGetCurrentProcessId, true);
      pfnVirtualFree = (PFN_VIRTUAL_FREE) KernelProc(CVirtualFree, true);
      pfnLoadLibraryW = (PFN_LOAD_LIBRARY_W) KernelProc(CLoadLibraryW, true);
      pfnGetModuleHandleW = (PFN_GET_MODULE_HANDLE_W) KernelProc(CGetModuleHandleW, true);
      pfnOpenFileMappingA = (PFN_OPEN_FILE_MAPPING_A) KernelProc(COpenFileMappingA, true);
      pfnMapViewOfFile = (PFN_MAP_VIEW_OF_FILE) KernelProc(CMapViewOfFile, true);
      pfnUnmapViewOfFile = (PFN_UNMAP_VIEW_OF_FILE) KernelProc(CUnmapViewOfFile, true);
      pfnCloseHandle = (PFN_CLOSE_HANDLE) KernelProc(CCloseHandle, true);
      pfnFreeLibrary = (PFN_FREE_LIBRARY) KernelProc(CFreeLibrary, true);
      pfnEnterCriticalSection = (PFN_ENTER_CRITICAL_SECTION) NtProc(CRtlEnterCriticalSection, true);
      pfnLeaveCriticalSection = (PFN_LEAVE_CRITICAL_SECTION) NtProc(CRtlLeaveCriticalSection, true);
      initialized = true;
    }
  }
}
