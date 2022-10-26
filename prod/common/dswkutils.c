// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc.,

// Redwood City CA,

// Ownership remains with Blue Jungle Inc, All rights reserved worldwide.

#include <ntifs.h>
#include "dstypes.h"
#include "dsipc.h"
#include "dswkutils.h"
#include "dskerneldebug.h"
#include "dskutypes.h"

//Global Variables
PKEVENT	gpkeRecovery = NULL;
HANDLE	ghRecovery = NULL;

BOOL DSK_CloseKernelRecoveryEvent()
{
	if (ghRecovery)
	{
		ZwClose(ghRecovery);
		return TRUE;
	}
	return FALSE;
}	

BOOL DSK_CreateKernelRecoveryEvent(BOOL fFileAgent)
{
	UNICODE_STRING	uszRecoveryEvent;

	if (fFileAgent)
		RtlInitUnicodeString(&uszRecoveryEvent, DSF_KERNEL_RECOVERY_EVENT_NAME);
	else
		RtlInitUnicodeString(&uszRecoveryEvent, DSD_KERNEL_RECOVERY_EVENT_NAME);

	gpkeRecovery = IoCreateNotificationEvent(&uszRecoveryEvent, &ghRecovery);
	if (gpkeRecovery == NULL)
	{
		DP(DSDERR,("InitIFSMasterObject<ERROR>Cannot create Recovery Event\n"));
		return FALSE;
	}
	else
		KeClearEvent(gpkeRecovery);
	return TRUE;
}

BOOL DSK_SetRecoveryEvent()
{
	if (gpkeRecovery)
	{
		KeSetEvent(gpkeRecovery, IO_NO_INCREMENT, FALSE);
		KeClearEvent(gpkeRecovery);
		return TRUE;
	}
	DP(DSDERR,("DSK_SetRecoveryEvent<ERROR>Recovery Event not initialized\n"));
	return FALSE;
}

NTSTATUS
DSK_GetRegistryNumKeyEntries(IN PUNICODE_STRING RegistryPath,
							 OUT PULONG pulNumOfShare)
{

	//TBD: Using ZwEnumerateValueKey loop around a counter until  get STATUS_NO_MORE_ENTRIES
	return STATUS_SUCCESS;
}

NTSTATUS 
DSK_ReadDriverParameters (
    IN PUNICODE_STRING RegistryPath,
	IN PCWSTR  pwzSourceString,
	OUT PVOID pReturnData
    )
{
	HANDLE driverRegKey;
	PVOID buffer = NULL;
    OBJECT_ATTRIBUTES attributes;
	UNICODE_STRING valueName;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG bufferSize, resultLength;   
    PKEY_VALUE_PARTIAL_INFORMATION pValuePartialInfo;

    PAGED_CODE();

    //
    //  All the global values are already set to default values.  Any
    //  values we read from the registry will override these defaults.
    //
    
    //
    //  Do the initial setup to start reading from the registry.
    //

    InitializeObjectAttributes( &attributes,
								RegistryPath,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL);

    status = ZwOpenKey( &driverRegKey,
						KEY_READ,
						&attributes);

    if (!NT_SUCCESS(status)) {

        driverRegKey = NULL;
        goto DSKReadDriverParameters_Exit;
    }

	//MAX_NAME_LENGTH is should be enough!!!
    bufferSize = sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + MAX_NAME_LENGTH;
    buffer = ExAllocatePool( NonPagedPool, bufferSize);

    if (NULL == buffer) {

        goto DSKReadDriverParameters_Exit;
    }

    RtlInitUnicodeString(&valueName, pwzSourceString);

    status = ZwQueryValueKey( driverRegKey,
							  &valueName,
							  KeyValuePartialInformation,
							  buffer,
							  bufferSize,
							  &resultLength);

    if (NT_SUCCESS(status)) {

        pValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
		memcpy(pReturnData, pValuePartialInfo->Data, pValuePartialInfo->DataLength);  		
    }
   
DSKReadDriverParameters_Exit:

    if (NULL != buffer) {

        ExFreePool(buffer);
    }

    if (NULL != driverRegKey) {

        ZwClose(driverRegKey);
    }

    return status;
}

