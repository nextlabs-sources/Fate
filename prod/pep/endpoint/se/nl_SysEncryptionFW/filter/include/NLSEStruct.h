/*++

Module Name:

    NLSEStruct.h

Abstract:

    This is the header file defining the data structures used by the 
    kernel mode filter driver implementing transparent file system encryption


Environment:

    Kernel mode


--*/
#ifndef __NLSE_STRUCT_H__
#define __NLSE_STRUCT_H__

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntddk.h>
#include "NLSECommon.h"
#include "nl_klog.h"
#include "nl_crypto.h"

/*************************************************************************
    constant
*************************************************************************/
#define NLFSE_ADS_SUFFIX_LEN            16
#define NLFSE_NAME_LEN                  NLSE_MAX_PATH
#define NLFSE_MAX_WORK_QUEUE_SIZE       32
#define NLSE_PORT_NAME                  L"\\NLSEFastWritePort"
#define NLSE_KEY_CACHE_TIME             300 //5 minutes
#define NLSE_DELAY_ONE_MICROSECOND      (-10)
#define NLSE_DELAY_ONE_MILLISECOND      (NLSE_DELAY_ONE_MICROSECOND*1000)
#define NLSE_DELAY_ONE_SECOND           (NLSE_DELAY_ONE_MILLISECOND*1000)

#ifndef MAX_PATH
#define MAX_PATH    260
#endif

/************************************************************************/
// Define DEBUG Flags
/************************************************************************/
#if defined(DBG)

/************************************************************************/
// In debug mode, developer can enable/disable these flags
/************************************************************************/
#define NLSE_DEBUG_PERFORMANCE_COUNT        FALSE
#define NLSE_DEBUG_FAKE_FILE_KEY            FALSE
#define NLSE_DEBUG_DATA_VERIFY              FALSE
#define NLSE_DEBUG_DATA_VERIFY_BUGCHECK     FALSE
#define NLSE_DEBUG_CRYPTO_PASSTHROUGH       FALSE

#else

/************************************************************************/
// In release mode, these flags should always be FALSE
// DON'T change them
/************************************************************************/
#define NLSE_DEBUG_PERFORMANCE_COUNT        FALSE
#define NLSE_DEBUG_FAKE_FILE_KEY            FALSE
#define NLSE_DEBUG_DATA_VERIFY              FALSE
#define NLSE_DEBUG_DATA_VERIFY_BUGCHECK     FALSE
#define NLSE_DEBUG_CRYPTO_PASSTHROUGH       FALSE

#endif

/*************************************************************************
    Pool Tags
*************************************************************************/

#define NLFSE_BUFFER_TAG             'tbES'
#define NLFSE_CONTEXT_TAG            'tcES'
#define NLFSE_NAME_TAG               'tnES'
#define NLFSE_PRE_2_POST_TAG         'ppES'
#define NLFSE_QUEUE_CONTEXT_TAG      'tqES'
#define NLFSE_IRP_ENTRY_TAG          'eiES'
#define NLSE_POLICY_TAG              'tpES'
#define NLSE_MESSAGE_TAG             'tmES'
#define NLSE_ENCRYPT_FILE_QUEUE_TAG  'qeES'
#define NLSE_RAW_ACCESS_TAG          'arES'

#define NLSE_GETADSHANDLE_TAG        '0 ES'
#define NLSE_ACOPYSTRING_TAG         '1 ES'
#define NLSE_FILENONUNC_TAG          '2 ES'
#define NLSE_COMPOSEDIRTOVOL_TAG     '3 ES'
#define NLSE_FILENONUNCNAMEINFO_TAG  '4 ES'
#define NLSE_OPENREADADS_TAG         '5 ES'
#define NLSE_CHECKDIRENCATTR_TAG     '6 ES'
#define NLSE_GETENCADS_TAG           '7 ES'
#define NLSE_CREATEADSPOST_TAG       '8 ES'
#define NLSE_DELETEENCADS_TAG        '9 ES'
#define NLSE_FILEOPHELP_TAG          '01ES'
#define NLSE_DRMPATHENTRY_TAG        '11ES'
#define NLSE_DRMPATHNAME_TAG         '21ES'
#define NLSE_CRYPTOKEY_TAG           '31ES'


#ifndef ROUND_TO_SIZE
#define ROUND_TO_SIZE(_length, _alignment)	\
    ((((ULONG_PTR)(_length))+((_alignment)-1))&(~((ULONG_PTR)((_alignment)-1))))
