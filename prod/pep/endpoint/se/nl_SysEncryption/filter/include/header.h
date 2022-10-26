

#pragma once
#ifndef __NLSE_FILEHEADER_H__
#define __NLSE_FILEHEADER_H__
#include "nlsecommon.h"
#include "NLSEStruct.h"

/*********************************************************************
 Header Structure
**********************************************************************/
#define NLSE_ENVELOPE_SIZE              0x1600
#define NLSE_DEFAULT_SECTOR_SIZE        0x0200   // 512 byte

#define NLSE_ENVELOPE_VER_MAJOR         2
#define NLSE_ENVELOPE_VER_MINOR         1
#define NLSE_ENCRYPT_VER_MAJOR          2
#define NLSE_ENCRYPT_VER_MINOR          0

#pragma pack(push)
#pragma pack(1)


typedef struct _NLSE_SECTION_HEADER
{
    ULONG   Size;
    UCHAR   Name[8];    // Section Name
} NLSE_SECTION_HEADER, *PNLSE_SECTION_HEADER;
typedef const NLSE_SECTION_HEADER*   PCNLSE_SECTION_HEADER;

/* Flags */
#define NLF_WRAPPED                     0x0000000000000001ui64

typedef struct _NLSE_ENVELOPE
{
    UCHAR   VerMajor;       /* Version Major */
    UCHAR   VerMinor;       /* Minor Version */
    UCHAR   Reserved[2];    /* Reserved */
    ULONG   Size;           /* Envelope Size (bytes).) */
    ULONG   SectionCount;   /* Section Count */
    UCHAR   Cookie[8];      /* Cookie(fixed) */
    ULONG   Attrs;          /* Original file attributes */
    UINT64  Flags;          /* Flags */
} NLSE_ENVELOPE, *PNLSE_ENVELOPE;
typedef const NLSE_ENVELOPE*    PCNLSE_ENVELOPE;

typedef struct _NLSE_ENCRYPT_SECTION
{
    NLSE_SECTION_HEADER   Header;             /* Stream Header */
    UCHAR                 VerMajor;           /* Major version */
    UCHAR                 VerMinor;           /* Minor version */
    UCHAR                 Reserved1[2];       /* Reserved 1 */
    UCHAR                 PcKeyRingName[16];  /* Key Ring Name*/
    UCHAR                 PcKeyId[36];        /* Key ID */
    UCHAR                 UnUsed1[4];         /* UnUsed 1 */
    LARGE_INTEGER         RawFileSize;        /* True File Size (Valid original file content length) */
    UCHAR                 AesKey[16];         /* AES-128 key */
    UINT64                Flags;              /* Flags */
    UCHAR                 Reserved2[8];       /* Reserved 2 */
    ULONG                 AesPadLength;       /* Padding Length */
    UCHAR                 AesPadData[16];     /* Padding Data: 15 bytes at most */
} NLSE_ENCRYPT_SECTION, *PNLSE_ENCRYPT_SECTION;
typedef const NLSE_ENCRYPT_SECTION* PCNLSE_ENCRYPT_SECTION;


#pragma pack(pop)

#define NLSE_IS_NONCACHE_OR_PAGING_IO(Data) (Data->Iopb->IrpFlags & (IRP_NOCACHE | IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO))
#define NLSE_IS_PAGING_IO(Data)             (Data->Iopb->IrpFlags & (IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO))


#define NLSE_INVALID_FILENAME       0x00000000
#define NLSE_VALID_FILENAME         0x00000001
#define NLSE_RESERVED_FILENAME      0x00000002
#define NLSE_NETSHARE_FILENAME      0x00000004
#define NLSE_STREAM_FILENAME        0x00000008
#define NLSE_SYSTEM_FILENAME        0x00000010
#define NLSE_NEXTLABS_FILENAME      0x00000020
#define NLSE_IGNORED_FILENAME       0x00000040
#define NLSE_NONDRM_FILENAME        0x00000080


#define NL_IGNORE_DIRECTORY         0x00000001
#define NL_PROGRAM_DIRECTORY        0x00000002


/*********************************************************************
 Global Structure
**********************************************************************/
__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NLSEParseFileName(
                  __in NLFSE_PVOLUME_CONTEXT VolContext,
                  __in PFLT_CALLBACK_DATA Data,
                  __out PUNICODE_STRING FullFileName,
                  __out PULONG FileNameType
                  );

