// ***************************************************************
//  IPC.h                     version: 1.0.1  ·  date: 2013-02-13
//  -------------------------------------------------------------
//  inter process communication functionality
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2013 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2013-02-13 1.0.1 fixed incompatability with MSVC++ 2012 on Windows 8
// 2010-01-10 1.0.0 initial version

#ifndef _IPC_H
#define _IPC_H

#include "Accctrl.h"

// required for LPC API
const int LPC_REQUEST = 0x01;
const int LPC_REPLY = 0x02;
const int LPC_DATAGRAM = 0x03;
const int LPC_LOST_REPLY = 0x04;
const int LPC_PORT_CLOSED = 0x05;
const int LPC_CLIENT_DIED = 0x06;
const int LPC_EXCEPTION = 0x07;
const int LPC_DEBUG_EVENT = 0x08;
const int LPC_ERROR_EVENT = 0x09;
const int LPC_CONNECTION_REQUEST = 0x0A;

typedef DWORD LPC_MESSAGE_DATA[60];

// ----------------------------------------------------------
// Function ptrs for AddAccessForEveryone
// ----------------------------------------------------------

typedef DWORD (WINAPI * PFN_GETSECURITYINFO)
(
  HANDLE hHandle,
  SE_OBJECT_TYPE dwObjectType,
  SECURITY_INFORMATION dwSecurityInfo,
  PSID *ppSidOwner,
  PSID *ppSidGroup,
  PACL *ppDacl,
  PACL *ppSacl,
  PSECURITY_DESCRIPTOR *ppSecurityDescriptor
);

typedef DWORD (WINAPI * PFN_SETSECURITYINFO)
(
  HANDLE hHandle,
  SE_OBJECT_TYPE dwObjectType,
  SECURITY_INFORMATION dwSecurityInfo,
  PSID pSidOwner,
  PSID pSidGroup,
  PACL pDacl,
  PACL pSacl
);

typedef DWORD (WINAPI * PFN_SETENTRIESINACLA)
(
  ULONG cCountOfExplicitEntries,
  PEXPLICIT_ACCESS pListOfExplicitEntries,
  PACL pOldAcl,
  PACL *ppNewAcl
);

// --------------------------------------------------
// NT Local Procedure Call and IPC structure typedefs
// --------------------------------------------------

typedef struct tagLpcSectionInfo
{
  DWORD Size;
  #ifdef _WIN64
    DWORD Dummy1;
  #endif
  HANDLE SectionHandle;
  DWORD Param1;
  #ifdef _WIN64
    DWORD Dummy2;
  #endif
  DWORD SectionSize;
  #ifdef _WIN64
    DWORD Dummy3;
  #endif
  void *ClientBaseAddress;
  void *ServerBaseAddress;
} LPC_SECTION_INFO, *P_LPC_SECTION_INFO;

typedef struct tagLpcSectionMapInfo
{
  DWORD Size;
  #ifdef _WIN64
    DWORD Dummy1;
  #endif
  DWORD SectionSize;
  #ifdef _WIN64
    DWORD Dummy2;
  #endif
  void *ServerBaseAddress;
} LPC_SECTION_MAP_INFO, *P_LPC_SECTION_MAP_INFO;

#ifndef _WIN64
  typedef struct tagLpcSectionMapInfo64
  {
    DWORD Size;
    DWORD Dummy1;
    DWORD SectionSize;
    DWORD Dummy2;
    void *ServerBaseAddress;
    DWORD Dummy3;
  } LPC_SECTION_MAP_INFO64, *P_LPC_SECTION_MAP_INFO64;
#endif

typedef struct tagLpcMessagePrivate
{
  DWORD ProcessId;
  DWORD MessageLength;
  DWORD Counter;
  DWORD Session;
  DWORD AnswerLength;
  LPC_MESSAGE_DATA Data;
} LPC_MESSAGE_PRIVATE;

typedef struct tagLpcMessage
{
  struct tagLpcMessage *Next;
  char Name[MAX_PATH];
  PFN_IPC_CALLBACK Callback;
  IPC_ANSWER Answer;
  DWORD PrivateOffset;
  WORD ActualMessageLength;
  WORD TotalMessageLength;
  USHORT MessageType;
  USHORT DataInfoOffset;
  DWORD ClientProcessId;
  DWORD ClientThreadId;
  DWORD MessageId;
  DWORD SharedSectionSize;
} LPC_MESSAGE, *P_LPC_MESSAGE;

