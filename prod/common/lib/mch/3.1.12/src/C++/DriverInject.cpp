// ***************************************************************
//  DriverInject.cpp          version: 1.0.1  ·  date: 2010-08-02
//  -------------------------------------------------------------
//  driver based injection into newly created processes
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-08-02 1.0.1 (1) added IsInjectionDriverInstalled/Running
//                  (2) improved injection driver APIs' GetLastError handling
//                  (3) added SetInjectionMethod API
//                  (4) added UninjectAllLibrariesA/W API
// 2010-01-10 1.0.0 initial version

#define _DRIVERINJECT_C
#define _CRT_SECURE_NO_WARNINGS

#include "SystemIncludes.h"

#pragma warning(disable: 4201)
#include <winioctl.h>
#pragma warning(default: 4201)

#include "Systems.h"
#include "SystemsInternal.h"

// *************************** Driver Management *********************************

bool CheckServicePath(LPCWSTR subStr, LPCWSTR str)
{
  int subLen = (int) wcslen(subStr);
  int strLen = (int) wcslen(str);
  return (subLen <= strLen) && (_wcsicmp(subStr, &str[strLen - subLen]));
}

bool CheckService(LPCWSTR driverName, LPCWSTR fileName, LPCWSTR description, bool start)
{
  bool result = false;
  // first we contact the service control manager
  SC_HANDLE c1 = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (c1 != NULL)
  {
    // okay, that worked, now we try to open our service
    SC_HANDLE c2 = OpenServiceW(c1, driverName, SERVICE_ALL_ACCESS);
    DWORD lastError;
    if (c2 != NULL)
    {
      // our service is installed, let's check the parameters
      DWORD c3 = 0;
      QueryServiceConfigW(c2, NULL, 0, &c3);
      if (c3 != 0)
      {
        LPQUERY_SERVICE_CONFIGW qsc = (LPQUERY_SERVICE_CONFIGW) LocalAlloc(LPTR, c3 * 2);
        if (QueryServiceConfigW(c2, qsc, c3 * 2, &c3))
        {
          SERVICE_STATUS ss;
          if (!QueryServiceStatus(c2, &ss))
            ss.dwCurrentState = SERVICE_STOPPED;
          // all is fine if either the parameters are already correct or:
          // (1) if the service isn't running yet and
          // (2) we're able to successfully reset the parameters
          result = ( (qsc->dwServiceType  == SERVICE_KERNEL_DRIVER    ) &&
                     (qsc->dwStartType    == SERVICE_SYSTEM_START     ) &&
                     (qsc->dwErrorControl == SERVICE_ERROR_NORMAL     ) &&
                     CheckServicePath(fileName, qsc->lpBinaryPathName)     ) ||
                   ( ChangeServiceConfigW(c2, SERVICE_KERNEL_DRIVER, SERVICE_SYSTEM_START,
                                          SERVICE_ERROR_NORMAL, fileName, NULL, NULL, NULL, NULL, NULL,
                                          description) &&
                     (ss.dwCurrentState  == SERVICE_STOPPED) );
          if (start)
            result = result && ((ss.dwCurrentState == SERVICE_RUNNING) || StartServiceW(c2, 0, NULL));
        }
        lastError = GetLastError();
        LocalFree((HLOCAL) qsc);
        SetLastError(lastError);
      }
      lastError = GetLastError();
      CloseServiceHandle(c2);
      SetLastError(lastError);
    }
    lastError = GetLastError();
    CloseServiceHandle(c1);
    SetLastError(lastError);
  }
  return result;
}

SYSTEMS_API BOOL WINAPI InstallInjectionDriver(LPCWSTR driverName, LPCWSTR fileName32bit, LPCWSTR fileName64bit, LPCWSTR description)
{
  bool result = false;

  LPCWSTR fileName = fileName32bit;
  if (Is64bitOS())
    fileName = fileName64bit;

  // Determine fully qualified library path
  SString filePath;
  if (!CheckLibFilePath(fileName, filePath))
    return FALSE;

  EnableAllPrivileges();

  // first we contact the service control manager
  SC_HANDLE c1 = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (c1 != NULL)
  {
    // okay, that worked, now we try to open our service
    SC_HANDLE c2 = OpenServiceW(c1, driverName, SERVICE_ALL_ACCESS);
    DWORD lastError;
    if (c2 != NULL)
    {
      // our service is already installed, so let's check the parameters
      result = CheckService(driverName, filePath.GetBuffer(), description, false);
      lastError = GetLastError();
      CloseServiceHandle(c2);
      SetLastError(lastError);
    }
    else
    {
      // probably our service is not installed yet, so we do that now
      c2 = CreateServiceW(c1, driverName, description,
                          SERVICE_ALL_ACCESS | STANDARD_RIGHTS_ALL,
                          SERVICE_KERNEL_DRIVER, SERVICE_SYSTEM_START,
                          SERVICE_ERROR_NORMAL, filePath.GetBuffer(), NULL, NULL, NULL, NULL, NULL);
      if (c2 != NULL)
      {
        // installation went smooth
        result = true;
        CloseServiceHandle(c2);
      }
    }
    lastError = GetLastError();
    CloseServiceHandle(c1);
    SetLastError(lastError);

    if (result)
    {
      result = StartInjectionDriver(driverName) != false;
      if (!result)
      {
        lastError = GetLastError();
        UninstallInjectionDriver(driverName);
        SetLastError(lastError);
      }
    }
  }
  return result;
}

SYSTEMS_API BOOL WINAPI UninstallInjectionDriver(LPCWSTR driverName)
{
  bool result = false;

  EnableAllPrivileges();

  // first we contact the service control manager
  SC_HANDLE c1 = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (c1 != NULL)
  {
    DWORD lastError;

    // okay, that worked, now we try to open our service
    SC_HANDLE c2 = OpenServiceW(c1, driverName, SERVICE_ALL_ACCESS | DELETE);
    if (c2 != NULL)
    {
      // our service is installed, let's try to remove it
      result = DeleteService(c2) != false;
      lastError = GetLastError();
      CloseServiceHandle(c2);
      SetLastError(lastError);
    }

    lastError = GetLastError();
    CloseServiceHandle(c1);
    SetLastError(lastError);
  }

  return result;
}

