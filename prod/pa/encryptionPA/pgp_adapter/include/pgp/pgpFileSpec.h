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
#ifndef Included_pgpFileSpec_h	/* [ */
#define Included_pgpFileSpec_h

#include <time.h>	/* Temporary, for time_t below */

#include "pgpMemoryMgr.h"

#include "pflTypes.h"


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

enum PFLFileInfoFlags_
{
	kPGPFileInfo_IsDirectory	= ( 1L << 0 ),
	kPGPFileInfo_IsPlainFile	= ( 1L << 1 ),
	kPGPFileInfo_IsSymLink		= ( 1L << 2 )
};
typedef PGPUInt32	PFLFileInfoFlags;
 
typedef struct PFLFileInfo_
{
	PFLFileInfoFlags	flags;
	PGPFileOffset		dataLength;
	time_t				modificationTime;
	PGPUInt32			protection;
	PGPUInt32			uid;
	PGPUInt32			gid;
} PFLFileInfo;

PGPError 	PFLCopyFileSpec( PFLConstFileSpecRef spec,
					PFLFileSpecRef * outRef );
	
PGPError 	PFLFreeFileSpec( PFLFileSpecRef ref);

PGPMemoryMgrRef	PFLGetFileSpecMemoryMgr( PFLConstFileSpecRef spec );

/* *outData must be freed with PGPFreeData */
PGPError 	PFLExportFileSpec( PFLConstFileSpecRef spec,
						PGPByte **outData, PGPSize *outSize );
						
PGPError  	PFLImportFileSpec( PGPMemoryMgrRef context,
					PGPByte const *buffer, PGPSize size,
					PFLFileSpecRef *outRef );

/* True if the two filespecs name the same file */
PGPBoolean  	PFLFileSpecsEqual( PFLConstFileSpecRef spec1,
					PFLConstFileSpecRef spec2 );

/* *outName must be freed with PGPFreeData */
PGPError 	PFLGetFileSpecName( PFLConstFileSpecRef spec,
					PFLChar **outName );

/* put C-String name into client-supplied buffer */
PGPError 	PFLGetFileSpecNameBuffer( PFLConstFileSpecRef spec,
				PGPSize maxSize, PFLChar *outName );

/* does not rename the file itself */
PGPError 		PFLSetFileSpecName( PFLFileSpecRef spec,
						PFLChar const *newName);
						
/* create a new filespec from a directory spec and a filename */
PGPError 		PFLComposeFileSpec( PFLConstFileSpecRef parentDir,
						PFLChar const *fileName,
						PFLFileSpecRef *outRef );

/* adapter version for PFLComposeFileSpec */
PGPError 		PFLComposeFileSpecU16( PFLConstFileSpecRef parentDir,
						PGPChar16 const *fileName,
						PFLFileSpecRef *outRef );
						
/* extend the file name with the extension.
   Takes into account max name length */
PGPError 		PFLExtendFileSpecName( PFLFileSpecRef spec,
						PFLChar const *extension);
						
/* get the maximum name length a file spec can handle */
PGPError 		PFLGetMaxFileSpecNameLength( PFLConstFileSpecRef spec,
						PGPSize *	maxLength);

/* remember meta information for file.  Used when create file */
PGPError		PFLSetFileSpecMetaInfo( PFLFileSpecRef spec,
					const void *info);

/* generate a unique FileSpec based on 'spec'.  If spec doesn't exist,
then output will be the same (but a copy) */
PGPError 		PFLFileSpecGetUniqueSpec( PFLConstFileSpecRef spec,
						PFLFileSpecRef *uniqueOut );

PGPError		PFLFileSpecExists( PFLFileSpecRef spec,
					PGPBoolean *exists );
PGPError		PFLFileSpecCreate( PFLFileSpecRef spec );
PGPError		PFLFileSpecDelete( PFLConstFileSpecRef spec );

/* Renames the file AND changes the name in the file spec itself.
	If an error occurs, the spec is not changed. The name is *not*
	the full path, but just the filename itself.
*/
PGPError		PFLFileSpecRename( PFLFileSpecRef spec,
					const PFLChar *newName );

PGPError		PFLGetParentDirectory( PFLConstFileSpecRef fileInDir,
					PFLFileSpecRef *outParent );
					
PGPError		PFLGetTempFileSpec( PGPMemoryMgrRef context, 
					PFLConstFileSpecRef optionalRef, PFLFileSpecRef * outRef );

/* Get file information */
PGPError		PFLGetFileInfo( PFLConstFileSpecRef spec,
					PFLFileInfo *outInfo );

/* Directory iteration functions */
PGPError		PFLNewDirectoryIter( PFLConstFileSpecRef parentDir,
					PFLDirectoryIterRef * outIter );
PGPError		PFLNextFileInDirectory( PFLDirectoryIterRef iter,
					PFLFileSpecRef * outRef );
PGPError		PFLFreeDirectoryIter( PFLDirectoryIterRef iter );

#if PGP_MACINTOSH || PGP_OSX
struct FSSpec;

PGPError 	PFLNewFileSpecFromFSSpec( PGPMemoryMgrRef context,
					const struct FSSpec * spec, PFLFileSpecRef * outRef );
					
PGPError 	PFLGetFSSpecFromFileSpec( PFLConstFileSpecRef ref,
					struct FSSpec * spec );
#endif

PGPBoolean		pflFileSpecIsValid( PFLConstFileSpecRef	ref );
#define PFLValidateFileSpec( ref )		\
	PGPValidateParam( pflFileSpecIsValid( ref ) )


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


/*
 * Flag indicating whether attempting to delete an open file will
 * disrupt the process which has the file open.  Depends on the platform.
 * If deletion fails in this case, or succeeds but keeps the file
 * accessable until the last close, then deletes are considered safe
 * and this flag can be set to 0 safely.
 */
#define PFL_PLATFORM_UNSAFE_DELETE	0


#define _pgpFileSpecIncludeRightStuff_

#if PGP_MACINTOSH
#include "pgpFileSpecMac.h"
#else
#include "pgpFileSpecStd.h"
#endif

#undef _pgpFileSpecIncludeRightStuff_

#endif	/* ] Included_pgpFileSpec_h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
