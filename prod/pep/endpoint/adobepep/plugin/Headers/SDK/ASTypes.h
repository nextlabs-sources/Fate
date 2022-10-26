/* ASTypes.h file for Plug-ins ONLY
*/

#ifndef _ASTYPES_H_
#define _ASTYPES_H_

#include "Environ.h"

#if PLUGIN

#include "ASExpT.h"
#include "CoreExpT.h"

#endif

/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
**
** The following definitions are ONLY to be used when accessing ADM -
** DO NOT USE THEM FOR ANY OTHER PURPOSE!!!!!! 
**
** ADM is only available in the Mac and Windows Viewer - it is not available in the Reader or in
** any configuration on UNIX.
**
** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
*/

/* ---- Begin new types for ADM ---- */

#ifdef MAC_ENV
#define	PRAGMA_ALIGN_BEGIN			"mark ignore_pragma"	/* We don't want to do anything here, but a empty pragma is just as unexpected as a unsupported one. */
#define PRAGMA_ALIGN_END			"mark ignore_pragma"	/* We don't want to do anything here, but a empty pragma is just as unexpected as a unsupported one. */
#define PRAGMA_IMPORT_BEGIN			"mark ignore_pragma"	/* We don't want to do anything here, but a empty pragma is just as unexpected as a unsupported one. */
#define PRAGMA_IMPORT_END			"mark ignore_pragma"	/* We don't want to do anything here, but a empty pragma is just as unexpected as a unsupported one. */
#endif

#ifdef WIN_ENV
#define	PRAGMA_ALIGN_BEGIN			pack(push, 4)			
#define PRAGMA_ALIGN_END			pack(pop)
#define PRAGMA_IMPORT_BEGIN			
#define PRAGMA_IMPORT_END		
#endif

/*
 * Constants
 */

/* true and false */

#ifndef __cplusplus	

#ifndef true
#define true	1
#endif

#ifndef false
#define false	0
#endif

#endif /* __cplusplus */

#ifndef TRUE
#define TRUE	true
#endif

#ifndef FALSE
#define FALSE	false
#endif

/* error codes */
#define kNoErr					0
#define kOutOfMemoryErr			'!MEM'
#define kBadParameterErr		'PARM'
#define kNotImplementedErr		'!IMP'
#define kCantHappenErr			'CANT'

#ifndef NULL

#ifdef MAC_ENV
#if !defined(__cplusplus) && (defined(__SC__) || defined(THINK_C))
#define NULL	((void *) 0)
#else
#define NULL	0
#endif
#endif

#ifdef WIN_ENV
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifdef UNIX_ENV
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#endif

/* dhearst 8/11/99 - we now specifically prefer NULL, so nil
** is obsolete. We no longer provide it, but can't enforce this
** policy because platform headers often provide nil.
**#ifndef nil
**#define nil NULL
**#endif
*/


/* AMPAPI  Adobe Standard Plugin API calling convention. */

#ifndef AMPAPI
#ifdef MAC_ENV
#define ASAPI
#endif
#ifdef WIN_ENV 
#define ASAPI
#endif
#ifdef UNIX_ENV
#define ASAPI
#endif
#endif

/* C calling convention for those places that need it.
** This doesn't really do anything, but is  more for
** an explicity declaration when it matters.
*/
#define ASCAPI	


/*
 * Types
 */

/* Integer Types */

typedef unsigned char ASUInt8;
typedef unsigned short ASUInt16;
typedef unsigned int ASUInt32;
typedef int ASInt32;

typedef ASInt32 ASErr;

/* Storage Types */

#if !UNIX_ENV
typedef unsigned char ASByte;
#endif
typedef ASByte *ASBytePtr;

/* Unicode Types */
typedef ASUTF16Val ASUnicode;

/* Pointer Types */

typedef void *ASPtr, **ASHandle;

/* Fixed and Real Types - These are already defined in ASExpT.h*/


/* Platform Structures */

#ifdef MAC_ENV


/* ASBoolean is the same a Macintosh Boolean. */
typedef unsigned char ASBoolean; 

/* ASPortRef is the same as a Macintosh GrafPtr. */
typedef struct _t_ADMPortRef *ADMPortRef;

/* ASWindowRef is the same as a Macintosh WindowPtr. */
typedef struct _t_ADMWindowRef *ADMWindowRef;

/* ASRect is the same size and layout as a Macintosh Rect. */
typedef struct _t_ASRect {
	short top, left, bottom, right;
} ASRect;

/* ASPoint is the same size and layout as a Macintosh Point. */
typedef struct _t_ASPoint {
	short v, h;
} ASPoint;

#endif /* MAC_ENV */


#ifdef WIN_ENV

/* ASBoolean is the same a Windows BOOL. */
typedef int ASBoolean;

/* ASPortRef is the same as a Windows HDC. */
typedef struct _t_ADMPortRef *ADMPortRef;

/* ASWindowRef is the same as a Windows HWND. */
typedef struct _t_ADMWindowRef *ADMWindowRef;

/* ASRect is the same size and layout as a Windows RECT. */
typedef struct _t_ASRect {
	ASInt32 left, top, right, bottom;
} ASRect;

/* ASPoint is the same size and layout as a Windows POINT. */
typedef struct _t_ASPoint  {
	ASInt32 h, v;
} ASPoint;

#endif /* WIN_ENV */


#ifdef UNIX_ENV

/* ASBoolean is the same as gboolean. */
typedef int ASBoolean;

/* ASRect is not the same size and layout as a GdkRectangle, 'cos that is { left, top, width, height }. */
typedef struct _t_ASRect {
	int left, top, right, bottom;
} ASRect; 

/* ASPoint is the same size and layout as a GdkPoint. */
typedef struct _t_ASPoint  {
	int h, v;
} ASPoint;

#endif /* UNIX_ENV */
		
/* ASRGBColor is the same as a Macintosh RGBColor on Macintosh and Windows. */
typedef struct _t_ASRGBColor {
	unsigned short red, green, blue;
} ASRGBColor;


/* AIEvent is the same as a Macintosh EventRecord on Macintosh and Windows. */
typedef struct _t_ASEvent {
	unsigned short	what;
	unsigned long	message;
	ASUInt32		when;
	ASPoint			where;
	unsigned short	modifiers;
} ASEvent;

/* This is a generic reference to a resource/plugin file.  If not otherwise stated,
** it is assumed to be equivalent to an SPPluginRef (see "SPPlugs.h")
*/
typedef struct ASAccess *ASAccessRef;

/* ---- End new types for ADM ---- */

/* The above ADM definitions are ONLY to be used when accessing ADM -
** DO NOT USE THEM FOR ANY OTHER PURPOSE!!!!!!
**
** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
*/

#endif /* _ASTYPES_H_ */
