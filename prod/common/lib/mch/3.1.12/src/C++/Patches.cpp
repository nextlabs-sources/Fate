// ***************************************************************
//  Patches.cpp               version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  import/export table patching
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _PATCHES_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

SYSTEMS_API LPVOID* WINAPI FindWs2InternalProcList(HMODULE hModule)
// Winsock2 internally has an additional list of some exported APIs
// we need to find this list and modify it, too
// or else Winsock2 will refuse to initialize, when we change the export table
{
  LPVOID *result = NULL;
  PIMAGE_NT_HEADERS nh = GetImageNtHeaders(hModule);
  PIMAGE_EXPORT_DIRECTORY pexp = GetImageExportDirectory(hModule);
  if ((nh) && (pexp))
  {
    PIMAGE_SECTION_HEADER ish = (PIMAGE_SECTION_HEADER) ((ULONG_PTR) nh + sizeof(IMAGE_NT_HEADERS));
    for (int i1 = 0; i1 < nh->FileHeader.NumberOfSections; i1++)
    {
      if (  (ish->Name[0] == '.') &&
           ((ish->Name[1] == 'd') || (ish->Name[1] == 'D')) &&
           ((ish->Name[2] == 'a') || (ish->Name[1] == 'A')) &&
           ((ish->Name[3] == 't') || (ish->Name[1] == 'T')) &&
           ((ish->Name[4] == 'a') || (ish->Name[1] == 'A')) &&
            (ish->Name[5] == 0  ) )
      {
        ULONG_PTR buf[10];
        for (int i2 = 0; i2 < 10; i2++)
          buf[i2] = (ULONG_PTR) hModule + ((DWORD *) ((ULONG_PTR) hModule + pexp->AddressOfFunctions))[i2];
        for (int i3 = 0; i3 < (int) (ish->SizeOfRawData - sizeof(buf)); i3++)
          if (!memcmp(buf, (LPVOID) ((ULONG_PTR) hModule + ish->VirtualAddress + i3), sizeof(buf)))
            return (LPVOID*) ((ULONG_PTR) hModule + ish->VirtualAddress + i3);
        break;
      }
      ish++;
    }
  }
  return result;
}

SYSTEMS_API BOOL WINAPI HackWinsock2IfNecessary(LPVOID *ws2procList, LPVOID pOld, LPVOID pNew)
{
  bool result = (!ws2procList);
  if (!result)
  {
    int i1 = 0;
    while (ws2procList[i1])
    {
      if (ws2procList[i1] == pOld)
      {
        DWORD op;
        if (VirtualProtect(ws2procList[i1], 4, PAGE_EXECUTE_READWRITE, &op))
        {
          ws2procList[i1] = pNew;
          result = true;
          VirtualProtect(ws2procList[i1], 4, op, &op);
        }
        break;
      }
      else
        i1++;
    }
  }
  return result;
}

SYSTEMS_API BOOL WINAPI PatchExportTable(HMODULE hModule, LPVOID pOld, LPVOID pNew, LPVOID *ws2procList)
{
  BOOL result = false;

  __try
  {
    IMAGE_EXPORT_DIRECTORY *pExport = GetImageExportDirectory(hModule);
    if (pExport != NULL)
    {
      for (DWORD i = 0; i < pExport->NumberOfFunctions; i++)
      {
        DWORD address = ((DWORD *) ((ULONG_PTR) hModule + pExport->AddressOfFunctions))[i];
        if (((ULONG_PTR) hModule + address) == (ULONG_PTR) pOld)
        {
          if (HackWinsock2IfNecessary(ws2procList, pOld, pNew))
          {
            DWORD *p = (DWORD *) &((DWORD *) ((ULONG_PTR) hModule + pExport->AddressOfFunctions))[i];
            DWORD op;
            if (VirtualProtect(p, 4, PAGE_EXECUTE_READWRITE, &op))
            {
              *p = (DWORD) ((ULONG_PTR) pNew - (ULONG_PTR) hModule);
              result = true;
              VirtualProtect(p, 4, op, &op);
            }
            else
              HackWinsock2IfNecessary(ws2procList, pNew, pOld);
          }
          break;
        }
      }
    }
  }
  __except (ExceptionFilter(L"PatchExportTable", GetExceptionInformation()))
  {
    result = false;
  }
  return result;
}

