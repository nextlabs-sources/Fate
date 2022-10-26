// ***************************************************************
//  CCollectCache.cpp         version: 1.0.2  ·  date: 2012-04-03
//  -------------------------------------------------------------
//  cache class to optimize hooking performance
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2012 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2012-04-03 1.0.2 fixed memory leak
// 2011-03-26 1.0.1 (1) fixed format ansi/wide mismatch
//                  (2) added special handling for "wine"
// 2010-01-10 1.0.0 initial version

#define _CCOLLECTCACHE_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

#include "tlhelp32.h"

// -------------- Static Implementation ----------------- 

CCollectCache *CCollectCache::pInstance = NULL;

CCollectCache& CCollectCache::Instance()
{
  if (!pInstance)
    // probably InitializeMadCHook() not called !?
    Initialize();
  return *pInstance;
}

void CCollectCache::AddReference()
{
  if (pInstance)
    pInstance->AddReferenceInternal();
}

void CCollectCache::ReleaseReference()
{
  if (pInstance)
    pInstance->ReleaseReferenceInternal();
}

void CCollectCache::Initialize(void)
{
  if (!pInstance)
    pInstance = new CCollectCache();
}

void CCollectCache::Finalize(void)
{
  if (pInstance)
  {
    delete pInstance;
    pInstance = NULL;
  }
}

LPVOID CCollectCache::CacheModuleFileMap(HMODULE hModule)
{
  if (pInstance)
    return pInstance->CacheModuleFileMapInternal(hModule);
  else
    return NULL;
}

// ---------------- Singleton Instance Implementation -------------
CCollectCache::CCollectCache() : mCollectCacheRecords()
{
}

CCollectCache::~CCollectCache(void)
{
}

int CCollectCache::AddThread(DWORD threadId)
{
  COLLECT_CACHE_RECORD t;
  t.ThreadId = threadId;
  t.ReferenceCount = 0;
  t.moduleOrg = NULL;
  t.moduleFile = NULL;
  mCollectCacheRecords.Add(t);
  return mCollectCacheRecords.Find(t);
}

SYSTEMS_API LPVOID WINAPI NeedModuleFileMapEx(HMODULE hModule)
{
  return CCollectCache::CacheModuleFileMap(hModule);
}

int CCollectCache::Open(HANDLE *hMutex, BOOL force)
{
  ASSERT(hMutex != NULL);

  pfnNeedModuleFileMapEx = NeedModuleFileMapEx;

  // Only one thread of a process can enter at a time
  char buffer[16];
  DecryptStr(CMchMixCache, buffer, 16);
  SString mutexName;
  mutexName.Format(L"%S$%x", buffer, GetCurrentProcessId());
  *hMutex = CreateLocalMutex(mutexName);
  if (*hMutex)
  {
    DWORD rc = WaitForSingleObject(*hMutex, INFINITE);
    rc = rc;  // get around warning "C4189: 'rc' : local variable is initialized but not referenced" when building in release
    ASSERT(rc != WAIT_ABANDONED);
    ASSERT(rc != WAIT_FAILED);
  }
  // See if this thread is in the array
  int index = -1;
  for (int i = 0; i < mCollectCacheRecords.GetCount(); i++)
    if (mCollectCacheRecords[i].ThreadId == GetCurrentThreadId())
    {
      index = i;
      break;
    }
  // Create a new one
  if ((index == -1) && force)
    index = AddThread(GetCurrentThreadId());

  return index;
}

void CCollectCache::Close(HANDLE hMutex)
{
  ASSERT(hMutex != NULL);
  if (hMutex != NULL)
  {
    VERIFY(ReleaseMutex(hMutex));
    VERIFY(CloseHandle(hMutex));
  }
}

void CCollectCache::AddReferenceInternal()
{
  HANDLE hMutex;
  int index;

  index = Open(&hMutex, TRUE);
  __try
  {

    ASSERT(index != -1);

    if (index != -1)
      mCollectCacheRecords[index].ReferenceCount++;

  }
  __finally
  {
    Close(hMutex);
  }
}

LPVOID CCollectCache::CacheModuleFileMapInternal(HMODULE hModule)
{
  if (IsWine())
    return NULL;

  LPVOID result = NULL;
  LPVOID buf = NULL;
  HANDLE hMutex;
  int index = Open(&hMutex, false);
  __try
  {

    if (index != -1)
      if (mCollectCacheRecords[index].moduleOrg != hModule)
      {
        if (mCollectCacheRecords[index].moduleOrg)
        {
          buf = mCollectCacheRecords[index].moduleFile;
          mCollectCacheRecords[index].moduleOrg  = NULL;
          mCollectCacheRecords[index].moduleFile = NULL;
        }
      }
      else
        result = mCollectCacheRecords[index].moduleFile;

  }
  __finally
  {
    Close(hMutex);
  }

  if (index != -1)
  {
    if (buf)
      UnmapViewOfFile(buf);
    if (!result)
    {
      buf = NeedModuleFileMap(hModule);
      if (buf)
      {
        index = Open(&hMutex, false);
        __try
        {

          if (index != -1)
          {
            mCollectCacheRecords[index].moduleOrg  = hModule;
            mCollectCacheRecords[index].moduleFile = buf;
            result = buf;
          }
          else
            UnmapViewOfFile(buf);

        }
        __finally
        {
          Close(hMutex);
        }
      }
    }
  }

  return result;
}

void CCollectCache::ReleaseReferenceInternal(void)
{
  HANDLE hMutex;
  LPVOID buf = NULL;
  int index;

  index = Open(&hMutex, FALSE);
  __try
  {

    if (index != -1)
    {
      mCollectCacheRecords[index].ReferenceCount--;
      if (mCollectCacheRecords[index].ReferenceCount == 0)
      {
        buf = mCollectCacheRecords[index].moduleFile;
        VERIFY(mCollectCacheRecords.RemoveAt(index));
      }
    }
    if (buf)
      UnmapViewOfFile(buf);

  }
  __finally
  {
    Close(hMutex);
  }
}
