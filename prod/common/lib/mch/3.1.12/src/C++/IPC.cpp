// ***************************************************************
//  IPC.cpp                   version: 1.0.9  ·  date: 2016-05-19
//  -------------------------------------------------------------
//  inter process communication functionality
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2016 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2016-05-19 1.0.9 fixed: SendIpcMessage blocked if queue didn't exist
// 2016-04-29 1.0.8 fixed: IPC reply sometimes didn't arrive (missing PID)
// 2016-03-16 1.0.7 fixed: resource handling bug in case of failure
// 2015-04-20 1.0.6 fixed: rare IPC stability bug
// 2013-12-03 1.0.5 fixed a rare crash
// 2013-04-26 1.0.4 fixed: IPC in Metro apps only worked without replies
// 2012-09-03 1.0.3 added support for Metro (AppContainer integrity) apps
// 2010-03-24 1.0.2 fixed format ansi/wide mismatch
// 2010-01-27 1.0.1 fixed: non-admin users sending IPC messages didn't work
// 2010-01-10 1.0.0 initial version

#define _IPC_C

#define _CRT_SECURE_NO_DEPRECATE

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

// LPC API func ptrs
static PFN_NTCREATEPORT pNtCreatePort = NULL;
static PFN_NTCONNECTPORT pNtConnectPort = NULL;
static PFN_NTREPLYWAITRECEIVEPORT pNtReplyWaitReceivePort = NULL;
static PFN_NTACCEPTCONNECTPORT pNtAcceptConnectPort = NULL;
static PFN_NTCOMPLETECONNECTPORT pNtCompleteConnectPort = NULL;

// globals
static CRITICAL_SECTION LpcSection;
static bool LpcReady = false;
static CCollection<P_LPC_QUEUE> LpcList;
static int *LpcCounterBuf = NULL;
static int IsVistaFlag = -1;
static bool InitCalled = false;

// list of possible message types that can be received with NtReplyWaitReceivePort
static LPC_MESSAGE_TYPE LpcMessageTypes[] = {
  {0x01, L"LPC_REQUEST"},
  {0x02, L"LPC_REPLY"},
  {0x03, L"LPC_DATAGRAM"},
  {0x04, L"LPC_LOST_REPLY"},
  {0x05, L"LPC_PORT_CLOSED"},
  {0x06, L"LPC_CLIENT_DIED"},
  {0x07, L"LPC_EXCEPTION"},
  {0x08, L"LPC_DEBUG_EVENT"},
  {0x09, L"LPC_ERROR_EVENT"},
  {0x0A, L"LPC_CONNECTION_REQUEST"}
};

// list of errors that the Nt* functions could return along with the symbol name and a brief description
static LPC_ERROR_INFO LpcErrorDefinitions[] = {
   {0xC0000005, L"STATUS_ACCESS_VIOLATION",        L"An Access Violation occured."},
   {0xC0000008, L"STATUS_INVALID_HANDLE",          L"An invalid handle was specified."},
   {0xC000000D, L"STATUS_INVALID_PARAMETER",       L"An invalid parameter was passed to a service or function."},
   {0xC000002E, L"STATUS_INVALID_PORT_ATTRIBUTES", L"Invalid Object Attributes specified to NtCreatePort or invalid Port Attributes specified to NtConnectPort"},
   {0xC000002F, L"STATUS_PORT_MESSAGE_TOO_LONG",   L"Length of message passed to NtRequestPort or NtRequestWaitReplyPort was longer than the maximum message allowed by the port"},
   {0xC0000030, L"STATUS_INVALID_PARAMETER_MIX",   L"An invalid combination of parameters was specified."},
   {0xC0000031, L"STATUS_INVALID_QUOTA_LOWER",     L"An attempt was made to lower a quota limit below the current usage."},
   {0xC0000032, L"STATUS_DISK_CORRUPT_ERROR",      L"The file system structure on the disk is corrupt and unusable.  Please run the Chkdsk utility on the volume"},
   {0xC0000033, L"STATUS_OBJECT_NAME_INVALID",     L"Object Name invalid."},
   {0xC0000034, L"STATUS_OBJECT_NAME_NOT_FOUND",   L"Object Name not found."},
   {0xC0000035, L"STATUS_OBJECT_NAME_COLLISION",   L"Object Name already exists."},
   {0xC0000037, L"STATUS_PORT_DISCONNECTED",       L"Attempt to send a message to a disconnected communication port."},
   {0xC0000038, L"STATUS_DEVICE_ALREADY_ATTACHED", L"An attempt was made to attach to a device that was already attached to another device."},
   {0xC0000039, L"STATUS_OBJECT_PATH_INVALID",     L"Object Path Component was not a directory object."},
   {0xC000003A, L"STATUS_OBJECT_PATH_NOT_FOUND",   L"The path does not exist."},
   {0xC000003B, L"STATUS_OBJECT_PATH_SYNTAX_BAD",  L"Object Path Component was not a directory object."},
   {0xC000003C, L"STATUS_DATA_OVERRUN",            L"A data overrun error occurred."},
   {0xC000003D, L"STATUS_DATA_LATE_ERROR",         L"A data late error occurred."},
   {0xC000003E, L"STATUS_DATA_ERROR",              L"An error in reading or writing data occurred."},
   {0xC000003F, L"STATUS_CRC_ERROR",               L"A cyclic redundancy check (CRC) checksum error occurred."},
   {0xC0000040, L"STATUS_SECTION_TOO_BIG",         L"The specified section is too big to map the file."},
   {0xC0000041, L"STATUS_PORT_CONNECTION_REFUSED", L"The NtConnectPort request is refused."},
   {0xC0000042, L"STATUS_INVALID_PORT_HANDLE",     L"The type of port handle is invalid for the operation requested."},
   {0xC0000043, L"STATUS_SHARING_VIOLATION",       L"A file cannot be opened because the share access flags are incompatible."},
   {0xC0000044, L"STATUS_QUOTA_EXCEEDED",          L"Insufficient quota exists to complete the operation"},
   {0xC0000045, L"STATUS_INVALID_PAGE_PROTECTION", L"The specified page protection was not valid."},
   {0xC0000048, L"STATUS_PORT_ALREADY_SET",        L"An attempt to set a processes DebugPort or ExceptionPort was made, but a port already exists in the process"}
};

static VOID InitLpcName(LPCSTR ipc, UNICODE_STRING *uniString);
static VOID InitLpcFuncs(void);
static bool IsVista(void);
static bool CheckNtFunction(LPCSTR szFunctionName, DWORD dwReturnCode);
static VOID Log(LPCWSTR szFmt, ...);

// CreateIpcQueue support
static int  WINAPI LpcWorkerThread(LPC_WORKER_THREAD *workerThread);
static int  WINAPI LpcDispatchThread(LPC_QUEUE *lpcQueue);
static int  WINAPI LpcPortThread(LPC_QUEUE *lpcQueue);
static bool CreateLpcQueue(LPCSTR ipcName, PFN_IPC_CALLBACK callback, DWORD maxThreadCount, DWORD maxQueueLength);

// DestroyIpcQueue
static bool DestroyLpcQueue(LPCSTR ipcName);
static bool DestroyPipedIpcQueue(LPCSTR ipcName);

// Send Message Support
static bool InitIpcAnswer(bool create, LPCSTR name, DWORD counter, DWORD processId, IPC_ANSWER *answer, DWORD session = 0);
static void CloseIpcAnswer(IPC_ANSWER *answer);
static bool WaitFor(HANDLE handle1, HANDLE handle2, DWORD timeout, BOOL handleMessages);
static int InterlockedIncrementEx(int *value);
static DWORD GetLpcCounter(void);

// Piped support
static bool CreatePipedIpcQueue(LPCSTR ipcName, PFN_IPC_CALLBACK callback, DWORD maxThreadCount, DWORD maxQueueLength);
static void HandlePipedIpcMessage(PIPED_IPC_REC *ipcRec);
static int  WINAPI PipedIpcThread2(PIPED_IPC_REC *ipcRec);
static bool ReadFromPipe(HANDLE hPipe, LPVOID targetBuffer, DWORD numToRead);
static int  WINAPI PipedIpcThread1(PIPED_IPC_REC *ipcRec);
static bool OpenPipedIpcMap(LPCSTR name, PIPED_IPC_REC *pipedIpcRec, PHANDLE pmutex = NULL, bool destroy = false);

// ------------------ Library Initialization Routines ------------------------------------