#endif

#ifndef ROUND_TO_SIZE_PRE
#define ROUND_TO_SIZE_PRE(_length, _alignment)	\
    (((ULONG_PTR)(_length))&(~((ULONG_PTR)((_alignment)-1))))
#endif

#ifndef ROUND_TO_SIZE_PADDING
#define ROUND_TO_SIZE_PADDING(_length, _alignment)	\
    (ROUND_TO_SIZE(_length, _alignment)-(_length))
#endif

#ifndef ROUND_TO_SIZE_PRE_EXTRA
#define ROUND_TO_SIZE_PRE_EXTRA(_length, _alignment)	\
    ((_length)-ROUND_TO_SIZE_INNER(_length, _alignment))
#endif

/*************************************************************************
    Local structures
*************************************************************************/
/** NLFSE_IRP_ENTRY
 *
 *  Information of queried IRP entry
 */
typedef struct _NLFSE_IRP_ENTRY
{
  UNICODE_STRING		fileParentDir;
  UNICODE_STRING		fileNameFinal;
  UNICODE_STRING                fileName;
  PNLFSE_ENCRYPT_EXTENSION      pEncryptExtension;
  BOOLEAN                       NeedEncrypt;
  BOOLEAN                       bExist;
}NLFSE_IRP_ENTRY,*NLFSE_PIRP_ENTRY;

//
//  This is a volume context, one of these are attached to each volume
//  we monitor.  This is used to get a "DOS" name for debug display.
//
typedef struct _NLFSE_VOLUME_CONTEXT {

  //  Holds the name to display
  UNICODE_STRING Name;

  //  Holds the sector size for this volume.
  ULONG SectorSize;

  //  Linked list of contexts associated with this volume along with the
  //  lock.
  LIST_ENTRY StreamCtxList;
  ERESOURCE StreamCtxLock;

  //
  //  When renaming a directory there is a window where the current names
  //  in the context cache may be invalid.  To eliminate this window we
  //  increment this count every time we start doing a directory rename
  //  and decrement this count when it is completed.  When this count is
  //  non-zero then we query for the name every time so we will get a
  //  correct name for that instance in time.
  //
  ULONG AllStreamContextsTemporary;

} NLFSE_VOLUME_CONTEXT, *NLFSE_PVOLUME_CONTEXT;

//minmum sector size of each volume
#define MIN_SECTOR_SIZE 0x200

//
//  Instance context data structure
//
typedef struct _NLFSE_INSTANCE_CONTEXT {

  //
  //  Instance for this context.
  //
  PFLT_INSTANCE Instance;

  //  Flag to control the life/death of the pre-create work item thread
  //  volatile LONG preCreateWorkerThreadFlag;

  //  Notify the worker thread that the instance is being torndown
  //
  KEVENT TeardownEvent;

} NLFSE_INSTANCE_CONTEXT, *NLFSE_PINSTANCE_CONTEXT;

//
//  Context specific flags
//
typedef enum _NLFSE_CTX_FLAGS {

    //
    //  If set, then we are currently linked into the device extension linked
    //  list.
    //
    CTXFL_InVolumeContextList       = 0x00000001,

    //
    //  If set, then we are linked into the stream list.  Note that there is
    //  a small period of time when we might be unlinked with this flag still
    //  set (when the file system is calling SpyDeleteContextCallback).  This is
    //  fine because we still handle not being found in the list when we do
    //  the search.  This flag handles the case when the file has been
    //  completely closed (and the memory freed) on us.
    //
    CTXFL_InStreamList          = 0x00000002,


    //
    //  If set, this is a temporary context and should not be linked into
    //  any of the context lists.  It will be freed as soon as the user is
    //  done with this operation.
    //
    CTXFL_Temporary             = 0x00000100,

    //
    //  If set, we are performing a significant operation that affects the state
    //  of this context so we should not use it.  If someone tries to get this
    //  context then create a temporary context and return it.  Cases where this
    //  occurs:
    //  - Source file of a rename.
    //  - Source file for the creation of a hardlink
    //
    CTXFL_DoNotUse              = 0x00000200

} NLFSE_CTX_FLAGS, *PNLFSE_CTX_FLAGS;

