#ifndef __ASHelp__
#define __ASHelp__

/*
 *        Name:	ASHelp.h
 *   $Revision$
 *      Author:	Rob Sargent
 *        Date:	2/15/97
 *     Purpose:	AS Help suite
 *
 * Copyright (c) 1986-1997 Adobe Systems Incorporated, All Rights Reserved.
 *
 */


/*
 * Includes
 */

#include "ASTypes.h"
#include "SPFiles.h"
#include "SPPlugs.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN_ENV
#pragma PRAGMA_ALIGN_BEGIN
#pragma PRAGMA_IMPORT_BEGIN
#elif defined(HPUX) && defined(__HP_aCC)
HPUX_Pragma (PRAGMA_ALIGN_BEGIN)
#else
_Pragma (PRAGMA_ALIGN_BEGIN)
#endif



/*******************************************************************************
 **
 ** Constants
 **
 **/
#define kASHelpSuite					"AS Help Suite"
#define kASHelpSuiteVersion1			1
#define kASHelpSuiteVersion				kASHelpSuiteVersion1
#define kASHelpVersion					kASHelpSuiteVersion1
#define kASHelpSuiteVersion2			2

#define kHelpInfoVersion		1L


#define kHelpError				'!Hlp'


#define kHelpMenuType			'Menu'
#define kHelpDialogType			'Dlog'
#define kHelpToolType			'Tool'
#define kHelpPaletteMenuType	'PMnu'
#define kHelpEndType			0L


#define kNoHelpID				0L

#define kHelpFileStrIDProperty	'HlpS'
#define kHelpNativeStrIndex		1


/*******************************************************************************
 **
 ** Types
 **
 **/

// WARNING: The size of these types
//			must match the resources
typedef ASUInt32	ASHelpType;
typedef ASInt32		ASHelpKey;		// can be an integer or a pooled string pointer

typedef ASInt32		ASHelpID;


// WinHelp/QuickHelp Commands
typedef enum
{
	/* #if(WINVER >= 0x0400) */
	kASHelpCmd_CONTEXTMENU  = 0x000a,
	kASHelpCmd_FINDER       = 0x000b,
	kASHelpCmd_WM_HELP      = 0x000c,
	kASHelpCmd_SETPOPUP_POS = 0x000d,

	kASHelpCmd_TCARD        = 0x8000,
	kASHelpCmd_TCARD_DATA   = 0x0010,
	kASHelpCmd_TCARD_OTHER_CALLER = 0x0011,
	/* #endif  // WINVER >= 0x0400 */

	kASHelpCmd_CONTEXT		= 0x0001L,	/* Display topic in ulTopic */
	kASHelpCmd_QUIT			= 0x0002L,	/* Terminate help */
	kASHelpCmd_CONTENTS		= 0x0003L,	/* NOTE: HELP_INDEX = HELP_CONTENTS in Windows.h */
	kASHelpCmd_HELPONHELP	= 0x0004L,	/* Display help on using help */
	kASHelpCmd_SETCONTENTS  = 0x0005L,	/* NOTE: HELP_SETINDEX = HELP_SETCONTENTS */
	kASHelpCmd_CONTEXTPOPUP = 0x0008L,
	kASHelpCmd_FORCEFILE    = 0x0009L,
	kASHelpCmd_KEY          = 0x0101L,  /* Display topic for keyword in offabData */
	kASHelpCmd_COMMAND      = 0x0102L,
	kASHelpCmd_PARTIALKEY   = 0x0105L,
	kASHelpCmd_MULTIKEY     = 0x0201L,
	kASHelpCmd_SETWINPOS    = 0x0203L
} ASHelpCommand;


typedef struct ASHelpInitInfo
{
	ASInt32 version;
	ADMWindowRef appWindow;
	SPPluginRef appPluginRef;
	ASInt16 helpFolderNameStrID;
	ASInt16 appHelpFileNameStrID;
	ASInt16 helpFolderNameStrIndex;
	ASInt16 appHelpFileNameStrIndex;
} ASHelpInitInfo;


typedef struct PIHelpFileDesc
{
	long fVersion;
	long fFileNameStrID;
} PIHelpFileDesc;


 /*******************************************************************************
 **
 ** Suite
 **
 **/

