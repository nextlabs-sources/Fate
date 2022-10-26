/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpStrings_h	/* [ */
#define Included_pgpStrings_h

#include "pgpBase.h"
#include <stdio.h>
#if !PGP_WIN32
#   include <string.h>
#endif

#if PGP_UNICODE
	#include <wchar.h>
#endif

#if PGP_SYMBIAN
#	include <stdarg.h>
#endif

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#if PGP_WIN32
#   define INLINE		__inline
#else
#   define INLINE		inline
#endif

/* PGP functions for use with PGPChar datatype 
 *
 *	Note the difference between 'N' and 'M' in the following function names.
 *  'N' is used for _character_ counts, while 'M' is used for counts of
 *	_PGPChars_.  A PGPChar is always 8 or 16 bits, as controlled by the
 *  PGP_UNICODE symbol.  A character is 8 bits if PGP_UNICODE is 0; it is
 *	either 16 or 32 bits (due to surrogates) if PGP_UNICODE is 1.
 *
 *	PGPCharIsAlpha				is alphabetic
 *	PGPCharIsASCII				is in 7-bit ASCII character range
 *	PGPCharIsBlank				is a whitespace
 *	PGPCharIsDigit				is a digit
 *	PGPCharIsLower				is lowercase
 *	PGPCharIsPunct				is a punctuation mark
 *	PGPCharIsPrint				is a printable character
 *	PGPCharIsSpace				is a whitespace
 *	PGPCharIsUpper				is uppercase
 *	PGPCharIsSurrogateCharacter is surrogate
 *	PGPCharIsLeadCharacter		is lead character of surrogate pair
 *	PGPCharIsTrailCharacter		is trail character of surrogate pair
 *
 *	PGPCharToLower				get lowercase representation of character
 *	PGPCharToUpper				get uppercase representation of character
 *
 *	PGPStrCat(dst,src)			concatenate strings
 *	PGPStrChr(str,chr)			find character in string
 *	PGPStrCmp(a,b)				compare strings
 *	PGPStrCpy(dst,src)			copy string
 *	PGPStrCSpn(str,set)			find first character in string from set
 *	PGPStrDec(frst,curr)		get pointer to previous character in string
 *	PGPStrICmp(a,b)				compare strings, ignoring case
 *	PGPStrInc(curr)				get pointer to next character in string
 *	PGPStrLen(str)				number of chracters in the string; this function 
 *								may return smaller value than PGPStrPGPChars
 *	PGPStrLwr(str)				convert string to lower case
 *	PGPStrMCat(dst,src,m)		concatenate string (bounded by m PGPChars)
 *	PGPStrMCmp(a,b,m)			compare strings (bounded by m PGPChars)
 *	PGPStrMCpy(dst,src,m)		copy strings (bounded by m PGPChars)
 *	PGPStrMICmp(a,b,m)			compare strings ignore case (bounded by m PGPChars)
 *	PGPStrMSet(str,val,n)		set string characters to value (bounded by m PGPChars)
 *	PGPStrNCat(dst,src,n)		concatenate string (bounded by n characters)
 *	PGPStrNCmp(a,b,n)			compare strings (bounded by n characters)
 *	PGPStrNCpy(dst,src,n)		copy strings (bounded by n characters)
 *	PGPStrNICmp(a,b,n)			compare strings ignore case (bounded by n characters)
 *	PGPStrNInc(str,n)			get pointer to n characters forward in string
 *	PGPStrPbrk(str,set)			find set of characters in string
 *	PGPStrPGPChars(str)			get number of PGPChars in string
 *	PGPStrRChr(str,chr)			find last occurance of character in string
 *	PGPStrSet(str,val)			set all characters in string to a value
 *	PGPStrSize(str)				get minimum size of string buffer in bytes
 *	PGPStrSpn(str,set)			find first character in string not from set
 *	PGPStrStr(str,set)			find substring in string
 *	PGPStrToD(str,end)			parse string to get double float
 *	PGPStrTok(str,delim)		find next token in string
 *	PGPStrToL(str,end,base)		parse string to get long int
 *	PGPStrToUL(str,end,base)	parse string to get unsigned long int
 *	PGPStrUpr(str)				convert string to upper case
 *
 *	PGPSPrintF					formatted string to buffer
 *	PGPSScanF					get data from formatted string
 *	PGPFPrintF					print formatted string to FILE
 */	
 
