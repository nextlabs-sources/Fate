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
 *        Name:	SimpleUnicode.h
 *     Purpose:	SweetPea interfaces for SimpleUnicode suites.
 *      Author:	Jon Reid
 *	   Created: December 7, 1998
 */

#ifndef __SimpleUnicode__
#define __SimpleUnicode__


#ifndef __UnicodeAPI__
#include "UnicodeAPI.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*
	Ordinarily when writing a Unicode conversion suite, you should define
	the encoding(s) it recognizes, but the SimpleUnicode functions ignore
	the encoding argument. The Mac version assumes you want the script
	corresponding to the application font; the Win version assumes you
	want ANSI 1252 (Windows Latin-1).
*/


/*
 * SimpleUnicode Suite
 */

#define kSimpleUnicodeSuite			"SimpleUnicode Suite"
#define kSimpleUnicodeSuiteVersion	1

typedef struct SimpleUnicodeSuite
{
	ToUnicodeSizeProc		ToUnicodeSize;
	ConvertToUnicodeProc	ConvertToUnicode;
	FromUnicodeSizeProc		FromUnicodeSize;
	ConvertFromUnicodeProc	ConvertFromUnicode;
} SimpleUnicodeSuite;


#ifdef __cplusplus
}
#endif

#endif	/* __SimpleUnicode__ */
