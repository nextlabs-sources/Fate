// ***************************************************************
//  CEnumProcesses.cpp        version: 1.0.1  ·  date: 2010-03-28
//  -------------------------------------------------------------
//  enumerate all running processes
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-03-28 1.0.1 improved SID copy operation
// 2010-01-10 1.0.0 initial version

#define _CENUMPROCESSES_C

#define _CRT_SECURE_NO_WARNINGS

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

SYSTEMS_API void WINAPI AnsiToWide(LPCSTR pAnsi, LPWSTR pWide)
{
  TraceVerbose(L"%S(%S, %p)", __FUNCTION__, pAnsi, pWide);

  ASSERT(pWide != NULL);
  ASSERT(!IsBadWritePtr2(pWide, strlen(pAnsi) * 2 + 2));

  const CHAR* pSrc = pAnsi;
  WCHAR* pDst = pWide;
  pDst[0] = pSrc[0];
  while (pSrc[0] != '\0')
  {
    pDst++;
    pSrc++;
    pDst[0] = pSrc[0];
  }
}

SYSTEMS_API void WINAPI WideToAnsi(LPCWSTR pWide, LPSTR pAnsi)
{
  TraceVerbose(L"%S(%s, %p)", __FUNCTION__, pWide, pAnsi);

  ASSERT(pAnsi != NULL);
  ASSERT(!IsBadWritePtr2(pAnsi, wcslen(pWide) + 1));

  const WCHAR* pSrc = pWide;
  CHAR* pDst = pAnsi;
  pDst[0] = (char) pSrc[0];
  while (pSrc[0] != L'\0')
  {
    pDst++;
    pSrc++;
    pDst[0] = (char) pSrc[0];
  }
}

static PFN_QUERY_FULL_PROCESS_IMAGE_NAME pfnQueryFullProcessImageName = NULL;
static bool QueryFullProcessImageNameDone = false;