__drv_maxIRQL(APC_LEVEL)
VOID
NLSEPurgeFileCache(
                   __in PFILE_OBJECT FileObject,
                   __in BOOLEAN bIsFlushCache,
                   __in_opt PLARGE_INTEGER FileOffset,
                   __in ULONG Length
                   );


__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NLSEGetFileAttributes(
                      __in PFLT_FILTER Filter,
                      __in PFLT_INSTANCE Instance,
                      __in USHORT SectorSize,
                      __in PUNICODE_STRING FullFileName,
                      __out PBOOLEAN Exists,
                      __out PBOOLEAN Directory,
                      __out PBOOLEAN Encrypted,
                      __out PULONG Attributes,
                      __out PLARGE_INTEGER FileSize
                      );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEEmptyFile(
              __in PFLT_INSTANCE Instance,
              __in PFILE_OBJECT FileObject
              );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEIsDirectory(
                __in PFLT_INSTANCE Instance,
                __in PFILE_OBJECT FileObject,
                __out PBOOLEAN Directory
                );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGetStdInfoSync(
                   __in PFLT_INSTANCE Instance,
                   __in PFILE_OBJECT FileObject,
                   __out_bcount_full(sizeof(FILE_STANDARD_INFORMATION)) PFILE_STANDARD_INFORMATION StdInfo
                   );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGetBasicInfoSync(
                     __in PFLT_INSTANCE Instance,
                     __in PFILE_OBJECT FileObject,
                     __out_bcount_full(sizeof(FILE_BASIC_INFORMATION)) PFILE_BASIC_INFORMATION BasicInfo
                     );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGetFileAttributesSync(
                          __in PFLT_INSTANCE Instance,
                          __in PFILE_OBJECT FileObject,
                          __out PULONG Attributes
                          );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSESetFileAttributesSync(
                          __in PFLT_INSTANCE Instance,
                          __in PFILE_OBJECT FileObject,
                          __in ULONG Attributes
                          );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGetCurrentOffsetSync(
                         __in PFLT_INSTANCE Instance,
                         __in PFILE_OBJECT FileObject,
                         __out PLARGE_INTEGER CurrentOffset
                         );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSESetCurrentOffsetSync(
                         __in PFLT_INSTANCE Instance,
                         __in PFILE_OBJECT FileObject,
                         __in PLARGE_INTEGER CurrentOffset
                         );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGetFileSizeSync(
                  __in PFLT_INSTANCE Instance,
                  __in PFILE_OBJECT FileObject,
                  __out PLARGE_INTEGER FileSize
                  );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSESetFileSizeSync(
                  __in PFLT_INSTANCE Instance,
                  __in PFILE_OBJECT FileObject,
                  __in const LARGE_INTEGER* FileSize
                  );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEReadFileSync(
                 __in PFLT_INSTANCE Instance,
                 __in PFILE_OBJECT FileObject,
                 __in BOOLEAN NonCached,
                 __in BOOLEAN Paging,
                 __in PLARGE_INTEGER ReadOffset,
                 __inout PULONG ReadLength,
                 __out_bcount_full(*ReadLength) PVOID ReadBuffer
                 );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEWriteFileSync(
                  __in PFLT_INSTANCE Instance,
                  __in PFILE_OBJECT FileObject,
                  __in BOOLEAN NonCached,
                  __in BOOLEAN Paging,
                  __in PLARGE_INTEGER WriteOffset,
                  __in PULONG WriteLength,
                  __in_bcount(*WriteLength) PVOID WriteBuffer
                  );

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEBuildStreamContext(
                       __in PFLT_FILTER Filter,
                       __in PCUNICODE_STRING FileName,
                       __in PLARGE_INTEGER FileSize,
                       __in ULONG FileAttributes,
                       __in ULONG SectorSize,
                       __deref_out PVOID* Context
                       );

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGenerateEmptyHeader(
                        __in PFLT_INSTANCE Instance,
                        __in PFILE_OBJECT FileObject,
                        __in_bcount_opt(16) PUCHAR PcKeyRingName,
                        __in_bcount(36) PUCHAR PcKeyId,
                        __in_bcount(16) PUCHAR PcKey,
                         __deref_out_bcount_full(sizeof(NLSE_ENCRYPT_SECTION)) PNLSE_ENCRYPT_SECTION* peSec
                        );

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGenerateEmptyHeader2(
                         __in PFLT_INSTANCE Instance,
                         __in PFILE_OBJECT FileObject,
                         __in_bcount_opt(16) PUCHAR PcKeyRingName,
                         __in_bcount(36) PUCHAR PcKeyId,
                         __in_bcount(16) PUCHAR PcKey,
                         __deref_out_bcount_full(sizeof(NLFSE_ENCRYPT_EXTENSION)) PNLFSE_ENCRYPT_EXTENSION* pExt
                         );

