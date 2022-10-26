
#pragma once
#ifndef _FILE_OPERATION_HELP_H_
#define _FILE_OPERATION_HELP_H_
#include <fltKernel.h>

NTSTATUS
FOH_GET_FILE_STANDARDINFO_BY_NAME(
                                  __in PFLT_FILTER Filter,
                                  __in PFLT_INSTANCE Instance,
                                  __in_opt PUNICODE_STRING FileName,
                                  __inout_opt PFILE_STANDARD_INFORMATION fsi
                                    );

NTSTATUS
FOH_GET_FILE_BASICINFO_BY_NAME(
                               __in PFLT_FILTER Filter,
                               __in PFLT_INSTANCE Instance,
                               __in PUNICODE_STRING FileName,
                               __inout PFILE_BASIC_INFORMATION fbi
                               );

NTSTATUS
FOH_GET_FILE_POSITIONINFO_BY_NAME(
                                  __in PFLT_FILTER Filter,
                                  __in PFLT_INSTANCE Instance,
                                  __in PUNICODE_STRING FileName,
                                  __inout_opt PFILE_POSITION_INFORMATION fpi
                                  );

NTSTATUS
FOH_SET_FILE_STANDARDINFO_BY_NAME(
                                  __in PFLT_FILTER Filter,
                                  __in PFLT_INSTANCE Instance,
                                  __in_opt PUNICODE_STRING FileName,
                                  __in_opt PFILE_STANDARD_INFORMATION fsi
                                  );

NTSTATUS
FOH_SET_FILE_BASICINFO_BY_NAME(
                               __in PFLT_FILTER Filter,
                               __in PFLT_INSTANCE Instance,
                               __in_opt PUNICODE_STRING FileName,
                               __in_opt PFILE_BASIC_INFORMATION fbi
                               );

NTSTATUS
FOH_GET_FILE_STANDARDINFO_VIA_SYNCIO(__in PFLT_INSTANCE Instance,
                                     __in PFILE_OBJECT FileObject,
                                     __inout PFILE_STANDARD_INFORMATION fsi);

NTSTATUS
FOH_GET_FILE_BASICINFO_VIA_SYNCIO(__in PFLT_INSTANCE Instance,
                                  __in PFILE_OBJECT FileObject,
                                  __inout PFILE_BASIC_INFORMATION fbi);

NTSTATUS
FOH_GET_FILE_POSITIONINFO_VIA_SYNCIO(__in PFLT_INSTANCE Instance,
                                     __in PFILE_OBJECT FileObject,
                                     __inout PFILE_POSITION_INFORMATION fpi);

NTSTATUS
FOH_GET_FILE_STANDARDINFO_BY_HANDLE(
                                    __in PFLT_INSTANCE Instance,
                                    __in PFILE_OBJECT FileObject,
                                    __inout_opt PFILE_STANDARD_INFORMATION fpi
                                    );

NTSTATUS
FOH_GET_FILE_BASICINFO_BY_HANDLE(
                                 __in PFLT_INSTANCE Instance,
                                 __in_opt PFILE_OBJECT FileObject,
                                 __inout_opt PFILE_BASIC_INFORMATION fbi
                                 );

NTSTATUS
FOH_GET_FILE_POSITIONINFO_BY_HANDLE(
                                    __in PFLT_INSTANCE Instance,
                                    __in_opt PFILE_OBJECT FileObject,
                                    __inout_opt PFILE_POSITION_INFORMATION fpi
                                    );


NTSTATUS
FOH_SET_FILE_STANDARDINFO_BY_HANDLE(
                                    __in PFLT_INSTANCE Instance,
                                    __in_opt PFILE_OBJECT FileObject,
                                    __in_opt PFILE_STANDARD_INFORMATION fsi
                                    );

NTSTATUS
FOH_SET_FILE_BASICINFO_BY_HANDLE(
                                 __in PFLT_INSTANCE Instance,
                                 __in_opt PFILE_OBJECT FileObject,
                                 __in_opt PFILE_BASIC_INFORMATION fbi
                                 );

NTSTATUS
FOH_SET_FILE_POSITIONINFO_BY_HANDLE(
                                    __in PFLT_INSTANCE Instance,
                                    __in_opt PFILE_OBJECT FileObject,
                                    __in LONGLONG CurrentOffset
                                    );

NTSTATUS
FOH_SET_FILE_POSITIONINFO_VIA_SYNCIO(__in PFLT_INSTANCE Instance,
                                     __in PFILE_OBJECT FileObject,
                                     __in LONGLONG CurrentOffset);

NTSTATUS
FOH_GET_FILE_SIZE_BY_NAME(__in PFLT_FILTER Filter,
                          __in PFLT_INSTANCE Instance,
                          __in PUNICODE_STRING FileName,
                          __inout PLARGE_INTEGER FileSize);

NTSTATUS
FOH_GET_FILE_SIZE_BY_HANDLE(__in PFLT_INSTANCE Instance,
                            __in PFILE_OBJECT FileObject,
                            __inout PLARGE_INTEGER FileSize);

NTSTATUS
FOH_GET_FILE_SIZE_VIA_SYNCIO(__in PFLT_INSTANCE Instance,
                             __in PFILE_OBJECT FileObject,
                             __inout PLARGE_INTEGER FileSize);

NTSTATUS
FOH_REMOVE_READONLY_ATTRIBUTES(__in PFLT_FILTER Filter,
                               __in PFLT_INSTANCE Instance,
                               __in PUNICODE_STRING FileName);

NTSTATUS
FOH_IS_DELETE_PENDING(__in PFLT_FILTER Filter,
                      __in PFLT_INSTANCE Instance,
                      __in PUNICODE_STRING FileName,
                      __inout PBOOLEAN DeletePending);

NTSTATUS
FOH_IS_READ_ONLY(__in PFLT_FILTER Filter,
                 __in PFLT_INSTANCE Instance,
                 __in PUNICODE_STRING FileName,
                 __inout PBOOLEAN ReadOnly);

/*IRP_LEVEL <= APC_LEVEL*/
NTSTATUS
FOH_SYNC_WRITE(
               __in PFLT_INSTANCE Instance,
               __in PFILE_OBJECT  FileObject,
               __in PLARGE_INTEGER  ByteOffset,
               __in ULONG  Length,
               __in PVOID  Buffer,
               __in ULONG  IrpFlags,
               __out_opt PULONG  BytesWritten
               );

/*IRP_LEVEL <= APC_LEVEL*/
NTSTATUS
FOH_SYNC_READ(
              __in PFLT_INSTANCE Instance,
              __in PFILE_OBJECT  FileObject,
              __in PLARGE_INTEGER  ByteOffset,
              __in ULONG  Length,
              __in PVOID  Buffer,
              __in ULONG  IrpFlags,
              __out_opt PULONG  BytesRead
              );


#endif