NTSTATUS
DSK_GetFileInfo(PVOID pInputBuffer, ULONG ulInputBufferLength,
				PVOID pOutputBuffer, ULONG ulOutputBufferLength,
				PIO_STATUS_BLOCK pIoStatus)
{
	PFILE_INFO pFileInfo = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	FILE_COMPLETE_INFORMATION strFileAllInfo;

	pFileInfo = (FILE_INFO *)pInputBuffer;

	if (pFileInfo == NULL || pFileInfo->hFileHandle == NULL || ulInputBufferLength < sizeof(FILE_INFO) ||
		pOutputBuffer == NULL || ulOutputBufferLength < sizeof(FILE_INFO) || pIoStatus == NULL)
	{
		DP(DSDERR,("DSK_GetFileInfo<ERROR>Invalid parameter(s) received\n"));
		status = STATUS_INVALID_PARAMETER;
		goto DSK_GetFileInfoExit;
	}

	strFileAllInfo.NameInformation.FileNameLength = MAX_NAME_LENGTH;

	status = ZwQueryInformationFile(pFileInfo->hFileHandle,
									pIoStatus,
									&strFileAllInfo,
									sizeof(FILE_COMPLETE_INFORMATION),
									FileAllInformation);

	if (status != STATUS_SUCCESS)
	{
		DP(DSDERR,("DSK_GetFileInfo<ERROR>ZwQueryInformationFile FAILED status = 0x%x\n",status));
		goto DSK_GetFileInfoExit;
	}

	pFileInfo->ulHighCreationTime = strFileAllInfo.BasicInformation.CreationTime.HighPart;
	pFileInfo->ulLowCreationTime = strFileAllInfo.BasicInformation.CreationTime.LowPart;
	pFileInfo->ulHighLastAccessTime = strFileAllInfo.BasicInformation.LastAccessTime.HighPart;
	pFileInfo->ulLowLastAccessTime = strFileAllInfo.BasicInformation.LastAccessTime.LowPart;
	pFileInfo->ulHighLastWriteTime = strFileAllInfo.BasicInformation.LastWriteTime.HighPart;
	pFileInfo->ulLowLastWriteTime = strFileAllInfo.BasicInformation.LastAccessTime.HighPart;
	pFileInfo->ulAttribute = strFileAllInfo.BasicInformation.FileAttributes;
	pFileInfo->ulHighAllocationSize = strFileAllInfo.StandardInformation.AllocationSize.HighPart;
	pFileInfo->ulLowAllocationSize = strFileAllInfo.StandardInformation.AllocationSize.LowPart;
	pFileInfo->ulHighEndOfFile = strFileAllInfo.StandardInformation.EndOfFile.HighPart;
	pFileInfo->ulLowEndOfFile = strFileAllInfo.StandardInformation.EndOfFile.LowPart;
	pFileInfo->ulAccessMaskFlag = strFileAllInfo.AccessInformation.AccessFlags;
	pFileInfo->ulMode = strFileAllInfo.ModeInformation.Mode;
	pFileInfo->ulHighCurrentByteOffset = strFileAllInfo.PositionInformation.CurrentByteOffset.HighPart;
	pFileInfo->ulLowCurrentByteOffset = strFileAllInfo.PositionInformation.CurrentByteOffset.LowPart;
	pFileInfo->ulHighUniqueID = strFileAllInfo.InternalInformation.IndexNumber.HighPart;
	pFileInfo->ulLowUniqueID = strFileAllInfo.InternalInformation.IndexNumber.LowPart;
	pFileInfo->ulNumOfLinks = strFileAllInfo.StandardInformation.NumberOfLinks;
	pFileInfo->fDirectory = strFileAllInfo.StandardInformation.Directory;
	pFileInfo->fDeletePending = strFileAllInfo.StandardInformation.DeletePending;	
	if (strFileAllInfo.NameInformation.FileNameLength < MAX_NAME_LENGTH || 
		strFileAllInfo.NameInformation.FileNameLength == 0)
	{
		pFileInfo->ulFileNameLength = strFileAllInfo.NameInformation.FileNameLength;
		pFileInfo->wzFileName[pFileInfo->ulFileNameLength / 2] = 0;	//NULL terminator
		memcpy(pFileInfo->wzFileName, strFileAllInfo.NameInformation.FileName, pFileInfo->ulFileNameLength);
	}
	else
	{
		DP(DSDWRN,("Cannot get FileName, FileNameSize = %d\n", strFileAllInfo.NameInformation.FileNameLength));
		pFileInfo->ulFileNameLength = 0;
		memset(pFileInfo->wzFileName, 0, MAX_NAME_LENGTH * 2);
	}

DSK_GetFileInfoExit:
	return status;
}

PCHAR PnPMinorFunctionString ( UCHAR MinorFunction )
{
    switch (MinorFunction)
    {
        case IRP_MN_START_DEVICE:
            return "IRP_MN_START_DEVICE";
        case IRP_MN_QUERY_REMOVE_DEVICE:
            return "IRP_MN_QUERY_REMOVE_DEVICE";
        case IRP_MN_REMOVE_DEVICE:
            return "IRP_MN_REMOVE_DEVICE";
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            return "IRP_MN_CANCEL_REMOVE_DEVICE";
        case IRP_MN_STOP_DEVICE:
            return "IRP_MN_STOP_DEVICE";
        case IRP_MN_QUERY_STOP_DEVICE:
            return "IRP_MN_QUERY_STOP_DEVICE";
        case IRP_MN_CANCEL_STOP_DEVICE:
            return "IRP_MN_CANCEL_STOP_DEVICE";
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            return "IRP_MN_QUERY_DEVICE_RELATIONS";
        case IRP_MN_QUERY_INTERFACE:
            return "IRP_MN_QUERY_INTERFACE";
        case IRP_MN_QUERY_CAPABILITIES:
            return "IRP_MN_QUERY_CAPABILITIES";
        case IRP_MN_QUERY_RESOURCES:
            return "IRP_MN_QUERY_RESOURCES";
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";
        case IRP_MN_QUERY_DEVICE_TEXT:
            return "IRP_MN_QUERY_DEVICE_TEXT";
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";
        case IRP_MN_READ_CONFIG:
            return "IRP_MN_READ_CONFIG";
        case IRP_MN_WRITE_CONFIG:
            return "IRP_MN_WRITE_CONFIG";
        case IRP_MN_EJECT:
            return "IRP_MN_EJECT";
        case IRP_MN_SET_LOCK:
            return "IRP_MN_SET_LOCK";
        case IRP_MN_QUERY_ID:
            return "IRP_MN_QUERY_ID";
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            return "IRP_MN_QUERY_PNP_DEVICE_STATE";
        case IRP_MN_QUERY_BUS_INFORMATION:
            return "IRP_MN_QUERY_BUS_INFORMATION";
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return "IRP_MN_DEVICE_USAGE_NOTIFICATION";
        case IRP_MN_SURPRISE_REMOVAL:
            return "IRP_MN_SURPRISE_REMOVAL";

        default:
            return "unknown_pnp_irp";
    }
}
