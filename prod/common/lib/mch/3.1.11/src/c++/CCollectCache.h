// ***************************************************************
//  CCollectCache.h           version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  cache class to optimize hooking performance
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _CCOLLECTCACHE_H
#define _CCOLLECTCACHE_H

#ifndef _CCOLLECTION_H
  #include "CCollection.h"
#endif

typedef struct tagCollectCacheRecord
{
  DWORD ThreadId;
  int ReferenceCount;

  // in RAM a lot of patching can happen
  // sometimes madCodeHook has to make sure that it can get original unpatched information
  // for such purposes it opens the original exe/dll files on harddisk
  // in order to speed things up, we're caching the last opened file here
  HMODULE moduleOrg;   // which loaded exe/dll module is currently open?
  LPVOID moduleFile;   // where is the file mapped into memory?
} COLLECT_CACHE_RECORD;

class CCollectCache
{
  private:
    static CCollectCache *pInstance;

  public:
    static void AddReference(void);
    static void ReleaseReference(void);
    static void Initialize(void);
    static void Finalize(void);
    static LPVOID CacheModuleFileMap(HMODULE hModule);

  protected:
    CCollectCache();
    CCollectCache(const CCollectCache&);
    CCollectCache& operator = (const CCollectCache&);
    ~CCollectCache();

    static CCollectCache& Instance();

    int Open(HANDLE *hMutex, BOOL force);
    void Close(HANDLE hMutex);

    void AddReferenceInternal(void);
    void ReleaseReferenceInternal(void);

    LPVOID CacheModuleFileMapInternal(HMODULE hModule);

    // Add a record to the collection for given ThreadId
    int AddThread(DWORD threadId);

    // Collection of structures
    CCollection<COLLECT_CACHE_RECORD, CStructureEqualHelper<COLLECT_CACHE_RECORD>> mCollectCacheRecords;
};

#endif