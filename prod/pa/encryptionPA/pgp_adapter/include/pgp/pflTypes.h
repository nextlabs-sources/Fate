/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pflTypes_h	/* [ */
#define Included_pflTypes_h

#include "pgpBase.h"
 
/* opaque declarations */
typedef struct PFLFileSpec *		PFLFileSpecRef;
typedef struct PFLFileSpec const *	PFLConstFileSpecRef;

typedef struct PFLDirectoryIter *		PFLDirectoryIterRef;
typedef struct PFLDirectoryIter const *	PFLConstDirectoryIterRef;

/* Validity checks */
#define	kInvalidPFLFileSpecRef			((PFLFileSpecRef) NULL)
#define	kInvalidPFLDirectoryIterRef		((PFLDirectoryIterRef) NULL)

#define PFLFileSpecRefIsValid( ref )		( (ref) != kInvalidPFLFileSpecRef && \
											 		pgpDebugPtrReadAccess( (ref), 8 ) )
#define PFLDirectoryIterRefIsValid( ref )	\
			( (ref) != kInvalidPFLDirectoryIterRef && pgpDebugPtrReadAccess( (ref), 8 ) )

/* Languages supported by pgpLocStrings  */
enum PFLLanguage_
{
	kPFLLanguage_Default     = 0,
	kPFLLanguage_English     = 1,
	kPFLLanguage_Japanese    = 2,
	kPFLLanguage_German      = 3,
	kPFLLanguage_Spanish     = 4,
	
 	PGP_ENUM_FORCE( PFLLanguage_ )
};

PGPENUM_TYPEDEF( PFLLanguage_, PFLLanguage );


#endif /* ] Included_pflTypes_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