bool InitializeIpc(void)
{
  InitCalled = false;
  bool result = false;
  __try
  {
    InitializeCriticalSection(&LpcSection);
    result = true;
    InitCalled = true;
  }
  __except (ExceptionFilter(L"InitializeIpc", GetExceptionInformation()))
  {
    result = false;
  }
  return result;
}

bool CloseIpc(void)
{
  // allow close when init wasn't previously called
  if (!InitCalled)
    return true;

  bool result = false;
  DeleteCriticalSection(&LpcSection);
  if (LpcCounterBuf != NULL)
    UnmapViewOfFile(LpcCounterBuf);
  result = true;
  InitCalled = false;
  return result;
}

// ------------------ Exported functions ------------------------------------

const SID_IDENTIFIER_AUTHORITY CEveryoneSia                 = {0, 0, 0, 0, 0,  1};
const SID_IDENTIFIER_AUTHORITY CSecurityAppPackageAuthority = {0, 0, 0, 0, 0, 15};

SYSTEMS_API BOOL WINAPI AddAccessForEveryone(HANDLE processOrService, DWORD access)
{
  BOOL result = false;
  char buffer[24];
  HMODULE dll = LoadLibraryA(DecryptStr(CAdvApi32, buffer, 24));
  SECURITY_DESCRIPTOR *pSecurityDescriptor = NULL;
  EXPLICIT_ACCESS ea[2];
  ACL *oldDacl = NULL;
  ACL *newDacl = NULL;
  PSID pSid1 = NULL;
  PSID pSid2 = NULL;
  SE_OBJECT_TYPE type;

  PFN_GETSECURITYINFO pGetSecurityInfo = (PFN_GETSECURITYINFO) GetProcAddress(dll, DecryptStr(CGetSecurityInfo, buffer, 24));
  PFN_SETSECURITYINFO pSetSecurityInfo = (PFN_SETSECURITYINFO) GetProcAddress(dll, DecryptStr(CSetSecurityInfo, buffer, 24));
  PFN_SETENTRIESINACLA pSetEntriesInACL = (PFN_SETENTRIESINACLA) GetProcAddress(dll, DecryptStr(CSetEntriesInAclA, buffer, 24));

  if ((pGetSecurityInfo != NULL) && (pSetSecurityInfo != NULL) && (pSetEntriesInACL != NULL))
  {
    // NOTE:  The call below to GetSecurityInfo succeeds, but GetLastError will return 122 if you check it.  Doesn't seem to
    // make a difference though.
    if (pGetSecurityInfo(processOrService, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &oldDacl, NULL, (PSECURITY_DESCRIPTOR *) &pSecurityDescriptor) == 0)
      type = SE_KERNEL_OBJECT;
    else 
      if (pGetSecurityInfo(processOrService, SE_SERVICE, DACL_SECURITY_INFORMATION, NULL, NULL, &oldDacl, NULL, (PSECURITY_DESCRIPTOR *) &pSecurityDescriptor) == 0)
        type = SE_SERVICE;
      else
        type = SE_UNKNOWN_OBJECT_TYPE;

    if (type != SE_UNKNOWN_OBJECT_TYPE)
    {
      int count = 1;
      if (AllocateAndInitializeSid((PSID_IDENTIFIER_AUTHORITY) &CEveryoneSia, 1, 0, 0, 0, 0, 0, 0, 0, 0, &pSid1))
      {
        ZeroMemory(&ea, sizeof(ea));
        ea[0].grfAccessPermissions = access;
        ea[0].grfAccessMode = GRANT_ACCESS;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea[0].Trustee.ptstrName = (LPWSTR) pSid1;
        count = 1;
        if ((IsMetro()) && (AllocateAndInitializeSid((PSID_IDENTIFIER_AUTHORITY) &CSecurityAppPackageAuthority, 2, 2, 1, 0, 0, 0, 0, 0, 0, &pSid2)))
        {
          // this adds access for metro "AppContainer" apps
          ea[1].grfAccessPermissions = access;
          ea[1].grfAccessMode = GRANT_ACCESS;
          ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
          ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
          ea[1].Trustee.ptstrName = (LPWSTR) pSid2;
          count++;
        }
        if (pSetEntriesInACL(count, ea, oldDacl, &newDacl) == 0)
        {
          result = (pSetSecurityInfo(processOrService, type, DACL_SECURITY_INFORMATION, NULL, NULL, newDacl, NULL) == 0);
          LocalFree(newDacl);
        }
        FreeSid(pSid1);
        if (pSid2 != NULL)
          FreeSid(pSid2);
      }
    }
    LocalFree(pSecurityDescriptor);
  }
  FreeLibrary(dll);
  return result;
}

// ----------------------------------------------------------------------------------
// Create the IPC queue based on the OS: Vista->LPC, everything else->anonymous pipes
// ----------------------------------------------------------------------------------

bool UseLpcTransport(void)
{
  return (GetMadCHookOption(USE_NEW_IPC_LOGIC)) || IsVista() || Is64bitOS();
}

SYSTEMS_API BOOL WINAPI CreateIpcQueueEx(LPCSTR ipcName, PFN_IPC_CALLBACK callback, DWORD maxThreadCount, DWORD maxQueueLen)
{
  BOOL result = false;
  EnableAllPrivileges();
  if (maxThreadCount == 0)
    maxThreadCount = 16;
  if (callback != NULL)
  {
    if (UseLpcTransport())
      result = CreateLpcQueue(ipcName, callback, maxThreadCount, maxQueueLen);
    else
      result = CreatePipedIpcQueue(ipcName, callback, maxThreadCount, maxQueueLen);
  }
  return result;
}

SYSTEMS_API BOOL WINAPI CreateIpcQueue(LPCSTR ipcName, PFN_IPC_CALLBACK callback)
{
  return CreateIpcQueueEx(ipcName, callback);
}

SYSTEMS_API BOOL WINAPI DestroyIpcQueue(LPCSTR ipcName)
{
  if (UseLpcTransport())
    return DestroyLpcQueue(ipcName);
  else
    return DestroyPipedIpcQueue(ipcName);
}

