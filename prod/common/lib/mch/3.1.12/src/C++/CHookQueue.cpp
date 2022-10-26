// ***************************************************************
//  CHookQueue.cpp            version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  fully automatic hooking queue
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _CHOOKQUEUE_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

extern LPVOID VirtualAlloc2(int size, LPVOID preferredAddress);

CHookQueue::CHookQueue(void) : mMapName()
{
  mHMap = NULL;
  mpMapBuffer = NULL;
  mpBuffer = NULL;
  mpHookQueueHeader = NULL;
  mpHookRecords = NULL;
}

CHookQueue::~CHookQueue()
{
}

// Initialize Hook Queue for a NEW hook
HANDLE CHookQueue::Initialize(LPCSTR mapName, LPVOID hookedFunction, LPVOID pa, LPVOID tramp, bool keepMapFile)
{
  TraceVerbose(L"%S(%p, %p, %p, %p)", __FUNCTION__, mapName, hookedFunction, pa, tramp);

  mMapName = mapName;

  mHMap = CreateLocalFileMapping(mMapName, sizeof(LPVOID));
  if (mHMap != NULL)
  {
    // We put in a named object the pointer to our data
    mpMapBuffer = (LPVOID *) MapViewOfFile(mHMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (mpMapBuffer != NULL)
    {
      Allocate();

      // Initialize header

      // Why is pPatchAddress defined as an ABOSLUTE_JUMP *?
      mpHookQueueHeader->pPatchAddress = (ABSOLUTE_JUMP *) pa;

      if (!keepMapFile)
        mpHookQueueHeader->hMap = mHMap;
      else
        mpHookQueueHeader->hMap = NULL;

      mpHookQueueHeader->NewCode = CAbsoluteJump;
      #ifdef _WIN64
        // caution: we need to use RIP relative addressing!
        ULONG_PTR newTarget = (ULONG_PTR) tramp + PAGE_SIZE - ((ULONG_PTR) mpHookQueueHeader->pPatchAddress + 6);
        mpHookQueueHeader->NewCode.Target = (DWORD) newTarget;
        ASSERT((int) mpHookQueueHeader->NewCode.Target == (LONG_PTR) newTarget);
      #else
        mpHookQueueHeader->NewCode.Target = (DWORD) ((ULONG_PTR) tramp + PAGE_SIZE);
      #endif
      *((LPVOID *) ((ULONG_PTR) tramp + PAGE_SIZE)) = tramp;

      mpHookQueueHeader->OldCode = *mpHookQueueHeader->pPatchAddress;

      // Add initial entries
      AddEntry(NULL, (LPVOID *) ((ULONG_PTR) tramp + PAGE_SIZE));
      AddEntry(tramp, NULL);
    }
    else
      Trace(L"%S Failure - MapViewOfFile: %X", __FUNCTION__, GetLastError());
  }
  else
    Trace(L"%S Failure - CreateLocalFileMapping", __FUNCTION__);
  return mHMap;
}

// Initialize a hook queue for an existing hook where the hMap is already known
HANDLE CHookQueue::Initialize(HANDLE hMap, LPVOID *pTramp)
{
  TraceVerbose(L"%S(%p, %p)", __FUNCTION__, hMap, pTramp);

  HANDLE result = NULL;

  ASSERT(hMap != NULL);
  ASSERT(pTramp != NULL);

  mpMapBuffer = (LPVOID *) MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (mpMapBuffer != NULL)
  {
    mpBuffer = *mpMapBuffer;

    mpHookQueueHeader = (HOOK_QUEUE_HEADER *) mpBuffer;
    mpHookRecords = (HOOK_QUEUE_RECORD *) ((ULONG_PTR) mpBuffer + sizeof(HOOK_QUEUE_HEADER));

    if (pTramp != NULL)
      *pTramp = mpHookRecords[mpHookQueueHeader->ItemCount - 1].pHookProc;

    result = mpHookQueueHeader->hMap;
  }
  else
    Trace(L"%S Failure - MapViewOfFile: %X", __FUNCTION__, GetLastError());
  return result;
}


bool CHookQueue::Allocate(void)
{
  if (mpMapBuffer == NULL)
    return false;

  int capacity;
  int count;
  if (mpBuffer == NULL)
  {
    capacity = CHookQueueInitialCapacity;
    count = 0;
  }
  else
  {
    capacity = mpHookQueueHeader->Capacity * 2;
    count = mpHookQueueHeader->ItemCount;
  }

  DWORD headerSize = sizeof(HOOK_QUEUE_HEADER);
  DWORD arraySize = capacity * sizeof(HOOK_QUEUE_RECORD);

  *mpMapBuffer = VirtualAlloc2(headerSize + arraySize);
  DWORD op;
  VirtualProtect(*mpMapBuffer, headerSize + arraySize, PAGE_READWRITE, &op);

  // If there are any existing items, copy the previous buffer
  if (mpBuffer != NULL)
  {
    MoveMemory(*mpMapBuffer, mpBuffer, headerSize + mpHookQueueHeader->Capacity * sizeof(HOOK_QUEUE_RECORD));
    VirtualFree(mpBuffer, 0, MEM_RELEASE);
  }

  mpBuffer = *mpMapBuffer;
  mpHookQueueHeader = (HOOK_QUEUE_HEADER *) mpBuffer;
  ULONG_PTR records = (ULONG_PTR) mpBuffer + headerSize;
  mpHookRecords = (HOOK_QUEUE_RECORD *) ((LPVOID) records);
  mpHookQueueHeader->Capacity = capacity;
  mpHookQueueHeader->ItemCount = count;

  return true;
}

void CHookQueue::AddEntry(LPVOID pHookProc, LPVOID *ppNextHook)
{
  if (mpHookQueueHeader->ItemCount == mpHookQueueHeader->Capacity)
  {
    Allocate();
  }

  int count = mpHookQueueHeader->ItemCount;
  if ((count == 0) || (count == 1))
  {
    mpHookRecords[count].pHookProc = pHookProc;
    mpHookRecords[count].ppNextHook = ppNextHook;
  }
  else
  {
    mpHookRecords[count] = mpHookRecords[count - 1];
    mpHookRecords[count - 1].pHookProc = pHookProc;
    mpHookRecords[count - 1].ppNextHook = ppNextHook;
    *(mpHookRecords[count - 1].ppNextHook) = mpHookRecords[count].pHookProc;
    *(mpHookRecords[count - 2].ppNextHook) = mpHookRecords[count - 1].pHookProc;
  }
  mpHookQueueHeader->ItemCount += 1;
}

HANDLE CHookQueue::MapHandle()
{
  return mpHookQueueHeader->hMap;
}

int CHookQueue::ItemCount()
{
  return mpHookQueueHeader->ItemCount;
}

const ABSOLUTE_JUMP *CHookQueue::PatchAddress() const
{
  return mpHookQueueHeader->pPatchAddress;
}

const ABSOLUTE_JUMP& CHookQueue::OldCode() const
{
  return mpHookQueueHeader->OldCode;
}
      
const ABSOLUTE_JUMP& CHookQueue::NewCode() const
{
  return mpHookQueueHeader->NewCode;
}

const HOOK_QUEUE_RECORD& CHookQueue::operator[] (int index) const
{
  ASSERT((index >= 0) && (index < mpHookQueueHeader->ItemCount));

  return mpHookRecords[index];
}