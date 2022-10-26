// ***************************************************************
//  ObjectTools.cpp           version: 1.0.5  ·  date: 2015-04-20
//  -------------------------------------------------------------
//  wraps the creation of global windows objects
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2015 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2015-04-20 1.0.5 small performance improvements
// 2013-12-03 1.0.4 fixed a rare crash
// 2012-09-03 1.0.3 added support for Metro (AppContainer integrity) apps
// 2012-05-21 1.0.2 fixed chrome uninjection problems
// 2010-03-24 1.0.1 (1) CreateGlobalFileMapping: LastError value is now restored
//                  (2) fixed format ansi/wide mismatch
// 2010-01-10 1.0.0 initial release

#define _OBJECTTOOLS_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

static int IsMetroFlag = -1;
bool IsMetro(void)
{
  if (IsMetroFlag == -1)
  {
    OSVERSIONINFO verInfo;
    ZeroMemory(&verInfo, sizeof(OSVERSIONINFO));
    verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&verInfo);
    IsMetroFlag = ((verInfo.dwMajorVersion > 6) || ((verInfo.dwMajorVersion == 6) && (verInfo.dwMinorVersion > 1)));
  }

  return IsMetroFlag == 1;
}

HANDLE InternalCreateMutex(LPCSTR name, BOOL global)
{
  TraceVerbose(L"%S(%S)", __FUNCTION__, name);

  ASSERT(name != NULL);
  ASSERT(name[0] != '\0');

  if ((name == NULL) || (name[0] == '\0'))
    return NULL;

  HANDLE result = NULL;
  if (global)
  {
    char buffer[8];
    DecryptStr(CGlobal, buffer, 8);
    SString mutexName(buffer);
    mutexName += name;
    result = CreateMutex(GetUnlimitedSa(), FALSE, mutexName.GetBuffer());
  }
  if (result == NULL)
  {
    if (global)
      Trace(L"%S Failure - CreateMutex([Global\\%S]): %X, retrying with [%S]", __FUNCTION__, name, GetLastError(), name);
    SString wide(name);
    result = CreateMutex(GetUnlimitedSa(), FALSE, wide.GetBuffer());
  }

  return result;
}

SYSTEMS_API HANDLE WINAPI CreateGlobalMutex(LPCSTR name)
{
  return InternalCreateMutex(name, true);
}

SYSTEMS_API HANDLE WINAPI CreateLocalMutex(LPCSTR name)
{
  return InternalCreateMutex(name, false);
}

SYSTEMS_API HANDLE WINAPI OpenGlobalMutex(LPCSTR name)
{
  TraceVerbose(L"%S(%S)", __FUNCTION__, name);

  ASSERT(name != NULL);
  ASSERT(name[0] != '\0');

  if ((name == NULL) || (name[0] == '\0'))
    return NULL;

  HANDLE result = NULL;
  char buffer[8];
  DecryptStr(CGlobal, buffer, 8);
  SString mutexName(buffer);
  mutexName += name;
  result = OpenMutex(SYNCHRONIZE, FALSE, mutexName.GetBuffer());
  if (result == NULL)
  {
    Trace(L"%S Failure - OpenMutexMutex([Global\\%S]): %X, retrying with [%S]", __FUNCTION__, name, GetLastError(), name);
    SString wide(name);
    result = OpenMutex(SYNCHRONIZE, FALSE, wide.GetBuffer());
  }

  return result;
}

SYSTEMS_API HANDLE WINAPI InternalCreateFileMapping(LPCSTR name, DWORD size, BOOL global)
{
  TraceVerbose(L"%S(%S, %d)", __FUNCTION__, name, size);

  ASSERT(name != NULL);
  ASSERT(name[0] != '\0');

  HANDLE result = NULL;
  if (global)
  {
    char buffer[8];
    DecryptStr(CGlobal, buffer, 8);
    SString mapName(buffer);
    mapName += name;
    result = CreateFileMapping((HANDLE) -1, GetLimitedSa(), PAGE_READWRITE, 0, size, mapName.GetBuffer());
  }
  if (result == NULL)
  {
    if (global)
      Trace(L"%S Failure - CreateFileMapping([Global\\%S]): %X, retrying with [%S]", __FUNCTION__, name, GetLastError(), name);
    SString wide(name);
    result = CreateFileMapping((HANDLE) -1, GetLimitedSa(), PAGE_READWRITE, 0, size, wide.GetBuffer());
  }

  return result;
}