SYSTEMS_API BOOL WINAPI SendIpcMessage(LPCSTR ipcName, void *messageBuf, DWORD messageLen,
                                       void *answerBuf, DWORD answerLen, 
                                       DWORD answerTimeout,
                                       BOOL handleMessages)
{
  DWORD dwError = GetLastError();
  BOOL result = false;
  HANDLE hProcess = NULL;
  PIPED_IPC_REC pipedIpcRec = {0};
  BOOL answerInitialized = false;

  if (UseLpcTransport())
  {
    // Windows major version >= 6 (Vista) uses LPC
    LPC_MESSAGE_PRIVATE lpcMessagePrv;
    SECURITY_QOS securityQos;

    ZeroMemory(&lpcMessagePrv, sizeof(lpcMessagePrv));
    lpcMessagePrv.Counter = GetLpcCounter();
    lpcMessagePrv.ProcessId = GetCurrentProcessId();

    pipedIpcRec.Answer.Length = answerLen;
    answerInitialized = InitIpcAnswer(true, ipcName, lpcMessagePrv.Counter, GetCurrentProcessId(), &pipedIpcRec.Answer);
    if (answerInitialized)
    {
      LPC_SECTION_INFO lpcSectionInfo;
      P_LPC_SECTION_INFO pLpcSectionInfo = NULL;
      LPC_SECTION_MAP_INFO lpcSectionMapInfo, *pLpcSectionMapInfo = NULL;
      DWORD bufLen = 0;
      HANDLE port = 0;

      InitLpcFuncs();
      UNICODE_STRING uniStr;
      InitLpcName(ipcName, &uniStr);

      ZeroMemory(&lpcSectionInfo, sizeof(lpcSectionInfo));

      ZeroMemory(&securityQos, sizeof(securityQos));
      lpcMessagePrv.MessageLength = messageLen;
      lpcMessagePrv.Session = GetProcessSessionId(GetCurrentProcessId());
      lpcMessagePrv.AnswerLength = answerLen;

      // see if the message is short enough to be sent in full directly with the API. Messages that are too long are copied to mapped memory
      // and the map info is sent to the server instead of the message itself
      if (messageLen > sizeof(lpcMessagePrv.Data))
      {
        // copy message contents to shared memory
        //bufLen = sizeof(lpcMessagePrv) - sizeof(lpcMessagePrv.Data);
        ZeroMemory(&lpcSectionMapInfo, sizeof(lpcSectionMapInfo));
        lpcSectionInfo.Size = sizeof(lpcSectionInfo);
        lpcSectionInfo.SectionHandle = CreateFileMapping((HANDLE) -1, NULL, PAGE_READWRITE, 0, messageLen, NULL);
        lpcSectionInfo.SectionSize = messageLen;
        lpcSectionInfo.ClientBaseAddress = MapViewOfFile(lpcSectionInfo.SectionHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (lpcSectionInfo.ClientBaseAddress != NULL)
        {
          memcpy(lpcSectionInfo.ClientBaseAddress, messageBuf, messageLen);
          UnmapViewOfFile(lpcSectionInfo.ClientBaseAddress);
          lpcSectionInfo.ClientBaseAddress = NULL;
        }
        lpcSectionMapInfo.Size = sizeof(lpcSectionMapInfo);
        lpcSectionMapInfo.SectionSize = messageLen;
        pLpcSectionInfo = &lpcSectionInfo;
        pLpcSectionMapInfo = &lpcSectionMapInfo;
      }
      else
      {
        // short message--send directly using API
        //bufLen = sizeof(lpcMessagePrv) - sizeof(lpcMessagePrv.Data) + messageLen;
        memcpy(lpcMessagePrv.Data, messageBuf, messageLen);
        pLpcSectionInfo = NULL;
        pLpcSectionMapInfo = NULL;
      }
      bufLen = sizeof(lpcMessagePrv);

      // don't use CheckNtFunction for NtConnectPort: There is a race condition on closing the handle between this thread and the Lpc port thread.
      // If the other thread closes the handle first, then NtConnectPort returns C0000034, but still works.  Instead of logging an ugly message
      // to the log, just ignore the return if it's non-zero
      if (pNtConnectPort(&port, &uniStr, &securityQos, pLpcSectionInfo, pLpcSectionMapInfo, NULL, &lpcMessagePrv, &bufLen) == 0)
        CloseHandle(port);
      Log(L"SendIpcMessage: NtConnectPort complete\n");

      LocalFree(uniStr.Buffer);

      if (lpcSectionInfo.SectionHandle != NULL)
        CloseHandle(lpcSectionInfo.SectionHandle);

      // after NtConnectPort has executed, ProcessId will have been filled in by the server
      result = (lpcMessagePrv.ProcessId != 0) && (lpcMessagePrv.ProcessId != GetCurrentProcessId());
      if (result && (answerLen != 0))
        hProcess = OpenProcess(SYNCHRONIZE, FALSE, lpcMessagePrv.ProcessId - 1);
      else
        Log(L"SendIpcMessage: *** LpcPortThread did not fill in the process ID\n");
    }
  }
  else
  {
    // all other versions of Windows use anonymous pipes
    HANDLE mutex;

    if (OpenPipedIpcMap(ipcName, &pipedIpcRec, &mutex))
    {
      pipedIpcRec.Answer.Length = answerLen;
      answerInitialized = InitIpcAnswer(true, ipcName, pipedIpcRec.Counter, 0, &pipedIpcRec.Answer);
      if (answerInitialized)
      {
        hProcess = OpenProcess(PROCESS_DUP_HANDLE | SYNCHRONIZE, FALSE, pipedIpcRec.ProcessId);

        if ( (hProcess != NULL) && 
             DuplicateHandle(hProcess, pipedIpcRec.WritePipeHandle, GetCurrentProcess(),
                             &pipedIpcRec.WritePipeHandle, 0, FALSE, DUPLICATE_SAME_ACCESS) )
        {
          DWORD dwSession = GetProcessSessionId(GetCurrentProcessId());
          DWORD dwBytesWritten;

          // write the message
          result = WriteFile(pipedIpcRec.WritePipeHandle, &messageLen,            4, &dwBytesWritten, NULL) && (dwBytesWritten == 4) &&
                   WriteFile(pipedIpcRec.WritePipeHandle, &(pipedIpcRec.Counter), 4, &dwBytesWritten, NULL) && (dwBytesWritten == 4) &&
                   WriteFile(pipedIpcRec.WritePipeHandle, &dwSession,             4, &dwBytesWritten, NULL) && (dwBytesWritten == 4) &&
                   WriteFile(pipedIpcRec.WritePipeHandle, &answerLen,             4, &dwBytesWritten, NULL) && (dwBytesWritten == 4) &&
                   WriteFile(pipedIpcRec.WritePipeHandle, messageBuf,    messageLen, &dwBytesWritten, NULL) && (dwBytesWritten == messageLen);

          CloseHandle(pipedIpcRec.WritePipeHandle);
        }
      }

      ReleaseMutex(mutex);
      CloseHandle(mutex);
    }
  }

  // now wait for the answer...
  if (hProcess != NULL)
  {
    if (result && (answerLen != 0))
    {
      // LPC: Event1 is set after NtCompleteConnectPort is done (port thread)
      if (WaitFor(pipedIpcRec.Answer.Event1, hProcess, (DWORD) (ULONG_PTR) GetMadCHookOption(SET_INTERNAL_IPC_TIMEOUT), handleMessages))
      {
        // LPC: Event2 is set after the worker thread invokes the callback function
        if (WaitFor(pipedIpcRec.Answer.Event2, hProcess, answerTimeout, handleMessages))
          memcpy(answerBuf, pipedIpcRec.Answer.Buffer, answerLen);
        else
          result = false;
      }
      else
        result = false;

    }
    CloseHandle(hProcess);
  }
  else
    Log(L"*** SendIpcMessage: ProcessID was not filled in by server\n");

  // cleanup
  if (answerInitialized)
  {
    CloseIpcAnswer(&pipedIpcRec.Answer);
  }

  SetLastError(dwError);
  return result;
}

// ------------------ Internal functions ------------------------------------

#ifdef _DEBUG
  static VOID Log(LPCWSTR szFmt, ...)
  {
    // print the message out
    va_list marker;
    va_start(marker, szFmt);
    vwprintf(szFmt , marker);
    va_end(marker);
  }
#else
  static VOID Log(LPCWSTR, ...)
  {
  }
#endif

static bool CheckLpcMessageType(USHORT dwMessageType)
{
  if (dwMessageType == LPC_CONNECTION_REQUEST)
    return true;

  if (dwMessageType == LPC_PORT_CLOSED)
    return false;  // no message since this is a sporadic normal event

  #ifdef _DEBUG
    for (int i = 0; i < (int) _countof(LpcMessageTypes); i++)
    {
      PLPC_MESSAGE_TYPE pMessageType = &LpcMessageTypes[i];
      if (pMessageType->dwMessageType == dwMessageType)
      {
        Log(L"*** LPC message type other than LPC_CONNECTION_REQUEST received: (%d) %s\n", dwMessageType, pMessageType->szDescription);
        return false;
      }
    }

    // message type other than 1-10 found.  Only 1-10 are valid, so this is corrupted data
    Log(L"*** LPC message type other than LPC_CONNECTION_REQUEST received, and type is invalid (not 1-10): %d\n", dwMessageType);
  #endif

  return false;
}

#ifdef _DEBUG
  static bool CheckNtFunction(LPCSTR szFunctionName, DWORD dwReturnCode)
#else
  static bool CheckNtFunction(LPCSTR, DWORD dwReturnCode)
#endif
{
  ASSERT(szFunctionName != NULL);

  if (dwReturnCode != 0)
  {
    #ifdef _DEBUG
      char buffer[64];
      DecryptStr(szFunctionName, buffer, 64);

      for (int i = 0; i < (int) _countof(LpcErrorDefinitions); i++)
      {
        PLPC_ERROR_INFO pError = &LpcErrorDefinitions[i];
        if (pError->dwErrorCode == dwReturnCode)
        {
          Log(L"*** %S failed with return code %X.  Error is \"%s\" (%s)\n", buffer, dwReturnCode, pError->szDescription, pError->szSymbolName);
          return false;
        }
      }
      // not found
      Log(L"*** %S failed with return code %X. No additional information available. See http://www.freepascal.org/svn/fpc/trunk/packages/extra/winunits/jwantstatus.pas for other error descriptions.\n", buffer, dwReturnCode);
    #endif

    return false;
  }
  else
    return true;
}

static VOID InitLpcName(LPCSTR ipc, UNICODE_STRING *uniString)
{
  char buffer[32];
  SString s(DecryptStr(CRpcControlIpc, buffer, 32));
  s += ipc;

  LPWSTR ws = (LPWSTR) LocalAlloc(LPTR, s.Length() * 2 + 2);
  wcscpy(ws, s.GetBuffer());
  uniString->Buffer = ws;
  uniString->Length = (WORD) s.Length() * 2;
  uniString->MaximumLength = (WORD) s.Length() * 2 + 2;
}

static VOID InitLpcFuncs(void)
{
  if (pNtCreatePort == NULL)
  {
    pNtCreatePort = (PFN_NTCREATEPORT) NtProc(CNtCreatePort);
    pNtConnectPort = (PFN_NTCONNECTPORT) NtProc(CNtConnectPort);
    pNtReplyWaitReceivePort = (PFN_NTREPLYWAITRECEIVEPORT) NtProc(CNtReplyWaitReceivePort);
    pNtAcceptConnectPort = (PFN_NTACCEPTCONNECTPORT) NtProc(CNtAcceptConnectPort);
    pNtCompleteConnectPort = (PFN_NTCOMPLETECONNECTPORT) NtProc(CNtCompleteConnectPort);
  }
}

static bool IsVista(void)
{
  if (IsVistaFlag == -1)
  {
    OSVERSIONINFO verInfo;
    ZeroMemory(&verInfo, sizeof(OSVERSIONINFO));
    verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&verInfo);
    IsVistaFlag = (verInfo.dwMajorVersion >= 6);
  }

  return IsVistaFlag == 1;
}

