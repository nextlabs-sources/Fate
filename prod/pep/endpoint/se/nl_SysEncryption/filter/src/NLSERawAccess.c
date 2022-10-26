/*++

Module Name:

    NLSERawAccess.c

Abstract:
    Functions used by the kernel mode filter driver implementing NLSE raw
    mode file accesses.

Environment:
    Kernel mode

--*/
#include <limits.h>
#include "NLSERawAccess.h"

//Global variables
extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG nlseKLog;



//
// Internal functions
//

//Find the filter instance for the passed full file path, which might or might
//not exist.
//The caller needs to remove the reference from the filter instance when done.
static NTSTATUS GetInstanceFromFilePath(__in_z const WCHAR *fname,
                                        __out PFLT_INSTANCE *pInstance)
{
  UNICODE_STRING fnameStr;
  PFLT_VOLUME volume;
  NTSTATUS status;

  *pInstance = NULL;

  // Set up a Unicode string that only contains the drive specifier.
  RtlInitUnicodeString(&fnameStr, fname);
  fnameStr.Length = 2 * sizeof(WCHAR);
  status = FltGetVolumeFromName(nlfseGlobal.filterHandle, &fnameStr, &volume);

  if (NT_SUCCESS(status))
  {
    status = FltGetVolumeInstanceFromName(nlfseGlobal.filterHandle, volume,
                                          NULL, pInstance);
    if (!NT_SUCCESS(status))
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                  "NLSE!GetInstanceFromFilePath: can't get instance from volume for %ws\n",
                  fname);
    }

    FltObjectDereference(volume);
  }
  else
  {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                "NLSE!GetInstanceFromFilePath: can't get volume for %ws\n",
                fname);
  }

  return status;
}

//Find the filter instance for the passed file object.
//The caller needs to remove the reference from the filter instance when done.
static NTSTATUS GetInstanceFromFileObject(__in PFILE_OBJECT fileObj,
                                          __out PFLT_INSTANCE *pInstance)
{
  PFLT_VOLUME volume;
  NTSTATUS status;

  *pInstance = NULL;

  status = FltGetVolumeFromFileObject(nlfseGlobal.filterHandle, fileObj,
                                      &volume);

  if (NT_SUCCESS(status))
  {
    status = FltGetVolumeInstanceFromName(nlfseGlobal.filterHandle, volume,
                                          NULL, pInstance);
    if (!NT_SUCCESS(status))
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                  "NLSE!GetInstanceFromFileObject: can't get instance from volume\n");
    }

    FltObjectDereference(volume);
  }
  else
  {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                "NLSE!GetInstanceFromFileObject: can't get volume\n");
  }

  return status;
}

//Find the volume properties for the passed file object.
static NTSTATUS GetVolumePropertiesFromFileObject(__in PFILE_OBJECT fileObj,
                                                  __out ULONG *pAlignmentReq,
                                                  __out USHORT *pSectorSize)
{
  PFLT_VOLUME volume;
  NTSTATUS status;

  status = FltGetVolumeFromFileObject(nlfseGlobal.filterHandle, fileObj,
                                      &volume);
  if (NT_SUCCESS(status))
  {
    FLT_VOLUME_PROPERTIES properties;
    ULONG lenReturned;

    status = FltGetVolumeProperties(volume, &properties, sizeof properties,
                                    &lenReturned);
    if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW)
    {
      *pAlignmentReq = properties.AlignmentRequirement;
      *pSectorSize = properties.SectorSize;
      status = STATUS_SUCCESS;
    }

    FltObjectDereference(volume);
  }
  else
  {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                "NLSE!GetVolumePropertiesFromFileObject: can't get volume\n");
  }

  return status;
}

//Align the offset and length to sector boundaries.
//Returns STATUS_INTEGER_OVERFLOW if the aligned length is too large.
static NTSTATUS alignOffsetAndLengthToSector(__in ULONGLONG offset,
                                             __in ULONG len,
                                             __in USHORT sectorSize,
                                             __out ULONGLONG *pAlignedOffset,
                                             __out ULONG *pAlignedLen)
{
  ULONGLONG sectorOffsetLastByte;

  *pAlignedOffset = (offset / sectorSize) * sectorSize;
  sectorOffsetLastByte = ((offset + len - 1) / sectorSize) * sectorSize;

  if (sectorOffsetLastByte - *pAlignedOffset <= ULONG_MAX - sectorSize)
  {
    *pAlignedLen = (ULONG)
      (sectorOffsetLastByte - *pAlignedOffset + sectorSize);
    return STATUS_SUCCESS;
  }
  else
  {
    return STATUS_INTEGER_OVERFLOW;
  }
}