HANDLE FindFileMappingHandle(LPCSTR name)
// browse through the process handles to find a specific memory mapped file
{
  HANDLE result = NULL;
  PFN_NT_QUERY_OBJECT pfnNtQueryObject = (PFN_NT_QUERY_OBJECT) NtProc(CNtQueryObject);
  if (pfnNtQueryObject)
  {
    PVOID *buf = (PVOID*) VirtualAlloc(NULL, 2048, MEM_COMMIT, PAGE_READWRITE);
    CHAR arrCh[MAX_PATH];
    for (ULONG_PTR i1 = 0; i1 < 0x1000; i1++)
    {
      BOOL res = FALSE;
      __try
      {
        res = (SUCCEEDED(pfnNtQueryObject((HANDLE) (i1 * 4), 2, buf, 2048, NULL))) && (buf[1]) &&
              (!_wcsicmp((LPWSTR) buf[1], L"Section")) &&
              (SUCCEEDED(pfnNtQueryObject((HANDLE) (i1 * 4), 1, buf, 2048, NULL))) && (buf[1]);
      }
      __except (1)
      {
        res = FALSE;
      }
      if (res)
      {
        WideToAnsi((LPCWSTR) buf[1], arrCh);
        int arrChLen = lstrlenA(arrCh);
        int nameLen = lstrlenA(name);
        if ( (arrChLen > nameLen) &&
             (!_stricmp(name, &arrCh[arrChLen - nameLen])) &&
             (DuplicateHandle(GetCurrentProcess(), (HANDLE) (i1 * 4), GetCurrentProcess(), &result, 0, false, DUPLICATE_SAME_ACCESS)) )
          break;
      }
    }
    VirtualFree(buf, 0, MEM_RELEASE);
  }
  return result;
}

HANDLE InternalOpenFileMapping(LPCSTR name, BOOL write, BOOL global)
{
  TraceVerbose(L"%S(%S, %d)", __FUNCTION__, name, write);

  ASSERT(name != NULL);
  ASSERT(name[0] != '\0');

  HANDLE result = NULL;
  DWORD access = 0;
  if (write)
    access = FILE_MAP_ALL_ACCESS;
  else
    access = FILE_MAP_READ;
  if (global)
  {
    char buffer[8];
    DecryptStr(CGlobal, buffer, 8);
    SString mapName(buffer);
    mapName += name;
    result = OpenFileMapping(access, false, mapName.GetBuffer());
  }
  if (result == NULL)
  {
    if (global)
      Trace(L"%S Failure - OpenFileMapping([Global\\%S]): %X, retrying with [%S]", __FUNCTION__, name, GetLastError(), name);
    SString wide(name);
    result = OpenFileMapping(access, false, wide.GetBuffer());
  }
  if ((result == NULL) && ((GetLastError() == 5) || (GetLastError() == 6)))
  {
    // chrome sandbox processes don't seem to be allowed to open memory mapped files, ouch
    // so we do some special processing to locate already open handles which fit our needs
    DWORD le = GetLastError();
    result = FindFileMappingHandle(name);
    if (result)
      SetLastError(0);
    else
      SetLastError(le);
  }
  return result;
}

SYSTEMS_API HANDLE WINAPI CreateGlobalFileMapping(LPCSTR name, DWORD size)
{
  EnableAllPrivileges();
  return InternalCreateFileMapping(name, size, true);
}

SYSTEMS_API HANDLE WINAPI CreateLocalFileMapping(LPCSTR name, DWORD size)
{
  return InternalCreateFileMapping(name, size, false);
}

SYSTEMS_API HANDLE WINAPI OpenGlobalFileMapping(LPCSTR name, BOOL write)
{
  return InternalOpenFileMapping(name, write, true);
}

SYSTEMS_API HANDLE WINAPI OpenLocalFileMapping(LPCSTR name, BOOL write)
{
  return InternalOpenFileMapping(name, write, false);
}

SYSTEMS_API HANDLE WINAPI CreateGlobalEvent(LPCSTR name, BOOL manual, BOOL initialState)
{
  TraceVerbose(L"%S(%S, %d, %d)", __FUNCTION__, name, manual, initialState);

  ASSERT(name != NULL);
  ASSERT(name[0] != '\0');

  if ((name == NULL) || (name[0] == '\0'))
    return NULL;

  HANDLE hEvent = NULL;
  char buffer[8];
  DecryptStr(CGlobal, buffer, 8);
  SString eventName(buffer);
  eventName += name;
  hEvent = CreateEvent(GetUnlimitedSa(), manual, initialState, eventName.GetBuffer());
  if (hEvent == NULL)
  {
    Trace(L"%S Failure - CreateEvent([%s]): %X, retrying with [%S]", __FUNCTION__, eventName.GetBuffer(), GetLastError(), name);
    SString wide(name);
    hEvent = CreateEvent(GetUnlimitedSa(), manual, initialState, wide.GetBuffer());
  }
  return hEvent;
}

