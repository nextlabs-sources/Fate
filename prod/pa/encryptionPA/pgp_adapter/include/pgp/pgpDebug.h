/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpDebug_h	/* [ */
#define Included_pgpDebug_h

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#if PGP_MOBILE

#elif PGP_WIN32
#include <crtdbg.h>

#ifdef __cplusplus
extern "C"
{
#endif

	long __cdecl _InterlockedExchangeAdd(long volatile * Addend, long Value);
	long _InterlockedIncrement(long volatile *Addend);

#ifdef __cplusplus
}
#endif

#pragma intrinsic (_InterlockedExchangeAdd)
#pragma intrinsic (_InterlockedIncrement)

#elif ! PGP_MACINTOSH
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "pgpTypes.h"

/*
 * we need to get the build flags to define UNFINISHED_CODE_ALLOWED
*/
#include "pgpBuild.h"

/*
 * pgpReviewCode is a macro which forces a compile error unless
 * the CODE_REVIEW macro is non-zero.
 * Example:  pgpReviewCode("Use better errors here")
 */
#ifndef pgpReviewCode

#ifndef UNFINISHED_CODE_ALLOWED
#define UNFINISHED_CODE_ALLOWED	1
#endif
#ifndef CODE_REVIEW
#define CODE_REVIEW	0
#endif


#if !CODE_REVIEW
#define pgpReviewCode(msg)
#else
#if PGP_WIN32
#define pgpReviewCode(msg)	@ ;
#else
#define pgpReviewCode(msg)	@@@@@@ #msg @@@@@@
#endif
#endif
#endif

/*
 * DEBUG_STRUCT_CONSTRUCTOR defines a null constructor for a struct
 * which initializes the structure which all 0xDDs if PGP_DEBUG is non-zero,
 * or does nothing otherwise.  It requires C++.  It shouldn't be used
 * for anything with virtual methods, because it will overwrite the
 * virtual dispatch table pointer.
 * Example:  foo { int a; DEBUG_STRUCT_CONSTRUCTOR(foo) }
 */
#if defined(__cplusplus) && PGP_DEBUG && ! PGP_UNIX_DARWIN
#define DEBUG_STRUCT_CONSTRUCTOR(type)						\
			type()											\
		{													\
			pgpDebugFillMemory(this, sizeof(*this));		\
		}
#else
#define DEBUG_STRUCT_CONSTRUCTOR(type)
#endif

PGP_BEGIN_C_DECLARATIONS

/*
 * All the FmtMsg macros accept the following special sequences:
 *		%%	Replaced with a single '%'
 *		%c	Replaced with the PFLChar argument
 *		%s	Replaced with the C string argument
 *		%S	Replaced with the Pascal string argument
 *		%B	Replaced with the memory buffer as a string
 *				(two args: length (int), buffer)
 *		%lB	Replaced with the memory buffer as a string
 *				(two args: length (long), buffer)
 *		%d	Replaced with the signed integer value (base 10)
 *		%ld	Replaced with the signed long value (base 10)
 *		%u	Replaced with the unsigned integer value (base 10)
 *		%lu	Replaced with the unsigned long value (base 10)
 *		%x	Replaced with the unsigned integer value (base 16)
 *		%lx	Replaced with the unsigned long value (base 16)
 *		%o	Replaced with the unsigned integer value (base 8)
 *		%lo	Replaced with the unsigned long value (base 8)
 *		%b	Replaced with the unsigned integer value (base 2)
 *		%lb	Replaced with the unsigned long value (base 2)
 *		%p	Replaced with the pointer value, printed in hex
 */

#if PGP_DEBUG				/* [ */

#define PGP_DEBUG_BUF_SIZE		256

#if PGP_UNIX
#define PGP_DEBUG_STACK_BUFFERS 1
#endif

#if PGP_DEBUG_STACK_BUFFERS		/* [ */

#define PGP_DEBUG_ALLOC_BUF											\
	PFLChar pgpaBuf[PGP_DEBUG_BUF_SIZE];						\
		int		pgpaBufSize = sizeof(pgpaBuf)/sizeof(PFLChar);
