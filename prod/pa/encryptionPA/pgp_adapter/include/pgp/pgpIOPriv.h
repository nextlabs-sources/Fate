/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	Private declarations for PGPIO.  CLIENTS SHOULD NOT INCLUDE THIS FILE.
	

	$Id$
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpIO.h"
#include "pgpFileSpec.h"


PGP_BEGIN_C_DECLARATIONS

typedef PGPError	(*PGPIOInitProc)( PGPIORef ref, void *data);
typedef PGPError	(*PGPIODestroyProc)( PGPIORef ref );

typedef PGPError	(*PGPIOReadProc)( PGPIORef ref, PGPSize requestCount,
						void *buffer, PGPSize *bytesRead );
						
typedef PGPError	(*PGPIOWriteProc)( PGPIORef ref, PGPSize requestCount,
						const void *buffer );
						
typedef PGPError	(*PGPIOSetPosProc)( PGPIORef ref, PGPFileOffset newPos );
						
typedef PGPError	(*PGPIOGetPosProc)( PGPIORef ref, PGPFileOffset *curPos );
						
typedef PGPError	(*PGPIOGetEOFProc)( PGPIORef ref, PGPFileOffset *eof );

#if PGPIO_EOF
typedef PGPError	(*PGPIOSetEOFProc)( PGPIORef ref, PGPFileOffset eof );
#endif

typedef PGPError	(*PGPIOFlushProc)( PGPIORef ref );


/* the virtual function table for a PGPIO */
typedef struct PGPIOVtbl
{
	PGPIOInitProc		initProc;
	PGPIODestroyProc	destroyProc;
	
	PGPIOReadProc		readProc;
	PGPIOWriteProc		writeProc;
	PGPIOGetPosProc		getPosProc;
	PGPIOSetPosProc		setPosProc;
	PGPIOGetEOFProc		getEOFProc;
#if PGPIO_EOF
	PGPIOSetEOFProc		setEOFProc;
#endif
	PGPIOFlushProc		flushProc;
} PGPIOVtbl;


PGPIOVtbl const *	pgpIOGetClassVTBL( void );


PGPError	pgpioInheritInit( PGPIORef ref, PGPIOVtbl const * vtbl,
				void *data );
PGPError	pgpioInheritDestroy( PGPIORef ref, PGPIOVtbl const * vtbl );

PGPError	pgpioInheritRead( PGPIORef ref, PGPIOVtbl const * vtbl,
				PGPSize requestCount, void *buffer, PGPSize *bytesRead );
						
PGPError	pgpioInheritWrite( PGPIORef ref, PGPIOVtbl const * vtbl,
				PGPSize requestCount, const void *buffer );
						
PGPError	pgpioInheritSetPos( PGPIORef ref, PGPIOVtbl const * vtbl,
				PGPFileOffset newPos );
						
PGPError	pgpioInheritGetEOF( PGPIORef ref, PGPIOVtbl const * vtbl,
				PGPFileOffset *eof );
				
#if PGPIO_EOF
PGPError	pgpioInheritSetEOF( PGPIORef ref, PGPIOVtbl const * vtbl,
				PGPFileOffset eof );
#endif
						
PGPError	pgpioInheritFlush( PGPIORef ref, PGPIOVtbl const * vtbl );


struct PGPIO
{
	PGPUInt32					magic;
	PGPIOVtbl const * const		vtbl;
	PGPMemoryMgrRef				context;
	PGPFileOffset				offset;
} ;
#define kPGPIOMagic		0x5047494f	/* 'PGIO' */

#define		pgpioGetObjectVTBL( ref ) \
				( (PGPIORef)(ref))->vtbl

PGPError	pgpNewIOFromVTBL( PGPMemoryMgrRef context, 
				PGPIOVtbl const * vtbl, PGPSize size, void *data,
				PGPIORef * outRef );

PGP_END_C_DECLARATIONS






















/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
