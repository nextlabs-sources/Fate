/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpTypes_h	/* [ */
#define Included_pgpTypes_h

#if PGP_MACINTOSH
#include <types.h>
#else
#include <sys/types.h>
#endif

#include "pgpBase.h"

#if ! HAVE_UCHAR
typedef unsigned char	uchar;
#endif

#if ! HAVE_USHORT
typedef unsigned short	ushort;
#endif

#if ! HAVE_UINT
typedef unsigned int	uint;
#endif

#if ! HAVE_ULONG
typedef unsigned long	ulong;
#endif


/* Lookie here!  An ANSI-compliant alignment finder! */
#ifndef	alignof

#ifdef __GNUC__
#   define alignof(type)	__alignof__(type)
#else
#	ifdef __cplusplus
    /* TODO: this seems wrong for modern Win32 cl; 
     * I guess we don't rely on this code
     */
#	define	alignof(type) (1)
#	else
#      if defined(_MSC_VER) && _MSC_VER >= 1400
#         define alignof(type) __alignof(type)
#      else
#	      define alignof(type) (sizeof(struct{type _x; PGPByte _y;}) - sizeof(type))
#      endif
#	endif
#endif

#if PGP_WIN32
#ifndef __MWERKS__
/* Causes "unnamed type definition in parentheses" warning" */
#pragma warning ( disable : 4116 )
#endif
#endif

#endif

#endif /* ] Included_pgpTypes_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
