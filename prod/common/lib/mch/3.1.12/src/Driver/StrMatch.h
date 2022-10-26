// ***************************************************************
//  StrMatch.h                version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  some utility functions for file/string matching
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-01-10 1.0.0 initial release

#ifndef _StrMatch_
#define _StrMatch_

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

// ********************************************************************

// splits up a string like e.g. L"C:\\Windows\\*.exe|Explorer.exe" into its elements
BOOLEAN SplitStrArray (LPCWSTR str, int strLen, LPWSTR **pathBuf, LPWSTR **nameBuf, int **pathLen, int **nameLen, int *count);

// C:\\Windows\*.exe  ->  path = C:\\Windows\*.exe; name = *.exe
// Explorer.exe       ->  path = NULL;              name = Explorer.exe
BOOLEAN SplitNamePath (LPWSTR str, LPWSTR *pathBuf, LPWSTR *nameBuf, int *pathLen, int *nameLen);

// does a string match with full "*" and "?" mask support
// the "fileMode" successfully matches e.g. a string "test" to a mask "test.*"
BOOLEAN StrMatch (LPWSTR str, LPWSTR mask, int strLen, int maskLen, BOOLEAN fileMode);

// does the supplied path/name match one of the items in the list?
BOOLEAN MatchStrArray (LPWSTR pathBuf, LPWSTR nameBuf, int pathLen, int nameLen, int count, LPWSTR *pathsBuf, LPWSTR *namesBuf, int *pathsLen, int *namesLen);

// ********************************************************************

#endif
