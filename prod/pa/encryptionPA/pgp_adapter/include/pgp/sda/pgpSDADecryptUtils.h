/*____________________________________________________________________________
	Copyright (C) 2002-2004 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpSDADecryptUtils_h
#define Included_pgpSDADecryptUtils_h

/****************************************************************************
	Includes
****************************************************************************/

#include "pgpSDA.h"
#include "pgpSDAPriv.h"


/****************************************************************************
	Data Types
****************************************************************************/

/* Signal handler variable */
extern long					g_bSignalBreak;

/*
 *	Exit codes
 */
#define kPGPsdaExit_Success						0
#define kPGPsdaExit_Interrupt					1

/*
 *	Basic logging
 */
enum PGPsdaLogLevel_
{
	kPGPsdaLogLevel_Error						= 0,
	kPGPsdaLogLevel_Prompt						= 1,
	kPGPsdaLogLevel_User						= 2,
	kPGPsdaLogLevel_Verbose						= 3,
	kPGPsdaLogLevel_Debug						= 4,

	PGP_ENUM_FORCE (PGPsdaLogLevel_)
};
PGPENUM_TYPEDEF (PGPsdaLogLevel_, PGPsdaLogLevel);


/*
 *	Keep track of parser options
 */
typedef struct _PGPsdaOptions
{
	PGPChar *				sdaName;

	PGPBoolean				bHelp;
	PGPBoolean				bVersion;
	PGPBoolean				bOverwrite;
	PGPChar8 *				passphrase8;
	PGPChar					outputDir[kPGPsdaPath_Max];
	PGPBoolean				bStripDirectories;
	PGPBoolean				bVerbose;
	PGPBoolean				bDebug;
	PGPBoolean				bList;

} PGPsdaOptions, *PGPsdaOptionsRef;

#define kInvalidPGPsdaOptionsRef				((PGPsdaOptionsRef) NULL)
#define PGPsdaOptionsRefIsValid(ref)			((ref) != kInvalidPGPsdaOptionsRef)


/*
 *	Keep track of decode state
 */
typedef struct _PGPsdaDecodeData
{
	PGPsdaOptionsRef				options;

	PGPsdaHeaderRef					header;
	PGPByte *						adkFooter;
	PGPsdaFooterRef					footer;

	PGPSymmetricCipherContextRef	cipherContext;
	PGPSize							blockSize;
	PGPSize							keySize;
	PGPCBCContextRef				cbcContext;

	/* Decrypt state */
	PGPsdaInputDataRef				input;		/* SDA input file */
	PGPsdaOutputDataRef				output;		/* Current output object */
	PGPsdaEventHandlerProcPtr		handler;
	PGPUserValue					userValue;
	PGPUInt32						numObjects;
	PGPChar							lastInternalName[kPGPsdaPath_Max];

} PGPsdaDecodeData, *PGPsdaDecodeDataRef;

#define kInvalidPGPsdaDecodeDataRef				((PGPsdaDecodeDataRef) NULL)
#define PGPsdaDecodeDataRefIsValid(ref)			((ref) != kInvalidPGPsdaDecodeDataRef)


/****************************************************************************
	Defines
****************************************************************************/

/* Functions */
#define logMessage								pgpSDALogMessage
#if PGP_WIN32
#define FPRINTF									_ftprintf
#define SNPRINTF								_sntprintf
#define STRERROR								_tcserror
#else
#define FPRINTF									fprintf
#define SNPRINTF								snprintf
#define STRERROR								strerror
#endif

/*
 *	Logging
 */
#define L_ERR									kPGPsdaLogLevel_Error
#define L_PMT									kPGPsdaLogLevel_Prompt
#define L_USR									kPGPsdaLogLevel_User
#define L_NFO									kPGPsdaLogLevel_Verbose
#define L_DBG									kPGPsdaLogLevel_Debug
/* Header strings */
#define kPGPsdaLogString_Caption				PGPTXT_USER("PGP SDA")
/* User strings */
#define kPGPsdaString_Unknown					PGPTEXT("unknown")
#define kPGPsdaString_ExtractingObject			PGPTEXT("Extracting '%s'")
#define kPGPsdaErrorString_UnknownError			PGPTEXT("Unknown error")
#define kPGPsdaErrorString_SDAOpenFailed		PGPTEXT("SDA could not be opened for reading")
#define kPGPsdaErrorString_InvalidSDA			PGPTEXT("SDA is not in the correct format")
#define kPGPsdaErrorString_InvalidPassphrase	PGPTEXT("Invalid passphrase")
#define kPGPsdaErrorString_InvalidOutputDir		PGPTEXT("Invalid output directory specified")
#define kPGPsdaErrorString_GenericError			PGPTEXT("Error decrypting archive")
#define kPGPsdaErrorString_FileDecodeError		PGPTEXT("Error decoding file")
#define kPGPsdaErrorString_UnsupportedCipher	PGPTEXT("Unsupported cipher algorithm")
#define kPGPsdaErrorString_UnsupportedHash		PGPTEXT("Unsupported hash algorithm")
#define kPGPsdaErrorString_UnsupportedCompress	PGPTEXT("Unsupported compression algorithm")
#define kPGPsdaErrorString_FilenameTooLong		PGPTEXT("File name too long")
#define kPGPsdaErrorString_FilenameTooLongU16	PGPTEXT("Unicode file name too long")
#define kPGPsdaErrorString_FilenameNotUnicode	PGPTEXT("Could not convert file name to Unicode")
#define kPGPsdaErrorString_MkdirHier			PGPTEXT("Error creating directory")
#define kPGPsdaErrorString_CantCreateFile		PGPTEXT("Error creating file")
#define kPGPsdaErrorString_CantCreateLink		PGPTEXT("Error creating symbolic link")
#define kPGPsdaErrorString_Unlink				PGPTEXT("Error overwriting file")
#define kPGPsdaErrorString_AbortQuestion		PGPTEXT("Cancel operation, are you sure?")
#define kPGPsdaErrorString_NumObjects			PGPTEXT("Number of extracted objects does not match footer")