// ----------------------------------------
// CreateIpcQueue and supporting functions
// ----------------------------------------

int WINAPI LpcWorkerThread(LPC_WORKER_THREAD *workerThread)
{
  void *msgTemp;
  while (true)
  {
    if ( (workerThread->ShuttingDown) ||
         (WaitForSingleObject(workerThread->Event, INFINITE) != WAIT_OBJECT_0) ||
         (workerThread->ShuttingDown) ||
         (workerThread->Message == NULL) )
      break;

    LPC_MESSAGE_PRIVATE *Private = (LPC_MESSAGE_PRIVATE*) ((ULONG_PTR) workerThread->Message + workerThread->Message->PrivateOffset);
    workerThread->Message->Callback(workerThread->Message->Name, Private->Data, Private->MessageLength,
                                    workerThread->Message->Answer.Buffer, workerThread->Message->Answer.Length);

    if (workerThread->Message->Answer.Length != 0)
    {
      // notify SendIpcMessage that the answer is ready
      SetEvent(workerThread->Message->Answer.Event2);
      CloseIpcAnswer(&(workerThread->Message->Answer));
    }

    workerThread->LastActive = GetTickCount();
    msgTemp = workerThread->Message;
    workerThread->Message = NULL;

    LocalFree(msgTemp);
    ReleaseSemaphore(workerThread->ParentSemaphore, 1, NULL);
  }

  EnterCriticalSection(&LpcSection);
  if (workerThread->Freed)
    *workerThread->Freed = true;
  CloseHandle(workerThread->Event);
  CloseHandle(workerThread->Handle);
  LocalFree(workerThread);
  LeaveCriticalSection(&LpcSection);

  return 0;
}

static int WINAPI LpcDispatchThread(LPC_QUEUE *lpcQueue)
{
  LPC_MESSAGE *pMsg = NULL;
  DWORD threadId = 0;

  while (true)
  {
    WaitForSingleObject(lpcQueue->Semaphore, INFINITE);
    if (lpcQueue->Shutdown)
      break;

    EnterCriticalSection(&lpcQueue->CriticalSection);
    __try
    {
      // get the current message and move the pointer to the next one in line
      pMsg = lpcQueue->Message;
      if (pMsg != NULL)
        lpcQueue->Message = lpcQueue->Message->Next;
    }
    __finally
    {
       LeaveCriticalSection(&lpcQueue->CriticalSection);
    }

    while (pMsg != NULL)
    {
      // loop until we find someone who is willing to handle this message
      for (int i = 0; i < lpcQueue->WorkerThreads->GetCount(); i++)
      {
        if (lpcQueue->WorkerThreads->operator [](i)->Message == NULL)
        {
          lpcQueue->WorkerThreads->operator [](i)->Message = pMsg;
          SetEvent(lpcQueue->WorkerThreads->operator [](i)->Event);
          pMsg = NULL;
          break;
        }
      }
      if (pMsg != NULL)
      {
        // there is a message to handle, but all worker threads are busy
        if (lpcQueue->MaxThreadCount > lpcQueue->WorkerThreads->GetCount())
        {
          // create a new worker thread
          LPC_WORKER_THREAD *newThread = (LPC_WORKER_THREAD *) LocalAlloc(LPTR, sizeof(LPC_WORKER_THREAD));
          lpcQueue->WorkerThreads->Add(newThread);
          newThread->ParentSemaphore = lpcQueue->Semaphore;
          newThread->Event = CreateEvent(NULL, FALSE, true, NULL);
          newThread->Message = pMsg;
          newThread->Handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &LpcWorkerThread, newThread, 0, &threadId);
          if (newThread->Handle != 0)
          {
            // thread successfully created
            SetThreadPriority(newThread->Handle, THREAD_PRIORITY_NORMAL);
            pMsg = NULL;
          }
          else
          {
            // couldn't allocate new thread, so free up allocated resources.  Message
            // will have to wait for an available thread
            CloseHandle(newThread->Event);
            LocalFree(newThread);
            lpcQueue->WorkerThreads->Remove(newThread);
          }
        }

        if (pMsg != NULL)
        {
          // message hasn't been handled yet, so wait for an available worker thread
          WaitForSingleObject(lpcQueue->Semaphore, INFINITE);
          if (lpcQueue->Shutdown)
            break;
        }
      }
    }
    if (lpcQueue->Shutdown)
      break;


    // now prune any worker threads that have been sitting idle
    int workerThreadCount = lpcQueue->WorkerThreads->GetCount();
    for (int i = workerThreadCount - 1; i >= 0; i--)
    {
      if (((i > 0) || (workerThreadCount > 1)) && (lpcQueue->WorkerThreads->operator [](i)->Message == NULL) &&
         (GetTickCount() - lpcQueue->WorkerThreads->operator [](i)->LastActive > 100))
      {
        // this thread was idle for more than 100 ms, so close it down
        Log(L"Dispatch thread: Removing idle thread from worker pool\n");

        lpcQueue->WorkerThreads->operator [](i)->ShuttingDown = true;
        SetEvent(lpcQueue->WorkerThreads->operator [](i)->Event);
        workerThreadCount--;
        lpcQueue->WorkerThreads->RemoveAt(i);
      }
    }
  }
  return 0;
}

