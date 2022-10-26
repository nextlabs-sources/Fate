#include <fltKernel.h>
#include "header.h"
#include "nlsestruct.h"
#include "nlseutility.h"
#include "nldircache.h"
#include "NLSEDrmPathList.h"



//  Global variables
extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG           nlseKLog;

const UCHAR EnvelopeCookie[8]       = {0x4E, 0xE5, 0x78, 0xF4, 0x4C, 0xE1, 0x62, 0xF3}; //{'N', 'e', 'x', 't', 'L', 'a', 'b', 's'};
const UCHAR EncryptSectionName[8]   = NLE_STREAM_NAME;
const UCHAR TagsSectionName[8]      = NLT_STREAM_NAME;

const UCHAR NLSELocalKeyRingName[16] = NLE_KEY_RING_LOCAL;

static UNICODE_STRING NtfsReservedNames[] = {
    { 8, 10, L"$Mft"},
    {16, 18, L"$MftMirr"},
    {16, 18, L"$LogFile"},
    {14, 16, L"$Volume"},
    {16, 18, L"$AttrDef"},
    {14, 16, L"$Bitmap"},
    {10, 12, L"$Boot"},
    {16, 18, L"$BadClus"},
    {14, 16, L"$Secure"},
    {14, 16, L"$UpCase"},
    {14, 16, L"$Extend"},
    { 0,  0, NULL}
};
static UNICODE_STRING NtfsExtendDir = {18, 20, L"\\$Extend\\"};
static UNICODE_STRING NtfsReservedExtendNames[] = {
    {14, 16, L"$Config"},
    {14, 16, L"$Delete"},
    {12, 14, L"$ObjId"},
    {12, 14, L"$Quota"},
    {14, 16, L"$Repair"},
    {22, 24, L"$Repair.log"},
    {16, 18, L"$Reparse"},
    {22, 24, L"$RmMetadata"},
    {10, 12, L"$Tops"},
    { 8, 10, L"$Txf"},
    {14, 16, L"$TxfLog"},
    { 0,  0, NULL}
};

DECLARE_CONST_UNICODE_STRING(IGNOREDDIR_BOOT,       L"\\Boot");
DECLARE_CONST_UNICODE_STRING(IGNOREDDIR_WINDOWS,    L"\\Windows");
DECLARE_CONST_UNICODE_STRING(IGNOREDDIR_RECYCLER,   L"\\RECYCLER");
DECLARE_CONST_UNICODE_STRING(IGNOREDDIR_SYSVOL,     L"\\System Volume Information");
DECLARE_CONST_UNICODE_STRING(IGNOREDDIR_NEXTLABS,   L"\\Program Files\\NextLabs");
DECLARE_CONST_UNICODE_STRING(IGNOREDDIR_VMWARE1,    L"\\PROGRAM FILES\\VMWARE");
DECLARE_CONST_UNICODE_STRING(IGNOREDDIR_VMWARE2,    L"\\ProgramData\\VMware");
DECLARE_CONST_UNICODE_STRING(PROGRAMDIR,            L"\\PROGRAM FILES");

const UNICODE_STRING IGNORED_FILE_EXTS[] = {
    {8, 10, L".LNK"},
    {8, 10, L".PDB"},
    {0,  0, NULL}
};

const UNICODE_STRING IGNORED_PEFILE_EXTS[] = {
    {8, 10, L".EXE"},
    {8, 10, L".EX_"},
    {8, 10, L".DLL"},
    {8, 10, L".OCX"},
    {8, 10, L".SYS"},
    {8, 10, L".SY_"},
    {8, 10, L".COM"},
    {0,  0, NULL}
};

/*************************************************************************
 Local routines.
*************************************************************************/
static
__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NLSEGetFileAttributesByHandle(
                              __in PFLT_INSTANCE Instance,
                              __in HANDLE FileHandle,
                              __in USHORT SectorSize,
                              __out PBOOLEAN Directory,
                              __out PBOOLEAN Encrypted,
                              __out PULONG Attributes,
                              __out PLARGE_INTEGER FileSize
                              );

static
VOID
NLSEGenAesKey(
              __out_bcount_full(16) PUCHAR AesKey
              );


//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NLSEParseFileName)
#endif



/*****************************************************************************

 GLOBAL ROUTINES

****************************************************************************/
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEBuildStreamContext(
                       __in PFLT_FILTER Filter,
                       __in PCUNICODE_STRING FileName,
                       __in PLARGE_INTEGER FileSize,
                       __in ULONG FileAttributes,
                       __in ULONG SectorSize,
                       __deref_out PVOID* Context
                       )
{
    NTSTATUS                Status      = STATUS_SUCCESS;
    PNLFSE_STREAM_CONTEXT   StmContext  = NULL;

    // Allocate memory for stream context
    Status = FltAllocateContext(Filter, FLT_STREAM_CONTEXT, sizeof(NLFSE_STREAM_CONTEXT), NonPagedPool, &StmContext);
    if(!NT_SUCCESS(Status))
        goto _exit;

	// Init the context structure
    RtlZeroMemory(StmContext, sizeof(NLFSE_STREAM_CONTEXT));
    StmContext->encryptExtDirty = FALSE;
	StmContext->bDelete         = FALSE;
	StmContext->UseCount        = 1;
	StmContext->FileName.Length = 0;
    StmContext->FileAttrs       = FileAttributes;
    StmContext->SectorSize      = SectorSize;
    StmContext->FileSize.QuadPart = (FileSize->QuadPart>=NLSE_ENVELOPE_SIZE)?(FileSize->QuadPart-NLSE_ENVELOPE_SIZE):FileSize->QuadPart;
	StmContext->FileName.MaximumLength  = FileName->Length + sizeof(WCHAR);
	KeInitializeSpinLock(&StmContext->encryptExtLock);
	ExInitializeFastMutex(&StmContext->deleteFlagLock);

    StmContext->FileName.Buffer = ExAllocatePoolWithTag(NonPagedPool, StmContext->FileName.MaximumLength, NLFSE_BUFFER_TAG);
    if(NULL == StmContext->FileName.Buffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(StmContext->FileName.Buffer, StmContext->FileName.MaximumLength);
    RtlCopyMemory(StmContext->FileName.Buffer, FileName->Buffer, FileName->Length);    
    StmContext->FileName.Length  = FileName->Length;

    // Succeed
    *Context = (PVOID)StmContext;
    StmContext = NULL;

_exit:
    if(StmContext) FltReleaseContext(StmContext); StmContext = NULL;
    return Status;
}

VOID
NLSEInitNextLabsHeader(
                       __out PNLSE_ENVELOPE Envelope
                       )
{
    Envelope->VerMajor    = NLSE_ENVELOPE_VER_MAJOR;
    Envelope->VerMinor    = NLSE_ENVELOPE_VER_MINOR;
    Envelope->Size        = 2*NLSE_DEFAULT_SECTOR_SIZE;
    Envelope->SectionCount= 2;
    RtlCopyMemory(Envelope->Cookie, EnvelopeCookie, 8);
}

VOID
NLSEInitEncryptSectionHeader(
                             __out PNLSE_ENCRYPT_SECTION eSec,
                             __in_bcount_opt(16) PUCHAR PcKeyRingName,
                             __in_bcount(36) PUCHAR PcKeyId
                             )
{
    eSec->Header.Size   = NLSE_DEFAULT_SECTOR_SIZE;
    eSec->VerMajor      = NLSE_ENCRYPT_VER_MAJOR;
    eSec->VerMinor      = NLSE_ENCRYPT_VER_MINOR;
    RtlCopyMemory(eSec->Header.Name, EncryptSectionName, 8);
    
    RtlCopyMemory(eSec->PcKeyRingName, (NULL!=PcKeyRingName)?PcKeyRingName:NLSELocalKeyRingName, 16);
    RtlCopyMemory(eSec->PcKeyId, PcKeyId, 36);
    NLSEGenAesKey(eSec->AesKey);
}

VOID
NLSEInitTagsSectionHeader(
                          __out PNLSE_SECTION_HEADER tagSec
                          )
{
    tagSec->Size    = 4096;
    RtlCopyMemory(tagSec->Name, TagsSectionName, 8);
}

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
                        )
{
    NTSTATUS        Status      = STATUS_SUCCESS;
    LARGE_INTEGER   HeaderSize  = {0};
    LARGE_INTEGER   WriteOffset = {0};
    ULONG           WriteSize   = NLSE_ENVELOPE_SIZE;
    PUCHAR          WriteBuffer = NULL;
    PNLSE_ENVELOPE          Envelope= NULL;
    PNLSE_ENCRYPT_SECTION   eSec    = NULL;
    PNLSE_ENCRYPT_SECTION   eSecOut = NULL;
    PNLSE_SECTION_HEADER    tagSec  = NULL;

    *peSec = NULL;

    // Allocate write buffer
    WriteBuffer = ExAllocatePoolWithTag(NonPagedPool, WriteSize, NLFSE_BUFFER_TAG);
    if(NULL == WriteBuffer)
        return STATUS_INSUFFICIENT_RESOURCES;
    RtlZeroMemory(WriteBuffer, WriteSize);

    // Generate new encrypt section
    eSecOut = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLSE_ENCRYPT_SECTION), NLFSE_BUFFER_TAG);
    if(NULL == eSecOut)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(eSecOut, sizeof(NLSE_ENCRYPT_SECTION));

    Envelope= (PNLSE_ENVELOPE)WriteBuffer;
    eSec    = (PNLSE_ENCRYPT_SECTION)(WriteBuffer + 2*NLSE_DEFAULT_SECTOR_SIZE);
    tagSec  = (PNLSE_SECTION_HEADER)(WriteBuffer + 3*NLSE_DEFAULT_SECTOR_SIZE);

    HeaderSize.QuadPart = NLSE_ENVELOPE_SIZE;
    Status = NLSEEmptyFile(Instance, FileObject);
    if(!NT_SUCCESS(Status)) goto _exit;
    Status = NLSESetFileSizeSync(Instance, FileObject, &HeaderSize);
    if(!NT_SUCCESS(Status)) goto _exit;

    // Initialize Header
    NLSEInitNextLabsHeader(Envelope);
    NLSEInitTagsSectionHeader(tagSec);
    NLSEInitEncryptSectionHeader(eSec, PcKeyRingName, PcKeyId);

    // Make a copy to eSecout before we encrypt the AesKey
    RtlCopyMemory(eSecOut, eSec, sizeof(NLSE_ENCRYPT_SECTION));

    // Encrypt AES Key using PC Key, so we can write it to file
    encrypt_buffer(PcKey, 16, 0, eSec->AesKey, 16); 

    // Write it to file
    Status = NLSEWriteFileSync(Instance, FileObject, TRUE, FALSE, &WriteOffset, &WriteSize, WriteBuffer);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Succeed, output the new encrypt section
    *peSec = eSecOut; eSecOut = NULL;

