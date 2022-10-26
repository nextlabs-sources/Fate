// ***************************************************************
//  Inject.cpp               version: 1.0.12  ·  date: 2016-03-16
//  -------------------------------------------------------------
//  injection into already running processes
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2016 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2016-03-16 1.0.12 (1) new process dll inject now always done in main thread
//                   (2) fixed some PAGE_EXECUTE_READWRITE security issues
//                   (3) dll injection loader lock improvement
// 2015-09-10 1.0.11 improved thread protection for multiple injections
// 2014-10-26 1.0.10 fixed: CreateProcessEx for x64 processes sometimes failed
// 2014-05-05 1.0.9  fixed: 32bit injection problems when compiled as 32bit
// 2013-12-03 1.0.8  (1) fixed: CreateProcessEx failed for .Net processes
// 2013-10-01 1.0.7  (1) added support for Windows 8.1
//                   (2) revert aligned UNICODE_STRING (compatability problems)
// 2013-05-13 1.0.6  (1) aligned UNICODE_STRING in internal structure
//                   (2) "driver only" injection now works without admin rights
// 2013-03-13 1.0.5  fixed: injecting multiple 32bit dlls in x64 OS crashed
// 2012-06-12 1.0.4  fixed: CreateProcessEx didn't always work in XP64
// 2012-05-21 1.0.3  (1) improved 64bit injection into already running processes
//                   (2) added support for non-large-address-aware x64 processes
//                   (3) fixed bug in InstallInjectThreadProc()
//                   (4) fixed CreateProcessEx for win8 x64 processes
// 2011-05-17 1.0.2  fixed crash in XP/03 x64 when uninjection a non-existing dll
// 2010-03-28 1.0.1  (1) fixed format ansi/wide mismatch
//                   (2) small change to make (un)injection slightly more stable
//                   (3) wow64 (un)injection now includes static link trick
//                   (4) fixed: crash when injecting wow64 from 32bit + 64bit exe
// 2010-01-10 1.0.0  initial version

#define _INJECT_C

#define _CRT_SECURE_NO_WARNINGS

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

#pragma warning(disable: 4505)

// use ntdll entry point patching in XP and higher?
// advantage: dll gets initialized even before the statically linked dlls are
// disadvantage: dll can't be uninjected anymore
//#define InjectLibraryPatchXp

static bool LibraryExists(LPCWSTR libraryName, SString& libraryPath, LPWSTR path = NULL, bool removeName = false);
static bool PathRemoveFileSpec(LPWSTR path);
static LPWSTR PathCombine(LPWSTR dest, LPCWSTR dir, LPCWSTR file);

static bool DoInject(LPCWSTR libraryName, HANDLE hProcess, DWORD processId, HANDLE firstThread, HANDLE *hThread, bool mayPatch, bool mayThread, bool forcePatch);
static int InjectLibraryPatch(LPCWSTR libraryName, ULONG_PTR pPeb32, ULONGLONG pPeb64, HANDLE hProcess, DWORD pid, HANDLE hThread, bool forcePatch);
static bool GetExeModuleInfos(ULONG_PTR pPeb32, ULONGLONG pPeb64, HANDLE hProcess, HMODULE *pModule, IMAGE_NT_HEADERS32 *pNtHeaders, LPVOID *dd);
static bool CheckModule(HANDLE hProcess, HMODULE hModule, IMAGE_NT_HEADERS32 *pNtHeaders, LPVOID *dd);

static HANDLE StartInjectLibraryX(LPCWSTR libraryName, HANDLE hProcess, DWORD processId, bool inject);
static bool WaitForInjectLibraryX(HANDLE hThread, DWORD timeOut, DWORD time);
static DWORD TickDif(DWORD tick);
static LPVOID InstallInjectThreadProc(HANDLE hProcess, DWORD processId);
static int __stdcall InjectThread(INJECT_REC *pInjectRec);
static BOOL Read(HANDLE hProcess, LPCVOID baseAddress, LPVOID pBuffer, SIZE_T length);
static BOOL Read(HANDLE hProcess, DWORD baseAddress, LPVOID pBuffer, SIZE_T length);

