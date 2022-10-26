/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pflPrefs_h	/* [ */
#define Included_pflPrefs_h

#include "pgpPFLConfig.h"
#include "pgpBase.h"
#include "pflTypes.h"
#include "pgpMemoryMgr.h"

typedef struct PGPPref *	PGPPrefRef;
typedef PGPInt32			PGPPrefIndex;

#define PGPValidatePref(prefRef)		\
	if (prefRef == NULL)				\
	{									\
		pgpAssert(prefRef != NULL);		\
		return( kPGPError_BadParams );	\
	}

/* You cannot have arrays of arrays, or structs with array members,
	but you can have arrays of structs */

typedef enum PGPPrefType
{
	kPGPPrefType_Invalid = 0,
	kPGPPrefType_Boolean = 1,
	kPGPPrefType_Number = 2,
	kPGPPrefType_String = 3,
	kPGPPrefType_Byte = 4,
	kPGPPrefType_Struct = 5,
	kPGPPrefType_Array = 6,
	kPGPPrefType_NumTypes,
	PGP_ENUM_FORCE( PGPPrefType )
} PGPPrefType;

#if PGP_MACINTOSH
#pragma options align=mac68k
#endif

typedef struct PGPPrefStructMember
{
	PGPPrefType	type;
	void *		data;
	PGPSize		size;	/* Only needed for Byte type */
} PGPPrefStructMember;

typedef struct PGPPrefStruct
{
	PGPByte					numMembers;
	PGPPrefStructMember *	members;
	PGPBoolean				dirty;		/* For internal use only */
} PGPPrefStruct;

typedef struct PGPPrefArrayElement
{
	void *		data;
	PGPSize		size; 		/* Only needed for Byte type */
} PGPPrefArrayElement;

typedef struct PGPPrefArray
{
	PGPPrefType				type;
	PGPUInt32				numElements;
	PGPPrefArrayElement *	elements;
	PGPPrefStruct *			templateStruct; /* Used with struct arrays */
	PGPBoolean				dirty;			/* For internal use only */
} PGPPrefArray;

typedef struct PGPPrefDefinition
{
	PGPPrefIndex	index;
	char *			name;
	PGPPrefType		type;
	void *			data;
	PGPSize			size;	/* Only needed for Byte type */
} PGPPrefDefinition;


typedef	void (*PGPFreePrefsUserValueProc)(PGPUserValue inUserValue);
 

#if PGP_MACINTOSH
#pragma options align=reset
#endif

#define	kInvalidPGPPrefRef			((PGPPrefRef) NULL)
#define PGPPrefRefIsValid( ref )	( (ref) != kInvalidPGPPrefRef )

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPError PGPOpenPrefFile(PFLFileSpecRef prefFileSpec, 
							const PGPPrefDefinition *prefs,
							PGPUInt32 numPrefs,
							PGPPrefRef *prefRef);

PGPError PGPSavePrefFile(PGPPrefRef prefRef);

PGPError PGPGetPrefFileSpec(PGPPrefRef prefRef,
								PFLFileSpecRef *prefFileSpec);

PGPError PGPNewMemoryPrefs(PGPMemoryMgrRef memoryMgr, 
							const PGPPrefDefinition *prefs,
							PGPUInt32 numPrefs,
							PGPPrefRef *prefRef);

PGPError PGPAddPrefs(PGPPrefRef prefsToAdd, PGPPrefRef prefRef);

PGPError PGPFreePrefs(PGPPrefRef prefRef);

PGPError PGPPeekPrefMemoryMgr(PGPPrefRef prefRef,
								PGPMemoryMgrRef *memoryMgr);

PGPError PGPGetPrefData(PGPPrefRef prefRef,
							 PGPPrefIndex prefIndex, 
							 PGPSize *dataLength, 
							 void **inBuffer);

PGPError PGPSetPrefData(PGPPrefRef prefRef, 
							 PGPPrefIndex prefIndex, 
							 PGPSize dataLength, 
							 const void *outBuffer);

PGPError PGPClearPrefData(PGPPrefRef prefRef, 
							PGPPrefIndex prefIndex);

PGPError PGPRemovePref(PGPPrefRef prefRef, 
							PGPPrefIndex prefIndex);

PGPError PGPCreatePrefStruct(PGPPrefRef prefRef,
								PGPPrefStruct *templateStruct,
								PGPPrefStruct **newStruct);

PGPError PGPFreePrefStruct(PGPPrefStruct *prefStruct);

PGPError PGPCreatePrefArray(PGPPrefRef prefRef,
								PGPPrefType arrayType,
								PGPUInt32 arraySize,
								PGPPrefStruct *templateStruct,
								PGPPrefArray **prefArray);

PGPError PGPFreePrefArray(PGPPrefArray *prefArray);

PGPError PGPExportPrefsToBuffer(PGPPrefRef prefRef, 
									PGPSize *bufferSize,
									void **buffer);

PGPError PGPImportBufferToMemoryPrefs(PGPMemoryMgrRef memoryMgr,
										const void *buffer, 
										PGPSize bufferSize,
										const PGPPrefDefinition *prefs,
										PGPUInt32 numPrefs,
										PGPPrefRef *prefRef);

PGPError PGPGetPrefFlags(PGPPrefRef prefRef,
							PGPPrefIndex prefIndex,
							PGPUInt32 *flags);

PGPError PGPSetPrefFlags(PGPPrefRef prefRef,
							PGPPrefIndex prefIndex,
							PGPUInt32 bitmask);

PGPError PGPClearPrefFlags(PGPPrefRef prefRef,
							PGPPrefIndex prefIndex,
							PGPUInt32 bitmask);
							
void		PGPSetPrefUserValue(PGPPrefRef prefRef,
								PGPUserValue userValue,
								PGPFreePrefsUserValueProc proc); /*	This will be called when PGPFreePrefs is called.
																	It can be NULL. Use it to dispose of your
																	userValue. Note that the prefRef is no longer
																	valid when proc is called. */
PGPUserValue PGPGetPrefUserValue(PGPPrefRef prefRef);

PGPError PGPSetPrefWrap(PGPPrefRef prefRef,
							PGPUInt32 wrap);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pflPrefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