SYSTEMS_API HANDLE WINAPI OpenGlobalEvent(LPCSTR name)
{
  TraceVerbose(L"%S(%S)", __FUNCTION__, name);

  ASSERT(name != NULL);
  ASSERT(name[0] != '\0');

  if ((name == NULL) || (name[0] == '\0'))
    return NULL;

  HANDLE hEvent = NULL;
  char buffer[8];
  DecryptStr(CGlobal, buffer, 8);
  SString eventName(buffer);
  eventName += name;
  hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName.GetBuffer());
  if (hEvent == NULL)
  {
    Trace(L"%S Failure - OpenEvent([%s]): %X, retrying with [%S]", __FUNCTION__, eventName.GetBuffer(), GetLastError(), name);
    SString wide(name);
    hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, wide.GetBuffer());
  }
  return hEvent;
}

// ------------------------------------------------------------------------------------

void ApiSpecialName(LPCSTR prefix1, LPCSTR prefix2, LPVOID api, BOOL shared, LPSTR specialName)
{
  char buffer[64];
  lstrcpyA(specialName, DecryptStr(prefix1, buffer, 64));
  lstrcatA(specialName, ", ");
  lstrcatA(specialName, DecryptStr(prefix2, buffer, 64));
  lstrcatA(specialName, ", ");
  if (shared)
    lstrcatA(specialName, DecryptStr(CShared, buffer, 64));
  else
  {
    lstrcatA(specialName, DecryptStr(CProcess, buffer, 64));
    lstrcatA(specialName, " ");
    sprintf_s(buffer, "$%08x", GetCurrentProcessId());
    lstrcatA(specialName, buffer);
  }
  lstrcatA(specialName, ", ");
  lstrcatA(specialName, DecryptStr(CApi, buffer, 64));
  lstrcatA(specialName, " ");
  #ifdef _WIN64
    sprintf_s(buffer, "$%016I64x", (ULONG_PTR) api);
  #else
    sprintf_s(buffer, "$%08x", (ULONG) (ULONG_PTR) api);
  #endif
  lstrcatA(specialName, buffer);
}

const SID_IDENTIFIER_AUTHORITY CEveryoneSia                 = {0, 0, 0, 0, 0,  1};
const SID_IDENTIFIER_AUTHORITY CSystemSia                   = {0, 0, 0, 0, 0,  5};
const SID_IDENTIFIER_AUTHORITY CSecurityAppPackageAuthority = {0, 0, 0, 0, 0, 15};

bool CreateMetroSd(SECURITY_DESCRIPTOR *sd)
// create a security descriptor which includes metro "AppContainer" apps
{
  char buffer1[32], buffer2[32];
  PFN_SET_ENTRIES_IN_ACL seia = (PFN_SET_ENTRIES_IN_ACL) GetProcAddress(GetModuleHandleA(DecryptStr(CAdvApi32, buffer1, 32)), DecryptStr(CSetEntriesInAclA, buffer2, 32));
  bool result = false;
  if (seia)
  {
    PSID sid1 = NULL;
    PSID sid2 = NULL;
    PACL dacl = NULL;
    if ( (AllocateAndInitializeSid((PSID_IDENTIFIER_AUTHORITY) &CEveryoneSia,                 1, 0, 0, 0, 0, 0, 0, 0, 0, &sid1)) &&
         (AllocateAndInitializeSid((PSID_IDENTIFIER_AUTHORITY) &CSecurityAppPackageAuthority, 2, 2, 1, 0, 0, 0, 0, 0, 0, &sid2))    )
    {
      EXPLICIT_ACCESS ea[2];
      memset(ea, 0, sizeof(ea));
      ea[0].grfAccessMode = SET_ACCESS;
      ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
      ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
      ea[0].grfAccessPermissions = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
      ea[0].Trustee.ptstrName = (LPTSTR) sid1;
      ea[1].grfAccessMode = SET_ACCESS;
      ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
      ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
      ea[1].grfAccessPermissions = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
      ea[1].Trustee.ptstrName = (LPTSTR) sid2;
      if (!seia(2, ea, NULL, &dacl))
      {
        result = true;
        InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(sd, true, dacl, false);
      }
    }
    if (sid1)
      FreeSid(sid1);
    if (sid2)
      FreeSid(sid2);
  }
  return result;
}