_exit:
    if(eSecOut) ExFreePool(eSecOut);
    if(WriteBuffer) ExFreePool(WriteBuffer);
    return Status;
}

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
                         )
{
    NTSTATUS                    Status      = STATUS_SUCCESS;
    PNLFSE_ENCRYPT_EXTENSION    Extension   = NULL;
    PNLSE_ENCRYPT_SECTION       eSec        = NULL;
    STRING                      tmpString   = {0};

    
    *pExt = NULL;

    Extension = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLFSE_ENCRYPT_EXTENSION), NLFSE_BUFFER_TAG);
    if(NULL == Extension)
        return STATUS_INSUFFICIENT_RESOURCES;
    RtlZeroMemory(Extension, sizeof(NLFSE_ENCRYPT_EXTENSION)); 

    Status = NLSEGenerateEmptyHeader(Instance, FileObject, PcKeyRingName, PcKeyId, PcKey, &eSec);
    if(!NT_SUCCESS(Status))
        goto _exit; 

    RtlInitString(&tmpString, NLE_STREAM_NAME);
    RtlCopyMemory(Extension->sh.stream_name, tmpString.Buffer, tmpString.Length);  
    Extension->sh.stream_size   = sizeof(NLFSE_ENCRYPT_EXTENSION);
    Extension->version_major    = NLE_FILE_VERSION_MAJOR;
    Extension->version_minor    = NLE_FILE_VERSION_MINOR;
    Extension->fileRealLength   = eSec->RawFileSize.QuadPart;
    Extension->flags            = 0;
    RtlCopyMemory(Extension->pcKeyRingName, eSec->PcKeyRingName, 16);
    RtlCopyMemory(&Extension->pcKeyID, eSec->PcKeyId, 36);
    RtlCopyMemory(Extension->key, eSec->AesKey, 16);

    // Output the extension
    *pExt = Extension; Extension = NULL;

_exit:
    if(eSec)      ExFreePool(eSec);
    if(Extension) ExFreePool(Extension);
    return Status;
}

__checkReturn
NTSTATUS
NLSEDecryptAesKey2(
                  __in ULONG ProcessId,
                  __inout PNLFSE_ENCRYPT_EXTENSION pExt,
                  __out PBOOLEAN PcKeyChanged
                  )
{
    NTSTATUS    Status      = STATUS_SUCCESS;
    UCHAR       PcKey[16]   = {0};
    ULONG       key[4]      = {0}; //128 bit long

    *PcKeyChanged = FALSE;

#if NLSE_DEBUG_FAKE_FILE_KEY

    key[0] = (ULONG)'z';
    key[1] = (ULONG)'z';
    key[2] = (ULONG)'z';
    key[3] = (ULONG)'z';
    RtlCopyMemory(pExt->key, key, 16);

#else

    RtlCopyMemory(PcKey, (PUCHAR)&nlfseGlobal.currentPCKey, 16);
    if(36 != RtlCompareMemory(&pExt->pcKeyID, (PUCHAR)&nlfseGlobal.currentPCKeyID, 36))
    {
        if(!NLSEGetPCKeyByID(pExt->pcKeyRingName, &pExt->pcKeyID, ProcessId, PcKey))
        {
            Status = STATUS_NOT_FOUND;
            goto _exit;
        }
        *PcKeyChanged = TRUE;
    }
    decrypt_buffer(PcKey, 16, 0, pExt->key, 16);

_exit:

#endif

    return Status;
}

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEReadEncryptSection(
                       __in PFLT_INSTANCE Instance,
                       __in PFILE_OBJECT FileObject,
                       __deref_out_bcount(sizeof(NLSE_ENCRYPT_SECTION)) PNLSE_ENCRYPT_SECTION* peSec
                       )
{
    NTSTATUS                Status      = STATUS_SUCCESS;
    LARGE_INTEGER           ReadOffset  = {0};
    ULONG                   ReadSize    = NLSE_DEFAULT_SECTOR_SIZE;
    PNLSE_ENCRYPT_SECTION   ReadBuffer  = NULL;
    PNLSE_ENCRYPT_SECTION   eSecOut     = NULL;

    ReadBuffer = ExAllocatePoolWithTag(NonPagedPool, NLSE_DEFAULT_SECTOR_SIZE, NLFSE_BUFFER_TAG);
    if(NULL == ReadBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(ReadBuffer, NLSE_DEFAULT_SECTOR_SIZE);
    
    eSecOut = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLSE_ENCRYPT_SECTION), NLFSE_BUFFER_TAG);
    if(NULL == eSecOut)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(eSecOut, sizeof(NLSE_ENCRYPT_SECTION));


    // Initialize Header
    RtlZeroMemory(ReadBuffer, NLSE_DEFAULT_SECTOR_SIZE);
    ReadOffset.QuadPart = 2*NLSE_DEFAULT_SECTOR_SIZE;

    // Read
    Status = NLSEReadFileSync(Instance, FileObject, TRUE, FALSE, &ReadOffset, &ReadSize, ReadBuffer);
    if(!NT_SUCCESS(Status)) goto _exit;
    if(NLSE_DEFAULT_SECTOR_SIZE != ReadSize) {Status=STATUS_UNSUCCESSFUL; goto _exit;}

    // Do some basic check
    if(8 != RtlCompareMemory(ReadBuffer->Header.Name, EncryptSectionName, 8))
    {
        Status = STATUS_UNSUCCESSFUL;
        goto _exit;
    }

    // Good, we got it
    RtlCopyMemory(eSecOut, ReadBuffer, sizeof(NLSE_ENCRYPT_SECTION));
    *peSec = eSecOut; eSecOut = NULL;

