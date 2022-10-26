/*____________________________________________________________________________
	Copyright (C) 2002-2004 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpSDA_h
#define Included_pgpSDA_h


/****************************************************************************
	Includes
****************************************************************************/

#include <stdio.h>
#if PGP_WIN32
#include <stdlib.h>
#else
#include <sys/param.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pgpSDAOpaque.h"

#include "pgpBase.h"
#include "pgpKeys.h"
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpEncode.h"
#include "pgpUtilities.h"
#include "pflPrefTypes.h"
#include "pgpPubTypes.h"


/****************************************************************************
	Defines
****************************************************************************/

#define kInvalidPGPsdaContextRef				((PGPsdaContextRef) NULL)
#define PGPsdaContextRefIsValid(ref)			((ref) != kInvalidPGPsdaContextRef)

/* Auto exec file flags */
#define kPGPsdaAutoExecFileFlags_None			0
#define kPGPsdaAutoExecFileFlags_Prompt			(1 << 0)


/****************************************************************************
	Events (based on events found in pgpEncode.h)
****************************************************************************/

/*
 *	Event Notes:
 *
 *	If there is an error during setup no initial event will be sent
 *		Basically we have to start processing the SDA for these to get sent
 *	Final events will only be sent if there was an initial event
 *	Error events are only sent once, just before the final event
 *		In the case of no initial event no error event is generated, but
 *		the decrypt function will still return an error code
 *	Warning events are not currently used
 *	Analyze events are not currently used
 *	Begin and end lex events are sent during encode for the various
 *		different blocks of data we write out
 *		These events are not sent for decode
 *		The offset field is not currently used
 *	New object events are sent before files are added and before files
 *		are extracted on decode.
 *
 *	Decode Only:
 *
 *	Ask file events get sent if the output exists
 *		If the caller does nothing then the file gets overwritten
 *		A new file may be specified by the caller
 *			If this file exists it will be overwritten
 *	Free file events are sent after ask file events so that any memory
 *		allocated for the new file name can be cleaned up.  This event
 *		are sent even if the caller didn't provide a new filename.
 *	Ask ADK events are sent when the caller needs to decrypt an ADK block
 *		Inside the block is a session key which must be passed back to
 *		the SDA function for decryption to continue.  If the session key
 *		is not valid, bad passphrase is returned.
 *		Note that this event will occur BEFORE the initial event because
 *			processing on the SDA has not begun.
 *		Note that after the session key is used it is not guaranteed to
 *			contain the same data it did before, if the caller wants to
 *			retain this information they must save it separately (it
 *			cannot be static)
 *	Free ADK events are sent after ask ADK events so that memory for the
 *		session key allocated by the caller can be cleaned up.  This event
 *		is sent even if the caller didn't provide a session key
 *		Note that this event will occur BEFORE the initial event because
 *			processing on the SDA has not begun.
 */
enum PGPsdaEventType_
{
	kPGPsdaEvent_NullEvent				= 0,		/* Nothing happened */
	kPGPsdaEvent_InitialEvent			= 1,		/* Start of operation */
	kPGPsdaEvent_FinalEvent				= 2,		/* End of operation (even on error) */
	kPGPsdaEvent_AnalyzeEvent			= 3,		/* Data analysis */
	kPGPsdaEvent_BeginLexEvent			= 4,		/* Start of block */
	kPGPsdaEvent_EndLexEvent			= 5,		/* End of block */
	kPGPsdaEvent_ErrorEvent				= 6,		/* Error */
	kPGPsdaEvent_WarningEvent			= 7,		/* Warning */

	kPGPsdaEvent_NewObjectEvent			= 10,		/* New object (encode or decode) */

	kPGPsdaEvent_AskFileEvent			= 20,		/* Filename conflict on decode */
	kPGPsdaEvent_FreeFileEvent			= 21,		/* Filename conflict on decode */
	kPGPsdaEvent_AskADKEvent			= 22,		/* Decrypt ADK block on decode */
	kPGPsdaEvent_FreeADKEvent			= 23,		/* Decrypt ADK block on decode */

