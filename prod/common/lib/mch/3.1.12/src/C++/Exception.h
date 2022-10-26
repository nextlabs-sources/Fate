// ***************************************************************
//  Exception.h               version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  exception handling
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#undef EXTERN
#ifdef _EXCEPTION_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN DWORD ExceptionFilter(LPCWSTR source, struct _EXCEPTION_POINTERS *ep);

#endif