void RegDelTree(HKEY key, LPCWSTR name)
{
  HKEY hk;
  if (RegOpenKeyEx(key, name, 0, KEY_ALL_ACCESS, &hk) == 0)
  {
    DWORD c1 = 0;
    WCHAR arrChW[MAX_PATH + 1];
    while (RegEnumKey(hk, c1, arrChW, MAX_PATH) == 0)
    {
      RegDelTree(hk, arrChW);
      c1++;
    }
    RegCloseKey(hk);
  }
  RegDeleteKey(key, name);
}

DWORD ConvertNtErrorCode(DWORD error)
{
  if (error & 0xc0000000)
  {
    PFN_RTL_NT_STATUS_TO_DOS_ERROR pfnRtlNtStatusToDosError = (PFN_RTL_NT_STATUS_TO_DOS_ERROR) NtProc(CRtlNtStatusToDosError);

    if (pfnRtlNtStatusToDosError != NULL)
      return pfnRtlNtStatusToDosError(error);
  }

  return error;
}

SYSTEMS_API BOOL WINAPI LoadInjectionDriver(LPCWSTR driverName, LPCWSTR fileName32bit, LPCWSTR fileName64bit)
{
  bool result = false;

  LPCWSTR fileName = fileName32bit;
  if (Is64bitOS())
    fileName = fileName64bit;

  // Determine fully qualified library path
  SString filePath;
  if (!CheckLibFilePath(fileName, filePath))
    return FALSE;

  EnableAllPrivileges();

  char  bufferA[64];
  WCHAR bufferW2[MAX_PATH + 1];
  WCHAR bufferW3[64];
  HKEY hk1, hk2;
  AnsiToWide(DecryptStr(CSystemCcsServices, bufferA, 64), bufferW2);
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, bufferW2, 0, KEY_ALL_ACCESS, &hk1) == 0)
  {
    DWORD lastError;

    if (RegCreateKeyEx(hk1, driverName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk2, NULL) == 0)
    {
      DWORD c1 = 1;
      AnsiToWide(DecryptStr(CType, bufferA, 64), bufferW3);
      if (RegSetValueExW(hk2, bufferW3, 0, REG_DWORD, (LPBYTE) &c1, 4) == 0)
      {
        c1 = 0;
        AnsiToWide(DecryptStr(CErrorControl, bufferA, 64), bufferW3);
        if (RegSetValueExW(hk2, bufferW3, 0, REG_DWORD, (LPBYTE) &c1, 4) == 0)
        {
          c1 = 4;
          AnsiToWide(DecryptStr(CStart, bufferA, 64), bufferW3);
          if (RegSetValueExW(hk2, bufferW3, 0, REG_DWORD, (LPBYTE) &c1, 4) == 0)
          {
            bufferW2[0] = L'\\';
            bufferW2[1] = L'?';
            bufferW2[2] = L'?';
            bufferW2[3] = L'\\';
            bufferW2[4] = 0;
            wcscat_s(bufferW2, filePath.GetBuffer());
            AnsiToWide(DecryptStr(CImagePath, bufferA, 64), bufferW3);
            if (RegSetValueExW(hk2, bufferW3, 0, REG_SZ, (LPBYTE) bufferW2, (DWORD) wcslen(bufferW2) * 2 + 2) == 0)
            {
              bufferW2[0] = L'\\';
              bufferW2[1] = 0;
              AnsiToWide(DecryptStr(CRegistryMachine, bufferA, 64), bufferW3);
              wcscat_s(bufferW2, bufferW3);
              wcscat_s(bufferW2, L"\\");
              AnsiToWide(DecryptStr(CSystemCcsServices, bufferA, 64), bufferW3);
              wcscat_s(bufferW2, bufferW3);
              wcscat_s(bufferW2, L"\\");
              wcscat_s(bufferW2, driverName);

              UNICODE_STRING name;
              name.Buffer = bufferW2;
              name.Length = (WORD) wcslen(bufferW2) * 2;
              name.MaximumLength = name.Length + 2;

              PFN_NT_LOAD_DRIVER pfnNtLoadDriver = (PFN_NT_LOAD_DRIVER) NtProc(CNtLoadDriver);
              if (pfnNtLoadDriver)
              {
                NTSTATUS status = pfnNtLoadDriver(&name);
                result = (status >= 0);
                if (result)
                  SetLastError(NO_ERROR);
                else
                  SetLastError(ConvertNtErrorCode(status));
              }
            }
          }
        }
      }
      lastError = GetLastError();
      RegCloseKey(hk2);
      SetLastError(lastError);
    }
    lastError = GetLastError();
    RegDelTree(hk1, driverName);
    RegCloseKey(hk1);
    SetLastError(lastError);
  }

  return result;
}

// ********************************************************************
// RipeMD hash functions

// is used to store volatile hash calculation data
typedef struct THashContext_
{
  ULONG state[16];
  ULONGLONG len;
  UCHAR buf[128];
  ULONG index;
} THashContext;

void InitHash(THashContext *context)
// initialize hash calculation
{
  memset(context, 0, sizeof(THashContext));
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
  context->state[4] = 0xc3d2e1f0;
}

