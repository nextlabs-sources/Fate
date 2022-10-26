// ***************************************************************
//  ProcessTypes.h            version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  internal (partially undocumented) resources
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _PROCESSTYPES_H
#define _PROCESSTYPES_H

#define NT_HEADERS_OFFSET 0x003C // offset of new EXE header
// CENEWHDR = 0x003C
// CEMAGIC  = IMAGE_DOS_SIGNATURE
// CPEMAGIC = IMAGE_NT_SIGNATURE

// =================================================================
// THREAD STATES
// =================================================================

#define THREAD_STATE_INITIALIZED             0
#define THREAD_STATE_READY                   1
#define THREAD_STATE_RUNNING                 2
#define THREAD_STATE_STANDBY                 3
#define THREAD_STATE_TERMINATED              4
#define THREAD_STATE_WAIT                    5
#define THREAD_STATE_TRANSITION              6
#define THREAD_STATE_UNKNOWN                 7

// =================================================================
// STATUS CODES
// =================================================================

#undef STATUS_INVALID_PARAMETER
#define STATUS_SUCCESS              ((NTSTATUS) 0x00000000)
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS) 0xC0000004)
#define STATUS_INVALID_PARAMETER    ((NTSTATUS) 0xC000000D)

// =================================================================
// SIMPLE TYPES
// =================================================================

typedef ULONG_PTR KPRIORITY, *PKPRIORITY, **PPKPRIORITY;
typedef LONG NTSTATUS,  *PNTSTATUS,  **PPNTSTATUS;

// =================================================================
// ENUMERATIONS
// =================================================================

typedef enum tagKWaitReason
{
/*000*/ Executive,
/*001*/ FreePage,
/*002*/ PageIn,
/*003*/ PoolAllocation,
/*004*/ DelayExecution,
/*005*/ Suspended,
/*006*/ UserRequest,
/*007*/ WrExecutive,
/*008*/ WrFreePage,
/*009*/ WrPageIn,
/*00A*/ WrPoolAllocation,
/*00B*/ WrDelayExecution,
/*00C*/ WrSuspended,
/*00D*/ WrUserRequest,
/*00E*/ WrEventPair,
/*00F*/ WrQueue,
/*010*/ WrLpcReceive,
/*011*/ WrLpcReply,
/*012*/ WrVirtualMemory,
/*013*/ WrPageOut,
/*014*/ WrRendezvous,
/*015*/ Spare2,
/*016*/ Spare3,
/*017*/ Spare4,
/*018*/ Spare5,
/*019*/ Spare6,
/*01A*/ WrKernel,
/*01B*/ MaximumWaitReason
} KWAIT_REASON, *PKWAIT_REASON, **PPKWAIT_REASON;

// =================================================================
// STRUCTURES
// =================================================================
//#pragma pack(1)

// -----------------------------------------------------------------
typedef struct tagClientId
{
/*000*/ HANDLE UniqueProcess;
/*004*/ HANDLE UniqueThread;
/*008*/
} CLIENT_ID, *PCLIENT_ID, **PPCLIENT_ID;
#define CLIENT_ID_SIZE sizeof(CLIENT_ID)

// -----------------------------------------------------------------

// sizeof(PVOID32) is always 32bit, even in the 64bit driver
// sizeof(PVOID) is 32bit in a 32bit driver and 64bit in a 64bit driver
typedef VOID * POINTER_32 PVOID32;

#ifdef _WIN64
  #define TypecastToPVOID32 PVOID32) (ULONG) (ULONG_PTR
#else
  #define TypecastToPVOID32 PVOID32
#endif

// -----------------------------------------------------------------
typedef struct tagUnicodeString
{
/*000*/ WORD  Length;
/*002*/ WORD  MaximumLength;
#ifdef _WIN64
        DWORD Dummy;
#endif
/*004*/ PWCHAR Buffer;
/*008*/
} UNICODE_STRING, *PUNICODE_STRING, **PPUNICODE_STRING;
#define UNICODE_STRING_SIZE sizeof(UNICODE_STRING)

typedef struct tagUnicodeString32
{
/*000*/ WORD  Length;
/*002*/ WORD  MaximumLength;
/*004*/ PVOID32 Buffer;
/*008*/
} UNICODE_STRING32, *PUNICODE_STRING32, **PPUNICODE_STRING32;

