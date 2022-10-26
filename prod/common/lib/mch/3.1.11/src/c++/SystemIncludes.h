// ***************************************************************
//  SystemIncludes.h          version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  includes the core OS and RTL includes common for all files
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _SYSTEM_INCLUDES_H
#define _SYSTEM_INCLUDES_H

#ifndef WINVER
  #define _WIN32_WINNT 0x0400
  #define WINVER 0x0400
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winnt.h>

#include <malloc.h>
#include <stdlib.h>
#include <Limits.h>
#include <stdio.h>
#include <intrin.h>

#include <crtdbg.h>
#ifdef _DEBUG
  #define ASSERT(stmt) _ASSERT(stmt)
  #define VERIFY(stmt) _ASSERT(stmt)
#else
  #define ASSERT(stmt) 
  #define VERIFY(stmt) stmt
#endif

#endif