//
//  Structure for tracking an individual stream context.  Note that the buffer
//  for the FileName is allocated as part of this structure and follows
//  immediately after it.
//
typedef struct _NLFSE_STREAM_CONTEXT
{
  //  OS Structure used to track contexts per stream.  Note how we use
  //  the following fields:
  //      OwnerID     -> Holds pointer to our volume context
  //      InstanceId  -> Holds Pointer to FsContext associated
  //                     with this structure
  //  We use these values to get back to these structures
  FSRTL_PER_STREAM_CONTEXT ContextCtrl;

  //
  //  Linked list used to track contexts per volume
  //
  LIST_ENTRY VolumeContextLink;

  //
  //  This is a counter of how many threads are currently using this
  //  context.  The count is used in this way:
  //  - It is set to 1 when it is created.
  //  - It is incremented every time it is returned to a thread
  //  - It is decremented when the thread is done with it.
  //  - It is decremented when the underlying stream that is using it is freed
  //  - The context is deleted when this count goes to zero
  //
  LONG UseCount;

  //  Holds the name of the file
  UNICODE_STRING FileName;

  //  Flags for this context.  All flags are set or cleared via
  //  the interlocked bit routines except when the entry is being
  //  created, at this time we know nobody is using this entry.
  NLFSE_CTX_FLAGS Flags;

  //  Contains the FsContext value for the stream we are attached to.  We
  //  track this so we can delete this entry at any time.
  PFSRTL_ADVANCED_FCB_HEADER Stream;

  //Encryption flag
  BOOLEAN NeedEncrypt; //If true, the file need to be encrypted

  //Ignore write Flag
  BOOLEAN IgnoreWrite;

  //stored encryption extension
  KSPIN_LOCK               encryptExtLock; //lock to encryption extension
  PNLFSE_ENCRYPT_EXTENSION encryptExt; 

  //Indicator that the file is being deleted
  FAST_MUTEX deleteFlagLock;
  BOOLEAN    bDelete;

  // FileSize
  LARGE_INTEGER FileSize;
} NLFSE_STREAM_CONTEXT, *PNLFSE_STREAM_CONTEXT;

typedef struct _NLFSE_ADS_WORKITEM
{
    LIST_ENTRY              adsWorkerThreadList;
    PFLT_INSTANCE           fltInstance;
    ULONG                   RetryCount;
    ULONG                   ProcessId;
    ULONG                   SectorSize;
    UNICODE_STRING          hostFilePath;
    WCHAR                   hostFilePathBuffer[MAX_PATH*2];
    NLFSE_ENCRYPT_EXTENSION encryptExt;
}NLFSE_ADS_WORKITEM, *PNLFSE_ADS_WORKITEM;

typedef struct _NLFSE_DRM_PATH_ENTRY
{
  LIST_ENTRY        listEntry;
  UNICODE_STRING    path;
} NLFSE_DRM_PATH_ENTRY, *PNLFSE_DRM_PATH_ENTRY;

/************************************************************************/
// PRE TO POST CONTEXT
/************************************************************************/
typedef struct _P2P_SETINFORMATION_EOF_PARAMETERS
{
    LARGE_INTEGER FileSizeBeforeSet;
    LARGE_INTEGER FileSizeAfterSet;
    PUCHAR        LastBlock;
    LARGE_INTEGER LastBlockOffset;
    ULONG         LastBlockValidLength;
    ULONG         EmptyDataLength;
    LARGE_INTEGER EmptyDataOffset;
    ULONG         PaddingLength;
    UCHAR         PaddingData[16];
}P2P_SETINFORMATION_EOF_PARAMETERS, *PP2P_SETINFORMATION_EOF_PARAMETERS;

typedef struct _P2P_SETINFORMATION_BASIC_PARAMETERS
{
    FILE_BASIC_INFORMATION fbi;
}P2P_SETINFORMATION_BASIC_PARAMETERS, *PP2P_SETINFORMATION_BASIC_PARAMETERS;

typedef struct _P2P_SETINFORMATION_STANDARD_PARAMETERS
{
    FILE_STANDARD_INFORMATION fsi;
}P2P_SETINFORMATION_STANDARD_PARAMETERS, *PP2P_SETINFORMATION_STANDARD_PARAMETERS;

typedef struct _P2P_SETINFORMATION_RENAME_PARAMETERS
{
    ULONG   Reserved;
}P2P_SETINFORMATION_RENAME_PARAMETERS, *PP2P_SETINFORMATION_RENAME_PARAMETERS;

