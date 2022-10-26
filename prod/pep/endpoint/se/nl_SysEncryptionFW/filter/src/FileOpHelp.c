

#include <ntifs.h>
#include "FileOpHelp.h"
#include "NLSEStruct.h"

NTSTATUS
FOH_GET_FILE_STANDARDINFO_BY_NAME(
                                  __in PFLT_FILTER Filter,
                                  __in PFLT_INSTANCE Instance,
                                  __in_opt PUNICODE_STRING FileName,
                                  __inout_opt PFILE_STANDARD_INFORMATION fsi
                                  )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES   objAttr;
    IO_STATUS_BLOCK     ioStatusBlock;
    HANDLE              FileHandle = NULL;
    PFILE_OBJECT        FileObject = NULL;

    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    // Sanity check
    if(NULL==FileName || NULL==fsi)
        return STATUS_INVALID_PARAMETER;

    //Initialization
    InitializeObjectAttributes(&objAttr,
        FileName,
        OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    // Open the file
    Status = FltCreateFile(Filter,
        Instance,
        &FileHandle,
        FILE_READ_ATTRIBUTES|SYNCHRONIZE,
        &objAttr,
        &ioStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT |FILE_OPEN_REPARSE_POINT|FILE_NO_INTERMEDIATE_BUFFERING,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Reference File Object
    Status = ObReferenceObjectByHandle(FileHandle,       //Handle
        0,            //DesiredAccess
        NULL,         //ObjectType
        KernelMode,   //AccessMode
        &FileObject,  //File Handle
        NULL);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Get standard file information
    Status = FOH_GET_FILE_STANDARDINFO_BY_HANDLE(Instance, FileObject, fsi);

_exit:
    if(FileObject) ObDereferenceObject(FileObject); FileObject=NULL;
    if(FileHandle) FltClose(FileHandle); FileHandle=NULL;
    return Status;
}


NTSTATUS
FOH_SET_FILE_STANDARDINFO_BY_NAME(
                                  __in PFLT_FILTER Filter,
                                  __in PFLT_INSTANCE Instance,
                                  __in_opt PUNICODE_STRING FileName,
                                  __in_opt PFILE_STANDARD_INFORMATION fsi
                                  )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES   objAttr;
    IO_STATUS_BLOCK     ioStatusBlock;
    HANDLE              FileHandle = NULL;
    PFILE_OBJECT        FileObject = NULL;

    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    // Sanity check
    if(NULL==FileName || NULL==fsi)
        return STATUS_INVALID_PARAMETER;

    //Initialization
    InitializeObjectAttributes(&objAttr,
        FileName,
        OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    // Open the file
    Status = FltCreateFile(Filter,
        Instance,
        &FileHandle,
        FILE_WRITE_ATTRIBUTES|SYNCHRONIZE,
        &objAttr,
        &ioStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT |FILE_OPEN_REPARSE_POINT|FILE_NO_INTERMEDIATE_BUFFERING,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Reference File Object
    Status = ObReferenceObjectByHandle(FileHandle,       //Handle
        0,            //DesiredAccess
        NULL,         //ObjectType
        KernelMode,   //AccessMode
        &FileObject,  //File Handle
        NULL);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Set standard file information
    Status = FOH_SET_FILE_STANDARDINFO_BY_HANDLE(Instance, FileObject, fsi);

_exit:
    if(FileObject) ObDereferenceObject(FileObject); FileObject=NULL;
    if(FileHandle) FltClose(FileHandle); FileHandle=NULL;
    return Status;
}

NTSTATUS
FOH_SET_FILE_BASICINFO_BY_NAME(
                               __in PFLT_FILTER Filter,
                               __in PFLT_INSTANCE Instance,
                               __in_opt PUNICODE_STRING FileName,
                               __in_opt PFILE_BASIC_INFORMATION fbi
                               )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES   objAttr;
    IO_STATUS_BLOCK     ioStatusBlock;
    HANDLE              FileHandle = NULL;
    PFILE_OBJECT        FileObject = NULL;

    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    // Sanity check
    if(NULL==FileName || NULL==fbi)
        return STATUS_INVALID_PARAMETER;

    //Initialization
    InitializeObjectAttributes(&objAttr,
        FileName,
        OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    // Open the file
    Status = FltCreateFile(Filter,
        Instance,
        &FileHandle,
        FILE_WRITE_ATTRIBUTES|SYNCHRONIZE,
        &objAttr,
        &ioStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT |FILE_OPEN_REPARSE_POINT|FILE_NO_INTERMEDIATE_BUFFERING,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Reference File Object
    Status = ObReferenceObjectByHandle(FileHandle,       //Handle
        0,            //DesiredAccess
        NULL,         //ObjectType
        KernelMode,   //AccessMode
        &FileObject,  //File Handle
        NULL);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Set basic file information
    Status = FOH_SET_FILE_BASICINFO_BY_HANDLE(Instance, FileObject, fbi);

_exit:
    if(FileObject) ObDereferenceObject(FileObject); FileObject=NULL;
    if(FileHandle) FltClose(FileHandle); FileHandle=NULL;
    return Status;
}

NTSTATUS
FOH_GET_FILE_BASICINFO_BY_NAME(
                               __in PFLT_FILTER Filter,
                               __in PFLT_INSTANCE Instance,
                               __in_opt PUNICODE_STRING FileName,
                               __inout_opt PFILE_BASIC_INFORMATION fbi
                               )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES   objAttr;
    IO_STATUS_BLOCK     ioStatusBlock;
    HANDLE              FileHandle = NULL;
    PFILE_OBJECT        FileObject = NULL;

    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    // Sanity check
    if(NULL==FileName || NULL==fbi)
        return STATUS_INVALID_PARAMETER;

    //Initialization
    InitializeObjectAttributes(&objAttr,
        FileName,
        OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    // Open the file
    Status = FltCreateFile(Filter,
        Instance,
        &FileHandle,
        FILE_READ_ATTRIBUTES|SYNCHRONIZE,
        &objAttr,
        &ioStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT |FILE_OPEN_REPARSE_POINT|FILE_NO_INTERMEDIATE_BUFFERING,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Reference File Object
    Status = ObReferenceObjectByHandle(FileHandle,       //Handle
        0,            //DesiredAccess
        NULL,         //ObjectType
        KernelMode,   //AccessMode
        &FileObject,  //File Handle
        NULL);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Get standard file information
    Status = FOH_GET_FILE_BASICINFO_BY_HANDLE(Instance,
        FileObject,
        fbi);


_exit:
    if(FileObject) ObDereferenceObject(FileObject); FileObject=NULL;
    if(FileHandle) FltClose(FileHandle); FileHandle=NULL;
    return Status;
}



NTSTATUS
FOH_GET_FILE_POSITIONINFO_BY_NAME(
                                  __in PFLT_FILTER Filter,
                                  __in PFLT_INSTANCE Instance,
                                  __in PUNICODE_STRING FileName,
                                  __inout_opt PFILE_POSITION_INFORMATION fpi
                                  )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES   objAttr;
    IO_STATUS_BLOCK     ioStatusBlock;
    HANDLE              FileHandle = NULL;
    PFILE_OBJECT        FileObject = NULL;

    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    // Sanity check
    if(NULL==FileName || NULL==fpi)
        return STATUS_INVALID_PARAMETER;

    //Initialization
    InitializeObjectAttributes(&objAttr,
        FileName,
        OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    // Open the file
    Status = FltCreateFile(Filter,
        Instance,
        &FileHandle,
        GENERIC_READ,
        &objAttr,
        &ioStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT |FILE_OPEN_REPARSE_POINT|FILE_NO_INTERMEDIATE_BUFFERING,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Reference File Object
    Status = ObReferenceObjectByHandle(FileHandle,       //Handle
        0,            //DesiredAccess
        NULL,         //ObjectType
        KernelMode,   //AccessMode
        &FileObject,  //File Handle
        NULL);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Get standard file information
    Status = FOH_GET_FILE_POSITIONINFO_BY_HANDLE(Instance,
        FileObject,
        fpi);

_exit:
    if(FileObject) ObDereferenceObject(FileObject); FileObject=NULL;
    if(FileHandle) FltClose(FileHandle); FileHandle=NULL;
    return Status;
}


NTSTATUS
FOH_GET_FILE_STANDARDINFO_BY_HANDLE(
                                    __in PFLT_INSTANCE Instance,
                                    __in_opt PFILE_OBJECT FileObject,
                                    __inout_opt PFILE_STANDARD_INFORMATION fsi
                                    )
{
    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    // Sanity check
    if(NULL==FileObject || NULL==fsi)
        return STATUS_INVALID_PARAMETER;

    RtlZeroMemory(fsi, sizeof(FILE_STANDARD_INFORMATION));
    return FltQueryInformationFile(Instance,
        FileObject,
        fsi, 
        sizeof(FILE_STANDARD_INFORMATION), 
        FileStandardInformation, 
        NULL);
}

NTSTATUS
FOH_GET_FILE_BASICINFO_BY_HANDLE(
                                 __in PFLT_INSTANCE Instance,
                                 __in_opt PFILE_OBJECT FileObject,
                                 __inout_opt PFILE_BASIC_INFORMATION fbi
                                 )
{
    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    // Sanity check
    if(NULL==FileObject || NULL==fbi)
        return STATUS_INVALID_PARAMETER;

    RtlZeroMemory(fbi, sizeof(FILE_BASIC_INFORMATION));
    return FltQueryInformationFile(Instance,
        FileObject,
        fbi, 
        sizeof(FILE_BASIC_INFORMATION), 
        FileBasicInformation, 
        NULL);
}

NTSTATUS
FOH_GET_FILE_POSITIONINFO_BY_HANDLE(
                                    __in PFLT_INSTANCE Instance,
                                    __in_opt PFILE_OBJECT FileObject,
                                    __inout_opt PFILE_POSITION_INFORMATION fpi
                                    )
{
    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    // Sanity check
    if(NULL==FileObject || NULL==fpi)
        return STATUS_INVALID_PARAMETER;

    RtlZeroMemory(fpi, sizeof(FILE_POSITION_INFORMATION));
    return FltQueryInformationFile(Instance,
        FileObject,
        fpi, 
        sizeof(FILE_POSITION_INFORMATION), 
        FilePositionInformation, 
        NULL);
}

NTSTATUS
FOH_SET_FILE_STANDARDINFO_BY_HANDLE(
                                    __in PFLT_INSTANCE Instance,
                                    __in_opt PFILE_OBJECT FileObject,
                                    __in_opt PFILE_STANDARD_INFORMATION fsi
                                    )
{
    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    // Sanity check
    if(NULL==FileObject || NULL==fsi)
        return STATUS_INVALID_PARAMETER;

    return FltSetInformationFile(Instance,
        FileObject,
        fsi, 
        sizeof(FILE_STANDARD_INFORMATION), 
        FileStandardInformation);
}

NTSTATUS
FOH_SET_FILE_BASICINFO_BY_HANDLE(
                                 __in PFLT_INSTANCE Instance,
                                 __in_opt PFILE_OBJECT FileObject,
                                 __in_opt PFILE_BASIC_INFORMATION fbi
                                 )
{
    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    // Sanity check
    if(NULL==FileObject || NULL==fbi)
        return STATUS_INVALID_PARAMETER;

    return FltSetInformationFile(Instance,
        FileObject,
        fbi, 
        sizeof(FILE_BASIC_INFORMATION), 
        FileBasicInformation);
}

NTSTATUS
FOH_SET_FILE_POSITIONINFO_BY_HANDLE(
                                    __in PFLT_INSTANCE Instance,
                                    __in_opt PFILE_OBJECT FileObject,
                                    __in LONGLONG CurrentOffset
                                    )
{
    FILE_POSITION_INFORMATION fpi = {0};

    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    // Sanity check
    if(NULL==FileObject)
        return STATUS_INVALID_PARAMETER;

    fpi.CurrentByteOffset.QuadPart = CurrentOffset;
    return FltSetInformationFile(Instance,
        FileObject,
        &fpi, 
        sizeof(FILE_POSITION_INFORMATION), 
        FilePositionInformation);
}

NTSTATUS
FOH_GET_FILE_STANDARDINFO_VIA_SYNCIO(__in PFLT_INSTANCE Instance,
                                     __in PFILE_OBJECT FileObject,
                                     __inout PFILE_STANDARD_INFORMATION fsi)
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  fltCD  = NULL;

    Status =FltAllocateCallbackData(Instance, FileObject, &fltCD);
    if(!NT_SUCCESS(Status))
        goto _exit;;

    // Fill IO Data
    fltCD->RequestorMode = KernelMode;
    fltCD->Iopb->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    fltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    fltCD->Iopb->Parameters.QueryFileInformation.FileInformationClass = FileStandardInformation;
    fltCD->Iopb->Parameters.QueryFileInformation.Length               = sizeof(FILE_STANDARD_INFORMATION);
    fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_STANDARD_INFORMATION),NLSE_FILEOPHELP_TAG);
    if(NULL == fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }

    // Perform the sync call
    FltPerformSynchronousIo(fltCD);
    Status = fltCD->IoStatus.Status;
    if(NT_SUCCESS(Status))
        RtlCopyMemory(fsi, fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer, sizeof(FILE_STANDARD_INFORMATION));

_exit:
    if(NULL!=fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer) ExFreePool(fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer);
    if(NULL!=fltCD) FltFreeCallbackData(fltCD);
    return Status;
}

NTSTATUS
FOH_GET_FILE_BASICINFO_VIA_SYNCIO(__in PFLT_INSTANCE Instance,
                                  __in PFILE_OBJECT FileObject,
                                  __inout PFILE_BASIC_INFORMATION fbi)
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  fltCD  = NULL;

    Status =FltAllocateCallbackData(Instance, FileObject, &fltCD);
    if(!NT_SUCCESS(Status))
        goto _exit;;

    // Fill IO Data
    fltCD->RequestorMode = KernelMode;
    fltCD->Iopb->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    fltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    fltCD->Iopb->Parameters.QueryFileInformation.FileInformationClass = FileBasicInformation;
    fltCD->Iopb->Parameters.QueryFileInformation.Length               = sizeof(FILE_BASIC_INFORMATION);
    fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_BASIC_INFORMATION),NLSE_FILEOPHELP_TAG);
    if(NULL == fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }

    // Perform the sync call
    FltPerformSynchronousIo(fltCD);
    Status = fltCD->IoStatus.Status;
    if(NT_SUCCESS(Status))
        RtlCopyMemory(fbi, fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer, sizeof(FILE_BASIC_INFORMATION));

