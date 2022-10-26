// ***************************************************************
//  Patches.h                 version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  import/export table patching
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _PATCHES_H
#define _PATCHES_H

#undef EXTERN
#ifdef _PATCHES_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN void PatchMyImportTables(LPVOID pOld, LPVOID pNew);
EXTERN void CheckStructureOffsets();

#endif