#if PGP_UNICODE
	
	#if PGP_WIN32 && !PGP_DRIVER98 && !PGP_DRIVERNT
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
		/* Win32 Unicode functions */
		
		/* use Win32 API equivalents when available */
		#define PGPStrCat(dst,src)			lstrcatW(dst,src)
		#define PGPStrCmp(a,b)				lstrcmpW(a,b)
		#define PGPStrCpy(dst,src)			lstrcpyW(dst,src)
		#define PGPStrMCpy(dst,src,n)		lstrcpynW(dst,src,n)
		#define PGPStrDec(frst,curr)		CharPrevW(frst,curr)
		#define PGPStrICmp(a,b)				lstrcmpiW(a,b)
		#define PGPStrInc(curr)				CharNextW(curr)
		#define PGPStrLwr(str)				CharLowerW(str)
		#define PGPStrUpr(str)				CharUpperW(str)

		/* these are ANSI functions in the C stdlib */
		#define PGPStrChr(str,chr)			wcschr(str,chr)
		#define PGPStrCSpn(str,set)			wcscspn(str,set)
		#define PGPStrMCat(dst,src,n)		wcsncat(dst,src,n)
		#define PGPStrMCmp(a,b,n)			wcsncmp(a,b,n)
		#define PGPStrPbrk(str,set)			wcspbrk(str,set)
		#define PGPStrRChr(str,chr)			wcsrchr(str,chr)
		#define PGPStrSpn(str,set)			wcsspn(str,set)
		#define PGPStrStr(str,set)			wcsstr(str,set)
		#define PGPStrToD(str,end)			wcstod(str,end)
		#define PGPStrTok(str,delim)		wcstok(str,delim)
		#define PGPStrToL(str,end,base)		wcstol(str,end,base)
		#define PGPStrToUL(str,end,base)	wcstoul(str,end,base)

		#define PGPSPrintF					swprintf
		#define PGPSNPrintF					_snwprintf
		#define PGPSScanF					swscanf
		
		#define PGPFPrintF					fwprintf
	
		/* these are *not* ANSI (ISO 9899:1999) standard functions,
		   but are implemented in the Win32 C runtime library */ 
		#define PGPStrMICmp(a,b,n)			_wcsnicmp(a,b,n)
		#define PGPStrNInc(str,n)			_wcsninc(str,n)
		#define PGPStrMSet(str,val,n)		_wcsnset(str,val,n)
		#define PGPStrSet(str,val)			_wcsset(str,val)

		#define PGPStrPGPChars(str)			wcslen(str)
		#define PGPStrSize(str)				(wcslen(str)*2+2)

	#else 
		/* non-Win32 Unicode functions */
	
		/* these are ANSI functions */
		#define PGPStrCat(dst,src)			wcscat(dst,src)
		#define PGPStrChr(str,chr)			wcschr(str,chr)
		#define PGPStrCmp(a,b)				wcscmp(a,b)
		#define PGPStrCpy(dst,src)			wcscpy(dst,src)
		#define PGPStrCSpn(str,set)			wcscspn(str,set)
		#define PGPStrMCat(dst,src,n)		wcsncat(dst,src,n)
		#define PGPStrMCmp(a,b,n)			wcsncmp(a,b,n)
		#define PGPStrMCpy(dst,src,n)		wcsncpy(dst,src,n)
		#define PGPStrMSet(str,val,n)		wmemset(str,val,n)
		#define PGPStrPbrk(str,set)			wcspbrk(str,set)
		#define PGPStrRChr(str,chr)			wcsrchr(str,chr)
		#define PGPStrSpn(str,set)			wcsspn(str,set)
		#define PGPStrStr(str,set)			wcsstr(str,set)
		#define PGPStrToD(str,end)			wcstod(str,end)
		#define PGPStrTok(str,delim)		wcstok(str,delim)
		#define PGPStrToL(str,end,base)		wcstol(str,end,base)
		#define PGPStrToUL(str,end,base)	wcstoul(str,end,base)

		#define PGPSPrintF					swprintf
		#define PGPSNPrintF					snwprintf
		#define PGPSScanF					swscanf
		
		#define PGPFPrintF					fwprintf
	
		/* these are *not* ANSI (ISO 9899:1999) standard functions */ 
		#error the following string functions need to be implemented
		
		#define PGPStrICmp(a,b)			pgpCompareStringsIgnoreCase(a,b)
		#define PGPStrMICmp(a,b,n)		pgpCompareStringsIgnoreCaseM(a,b,n)
		#define PGPStrDec(frst,curr)	pgpDecrementStringPointer(frst,curr)
		#define PGPStrInc(curr)			pgpIncrementStringPointer(curr)
		#define PGPStrLwr(str)			pgpConvertStringToLowerCase(str)
		#define PGPStrNInc(curr,n)		pgpIncrementStringPointerN(curr,n)
		#define PGPStrSet(str,val)		pgpSetStringToValue(str,val)
		#define PGPStrUpr(str)			pgpConvertStringToUpperCase(str)

		#define PGPStrPGPChars(str)			pgpStringLengthPGPChars(str)
		#define PGPStrSize(str)				(wcslen(str)*2+2)
		
	#endif

	#define PGPStrLen(str)				pgpStringLengthCharacters(str)		

