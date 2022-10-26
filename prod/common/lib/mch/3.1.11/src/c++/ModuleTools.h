// ***************************************************************
//  ModuleTools.h             version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  module/dll related tools
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _MODULETOOLS_H
#define _MODULETOOLS_H

#undef EXTERN
#ifdef _MODULETOOLS_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN BOOL WINAPI ResolveMixtureMode(LPVOID *pCode);

#endif