//
// Exported functions
//

//Create or open a file in raw mode
VOID NLSECreateFileRaw(__in const PNLSE_MESSAGE msg)
{
  int i;
  PFLT_INSTANCE                 instance = NULL;
  WCHAR                         fNameBuf[NLFSE_NAME_LEN] = L"";
  UNICODE_STRING                fNameU;
  OBJECT_ATTRIBUTES             objAttr;
  IO_STATUS_BLOCK               ioStatusBlock;
  NTSTATUS status;

  NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
             "NLSE!NLSECreateFileRaw: fname=%ws, desiredAccess=0x%08X, fileAttributes=0x%08X, shareAccess=0x%08X, createDisposition=0x%08X\n",
             msg->fname,
             msg->params.createFileRaw.desiredAccess,
             msg->params.createFileRaw.fileAttributes,
             msg->params.createFileRaw.shareAccess,
             msg->params.createFileRaw.createDisposition);

  // Return error if the passed file path does not contain drive specifier.
  if (msg->fname[0] == L'\0' || msg->fname[1] != L':')
  {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                "NLSE!NLSECreateFileRaw: \"%ws\" does not contain drive specifier\n",
                msg->fname);
    status = STATUS_OBJECT_PATH_INVALID;
    goto cleanup;
  }

  status = GetInstanceFromFilePath(msg->fname, &instance);
  if (!NT_SUCCESS(status))
  {
    goto cleanup;
  }

  // Prepend "\??\" to the file name
  fNameU.Buffer = fNameBuf;
  fNameU.Length = 0;
  fNameU.MaximumLength = sizeof fNameBuf;
  RtlAppendUnicodeToString(&fNameU, L"\\??\\");
  RtlAppendUnicodeToString(&fNameU, msg->fname);

  InitializeObjectAttributes(&objAttr,
                             &fNameU,
                             OBJ_KERNEL_HANDLE,
                             NULL,
                             NULL);

  // Add FILE_READ_DATA if FILE_WRITE_DATA or FILE_APPEND_DATA are passed,
  // since we may need to first read data from file when the user does
  // un-sector-aligned write operation.
  if (msg->params.createFileRaw.desiredAccess &
      (FILE_WRITE_DATA | FILE_APPEND_DATA))
  {
    msg->params.createFileRaw.desiredAccess |= FILE_READ_DATA;
  }

  status = FltCreateFile(nlfseGlobal.filterHandle,
                         instance,
                         msg->params.createFileRaw.pHandle,
                         msg->params.createFileRaw.desiredAccess,
                         &objAttr,
                         &ioStatusBlock,
                         0,
                         msg->params.createFileRaw.fileAttributes,
                         msg->params.createFileRaw.shareAccess,
                         msg->params.createFileRaw.createDisposition,
                         (FILE_NON_DIRECTORY_FILE |
                          FILE_NO_INTERMEDIATE_BUFFERING),
                         NULL,
                         0,
                         0);
  NL_KLOG_Log(&nlseKLog,
              (NT_SUCCESS(status) || (0xC00000BA==status)) ? NL_KLOG_LEVEL_DEBUG : NL_KLOG_LEVEL_ERR,
              "NLSE!NLSECreateFileRaw: FltCreateFile returns 0x%08lX, handle=0x%08X\n    FileName: %wZ\n",
             status, *msg->params.createFileRaw.pHandle, &fNameU);

cleanup:
  if (instance != NULL)
  {
    FltObjectDereference(instance);
  }

  *msg->params.createFileRaw.status = status;
}