#define PGP_DEBUG_DEALLOC_BUF

#else

#ifdef PGP_MOBILE

 #define PGP_DEBUG_NUM_BUFS		2

#define PGP_DEBUG_ALLOC_BUF											\
	int pgpaBufSize = PGP_DEBUG_BUF_SIZE;							\
	PFLChar *pgpaBuf = (pgpaDebugBufStack -= PGP_DEBUG_BUF_SIZE);

#define PGP_DEBUG_DEALLOC_BUF										\
	pgpaDebugBufStack += PGP_DEBUG_BUF_SIZE;

#elif PGP_WIN32 
#define PGP_DEBUG_NUM_BUFS		20

#define PGP_DEBUG_ALLOC_BUF											\
	int		pgpaBufSize = PGP_DEBUG_BUF_SIZE;						\
	PFLChar *pgpaBuf;												\
	_InterlockedExchangeAdd((long*)&pgpaDebugBufStack, -PGP_DEBUG_BUF_SIZE); \
	pgpaBuf = pgpaDebugBufStack;									\
	if (pgpaBuf < pgpaDebugBuf)	/* opps, have gone back too far */	\
	{																\
		pgpaBuf = 0;												\
		_InterlockedExchangeAdd((long*)&pgpaDebugBufStack, PGP_DEBUG_BUF_SIZE); \
	}

#define PGP_DEBUG_DEALLOC_BUF										\
	if (pgpaBuf != 0)												\
		_InterlockedExchangeAdd((long*)&pgpaDebugBufStack, PGP_DEBUG_BUF_SIZE);

#else

#define PGP_DEBUG_NUM_BUFS		2

#define PGP_DEBUG_ALLOC_BUF											\
	int pgpaBufSize = PGP_DEBUG_BUF_SIZE;							\
	PFLChar *pgpaBuf = (pgpaDebugBufStack -= PGP_DEBUG_BUF_SIZE);

#define PGP_DEBUG_DEALLOC_BUF										\
	pgpaDebugBufStack += PGP_DEBUG_BUF_SIZE;

#endif

extern  PFLChar	pgpaDebugBuf[PGP_DEBUG_BUF_SIZE	* PGP_DEBUG_NUM_BUFS];
extern  PFLChar *	pgpaDebugBufStack;
extern	long pgpDebugRateCount;
#endif	/* ] PGP_DEBUG_STACK_BUFFERS */

#if PGP_MOBILE

#define pgpDebugPStr(pStr)		
#define pgpDebugHybridStr(hStr)				
#define pgpDebugMsg(message)									
#define pgpDebugPtrReadAccess(ptr,size) 1

#elif PGP_MACINTOSH			/* [ */

#define pgpDebugPStr(pStr)			DebugStr(pStr)
#define pgpDebugHybridStr(hStr)		pgpDebugPStr( hStr ) 
		
#define pgpDebugMsg(message)										\
		do															\
		{															\
			PFLChar	PGP__msg_buf[256];								\
																	\
			pgpFormatPStr(PGP__msg_buf, sizeof(PGP__msg_buf),		\
							TRUE, PFLTXT_DEBUG("%s"), (message));	\
			pgpDebugPStr(PGP__msg_buf);								\
		} while (0)

#elif defined(PGP_WIN32)	/* ] [ */

extern char *pgpAllocCurrentStackAsString(void);
__inline void pgpDebugMsgWin32( const char *message )  {
#if !defined(PGP_DRIVERNT)
	/* no support in drivers */
	const int old = _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_REPORT_MODE );
	int do_free=1;
	const char * stack = 
#ifdef PGP_SDK
		pgpAllocCurrentStackAsString();
#else
		NULL;
#endif
	if( stack==NULL )  {
		do_free = 0;
		stack = "";
	}
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
	_CrtDbgReport(_CRT_ASSERT, NULL, 0, NULL,
		PGPTXT_DEBUG8("\r\n%s%s"), (message), stack);
	_CrtSetReportMode( _CRT_ASSERT, old );
	if( do_free )
		free( (void*)stack );
