/***********************************************************************/
/*                                                                     */
/* ASFunctionLogger.h                                                  */
/*                                                                     */
/* Copyright 1999 Adobe Systems Incorporated.                          */
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
/* Started by Eric Scouten, 08/02/1999                                 */
/*                                                                     */
/***********************************************************************/

#ifndef __ASFunctionLogger__
#define __ASFunctionLogger__

	// ASAPI
#include "ASTypes.h"


#ifdef WIN_ENV
#pragma PRAGMA_ALIGN_BEGIN
#pragma PRAGMA_IMPORT_BEGIN
#elif defined(HPUX) && defined(__HP_aCC)
HPUX_Pragma (PRAGMA_ALIGN_BEGIN)
#else
_Pragma (PRAGMA_ALIGN_BEGIN)
#endif


#ifdef __cplusplus
extern "C" {
#endif



// =============================================================================
//		* ASFunctionLoggerSuite
// =============================================================================

#define kASFunctionLoggerSuite			"AS Function Logger Suite"
#define kASFunctionLoggerSuiteVersion1	1

// -----------------------------------------------------------------------------

typedef struct
{

	void ASAPI (*EnterFunction)(const char* inSuiteName, const char* inFunctionName);
	void ASAPI (*ExitFunction)(const char* inSuiteName, const char* inFunctionName);
	void ASAPI (*FunctionParameter)(const char* inParamName, const char* inParamValue);

} ASFunctionLoggerSuite1;


// -----------------------------------------------------------------------------

#ifdef WIN_ENV
#pragma PRAGMA_IMPORT_END
#pragma PRAGMA_ALIGN_END
#elif defined(HPUX) && defined(__HP_aCC)
HPUX_Pragma (PRAGMA_ALIGN_END)
#else
_Pragma (PRAGMA_ALIGN_END)
#endif

#ifdef __cplusplus
}
#endif

#endif