//Read a file in raw mode
VOID NLSEReadFileRaw(__in const PNLSE_MESSAGE msg)
{
  NTSTATUS status;
  LARGE_INTEGER byteOffset;
  PFILE_OBJECT fileObj = NULL;
  PFLT_INSTANCE instance = NULL;
  ULONG alignmentReq;
  unsigned char *alignedBuf = NULL;
  USHORT sectorSize;

  NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
              "NLSE!NLSEReadFileRaw: offset=0x%I64X, len=0x%X\n",
              msg->params.readFileRaw.offset, msg->params.readFileRaw.len);

  if (msg->params.readFileRaw.bufSize < msg->params.readFileRaw.len)
  {
    *msg->params.readFileRaw.status = STATUS_BUFFER_TOO_SMALL;
    return;
  }

  status = ObReferenceObjectByHandle(msg->params.readFileRaw.handle,//Handle
                                     0,            //DesiredAccess
                                     *IoFileObjectType, //ObjectType
                                     KernelMode,   //AccessMode
                                     &fileObj,  //File Handle
                                     NULL);
  if (!NT_SUCCESS(status))
  {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                "NLSE!NLSEReadFileRaw: ObReferenceObjectByHandle returns 0x%08lX\n",
                status);
    goto cleanup;
  }

  status = GetInstanceFromFileObject(fileObj, &instance);
  if (!NT_SUCCESS(status))
  {
    goto cleanup;
  }

  status = GetVolumePropertiesFromFileObject(fileObj, &alignmentReq,
                                             &sectorSize);
  if (!NT_SUCCESS(status))
  {
    goto cleanup;
  }

  // See if the read request satisfies the restrictions for noncached I/O.
  if ((((ULONG_PTR) msg->params.readFileRaw.buf) & alignmentReq) == 0 &&
      msg->params.readFileRaw.offset % sectorSize == 0 &&
      msg->params.readFileRaw.len % sectorSize == 0)
  {
    // The request satisfies the restrictions for noncached I/O.  We can use
    // the passed buffer directly.

    // No need to do try/except and ProbeForWrite() to valide the user buffer,
    // since FltReadFile() gracefully returns STATUS_INVALID_USER_BUFFER when
    // the user buffer is invalid.
    byteOffset.QuadPart = msg->params.readFileRaw.offset;
    status = FltReadFile(instance,
                         fileObj,
                         &byteOffset,
                         msg->params.readFileRaw.len,
                         msg->params.readFileRaw.buf,
                         FLTFL_IO_OPERATION_NON_CACHED,
                         msg->params.readFileRaw.bytesRead,
                         NULL,
                         NULL);
    if (!NT_SUCCESS(status))
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                  "NLSE!NLSEReadFileRaw: FltReadFile returns 0x%08lX\n",
                  status);
    }
  }
  else
  {
    // The request does not satisfy the restrictions for noncached I/O.  We
    // have to allocate a temp buffer to perform the noncached I/O.
    ULONGLONG alignedOffset;
    ULONG alignedLen;
    ULONG alignedBytesRead;

    // Calculate the sector-aligned offset and length that covers the requested
    // data.
    status = alignOffsetAndLengthToSector(msg->params.readFileRaw.offset,
                                          msg->params.readFileRaw.len,
                                          sectorSize,
                                          &alignedOffset, &alignedLen);
    if (!NT_SUCCESS(status))
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                  "NLSE!NLSEReadFileRaw: Cannot align to sector\n");
      goto cleanup;
    }

    // Allocate the sector-aligned temp buffer.
    alignedBuf = FltAllocatePoolAlignedWithTag(instance, PagedPool, alignedLen,
                                               NLSE_RAW_ACCESS_TAG);
    if (alignedBuf == NULL)
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                  "NLSE!NLSEReadFileRaw: Cannot allocated aligned buffer\n");
      status = STATUS_NO_MEMORY;
      goto cleanup;
    }

    // Read into the sector-aligned temp buffer.
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
                "NLSE!NLSEReadFileRaw: Doing sector-aligned read, alignedOffset=0x%I64X, alignedLen=0x%X\n",
                alignedOffset, alignedLen);
    byteOffset.QuadPart = alignedOffset;
    status = FltReadFile(instance,
                         fileObj,
                         &byteOffset,
                         alignedLen,
                         alignedBuf,
                         FLTFL_IO_OPERATION_NON_CACHED,
                         &alignedBytesRead,
                         NULL,
                         NULL);
    if (!NT_SUCCESS(status))
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                  "NLSE!NLSEReadFileRaw: FltReadFile returns 0x%08lX\n",
                  status);
      goto cleanup;
    }

    // Calculate # of requested bytes that are read.
    if (alignedBytesRead <= msg->params.readFileRaw.offset - alignedOffset)
    {
      *msg->params.readFileRaw.bytesRead = 0;
      status = STATUS_END_OF_FILE;
      goto cleanup;
    }

    *msg->params.readFileRaw.bytesRead = (ULONG)
      min(alignedBytesRead - (msg->params.readFileRaw.offset - alignedOffset),
          msg->params.readFileRaw.len);

    // Copy the data from the sector-aligned temp buffer to the user buffer.
    try
    {
      ProbeForWrite(msg->params.readFileRaw.buf,
                    msg->params.readFileRaw.len,
                    1);
      RtlCopyMemory(msg->params.readFileRaw.buf,
                    alignedBuf + (msg->params.readFileRaw.offset -
                                  alignedOffset),
                    *msg->params.readFileRaw.bytesRead);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
      status = STATUS_INVALID_USER_BUFFER;
    }
  }