_exit:
    if(NULL!=fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer) ExFreePool(fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer);
    if(NULL!=fltCD) FltFreeCallbackData(fltCD);
    return Status;
}

NTSTATUS
FOH_GET_FILE_POSITIONINFO_VIA_SYNCIO(__in PFLT_INSTANCE Instance,
                                     __in PFILE_OBJECT FileObject,
                                     __inout PFILE_POSITION_INFORMATION fpi)
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  fltCD  = NULL;

    Status =FltAllocateCallbackData(Instance, FileObject, &fltCD);
    if(!NT_SUCCESS(Status))
        goto _exit;;

    // Fill IO Data
    fltCD->RequestorMode = KernelMode;
    fltCD->Iopb->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    fltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    fltCD->Iopb->Parameters.QueryFileInformation.FileInformationClass = FilePositionInformation;
    fltCD->Iopb->Parameters.QueryFileInformation.Length               = sizeof(FILE_POSITION_INFORMATION);
    fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_POSITION_INFORMATION),NLSE_FILEOPHELP_TAG);
    if(NULL == fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }

    // Perform the sync call
    FltPerformSynchronousIo(fltCD);
    Status = fltCD->IoStatus.Status;
    if(NT_SUCCESS(Status))
        RtlCopyMemory(fpi, fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer, sizeof(FILE_POSITION_INFORMATION));