const LONG OFFSET_VIRTUAL_ADDRESS = FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
#define OFFSET_VIRTUAL_ADDRESS_LITERAL 0x80

const LONG OFFSET_SIZEOF_IMAGE = FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader.SizeOfImage);
#define OFFSET_SIZEOF_IMAGE_LITERAL 0x50

const LONG OFFSET_NAME = FIELD_OFFSET(IMAGE_IMPORT_DESCRIPTOR, Name);
#define OFFSET_NAME_LITERAL 0x0c

const LONG OFFSET_FIRST_THUNK = FIELD_OFFSET(IMAGE_IMPORT_DESCRIPTOR, FirstThunk);
#define OFFSET_FIRST_THUNK_LITERAL 0x10

const LONG IMAGE_IMPORT_DESCRIPTOR_SIZE = sizeof(IMAGE_IMPORT_DESCRIPTOR);

// The following can be used in DEBUG builds to assure offsets have not changed
// as the SDK/Windows changes.
void CheckStructureOffsets()
{
#ifndef _WIN64
  ASSERT(OFFSET_VIRTUAL_ADDRESS == OFFSET_VIRTUAL_ADDRESS_LITERAL);
  ASSERT(OFFSET_SIZEOF_IMAGE == OFFSET_SIZEOF_IMAGE_LITERAL);
  ASSERT(OFFSET_NAME == OFFSET_NAME_LITERAL);
  ASSERT(OFFSET_FIRST_THUNK == OFFSET_FIRST_THUNK_LITERAL);
#endif
}

SYSTEMS_API void WINAPI PatchImportTable(HMODULE hModule, LPVOID pOld, LPVOID pNew)
{
  if (*((WORD *) hModule) == IMAGE_DOS_SIGNATURE)
  {
    DWORD offset = *(DWORD *) ((ULONG_PTR) hModule + NT_HEADERS_OFFSET);
    IMAGE_NT_HEADERS *pNtHeaders = (IMAGE_NT_HEADERS *) ((ULONG_PTR) hModule + offset);
    if (pNtHeaders->Signature == IMAGE_NT_SIGNATURE)
    {
      DWORD virtualAddress = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
      if ((virtualAddress != 0) && (virtualAddress < pNtHeaders->OptionalHeader.SizeOfImage))
      {
        IMAGE_IMPORT_DESCRIPTOR *pImportDescriptor = (IMAGE_IMPORT_DESCRIPTOR *) ((ULONG_PTR) hModule + virtualAddress);
        if (pImportDescriptor != NULL)
        {
          while ( (pImportDescriptor->Name != NULL) && 
                  (pImportDescriptor->FirstThunk != 0) &&
                  (pImportDescriptor->FirstThunk < pNtHeaders->OptionalHeader.SizeOfImage) &&
                  ((ULONG_PTR) hModule + pImportDescriptor->FirstThunk != NULL) )
          {
            IMAGE_THUNK_DATA *pThunk = (IMAGE_THUNK_DATA *) ((ULONG_PTR) hModule + pImportDescriptor->FirstThunk);
            while (pThunk->u1.Function != NULL)
            {
#pragma warning(disable: 4312 4244)
              DWORD op;
              if ( ((LPVOID) pThunk->u1.Function == pOld) &&
                   (VirtualProtect((LPVOID) pThunk, sizeof(LPVOID), PAGE_EXECUTE_READWRITE, &op)) )
              {
                pThunk->u1.Function = (ULONG_PTR) pNew;
                VirtualProtect((LPVOID) pThunk, sizeof(LPVOID), op, &op);
              }
              pThunk = (IMAGE_THUNK_DATA *) ((ULONG_PTR) pThunk + sizeof(LPVOID));
#pragma warning(default: 4312 4244)
            }
            pImportDescriptor = (IMAGE_IMPORT_DESCRIPTOR *) ((ULONG_PTR) pImportDescriptor + IMAGE_IMPORT_DESCRIPTOR_SIZE);
          }
        }
      }
    }
  }
}