/****************************************************************************
	Public Functions
****************************************************************************/

/*
 *	Free a PGPsdaOptionsRef type
 */
PGPError
pgpFreeSDAOptions(
	PGPsdaOptionsRef			options);

/*
 *	Log an error, verbose or debug message
 *
 *	Notes:
 *		Win32:
 *			Errors - logged with message box (stop icon)
 *			Prompt - do not use (use Win32 GUI instead)
 *			User - logged with message box (info icon)
 *			Verbose - logged to a log file
 *			Debug - logged to a log file
 *				The name of the log file is <sdaName>.log (sda.exe.log for example)
 *		Unix:
 *			Errors - logged to stderr (with newline added)
 *			Prompt - logged to stderr (with newline added)
 *			User - logged to stdout (no newline added)
 *			Verbose - logged to stdout (no newline added)
 *			Debug - logged to stdout (no newline added)
 */
void
pgpSDALogMessage(
	PGPsdaOptionsRef		options,
	PGPsdaLogLevel			level,
	PGPChar *				format,
	...						);

/*
 *	Initialize signal handlers
 *
 *	Return Value:
 *		PGPError - unexpected error
 */
PGPError
pgpSDAInitSignals(
	void				);

/*
 *	Parse the command line
 *
 *	Format:
 *		<SDAName> [--help] | [--version]
 *		<SDAName> <Options>
 *
 *		[--verbose] and [--debug] can be used anywhere
 *
 *	Parameters:
 *		argc - number of arguments
 *		argv - argument list
 *		options - allocated options struct
 *			Must be freed with pgpFreeSDAOptions()
 *
 *	Return Value:
 *		PGPError - internal error
 *		kPGPError_NoErr + options->bHelp for other errors
 */
PGPError
pgpSDAParseCommandLine(
	int								argc,
	PGPChar *						argv[],
	PGPsdaOptionsRef *				pOptions);

/*
 *	Display a help message
 *
 *	Win32:
 *		A message box is used
 *	Unix:
 *		stderr is used
 */
void
pgpSDAShowHelp(
	PGPsdaOptionsRef			options);

/*
 *	Display information about the current SDA
 *
 *	Win32:
 *		A message box is used
 *	Unix:
 *		stderr is used
 */
void
pgpSDAShowVersion(
	PGPsdaOptionsRef			options,
	PGPsdaFooterRef				footer,
	PGPsdaHeaderRef				header);

/*
 *	Read and verify an SDA footer
 *
 *	Parameters:
 *		fin - where to read from
 *		footer - pointer to an existing (empty) footer structure
 *
 *	Return Value:
 *		PGPError
 *			kPGPError_BadParams
 *			kPGPError_CorruptData - Invalid PGP SDA footer
 */
PGPError
pgpSDAReadFooter(
	FILE *				fin,
	PGPsdaFooterRef		footer);

/*
 *	Read and verify an SDA header
 *
 *	Parameters:
 *		fin - where to read from
 *		footer - pointer to an existing footer structure
 *			This must contain valid data for the SDA we are working on
 *		header - pointer to an existing (empty) header structure
 *
 *	Return Value:
 *		PGPError
 *			kPGPError_BadParams
 *			kPGPError_CorruptData - Invalid PGP SDA header
 */
PGPError
pgpSDAReadHeader(
	FILE *				fin,
	PGPsdaFooterRef		footer,
	PGPsdaHeaderRef		header);

/*
 *	Read and verify an SDA ADK footer
 *
 *	Parameters:
 *		fin - where to read from
 *		footer - pointer to an existing footer structure
 *			This must contain valid data for the SDA we are working on
 *		pADKFooter - pointer to a location to return the ADK footer
 *		pADKFooterSize - pointer to a location to return the size of the footer
 *			These must be freed by the caller with free
 *
 *	Return Value:
 *		PGPError
 *			kPGPError_BadParams
 *			kPGPError_CorruptData - Invalid PGP SDA header
 */
