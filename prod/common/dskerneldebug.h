#ifndef _DSKERNELDEBUG_H_
#define _DSKERNELDEBUG_H_	1

#include "dstypes.h"

#define DSDNOLOG	0x00000000
#define DSDFENTRY	0x00000001
#define DSDFLEAVE	0x00000002
#define DSDINFO		0x00000004
#define DSDWRN		0x00000008
#define DSDERR		0x00000010
#define DSDDEV		0x00000020	//For developement purpose only
#define DSDSPECIAL	0x00001000	//For temporarily special debug
#define DSDALL		0xffffffff

//
// Debug global variable per driver
//
extern ULONG g_ulDebugLevel;

#define DSFlagOn(a, b)	(a & b)

#ifdef WIN32
#define DP( _dbgLevel, _string )(DSFlagOn(g_ulDebugLevel,(_dbgLevel)) ? \
        DbgPrint _string  : ((void)0))

#elif LINUX
#define DebugPrint(_DbgLevelLevel, msg) \
	FlagOn(_DbgLevel, g_ulDebugLevel) ? printk msg : ((void)0))
#endif

		
		

#endif //#ifndef _DSKERNELDEBUG_H_