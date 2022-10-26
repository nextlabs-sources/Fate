// ***************************************************************
//  Exception.cpp             version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  exception handling
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _EXCEPTION_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

#include <eh.h>

DWORD ExceptionFilter(LPCWSTR source, struct _EXCEPTION_POINTERS *ep)
{
  if (source != NULL)
    Trace(L"%s - %s: %d", SECTION, source, ep->ExceptionRecord->ExceptionCode);

  return EXCEPTION_EXECUTE_HANDLER;
}