typedef struct _P2P_SETINFORMATION_DISPOSITION_PARAMETERS
{
    ULONG   Reserved;
}P2P_SETINFORMATION_DISPOSITION_PARAMETERS, *PP2P_SETINFORMATION_DISPOSITION_PARAMETERS;

typedef struct _P2P_SETINFORMATION_PARAMETERS
{
    FILE_INFORMATION_CLASS  FileInformationClass;
    union
    {
        P2P_SETINFORMATION_EOF_PARAMETERS           EndOfFile;
        P2P_SETINFORMATION_BASIC_PARAMETERS         BasicInformation;
        P2P_SETINFORMATION_STANDARD_PARAMETERS      StdInformation;
        P2P_SETINFORMATION_RENAME_PARAMETERS        Rename;
        P2P_SETINFORMATION_DISPOSITION_PARAMETERS   Disposition;
    }Information;
}P2P_SETINFORMATION_PARAMETERS, *PP2P_SETINFORMATION_PARAMETERS;

typedef struct _P2P_WRITE_PARAMETERS
{
    LARGE_INTEGER   OriginalOffset;
    ULONG           OriginalLength;
    PUCHAR          SwapBuffer;
    PMDL            SwapMdl;
    LARGE_INTEGER   NewOffset;
    ULONG           NewLength;
    ULONG           NewWriteLength;
    LARGE_INTEGER   FSizeBeforeWrite;
}P2P_WRITE_PARAMETERS, *PP2P_WRITE_PARAMETERS;

typedef struct _P2P_READ_PARAMETERS
{
    ULONG   Reserved;
}P2P_READ_PARAMETERS, *PP2P_READ_PARAMETERS;

typedef struct _PRETOPOST_CONTEXT
{
    ULONG                   ProcessId;
    UCHAR                   MajorFunction;
    UCHAR                   MinorFunction;
    ULONG                   SectorSize;
    PNLFSE_STREAM_CONTEXT   psCtx;
    union
    {
        P2P_SETINFORMATION_PARAMETERS SetInformationFile;
        P2P_WRITE_PARAMETERS          Write;
        P2P_READ_PARAMETERS           Read;
    } Parameters;
}PRETOPOST_CONTEXT, *PPRETOPOST_CONTEXT;

//
//  This is a context structure that is used to pass state from our
//  pre-operation callback to our post-operation callback.
typedef struct _NLFSE_PRE_2_POST_CONTEXT 
{
  //For handling delayed ads work in ads worker thread list
  LIST_ENTRY                    adsWorkerThreadList;

  //  Pointer to our volume context structure.  We always get the context
  //  in the preOperation path because you can not safely get it at DPC
  //  level.  We then release it in the postOperation path.  It is safe
  //  to release contexts at DPC level.
  //
  NLFSE_PVOLUME_CONTEXT         VolCtx;

  //
  //  Since the post-operation parameters always receive the "original"
  //  parameters passed to the operation, we need to pass our new destination
  //  buffer to our post operation routine so we can free it.
  //
  PVOID                         SwappedBuffer;

  //Per stream context
  PNLFSE_STREAM_CONTEXT         streamCtx; 

  //pointer to drvier instance context
  NLFSE_PINSTANCE_CONTEXT       instanceCtx;

  //pointer to the filter instance
  PFLT_INSTANCE                 fltInstance;

  //process id of this operation
  ULONG                         pid;


  ULONG                         sectorSize;
  //support CBC block cipher mode in READ operation
  LARGE_INTEGER                 newOffset; //new read offset
  ULONG                         newSize; //new read size
  ULONG                         cryptoSize; //crypto size 
  ULONG                         realReadSize; //really read-in data size
  ULONG                         originalLen; //orignal length
 
  //support FileRename operation
  PFLT_FILE_NAME_INFORMATION    fileNameInfo; //source file name info
  BOOLEAN                       bSourceEncrypted; //if true, source is encrypted

  //support CBC encryption padding for Write and SetEndOfFile operation
  LARGE_INTEGER                 cryptoOffset; //start offset of crypto buffer
  LARGE_INTEGER                 cryptoLen; //length of data in crypto buffer
  LARGE_INTEGER                 cryptoBufferSize; //crypto buffer size; aligned with sector
  PVOID                         cryptoBuffer; //Buffer of crypto
  ULONG                         paddingLen; //the length of padding
  PVOID                         paddingBuf; //the pointer to buffer storing padding

  // A duplicate encrypt extension, used for presetfileinfo to post set file info
  NLFSE_ENCRYPT_EXTENSION       dupEncryptExtension;
} NLFSE_PRE_2_POST_CONTEXT, *NLFSE_PPRE_2_POST_CONTEXT;