	PGP_ENUM_FORCE (PGPsdaEventType_)

};
PGPENUM_TYPEDEF (PGPsdaEventType_, PGPsdaEventType);


/* PGP sda lex section codes */
enum PGPsdaLexType_
{
	kPGPsdaLex_Stub						= 0,
	kPGPsdaLex_Header					= 1,
	kPGPsdaLex_Data						= 2,
	kPGPsdaLex_ADKFooter				= 3,
	kPGPsdaLex_Footer					= 4,

	PGP_ENUM_FORCE (PGPsdaLexType_)
};
PGPENUM_TYPEDEF (PGPsdaLexType_, PGPsdaLexType);


/* PGP sda object type codes */
enum PGPsdaObjectType_
{
	kPGPsdaObject_Unknown				= 0,
	kPGPsdaObject_File					= 1,
	kPGPsdaObject_Directory				= 2,
	kPGPsdaObject_SymLink				= 3,

	kPGPsdaObject_SymLinkTarget			= 10,

	PGP_ENUM_FORCE (PGPsdaObjectType_)
};
PGPENUM_TYPEDEF (PGPsdaObjectType_, PGPsdaObjectType);


/*
 *	Individual event information structs, combined as a union in PGPsdaEvent
 */
typedef struct PGPsdaEventNullData_
{
	PGPFileOffset			bytesWritten;
	PGPFileOffset			bytesTotal;

} PGPsdaEventNullData;


typedef struct PGPsdaEventAnalyzeData_
{
//	PGPsdaAnalyzeType		sectionType;
	PGPUInt32				sectionType;

} PGPsdaEventAnalyzeData;


typedef struct PGPsdaEventBeginLexData_
{
	PGPUInt32				sectionNumber;
//	PGPSize					sectionOffset;
	PGPUInt64				sectionOffset;

} PGPsdaEventBeginLexData;


typedef struct PGPsdaEventEndLexData_
{
	PGPUInt32				sectionNumber;

} PGPsdaEventEndLexData;


typedef struct PGPsdaEventErrorData_
{
	PGPError				error;
	void *					errorArg;

} PGPsdaEventErrorData;


typedef struct PGPsdaEventWarningData_
{
	PGPError				warning;
	void *					warningArg;

} PGPsdaEventWarningData;


typedef struct PGPsdaEventNewObjectData_
{
	PGPChar *				name;		/* Fully processed name */
	PGPsdaObjectType		type;

	/* Stored name (currently only used for decode) */
	PGPChar *				internalName;

} PGPsdaEventNewObjectData;


typedef struct PGPsdaEventAskFileData_
{
	PGPChar *				oldName;
	PGPChar *				newName;

} PGPsdaEventAskFileData;


typedef struct PGPsdaEventFreeFileData_
{
	PGPChar *				newName;

} PGPsdaEventFreeFileData;


typedef struct PGPsdaEventAskADKData_
{
	/* Outgoing */
	PGPByte *				adkBlock;
	PGPSize					adkSize;

	/* Incoming */
	PGPByte *				sessionKey;
	PGPSize					sessionKeySize;

} PGPsdaEventAskADKData;


typedef struct PGPsdaEventFreeADKData_
{
	PGPByte *				sessionKey;

} PGPsdaEventFreeADKData;


/*
 *	The following events have no event-specific data defined for them:
 *		kPGPsdaEvent_InitialEvent
 *		kPGPsdaEvent_FinalEvent
 */


/* Union of all event data structures above */
typedef union PGPsdaEventData_
{
	PGPsdaEventNullData				nullData;
	PGPsdaEventAnalyzeData			analyzeData;
	PGPsdaEventBeginLexData			beginLexData;
	PGPsdaEventEndLexData			endLexData;
	PGPsdaEventErrorData			errorData;
	PGPsdaEventWarningData			warningData;
	PGPsdaEventNewObjectData		newObjectData;
	PGPsdaEventAskFileData			askFileData;
	PGPsdaEventFreeFileData			freeFileData;
	PGPsdaEventAskADKData			askADKData;
	PGPsdaEventFreeADKData			freeADKData;

} PGPsdaEventData;