BOOLEAN SplitStrArray(LPCWSTR str, LPWSTR **pathBuf, LPWSTR **nameBuf, int **pathLen, int **nameLen, int *count, LPWSTR item1, LPWSTR item2)
// splits up a string like e.g. L"C:\\Windows\\*.exe|Explorer.exe" into its elements
{
  *count = 0;

  if ( ((!str) || (!str[0])) && (!item1) && (!item2) )
    return FALSE;

  BOOLEAN result = FALSE;

  // first of all count how many sub strings there are
  int last = -1;
  int len = (int) lstrlenW(str);
  int i1;
  for (i1 = 0; i1 < len; i1++)
    if (str[i1] == L'|')
    {
      if (i1 > last + 1)
        (*count)++;
      last = i1;
    }
  if (last < len - 1)
    (*count)++;
  if (item1)
    (*count)++;
  if (item2)
    (*count)++;

  if (!*count)
    // no valid sub strings
    return FALSE;

  // now let's allocate helper arrays for the sub strings
  *pathBuf = (LPWSTR*) LocalAlloc(LPTR, *count * sizeof(LPWSTR));
  *nameBuf = (LPWSTR*) LocalAlloc(LPTR, *count * sizeof(LPWSTR));
  *pathLen = (int*) LocalAlloc(LPTR, *count * sizeof(int));
  *nameLen = (int*) LocalAlloc(LPTR, *count * sizeof(int));

  // now let's fill the helper arrays
  *count = 0;
  last = -1;
  for (i1 = 0; i1 < len; i1++)
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
  if (last < len - 1)
  {
    (*pathBuf)[*count] = (LPWSTR) &str[last + 1];
    (*pathLen)[*count] = len - last - 1;
    (*count)++;
  }
  if (item1)
  {
    (*pathBuf)[*count] = (LPWSTR) item1;
    (*pathLen)[*count] = lstrlenW(item1);
    (*count)++;
  }
  if (item2)
  {
    (*pathBuf)[*count] = (LPWSTR) item2;
    (*pathLen)[*count] = lstrlenW(item2);
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

bool InternalInjectLibrary(HMODULE hOwner,
                           LPCWSTR libraryName,
                           bool    multiInject,
                           HANDLE  hProcess,
                           LPCWSTR driverName,
                           DWORD   session,
                           BOOL    system,
                           LPCWSTR includes,
                           LPCWSTR excludes,
                           PULONG  excludePIDs,
                           DWORD   timeOut,
                           bool    mayPatch,
                           bool    mayThread,
                           bool    forcePatch,
                           HANDLE  thread)
{
  TraceVerbose(L"%S(%X, %d, %X, %p, %d, %d, %d)", __FUNCTION__, hOwner, session, hProcess, libraryName, timeOut, mayPatch, mayThread);

  ASSERT(libraryName != NULL);

  bool result = false;

  // Determine fully qualified library path
  SString libraryPath;
  if (!CheckLibFilePath(libraryName, libraryPath))
    return FALSE;

  // Make sure that all privileges are enabled
  EnableAllPrivileges();

  // Assure that the function pointers are initialized
  InitKernelProcs();

  BOOL dll64 = Is64bitModule(libraryPath.GetBuffer());

  if (multiInject)
  {
    DebugTrace((L"%S - Multi Injection", __FUNCTION__));

    #ifndef _WIN64
      if (((GetMadCHookOption(INJECT_INTO_RUNNING_PROCESSES)) || (!Is64bitOS())) && (dll64))
      {
        SetLastError(ERROR_NOT_SUPPORTED);
        return false;
      }
    #endif

    if ((GetMadCHookOption(INJECT_INTO_RUNNING_PROCESSES)) && (!IsAdminAndElevated()))
    {
      SetLastError(ERROR_ACCESS_DENIED);
      return false;
    }

    if (!StartDllInjection(driverName, libraryPath.GetBuffer(), session, system, includes, excludes))
      return false;

    if (!GetMadCHookOption(INJECT_INTO_RUNNING_PROCESSES))
      return true;

    if (session == CURRENT_SESSION)
      session = GetCurrentSessionId();

    result = TRUE;

    char smssAnsi[16], slsvcAnsi[16];
    WCHAR smssWide[16], slsvcWide[16];
    DecryptStr( CSmss,  smssAnsi, 16);
    DecryptStr(CSLSvc, slsvcAnsi, 16);
    AnsiToWide(smssAnsi, smssWide);
    AnsiToWide(slsvcAnsi, slsvcWide);

    BOOLEAN needFullPath = false;
    LPWSTR helperBuf = NULL;
    LPWSTR lowCaseIncludes = NULL, lowCaseExcludes = NULL;
    LPWSTR *incPathBuf, *excPathBuf;
    LPWSTR *incNameBuf, *excNameBuf;
    int *incPathLen, *excPathLen;
    int *incNameLen, *excNameLen;
    int incCount, excCount;

    if (includes)
    {
      lowCaseIncludes = (LPWSTR) LocalAlloc(LPTR, lstrlenW(includes) * 2 + 2);
      lstrcpyW(lowCaseIncludes, includes);
      _wcslwr(lowCaseIncludes);
    }
    if (excludes)
    {
      lowCaseExcludes = (LPWSTR) LocalAlloc(LPTR, lstrlenW(excludes) * 2 + 2);
      lstrcpyW(lowCaseExcludes, excludes);
      _wcslwr(lowCaseExcludes);
    }
    needFullPath = SplitStrArray(lowCaseIncludes, &incPathBuf, &incNameBuf, &incPathLen, &incNameLen, &incCount, NULL, NULL);
    needFullPath = SplitStrArray(lowCaseExcludes, &excPathBuf, &excNameBuf, &excPathLen, &excNameLen, &excCount, smssWide, slsvcWide) || needFullPath;
    if (needFullPath)
      helperBuf = (LPWSTR) LocalAlloc(LPTR, 64 * 1024);

    bool b2;
    bool delay = false;
    int count = 0;
    DWORD time = 0;
    CEnumProcesses *prcs1 = NULL;
    CEnumProcesses *prcs2 = NULL;
    CCollection<HANDLE, CStructureEqualHelper<HANDLE>> ths;
    do
    {
      if (delay)
        Sleep(10);

      delay = false;
      b2 = true;
      prcs1 = new CEnumProcesses(false);
      ths.RemoveAll();

      for (int i1 = (*prcs1).GetCount() - 1; i1 >= 0; i1--)
      {
        bool b1 = true;
        if ((b1) && (prcs2 != NULL))
          for (int i2 = 0; i2 < (*prcs2).GetCount(); i2++)
            if ((*prcs2)[i2].Id == (*prcs1)[i1].Id)
            {
              b1 = false;
              break;
            }
        if (excludePIDs)
        {
          int i2 = 0;
          while ((excludePIDs[i2]) && (excludePIDs[i2] != (*prcs1)[i1].Id))
            i2++;
          if (excludePIDs[i2])
            b1 = false;
        }
        if (b1)
        {
          LPWSTR pathBuf, nameBuf;
          int pathLen, nameLen;
          _wcslwr((*prcs1)[i1].ExeFile);
          if ( (!SplitNamePath((*prcs1)[i1].ExeFile, &pathBuf, &nameBuf, &pathLen, &nameLen)) &&
               (needFullPath) &&
               (ProcessIdToFileNameW((*prcs1)[i1].Id, helperBuf, 32 * 1024)) )
          {
            _wcslwr(helperBuf);
            SplitNamePath(helperBuf, &pathBuf, &nameBuf, &pathLen, &nameLen);
          }
          if ( ((!incCount) || (MatchStrArray(pathBuf, nameBuf, pathLen, nameLen, incCount, incPathBuf, incNameBuf, incPathLen, incNameLen))) &&
               (!MatchStrArray(pathBuf, nameBuf, pathLen, nameLen, excCount, excPathBuf, excNameBuf, excPathLen, excNameLen)) )
          {
            b2 = false;
            HANDLE ph = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION |
                                    PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, false, (*prcs1)[i1].Id);
            if (ph != NULL)
            {
              BOOL process64 = Is64bitProcess(ph);
              #ifndef _WIN64
                if (!process64)
              #endif
              if (dll64 == process64)
                if ( ( (system) || (!IsSystemProcess(ph, (PSID) (*prcs1)[i1].UserSID)) ) &&
                     ( (session == ALL_SESSIONS) ||
                       (GetProcessSessionId((*prcs1)[i1].Id) == session) ||
                       ((GetProcessSessionId((*prcs1)[i1].Id) == 0) && IsSystemProcess(ph, (PSID) (*prcs1)[i1].UserSID)) ) )
                {
                  HANDLE th;
                  DoInject(libraryPath.GetBuffer(), ph, (*prcs1)[i1].Id, thread, &th, mayPatch, mayThread, forcePatch);
                  if (th != NULL)
                    ths.Add(th);
                }
              CloseHandle(ph);
            }
            else
            {
              if (((*prcs1)[i1].Id != NULL) && (GetLastError() != ERROR_ACCESS_DENIED))
              {
                (*prcs1).RemoveAt(i1);
                delay = true;
              }
            }
          }
        }
      }
      if (time == 0)
        time = GetTickCount();
      for (int i1 = 0; i1 < ths.GetCount(); i1++)
        if (ths[i1] != NULL)
          WaitForInjectLibraryX(ths[i1], timeOut, time);
      if (prcs2 != NULL)
        delete prcs2;
      prcs2 = prcs1;
      prcs1 = NULL;
      count++;
    } while ((!b2) && (count != 5));

    if (helperBuf)
      LocalFree(helperBuf);
    if (lowCaseIncludes)
      LocalFree(lowCaseIncludes);
    if (lowCaseExcludes)
      LocalFree(lowCaseExcludes);
    if (incCount)
    {
      LocalFree(incPathBuf);
      LocalFree(incNameBuf);
      LocalFree(incPathLen);
      LocalFree(incNameLen);
    }
    if (excCount)
    {
      LocalFree(excPathBuf);
      LocalFree(excNameBuf);
      LocalFree(excPathLen);
      LocalFree(excNameLen);
    }

    delete prcs2;
  }
  else
  {
    DebugTrace((L"%S - Direct Injection", __FUNCTION__));

    HANDLE hThread;
    DWORD processId = ProcessHandleToId(hProcess);
    BOOL process64 = Is64bitProcess(hProcess);

    #ifndef _WIN64
      if (forcePatch || (!process64))
    #endif
    if (dll64 == process64)
      if (DoInject(libraryPath.GetBuffer(), hProcess, processId, thread, &hThread, mayPatch, mayThread, forcePatch))
      {
        if ((hThread == NULL) || WaitForInjectLibraryX(hThread, timeOut, GetTickCount()))
          result = true;
        else
          Trace(L"%S Failure - WaitForInjectLibraryX", __FUNCTION__);
      }
      else
        Trace(L"%S Failure - DoInject", __FUNCTION__);
  }
  return result;
}

#pragma warning(disable: 4100)

BOOL CheckSkipUninject(HANDLE hProcess, LPCWSTR libraryName, BOOL is64bit)
// XP / 2003 x64 eventually crash when trying to uninject a non-existing dll
// so in these OSs we skip uninjection when the dll is not injected
{
  #ifndef _WIN64

    // 32bit OS
    return false;

  #else

    if (LOBYTE(LOWORD(GetVersion())) >= 6)
      // Vista or newer
      return false;

    // when we reach this we're either XP or 2003 x64
    WCHAR arrCh [MAX_PATH + 1];
    if (is64bit)
      return (GetRemoteModuleHandle64(hProcess, libraryName) == NULL);
    else
      return (GetRemoteModuleHandle32(hProcess, libraryName, arrCh) == NULL);

  #endif
}

#pragma warning(default: 4100)

SYSTEMS_API BOOL InternalUninjectLibrary(HMODULE hOwner, LPCWSTR libraryName, bool multiInject, HANDLE hProcess, LPCWSTR driverName, DWORD session, BOOL system, LPCWSTR includes, LPCWSTR excludes, PULONG excludePIDs, DWORD timeOut)
{
  TraceVerbose(L"%S(%X, %d, %X, %p, %d)", __FUNCTION__, hOwner, session, hProcess, libraryName, timeOut);

  ASSERT(libraryName != NULL);

  bool result = false;

  // Determine fully qualified library path
  SString libraryPath;
  if (!CheckLibFilePath(libraryName, libraryPath))
    return FALSE;

  // Make sure that all privileges are enabled
  EnableAllPrivileges();

  // Assure that the function pointers are initialized
  InitKernelProcs();

  BOOL dll64 = Is64bitModule(libraryPath.GetBuffer());

  if (multiInject)
  {
    DebugTrace((L"%S - Multi Uninjection", __FUNCTION__));

    #ifndef _WIN64
      if (((GetMadCHookOption(UNINJECT_FROM_RUNNING_PROCESSES)) || (!Is64bitOS())) && (dll64))
      {
        SetLastError(ERROR_NOT_SUPPORTED);
        return false;
      }
    #endif

    if ((GetMadCHookOption(UNINJECT_FROM_RUNNING_PROCESSES)) && (!IsAdminAndElevated()))
    {
      SetLastError(ERROR_ACCESS_DENIED);
      return false;
    }

    if (!StopDllInjection(driverName, libraryPath.GetBuffer(), session, system, includes, excludes))
      return false;

    if (!GetMadCHookOption(UNINJECT_FROM_RUNNING_PROCESSES))
      return true;

    if (session == CURRENT_SESSION)
      session = GetCurrentSessionId();

    result = TRUE;

    char smssAnsi[16], slsvcAnsi[16];
    WCHAR smssWide[16], slsvcWide[16];
    DecryptStr( CSmss,  smssAnsi, 16);
    DecryptStr(CSLSvc, slsvcAnsi, 16);
    AnsiToWide(smssAnsi, smssWide);
    AnsiToWide(slsvcAnsi, slsvcWide);

    BOOLEAN needFullPath = false;
    LPWSTR helperBuf = NULL;
    LPWSTR lowCaseIncludes = NULL, lowCaseExcludes = NULL;
    LPWSTR *incPathBuf, *excPathBuf;
    LPWSTR *incNameBuf, *excNameBuf;
    int *incPathLen, *excPathLen;
    int *incNameLen, *excNameLen;
    int incCount, excCount;

    if (includes)
    {
      lowCaseIncludes = (LPWSTR) LocalAlloc(LPTR, lstrlenW(includes) * 2 + 2);
      lstrcpyW(lowCaseIncludes, includes);
      _wcslwr(lowCaseIncludes);
    }
    if (excludes)
    {
      lowCaseExcludes = (LPWSTR) LocalAlloc(LPTR, lstrlenW(excludes) * 2 + 2);
      lstrcpyW(lowCaseExcludes, excludes);
      _wcslwr(lowCaseExcludes);
    }
    needFullPath = SplitStrArray(lowCaseIncludes, &incPathBuf, &incNameBuf, &incPathLen, &incNameLen, &incCount, NULL, NULL);
    needFullPath = SplitStrArray(lowCaseExcludes, &excPathBuf, &excNameBuf, &excPathLen, &excNameLen, &excCount, smssWide, slsvcWide) || needFullPath;
    if (needFullPath)
      helperBuf = (LPWSTR) LocalAlloc(LPTR, 64 * 1024);

    bool b2;
    bool delay = false;
    DWORD time = 0;
    CEnumProcesses *prcs1 = NULL;
    CEnumProcesses *prcs2 = NULL;
    CCollection<HANDLE, CStructureEqualHelper<HANDLE>> ths;
    do
    {
      if (delay)
        Sleep(10);

      delay = false;
      b2 = true;
      prcs1 = new CEnumProcesses(false);
      ths.RemoveAll();

      for (int i1 = (*prcs1).GetCount() - 1; i1 >= 0; i1--)
      {
        bool b1 = true;
        if ((b1) && (prcs2 != NULL))
          for (int i2 = 0; i2 < (*prcs2).GetCount(); i2++)
            if ((*prcs2)[i2].Id == (*prcs1)[i1].Id)
            {
              b1 = false;
              break;
            }
        if (excludePIDs)
        {
          int i2 = 0;
          while ((excludePIDs[i2]) && (excludePIDs[i2] != (*prcs1)[i1].Id))
            i2++;
          if (excludePIDs[i2])
            b1 = false;
        }
        if (b1)
        {
          LPWSTR pathBuf, nameBuf;
          int pathLen, nameLen;
          _wcslwr((*prcs1)[i1].ExeFile);
          if ( (!SplitNamePath((*prcs1)[i1].ExeFile, &pathBuf, &nameBuf, &pathLen, &nameLen)) &&
               (needFullPath) &&
               (ProcessIdToFileNameW((*prcs1)[i1].Id, helperBuf, 32 * 1024)) )
          {
            _wcslwr(helperBuf);
            SplitNamePath(helperBuf, &pathBuf, &nameBuf, &pathLen, &nameLen);
          }
          if ( ((!incCount) || (MatchStrArray(pathBuf, nameBuf, pathLen, nameLen, incCount, incPathBuf, incNameBuf, incPathLen, incNameLen))) &&
               (!MatchStrArray(pathBuf, nameBuf, pathLen, nameLen, excCount, excPathBuf, excNameBuf, excPathLen, excNameLen)) )
          {
            b2 = false;
            HANDLE ph = OpenProcess(PROCESS_ALL_ACCESS, false, (*prcs1)[i1].Id);
            if (ph != NULL)
            {
              BOOL process64 = Is64bitProcess(ph);
              #ifndef _WIN64
                if (!process64)
              #endif
              if (dll64 == process64)
                if ( ( (system) || (!IsSystemProcess(ph, (PSID) (*prcs1)[i1].UserSID)) ) &&
                     ( (session == ALL_SESSIONS) ||
                       (GetProcessSessionId((*prcs1)[i1].Id) == session) ||
                       ((GetProcessSessionId((*prcs1)[i1].Id) == 0) && IsSystemProcess(ph, (PSID) (*prcs1)[i1].UserSID)) ) )
                {
                  HANDLE th;
                  if (CheckSkipUninject(ph, libraryPath.GetBuffer(), dll64))
                    th = NULL;
                  else
                    th = StartInjectLibraryX(libraryPath.GetBuffer(), ph, (*prcs1)[i1].Id, false);
                  if (th != NULL)
                    ths.Add(th);
                }
              CloseHandle(ph);
            }
          }
        }
      }
      if (time == 0)
        time = GetTickCount();
      for (int i1 = 0; i1 < ths.GetCount(); i1++)
        if (ths[i1] != NULL)
          WaitForInjectLibraryX(ths[i1], timeOut, time);
      if (prcs2 != NULL)
        delete prcs2;
      prcs2 = prcs1;
      prcs1 = NULL;
    } while (!b2);

    if (helperBuf)
      LocalFree(helperBuf);
    if (lowCaseIncludes)
      LocalFree(lowCaseIncludes);
    if (lowCaseExcludes)
      LocalFree(lowCaseExcludes);
    if (incCount)
    {
      LocalFree(incPathBuf);
      LocalFree(incNameBuf);
      LocalFree(incPathLen);
      LocalFree(incNameLen);
    }
    if (excCount)
    {
      LocalFree(excPathBuf);
      LocalFree(excNameBuf);
      LocalFree(excPathLen);
      LocalFree(excNameLen);
    }

    delete prcs2;
  }
  else
  {
    DebugTrace((L"%S - Direct Uninjection", __FUNCTION__));

    HANDLE hThread;
    DWORD processId = ProcessHandleToId(hProcess);
    BOOL process64 = Is64bitProcess(hProcess);

    #ifndef _WIN64
      if (!process64)
    #endif
    if (dll64 == process64)
    {
      if (CheckSkipUninject(hProcess, libraryPath.GetBuffer(), dll64))
        result = true;
      else
      {
        hThread = StartInjectLibraryX(libraryPath.GetBuffer(), hProcess, processId, false);
        if (hThread != NULL)
        {
          if (WaitForInjectLibraryX(hThread, timeOut, GetTickCount()))
            result = true;
          else
            Trace(L"%S Failure - WaitForInjectLibraryX", __FUNCTION__);
        }
        else
          Trace(L"%S Failure - StartInjectLibraryX", __FUNCTION__);
      }
    }
  }
  return result;
}

SYSTEMS_API BOOL WINAPI InjectLibraryA(LPCSTR pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut)
{
  SString wideLib (pLibFileName);
  return InternalInjectLibrary(GetCallingModule(_ReturnAddress()), wideLib.GetBuffer(), false, hProcessHandle, NULL, 0, true, NULL, NULL, NULL, dwTimeOut, true, true, false, NULL);
}

SYSTEMS_API BOOL WINAPI InjectLibraryW(LPCWSTR pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut)
{
  return InternalInjectLibrary(GetCallingModule(_ReturnAddress()), pLibFileName, false, hProcessHandle, NULL, 0, true, NULL, NULL, NULL, dwTimeOut, true, true, false, NULL);
}

SYSTEMS_API BOOL WINAPI UninjectLibraryA(LPCSTR pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut)
{
  SString wideLib (pLibFileName);
  return InternalUninjectLibrary(GetCallingModule(_ReturnAddress()), wideLib.GetBuffer(), false, hProcessHandle, NULL, 0, true, NULL, NULL, NULL, dwTimeOut);
}

SYSTEMS_API BOOL WINAPI UninjectLibraryW(LPCWSTR pLibFileName, HANDLE hProcessHandle, DWORD dwTimeOut)
{
  return InternalUninjectLibrary(GetCallingModule(_ReturnAddress()), pLibFileName, false, hProcessHandle, NULL, 0, true, NULL, NULL, NULL, dwTimeOut);
}

SYSTEMS_API BOOL WINAPI InjectLibrarySystemWideA(LPCSTR pDriverName, LPCSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCSTR pIncludeMask, LPCSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut)
{
  SString wideDrv (pDriverName);
  SString wideLib (pLibFileName);
  SString wideInc (pIncludeMask);
  SString wideExc (pExcludeMask);
  return InternalInjectLibrary(GetCallingModule(_ReturnAddress()), wideLib.GetBuffer(), true, 0, wideDrv.GetBuffer(), dwSession, bSystemProcesses, wideInc.GetBuffer(), wideExc.GetBuffer(), pExcludePIDs, dwTimeOut, true, true, false, NULL);
}

SYSTEMS_API BOOL WINAPI InjectLibrarySystemWideW(LPCWSTR pDriverName, LPCWSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCWSTR pIncludeMask, LPCWSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut)
{
  return InternalInjectLibrary(GetCallingModule(_ReturnAddress()), pLibFileName, true, 0, pDriverName, dwSession, bSystemProcesses, pIncludeMask, pExcludeMask, pExcludePIDs, dwTimeOut, true, true, false, NULL);
}

SYSTEMS_API BOOL WINAPI UninjectLibrarySystemWideA(LPCSTR pDriverName, LPCSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCSTR pIncludeMask, LPCSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut)
{
  SString wideDrv (pDriverName);
  SString wideLib (pLibFileName);
  SString wideInc (pIncludeMask);
  SString wideExc (pExcludeMask);
  return InternalUninjectLibrary(GetCallingModule(_ReturnAddress()), wideLib.GetBuffer(), true, 0, wideDrv.GetBuffer(), dwSession, bSystemProcesses, wideInc.GetBuffer(), wideExc.GetBuffer(), pExcludePIDs, dwTimeOut);
}

SYSTEMS_API BOOL WINAPI UninjectLibrarySystemWideW(LPCWSTR pDriverName, LPCWSTR pLibFileName, DWORD dwSession, BOOL bSystemProcesses, LPCWSTR pIncludeMask, LPCWSTR pExcludeMask, PULONG pExcludePIDs, DWORD dwTimeOut)
{
  return InternalUninjectLibrary(GetCallingModule(_ReturnAddress()), pLibFileName, true, 0, pDriverName, dwSession, bSystemProcesses, pIncludeMask, pExcludeMask, pExcludePIDs, dwTimeOut);
}

SYSTEMS_API BOOL WINAPI CreateProcessExW(LPCWSTR applicationName,
                                         LPWSTR commandLine,
                                         SECURITY_ATTRIBUTES *processAttr,
                                         SECURITY_ATTRIBUTES *threadAttr,
                                         BOOL inheritHandles,
                                         DWORD creationFlags,
                                         LPVOID environment,
                                         LPCWSTR currentDirectory,
                                         STARTUPINFOW *startupInfo,
                                         PROCESS_INFORMATION *processInfo,
                                         LPCWSTR loadLibrary)
{
  if (CreateProcessW(applicationName, commandLine, processAttr, threadAttr,
                     inheritHandles, creationFlags | CREATE_SUSPENDED,
                     environment, currentDirectory, startupInfo, processInfo))
  {
    if (InternalInjectLibrary(GetCallingModule(_ReturnAddress()), loadLibrary, false, processInfo->hProcess, NULL, 0, true, NULL, NULL, NULL, INFINITE, true, false, true, processInfo->hThread))
    {
      if ((creationFlags & CREATE_SUSPENDED) == 0)
        ResumeThread(processInfo->hThread);
      return true;
    }
    else
      TerminateProcess(processInfo->hProcess, 0);
  }
  return false;
}

SYSTEMS_API BOOL WINAPI CreateProcessExA(LPCSTR applicationName,
                                         LPSTR commandLine,
                                         SECURITY_ATTRIBUTES *processAttr,
                                         SECURITY_ATTRIBUTES *threadAttr,
                                         BOOL inheritHandles,
                                         DWORD creationFlags,
                                         LPVOID environment,
                                         LPCSTR currentDirectory,
                                         STARTUPINFOA *startupInfo,
                                         PROCESS_INFORMATION *processInfo,
                                         LPCSTR loadLibrary)
{
  if (CreateProcessA(applicationName, commandLine, processAttr, threadAttr,
                     inheritHandles, creationFlags | CREATE_SUSPENDED,
                     environment, currentDirectory, startupInfo, processInfo))
  {
    SString wideLib (loadLibrary);
    if (InternalInjectLibrary(GetCallingModule(_ReturnAddress()), wideLib.GetBuffer(), false, processInfo->hProcess, NULL, 0, true, NULL, NULL, NULL, INFINITE, true, false, true, processInfo->hThread))
    {
      if ((creationFlags & CREATE_SUSPENDED) == 0)
        ResumeThread(processInfo->hThread);
      return true;
    }
    else
      TerminateProcess(processInfo->hProcess, 0);
  }
  return false;
}

//------------------------------------------------------------------------------------------

static bool DoInject(LPCWSTR libraryName, HANDLE hProcess, DWORD processId, HANDLE firstThread, HANDLE *hThread, bool mayPatch, bool mayThread, bool forcePatch)
{
  TraceVerbose(L"%S(%s, 0x%p, %d, %X, %d, %d)", __FUNCTION__, libraryName, hProcess, processId, hThread, mayPatch, mayThread);

  bool result = true;

  *hThread = NULL;

  ULONG_PTR pPeb32 = 0;
  ULONGLONG pPeb64 = 0;

  #ifdef _WIN64
    pPeb64 = (ULONGLONG) GetPeb(hProcess);
  #else
    pPeb32 = (ULONG_PTR) GetPeb(hProcess);
    if (Is64bitProcess(hProcess))
      pPeb64 = GetPeb64(hProcess);
  #endif

  if (forcePatch || NotInitializedYet(pPeb32, pPeb64, hProcess))
  {
    DebugTrace((L"%S - Process not initialized, using inject patching", __FUNCTION__));

    // If NotInitializedYet is true, we have to hook the right function
    //  and copy in our stub that acts as the initializer
    if (mayPatch)
    {
      switch (InjectLibraryPatch(libraryName, pPeb32, pPeb64, hProcess, processId, firstThread, forcePatch))
      {
        case 0:  // patching failed
          result = false;
          break;
        case 1:
          if (mayThread)
            *hThread = StartInjectLibraryX(libraryName, hProcess, processId, true);
          else
            *hThread = NULL;
          result = (*hThread != NULL);
          break;
      }
    }
    else
      result = false;
  }
  else
  {
    DebugTrace((L"%S - Process Initialized, normal injection", __FUNCTION__));

    // If we are initialized, we just do the normal thing
    if (mayThread)
      *hThread = StartInjectLibraryX(libraryName, hProcess, processId, true);
    else
      *hThread = NULL;

    result = (*hThread != NULL);
  }
  return result;
}

SYSTEMS_API BOOL WINAPI NotInitializedYet(ULONG_PTR pPeb32, ULONGLONG pPeb64, HANDLE hProcess)
{
  TraceVerbose(L"%S(%p, %X, %X, ...)", __FUNCTION__, pPeb32, pPeb64, hProcess);

  BOOL result = false;

  if ((!pPeb32) && (!pPeb64))
  {
    #ifdef _WIN64
      pPeb64 = (ULONGLONG) GetPeb(hProcess);
    #else
      pPeb32 = (ULONG_PTR) GetPeb(hProcess);
      if (Is64bitProcess(hProcess))
        pPeb64 = GetPeb64(hProcess);
    #endif
  }

  if (pPeb64)
  {
    ULONGLONG ldrData;

    #ifdef _WIN64
      if (Read(hProcess, (LPCVOID) (pPeb64 + 0x18), &ldrData, sizeof(ldrData)))
    #else
      ULONG64 bytesRead;
      if (ReadProcessMemory64(hProcess, pPeb64 + 0x18, &ldrData, sizeof(ldrData), &bytesRead) && (bytesRead == sizeof(ldrData)))
    #endif
    {
      if (ldrData == NULL)
        result = true;
    }
    else
      Trace(L"%S Failure - Read", __FUNCTION__);
  }
  else
    if (pPeb32)
    {
      ULONG ldrData;

      if (Read(hProcess, (LPCVOID) (pPeb32 + 0xc), &ldrData, sizeof(ldrData)))
      {
        if (ldrData == NULL)
          result = true;
      }
      else
        Trace(L"%S Failure - Read", __FUNCTION__);
    }
    else
      DebugTrace((L"%S - pPeb is NULL, do nothing", __FUNCTION__));

  return result;
}

#ifndef _WIN64

  static HMODULE GetModuleHandle64(LPWSTR dll)
  // retrieve the module handle of a 64bit system module in our 32bit process
  {
    HMODULE result = NULL;

    __try
    {
      // first let's get the 64bit PEB (Process Environment Block)
      ULONG pebl, pebh;
      __asm
      {
        mov ecx, gs:[0x60]
        mov pebl, ecx
        mov ecx, gs:[0x64]
        mov pebh, ecx
      }
      if (pebh == 0)
      {
        // we got it - and it's only a 32bit pointer, fortunately
        // now let's get the loader data
        ULONGLONG i64 = *(ULONGLONG*) (ULONGLONG) (pebl + 0x18);
        if ((ULONG) i64 == i64)
        {
          // fortunately the loader also is only a 32bit pointer
          // now let's loop through the list of dlls
          PVOID loopEnd = (PVOID) (i64 + 0x20);
          PVOID *mi = (PVOID*) loopEnd;
          while ((mi[0] != loopEnd) && (mi[1] == NULL))
          {
            // mi[ 0] + mi[ 1] = pointer to next dll
            // mi[ 8] + mi[ 9] = dll handle
            // mi[16] + mi[17] = full dll file name
            mi = (PVOID*) mi[0];
            if ((mi[16] != NULL) && (mi[17] == NULL) && (CheckDllName(dll, (LPCWSTR) mi[16])))
            {
              // found the dll we're looking for
              if (mi[9] == NULL)
                // and it happens to have a 32bit handle - yippieh!
                result = (HMODULE) mi[8];
              break;
            }
          }
        }
      }
    } __except (1) {}

    return result;
  }

#endif

static HANDLE StartInjectLibraryX(LPCWSTR libraryName, HANDLE hProcess, DWORD processId, bool inject)
{
  TraceVerbose(L"%S(%p, %X, %d, %d)", __FUNCTION__, libraryName, hProcess, processId, inject);

  HANDLE hThread = NULL;

  LPVOID pProc = InstallInjectThreadProc(hProcess, processId);
  if (pProc)
  {
    ULONG injectRecSize;
    INJECT_REC ir;
    LPVOID par = NULL;

    ir.Load = inject;
    VERIFY((wcscpy_s(ir.LibraryNameW, _countof(ir.LibraryNameW), libraryName) == 0));

    #ifdef _WIN64
      if (!Is64bitProcess(hProcess))
      {
        injectRecSize = sizeof(INJECT_REC32);

        if (Init32bitKernelAPIs(hProcess))
        {
          INJECT_REC32 *pir = (INJECT_REC32*) &ir;
          pir->pfnGetVersion           = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(0);
          pir->pfnSharedMem9x_Free     = NULL;
          pir->pfnVirtualFree          = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(1);
          pir->pfnSetErrorMode         = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(2);
          pir->pfnLoadLibraryA         = NULL;
          pir->pfnLoadLibraryW         = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(3);
          pir->pfnGetLastError         = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(4);
          pir->pfnGetModuleHandleA     = NULL;
          pir->pfnGetModuleHandleW     = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(5);
          pir->pfnGetCurrentProcessId  = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(6);
          pir->pfnOpenFileMappingA     = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(7);
          pir->pfnMapViewOfFile        = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(8);
          pir->pfnUnmapViewOfFile      = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(9);
          pir->pfnCloseHandle          = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(10);
          pir->pfnFreeLibrary          = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(11);
          pir->pfnEnterCriticalSection = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(12);
          pir->pfnLeaveCriticalSection = (PVOID32) (ULONG) (ULONGLONG) GetKernelAPI(13);
          par = AllocMemEx(injectRecSize, hProcess, (LPVOID) (((ULONG_PTR) GetMadCHookOption(X86_ALLOCATION_ADDRESS)) - 0x10000));
        }
      }
      else
        if (!IsProcessLargeAddressAware(hProcess))
        {
          if (!pfnGetVersion2GB)
          {
            pfnGetVersion2GB           = (PFN_GET_LAST_ERROR        ) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnGetVersion          );
            pfnGetLastError2GB         = (PFN_GET_LAST_ERROR        ) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnGetLastError        );
            pfnGetCurrentProcessId2GB  = (PFN_GET_CURRENT_PROCESS_ID) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnGetCurrentProcessId );
            pfnVirtualFree2GB          = (PFN_VIRTUAL_FREE          ) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnVirtualFree         );
            pfnSetErrorMode2GB         = (PFN_SET_ERROR_MODE        ) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnSetErrorMode        );
            pfnLoadLibraryW2GB         = (PFN_LOAD_LIBRARY_W        ) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnLoadLibraryW        );
            pfnGetModuleHandleW2GB     = (PFN_GET_MODULE_HANDLE_W   ) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnGetModuleHandleW    );
            pfnOpenFileMappingA2GB     = (PFN_OPEN_FILE_MAPPING_A   ) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnOpenFileMappingA    );
            pfnMapViewOfFile2GB        = (PFN_MAP_VIEW_OF_FILE      ) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnMapViewOfFile       );
            pfnUnmapViewOfFile2GB      = (PFN_UNMAP_VIEW_OF_FILE    ) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnUnmapViewOfFile     );
            pfnCloseHandle2GB          = (PFN_CLOSE_HANDLE          ) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnCloseHandle         );
            pfnFreeLibrary2GB          = (PFN_FREE_LIBRARY          ) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnFreeLibrary         );
            pfnEnterCriticalSection2GB = (PFN_ENTER_CRITICAL_SECTION) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnEnterCriticalSection);
            pfnLeaveCriticalSection2GB = (PFN_LEAVE_CRITICAL_SECTION) LargeAddressAwareApiTo2GB(hProcess, (PVOID) pfnLeaveCriticalSection);
          }
          injectRecSize = sizeof(INJECT_REC);
          ir.pfnGetVersion           = pfnGetVersion2GB;
          ir.pfnSharedMem9x_Free     = NULL;
          ir.pfnVirtualFree          = pfnVirtualFree2GB;
          ir.pfnSetErrorMode         = pfnSetErrorMode2GB;
          ir.pfnLoadLibraryA         = NULL;
          ir.pfnLoadLibraryW         = pfnLoadLibraryW2GB;
          ir.pfnGetLastError         = pfnGetLastError2GB;
          ir.pfnGetModuleHandleA     = NULL;
          ir.pfnGetModuleHandleW     = pfnGetModuleHandleW2GB;
          ir.pfnGetCurrentProcessId  = pfnGetCurrentProcessId2GB;
          ir.pfnOpenFileMappingA     = pfnOpenFileMappingA2GB;
          ir.pfnMapViewOfFile        = pfnMapViewOfFile2GB;
          ir.pfnUnmapViewOfFile      = pfnUnmapViewOfFile2GB;
          ir.pfnCloseHandle          = pfnCloseHandle2GB;
          ir.pfnFreeLibrary          = pfnFreeLibrary2GB;
          ir.pfnEnterCriticalSection = pfnEnterCriticalSection2GB;
          ir.pfnLeaveCriticalSection = pfnLeaveCriticalSection2GB;
          if (ir.pfnVirtualFree)
            par = AllocMemEx(injectRecSize, hProcess);
        }
        else
        {
    #endif

    injectRecSize = sizeof(INJECT_REC);

    ir.pfnGetVersion           = pfnGetVersion;
    ir.pfnSharedMem9x_Free     = NULL;
    ir.pfnVirtualFree          = pfnVirtualFree;
    ir.pfnSetErrorMode         = pfnSetErrorMode;
    ir.pfnLoadLibraryA         = NULL;
    ir.pfnLoadLibraryW         = pfnLoadLibraryW;
    ir.pfnGetLastError         = pfnGetLastError;
    ir.pfnGetModuleHandleA     = NULL;
    ir.pfnGetModuleHandleW     = pfnGetModuleHandleW;
    ir.pfnGetCurrentProcessId  = pfnGetCurrentProcessId;
    ir.pfnOpenFileMappingA     = pfnOpenFileMappingA;
    ir.pfnMapViewOfFile        = pfnMapViewOfFile;
    ir.pfnUnmapViewOfFile      = pfnUnmapViewOfFile;
    ir.pfnCloseHandle          = pfnCloseHandle;
    ir.pfnFreeLibrary          = pfnFreeLibrary;
    ir.pfnEnterCriticalSection = pfnEnterCriticalSection;
    ir.pfnLeaveCriticalSection = pfnLeaveCriticalSection;
    if (ir.pfnVirtualFree)
      par = AllocMemEx(injectRecSize, hProcess);

    #ifdef _WIN64
      }
    #endif

    if (par != NULL)
    {
      DWORD op;
      VirtualProtectEx(hProcess, par, injectRecSize, PAGE_READWRITE, &op);
      SIZE_T numberOfBytesWritten;
      if (WriteProcessMemory(hProcess, par, &ir, injectRecSize, &numberOfBytesWritten))
      {
        DWORD threadId;
        hThread = madCreateRemoteThread(hProcess, NULL, 1 * 1024 * 1024, pProc, par, 0, &threadId);
        if (hThread == NULL)
        {
          DWORD lastError = GetLastError();
          VERIFY(FreeMemEx(par, hProcess));
          SetLastError(lastError);
          Trace(L"%S Failure - madCreateRemoteThread", __FUNCTION__);
        }
      }
      else
        Trace(L"%S Failure - WriteProcessMemory: %X", __FUNCTION__, GetLastError());
    }
    else
      Trace(L"%S Failure - AllocMemEx", __FUNCTION__);
  }
  else
    Trace(L"%S Failure - InstallInjectThreadProc", __FUNCTION__);

  return hThread;
}