_exit:
    if(eSecOut) ExFreePool(eSecOut);
    if(ReadBuffer) ExFreePool(ReadBuffer);
    return Status;
}

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEReadEncryptSection2(
                        __in PFLT_INSTANCE Instance,
                        __in PFILE_OBJECT FileObject,
                        __deref_out_bcount(sizeof(NLFSE_ENCRYPT_EXTENSION)) PNLFSE_ENCRYPT_EXTENSION* ppExt
                        )
{
    NTSTATUS                    Status      = STATUS_SUCCESS;
    PNLFSE_ENCRYPT_EXTENSION    Extension   = NULL;
    PNLSE_ENCRYPT_SECTION       eSec        = NULL;
    STRING                      tmpString   = {0};

    Extension = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLFSE_ENCRYPT_EXTENSION), NLFSE_BUFFER_TAG);
    if(NULL == Extension)
        return STATUS_INSUFFICIENT_RESOURCES;

    Status = NLSEReadEncryptSection(Instance, FileObject, &eSec);
    if(!NT_SUCCESS(Status))
        goto _exit;
    
    RtlInitString(&tmpString, NLE_STREAM_NAME);
    RtlCopyMemory(Extension->sh.stream_name, tmpString.Buffer, tmpString.Length);  
    Extension->sh.stream_size   = sizeof(NLFSE_ENCRYPT_EXTENSION);
    Extension->version_major    = NLE_FILE_VERSION_MAJOR;
    Extension->version_minor    = NLE_FILE_VERSION_MINOR;
    RtlCopyMemory(Extension->pcKeyRingName, eSec->PcKeyRingName, 16);
    RtlCopyMemory(&Extension->pcKeyID, eSec->PcKeyId, 36);
    RtlCopyMemory(&Extension->key, eSec->AesKey, 16);

    Extension->fileRealLength   = eSec->RawFileSize.QuadPart;
    Extension->flags            = eSec->Flags;
    Extension->paddingLen       = eSec->AesPadLength;
    RtlCopyMemory(Extension->paddingData, eSec->AesPadData, 16);

    // Output the extension
    *ppExt = Extension; Extension = NULL;

_exit:
    if(eSec)      ExFreePool(eSec);
    if(Extension) ExFreePool(Extension);
    return Status;
}

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
                         )
{
    NTSTATUS                Status      = STATUS_SUCCESS;
    LARGE_INTEGER           WriteOffset = {0};
    ULONG                   WriteSize   = NLSE_DEFAULT_SECTOR_SIZE;
    PNLSE_ENCRYPT_SECTION   WriteBuffer = NULL;

    WriteBuffer = ExAllocatePoolWithTag(NonPagedPool, NLSE_DEFAULT_SECTOR_SIZE, NLFSE_BUFFER_TAG);
    if(NULL == WriteBuffer)
        return STATUS_INSUFFICIENT_RESOURCES;

    // Initialize Header
    RtlZeroMemory(WriteBuffer, NLSE_DEFAULT_SECTOR_SIZE);
    RtlCopyMemory(eSec->PcKeyRingName, (NULL!=PcKeyRingName)?PcKeyRingName:NLSELocalKeyRingName, 16);
    RtlCopyMemory(eSec->PcKeyId, PcKeyId, 36);
    RtlCopyMemory(WriteBuffer, eSec, sizeof(NLSE_ENCRYPT_SECTION));
    encrypt_buffer(PcKey, 16, 0, WriteBuffer->AesKey, 16); 

    // Write it to file
    WriteOffset.QuadPart = 2*NLSE_DEFAULT_SECTOR_SIZE;
    Status = NLSEWriteFileSync(Instance, FileObject, TRUE, Paging, &WriteOffset, &WriteSize, WriteBuffer);

    if(WriteBuffer) ExFreePool(WriteBuffer);
    return Status;
}

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
                          )
{
    NTSTATUS                    Status      = STATUS_SUCCESS;
    PNLFSE_ENCRYPT_EXTENSION    Extension   = NULL;
    PNLSE_ENCRYPT_SECTION       eSec        = NULL;
    UNICODE_STRING              tmpString   = {0};

    eSec = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLSE_ENCRYPT_SECTION), NLFSE_BUFFER_TAG);
    if(NULL == eSec)
        return STATUS_INSUFFICIENT_RESOURCES;

    // Init encrypt section
    RtlZeroMemory(eSec, sizeof(NLSE_ENCRYPT_SECTION));
    
    eSec->Header.Size   = NLSE_DEFAULT_SECTOR_SIZE;
    eSec->VerMajor      = NLSE_ENCRYPT_VER_MAJOR;
    eSec->VerMinor      = NLSE_ENCRYPT_VER_MINOR;
    RtlCopyMemory(eSec->Header.Name, EncryptSectionName, 8);
    RtlCopyMemory(eSec->PcKeyRingName, pExt->pcKeyRingName, 16);
    RtlCopyMemory(eSec->PcKeyId, &pExt->pcKeyID, 36);
    RtlCopyMemory(eSec->AesKey, pExt->key, 16);
    eSec->RawFileSize.QuadPart = pExt->fileRealLength;
    eSec->AesPadLength         = pExt->paddingLen;
    RtlCopyMemory(eSec->AesPadData, pExt->paddingData, 16);
    eSec->Flags                = pExt->flags;

    Status = NLSEUpdateEncryptSection(Instance, FileObject, Paging, PcKeyRingName, PcKeyId, PcKey, eSec);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // The KeyRingName and KeyId might be changed
    RtlCopyMemory(pExt->pcKeyRingName, eSec->PcKeyRingName, 16);
    RtlCopyMemory(&pExt->pcKeyID, eSec->PcKeyId, 36);