static int WINAPI LpcPortThread(LPC_QUEUE *lpcQueue)
{
  int result = 0;
  HANDLE hPort = 0;
  HANDLE hPrevPort = 0;
  P_LPC_SECTION_MAP_INFO pMapInfo = NULL;
  P_LPC_MESSAGE pMsg = NULL;

  hPrevPort = 0;
  while (true)
  {
    pMsg = (P_LPC_MESSAGE) LocalAlloc(LPTR, sizeof(LPC_MESSAGE) + sizeof(LPC_MESSAGE_PRIVATE) + 128);

    PULONG pulUnknown = NULL;
    P_LPC_MESSAGE pLpcMsgOut = NULL;

    // block waiting for the client to send a message with NtConnectPort()
    BOOL success = CheckNtFunction(CNtReplyWaitReceivePort, pNtReplyWaitReceivePort(lpcQueue->Port, pulUnknown, pLpcMsgOut, (P_LPC_MESSAGE) &(pMsg->ActualMessageLength))) &&
                   CheckLpcMessageType(pMsg->MessageType);

    // resource cleanup
    if (hPrevPort)
    {
      CloseHandle(hPrevPort);
      hPrevPort = 0;
    }

    if (success)
    {
      Log(L"LpcPortThread: Got message from client\n");

      // some newer OSs have additional fields in the LPC_MESSAGE_PRIVATE structure (ouch)
      // so we have to calculate where our private data block is located
      pMsg->PrivateOffset = (DWORD) ((ULONG_PTR) (&(pMsg->ActualMessageLength)) - (ULONG_PTR) pMsg + pMsg->TotalMessageLength - sizeof(LPC_MESSAGE_PRIVATE));
      LPC_MESSAGE_PRIVATE *Private = (LPC_MESSAGE_PRIVATE*) ((ULONG_PTR) pMsg + pMsg->PrivateOffset);

      if (lpcQueue->Shutdown)
      {
        // reject the connection request (acceptIt = 0)
        Log(L"LpcPortThread: Rejecting connection request because queue is shutting down\n");
        CheckNtFunction(CNtAcceptConnectPort, pNtAcceptConnectPort(&hPort, 0, (P_LPC_MESSAGE) &(pMsg->ActualMessageLength), 0, 0, NULL));
        LocalFree(pMsg);

        // break out of thread while loop
        break;
      }

      // due to 32/64 bit LPC incompatabilities, pMsg->ClientProcessId is not reliable, so we extract the PID from our own transport buffer
      if (Private->ProcessId)
        pMsg->ClientProcessId = Private->ProcessId;

      // set the process ID.  The client checks this value as verification that the command completed
      Private->ProcessId = GetCurrentProcessId() + 1;

      hPort = 0;

      #ifdef _WIN64
        pMapInfo = (P_LPC_SECTION_MAP_INFO) LocalAlloc(LPTR, sizeof(LPC_SECTION_MAP_INFO));
        pMapInfo->Size = sizeof(LPC_SECTION_MAP_INFO);
      #else
        pMapInfo = (P_LPC_SECTION_MAP_INFO) LocalAlloc(LPTR, sizeof(LPC_SECTION_MAP_INFO64));
        if (Is64bitOS())
          // it doesn't make much sense, but when running on a 64bit OS even a 32bit
          // process has to use this 64bit structure when calling NtAcceptConnectPort()
          pMapInfo->Size = sizeof(LPC_SECTION_MAP_INFO64);
        else
          pMapInfo->Size = sizeof(LPC_SECTION_MAP_INFO);
      #endif

      // accept the connection request (acceptIt = 1)
      Log(L"LpcPortThread: Accepting connection request\n");
      if (CheckNtFunction(CNtAcceptConnectPort, pNtAcceptConnectPort(&hPort, 0, (P_LPC_MESSAGE) &(pMsg->ActualMessageLength), 1, 0, pMapInfo)))
      {
        #ifndef _WIN64
          if (Is64bitOS())
          {
            pMapInfo->SectionSize       = ((LPC_SECTION_MAP_INFO64*) pMapInfo)->SectionSize;
            pMapInfo->ServerBaseAddress = ((LPC_SECTION_MAP_INFO64*) pMapInfo)->ServerBaseAddress;
          }
        #endif

        if (pMapInfo->SectionSize > 0)
        {
          P_LPC_MESSAGE pMsg2 = NULL;
          pMsg2 = (P_LPC_MESSAGE) LocalAlloc(LPTR, (ULONG_PTR) &Private->Data - (ULONG_PTR) pMsg + Private->MessageLength);
          memcpy(pMsg2, pMsg, (ULONG_PTR) &Private->Data - (ULONG_PTR) pMsg);
          Private = (LPC_MESSAGE_PRIVATE*) ((ULONG_PTR) pMsg2 + pMsg2->PrivateOffset);
          memcpy(&Private->Data, pMapInfo->ServerBaseAddress, Private->MessageLength);
          LocalFree(pMsg);
          pMsg = pMsg2;
        }

        // complete the connection.  Client will unblock after this function completes
        Log(L"LpcPortThread: Completing connection request\n");

        pNtCompleteConnectPort(hPort);

        // if we close the port here, the client will get into trouble
        // SendIpcMessage might then fail (especially in Windows 7)
        // so we have to delay closing the port
        // we simply close it the next time something happens
        // usually that should be a LPC_CLOSE_PORT message
        hPrevPort = hPort;

        pMsg->Answer.Length = Private->AnswerLength;
        if (InitIpcAnswer(false, lpcQueue->Name, Private->Counter, pMsg->ClientProcessId, &pMsg->Answer, Private->Session))
        {
          // notify client that we've received the message
          if (pMsg->Answer.Length != 0)
            SetEvent(pMsg->Answer.Event1);

          // copy the message to the lpc queue where the dispatch thread can get to it
          strcpy(pMsg->Name, lpcQueue->Name);

          pMsg->Callback = lpcQueue->Callback;
          EnterCriticalSection(&lpcQueue->CriticalSection);
          __try
          {
            P_LPC_MESSAGE *ppLpcMsg = &(lpcQueue->Message);
            while (*ppLpcMsg != NULL)
            {
              ppLpcMsg = &((*ppLpcMsg)->Next);
            }
            *ppLpcMsg = pMsg;
          }
          __finally
          {
            LeaveCriticalSection(&lpcQueue->CriticalSection);
          }

          ReleaseSemaphore(lpcQueue->Semaphore, 1, NULL);
        }
        else
          LocalFree(pMsg);
      }
      else
      {
        // reject the connection request (acceptIt = 0)
        Log(L"LpcPortThread: Rejecting connection request because accepting it failed\n");
        Private->ProcessId = 0;
        CheckNtFunction(CNtAcceptConnectPort, pNtAcceptConnectPort(&hPort, 0, (P_LPC_MESSAGE) &(pMsg->ActualMessageLength), 0, 0, NULL));
        LocalFree(pMsg);
      }

      LocalFree(pMapInfo);
    }
    else
      LocalFree(pMsg);

  }

  // resource cleanup
  if (hPrevPort)
    CloseHandle(hPrevPort);

  return result;
}

static bool CreateLpcQueue(LPCSTR ipcName, PFN_IPC_CALLBACK callback, DWORD maxThreadCount, DWORD maxQueueLength)
{
  bool result = false;
  HANDLE hProcess;
  if (DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(), &hProcess, 0, FALSE, DUPLICATE_SAME_ACCESS))
  {
    // NOTE:  GetLastError returns 122 after coming out of this function, although no errors were raised   
    AddAccessForEveryone(hProcess, SYNCHRONIZE);
    SetLastError(0);  // clear 122 to avoid confusion

    CloseHandle(hProcess);
  }

  if (!LpcReady)
  {
    if (!InitCalled)
      InitializeIpc();

    LpcReady = true;
  }

  result = true;
  EnterCriticalSection(&LpcSection);
  __try
  {
    for (int i = 0; i < LpcList.GetCount(); i++)
    {
      if (_stricmp(LpcList[i]->Name, ipcName) == 0)
      {
        result = false;
        break;
      }
    }
  }
  __finally
  {
    LeaveCriticalSection(&LpcSection);
  }

  if (result)
  {
    UNICODE_STRING unicodeIpcName;
    ZeroMemory(&unicodeIpcName, sizeof(UNICODE_STRING));
    InitLpcFuncs();
    InitLpcName(ipcName, &unicodeIpcName);

    SECURITY_DESCRIPTOR securityDesc;
    if (IsMetro())
      CreateMetroSd(&securityDesc);
    else
    {
      InitializeSecurityDescriptor(&securityDesc, SECURITY_DESCRIPTOR_REVISION);
      SetSecurityDescriptorDacl(&securityDesc, true, NULL, FALSE);
    }
    POBJECT_ATTRIBUTES pObjectAttr;
    pObjectAttr = (OBJECT_ATTRIBUTES *) LocalAlloc(LPTR, sizeof(OBJECT_ATTRIBUTES));
    pObjectAttr->Length = sizeof(OBJECT_ATTRIBUTES);
    pObjectAttr->ObjectName = &unicodeIpcName;
    pObjectAttr->SecurityDescriptor = &securityDesc;
    HANDLE port = (void *) -1;

    // Notes about NtCreatePort:
    // - if it fails, it returns a DWORD which indicates the error--GetLastError doesn't show the error code.  CheckNtFunction will look the error up and log 
    //   additional information about it
    // - the object name you specify must begin with \, or else it fails.  
    // - contrary to the information on /www.windowsitlibrary.com, you CANNOT pass 0 for the 2 'max' length fields.  If you do, then the ActualMessageLength
    //   field in the lpc message received by NtReplyWaitReceive will always be 0 and the data buffer will always contain NULLs.  The last param is passed
    //   in as 0
    result = CheckNtFunction(CNtCreatePort, pNtCreatePort(&port, pObjectAttr, 260, 40 + 260, 0));

    LocalFree(unicodeIpcName.Buffer);
    LocalFree(pObjectAttr);
    if (securityDesc.Dacl)
      LocalFree(securityDesc.Dacl);

    if (!result)
    {
      return false;
    }

    if (result)
    {
      Log(L"NtCreatePort succeeded\n");

      if (maxThreadCount > 64)
        maxThreadCount = 64;

      DWORD newThreadId;

      // create a new LPC queue
      P_LPC_QUEUE newLpcQueue = (P_LPC_QUEUE) LocalAlloc(LPTR, sizeof(LPC_QUEUE));
      newLpcQueue->Name = (LPSTR) LocalAlloc(LPTR, lstrlenA(ipcName) + 1);
      lstrcpyA(newLpcQueue->Name, ipcName);
      newLpcQueue->WorkerThreads = new CCollection<P_LPC_WORKER_THREAD>;
      newLpcQueue->Callback = callback;
      newLpcQueue->MaxThreadCount = maxThreadCount;
      newLpcQueue->MadQueueLength = maxQueueLength;
      newLpcQueue->Port = port;
      newLpcQueue->Semaphore = CreateSemaphore(NULL, 0, INT_MAX, NULL);
      InitializeCriticalSection(&newLpcQueue->CriticalSection);
      newLpcQueue->PortThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &LpcPortThread, newLpcQueue, 0, &newThreadId);
      newLpcQueue->DispatchThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &LpcDispatchThread, newLpcQueue, 0, &newThreadId);
      SetThreadPriority(newLpcQueue->PortThread, 7);
      SetThreadPriority(newLpcQueue->DispatchThread, THREAD_PRIORITY_ABOVE_NORMAL);

      EnterCriticalSection(&LpcSection);
      __try
      {
        // add it to the global list
        LpcList.Add(newLpcQueue);
      }
      __finally
      {
        LeaveCriticalSection(&LpcSection);
      }
    }
  }

  return result;
}