#else 

	#if PGP_WIN32 && !PGP_DRIVER98 && !PGP_DRIVERNT
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
		/* Win32 non-Unicode functions */

		/* use Win32 API equivalents when available */
		#define PGPStrCat(dst,src)			lstrcatA(dst,src)
		#define PGPStrCmp(a,b)				lstrcmpA(a,b)
		#define PGPStrCpy(dst,src)			lstrcpyA(dst,src)
		#define PGPStrMCpy(dst,src,n)		lstrcpynA(dst,src,n)
		#define PGPStrDec(frst,curr)		CharPrevA(frst,curr)
		#define PGPStrICmp(a,b)				lstrcmpiA(a,b)
		#define PGPStrInc(curr)				CharNextA(curr)
		#define PGPStrLwr(str)				CharLowerA(str)
		#define PGPStrUpr(str)				CharUpperA(str)

		/* these are ANSI functions in the C stdlib */
		#define PGPStrChr(str,chr)			strchr(str,chr)
		#define PGPStrCSpn(str,set)			strcspn(str,set)
		#define PGPStrMCat(dst,src,n)		strncat(dst,src,n)
		#define PGPStrMCmp(a,b,n)			strncmp(a,b,n)
		#define PGPStrPbrk(str,set)			strpbrk(str,set)
		#define PGPStrRChr(str,chr)			strrchr(str,chr)
		#define PGPStrSpn(str,set)			strspn(str,set)
		#define PGPStrStr(str,set)			strstr(str,set)
		#define PGPStrToD(str,end)			strtod(str,end)
		#define PGPStrTok(str,delim)		strtok(str,delim)
		#define PGPStrToL(str,end,base)		strtol(str,end,base)
		#define PGPStrToUL(str,end,base)	strtoul(str,end,base)

		#define PGPSPrintF					sprintf
		#define PGPSNPrintF					_snprintf
		#define PGPSScanF					sscanf
		
		#define PGPFPrintF					fprintf

		/* these are *not* ANSI (ISO 9899:1999) standard functions,
		   but are implemented in the Win32 C runtime library */ 
		#define PGPStrMICmp(a,b,n)			strnicmp(a,b,n)
		#define PGPStrNInc(str,n)			_strninc(str,n)
		#define PGPStrMSet(str,val,n)		strnset(str,val,n)
		#define PGPStrSet(str,val)			strset(str,val)

	#else 
		/* non-Win32 non-Unicode functions */
	
		/* these are ANSI functions in the C stdlib */
		#define PGPStrCat(dst,src)			strcat(dst,src)
		#define PGPStrChr(str,chr)			strchr(str,chr)
		#define PGPStrCmp(a,b)				strcmp(a,b)
		#define PGPStrCpy(dst,src)			strcpy(dst,src)
		#define PGPStrCSpn(str,set)			strcspn(str,set)
		#define PGPStrMCat(dst,src,n)		strncat(dst,src,n)
		#define PGPStrMCmp(a,b,n)			strncmp(a,b,n)
		#define PGPStrMCpy(dst,src,n)		strncpy(dst,src,n)
		#define PGPStrMSet(str,val,n)		memset(str,val,n)
		#define PGPStrPbrk(str,set)			strpbrk(str,set)
		#define PGPStrRChr(str,chr)			strrchr(str,chr)
		#define PGPStrSpn(str,set)			strspn(str,set)
		#define PGPStrStr(str,set)			strstr(str,set)
		#define PGPStrToD(str,end)			strtod(str,end)
		#define PGPStrTok(str,delim)		strtok(str,delim)
		#define PGPStrToL(str,end,base)		strtol(str,end,base)
		#define PGPStrToUL(str,end,base)	strtoul(str,end,base)

		#define PGPSPrintF					sprintf
