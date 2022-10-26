/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	Platform-independent file-specification.  Specifies the *location* of a
	file in the file system.  Also included are utility routines for
	manipulating files.
	
	Note that a PGPFileSpec is not an object; it does not have subclasses
	or inheritence.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpFileUtilities_h	/* [ */
#define Included_pgpFileUtilities_h

#include <stdio.h>
#include "pgpMemoryMgr.h"
#include "pgpFileSpec.h"
#include "pgpFileIO.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


enum PFLFileOpenFlags_
{
	kPFLFileOpenFlags_ReadOnly		= 0,
	kPFLFileOpenFlags_ReadWrite		= (1L << 0),
	kPFLFileOpenFlags_LockFile		= (1L << 1),
	kPFLFileOpenFlags_Create		= (1L << 2)
};
typedef PGPUInt32	PFLFileOpenFlags;

PGPError	PGPOpenFileSpec( PFLFileSpecRef spec, PFLFileOpenFlags flags,
				PGPFileIORef *outRef );

/*
 * Perform any optional locking which isn't done by fopen().
 * Normally you shouldn't use this directly.  Instead just pass the
 * kPFLFileOpenFlags_LockFile flag to PGPOpenFileSpec().  This routine
 * is solely to support similar functionality in pgpFileRefStdIOOpen()
 * of the PGPsdk library.
 */
PGPError		PFLLockFILE( FILE * file, PFLFileOpenFlags flags );


/* set the file size of a *closed* file; it remains closed afterwards */
PGPError	PGPSetFileSize( PFLConstFileSpecRef spec, PGPFileOffset size);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif
PGP_END_C_DECLARATIONS

#endif	/* ] Included_pgpFileUtilities_h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
