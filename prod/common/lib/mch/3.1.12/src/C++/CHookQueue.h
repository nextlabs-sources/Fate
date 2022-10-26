// ***************************************************************
//  CHookQueue.h              version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  fully automatic hooking queue
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _CHOOKQUEUE_H
#define _CHOOKQUEUE_H

const int CHookQueueInitialCapacity = 8;

typedef struct tagHookQueueRecord
{
  LPVOID pHookProc;
  LPVOID *ppNextHook;
} HOOK_QUEUE_RECORD;

typedef struct tagHookQueueHeader
{
  int ItemCount;
  int Capacity;
  HANDLE hMap;
  ABSOLUTE_JUMP *pPatchAddress;
  ABSOLUTE_JUMP OldCode;
  ABSOLUTE_JUMP NewCode;
} HOOK_QUEUE_HEADER;

class CHookQueue
{
  public:
    CHookQueue();
    ~CHookQueue();
    HANDLE Initialize(LPCSTR mapName, LPVOID hookedFunction, LPVOID pa, LPVOID pTramp, bool keepMapFile);
    HANDLE Initialize(HANDLE hMap, LPVOID *pTramp);

    void AddEntry(LPVOID pHookProc, LPVOID *ppNextHook);

    HANDLE MapHandle();
    int ItemCount();
    const ABSOLUTE_JUMP *PatchAddress() const;
    const ABSOLUTE_JUMP& OldCode() const;
    const ABSOLUTE_JUMP& NewCode() const;
    const HOOK_QUEUE_RECORD& operator[] (int index) const;

  protected:
    bool Allocate();

    SString mMapName;

    HANDLE mHMap;                          // Handle to map buffer.
    LPVOID *mpMapBuffer;                   // Pointer to a Pointer, this is stored in a 4 byte memory mapped file
    LPVOID mpBuffer;                       // Pointer to the buffer the holds the Queue data
    HOOK_QUEUE_HEADER *mpHookQueueHeader;  // Queue Header
    HOOK_QUEUE_RECORD *mpHookRecords;      // Queue Array
};

#endif