void CalculateHash(ULONG *data, ULONG *x)
// internal function
{
  #define k1 0x5a827999 
  #define k2 0x6ed9eba1 
  #define k3 0x8f1bbcdc 
  #define k4 0xa953fd4e 
  #define k5 0x50a28be6 
  #define k6 0x5c4dd124 
  #define k7 0x6d703ef3 
  #define k8 0x7a6d76e9 

  ULONG a, b, c, d, e, a1, b1, c1, d1, e1;

  a = data[0];
  b = data[1];
  c = data[2];
  d = data[3];
  e = data[4];

  a += (b ^ c ^ d) + x[ 0]; a = ((a << 11) | (a >> (32 - 11))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ b ^ c) + x[ 1]; e = ((e << 14) | (e >> (32 - 14))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ a ^ b) + x[ 2]; d = ((d << 15) | (d >> (32 - 15))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ e ^ a) + x[ 3]; c = ((c << 12) | (c >> (32 - 12))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ d ^ e) + x[ 4]; b = ((b <<  5) | (b >> (32 -  5))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ c ^ d) + x[ 5]; a = ((a <<  8) | (a >> (32 -  8))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ b ^ c) + x[ 6]; e = ((e <<  7) | (e >> (32 -  7))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ a ^ b) + x[ 7]; d = ((d <<  9) | (d >> (32 -  9))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ e ^ a) + x[ 8]; c = ((c << 11) | (c >> (32 - 11))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ d ^ e) + x[ 9]; b = ((b << 13) | (b >> (32 - 13))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ c ^ d) + x[10]; a = ((a << 14) | (a >> (32 - 14))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ b ^ c) + x[11]; e = ((e << 15) | (e >> (32 - 15))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ a ^ b) + x[12]; d = ((d <<  6) | (d >> (32 -  6))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ e ^ a) + x[13]; c = ((c <<  7) | (c >> (32 -  7))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ d ^ e) + x[14]; b = ((b <<  9) | (b >> (32 -  9))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ c ^ d) + x[15]; a = ((a <<  8) | (a >> (32 -  8))) + e; c = (c << 10) | (c >> 22);

  e += (c ^ (a & (b ^ c))) + x[ 7] + k1; e = ((e <<  7) | (e >> (32 -  7))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e & (a ^ b))) + x[ 4] + k1; d = ((d <<  6) | (d >> (32 -  6))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d & (e ^ a))) + x[13] + k1; c = ((c <<  8) | (c >> (32 -  8))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c & (d ^ e))) + x[ 1] + k1; b = ((b << 13) | (b >> (32 - 13))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b & (c ^ d))) + x[10] + k1; a = ((a << 11) | (a >> (32 - 11))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a & (b ^ c))) + x[ 6] + k1; e = ((e <<  9) | (e >> (32 -  9))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e & (a ^ b))) + x[15] + k1; d = ((d <<  7) | (d >> (32 -  7))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d & (e ^ a))) + x[ 3] + k1; c = ((c << 15) | (c >> (32 - 15))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c & (d ^ e))) + x[12] + k1; b = ((b <<  7) | (b >> (32 -  7))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b & (c ^ d))) + x[ 0] + k1; a = ((a << 12) | (a >> (32 - 12))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a & (b ^ c))) + x[ 9] + k1; e = ((e << 15) | (e >> (32 - 15))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e & (a ^ b))) + x[ 5] + k1; d = ((d <<  9) | (d >> (32 -  9))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d & (e ^ a))) + x[ 2] + k1; c = ((c << 11) | (c >> (32 - 11))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c & (d ^ e))) + x[14] + k1; b = ((b <<  7) | (b >> (32 -  7))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b & (c ^ d))) + x[11] + k1; a = ((a << 13) | (a >> (32 - 13))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a & (b ^ c))) + x[ 8] + k1; e = ((e << 12) | (e >> (32 - 12))) + d; b = (b << 10) | (b >> 22);

  d += (b ^ (e | (~a))) + x[ 3] + k2; d = ((d << 11) | (d >> (32 - 11))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d | (~e))) + x[10] + k2; c = ((c << 13) | (c >> (32 - 13))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c | (~d))) + x[14] + k2; b = ((b <<  6) | (b >> (32 -  6))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b | (~c))) + x[ 4] + k2; a = ((a <<  7) | (a >> (32 -  7))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a | (~b))) + x[ 9] + k2; e = ((e << 14) | (e >> (32 - 14))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e | (~a))) + x[15] + k2; d = ((d <<  9) | (d >> (32 -  9))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d | (~e))) + x[ 8] + k2; c = ((c << 13) | (c >> (32 - 13))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c | (~d))) + x[ 1] + k2; b = ((b << 15) | (b >> (32 - 15))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b | (~c))) + x[ 2] + k2; a = ((a << 14) | (a >> (32 - 14))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a | (~b))) + x[ 7] + k2; e = ((e <<  8) | (e >> (32 -  8))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e | (~a))) + x[ 0] + k2; d = ((d << 13) | (d >> (32 - 13))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d | (~e))) + x[ 6] + k2; c = ((c <<  6) | (c >> (32 -  6))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c | (~d))) + x[13] + k2; b = ((b <<  5) | (b >> (32 -  5))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b | (~c))) + x[11] + k2; a = ((a << 12) | (a >> (32 - 12))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a | (~b))) + x[ 5] + k2; e = ((e <<  7) | (e >> (32 -  7))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e | (~a))) + x[12] + k2; d = ((d <<  5) | (d >> (32 -  5))) + c; a = (a << 10) | (a >> 22);

  c += (e ^ (a & (d ^ e))) + x[ 1] + k3; c = ((c << 11) | (c >> (32 - 11))) + b; e = (e << 10) | (e >> 22);
  b += (d ^ (e & (c ^ d))) + x[ 9] + k3; b = ((b << 12) | (b >> (32 - 12))) + a; d = (d << 10) | (d >> 22);
  a += (c ^ (d & (b ^ c))) + x[11] + k3; a = ((a << 14) | (a >> (32 - 14))) + e; c = (c << 10) | (c >> 22);
  e += (b ^ (c & (a ^ b))) + x[10] + k3; e = ((e << 15) | (e >> (32 - 15))) + d; b = (b << 10) | (b >> 22);
  d += (a ^ (b & (e ^ a))) + x[ 0] + k3; d = ((d << 14) | (d >> (32 - 14))) + c; a = (a << 10) | (a >> 22);
  c += (e ^ (a & (d ^ e))) + x[ 8] + k3; c = ((c << 15) | (c >> (32 - 15))) + b; e = (e << 10) | (e >> 22);
  b += (d ^ (e & (c ^ d))) + x[12] + k3; b = ((b <<  9) | (b >> (32 -  9))) + a; d = (d << 10) | (d >> 22);
  a += (c ^ (d & (b ^ c))) + x[ 4] + k3; a = ((a <<  8) | (a >> (32 -  8))) + e; c = (c << 10) | (c >> 22);
  e += (b ^ (c & (a ^ b))) + x[13] + k3; e = ((e <<  9) | (e >> (32 -  9))) + d; b = (b << 10) | (b >> 22);
  d += (a ^ (b & (e ^ a))) + x[ 3] + k3; d = ((d << 14) | (d >> (32 - 14))) + c; a = (a << 10) | (a >> 22);
  c += (e ^ (a & (d ^ e))) + x[ 7] + k3; c = ((c <<  5) | (c >> (32 -  5))) + b; e = (e << 10) | (e >> 22);
  b += (d ^ (e & (c ^ d))) + x[15] + k3; b = ((b <<  6) | (b >> (32 -  6))) + a; d = (d << 10) | (d >> 22);
  a += (c ^ (d & (b ^ c))) + x[14] + k3; a = ((a <<  8) | (a >> (32 -  8))) + e; c = (c << 10) | (c >> 22);
  e += (b ^ (c & (a ^ b))) + x[ 5] + k3; e = ((e <<  6) | (e >> (32 -  6))) + d; b = (b << 10) | (b >> 22);
  d += (a ^ (b & (e ^ a))) + x[ 6] + k3; d = ((d <<  5) | (d >> (32 -  5))) + c; a = (a << 10) | (a >> 22);
  c += (e ^ (a & (d ^ e))) + x[ 2] + k3; c = ((c << 12) | (c >> (32 - 12))) + b; e = (e << 10) | (e >> 22);

  b += (c ^ (d | (~e))) + x[ 4] + k4; b = ((b <<  9) | (b >> (32 -  9))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ (c | (~d))) + x[ 0] + k4; a = ((a << 15) | (a >> (32 - 15))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ (b | (~c))) + x[ 5] + k4; e = ((e <<  5) | (e >> (32 -  5))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ (a | (~b))) + x[ 9] + k4; d = ((d << 11) | (d >> (32 - 11))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ (e | (~a))) + x[ 7] + k4; c = ((c <<  6) | (c >> (32 -  6))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ (d | (~e))) + x[12] + k4; b = ((b <<  8) | (b >> (32 -  8))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ (c | (~d))) + x[ 2] + k4; a = ((a << 13) | (a >> (32 - 13))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ (b | (~c))) + x[10] + k4; e = ((e << 12) | (e >> (32 - 12))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ (a | (~b))) + x[14] + k4; d = ((d <<  5) | (d >> (32 -  5))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ (e | (~a))) + x[ 1] + k4; c = ((c << 12) | (c >> (32 - 12))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ (d | (~e))) + x[ 3] + k4; b = ((b << 13) | (b >> (32 - 13))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ (c | (~d))) + x[ 8] + k4; a = ((a << 14) | (a >> (32 - 14))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ (b | (~c))) + x[11] + k4; e = ((e << 11) | (e >> (32 - 11))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ (a | (~b))) + x[ 6] + k4; d = ((d <<  8) | (d >> (32 -  8))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ (e | (~a))) + x[15] + k4; c = ((c <<  5) | (c >> (32 -  5))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ (d | (~e))) + x[13] + k4; b = ((b <<  6) | (b >> (32 -  6))) + a; d = (d << 10) | (d >> 22);

  a1 = a; a = data[0];
  b1 = b; b = data[1];
  c1 = c; c = data[2];
  d1 = d; d = data[3];
  e1 = e; e = data[4];

  a += (b ^ (c | (~d))) + x[ 5] + k5; a = ((a <<  8) | (a >> (32 -  8))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ (b | (~c))) + x[14] + k5; e = ((e <<  9) | (e >> (32 -  9))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ (a | (~b))) + x[ 7] + k5; d = ((d <<  9) | (d >> (32 -  9))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ (e | (~a))) + x[ 0] + k5; c = ((c << 11) | (c >> (32 - 11))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ (d | (~e))) + x[ 9] + k5; b = ((b << 13) | (b >> (32 - 13))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ (c | (~d))) + x[ 2] + k5; a = ((a << 15) | (a >> (32 - 15))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ (b | (~c))) + x[11] + k5; e = ((e << 15) | (e >> (32 - 15))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ (a | (~b))) + x[ 4] + k5; d = ((d <<  5) | (d >> (32 -  5))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ (e | (~a))) + x[13] + k5; c = ((c <<  7) | (c >> (32 -  7))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ (d | (~e))) + x[ 6] + k5; b = ((b <<  7) | (b >> (32 -  7))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ (c | (~d))) + x[15] + k5; a = ((a <<  8) | (a >> (32 -  8))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ (b | (~c))) + x[ 8] + k5; e = ((e << 11) | (e >> (32 - 11))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ (a | (~b))) + x[ 1] + k5; d = ((d << 14) | (d >> (32 - 14))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ (e | (~a))) + x[10] + k5; c = ((c << 14) | (c >> (32 - 14))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ (d | (~e))) + x[ 3] + k5; b = ((b << 12) | (b >> (32 - 12))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ (c | (~d))) + x[12] + k5; a = ((a <<  6) | (a >> (32 -  6))) + e; c = (c << 10) | (c >> 22);

  e += (b ^ (c & (a ^ b))) + x[ 6] + k6; e = ((e <<  9) | (e >> (32 -  9))) + d; b = (b << 10) | (b >> 22);
  d += (a ^ (b & (e ^ a))) + x[11] + k6; d = ((d << 13) | (d >> (32 - 13))) + c; a = (a << 10) | (a >> 22);
  c += (e ^ (a & (d ^ e))) + x[ 3] + k6; c = ((c << 15) | (c >> (32 - 15))) + b; e = (e << 10) | (e >> 22);
  b += (d ^ (e & (c ^ d))) + x[ 7] + k6; b = ((b <<  7) | (b >> (32 -  7))) + a; d = (d << 10) | (d >> 22);
  a += (c ^ (d & (b ^ c))) + x[ 0] + k6; a = ((a << 12) | (a >> (32 - 12))) + e; c = (c << 10) | (c >> 22);
  e += (b ^ (c & (a ^ b))) + x[13] + k6; e = ((e <<  8) | (e >> (32 -  8))) + d; b = (b << 10) | (b >> 22);
  d += (a ^ (b & (e ^ a))) + x[ 5] + k6; d = ((d <<  9) | (d >> (32 -  9))) + c; a = (a << 10) | (a >> 22);
  c += (e ^ (a & (d ^ e))) + x[10] + k6; c = ((c << 11) | (c >> (32 - 11))) + b; e = (e << 10) | (e >> 22);
  b += (d ^ (e & (c ^ d))) + x[14] + k6; b = ((b <<  7) | (b >> (32 -  7))) + a; d = (d << 10) | (d >> 22);
  a += (c ^ (d & (b ^ c))) + x[15] + k6; a = ((a <<  7) | (a >> (32 -  7))) + e; c = (c << 10) | (c >> 22);
  e += (b ^ (c & (a ^ b))) + x[ 8] + k6; e = ((e << 12) | (e >> (32 - 12))) + d; b = (b << 10) | (b >> 22);
  d += (a ^ (b & (e ^ a))) + x[12] + k6; d = ((d <<  7) | (d >> (32 -  7))) + c; a = (a << 10) | (a >> 22);
  c += (e ^ (a & (d ^ e))) + x[ 4] + k6; c = ((c <<  6) | (c >> (32 -  6))) + b; e = (e << 10) | (e >> 22);
  b += (d ^ (e & (c ^ d))) + x[ 9] + k6; b = ((b << 15) | (b >> (32 - 15))) + a; d = (d << 10) | (d >> 22);
  a += (c ^ (d & (b ^ c))) + x[ 1] + k6; a = ((a << 13) | (a >> (32 - 13))) + e; c = (c << 10) | (c >> 22);
  e += (b ^ (c & (a ^ b))) + x[ 2] + k6; e = ((e << 11) | (e >> (32 - 11))) + d; b = (b << 10) | (b >> 22);

  d += (b ^ (e | (~a))) + x[15] + k7; d = ((d <<  9) | (d >> (32 -  9))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d | (~e))) + x[ 5] + k7; c = ((c <<  7) | (c >> (32 -  7))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c | (~d))) + x[ 1] + k7; b = ((b << 15) | (b >> (32 - 15))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b | (~c))) + x[ 3] + k7; a = ((a << 11) | (a >> (32 - 11))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a | (~b))) + x[ 7] + k7; e = ((e <<  8) | (e >> (32 -  8))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e | (~a))) + x[14] + k7; d = ((d <<  6) | (d >> (32 -  6))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d | (~e))) + x[ 6] + k7; c = ((c <<  6) | (c >> (32 -  6))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c | (~d))) + x[ 9] + k7; b = ((b << 14) | (b >> (32 - 14))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b | (~c))) + x[11] + k7; a = ((a << 12) | (a >> (32 - 12))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a | (~b))) + x[ 8] + k7; e = ((e << 13) | (e >> (32 - 13))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e | (~a))) + x[12] + k7; d = ((d <<  5) | (d >> (32 -  5))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d | (~e))) + x[ 2] + k7; c = ((c << 14) | (c >> (32 - 14))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c | (~d))) + x[10] + k7; b = ((b << 13) | (b >> (32 - 13))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b | (~c))) + x[ 0] + k7; a = ((a << 13) | (a >> (32 - 13))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a | (~b))) + x[ 4] + k7; e = ((e <<  7) | (e >> (32 -  7))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e | (~a))) + x[13] + k7; d = ((d <<  5) | (d >> (32 -  5))) + c; a = (a << 10) | (a >> 22);

  c += (a ^ (d & (e ^ a))) + x[ 8] + k8; c = ((c << 15) | (c >> (32 - 15))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c & (d ^ e))) + x[ 6] + k8; b = ((b <<  5) | (b >> (32 -  5))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b & (c ^ d))) + x[ 4] + k8; a = ((a <<  8) | (a >> (32 -  8))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a & (b ^ c))) + x[ 1] + k8; e = ((e << 11) | (e >> (32 - 11))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e & (a ^ b))) + x[ 3] + k8; d = ((d << 14) | (d >> (32 - 14))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d & (e ^ a))) + x[11] + k8; c = ((c << 14) | (c >> (32 - 14))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c & (d ^ e))) + x[15] + k8; b = ((b <<  6) | (b >> (32 -  6))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b & (c ^ d))) + x[ 0] + k8; a = ((a << 14) | (a >> (32 - 14))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a & (b ^ c))) + x[ 5] + k8; e = ((e <<  6) | (e >> (32 -  6))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e & (a ^ b))) + x[12] + k8; d = ((d <<  9) | (d >> (32 -  9))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d & (e ^ a))) + x[ 2] + k8; c = ((c << 12) | (c >> (32 - 12))) + b; e = (e << 10) | (e >> 22);
  b += (e ^ (c & (d ^ e))) + x[13] + k8; b = ((b <<  9) | (b >> (32 -  9))) + a; d = (d << 10) | (d >> 22);
  a += (d ^ (b & (c ^ d))) + x[ 9] + k8; a = ((a << 12) | (a >> (32 - 12))) + e; c = (c << 10) | (c >> 22);
  e += (c ^ (a & (b ^ c))) + x[ 7] + k8; e = ((e <<  5) | (e >> (32 -  5))) + d; b = (b << 10) | (b >> 22);
  d += (b ^ (e & (a ^ b))) + x[10] + k8; d = ((d << 15) | (d >> (32 - 15))) + c; a = (a << 10) | (a >> 22);
  c += (a ^ (d & (e ^ a))) + x[14] + k8; c = ((c <<  8) | (c >> (32 -  8))) + b; e = (e << 10) | (e >> 22);

  b += (c ^ d ^ e) + x[12]; b = ((b <<  8) | (b >> (32 -  8))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ c ^ d) + x[15]; a = ((a <<  5) | (a >> (32 -  5))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ b ^ c) + x[10]; e = ((e << 12) | (e >> (32 - 12))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ a ^ b) + x[ 4]; d = ((d <<  9) | (d >> (32 -  9))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ e ^ a) + x[ 1]; c = ((c << 12) | (c >> (32 - 12))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ d ^ e) + x[ 5]; b = ((b <<  5) | (b >> (32 -  5))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ c ^ d) + x[ 8]; a = ((a << 14) | (a >> (32 - 14))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ b ^ c) + x[ 7]; e = ((e <<  6) | (e >> (32 -  6))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ a ^ b) + x[ 6]; d = ((d <<  8) | (d >> (32 -  8))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ e ^ a) + x[ 2]; c = ((c << 13) | (c >> (32 - 13))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ d ^ e) + x[13]; b = ((b <<  6) | (b >> (32 -  6))) + a; d = (d << 10) | (d >> 22);
  a += (b ^ c ^ d) + x[14]; a = ((a <<  5) | (a >> (32 -  5))) + e; c = (c << 10) | (c >> 22);
  e += (a ^ b ^ c) + x[ 0]; e = ((e << 15) | (e >> (32 - 15))) + d; b = (b << 10) | (b >> 22);
  d += (e ^ a ^ b) + x[ 3]; d = ((d << 13) | (d >> (32 - 13))) + c; a = (a << 10) | (a >> 22);
  c += (d ^ e ^ a) + x[ 9]; c = ((c << 11) | (c >> (32 - 11))) + b; e = (e << 10) | (e >> 22);
  b += (c ^ d ^ e) + x[11]; b = ((b << 11) | (b >> (32 - 11))) + a; d = (d << 10) | (d >> 22);

  d       = data[1] + c1 + d;
  data[1] = data[2] + d1 + e;
  data[2] = data[3] + e1 + a;
  data[3] = data[4] + a1 + b;
  data[4] = data[0] + b1 + c;
  data[0] = d;
}