void InitSecurityAttributes(SECURITY_ATTRIBUTES *sa, SECURITY_DESCRIPTOR *sd, bool limited)
{
  ASSERT(sa != NULL);
  ASSERT(sd != NULL);
  ASSERT(!IsBadWritePtr2(sa, sizeof(SECURITY_ATTRIBUTES)));
  ASSERT(!IsBadWritePtr2(sd, sizeof(SECURITY_DESCRIPTOR)));

  memset(sa, 0, sizeof(SECURITY_ATTRIBUTES));
  memset(sd, 0, sizeof(SECURITY_DESCRIPTOR));

  if ((limited) && (GetMadCHookOption(SECURE_MEMORY_MAPS)))
  {
    PSID sid1 = NULL;
    PSID sid2 = NULL;
    PSID sid3 = NULL;
    PSID_AND_ATTRIBUTES saa = NULL;
    PACL dacl = NULL;
    char buffer1[32], buffer2[32];
    PFN_SET_ENTRIES_IN_ACL seia = (PFN_SET_ENTRIES_IN_ACL) GetProcAddress(GetModuleHandleA(DecryptStr(CAdvApi32, buffer1, 32)), DecryptStr(CSetEntriesInAclA, buffer2, 32));
    if (seia)
    {
      if ( (AllocateAndInitializeSid((PSID_IDENTIFIER_AUTHORITY) &CEveryoneSia, 1, 0,  0, 0, 0, 0, 0, 0, 0, &sid1)) &&
           (AllocateAndInitializeSid((PSID_IDENTIFIER_AUTHORITY) &CSystemSia,   1, 18, 0, 0, 0, 0, 0, 0, 0, &sid2)) &&
           (GetProcessSid(GetCurrentProcess(), &saa) && (saa) ) )
      {
        EXPLICIT_ACCESS ea[4];
        int count = 3;
        memset(ea, 0, sizeof(ea));
        ea[0].grfAccessMode = GRANT_ACCESS;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea[0].grfAccessPermissions = SECTION_MAP_READ;
        ea[0].Trustee.ptstrName = (LPTSTR) sid1;
        ea[1].grfAccessMode = GRANT_ACCESS;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea[1].grfAccessPermissions = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
        ea[1].Trustee.ptstrName = (LPTSTR) sid2;
        ea[2].grfAccessMode = GRANT_ACCESS;
        ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[2].Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea[2].grfAccessPermissions = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
        ea[2].Trustee.ptstrName = (LPTSTR) saa->Sid;
        if ((IsMetro()) && (AllocateAndInitializeSid((PSID_IDENTIFIER_AUTHORITY) &CSecurityAppPackageAuthority, 2, 2, 1, 0, 0, 0, 0, 0, 0, &sid3)))
        {
          // this adds access for metro "AppContainer" apps
          ea[3].grfAccessMode = GRANT_ACCESS;
          ea[3].Trustee.TrusteeForm = TRUSTEE_IS_SID;
          ea[3].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
          ea[3].grfAccessPermissions = SECTION_MAP_READ;
          ea[3].Trustee.ptstrName = (LPTSTR) sid3;
          count++;
        }
        if (seia(count, ea, NULL, &dacl))
          dacl = NULL;
      }
    }
    VERIFY(InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION));
    VERIFY(SetSecurityDescriptorDacl(sd, true, dacl, false));
    if (sid1)
      FreeSid(sid1);
    if (sid2)
      FreeSid(sid2);
    if (sid3)
      FreeSid(sid3);
    if (saa)
      LocalFree(saa);
  }
  else
    if (IsMetro())
      CreateMetroSd(sd);
    else
    {
      VERIFY(InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION));
      VERIFY(SetSecurityDescriptorDacl(sd, true, NULL, false));
    }
  sa->nLength = sizeof(SECURITY_ATTRIBUTES);
  sa->lpSecurityDescriptor = sd;
  sa->bInheritHandle = false;
}

static SECURITY_ATTRIBUTES LimitedSa;
static SECURITY_DESCRIPTOR LimitedSd;
static SECURITY_ATTRIBUTES UnlimitedSa;
static SECURITY_DESCRIPTOR UnlimitedSd;

void InitSas()
{
  InitSecurityAttributes(&LimitedSa, &LimitedSd, true);
  InitSecurityAttributes(&UnlimitedSa, &UnlimitedSd, false);
}

void FinalSas()
{
  if (LimitedSd.Dacl)
    LocalFree(LimitedSd.Dacl);
  if (UnlimitedSd.Dacl)
    LocalFree(UnlimitedSd.Dacl);
}

PSECURITY_ATTRIBUTES GetLimitedSa()
{
  return &LimitedSa;
}

PSECURITY_ATTRIBUTES GetUnlimitedSa()
{
  return &UnlimitedSa;
}
