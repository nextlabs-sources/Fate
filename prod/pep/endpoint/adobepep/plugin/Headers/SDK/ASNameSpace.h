#ifndef __ASNameSpace__
#define __ASNameSpace__

/*
 *        Name:	ASNameSpace.h
 *   $Revision$
 *      Author:	 
 *        Date:	   
 *     Purpose:	AS Name Space Suite (from Adobe Illustrator).
 *
 * Copyright (c) 1986-1996 Adobe Systems Incorporated, All Rights Reserved.
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

#ifndef __PlatformPragma__
#include "PlatformPragma.h"
#endif

#ifndef __SPStrings__
#include "SPStrngs.h"
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
 ** Constants
 **
 **/

#define kASNameSpaceSuite			"AS Name Space Suite"
#define kASNameSpaceSuiteVersion	2
#define kASNameSpaceVersion		kASNameSpaceSuiteVersion

#define kASNameSpaceErr	  		'NAME'


/*******************************************************************************
 **
 **	Types
 **
 **/

typedef struct _t_ASNameSpace *ASNameSpaceRef;
typedef struct t_ASDataStream *ASDataStreamRef;

#if __BUILD_PLUGIN__
#define kNSMaxNameLength		(100)
#define kNSMaxPathLength		((kNSMaxNameLength + 1) * 5)
#define kNSPathSeparator		'/'
#endif


/*******************************************************************************
 **
 **	Suite
 **
 **/

typedef struct {

	ASErr (*AllocateNameSpace) ( SPStringPoolRef pool, ASNameSpaceRef *space );
	ASErr (*DisposeNameSpace) ( ASNameSpaceRef space );

	ASErr (*SetValue) ( ASNameSpaceRef space, char *path, char *type, ... );
	ASErr (*GetValue) ( ASNameSpaceRef space, char *path, char *type, ... );

	ASErr (*GetType) ( ASNameSpaceRef space, char *path, char **type );
	ASErr (*GetChangeCount) ( ASNameSpaceRef space, char *path, long *count );
	ASErr (*RemoveValue) ( ASNameSpaceRef space, char *path );
	ASErr (*CountPaths) ( ASNameSpaceRef space, char *path, long *count );
	ASErr (*GetNthPath) ( ASNameSpaceRef space, char *path, long n, char *nthPath );

	ASErr (*ParseValue) ( ASNameSpaceRef space, char *path, ASDataStreamRef filter );
	ASErr (*FlushValue) ( ASNameSpaceRef space, char *path, ASDataStreamRef filter );

} ASNameSpaceSuite;


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
