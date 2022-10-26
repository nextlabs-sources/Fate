

#pragma once
#ifndef __NL_DIRECTORY_CACHE__
#define __NL_DIRECTORY_CACHE__


typedef struct _NLFINFO
{
    LIST_ENTRY      Entry;
    UNICODE_STRING  Name;
    BOOLEAN         Enc;
}NLFINFO, *PNLFINFO;

typedef struct _NLDINFO
{
    LIST_ENTRY      Entry;
    LARGE_INTEGER   Time;
    ULONG           ReferenceCount;
    FAST_MUTEX      ReferenceLock;
    UNICODE_STRING  Path;   // No Volume Name    
    LIST_ENTRY      FInfoList;
    KSPIN_LOCK      FInfoLock;
}NLDINFO, *PNLDINFO;

typedef struct _NLDINFOLIST
{
    LIST_ENTRY  ListHead;
    ULONG       Count;
    KSPIN_LOCK  Lock;
} NLDINFOLIST, *PNLDINFOLIST;

#define NL_MAX_DIRINFO_LIST_SIZE    300


VOID
NLInitDList(
            __inout PNLDINFOLIST DList
            );

VOID
NLFreeDList(
            __inout PNLDINFOLIST DList
            );

PNLDINFO
NLFindInDList(
              __in PNLDINFOLIST DList,
              __in PCUNICODE_STRING Path
              );

PNLFINFO
NLFindInDInfo(
              __in PNLDINFO DInfo,
              __in_z WCHAR* FileName,
              __in USHORT FileNameLength
              );

PNLDINFO
NLCreateDInfo(
              __in PCUNICODE_STRING Path
              );

PNLFINFO
NLCreateFInfo(
              __inout WCHAR* FileName,
              __inout USHORT FileNameLength
              );

VOID
NLInsertFInfo(
              __in PNLDINFO DInfo,
              __in PNLFINFO FInfo
              );

VOID
NLRefDInfo(
           __inout PNLDINFO DInfo
           );

VOID
NLDerefDInfo(
               __inout PNLDINFO DInfo
               );

BOOLEAN
NLRemoveFirstExpiredRecord(
                           __inout PNLDINFOLIST DList
                           );

VOID
NLRemoveAllExpiredRecords(
                         __inout PNLDINFOLIST DList
                         );

VOID
NLRemoveFirstRecord(
                    __inout PNLDINFOLIST DList
                    );

#endif