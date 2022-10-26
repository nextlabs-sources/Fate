/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	Standard implementation of a file specification.  Assumes system uses
	full paths of the form /a/b/c or \a\b\c.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpFileSpecStd_h	/* [ */
#define Included_pgpFileSpecStd_h


/* this file is included by pgpFileSpec.h and shouldn't be included directly */
#ifndef _pgpFileSpecIncludeRightStuff_
#error "don't include this file directly.  Use pgpFileSpec.h instead."
#endif


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


#if ! PGP_MACINTOSH /* [ */

PGPError 	PFLNewFileSpecFromFullPath( PGPMemoryMgrRef memoryMgr,
					PFLChar const * path, PFLFileSpecRef * outRef );

PGPError 	PFLNewFileSpecFromFullPathU16( PGPMemoryMgrRef memoryMgr,
					PGPChar16 const * path16, PFLFileSpecRef * outRef );

PGPError  	PFLGetFullPathFromFileSpec(PFLConstFileSpecRef fileRef,
						PFLChar **fullPathPtr);

PGPError	pgpNewFileSpecFromCanonicalPath( PGPMemoryMgrRef memoryMgr,
				PFLChar const *path, PFLFileSpecRef *outRef );

PGPError	pgpGetCanonicalPathFromFileSpec( PFLConstFileSpecRef fileRef, PFLChar **fullPathPtr);

#endif	/* ] */



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


#endif	/* ] Included_pgpFileSpecStd_h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
