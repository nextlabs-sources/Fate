/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpParseCommandLine_h	/* [ */
#define Included_pgpParseCommandLine_h

#include "pgpBase.h"
#include "pgpMemoryMgr.h"

PGP_BEGIN_C_DECLARATIONS

typedef struct PGPCommandLineContext *		PGPCommandLineContextRef;

#define kInvalidPGPCommandLineContextRef		( (PGPCommandLineContextRef) NULL )

#define PGPCommandLineContextRefIsValid(con)	( (con) != kInvalidPGPCommandLineContextRef )

typedef struct PGPCommandLineLongArg
{
	char *		argString;
	PGPUInt32	value;
} PGPCommandLineLongArg;


/*
 * Create a new PGPCommandLineContext.  This is needed to allocate needed
 * memory and to pass the argc/argv to the ParseCommandLine routines.
 */
	PGPError
PGPNewCommandLineContext(
	PGPMemoryMgrRef				mgr,
	PGPUInt32					argc,
	char *						argv[],
	PGPCommandLineLongArg *		longArgs,
	PGPSize						numLongArgs,
	PGPCommandLineContextRef *	clContext );

/* Free the PGPCommandLineContext */
	PGPError
PGPFreeCommandLineContext(
	PGPCommandLineContextRef	con );

/*
 * Return the next flag given on the command line.
 *
 * A 'flag' is defined as a single letter following a hyphen ('-'), or a single
 * letter in a group of letters following a hyphen ('-'), or a string following
 * two hyphens ("--").  If a long argument is found, the value associated with
 * that long argument in the array of PGPCommandLineLongArgs passed into
 * PGPNewCommandLineContext is returned in flag.
 *
 * kPGPError_EndOfIteration is returned when there are no more flags left to
 * read.
 */
	PGPError
PGPCommandLineNextFlag(
	PGPCommandLineContextRef	con,
	PGPUInt32 *					flag );

/*
 * Return the next argument given on the command line.
 *
 * An 'argument' is defined as a string that does not begin with one or two 
 * hyphens ('-').  However, the single character string "-" DOES qualify as
 * an argument because some programs use "-" to signify reading input from
 * stdin.
 *
 * Note that the terminating null byte is NOT included in outLen.
 *
 * kPGPError_EndOfIteration is returned when there are no more arguments left
 * to read.
 *
 * kPGPError_ItemNotFound is returned when the next argument found was a flag
 * or set of flags.  We limit arguments found to be the arguments found after
 * the last flag we returned and before the next flag on the command line.
 */
	PGPError
PGPCommandLineNextArgument(
	PGPCommandLineContextRef	con,
	PFLChar *					arg,
	PGPSize						allocLen,
	PGPSize *					outLen );

/*
 * Returns the text following a single character argument.
 *
 * For example, if "-krs" is given on the command line, PGPCommandLineNextFlag
 * will return the character 'k', and PGPCommandLineGetFollowingText will
 * return the string "rs".
 *
 * Note that the terminating null byte is NOT included in outLen.
 */
	PGPError
PGPCommandLineGetFollowingText(
	PGPCommandLineContextRef	con,
	PFLChar *					arg,
	PGPSize						allocLen,
	PGPSize *					outLen );

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpParseCommandLine_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
