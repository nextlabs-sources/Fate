// ***************************************************************
//  ObjectTools.cpp           version: 1.0.0   date: 2010-01-10
//  -------------------------------------------------------------
//  wraps the creation of global windows objects
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _OBJECTTOOLS_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

HANDLE InternalCreateMutex(LPCSTR name, BOOL global)
{
  TraceVerbose(L"%S(%S)", __FUNCTION__, name);

  ASSERT(name != NULL);
  ASSERT(name[0] != '\0');

  if ((name == NULL) || (name[0] == '\0'))
    return NULL;

  HANDLE result = NULL;
  SECURITY_ATTRIBUTES sa;
  SECURITY_DESCRIPTOR sd;
  InitSecurityAttributes(&sa, &sd, false);
  if (global)
  {
    char buffer[8];
    DecryptStr(CGlobal, buffer, 8);
    SString mutexName(buffer);
    mutexName += name;
    result = CreateMutex(&sa, FALSE, mutexName.GetBuffer());
  }
  if (result == NULL)
  {
    if (global)
      Trace(L"%S Failure - CreateMutex([Global\\%s]): %X, retrying with [%s]", __FUNCTION__, name, GetLastError(), name);
    SString wide(name);
    result = CreateMutex(&sa, FALSE, wide.GetBuffer());
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
    Trace(L"%S Failure - OpenMutexMutex([Global\\%s]): %X, retrying with [%s]", __FUNCTION__, name, GetLastError(), name);
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
  SECURITY_ATTRIBUTES sa;
  SECURITY_DESCRIPTOR sd;
  sd.Dacl = NULL;
  InitSecurityAttributes(&sa, &sd, true);
  if (global)
  {
    char buffer[8];
    DecryptStr(CGlobal, buffer, 8);
    SString mapName(buffer);
    mapName += name;
    result = CreateFileMapping((HANDLE) -1, &sa, PAGE_READWRITE, 0, size, mapName.GetBuffer());
  }
  if (result == NULL)
  {
    if (global)
      Trace(L"%S Failure - CreateFileMapping([Global\\%s]): %X, retrying with [%s]", __FUNCTION__, name, GetLastError(), name);
    SString wide(name);
    result = CreateFileMapping((HANDLE) -1, &sa, PAGE_READWRITE, 0, size, wide.GetBuffer());
  }

  if (sd.Dacl)
    LocalFree(sd.Dacl);

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
      Trace(L"%S Failure - OpenFileMapping([Global\\%s]): %X, retrying with [%s]", __FUNCTION__, name, GetLastError(), name);
    SString wide(name);
    result = OpenFileMapping(access, false, wide.GetBuffer());
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
  SECURITY_ATTRIBUTES sa;
  SECURITY_DESCRIPTOR sd;
  InitSecurityAttributes(&sa, &sd, false);
  char buffer[8];
  DecryptStr(CGlobal, buffer, 8);
  SString eventName(buffer);
  eventName += name;
  hEvent = CreateEvent(&sa, manual, initialState, eventName.GetBuffer());
  if (hEvent == NULL)
  {
    Trace(L"%S Failure - CreateEvent([%s]): %X, retrying with [%s]", __FUNCTION__, eventName.GetBuffer(), GetLastError(), name);
    SString wide(name);
    hEvent = CreateEvent(&sa, manual, initialState, wide.GetBuffer());
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
    Trace(L"%S Failure - OpenEvent([%s]): %X, retrying with [%s]", __FUNCTION__, eventName.GetBuffer(), GetLastError(), name);
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
    sprintf_s(buffer, "$%016x", (ULONG_PTR) api);
  #else
    sprintf_s(buffer, "$%08x", (ULONG) (ULONG_PTR) api);
  #endif
  lstrcatA(specialName, buffer);
}

const SID_IDENTIFIER_AUTHORITY CEveryoneSia = {0, 0, 0, 0, 0, 1};
const SID_IDENTIFIER_AUTHORITY CSystemSia   = {0, 0, 0, 0, 0, 5};
void InitSecurityAttributes(SECURITY_ATTRIBUTES *sa, SECURITY_DESCRIPTOR *sd, bool limited)
{
  ASSERT(sa != NULL);
  ASSERT(sd != NULL);
  ASSERT(!IsBadWritePtr(sa, sizeof(SECURITY_ATTRIBUTES)));
  ASSERT(!IsBadWritePtr(sd, sizeof(SECURITY_DESCRIPTOR)));

  PSID sid1 = NULL;
  PSID sid2 = NULL;
  PSID_AND_ATTRIBUTES saa = NULL;
  PACL dacl = NULL;
  if ((limited) && (GetMadCHookOption(SECURE_MEMORY_MAPS)))
  {
    char buffer1[32], buffer2[32];
    PFN_SET_ENTRIES_IN_ACL seia = (PFN_SET_ENTRIES_IN_ACL) GetProcAddress(GetModuleHandleA(DecryptStr(CAdvApi32, buffer1, 32)), DecryptStr(CSetEntriesInAclA, buffer2, 32));
    if (seia)
    {
      if ( (AllocateAndInitializeSid((PSID_IDENTIFIER_AUTHORITY) &CEveryoneSia, 1, 0,  0, 0, 0, 0, 0, 0, 0, &sid1)) &&
           (AllocateAndInitializeSid((PSID_IDENTIFIER_AUTHORITY) &CSystemSia,   1, 18, 0, 0, 0, 0, 0, 0, 0, &sid2)) &&
           (GetProcessSid(GetCurrentProcess(), &saa) && (saa) ) )
      {
        EXPLICIT_ACCESS ea[3];
        memset(ea, 0, sizeof(ea));
        ea[0].grfAccessMode = GRANT_ACCESS;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea[1] = ea[0];
        ea[2] = ea[1];
        ea[0].grfAccessPermissions = SECTION_MAP_READ;
        ea[1].grfAccessPermissions = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
        ea[2].grfAccessPermissions = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
        ea[0].Trustee.ptstrName = (LPTSTR) sid1;
        ea[1].Trustee.ptstrName = (LPTSTR) sid2;
        ea[2].Trustee.ptstrName = (LPTSTR) saa->Sid;
        if (seia(3, ea, NULL, &dacl))
          dacl = NULL;
      }
    }
  }
  sa->nLength = sizeof(SECURITY_ATTRIBUTES);
  sa->lpSecurityDescriptor = sd;
  sa->bInheritHandle = false;
  VERIFY(InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION));
  VERIFY(SetSecurityDescriptorDacl(sd, true, dacl, false));
  if (sid1)
    FreeSid(sid1);
  if (sid2)
    FreeSid(sid2);
  if (saa)
    LocalFree(saa);
}