_exit:
    if(eSec)      ExFreePool(eSec);
    if(Extension) ExFreePool(Extension);
    return Status;
}

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEIsEncryptedOrWrappedFile2(
                     __in PFLT_INSTANCE Instance,
                     __in PFILE_OBJECT FileObject,
                     __out_opt PBOOLEAN Encrypted,
                     __out_opt PBOOLEAN Wrapped
                     )
{
    NTSTATUS            Status      = STATUS_SUCCESS;
    PUCHAR              ReadBuffer  = NULL;
    LARGE_INTEGER       ReadOffset  = {0};
    ULONG               BytesRead   = 0;

    PCNLSE_ENVELOPE     Envelope = NULL;
    PCNLSE_ENCRYPT_SECTION  eSec = NULL;

    if (Encrypted != NULL)
        *Encrypted = FALSE;
    if (Wrapped != NULL)
        *Wrapped = FALSE;

    ReadBuffer = ExAllocatePoolWithTag(NonPagedPool, 3*NLSE_DEFAULT_SECTOR_SIZE, NLFSE_BUFFER_TAG);   // 0x600
    if(NULL == ReadBuffer)
        return STATUS_INSUFFICIENT_RESOURCES;

    // Read Encrypt Header
    BytesRead = 3*NLSE_DEFAULT_SECTOR_SIZE;
    Status = NLSEReadFileSync(Instance, FileObject, TRUE, FALSE, &ReadOffset, &BytesRead, ReadBuffer);
    if(!NT_SUCCESS(Status) || (3*NLSE_DEFAULT_SECTOR_SIZE)!=BytesRead)
        goto _exit;


    Envelope = (PCNLSE_ENVELOPE)ReadBuffer;
    eSec = (PCNLSE_ENCRYPT_SECTION)(ReadBuffer+2*NLSE_DEFAULT_SECTOR_SIZE);
    if(8 != RtlCompareMemory(Envelope->Cookie, EnvelopeCookie, 8))
        goto _exit;
    if(8 != RtlCompareMemory(eSec->Header.Name, EncryptSectionName, 8))
        goto _exit;

    if (Encrypted != NULL)
        *Encrypted = !BooleanFlagOn(Envelope->Flags, NLF_WRAPPED);
    if (Wrapped != NULL)
        *Wrapped = BooleanFlagOn(Envelope->Flags, NLF_WRAPPED);

_exit:
    if(ReadBuffer) ExFreePool(ReadBuffer);
    return Status;
}

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEIsEncryptedFile(
                    __in PFLT_FILTER Filter,
                    __in PFLT_INSTANCE Instance,
                    __in PUNICODE_STRING FullFileName,
                    __out PBOOLEAN Encrypted
                    )
{
    NTSTATUS            Status      = STATUS_SUCCESS;
    HANDLE              FileHandle  = NULL;
    PFILE_OBJECT        FileObject  = NULL;
    OBJECT_ATTRIBUTES   ObjAttr;
    IO_STATUS_BLOCK     IoStatusBlock;

    *Encrypted = FALSE;

    //Initialization
    InitializeObjectAttributes(&ObjAttr, FullFileName, OBJ_KERNEL_HANDLE, NULL, NULL);

    // Open the file
    Status = FltCreateFile(Filter,
        Instance,
        &FileHandle,
        GENERIC_READ,
        &ObjAttr,
        &IoStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
        FILE_OPEN,
        ( FILE_SYNCHRONOUS_IO_NONALERT |
          FILE_OPEN_REPARSE_POINT |
          FILE_NO_INTERMEDIATE_BUFFERING |  // Non-cached I/O
          FILE_COMPLETE_IF_OPLOCKED ),      // Use this flag to avoid pending
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if(STATUS_OPLOCK_BREAK_IN_PROGRESS == Status)
    {
        // File is locked?        
        // According to DDK Help (See help page for IRP_MJ_CREATE):
        //   If a filter or minifilter cannot honor the FILE_COMPLETE_IF_OPLOCKED flag,
        //   it must complete the IRP_MJ_CREATE request with STATUS_SHARING_VIOLATION.
        Status  = STATUS_SHARING_VIOLATION;
        goto _exit;
    }
    if(!NT_SUCCESS(Status))
    {
        goto _exit;
    }

    // Reference File Object
    Status = ObReferenceObjectByHandle(FileHandle, 0, NULL, KernelMode, &FileObject, NULL);
    if(!NT_SUCCESS(Status))
        goto _exit;

    Status = NLSEIsEncryptedOrWrappedFile2(Instance, FileObject, Encrypted, NULL);

_exit:
    if(FileObject) ObDereferenceObject(FileObject); FileObject=NULL;
    if(FileHandle) FltClose(FileHandle); FileHandle=NULL;
    return Status;
}



/*****************************************************************************

 LOCAL ROUTINES

****************************************************************************/
static
BOOLEAN
NLSEIsDriverFile(
                 __in PUNICODE_STRING Extension
                 )
{
    UNICODE_STRING SysExt   = {0};

    RtlInitUnicodeString(&SysExt, L"sys");
    if(0 == RtlCompareUnicodeString(Extension, &SysExt, TRUE))
        return TRUE;

    return FALSE;
}

static
BOOLEAN
NLSECompareUnicodeString(
                         __in PCUNICODE_STRING Str1,
                         __in PCUNICODE_STRING Str2,
                         __in USHORT Length,
                         __in BOOLEAN CaseInSensitive
                         )
{
    UNICODE_STRING ComapreStr1 = {0};
    UNICODE_STRING ComapreStr2 = {0};

    if(Str1->Length < Length || Str2->Length < Length)
        return FALSE;

    ComapreStr1.Buffer          = Str1->Buffer;
    ComapreStr1.Length          = Length;
    ComapreStr1.MaximumLength   = Str1->MaximumLength;
    ComapreStr2.Buffer          = Str2->Buffer;
    ComapreStr2.Length          = Length;
    ComapreStr2.MaximumLength   = Str2->MaximumLength;
    return (0 == RtlCompareUnicodeString(&ComapreStr1, &ComapreStr2, CaseInSensitive))?TRUE:FALSE;
}

BOOLEAN
NLSEIsSystemDirectoryOrFile(
                            __in PCUNICODE_STRING ParentDir, /*Without Volume*/
                            __in PCUNICODE_STRING FileName
                            )
{
    UNICODE_STRING  BootPath    = {0};
    UNICODE_STRING  WindowsPath = {0};
    UNICODE_STRING  WindowsTemp = {0};
    UNICODE_STRING  SystemPath  = {0};
    UNICODE_STRING  System32Path= {0};
    UNICODE_STRING  SysVolPath  = {0};
    UNICODE_STRING  NextLabsPath= {0};
    UNICODE_STRING  VmWarePath  = {0};
    UNICODE_STRING  WinIni      = {0};
    UNICODE_STRING  PageFileName= {0};
    UNICODE_STRING  BCDFile     = {0};
    UNICODE_STRING  BCDLogFile  = {0};
    UNICODE_STRING  WindowsName = {0};
    UNICODE_STRING  SysVolName  = {0};
    UNICODE_STRING  ProgDataName= {0};


    RtlInitUnicodeString(&BootPath,     L"\\Boot\\");
    RtlInitUnicodeString(&WindowsPath,  L"\\Windows\\");
    RtlInitUnicodeString(&WindowsTemp,  L"\\Windows\\Temp\\");
    RtlInitUnicodeString(&SystemPath,   L"\\Windows\\System\\");
    RtlInitUnicodeString(&System32Path, L"\\Windows\\System32\\");
    RtlInitUnicodeString(&SysVolPath,   L"\\System Volume Information\\");
    RtlInitUnicodeString(&NextLabsPath, L"\\Program Files\\NextLabs\\");
    RtlInitUnicodeString(&VmWarePath,   L"\\ProgramData\\VMware");

    RtlInitUnicodeString(&WindowsName,  L"Windows");
    RtlInitUnicodeString(&SysVolName,   L"System Volume Information");
    RtlInitUnicodeString(&WinIni,       L"win.ini");
    RtlInitUnicodeString(&PageFileName, L"pagefile.sys");
    RtlInitUnicodeString(&BCDFile,      L"BCD");
    RtlInitUnicodeString(&BCDLogFile,   L"BCD.LOG");
    RtlInitUnicodeString(&ProgDataName, L"ProgramData");

    
    if(0 == RtlCompareUnicodeString(FileName, &PageFileName, TRUE))
        return TRUE;
    if(0 == RtlCompareUnicodeString(FileName, &WinIni, TRUE))
        return TRUE;

    // Root?
    if(ParentDir->Length==2 && ParentDir->Buffer[0]==L'\\')
    {
        if(0 == RtlCompareUnicodeString(FileName, &WindowsName, TRUE))
            return TRUE;
        if(0 == RtlCompareUnicodeString(FileName, &SysVolName, TRUE))
            return TRUE;
        if(0 == RtlCompareUnicodeString(FileName, &ProgDataName, TRUE))
            return TRUE;
    }

    if(NLSECompareUnicodeString(ParentDir, &WindowsTemp, WindowsTemp.Length, TRUE))
        return FALSE;
    if(NLSECompareUnicodeString(ParentDir, &NextLabsPath, NextLabsPath.Length, TRUE))
        return TRUE;
    if(NLSECompareUnicodeString(ParentDir, &VmWarePath, VmWarePath.Length, TRUE))
        return TRUE;
    if(NLSECompareUnicodeString(ParentDir, &SysVolPath, SysVolPath.Length, TRUE))
        return TRUE;
    if(NLSECompareUnicodeString(ParentDir, &System32Path, System32Path.Length, TRUE))
        return TRUE;
    if(NLSECompareUnicodeString(ParentDir, &SystemPath, SystemPath.Length, TRUE))
        return TRUE;
    if(NLSECompareUnicodeString(ParentDir, &WindowsPath, WindowsPath.Length, TRUE))
        return TRUE;
    if(NLSECompareUnicodeString(ParentDir, &BootPath, BootPath.Length, TRUE))
    {
        if(0 == RtlCompareUnicodeString(FileName, &BCDFile, TRUE))
            return TRUE;
        if(0 == RtlCompareUnicodeString(FileName, &BCDLogFile, TRUE))
            return TRUE;
    }

    return FALSE;
}

__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NLSEParseFileName(
                  __in NLFSE_PVOLUME_CONTEXT VolCtx,
                  __in PFLT_CALLBACK_DATA Data,
                  __out PUNICODE_STRING FullFileName,
                  __out PULONG FileNameType
                  )
{
    NTSTATUS                    Status   = STATUS_SUCCESS;
    PFLT_FILE_NAME_INFORMATION  OpenedNameInfo = NULL;
    PFLT_FILE_NAME_INFORMATION  NameInfo = NULL;
    ULONG                       DirFlags = 0;
    WCHAR                       DriveLetter = L'\0';
    BOOLEAN                     IsNonDRM    = FALSE;

    PAGED_CODE();

    *FileNameType = NLSE_INVALID_FILENAME;
    DriveLetter   = VolCtx->DosName.Buffer[0];

    // Get the opened name of the file.
    Status = FltGetFileNameInformation
      (Data, FLT_FILE_NAME_OPENED|FLT_FILE_NAME_QUERY_DEFAULT,
       &OpenedNameInfo);
    if (!NT_SUCCESS(Status)) goto _exit;

    // Use the opened name to look up the cache.  If found, done.
    Status = NLFNInfoCacheFind(&VolCtx->FNInfoCache, &OpenedNameInfo->Name,
                               FullFileName, FileNameType);
    if (NT_SUCCESS(Status)) goto _exit;

    // File name not found in cache.  Continue to get it from the system.

    Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED|FLT_FILE_NAME_QUERY_DEFAULT, &NameInfo);
    if(!NT_SUCCESS(Status)) goto _exit;

    Status = FltParseFileNameInformation(NameInfo);
    if(!NT_SUCCESS(Status)) goto _exit;

    // Is valid path?
    if(0==NameInfo->Name.Length || NULL==NameInfo->Name.Buffer) goto _exit;
    if(0==NameInfo->ParentDir.Length || NULL==NameInfo->ParentDir.Buffer) goto _exit;
    if(0==NameInfo->FinalComponent.Length || NULL==NameInfo->FinalComponent.Buffer) goto _exit;
    *FileNameType |= NLSE_VALID_FILENAME;

    
    Status = NxIsNonDrmDirectoryEx(DriveLetter, NameInfo, &IsNonDRM);
    if(NT_SUCCESS(Status) && IsNonDRM)
    {
        // This directory is (or is under) a NON-DRM directory
        // Then we don't need to check it
        *FileNameType |= NLSE_NONDRM_FILENAME;
    }
    else
    {
        // Is it a system file
        if(NLSEIsSystemDirectoryOrFile(&NameInfo->ParentDir, &NameInfo->FinalComponent)) *FileNameType |= NLSE_SYSTEM_FILENAME;

        // Is it NTFS reserved name?
        if(NLSEIsReservedFileName(&NameInfo->ParentDir, &NameInfo->FinalComponent)) *FileNameType |= NLSE_RESERVED_FILENAME;

        // Is network share file name?
        if(0!=NameInfo->Share.Length && NULL!=NameInfo->Share.Buffer) *FileNameType |= NLSE_NETSHARE_FILENAME;

        // Is stream file name?
        if(0!=NameInfo->Stream.Length && NULL!=NameInfo->Stream.Buffer)
        {
            *FileNameType |= NLSE_STREAM_FILENAME;
        }

        // Is NextLabs file name?
        if(NLSEIsNextLabsFileName(&NameInfo->FinalComponent))
        {
            *FileNameType |= NLSE_NEXTLABS_FILENAME;
        }

        // 
        DirFlags = NLCheckDiretory(&NameInfo->ParentDir, &NameInfo->FinalComponent);
        if( FlagOn(DirFlags, NL_IGNORE_DIRECTORY) )
        {
            *FileNameType |= NLSE_IGNORED_FILENAME;
        }
        if(NameInfo->FinalComponent.Length>0
            && IsIgnoredFiles(NameInfo->FinalComponent.Buffer, NameInfo->FinalComponent.Length, FlagOn(DirFlags, NL_PROGRAM_DIRECTORY)))
                *FileNameType |= NLSE_IGNORED_FILENAME;
    }

    // Allocate FullFileName
    FullFileName->Length = NameInfo->Name.Length;
    FullFileName->MaximumLength = FullFileName->Length + sizeof(WCHAR);
    FullFileName->Buffer = (PWCH)ExAllocatePoolWithTag(NonPagedPool, FullFileName->MaximumLength, NLFSE_BUFFER_TAG);
    if(NULL == FullFileName->Buffer)
    {
        FullFileName->Length        = 0;
        FullFileName->MaximumLength = 0;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(FullFileName->Buffer, FullFileName->MaximumLength);
    RtlCopyMemory(FullFileName->Buffer, NameInfo->Name.Buffer, FullFileName->Length);

    // Add result to cache.
    NLFNInfoCacheAdd(&VolCtx->FNInfoCache, &OpenedNameInfo->Name,
                     &NameInfo->Name, *FileNameType);

_exit:
    if(NULL != NameInfo)
    {
      FltReleaseFileNameInformation(NameInfo);
      NameInfo=NULL;
    }
    if(NULL != OpenedNameInfo)
    {
      FltReleaseFileNameInformation(OpenedNameInfo);
      OpenedNameInfo=NULL;
    }
    return Status;
}


__checkReturn
BOOLEAN
NLSEIsReservedFileName(
                       __in PCUNICODE_STRING ParentDir,
                       __in PCUNICODE_STRING FileName
                       )
{
    int i = 0;

    // Sanity check
    if(FileName->Length < 8) return FALSE;
    if(L'$' != FileName->Buffer[0]) return FALSE;
    if(ParentDir->Length < 1) return FALSE;

    if(2==ParentDir->Length && L'\\'==ParentDir->Buffer[0])
    {
        while(0 != NtfsReservedNames[i].Length)
        {
            if(0 == RtlCompareUnicodeString(FileName, &NtfsReservedNames[i++], TRUE)) return TRUE;
        }
    }
    else if (0 == RtlCompareUnicodeString(ParentDir, &NtfsExtendDir, TRUE))
    {
        while(0 != NtfsReservedExtendNames[i].Length)
        {
            if(0 == RtlCompareUnicodeString(FileName, &NtfsReservedExtendNames[i++], TRUE)) return TRUE;
        }
    }
    else
    {
        return FALSE;
    }

    return FALSE;
}


__checkReturn
BOOLEAN
NLSEIsNextLabsFileName(
                      __in PCUNICODE_STRING FileName
                      )
{
  DECLARE_CONST_UNICODE_STRING(nlFileExt, NL_FILE_EXT);
  UNICODE_STRING maybeFileExt;

  if (FileName->Length < nlFileExt.Length)
  {
    return FALSE;
  }

  // Create a string that is the last 4 characters of the passed filename.
  // (This may or may not be the actual extension part of the passed
  // filename.)
  maybeFileExt.Buffer = FileName->Buffer +
    (FileName->Length - nlFileExt.Length) / sizeof(WCHAR);
  maybeFileExt.Length = nlFileExt.Length;
  maybeFileExt.MaximumLength = FileName->MaximumLength - FileName->Length +
    nlFileExt.Length;

  return RtlEqualUnicodeString(&maybeFileExt, &nlFileExt, TRUE);
}

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
                      )
{
    NTSTATUS            Status          = STATUS_SUCCESS;
    HANDLE              FileHandle      = NULL;
    OBJECT_ATTRIBUTES   ObjAttr         = {0};
    IO_STATUS_BLOCK     IoStatusBlock   = {0};

    // Initialize return value
    *Exists             = TRUE;

    // Sanity check
    if(0==SectorSize) return STATUS_INVALID_PARAMETER;

    // Try to open file to read attributes
    InitializeObjectAttributes(&ObjAttr, FullFileName, OBJ_KERNEL_HANDLE, NULL, NULL);
    Status = FltCreateFile(Filter,
        Instance,
        &FileHandle,
        GENERIC_READ,//FILE_READ_ATTRIBUTES|SYNCHRONIZE,   // Use min privilege
        &ObjAttr,
        &IoStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
        FILE_OPEN,                          // Try to open existing file
        ( FILE_SYNCHRONOUS_IO_NONALERT |
          FILE_OPEN_REPARSE_POINT |
          FILE_NO_INTERMEDIATE_BUFFERING |  // Non-cache I/O
          FILE_COMPLETE_IF_OPLOCKED ),      // Use this flag to avoid pending
        NULL, 0, IO_IGNORE_SHARE_ACCESS_CHECK);
    if(STATUS_OPLOCK_BREAK_IN_PROGRESS == Status)
    {
        // File is locked?
        // According to DDK Help (See help page for IRP_MJ_CREATE):
        //   If a filter or minifilter cannot honor the FILE_COMPLETE_IF_OPLOCKED flag,
        //   it must complete the IRP_MJ_CREATE request with STATUS_SHARING_VIOLATION.
        Status  = STATUS_SHARING_VIOLATION;
        goto _exit;
    }

    if(!NT_SUCCESS(Status))
    {
        if(STATUS_OBJECT_NAME_NOT_FOUND==Status || STATUS_OBJECT_PATH_NOT_FOUND==Status)
        {
            // If the file doesn't exist, we think this function still succeed
            // Just return STATUS_SUCCESS and set Exists = FALSE;
            *Exists = FALSE;
            Status  = STATUS_SUCCESS;
        }
        goto _exit;
    }

    Status = NLSEGetFileAttributesByHandle(Instance, FileHandle, SectorSize, Directory, Encrypted, Attributes, FileSize);

_exit:
    if(FileHandle) FltClose(FileHandle); FileHandle=NULL;
    return Status;
}