_exit:
    if(NULL!=fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer) ExFreePool(fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer);
    if(NULL!=fltCD) FltFreeCallbackData(fltCD);
    return Status;
}


NTSTATUS
FOH_SET_FILE_POSITIONINFO_VIA_SYNCIO(__in PFLT_INSTANCE Instance,
                                     __in PFILE_OBJECT FileObject,
                                     __in LONGLONG CurrentOffset)
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  fltCD  = NULL;

    Status =FltAllocateCallbackData(Instance, FileObject, &fltCD);
    if(!NT_SUCCESS(Status))
        goto _exit;;

    // Fill IO Data
    fltCD->RequestorMode = KernelMode;
    fltCD->Iopb->MajorFunction = IRP_MJ_SET_INFORMATION;
    fltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    fltCD->Iopb->Parameters.SetFileInformation.FileInformationClass   = FilePositionInformation;
    fltCD->Iopb->Parameters.SetFileInformation.Length                 = sizeof(FILE_POSITION_INFORMATION);
    fltCD->Iopb->Parameters.SetFileInformation.InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_POSITION_INFORMATION),NLSE_FILEOPHELP_TAG);
    if(NULL == fltCD->Iopb->Parameters.SetFileInformation.InfoBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    ((PFILE_POSITION_INFORMATION)fltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer)->CurrentByteOffset.QuadPart = CurrentOffset;

    // Perform the sync call
    FltPerformSynchronousIo(fltCD);
    Status = fltCD->IoStatus.Status;

    ExFreePool(fltCD->Iopb->Parameters.SetFileInformation.InfoBuffer);

_exit:
    if(NULL!=fltCD) FltFreeCallbackData(fltCD);
    return Status;
}