// ----------------------------------------
// DestroyIpcQueue and supporting functions
// ----------------------------------------

static bool DestroyLpcQueue(LPCSTR ipcName)
{
  HANDLE hPort;
  SECURITY_QUALITY_OF_SERVICE qualityOfService;
  DWORD dwBufferLength;
  UNICODE_STRING unicodeIpcName;
  P_LPC_QUEUE pLpcQueue = NULL;
  int i;
  bool result = false;

  if (LpcReady)
  {
    EnterCriticalSection(&LpcSection);
    __try
    {
      pLpcQueue = NULL;
      int lpcCount = LpcList.GetCount();

      // see if the name is present in the list of LPC connections
      for (i = 0; i < lpcCount; i++)
      {
        if (_stricmp(LpcList[i]->Name, ipcName) == 0)
        {
          pLpcQueue = LpcList[i];
          LpcList.RemoveAt(i);
          break;
        }
      }
    }
    __finally
    {
      LeaveCriticalSection(&LpcSection);
    }

    if (pLpcQueue != NULL)
    {
      // found a match.  Destroy it and remove from the LPC list
      pLpcQueue->Shutdown = true;
      ReleaseSemaphore(pLpcQueue->Semaphore, 1, NULL);
      InitLpcName(pLpcQueue->Name, &unicodeIpcName);
      ZeroMemory(&qualityOfService, sizeof(qualityOfService));
      hPort = 0;
      dwBufferLength = 0;
      CheckNtFunction(CNtConnectPort, pNtConnectPort(&hPort, &unicodeIpcName, &qualityOfService, NULL, NULL, NULL, NULL, &dwBufferLength));
      LocalFree(unicodeIpcName.Buffer);

      WaitForSingleObject(pLpcQueue->PortThread, 100);
      TerminateThread(pLpcQueue->PortThread, 0);
      CloseHandle(pLpcQueue->PortThread);

      WaitForSingleObject(pLpcQueue->DispatchThread, 100);
      TerminateThread(pLpcQueue->DispatchThread, 0);
      CloseHandle(pLpcQueue->DispatchThread);

      if (pLpcQueue->WorkerThreads != NULL)
      {
        for (i = 0; i < pLpcQueue->WorkerThreads->GetCount(); i++)
        {
          if ((*pLpcQueue->WorkerThreads)[i] != NULL)
          {
            LPC_WORKER_THREAD *workerThread = (*pLpcQueue->WorkerThreads)[i];
            BOOL freed = false;
            workerThread->Freed = &freed;
            workerThread->ShuttingDown = true;
            SetEvent(workerThread->Event);
            WaitForSingleObject(workerThread->Handle, 1000);
            if (!freed)
            {
              EnterCriticalSection(&LpcSection);
              if (!freed)
              {
                TerminateThread(workerThread->Handle, 0);
                CloseHandle(workerThread->Event);
                CloseHandle(workerThread->Handle);
                if (workerThread->Message != NULL)
                  LocalFree(workerThread->Message);
                LocalFree(workerThread);
              }
              LeaveCriticalSection(&LpcSection);
            }
          }
        }
        delete pLpcQueue->WorkerThreads;
        CloseHandle(pLpcQueue->Port);
        CloseHandle(pLpcQueue->Semaphore);
        DeleteCriticalSection(&pLpcQueue->CriticalSection);
        LocalFree(pLpcQueue->Name);
        LocalFree(pLpcQueue);
        result = true;
      }
    }
  }

  return result;
}

static bool DestroyPipedIpcQueue(LPCSTR ipcName)
{
  PIPED_IPC_REC pipedRec;
  bool result = OpenPipedIpcMap(ipcName, &pipedRec, NULL, true);
  if (result)
  {
    CloseHandle(pipedRec.WritePipeHandle);
    WaitForSingleObject(pipedRec.ThreadHandle, 100);
    TerminateThread(pipedRec.ThreadHandle, 0);
    CloseHandle(pipedRec.ReadPipeHandle);
    CloseHandle(pipedRec.ThreadHandle);
  }
  return result;
}

// ----------------------------------------
// SendIpcMessage and supporting functions
// ----------------------------------------

bool GetMetroPath(DWORD processId, LPSTR path)
{
  bool result = false;
  PFN_GET_APP_CONTAINER_NAMED_OBJECT_PATH gacnop = (PFN_GET_APP_CONTAINER_NAMED_OBJECT_PATH) GetProcAddress(GetModuleHandleA("kernelbase.dll"), "GetAppContainerNamedObjectPath");
  if (gacnop)
  {
    HANDLE ph = OpenProcess(PROCESS_QUERY_INFORMATION, false, processId);
    if (ph)
    {
      HANDLE th;
      if (OpenProcessToken(ph, TOKEN_QUERY, &th))
      {
        WCHAR arrCh[MAX_PATH + 1];
        DWORD len;
        if (gacnop(th, NULL, MAX_PATH, arrCh, &len))
        {
          result = true;
          WideToAnsi(arrCh, path);
        }
        CloseHandle(th);
      }
      CloseHandle(ph);
    }
  }
  return result;
}