PGPError
pgpSDAReadADKFooter(
	FILE *				fin,
	PGPsdaFooterRef		footer,
	PGPByte **			pADKFooter,
	PGPSize *			pADKFooterSize);

/*
 *	Get a passphrase from the user if one wasn't supplied
 *	Verify the passphrase before returning
 *	Loop until cancelled or passphrase is correct
 *	This will work for Windows and Unix
 *
 *	Parameters:
 *		decodeData - current decode state
 *
 *	Return Value:
 *		Unexpected error:
 *			kPGPError_BadParams
 *		Since we check the passphrase see the return value for
 *			pgpSDAValidatePassphrase()
 */
PGPError
pgpSDAGetPassphrase(
	PGPsdaDecodeDataRef		decodeData);

/*
 *	Check a passphrase to see if it can decrypt this SDA
 *
 *	This function shows error log messages for unexpected errors
 *	This function shows error log messages for invalid passphrases
 *
 *	Parameters:
 *		passphrase8 - UTF8 passphrase to check
 *		decodeData - SDA decode data
 *
 *	Return Value:
 *		Good Passphrase:
 *			kPGPError_NoErr
 *		Bad Passphrase:
 *			kPGPError_BadPassphrase
 *		Unexpected Error:
 *			PGPError
 */
PGPError
pgpSDAValidatePassphrase(
	PGPChar8 *				passphrase8,
	PGPsdaDecodeDataRef		decodeData);

/*
 *	Check a passkey to see if it can decrypt this SDA
 *
 *	This function DOES NOT show error log messages for unexpected errors
 *	This function DOES NOT show error log messages for invalid passphrases
 *		This is because it is designed to only be called from the library interface
 *
 *	Parameters:
 *		passkey - session key bytes
 *		passkeySize - size of the session key
 *		decodeData - SDA decode data
 *
 *	Return Value:
 *		Good Passphrase:
 *			kPGPError_NoErr
 *		Bad Passphrase:
 *			kPGPError_BadPassphrase
 *		Unexpected Error:
 *			PGPError
 */
PGPError
pgpSDAValidatePasskey(
	PGPByte *				passkey,
	PGPSize					passkeySize,
	PGPsdaDecodeDataRef		decodeData);

/*
 *	Create a new directory hierarchy
 *
 *	This function does not show error log messages for unexpected errors
 *
 *	Parameters:
 *		path - Path information (must have trailing /)
 *			If any of the pieces of path exist and they are directories they
 *			will be ignored.  If they are not directories or for some reason
 *			cannot be created an error will be returned.
 *			If there is no trailing / the last item is assumed to be a file
 *
 *	Return Value:
 *		PGPError - Status is logged for errors
 */
PGPError
pgpSDAMkdirHier(
	PGPChar *				path,
	int *					pErrno);

/*
 *	Remove a file
 */
PGPError
pgpSDARemoveFile(
	PGPChar *			file,
	int	*				pErrno);

/*
 *	Prompt the user for a new output file
 *	Validate the new name provided
 *	Prompt for overwrite if the new name exists
 *	Make sure we can create the new name
 *
 *	Parameters:
 *		decodeData - current decode state
 *		oldName - original file name
 *		pNewName - pointer to receive the new name
 *			must be freed by the caller with pgpSDAFreeNewOutputFile
 *
 *	Return Value:
 *		This function will try to loop until everything is ok
 *		Expected Errors:
 *			kPGPError_UserAbort
 */
PGPError
pgpSDAGetNewOutputFile(
	PGPsdaDecodeDataRef		decodeData,
	PGPChar *				oldName,
	PGPChar **				pNewName);

/*
 *	Free a file name allocated by pgpSDAGetNewOutputFile
 */
PGPError
pgpSDAFreeNewOutputFile(
	PGPChar *				newName);


/*
 *	Ask a yes no question (requires enter to be pressed)
 *
 *	Parameters:
 *		question - question to ask (should include ?)
 *		bDefault - default answer
 *		pbAnswer - pointer to the location to store the answer
 *
 *	Return Value:
 *		PGPError - unexpected error
 *		kPGPError_UserAbort
 */
#if PGP_UNIX
PGPError
pgpSDAAskYesNo(
	PGPChar *				question,
	PGPBoolean				bDefault,
	PGPBoolean *			pbAnswer);
#endif

/*
 *	Ask a question (requires enter to be pressed)
 *
 *	Parameters:
 *		question - question to ask (should include ?)
 *		answer - buffer to store answer
 *		answerSize - sizeof answer
 *			If too many characters are read answer is terminated
 *			and we return (just like fgets)
 *
 *	Return Value:
 *		PGPError - unexpected error
 *		kPGPError_UserAbort
 */
#if PGP_UNIX
PGPError
pgpSDAAskQuestion(
	PGPChar *				question,
	PGPChar *				answer,
	unsigned int			answerSize);
#endif


/*
 *	Prompt and return a PGPError if the user really wants to cancel
 */
PGPError
pgpSDAVerifyUserAbort(
	void					);


#endif /* Included_pgpSDADecryptUtils_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
