/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	pgpUnicode.h	-	cross-platform Unicode conversion calls
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpUnicode_h	/* [ */
#define Included_pgpUnicode_h


#include "pgpTypes.h"
#include "pgpPFLErrors.h"

/*____________________________________________________________________________
 * For functions that accept a 'Flags' value, passing kPGPUnicodeFlag_Secure
 * will cause the function to break up strings into individual characters
 * when passing info to OS-supplied conversion functions.  This prevents
 * buffers of sensitive information being leaked by the OS routines.
 */

#define kPGPUnicodeNullTerminated	0xFFFFFFFF

PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
 * Convert a string from Unicode UTF-8 to UTF-16.  This doesn't call
 * into any system functions and thus is secure.
 *
 * The output of the 'Z' variant will always be Null-terminated.  The
 * other two variants will null-terminate the output if there is room.
 * The output length does not include the terminating Null.
 * If the input string is not a valid UTF-8 string, the output length
 * will be returned as zero and the return value will indicate the
 * position of the invalid character in the input string.
 *
 * uLenUTF8 is in units of PGPUTF8s (i.e. bytes) while uMaxLenUTF16
 * and puLenOut are in units of PGPChar16s.
 *
 * If the input string is Null-terminated, you can avoid calculating
 * the length and just pass in kPGPUnicodeNullTerminated for uLenUTF8. 
 *
 * You can pass in NULL for pszUTF16 if you don't want the output, but
 * just want to get the output length (for allocating buffers, etc.).
 *
 * You can pass in NULL for puLenOut if you don't care about the output
 * length.
 *
 * The "Ex" version of the function returns the number of bytes used 
 * from the input string in puBytesUsed.  If there is an error in the
 * conversion, the value of *puBytesUsed is the byte offset of the 
 * first offending character in the input string.
 *
 * The return value is one of kPGPError_NoErr, kPGPError_BufferTooSmall,
 * or kPGPError_StringOpFailed.
 */

PGPError
pgpUTF8StringToUTF16 (
		const PGPUTF8*	pszUTF8,
		PGPUInt32		uLenUTF8,
		PGPChar16*		pszUTF16,
		PGPUInt32		uMaxLenUTF16,
		PGPUInt32*		puLenOut);

PGPError
pgpUTF8StringToUTF16Z (
		const PGPUTF8*	pszUTF8,
		PGPUInt32		uLenUTF8,
		PGPChar16*		pszUTF16,
		PGPUInt32		uMaxLenUTF16,
		PGPUInt32*		puLenOut);

PGPError
pgpUTF8StringToUTF16Ex (
		const PGPUTF8*	pszUTF8,
		PGPUInt32		uLenUTF8,
		PGPChar16*		pszUTF16,
		PGPUInt32		uMaxLenUTF16,
		PGPUInt32*		puLenOut,
		PGPUInt32*		puBytesUsed);


/*____________________________________________________________________________
 * Convert a string from Unicode UTF-8 to UCS-2.  Since UCS-2 is a 
 * subset of UTF-16, this call is deprecated; use the above 
 * pgpUTF8StringToUTF16 instead.  This doesn't call into any system 
 * functions and thus is secure.
 *
 * The output of the 'Z' variant will always be Null-terminated.  The
 * other two variants will null-terminate the output if there is room.
 * The output length does not include the terminating Null.
 * If the input string is not a valid UTF-8 string, the output length
 * will be returned as zero and the return value will indicate the
 * position of the invalid character in the input string.
 *
 * uLenUTF8 is in units of PGPUTF8s (i.e. bytes) while uMaxLenUCS2
 * and puLenOut are in units of PGPChar16s.
 *
 * If the input string is Null-terminated, you can avoid calculating
 * the length and just pass in kPGPUnicodeNullTerminated for uLenUTF8. 
 *
 * You can pass in NULL for pszUCS2 if you don't want the output, but
 * just want to get the output length (for allocating buffers, etc.).
 *
 * You can pass in NULL for puLenOut if you don't care about the output
 * length.
 *
 * The return value is one of kPGPError_NoErr, kPGPError_BufferTooSmall,
 * or kPGPError_StringOpFailed.
 */

PGPError
pgpUTF8StringToUCS2 (
		const PGPUTF8*	pszUTF8,
		PGPUInt32		uLenUTF8,
		PGPChar16*		pszUCS2,
		PGPUInt32		uMaxLenUCS2,
		PGPUInt32*		puLenOut);

PGPError
pgpUTF8StringToUCS2Z (
		const PGPUTF8*	pszUTF8,
		PGPUInt32		uLenUTF8,
		PGPChar16*		pszUCS2,
		PGPUInt32		uMaxLenUCS2,
		PGPUInt32*		puLenOut);


/*____________________________________________________________________________
 * Convert a string from Unicode UTF-16 to UTF-8.  This doesn't call
 * into any system functions and thus is secure.
 *
 * The output of the 'Z' variant will always be Null-terminated.  The
 * other two variants will null-terminate the output if there is room.
 * The output length does not include the terminating Null.
 *
 * uLenUTF16 is in units of PGPChar16s while uMaxLenUTF8 and puLenOut
 * are in units of PGPUTF8s (i.e. bytes).
 *
 * If the input string is Null-terminated, you can avoid calculating
 * the length and just pass in kPGPUnicodeNullTerminated for uLenUTF16. 
 *
 * You can pass in Null for pszUTF8 if you don't want the output, but
 * just want to get the output length (for allocating buffers, etc.).
 *
 * You can pass in Null for puLenOut if you don't care about the output
 * length.
 *
 * The return value is one of kPGPError_NoErr, kPGPError_BufferTooSmall,
 * or kPGPError_StringOpFailed.
 */

