/***********************************************************************/
/*                                                                     */
/* ASDebug.h                                                           */
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
/* Started by Eric Scouten, 06/14/1999                                 */
/*                                                                     */
/***********************************************************************/


#ifndef __ASDebug__
#define __ASDebug__

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



// -----------------------------------------------------------------------------

// ASDebugAction enum defines what to do when an exception or assertion happens.

typedef enum {
	kASDebugAction_Nothing				= 0,
 	kASDebugAction_Alert				= 1,
	kASDebugAction_LowLevelDebugger		= 2,
	kASDebugAction_SourceDebugger		= 3,
	kASDebugAction_Log					= 4,
	kASDebugAction_DummyAction			= 0xFFFFFFFF
} ASDebugAction;


// =============================================================================
//		* ASDebugSuite
// =============================================================================

#define kASDebugSuite			"AS Debug Suite"
#define kASDebugSuiteVersion1	1

// -----------------------------------------------------------------------------

typedef struct
{

	// debugging traps
	
	void ASAPI (*RaiseSignal)(const char* inMessage, const char* inPlugin,
				const char* inFile, ASUInt32 inLineNumber);

	void ASAPI (*AboutToThrow)(const char* inMessage, const char* inPlugin,
				const char* inFile, ASUInt32 inLineNumber);

	// debugging log

	void ASAPI (*LogMessage)(const char* inMessage);
	void ASAPI (*IndentLog)();
	void ASAPI (*UnindentLog)();

	// debugging behavior control
	
	ASDebugAction ASAPI (*GetSignalAction)();
	void ASAPI (*SetSignalAction)(ASDebugAction inSignalAction);
	
	ASDebugAction ASAPI (*GetThrowAction)();
	void ASAPI (*SetThrowAction)(ASDebugAction inThrowAction);
	
	// debugging message dialog

	void ASAPI (*ShowDebugAlert)(const char* inHeading, const char* inMessage,
				const char* inPlugin, const char* inFile, ASUInt32 inLineNumber);

} ASDebugSuite1;


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