const UCHAR CInjectThread32[772] =
  {0x55, 0x8B, 0xEC, 0x81, 0xC4, 0x58, 0xFC, 0xFF, 0xFF, 0x53, 0x56, 0x57, 0x8B, 0x5D, 0x08, 0x64,
   0x8B, 0x05, 0x30, 0x00, 0x00, 0x00, 0x89, 0x45, 0xFC, 0x33, 0xFF, 0x57, 0x8B, 0xF3, 0x8D, 0xBD,
   0x5B, 0xFC, 0xFF, 0xFF, 0xB9, 0xDC, 0x00, 0x00, 0x00, 0xF3, 0xA5, 0x66, 0xA5, 0x5F, 0xFF, 0x55,
   0x89, 0xA9, 0x00, 0x00, 0x00, 0x80, 0x75, 0x0D, 0x68, 0x00, 0x80, 0x00, 0x00, 0x6A, 0x00, 0x53,
   0xFF, 0x55, 0x91, 0xEB, 0x04, 0x53, 0xFF, 0x55, 0x8D, 0x68, 0x03, 0x80, 0x00, 0x00, 0xFF, 0x55,
   0x95, 0x8B, 0xD8, 0x80, 0xBD, 0x5B, 0xFC, 0xFF, 0xFF, 0x00, 0x74, 0x2F, 0xFF, 0x55, 0x89, 0xA9,
   0x00, 0x00, 0x00, 0x80, 0x75, 0x0E, 0x8D, 0x85, 0x6B, 0xFD, 0xFF, 0xFF, 0x50, 0xFF, 0x55, 0x9D,
   0x8B, 0xF0, 0xEB, 0x0C, 0x8D, 0x85, 0x5C, 0xFC, 0xFF, 0xFF, 0x50, 0xFF, 0x55, 0x99, 0x8B, 0xF0,
   0x85, 0xF6, 0x75, 0x34, 0xFF, 0x55, 0xA1, 0x8B, 0xF8, 0xEB, 0x2D, 0xFF, 0x55, 0x89, 0xA9, 0x00,
   0x00, 0x00, 0x80, 0x75, 0x0E, 0x8D, 0x85, 0x6B, 0xFD, 0xFF, 0xFF, 0x50, 0xFF, 0x55, 0xA9, 0x8B,
   0xF0, 0xEB, 0x0C, 0x8D, 0x85, 0x5C, 0xFC, 0xFF, 0xFF, 0x50, 0xFF, 0x55, 0xA5, 0x8B, 0xF0, 0x85,
   0xF6, 0x75, 0x05, 0xFF, 0x55, 0xA1, 0x8B, 0xF8, 0x53, 0xFF, 0x55, 0x95, 0x85, 0xF6, 0x0F, 0x84,
   0x35, 0x02, 0x00, 0x00, 0x80, 0xBD, 0x5B, 0xFC, 0xFF, 0xFF, 0x00, 0x74, 0x67, 0xFF, 0x55, 0x89,
   0xA9, 0x00, 0x00, 0x00, 0x80, 0x0F, 0x85, 0x1E, 0x02, 0x00, 0x00, 0x8B, 0x45, 0xFC, 0x05, 0xA0,
   0x00, 0x00, 0x00, 0x8B, 0x00, 0x89, 0x45, 0xF4, 0x8B, 0x45, 0xF4, 0x50, 0xFF, 0x55, 0xC5, 0x8B,
   0x45, 0xFC, 0x83, 0xC0, 0x0C, 0x8B, 0x00, 0x83, 0xC0, 0x0C, 0x8B, 0x10, 0x85, 0xD2, 0x74, 0x28,
   0x8B, 0xC2, 0x8B, 0x48, 0x18, 0x3B, 0xCE, 0x75, 0x19, 0x66, 0x8B, 0x4A, 0x38, 0x66, 0x81, 0xF9,
   0xFF, 0x00, 0x72, 0x06, 0x66, 0x89, 0x48, 0x38, 0xEB, 0x0E, 0x66, 0xC7, 0x40, 0x38, 0xFF, 0x00,
   0xEB, 0x06, 0x8B, 0x00, 0x3B, 0xD0, 0x75, 0xDA, 0x8B, 0x45, 0xF4, 0x50, 0xFF, 0x55, 0xC9, 0xE9,
   0xC5, 0x01, 0x00, 0x00, 0x33, 0xC0, 0x89, 0x45, 0xF8, 0x8D, 0x45, 0xCD, 0xC7, 0x00, 0x47, 0x6C,
   0x6F, 0x62, 0x8D, 0x45, 0xD1, 0xC7, 0x00, 0x61, 0x6C, 0x5C, 0x41, 0x8D, 0x45, 0xD5, 0xC7, 0x00,
   0x75, 0x74, 0x6F, 0x55, 0x8D, 0x45, 0xD9, 0xC7, 0x00, 0x6E, 0x68, 0x6F, 0x6F, 0x8D, 0x45, 0xDD,
   0xC7, 0x00, 0x6B, 0x4D, 0x61, 0x70, 0xC6, 0x45, 0xE1, 0x24, 0xC6, 0x45, 0xEA, 0x24, 0xC6, 0x45,
   0xF3, 0x00, 0xFF, 0x55, 0xAD, 0xBB, 0xF8, 0xFF, 0xFF, 0xFF, 0x8D, 0x55, 0xE9, 0x8B, 0xC8, 0x83,
   0xE1, 0x0F, 0x83, 0xF9, 0x09, 0x76, 0x0F, 0x8B, 0xC8, 0x80, 0xE1, 0x0F, 0x80, 0xC1, 0x61, 0x80,
   0xE9, 0x0A, 0x88, 0x0A, 0xEB, 0x0A, 0x8B, 0xC8, 0x80, 0xE1, 0x0F, 0x80, 0xC1, 0x30, 0x88, 0x0A,
   0xC1, 0xE8, 0x04, 0x4A, 0x43, 0x75, 0xD6, 0x8B, 0xC6, 0xBB, 0xF8, 0xFF, 0xFF, 0xFF, 0x8D, 0x55,
   0xF2, 0x8B, 0xC8, 0x83, 0xE1, 0x0F, 0x83, 0xF9, 0x09, 0x76, 0x0F, 0x8B, 0xC8, 0x80, 0xE1, 0x0F,
   0x80, 0xC1, 0x61, 0x80, 0xE9, 0x0A, 0x88, 0x0A, 0xEB, 0x0A, 0x8B, 0xC8, 0x80, 0xE1, 0x0F, 0x80,
   0xC1, 0x30, 0x88, 0x0A, 0xC1, 0xE8, 0x04, 0x4A, 0x43, 0x75, 0xD6, 0xFF, 0x55, 0x89, 0xA9, 0x00,
   0x00, 0x00, 0x80, 0x75, 0x0F, 0x8D, 0x45, 0xCD, 0x50, 0x6A, 0x00, 0x6A, 0x04, 0xFF, 0x55, 0xB1,
   0x8B, 0xD8, 0xEB, 0x02, 0x33, 0xDB, 0x85, 0xDB, 0x75, 0x0D, 0x8D, 0x45, 0xD4, 0x50, 0x6A, 0x00,
   0x6A, 0x04, 0xFF, 0x55, 0xB1, 0x8B, 0xD8, 0x85, 0xDB, 0x75, 0x38, 0xC6, 0x45, 0xEB, 0x6D, 0xC6,
   0x45, 0xEC, 0x63, 0xC6, 0x45, 0xED, 0x68, 0xC6, 0x45, 0xEE, 0x00, 0xFF, 0x55, 0x89, 0xA9, 0x00,
   0x00, 0x00, 0x80, 0x75, 0x0D, 0x8D, 0x45, 0xCD, 0x50, 0x6A, 0x00, 0x6A, 0x04, 0xFF, 0x55, 0xB1,
   0x8B, 0xD8, 0x85, 0xDB, 0x75, 0x0D, 0x8D, 0x45, 0xD4, 0x50, 0x6A, 0x00, 0x6A, 0x04, 0xFF, 0x55,
   0xB1, 0x8B, 0xD8, 0x85, 0xDB, 0x74, 0x1D, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x04, 0x53,
   0xFF, 0x55, 0xB5, 0x85, 0xC0, 0x74, 0x09, 0x8B, 0x10, 0x89, 0x55, 0xF8, 0x50, 0xFF, 0x55, 0xB9,
   0x53, 0xFF, 0x55, 0xBD, 0x83, 0x7D, 0xF8, 0x00, 0x74, 0x05, 0x8B, 0xC6, 0xFF, 0x55, 0xF8, 0xFF,
   0x55, 0x89, 0xA9, 0x00, 0x00, 0x00, 0x80, 0x75, 0x3C, 0x8B, 0x45, 0xFC, 0x05, 0xA0, 0x00, 0x00,
   0x00, 0x8B, 0x00, 0x89, 0x45, 0xF4, 0x8B, 0x45, 0xF4, 0x50, 0xFF, 0x55, 0xC5, 0x8B, 0x45, 0xFC,
   0x83, 0xC0, 0x0C, 0x8B, 0x00, 0x83, 0xC0, 0x0C, 0x8B, 0x10, 0x85, 0xD2, 0x74, 0x17, 0x8B, 0xC2,
   0x8B, 0x48, 0x18, 0x3B, 0xCE, 0x75, 0x08, 0x66, 0xC7, 0x40, 0x38, 0x01, 0x00, 0xEB, 0x06, 0x8B,
   0x00, 0x3B, 0xD0, 0x75, 0xEB, 0x56, 0xFF, 0x55, 0xC1, 0x85, 0xC0, 0x74, 0x19, 0x33, 0xDB, 0xEB,
   0x01, 0x43, 0x56, 0xFF, 0x55, 0xC1, 0x85, 0xC0, 0x74, 0x08, 0x81, 0xFB, 0xFF, 0xFF, 0x00, 0x00,
   0x7C, 0xEF, 0x33, 0xFF, 0xEB, 0x05, 0xFF, 0x55, 0xA1, 0x8B, 0xF8, 0xFF, 0x55, 0x89, 0xA9, 0x00,
   0x00, 0x00, 0x80, 0x75, 0x14, 0x8B, 0x45, 0xFC, 0x05, 0xA0, 0x00, 0x00, 0x00, 0x8B, 0x00, 0x89,
   0x45, 0xF4, 0x8B, 0x45, 0xF4, 0x50, 0xFF, 0x55, 0xC9, 0x8B, 0xC7, 0x5F, 0x5E, 0x5B, 0x8B, 0xE5,
   0x5D, 0xC2, 0x04, 0x00};