#endif
	if( pgpDebugRateCount >= 0 )  {
		if (_CrtDbgReport(
			_CRT_ASSERT, NULL, 0, NULL,	PGPTXT_DEBUG8("\r\n%s"), (message)) == 1 )
		{
			_CrtDbgBreak();
			_InterlockedIncrement( &pgpDebugRateCount );
		}
#if !defined(PGP_DRIVERNT)
		else  {
			if( _InterlockedIncrement( &pgpDebugRateCount ) >= 5 && 
				_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_REPORT_MODE )==_CRTDBG_MODE_WNDW ) 
			{
				if (_CrtDbgReport(_CRT_ASSERT, NULL, 0, NULL,
					"Do you want to suspend all subsequent SDK asserts in this process?\n"
					"Press Ignore to do so or press Retry to see next assert.\n\n"
					"Debug asserts will be sent to debug window regardless of your choice\n"
					"and you will be able to suspend asserts later if you choose Retry\n")==0 )
				{
					pgpDebugRateCount=-1;
					_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
				}
				else  {
					pgpDebugRateCount = 0;
				}
			}
		}
#endif
	}
}

#define pgpDebugMsg8(message) pgpDebugMsgWin32(message)

#define pgpDebugMsg pgpDebugMsg8

#define pgpDebugHybridStr(hStr)	pgpDebugMsg((PFLChar *)(hStr) + 1)

/* Allows to probe potentially invalid memory so it can be safely read */
#if !defined(PGP_DRIVERNT)
#define pgpDebugPtrReadAccess(ptr,size)	_CrtIsValidPointer( ptr, size, FALSE )		
#else
#define pgpDebugPtrReadAccess(ptr,size)	1
#endif

#else	/* ] [ */

#if PGP_UNICODE
#define pgpDebugMsg(message)										\
		do															\
		{															\
			fwprintf(stderr, PFLTXT_DEBUG("PGP[%d] %s\n"), (int) getpid(), (PFLChar *)(message));				\
			pgpDebugHook();											\
		} while (0)
#else
#define pgpDebugMsg(message)										\
		do															\
		{															\
			fprintf(stderr, PFLTXT_DEBUG("%s\n"),					\
									(PFLChar *)(message));			\
			pgpDebugHook();											\
		} while (0)
#endif

#define pgpDebugHybridStr(hStr)	pgpDebugMsg((PFLChar *)(hStr) + 1)

#define pgpDebugPtrReadAccess(ptr,size) (ptr != NULL)

#endif	/* ] */

#define pgpDebugFmtMsg(params)										\
		do															\
		{															\
			PFLChar	pgpaBuf[256];									\
			int		pgpaBufSize = sizeof(pgpaBuf)/sizeof(PFLChar);	\
																	\
			pgpDebugHybridStr(pgpDebugFormatHStr params);			\
		} while (0)

#define pgpDebugMsgIf(condition, message)							\
		do															\
		{															\
			if (condition)											\
				pgpDebugMsg(message);								\
		} while (0)

/*
 * The first couple parameters which must always be
 * passed as the beginning of a format list.
 */
#define pgpaFmtPrefix												\
		pgpaBuf, pgpaBufSize

/*
 * The general pgpa command to check a condition and
 * print out a formatted message if it's not true.
 */
#define pgpaFailIf(condition, params)								\
		((void)(pgpaFailed || !(condition) ||						\
			((pgpaFailed = TRUE), (pgpDebugFormatHStr params))))


/*
 * A simple pgpa command to assert a given condition.
 */
