// ***************************************************************
//  StubTools.cpp             version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  tools for dynamic code stub creation
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _STUBTOOLS_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

#pragma warning(disable: 4311)

// ----------------- Token Replacement Functions ----------------------------

SYSTEMS_API BOOL WINAPI ReplaceDword(LPVOID buffer, DWORD bufferSize, DWORD token, DWORD value)
{
  BOOL result = FALSE;
  for (DWORD i = 0; i < bufferSize - sizeof(DWORD) + 1; i++)
  {
    BYTE *p = (BYTE *) buffer;
    if (memcmp(&p[i], &token, sizeof(DWORD)) == 0)
    {
      memcpy(&p[i], &value, sizeof(DWORD));
      result = TRUE;
    }
  }
  return result;
}

SYSTEMS_API BOOL WINAPI ReplaceQword(LPVOID buffer, DWORD bufferSize, ULONG_PTR token, ULONG_PTR value)
{
  BOOL result = FALSE;
  for (DWORD i = 0; i < bufferSize - sizeof(ULONG_PTR) + 1; i++)
  {
    BYTE *p = (BYTE *) buffer;
    if (memcmp(&p[i], &token, sizeof(ULONG_PTR)) == 0)
    {
      memcpy(&p[i], &value, sizeof(ULONG_PTR));
      result = TRUE;
    }
  }
  return result;
}

// Replaces given token with a relative offset to given address
// The offset will be calculated from the offset of the token within the buffer.
SYSTEMS_API BOOL WINAPI ReplaceRelativeOffset(LPVOID buffer, DWORD bufferSize, DWORD token, LPVOID address)
{
   BOOL result = FALSE;
   for (DWORD i = 0; i < bufferSize - sizeof(DWORD) + 1; i++)
   {
      BYTE *p = (BYTE *) buffer;
      if (memcmp(&p[i], &token, sizeof(DWORD)) == 0)
      {
         // We found location of address
         DWORD offset = (DWORD) address - (DWORD) buffer - (i + sizeof(DWORD));
         memcpy(&p[i], &offset, sizeof(DWORD));
         result = TRUE;
      }
   }
   return result;
}