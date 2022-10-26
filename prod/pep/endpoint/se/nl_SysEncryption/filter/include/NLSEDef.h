/*++

Module Name:

    NLFSEDef.h

Abstract:
    This is the header file defining the macros used by 
    the kernel mode filter driver implementing NLFSE

Environment:

    Kernel mode

--*/
#ifndef __NLSE_DEF_H__
#define __NLSE_DEF_H__

//---------------------------------------------------------------------------
//  Generic Resource acquire/release macros
//---------------------------------------------------------------------------

#define NLFSEAcquireResourceExclusive( _r, _wait )                            \
    (ASSERT( ExIsResourceAcquiredExclusiveLite((_r)) ||                     \
            !ExIsResourceAcquiredSharedLite((_r)) ),                        \
     KeEnterCriticalRegion(),                                               \
     ExAcquireResourceExclusiveLite( (_r), (_wait) ))

#define NLFSEAcquireResourceShared( _r, _wait )                               \
    (KeEnterCriticalRegion(),                                               \
     ExAcquireResourceSharedLite( (_r), (_wait) ))

#define NLFSEReleaseResource( _r )                                            \
    (ASSERT( ExIsResourceAcquiredSharedLite((_r)) ||                        \
             ExIsResourceAcquiredExclusiveLite((_r)) ),                     \
     ExReleaseResourceLite( (_r) ),                                         \
     KeLeaveCriticalRegion())

#endif