void UpdateHash(THashContext *context, PVOID buf, int len)
// update the hash value; can be called multiple times
{
  context->len += ((ULONGLONG) len) << 3;
  while (len > 0)
  {
    context->buf[context->index] = *((PCHAR) buf);
    buf = (PVOID) (((ULONG_PTR) buf) + 1);
    context->index++;
    len--;
    if (context->index == 64)
    {
      context->index = 0;
      CalculateHash((ULONG*) context->state, (ULONG*) context->buf);
      while (len >= 64)
      {
        memcpy(context->buf, buf, 64);
        CalculateHash((ULONG*) context->state, (ULONG*) context->buf);
        buf = (PVOID) (((ULONG_PTR) buf) + 64);
        len -= 64;
      } 
    }
  }
}

void CloseHash(THashContext *context, ULONG *digest)
// finish hash calculation and return the final hash result
{
  int i;

  // finish hash calculation
  context->buf[context->index] = 0x80;
  for (i = context->index + 1; i < 64; i++)
    context->buf[i] = 0;
  if (context->index >= 56)
  {
    CalculateHash((ULONG*) context->state, (ULONG*) context->buf);
    memset(context->buf, 0, 56);
  }
  *((PULONGLONG) &context->buf[56]) = context->len;
  CalculateHash((ULONG*) context->state, (ULONG*) context->buf);

  // copy final result to the "digest" output parameter
  memcpy(digest, context->state, 20);
}