cleanup:
  if (alignedBuf != NULL)
  {
    FltFreePoolAlignedWithTag(instance, alignedBuf, NLSE_RAW_ACCESS_TAG);
  }

  if (instance != NULL)
  {
    FltObjectDereference(instance);
  }

  if (fileObj != NULL)
  {
    ObDereferenceObject(fileObj);
  }

  *msg->params.readFileRaw.status = status;
}

//Write to a file in raw mode
VOID NLSEWriteFileRaw(__in const PNLSE_MESSAGE msg)
{
  NTSTATUS status;
  LARGE_INTEGER byteOffset;
  PFILE_OBJECT fileObj = NULL;
  PFLT_INSTANCE instance = NULL;
  ULONG alignmentReq;
  unsigned char *alignedBuf = NULL;
  USHORT sectorSize;

  if (msg->params.writeFileRaw.bufSize < msg->params.writeFileRaw.len)
  {
    *msg->params.writeFileRaw.status = STATUS_BUFFER_TOO_SMALL;
    return;
  }

  status = ObReferenceObjectByHandle(msg->params.writeFileRaw.handle,//Handle
                                     0,            //DesiredAccess
                                     *IoFileObjectType, //ObjectType
                                     KernelMode,   //AccessMode
                                     &fileObj,  //File Handle
                                     NULL);
  if (!NT_SUCCESS(status))
  {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                "NLSE!NLSEWriteFileRaw: ObReferenceObjectByHandle returns 0x%08lX\n",
                status);
    goto cleanup;
  }

  status = GetInstanceFromFileObject(fileObj, &instance);
  if (!NT_SUCCESS(status))
  {
    goto cleanup;
  }

  status = GetVolumePropertiesFromFileObject(fileObj, &alignmentReq,
                                             &sectorSize);
  if (!NT_SUCCESS(status))
  {
    goto cleanup;
  }

  // See if the write request satisfies the restrictions for noncached I/O.
  if ((((ULONG_PTR) msg->params.writeFileRaw.buf) & alignmentReq) == 0 &&
      msg->params.writeFileRaw.offset % sectorSize == 0 &&
      msg->params.writeFileRaw.len % sectorSize == 0)
  {
    // The request satisfies the restrictions for noncached I/O.  We can use
    // the passed buffer directly.


    // No need to do try/except and ProbeForRead() to valide the user buffer,
    // since FltWriteFile() gracefully returns STATUS_INVALID_USER_BUFFER when
    // the user buffer is invalid.
    byteOffset.QuadPart = msg->params.writeFileRaw.offset;
    status = FltWriteFile(instance,
                          fileObj,
                          &byteOffset,
                          msg->params.writeFileRaw.len,
                          msg->params.writeFileRaw.buf,
                          FLTFL_IO_OPERATION_NON_CACHED,
                          msg->params.writeFileRaw.bytesWritten,
                          NULL,
                          NULL);
    if (!NT_SUCCESS(status))
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                  "NLSE!NLSEWriteFileRaw: FltWriteFile returns 0x%08lX\n",
                  status);
    }
  }
  else
  {
    // The request does not satisfy the restrictions for noncached I/O.  We
    // have to allocate a temp buffer to perform the noncached I/O.
    FILE_STANDARD_INFORMATION stdInfo;
    ULONGLONG origFileSize, newFileSize;
    ULONGLONG alignedOffset;
    ULONG alignedLen;
    ULONG alignedBytesRead, alignedBytesWritten;
    BOOLEAN firstSectorAlreadyRead = FALSE;

    // Get the origial file size, so that we can truncate the file if needed
    // after doing sector-aligned writes.
    status = FltQueryInformationFile(instance, fileObj, &stdInfo,
                                     sizeof stdInfo, FileStandardInformation,
                                     NULL);
    if (!NT_SUCCESS(status))
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                  "NLSE!NLSEWriteFileRaw: Cannot get file size\n");
      goto cleanup;
    }

    origFileSize = stdInfo.EndOfFile.QuadPart;

    // Calculate the sector-aligned offset and length that covers the requested
    // data.
    status = alignOffsetAndLengthToSector(msg->params.writeFileRaw.offset,
                                          msg->params.writeFileRaw.len,
                                          sectorSize,
                                          &alignedOffset, &alignedLen);
    if (!NT_SUCCESS(status))
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                  "NLSE!NLSEWriteFileRaw: Cannot align to sector\n");
      goto cleanup;
    }

    // Allocate the sector-aligned temp buffer.
    alignedBuf = FltAllocatePoolAlignedWithTag(instance, PagedPool, alignedLen,
                                               NLSE_RAW_ACCESS_TAG);
    if (alignedBuf == NULL)
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                  "NLSE!NLSEWriteFileRaw: Cannot allocated aligned buffer\n");
      status = STATUS_NO_MEMORY;
      goto cleanup;
    }

    // See if we need to read in the first sector.
    if (alignedOffset < msg->params.writeFileRaw.offset)
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
                  "NLSE!NLSEWriteFileRaw: Reading in first sector, alignedOffset=0x%I64X\n",
                  alignedOffset);
      byteOffset.QuadPart = alignedOffset;
      status = FltReadFile(instance,
                           fileObj,
                           &byteOffset,
                           sectorSize,
                           alignedBuf,
                           FLTFL_IO_OPERATION_NON_CACHED,
                           &alignedBytesRead,
                           NULL,
                           NULL);

      // Ignore any EOF, since there might currently be no data at these
      // locations in the file.
      if (!NT_SUCCESS(status) && status != STATUS_END_OF_FILE)
      {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                    "NLSE!NLSEWriteFileRaw: FltReadFile returns 0x%08lX\n",
                    status);
        goto cleanup;
      }

      firstSectorAlreadyRead = TRUE;
    }

    // See if we need to read in the last sector.
    if (!(firstSectorAlreadyRead && alignedLen == sectorSize) &&
        (alignedOffset + alignedLen >
         msg->params.writeFileRaw.offset + msg->params.writeFileRaw.len))
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
                  "NLSE!NLSEWriteFileRaw: Reading in last sector, alignedOffset=0x%I64X\n",
                  alignedOffset + alignedLen - sectorSize);
      byteOffset.QuadPart = alignedOffset + alignedLen - sectorSize;
      status = FltReadFile(instance,
                           fileObj,
                           &byteOffset,
                           sectorSize,
                           alignedBuf + alignedLen - sectorSize,
                           FLTFL_IO_OPERATION_NON_CACHED,
                           &alignedBytesRead,
                           NULL,
                           NULL);

      // Ignore any EOF, since there might currently be no data at these
      // locations in the file.
      if (!NT_SUCCESS(status) && status != STATUS_END_OF_FILE)
      {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                    "NLSE!NLSEWriteFileRaw: FltReadFile returns 0x%08lX\n",
                    status);
        goto cleanup;
      }
    }

    // Copy the data from the user buffer to the sector-aligned temp buffer.
    try
    {
      ProbeForRead(msg->params.writeFileRaw.buf,
                   msg->params.writeFileRaw.len,
                    1);
      RtlCopyMemory(alignedBuf + (msg->params.writeFileRaw.offset -
                                  alignedOffset),
                    msg->params.writeFileRaw.buf,
                    msg->params.writeFileRaw.len);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
      status = STATUS_INVALID_USER_BUFFER;
    }

    // Write the sector-aligned temp buffer out to the file.
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
                "NLSE!NLSEWriteFileRaw: Doing sector-aligned write, alignedOffset=0x%I64X, alignedLen=0x%X\n",
                alignedOffset, alignedLen);
    byteOffset.QuadPart = alignedOffset;
    status = FltWriteFile(instance,
                          fileObj,
                          &byteOffset,
                          alignedLen,
                          alignedBuf,
                          FLTFL_IO_OPERATION_NON_CACHED,
                          &alignedBytesWritten,
                          NULL,
                          NULL);
    if (!NT_SUCCESS(status))
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                  "NLSE!NLSEWriteFileRaw: FltWriteFile returns 0x%08lX\n",
                  status);
      goto cleanup;
    }

    // Calculate # of requested bytes that are written.
    if (alignedBytesWritten <= msg->params.writeFileRaw.offset - alignedOffset)
    {
      *msg->params.writeFileRaw.bytesWritten = 0;
      status = STATUS_UNEXPECTED_IO_ERROR;
      goto cleanup;
    }

    *msg->params.writeFileRaw.bytesWritten = (LONG)
      min(alignedBytesWritten - (msg->params.writeFileRaw.offset -
                                 alignedOffset),
          msg->params.writeFileRaw.len);

    // See if we need to truncate the file to reflect the real EOF.
    newFileSize = max(origFileSize,
                      (msg->params.writeFileRaw.offset +
                       msg->params.writeFileRaw.len));
    if (newFileSize < alignedOffset + alignedLen)
    {
      FILE_END_OF_FILE_INFORMATION eofInfo;

      eofInfo.EndOfFile.QuadPart = newFileSize;
      status = FltSetInformationFile(instance, fileObj, &eofInfo,
                                     sizeof eofInfo, FileEndOfFileInformation);
      if (!NT_SUCCESS(status))
      {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                    "NLSE!NLSEWriteFileRaw: FltSetInformation returns 0x%08lX\n",
                    status);
        goto cleanup;
      }
    }
  }