/* SDAEvent structure */
struct PGPsdaEvent
{
	PGPsdaEventType					type;			/* Type of event */
	PGPsdaEventData					data;			/* Event specific data */
};


/****************************************************************************
	Call back functions
****************************************************************************/

typedef PGPError (*PGPsdaEventHandlerProcPtr)(PGPContextRef				context,
											  struct PGPsdaEvent *		event,
											  PGPUserValue				userValue);


/****************************************************************************
	Enumerations
****************************************************************************/

enum PGPsdaTargetPlatform_
{
	kPGPsdaTargetPlatform_None				= 0,
	kPGPsdaTargetPlatform_Win32				= 1,
	kPGPsdaTargetPlatform_Linux				= 2,
	kPGPsdaTargetPlatform_Solaris			= 3,
	kPGPsdaTargetPlatform_AIX				= 4,
	kPGPsdaTargetPlatform_HPUX				= 5,
	kPGPsdaTargetPlatform_MacOSX			= 6,

	PGP_ENUM_FORCE (PGPsdaTargetPlatform_)
};
PGPENUM_TYPEDEF (PGPsdaTargetPlatform_, PGPsdaTargetPlatform);

/* Compression optimization */
enum PGPsdaCompressionLevel_
{
	kPGPsdaCompressionLevel_None			= 0,	/* no compression */
	kPGPsdaCompressionLevel_Speed			= 1,	/* fastest / least compression */
	kPGPsdaCompressionLevel_Low				= 3,
	kPGPsdaCompressionLevel_Medium			= 5,	/* PGP SDK default (see pgpEnv.c) */
	kPGPsdaCompressionLevel_Compromise		= 6,	/* compromise between speed and size */
	kPGPsdaCompressionLevel_High			= 7,
	kPGPsdaCompressionLevel_Size			= 9,	/* slowest / most compression */

	PGP_ENUM_FORCE (PGPsdaCompressionLevel_)
};
PGPENUM_TYPEDEF (PGPsdaCompressionLevel_, PGPsdaCompressionLevel);


/****************************************************************************
	Public Functions
****************************************************************************/

/*
 *	Warning: This API is experimental and may change from release to release.
 */

PGP_BEGIN_C_DECLARATIONS

/*
 *	Create a new PGP SDA context
 *
 *	Parameters:
 *		context - PGP context
 *		pSDAContext - Pointer to a location to store the SDA context
 *			This context can only be used for a single call to PGPsdaCreate()
 *			This context must be freed by the caller with PGPFreeSDAContext()
 */
PGPError
PGPNewSDAContext(
	PGPContextRef				context,
	PGPsdaContextRef *			pSDAContext);


/*
 *	Free a PGP SDA context
 */
PGPError
PGPFreeSDAContext(
	PGPsdaContextRef			sdaContext);


/*
 *	Import objects into an SDA.
 *
 *	These objects get cached internally and are only read when PGPsdaCreate() is called.
 *	Note that they get accessed at cache time so we know things like file size, etc.
 *
 *	Parameters:
 *		sdaContext - PGP SDA context
 *		object - Object to add
 *			This must not contain wildcard characters
 *			Unicode on Win32, UTF8 otherwise
 *		rootPath - Root path to strip from the input object (optional)
 *			This path must match the input object if it is provided
 *			If object is a directory, rootPath applies to all children as well
 *			Unicode on Win32, UTF8 otherwise
 *		bRecursive - Add all objects in a directory or just the name itself
 *
 *	Return Value:
 *		PGPError
 *			kPGPError_BadParams
 *			kPGPError_NoUnicodeEquivalent
 *			kPGPError_OutOfMemory
 *			kPGPError_FileNotFound
 *			kPGPError_FilePermissions
 *			kPGPError_FileOpFailed - Generic file access error
 *			kPGPError_IllegalFileOp - Unsupported file type
 *			kPGPError_FeatureNotAvailable - Bad auto launch file type
 *			kPGPError_ImproperInitialization - Bad filename
 */
