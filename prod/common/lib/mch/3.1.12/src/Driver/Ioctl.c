// ***************************************************************
//  Ioctl.c                   version: 1.0.4  ·  date: 2016-04-29
//  -------------------------------------------------------------
//  handling of encrypted IOCTL requests from user land
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2016 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2016-04-29 1.0.4 fixed leaked thread handle
// 2014-09-08 1.0.3 fixed denial of service vulnerability (found by Parvez Anwar)
// 2013-12-03 1.0.2 fixed a rare crash
// 2010-08-02 1.0.1 added some new user mode IOCTLs
// 2010-01-10 1.0.0 initial release

#include <ntddk.h>
#ifndef _WIN64
  #undef ExAllocatePool
  #undef ExFreePool      // NT4 doesn't support ExFreePoolWithTag
#endif

#include "RipeMD.h"
#include "ToolFuncs.h"
#include "DriverEvents.h"

// ********************************************************************

// IOCTLs for communication with user land
#define IOCTL_INJECT_DLL    CTL_CODE(0x22, 0x0800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_UNINJECT_DLL  CTL_CODE(0x22, 0x0801, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ALLOW_STOP    CTL_CODE(0x22, 0x0802, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_INJECT_METHOD CTL_CODE(0x22, 0x0803, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ENUM_INJECT   CTL_CODE(0x22, 0x0804, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// ********************************************************************

#pragma pack(push, 1)

// header used for all commands which are sent from user land
typedef struct _DrvCmdHeader {
    ULONG   Size;            // size of the total command data block
    UCHAR   Hash [20];       // hash of the data following the header
} DrvCmdHeader, *PDrvCmdHeader;

// for IOCTL_ALLOW_STOP
typedef struct _DrvCmdAllowStop {
    ULONG   Size;
    UCHAR   Hash [20];
    UCHAR   Allow;           // shall the driver allow stopping or not?
} DrvCmdAllowStop, *PDrvCmdAllowStop;

// for IOCTL_INJECT_METHOD
typedef struct _DrvCmdInjectMethod {
    ULONG   Size;
    UCHAR   Hash [20];
    UCHAR   Method;           // which injection method shall the driver use?
} DrvCmdInjectMethod, *PDrvCmdInjectMethod;

// for IOCTL_ENUM_INJECT
typedef struct _DrvCmdEnumInject {
    ULONG   Size;
    UCHAR   Hash [20];
    ULONG   Index;            // return the injection request [Index]
} DrvCmdEnumInject, *PDrvCmdEnumInject;

#pragma pack(pop)

// ********************************************************************

BOOLEAN DecryptIoctl(PDrvCmdHeader Buf, PDrvCmdHeader Buf2, HANDLE ThreadId)
{
  BOOLEAN result = FALSE;
  UCHAR hash [20];
  UCHAR *pb, *sp;
  int i1, hashInd, bufSize;

  // first let's copy the sent buffer to a newly allocated buffer
  memcpy(Buf2, Buf, Buf->Size);

  // the main info block is xor'ed with the hash
  sp = (UCHAR*) ((ULONG_PTR)Buf2 + sizeof(DrvCmdHeader));
  bufSize = (Buf2->Size - sizeof(DrvCmdHeader));

  pb = sp;
  hashInd = 0;
  for (i1 = 0; i1 < bufSize - (bufSize % 4); i1+=4) // unroll loop
  {
    UINT32 b = *(UINT32*)(&Buf2->Hash[hashInd]); // read 4 hash
    UINT32* data = (UINT32*)pb;
    *data ^= ((b >> 4) & 0x0f0f0f0f) ^ ((b << 4) & 0xf0f0f0f0);
    pb+=4;
    hashInd+=4;
    if (hashInd == 20) hashInd = 0; // hashInd = i1 % 20 (% very slow)
  }
  for (; i1 < bufSize; i1++)
  {
    UCHAR b1 = Buf2->Hash[hashInd]; 
    *pb ^= ((b1 >> 4) & 0x0f) ^ ((b1 << 4) & 0xf0);
    pb++;
    hashInd++;
  }

  // now let's check whether the hash matches
  Hash(sp, bufSize, (PVOID) &hash);
  if (RtlCompareMemory(&Buf2->Hash, &hash, sizeof(hash)) == sizeof(hash))
  {
    // the hash matches

    UCHAR ctid = (UCHAR)((ULONG_PTR) ThreadId & 0xff);
    UINT32 tid = ctid | ctid << 8 | ctid << 16 | ctid << 24;

    // the main info block is xor'ed with the caller's thread id
    pb = sp;
    for (i1 = 0; i1 < bufSize - (bufSize % 4);) // unroll loop
    {
      UCHAR b1 = (UCHAR)(i1++ & 0xff); 
      UCHAR b2 = (UCHAR)(i1++ & 0xff); 
      UCHAR b3 = (UCHAR)(i1++ & 0xff); 
      UCHAR b4 = (UCHAR)(i1++ & 0xff); 
      UINT32 *data = (UINT32*)pb;
      *data = *data ^ tid ^ (b1 | (b2 << 8) | (b3 << 16) | (b4 << 24));
      pb+=4;
    }
    for (; i1 < bufSize; i1++)
    {
      *pb = *pb ^ ctid ^ (UCHAR)(i1 & 0xff);
      pb++;
    }
    result = TRUE;
  }

  return result;
}