#if PGP_SYMBIAN	/*	snprintf is a part of C99 and isn't included by Symbian	*/
		int PGPSNPrintF(char * str, size_t size, const char * format, ...);
		int PGPVSNPrintF(char * str, size_t size, const char * format, va_list ap);
#else
		#define PGPSNPrintF					snprintf
#endif
		#define PGPSScanF					sscanf
		
		#define PGPFPrintF					fprintf

		/* these are *not* ANSI (ISO 9899:1999) standard functions 
		   but are implemented in pgpStrings.c */ 
		#define PGPStrICmp(a,b)			pgpCompareStringsIgnoreCase(a,b)
		#define PGPStrMICmp(a,b,n)		pgpCompareStringsIgnoreCaseM(a,b,n)
		#define PGPStrDec(frst,curr)	pgpDecrementStringPointer(frst,curr)
		#define PGPStrInc(curr)			pgpIncrementStringPointer(curr)
		#define PGPStrLwr(str)			pgpConvertStringToLowerCase(str)
		#define PGPStrNInc(curr,n)		pgpIncrementStringPointerN(curr,n)
		#define PGPStrSet(str,val)		pgpSetStringToValue(str,val)
		#define PGPStrUpr(str)			pgpConvertStringToUpperCase(str)

	#endif

		#define PGPStrSize(str)				(strlen(str)+1)
		#define PGPStrLen(str)				strlen(str)		
		#define PGPStrPGPChars(str)			strlen(str)

#endif 

#define PGPStrIsEmpty(str)			(PGPStrInc((str)) == (str))
	
#define PGPStrNCat(dst,src,n)		pgpConcatenateStringCharacters(src,dst,n)
#define PGPStrNCmp(a,b,n)			pgpCompareStringCharacters(a,b,n)
#define PGPStrNCpy(dst,src,n)		pgpCopyStringCharacters(src,dst,n)
#define PGPStrNICmp(a,b,n)			pgpCompareStringsIgnoreCaseN(a,b,n)


/* Subset of routines above for PFL. They are currently defined as 8 bit routines */
#define PFLStrCat					strcat
#define PFLStrCmp					strcmp
#define PFLStrCpy					strcpy
#define PFLStrChr					strchr
#define PFLStrLen(s)				(PGPUInt)strlen(s)

#define PFLStrMCpy					strncpy	
#define PFLStrMCmp					strncmp	
#if PGP_WIN32
#define PFLStrMICmp					strnicmp	
#else
#define PFLStrMICmp					pgpCompareStringsIgnoreCaseN	
#endif
#define PFLStrMCat					strncat	

#define PFLStrPGPChars(str)			(PGPUInt)strlen(str)

#define PFLStrRChr					strrchr
#define PFLStrSize(str)				(PGPUInt)(strlen(str)+1)				

#define PFLSScanF					sscanf
#define PFLSPrintF					sprintf

/* Subset of routines above for SDK UI. 
 * They are currently defined identically to 8 bit routines in PFL */
#define SDKUIStrCmp					PFLStrCmp
#define SDKUIStrCat					PFLStrCat
#define SDKUIStrCpy					PFLStrCpy
#define SDKUIStrLen					strlen
#define SDKUIStrPGPChars			PFLStrPGPChars
#define SDKUIStrSize				PFLStrSize
#define SDKUISScanF					PFLSScanF
#define SDKUISPrintF				PFLSPrintF

/**
 *      Duplicate a PGP string
 *
 *      Parameters:
 *              string - String to duplicate
 *              bSecure - Put the duplicate in secure memory
 *
 *      Return Value:
 *              NULL - Input string was null or memory allocation error
 *				Caller should free the return with PGPFreeData()
 */
PGPChar *
PGPStrDup(
        const PGPChar*                          string,
        PGPBoolean                              bSecure);

/**
 *      Duplicate a PGPChar8 string, NULL terminate it,
 *      and return it as a PGPChar string
 *
 *      Notes:
 *              This function just makes a copy of the string on Unix
 *              (everything is UTF8)
 *
 *      Parameters:
 *              string8 - UTF8 string to duplicate
 *              bSecure - Put the duplicate in secure memory
 *
 *      Return Value:
 *              NULL - Input string was null or memory allocation error or
 *                     no unicode equivalent
 *				Caller should free the return with PGPFreeData()
 */
PGPChar *
PGPStrDupU8(
        const PGPChar8*                         string8,
        PGPBoolean                              bSecure);

