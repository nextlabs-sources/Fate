// ***************************************************************
//  CMemoryMap.cpp            version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  getting an overview over the current memory range
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _CMEMORYMAP_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

CMemoryMap::CMemoryMap() : mMemoryAreas()
{
  MEMORY_BASIC_INFORMATION mbi;

  BOOL readable;
  ULONG_PTR baseAddress = 0;

  // Processes have access to the Low 2GB (0x00000000 through 0x7FFFFFFF) of the Virtual address space
  // The VirtualQuery function provides information about a range of pages in the virtual address 
  // space of the calling process.

  // So, loop through process virtual address space
  while ((baseAddress < 0x80000000) && (VirtualQuery((LPCVOID) baseAddress, &mbi, sizeof(mbi)) == sizeof(mbi)))
  {
    // if committed and accessible, its readable
    readable = (mbi.State == MEM_COMMIT) && (mbi.Protect == PAGE_READONLY     ||
                                             mbi.Protect == PAGE_READWRITE    ||
                                             mbi.Protect == PAGE_EXECUTE      || 
                                             mbi.Protect == PAGE_EXECUTE_READ ||
                                             mbi.Protect == PAGE_EXECUTE_READWRITE);

    // If first record, or if readable status has changed 
    // Then we populate a new record
    if ((mMemoryAreas.GetCount() == 0) || (mMemoryAreas[mMemoryAreas.GetCount() - 1].Readable != readable))
    {
      MEMORY_AREA m;
      m.AreaBegin = (LPVOID) baseAddress;
      m.AreaEnd = 0;
      m.Readable = readable;
      mMemoryAreas.Add(m);
    }
    // whether a new record created, or not, we update the AreadEnd
    baseAddress += mbi.RegionSize;
    mMemoryAreas[mMemoryAreas.GetCount() - 1].AreaEnd = (LPVOID) baseAddress;
  }
}

CMemoryMap::~CMemoryMap()
{
}

int CMemoryMap::GetCount() const
{
  return mMemoryAreas.GetCount();
}

const MEMORY_AREA& CMemoryMap::operator[] (int index) const
{
  return mMemoryAreas[index];
}

BOOL CMemoryMap::CheckTryRead(LPCVOID pMem, int) const
{
  BOOL result = FALSE;

  int i1 = mMemoryAreas.GetCount() / 2;
  int i2 = (i1 + 2) / 2;
  BOOL b1 = FALSE;

  // This is a binary search
  while (i2 > 0)
  {
    // If we are before this segment, back up more
    if (pMem < mMemoryAreas[i1].AreaBegin)
    {
      i1 -= i2;
      if (i1 < 0)
        i1 = 0;
    }
    else if (pMem < mMemoryAreas[i1].AreaEnd)
    {
      result = mMemoryAreas[i1].Readable;
    }
    else
    {
      i1 += i2;
      if (i1 >= mMemoryAreas.GetCount())
        i1 = mMemoryAreas.GetCount() - 1;
    }
    if (b1)
      break;
    // Keep making increment smaller
    if (i2 == 1)
      b1 = TRUE;
    else
      i2 += ((i2 + 1) / 2);
  }

  return result;
}

BOOL CMemoryMap::TryRead(LPCVOID source, LPVOID destination, DWORD count, CMemoryMap *pMemoryMap)
{
  BOOL result = FALSE;
  if ((pMemoryMap == NULL) || pMemoryMap->CheckTryRead(source, count))
  {
    SIZE_T bytesRead;
    if (source != NULL)
      if (ReadProcessMemory(GetCurrentProcess(), source, destination, count, &bytesRead))
        result = (bytesRead == count);
  }
  return result;
}

BOOL CMemoryMap::TryWrite(LPCVOID source, LPVOID destination, DWORD count, CMemoryMap *pMemoryMap)
{
  BOOL result = FALSE;
  if ((pMemoryMap == NULL) || pMemoryMap->CheckTryRead(destination, count))
  {
    SIZE_T bytesWritten;
    if (source != NULL)
      if (WriteProcessMemory(GetCurrentProcess(), destination, source, count, &bytesWritten))
        result = (bytesWritten == count);
  }
  return result;
}