static bool InitIpcAnswer(bool create, LPCSTR name, DWORD counter, DWORD processId, IPC_ANSWER *answer, DWORD session)
{
  bool result = false;

  ASSERT(name != NULL);
  ASSERT(name[0] != L'\0');

  if ((name == NULL) || (name[0] == L'\0'))
    return false;

  if (answer->Length > 0)
  {
    char answerBufName[16];
    DecryptStr(CAnswerBuf, answerBufName, 16);
    char nameBuf[350];

    if (processId != 0)
      sprintf_s(nameBuf, 350, "%s%s%d$%x", name, answerBufName, counter, processId);
    else
      sprintf_s(nameBuf, 350, "%s%s%d", name, answerBufName, counter);
    int nameBufLen = lstrlenA(nameBuf);

    if (create)
    {
      Log(L"InitIpcAnswer: Creating Answer file mapping.  name=%S\n", nameBuf);

      DecryptStr(CMap, &nameBuf[nameBufLen], 8);
      answer->Map = InternalCreateFileMapping(nameBuf, answer->Length, true);
      DecryptStr(CEvent, &nameBuf[nameBufLen], 8);
      lstrcatA(nameBuf, "1");
      answer->Event1 = CreateGlobalEvent(nameBuf, FALSE, FALSE);
      DecryptStr(CEvent, &nameBuf[nameBufLen], 8);
      lstrcatA(nameBuf, "2");
      answer->Event2 = CreateGlobalEvent(nameBuf, FALSE, FALSE);
    }
    else
    {
      Log(L"InitIpcAnswer: Opening existing answer file mapping.  name=%S\n", nameBuf);
      char sessionName[16];
      char sessionPath[MAX_PATH];
      DecryptStr(CSession, sessionName, 16);
      sprintf_s(sessionPath, MAX_PATH, "%s%d\\", sessionName, session);
      DecryptStr(CMap, &nameBuf[nameBufLen], 8);
      answer->Map = OpenGlobalFileMapping(nameBuf, true);
      if (answer->Map == NULL)
      {
        // if the IPC sender doesn't have the SeCreateGlobalPrivilege in w2k3
        // or in xp sp2 or higher then the objects were not created in global
        // namespace but in the namespace of the sender
        // so if opening of the file map didn't work, we try to open the
        // objects in the session of the sender
        if (processId != 0)
          sprintf_s(nameBuf, 350, "%s%s%s%d$%x", sessionPath, name, answerBufName, counter, processId);
        else
          sprintf_s(nameBuf, 350, "%s%s%s%d", sessionPath, name, answerBufName, counter);
        nameBufLen = lstrlenA(nameBuf);
        DecryptStr(CMap, &nameBuf[nameBufLen], 8);
        answer->Map = OpenGlobalFileMapping(nameBuf, true);
        if ((answer->Map == NULL) && (IsMetro()))
        {
          char metroPath[MAX_PATH];
          if (GetMetroPath(processId, metroPath))
          {
            sprintf_s(sessionPath, MAX_PATH, "%s%d\\%s\\", sessionName, session, metroPath);
            sprintf_s(nameBuf, 350, "%s%s%s%d$%x", sessionPath, name, answerBufName, counter, processId);
            nameBufLen = lstrlenA(nameBuf);
            DecryptStr(CMap, &nameBuf[nameBufLen], 8);
            answer->Map = OpenGlobalFileMapping(nameBuf, true);
          }
        }
        // restore global names
        if (processId != 0)
          sprintf_s(nameBuf, 350, "%s%s%d$%x", name, answerBufName, counter, processId);
        else
          sprintf_s(nameBuf, 350, "%s%s%d", name, answerBufName, counter);
        nameBufLen = lstrlenA(nameBuf);
      }
      DecryptStr(CEvent, &nameBuf[nameBufLen], 8);
      lstrcatA(nameBuf, "1");
      answer->Event1 = OpenGlobalEvent(nameBuf);
      DecryptStr(CEvent, &nameBuf[nameBufLen], 8);
      lstrcatA(nameBuf, "2");
      answer->Event2 = OpenGlobalEvent(nameBuf);
      if (answer->Event1 == NULL)
      {
        if (processId != 0)
          sprintf_s(nameBuf, 350, "%s%s%s%d$%x", sessionPath, name, answerBufName, counter, processId);
        else
          sprintf_s(nameBuf, 350, "%s%s%s%d", sessionPath, name, answerBufName, counter);
        nameBufLen = lstrlenA(nameBuf);
        DecryptStr(CEvent, &nameBuf[nameBufLen], 8);
        lstrcatA(nameBuf, "1");
        answer->Event1 = OpenGlobalEvent(nameBuf);
        DecryptStr(CEvent, &nameBuf[nameBufLen], 8);
        lstrcatA(nameBuf, "2");
        answer->Event2 = OpenGlobalEvent(nameBuf);
      }
    }

    if (answer->Map != NULL)
      answer->Buffer = MapViewOfFile(answer->Map, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    result = (answer->Event1 != NULL) && (answer->Event2 != NULL) && (answer->Buffer != NULL);
    if (result)
    {
      if (create)
        ZeroMemory((void *) answer->Buffer, answer->Length);
    }
    else
      CloseIpcAnswer(answer);
  }
  else
  {
    answer->Map = NULL;
    answer->Buffer = NULL;
    answer->Event1 = NULL;
    answer->Event2 = NULL;
    result = true;
  }

  return result;
}

static void CloseIpcAnswer(IPC_ANSWER *answer)
{
  if (answer->Buffer != NULL)
  {
    if (!UnmapViewOfFile(answer->Buffer))
    {
      Log(L"CloseIpcAnswer: UnmapViewOfFile failed: %d\n", GetLastError());
    }
  }

  if (answer->Map != NULL)
  {
    //printf("maphandle=%d\n", answer->Map);
    if (!CloseHandle(answer->Map))
    {
      Log(L"CloseIpcAnswer: CloseHandle(Map) failed: %d\n", GetLastError());
    }
    answer->Map = NULL;
  }

  if (answer->Event1 != NULL)
  {
    if (!CloseHandle(answer->Event1))
    {
      Log(L"CloseIpcAnswer: CloseHandle(Event1) failed: %d\n", GetLastError());
    }
    answer->Event1 = NULL;
  }

  if (answer->Event2 != NULL)
  {
    if (!CloseHandle(answer->Event2))
    {
      Log(L"CloseIpcAnswer: CloseHandle(Event2) failed: %d\n", GetLastError());
    }
    answer->Event2 = NULL;
  }
}

static bool WaitFor(HANDLE handle1, HANDLE handle2, DWORD timeout, BOOL handleMessages)
{
  HANDLE handleArray[2];
  bool result = false;
  MSG msg;

  handleArray[0] = handle1;
  handleArray[1] = handle2;

  if (handleMessages)
  {
    bool breakLoop = false;
    while (!breakLoop)
    {
      switch (MsgWaitForMultipleObjects(2, handleArray, FALSE, timeout, QS_ALLINPUT))
      {
        case WAIT_OBJECT_0:
        {
          result = true;
          breakLoop = true;
          break;
        }

        case WAIT_OBJECT_0 + 2:
        {
          while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
          {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
          break;
        }

        case WAIT_TIMEOUT:
        {
          breakLoop = true;
          break;
        }

        default:
        {
          breakLoop = true;
          break;
        }
      }
    }
    return result;
  }
  else
  {
    return WaitForMultipleObjects(2, handleArray, FALSE, timeout) == WAIT_OBJECT_0;
  }
}

#ifdef _WIN64
  extern "C" int _InterlockedIncrementEx(int *value);

  static int InterlockedIncrementEx(int *value)
  {
    return _InterlockedIncrementEx(value);
  }
#else
  static int InterlockedIncrementEx(int *value)
  {
    __asm
    {
      mov edx, value
      mov eax, [edx]
     Again:
      mov ecx, eax
      inc ecx
      lock cmpxchg [edx], ecx
      jnz Again
     }
  }
#endif

static DWORD GetLpcCounter(void)
{
  if (LpcCounterBuf == NULL)
  {
    SString mutexName, fileMapName;
    char buffer1[16], buffer2[16], buffer3[8];
    mutexName.Format(L"%S%S$%x%S", DecryptStr(CIpc, buffer1, 16), DecryptStr(CCounter, buffer2, 16), GetCurrentProcessId(), DecryptStr(CMutex, buffer3, 8));
    HANDLE mutex = CreateMutexW(NULL, false, mutexName.GetBuffer());
    if (mutex)
    {
      WaitForSingleObject(mutex, INFINITE);
      fileMapName.Format(L"%S%S$%x", DecryptStr(CIpc, buffer1, 16), DecryptStr(CCounter, buffer2, 16), GetCurrentProcessId());
      HANDLE map = CreateFileMappingW((HANDLE) -1, NULL, PAGE_READWRITE, 0, 4, fileMapName.GetBuffer());
      if (map)
      {
        BOOL newMap = (GetLastError() != ERROR_ALREADY_EXISTS);
        LpcCounterBuf = (int *) MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if ((newMap) && (LpcCounterBuf))
          *LpcCounterBuf = 0;
        CloseHandle(map);
      }
      ReleaseMutex(mutex);
      CloseHandle(mutex);
    }
  }

  if (LpcCounterBuf != NULL)
    return (DWORD) InterlockedIncrementEx(LpcCounterBuf);
  else
    return 0;
}


// ------------------------------------------------------------------------
// Functions to do legacy IPC using anonymous pipes
// ------------------------------------------------------------------------

static void HandlePipedIpcMessage(PIPED_IPC_REC *pIpcRec)
{
  Log(L"HandlePipedIpcMessage: Invoking client registered callback...\n");

  pIpcRec->Callback(pIpcRec->Name, pIpcRec->Message.Buffer, pIpcRec->Message.Length, pIpcRec->Answer.Buffer, pIpcRec->Answer.Length);
  if (pIpcRec->Answer.Length != 0)
  {
    Log(L"PipedIpcThread1: After callback issued, answer length is %d.  Setting Event2 and closing answer\n", pIpcRec->Answer.Length);

    SetEvent(pIpcRec->Answer.Event2);
    CloseIpcAnswer(&pIpcRec->Answer);
  }
  LocalFree(pIpcRec->Message.Buffer);
}

static int WINAPI PipedIpcThread2(PIPED_IPC_REC *pIpcRec)
{
  HandlePipedIpcMessage(pIpcRec);
  LocalFree(pIpcRec);
  return 0;
}

static bool ReadFromPipe(HANDLE hPipe, LPVOID targetBuffer, DWORD numToRead)
{
  bool result = false;
  DWORD numRead;

  if (ReadFile(hPipe, targetBuffer, numToRead, &numRead, NULL) != 0)
  {
    if (numRead == numToRead)
      result = true;
    else
    {
      Log(L"ReadFromPipe: Error reading from pipe: Number read was %d, requested was %d\n", numRead, numToRead);
    }
  }
  else
  {
    Log(L"ReadFromPipe: Error reading from pipe: ReadFile returned False\n");
  }

  return result;
}

static int WINAPI PipedIpcThread1(PIPED_IPC_REC *ipcRec)
{
  DWORD session;

  while (true)
  {
    if (!ReadFromPipe(ipcRec->ReadPipeHandle, &ipcRec->Message.Length, 4))
    {
      Log(L"PipedIpcThread1: Failed reading value of message length\n");
      break;
    }

    Log(L"PipedIpcThread1: Starting a new message...\n");

    ipcRec->Message.Buffer = (char *) LocalAlloc(LPTR, ipcRec->Message.Length);
    if (ipcRec->Message.Buffer != NULL)
    {
      if (!ReadFromPipe(ipcRec->ReadPipeHandle, &ipcRec->Counter, 4))
      {
        Log(L"PipedIpcThread1: Failed reading value of counter\n");
        break;
      }

      if (!ReadFromPipe(ipcRec->ReadPipeHandle, &session, 4))
      {
        Log(L"PipedIpcThread1: Failed reading value of session\n");
        break;
      }

      if (!ReadFromPipe(ipcRec->ReadPipeHandle, &ipcRec->Answer.Length, 4))
      {
        Log(L"PipedIpcThread1: Failed reading value of answer length\n");
        break;
      }

      if (!ReadFromPipe(ipcRec->ReadPipeHandle, ipcRec->Message.Buffer, ipcRec->Message.Length))
      {
        Log(L"PipedIpcThread1: Failed reading message buffer\n");
        break;
      }

      if (InitIpcAnswer(false, ipcRec->Name, ipcRec->Counter, 0, &ipcRec->Answer, session))
      {
        Log(L"PipedIpcThread1: InitIpcAnswer returned True\n");
        if (ipcRec->Answer.Length != 0)
          SetEvent(ipcRec->Answer.Event1);

        if (ipcRec->MaxThreads > 1)
        {
          PIPED_IPC_REC *pIpcRec = (PIPED_IPC_REC *) LocalAlloc(LPTR, sizeof(*ipcRec));
          memcpy(pIpcRec, ipcRec, sizeof(*ipcRec));

          DWORD newThreadId;
          Log(L"PipedIpcThread1: Creating PipedIpcThread2\n");
          HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &PipedIpcThread2, pIpcRec, 0, &newThreadId);
          if (hThread != NULL)
          {
            SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
            CloseHandle(hThread);
          }
          else
          {
            LocalFree(pIpcRec);
            LocalFree(ipcRec->Message.Buffer);
            CloseIpcAnswer(&ipcRec->Answer);
          }
        }
        else
        {
          HandlePipedIpcMessage(ipcRec);
        }
      }
      else
      {
        Log(L"** PipedIpcThread1: InitIpcAnswer returned False\n");
        LocalFree(ipcRec->Message.Buffer);
      }
    }
  }
  LocalFree(ipcRec);

  return 0;
}

static bool CreatePipedIpcQueue(LPCSTR ipcName, PFN_IPC_CALLBACK callback, DWORD maxThreadCount, DWORD maxQueueLength)
{
  bool result = false;
  HANDLE hTarget;

  if (DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(), &hTarget, 0, FALSE, DUPLICATE_SAME_ACCESS))
  {
    AddAccessForEveryone(hTarget, PROCESS_DUP_HANDLE | SYNCHRONIZE);
    CloseHandle(hTarget);
  }

  LPSTR mutexName = (LPSTR) LocalAlloc(LPTR, lstrlenA(ipcName) + 32);
  LPSTR   mapName = (LPSTR) LocalAlloc(LPTR, lstrlenA(ipcName) + 32);
  __try
  {
    lstrcpyA(mutexName, ipcName);
    char buffer[16];
    lstrcatA(mutexName, DecryptStr(CIpc, buffer, 16));
    lstrcpyA(mapName, mutexName);
    lstrcatA(mutexName, DecryptStr(CMutex, buffer, 16));
    lstrcatA(mapName, DecryptStr(CMap, buffer, 16));

    HANDLE hMutex = CreateGlobalMutex(mutexName);
    if (hMutex != NULL)
    {
      WaitForSingleObject(hMutex, INFINITE);
      HANDLE hMap = CreateGlobalFileMapping(mapName, sizeof(PIPED_IPC_REC));
      __try
      {
        if (hMap != NULL)
        {
          if (GetLastError() != ERROR_ALREADY_EXISTS)
          {
            PIPED_IPC_REC *pPipedIpcRec = (PIPED_IPC_REC *) MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if (pPipedIpcRec != NULL)
            {
              result = true;
              ZeroMemory(pPipedIpcRec, sizeof(PIPED_IPC_REC));
              CreatePipe(&pPipedIpcRec->ReadPipeHandle, &pPipedIpcRec->WritePipeHandle, GetUnlimitedSa(), 0);
              pPipedIpcRec->Map = hMap;
              pPipedIpcRec->ProcessId = GetCurrentProcessId();
              pPipedIpcRec->Callback = callback;
              pPipedIpcRec->MaxThreads = maxThreadCount;
              pPipedIpcRec->MaxQueue = maxQueueLength;
              strcpy(pPipedIpcRec->Name, ipcName);

              PIPED_IPC_REC *pPipedIpcRec2 = (PIPED_IPC_REC *) LocalAlloc(LPTR, sizeof(PIPED_IPC_REC));
              *pPipedIpcRec2 = *pPipedIpcRec;

              DWORD newThreadId;
              pPipedIpcRec->ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &PipedIpcThread1, pPipedIpcRec2, 0, &newThreadId);
              SetThreadPriority(pPipedIpcRec->ThreadHandle, 7);
              pPipedIpcRec->Counter = 0;
              UnmapViewOfFile(pPipedIpcRec);
            }
            else
              CloseHandle(hMap);
          }
          else
            CloseHandle(hMap);
        }
      }
      __finally
      {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
      }
    }
  }
  __finally
  {
    LocalFree(mutexName);
    LocalFree(mapName);
  }

  return result;
}

