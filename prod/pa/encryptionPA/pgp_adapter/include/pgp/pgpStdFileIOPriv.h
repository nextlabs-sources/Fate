/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	File IO that uses a FILE * for its implementation

	$Id$
____________________________________________________________________________*/

#include "pgpStdFileIO.h"
#include "pgpFileIOPriv.h"
#include <stdio.h>

PGP_BEGIN_C_DECLARATIONS


/* the virtual function table for a PGPFileIO */
typedef struct PGPStdFileIOVtbl
{
	PGPFileIOVtbl	parentVTBL;
} PGPStdFileIOVtbl;


PGPStdFileIOVtbl const *	pgpStdFileIOGetClassVTBL( void );


/* used to initialize */
typedef struct PGPStdFileIOData
{
	PGPBoolean		autoClose;
	FILE *			stdioFILE;
} PGPStdFileIOData;

struct PGPStdFileIO
{
	PGPFileIO		parent;
	
	/* borrowed from the Mac for 64 bit ftell support */
	PGPFileOffset totalSize;
	PGPFileOffset filePos;

	PGPUInt32		stdFileIOMagic;
	FILE *			stdioFILE;
	PGPBoolean		autoClose;
};
#define kPGPStdFileIOMagic		0x5354494f



PGP_END_C_DECLARATIONS

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
