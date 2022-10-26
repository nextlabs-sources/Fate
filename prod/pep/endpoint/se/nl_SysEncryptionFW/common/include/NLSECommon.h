/*++

Module Name:

    NLSECommon.h

Abstract:

    This is the header file defining the common data structures used by the 
    both kernel mode NLSE driver and user mode

Environment:

    Kernel/User mode


--*/
#ifndef __NLSE_COMMON_H__
#define __NLSE_COMMON_H__

#include "SystemEncryption_Types.h"

/*************************************************************************
    constant
*************************************************************************/
#define NLSE_PORT_NAME                     L"\\NLSEFastWritePort"
#define NLSE_KEY_RING_LOCAL                "NL_LOCAL"
#define NLSE_KEY_RING_SHARING              "NL_SHARING"
#define NLSE_STREAM_NAME                   "NLSE"
#define NLSE_PORT_TAG_MAIN_REQUEST         'npmr'
#define NLSE_PORT_TAG_MAIN_CMD             'npmc'
#define NLSE_PORT_TAG_DRM                  'nptd'

#define NLSE_MAX_PATH                      280
#define NLSE_MAX_PORT_CONNECTION           10
#define NLSE_PORT_MAIN_REQUEST_INDEX       0
#define NLSE_PORT_MAIN_CMD_INDEX           1

/*************************************************************************
    data structure
*************************************************************************/

typedef NextLabsFile_Header_t           NextLabsFile_TYPE;
typedef NextLabsFile_StreamHeader_t     NextLabsFile_StreamHeader;
typedef SystemEncryption_KeyID_t        NLSE_KEY_ID;
typedef SystemEncryptionFile_Header_t   NLFSE_ENCRYPT_EXTENSION;
typedef SystemEncryptionFile_Header_t*  PNLFSE_ENCRYPT_EXTENSION;

//data structures for the communication between user and kernel
typedef struct _NLSE_MESSAGE
{
  WCHAR         fname[NLSE_MAX_PATH];  /* File or directory name */
  NLSE_KEY_ID   keyID; //Policy Controller key ID
  char          keyRingName[NLSE_KEY_RING_NAME_MAX_LEN]; //16 bytes key ring name in ascii
  unsigned char key[NLSE_KEY_LENGTH_IN_BYTES]; //128 bit long AES key 
  ULONG         pid;            //process id
  ULONG         result; //result; 0 is successful

  // The following user parameters are based on the command that is being
  // invoked.
  union
  {
    // Parameters for NLSE_USER_COMMAND_CREATE_FILE_RAW
    struct
    {
      IN  ACCESS_MASK   desiredAccess;
      IN  ULONG         fileAttributes;
      IN  ULONG         shareAccess;
      IN  ULONG         createDisposition;
      OUT PHANDLE       pHandle;
      OUT PNTSTATUS     status;
    } createFileRaw;

    // Parameters for NLSE_USER_COMMAND_READ_FILE_RAW
    struct
    {
      IN  HANDLE        handle;
      IN  ULONGLONG     offset;
      IN  ULONG         len;
      __field_bcount_part(bufSize, *bytesRead)
      OUT PVOID         buf;
      IN  ULONG         bufSize;
      OUT PULONG        bytesRead;
      OUT PNTSTATUS     status;
    } readFileRaw;

    // Parameters for NLSE_USER_COMMAND_WRITE_FILE_RAW
    struct
    {
      IN  HANDLE        handle;
      IN  ULONGLONG     offset;
      IN  ULONG         len;
      __field_bcount_part(bufSize, len)
      IN  PVOID         buf;
      IN  ULONG         bufSize;
      OUT PULONG        bytesWritten;
      OUT PNTSTATUS     status;
    } writeFileRaw;

    // Parameters for NLSE_USER_COMMAND_CLOSE_FILE_RAW
    struct
    {
      IN  HANDLE        handle;
      OUT PNTSTATUS     status;
    } closeFileRaw;

    // Parameters for NLSE_USER_COMMAND_SET_DRM_PATHS
    struct
    {
      IN  ULONG         numPaths;
      IN  CONST WCHAR  *paths;          // blob of paths separated by
                                        // null-terminators
    } setDrmPaths;
  } params;
} NLSE_MESSAGE, *PNLSE_MESSAGE;
 
typedef struct _NLSE_USER_CMD
{
  int          type;  /* Command type */
  NLSE_MESSAGE msg;   /* message */
} NLSE_USER_COMMAND, *PNLSE_USER_COMMAND;

typedef struct _NLSE_KERNEL_REQUEST
{
  FILTER_MESSAGE_HEADER header;

  int           type;  /* Command type */
  NLSE_MESSAGE  msg;   /* message */
} NLSE_KERNEL_REQUEST, *PNLSE_KERNEL_REQUEST;
  
typedef struct _NLSE_KERNEL_REQUEST_RESPONSE
{
  FILTER_REPLY_HEADER replyHeader;

  NLSE_MESSAGE  msg;   /* message */
}NLSE_KERNEL_REQUEST_RESPONSE, *PNLSE_KERNEL_REQUEST_RESPONSE;

/* User Command Type */
enum
{
  NLSE_USER_COMMAND_ENABLE_FILTER, //enable filter driver functionality
  NLSE_USER_COMMAND_DISABLE_FILTER, //disable filter driver functionality
  NLSE_USER_COMMAND_SET_DRM_FILE, //set DRM attribute on a file 
  NLSE_USER_COMMAND_SET_IGNORE_PROCESS_BY_PID, //ignore process's io
  NLSE_USER_COMMAND_UNSET_IGNORE_PROCESS_BY_PID, //unset ignore process
  NLSE_USER_COMMAND_CREATE_FILE_RAW, //create or open a file in raw mode
  NLSE_USER_COMMAND_READ_FILE_RAW, //read a file in raw mode
  NLSE_USER_COMMAND_WRITE_FILE_RAW, //write to a file in raw mode
  NLSE_USER_COMMAND_CLOSE_FILE_RAW, //close a file opened in raw mode
  NLSE_USER_COMMAND_SET_DRM_PATHS //set the list of DRM paths
};

/* Kernel Request Type */
enum {
  NLSE_KERNEL_REQ_GET_KEY //get policy controller encryption key
};

/* Kernel Request Response Result */
enum
{
  NLSE_KERNEL_REQ_RESULT_SUCCESS,
  NLSE_KERNEL_REQ_RESULT_UNTRUSTED, //request denied because not trusted
  NLSE_KERNEL_REQ_RESULT_SDK_FAILURE //CE SDK call failed
};

typedef struct _NLSE_PORT_CONTEXT
{
  PVOID          port;
  ULONG          portTag;
}NLSE_PORT_CONTEXT, *PNLSE_PORT_CONTEXT;

#endif
