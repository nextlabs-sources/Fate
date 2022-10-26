/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpFileSpecPriv_h	/* [ */
#define Included_pgpFileSpecPriv_h

#ifndef PGP_USE_FILE_SPEC_PRIV
#error you should not be including this file
#endif

#include <stdio.h>

#include "pgpFileSpec.h"
#include "pgpFileUtilities.h"

#define kPFLFileSpecMagic			0x251DEA9B


typedef enum PFLFileSpecType
{
	kPFLFileSpecInvalidType		= 13,
	kPFLFileSpecFullPathType	= 14,
	kPFLFileSpecMacType			= 15
	
} PFLFileSpecType;



typedef PGPError	(*PGPFileSpecExportProc)( PFLConstFileSpecRef ref,
						PGPByte **data, PGPSize *dataSize );
						
typedef PGPError	(*PGPFileSpecImportProc)( PFLFileSpecRef ref,
						PGPByte const *data,
						PGPUInt dataSize );
						
typedef PGPBoolean	(*PGPFileSpecsEqualProc)( PFLConstFileSpecRef ref1,
						PFLConstFileSpecRef ref2 );
						
typedef PGPError	(*PGPFileSpecGetNameProc)(
						PFLConstFileSpecRef ref, PFLChar name[ 256*3 ] );
						
typedef PGPError	(*PGPFileSpecSetNameProc)(
						PFLFileSpecRef ref, PFLChar const *name );

typedef PGPError	(*PGPFileSpecSetMetaInfoProc)(
						PFLFileSpecRef ref, const void *info );
						
typedef PGPError	(*PGPFileSpecCreateProc)( PFLFileSpecRef ref );
						
typedef PGPError	(*PGPFileSpecDeleteProc)( PFLConstFileSpecRef ref );
						
typedef PGPError	(*PGPFileSpecRenameProc)( PFLFileSpecRef ref,
						const PFLChar *newName);
						
typedef PGPError	(*PGPFileSpecExistsProc)( PFLFileSpecRef ref,
						PGPBoolean *exists );
						
typedef PGPError	(*PGPFileGetMaxNameLengthProc)( PFLConstFileSpecRef ref,
						PGPSize *	maxLength );

typedef PGPError	(*PGPFileGetParentDirProc)(
						PFLConstFileSpecRef fileFromDir,
						PFLFileSpecRef *outParent );

typedef PGPError	(*PGPFileSpecComposeProc)(
						PFLConstFileSpecRef parent, PFLChar const *name,
						PFLFileSpecRef *outRef );

typedef struct PGPFileSpecVTBL	PGPFileSpecVTBL;

struct PGPFileSpecVTBL
{
	PGPFileSpecExportProc		exportProc;
	PGPFileSpecImportProc		importProc;
	PGPFileSpecsEqualProc		equalProc;
	PGPFileSpecGetNameProc		getNameProc;
	PGPFileSpecSetNameProc		setNameProc;
	
	PGPFileSpecSetMetaInfoProc	setMetaInfoProc;
	PGPFileSpecExistsProc		existsProc;
	PGPFileSpecCreateProc		createProc;
	PGPFileSpecDeleteProc		deleteProc;
	PGPFileSpecRenameProc		renameProc;
	PGPFileGetMaxNameLengthProc	maxNameLengthProc;
	PGPFileGetParentDirProc		getParentDirProc;
	PGPFileSpecComposeProc		composeProc;
} ;

#define CallExportProc( ref, data, dataSize ) \
	(*((ref)->vtbl->exportProc))( ref, data, dataSize )
	
#define CallImportProc( ref, data, dataSize ) \
	(*((ref)->vtbl->importProc))( ref, data, dataSize )
	
#define CallFileSpecsEqualProc( ref1, ref2 ) \
	(*((ref1)->vtbl->equalProc))( ref1, ref2 )
	
#define CallGetNameProc( ref, name ) \
	(*((ref)->vtbl->getNameProc))( ref, name )
	
#define CallSetNameProc( ref, name ) \
	(*((ref)->vtbl->setNameProc))( ref, name )
	
#define CallSetMetaInfoProc( ref, info ) \
	(*((ref)->vtbl->setMetaInfoProc))( ref, info )
	
#define CallExistsProc( ref, exists ) \
	(*((ref)->vtbl->existsProc))( ref, exists )
	
#define CallCreateProc( ref ) \
	(*((ref)->vtbl->createProc))( ref )
	
#define CallDeleteProc( ref, name ) \
	(*((ref)->vtbl->deleteProc))( ref )
	
#define CallRenameProc( ref, name ) \
	(*((ref)->vtbl->renameProc))( ref, name )
	
#define CallGetMaxNameLength( ref, length ) \
	(*((ref)->vtbl->maxNameLengthProc))( ref, length )

#define CallGetParentDirProc( childSpec, outParent ) \
	(*((childSpec)->vtbl->getParentDirProc))( childSpec, outParent )

#define CallComposeProc( parent, name, outRef ) \
	(*((parent)->vtbl->composeProc))( parent, name, outRef )
	

struct PFLFileSpec
{
	PGPUInt32				magic;
	PGPMemoryMgrRef			memoryMgr;
	PFLFileSpecType			type;
	PGPUInt			dataSize;
	PGPFileSpecVTBL const *	vtbl;
	void *					data;
} ;


PGPError	pgpNewFileSpec( PGPMemoryMgrRef context,
				PFLFileSpecType type, 
				PGPUInt32 dataSize, PFLFileSpecRef * outRef );


PGPError	pgpPlatformGetTempFileSpec( PGPMemoryMgrRef context,
				PFLConstFileSpecRef optionalRef, PFLFileSpecRef * outRef );
				
PGPError	pgpPlatformOpenFileSpecAsFILE( PFLFileSpecRef spec,
				const PFLChar * openMode, FILE ** fileOut );

PGPError	pgpPlatformGetFileInfo( PFLConstFileSpecRef spec,
				PFLFileInfo *outInfo );
PGPError	pgpPlatformNewDirectoryIter( PFLConstFileSpecRef parentDir,
				PFLDirectoryIterRef * outIter );
PGPError	pgpPlatformNextFileInDirectory( PFLDirectoryIterRef iter,
				PFLFileSpecRef * outRef );
PGPError	pgpPlatformFreeDirectoryIter( PFLDirectoryIterRef iter );

PGPError	pgpPlatformLockFILE( FILE * file, PFLFileOpenFlags flags );

PGPBoolean   pgpGetNormalizedShortUTF8NameWin32( const PGPChar8 *name, PGPChar8 out[256*3] );

#endif	/* ] Included_pgpFileSpecPriv_h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