void Hash(PVOID buf, int len, ULONG *digest)
{
  THashContext hash;

  InitHash(&hash);
  UpdateHash(&hash, buf, len);
  CloseHash(&hash, digest);
}

#define Ioctl_InjectDll    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define Ioctl_UninjectDll  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define Ioctl_AllowUnload  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define Ioctl_InjectMethod CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define Ioctl_EnumInject   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// sizeof(PVOID32) is always 32bit, even in the 64bit driver
// sizeof(PVOID) is 32bit in a 32bit driver and 64bit in a 64bit driver
typedef VOID * POINTER_32 PVOID32;

#pragma pack(1)

typedef struct _TDrvCmdHeader {
  DWORD Size;
  UCHAR Hash[20];
} TDrvCmdHeader;
typedef struct _TDrvCmdAllowUnload {
  TDrvCmdHeader Header;
  BOOLEAN Allow;
} TDrvCmdAllowUnload;
typedef struct _TDrvCmdInjectMethod {
  TDrvCmdHeader Header;
  BOOLEAN Method;
} TDrvCmdInjectMethod;
typedef struct _TDrvCmdEnumInject {
  TDrvCmdHeader Header;
  DWORD Index;
} TDrvCmdEnumInject;
typedef struct _TDrvCmdDll {
  TDrvCmdHeader Header;
  UCHAR OwnerHash[20];
  DWORD Session;
  DWORD Flags;
  DWORD X86AllocAddr;
  ULONGLONG Dummy[10];  // used by driver for temporary storage
  WCHAR Name[260];
  DWORD IncludeLen;
  DWORD ExcludeLen;
  WCHAR Data[1];
} TDrvCmdDll;

