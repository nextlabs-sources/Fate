// ***************************************************************
//  CCodeHook.h               version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  API hooking framework
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _CCODEHOOK_H
#define _CCODEHOOK_H

#include "CHookQueue.h"

#define PAGE_SIZE 4096

typedef struct tagHookEntry
{
  LPVOID  pHookProc;  // points to the hook proc
  LPVOID *pNextHook;  // points to a location that contains the pointer to the next hook
} HOOK_ENTRY;

typedef struct tagHookQueue
{
  int ItemCount;
  int Capacity;
  HANDLE hMap;
  ABSOLUTE_JUMP *pPatchAddress;
  ABSOLUTE_JUMP OldCode;
  ABSOLUTE_JUMP NewCode;
  HOOK_ENTRY *pItems;
} HOOK_QUEUE;

// Exception handling material
typedef int (WINAPI *PFN_RTL_DISPATCH_EXCEPTION_NEXT)(EXCEPTION_RECORD *pExceptionRecord, CONTEXT *pExceptionContext);
typedef LPVOID (WINAPI *PFN_ADD_VECTORED_EXCEPTION_HANDLER)(ULONG FirstHandler, PVECTORED_EXCEPTION_HANDLER pVectoredHandler);
typedef ULONG (WINAPI *PFN_REMOVE_VECTORED_EXCEPTION_HANDLER)(LPVOID VectoredHandlerHandle);

#undef EXTERN
#ifdef _CCODEHOOK_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN ULONG_PTR gOldEip
#ifdef _CCODEHOOK_C
 = 0
#endif
;
EXTERN ULONG_PTR gNewEip
#ifdef _CCODEHOOK_C
 = 0
#endif
;

EXTERN PFN_RTL_DISPATCH_EXCEPTION_NEXT pfnRtlDispatchExceptionNext
#ifdef _CCODEHOOK_C
 = NULL
#endif
;

EXTERN PFN_ADD_VECTORED_EXCEPTION_HANDLER pfnAddVectoredExceptionHandler
#ifdef _CCODEHOOK_C
 = NULL
#endif
;

EXTERN PFN_REMOVE_VECTORED_EXCEPTION_HANDLER pfnRemoveVectoredExceptionHandler
#ifdef _CCODEHOOK_C
 = NULL
#endif
;

int WINAPI RtlDispatchExceptionCallback (EXCEPTION_RECORD *pExceptionRecord, CONTEXT *pExceptionContext);
int WINAPI VectoredExceptionHandler (EXCEPTION_POINTERS *exceptionInfo);

// Code Hook Class
class SYSTEMS_API CCodeHook
{
  public:
    // Constructor
    CCodeHook(HMODULE hModule, LPCSTR apiName, LPVOID hookThisFunction,
              LPVOID callbackFunction, LPVOID *nextHook, DWORD flags);

    // Destructor
    virtual ~CCodeHook(void);

    void WritePatch(void);

    int IsInUse(void);

    _declspec(property(get = getIsValid)) bool IsValid;

    bool getIsValid()
    {
      return mValid;
    }

    IN_USE *mpInUseCodeArray;
    LPVOID *mpInUseTargetArray;
    bool mLeakUnhook;
    bool mDoubleHook;
    HANDLE mMapHandle;
    ABSOLUTE_JUMP *mPatchAddr;
    ABSOLUTE_JUMP mNewCode;
    ABSOLUTE_JUMP mOldCode;
    LPVOID *mpNextHook;
    INDIRECT_ABSOLUTE_JUMP *mpHookStub;
    LPVOID *mpHookStubTarget;

  protected:

    // Constructor helpers
    void Initialize(HMODULE hModule, LPCSTR apiName, LPVOID hookFunction,
                    LPVOID callbackFunction, LPVOID *nextHook, DWORD flags);
    BOOL DoNewHookPrep(DWORD flags, LPVOID *pa, DWORD *error, bool *keepMapFile);
    HANDLE InitializeQueue(HANDLE map, LPCSTR mapName, LPVOID p, bool keepMapFile);
    void Enqueue(LPVOID pHookProc, LPVOID *ppNextHook);

    BOOL CheckMap(HANDLE *hMap);

    BOOL InitRtlDispatchException(HANDLE& hMutex, PVOID& vectoredHandle, LPVOID oldEip, LPVOID newEip);
    void CloseRtlDispatchException(HANDLE& hMutex, PVOID vectoredHandle);
    LPVOID DoWritePatch(LPVOID p);

    CHookQueue *mpHookQueue;

    HMODULE mHModule;

    BOOL mSafeHooking;
    BOOL mNoImproveUnhook;
    bool mValid;
    bool mNewHook;
    bool mDestroying;
    bool mIsWinsock2;
    bool mPatchExportTable;

    LPVOID mpHookedFunction;
    LPVOID mpCallbackFunction;
    LPVOID mpShareCode;
    LPVOID mpTramp;

    #ifdef _WIN64
      LPVOID mpSparePage;
    #endif
};

#endif