//
//  Pending IO Queue context data structure
//
typedef struct _PENDING_IO_QUEUE_CONTEXT {
  FLT_CALLBACK_DATA_QUEUE_IO_CONTEXT CbdqIoContext;
  PCFLT_RELATED_OBJECTS FltObjects;//pending irp flt objects
  union {
    NLFSE_PIRP_ENTRY irpEntry; //pending IRP record
    NLFSE_PPRE_2_POST_CONTEXT p2pCtx; //pending write IRP record
  };
  union {
    FLT_POST_OPERATION_FLAGS postFlags;
  };
} NLSE_PENDING_IO_QUEUE_CONTEXT, *NLSE_PENDING_IO_QUEUE_PCONTEXT;

//
//  context of worker thread queue to encrypt file
//
typedef struct _ENCRYPT_FILE_QUEUE_CONTEXT {
  PFLT_INSTANCE              filterInstance;
  PNLFSE_STREAM_CONTEXT      fileStreamCtx; //per stream context 
  PFLT_FILE_NAME_INFORMATION file; //encrypted file information 
  ULONG                      pid; //process ID
  PFLT_CALLBACK_DATA         data;
} NLSE_ENCRYPT_FILE_QUEUE_CONTEXT, *NLSE_ENCRYPT_FILE_QUEUE_PCONTEXT;

typedef struct _NLFSE_GLOBAL_DATA {
  PFLT_FILTER  filterHandle;   //handle of this filter

  //Lookasid buffer
  NPAGED_LOOKASIDE_LIST queueContextLookaside; //pending io context lookaside
  NPAGED_LOOKASIDE_LIST pre2PostContextList; //pre-post-operation context
  NPAGED_LOOKASIDE_LIST fileEncryptCtxList; //file encrypt queue context
  NPAGED_LOOKASIDE_LIST irpEntryLookaside; //lookaside buffer of irp entry

  //worker thread
  LIST_ENTRY adsWorkQueue; //ads work queue 
  KSPIN_LOCK adsWorkQueueSpinLock; //lock to ads work queue
  KEVENT     adsWorkerThreadEvent; //event of ads worker thread
  LONG       adsWorkQueueSize; //size of updating encryption ADS queue
  LONG       encryptWorkQueueSize; //size of file encryption queue

  //Communication Port
  PFLT_PORT         serverPort;
  //[0] has to be the main client port; 
  //the rests are for DRM setting
  NLSE_PORT_CONTEXT clientPorts[NLSE_MAX_PORT_CONNECTION];

  //crypto
  FAST_MUTEX  currentPCKeyLock;
  BOOLEAN     hasCurrentPCKey; //If true, we have a valid current PC key
  char        currentPCKey[NLSE_KEY_LENGTH_IN_BYTES];//Policy Controller key 
  NLSE_KEY_ID currentPCKeyID; //PC key ID 
  ULONG       keyRefreshTime; //time to try to get the current PC key again (in
                              //seconds since 1/1/1980)
  ULONG       cryptoBlockSize; //size of block in which CBC blocks associated
  ULONG       cbcBlockSize; //size of CBC block that a block cipher operates on
  ULONG       maxWriteBlockSize; // the max length of data that driver can write  in one time

  struct nl_crypto_context crypto_ctx;
  BOOLEAN CryptoInit, CryptoInitSuccess;

  //mini-filter status
  BOOLEAN bEnable; //If true, driver do encryption/decryption
  FAST_MUTEX enableStatusMutex; //synchronization protection
  BOOLEAN log_file_set; // Is log file setup?

  //DRM path list
  ERESOURCE     drmPathListLock;
  LIST_ENTRY    drmPathList;
} NLFSE_GLOBAL_DATA;


BOOLEAN
encrypt_buffer(__in char  *encryptKey, 
               __in size_t keyLen,
               __in ULONGLONG startOffset,
               __inout PVOID cryptoBuffer, 
               __in size_t cryptoSize
               );

BOOLEAN
decrypt_buffer(__in char  *decryptKey, 
               __in size_t keyLen,
               __in ULONGLONG startOffset,
               __inout PVOID cryptoBuffer, 
               __in size_t cryptoSize
               );

#endif
