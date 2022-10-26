/*____________________________________________________________________________
	Copyright (C) 2002-2004 PGP Corporation
	All rights reserved.

	Private SDA header file.

	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpSDAPriv_h
#define Included_pgpSDAPriv_h


/****************************************************************************
	Includes
****************************************************************************/

#if PGP_WIN32
#include <windows.h>
#include <io.h>
#endif

#include "pgpSDA.h"
#include "pgpPubTypes.h"

#if PGP_WIN32
#pragma warning (disable : 4127)	/* Conditional expression is constant */
#if !PGP_DEBUG
#pragma warning (disable : 4702)	/* Unreachable code, in RETVAL */
#endif /* !PGP_DEBUG */
#endif/* PGP_WIN32 */


/****************************************************************************
	Notes
****************************************************************************/

/*
 *	Internal SDA format:
 *
 *		+----------+
 *		|          |
 *		|          |	SDA Stub (variable size)
 *		|          |
 *		+----------+
 *		|          |	SDA Header (fixed size, 128 octets)
 *		+----------+
 *		|          |
 *		|          |	Compressed and encrypted data (variable size)
 *		|          |
 *		+----------+
 *		|          |	SDA ADK Footer (variable size, can be 0 bytes)
 *		+----------+
 *		|          |	SDA Footer (fixed size, 1024 octets)
 *		+----------+
 *
 *		SDA Stub - executable stub for correct platform
 *		SDA Header - header information for SDA
 *		data - the compressed and encrypted data
 *			type (1 octet)
 *			size (8 octets)
 *				for directories this field is always zero
 *			name (variable, terminated with UTF8('\0'))
 *			data (variable)
 *				file:
 *					size = length of file
 *					contains file data
 *				directory:
 *					size = 0
 *					contains nothing (zero octets)
 *				symbolic link (not Windows shortcuts):
 *					size = length of link target (including '\0')
 *					contains the name of the link target in UTF8
 *						note: name is terminated with UTF8('\0')
 *		ADK Footer - session key encrypted to ADKs
 *			This is not required
 *			Not present when (footer->adkFooterStart == footer->adkFooterEnd)
 *		SDA Footer - most of the SDA information is stored here
 *
 */

/****************************************************************************
	Defines
****************************************************************************/

#if PGP_UNIX_DARWIN
/*
 *	In order to build fat binaries on OSX we need to build each file twice,
 *	once for PPC and once for i386.  Each of those targets requires a different
 *	endian setting.  Because of this, we cannot set this option in the project
 *	like we do for Unix and win32.  All SDA files that do endian conversion
 *	will need to include this header file (and in the right order).
 */
#if BYTE_ORDER == BIG_ENDIAN
#define WORDS_BIGENDIAN				1
#else
#define WORDS_BIGENDIAN				0
#endif
#endif /* PGP_UNIX_DARWIN */

#define kPGPsdaMagic_Header			0x50475368		/* 'PGSh' */
#define kPGPsdaMagic_Footer			0x50475366		/* 'PGSf' */
#define kPGPsdaVersion_Major		9
#define kPGPsdaVersion_Minor		0
#define kPGPsdaHeaderSize_Bytes		128
#define kPGPsdaFooterSize_Bytes		1024

#define kPGPsdaBufferSize_In		4096
#define kPGPsdaBufferSize_Out		4096
#define kPGPsdaBufferSize_Inflate	(kPGPsdaBufferSize_Out * 8)

/* File modes */
#define kPGPsda_FileReadMode		PGPTXT_MACHINE("rb")
#define kPGPsda_FileWriteMode		PGPTXT_MACHINE("wb")
#define kPGPsda_FileAppendMode		PGPTXT_MACHINE("ab")

/* No confusion */
#if PGP_WIN32
#define kPGPsdaPath_Max				_MAX_PATH
#else
#define kPGPsdaPath_Max				MAXPATHLEN
#endif

/* Return codes (used in decode) */
#define kPGPsdaExit_Success			0
#define kPGPsdaExit_RegisterClass	128
#define kPGPsdaExit_CreateWindow	129

/* Temporary debugging code (check-in with this set to zero) */
#define SDA_FIXED_BREAK_POINT		0

/* Tick sizes for misc objects */
#define kPGPsdaTickSize_NonFile		1
#define kPGPsdaTickSize_EmptyFile	1


