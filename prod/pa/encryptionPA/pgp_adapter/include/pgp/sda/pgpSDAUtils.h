/*____________________________________________________________________________
	Copyright (C) 2002-2004 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpSDAUtils_h
#define Included_pgpSDAUtils_h


/****************************************************************************
	Includes
****************************************************************************/

#include "pgpSDA.h"
#include "pgpSDAPriv.h"


/****************************************************************************
	Public Functions
****************************************************************************/

/*
 *	Endian convert PGPUInt* data types if necessary.
 *	If no conversion is necessary we just return the input value.
 *	Storage format is little endian.
 */
void
pgpUInt64EndianConversion(
	PGPUInt64		input,
	PGPByte *		pOutput);

void
pgpUInt32EndianConversion(
	PGPUInt32		input,
	PGPByte *		pOutput);

void
pgpUInt16EndianConversion(
	PGPUInt16		input,
	PGPByte *		pOutput);


/*
 *	Endian convert the correct bits of the SDA headers if necessary.
 *	Storage format is little endian.
 */
PGPError
pgpSDACorrectHeaderForEndianness(
	PGPsdaHeaderRef		header);

PGPError
pgpSDACorrectFooterForEndianness(
	PGPsdaFooterRef		footer);


/*
 *	Convert paths to local encoding
 */
PGPError
pgpSDANormalizePathChars(
	PGPChar8 *				object);

/*
 *	Convert path to local encoding, remove any relative-ness and check for errors
 *
 *	Return Value:
 *		kPGPError_BadParams - No path (NULL) or path is ""
 *		kPGPError_ImproperInitialization - Path is ""
 *		kPGPError_CorruptData - Path ends with /. or /..
 *		kPGPError_FeatureNotAvailable - Path contains ../
 *		kPGPError_FileNotFound - Nothing left after changes
 */
PGPError
pgpSDANormalizePath(
	PGPChar8 *				path);

/*
 *	Remove root path from an object
 *
 *	Notes:
 *		Both these inputs must be normalized
 *
 *	Parameters:
 *		object - input object
 *		rootPath - Root path to remove from the input object (optional)
 *			If this is specified, it must exist in the input
 *
 *	Return Value:
 *		kPGPError_ItemNotFound - Root path doesn't match
 *		kPGPError_InputFile - Root path matched in the middle of object name
 *		kPGPError_SkipSection - Root path matched exactly
 */
PGPError
pgpSDAApplyRootPath(
	PGPChar8 *				object,
	PGPChar8 *				rootPath);

/*
 *	Return a pointer to the last character in a string.
 *
 *	Return Value:
 *		NULL if string is NULL
 *		NULL if string is the empty string
 */
PGPChar *
pgpSDAGetLastChar(
	PGPChar *			string);


/*
 *	Check to see if an object exists (as anything)
 */
PGPBoolean
pgpSDAFileExists(
	PGPChar *			object);


/*
 *	Memory functions for use with deflate
 */
void *
pgpSDADeflateAllocMem(
	void *			dummy,
	unsigned int	nItems,
	unsigned int	nSize);

void
pgpSDADeflateFreeMem(
	void *		dummy,
	void *		address);


#endif /* Included_pgpSDAUtils_h	*/

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


