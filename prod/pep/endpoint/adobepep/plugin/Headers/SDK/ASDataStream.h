#ifndef __ASDataStream__
#define __ASDataStream__

/*
 *        Name:	ASDataStream.h
 *   $Revision$
 *      Author:	 
 *        Date:	   
 *     Purpose:	Adobe AS 2.0 Data Filter Suite.
 *
 * Copyright (c) 1986-1998,2002 Adobe Systems Incorporated, All Rights Reserved.
 *
 */


/*******************************************************************************
 **
 **	Imports
 **
 **/

#ifndef __ASTypes__
#include "ASTypes.h"
#endif

#ifndef __ASNameSpace__
#include "ASNameSpace.h"
#endif

#ifndef __SPFiles__
#include "SPFiles.h"
#endif

#ifndef __SPPlugins__
#include "SPPlugs.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN_ENV
#pragma PRAGMA_ALIGN_BEGIN
#pragma PRAGMA_IMPORT_BEGIN
#elif defined(HPUX) && defined(__HP_aCC)
HPUX_Pragma (PRAGMA_ALIGN_BEGIN)
#else
_Pragma (PRAGMA_ALIGN_BEGIN)
#endif


/*******************************************************************************
 **
 **	Constants
 **
 **/

#define kASDataStreamSuite		"AS Data Filter Suite"
#define kASDataStreamSuiteVersion	2


#define kASDataStreamErr			'DFER'


/*******************************************************************************
 **
 **	Types
 **
 **/
 
#ifndef ASDataStreamRef
typedef struct t_ASDataStream *ASDataStreamRef;
#endif

/*******************************************************************************
 **
 **	Suite
 **
 **/

typedef struct {

	ASAPI ASErr (*LinkDataStream) ( ASDataStreamRef prev, ASDataStreamRef next );
	ASAPI ASErr (*UnlinkDataStream) ( ASDataStreamRef next, ASDataStreamRef *prev );
	ASAPI ASErr (*ReadDataStream) ( ASDataStreamRef filter, char *store, ASInt32 *count );
	ASAPI ASErr (*WriteDataStream) ( ASDataStreamRef filter, char *store, ASInt32 *count );
	ASAPI ASErr (*SeekDataStream) ( ASDataStreamRef filter, ASInt32 *count );
	ASAPI ASErr (*MarkDataStream) ( ASDataStreamRef filter, ASInt32 *count );
	ASAPI ASErr (*NewFileDataStream) ( SPPlatformFileSpecification *spec, char *mode, ASInt32 creator, ASInt32 type, ASDataStreamRef *filter );
	ASAPI ASErr (*NewBufferDataStream) ( ASInt32 size, ASDataStreamRef *filter );
	ASAPI ASErr (*NewHexdecDataStream) ( char *state, ASInt32 shift, ASDataStreamRef *filter );
	ASAPI ASErr (*NewBlockDataStream) ( void *address, ASInt32 size, ASDataStreamRef *filter );
	ASAPI ASErr (*NewResourceDataStream) ( SPPluginRef plugin, ASInt32 type, ASInt32 id, char *name, ASDataStreamRef *filter);

} ASDataStreamSuite;


#ifdef WIN_ENV
#pragma PRAGMA_IMPORT_END
#pragma PRAGMA_ALIGN_END
#elif defined(HPUX) && defined(__HP_aCC)
HPUX_Pragma (PRAGMA_ALIGN_END)
#else
_Pragma (PRAGMA_ALIGN_END)
#endif

#ifdef __cplusplus
}
#endif


#endif