/**
 *      Duplicate a PGPChar(16) string, NULL terminate it,
 *      and return it as a PGPChar8 (UTF8) string.
 *
 *      Notes:
 *              This function just makes a copy of the string on Unix
 *              (everything is UTF8)
 *
 *      Parameters:
 *              string16 - "Unicode" string to duplicate
 *              bSecure - Put the duplicate in secure memory
 *
 *      Return Value:
 *              NULL - Input string was null or memory allocation error or
 *                      conversion error
 *				Caller should free the return with PGPFreeData()
 */
PGPChar8 *
PGPStrDupU16(
        const PGPChar*                          string16,
        PGPBoolean                              bSecure);
/**
 *      Return the length of a PGPChar string convertion to PGPChar8
 *      i.e., how much data would you get from PGPStrDupU16()?
 *
 */
PGPUInt32
PGPStrDupU16Len(
		const PGPChar*							string16);

/*____________________________________________________________________________
	Return a pointer to the file name extension in the name.  The pointer
	does NOT include the period.
	
	File name extensions must be 3 PGPChars or less and not be the whole name.
	
	Return NULL if extension not found
____________________________________________________________________________*/
const PFLChar *	PGPGetFileNameExtension( const PFLChar * name );

const PFLChar *	PGPGetEndOfBaseFileName( const PFLChar * name );

/*____________________________________________________________________________
	Form a new file name based on the base name, a number.
	
	Example (with number = 99, separator = "-", maxSize = 13):
	Foo.tmp			=> Foo-99.tmp
	Foo				=> Foo-99
	TooLongName.tmp => TooLo-99.tmp
	
	'maxSize' includes trailing null.  If necessary, first the base name
	will be truncated, then the separator, and then the extension, in
	order to fit in maxSize.
	
	returns an error if new name won't fit in buffer
____________________________________________________________________________*/
PGPError	PGPNewNumberFileName( const PFLChar * baseName,
				const PFLChar * separator, PGPUInt32 number,
				PGPSize maxSize, PFLChar *outName );

/*____________________________________________________________________________
	Checks to see if PGPNewNumberFileName() could have generated <fileName>
	from <origName> for some number, and returns the number.

	maxSize must be <= the maxSize which was passed to PGPNewNumberFileName()
	and is used to make sure the origName wasn't truncated more than
	necessary.  Passing 0 is fine, but then any filename of the form
	<number>.xxx (with matching extension) will succeed.

	WARNING: This may not work properly if the separator contains a digit,
			 or if the separator is empty and the basename contains a digit.
____________________________________________________________________________*/
PGPError	PGPVerifyNumberFileName( const PFLChar * origName,
				const PFLChar * separator, const PFLChar * fileName,
				PGPSize maxSize, PGPBoolean * outValid,
				PGPUInt32 * outNumber );

/*____________________________________________________________________________
	Case insensitive string compare
____________________________________________________________________________*/
int		pgpCompareStringsIgnoreCase( const PGPChar *str1, const PGPChar *str2 );
int		pgpCompareStringsIgnoreCaseM( const PGPChar *str1, const PGPChar *str2,
			int length);
int		pgpCompareStringsIgnoreCaseN( const PGPChar *str1, const PGPChar *str2,
			int length);
	
PGPChar *	FindSubStringNoCase( const PGPChar * mainString,
					const PGPChar * subString);

/*____________________________________________________________________________
	Miscellaneous string functions.  These should not be called directly,
	but by using the above-defined macros.
____________________________________________________________________________*/
#if !PGP_WIN32
PGPChar* pgpDecrementStringPointer( const PGPChar* frst, const PGPChar* curr );
PGPChar* pgpIncrementStringPointer( const PGPChar* curr );
PGPChar* pgpConvertStringToLowerCase( PGPChar* str );
PGPChar* pgpIncrementStringPointerN( const PGPChar* curr, PGPSize n );
PGPChar* pgpSetStringToValue( PGPChar* str, PGPChar value );
PGPChar* pgpConvertStringToUpperCase( PGPChar* str );
#endif

/* Calculate the length of the string in characters, not including the null
   termination. UTF-16 character that occupies two 2-byte PGPChars
   will be counted as 1. Use in cases when the difference between 
   UCS-2 and UTF-16 is important. */
int		pgpStringLengthCharacters( const PGPChar* str );

/* Calculate the length of the string in PGPChars, not including the null
   termination. UTF-16 character that occupies two 2-byte PGPChars
   will be counted as 2. */
int		pgpStringLengthPGPChars( const PGPChar* str );