NTSTATUS
FOH_GET_FILE_SIZE_BY_NAME(__in PFLT_FILTER Filter,
                          __in PFLT_INSTANCE Instance,
                          __in PUNICODE_STRING FileName,
                          __inout PLARGE_INTEGER FileSize)
{
    FILE_STANDARD_INFORMATION fsi = {0};
    NTSTATUS Status = STATUS_SUCCESS;

    Status = FOH_GET_FILE_STANDARDINFO_BY_NAME(Filter, Instance, FileName, &fsi);
    if(NT_SUCCESS(Status))
        FileSize->QuadPart = fsi.EndOfFile.QuadPart;

    return Status;
}

NTSTATUS
FOH_GET_FILE_SIZE_BY_HANDLE(__in PFLT_INSTANCE Instance,
                            __in PFILE_OBJECT FileObject,
                            __inout PLARGE_INTEGER FileSize)
{
    FILE_STANDARD_INFORMATION fsi = {0};
    NTSTATUS Status = STATUS_SUCCESS;

    Status = FOH_GET_FILE_STANDARDINFO_BY_HANDLE(Instance, FileObject, &fsi);
    if(NT_SUCCESS(Status))
        FileSize->QuadPart = fsi.EndOfFile.QuadPart;

    return Status;
}


NTSTATUS
FOH_GET_FILE_SIZE_VIA_SYNCIO(__in PFLT_INSTANCE Instance,
                             __in PFILE_OBJECT FileObject,
                             __inout PLARGE_INTEGER FileSize)
{
    FILE_STANDARD_INFORMATION fsi = {0};
    NTSTATUS Status = STATUS_SUCCESS;

    Status = FOH_GET_FILE_STANDARDINFO_VIA_SYNCIO(Instance, FileObject, &fsi);
    if(NT_SUCCESS(Status))
        FileSize->QuadPart = fsi.EndOfFile.QuadPart;

    return Status;
}