__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NLSEGetFileAttributesByHandle(
                              __in PFLT_INSTANCE Instance,
                              __in HANDLE FileHandle,
                              __in USHORT SectorSize,
                              __out PBOOLEAN Directory,
                              __out PBOOLEAN Encrypted,
                              __out PULONG Attributes,
                              __out PLARGE_INTEGER FileSize
                              )
{    
    NTSTATUS            Status = STATUS_SUCCESS;
    PFILE_OBJECT        FileObject = NULL;
    FILE_STANDARD_INFORMATION  fsi = {0};
    FILE_BASIC_INFORMATION     fbi = {0};
    
    // Initialize return value
    *Directory         = FALSE;
    *Encrypted         = FALSE;
    *Attributes        = 0;
    FileSize->QuadPart = 0;

    // Reference File Object
    Status = ObReferenceObjectByHandle(FileHandle, 0, NULL, KernelMode, &FileObject, NULL);
    if(!NT_SUCCESS(Status)) goto _exit;

    // Try to get file attributes
    Status = FltQueryInformationFile(Instance, FileObject, &fbi, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation, NULL);
    if(!NT_SUCCESS(Status)) goto _exit;
    *Attributes = fbi.FileAttributes;

    // Try to get file size and directory flag
    Status = FltQueryInformationFile(Instance, FileObject, &fsi, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation, NULL);
    if(!NT_SUCCESS(Status)) goto _exit;
    *Directory          = fsi.Directory;
    FileSize->QuadPart  = fsi.EndOfFile.QuadPart;

    if(FileSize->QuadPart < NLSE_ENVELOPE_SIZE)
        goto _exit;

    // Read File Header, and see if it has NLSE file flag
    Status = NLSEIsEncryptedOrWrappedFile2(Instance, FileObject, Encrypted, NULL);

_exit:
    if(FileObject) ObDereferenceObject(FileObject); FileObject=NULL;
    return Status;
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEEmptyFile(
              __in PFLT_INSTANCE Instance,
              __in PFILE_OBJECT FileObject
              )
{
    LARGE_INTEGER FileSize = {0};
    return NLSESetFileSizeSync(Instance, FileObject, &FileSize);
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEIsDirectory(
                __in PFLT_INSTANCE Instance,
                __in PFILE_OBJECT FileObject,
                __out PBOOLEAN Directory
                )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    FILE_STANDARD_INFORMATION   fsi = {0};

    *Directory = FALSE;

    Status = NLSEGetStdInfoSync(Instance, FileObject, &fsi);
    if(!NT_SUCCESS(Status)) return Status;

    *Directory = fsi.Directory;
    return STATUS_SUCCESS;
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGetStdInfoSync(
                   __in PFLT_INSTANCE Instance,
                   __in PFILE_OBJECT FileObject,
                   __out_bcount_full(sizeof(FILE_STANDARD_INFORMATION)) PFILE_STANDARD_INFORMATION StdInfo
                   )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  FltCD  = NULL;

    Status =FltAllocateCallbackData(Instance, FileObject, &FltCD);
    if(!NT_SUCCESS(Status))
        return Status;

    // Fill IO Data
    FltCD->RequestorMode = KernelMode;
    FltCD->Iopb->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    FltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    FltCD->Iopb->Parameters.QueryFileInformation.FileInformationClass = FileStandardInformation;
    FltCD->Iopb->Parameters.QueryFileInformation.Length               = sizeof(FILE_STANDARD_INFORMATION);
    FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_STANDARD_INFORMATION), NLFSE_BUFFER_TAG);
    if(NULL == FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }

    // Perform the sync call
    FltPerformSynchronousIo(FltCD);
    Status = FltCD->IoStatus.Status;
    if(NT_SUCCESS(Status))
    {
        RtlCopyMemory(StdInfo, FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer, sizeof(FILE_STANDARD_INFORMATION));
    }