void WINAPI PatchAllImportTables(LPVOID pOld, LPVOID pNew, BOOL shared)
{
  // Declare ALL locals
  MEMORY_BASIC_INFORMATION mbi;
  HMODULE hModule = NULL;
  ULONG_PTR address = 0;
  wchar_t fileName[8];
  while (VirtualQuery((LPCVOID) address, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
  {
    if ((mbi.State == MEM_COMMIT) && (mbi.AllocationBase != NULL) && (mbi.AllocationBase != hModule))
    {
#pragma warning(disable: 4312)
      if ((!shared) && (mbi.AllocationBase >= (LPVOID) 0x80000000))
#pragma warning(default: 4312)
        break;
      hModule = (HMODULE) mbi.AllocationBase;
      if (GetModuleFileName(hModule, fileName, 2) != 0)
        PatchImportTable(hModule, pOld, pNew);
    }
    address += mbi.RegionSize;
  }
}

void PatchMyImportTables(LPVOID pOld, LPVOID pNew)
{
  PatchAllImportTables(pOld, pNew, true);
}

BOOL IsAlreadyKnown(LPVOID pCode)
{
  TraceVerbose(L"%S(%p)", __FUNCTION__, pCode);

  BOOL result = FALSE;
  char mapName[MAX_PATH];
  ApiSpecialName(CNamedBuffer, CMAHPrefix, pCode, FALSE, mapName);
  HANDLE hMap = OpenGlobalFileMapping(mapName, false);
  if (hMap != NULL)
  {
    result = TRUE;
    VERIFY(CloseHandle(hMap));
  }
  else
    Trace(L"%S Failure - OpenGlobalFileMapping([%S])", __FUNCTION__, mapName);

  return result;
}

BOOL CheckCode(LPVOID *pCode)
{
  TraceVerbose(L"%S(%p)", __FUNCTION__, *pCode);

  HMODULE hModule;
  IMAGE_EXPORT_DIRECTORY *pImageExportDirectory;

  BOOL result = ResolveMixtureMode(pCode) || IsAlreadyKnown(*pCode);
  if (!result)
  {
    if (GetExportDirectory(*pCode, &hModule, &pImageExportDirectory))
    {
      for (DWORD i = 0; i < pImageExportDirectory->NumberOfFunctions; i++)
      {
        DWORD functionAddress = ((DWORD *) ((ULONG_PTR) hModule + pImageExportDirectory->AddressOfFunctions))[i];
        if (*pCode == (LPVOID) ((ULONG_PTR) hModule + functionAddress))
        {
          result = TRUE;
          break;
        }
      }
      if (!result)
        Trace(L"%S Failure - Failed to find pCode", __FUNCTION__);
    }
    else
      Trace(L"%S Failure - GetExportDirectory", __FUNCTION__);
  }
  return result;
}

SYSTEMS_API LPVOID WINAPI FindRealCode(LPVOID pCode)
{
  TraceVerbose(L"%S(%p)", __FUNCTION__);

  LPVOID result = pCode;
  if (pCode != NULL)
  {
    if (CheckCode(&pCode))
      result = pCode;
    else
    {
      DebugTrace((L"%S - CheckCode is False", __FUNCTION__));
      if (*(WORD *) pCode == 0x25ff)
      {
        #ifdef _WIN64
          // RIP relative addressing
          pCode = *(LPVOID*) ((ULONG_PTR) pCode + 6 + *((DWORD *) ((ULONG_PTR) pCode + 2)));
        #else
          pCode = (LPVOID) (ULONG_PTR) (*(*((DWORD **) ((ULONG_PTR) pCode + 2))));
        #endif
        if (CheckCode(&pCode))
          result = pCode;
        else
        {
          // win9x debug mode
        }
      }
    }
  }
  return result;
}