/****************************************************************************
	Macros
****************************************************************************/

/*
 *	Error checking
 */
#define CKERR						do { if (IsPGPError (err)) goto done; } while (0)
#define RETVAL(x)					do { err = (x); goto done; } while (0)
#define CKNULL(ptr)					do { if ((ptr) == NULL) { err = kPGPError_OutOfMemory; goto done; } } while (0)

#if PGP_UNIX_AIX
/* dirent.h defines STAT which we don't use (and collides with our name) */
#undef STAT
/* fcntl.h defines FOPEN when we don't use (and collides with our name) */
#undef FOPEN
#endif

/*
 *	File helpers
 */
#if PGP_WIN32
 #define FOPEN						_tfopen
 #define MKDIR						_tmkdir
 #define UNLINK						_tunlink
 #define GETCWD						_tgetcwd
 #define STAT						_tstati64
 #define STAT_STRUCT				struct _stati64
 #define FSEEK						_lseeki64
#else /* ! PGP_WIN32 */
 #define MKDIR						mkdir
 #define UNLINK						unlink
 #define GETCWD						getcwd
#if HAVE_64BIT_FILES
 #define FOPEN						fopen64
 #define STAT						stat64
 #define LSTAT						lstat64
 #define STAT_STRUCT				struct stat64
 #define FSEEK						fseeko64
#else
 #define FOPEN						fopen
 #define STAT						stat
 #define LSTAT						lstat
 #define STAT_STRUCT				struct stat
 #define FSEEK						fseek
#endif
#endif /* PGP_WIN32 */

#if PGP_WIN32
 #define S_ISREG(mode)				(((mode) & _S_IFREG) == _S_IFREG)
 #define S_ISDIR(mode)				(((mode) & _S_IFDIR) == _S_IFDIR)
 #define S_ISLNK(mode)				FALSE
#endif

#if PGP_WIN32
 #define DIR_SEP_CHR				PGPTXT_MACHINE8('\\')
 #define DIR_SEP_STR				PGPTXT_MACHINE8("\\")
#else
 #define DIR_SEP_CHR				PGPTXT_MACHINE8('/')
 #define DIR_SEP_STR				PGPTXT_MACHINE8("/")
#endif


/****************************************************************************
	Data Types
****************************************************************************/

/*
 *	Supported file types with SDAs
 *
 *	SEE_ALSO pgpSDA.h:PGPsdaObjectType
 */
enum PGPsdaFileType_
{
	kPGPsdaFileType_Unknown				= 0,
	kPGPsdaFileType_File				= 1,
	kPGPsdaFileType_Directory			= 2,
	kPGPsdaFileType_SymLink				= 3,

	kPGPsdaFileType_SymLinkTarget		= 10,

	PGP_ENUM_FORCE (PGPsdaFileType_)
};
PGPENUM_TYPEDEF (PGPsdaFileType_, PGPsdaFileType);


/*
 *	Internal file list format
 */
typedef struct _PGPsdaInputFileList
{
	PGPChar *							name;		/* User provided name */

	PGPChar8 *							workingName;
	PGPsdaFileType						type;
	PGPUInt64							size;
	PGPChar8 *							linkTarget;
	PGPBoolean							bAutoLaunchOnDecrypt;
	PGPUInt8							autoLaunchFileFlags;

	struct _PGPsdaInputFileList *		next;

} PGPsdaInputFileListElement, *PGPsdaInputFileList;


/*
 *	SDA header
 */
typedef struct _PGPsdaHeader
{
	/* Magic */
	PGPUInt32				magic;

	/* Reserved for future use */
	PGPByte					reserved[124];		/* 128 - 4 */

} PGPsdaHeader, *PGPsdaHeaderRef;

#define kInvalidPGPsdaHeaderRef					((PGPsdaHeaderRef) NULL)
#define PGPsdaHeaderRefIsValid(ref)				((ref) != kInvalidPGPsdaHeaderRef)


/*
 *	SDA footer
 */