_exit:
    if(NULL!=FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer) ExFreePool(FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer);
    if(NULL!=FltCD) FltFreeCallbackData(FltCD);
    return Status;
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGetBasicInfoSync(
                     __in PFLT_INSTANCE Instance,
                     __in PFILE_OBJECT FileObject,
                     __out_bcount_full(sizeof(FILE_BASIC_INFORMATION)) PFILE_BASIC_INFORMATION BasicInfo
                     )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  FltCD  = NULL;

    Status =FltAllocateCallbackData(Instance, FileObject, &FltCD);
    if(!NT_SUCCESS(Status))
        return Status;

    // Fill IO Data
    FltCD->RequestorMode = KernelMode;
    FltCD->Iopb->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    FltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    FltCD->Iopb->Parameters.QueryFileInformation.FileInformationClass = FileBasicInformation;
    FltCD->Iopb->Parameters.QueryFileInformation.Length               = sizeof(FILE_BASIC_INFORMATION);
    FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_BASIC_INFORMATION), NLFSE_BUFFER_TAG);
    if(NULL == FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }

    // Perform the sync call
    FltPerformSynchronousIo(FltCD);
    Status = FltCD->IoStatus.Status;
    if(NT_SUCCESS(Status))
    {
        RtlCopyMemory(BasicInfo, FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer, sizeof(FILE_BASIC_INFORMATION));
    }

_exit:
    if(NULL!=FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer) ExFreePool(FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer);
    if(NULL!=FltCD) FltFreeCallbackData(FltCD);
    return Status;
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGetFileAttributesSync(
                          __in PFLT_INSTANCE Instance,
                          __in PFILE_OBJECT FileObject,
                          __out PULONG Attributes
                          )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    FILE_BASIC_INFORMATION  fbi = {0};

    RtlZeroMemory(&fbi, sizeof(FILE_BASIC_INFORMATION));
    Status = NLSEGetBasicInfoSync(Instance, FileObject, &fbi);
    *Attributes = NT_SUCCESS(Status)?fbi.FileAttributes:0;

    return Status;
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSESetFileAttributesSync(
                          __in PFLT_INSTANCE Instance,
                          __in PFILE_OBJECT FileObject,
                          __in ULONG Attributes
                          )
{
    FILE_BASIC_INFORMATION  fbi = {0};

    RtlZeroMemory(&fbi, sizeof(FILE_BASIC_INFORMATION));
    fbi.FileAttributes = Attributes;
    return NLSEGetBasicInfoSync(Instance, FileObject, &fbi);
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGetCurrentOffsetSync(
                         __in PFLT_INSTANCE Instance,
                         __in PFILE_OBJECT FileObject,
                         __out PLARGE_INTEGER CurrentOffset
                         )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  FltCD  = NULL;

    CurrentOffset->QuadPart = 0;

    Status =FltAllocateCallbackData(Instance, FileObject, &FltCD);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Fill IO Data
    FltCD->RequestorMode = KernelMode;
    FltCD->Iopb->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    FltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    FltCD->Iopb->Parameters.QueryFileInformation.FileInformationClass = FilePositionInformation;
    FltCD->Iopb->Parameters.QueryFileInformation.Length               = sizeof(FILE_POSITION_INFORMATION);
    FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_POSITION_INFORMATION), NLFSE_BUFFER_TAG);
    if(NULL == FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }

    // Perform the sync call
    FltPerformSynchronousIo(FltCD);
    Status = FltCD->IoStatus.Status;
    if(NT_SUCCESS(Status))
        CurrentOffset->QuadPart = ((PFILE_POSITION_INFORMATION)FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer)->CurrentByteOffset.QuadPart;