cleanup:
  if (alignedBuf != NULL)
  {
    FltFreePoolAlignedWithTag(instance, alignedBuf, NLSE_RAW_ACCESS_TAG);
  }

  if (instance != NULL)
  {
    FltObjectDereference(instance);
  }

  if (fileObj != NULL)
  {
    ObDereferenceObject(fileObj);
  }

  *msg->params.writeFileRaw.status = status;
}

//Close a file opened in raw mode
VOID NLSECloseFileRaw(__in const PNLSE_MESSAGE msg)
{
  NTSTATUS status;
  PFILE_OBJECT fileObj;

  // FltClose() crashes if the handle is invalid.  So we have to use
  // ObReferenceObjectByHandle() to validate the handle first, even though we
  // don't actually need the object.
  status = ObReferenceObjectByHandle(msg->params.closeFileRaw.handle,//Handle
                                     0,            //DesiredAccess
                                     *IoFileObjectType, //ObjectType
                                     KernelMode,   //AccessMode
                                     &fileObj,  //File Handle
                                     NULL);
  if (!NT_SUCCESS(status))
  {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                "NLSE!NLSECloseFileRaw: ObReferenceObjectByHandle returns 0x%08lX\n",
                status);
    *msg->params.closeFileRaw.status = status;
    return;
  }
  ObDereferenceObject(fileObj);

  // Close the file.
  status = FltClose(msg->params.closeFileRaw.handle);
  NL_KLOG_Log(&nlseKLog,
              NT_SUCCESS(status) ? NL_KLOG_LEVEL_DEBUG : NL_KLOG_LEVEL_ERR,
             "NLSE!NLSECloseFileRaw: FltClose returns 0x%08lX, handle=0x%08X\n",
             status, msg->params.closeFileRaw.handle);

  *msg->params.closeFileRaw.status = status;
}