#define pgpaAssert(condition)										\
		pgpaFailIf(!(condition),									\
			(pgpaFmtPrefix, PFLTXT_DEBUG("(%s) not true"), PFLTXT_MACHINE(#condition)))

/*
 * Used to add more formatted information to an assertion failure
 */
#define pgpaFmtMsg(params)											\
		((void)(pgpaFailed && (pgpDebugPrefixFormatHStr params)))
#define pgpaMsg(message)											\
		pgpaFmtMsg((pgpaFmtPrefix, PFLTXT_DEBUG("%s"), message))

/*
 * The first couple parameters which must always be
 * passed as the beginning of a pgpaCall parameter list.
 */
#define pgpaCallPrefix												\
		pgpaBuf, pgpaBufSize, pgpaFailed
#define pgpaCallPrefixDef											\
		PFLChar *pgpaBuf, int pgpaBufSize, PGPBoolean pgpaFailed

/*
 * The general pgpa command to call a pgpa-style function.
 */
#define pgpaCall(function, params)									\
		((void)(pgpaFailed || ((pgpaFailed = (function params)) != FALSE)))

#if !defined( __FUNCTION__ ) && !defined( __GNUC__ ) 
#define __FUNCTION__ "<unknown>"
#endif

/*
 * The main pgpa macro from which all the pgpa
 * macros must be used as parameters.
 */
#define pgpa(assertions)											\
		{															\
			PGPBoolean		pgpaFailed = FALSE;						\
			PGP_DEBUG_ALLOC_BUF										\
																	\
			(assertions);											\
			pgpaFmtMsg((pgpaFmtPrefix,								\
			PFLTXT_DEBUG("%s(%d): function %s: ASSERTION FAILED"),			\
						PFLTXT_MACHINE(__FILE__), (long)__LINE__, __FUNCTION__));					\
			if (pgpaFailed)											\
				pgpDebugHybridStr(pgpaBuf);							\
			PGP_DEBUG_DEALLOC_BUF									\
		}

#define pgpaAddrValid(addr, pointedAtType)							\
		pgpaCall(pgpaInternalAddrValid,								\
				(pgpaCallPrefix, addr, alignof(pointedAtType), PFLTXT_MACHINE(#addr)))

#define pgpaStrLenValid(str, minLen, maxLen)						\
		pgpaCall(pgpaInternalStrLenValid,							\
				(pgpaCallPrefix, str, minLen, maxLen, PFLTXT_MACHINE(#str)))
#define pgpaPStrLenValid(pstr, minLen, maxLen)						\
		pgpaCall(pgpaInternalPStrLenValid,							\
				(pgpaCallPrefix, pstr, minLen, maxLen, PFLTXT_MACHINE(#pstr)))

/* pgpaStrValid currently assumes an arbitrary reasonable length */
#define pgpaStrValid(str)											\
		pgpaStrLenValid(str, 0, 32767)
#define pgpaPStrValid(pstr)											\
		pgpaPStrLenValid(pstr, 0, 255)

#else /* ] PGP_DEBUG [ */

#define pgpa(assertions)			((void)(0))
#define pgpDebugMsgIf(cond, msg)	((void)(0))
#define pgpDebugPStr(pStr)			((void)(0))
#define pgpDebugFmtMsg(params)		((void)(0))
#define pgpDebugMsg(msg)			((void)(0))
#define pgpDebugPtrReadAccess(ptr,size)			1

#endif /* ] PGP_DEBUG */

/*
 * Convenient short-hands follow
 */

#define pgpAssert(condition)										\
		pgpa(pgpaAssert(condition))
#define pgpAssertMsg(condition, message)							\
		pgpa((														\
			pgpaAssert(condition),									\
			pgpaMsg(message)))

#define pgpAssertAddrValid(ptr, type)								\
		pgpa(pgpaAddrValid(ptr, type))
#define pgpAssertAddrValidMsg(ptr, type, message)					\
		pgpa((														\
			pgpaAddrValid(ptr, type),								\
			pgpaMsg(message)))

#define pgpAssertStrLenValid(str, minLen, maxLen)					\
		pgpa(pgpaStrLenValid(str, minLen, maxLen))
#define pgpAssertStrLenValidMsg(str, minLen, maxLen, message)		\
		pgpa((														\
			pgpaStrLenValid(str, minLen, maxLen),					\
			pgpaMsg(message)))

#define pgpAssertPStrLenValid(pstr, minLen, maxLen)					\
		pgpa(pgpaPStrLenValid(pstr, minLen, maxLen))
#define pgpAssertPStrLenValidMsg(pstr, minLen, maxLen, message)		\
		pgpa((														\
			pgpaPStrLenValid(pstr, minLen, maxLen),					\
			pgpaMsg(message)))
#ifdef UNICODE
#define pgpAssertStrValid(str)
#else
#define pgpAssertStrValid(str)										\
		pgpa(pgpaStrValid(str))
#endif
#define pgpAssertStrValidMsg(str, msg)								\
		pgpa((														\
			pgpaStrValid(str, minLen, maxLen),						\
			pgpaMsg(message)))

#define pgpAssertPStrValid(pstr)									\
		pgpa(pgpaPStrValid(pstr))
#define pgpAssertPStrValidMsg(pstr, message)						\
		pgpa((														\
			pgpaPStrValid(pstr, minLen, maxLen),					\
			pgpaMsg(message)))


#define pgpAssertNoErr( err ) 	pgpAssert( IsntPGPError( err ) || 	\
		err == kPGPError_UserAbort )
#define pgpAssertErrWithPtr(err, ptr) \
		pgpAssert( ( IsntPGPError( err ) && (ptr) != NULL ) || \
		( IsPGPError( err ) && (ptr) == NULL ) )
		
		
int			pgpFormatVAStr(PFLChar *buffer, int bufferSize,
				PGPBoolean putLengthPrefix, PGPBoolean putNullTerminator,
				PGPBoolean canonicalizeNLs, PFLChar const *formatStr,
				va_list args);
PFLChar *		pgpFormatHStr(PFLChar *buffer, int bufferSize,
						PGPBoolean canonicalizeNLs, PFLChar const *formatStr, ...);
PFLChar *		pgpFormatPStr(PFLChar *buffer, int bufferSize,
						PGPBoolean canonicalizeNLs, PFLChar const *formatStr, ...);
PFLChar *		pgpFormatPStr255(PFLChar *buffer, PGPBoolean canonicalizeNLs,
						PFLChar const *formatStr, ...);
PFLChar *		pgpFormatStr(PFLChar *buffer, int bufferSize,
					PGPBoolean canonicalizeNLs,
					PFLChar const *formatStr, ...);


#if PGP_DEBUG	/* [ */
#define pgpDebugWhackMemory( buffer, length )	\
	pgpFillMemory( buffer, length, 0xDD )
#else
#define pgpDebugWhackMemory( buffer, length )	/* nothing */
#endif

#if PGP_DEBUG	/* [ */

PFLChar  *	pgpDebugFormatHStr(PFLChar *buffer, int bufferSize,
								PFLChar const *formatStr, ...);
PFLChar  *	pgpDebugPrefixFormatHStr(PFLChar *buffer,
								int bufferSize, PFLChar const *formatStr, ...);

/*
 * These are internal routines which should only be called by the above macros
 */
void 		pgpDebugFillMemory(void *buffer, size_t length);
PGPBoolean		pgpaInternalAddrValid(pgpaCallPrefixDef,
									void const *addr, int alignSize,
									PFLChar const *varName);
PGPBoolean		pgpaInternalStrLenValid(pgpaCallPrefixDef,
									PFLChar const *str, PGPUInt32 minLen,
									PGPUInt32 maxLen, PFLChar const *varName);
PGPBoolean		pgpaInternalPStrLenValid(pgpaCallPrefixDef,
									PFLChar const *str, int minLen,
									int maxLen, PFLChar const *varName);

/*
 * Except on the Mac, when debugger messages are printed or assertions
 * fail, this routine will always be called.  Its only purpose is to be
 * used as a breakpoint location, to catch failed assertions from the
 * debugger.
 */
void		pgpDebugHook(void);

#endif /* ] PGP_DEBUG */

PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpDebug_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