/*
NOTE: the AesKey in NLSE_ENCRYPT_SECTION is still encrypted
      caller need to get PC key and decrypt AesKey
*/
__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEReadEncryptSection(
                       __in PFLT_INSTANCE Instance,
                       __in PFILE_OBJECT FileObject,
                       __deref_out_bcount(sizeof(NLSE_ENCRYPT_SECTION)) PNLSE_ENCRYPT_SECTION* peSec
                       );

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEReadEncryptSection2(
                        __in PFLT_INSTANCE Instance,
                        __in PFILE_OBJECT FileObject,
                        __deref_out_bcount(sizeof(NLFSE_ENCRYPT_EXTENSION)) PNLFSE_ENCRYPT_EXTENSION* ppExt
                        );

__checkReturn
NTSTATUS
NLSEDecryptAesKey2(
                  __in ULONG ProcessId,
                  __inout PNLFSE_ENCRYPT_EXTENSION pExt,
                  __out PBOOLEAN PcKeyChanged
                  );

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEUpdateEncryptSection(
                         __in PFLT_INSTANCE Instance,
                         __in PFILE_OBJECT FileObject,
                         __in BOOLEAN Paging,
                         __in_bcount_opt(16) PUCHAR PcKeyRingName,
                         __in_bcount(36) PUCHAR PcKeyId,
                         __in_bcount(16) PUCHAR PcKey,
                         __inout_bcount(sizeof(NLSE_ENCRYPT_SECTION)) PNLSE_ENCRYPT_SECTION eSec
                         );

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEUpdateEncryptSection2(
                          __in PFLT_INSTANCE Instance,
                          __in PFILE_OBJECT FileObject,
                          __in BOOLEAN Paging,
                          __in_bcount_opt(16) PUCHAR PcKeyRingName,
                          __in_bcount(36) PUCHAR PcKeyId,
                          __in_bcount(16) PUCHAR PcKey,
                          __inout_bcount(sizeof(NLFSE_ENCRYPT_EXTENSION)) PNLFSE_ENCRYPT_EXTENSION pExt
                          );

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEIsEncryptedFile(
                    __in PFLT_FILTER Filter,
                    __in PFLT_INSTANCE Instance,
                    __in PUNICODE_STRING FullFileName,
                    __out PBOOLEAN Encrypted
                    );

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEIsEncryptedOrWrappedFile2(
                     __in PFLT_INSTANCE Instance,
                     __in PFILE_OBJECT FileObject,
                     __out_opt PBOOLEAN Encrypted,
                     __out_opt PBOOLEAN Wrapped
                     );

BOOLEAN
NLSEIsSystemDirectoryOrFile(
                            __in PCUNICODE_STRING ParentDir, /*Without Volume*/
                            __in PCUNICODE_STRING FileName
                            );

__checkReturn
BOOLEAN
NLSEIsReservedFileName(
                       __in PCUNICODE_STRING ParentDir,
                       __in PCUNICODE_STRING FileName
                       );

VOID
NLSEInitNextLabsHeader(
                       __out PNLSE_ENVELOPE Envelope
                       );

VOID
NLSEInitEncryptSectionHeader(
                             __out PNLSE_ENCRYPT_SECTION eSec,
                             __in_bcount_opt(16) PUCHAR PcKeyRingName,
                             __in_bcount(36) PUCHAR PcKeyId
                             );
VOID
NLSEInitTagsSectionHeader(
                          __out PNLSE_SECTION_HEADER tagSec
                          );

__checkReturn
BOOLEAN
NLSEIsNextLabsFileName(
                      __in PCUNICODE_STRING FileName
                      );


__checkReturn
ULONG
NLCheckDiretory(
                __in PCUNICODE_STRING ParentDir, /*Without Volume*/
                __in PCUNICODE_STRING FileName
                );


__checkReturn
ULONG
NLCheckDiretoryEx(
                  __in PCUNICODE_STRING DirPath /*Without Volume*/
                  );


BOOLEAN
IsIgnoredFiles(
               __in PCWCH FileName,
               __in ULONG FileNameLength,
               __in BOOLEAN InProgramDir
               );


#endif