typedef struct _PGPsdaFooter
{
	/* Magic */
	PGPUInt32				topMagic;

	/* Offsets */
	PGPUInt64				headerStart;
	PGPUInt64				headerEnd;
	PGPUInt64				dataStart;
	PGPUInt64				dataEnd;
	PGPUInt64				adkFooterStart;
	PGPUInt64				adkFooterEnd;
	PGPUInt64				footerStart;
	PGPUInt64				footerEnd;			/* Note: One past the EOF */

	/* Data information */
	PGPUInt64				dataSize;
	PGPUInt16				cipherAlgorithm;
	PGPUInt16				hashAlgorithm;
	PGPUInt16				compressionAlgorithm;
	PGPUInt8				salt[16];
	PGPUInt8				checkBytes[16];
	PGPUInt16				hashReps;

	/* Other features */
	PGPUInt32				numObjects;			/* Total number of objects in the archive */
	PGPUInt32				autoExecFileNum;	/* Number of the object to exec on decrypt (1 based) */
	PGPUInt8				autoExecFileFlags;

	/* Reserved for future use */
	PGPByte					reserved[883];		/* 1024 - 141 */

	/* Size, version, and more magic */
	PGPUInt64				footerSize;
	PGPUInt16				majorVersion;
	PGPUInt16				minorVersion;
	PGPUInt32				bottomMagic;

} PGPsdaFooter, *PGPsdaFooterRef;

#define kInvalidPGPsdaFooterRef					((PGPsdaFooterRef) NULL)
#define PGPsdaFooterRefIsValid(ref)				((ref) != kInvalidPGPsdaFooterRef)


/*
 *	Read state in the state machine (these values must not exceed 8-bits)
 */
enum PGPsdaReadState_
{
	kPGPsdaReadState_Invalid			= 0,
	kPGPsdaReadState_FileType			= 1,
	kPGPsdaReadState_FileSize			= 2,
	kPGPsdaReadState_FileName			= 3,
	kPGPsdaReadState_FileContents		= 4,

	PGP_ENUM_FORCE (PGPsdaReadState_)

};
PGPENUM_TYPEDEF (PGPsdaReadState_, PGPsdaReadState);


/*
 *	Keep track of the input during encode
 */
typedef struct _PGPsdaInputData
{
	PGPUInt32				placeHolder;
	PGPChar8				fileName[kPGPsdaPath_Max];
	FILE *					fp;
	PGPUInt64				bytesRead;
	PGPsdaInputFileList		fileList;
	PGPsdaReadState			readState;

} PGPsdaInputData, *PGPsdaInputDataRef;

#define kInvalidPGPsdaInputDataRef				((PGPsdaInputDataRef) NULL)
#define PGPsdaInputDataRefIsValid(ref)			((ref) != kInvalidPGPsdaInputDataRef)


/*
 *	Keep track of the output during decode
 */
typedef struct _PGPsdaOutputData
{
	PGPsdaReadState			readState;
	PGPUInt32				placeHolder;
	PGPUInt64				fileSize;
	PGPsdaFileType			fileType;
	PGPChar8				fileName[kPGPsdaPath_Max];
	PGPChar					workingName[kPGPsdaPath_Max];
	PGPChar8				linkTarget[kPGPsdaPath_Max];
	FILE *					fp;
	PGPUInt64				bytesWritten;

} PGPsdaOutputData, *PGPsdaOutputDataRef;

#define kInvalidPGPsdaOutputDataRef				((PGPsdaOutputDataRef) NULL)
#define PGPsdaOutputDataRefIsValid(ref)			((ref) != kInvalidPGPsdaOutputDataRef)


/*
 *	PGPsdaContext
 */
struct _PGPsdaContext
{
	PGPContextRef					context;
	PGPMemoryMgrRef					memMgr;

	PGPBoolean						bUsed;
	PGPBoolean						bAutoLaunchUsed;

	PGPsdaInputFileList				fileList;
	PGPUInt64						listSize;

	PGPByte *						sessionKey;
	PGPSize							sessionKeySize;

	/* Callbacks */
	PGPsdaEventHandlerProcPtr		handler;
	PGPUserValue					userValue;

	/* Keep track of status (for null events, etc.) */
	PGPUInt32						numObjects;
	PGPUInt32						currentObject;
	PGPUInt64						ticks;
	PGPUInt64						totalTicks;
	PGPUInt32						autoExecFileNum;
	PGPUInt8						autoExecFileFlags;
};


#endif /* Included_pgpSDAPriv_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