typedef struct tagUnicodeString64
{
/*000*/ WORD  Length;
/*002*/ WORD  MaximumLength;
/*004*/ DWORD Dummy;
/*008*/ ULONGLONG Buffer;
/*010*/
} UNICODE_STRING64, *PUNICODE_STRING64, **PPUNICODE_STRING64;

typedef struct tagAnsiString
{
   WORD  Length;
   WORD  MaximumLength;
   PCHAR Buffer;
} ANSI_STRING, *PANSI_STRING;
#define ANSI_STRING_SIZE sizeof(ANSI_STRING)
// -----------------------------------------------------------------

typedef struct tagRtlDriveLetterCurdir  // Size = 0x10
{   
   USHORT          Flags;
   USHORT          Length;
   ULONG           TimeStamp;
   UNICODE_STRING  DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct tagRtlUserProcessParameters // Size = 0x290
{   
    ULONG           AllocationSize;
    ULONG           Size;
    ULONG           Flags;
    ULONG           DebugFlags;
    HANDLE          hConsole;
    ULONG_PTR       ProcessGroup;
    HANDLE          hStdInput;
    HANDLE          hStdOutput;
    HANDLE          hStdError;
    UNICODE_STRING  CurrentDirectoryName;
    HANDLE          CurrentDirectoryHandle;
    UNICODE_STRING  DllPath;
    UNICODE_STRING  ImagePathName;
    UNICODE_STRING  CommandLine;
    PWSTR           Environment;
    ULONG           StartingPositionLeft;
    ULONG           StartingPositionTop;
    ULONG           Width;
    ULONG           Height;
    ULONG           CharWidth;
    ULONG           CharHeight;
    ULONG_PTR       ConsoleTextAttributes;
    ULONG           WindowFlags;
    ULONG           ShowWindowFlags;
    UNICODE_STRING  WindowTitle;
    UNICODE_STRING  DesktopName;
    UNICODE_STRING  ShellInfo;
    UNICODE_STRING  RuntimeInfo;
    RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

// -----------------------------------------------------------------
typedef struct tagPEB
{
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[229];
    PVOID Reserved3[59];
    ULONG SessionId;
} PEB, *PPEB;
// -----------------------------------------------------------------
typedef struct _LDR_DATA_TABLE_ENTRY
{
     LIST_ENTRY InLoadOrderLinks;
     LIST_ENTRY InMemoryOrderLinks;
     LIST_ENTRY InInitializationOrderLinks;
     PVOID DllBase;
     PVOID EntryPoint;
     ULONG SizeOfImage;
     UNICODE_STRING FullDllName;
     UNICODE_STRING BaseDllName;
     ULONG Flags;
     WORD LoadCount;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
// -----------------------------------------------------------------
typedef void (*PPEBLOCKROUTINE)(PVOID);

typedef struct tagPebLdrDATA  
{                          
    ULONG           Length;                            
    #ifdef _WIN64
      BOOL            Initialized;
      PVOID           Dummy;
    #else
      BOOLEAN         Initialized;
    #endif
    PVOID           SsHandle;                           
    LIST_ENTRY      InLoadOrderModuleList;              
    LIST_ENTRY      InMemoryOrderModuleList;            
    LIST_ENTRY      InInitializationOrderModuleList;    
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB_FREE_BLOCK  // Size = 8
{ 
   struct _PEB_FREE_BLOCK *Next;
   ULONG Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

// PEB (Process Environment Block) data structure (FS:[0x30])
// Located at addr. 0x7FFDF000, Size = 0x1E8
typedef struct tagPebNt
{       
   BOOLEAN                      InheritedAddressSpace;           //000 000
   BOOLEAN                      ReadImageFileExecOptions;        //001 001
   BOOLEAN                      BeingDebugged;                   //002 002
   BOOLEAN                      SpareBool;                       //003 003 Allocation size
   #ifdef _WIN64
   DWORD                        Dummy1;                          //    004
   #endif
   HANDLE                       Mutant;                          //004 008
   HINSTANCE                    ImageBaseAddress;                //008 010 Instance
   PPEB_LDR_DATA                LdrData;                         //00C 018
   PRTL_USER_PROCESS_PARAMETERS ProcessParameters;               //010 020 020
   PVOID                        SubSystemData;                   //014 028
   HANDLE                       ProcessHeap;                     //018 030
   KSPIN_LOCK                   FastPebLock;                     //01C 038
   PPEBLOCKROUTINE              FastPebLockRoutine;              //020 040
   PPEBLOCKROUTINE              FastPebUnlockRoutine;            //024 048
   ULONG                        EnvironmentUpdateCount;          //028 050
   PVOID *                      KernelCallbackTable;             //02C 054
   PVOID                        EventLogSection;                 //030 05C
   PVOID                        EventLog;                        //034 064
   PPEB_FREE_BLOCK              FreeList;                        //038 06C
   ULONG                        TlsExpansionCounter;             //03C 074
   PVOID                        TlsBitmap;                       //040 078
   LARGE_INTEGER                TlsBitmapBits;                   //044 080
   PVOID                        ReadOnlySharedMemoryBase;        //04C 088
   PVOID                        ReadOnlySharedMemoryHeap;        //050 090
   PVOID *                      ReadOnlyStaticServerData;        //054 098
   PVOID                        AnsiCodePageData;                //058 0A0
   PVOID                        OemCodePageData;                 //05C 0A8
   PVOID                        UnicodeCaseTableData;            //060 0B0
   ULONG                        NumberOfProcessors;              //064 0B8
   ULONG                        NtGlobalFlag;                    //068 0BC Address of a local copy
   LARGE_INTEGER                CriticalSectionTimeout;          //070 0C0
   ULONG_PTR                    HeapSegmentReserve;              //078 0C8
   ULONG_PTR                    HeapSegmentCommit;               //07C 0D0
   ULONG_PTR                    HeapDeCommitTotalFreeThreshold;  //080 0D8
   ULONG_PTR                    HeapDeCommitFreeBlockThreshold;  //084 0E0
   ULONG                        NumberOfHeaps;                   //088 0E8
   ULONG                        MaximumNumberOfHeaps;            //08C 0EC
   PVOID **                     ProcessHeaps;                    //090 0F0
   PVOID                        GdiSharedHandleTable;            //094 0F8
   PVOID                        ProcessStarterHelper;            //098 100
   PVOID                        GdiDCAttributeList;              //09C 108
   LPCRITICAL_SECTION           LoaderLock;                      //0A0 110
   ULONG                        OSMajorVersion;                  //0A4 118
   ULONG                        OSMinorVersion;                  //0A8 11C
   USHORT                       OSBuildNumber;                   //0AC 120
   USHORT                       OSCSDVersion;                    //0AE 122
   ULONG                        OSPlatformId;                    //0B0 124
   ULONG                        ImageSubsystem;                  //0B4 128
   ULONG                        ImageSubsystemMajorVersion;      //0B8 12C
   ULONG                        ImageSubsystemMinorVersion;      //0BC 130
   #ifdef _WIN64
   DWORD                        Dummy2;                          //    134
   #endif
   ULONG_PTR                    ImageProcessAffinityMask;        //0C0 138
   #ifdef _WIN64
   ULONG                        GdiHandleBuffer[0x3C];           //    140
   #else
   HANDLE                       GdiHandleBuffer[0x22];           //0C4
   #endif
   LPVOID                       PostProcessInitRoutine;          //14C 230
   LPVOID                       TlsExpansionBitmap;              //150 238
   UCHAR                        TlsExpansionBitmapBits[0x80];    //154 240
   ULONG                        SessionId;                       //1D4 2C0
   LPVOID                       AppCompatInfo;                   //1D8 2C4
   UNICODE_STRING               CSDVersion;                      //1DC 2CC
} PEB_NT, *PPEB_NT;

// TEB (Thread Environment Block) data structure (FS:[0x18])
// Located at 0x7FFDE000, 0x7FFDD000, ...
typedef struct _TEB_NT {                        // Size = 0xF88
  NT_TIB        Tib;                            //000
  PVOID         EnvironmentPointer;             //01C
  CLIENT_ID     ClientId;                       //020
  HANDLE        ActiveRpcHandle;                //028
  PVOID         ThreadLocalStoragePointer;      //02C
  PPEB_NT       ProcessEnvironmentBlock;        //030 PEB
  ULONG         LastErrorValue;                 //034
  ULONG         CountOfOwnedCriticalSections;   //038
  ULONG         CsrClientThread;                //03C
  ULONG         Win32ThreadInfo;                //040
  UCHAR         Win32ClientInfo[0x7C];          //044
  ULONG         WOW32Reserved;                  //0C0
  ULONG         CurrentLocale;                  //0C4
  ULONG         FpSoftwareStatusRegister;       //0C8
  UCHAR         SystemReserved1[0xD8];          //0CC
  ULONG         Spare1;                         //1A4
  ULONG         ExceptionCode;                  //1A8
  UCHAR         SpareBytes1[0x28];              //1AC
  UCHAR         SystemReserved2[0x28];          //1D4
  UCHAR         GdiTebBatch[0x4E0];             //1FC
  ULONG         GdiRgn;                         //6DC
  ULONG         GdiPen;                         //6E0
  ULONG         GdiBrush;                       //6E4
  CLIENT_ID     RealClientId;                   //6E8
  ULONG         GdiCachedProcessHandle;         //6F0
  ULONG         GdiClientPID;                   //6F4
  ULONG         GdiClientTID;                   //6F8
  ULONG         GdiThreadLocalInfo;             //6FC
  UCHAR         UserReserved[0x14];             //700
  UCHAR         glDispatchTable[0x460];         //714
  UCHAR         glReserved1[0x68];              //B74
  ULONG         glReserved2;                    //BDC
  ULONG         glSectionInfo;                  //BE0
  ULONG         glSection;                      //BE4
  ULONG         glTable;                        //BE8
  ULONG         glCurrentRC;                    //BEC
  ULONG         glContext;                      //BF0
  ULONG         LastStatusValue;                //BF4
  LARGE_INTEGER StaticUnicodeString;            //BF8
  UCHAR         StaticUnicodeBuffer[0x20C];     //C00
  ULONG         DeallocationStack;              //E0C
  UCHAR         TlsSlots[0x100];                //E10
  LARGE_INTEGER TlsLinks;                       //F10
  ULONG         Vdm;                            //F18
  ULONG         ReservedForNtRpc;               //F1C
  LARGE_INTEGER DbgSsReserved;                  //F20
  ULONG         HardErrorsAreDisabled;          //F28
  UCHAR         Instrumentation[0x40];          //F2C
  ULONG         WinSockData;                    //F6C
  ULONG         GdiBatchCount;                  //F70
  ULONG         Spare2;                         //F74
  ULONG         Spare3;                         //F78
  ULONG         Spare4;                         //F7C
  ULONG         ReservedForOle;                 //F80
  ULONG         WaitingOnLoaderLock;            //F84
//  PVOID         StackCommit;
//  PVOID         StackCommitMax;
//  PVOID         StackReserved;
//  PVOID         MessageQueue;
} TEB_NT, *PTEB_NT;

// -----------------------------------------------------------------
typedef struct tagObjectAttributes
{
    ULONG Length;
    #ifdef _WIN64
       ULONG Dummy1;
    #endif
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    #ifdef _WIN64
       ULONG Dummy2;
    #endif
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
#define OBJECT_ATTRIBUTES_SIZE sizeof(OBJECT_ATTRIBUTES_SIZE)
// -----------------------------------------------------------------
#pragma pack(1)
typedef struct tagVmCounters
{
/*000*/ SIZE_T PeakVirtualSize;
/*004*/ SIZE_T VirtualSize;
/*008*/ ULONG PageFaultCount;
/*00C*/ SIZE_T PeakWorkingSetSize;
/*010*/ SIZE_T WorkingSetSize;
/*014*/ SIZE_T QuotaPeakPagedPoolUsage;
/*018*/ SIZE_T QuotaPagedPoolUsage;
/*01C*/ SIZE_T QuotaPeakNonPagedPoolUsage;
/*020*/ SIZE_T QuotaNonPagedPoolUsage;
/*024*/ SIZE_T PagefileUsage;
/*028*/ SIZE_T PeakPagefileUsage;
/*02C*/ 
} VM_COUNTERS, *PVM_COUNTERS, **PPVM_COUNTERS;
#define VM_COUNTERS_SIZE sizeof(VM_COUNTERS)
#pragma pack()

// =================================================================
// SYSTEM INFO CLASSES
// =================================================================
typedef enum tagSystemInfoClass
{
    SystemBasicInformation,             // 0x002C
    SystemProcessorInformation,         // 0x000C
    SystemPerformanceInformation,       // 0x0138
    SystemTimeInformation,              // 0x0020
    SystemPathInformation,              // not implemented
    SystemProcessInformation,           // 0x00F8+ per process
    SystemCallInformation,              // 0x0018 + (n * 0x0004)
    SystemConfigurationInformation,     // 0x0018
    SystemProcessorCounters,            // 0x0030 per cpu
    SystemGlobalFlag,                   // 0x0004
    SystemInfo10,                       // not implemented
    SystemModuleInformation,            // 0x0004 + (n * 0x011C)
    SystemLockInformation,              // 0x0004 + (n * 0x0024)
    SystemInfo13,                       // not implemented
    SystemPagedPoolInformation,         // checked build only
    SystemNonPagedPoolInformation,      // checked build only
    SystemHandleInformation,            // 0x0004  + (n * 0x0010)
    SystemObjectInformation,            // 0x0038+ + (n * 0x0030+)
    SystemPagefileInformation,          // 0x0018+ per page file
    SystemInstemulInformation,          // 0x0088
    SystemInfo20,                       // invalid info class
    SystemCacheInformation,             // 0x0024
    SystemPoolTagInformation,           // 0x0004 + (n * 0x001C)
    SystemProcessorStatistics,          // 0x0000, or 0x0018 per cpu
    SystemDpcInformation,               // 0x0014
    SystemMemoryUsageInformation1,      // checked build only
    SystemLoadImage,                    // 0x0018, set mode only
    SystemUnloadImage,                  // 0x0004, set mode only
    SystemTimeAdjustmentInformation,    // 0x000C, 0x0008 writeable
    SystemMemoryUsageInformation2,      // checked build only
    SystemInfo30,                       // checked build only
    SystemInfo31,                       // checked build only
    SystemCrashDumpInformation,         // 0x0004
    SystemExceptionInformation,         // 0x0010
    SystemCrashDumpStateInformation,    // 0x0008
    SystemDebuggerInformation,          // 0x0002
    SystemThreadSwitchInformation,      // 0x0030
    SystemRegistryQuotaInformation,     // 0x000C
    SystemLoadDriver,                   // 0x0008, set mode only
    SystemPrioritySeparationInformation,// 0x0004, set mode only
    SystemInfo40,                       // not implemented
    SystemInfo41,                       // not implemented
    SystemInfo42,                       // invalid info class
    SystemInfo43,                       // invalid info class
    SystemTimeZoneInformation,          // 0x00AC
    SystemLookasideInformation,         // n * 0x0020

// info classes specific to Windows 2000
// WTS = Windows Terminal Server

    SystemSetTimeSlipEvent,             // set mode only
    SystemCreateSession,                // WTS, set mode only
    SystemDeleteSession,                // WTS, set mode only
    SystemInfo49,                       // invalid info class
    SystemRangeStartInformation,        // 0x0004
    SystemVerifierInformation,          // 0x0068
    SystemAddVerifier,                  // set mode only
    SystemSessionProcessesInformation,  // WTS
}  SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS, **PPSYSTEM_INFORMATION_CLASS;

typedef enum tagProcessInfoClass
{
    ProcessBasicInformation   = 0,
    ProcessSessionInformation = 24,
    ProcessWow64Information   = 26
} PROCESS_INFO_CLASS;

typedef enum tagThreadInfoClass
{
    ThreadBasicInformation = 0
} THREAD_INFO_CLASS;

typedef struct tagSystemThread
{
/*000*/ FILETIME   KernelTime;      // 100 nsec units          
/*008*/ FILETIME   UserTime;        // 100 nsec units
/*010*/ FILETIME   CreateTime;      // relative to 01-01-1601
/*018*/ ULONG_PTR  WaitTime;
/*01C*/ PVOID      StartAddress;
/*020*/ CLIENT_ID  ClientId;        // Unique Process / Unique Thread Handles
/*028*/ DWORD      Priority;
/*02C*/ DWORD      BasePriority;
/*030*/ DWORD      ContextSwitches;
/*034*/ DWORD      ThreadState;     // 2=running, 5=waiting
/*038*/ KWAIT_REASON WaitReason;
/*03C*/ DWORD      Reserved01;
/*040*/ 
}  SYSTEM_THREAD, *PSYSTEM_THREAD;
#define SYSTEM_THREAD_SIZE sizeof(SYSTEM_THREAD)

// -----------------------------------------------------------------
typedef struct tagProcessBasicInformation
{
    PVOID Reserved1;
    PPEB  PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;
typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;

typedef struct tagThreadBasicInformation
{
    NTSTATUS  ExitStatus;
    PVOID     TebBaseAddress;
    ULONG     UniqueProcessId;
    ULONG     UniqueThreadId;
    KAFFINITY AffinityMask;
    KPRIORITY BasePriority;
    ULONG     DiffProcessPriority;
} THREAD_BASIC_INFORMATION;
// -----------------------------------------------------------------
typedef struct tagSystemProcess     // common members
{
/*000*/ DWORD     Next;             // relative offset
/*004*/ DWORD     ThreadCount;
/*008*/ DWORD     Reserved01;
/*00C*/ DWORD     Reserved02;
/*010*/ DWORD     Reserved03;
/*014*/ DWORD     Reserved04;
/*018*/ DWORD     Reserved05;
/*01C*/ DWORD     Reserved06;
/*020*/ FILETIME  CreateTime;       // relative to 01-01-1601
/*028*/ FILETIME  UserTime;         // 100 nsec units
/*030*/ FILETIME  KernelTime;       // 100 nsec units
/*038*/ UNICODE_STRING Name;
/*040*/ KPRIORITY BasePriority;
/*044*/ ULONG_PTR UniqueProcessId;
/*048*/ ULONG_PTR InheritedFromUniqueProcessId;
/*04C*/ DWORD     HandleCount;
/*050*/ DWORD     SessionId;        // per madshi
/*054*/ DWORD     Reserved08;
/*058*/ VM_COUNTERS  VmCounters;    // see ntddk.h
/*084*/ ULONG_PTR CommitCharge;     // bytes
#ifdef _WIN64
        ULONG_PTR Unknown;
#endif
/*088*/ 
} SYSTEM_PROCESS, *PSYSTEM_PROCESS;
#define SYSTEM_PROCESS_SIZE sizeof(SYSTEM_PROCESS)

// -----------------------------------------------------------------
typedef struct tagSystemProcessNT4     // Windows NT 4.0
{
/*000*/ SYSTEM_PROCESS  Process;       // common members
/*088*/ SYSTEM_THREAD   Threads[1];     // thread array
/*088*/
} SYSTEM_PROCESS_NT4, *PSYSTEM_PROCESS_NT4;
#define SYSTEM_PROCESS_NT4_SIZE sizeof(SYSTEM_PROCESS_NT4)

// -----------------------------------------------------------------
typedef struct tagSystemProcessNT5     // Windows 2000
{
/*000*/ SYSTEM_PROCESS  Process;       // common members
/*088*/ IO_COUNTERS     IoCounters;    // see ntddk.h
/*0B8*/ SYSTEM_THREAD   Threads[1];     // thread array
/*0B8*/
} SYSTEM_PROCESS_NT5, *PSYSTEM_PROCESS_NT5;
#define SYSTEM_PROCESS_NT5_SIZE sizeof(SYSTEM_PROCESS_NT5)

// -----------------------------------------------------------------
typedef union tagSystemProcessInformation
{
/*000*/ SYSTEM_PROCESS     Process;
/*000*/ SYSTEM_PROCESS_NT4 Process_NT4;
/*000*/ SYSTEM_PROCESS_NT5 Process_NT5;
/*0B8*/ 
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;
#define SYSTEM_PROCESS_INFORMATION_SIZE sizeof(SYSTEM_PROCESS_INFORMATION)

//-----------------------------------------------------------------------------------------------------------------------------------------

/** Typedef for the exception handler function prototype */
typedef DWORD (FN_EXCEPTION_HANDLER)( EXCEPTION_RECORD *pException, struct _EXCEPTION_REGISTRATION_RECORD *pRegistrationRecord, CONTEXT *pContext );

#if (_MSC_VER < 1700)

  /** Definition of 'raw' WinNt exception registration record - this ought to be in WinNt.h */
  struct _EXCEPTION_REGISTRATION_RECORD
  {
     struct _EXCEPTION_REGISTRATION_RECORD *PreviousExceptionRegistrationRecord; // Chain to previous record
     FN_EXCEPTION_HANDLER *ExceptionHandler;                                     // Handler function being registered
  };

#endif

//#pragma pack()

#endif