_exit:
    if(NULL!=FltCD)
    {
      if(NULL!=FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer) ExFreePool(FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer);
      FltFreeCallbackData(FltCD);
    }
    return Status;
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSESetCurrentOffsetSync(
                         __in PFLT_INSTANCE Instance,
                         __in PFILE_OBJECT FileObject,
                         __in PLARGE_INTEGER CurrentOffset
                         )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  FltCD  = NULL;

    Status =FltAllocateCallbackData(Instance, FileObject, &FltCD);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Fill IO Data
    FltCD->RequestorMode       = KernelMode;
    FltCD->Iopb->MajorFunction = IRP_MJ_SET_INFORMATION;
    FltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    FltCD->Iopb->Parameters.SetFileInformation.FileInformationClass = FilePositionInformation;
    FltCD->Iopb->Parameters.SetFileInformation.Length               = sizeof(FILE_POSITION_INFORMATION);
    FltCD->Iopb->Parameters.SetFileInformation.InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_POSITION_INFORMATION), NLFSE_BUFFER_TAG);
    if(NULL == FltCD->Iopb->Parameters.SetFileInformation.InfoBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    ((PFILE_POSITION_INFORMATION)FltCD->Iopb->Parameters.SetFileInformation.InfoBuffer)->CurrentByteOffset.QuadPart = CurrentOffset->QuadPart;

    // Perform the sync call
    FltPerformSynchronousIo(FltCD);
    Status = FltCD->IoStatus.Status;

_exit:
    if(NULL!=FltCD->Iopb->Parameters.SetFileInformation.InfoBuffer) ExFreePool(FltCD->Iopb->Parameters.SetFileInformation.InfoBuffer);
    if(NULL!=FltCD) FltFreeCallbackData(FltCD);
    return Status;
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEGetFileSizeSync(
                    __in PFLT_INSTANCE Instance,
                    __in PFILE_OBJECT FileObject,
                    __out PLARGE_INTEGER FileSize
                    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  FltCD  = NULL;

    FileSize->QuadPart = 0;

    Status =FltAllocateCallbackData(Instance, FileObject, &FltCD);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Fill IO Data
    FltCD->RequestorMode = KernelMode;
    FltCD->Iopb->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    FltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    FltCD->Iopb->IrpFlags      = IRP_NOCACHE;
    FltCD->Iopb->Parameters.QueryFileInformation.FileInformationClass = FileStandardInformation;
    FltCD->Iopb->Parameters.QueryFileInformation.Length               = sizeof(FILE_STANDARD_INFORMATION);
    FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_STANDARD_INFORMATION), NLFSE_BUFFER_TAG);
    if(NULL == FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }

    // Perform the sync call
    FltPerformSynchronousIo(FltCD);
    Status = FltCD->IoStatus.Status;
    if(NT_SUCCESS(Status))
        FileSize->QuadPart = ((PFILE_STANDARD_INFORMATION)FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer)->EndOfFile.QuadPart;

_exit:
    if(NULL!=FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer) ExFreePool(FltCD->Iopb->Parameters.QueryFileInformation.InfoBuffer);
    if(NULL!=FltCD) FltFreeCallbackData(FltCD);
    return Status;
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSESetFileSizeSync(
                    __in PFLT_INSTANCE Instance,
                    __in PFILE_OBJECT FileObject,
                    __in const LARGE_INTEGER* FileSize
                    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  FltCD  = NULL;
    LARGE_INTEGER       CurrentOffset = {0};

    Status =FltAllocateCallbackData(Instance, FileObject, &FltCD);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Fill IO Data
    FltCD->RequestorMode = KernelMode;
    FltCD->Iopb->MajorFunction = IRP_MJ_SET_INFORMATION;
    FltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    FltCD->Iopb->IrpFlags      = IRP_NOCACHE;
    FltCD->Iopb->Parameters.SetFileInformation.FileInformationClass = FileEndOfFileInformation;
    FltCD->Iopb->Parameters.SetFileInformation.Length               = sizeof(FILE_END_OF_FILE_INFORMATION);
    FltCD->Iopb->Parameters.SetFileInformation.InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_END_OF_FILE_INFORMATION), NLFSE_BUFFER_TAG);
    if(NULL == FltCD->Iopb->Parameters.SetFileInformation.InfoBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(FltCD->Iopb->Parameters.SetFileInformation.InfoBuffer, sizeof(FILE_END_OF_FILE_INFORMATION));

    ((PFILE_END_OF_FILE_INFORMATION)FltCD->Iopb->Parameters.SetFileInformation.InfoBuffer)->EndOfFile.QuadPart = FileSize->QuadPart;

    // Perform the sync call
    Status = NLSEGetCurrentOffsetSync(Instance, FileObject, &CurrentOffset);
    FltPerformSynchronousIo(FltCD);
    if(NT_SUCCESS(Status)) NLSESetCurrentOffsetSync(Instance, FileObject, &CurrentOffset);
    Status = FltCD->IoStatus.Status;

_exit:
    if(NULL!=FltCD->Iopb->Parameters.SetFileInformation.InfoBuffer) ExFreePool(FltCD->Iopb->Parameters.SetFileInformation.InfoBuffer);
    if(NULL!=FltCD) FltFreeCallbackData(FltCD);
    return Status;
}

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
                 )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  FltCD  = NULL;
    ULONG               Flags  = IRP_READ_OPERATION;
    LARGE_INTEGER       CurrentOffset = {0};

    if(Paging)
        Flags |= (IRP_NOCACHE|IRP_PAGING_IO|IRP_SYNCHRONOUS_PAGING_IO);
    else
        Flags |= (NonCached?IRP_NOCACHE:IRP_BUFFERED_IO);

    Status =FltAllocateCallbackData(Instance, FileObject, &FltCD);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Fill IO Data
    FltCD->RequestorMode       = KernelMode;
    FltCD->Iopb->MajorFunction = IRP_MJ_READ;
    FltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    FltCD->Iopb->IrpFlags      = Flags;
    FltCD->Iopb->Parameters.Read.ByteOffset.QuadPart= ReadOffset->QuadPart;
    FltCD->Iopb->Parameters.Read.Length             = *ReadLength;
    FltCD->Iopb->Parameters.Read.MdlAddress         = NULL;
    FltCD->Iopb->Parameters.Read.ReadBuffer         = ReadBuffer;

    // Perform the sync call
    if(!Paging) Status = NLSEGetCurrentOffsetSync(Instance, FileObject, &CurrentOffset);
    FltPerformSynchronousIo(FltCD);
    if(!Paging && NT_SUCCESS(Status)) NLSESetCurrentOffsetSync(Instance, FileObject, &CurrentOffset);
    Status      = FltCD->IoStatus.Status;
    *ReadLength = (ULONG)FltCD->IoStatus.Information;

_exit:
    if(NULL!=FltCD) FltFreeCallbackData(FltCD);
    return Status;
}

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
                  )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PFLT_CALLBACK_DATA  FltCD  = NULL;
    ULONG               Flags  = IRP_WRITE_OPERATION;
    LARGE_INTEGER       CurrentOffset = {0};

    if(Paging)
        Flags |= (IRP_NOCACHE|IRP_PAGING_IO|IRP_SYNCHRONOUS_PAGING_IO);
    else
        Flags |= (NonCached?IRP_NOCACHE:IRP_BUFFERED_IO);

    Status =FltAllocateCallbackData(Instance, FileObject, &FltCD);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Fill IO Data
    FltCD->RequestorMode       = KernelMode;
    FltCD->Iopb->MajorFunction = IRP_MJ_WRITE;
    FltCD->Iopb->MinorFunction = IRP_MN_NORMAL;
    FltCD->Iopb->IrpFlags      = Flags;
    FltCD->Iopb->Parameters.Write.ByteOffset.QuadPart   = WriteOffset->QuadPart;
    FltCD->Iopb->Parameters.Write.Length                = *WriteLength;
    FltCD->Iopb->Parameters.Write.MdlAddress            = NULL;
    FltCD->Iopb->Parameters.Write.WriteBuffer           = WriteBuffer;

    // Perform the sync call
    if(!Paging) Status = NLSEGetCurrentOffsetSync(Instance, FileObject, &CurrentOffset);
    FltPerformSynchronousIo(FltCD);
    if(!Paging && NT_SUCCESS(Status)) NLSESetCurrentOffsetSync(Instance, FileObject, &CurrentOffset);
    Status      = FltCD->IoStatus.Status;
    *WriteLength= (ULONG)FltCD->IoStatus.Information;