typedef struct tagSecurityQualityOfService
{
  DWORD Size;
  DWORD Dummy[3];
  #ifdef _WIN64
    DWORD Dummy2;
  #endif
} SECURITY_QOS;

typedef struct tagLpcWorkerThread
{
  HANDLE Handle;
  HANDLE ParentSemaphore;
  HANDLE Event;
  P_LPC_MESSAGE Message;
  DWORD LastActive;
  BOOL ShuttingDown;
  BOOL *Freed;
} LPC_WORKER_THREAD, *P_LPC_WORKER_THREAD;

typedef struct tagLpcQueue
{
  LPSTR Name;
  PFN_IPC_CALLBACK Callback;
  int MaxThreadCount;
  int MadQueueLength;
  HANDLE Port;
  DWORD Counter;
  LPC_MESSAGE *Message;
  CRITICAL_SECTION CriticalSection;
  HANDLE Semaphore;
  BOOL Shutdown;
  HANDLE PortThread;
  HANDLE DispatchThread;
  CCollection<P_LPC_WORKER_THREAD> *WorkerThreads;
} LPC_QUEUE, *P_LPC_QUEUE;

// ------------------------------------------------
// Kernel functions
// ------------------------------------------------

typedef BOOL (WINAPI * PFN_PROCESSIDTOSESSIONID)(DWORD dwProcessId, DWORD *pdwSessionId);

typedef DWORD (WINAPI * PFN_GETINPUTSESSIONID)(void);

// ------------------------------------------------
// Struct to hold error information for NT LPC
// ------------------------------------------------

typedef struct tagLpcErrorInfo
{
  DWORD dwErrorCode;
  wchar_t szSymbolName[60];
  wchar_t szDescription[512];
} LPC_ERROR_INFO, *PLPC_ERROR_INFO;

// ------------------------------------------------
// Struct to hold message types for NT LPC
// ------------------------------------------------

typedef struct tagLpcMessageType
{
  USHORT dwMessageType;
  wchar_t szDescription[128];
} LPC_MESSAGE_TYPE, *PLPC_MESSAGE_TYPE;

// ------------------------------------------------
// NT Local Procedure Call API function definitions
// ------------------------------------------------

typedef NTSTATUS (WINAPI * PFN_NTCREATEPORT)
(
  PHANDLE PortHandle,                   // returns handle to port
  POBJECT_ATTRIBUTES ObjectAttributes,  // contains port name and security info
  ULONG_PTR MaxConnectInfoLength,       // has to be less than 260.  0 is fine
  ULONG_PTR MaxDataLength,              // has to be less than 328.  0 is fine
  PVOID Unknown                         // always specify 0
);

typedef NTSTATUS (WINAPI * PFN_NTCONNECTPORT)
(
  PHANDLE PortHandle,
  PUNICODE_STRING PortName,
  PVOID Unknown,                    // Can not be NULL
  P_LPC_SECTION_INFO Unknown1,      // Used in Big LPC
  P_LPC_SECTION_MAP_INFO Unknown2,  // Used in Big LPC
  PVOID Unknown3,                   // Can be NULL
  PVOID ConnectInfo,
  PULONG pConnectInfoLength
);

typedef NTSTATUS (WINAPI * PFN_NTREPLYWAITRECEIVEPORT)
(
  HANDLE PortHandle,
  PULONG Unknown,
  P_LPC_MESSAGE pLpcMessageOut,
  P_LPC_MESSAGE pLpcMessageIn
);

typedef NTSTATUS (WINAPI * PFN_NTACCEPTCONNECTPORT)
(
  PHANDLE PortHandle,
  ULONG_PTR Unknown,              // Pass 0
  P_LPC_MESSAGE pLpcMessage,
  ULONG_PTR Unknown1,             // 1
  ULONG_PTR Unknown3,             // 0
  P_LPC_SECTION_MAP_INFO pSectionMapInfo
);

typedef NTSTATUS (WINAPI * PFN_NTCOMPLETECONNECTPORT)
(
  HANDLE PortHandle
);

#undef EXTERN
#ifdef _IPC_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

#endif