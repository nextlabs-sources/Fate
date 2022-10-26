#ifndef __NL_FILE_NAME_INFO_CACHE_H__
#define __NL_FILE_NAME_INFO_CACHE_H__

#include <ntifs.h>

typedef struct _NLFNINFO
{
  LIST_ENTRY            Entry;
  LARGE_INTEGER         AddedTime;              /* time when file was added to
                                                   cache */
  UNICODE_STRING        OpenedFileName;         /* non-normalized name*/
  UNICODE_STRING        NormalizedFileName;     /* normalized name */
  ULONG                 FileNameType;
} NLFNINFO, *PNLFNINFO;

typedef struct _NLFNINFOLIST
{
  LIST_ENTRY            ListHead;
  ULONG                 Count;
  FAST_MUTEX            Lock;
} NLFNINFOLIST, *PNLFNINFOLIST;



#define NL_FNINFO_CACHE_SIZE_LIMIT      1000    /* # of files */
#define NL_FNINFO_CACHE_TIME_LIMIT_MS   1000    /* # of milliseconds */



VOID
NLFNInfoCacheInit(__inout PNLFNINFOLIST);

VOID
NLFNInfoCacheDestroy(__inout PNLFNINFOLIST);

NTSTATUS
NLFNInfoCacheAdd(__inout PNLFNINFOLIST list,
                 __in PUNICODE_STRING openedFileName,
                 __in PUNICODE_STRING fullFileName,
                 __in ULONG fileNameType);

NTSTATUS
NLFNInfoCacheFind(__in PNLFNINFOLIST list,
                  __in PUNICODE_STRING openedFileName,
                  __out PUNICODE_STRING fullFileName,
                  __out PULONG fileNameType);

#endif /* __NL_FILE_NAME_INFO_CACHE_H__ */
