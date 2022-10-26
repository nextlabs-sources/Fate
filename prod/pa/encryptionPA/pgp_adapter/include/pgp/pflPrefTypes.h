/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pflPrefTypes_h	/* [ */
#define Included_pflPrefTypes_h

#include "pgpBase.h"
#include "pgpPFLErrors.h"
#include "pflPrefs.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



PGPError PGPGetPrefBoolean(PGPPrefRef prefRef,
								PGPPrefIndex prefIndex,
								PGPBoolean *data);

PGPError PGPSetPrefBoolean(PGPPrefRef prefRef,
								PGPPrefIndex prefIndex,
								PGPBoolean data);

PGPError PGPGetPrefNumber(PGPPrefRef prefRef,
							   PGPPrefIndex prefIndex,
							   PGPUInt32 *data);

PGPError PGPSetPrefNumber(PGPPrefRef prefRef,
							   PGPPrefIndex prefIndex,
							   PGPUInt32 data);

PGPError PGPGetPrefStringAlloc(PGPPrefRef prefRef,
									PGPPrefIndex prefIndex,
									PGPUTF8 **string);

PGPError PGPGetPrefStringBuffer(PGPPrefRef prefRef,
									PGPPrefIndex prefIndex,
									PGPSize maxPGPUTF8s,
									PGPUTF8 *string);

PGPError PGPSetPrefString(PGPPrefRef prefRef,
							   PGPPrefIndex prefIndex,
							   const PGPUTF8 *string);

PGPError PGPGetPrefStruct(PGPPrefRef prefRef,
							PGPPrefIndex prefIndex,
							PGPPrefStruct **structPtr);

PGPError PGPSetPrefStruct(PGPPrefRef prefRef,
							PGPPrefIndex prefIndex,
							PGPPrefStruct *structPtr);

PGPError PGPGetPrefArray(PGPPrefRef prefRef,
							PGPPrefIndex prefIndex,
							PGPPrefArray **arrayPtr);

PGPError PGPSetPrefArray(PGPPrefRef prefRef,
							PGPPrefIndex prefIndex,
							PGPPrefArray *arrayPtr);

PGPError PGPGetPrefArrayBoolean(PGPPrefRef prefRef,
									PGPPrefArray *prefArray,
									PGPUInt32 arrayIndex,
									PGPBoolean *data);

PGPError PGPSetPrefArrayBoolean(PGPPrefRef prefRef,
									PGPPrefArray *prefArray,
									PGPUInt32 arrayIndex,
									PGPBoolean data);

PGPError PGPGetPrefArrayNumber(PGPPrefRef prefRef,
									PGPPrefArray *prefArray,
									PGPUInt32 arrayIndex,
									PGPUInt32 *data);

PGPError PGPSetPrefArrayNumber(PGPPrefRef prefRef,
									PGPPrefArray *prefArray,
									PGPUInt32 arrayIndex,
									PGPUInt32 data);

PGPError PGPGetPrefArrayString(PGPPrefRef prefRef,
									PGPPrefArray *prefArray,
									PGPUInt32 arrayIndex,
									PGPUTF8 **string);

PGPError PGPSetPrefArrayString(PGPPrefRef prefRef,
									PGPPrefArray *prefArray,
									PGPUInt32 arrayIndex,
									PGPUTF8 *string);

PGPError PGPGetPrefArrayByte(PGPPrefRef prefRef,
									PGPPrefArray *prefArray,
									PGPUInt32 arrayIndex,
									PGPSize *dataLength,
									void **data);

PGPError PGPSetPrefArrayByte(PGPPrefRef prefRef,
									PGPPrefArray *prefArray,
									PGPUInt32 arrayIndex,
									PGPSize dataLength,
									void *data);

PGPError PGPGetPrefArrayStruct(PGPPrefRef prefRef,
									PGPPrefArray *prefArray,
									PGPUInt32 arrayIndex,
									PGPPrefStruct **prefStruct);

PGPError PGPSetPrefArrayStruct(PGPPrefRef prefRef,
									PGPPrefArray *prefArray,
									PGPUInt32 arrayIndex,
									PGPPrefStruct *prefStruct);

PGPError PGPGetPrefStructBoolean(PGPPrefRef prefRef,
									PGPPrefStruct *prefStruct,
									PGPUInt16 memberIndex,
									PGPBoolean *data);

PGPError PGPSetPrefStructBoolean(PGPPrefRef prefRef,
									PGPPrefStruct *prefStruct,
									PGPUInt16 memberIndex,
									PGPBoolean data);

PGPError PGPGetPrefStructNumber(PGPPrefRef prefRef,
									PGPPrefStruct *prefStruct,
									PGPUInt16 memberIndex,
									PGPUInt32 *data);

PGPError PGPSetPrefStructNumber(PGPPrefRef prefRef,
									PGPPrefStruct *prefStruct,
									PGPUInt16 memberIndex,
									PGPUInt32 data);

PGPError PGPGetPrefStructString(PGPPrefRef prefRef,
									PGPPrefStruct *prefStruct,
									PGPUInt16 memberIndex,
									PGPUTF8 **string);

PGPError PGPSetPrefStructString(PGPPrefRef prefRef,
									PGPPrefStruct *prefStruct,
									PGPUInt16 memberIndex,
									PGPUTF8 *string);

PGPError PGPGetPrefStructByte(PGPPrefRef prefRef,
									PGPPrefStruct *prefStruct,
									PGPUInt16 memberIndex,
									PGPSize *dataLength,
									void **data);

PGPError PGPSetPrefStructByte(PGPPrefRef prefRef,
									PGPPrefStruct *prefStruct,
									PGPUInt16 memberIndex,
									PGPSize dataLength,
									void *data);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pflPrefTypes_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
