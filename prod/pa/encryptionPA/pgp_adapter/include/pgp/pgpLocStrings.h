/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpLocStrings_h	/* [ */
#define Included_pgpLocStrings_h


#include "pgpBase.h"

#include "pflTypes.h"

#if ! PGP_LOC_STR_LEAN
#include <stdio.h>
#include "pgpMemoryMgr.h"
#endif

PGP_BEGIN_C_DECLARATIONS

#if PGP_OSX || PGP_UNIX_SOLARIS || PGP_UNIX_AIX
	#pragma pack(1)
#elif PGP_UNIX_HPUX
	#warning("gcc on HP-UX ignores pragma pack")
#else
	#pragma pack(push, 1)
#endif
typedef struct _pgpLocStringsMainHeader {
	PGPByte magic[10];		/* always "PGPlocstr\n"  */
	PGPUInt16 headerSize;	/* size of this structure and following array of pgpLocStringsHashRow */
	PGPUInt16 version;		/* 1 */
	PGPByte format_byteOrder[2]; /* [0]: sizeof(wchar_t), 1 for UTF-8
									[1]: 1 for little endian, 2 for big endian data */
	PGPUInt32 hashInit;		/* parameter to hash function */
	PGPUInt32 hashPrime;	/* parameter to hash function that defines the maax value of it */
} pgpLocStringsMainHeader;
#if PGP_OSX || PGP_UNIX_SOLARIS || PGP_UNIX_AIX
	#pragma pack()
#elif PGP_UNIX_HPUX
	#warning("gcc on HP-UX ignores pragma pack")
#else
	#pragma pack(pop)
#endif

#if PGP_OSX || PGP_UNIX_SOLARIS || PGP_UNIX_AIX
	#pragma pack(1)
#elif PGP_UNIX_HPUX
	#warning("gcc on HP-UX ingores pragma pack")
#else
	#pragma pack(push, 1)
#endif
typedef struct _pgpLocStringsHashRow {
	PGPUInt32 hash;			/* hash of the key string */
	PGPUInt32 offset;		/* offset from the beginning of the file (data start from header.headerSize offset) */
	PGPUInt32 size;			/* strlen at that offset */
} pgpLocStringsHashRow;
#if PGP_OSX || PGP_UNIX_SOLARIS || PGP_UNIX_AIX
	#pragma pack()
#elif PGP_UNIX_HPUX
	#warning("gcc on HP-UX ingores pragma pack")
#else
	#pragma pack(pop)
#endif

typedef unsigned char ** PGPLocStringsContextRef;


/*
 * PGPStrLocWChar is a platform-specific type that defines the preffered size of Unicode character.
 * Following default definitions take into account the size of wchar_t for the platform 
 * and the most convenient Unicode encoding.
 *
 * The size must match the sizeof_PGPStrLocWChar for few functions bellow.
 */
#if !defined( PGPStrLocWChar )
	#if PGP_UNIX && !PGP_OSX
		#define PGPStrLocWChar PGPChar8
	#elif PGP_OSX
		#define PGPStrLocWChar PGPChar8
	#elif PGP_WIN32
		#define PGPStrLocWChar PGPUInt16 
	#else
		#define PGPStrLocWChar void
	#endif
#endif

/* pointers to memory allocation routines */
typedef void *(*PGPStrLocAllocRoutine)(void *userValue, size_t size, int clean);
typedef void (*PGPStrLocFreeRoutine)(void *p);

#if ! PGP_LOC_STR_LEAN
/* simple API to get a localized string */
PGPError PGPInitModuleLocString(PFLFileSpecRef homePath, const PFLChar *baseName, PFLLanguage lang, int sizeof_PGPStrLocWChar);
void PGPFreeModuleLocString(void);
const PGPStrLocWChar *PGPGetModuleLocString( const char *string, const char *description );
#endif

#if PGP_LOC_STR_LEAN
#define PGP_LOC_STR_CONTEXT_SIZE (40+sizeof(void*)*7)
#endif

/* advanced API to get a localized string */
#if ! PGP_LOC_STR_LEAN
PGPError pgpNewLocStringsContext( PGPMemoryMgrRef memMgr, PGPLocStringsContextRef *context );
#endif
PGPError pgpNewLocStringsContextWithAlloc( PGPStrLocAllocRoutine allocRoutine, PGPStrLocFreeRoutine freeRoutine, void *reserved,
		        void *allocUserValue, PGPLocStringsContextRef *context );
void pgpFreeLocStringsContext( PGPLocStringsContextRef context );

#if PGP_LOC_STR_LEAN
/* On the input the buffer of size 'size' is structured like this: [PGP_LOC_STR_CONTEXT_SIZE zero bytes] + [processed strings file]
 * On the output the buffer can be used as a PGPLocStringsContextRef. This buffer cannot be memmoved, once the context is initialized.
 * This way no allocation or deallocation of resources is required in this API. 
 * Useful in projects where dynamic allocation is problematic. 
 */
PGPError pgpInitLocStringsContextFromBuffer( void *buffer, int size, int sizeof_PGPStrLocWChar );
#endif

#if ! PGP_LOC_STR_LEAN
PGPError pgpLocStringsMakeFileSpec( PGPLocStringsContextRef context, const PFLChar *filename, PFLFileSpecRef *specout );
PGPError pgpLocStringsLoadPGP( PGPLocStringsContextRef context, int sizeof_PGPStrLocWChar, PFLFileSpecRef filespec );
#endif

PGPError pgpLocStringsLoadPGPFromMem( PGPLocStringsContextRef context, int sizeof_PGPStrLocWChar,
	   	const PGPByte *data, PGPUInt32 dataSize );
const PGPStrLocWChar *pgpGetLocString( const PGPLocStringsContextRef context, const char *string, const char *description );

/* exposed for testing */
PGPUInt32 pgpGetHashInitValue( const PGPLocStringsContextRef context );

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

#define PGPLS( string, description ) PGPGetModuleLocString( string, description )

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpLocStrings_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