/* Calculate the minimum buffer size (in bytes) necessary to hold the string,
   including the null termination (so it returns 1 char more then strlen) */
int		pgpStringBufferSize( const PGPChar* str );

/* Compare two strings bounded by the specified number of characters */
int		pgpCompareStringCharacters( const PGPChar *str1, const PGPChar *str2,
			int numchars);

/* Concatenate one string to another, bounded by the specified number of
   characters */
PGPChar*	pgpConcatenateStringCharacters( const PGPChar *src, PGPChar *dst,
			int numchars);

/* Copy one string to another, bounded by the specified number of characters */
PGPChar*	pgpCopyStringCharacters( const PGPChar *src, PGPChar *dst, 
			int numchars);

/*____________________________________________________________________________
	Functions for classifying PGPChars
____________________________________________________________________________*/
PGPBoolean	PGPCharIsSurrogateCharacter( PGPChar c );
PGPBoolean	PGPCharIsLeadCharacter( PGPChar c );
PGPBoolean	PGPCharIsTrailCharacter( PGPChar c );
PGPBoolean	PGPCharIsAlpha( PGPChar c );
PGPBoolean	PGPCharIsASCII( PGPChar c );
PGPBoolean	PGPCharIsBlank( PGPChar c );
PGPBoolean	PGPCharIsDigit( PGPChar c );
PGPBoolean	PGPCharIsLower( PGPChar c );
PGPBoolean	PGPCharIsPunct( PGPChar c );
PGPBoolean	PGPCharIsPrint( PGPChar c );
PGPBoolean	PGPCharIsSpace( PGPChar c );
PGPBoolean	PGPCharIsUpper( PGPChar c );

/*____________________________________________________________________________
	Functions for converting PGPChars
____________________________________________________________________________*/
PGPChar		PGPCharToLower( PGPChar c );
PGPChar		PGPCharToUpper( PGPChar c );

/*____________________________________________________________________________
	Functions for PGP 8 Bit strings  kept as inline to provide type checking..
____________________________________________________________________________*/
#if PGP_DEBUG	/* [ */

static INLINE size_t PGPStrLen8(unsigned const char* str)
	{ return (strlen( (char*)str)); }

static INLINE unsigned char* PGPStrCpy8(unsigned char* dst, const unsigned char * src)
	{ return ((unsigned char*) strcpy( (char*)dst,  (char*)src)); }

static INLINE unsigned char* PGPStrNCpy8(unsigned char* dst, const unsigned char * src, size_t len)
	{ return ((unsigned char*) strncpy( (char*)dst,  (char*)src, len)); }

static INLINE int PGPStrCmp8(const unsigned char *s1, const unsigned char *s2)
	{ return (strcmp((char*) s1,(char*) s2)); }

static INLINE int PGPStrNCmp8(const unsigned char *s1, const unsigned char *s2, size_t len)
	{ return (strncmp((char*) s1,(char*) s2, len)); }

static INLINE unsigned char* PGPStrCat8(unsigned char* s, const unsigned char * append )
	{ return ((unsigned char*) strcat( (char*)s,  (char*)append)); }

static INLINE unsigned char* PGPStrNCat8(unsigned char* s, const unsigned char * append, size_t count )
	{ return ((unsigned char*) strncat( (char*)s,  (char*)append, count)); }

static INLINE unsigned char* PGPStrStr8(unsigned char* big, const unsigned char * little )
	{ return ((unsigned char*) strstr( (char*)big,  (char*)little)); }

#else /* ] !PGP_DEBUG [ */

#define PGPStrLen8(str)   				strlen( (char*)(str))
#define PGPStrCpy8(dst, src) 			((unsigned char*) strcpy( (char*)(dst),  (char*)(src)))
#define PGPStrNCpy8(dst, src, len)		((unsigned char*) strncpy( (char*)(dst),  (char*)(src), (len)))
#define PGPStrCmp8(s1, s2)				(strcmp((char*) (s1),(char*) (s2)))
#define PGPStrNCmp8(s1, s2, len)		(strncmp((char*) (s1),(char*) (s2), (len)))
#define PGPStrCat8(s1, s2)				((unsigned char*) strcat( (char*)(s1),  (char*)(s2)))
#define PGPStrNCat8(s1, s2, count)		((unsigned char*) strncat( (char*)(s1),  (char*)(s2), (count)))
#define PGPStrStr8(s1, s2)				strstr((char*) (s1),(char*) (s2))

#endif	/* ] PGP_DEBUG */

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpStrings_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