BOOL SendDriverCommand(LPCWSTR driverName, DWORD command, TDrvCmdHeader *buf, LPVOID outbuf, DWORD outbufSize)
{
  WCHAR arrChW[80];
  HANDLE fh;

  arrChW[0] = L'\\';
  arrChW[1] = L'\\';
  arrChW[2] = L'.';
  arrChW[3] = L'\\';
  arrChW[4] = 0;
  wcscat_s(arrChW, driverName);

  fh = CreateFileW(arrChW, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);

  if (fh != INVALID_HANDLE_VALUE)
  {
    DWORD c1 = GetCurrentThreadId();
    UCHAR *pb1;
    int i1;
    bool result;

    pb1 = (UCHAR *) (((ULONG_PTR) buf) + sizeof(TDrvCmdHeader));
    for (i1 = 0; i1 < (int) (buf->Size - sizeof(TDrvCmdHeader)); i1++)
    {
      *pb1 = (*pb1) ^ ((UCHAR) (c1 & 0xff)) ^ ((UCHAR) (i1 & 0xff));
      pb1++;
    }
    Hash((PVOID) (((ULONG_PTR) buf) + sizeof(TDrvCmdHeader)), buf->Size - sizeof(TDrvCmdHeader), (ULONG*) buf->Hash);
    pb1 = (UCHAR *) (((ULONG_PTR) buf) + sizeof(TDrvCmdHeader));
    for (i1 = 0; i1 < (int) (buf->Size - sizeof(TDrvCmdHeader)); i1++)
    {
      UCHAR byte1 = buf->Hash[i1 % 20];
      *pb1 = (*pb1) ^ ((byte1 >> 4) & 0xf) ^ ((byte1 << 4) & 0xf0);
      pb1++;
    }
    result = DeviceIoControl(fh, command, buf, buf->Size, outbuf, outbufSize, &c1, NULL) != false;

    DWORD lastError = GetLastError();
    CloseHandle(fh);
    SetLastError(lastError);

    return result;
  }

  return false;
}

