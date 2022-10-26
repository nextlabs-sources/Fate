// ***************************************************************
//  Hooking.h                 version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  hooking groundwork
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _HOOKING_H
#define _HOOKING_H

class CCodeHook;

typedef struct tagHookItem
{
  HMODULE hOwner;
  HMODULE hModule;
  char ModuleName[MAX_PATH];
  char ProcName[MAX_PATH];
  LPVOID pCode;
  CCodeHook *pCodeHook;
  LPVOID *pNextHook;
  LPVOID pCallback;
  DWORD Flags;
}HOOK_ITEM;

#undef EXTERN
#ifdef _HOOKING_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN BOOL OpenHooking(void);
EXTERN BOOL CloseHooking(void);

EXTERN LPVOID VirtualAlloc2(int size, LPVOID preferredAddress = NULL);

EXTERN bool gHookReady
#ifdef _HOOKING_C
= false
#endif
;

EXTERN CRITICAL_SECTION gHookCriticalSection;

EXTERN CCollection<HOOK_ITEM, CStructureEqualHelper<HOOK_ITEM>> *g_pHookCollection 
#ifdef _HOOKING_C
= NULL
#endif
;

#endif