PGPError
pgpUTF16StringToUTF8 (
		const PGPChar16*	pszUTF16,
		PGPUInt32		uLenUTF16,
		PGPUTF8*		pszUTF8,
		PGPUInt32		uMaxLenUTF8,
		PGPUInt32*		puLenOut);

PGPError
pgpUTF16StringToUTF8Z (
		const PGPChar16*	pszUTF16,
		PGPUInt32		uLenUTF16,
		PGPUTF8*		pszUTF8,
		PGPUInt32		uMaxLenUTF8,
		PGPUInt32*		puLenOut);


/*____________________________________________________________________________
 * Convert a string from Unicode UCS-2 to UTF-8.  Since UCS-2 is a 
 * subset of UTF-16, this call is deprecated; use the above 
 * pgpUTF16StringToUTF8 instead.  This doesn't call into any system 
 * functions and thus is secure.
 *
 * The output of the 'Z' variant will always be Null-terminated.  The
 * other two variants will null-terminate the output if there is room.
 * The output length does not include the terminating Null.
 *
 * uLenUCS2 is in units of PGPChar16s while uMaxLenUTF8 and puLenOut
 * are in units of PGPUTF8s (i.e. bytes).
 *
 * If the input string is Null-terminated, you can avoid calculating
 * the length and just pass in kPGPUnicodeNullTerminated for uLenUCS2. 
 *
 * You can pass in Null for pszUTF8 if you don't want the output, but
 * just want to get the output length (for allocating buffers, etc.).
 *
 * You can pass in Null for puLenOut if you don't care about the output
 * length.
 *
 * The return value is one of kPGPError_NoErr, kPGPError_BufferTooSmall,
 * or kPGPError_StringOpFailed.
 */

PGPError
pgpUCS2StringToUTF8 (
		const PGPChar16*	pszUCS2,
		PGPUInt32		uLenUCS2,
		PGPUTF8*		pszUTF8,
		PGPUInt32		uMaxLenUTF8,
		PGPUInt32*		puLenOut);

PGPError
pgpUCS2StringToUTF8Z (
		const PGPChar16*	pszUCS2,
		PGPUInt32		uLenUCS2,
		PGPUTF8*		pszUTF8,
		PGPUInt32		uMaxLenUTF8,
		PGPUInt32*		puLenOut);


/*____________________________________________________________________________
 * Determine if the input string is not a 7-bit ASCII string (i.e. test
 * if any of the bytes have bit-8 set). 
 *
 * uLength is in units of PGPChar8s (i.e. bytes). 
 *
 * If the input string is Null-terminated, you can avoid calculating
 * the length and just pass in kPGPUnicodeNullTerminated for uLength. 
 */

PGPBoolean
pgpIsntASCIIString (
		const PGPChar8*		pszString,
		PGPUInt32		uLength);


/*____________________________________________________________________________
 * Determine if the input string is a valid UTF-8 encoded string. It's 
 * easier to test if a string is *not* UTF-8 than to prove that it is!
 *
 * uLength is in units of PGPChar8s (i.e. bytes). 
 *
 * If the input string is Null-terminated, you can avoid calculating
 * the length and just pass in kPGPUnicodeNullTerminated for uLength. 
 */

PGPBoolean
pgpIsntUTF8String (
		const PGPChar8*		pszString,
		PGPUInt32		uLength);


/*____________________________________________________________________________
 * Convert UCS2 string to uppercase
 */

void 
pgpUCS2StringToUpper( PGPUInt16 *in, PGPSize outSize ); 


/*____________________________________________________________________________
 * Convert Unicode code point to its uppercase alternative, if one exists.
 * Otherwise returns original value (similiar to toupper()). 
 * Note that 'c', generally speaking, is not UTF-8, UCS-2, UTF-16, etc, element 
 * of Unicode encodings or a value on the cdepage. 
 */
PGPUInt pgpUnicodePointToUpper(PGPUInt c);

/* ____________________________________________________________________________
 * Return TRUE for Unicode whitespace character
 */

PGPBoolean 
pgpUnicodePointIsWhitespace( PGPUInt32 c );
#define pgpUnicodeCharIsWhitespace pgpUnicodePointIsWhitespace

/* ____________________________________________________________________________
 * Return TRUE for Unicode punctuation character
 */

PGPBoolean 
pgpUnicodePointIsPunctuation( PGPUInt32 c );
#define pgpUnicodeCharIsPunctuation pgpUnicodePointIsPunctuation

/*____________________________________________________________________________
	Functions for working with UTF-8 directly. 
____________________________________________________________________________*/

/* see also pgpIncrementStringPointer and PGPStrInc */
const PGPUTF8* pgpIncrementUTF8StringPointer( const PGPUTF8* p );

/* returns current Unicode code point at the position p */
PGPUInt pgpUTF8ToUnicodePoint( const PGPUTF8* p );

/* returns size of the sequence representing one code point (in bytes), or 0 for error */
unsigned pgpUTF8CharSize( const PGPUTF8* p );

/* Writes UTF-8 sequence corresponding to condepoint into p, null-terminating it;
 * returns the size of the sequence in bytes */
PGPUInt pgpUnicodePointToUTF8( PGPUInt codepoint, PGPUTF8 p[5] );

PGP_END_C_DECLARATIONS


#endif /* ] */