static bool OpenPipedIpcMap(LPCSTR ipcName, PIPED_IPC_REC *pPipedIpcRec, PHANDLE pmutex, bool destroy)
// NOTE: This function makes a copy of the mapped ipc rec, so pPipedIpcRec must point to an existing struct
{
  ASSERT(pPipedIpcRec != NULL);
  ASSERT(ipcName != NULL);

  bool result = false;

  LPSTR mutexName = (LPSTR) LocalAlloc(LPTR, lstrlenA(ipcName) + 32);
  LPSTR   mapName = (LPSTR) LocalAlloc(LPTR, lstrlenA(ipcName) + 32);
  __try
  {
    lstrcpyA(mutexName, ipcName);
    char buffer[16];
    lstrcatA(mutexName, DecryptStr(CIpc, buffer, 16));
    lstrcpyA(mapName, mutexName);
    lstrcatA(mutexName, DecryptStr(CMutex, buffer, 16));
    lstrcatA(mapName, DecryptStr(CMap, buffer, 16));

    HANDLE mutex = CreateGlobalMutex(mutexName);
    if (mutex != NULL)
    {
      WaitForSingleObject(mutex, INFINITE);
      __try
      {
        HANDLE map = OpenGlobalFileMapping(mapName, true);
        if (map != NULL)
        {
          PIPED_IPC_REC *buf = (PIPED_IPC_REC*) MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
          if (buf != NULL)
          {
            result = true;
            (buf->Counter)++;
            memcpy(pPipedIpcRec, buf, sizeof(PIPED_IPC_REC));
            if (destroy)
            {
              if (buf->ProcessId == GetCurrentProcessId())
                CloseHandle(buf->Map);
              else
                result = false;

            }
            UnmapViewOfFile(buf);
          }
          CloseHandle(map);
        }
      }
      __finally
      {
        if ((!result) || (pmutex == NULL))
        {
          ReleaseMutex(mutex);
          CloseHandle(mutex);
        }
        else
        {
          *pmutex = mutex;
        }
      }
    }
  }
  __finally
  {
    LocalFree(mutexName);
    LocalFree(mapName);
  }

  return result;
}