PGPError
PGPsdaImportObject(
	PGPsdaContextRef			sdaContext,
	PGPChar *					object,
	PGPChar *					rootPath,
	PGPBoolean					bRecursive);


/*
 *	Set the auto launch flag on an imported object
 *
 *	Parameters:
 *		sdaContext - PGP SDA context
 *		object - the name of the object to auto launch on decrypt
 *			This must match exactly an existing imported object name
 *			Unicode on win32, UTF8 everywhere else
 *		flags - settings for the auto launch file
 *
 *	Return Value:
 *		kPGPError_ItemNotFound - no match
 *		kPGPError_ItemAlreadyExists - multiple matches
 *		kPGPError_FeatureNotAvailable - only one auto launch allowed
 *		kPGPError_IllegalFileOp - non-file object matched
 */
PGPError
PGPsdaSetAutoLaunchObject(
	PGPsdaContextRef			sdaContext,
	PGPChar *					object,
	PGPUInt8					flags);


/*
 *	Function to create an SDA from user options
 *
 *	Callbacks:
 *		Initial / Final - Always sent unless there is an error during the setup phase
 *			Bad parameters, feature not available, etc.
 *		Begin / End Lex - Wraps each section of the SDA as it is created
 *			Stub, header, data, footers, etc.  See PGPsdaLexType.
 *			Section offset is not currently used
 *		New Object - Sent before we start processing each new object
 *			Event will contain the name and type of object.
 *		Null - Sent periodically for progress updates.  These events are only sent for
 *			the user data portion of the SDA.  The bytes total is not the actual number
 *			of bytes to be read from the file system.  It will be close but there are
 *			other numbers added to support progress increases for empty files and non
 *			file objects.  There will always be a 0% and 100% event unless an error
 *			occurs during processing.
 *		Analyze - Not currently used
 *		Error - Sent if an error occurs creating the SDA
 *			If there is an object involved in the error the name is sent in the
 *			errorArg field of the event struct (as PGPChar *)
 *		Warning - Not currently used
 *
 *	Parameters:
 *		sdaContext - PGP SDA context
 *		cipherAlgorithm - cipher algorithm to use
 *			Currently only 3DES and AES-{128,192.256} are supported
 *		hashAlgorithm - hash algorithm to use
 *			Currently only SHA is supported
 *		compressionAlgorithm - compression algorithm to use
 *			Currently only ZIP (deflate) is supported
 *		compressionLevel - compression level to use
 *			Use any of the pre-defined enums (or a number between 0 and 9 inclusive)
 *		targetPlatform - target platform that the SDA will run on
 *		bStripDirectories - remove directory information
 *		adkKeySet - additional keys that the SDA should be encrypted to
 *			optional
 *		passphrase - passphrase for the SDA
 *			Unicode on win32, UTF8 everywhere else
 *		outputFilename - where to write the SDA data
 *			Unicode on win32, UTF8 everywhere else
 *		handler - user event handler
 *			required
 *		userValue - user value passed during event callbacks
 *			optional
 *
 *	Return Value:
 *		PGPError + status callbacks (if we get that far)
 *		kPGPError_BadParams - most cases of invalid input
 *		kPGPError_FeatureNotAvailable - valid but unsupported input
 *		kPGPError_InputFile - no input files found
 *			This can happen when using root paths that happen to strip off everything
 */
PGPError
PGPsdaCreate(
	PGPsdaContextRef			sdaContext,
	PGPCipherAlgorithm			cipherAlgorithm,
	PGPHashAlgorithm			hashAlgorithm,
	PGPCompressionAlgorithm		compressionAlgorithm,
	PGPsdaCompressionLevel		compressionLevel,
	PGPsdaTargetPlatform		targetPlatform,
	PGPBoolean					bStripDirectories,
	PGPKeySetRef				adkKeySet,
	PGPChar *					passphrase,
	PGPChar *					outputFilename,
	PGPsdaEventHandlerProcPtr	handler,
	PGPUserValue				userValue);


PGP_END_C_DECLARATIONS

#endif /* Included_pgpSDA_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