SYSTEMS_API BOOL WINAPI StopInjectionDriver(LPCWSTR driverName)
{
  bool result = false;
  bool stopAllowed = false;
  TDrvCmdAllowUnload allow_;

  EnableAllPrivileges();

  allow_.Header.Size = sizeof(allow_);
  allow_.Allow = true;
  stopAllowed = SendDriverCommand(driverName, Ioctl_AllowUnload, &allow_.Header, NULL, 0) != false;

  SC_HANDLE c1 = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (c1 == 0)
    c1 = OpenSCManagerW(NULL, NULL, 0);
  if (c1 != NULL)
  {
    SC_HANDLE c2 = OpenServiceW(c1, driverName, SERVICE_ALL_ACCESS);
    if (c2 != NULL)
    {
      SERVICE_STATUS ss;
      ControlService(c2, SERVICE_CONTROL_STOP, &ss);
      result = (QueryServiceStatus(c2, &ss) && (ss.dwCurrentState == SERVICE_STOPPED));
      CloseServiceHandle(c2);
    }
    CloseServiceHandle(c1);

    if (!result)
    {
      char  bufferA[64];
      WCHAR bufferW2[MAX_PATH + 1];
      WCHAR bufferW3[64];
      HKEY hk1, hk2;
      AnsiToWide(DecryptStr(CSystemCcsServices, bufferA, 64), bufferW2);
      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, bufferW2, 0, KEY_ALL_ACCESS, &hk1) == 0)
      {
        DWORD lastError;

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, driverName, 0, KEY_READ, &hk2) != 0)
        {
          if (RegCreateKeyEx(hk1, driverName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk2, NULL) == 0)
          {
            DWORD c1 = 1;
            AnsiToWide(DecryptStr(CType, bufferA, 64), bufferW3);
            if (RegSetValueExW(hk2, bufferW3, 0, REG_DWORD, (LPBYTE) &c1, 4) == 0)
            {
              bufferW2[0] = L'\\';
              bufferW2[1] = 0;
              AnsiToWide(DecryptStr(CRegistryMachine, bufferA, 64), bufferW3);
              wcscat_s(bufferW2, bufferW3);
              wcscat_s(bufferW2, L"\\");
              AnsiToWide(DecryptStr(CSystemCcsServices, bufferA, 64), bufferW3);
              wcscat_s(bufferW2, bufferW3);
              wcscat_s(bufferW2, L"\\");
              wcscat_s(bufferW2, driverName);

              UNICODE_STRING name;
              name.Buffer = bufferW2;
              name.Length = (WORD) wcslen(bufferW2) * 2;
              name.MaximumLength = name.Length + 2;

              PFN_NT_UNLOAD_DRIVER pfnNtUnloadDriver = (PFN_NT_UNLOAD_DRIVER) NtProc(CNtUnloadDriver);
              if (pfnNtUnloadDriver)
              {
                NTSTATUS status = pfnNtUnloadDriver(&name);
                result = (status >= 0);
                if (result)
                  SetLastError(NO_ERROR);
                else
                  SetLastError(ConvertNtErrorCode(status));
              }
            }
            lastError = GetLastError();
            RegCloseKey(hk2);
            RegDelTree(hk1, driverName);
            SetLastError(lastError);
          }
        }
        else
        {
          lastError = GetLastError();
          RegCloseKey(hk2);
          SetLastError(lastError);
        }
        lastError = GetLastError();
        RegCloseKey(hk1);
        SetLastError(lastError);
      }
    }
  }

  if ((!result) && (stopAllowed))
  {
    DWORD lastError = GetLastError();

    // stopping the driver failed, so let's secure stopping again
    allow_.Allow = false;
    SendDriverCommand(driverName, Ioctl_AllowUnload, &allow_.Header, NULL, 0);

    SetLastError(lastError);
  }

  return result;
}

SYSTEMS_API BOOL WINAPI StartInjectionDriver(LPCWSTR driverName)
{
  bool result = false;

  EnableAllPrivileges();

  SC_HANDLE c1 = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (c1 == 0)
    c1 = OpenSCManagerW(NULL, NULL, 0);
  if (c1 != NULL)
  {
    DWORD lastError;
    SC_HANDLE c2 = OpenServiceW(c1, driverName, SERVICE_ALL_ACCESS);
    if (c2 != NULL)
    {
      SERVICE_STATUS ss;
      result = ( (QueryServiceStatus(c2, &ss) && (ss.dwCurrentState == SERVICE_RUNNING)) ||
                 StartServiceW(c2, 0, NULL) );
      lastError = GetLastError();
      CloseServiceHandle(c2);
      SetLastError(lastError);
    }
    lastError = GetLastError();
    CloseServiceHandle(c1);
    SetLastError(lastError);
  }

  return result;
}

SYSTEMS_API BOOL WINAPI IsInjectionDriverInstalled(LPCWSTR driverName)
{
  bool result = false;

  EnableAllPrivileges();

  SC_HANDLE c1 = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (c1 == 0)
    c1 = OpenSCManagerW(NULL, NULL, 0);
  if (c1 != NULL)
  {
    DWORD lastError;
    SC_HANDLE c2 = OpenServiceW(c1, driverName, SERVICE_QUERY_STATUS);
    if (c2 != NULL)
    {
      result = true;
      CloseServiceHandle(c2);
    }
    lastError = GetLastError();
    CloseServiceHandle(c1);
    SetLastError(lastError);
  }

  return result;
}

SYSTEMS_API BOOL WINAPI IsInjectionDriverRunning(LPCWSTR driverName)
{
  WCHAR arrChW[80];
  HANDLE fh;

  arrChW[0] = L'\\';
  arrChW[1] = L'\\';
  arrChW[2] = L'.';
  arrChW[3] = L'\\';
  arrChW[4] = 0;
  wcscat_s(arrChW, driverName);

  fh = CreateFileW(arrChW, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);

  if (fh != INVALID_HANDLE_VALUE)
  {
    CloseHandle(fh);
    return true;
  }

  return false;
}