// ********************************************************************

BOOLEAN HandleEncryptedIoctl(ULONG Ioctl, PVOID Buf, ULONG InSize, ULONG OutSize, PETHREAD Caller, BOOLEAN *DriverUnloadEnabled)
{
  BOOLEAN result = FALSE;

  __try
  {
    if (Caller)
    {
      // there is information available about which thread called us
      HANDLE th = OpenEThread(Caller);
      if (th)
      {
        // we were able to open a handle to the calling thread
        HANDLE threadId, processId;
        if (ThreadHandleToThreadProcessId(th, &threadId, &processId))
        {
          // we were able to find out the thread id and process id of the caller
          if ((((PDrvCmdHeader) Buf)->Size <= InSize) && (((PDrvCmdHeader) Buf)->Size < 1024 * 1024) && (((PDrvCmdHeader) Buf)->Size > sizeof(DrvCmdHeader)))
          {
            // input buffer size seems to be reasonable
            PDrvCmdHeader buf2 = ExAllocatePool(PagedPool, ((PDrvCmdHeader) Buf)->Size);
            if (buf2)
            {
              if (DecryptIoctl((PDrvCmdHeader) Buf, buf2, threadId))
              {
                // finally let's execute the command
                switch (Ioctl)
                {
                  case IOCTL_ALLOW_STOP:
                    result = (InSize == sizeof(DrvCmdAllowStop)) &&
                             DriverEvent_AllowUnloadRequest(((PDrvCmdAllowStop) buf2)->Allow, DriverUnloadEnabled);
                    ExFreePool(buf2);
                    break;

                  case IOCTL_INJECT_METHOD:
                    result = (InSize == sizeof(DrvCmdInjectMethod)) &&
                             DriverEvent_InjectMethodChange(((PDrvCmdInjectMethod) buf2)->Method);
                    ExFreePool(buf2);
                    break;

                  case IOCTL_INJECT_DLL:
                    {
                      // buf2 is released in DriverEvent_InjectionRequest
                      PVOID x86AllocAddr = (InSize >= sizeof(DllItem)) ? ((PVOID) ((PDllItem) buf2)->X86AllocAddr) : NULL;
                      result = (InSize >= sizeof(DllItem)) &&
                               DriverEvent_InjectionRequest((PDllItem) buf2, processId, DriverUnloadEnabled);
                      if ((result) && (x86AllocAddr))
                        SetX86AllocAddr(x86AllocAddr);
                    }
                    break;

                  case IOCTL_UNINJECT_DLL:
                    // buf2 is released in DriverEvent_UninjectionRequest
                    result = (InSize >= sizeof(DllItem)) &&
                             DriverEvent_UninjectionRequest((PDllItem) buf2, processId, DriverUnloadEnabled);
                    break;

                  case IOCTL_ENUM_INJECT:
                    result = (InSize == sizeof(DrvCmdEnumInject)) &&
                             (OutSize >= sizeof(DllItem)) &&
                             DriverEvent_EnumInjectRequest(((PDrvCmdEnumInject) buf2)->Index, (PDllItem) Buf, OutSize);
                    ExFreePool(buf2);
                    break;

                  default:
                    ExFreePool(buf2);
                    break;
                }
              }
              else
                ExFreePool(buf2);
            }
          }
        }
        ZwClose(th);
      }
    }
  } __except (1) {

    // ooops!
    LogUlonglong(L"\\??\\C:\\Desktop\\Ioctl crash", 0);
    result = FALSE;
  }

  return result;
}

// ********************************************************************