typedef struct t_ASHelpSuite1
{
	ASAPI ASErr (*Init) (ASHelpInitInfo *info);
	ASAPI ASErr (*Quit) (void);

	ASAPI ASErr (*SetContextHelpMode) (ASBoolean helpMode);
	ASAPI ASBoolean (*InContextHelpMode) (void);

	ASAPI ASErr (*Help) (ADMWindowRef asWinRef, SPPlatformFileSpecification *spFile,
						 ASHelpCommand asHelpCommand, ASUInt32 dwData);
		// spFile == NULL uses the App's help file

	ASAPI ASErr (*HelpContents)	(SPPluginRef pluginRef);
	ASAPI ASErr (*SearchHelp)	(SPPluginRef pluginRef);
	ASAPI ASErr (*HowToUseHelp)	(SPPluginRef pluginRef);
	ASAPI ASErr (*ContextHelp)	(SPPluginRef pluginRef, ASHelpID helpID);
	
	ASAPI ASErr (*GetHelpID)	(SPPluginRef pluginRef, ASHelpType helpType,
								 ASHelpKey key1, ASHelpKey key2, ASHelpID *helpID);

	ASAPI ASErr (*TypedHelp)	(SPPluginRef pluginRef, ASHelpType helpType, ASHelpKey key1, ASHelpKey key2);

	ASAPI ASErr (*MenuHelp)		(SPPluginRef pluginRef, ASHelpKey commandID);

	ASAPI ASErr (*DialogHelp)	(SPPluginRef pluginRef, ASHelpKey dialogID, ASHelpKey itemID);
		// itemID == 0 means whole dialog

	ASAPI ASErr (*PaletteMenuHelp)	(SPPluginRef pluginRef, ASHelpKey dialogID, ASHelpKey menuItem );

	ASAPI ASErr (*ToolHelp)		(SPPluginRef pluginRef, ASHelpKey toolName );
		// toolName should be a pooled string pointer cast to ASHelpKey

} ASHelpSuite1;

typedef ASHelpSuite1 ASHelpSuite;  //For historical reasons.

typedef struct t_ASHelpSuite2
{
	ASAPI ASErr (*Init) (ASHelpInitInfo *info);
	ASAPI ASErr (*Quit) (void);

	ASAPI ASErr (*SetContextHelpMode) (ASBoolean helpMode);
	ASAPI ASBoolean (*InContextHelpMode) (void);

	ASAPI ASErr (*Help) (ADMWindowRef asWinRef, SPPlatformFileSpecification *spFile,
						 ASHelpCommand asHelpCommand, ASUInt32 dwData);
		// spFile == NULL uses the App's help file

	ASAPI ASErr (*HelpContents)	(SPPluginRef pluginRef);
	ASAPI ASErr (*SearchHelp)	(SPPluginRef pluginRef);
	ASAPI ASErr (*HowToUseHelp)	(SPPluginRef pluginRef);
	ASAPI ASErr (*ContextHelp)	(SPPluginRef pluginRef, ASHelpID helpID);
	
	ASAPI ASErr (*GetHelpID)	(SPPluginRef pluginRef, ASHelpType helpType,
								 ASHelpKey key1, ASHelpKey key2, ASHelpID *helpID);

	ASAPI ASErr (*TypedHelp)	(SPPluginRef pluginRef, ASHelpType helpType, ASHelpKey key1, ASHelpKey key2);

	ASAPI ASErr (*MenuHelp)		(SPPluginRef pluginRef, ASHelpKey commandID);

	ASAPI ASErr (*DialogHelp)	(SPPluginRef pluginRef, ASHelpKey dialogID, ASHelpKey itemID);
		// itemID == 0 means whole dialog

	ASAPI ASErr (*PaletteMenuHelp)	(SPPluginRef pluginRef, ASHelpKey dialogID, ASHelpKey menuItem );

	ASAPI ASErr (*ToolHelp)		(SPPluginRef pluginRef, ASHelpKey toolName );
		// toolName should be a pooled string pointer cast to ASHelpKey

	ASAPI ASErr (*ContextHelpQRC) (SPPluginRef pluginRef, ASHelpID helpID);

} ASHelpSuite2;

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