NTSTATUS
FOH_REMOVE_READONLY_ATTRIBUTES(__in PFLT_FILTER Filter,
                               __in PFLT_INSTANCE Instance,
                               __in PUNICODE_STRING FileName)
{
    NTSTATUS                Status = STATUS_SUCCESS;
    FILE_BASIC_INFORMATION  fbi = {0};

    Status = FOH_GET_FILE_BASICINFO_BY_NAME(Filter, Instance, FileName, &fbi);
    if(!NT_SUCCESS(Status))
        return Status;

    if(fbi.FileAttributes&FILE_ATTRIBUTE_READONLY)
    {
        fbi.FileAttributes &= (~FILE_ATTRIBUTE_READONLY);
        Status = FOH_SET_FILE_BASICINFO_BY_NAME(Filter, Instance, FileName, &fbi);
    }

    return Status;
}

NTSTATUS
FOH_IS_READ_ONLY(__in PFLT_FILTER Filter,
                 __in PFLT_INSTANCE Instance,
                 __in PUNICODE_STRING FileName,
                 __inout PBOOLEAN ReadOnly)
{
    NTSTATUS                Status = STATUS_SUCCESS;
    FILE_BASIC_INFORMATION  fbi = {0};

    *ReadOnly = FALSE;
    Status = FOH_GET_FILE_BASICINFO_BY_NAME(Filter, Instance, FileName, &fbi);
    if(!NT_SUCCESS(Status))
        return Status;

    if(fbi.FileAttributes&FILE_ATTRIBUTE_READONLY)
        *ReadOnly = TRUE;

    return Status;
}

