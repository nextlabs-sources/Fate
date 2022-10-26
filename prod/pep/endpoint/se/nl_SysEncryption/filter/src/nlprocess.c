

#include <ntifs.h>
#include "nlprocess.h"


typedef struct _PROCESSMGR
{
    RTL_GENERIC_TABLE   Table;
    FAST_MUTEX          TableMutex;
} PROCESSMGR, *PPROCESSMGR;

typedef struct _PROCESSINFO
{
    HANDLE          ProcessId;
    ULONG           Trusted;    // 0: unknown, 1: untrusted, 2: trusted
} PROCESSINFO, *PPROCESSINFO;


static PPROCESSMGR Mgr = NULL;

RTL_GENERIC_COMPARE_ROUTINE     NLTableCompareRoutine;
RTL_GENERIC_ALLOCATE_ROUTINE    NLTableAllocateRoutine;
RTL_GENERIC_FREE_ROUTINE        NLTableFreeRoutine;

static
VOID
NLCreateProcessNotifyRoutine(
                             __in HANDLE  ParentId,
                             __in HANDLE  ProcessId,
                             __in BOOLEAN Create
                             );

static
RTL_GENERIC_COMPARE_RESULTS
NLTableCompareRoutine(
                      __in struct _RTL_GENERIC_TABLE  *Table,
                      __in PVOID  FirstStruct,
                      __in PVOID  SecondStruct
                      );

static
PVOID
NLTableAllocateRoutine(
                       __in struct _RTL_GENERIC_TABLE  *Table,
                       __in CLONG  ByteSize
                       );

static
VOID
NLTableFreeRoutine(
                   __in struct _RTL_GENERIC_TABLE  *Table,
                   __in PVOID  Buffer
                   );



//
// Create
//
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
ProcessMgrCreate(
                 )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    if(NULL != Mgr)
        return STATUS_SUCCESS;

    Mgr = ExAllocatePoolWithTag(NonPagedPool, sizeof(PROCESSMGR), 'esln');
    if(NULL == Mgr)
        return STATUS_INSUFFICIENT_RESOURCES;
    RtlZeroMemory(Mgr, sizeof(PROCESSMGR));

    Status = PsSetCreateProcessNotifyRoutine(NLCreateProcessNotifyRoutine, FALSE);
    if(!NT_SUCCESS(Status))
    {
        ExFreePool(Mgr);
        Mgr = NULL;
        return Status;
    }

    ExInitializeFastMutex(&Mgr->TableMutex);
    RtlInitializeGenericTable(&Mgr->Table, NLTableCompareRoutine, NLTableAllocateRoutine, NLTableFreeRoutine, NULL);
    return STATUS_SUCCESS;
}


VOID
ProcessMgrClose(
                )
{
    PPROCESSINFO info = NULL;
    PROCESSINFO  id   = {NULL, 0};

    if(NULL == Mgr) return;

    PsSetCreateProcessNotifyRoutine(NLCreateProcessNotifyRoutine, TRUE);

    ExAcquireFastMutex(&Mgr->TableMutex);
    while(!RtlIsGenericTableEmpty(&Mgr->Table))
    {
        info = RtlGetElementGenericTable(&Mgr->Table, 0);
        if(NULL==info)
            break;

        id.ProcessId = info->ProcessId;
        id.Trusted   = info->Trusted;
        RtlDeleteElementGenericTable(&Mgr->Table, &id);
    }
    ExReleaseFastMutex(&Mgr->TableMutex);

    // Free memory
    ExFreePool(Mgr);
}


VOID
NLUpdateProcessInfo(
                    __in HANDLE ProcessId,
                    __in ULONG Trusted
                    )
{
    PPROCESSINFO info = NULL;
    PROCESSINFO  id   = {ProcessId, 0};

    if(NULL == Mgr) return;

    id.ProcessId = ProcessId;
    id.Trusted   = Trusted; // trusted : untrusted

    
    ExAcquireFastMutex(&Mgr->TableMutex);
    info = RtlLookupElementGenericTable(&Mgr->Table, &id);
    if(NULL == info)
        RtlInsertElementGenericTable(&Mgr->Table, &id, sizeof(PROCESSINFO), NULL);
    else
        info->Trusted = Trusted;
    ExReleaseFastMutex(&Mgr->TableMutex);
}

ULONG
NLGetProcessInfo(
                 __in HANDLE ProcessId
                 )
{
    PPROCESSINFO info = NULL;
    PROCESSINFO  id   = {ProcessId, 0};
    ULONG        Status = 0;
    
    if(NULL == Mgr) return Status;

    ExAcquireFastMutex(&Mgr->TableMutex);
    info = RtlLookupElementGenericTable(&Mgr->Table, &id);
    if(NULL == info)
        RtlInsertElementGenericTable(&Mgr->Table, &id, sizeof(PROCESSINFO), NULL);
    else
        Status = info->Trusted;
    ExReleaseFastMutex(&Mgr->TableMutex);

    return Status;
}


VOID
NLCreateProcessNotifyRoutine(
                             __in HANDLE  ParentId,
                             __in HANDLE  ProcessId,
                             __in BOOLEAN Create
                             )
{
    PROCESSINFO info = {ProcessId, 0};

    if(Create)
    {
        //
        // Process create
        //
        // Add this item to table
        ExAcquireFastMutex(&Mgr->TableMutex);
        RtlInsertElementGenericTable(&Mgr->Table, &info, sizeof(PROCESSINFO), NULL);
        ExReleaseFastMutex(&Mgr->TableMutex);
    }
    else
    {
        //
        // Process destory
        //
        // Remove this process from table
        ExAcquireFastMutex(&Mgr->TableMutex);
        RtlDeleteElementGenericTable(&Mgr->Table, &info);
        ExReleaseFastMutex(&Mgr->TableMutex);
    }
}

RTL_GENERIC_COMPARE_RESULTS
NLTableCompareRoutine(
                      __in struct _RTL_GENERIC_TABLE  *Table,
                      __in PVOID  FirstStruct,
                      __in PVOID  SecondStruct
                      )
{
    HANDLE pid1 = *((HANDLE*)FirstStruct);
    HANDLE pid2 = *((HANDLE*)SecondStruct);

    if(pid1 == pid2) return GenericEqual;
    else if(pid1 < pid2) return GenericLessThan;
    else return GenericGreaterThan;
}

PVOID
NLTableAllocateRoutine(
                       __in struct _RTL_GENERIC_TABLE  *Table,
                       __in CLONG  ByteSize
                       )
{
    return ExAllocatePoolWithTag(NonPagedPool, ByteSize, 'esln');
}

VOID
NLTableFreeRoutine(
                   __in struct _RTL_GENERIC_TABLE  *Table,
                   __in PVOID  Buffer
                   )
{
    ExFreePool(Buffer);
}