SYSTEMS_API BOOL WINAPI InternalProcessIdToFileNameW(DWORD processId, LPWSTR fileName, DWORD bufLenInChars, bool enumIfNecessary)
{
  TraceVerbose(L"%S(%X, %X)", __FUNCTION__, processId, fileName);

  ASSERT(processId != 0);
  ASSERT(fileName != NULL);
  ASSERT(!IsBadWritePtr2(fileName, MAX_PATH));

  BOOL result = false;

  DWORD dwError = GetLastError();

  fileName[0] = '?';
  fileName[1] = '\0';

  if (processId)
  {
    if (!QueryFullProcessImageNameDone)
    {
      pfnQueryFullProcessImageName = (PFN_QUERY_FULL_PROCESS_IMAGE_NAME) KernelProc(CQueryFullProcessImageName);
      QueryFullProcessImageNameDone = true;
    }
    if (pfnQueryFullProcessImageName)
    {
      HANDLE ph = OpenProcess(PROCESS_QUERY_INFORMATION, false, processId);
      if (!ph)
        ph = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, processId);
      if (ph)
      {
        DWORD bufLenDword = bufLenInChars;
        result = pfnQueryFullProcessImageName(ph, 0, fileName, &bufLenDword);
        CloseHandle(ph);
      }
    }
    else
    {
      HANDLE ph = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processId);
      if (ph)
      {
        ULONG_PTR peb = (ULONG_PTR) GetPeb(ph);
        #ifndef _WIN64
          ULONGLONG peb64 = 0;
          if ((!peb) && (Is64bitProcess(ph)))
            peb64 = GetPeb64(ph);
          if ((peb) || (peb64))
        #else
          if (peb)
        #endif
        {
          bool readOk = false;
          WORD ifnLen = 0;
          #ifndef _WIN64
            if (peb64)
            {
              ULONGLONG pp;
              UNICODE_STRING64 ifn;
              ULONGLONG len;
              ULONG flags;
              if ( ReadProcessMemory64(ph, peb64 + 0x20, &pp,    sizeof(pp),    &len) && (len == sizeof(pp))    && (pp) &&
                   ReadProcessMemory64(ph, pp    + 0x08, &flags, sizeof(flags), &len) && (len == sizeof(flags)) &&
                   ReadProcessMemory64(ph, pp    + 0x60, &ifn,   sizeof(ifn),   &len) && (len == sizeof(ifn))   && (ifn.Length) && (ifn.Buffer) )
              {
                if (ifn.Length >= bufLenInChars * 2)
                {
                  ifn.Buffer = ifn.Buffer + ifn.Length - (bufLenInChars - 1) * 2;
                  ifn.Length = (WORD) ((bufLenInChars - 1) * 2);
                }
                if (!(flags & 1))  // PROCESS_PARAMETERS_NORMALIZED
                  ifn.Buffer = ifn.Buffer + pp;
                readOk = (ReadProcessMemory64(ph, ifn.Buffer, fileName, ifn.Length, &len) && (len == ifn.Length));
                ifnLen = ifn.Length;
              }
            }
            else
          #endif
          {
            RTL_USER_PROCESS_PARAMETERS *pp;
            UNICODE_STRING ifn;
            SIZE_T len;
            ULONG flags;
            if ( ReadProcessMemory(ph, &((PEB_NT*) peb)->ProcessParameters, &pp, sizeof(pp), &len) && (len == sizeof(pp)) && (pp) &&
                 ReadProcessMemory(ph, &(pp->Flags), &flags, sizeof(flags), &len) && (len == sizeof(flags)) &&
                 ReadProcessMemory(ph, &(pp->ImagePathName), &ifn, sizeof(ifn), &len) && (len == sizeof(ifn)) && (ifn.Length) && (ifn.Buffer) )
            {
              if (ifn.Length >= bufLenInChars * 2)
              {
                ifn.Buffer = (LPWSTR) ((ULONG_PTR) ifn.Buffer + ifn.Length - (bufLenInChars - 1) * 2);
                ifn.Length = (USHORT) ((bufLenInChars - 1) * 2);
              }
              if (!(flags & 1))  // PROCESS_PARAMETERS_NORMALIZED
                ifn.Buffer = (LPWSTR) ((ULONG_PTR) ifn.Buffer + (ULONG_PTR) pp);
              readOk = (ReadProcessMemory(ph, ifn.Buffer, fileName, ifn.Length, &len) && (len == ifn.Length));
              ifnLen = ifn.Length;
            }
          }
          if (readOk)
          {
            WCHAR helperChar;
            int compare;
            fileName[ifnLen / 2] = 0;
            helperChar = fileName[4];
            fileName[4] = 0;
            compare = _wcsicmp(L"\\??\\", fileName);
            fileName[4] = helperChar;
            if (!compare)
              memmove(fileName, &fileName[4], ifnLen + 2 - 4 * 2);
            else
            {
              helperChar = fileName[12];
              fileName[12] = 0;
              compare = _wcsicmp(L"\\SystemRoot\\", fileName);
              fileName[12] = helperChar;
              if (!compare)
              {
                WCHAR arrCh[MAX_PATH];
                int len = GetWindowsDirectoryW(arrCh, MAX_PATH);
                if ((DWORD) ifnLen / 2 + 1 - 11 + len >= bufLenInChars)
                {
                  ifnLen = (WORD) ((bufLenInChars - 1 + 11 - len) * 2);
                  fileName[ifnLen / 2] = 0;
                }
                memmove(&fileName[len], &fileName[11], ifnLen + 2 - 11 * 2);
                memmove(fileName, arrCh, len * 2);
              }
            }
            result = true;
          }
        }
        CloseHandle(ph);
      }
    }
  }

  if ((!result) && (enumIfNecessary))
  {
    __try
    {
      PFN_NT_QUERY_SYSTEM_INFORMATION pNtQuerySi = (PFN_NT_QUERY_SYSTEM_INFORMATION) NtProc(CNtQuerySystemInformation);

      ULONG retLength = 0;
      pNtQuerySi(SystemProcessInformation, NULL, 0, &retLength);

      NTSTATUS status;
      HLOCAL p = NULL;
      if (retLength == 0)
      {
        retLength = 0x10000;
        do
        {
          retLength = retLength * 2;
          LocalFree(p);
          p = LocalAlloc(LPTR, retLength);
          status = pNtQuerySi(SystemProcessInformation, p, retLength, NULL);
        } while ((status != 0) && (retLength != 0x400000));
      }
      else
      {
        retLength = retLength * 2;
        p = LocalAlloc(LPTR, retLength);
        status = pNtQuerySi(SystemProcessInformation, p, retLength, NULL);
      }
      if (status == 0)
      {
        SYSTEM_PROCESS_INFORMATION *pSpi = (SYSTEM_PROCESS_INFORMATION *) p;
        while (TRUE)
        {
          if ((DWORD) pSpi->Process.UniqueProcessId == processId)
          {
            if (pSpi->Process.Name.Buffer != NULL)
              wcscpy_s(fileName, bufLenInChars, pSpi->Process.Name.Buffer);
            else
              wcscpy_s(fileName, bufLenInChars, L"[System Process]");
            break;
          }
          if (pSpi->Process.Next == 0)
            break;
          pSpi = (SYSTEM_PROCESS_INFORMATION *) ((ULONG_PTR) pSpi + pSpi->Process.Next);
        }
      }
      LocalFree(p);
    }
    __except (ExceptionFilter(L"CEnumProcesses Constructor", GetExceptionInformation()))
    {
    }
  }

  SetLastError(dwError);
  return result;
}

