/*____________________________________________________________________________
	Copyright (C) 2002-2004 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpSDADecrypt_h
#define Included_pgpSDADecrypt_h


/****************************************************************************
	Includes
****************************************************************************/

#include "pgpBase.h"
#include "pgpPubTypes.h"

#include "pgpSDA.h"


/****************************************************************************
	Public Functions
****************************************************************************/

PGP_BEGIN_C_DECLARATIONS

/*
 *	Decrypt an SDA without executing the stub code
 *
 *	Parameters:
 *		sda - name of the SDA
 *			Win32: Wide chars
 *			Unix: UTF8
 *		passphrase8 - passphrase in UTF8
 *			If this is NULL, the caller will get a callback event with the
 *			ADK information in it.  Note that there may be no ADK information
 *			stored with this SDA, in which case a callback will still be sent
 *		outputDirectory - output files into a different directory than .
 *		bStripDirectories - Output to the current directory
 *			Using this option causes outputDirectory to be ignored
 *		bListOnly - List the objects in the SDA only, do not output them
 *			Files are presented through the new object callback as if they
 *			were being written out.
 *		sdaEventHandlerFunction - event handler (required)
 *		sdaEventHandlerUserValue - event handler user value (optional)
 *
 *	Return Value:
 *		kPGPError_NoErr - success
 *		kPGPError_CorruptData - Not a valid SDA
 *		kPGPError_FeatureNotAvailable - Cannot process this SDA
 *		kPGPError_BadPassphrase - Invalid passphrase
 *		kPGPError_UserAbory - Passed back up from a user abort
 *		kPGPError_MissingPassphrase - No ADK present and ADK decryption
 *			was requested (library interface only)
 */
PGPError
PGPsdaDecrypt(
	PGPChar *					sda,
	PGPChar8 *					passphrase8,
	PGPChar *					outputDirectory,
	PGPBoolean					bStripDirectories,
	PGPBoolean					bListOnly,
	PGPsdaEventHandlerProcPtr	sdaEventHandlerFunction,
	PGPUserValue				sdaEventHandlerUserValue);


/*
 *	Check to see if a file is a self decrypting archive
 *
 *	Parameters:
 *		sda - name of the SDA
 *			Win32: Wide chars
 *			Unix: UTF8
 *
 *	Return Value:
 *		kPGPError_NoErr - file is an SDA
 *		kPGPError_CantOpenFile - could not open the file
 *		kPGPError_MatchNotFound - file is not an SDA
 *		kPGPError_CorruptData - corrupt SDA
 */
PGPError
PGPsdaVerify(
	PGPChar *					sda);


PGP_END_C_DECLARATIONS

#endif /* Included_pgpSDADecrypt_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
