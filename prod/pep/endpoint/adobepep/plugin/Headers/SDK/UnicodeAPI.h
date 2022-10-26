/***********************************************************************/
/*                                                                     */
/* Copyright 1990-1998 Adobe Systems Incorporated.                     */
/* All Rights Reserved.                                                */
/*                                                                     */
/* Patents Pending                                                     */
/*                                                                     */
/* NOTICE: All information contained herein is the property of Adobe   */
/* Systems Incorporated. Many of the intellectual and technical        */
/* concepts contained herein are proprietary to Adobe, are protected   */
/* as trade secrets, and are made available only to Adobe licensees    */
/* for their internal use. Any reproduction or dissemination of this   */
/* software is strictly forbidden unless prior written permission is   */
/* obtained from Adobe.                                                */
/*                                                                     */
/* PostScript and Display PostScript are trademarks of Adobe Systems   */
/* Incorporated or its subsidiaries and may be registered in certain   */
/* jurisdictions.                                                      */
/*                                                                     */
/***********************************************************************/

/*
 *		  Name: UnicodeAPI.h
 *	   Purpose: Unicode conversion engine API.
 *		Author: Jon Reid
 *	   Created: December 7, 1998
 */

#ifndef __UnicodeAPI__
#define __UnicodeAPI__


#ifndef __ASTypes__
#include "ASTypes.h"
#endif

#include <stddef.h>		// Define size_t.

#ifdef __cplusplus
extern "C" {
#endif


/*
	ZStrings are represented internally as unterminated Unicode strings.
	A conversion engine handles conversions between Unicode and local
	encodings. Different clients can provide different conversion engines
	according to their needs.

	Each conversion engine can represent encodings in different ways. To
	pass different representations through the same C-based API, we use
	"poor man's run-time type identification." The following data structure
	identifies the type:
*/

typedef struct EncodingInfo
{
	long typeID;
} EncodingInfo;

/*
	The typeID is an arbitrary value that identifies the actual
	representation. To use "poor man's inheritance," EncodingInfo must be
	the first element of the actual data structure. Here is an example of
	a simple representation of a Mac script:

	struct MacScript
	{
		EncodingInfo type;	// Set type.typeID to 'MacS'
		short script;
	};

	A pointer to an actual representation is cast to an EncodingInfo* for
	use in one of the following functions:
*/

// Return size of buffer needed to convert multi-byte string to Unicode
// string without terminating null character.
typedef ASAPI ASErr
(*ToUnicodeSizeProc)(
	const ASByte*		source,			// Multi-byte string.
	ASUInt32			sourceLength,	// Number of bytes to convert.
	ASUInt32*			bufferSize,
	EncodingInfo*	 	encodingInfo
	);

// Convert multi-byte string to Unicode, up to destBufferSize characters.
// Do not add terminating null character.
typedef ASAPI ASErr
(*ConvertToUnicodeProc)(
	const ASByte*		source,			// Multi-byte string.
	ASUInt32			sourceLength,	// Number of bytes to convert.
	ASUnicode*			destination,
	ASUInt32			destBufferSize,	// Size of destination buffer.
	EncodingInfo*	 	encodingInfo
	);

// Return size of buffer needed to convert from Unicode. Be sure to include
// extra byte for terminating null character or Pascal length.
typedef ASAPI ASErr
(*FromUnicodeSizeProc)(
	const ASUnicode*	source,			// Unterminated Unicode string.
	ASUInt32			sourceLength,	// Number of characters to convert.
	ASUInt32*			bufferSize,
	EncodingInfo*		encodingInfo
	);

// Convert Unicode to multi-byte string, up to destBufferSize-1 bytes.
// Do *not* add a terminating null character, because this routine is used
// for both C and Pascal strings.
typedef ASAPI ASErr
(*ConvertFromUnicodeProc)(
	const ASUnicode*	source,				// Unterminated Unicode string.
	ASUInt32			sourceLength,		// Number of characters to convert.
	ASByte*				destination,
	ASUInt32		 	destBufferSize,		// Size of destination buffer.
	ASUInt32*			convertedLength,	// Number of converted bytes (without null).
	EncodingInfo*		encodingInfo
	);

/*	
	These functions must be registered as callbacks with the ZString
	plug-in. If the Unicode conversion engine is packaged as a plug-in,
	make sure it cannot be unloaded so that its callbacks are always
	available.

	The conversion engine takes the encodingInfo and casts it back to a
	pointer to the data structure associated with the particular typeID.
	If it does not recognize the typeID, it should return kUnknownEncodingType.
*/

#define kUnknownEncodingType	'UETp'

	
/*
	The Unicode conversion engine may modify the contents of the data
	structure passed in encodingInfo. This allows the conversion engine to
	pass back more than just an error value and the converted string; it
	can more fully describe the conversion results, such as whether
	fallback representations were used for characters that do not have
	precise representations in the local encoding. One could also ask that
	a Unicode string be converted to an encoding that best represents the
	string, and the conversion engine would pass back the encoding it chose.
	
	The byte-order of the Unicode data (big-endian / little-endian) is the
	platform's native order, unless specified otherwise in encodingInfo.

	If a conversion routine detects that the buffer is not large enough to
	hold the result of a conversion, it should convert as much as it can
	and then return an error value of kBufferTooSmallErr.
*/

#define kBufferTooSmallErr		'BUFF'


/*
	The following function is registered separately from the other callbacks:
*/

// Compare two Unicode strings, analogous to strcmp except that the strings
// are not null-terminated.
typedef ASAPI int
(*UnicodeCompareProc)( const ASUnicode* str1, ASUInt32 len1,
					const ASUnicode* str2, ASUInt32 len2 );
	

#ifdef __cplusplus
}
#endif

#endif	// __UnicodeAPI__
