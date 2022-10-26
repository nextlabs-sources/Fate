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
#include "NLSECommon.h"
#include "nl_klog.h"
#include "nl_crypto.h"
#include "nldircache.h"
#include "NLFileNameInfoCache.h"
#include "pathList.h"

/*************************************************************************
    constant
*************************************************************************/
#define NLFSE_NAME_LEN                  NLSE_MAX_PATH
#define NLSE_PORT_NAME                  L"\\NLSysEncryptionPort"
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
#define NLSE_DEBUG_FAKE_PC_KEY              FALSE
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
#define NLSE_DEBUG_FAKE_PC_KEY              FALSE
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
#define NLSE_MESSAGE_TAG             'tmES'
#define NLSE_RAW_ACCESS_TAG          'arES'

#define NLSE_CRYPTOKEY_TAG           '1 ES'
#define NLSE_CHECKDIRENCATTR_TAG     '2 ES'
#define NLSE_FILENAMEINFOCACHE_TAG   '3 ES'
#define NLSE_PATHENTRY_TAG           '4 ES'
#define NLSE_PATHNAME_TAG            '5 ES'


#ifndef ROUND_TO_SIZE
#define ROUND_TO_SIZE(_length, _alignment)	\
    ((((ULONG_PTR)(_length))+((_alignment)-1))&(~((ULONG_PTR)((_alignment)-1))))
#endif

/*************************************************************************
    Local structures
*************************************************************************/
//
//  This is a volume context, one of these are attached to each volume
//  we monitor.  This is used to get a "DOS" name for debug display.
//
typedef struct _NLFSE_VOLUME_CONTEXT {

  //  Holds the name to display
  UNICODE_STRING Name;

  //  Holds the dos name to display
  UNICODE_STRING DosName;

  //  Holds the sector size for this volume.
  ULONG SectorSize;

  //
  //  When renaming a directory there is a window where the current names
  //  in the context cache may be invalid.  To eliminate this window we
  //  increment this count every time we start doing a directory rename
  //  and decrement this count when it is completed.  When this count is
  //  non-zero then we query for the name every time so we will get a
  //  correct name for that instance in time.
  //
  ULONG AllStreamContextsTemporary;

  NLDINFOLIST   DInfoList;
  NLFNINFOLIST  FNInfoCache;

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
  BOOLEAN                  encryptExtDirty;

  //Indicator that the file is being deleted
  FAST_MUTEX deleteFlagLock;
  BOOLEAN    bDelete;

  // FileSize
  LARGE_INTEGER FileSize;
  // File Attributes
  ULONG           FileAttrs;
  // Sectorsize of the volume where the file is in
  ULONG           SectorSize;
} NLFSE_STREAM_CONTEXT, *PNLFSE_STREAM_CONTEXT;

typedef struct _NLFSE_DRM_PATH_ENTRY
{
  LIST_ENTRY        listEntry;
  UNICODE_STRING    path;
} NLFSE_DRM_PATH_ENTRY, *PNLFSE_DRM_PATH_ENTRY;


//
//  context of worker thread queue to encrypt file
//
typedef struct _NLFSE_GLOBAL_DATA {
  PFLT_FILTER  filterHandle;   //handle of this filter

  //worker thread
  LONG       encryptWorkQueueSize; //size of file encryption queue

  //Communication Port
  PFLT_PORT         serverPort;
  //[0] has to be the main client port; 
  //the rests are for DRM setting
  NLSE_PORT_CONTEXT clientPorts[NLSE_MAX_PORT_CONNECTION];

  //crypto
  FAST_MUTEX  currentPCKeyLock;
  BOOLEAN     hasCurrentPCKey; //If true, we have a valid current PC key
  char        currentPCKey[NLE_KEY_LENGTH_IN_BYTES];//Policy Controller key 
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
  NL_PATH_LIST  drmPathList;

  //One-shot DRM file list
  NL_PATH_LIST  drmFileOneShotList;
} NLFSE_GLOBAL_DATA;


BOOLEAN
encrypt_buffer(__in_bcount(keyLen) char  *encryptKey, 
               __in size_t keyLen,
               __in ULONGLONG startOffset,
               __inout PVOID cryptoBuffer, 
               __in size_t cryptoSize
               );

BOOLEAN
decrypt_buffer(__in_bcount(keyLen) char  *decryptKey, 
               __in size_t keyLen,
               __in ULONGLONG startOffset,
               __inout PVOID cryptoBuffer, 
               __in size_t cryptoSize
               );

#endif
