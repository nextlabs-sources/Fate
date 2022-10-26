// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc.,

// Redwood City CA,

// Ownership remains with Blue Jungle Inc, All rights reserved worldwide.

#ifndef _DSTYPES_H_
#define _DSTYPES_H_

#define MAX_NAME_LENGTH		512
#define MAX_SID_LENGTH		64
#define MAX_PATH_LENGTH		512
#define MAX_USER_LENGTH		512
#define MAX_DRIVE_LETTERS	26


#ifdef Linux

#ifndef BJ_PATH_MAX            
#define BJ_PATH_MAX             PATH_MAX+1
#endif

/* Basic function declaration */
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef CALLBACK
#define CALLBACK  __stdcall
#endif

/* Note that native wchar_t on Linux is different */
#ifndef wchar
#define wchar __u16
#endif

/* Basic Types */
/* Ordered by size, more or less */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef HANDLE
typedef void *HANDLE;
#endif

#ifndef VOID
typedef void VOID;
#endif

#ifndef PVOID
typedef void* PVOID;
#endif

#ifndef BYTE
typedef unsigned char  BYTE;
#endif

#ifndef PBYTE
typedef BYTE *PBYTE;
#endif

#ifndef CHAR
typedef char CHAR,*PCHAR;
#endif

#ifndef UCHAR
typedef unsigned char UCHAR;
#endif

#ifndef WCHAR
typedef short WCHAR;
#endif

#ifndef PWCHAR
typedef WCHAR *PWCHAR;
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

/* DWORD should be int, or we will get into trouble for 64bit */
#ifndef DWORD
typedef unsigned int DWORD;
#endif

#ifndef USHORT
typedef unsigned short USHORT;
#endif

#ifndef SHORT
typedef short SHORT;
#endif

#ifndef PSHORT
typedef SHORT *PSHORT;
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

#ifndef INT
typedef int INT;
#endif

#ifndef UINT32
typedef unsigned int UINT32;
#endif

#ifndef INT32
typedef int INT32;
#endif

#ifndef ULONG
typedef unsigned long ULONG;
#endif

#ifndef PULONG
typedef unsigned long*  PULONG;
#endif

#ifndef LONG
typedef long LONG;
#endif

#ifndef PLONG
typedef LONG *PLONG;
#endif

/* These are the Event type from Window Eventlog */
/* Matching the EventMessages.java class         */

#define EVENTLOG_SUCCESS          0x0000
#define EVENTLOG_ERROR_TYPE       0x0001
#define EVENTLOG_WARNING_TYPE     0x0002
#define EVENTLOG_INFORMATION_TYPE 0x0004
#define EVENTLOG_AUDIT_SUCCESS    0x0008
#define EVENTLOG_AUDIT_FAILURE    0x0010
#define MSG_HEARTBEAT             0x40000100
#define MSG_HEARTBEAT_FAILED      0x80000101
#define MSG_LOG_UPLOAD            0x40000102
#define MSG_LOG_UPLOAD_FAILED     0x80000103
#define MSG_POLICY_UPDATE         0x40000104
#define MSG_PROFILE_UPDATE        0x40000105
#define MSG_SERVICE_STARTED       0x40000106
#define MSG_SERVICE_STOPPED       0x40000107

#endif  /* #ifdef Linux */


// Our approach to is provide tight control
// Only use the one that's define for you

#ifdef WIN32

#ifndef BJ_PATH_MAX
#define BJ_PATH_MAX    MAX_PATH
#endif

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef int BOOL;
#ifndef INT
typedef int INT;
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

#ifndef PUINT
typedef UINT *PUINT;
#endif

#ifndef BYTE
typedef unsigned char  BYTE;
#endif

#ifndef PBYTE
typedef BYTE *PBYTE;
#endif

#ifndef INOUT
#define INOUT
#endif

#ifndef DWORD
typedef unsigned long DWORD;
#endif

#ifndef CALLBACK
#define CALLBACK  __stdcall
#endif

#ifndef WINAPI
#define WINAPI __stdcall
#endif

