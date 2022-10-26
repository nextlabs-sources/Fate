// ***************************************************************
//  StrMatch.c                version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  some utility functions for file/string matching
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-01-10 1.0.0 initial release

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

// ********************************************************************

BOOLEAN SplitStrArray(LPCWSTR str, int strLen, LPWSTR **pathBuf, LPWSTR **nameBuf, int **pathLen, int **nameLen, int *count)
// splits up a string like e.g. L"C:\\Windows\\*.exe|Explorer.exe" into its elements
{
  *count = 0;

  if ((!strLen) || (!str[0]))
  {
    *pathBuf = NULL;
    *nameBuf = NULL;
    *pathLen = NULL;
    *nameLen = NULL;
    return FALSE;
  }
  else
  {
    BOOLEAN result = FALSE;

    // first of all count how many sub strings there are
    int last = -1;
    int i1;
    for (i1 = 0; i1 < strLen; i1++)
      if (str[i1] == L'|')
      {
        if (i1 > last + 1)
          (*count)++;
        last = i1;
      }
    if (last < strLen - 1)
      (*count)++;

    if (!*count)
      // no valid sub strings
      return FALSE;

    // now let's allocate helper arrays for the sub strings
    *pathBuf = (LPWSTR*) ExAllocatePool(PagedPool, *count * sizeof(LPWSTR));
    *nameBuf = (LPWSTR*) ExAllocatePool(PagedPool, *count * sizeof(LPWSTR));
    *pathLen = (int*) ExAllocatePool(PagedPool, *count * sizeof(int));
    *nameLen = (int*) ExAllocatePool(PagedPool, *count * sizeof(int));

    // now let's fill the helper arrays
    *count = 0;
    last = -1;
    for (i1 = 0; i1 < strLen; i1++)
      if (str[i1] == L'|')
      {
        if (i1 > last + 1)
        {
          (*pathBuf)[*count] = (LPWSTR) &str[last + 1];
          (*pathLen)[*count] = i1 - last - 1;
          (*count)++;
        }
        last = i1;
      }
    if (last < strLen - 1)
    {
      (*pathBuf)[*count] = (LPWSTR) &str[last + 1];
      (*pathLen)[*count] = strLen - last - 1;
      (*count)++;
    }

    // C:\\Windows\*.exe  ->  path = C:\\Windows\*.exe; name = *.exe
    // Explorer.exe       ->  path = NULL;              name = Explorer.exe
    for (i1 = 0; i1 < *count; i1++)
    {
      int i2;
      (*nameBuf)[i1] = NULL;
      (*nameLen)[i1] = 0;
      for (i2 = (*pathLen)[i1] - 2; i2 >= 0; i2--)
        if ((*pathBuf)[i1][i2] == L'\\')
        {
          result = TRUE;
          (*nameBuf)[i1] = &((*pathBuf)[i1][i2 + 1]);
          (*nameLen)[i1] = (*pathLen)[i1] - i2 - 1;
          break;
        }
      if (!(*nameBuf)[i1])
      {
        (*nameBuf)[i1] = (*pathBuf)[i1];
        (*nameLen)[i1] = (*pathLen)[i1];
        (*pathBuf)[i1] = NULL;
        (*pathLen)[i1] = 0;
      }
    }

    return result;
  }
}

// ********************************************************************

BOOLEAN SplitNamePath(LPWSTR str, LPWSTR *pathBuf, LPWSTR *nameBuf, int *pathLen, int *nameLen)
// C:\\Windows\*.exe  ->  path = C:\\Windows\*.exe; name = *.exe
// Explorer.exe       ->  path = NULL;              name = Explorer.exe
{
  int len = (int) wcslen(str);
  int i1;
  for (i1 = len - 2; i1 >= 0; i1--)
    if (str[i1] == L'\\')
    {
      *pathBuf = str;
      *pathLen = len;
      *nameBuf = (LPWSTR) &str[i1 + 1];
      *nameLen = len - i1 - 1;
      return TRUE;
    }
  *pathBuf = NULL;
  *pathLen = 0;
  *nameBuf = str;
  *nameLen = len;
  return FALSE;
}

// ********************************************************************

BOOLEAN StrMatch(LPWSTR str, LPWSTR mask, int strLen, int maskLen, BOOLEAN fileMode)
// does a string match with full "*" and "?" mask support
// the "fileMode" successfully matches e.g. a string "test" to a mask "test.*"
{
  int strPos = 0, maskPos = 0;
  int strMem = 0, maskMem = 0;
  BOOLEAN asteriskActive = FALSE;

  if ((mask[0] == L'*') && (maskLen == 1))
    // the mask "*" always fits
    return TRUE;

  while ((maskPos < maskLen) || (strPos < strLen))
  {

    if (maskPos == maskLen)
      // the mask has run out, but there's still text
      return FALSE;

    if (strPos == strLen)
    {
      // the text has run out, but there's still mask
      // this can be ok, if the rest of the mask contains only "*" chars
      // it can also be ok in "fileMode", if the rest of the mask is e.g. ".*"
      if ((fileMode) && (mask[maskPos] == L'.') && (strLen) && (str[strLen - 1] != L'.'))
        maskPos++;
      while ((maskPos < maskLen) && (mask[maskPos] == L'*'))
        maskPos++;
      return (maskPos == maskLen);
    }

    switch (mask[maskPos])
    {
      case L'*' : // we found a "*" in the mask!
                  maskPos++;
                  if (maskPos < maskLen)
                  {
                    // there's still something in the mask after the "*"
                    // so we enter special mode and store current text/match positions
                    asteriskActive = TRUE;
                    strMem = strPos;
                    maskMem = maskPos;
                    break;
                  }
                  else
                    // the mask has ended with a "*"
                    // and there was no mismatch until yet
                    return TRUE;

      case L'?' : // we found a "?" in the mask, so we simply skip one char
                  strPos++;
                  maskPos++;
                  break;

      default   : if (mask[maskPos] == str[strPos])
                  {
                    // mask and text match
                    strPos++;
                    maskPos++;
                  }
                  else
                    if (asteriskActive)
                    {
                      // mask and text mismatch, but we are in special mode
                      // which means there was a "*" in the mask and
                      // we have yet to find out how many text chars the "*" represents
                      // we have to restart matching for every text char until the end of the text
                      strMem++;
                      strPos = strMem;
                      maskPos = maskMem;
                    }
                    else
                      // mask and text mismatch
                      return FALSE;
    }
  }

  // text and match both ran out at the same time without any mismatch
  return TRUE;
}

// ********************************************************************

BOOLEAN MatchStrArray(LPWSTR pathBuf, LPWSTR nameBuf, int pathLen, int nameLen, int count, LPWSTR *pathsBuf, LPWSTR *namesBuf, int *pathsLen, int *namesLen)
// does the supplied path/name match one of the items in the list?
{
  BOOLEAN result = FALSE;
  int i1;

  for (i1 = 0; i1 < count; i1++)
  {
    if ((pathBuf) && (pathsBuf[i1]))
      result = StrMatch(pathBuf, pathsBuf[i1], pathLen, pathsLen[i1], TRUE);
    else
      result = StrMatch(nameBuf, namesBuf[i1], nameLen, namesLen[i1], TRUE);
    if (result)
      break;
  }

  return result;
}

// ********************************************************************
