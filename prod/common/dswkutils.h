// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc.,

// Redwood City CA,

// Ownership remains with Blue Jungle Inc, All rights reserved worldwide.

#include <ntifs.h>
#include "dstypes.h"
#ifndef _DSWKUTILS_H_
#define _DSWKUTILS_H_


//typedef
typedef struct tagFILE_NAMESTRING_INFORMATION
{
	ULONG FileNameLength;
	WCHAR FileName[MAX_NAME_LENGTH];
}FILE_NAMESTRING_INFORMATION, *PFILE_NAMESTRING_INFORMATION;

typedef struct tagFILE_COMPLETE_INFORMATION
{
    FILE_BASIC_INFORMATION BasicInformation;
    FILE_STANDARD_INFORMATION StandardInformation;
    FILE_INTERNAL_INFORMATION InternalInformation;
    FILE_EA_INFORMATION EaInformation;
    FILE_ACCESS_INFORMATION AccessInformation;
    FILE_POSITION_INFORMATION PositionInformation;
    FILE_MODE_INFORMATION ModeInformation;
    FILE_ALIGNMENT_INFORMATION AlignmentInformation;
	FILE_NAMESTRING_INFORMATION NameInformation;
}FILE_COMPLETE_INFORMATION, *PFILE_COMPLETE_INFORMATION;

//Function prototypes
BOOL DSK_CloseKernelRecoveryEvent();
BOOL DSK_CreateKernelRecoveryEvent(BOOL fFileAgent);
BOOL DSK_SetRecoveryEvent();

NTSTATUS
DSK_GetRegistryNumKeyEntries(IN PUNICODE_STRING RegistryPath,
							 OUT PULONG pulNumOfShare);

NTSTATUS 
DSK_ReadDriverParameters (
    IN PUNICODE_STRING RegistryPath,
	IN PCWSTR  pwzSourceString,
	OUT PVOID pReturnData
    );

NTSTATUS
DSK_GetFileInfo(PVOID pInputBuffer, ULONG ulInputBufferLength,
				PVOID pOutputBuffer, ULONG ulOutputBufferLenth,
				PIO_STATUS_BLOCK pIoStatus);

PCHAR 
PnPMinorFunctionString ( UCHAR MinorFunction );


//ntifs.h do not contain this like ntddk.h
typedef VOID(*PREQUEST_POWER_COMPLETE) (
			IN PDEVICE_OBJECT DeviceObject,
			IN UCHAR MinorFunction,
			IN POWER_STATE PowerState,
			IN PVOID Context,
			IN PIO_STATUS_BLOCK IoStatus );

//ntifs.h do not contain this like ntddk.h
NTKERNELAPI NTSTATUS
PoRequestPowerIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PREQUEST_POWER_COMPLETE CompletionFunction,
    IN PVOID Context,
    OUT PIRP *Irp OPTIONAL);

//ntifs.h for Win2K doesn't include these files
#if WINVER < 0x0501
NTKERNELAPI
NTSTATUS
PoCallDriver (
    IN PDEVICE_OBJECT   DeviceObject,
    IN OUT PIRP         Irp
    );

NTKERNELAPI
VOID
PoStartNextPowerIrp(
    IN PIRP    Irp
    );


NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Length,
    OUT PULONG LengthNeeded
    );
#endif //#if WINVER < 0x0501


#endif	//#ifndef _DSWKUTILS_H_