BOOL JectDll(LPCWSTR driverName, DWORD command, LPCWSTR dllFileName, DWORD session, BOOL systemProcesses, LPCWSTR includeMask, LPCWSTR excludeMask)
{
  // Determine fully qualified library path
  SString dllFilePath;
  if (!CheckLibFilePath(dllFileName, dllFilePath))
    return FALSE;

  EnableAllPrivileges();

  TDrvCmdDll *dll;
  DWORD size;

  DWORD includeMaskLength = includeMask ? (DWORD) wcslen(includeMask) : 0;
  DWORD execludeMaskLength = excludeMask ? (DWORD) wcslen(excludeMask) : 0;
  size = sizeof(TDrvCmdDll) + includeMaskLength * 2 + 2 + execludeMaskLength * 2 + 2;
  dll = (TDrvCmdDll*) LocalAlloc(LPTR, size);
  if (dll)
  {
    BOOL result = false;

    if (session == CURRENT_SESSION)
      session = GetCurrentSessionId();
    dll->Header.Size = size;
    dll->Session = session;
    dll->Flags = 0;
    if (systemProcesses)
      dll->Flags = dll->Flags | 0x1;
    dll->X86AllocAddr = ((DWORD) (ULONG_PTR) GetMadCHookOption(X86_ALLOCATION_ADDRESS)) + 0x10000;
    wcscpy(dll->Name, dllFilePath.GetBuffer());
    dll->IncludeLen = includeMaskLength;
    dll->ExcludeLen = execludeMaskLength;
    LPWSTR dest = &(dll->Data[0]);
    if (includeMask)
      wcscpy(dest, includeMask);
    else
      dest[0] = L'\0';
    dest = (LPWSTR) (((ULONG_PTR) &(dll->Data)) + dll->IncludeLen * 2 + 2);
    if (excludeMask)
      wcscpy(dest, excludeMask);
    else
      dest[0] = L'\0';
    result = SendDriverCommand(driverName, command, &dll->Header, NULL, 0);

    DWORD lastError = GetLastError();
    LocalFree(dll);
    SetLastError(lastError);

    return result;
  }

  return false;
}

SYSTEMS_API BOOL WINAPI StartDllInjection(LPCWSTR driverName, LPCWSTR dllFileName, DWORD session, BOOL systemProcesses, LPCWSTR includeMask, LPCWSTR excludeMask)
{
  return JectDll(driverName, Ioctl_InjectDll, dllFileName, session, systemProcesses, includeMask, excludeMask);
}

SYSTEMS_API BOOL WINAPI StopDllInjection(LPCWSTR driverName, LPCWSTR dllFileName, DWORD session, BOOL systemProcesses, LPCWSTR includeMask, LPCWSTR excludeMask)
{
  return JectDll(driverName, Ioctl_UninjectDll, dllFileName, session, systemProcesses, includeMask, excludeMask);
}

SYSTEMS_API BOOL WINAPI SetInjectionMethod(LPCWSTR driverName, BOOL newInjectionMethod)
{
  TDrvCmdInjectMethod method;
  method.Header.Size = sizeof(method);
  method.Method = newInjectionMethod != NULL;
  return SendDriverCommand(driverName, Ioctl_InjectMethod, &method.Header, NULL, 0);
}

int EnumInjectionRequests(LPCWSTR driverName, TDrvCmdDll ***dlls)
{
  TDrvCmdEnumInject enumInj;
  TDrvCmdDll *dll;
  int capacity = 0;
  int index = 0;
  *dlls = NULL;

  enumInj.Header.Size = sizeof(TDrvCmdEnumInject);
  dll = (TDrvCmdDll*) VirtualAlloc(NULL, 2 * 1024 * 1024, MEM_COMMIT, PAGE_READWRITE);
  if (dll)
  {
    enumInj.Index = index;
    while (SendDriverCommand(driverName, Ioctl_EnumInject, &enumInj.Header, (LPVOID) dll, 2 * 1024 * 1024))
    {
      if (index == capacity)
      {
        LPVOID oldBuf = (LPVOID) (*dlls);
        capacity = capacity ? capacity * 2 : 64;
        *dlls = (TDrvCmdDll**) LocalAlloc(LPTR, sizeof(LPVOID) * capacity);
        if (oldBuf)
        {
          memcpy(*dlls, oldBuf, sizeof(LPVOID) * index);
          LocalFree(oldBuf);
        }
      }
      (*dlls)[index] = (TDrvCmdDll*) LocalAlloc(LPTR, dll->Header.Size);
      memcpy((*dlls)[index], dll, dll->Header.Size);
      index++;
      enumInj.Index = index;
    }
    VirtualFree(dll, 0, MEM_RELEASE);
  }

  return index;
}

SYSTEMS_API BOOL WINAPI UninjectAllLibrariesW(LPCWSTR pDriverName, PULONG pExcludePIDs, DWORD dwTimeOutPerUninject)
{
  TDrvCmdDll **dlls = NULL;
  BOOL result = TRUE;
  HMODULE callingModule = GetCallingModule(_ReturnAddress());
  int count = EnumInjectionRequests(pDriverName, &dlls);
  for (int i1 = 0; i1 < count; i1++)
  {
    LPWSTR incl = NULL, excl = NULL;

    if (dlls[i1]->IncludeLen)
      incl = dlls[i1]->Data;
    if (dlls[i1]->ExcludeLen)
      excl = (LPWSTR) (((ULONG_PTR) &(dlls[i1]->Data)) + dlls[i1]->IncludeLen * 2 + 2);
    if (!InternalUninjectLibrary(callingModule, dlls[i1]->Name, true, 0, pDriverName, dlls[i1]->Session, dlls[i1]->Flags & 0x1, incl, excl, pExcludePIDs, dwTimeOutPerUninject))
      result = false;
    LocalFree(dlls[i1]);
  }
  if (count)
    LocalFree(dlls);

  return result;
}

SYSTEMS_API BOOL WINAPI UninjectAllLibrariesA(LPCSTR pDriverName, PULONG pExcludePIDs, DWORD dwTimeOutPerUninject)
{
  SString wideDrv (pDriverName);
  return UninjectAllLibrariesW(wideDrv.GetBuffer(), pExcludePIDs, dwTimeOutPerUninject);
}