SYSTEMS_API BOOL WINAPI ProcessIdToFileNameW(DWORD processId, LPWSTR fileName, USHORT bufLenInChars)
{
  return InternalProcessIdToFileNameW(processId, fileName, bufLenInChars, true);
}

SYSTEMS_API BOOL WINAPI ProcessIdToFileNameA(DWORD processId, LPSTR fileName, USHORT bufLenInChars)
{
  LPWSTR wideStr = (LPWSTR) LocalAlloc(LPTR, bufLenInChars * 2);
  BOOL result = false;
  if (ProcessIdToFileNameW(processId, wideStr, bufLenInChars))
  {
    WideToAnsi(wideStr, fileName);
    result = true;
  }
  LocalFree(wideStr);
  return result;
}

CEnumProcesses::CEnumProcesses(BOOL needFullPath) : mArray(), mIsValid(false)
{
  __try
  {
    PFN_NT_QUERY_SYSTEM_INFORMATION pNtQuerySi = (PFN_NT_QUERY_SYSTEM_INFORMATION) NtProc(CNtQuerySystemInformation);

    ULONG retLength = 0;
    pNtQuerySi(SystemProcessInformation, NULL, 0, &retLength);

    NTSTATUS status;
    HLOCAL p = NULL;
    if (retLength == 0)
    {
      retLength = 0x10000;
      do
      {
        retLength = retLength * 2;
        LocalFree(p);
        p = LocalAlloc(LPTR, retLength);
        status = pNtQuerySi(SystemProcessInformation, p, retLength, NULL);
      } while ((status != 0) && (retLength != 0x400000));
    }
    else
    {
      retLength = retLength * 2;
      p = LocalAlloc(LPTR, retLength);
      status = pNtQuerySi(SystemProcessInformation, p, retLength, NULL);
    }
    if (status == 0)
    {
      SYSTEM_PROCESS_INFORMATION *pSpi = (SYSTEM_PROCESS_INFORMATION *) p;
      LPWSTR helperBuf = NULL;
      if (needFullPath)
        helperBuf = (LPWSTR) LocalAlloc(LPTR, 64 * 1024);
      while (TRUE)
      {
        if (pSpi->Process.Name.Buffer != NULL)
        {
          if ((helperBuf) && (InternalProcessIdToFileNameW((DWORD) pSpi->Process.UniqueProcessId, helperBuf, 32 * 1024, false)))
            AddRecord((DWORD) pSpi->Process.UniqueProcessId, pSpi->Process.SessionId, helperBuf, NULL);
          else
            AddRecord((DWORD) pSpi->Process.UniqueProcessId, pSpi->Process.SessionId, pSpi->Process.Name.Buffer, NULL);
        }
        else
          AddRecord((DWORD) pSpi->Process.UniqueProcessId, pSpi->Process.SessionId, L"[System Process]", NULL);
        if (pSpi->Process.Next == 0)
          break;
        pSpi = (SYSTEM_PROCESS_INFORMATION *) ((ULONG_PTR) pSpi + pSpi->Process.Next);
      }
      if (helperBuf)
        LocalFree(helperBuf);
    }
    LocalFree(p);
    mIsValid = true;
  }
  __except (ExceptionFilter(L"CEnumProcesses Constructor", GetExceptionInformation()))
  {
    mIsValid = false;
  }
}

CEnumProcesses::~CEnumProcesses()
{
  for (int i1 = 0; i1 < mArray.GetCount(); i1++)
    if (mArray[i1].ExeFile)
      LocalFree(mArray[i1].ExeFile);
}

int CEnumProcesses::GetCount()
{
  return mArray.GetCount();
}

const PROCESS_RECORD& CEnumProcesses::operator[] (int index) const
{
  return mArray[index];
}

void CEnumProcesses::AddRecord(DWORD id, DWORD sessionId, wchar_t *exeFile, wchar_t *sid)
{
  PROCESS_RECORD record;

  record.Id = id;
  record.SessionId = sessionId;
  if ((exeFile != NULL) && (!IsBadReadPtr2(exeFile, 1)))
  {
    record.ExeFile = (LPWSTR) LocalAlloc(LPTR, lstrlenW(exeFile) * 2 + 2);
    lstrcpyW(record.ExeFile, exeFile);
  }
  if ((sid != NULL) && (!IsBadReadPtr2(sid, 1)) && (GetLengthSid(sid)) && (GetLengthSid(sid) <= sizeof(record.UserSID)))
    memcpy(record.UserSID, sid, GetLengthSid(sid));
  else
    ZeroMemory(record.UserSID, sizeof(record.UserSID));
  mArray.Add(record);
}

void CEnumProcesses::RemoveAt(int index)
{
  if (mArray[index].ExeFile)
    LocalFree(mArray[index].ExeFile);
  mArray.RemoveAt(index);
}
