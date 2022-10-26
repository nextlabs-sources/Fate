// ***************************************************************
//  DllMain.h                 version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  madCodeHook initialization/finalization
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _DLLMAIN_H
#define _DLLMAIN_H

enum ATTACH_TYPE
{
  staticLoad,
  dynamicLoad
};

enum DETACH_TYPE
{
  freeLibrary,
  processTermination
};

#undef EXTERN
#ifdef _DLLMAIN_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN bool gSystemDll
#ifdef _DLLMAIN_C
= false
#endif
;

EXTERN ATTACH_TYPE gAttachType;

EXTERN OSVERSIONINFOEX gOSVersionInfoEx;

#endif