#endif	//#ifdef WIN32

#ifndef _USHORT
#define _USHORT( field ) USHORT field
#endif	//#ifndef _USHORT

#ifndef _ULONG
#define _ULONG( field ) ULONG field
#endif	//#ifndef _ULONG


//Generic Link list stuffs
/* entry for linked list */
typedef struct DSListEntry {
    struct DSListEntry *pFLink;
    struct DSListEntry *pBLink;
} DS_LIST_ENTRY, *PDS_LIST_ENTRY;

//Defines for Q management functions
#define DS_IFS_TDI_SHARED_OBJECT	0x1
#define DS_IFS_IPC_OBJECT		0x2
#define DS_IFS_FILECTX_OBJECT		0x3
#define DS_IFS_COMPLETIONCTX_OBJECT	0x4

/* DS linked list macros */
#define DS_INITIALIZE_LIST(ListHead)   \
    ( (ListHead)->pFLink = (ListHead)->pBLink = (ListHead) )

#define IS_DS_LIST_EMPTY(ListHead)  \
     ( (ListHead)->pFLink == (ListHead) || \
	 ((ListHead)->pFLink == NULL && (ListHead)->pFLink == NULL))

#define DS_REMOVE_HEAD_LIST(ListHead) \
    (ListHead)->pFLink; { DS_REMOVE_LIST_ENTRY( ((ListHead)->pFLink) ) }

#define DS_GET_LIST_HEAD(ListHead) \
            (ListHead)->pFLink

#define DS_GET_LIST_TAIL(ListHead) \
            (ListHead)->pBLink

#define DS_REMOVE_TAIL_LIST(ListHead) \
    (ListHead)->pBLink; {  DS_REMOVE_LIST_ENTRY( ((ListHead)->pBLink) ) }

#define DS_REMOVE_LIST_ENTRY(Entry) { \
    PDS_LIST_ENTRY _EX_Blink;     \
    PDS_LIST_ENTRY _EX_Flink;     \
    _EX_Flink = (Entry)->pFLink;   \
    _EX_Blink = (Entry)->pBLink;   \
    _EX_Blink->pFLink = _EX_Flink; \
    _EX_Flink->pBLink = _EX_Blink; \
 }

#define DS_INSERT_TAIL_LIST(ListHead, Entry) { \
    PDS_LIST_ENTRY  _EX_Blink;       \
    PDS_LIST_ENTRY  _EX_ListHead;    \
    _EX_ListHead = (ListHead);        \
    _EX_Blink = _EX_ListHead->pBLink; \
    (Entry)->pFLink = _EX_ListHead;   \
    (Entry)->pBLink = _EX_Blink;      \
    _EX_Blink->pFLink = (Entry);      \
    _EX_ListHead->pBLink = (Entry);   \
}

#define DS_INSERT_HEAD_LIST(ListHead, Entry) { \
    PDS_LIST_ENTRY _EX_Flink;        \
    PDS_LIST_ENTRY _EX_ListHead;     \
    _EX_ListHead = (ListHead);        \
    _EX_Flink = _EX_ListHead->pFLink; \
    (Entry)->pFLink = _EX_Flink;      \
    (Entry)->pBLink = _EX_ListHead;   \
    _EX_Flink->pBLink = (Entry);      \
    _EX_ListHead->pFLink = (Entry);   \
 }

// Given an address, data type, and DS list item this
// macro will return a pointer to the item/type containing 
// the DS_LIST_ENTRY.  For example:
//
//    DS_GET_LIST_ITEM_PTR(pEntry, DS_JOB_LIST, JobListEntry)
//
// Will return a pointer to the DS_JOB_LIST item that contains
// this list entry. Essentially a method for getting a pointer
// to the structure that contains the DS_LIST_ENTRY
//
#define DS_GET_LIST_ITEM_PTR(address, type, field) ((type *)( \
                             (PCHAR)(address) - \
                              (ULONG_PTR)(&((type *)0)->field)))

#endif	//ifdef _DSTYPES_H_