NTSTATUS
FOH_IS_DELETE_PENDING(__in PFLT_FILTER Filter,
                      __in PFLT_INSTANCE Instance,
                      __in PUNICODE_STRING FileName,
                      __inout PBOOLEAN DeletePending)
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    FILE_STANDARD_INFORMATION   fsi = {0};

    Status = FOH_GET_FILE_STANDARDINFO_BY_NAME(Filter, Instance, FileName, &fsi);
    if(!NT_SUCCESS(Status))
        return Status;

    *DeletePending = fsi.DeletePending;

    return Status;
}

NTSTATUS
FOH_SYNC_WRITE(
               __in PFLT_INSTANCE Instance,
               __in PFILE_OBJECT  FileObject,
               __in PLARGE_INTEGER  ByteOffset,
               __in ULONG  Length,
               __in PVOID  Buffer,
               __in ULONG  IrpFlags,
               __out_opt PULONG  BytesWritten
               )
{
    NTSTATUS            Status    = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  writeData = NULL;

    Status =FltAllocateCallbackData(Instance, FileObject, &writeData);
    if(!NT_SUCCESS(Status))
        return Status;

    writeData->RequestorMode = KernelMode;
    writeData->Iopb->MajorFunction = IRP_MJ_WRITE;
    writeData->Iopb->MinorFunction = IRP_MN_NORMAL;
    writeData->Iopb->IrpFlags = IrpFlags;
    writeData->Iopb->Parameters.Write.ByteOffset.QuadPart = ByteOffset->QuadPart;
    writeData->Iopb->Parameters.Write.Length              = Length;
    writeData->Iopb->Parameters.Write.MdlAddress          = NULL;
    writeData->Iopb->Parameters.Write.WriteBuffer         = Buffer;

    FltPerformSynchronousIo(writeData);
    Status=writeData->IoStatus.Status;
    if(NULL != BytesWritten)
        *BytesWritten = NT_SUCCESS(Status)?((ULONG)writeData->IoStatus.Information):0;

    FltFreeCallbackData(writeData);
    return Status;
}

NTSTATUS
FOH_SYNC_READ(
              __in PFLT_INSTANCE Instance,
              __in PFILE_OBJECT  FileObject,
              __in PLARGE_INTEGER  ByteOffset,
              __in ULONG  Length,
              __in PVOID  Buffer,
              __in ULONG  IrpFlags,
              __out_opt PULONG  BytesRead
              )
{
    NTSTATUS            Status    = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  readData  = NULL;

    Status =FltAllocateCallbackData(Instance, FileObject, &readData);
    if(!NT_SUCCESS(Status))
        return Status;

    readData->RequestorMode = KernelMode;
    readData->Iopb->MajorFunction = IRP_MJ_READ;
    readData->Iopb->MinorFunction = IRP_MN_NORMAL;
    readData->Iopb->IrpFlags = IrpFlags;
    readData->Iopb->Parameters.Read.ByteOffset.QuadPart = ByteOffset->QuadPart;
    readData->Iopb->Parameters.Read.Length              = Length;
    readData->Iopb->Parameters.Read.MdlAddress          = NULL;
    readData->Iopb->Parameters.Read.ReadBuffer          = Buffer;

    FltPerformSynchronousIo(readData);
    Status=readData->IoStatus.Status;
    if(NULL != BytesRead)
        *BytesRead = NT_SUCCESS(Status)?((ULONG)readData->IoStatus.Information):0;

    FltFreeCallbackData(readData);
    return Status;
}