static LPVOID InstallInjectThreadProc(HANDLE hProcess, DWORD processId)
{
  LPVOID pThreadProc = NULL;

  HANDLE hMap = NULL;

  #ifdef _WIN64
    bool is32bitProcess = true;
  #endif

  char s1[9];
  DecryptStr(CMchIInjT, s1, 9);
  if (processId != 0)
  {
    char mapName[MAX_PATH];
    #ifdef _WIN64
      if (Is64bitProcess(hProcess))
        is32bitProcess = false;
    #endif
    sprintf_s(mapName, MAX_PATH, "%s$%x", s1, processId);
    hMap = CreateGlobalFileMapping(mapName, sizeof(MCH_I_INJ_T));
  }
  if (hMap != NULL)
  {
    BOOL newMap = (GetLastError() != ERROR_ALREADY_EXISTS);
    MCH_I_INJ_T *pBuffer = (MCH_I_INJ_T *) MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (pBuffer != NULL)
    {
      for (int i = 0; i < 50; i++)
      {
        if (newMap || (*((__int64 *) &(pBuffer->Name)) == *((__int64 *) s1) && (pBuffer->ProcessId == processId)))
          break;
        else
          Sleep(50);
      }
      if (newMap || (*((__int64 *) &(pBuffer->Name)) != *((__int64 *) s1) && (pBuffer->ProcessId == processId)))
      {
        #ifdef _WIN64
          if (is32bitProcess)
          {
            SIZE_T c1;
            pThreadProc = AllocMemEx(sizeof(CInjectThread32), hProcess, (LPVOID) GetMadCHookOption(X86_ALLOCATION_ADDRESS));
            WriteProcessMemory(hProcess, pThreadProc, CInjectThread32, sizeof(CInjectThread32), &c1);
            DWORD op;
            VirtualProtectEx(hProcess, pThreadProc, sizeof(CInjectThread32), PAGE_EXECUTE_READ, &op);
            ((MCH_I_INJ_T32*) pBuffer)->InjThread = (PVOID32) (ULONG) (ULONG_PTR) pThreadProc;
          }
          else
          {
            pThreadProc = CopyFunction(InjectThread, hProcess, true, NULL, NULL);
            pBuffer->InjThread = pThreadProc;
          }
        #else
          SIZE_T c1;
          pThreadProc = AllocMemEx(sizeof(CInjectThread32), hProcess);
          WriteProcessMemory(hProcess, pThreadProc, CInjectThread32, sizeof(CInjectThread32), &c1);
          DWORD op;
          VirtualProtectEx(hProcess, pThreadProc, sizeof(CInjectThread32), PAGE_EXECUTE_READ, &op);
          ((MCH_I_INJ_T*) pBuffer)->InjThread = pThreadProc;
        #endif

        pBuffer->ProcessId = processId;
        AtomicMove((LPVOID) s1, (LPVOID) &(pBuffer->Name), 8);
        HANDLE targetHandle;
        DuplicateHandle(GetCurrentProcess(), hMap, hProcess, &targetHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
      }
      else
      {
        DWORD dummy;
        #ifdef _WIN64
          if (is32bitProcess)
            pThreadProc = (LPVOID) (((MCH_I_INJ_T32*) pBuffer)->InjThread);
          else
        #endif
          pThreadProc = pBuffer->InjThread;

        if (!VirtualProtectEx(hProcess, pThreadProc, 0x41C, PAGE_EXECUTE_READ, &dummy))
          Trace(L"%S Warning - VirtualProtectEx: %X", __FUNCTION__, GetLastError());
      }

      UnmapViewOfFile(pBuffer);
    }
    CloseHandle(hMap);
  }
  return pThreadProc;
}

static bool WaitForInjectLibraryX(HANDLE hThread, DWORD timeOut, DWORD time)
{
  if (timeOut == INFINITE)
    time = 0;
  else
    time = TickDif(time);

  DWORD error;
  if (timeOut > time)
  {
    if (WaitForSingleObject(hThread, timeOut - time) == WAIT_OBJECT_0)
      GetExitCodeThread(hThread, &error);
    else
      error = GetLastError();
  }
  else
  {
    if (timeOut == 0)
      error = 0;
    else
      error = ERROR_TIMEOUT;
  }
  CloseHandle(hThread);
  bool result = (error == 0);
  if (!result)
    SetLastError(error);
  return result;
}

static DWORD TickDif(DWORD tick)
{
  DWORD result;
  DWORD tickCount = GetTickCount();
  if (tickCount >= tick)
    result = tickCount - tick;
  else
    result = MAXDWORD - tick + tickCount;
  return result;
}

static BOOL Read(HANDLE hProcess, LPCVOID baseAddress, LPVOID pBuffer, SIZE_T length)
{
  SIZE_T bytesRead;
  return ReadProcessMemory(hProcess, baseAddress, pBuffer, length, &bytesRead);
}

static BOOL Read(HANDLE hProcess, ULONG_PTR baseAddress, LPVOID pBuffer, SIZE_T length)
{
  SIZE_T bytesRead;
  return ReadProcessMemory(hProcess, (LPCVOID) baseAddress, pBuffer, length, &bytesRead);
}

#pragma warning(disable: 4748)  // Disable warning about /GS being disabled due to optimizations being disabled
#pragma optimize("", off)

static int __stdcall InjectThread(INJECT_REC *pInjectRec)
{
  #ifndef _WIN64
    PEB_NT *peb;
    _asm
    {
      mov eax, fs:[0x30]     // eax = threadEnvironmentBlock->processDataBase
      mov [peb], eax
    }
  #endif

  int result = 0;
  INJECT_REC ir = *pInjectRec;
  ir.pfnVirtualFree(pInjectRec, 0, MEM_RELEASE);

  UINT oldErrorMode = ir.pfnSetErrorMode(SEM_FAILCRITICALERRORS);
  HMODULE hModule;
  if (ir.Load)
  {
    hModule = ir.pfnLoadLibraryW(ir.LibraryNameW);
    if (hModule == NULL)
      result = ir.pfnGetLastError();
  }
  else
  {
    hModule = ir.pfnGetModuleHandleW(ir.LibraryNameW);
    if (hModule == NULL)
      result = ir.pfnGetLastError();
  }
  ir.pfnSetErrorMode(oldErrorMode);
  if (hModule != NULL)
  {
    if (ir.Load)
    {
      #ifndef _WIN64
        // the following code sets the load count of the dll
        // after injection it's set to 0xff, which means "statically linked"
        // before uninjection it's set back to 1, which means "dynamically linked"
        // this logic protects the dll from being unloaded manually
        // only UninjectLibrary can successfully unload an injected dll
        ir.pfnEnterCriticalSection(peb->LoaderLock);
        PLDR_DATA_TABLE_ENTRY firstDll = (PLDR_DATA_TABLE_ENTRY) peb->LdrData->InLoadOrderModuleList.Flink;
        if (firstDll)
        {
          PLDR_DATA_TABLE_ENTRY dll = firstDll;
          do
          {
            if ((HMODULE) dll->DllBase == hModule)
            {
              if (firstDll->LoadCount >= 0xff)
                dll->LoadCount = firstDll->LoadCount;
              else
                dll->LoadCount = 0xff;
              break;
            }
            dll = (PLDR_DATA_TABLE_ENTRY) dll->InLoadOrderLinks.Flink;
          } while (dll != firstDll);
        }
        ir.pfnLeaveCriticalSection(peb->LoaderLock);
      #endif
    }
    else
    {
      // ok, so now let's finally unload the dll
      #ifdef _WIN64
        #define nameLen 47
      #else
        #define nameLen 39
      #endif
      char name[nameLen];
      *((DWORD*) &name[ 0]) = 0x626f6c47;  // Glob
      *((DWORD*) &name[ 4]) = 0x415c6c61;  // al\A
      *((DWORD*) &name[ 8]) = 0x556f7475;  // utoU
      *((DWORD*) &name[12]) = 0x6f6f686e;  // nhoo
      *((DWORD*) &name[16]) = 0x70614d6b;  // kMap
      name[20] = '$';
      name[29] = '$';
      name[38] = '\0';

      DWORD pid = ir.pfnGetCurrentProcessId();

      for (int i = 28; i > 20; i--)
      {
        if ((pid & 0x0f) > 9)
          name[i] = 'a' + (char) ((pid & 0x0f) - 0x0a);
        else
          name[i] = '0' + (char) (pid & 0x0f);
        pid = pid >> 4;
      }
      ULONG_PTR mid = (ULONG_PTR) hModule;
      for (int i = 37; i > 29; i--)
      {
        if ((mid & 0x0f) > 9)
          name[i] = 'a' + (char) ((mid & 0x0f) - 0x0a);
        else
          name[i] = '0' + (char) (mid & 0x0f);
        mid = mid >> 4;
      }
      HANDLE hMap;
      hMap = ir.pfnOpenFileMappingA(FILE_MAP_READ, FALSE, name);
      if (hMap == NULL)
        hMap = ir.pfnOpenFileMappingA(FILE_MAP_READ, FALSE, &name[7]);

      if (hMap == NULL)
      {
        name[30] = 'm';
        name[31] = 'c';
        name[32] = 'h';
        name[33] = '\0';
        hMap = ir.pfnOpenFileMappingA(FILE_MAP_READ, FALSE, name);
        if (hMap == NULL)
          hMap = ir.pfnOpenFileMappingA(FILE_MAP_READ, FALSE, &name[7]);
      }

      PFN_AUTO_UNHOOK pfnAutoUnhook = NULL;
      LPVOID *pBuffer;
      if (hMap != NULL)
      {
        pBuffer = (LPVOID *) ir.pfnMapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
        if (pBuffer != NULL)
        {
          pfnAutoUnhook = (PFN_AUTO_UNHOOK) *pBuffer;
          ir.pfnUnmapViewOfFile(pBuffer);
        }
        ir.pfnCloseHandle(hMap);
      }

      if (pfnAutoUnhook != NULL)
        pfnAutoUnhook(hModule);

      #ifndef _WIN64
        ir.pfnEnterCriticalSection(peb->LoaderLock);
        PLDR_DATA_TABLE_ENTRY firstDll = (PLDR_DATA_TABLE_ENTRY) peb->LdrData->InLoadOrderModuleList.Flink;
        if (firstDll)
        {
          PLDR_DATA_TABLE_ENTRY dll = firstDll;
          do
          {
            if ((HMODULE) dll->DllBase == hModule)
            {
              dll->LoadCount = 1;
              break;
            }
            dll = (PLDR_DATA_TABLE_ENTRY) dll->InLoadOrderLinks.Flink;
          } while (dll != firstDll);
        }
      #endif
      if (ir.pfnFreeLibrary(hModule))
      {
        int i = 0;
        while (ir.pfnFreeLibrary(hModule))
        {
          i++;
          if (i == 0xffff)
            break;
        }
      }
      else
        result = ir.pfnGetLastError();
      #ifndef _WIN64
        ir.pfnLeaveCriticalSection(peb->LoaderLock);
      #endif
    }
  }
  return result;
}

#pragma optimize("", on)
#pragma warning(default: 4748)

//-------------------------- Injecting prior to process initialized -----------------------------

#pragma pack(1)

// ********************************************************************

PVOID Nta64 = NULL;      // ntdll.NtTestAlert
PVOID Npvm64 = NULL;     // ntdll.NtProtectVirtualMemory
PVOID Lld64 = NULL;      // ntdll.LdrLoadDll
PVOID Navm64 = NULL;     // ntdll.NtAllocateVirtualMemory
PVOID Nfvm64 = NULL;     // ntdll.NtFreeVirtualMemory
PVOID Nqsi64 = NULL;     // ntdll.NtQuerySystemInformation
PVOID Nde64 = NULL;      // ntdll.NtDelayExecution
#ifndef _WIN64
  PVOID32 Nta32 = NULL;  // ntdll.NtTestAlert
#endif
PVOID32 Npvm32 = NULL;   // ntdll.NtProtectVirtualMemory
PVOID32 Lld32 = NULL;    // ntdll.LdrLoadDll
PVOID32 Navm32 = NULL;   // ntdll.NtAllocateVirtualMemory
PVOID32 Nfvm32 = NULL;   // ntdll.NtFreeVirtualMemory
PVOID32 Nqsi32 = NULL;   // ntdll.NtQuerySystemInformation
PVOID32 Nde32 = NULL;    // ntdll.NtDelayExecution

PFN_QUEUE_USER_APC QueueUserApcProc = NULL;  // kernel32.QueueUserAPC

// ********************************************************************

// sizeof(PRelJump32) is always 32bit
// sizeof(PRelJump64) is 32bit in a 32bit driver and 64bit in a 64bit driver
typedef struct _RelJump
{
  UCHAR jmp;     // 0xe9
  ULONG target;  // relative target
} RelJump, *PRelJump64, * POINTER_32 PRelJump32;

// sizeof(PInjectLib32) is always 32bit
// sizeof(PInjectLib64) is 32bit in a 32bit driver and 64bit in a 64bit driver
typedef struct _InjectLib32 * POINTER_32 PInjectLib32;
typedef struct _InjectLib64 *PInjectLib64;

typedef struct _InjectLib32
// this structure has the same size in both the 32bit and 64bit driver
{
  UCHAR            movEax;       // 0xb8
  PInjectLib32     param;        // pointer to "self"
  UCHAR            movEcx;       // 0xb9
  PVOID32          proc;         // pointer to "self.injectFunc"
  USHORT           callEcx;      // 0xd1ff
  ULONG            magic;        // "mciL" / 0x4c69636d
  PInjectLib32     next;         // next dll (if any)
  PRelJump32       pOldApi;      // which code location in user land do we patch?
  RelJump          oldApi;       // patch backup buffer, contains overwritten code
//  UCHAR            align;
  UNICODE_STRING32 dll;          // dll path/name
  WCHAR            dllBuf[260];  // string buffer for the dll path/name
  PVOID32          npvm;         // ntdll.NtProtectVirtualMemory
  PVOID32          lld;          // ntdll.LdrLoadDll
  PVOID32          navm;         // ntdll.NtAllocateVirtualMemory
  PVOID32          nfvm;         // ntdll.NtFreeVirtualMemory
  PVOID32          nqsi;         // ntdll.NtQuerySystemInformation
  PVOID32          nde;          // ntdll.NtDelayExcecution
  UCHAR            injectFunc;   // will be filled with CInjectLibFunc32 (see below)
} InjectLib32;

#ifdef _WIN64

  const UCHAR CInjectLib32FuncOld[1171] =
    {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0x90, 0x53, 0x56, 0x57, 0x8B, 0xF0, 0x64, 0x8B, 0x05, 0x30, 0x00,
     0x00, 0x00, 0x89, 0x45, 0xFC, 0x64, 0x8B, 0x05, 0x18, 0x00, 0x00, 0x00, 0x8B, 0x40, 0x20, 0x89,
     0x45, 0xF8, 0x64, 0x8B, 0x05, 0x18, 0x00, 0x00, 0x00, 0x8B, 0x40, 0x24, 0x89, 0x45, 0xF4, 0x89,
     0x6D, 0xF0, 0x8B, 0x46, 0x14, 0x8B, 0x55, 0xF0, 0x83, 0xC2, 0x04, 0x89, 0x02, 0x33, 0xC0, 0x89,
     0x45, 0xCC, 0x33, 0xC0, 0x89, 0x45, 0xC8, 0x33, 0xC0, 0x89, 0x45, 0xC4, 0x33, 0xC0, 0x89, 0x45,
     0xC0, 0x33, 0xC0, 0x89, 0x45, 0xBC, 0x33, 0xC0, 0x89, 0x45, 0xB8, 0x8B, 0x45, 0xFC, 0x83, 0xC0,
     0x0C, 0x8B, 0x00, 0x83, 0xC0, 0x14, 0x89, 0x45, 0xDC, 0x8B, 0x45, 0xDC, 0x56, 0x8B, 0xF0, 0x8D,
     0x7D, 0x90, 0xB9, 0x09, 0x00, 0x00, 0x00, 0xF3, 0xA5, 0x5E, 0xE9, 0x01, 0x04, 0x00, 0x00, 0x56,
     0x8B, 0xF0, 0x8D, 0x7D, 0x90, 0xB9, 0x09, 0x00, 0x00, 0x00, 0xF3, 0xA5, 0x5E, 0x83, 0x7D, 0xB0,
     0x00, 0x0F, 0x84, 0xE9, 0x03, 0x00, 0x00, 0x33, 0xC9, 0xEB, 0x01, 0x41, 0x8B, 0x45, 0xB0, 0x66,
     0x83, 0x3C, 0x48, 0x00, 0x75, 0xF5, 0x8B, 0x45, 0xB0, 0x8D, 0x44, 0x48, 0xEE, 0x81, 0x78, 0x04,
     0x64, 0x00, 0x6C, 0x00, 0x0F, 0x85, 0xC6, 0x03, 0x00, 0x00, 0x81, 0x38, 0x6E, 0x00, 0x74, 0x00,
     0x0F, 0x85, 0xBA, 0x03, 0x00, 0x00, 0x8B, 0x45, 0xB0, 0x8D, 0x44, 0x48, 0xF6, 0x81, 0x78, 0x04,
     0x64, 0x00, 0x6C, 0x00, 0x0F, 0x85, 0xA6, 0x03, 0x00, 0x00, 0x81, 0x38, 0x6C, 0x00, 0x2E, 0x00,
     0x0F, 0x85, 0x9A, 0x03, 0x00, 0x00, 0x8B, 0x5D, 0xA0, 0x8D, 0x43, 0x3C, 0x8B, 0x00, 0x03, 0xC3,
     0x8B, 0x40, 0x78, 0x03, 0xC3, 0x89, 0xC7, 0x8B, 0x47, 0x18, 0x48, 0x85, 0xC0, 0x0F, 0x8C, 0x01,
     0x02, 0x00, 0x00, 0x40, 0x89, 0x45, 0xB4, 0xC7, 0x45, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x47,
     0x20, 0x03, 0xC3, 0x8B, 0x55, 0xD8, 0x8B, 0x0C, 0x90, 0x03, 0xCB, 0x85, 0xC9, 0x0F, 0x84, 0xD5,
     0x01, 0x00, 0x00, 0x81, 0x79, 0x04, 0x6F, 0x61, 0x64, 0x44, 0x75, 0x1D, 0x81, 0x39, 0x4C, 0x64,
     0x72, 0x4C, 0x75, 0x15, 0x8D, 0x41, 0x08, 0x8B, 0x00, 0x25, 0xFF, 0xFF, 0xFF, 0x00, 0x3D, 0x6C,
     0x6C, 0x00, 0x00, 0x0F, 0x84, 0x31, 0x01, 0x00, 0x00, 0x81, 0x79, 0x04, 0x6F, 0x74, 0x65, 0x63,
     0x75, 0x3D, 0x81, 0x39, 0x4E, 0x74, 0x50, 0x72, 0x75, 0x35, 0x8D, 0x41, 0x08, 0x81, 0x78, 0x04,
     0x74, 0x75, 0x61, 0x6C, 0x75, 0x29, 0x81, 0x38, 0x74, 0x56, 0x69, 0x72, 0x75, 0x21, 0x8D, 0x41,
     0x10, 0x8B, 0x50, 0x04, 0x8B, 0x00, 0x81, 0xE2, 0xFF, 0xFF, 0xFF, 0x00, 0x81, 0xFA, 0x72, 0x79,
     0x00, 0x00, 0x75, 0x05, 0x3D, 0x4D, 0x65, 0x6D, 0x6F, 0x0F, 0x84, 0xEB, 0x00, 0x00, 0x00, 0x81,
     0x79, 0x04, 0x6C, 0x6F, 0x63, 0x61, 0x75, 0x34, 0x81, 0x39, 0x4E, 0x74, 0x41, 0x6C, 0x75, 0x2C,
     0x8D, 0x41, 0x08, 0x81, 0x78, 0x04, 0x72, 0x74, 0x75, 0x61, 0x75, 0x20, 0x81, 0x38, 0x74, 0x65,
     0x56, 0x69, 0x75, 0x18, 0x8D, 0x41, 0x10, 0x81, 0x78, 0x04, 0x6F, 0x72, 0x79, 0x00, 0x75, 0x06,
     0x81, 0x38, 0x6C, 0x4D, 0x65, 0x6D, 0x0F, 0x84, 0xAE, 0x00, 0x00, 0x00, 0x81, 0x79, 0x04, 0x65,
     0x65, 0x56, 0x69, 0x75, 0x27, 0x81, 0x39, 0x4E, 0x74, 0x46, 0x72, 0x75, 0x1F, 0x8D, 0x41, 0x08,
     0x81, 0x78, 0x04, 0x6C, 0x4D, 0x65, 0x6D, 0x75, 0x13, 0x81, 0x38, 0x72, 0x74, 0x75, 0x61, 0x75,
     0x0B, 0x8D, 0x41, 0x10, 0x81, 0x38, 0x6F, 0x72, 0x79, 0x00, 0x74, 0x7E, 0x81, 0x79, 0x04, 0x65,
     0x72, 0x79, 0x53, 0x75, 0x38, 0x81, 0x39, 0x4E, 0x74, 0x51, 0x75, 0x75, 0x30, 0x8D, 0x41, 0x08,
     0x81, 0x78, 0x04, 0x6D, 0x49, 0x6E, 0x66, 0x75, 0x24, 0x81, 0x38, 0x79, 0x73, 0x74, 0x65, 0x75,
     0x1C, 0x8D, 0x41, 0x10, 0x81, 0x78, 0x04, 0x74, 0x69, 0x6F, 0x6E, 0x75, 0x10, 0x81, 0x38, 0x6F,
     0x72, 0x6D, 0x61, 0x75, 0x08, 0x8D, 0x41, 0x18, 0x80, 0x38, 0x00, 0x74, 0x3D, 0x81, 0x79, 0x04,
     0x6C, 0x61, 0x79, 0x45, 0x0F, 0x85, 0xAE, 0x00, 0x00, 0x00, 0x81, 0x39, 0x4E, 0x74, 0x44, 0x65,
     0x0F, 0x85, 0xA2, 0x00, 0x00, 0x00, 0x8D, 0x41, 0x08, 0x81, 0x78, 0x04, 0x74, 0x69, 0x6F, 0x6E,
     0x0F, 0x85, 0x92, 0x00, 0x00, 0x00, 0x81, 0x38, 0x78, 0x65, 0x63, 0x75, 0x0F, 0x85, 0x86, 0x00,
     0x00, 0x00, 0x8D, 0x41, 0x10, 0x80, 0x38, 0x00, 0x75, 0x7E, 0x8B, 0x47, 0x24, 0x03, 0xC3, 0x8B,
     0x55, 0xD8, 0x0F, 0xB7, 0x04, 0x50, 0x89, 0x45, 0xE8, 0x8B, 0x47, 0x1C, 0x03, 0xC3, 0x8B, 0x55,
     0xE8, 0x8B, 0x04, 0x90, 0x89, 0x45, 0xE8, 0x33, 0xC0, 0x8A, 0x41, 0x02, 0x83, 0xF8, 0x50, 0x7F,
     0x13, 0x74, 0x25, 0x83, 0xE8, 0x41, 0x74, 0x2A, 0x83, 0xE8, 0x03, 0x74, 0x43, 0x83, 0xE8, 0x02,
     0x74, 0x2A, 0xEB, 0x44, 0x83, 0xE8, 0x51, 0x74, 0x2D, 0x83, 0xE8, 0x21, 0x75, 0x3A, 0x8B, 0x45,
     0xE8, 0x03, 0xC3, 0x89, 0x45, 0xC8, 0xEB, 0x30, 0x8B, 0x45, 0xE8, 0x03, 0xC3, 0x89, 0x45, 0xCC,
     0xEB, 0x26, 0x8B, 0x45, 0xE8, 0x03, 0xC3, 0x89, 0x45, 0xC4, 0xEB, 0x1C, 0x8B, 0x45, 0xE8, 0x03,
     0xC3, 0x89, 0x45, 0xC0, 0xEB, 0x12, 0x8B, 0x45, 0xE8, 0x03, 0xC3, 0x89, 0x45, 0xBC, 0xEB, 0x08,
     0x8B, 0x45, 0xE8, 0x03, 0xC3, 0x89, 0x45, 0xB8, 0xFF, 0x45, 0xD8, 0xFF, 0x4D, 0xB4, 0x0F, 0x85,
     0x0A, 0xFE, 0xFF, 0xFF, 0x83, 0x7D, 0xC8, 0x00, 0x0F, 0x84, 0x7E, 0x01, 0x00, 0x00, 0x83, 0x7D,
     0xCC, 0x00, 0x0F, 0x84, 0x74, 0x01, 0x00, 0x00, 0x33, 0xDB, 0x83, 0x7D, 0xC4, 0x00, 0x0F, 0x84,
     0xA6, 0x00, 0x00, 0x00, 0x83, 0x7D, 0xC0, 0x00, 0x0F, 0x84, 0x9C, 0x00, 0x00, 0x00, 0x83, 0x7D,
     0xBC, 0x00, 0x0F, 0x84, 0x92, 0x00, 0x00, 0x00, 0x83, 0x7D, 0xB8, 0x00, 0x0F, 0x84, 0x88, 0x00,
     0x00, 0x00, 0x33, 0xC0, 0x89, 0x45, 0xE8, 0x8D, 0x45, 0xE8, 0x50, 0x6A, 0x00, 0x6A, 0x00, 0x6A,
     0x05, 0xFF, 0x55, 0xBC, 0x83, 0x7D, 0xE8, 0x00, 0x74, 0x70, 0x8B, 0x45, 0xE8, 0x03, 0xC0, 0x89,
     0x45, 0xEC, 0x33, 0xC0, 0x89, 0x45, 0xE0, 0x6A, 0x40, 0x68, 0x00, 0x10, 0x00, 0x00, 0x8D, 0x45,
     0xEC, 0x50, 0x6A, 0x00, 0x8D, 0x45, 0xE0, 0x50, 0x6A, 0xFF, 0xFF, 0x55, 0xC4, 0x85, 0xC0, 0x75,
     0x49, 0x6A, 0x00, 0x8B, 0x45, 0xE8, 0x50, 0x8B, 0x45, 0xE0, 0x50, 0x6A, 0x05, 0xFF, 0x55, 0xBC,
     0x85, 0xC0, 0x75, 0x1F, 0x8B, 0x45, 0xE0, 0x8B, 0x50, 0x44, 0x3B, 0x55, 0xF8, 0x75, 0x08, 0x8B,
     0x98, 0xDC, 0x00, 0x00, 0x00, 0xEB, 0x0C, 0x8B, 0x10, 0x85, 0xD2, 0x74, 0x06, 0x03, 0xD0, 0x8B,
     0xC2, 0xEB, 0xE4, 0x33, 0xC0, 0x89, 0x45, 0xEC, 0x68, 0x00, 0x80, 0x00, 0x00, 0x8D, 0x45, 0xEC,
     0x50, 0x8D, 0x45, 0xE0, 0x50, 0x6A, 0xFF, 0xFF, 0x55, 0xC0, 0x85, 0xDB, 0x74, 0x41, 0x3B, 0x5D,
     0xF4, 0x74, 0x3C, 0xC7, 0x45, 0xE8, 0x01, 0x00, 0x00, 0x00, 0x8B, 0x46, 0x14, 0x8A, 0x10, 0x3A,
     0x56, 0x18, 0x75, 0x08, 0x8B, 0x40, 0x01, 0x3B, 0x46, 0x19, 0x74, 0x23, 0xC7, 0x45, 0xD0, 0x60,
     0x79, 0xFE, 0xFF, 0xC7, 0x45, 0xD4, 0xFF, 0xFF, 0xFF, 0xFF, 0x8D, 0x45, 0xD0, 0x50, 0x6A, 0x00,
     0xFF, 0x55, 0xB8, 0xFF, 0x45, 0xE8, 0x81, 0x7D, 0xE8, 0xE9, 0x03, 0x00, 0x00, 0x75, 0xCB, 0x8B,
     0x46, 0x14, 0x8A, 0x10, 0x3A, 0x56, 0x18, 0x75, 0x08, 0x8B, 0x50, 0x01, 0x3B, 0x56, 0x19, 0x74,
     0x6B, 0x89, 0x45, 0xE0, 0xC7, 0x45, 0xE8, 0x05, 0x00, 0x00, 0x00, 0x8D, 0x45, 0xE4, 0x50, 0x6A,
     0x40, 0x8D, 0x45, 0xE8, 0x50, 0x8D, 0x45, 0xE0, 0x50, 0x6A, 0xFF, 0xFF, 0x55, 0xCC, 0x8B, 0x46,
     0x14, 0x8B, 0x56, 0x18, 0x89, 0x10, 0x8A, 0x56, 0x1C, 0x88, 0x50, 0x04, 0xC7, 0x45, 0xE8, 0x05,
     0x00, 0x00, 0x00, 0x8D, 0x45, 0xE4, 0x50, 0x8B, 0x45, 0xE4, 0x50, 0x8D, 0x45, 0xE8, 0x50, 0x8D,
     0x45, 0xE0, 0x50, 0x6A, 0xFF, 0xFF, 0x55, 0xCC, 0x8D, 0x45, 0xE8, 0x50, 0x8D, 0x46, 0x1D, 0x50,
     0x6A, 0x00, 0x6A, 0x00, 0xFF, 0x55, 0xC8, 0x8B, 0x76, 0x10, 0x85, 0xF6, 0x75, 0xEA, 0xEB, 0x0C,
     0x8B, 0x45, 0x90, 0x3B, 0x45, 0xDC, 0x0F, 0x85, 0xF3, 0xFB, 0xFF, 0xFF, 0x5F, 0x5E, 0x5B, 0x8B,
     0xE5, 0x5D, 0xC3};

  // the CInjectLib32FuncOld data is a compilation of the following Delphi code
  // this user land code is copied to newly created wow64 processes to execute the dll injection

  // this code is used by default for all 64bit OSs

  // the 32bit injection code is rather complicated in the 64bit driver
  // the reason for that is that the 64bit driver doesn''t know some 32bit ntdll APIs
  // so the 32bit code has to find out the address of these APIs at runtime

  // procedure InjectLibFunc32Old(buf: PInjectLib32);
  // var ctid, mtid : dword;  // current and main thread ids
  //     cpid       : dword;
  //     ebp_       : dword;
  //     size       : NativeInt;
  //     c1, c2     : dword;
  //     peb        : dword;
  //     p1         : pointer;
  //     loopEnd    : TPNtModuleInfo;
  //     mi         : TNtModuleInfo;
  //     len        : dword;
  //     ntdll      : dword;
  //     nh         : PImageNtHeaders;
  //     ed         : PImageExportDirectory;
  //     i1         : integer;
  //     api        : pchar;
  //     npi        : ^TNtProcessInfo;
  //     sleep      : int64;
  //     npvm       : function (process: THandle; var addr: pointer; var size: NativeUInt; newProt: dword; var oldProt: dword) : dword; stdcall;
  //     lld        : function (path, flags, name: pointer; var handle: HMODULE) : dword; stdcall;
  //     navm       : function (process: THandle; var addr: pointer; zeroBits: NativeUInt; var regionSize: NativeInt; allocationType, protect: dword) : dword; stdcall;
  //     nfvm       : function (process: THandle; var addr: pointer; var regionSize: NativeInt; freeType: dword) : dword; stdcall;
  //     nqsi       : function (infoClass: dword; buffer: pointer; bufSize: dword; returnSize: TPCardinal) : dword; stdcall;
  //     nde        : function (alertable: pointer; var delayInterval: int64) : dword; stdcall;
  // begin
  //   asm
  //     mov eax, fs:[$30]
  //     mov peb, eax
  //     mov eax, fs:[$18]
  //     mov eax, [eax+$20]
  //     mov cpid, eax
  //     mov eax, fs:[$18]
  //     mov eax, [eax+$24]
  //     mov ctid, eax
  //     mov ebp_, ebp
  //   end;
  //   TPPointer(ebp_ + 4)^ := buf.pOldApi;
  // 
  //   npvm := nil;
  //   lld := nil;
  //   navm := nil;
  //   nfvm := nil;
  //   nqsi := nil;
  //   nde := nil;
  // 
  //   // step 1: locate ntdll.dll
  //   loopEnd := pointer(dword(pointer(peb + $C)^) + $14);
  //   mi := loopEnd^;
  //   while mi.next <> loopEnd do begin
  //     mi := mi.next^;
  //     if mi.name <> nil then begin
  //       len := 0;
  //       while mi.name[len] <> #0 do
  //         inc(len);
  //       if (int64(pointer(@mi.name[len - 9])^) = $006c00640074006e) and         // ntdl
  //          (int64(pointer(@mi.name[len - 5])^) = $006c0064002e006c) then begin  // l.dl
  //         // found it!
  //         ntdll := mi.handle;
  // 
  //         // step 2: locate LdrLoadDll and NtProtectVirtualMemory
  //         nh := pointer(ntdll + dword(pointer(ntdll + $3C)^));
  //         dword(ed) := ntdll + nh^.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  //         for i1 := 0 to ed^.NumberOfNames - 1 do begin
  //           api := pointer(ntdll + TPACardinal(ntdll + ed^.AddressOfNames)^[i1]);
  //           if (api <> nil) and
  //              ( ( (int64(pointer(@api[ 0])^)                       = $4464616f4c72644c) and                    // LdrLoadD
  //                  (dword(pointer(@api[ 8])^) and $00ffffff         = $00006c6c        )     ) or               // ll
  //                ( (int64(pointer(@api[ 0])^)                       = $6365746f7250744e) and                    // NtProtec
  //                  (int64(pointer(@api[ 8])^)                       = $6c61757472695674) and                    // tVirtual
  //                  (int64(pointer(@api[16])^) and $00ffffffffffffff = $000079726f6d654d)     ) or               // Memory
  //                ( (int64(pointer(@api[ 0])^)                       = $61636F6C6C41744E) and                    // NtAlloca
  //                  (int64(pointer(@api[ 8])^)                       = $6175747269566574) and                    // teVirtua
  //                  (int64(pointer(@api[16])^)                       = $0079726F6D654D6C)     ) or               // lMemory
  //                ( (int64(pointer(@api[ 0])^)                       = $695665657246744E) and                    // NtFreeVi
  //                  (int64(pointer(@api[ 8])^)                       = $6D654D6C61757472) and                    // rtualMem
  //                  (dword(pointer(@api[16])^)                       = $0079726F        )     ) or               // ory
  //                ( (int64(pointer(@api[ 0])^)                       = $537972657551744E) and                    // NtQueryS
  //                  (int64(pointer(@api[ 8])^)                       = $666E496D65747379) and                    // ystemInf
  //                  (int64(pointer(@api[16])^)                       = $6E6F6974616D726F) and                    // ormation
  //                  (byte (pointer(@api[24])^)                       = $00              )     ) or               // ormation
  //                ( (int64(pointer(@api[ 0])^)                       = $4579616c6544744e) and                    // NtDelayE
  //                  (int64(pointer(@api[ 8])^)                       = $6e6f697475636578) and                    // xecution
  //                  (byte (pointer(@api[16])^)                       = $00              )     )    ) then begin
  //             c1 := TPAWord(ntdll + ed^.AddressOfNameOrdinals)^[i1];
  //             c1 := TPACardinal(ntdll + ed^.AddressOfFunctions)^[c1];
  //             case api[2] of
  //               'r': lld  := pointer(ntdll + c1);
  //               'P': npvm := pointer(ntdll + c1);
  //               'A': navm := pointer(ntdll + c1);
  //               'F': nfvm := pointer(ntdll + c1);
  //               'Q': nqsi := pointer(ntdll + c1);
  //               'D': nde  := pointer(ntdll + c1);
  //             end;
  //           end;
  //         end;
  //         if (@lld <> nil) and (@npvm <> nil) then begin
  //           // found both APIs!
  //           mtid := 0;
  //           if (@navm <> nil) and (@nfvm <> nil) and (@nqsi <> nil) and (@nde <> nil) then begin
  //             c1 := 0;
  //             nqsi(5, nil, 0, @c1);
  //             if c1 <> 0 then begin
  //               size := c1 * 2;
  //               p1 := nil;
  //               if navm(THandle(-1), p1, 0, size, MEM_COMMIT, PAGE_READWRITE) = 0 then begin
  //                 if nqsi(5, p1, size, nil) = 0 then begin
  //                   npi := p1;
  //                   while true do begin
  //                     if npi^.pid = cpid then begin
  //                       mtid := npi^.threads[0].tid_nt5;
  //                       break;
  //                     end;
  //                     if npi^.offset = 0 then
  //                       break;
  //                     npi := pointer(NativeUInt(npi) + npi^.offset);
  //                   end;
  //                 end;
  //                 size := 0;
  //                 nfvm(THandle(-1), p1, size, MEM_RELEASE);
  //               end;
  //             end;
  //           end;
  //           if (mtid <> 0) and (mtid <> ctid) then begin
  //             // This is not the main thread! This usually doesn't happen, except sometimes in win10.
  //             // We "solve" this by waiting until the main thread has completed executing our loader stub.
  //             // Max wait time 10 seconds, just to be safe.
  //             for c1 := 1 to 1000 do begin
  //               if (buf.pOldApi^.jmp = buf.oldApi.jmp) and (buf.pOldApi^.target = buf.oldApi.target) then
  //                 // Our loader stub patch was removed, so we assume that the main thread has completed running it.
  //                 break;
  //               sleep := -100000;  // 10 milliseconds
  //               nde(nil, sleep);
  //             end;
  //           end;
  //           if (buf.pOldApi^.jmp <> buf.oldApi.jmp) or (buf.pOldApi^.target <> buf.oldApi.target) then begin
  //             // step 3: finally load the to-be-injected dll
  //             p1 := buf.pOldApi;
  //             c1 := 5;
  //             npvm(dword(-1), p1, c1, PAGE_EXECUTE_READWRITE, c2);
  //             buf.pOldApi^ := buf.oldApi;
  //             c1 := 5;
  //             npvm(dword(-1), p1, c1, c2, c2);
  //             repeat
  //               lld(nil, nil, @buf.dll, c1);
  //               buf := buf.next;
  //             until buf = nil;
  //           end;
  //         end;
  //         break;
  //       end;
  //     end;
  //   end;
  // end;

#else

  const UCHAR CInjectLib32FuncOld[411] =
    {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xD8, 0x53, 0x56, 0x8B, 0xD8, 0x64, 0x8B, 0x05, 0x18, 0x00, 0x00,
     0x00, 0x8B, 0x40, 0x20, 0x89, 0x45, 0xFC, 0x64, 0x8B, 0x05, 0x18, 0x00, 0x00, 0x00, 0x8B, 0x40,
     0x24, 0x89, 0x45, 0xF8, 0x89, 0x6D, 0xF4, 0x8B, 0x45, 0xF4, 0x83, 0xC0, 0x04, 0x8B, 0x53, 0x14,
     0x89, 0x10, 0x33, 0xF6, 0x83, 0xBB, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x0F, 0x84, 0x94, 0x00, 0x00,
     0x00, 0x33, 0xC0, 0x89, 0x45, 0xF0, 0x8D, 0x45, 0xF0, 0x50, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x05,
     0xFF, 0x93, 0x3D, 0x02, 0x00, 0x00, 0x83, 0x7D, 0xF0, 0x00, 0x74, 0x79, 0x8B, 0x45, 0xF0, 0x03,
     0xC0, 0x89, 0x45, 0xE8, 0x33, 0xC0, 0x89, 0x45, 0xE4, 0x6A, 0x40, 0x68, 0x00, 0x10, 0x00, 0x00,
     0x8D, 0x45, 0xE8, 0x50, 0x6A, 0x00, 0x8D, 0x45, 0xE4, 0x50, 0x6A, 0xFF, 0xFF, 0x93, 0x35, 0x02,
     0x00, 0x00, 0x85, 0xC0, 0x75, 0x4F, 0x6A, 0x00, 0x8B, 0x45, 0xF0, 0x50, 0x8B, 0x45, 0xE4, 0x50,
     0x6A, 0x05, 0xFF, 0x93, 0x3D, 0x02, 0x00, 0x00, 0x85, 0xC0, 0x75, 0x1F, 0x8B, 0x45, 0xE4, 0x8B,
     0x50, 0x44, 0x3B, 0x55, 0xFC, 0x75, 0x08, 0x8B, 0xB0, 0xDC, 0x00, 0x00, 0x00, 0xEB, 0x0C, 0x8B,
     0x10, 0x85, 0xD2, 0x74, 0x06, 0x03, 0xD0, 0x8B, 0xC2, 0xEB, 0xE4, 0x33, 0xC0, 0x89, 0x45, 0xE8,
     0x68, 0x00, 0x80, 0x00, 0x00, 0x8D, 0x45, 0xE8, 0x50, 0x8D, 0x45, 0xE4, 0x50, 0x6A, 0xFF, 0xFF,
     0x93, 0x39, 0x02, 0x00, 0x00, 0x85, 0xF6, 0x74, 0x44, 0x3B, 0x75, 0xF8, 0x74, 0x3F, 0xC7, 0x45,
     0xF0, 0x01, 0x00, 0x00, 0x00, 0x8B, 0x43, 0x14, 0x8A, 0x10, 0x3A, 0x53, 0x18, 0x75, 0x08, 0x8B,
     0x40, 0x01, 0x3B, 0x43, 0x19, 0x74, 0x26, 0xC7, 0x45, 0xD8, 0x60, 0x79, 0xFE, 0xFF, 0xC7, 0x45,
     0xDC, 0xFF, 0xFF, 0xFF, 0xFF, 0x8D, 0x45, 0xD8, 0x50, 0x6A, 0x00, 0xFF, 0x93, 0x41, 0x02, 0x00,
     0x00, 0xFF, 0x45, 0xF0, 0x81, 0x7D, 0xF0, 0xE9, 0x03, 0x00, 0x00, 0x75, 0xC8, 0x8B, 0x43, 0x14,
     0x8A, 0x10, 0x3A, 0x53, 0x18, 0x75, 0x08, 0x8B, 0x50, 0x01, 0x3B, 0x53, 0x19, 0x74, 0x66, 0x89,
     0x45, 0xE4, 0xC7, 0x45, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x8D, 0x45, 0xEC, 0x50, 0x6A, 0x40, 0x8D,
     0x45, 0xF0, 0x50, 0x8D, 0x45, 0xE4, 0x50, 0x6A, 0xFF, 0xFF, 0x93, 0x2D, 0x02, 0x00, 0x00, 0x8B,
     0x43, 0x14, 0x8B, 0x53, 0x18, 0x89, 0x10, 0x8A, 0x53, 0x1C, 0x88, 0x50, 0x04, 0xC7, 0x45, 0xF0,
     0x05, 0x00, 0x00, 0x00, 0x8D, 0x45, 0xEC, 0x50, 0x8B, 0x45, 0xEC, 0x50, 0x8D, 0x45, 0xF0, 0x50,
     0x8D, 0x45, 0xE4, 0x50, 0x6A, 0xFF, 0xFF, 0x93, 0x2D, 0x02, 0x00, 0x00, 0x8D, 0x45, 0xF0, 0x50,
     0x8D, 0x43, 0x1D, 0x50, 0x6A, 0x00, 0x6A, 0x00, 0xFF, 0x93, 0x31, 0x02, 0x00, 0x00, 0x8B, 0x5B,
     0x10, 0x85, 0xDB, 0x75, 0xE7, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC3};

  // the CInjectLibFunc32Old data is a compilation of the following Delphi code
  // this user land code is copied to newly created 32bit processes to execute the dll injection
  // this solution is used by default in all 32bit OSs

  // procedure InjectLibFunc32Old(buf: PInjectLib32);
  // var ctid, mtid : dword;  // current and main thread ids
  //     cpid       : dword;
  //     ebp_       : dword;
  //     c1, c2     : dword;
  //     size       : NativeInt;
  //     p1         : pointer;
  //     sleep      : int64;
  //     npi        : ^TNtProcessInfo;
  // begin
  //   asm
  //     mov eax, fs:[$18]
  //     mov eax, [eax+$20]
  //     mov cpid, eax
  //     mov eax, fs:[$18]
  //     mov eax, [eax+$24]
  //     mov ctid, eax
  //     mov ebp_, ebp
  //   end;
  //   TPPointer(ebp_ + 4)^ := buf.pOldApi;
  //   mtid := 0;
  //   if @buf.nqsi <> nil then begin
  //     c1 := 0;
  //     buf.nqsi(5, nil, 0, @c1);
  //     if c1 <> 0 then begin
  //       size := c1 * 2;
  //       p1 := nil;
  //       if buf.navm(THandle(-1), p1, 0, size, MEM_COMMIT, PAGE_READWRITE) = 0 then begin
  //         if buf.nqsi(5, p1, c1, nil) = 0 then begin
  //           npi := p1;
  //           while true do begin
  //             if npi^.pid = cpid then begin
  //               mtid := npi^.threads[0].tid_nt5;
  //               break;
  //             end;
  //             if npi^.offset = 0 then
  //               break;
  //             npi := pointer(NativeUInt(npi) + npi^.offset);
  //           end;
  //         end;
  //         size := 0;
  //         buf.nfvm(THandle(-1), p1, size, MEM_RELEASE);
  //       end;
  //     end;
  //   end;
  //   if (mtid <> 0) and (mtid <> ctid) then begin
  //   // This is not the main thread! This usually doesn't happen, except sometimes in win10.
  //   // We "solve" this by waiting until the main thread has completed executing our loader stub.
  //   // Max wait time 10 seconds, just to be safe.
  //     for c1 := 1 to 1000 do begin
  //       if (buf.pOldApi^.jmp = buf.oldApi.jmp) and (buf.pOldApi^.target = buf.oldApi.target) then
  //       // Our loader stub patch was removed, so we assume that the main thread has completed running it.
  //         break;
  //       sleep := -100000;// 10 milliseconds
  //       buf.nde(nil, sleep);
  //     end;
  //   end;
  //   if (buf.pOldApi^.jmp <> buf.oldApi.jmp) or (buf.pOldApi^.target <> buf.oldApi.target) then begin
  //   // step 3: finally load the to-be-injected dll
  //     p1 := buf.pOldApi;
  //     c1 := 5;
  //     buf.npvm(dword(-1), p1, c1, PAGE_EXECUTE_READWRITE, c2);
  //     buf.pOldApi^ := buf.oldApi;
  //     c1 := 5;
  //     buf.npvm(dword(-1), p1, c1, c2, c2);
  //     repeat
  //       buf.lld(nil, nil, @buf.dll, c1);
  //       buf := buf.next;
  //     until buf = nil;
  //   end;
  // end;

  const UCHAR CInjectLibApcFunc32[30] =
    {0x55, 0x8B, 0xEC, 0x51, 0x8B, 0x45, 0x08, 0x8D, 0x55, 0xFC, 0x52, 0x8D, 0x50, 0x1D, 0x52, 0x6A,
     0x00, 0x6A, 0x00, 0xFF, 0x90, 0x31, 0x02, 0x00, 0x00, 0x59, 0x5D, 0xC2, 0x04, 0x00};

  // the CInjectLibApcFunc32 data is a compilation of the following Delphi code
  // this user land code is copied to newly created 32bit processes to execute the dll injection
  // this solution is used by default in all 32bit OSs

  // procedure InjectLibApcFunc32(buf: PInjectLib32); stdcall;
  // var c1 : HMODULE;
  // begin
  //   buf.lld(0, nil, @buf.dll, c1);
  // end;

#endif

typedef struct _InjectLib64
{
  USHORT           movRax;       // 0xb848
  ULONGLONG        retAddr;      // patched API
  UCHAR            pushRax;      // 0x50
  UCHAR            pushRcx;      // 0x51
  UCHAR            pushRdx;      // 0x52
  USHORT           pushR8;       // 0x5041
  USHORT           pushR9;       // 0x5141
  ULONG            subRsp28;     // 0x28ec8348
  USHORT           movRcx;       // 0xb948
  ULONGLONG        param;        // pointer to "self"
  USHORT           movRdx;       // 0xba48
  ULONGLONG        proc;         // pointer to "self.injectFunc"
  USHORT           jmpEdx;       // 0xe2ff
  ULONG            magic;        // "mciL" / 0x4c69636d
  ULONGLONG        next;         // next dll (if any)
  ULONGLONG        pOldApi;      // which code location in user land do we patch?
  RelJump          oldApi;       // patch backup buffer, contains overwritten code
  UNICODE_STRING64 dll;          // dll path/name
  WCHAR            dllBuf[260];  // string buffer for the dll path/name
  ULONGLONG        npvm;         // ntdll.NtProtectVirtualMemory
  ULONGLONG        lld;          // ntdll.LdrLoadDll
  ULONGLONG        navm;         // ntdll.NtAllocateVirtualMemory
  ULONGLONG        nfvm;         // ntdll.NtFreeVirtualMemory
  ULONGLONG        nqsi;         // ntdll.NtQuerySystemInformation
  ULONGLONG        nde;          // ntdll.NtDelayExcecution
  UCHAR            injectFunc;   // will be filled with CInjectLibFunc64 (see below)
} InjectLib64;

const UCHAR CInjectLib64Func[562] =
 {0x4C, 0x8B, 0xDC, 0x53, 0x55, 0x56, 0x57, 0x41, 0x55, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x58, 0x65,
  0x48, 0x8B, 0x04, 0x25, 0x30, 0x00, 0x00, 0x00, 0x41, 0xBE, 0x05, 0x00, 0x00, 0x00, 0x33, 0xFF,
  0x8B, 0x70, 0x40, 0x8B, 0x68, 0x48, 0x48, 0x8B, 0x41, 0x37, 0x49, 0x89, 0x43, 0xB0, 0x48, 0x8B,
  0x81, 0x7C, 0x02, 0x00, 0x00, 0x49, 0x83, 0xCD, 0xFF, 0x48, 0x85, 0xC0, 0x48, 0x8B, 0xD9, 0x4D,
  0x89, 0x73, 0xA8, 0x0F, 0x84, 0x10, 0x01, 0x00, 0x00, 0x41, 0x21, 0x7B, 0x08, 0x4D, 0x8D, 0x4B,
  0x08, 0x45, 0x33, 0xC0, 0x33, 0xD2, 0x41, 0x8B, 0xCE, 0xFF, 0xD0, 0x8B, 0x84, 0x24, 0x90, 0x00,
  0x00, 0x00, 0x85, 0xC0, 0x0F, 0x84, 0xEF, 0x00, 0x00, 0x00, 0x48, 0x21, 0xBC, 0x24, 0xA8, 0x00,
  0x00, 0x00, 0x8D, 0x0C, 0x00, 0x4C, 0x8D, 0x8C, 0x24, 0xA0, 0x00, 0x00, 0x00, 0x48, 0x89, 0x8C,
  0x24, 0xA0, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x94, 0x24, 0xA8, 0x00, 0x00, 0x00, 0x45, 0x33, 0xC0,
  0x49, 0x8B, 0xCD, 0xC7, 0x44, 0x24, 0x28, 0x40, 0x00, 0x00, 0x00, 0xC7, 0x44, 0x24, 0x20, 0x00,
  0x10, 0x00, 0x00, 0xFF, 0x93, 0x6C, 0x02, 0x00, 0x00, 0x85, 0xC0, 0x0F, 0x85, 0xA8, 0x00, 0x00,
  0x00, 0x44, 0x8B, 0x84, 0x24, 0xA0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x94, 0x24, 0xA8, 0x00, 0x00,
  0x00, 0x45, 0x33, 0xC9, 0x41, 0x8B, 0xCE, 0xFF, 0x93, 0x7C, 0x02, 0x00, 0x00, 0x85, 0xC0, 0x75,
  0x1F, 0x48, 0x8B, 0x8C, 0x24, 0xA8, 0x00, 0x00, 0x00, 0xEB, 0x09, 0x39, 0x39, 0x74, 0x11, 0x8B,
  0x01, 0x48, 0x03, 0xC8, 0x48, 0x39, 0x71, 0x50, 0x75, 0xF1, 0x8B, 0xB9, 0x30, 0x01, 0x00, 0x00,
  0x48, 0x83, 0xA4, 0x24, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x8D, 0x84, 0x24, 0xA0, 0x00, 0x00,
  0x00, 0x48, 0x8D, 0x94, 0x24, 0xA8, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x80, 0x00, 0x00, 0x49,
  0x8B, 0xCD, 0xFF, 0x93, 0x74, 0x02, 0x00, 0x00, 0x85, 0xFF, 0x74, 0x3D, 0x3B, 0xFD, 0x74, 0x39,
  0xBF, 0x01, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x4B, 0x37, 0x8A, 0x43, 0x3F, 0x38, 0x01, 0x75, 0x08,
  0x8B, 0x43, 0x40, 0x39, 0x41, 0x01, 0x74, 0x21, 0x48, 0x8D, 0x54, 0x24, 0x40, 0x33, 0xC9, 0x48,
  0xC7, 0x44, 0x24, 0x40, 0x60, 0x79, 0xFE, 0xFF, 0xFF, 0x93, 0x84, 0x02, 0x00, 0x00, 0x83, 0xC7,
  0x01, 0x81, 0xFF, 0xE8, 0x03, 0x00, 0x00, 0x7C, 0xCC, 0x48, 0x8B, 0x4B, 0x37, 0x8A, 0x43, 0x3F,
  0x38, 0x01, 0x75, 0x0C, 0x8B, 0x43, 0x40, 0x39, 0x41, 0x01, 0x0F, 0x84, 0xAB, 0x00, 0x00, 0x00,
  0x48, 0x8D, 0x84, 0x24, 0x98, 0x00, 0x00, 0x00, 0x4C, 0x8D, 0x44, 0x24, 0x30, 0x48, 0x8D, 0x54,
  0x24, 0x38, 0x41, 0xB9, 0x40, 0x00, 0x00, 0x00, 0x49, 0x8B, 0xCD, 0x48, 0x89, 0x44, 0x24, 0x20,
  0xFF, 0x93, 0x5C, 0x02, 0x00, 0x00, 0x85, 0xC0, 0x75, 0x3E, 0x8B, 0x43, 0x3F, 0x48, 0x8B, 0x53,
  0x37, 0x4C, 0x8D, 0x44, 0x24, 0x30, 0x89, 0x02, 0x8A, 0x43, 0x43, 0x49, 0x8B, 0xCD, 0x88, 0x42,
  0x04, 0x44, 0x8B, 0x8C, 0x24, 0x98, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x84, 0x24, 0x98, 0x00, 0x00,
  0x00, 0x48, 0x8D, 0x54, 0x24, 0x38, 0x48, 0x89, 0x44, 0x24, 0x20, 0x4C, 0x89, 0x74, 0x24, 0x30,
  0xFF, 0x93, 0x5C, 0x02, 0x00, 0x00, 0xEB, 0x27, 0x48, 0x8B, 0x43, 0x37, 0x48, 0x8B, 0x08, 0x8B,
  0x43, 0x3F, 0x48, 0x89, 0x0B, 0x89, 0x03, 0x8A, 0x43, 0x43, 0x88, 0x43, 0x04, 0x48, 0x8D, 0x4B,
  0x08, 0xC6, 0x01, 0xE9, 0x8B, 0x43, 0x37, 0x2B, 0xC1, 0x83, 0xC0, 0x03, 0x89, 0x41, 0x01, 0x4C,
  0x8D, 0x43, 0x44, 0x4C, 0x8D, 0x4C, 0x24, 0x30, 0x33, 0xD2, 0x33, 0xC9, 0xFF, 0x93, 0x64, 0x02,
  0x00, 0x00, 0x48, 0x8B, 0x5B, 0x2F, 0x48, 0x85, 0xDB, 0x75, 0xE4, 0x48, 0x83, 0xC4, 0x58, 0x41,
  0x5E, 0x41, 0x5D, 0x5F, 0x5E, 0x5D, 0x5B, // 0xC3};
  0x48, 0x83, 0xC4, 0x28, 0x41, 0x59, 0x41, 0x58, 0x5A, 0x59, 0xC3};

// the CInjectLib64Func data is a compilation of the following C++ code
// it got a manually created extended "tail", though (last 11 bytes)
// this user land code is copied to newly created 64bit processes to execute the dll injection

// static void InjectLib64Func(InjectLib64 *buf)
// {
//   ULONG_PTR* ptib = (ULONG_PTR*) NtCurrentTeb();
//   DWORD cpid = (DWORD) ptib[8];
//   DWORD ctid = (DWORD) ptib[9];
// 
//   LPVOID p1 = (LPVOID)buf->pOldApi;
//   ULONG_PTR c1 = 5;
//   ULONG c2;
// 
//   DWORD mtid = 0;
//   if (buf->nqsi)
//   {
//     ULONG c1 = 0;
//     buf->nqsi(SystemProcessInformation, NULL, 0, &c1);
//     if (c1)
//     {
//       SIZE_T size = c1 * 2;
//       LPVOID p1 = NULL;
//       if (buf->navm((HANDLE) -1, &p1, 0, &size, MEM_COMMIT, PAGE_READWRITE) == 0)
//       {
//         if (buf->nqsi(SystemProcessInformation, p1, (ULONG) size, NULL) == 0)
//         {
//           SYSTEM_PROCESS_INFORMATION* npi = (SYSTEM_PROCESS_INFORMATION*) p1;
//           while (true)
//           {
//             if (npi->Process.UniqueProcessId == cpid)
//             {
//               mtid = (DWORD) npi->Process_NT5.Threads[0].ClientId.UniqueThread;
//               break;
//             }
//             if (npi->Process.Next == 0)
//               break;
//             npi = (SYSTEM_PROCESS_INFORMATION*) (((ULONG_PTR) npi) + npi->Process.Next);
//           }
//         }
//         size = 0;
//         buf->nfvm((HANDLE) -1, &p1, &size, MEM_RELEASE);
//       }
//     }
//   }
//   if ((mtid) && (mtid != ctid))
//   {
//     // This is not the main thread! This usually doesn't happen, except sometimes in win10.
//     // We "solve" this by waiting until the main thread has completed executing our loader stub.
//     // Max wait time 10 seconds, just to be safe.
//     for (int i1 = 1; i1 < 1000; i1++)
//     {
//       if ((buf->pOldApi->jmp == buf->oldApi.jmp) && (buf->pOldApi->target == buf->oldApi.target))
//         // Our loader stub patch was removed, so we assume that the main thread has completed running it.
//         break;
//       LONGLONG sleep = -100000;  // 10 milliseconds
//       buf->nde(NULL, (PLARGE_INTEGER) &sleep);
//     }
//   }
//   if ((buf->pOldApi->jmp != buf->oldApi.jmp) || (buf->pOldApi->target != buf->oldApi.target))
//   {
//     if (!buf->npvm((HANDLE) -1, &p1, &c1, PAGE_EXECUTE_READWRITE, &c2))
//     {
//       *(buf->pOldApi) = buf->oldApi;
//       c1 = 5;
//       buf->npvm((HANDLE) -1, &p1, &c1, c2, &c2);
//     }
//     else
//     {
//       // For some reason we can't uninstall our patch correctly.
//       // This is known to happen with StormShield AV installed.
//       // So what we do is modify our hook callback into a simple passthrough trampoline.
//       RelJump *rj = (RelJump*) buf;
//       // (1) Copy first 8 bytes from original API to our hook callback.
//       *((LPVOID*) buf) = *((LPVOID*) buf->pOldApi);
//       // (2) Uninstall the JMP that is placed in the first 5 bytes.
//       *rj = buf->oldApi;
//       // (3) Finally, after the first 8 bytes, JMP back to the original API.
//       rj = (RelJump*) (((ULONG_PTR) buf) + 8);
//       rj->jmp = 0xe9;
//       rj->target = (ULONG) ((((ULONG_PTR) buf->pOldApi) + 8) - (ULONG_PTR) rj - 5);
//     }
//     do
//     {
//       buf->lld(0, NULL, (PUNICODE_STRING) &(buf->dll), (HMODULE*) &c1);
//       buf = (InjectLib64*) buf->next;
//     } while (buf);
//   }
// }

// ********************************************************************

static PVOID32 Find32bitPatchAddress(HMODULE hModule, IMAGE_NT_HEADERS32 ntHeaders)
{
  #ifndef _WIN64
    if ((GetMadCHookOption(VMWARE_INJECTION_MODE)) || (Is64bitOS()))
    {
  #endif
      if (ntHeaders.OptionalHeader.AddressOfEntryPoint != 0)
        return (TypecastToPVOID32) ((ULONG_PTR) hModule + ntHeaders.OptionalHeader.AddressOfEntryPoint);
      else
        return NULL;
  #ifndef _WIN64
    }
    else
    {
      if (Nta32 == NULL)
        Nta32 = NtProc(CNtTestAlert, true);
      return Nta32;
    }
  #endif
}

static BOOLEAN InjectLibraryPatchNt32(HANDLE ProcessHandle, LPCWSTR LibraryFileName, HMODULE hModule, IMAGE_NT_HEADERS32 ntHeaders)
// inject a dll into a newly started 32bit process
{
  BOOLEAN result = FALSE;
  PInjectLib32 buf = (PInjectLib32) (TypecastToPVOID32) (ULONG_PTR) AllocMemEx(sizeof(InjectLib32) + sizeof(CInjectLib32FuncOld), ProcessHandle);

  if (buf)
  {
    // we were able to allocate a buffer in the newly created process

    InjectLib32 il;
    RelJump rj;
    SIZE_T c1;

    #ifndef _WIN64
      if (Npvm32 == NULL)
        Npvm32 = NtProc(CNtProtectVirtualMemory, true);
      if (Lld32 == NULL)
        Lld32 = NtProc(CLdrLoadDll, true);
      if (Navm32 == NULL)
        Navm32 = NtProc(CNtAllocateVirtualMemory, true);
      if (Nfvm32 == NULL)
        Nfvm32 = NtProc(CNtFreeVirtualMemory, true);
      if (Nqsi32 == NULL)
        Nqsi32 = NtProc(CNtQuerySystemInformation, true);
      if (Nde32 == NULL)
        Nde32 = NtProc(CNtDelayExecution, true);
    #endif

    il.movEax            = 0xb8;
    il.param             = buf;
    il.movEcx            = 0xb9;
    il.proc              = (TypecastToPVOID32) (&buf->injectFunc);
    il.callEcx           = 0xd1ff;
    il.magic             = 0;
    il.next              = NULL;
    il.pOldApi           = (PRelJump32) Find32bitPatchAddress(hModule, ntHeaders);
    il.dll.Length        = (USHORT) wcslen(LibraryFileName) * 2;
    il.dll.MaximumLength = il.dll.Length + 2;
    il.dll.Buffer        = (TypecastToPVOID32) (&buf->dllBuf[0]);
    wcscpy(il.dllBuf, LibraryFileName);
    il.npvm              = Npvm32;
    il.lld               = Lld32;
    il.navm              = Navm32;
    il.nfvm              = Nfvm32;
    il.nqsi              = Nqsi32;
    il.nde               = Nde32;

    if ( (il.pOldApi) &&
         WriteProcessMemory(ProcessHandle, buf, &il, sizeof(InjectLib32), &c1) && (c1 == sizeof(InjectLib32)) &&
         WriteProcessMemory(ProcessHandle, &(buf->injectFunc), (PVOID) CInjectLib32FuncOld, sizeof(CInjectLib32FuncOld), &c1) && (c1 == sizeof(CInjectLib32FuncOld)) &&
         ReadProcessMemory (ProcessHandle, il.pOldApi, &rj, 5, &c1) && (c1 == 5) &&
         WriteProcessMemory(ProcessHandle, &(buf->oldApi), &rj, 5, &c1) && (c1 == 5) )
    {
      // we've successfully initialized the buffer

      ULONG op;

      if (VirtualProtectEx(ProcessHandle, il.pOldApi, 5, PAGE_EXECUTE_READWRITE, &op))
      {
        // we successfully unprotected the to-be-patched code, so now we can patch it

        rj.jmp = 0xe9;
        rj.target = (ULONG) (ULONG_PTR) buf - (ULONG) (ULONG_PTR) il.pOldApi - 5;
        WriteProcessMemory(ProcessHandle, il.pOldApi, &rj, 5, &c1);

        // now restore the original page protection

        VirtualProtectEx(ProcessHandle, il.pOldApi, 5, op, &op);

        result = TRUE;
      }
    }

    ULONG op;
    VirtualProtectEx(ProcessHandle, buf, sizeof(InjectLib32) + sizeof(CInjectLib32FuncOld), PAGE_EXECUTE_READ, &op);
  }

  return result;
}

#ifndef _WIN64

  static BOOLEAN InjectLibraryApc32(HANDLE ProcessHandle, DWORD pid, HANDLE hThread, LPCWSTR LibraryFileName)
  // inject a dll into a newly started 32bit process using APC
  {
    BOOLEAN result = FALSE;

    if (!QueueUserApcProc)
      QueueUserApcProc = (PFN_QUEUE_USER_APC) KernelProc(CQueueUserAPC, true);

    if (QueueUserApcProc)
    {
      HANDLE th;
      if (!hThread)
        th = OpenFirstThread(pid);
      else
        th = hThread;

      if (th)
      {
        // we have a valid thread handle for the newly created process

        PInjectLib32 buf = (PInjectLib32) (TypecastToPVOID32) (ULONG_PTR) AllocMemEx(sizeof(InjectLib32) + sizeof(CInjectLibApcFunc32), ProcessHandle);
        if (buf)
        {
          // we were able to allocate a buffer in the newly created process

          InjectLib32 il;
          SIZE_T c1;

          #ifndef _WIN64
            if (Lld32 == NULL)
              Lld32 = NtProc(CLdrLoadDll, true);
          #endif

          memset(&il, 0, sizeof(il));
          il.dll.Length        = (USHORT) wcslen(LibraryFileName) * 2;
          il.dll.MaximumLength = il.dll.Length + 2;
          il.dll.Buffer        = (TypecastToPVOID32) (&buf->dllBuf[0]);
          wcscpy(il.dllBuf, LibraryFileName);
          il.lld               = Lld32;

          if ( WriteProcessMemory(ProcessHandle, buf, &il, sizeof(InjectLib32), &c1) && (c1 == sizeof(InjectLib32)) &&
               WriteProcessMemory(ProcessHandle, &(buf->injectFunc), (PVOID) CInjectLibApcFunc32, sizeof(CInjectLibApcFunc32), &c1) && (c1 == sizeof(CInjectLibApcFunc32)) &&
               QueueUserApcProc((PAPCFUNC) &(buf->injectFunc), th, (ULONG_PTR) buf) )
            result = TRUE;

          ULONG op;
          VirtualProtectEx(ProcessHandle, buf, sizeof(InjectLib32) + sizeof(CInjectLibApcFunc32), PAGE_EXECUTE_READ, &op);
        }
      }
    }

    return result;
  }

#endif

// ********************************************************************

static BOOLEAN InjectLibraryPatchNt64(HANDLE ProcessHandle, LPCWSTR LibraryFileName)
// inject a dll into a newly started 64bit process
{
  BOOLEAN result = FALSE;

  if ((Nta64 == NULL) || (Npvm64 == NULL) || (Lld64 == NULL))
  {
    #ifdef _WIN64
      Nta64 = NtProc(CNtTestAlert, true);
      Npvm64 = NtProc(CNtProtectVirtualMemory, true);
      Lld64 = NtProc(CLdrLoadDll, true);
      Navm64 = NtProc(CNtAllocateVirtualMemory, true);
      Nfvm64 = NtProc(CNtFreeVirtualMemory, true);
      Nqsi64 = NtProc(CNtQuerySystemInformation, true);
      Nde64 = NtProc(CNtDelayExecution, true);
    #else
      char buffer[64];
      HMODULE dll = GetModuleHandle64(L"ntdll.dll");
      Nta64 = GetImageProcAddress(dll, DecryptStr(CNtTestAlert, buffer, 64));
      Npvm64 = GetImageProcAddress(dll, DecryptStr(CNtProtectVirtualMemory, buffer, 64));
      Lld64 = GetImageProcAddress(dll, DecryptStr(CLdrLoadDll, buffer, 64));
      Navm64 = GetImageProcAddress(dll, DecryptStr(CNtAllocateVirtualMemory, buffer, 64));
      Nfvm64 = GetImageProcAddress(dll, DecryptStr(CNtFreeVirtualMemory, buffer, 64));
      Nqsi64 = GetImageProcAddress(dll, DecryptStr(CNtQuerySystemInformation, buffer, 64));
      Nde64 = GetImageProcAddress(dll, DecryptStr(CNtDelayExecution, buffer, 64));
    #endif
  }

  if ((Nta64 != NULL) && (Npvm64 != NULL) && (Lld64 != NULL))
  {

    #ifdef _WIN64
      PInjectLib64 buf = (PInjectLib64) AllocMemEx(sizeof(InjectLib64) + sizeof(CInjectLib64Func), ProcessHandle, Nta64);
    #else
      PInjectLib64 buf = (PInjectLib64) AllocMemEx(sizeof(InjectLib64) + sizeof(CInjectLib64Func), ProcessHandle);
    #endif

    if (buf)
    {
      // we were able to allocate a buffer in the newly created process

      InjectLib64 il;
      RelJump rj;
      SIZE_T c1;

      il.movRax            = 0xb848;
      il.retAddr           = (ULONGLONG) Nta64;
      il.pushRax           = 0x50;
      il.pushRcx           = 0x51;
      il.pushRdx           = 0x52;
      il.pushR8            = 0x5041;
      il.pushR9            = 0x5141;
      il.subRsp28          = 0x28ec8348;
      il.movRcx            = 0xb948;
      il.param             = (ULONGLONG) buf;
      il.movRdx            = 0xba48;
      il.proc              = (ULONGLONG) &(buf->injectFunc);
      il.jmpEdx            = 0xe2ff;
      il.magic             = 0;
      il.next              = NULL;
      il.pOldApi           = il.retAddr;
      il.dll.Length        = (USHORT) wcslen(LibraryFileName) * 2;
      il.dll.MaximumLength = il.dll.Length + 2;
      il.dll.Buffer        = (ULONGLONG) &(buf->dllBuf[0]);
      wcscpy(il.dllBuf, LibraryFileName);
      il.npvm              = (ULONGLONG) Npvm64;
      il.lld               = (ULONGLONG) Lld64;
      il.navm              = (ULONGLONG) Navm64;
      il.nfvm              = (ULONGLONG) Nfvm64;
      il.nqsi              = (ULONGLONG) Nqsi64;
      il.nde               = (ULONGLONG) Nde64;

      if ( WriteProcessMemory(ProcessHandle, buf, &il, sizeof(InjectLib64), &c1) && (c1 == sizeof(InjectLib64)) &&
           WriteProcessMemory(ProcessHandle, &(buf->injectFunc), (PVOID) CInjectLib64Func, sizeof(CInjectLib64Func), &c1) && (c1 == sizeof(CInjectLib64Func)) &&
           ReadProcessMemory (ProcessHandle, (PVOID) il.pOldApi, &rj, 5, &c1) && (c1 == 5) &&
           WriteProcessMemory(ProcessHandle, &(buf->oldApi), &rj, 5, &c1) && (c1 == 5) )
      {
        // we've successfully initialized the buffer

        ULONG op;

        if (VirtualProtectEx(ProcessHandle, (PVOID) il.pOldApi, 5, PAGE_EXECUTE_READWRITE, &op))
        {
          // we successfully unprotected the to-be-patched code, so now we can patch it

          rj.jmp = 0xe9;
          rj.target = (ULONG) ((ULONG_PTR) buf - (ULONG_PTR) il.pOldApi - 5);
          WriteProcessMemory(ProcessHandle, (PVOID) il.pOldApi, &rj, 5, &c1);

          // now restore the original page protection

          VirtualProtectEx(ProcessHandle, (PVOID) il.pOldApi, 5, op, &op);

          result = TRUE;
        }
      }

      DWORD op;
      VirtualProtectEx(ProcessHandle, buf, sizeof(InjectLib64) + sizeof(CInjectLib64Func), PAGE_EXECUTE_READ, &op);
    }
  }

  return result;
}

// ********************************************************************

#ifndef _WIN64

  typedef struct _InjectLibXp32
  {
    PULONG         pEntryPoint;
    ULONG          oldEntryPoint;
    LPVOID         oldEntryPointFunc;
    LPVOID         NtProtectVirtualMemory;
    LPVOID         LdrLoadDll;
    UNICODE_STRING dllStr;
    WCHAR          dllBuf[260];
    BYTE           movEax;
    LPVOID         buf;
    BYTE           jmp;
    int            target;
  } InjectLibXp32;

  const UCHAR CNewEntryPoint32[155] =
    {0x55, 0x8b, 0xec, 0x83, 0xc4, 0xf4, 0x53, 0x56, 0x57, 0x8b, 0xd8, 0x8b, 0x75, 0x0c, 0x83, 0xfe,
     0x01, 0x75, 0x49, 0x8b, 0x03, 0x89, 0x45, 0xfc, 0xc7, 0x45, 0xf8, 0x04, 0x00, 0x00, 0x00, 0x8d,
     0x45, 0xf4, 0x50, 0x6a, 0x40, 0x8d, 0x45, 0xf8, 0x50, 0x8d, 0x45, 0xfc, 0x50, 0x6a, 0xff, 0xff,
     0x53, 0x0c, 0x85, 0xc0, 0x75, 0x26, 0x8b, 0x53, 0x04, 0x8b, 0x03, 0x89, 0x10, 0x89, 0x45, 0xfc,
     0xc7, 0x45, 0xf8, 0x04, 0x00, 0x00, 0x00, 0x8d, 0x45, 0xf4, 0x50, 0x8b, 0x45, 0xf4, 0x50, 0x8d,
     0x45, 0xf8, 0x50, 0x8d, 0x45, 0xfc, 0x50, 0x6a, 0xff, 0xff, 0x53, 0x0c, 0x83, 0x7b, 0x04, 0x00,
     0x74, 0x14, 0x8b, 0x45, 0x10, 0x50, 0x56, 0x8b, 0x45, 0x08, 0x50, 0xff, 0x53, 0x08, 0x85, 0xc0,
     0x75, 0x04, 0x33, 0xc0, 0xeb, 0x02, 0xb0, 0x01, 0xf6, 0xd8, 0x1b, 0xff, 0x83, 0xfe, 0x01, 0x75,
     0x0f, 0x8d, 0x45, 0xf8, 0x50, 0x8d, 0x43, 0x14, 0x50, 0x6a, 0x00, 0x6a, 0x00, 0xff, 0x53, 0x10,
     0x8b, 0xc7, 0x5f, 0x5e, 0x5b, 0x8b, 0xe5, 0x5d, 0xc2, 0x0c, 0x00};

  // the CNewEntryPoint32 data is a compilation of the following Delphi code
  // this user land code is copied to newly created 32bit processes to execute the dll injection
  // this solution is used in XP, Windows 2003 and all newer 32bit OSs

  // function NewEntryPoint(var injectRec: TInjectRec; d1, d2, reserved, reason, hInstance: dword) : bool;
  // var p1     : pointer;
  //     c1, c2 : dword;
  // begin
  //   with injectRec do begin
  //     if reason = DLL_PROCESS_ATTACH then begin
  //       p1 := pEntryPoint;
  //       c1 := 4;
  //       if NtProtectVirtualMemory($ffffffff, p1, c1, PAGE_EXECUTE_READWRITE, c2) = 0 then begin
  //         pEntryPoint^ := oldEntryPoint;
  //         p1 := pEntryPoint;
  //         c1 := 4;
  //         NtProtectVirtualMemory($ffffffff, p1, c1, c2, c2);
  //       end;
  //     end;
  //     result := (oldEntryPoint = 0) or oldEntryPointFunc(hInstance, reason, reserved);
  //     if reason = DLL_PROCESS_ATTACH then
  //       LdrLoadDll(0, nil, @dll, c1);
  //   end;
  // end;

  static BOOLEAN InjectLibraryPatchXp32(HANDLE ProcessHandle, LPCWSTR LibraryFileName)
  {
    SIZE_T c1;
    DWORD c2, c3;
    InjectLibXp32 ir;
    BOOLEAN result = false;

    ir.pEntryPoint = (PULONG) NtProc(NULL);  // get the address where ntdll.dll's entry point is stored
    if ( (ir.pEntryPoint) &&
         ReadProcessMemory(ProcessHandle, ir.pEntryPoint, &ir.oldEntryPoint, 4, &c1) )
    {
      LPVOID buf = AllocMemEx(sizeof(CNewEntryPoint32) + sizeof(ir), ProcessHandle);
      if (buf)
      {
        HMODULE ntdllhandle = GetNtDllHandle();
        if (!ir.oldEntryPoint)
          ir.oldEntryPointFunc = NULL;
        else
          ir.oldEntryPointFunc = (LPVOID) ((ULONG_PTR) ntdllhandle + ir.oldEntryPoint);
        ir.NtProtectVirtualMemory = NtProc(CNtProtectVirtualMemory, true);
        ir.LdrLoadDll             = NtProc(CLdrLoadDll, true);
        ir.dllStr.Length          = (WORD) (lstrlenW(LibraryFileName) * 2);
        ir.dllStr.MaximumLength   = ir.dllStr.Length + 2;
        ir.dllStr.Buffer          = (PWCHAR) ((ULONG_PTR) buf + sizeof(CNewEntryPoint32) + ((ULONG_PTR) ir.dllBuf - (ULONG_PTR) (&ir)));
        memcpy(ir.dllBuf, LibraryFileName, ir.dllStr.MaximumLength);
        ir.movEax = 0xb8;  // mov eax, dw
        ir.buf    = (LPVOID) ((ULONG_PTR) buf + sizeof(CNewEntryPoint32));
        ir.jmp    = 0xe9;  // jmp dw
        ir.target = - (int) sizeof(CNewEntryPoint32) - (int) sizeof(ir);
        c2 = (DWORD) ((ULONG_PTR) buf + sizeof(CNewEntryPoint32) + sizeof(ir) - 10 - (ULONG_PTR) ntdllhandle);
        if ( WriteProcessMemory(ProcessHandle, buf, &CNewEntryPoint32[0], sizeof(CNewEntryPoint32), &c1) &&
             WriteProcessMemory(ProcessHandle, (LPVOID) ((ULONG_PTR) buf + sizeof(CNewEntryPoint32)), &ir, sizeof(ir), &c1) &&
             VirtualProtectEx(ProcessHandle, ir.pEntryPoint, 4, PAGE_EXECUTE_READWRITE, &c3) )
        {
          if (WriteProcessMemory(ProcessHandle, ir.pEntryPoint, &c2, 4, &c1))
            result = true;
          VirtualProtectEx(ProcessHandle, ir.pEntryPoint, 4, c3, &c2);
        }
        DWORD op;
        VirtualProtectEx(ProcessHandle, buf, sizeof(CNewEntryPoint32) + sizeof(ir), PAGE_EXECUTE_READ, &op);
      }
    }

    return result;
  }

#endif

// ********************************************************************

static int InjectLibraryPatch(LPCWSTR libraryName, ULONG_PTR pPeb32, ULONGLONG pPeb64, HANDLE hProcess, DWORD pid, HANDLE hThread, bool forcePatch)
{
  int result = 0;
  if (forcePatch || (NotInitializedYet(pPeb32, pPeb64, hProcess)))
  {
    HMODULE hModule;
    IMAGE_NT_HEADERS32 ntHeaders;
    LPVOID dd;

    if (GetExeModuleInfos(pPeb32, pPeb64, hProcess, &hModule, &ntHeaders, &dd))
    {
      BOOLEAN boolResult;

      #ifndef _WIN64
        OSVERSIONINFOW os;
        memset(&os, 0, sizeof(os));
        os.dwOSVersionInfoSize = sizeof(os);
        GetVersionEx(&os);
      #endif

      if (Is64bitProcess(hProcess))
        boolResult = InjectLibraryPatchNt64(hProcess, libraryName);
      else
      {
        #ifndef _WIN64
          #ifdef InjectLibraryPatchXp
            if ( ((os.dwMajorVersion > 5) || ((os.dwMajorVersion == 5) && (os.dwMinorVersion > 0))) &&
                 (!Is64bitOS()) )
              boolResult = InjectLibraryPatchXp32(hProcess, libraryName);
            else
          #endif
          if (IsProcessDotNet(hProcess))
            boolResult = InjectLibraryApc32(hProcess, pid, hThread, libraryName);
          else
        #else
          hThread;
          pid;
        #endif
          boolResult = InjectLibraryPatchNt32(hProcess, libraryName, hModule, ntHeaders);
      }

      if (boolResult)
        if (forcePatch || (NotInitializedYet(pPeb32, pPeb64, hProcess)))
          result = 2;
        else
          result = 1;
    }
  }
  else
    result = 1;

  return result;
}

static bool GetExeModuleInfos(ULONG_PTR pPeb32, ULONGLONG pPeb64, HANDLE hProcess, HMODULE *pModule, IMAGE_NT_HEADERS32 *pNtHeaders, LPVOID *dd)
{
  *pModule = NULL;

  if (pPeb32)
  {
    ULONG c1;
    if (Read(hProcess, (LPCVOID) (pPeb32 + 0x8), &c1, sizeof(c1)))
      *pModule = (HMODULE) (ULONG_PTR) c1;
  }
  else
  {
    #ifdef _WIN64
      HMODULE c1;
      if (Read(hProcess, (LPCVOID) (pPeb64 + 0x10), &c1, sizeof(c1)))
        *pModule = c1;
    #else
      ULONGLONG c1;
      ULONG64 bytesRead;
      if (ReadProcessMemory64(hProcess, pPeb64 + 0x10, &c1, sizeof(c1), &bytesRead) && (bytesRead == sizeof(c1)) && ((ULONGLONG) (ULONG) c1 == c1))
        *pModule = (HMODULE) c1;
    #endif
  }

  return ((*pModule) && CheckModule(hProcess, *pModule, pNtHeaders, dd));
}

static bool CheckModule(HANDLE hProcess, HMODULE hModule, IMAGE_NT_HEADERS32 *pNtHeaders, LPVOID *dd)
{
  bool result;
  DWORD offset;
  WORD emagic;
  // CENEWHDR = NT_HEADERS_OFFSET 0x003C
  // CEMAGIC  = IMAGE_DOS_SIGNATURE
  // CPEMAGIC = IMAGE_NT_SIGNATURE
  result = (hModule != NULL) &&
           Read(hProcess, (LPCVOID) hModule, &emagic, 2) && (emagic == IMAGE_DOS_SIGNATURE) &&
           Read(hProcess, (LPCVOID) ((ULONG_PTR) hModule + NT_HEADERS_OFFSET), &offset, 4) &&
           Read(hProcess, (LPCVOID) ((ULONG_PTR) hModule + offset), pNtHeaders, sizeof(IMAGE_NT_HEADERS32)) &&
           (pNtHeaders->Signature == IMAGE_NT_SIGNATURE) &&
           ((pNtHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL) == 0);
  if (result)
    *dd = &(pNtHeaders->OptionalHeader.DataDirectory[0]);
  return result;
}

// ----------------------------- Library path --------------------------------------------

static bool PathRemoveFileSpec(LPWSTR path)
{
  ASSERT(path != NULL);
  bool result = false;

  wchar_t *pPeriod = wcsrchr(path, L'.');
  ASSERT(pPeriod != NULL);

  if (pPeriod != NULL)
  {
    *pPeriod = L'\0';
    wchar_t *pLastSlash = wcsrchr(path, L'\\');
    ASSERT(pLastSlash != NULL);

    if (pLastSlash != NULL)
    {
      *pLastSlash = L'\0';
      result = true;
    }
  }
  return result;
}

static LPWSTR PathCombine(LPWSTR dest, LPCWSTR dir, LPCWSTR file)
{
  HRESULT hr;
  if (dir == NULL)
  {
    hr = swprintf(dest, MAX_PATH, L"%s", file);
  }
  else
  {
    size_t len = wcslen(dir);
    if (dir[len - 1] != L'\\')
    {
      hr = swprintf(dest, MAX_PATH, L"%s\\%s", dir, file);
    }
    else
    {
      hr = swprintf(dest, MAX_PATH, L"%s%s", dir, file);
    }
  }
  ASSERT(hr >= 0);
  return dest;
}

static bool LibraryExists(LPCWSTR libraryName, SString& libraryPath, LPWSTR path, bool removeName)
{
  bool result = false;

  if (removeName && (path != NULL))
    PathRemoveFileSpec(path);

  wchar_t destination[MAX_PATH + 1];
  PathCombine(destination, path, libraryName);

  if ( (wcslen(destination) > 6) && 
       ( (destination[1] == ':') || ((destination[0] == '\\') && (destination[1] == '\\')) ) &&
       (GetFileAttributes(destination) != -1) )
  {
    libraryPath = destination;
    result = true;
  }
  return result;
}

SYSTEMS_API BOOL WINAPI CheckLibFilePath(LPCWSTR libraryName, SString& libraryPath)
{
  bool result;

  // First see if the "libraryName" already contains the correct path
  result = LibraryExists(libraryName, libraryPath, NULL, false);
  if (!result)
  {
    wchar_t *searchPath = (wchar_t*) LocalAlloc(LPTR, 32 * 1024 * 2);
    __try
    {

      // Check using path of current module
      if (GetModuleFileName(NULL, searchPath, 32 * 1024) == 0)
        return false;
      result = LibraryExists(libraryName, libraryPath, searchPath, true);

      if (!result)
      {
        // Check using path of calling process instance
        if (GetModuleFileName(gHModule, searchPath, 32 * 1024) == 0)
          return false;

        result = LibraryExists(libraryName, libraryPath, searchPath, true);
        if (!result)
        {
          // Check using path of current directory
          if (GetCurrentDirectory(32 * 1024, searchPath) == 0)
            return false;

          result = LibraryExists(libraryName, libraryPath, searchPath, false);
          if (!result)
          {
            // Check using system path
            if (GetSystemDirectory(searchPath, 32 * 1024) == 0)
              return false;
            result = LibraryExists(libraryName, libraryPath, searchPath, false);
          }
        }
      }
    }
    __finally
    {
      LocalFree(searchPath);
    }
  }
  return result;
}