_exit:
    if(NULL!=FltCD) FltFreeCallbackData(FltCD);
    return Status;
}

__drv_maxIRQL(APC_LEVEL)
VOID
NLSEPurgeFileCache(
                   __in PFILE_OBJECT FileObject,
                   __in BOOLEAN bIsFlushCache,
                   __in_opt PLARGE_INTEGER FileOffset,
                   __in ULONG Length
                   )
{
	BOOLEAN PurgeRes            = FALSE;
	BOOLEAN ResourceAcquired    = FALSE;
	BOOLEAN PagingIoResourceAcquired  = FALSE;
	PFSRTL_COMMON_FCB_HEADER Fcb      = NULL;
	LARGE_INTEGER Delay50Milliseconds = {(ULONG)(-50 * 1000 * 10), -1};
	IO_STATUS_BLOCK IoStatus = {0} ;
    INT         Loop = 0;

    // Sanity check
	if (FileObject == NULL) return;

    Fcb = (PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext ;
	if (Fcb == NULL) return ;
	
Acquire:
    Loop++;
	FsRtlEnterFileSystem() ;

	if (Fcb->Resource) ResourceAcquired = ExAcquireResourceExclusiveLite(Fcb->Resource, TRUE) ;
	if (Fcb->PagingIoResource) PagingIoResourceAcquired = ExAcquireResourceExclusiveLite(Fcb->PagingIoResource,FALSE);
	else PagingIoResourceAcquired = TRUE ;

	if (!PagingIoResourceAcquired)
	{
		if (Fcb->Resource)  ExReleaseResource(Fcb->Resource);
		FsRtlExitFileSystem();
        if(Loop < 20)
        {
		    KeDelayExecutionThread(KernelMode,FALSE,&Delay50Milliseconds);	
		    goto Acquire;	
        }
        else
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"NLSE: Purge Cache >> Fail to lock resource\n");
            return;
        }
	}

	if(FileObject->SectionObjectPointer)
	{
		IoSetTopLevelIrp( (PIRP)FSRTL_FSP_TOP_LEVEL_IRP );

		if (bIsFlushCache)
		{
			CcFlushCache( FileObject->SectionObjectPointer, FileOffset, Length, &IoStatus );
		}

		if(FileObject->SectionObjectPointer->ImageSectionObject)
			MmFlushImageSection(FileObject->SectionObjectPointer, MmFlushForWrite) ;

		if(FileObject->SectionObjectPointer->DataSectionObject)
			PurgeRes = CcPurgeCacheSection(FileObject->SectionObjectPointer, NULL, 0, FALSE);
                                      
		IoSetTopLevelIrp(NULL);                                   
	}

	if (Fcb->PagingIoResource) ExReleaseResourceLite(Fcb->PagingIoResource );                                       
	if (Fcb->Resource) ExReleaseResourceLite(Fcb->Resource );
	FsRtlExitFileSystem() ;
}

VOID
NLSEGenAesKey(
              __out_bcount_full(16) PUCHAR AesKey
              )
{
  ULONG key[4]={0}; //128 bit long

  ASSERT( NULL != AesKey );

#if NLSE_DEBUG_FAKE_FILE_KEY

  key[0] = (ULONG)'z';
  key[1] = (ULONG)'z';
  key[2] = (ULONG)'z';
  key[3] = (ULONG)'z';

#else

  nlfseGlobal.crypto_ctx.rand(&nlfseGlobal.crypto_ctx, (int*)&key[0]); /* random ULONG w/o presisted key */
  nlfseGlobal.crypto_ctx.rand(&nlfseGlobal.crypto_ctx, (int*)&key[1]); /* random ULONG w/o presisted key */
  nlfseGlobal.crypto_ctx.rand(&nlfseGlobal.crypto_ctx, (int*)&key[2]); /* random ULONG w/o presisted key */
  nlfseGlobal.crypto_ctx.rand(&nlfseGlobal.crypto_ctx, (int*)&key[3]); /* random ULONG w/o presisted key */

#endif

  RtlCopyMemory(AesKey, key, 16);  
}


BOOLEAN
IsIgnoredFiles(
               __in PCWCH FileName,
               __in ULONG FileNameLength,
               __in BOOLEAN InProgramDir
               )
{
    INT i = 0;

    do
    {
        if(FileNameLength <= IGNORED_FILE_EXTS[i].Length)
            continue;
        if(0 == NLCompareName(FileName, IGNORED_FILE_EXTS[i].Length, IGNORED_FILE_EXTS[i].Buffer, IGNORED_FILE_EXTS[i].Length))
            return TRUE;
    } while(IGNORED_FILE_EXTS[++i].Length > 0);

    if(InProgramDir)
    {
        i = 0;
        do
        {
            if(FileNameLength <= IGNORED_PEFILE_EXTS[i].Length)
                continue;
            if(0 == NLCompareName(FileName, IGNORED_PEFILE_EXTS[i].Length, IGNORED_PEFILE_EXTS[i].Buffer, IGNORED_PEFILE_EXTS[i].Length))
                return TRUE;
        } while(IGNORED_PEFILE_EXTS[++i].Length > 0);
    }

    return FALSE;
}

__checkReturn
ULONG
NLCheckDiretory(
                __in PCUNICODE_STRING ParentDir, /*Without Volume*/
                __in PCUNICODE_STRING FileName
                )
{
    ULONG          ulRet   = 0;
    UNICODE_STRING DirName = {0, 0, NULL};

    DirName.MaximumLength = ParentDir->Length + FileName->Length + sizeof(WCHAR);
    DirName.Length = 0;
    DirName.Buffer = ExAllocatePoolWithTag(NonPagedPool, DirName.MaximumLength, 'esln');
    if(NULL == DirName.Buffer)
        return ulRet;

    RtlZeroMemory(DirName.Buffer, DirName.MaximumLength);
    RtlCopyUnicodeString(&DirName, ParentDir);
    if(FileName->Length) RtlAppendUnicodeStringToString(&DirName, FileName);

    ulRet = NLCheckDiretoryEx(&DirName);
    ExFreePool(DirName.Buffer); DirName.Buffer=NULL;
    return ulRet;
}

__checkReturn
ULONG
NLCheckDiretoryEx(
                  __in PCUNICODE_STRING DirPath /*Without Volume*/
                  )
{
    ULONG   ulRet = 0;

    if(RtlPrefixUnicodeString(&IGNOREDDIR_BOOT, DirPath, TRUE))
        ulRet |= NL_IGNORE_DIRECTORY;
    if(RtlPrefixUnicodeString(&IGNOREDDIR_WINDOWS, DirPath, TRUE))
        ulRet |= NL_IGNORE_DIRECTORY;
    if(RtlPrefixUnicodeString(&IGNOREDDIR_RECYCLER, DirPath, TRUE))
        ulRet |= NL_IGNORE_DIRECTORY;
    if(RtlPrefixUnicodeString(&IGNOREDDIR_SYSVOL, DirPath, TRUE))
        ulRet |= NL_IGNORE_DIRECTORY;
    if(RtlPrefixUnicodeString(&IGNOREDDIR_NEXTLABS, DirPath, TRUE))
        ulRet |= NL_IGNORE_DIRECTORY;
    if(RtlPrefixUnicodeString(&IGNOREDDIR_VMWARE1, DirPath, TRUE))
        ulRet |= NL_IGNORE_DIRECTORY;
    if(RtlPrefixUnicodeString(&IGNOREDDIR_VMWARE2, DirPath, TRUE))
        ulRet |= NL_IGNORE_DIRECTORY;

    if(RtlPrefixUnicodeString(&PROGRAMDIR, DirPath, TRUE))
        ulRet |= NL_PROGRAM_DIRECTORY;

    return ulRet;
}
