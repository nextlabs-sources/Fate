/*
Written by Derek Zheng
March 2008
*/

#ifndef _PGP_SDK_FUNC_H
#define _PGP_SDK_FUNC_H

#include "pgp/pgpPubTypes.h"
#include "pgp/pgpFeatures.h"
#include "pgp/pgpErrors.h"
#include "pgp/pgpUtilities.h"
#include "pgp/pgpKeyServer.h"
#include "pgp/pgpRandomPool.h"
#include "pgp/pgpEncode.h"
#include "pgp/pgpOptionList.h"
#include "pgp/pgpKeys.h"

#include "pgp/pgpLDAP.h"
#include "pgp/pgpKeyServer.h"
#include "pgp/pgpTLS.h"
#include "pgp/sda/pgpSDA.h"

typedef PGPError	(__cdecl *funcPGPsdkInit)( PGPFlags options ) ;
typedef PGPError	(__cdecl *funcPGPsdkCleanup)( void );

/*____________________________________________________________________________
PGPsdk context manipulation
_____________________________________________________________________________*/
typedef PGPError 	(__cdecl *funcPGPNewContext)( PGPUInt32 sdkAPIVersion, PGPContextRef *newContext );
typedef PGPError 	(__cdecl *funcPGPNewContextCustom)( const PGPCustomContextInfo *contextInfo,
								PGPContextRef *newContext );
typedef PGPError 	(__cdecl *funcPGPFreeContext)( PGPContextRef context );

/*
**	Functions to encode and decode. The variable parameters are one or more
**	PGPOptionListRef's which describe the inputs, outputs, and options.
*/
typedef PGPError 		(__cdecl *funcPGPEncode)(PGPContextRef context,
						  PGPOptionListRef firstOption, ...);
typedef PGPError 		(__cdecl *funcPGPDecode)(PGPContextRef context,
						  PGPOptionListRef firstOption, ...);

/* return a flags word for the feature selector */
typedef PGPError	(__cdecl *funcPGPGetPGPsdkVersionString)( PGPChar16 versionString[ 256 ] );
typedef PGPError 	(__cdecl *funcPGPGetFeatureFlags)( PGPFeatureSelector selector, PGPFlags *flags );

/* Extra functions for entropy estimation */
typedef PGPUInt32 	(__cdecl *funcPGPGlobalRandomPoolGetEntropy)( void );
typedef PGPUInt32 	(__cdecl *funcPGPGlobalRandomPoolGetSize)( void );
typedef PGPUInt32	(__cdecl *funcPGPGlobalRandomPoolGetMinimumEntropy)( void );
typedef PGPBoolean	(__cdecl *funcPGPGlobalRandomPoolHasMinimumEntropy)( void );
typedef PGPError	(__cdecl *funcPGPSetRandSeedFile)( PFLFileSpecRef randSeedFile );
typedef PGPUInt32 	(__cdecl *funcPGPGlobalRandomPoolAddKeystroke)( PGPInt32 event);
typedef PGPUInt32 	(__cdecl *funcPGPGlobalRandomPoolMouseMoved)(void);
typedef PGPError	(__cdecl *funcPGPGlobalRandomPoolAddSystemState)(void);
typedef PGPUInt32	(__cdecl *funcPGPContextReserveRandomBytes)(PGPContextRef context, PGPUInt32 minSize );


/*____________________________________________________________________________
Routines to determine which algorithms are present.

To determine if a specific algorithm is available, you will need to
index through the available algorithms and check the algorithm ID.
____________________________________________________________________________*/
typedef PGPError	(__cdecl *funcPGPCountPublicKeyAlgorithms)( PGPUInt32 *numPKAlgs );
typedef PGPError	(__cdecl *funcPGPGetIndexedPublicKeyAlgorithmInfo)( PGPUInt32 theIndex,
												PGPPublicKeyAlgorithmInfo *info);
typedef PGPError	(__cdecl *funcPGPCountSymmetricCiphers)( PGPUInt32 *numPKAlgs );
typedef PGPError	(__cdecl *funcPGPGetIndexedSymmetricCipherInfo)( PGPUInt32 theIndex,
											 PGPSymmetricCipherInfo *info);

/*____________________________________________________________________________
PGP file management

All files in PGP are represented using an opage data type PGPFileSpecRef.
These data types are created using a fully qualified path or, on the
Macintosh, an FSSpec. The 
____________________________________________________________________________*/
typedef PGPError 	(__cdecl *funcPGPNewFileSpecFromFullPath)( PGPContextRef context,
									   const PGPChar16 *path, PGPFileSpecRef *ref );
typedef PGPError 	(__cdecl *funcPGPGetFullPathFromFileSpec)( PGPFileSpecRef fileRef,
									   PGPChar16 **fullPathPtr);
typedef PGPError 	(__cdecl *funcPGPCopyFileSpec)( PGPFileSpecRef fileRef, PGPFileSpecRef *ref );
typedef PGPError 	(__cdecl *funcPGPFreeFileSpec)( PGPFileSpecRef fileRef );
typedef PGPError	(__cdecl *funcPGPRenameFile)( PGPFileSpecRef fileRef, const PGPChar16 *newName );
typedef PGPError	(__cdecl *funcPGPDeleteFile)( PGPFileSpecRef fileRef );

/*____________________________________________________________________________
Key DB functions
____________________________________________________________________________*/

/* Creat a new, in-memory temporary key DB */
typedef PGPError	(__cdecl *funcPGPNewKeyDB)( PGPContextRef context, PGPKeyDBRef *keyDBRef );
/* Open a (possibly) existing key ring pair on disk */
typedef PGPError	(__cdecl *funcPGPOpenKeyDBFile)( PGPContextRef context,
							 PGPOpenKeyDBFileOptions options,
							 PGPFileSpecRef pubKeysFileSpec,
							 PGPFileSpecRef privKeysFileSpec,
							 PGPKeyDBRef *keyDBRef );
typedef PGPError	(__cdecl *funcPGPFreeKeyDB)( PGPKeyDBRef keyDBRef );
typedef PGPError	(__cdecl *funcPGPFlushKeyDB)( PGPKeyDBRef keyDBRef );
typedef PGPError	(__cdecl *funcPGPIncKeyDBRefCount)( PGPKeyDBRef keyDBRef );
typedef PGPBoolean	(__cdecl *funcPGPKeyDBIsMutable)( PGPKeyDBRef keyDBRef );
typedef PGPError 	(__cdecl *funcPGPFindKeyByKeyID)( PGPKeyDBRef keyDBRef, const PGPKeyID * keyID,
							  PGPKeyDBObjRef *keyRef);
typedef PGPError 	(__cdecl *funcPGPFindKeyByV3orV4KeyID)( PGPKeyDBRef keyDBRef, const PGPKeyID * keyID,
									PGPKeyDBObjRef *keyRef);
typedef PGPError 	(__cdecl *funcPGPCountKeysInKeyDB)( PGPKeyDBRef keyDBRef, PGPUInt32 *numKeys );
typedef PGPError	(__cdecl *funcPGPKeyDBIsUpdated)( PGPKeyDBRef keyDBRef, PGPBoolean *isUpdated );

/* Cache a keydb in memory for specified number of seconds */
typedef PGPError	(__cdecl *funcPGPCacheKeyDB)( PGPKeyDBRef keyDBRef, PGPUInt32 timeoutSeconds );

/* Remove all cached keydbs from memory */
typedef PGPError	(__cdecl *funcPGPPurgeKeyDBCache)( PGPContextRef context );

/*____________________________________________________________________________
Key set functions
____________________________________________________________________________*/

/* Create a new key set containing all of the keys in the key DB */
typedef PGPError  	(__cdecl *funcPGPNewKeySet)( PGPKeyDBRef keyDB, PGPKeySetRef *keySet );
/* Create a new, empty key set */
typedef PGPError  	(__cdecl *funcPGPNewEmptyKeySet)( PGPKeyDBRef keyDB, PGPKeySetRef *keySet );
/* Create a new key set containing a single key */
typedef PGPError  	(__cdecl *funcPGPNewOneKeySet)( PGPKeyDBObjRef key, PGPKeySetRef *keySet );
/* Like PGPNewKeySet but allows certain stub key objects */
typedef PGPError	(__cdecl *funcPGPNewEmptyInclusiveKeySet)( PGPKeyDBRef keyDB, PGPKeySetRef *pset );

/* Like PGPNewOneKeySet but allows certain stub key objects */
typedef PGPError  	(__cdecl *funcPGPNewOneInclusiveKeySet)( PGPKeyDBObjRef key, PGPKeySetRef *keySet );
typedef PGPError 	(__cdecl *funcPGPFreeKeySet)( PGPKeySetRef keys);
typedef PGPError 	(__cdecl *funcPGPIncKeySetRefCount)( PGPKeySetRef keys);
typedef PGPBoolean 	(__cdecl *funcPGPKeySetIsMember)( PGPKeyDBObjRef key, PGPKeySetRef set );
typedef PGPError 	(__cdecl *funcPGPCountKeys)( PGPKeySetRef keys, PGPUInt32 *numKeys );
typedef PGPError 	(__cdecl *funcPGPAddKey)( PGPKeyDBObjRef keyToAdd, PGPKeySetRef set );
typedef PGPError 	(__cdecl *funcPGPAddKeys)( PGPKeySetRef keysToAdd, PGPKeySetRef set );
typedef PGPKeyDBRef		(__cdecl *funcPGPPeekKeySetKeyDB)( PGPKeySetRef keySet );
typedef PGPKeySetRef	(__cdecl *funcPGPPeekKeyDBRootKeySet)( PGPKeyDBRef keyDB );

/*____________________________________________________________________________
Key DB object properties
____________________________________________________________________________*/
typedef PGPError	(__cdecl *funcPGPGetKeyDBObjBooleanProperty)( PGPKeyDBObjRef key,
										  PGPKeyDBObjProperty whichProperty, PGPBoolean *prop );
typedef PGPError 	(__cdecl *funcPGPGetKeyDBObjNumericProperty)( PGPKeyDBObjRef key,
										  PGPKeyDBObjProperty whichProperty, PGPInt32 *prop );
typedef PGPError 	(__cdecl *funcPGPGetKeyDBObjTimeProperty)( PGPKeyDBObjRef key,
									   PGPKeyDBObjProperty whichProperty, PGPTime *prop);
/*
**	Get the data for a binary property. Returns kPGPError_BufferTooSmall if
**	the buffer is too small. Both buffer and dataSize can be NULL.
*/
typedef PGPError 	(__cdecl *funcPGPGetKeyDBObjDataProperty)( PGPKeyDBObjRef key,
									   PGPKeyDBObjProperty whichProperty, void *buffer,
									   PGPSize bufferSize, PGPSize *dataSize);
/*
**	Get the data for a binary property using an allocated output buffer. The
**	allocated buffer must be freed with PGPFreeData(). For convenience, the
**	allocated buffer is null-terminated. The terminating null byte is NOT included
**	is the output dataSize parameter.
*/
typedef PGPError 	(__cdecl *funcPGPGetKeyDBObjAllocatedDataProperty)( PGPKeyDBObjRef key,
												PGPKeyDBObjProperty whichProperty, void **buffer,
												PGPSize *dataSize);
typedef PGPError 	(__cdecl *funcPGPSetKeyEnabled)( PGPKeyDBObjRef key, PGPBoolean enable );
typedef PGPError	(__cdecl *funcPGPSetKeyAxiomatic)( PGPKeyDBObjRef key, PGPBoolean setAxiomatic,
							   PGPOptionListRef firstOption, ...);

/*____________________________________________________________________________
Key DB object property convenience functions
____________________________________________________________________________*/

/* Get the key ID of a key or subkey key DB object */
typedef PGPError	(__cdecl *funcPGPGetKeyID)( PGPKeyDBObjRef key, PGPKeyID *keyID );
typedef PGPError  	(__cdecl *funcPGPGetPrimaryUserID)( PGPKeyDBObjRef key, PGPKeyDBObjRef *outRef );
typedef PGPError	(__cdecl *funcPGPGetPrimaryAttributeUserID) (PGPKeyDBObjRef key,
										  PGPAttributeType attributeType, PGPKeyDBObjRef *outRef);
typedef PGPError 	(__cdecl *funcPGPGetPrimaryUserIDValidity)(PGPKeyDBObjRef key,
										PGPValidity *validity);
typedef PGPError 	(__cdecl *funcPGPGetPrimaryUserIDName)(PGPKeyDBObjRef key, void *buffer,
									PGPSize bufferSize, PGPSize *dataSize);
typedef PGPError 	(__cdecl *funcPGPGetKeyForUsage)( PGPKeyDBObjRef key, PGPUInt32 usageFlags,
							  PGPKeyDBObjRef *outRef );
/*____________________________________________________________________________
Key filters
____________________________________________________________________________*/
typedef PGPError 	(__cdecl *funcPGPNewKeyDBObjBooleanFilter)( PGPContextRef context,
										PGPKeyDBObjProperty whichProperty, PGPBoolean match,
										PGPFilterRef *outFilter );
typedef PGPError 	(__cdecl *funcPGPNewKeyDBObjNumericFilter)( PGPContextRef context,
										PGPKeyDBObjProperty whichProperty, PGPUInt32 matchValue,
										PGPMatchCriterion matchCriteria, PGPFilterRef *outFilter );
typedef PGPError 	(__cdecl *funcPGPNewKeyDBObjTimeFilter)( PGPContextRef context,
									 PGPKeyDBObjProperty whichProperty, PGPTime matchValue,
									 PGPMatchCriterion matchCriteria, PGPFilterRef *outFilter );
typedef PGPError 	(__cdecl *funcPGPNewKeyDBObjDataFilter)( PGPContextRef context,
									 PGPKeyDBObjProperty whichProperty, const void *matchData,
									 PGPSize matchDataSize, PGPMatchCriterion matchCriteria,
									 PGPFilterRef *outFilter );
typedef PGPError 	(__cdecl *funcPGPFreeFilter)( PGPFilterRef filter );
typedef PGPError 	(__cdecl *funcPGPIncFilterRefCount)( PGPFilterRef filter );
typedef PGPError 	(__cdecl *funcPGPFilterChildObjects)( PGPFilterRef filter,
								  PGPBoolean filterChildren );
/* freeing outfilter will call PGPFreeFilter on filter */
typedef PGPError 	(__cdecl *funcPGPNegateFilter)( PGPFilterRef filter, PGPFilterRef *outFilter);
/* freeing outfilter will call PGPFreeFilter on filter1, filter2 */
typedef PGPError 	(__cdecl *funcPGPIntersectFilters)( PGPFilterRef filter1, PGPFilterRef filter2,
								PGPFilterRef *outFilter);
/* freeing outfilter will call PGPFreeFilter on filter1, filter2 */
typedef PGPError 	(__cdecl *funcPGPUnionFilters)( PGPFilterRef filter1, PGPFilterRef filter2,
							PGPFilterRef *outFilter);
typedef PGPError 	(__cdecl *funcPGPFilterKeySet)( PGPKeySetRef origSet, PGPFilterRef filter,
							PGPKeySetRef *resultSet );
typedef PGPError	(__cdecl *funcPGPFilterKeyDB)( PGPKeyDBRef keyDB, PGPFilterRef filter,
						   PGPKeySetRef *resultSet );

/*____________________________________________________________________________
Key DB object creation/deletion
____________________________________________________________________________*/

typedef PGPError	(__cdecl *funcPGPGenerateKey)( PGPContextRef context, PGPKeyDBObjRef *key,
						   PGPOptionListRef firstOption, ...);
typedef PGPError	(__cdecl *funcPGPGenerateSubKey)( PGPContextRef context, PGPKeyDBObjRef *subkey,
							  PGPOptionListRef firstOption, ...);
typedef PGPUInt32	(__cdecl *funcPGPGetKeyEntropyNeeded)( PGPContextRef context,
								   PGPOptionListRef firstOption, ...);
typedef PGPError 	(__cdecl *funcPGPCopyKeyDBObj)( PGPKeyDBObjRef keyDBObj, PGPKeyDBRef destKeyDB,
							PGPKeyDBObjRef *destKeyDBObj );

typedef PGPError 	(__cdecl *funcPGPCopyKeys)( PGPKeySetRef keySet, PGPKeyDBRef destKeyDB,
						PGPKeySetRef *destKeySet );
typedef PGPError 	(__cdecl *funcPGPDeleteKeyDBObj)( PGPKeyDBObjRef keyDBObj );
typedef PGPError 	(__cdecl *funcPGPDeleteKeys)( PGPKeySetRef keySet );

typedef PGPError	(__cdecl *funcPGPExport)( PGPContextRef context,
					  PGPOptionListRef firstOption, ... );
typedef PGPError	(__cdecl *funcPGPImport)( PGPContextRef context, PGPKeyDBRef *importedKeysDB,
					  PGPOptionListRef firstOption, ...);
typedef PGPError	(__cdecl *funcPGPRevokeSig)( PGPKeyDBObjRef cert,
						 PGPOptionListRef firstOption, ...);
typedef PGPError	(__cdecl *funcPGPRevoke)( PGPKeyDBObjRef key,
					  PGPOptionListRef firstOption, ...);
typedef PGPError	(__cdecl *funcPGPChangePassphrase)( PGPKeyDBObjRef key,
								PGPOptionListRef firstOption, ...);
typedef PGPBoolean	(__cdecl *funcPGPPassphraseIsValid)( PGPKeyDBObjRef key,
								 PGPOptionListRef firstOption, ...);
/*____________________________________________________________________________
Key lists
____________________________________________________________________________*/

typedef PGPError  	(__cdecl *funcPGPOrderKeySet)( PGPKeySetRef src, PGPKeyOrdering order,
						   PGPBoolean reverseOrder, PGPKeyListRef *outRef );
typedef PGPError 	(__cdecl *funcPGPIncKeyListRefCount)( PGPKeyListRef keys);
typedef PGPError 	(__cdecl *funcPGPFreeKeyList)( PGPKeyListRef keys );

/*____________________________________________________________________________
Key list iteration
____________________________________________________________________________*/

typedef PGPError 	(__cdecl *funcPGPNewKeyIter)( PGPKeyListRef keys, PGPKeyIterRef *outRef);
typedef PGPError 	(__cdecl *funcPGPNewKeyIterFromKeySet)( PGPKeySetRef keys, PGPKeyIterRef *outRef);
typedef PGPError 	(__cdecl *funcPGPNewKeyIterFromKeyDB)( PGPKeyDBRef keyDB, PGPKeyIterRef *outRef);
typedef PGPError 	(__cdecl *funcPGPCopyKeyIter)( PGPKeyIterRef orig, PGPKeyIterRef *outRef);
typedef PGPError 	(__cdecl *funcPGPFreeKeyIter)( PGPKeyIterRef iter);
typedef PGPInt32 	(__cdecl *funcPGPKeyIterIndex)( PGPKeyIterRef iter);
typedef PGPError 	(__cdecl *funcPGPKeyIterRewind)( PGPKeyIterRef iter, PGPKeyDBObjType objectType);
typedef PGPInt32 	(__cdecl *funcPGPKeyIterSeek)( PGPKeyIterRef iter, PGPKeyDBObjRef key);
typedef PGPError 	(__cdecl *funcPGPKeyIterMove)( PGPKeyIterRef iter, PGPInt32 relOffset,
						   PGPKeyDBObjRef *outRef);
typedef PGPError 	(__cdecl *funcPGPKeyIterNextKeyDBObj)( PGPKeyIterRef iter,
								   PGPKeyDBObjType objectType, PGPKeyDBObjRef *outRef);
typedef PGPError 	(__cdecl *funcPGPKeyIterPrevKeyDBObj)( PGPKeyIterRef iter,
								   PGPKeyDBObjType objectType, PGPKeyDBObjRef *outRef);
typedef PGPError 	(__cdecl *funcPGPKeyIterGetKeyDBObj)( PGPKeyIterRef iter,
								  PGPKeyDBObjType objectType, PGPKeyDBObjRef *outRef);

/* Change key options which are stored in self signatures internally */
typedef PGPError	(__cdecl *funcPGPAddKeyOptions)( PGPKeyDBObjRef key,
							 PGPOptionListRef firstOption, ...);
typedef PGPError	(__cdecl *funcPGPRemoveKeyOptions)( PGPKeyDBObjRef key,
								PGPOptionListRef firstOption, ...);
typedef PGPError	(__cdecl *funcPGPUpdateKeyOptions)( PGPKeyDBObjRef key,
								PGPOptionListRef firstOption, ...);

/*____________________________________________________________________________
Key IDs
____________________________________________________________________________*/
typedef PGPError 	(__cdecl *funcPGPNewKeyID)( const PGPByte *keyIDBytes, PGPSize numKeyIDBytes,
						PGPPublicKeyAlgorithm pkalg, PGPKeyID *id );
typedef PGPError 	(__cdecl *funcPGPNewKeyIDFromString)( const PGPChar16 *string,
								  PGPPublicKeyAlgorithm pkalg, PGPKeyID *id );
typedef PGPError 	(__cdecl *funcPGPGetKeyIDAlgorithm)( const PGPKeyID *keyID,
								 PGPPublicKeyAlgorithm *pkalg );
typedef PGPError 	(__cdecl *funcPGPGetKeyIDString)( PGPKeyID const * ref, PGPKeyIDStringType type,
							  PGPChar16 outString[ kPGPMaxKeyIDStringSize ] );


typedef PGPError	(__cdecl *funcPGPNewOptionList)( PGPContextRef context, PGPOptionListRef *outList );
typedef PGPError 	(__cdecl *funcPGPAppendOptionList)( PGPOptionListRef optionList,
								PGPOptionListRef firstOption, ... );
typedef PGPError 	(__cdecl *funcPGPBuildOptionList)( PGPContextRef context,
							   PGPOptionListRef *outList,
							   PGPOptionListRef firstOption, ... );
typedef PGPError	(__cdecl *funcPGPCopyOptionList)( PGPOptionListRef optionList,
							  PGPOptionListRef *outList );
typedef PGPError 	(__cdecl *funcPGPFreeOptionList)( PGPOptionListRef optionList );
/*
** Special PGPOptionListRef to mark last option passed to those functions
** which take variable lists of PGPOptionListRef's:
*/

typedef PGPOptionListRef	(__cdecl *funcPGPOLastOption)( PGPContextRef context );

typedef PGPError	(__cdecl *funcPGPWipeFile)( PGPContextRef   	context, 
					 PGPInt32			numPasses,
					 PGPOptionListRef 	firstOption, 
					 ...);

/* Special PGPOptionListRef which is always ignored: */

typedef PGPOptionListRef	(__cdecl *funcPGPONullOption)( PGPContextRef context);

/* Data input (required): */

typedef PGPOptionListRef 	(__cdecl *funcPGPOInputFile)( PGPContextRef context,
								  PGPFileSpecRef fileRef );
typedef PGPOptionListRef 	(__cdecl *funcPGPOInputBuffer)( PGPContextRef context,
									void const *buffer, PGPSize bufferSize );

/* Data output (optional, generates event if missing): */

typedef PGPOptionListRef 	(__cdecl *funcPGPOOutputFile)( PGPContextRef context,
								   PGPFileSpecRef fileRef );
typedef PGPOptionListRef 	(__cdecl *funcPGPOOutputBuffer)( PGPContextRef context,
									 void *buffer, PGPSize bufferSize,
									 PGPSize *outputDataLength );
typedef PGPOptionListRef 	(__cdecl *funcPGPOOutputDirectory)( PGPContextRef context,
										PGPFileSpecRef fileRef );

/* Encrypting and signing */

typedef PGPOptionListRef 		(__cdecl *funcPGPOEncryptToKeyDBObj)( PGPContextRef context,
											  PGPKeyDBObjRef keyDBObjRef);
typedef PGPOptionListRef 		(__cdecl *funcPGPOEncryptToKeySet)( PGPContextRef context,
											PGPKeySetRef keySetRef);
typedef PGPOptionListRef 		(__cdecl *funcPGPOIntegrityProtection)( PGPContextRef context,
												PGPBoolean integrity);
typedef PGPOptionListRef		(__cdecl *funcPGPOObfuscateRecipients)(PGPContextRef context,
												PGPBoolean obfuscate );
typedef PGPOptionListRef 		(__cdecl *funcPGPOSignWithKey)( PGPContextRef context,
										PGPKeyDBObjRef keyDBObjRef,
										PGPOptionListRef firstOption, ...);
typedef PGPOptionListRef 		(__cdecl *funcPGPOConventionalEncrypt)( PGPContextRef context,
												PGPOptionListRef firstOption,
												...);
typedef PGPOptionListRef 		(__cdecl *funcPGPOPassphraseBuffer)( PGPContextRef context,
											 const PGPChar16 *passphrase,
											 PGPSize passphraseLength);
typedef PGPOptionListRef 		(__cdecl *funcPGPOPassphrase)( PGPContextRef context,
									   const PGPChar16 *passphrase);
typedef PGPOptionListRef 		(__cdecl *funcPGPOPasskeyBuffer)( PGPContextRef context,
										  const void *passkey, PGPSize passkeyLength);
typedef PGPOptionListRef 		(__cdecl *funcPGPODetachedSig)( PGPContextRef context,
										PGPOptionListRef firstOption,
										...);

typedef PGPOptionListRef 		(__cdecl *funcPGPOCipherAlgorithm)( PGPContextRef context,
											PGPCipherAlgorithm algorithm);
typedef PGPOptionListRef 		(__cdecl *funcPGPOHashAlgorithm)( PGPContextRef context,
										  PGPHashAlgorithm algorithm);
typedef PGPOptionListRef 		(__cdecl *funcPGPOCompressionAlgorithm)( PGPContextRef context,
												 PGPCompressionAlgorithm algorithm);

typedef PGPOptionListRef 		(__cdecl *funcPGPOEventHandler)( PGPContextRef context,
										 PGPEventHandlerProcPtr eventHandler,
										 PGPUserValue eventHandlerData);
typedef PGPOptionListRef 		(__cdecl *funcPGPOSendNullEvents)( PGPContextRef context,
										   PGPTimeInterval approxInterval);

typedef PGPOptionListRef 		(__cdecl *funcPGPOArmorOutput)( PGPContextRef context,
										PGPBoolean armorOutput );
typedef PGPOptionListRef 		(__cdecl *funcPGPODataIsASCII)( PGPContextRef context,
										PGPBoolean dataIsASCII );
typedef PGPOptionListRef 		(__cdecl *funcPGPOClearSign)( PGPContextRef context,
									  PGPBoolean clearSign );
typedef PGPOptionListRef 		(__cdecl *funcPGPOForYourEyesOnly)( PGPContextRef context,
											PGPBoolean forYourEyesOnly );
typedef PGPOptionListRef 		(__cdecl *funcPGPOKeyDBRef)( PGPContextRef context,
									 PGPKeyDBRef keydbRef);

typedef PGPOptionListRef 		(__cdecl *funcPGPOExportKeySet)( PGPContextRef context,
										 PGPKeySetRef keysetRef);
typedef PGPOptionListRef 		(__cdecl *funcPGPOExportKeyDBObj)( PGPContextRef context,
										   PGPKeyDBObjRef keyDBObjRef);
typedef PGPOptionListRef 		(__cdecl *funcPGPOImportKeysTo)( PGPContextRef context,
										 PGPKeyDBRef keydbRef);
typedef PGPOptionListRef		(__cdecl *funcPGPOKeyGenParams)( PGPContextRef context,
										 PGPPublicKeyAlgorithm pubKeyAlg,
										 PGPUInt32 bits);

typedef PGPOptionListRef		(__cdecl *funcPGPOKeyGenName)( PGPContextRef context,
									   const void *name, PGPSize nameLength);

typedef PGPOptionListRef		(__cdecl *funcPGPOCreationDate)( PGPContextRef context,
										 PGPTime creationDate);
typedef PGPOptionListRef		(__cdecl *funcPGPOExpiration)( PGPContextRef context,
									   PGPUInt32 expirationDays);

/* '*buffer' must be disposed of via PGPFreeData() */
/* maximum memory usage will be no more than maximumBufferSize */
typedef PGPOptionListRef 	(__cdecl *funcPGPOAllocatedOutputBuffer)(PGPContextRef context,
											  void **buffer, PGPSize maximumBufferSize,
											  PGPSize *actualBufferSize);
typedef PGPOptionListRef 	(__cdecl *funcPGPOAppendOutput)( PGPContextRef context,
									 PGPBoolean appendOutput );
typedef PGPOptionListRef 	(__cdecl *funcPGPODiscardOutput)( PGPContextRef context,
									  PGPBoolean discardOutput );
typedef PGPOptionListRef    (__cdecl *funcPGPOAllocatedOutputKeyContainer)(PGPContextRef context,
													void **keyContName, PGPSize maximumKeyContNameSize, 
													PGPSize *actualKeyContNameSize );

typedef PGPOptionListRef	(__cdecl *funcPGPOKeyGenUseExistingEntropy)( PGPContextRef context,
													 PGPBoolean useExistingEntropy);

typedef PGPOptionListRef	(__cdecl *funcPGPOKeyGenFast)( PGPContextRef context,
										PGPBoolean fastGen);

typedef PGPOptionListRef	(__cdecl *funcPGPOKeyFlags)( PGPContextRef context,
										PGPUInt32 flags);
typedef PGPOptionListRef	(__cdecl *funcPGPOKeyGenMasterKey)( PGPContextRef context,
											PGPKeyDBObjRef masterKeyDBObjRef);

/* allocate a block of the specified size */
typedef void *  	(__cdecl *funcPGPNewData)( PGPMemoryMgrRef mgr,
					   PGPSize requestSize, PGPMemoryMgrFlags flags );

/* allocate a block of the specified size in non-pageable memory */
/* *isSecure is TRUE if the block definitely can't be paged */
typedef void *  	(__cdecl *funcPGPNewSecureData)( PGPMemoryMgrRef mgr,
							 PGPSize requestSize, PGPMemoryMgrFlags flags );

/* properly reallocs secure or non-secure blocks */
/* WARNING: the block may move, even if its size is being reduced */
typedef PGPError  	(__cdecl *funcPGPReallocData)( PGPMemoryMgrRef mgr,
						   void **allocation, PGPSize newAllocationSize,
						   PGPMemoryMgrFlags flags );

/* properly frees secure or non-secure blocks */
typedef PGPError 	(__cdecl *funcPGPFreeData)( void *allocation );

typedef PGPError 	(__cdecl *funcPGPGetErrorString)( PGPError theError,
							  PGPSize bufferSize, PGPChar8 * theString );

typedef PGPError 	(__cdecl *funcPGPCheckKeyRingSigs)( PGPKeySetRef keysToCheck,
								PGPKeyDBRef optionalSigningKeyDB, PGPBoolean checkAll,
								PGPEventHandlerProcPtr eventHandler,
								PGPUserValue eventHandlerData );

typedef PGPOptionListRef		(__cdecl *funcPGPOExportPrivateKeys)( PGPContextRef context,
											  PGPBoolean exportKeys);

typedef PGPOptionListRef		(__cdecl *funcPGPOExportPrivateSubkeys)( PGPContextRef context,
												 PGPBoolean exportSubkeys);
typedef PGPOptionListRef		(__cdecl *funcPGPOVersionString)( PGPContextRef context,
										  PGPChar16 const *version);
typedef PGPOptionListRef		(__cdecl *funcPGPOExportFormat)(PGPContextRef context,
										  PGPExportFormat exportFormat);
typedef PGPOptionListRef		(__cdecl *funcPGPOInputFormat)( PGPContextRef context,
										  PGPInputFormat inputFormat );
typedef PGPOptionListRef		(__cdecl *funcPGPOPreferredCompressionAlgorithms)(
											PGPContextRef context, 
											PGPCompressionAlgorithm const *prefAlg,
											PGPUInt32 numAlgs);
typedef PGPOptionListRef		(__cdecl *funcPGPOPreferredAlgorithms)(
											PGPContextRef context, 
											PGPCipherAlgorithm const *prefAlg,
											PGPUInt32 numAlgs);
typedef PGPOptionListRef		(__cdecl *funcPGPOPreferredHashAlgorithms)(
											PGPContextRef context, 
											PGPHashAlgorithm const *prefAlg,
											PGPUInt32 numAlgs);

typedef PGPError (__cdecl *funcPGPsdkSetLanguage)( PGPFileSpecRef langStringsHome, PGPLanguage lang );

typedef PGPOptionListRef		(__cdecl *funcPGPOOutputFormat)( PGPContextRef context,
											PGPOutputFormat outputFormat );
typedef PGPMemoryMgrRef	(__cdecl *funcPGPGetDefaultMemoryMgr)(void);
typedef PGPKeyDBObjRef	(__cdecl *funcPGPPeekKeyDBObjKey)( PGPKeyDBObjRef ref );
typedef PGPKeyDBRef		(__cdecl *funcPGPPeekKeyDBObjKeyDB)( PGPKeyDBObjRef ref );
typedef PGPInt32		(__cdecl *funcPGPCompareKeyIDs)(PGPKeyID const *key1, PGPKeyID const *key2);
typedef PGPError		(__cdecl *funcPGPAddJobOptions)(PGPJobRef	job, PGPOptionListRef firstOption, ...);
typedef PGPOptionListRef (__cdecl *funcPGPOCharsetString)(PGPContextRef	context, PGPChar8 const	*charset);
typedef PGPOptionListRef (__cdecl *funcPGPORawPGPInput)(PGPContextRef context, PGPBoolean rawPGPInput);
typedef PGPOptionListRef (__cdecl *funcPGPOCachePassphrase)(PGPContextRef	context, 
															PGPUInt32		timeOutSeconds,
															PGPBoolean		globalCache );
typedef PGPOptionListRef (__cdecl *funcPGPOPassThroughIfUnrecognized)(
								PGPContextRef	context,
								PGPBoolean		passThroughIfUnrecognized);
typedef PGPOptionListRef (__cdecl *funcPGPOPassThroughKeys)(
								PGPContextRef	context,
								PGPBoolean		passThroughKeys);

typedef PGPMemoryMgrRef (__cdecl *funcPGPPeekContextMemoryMgr)( PGPContextRef context ); 

/************************************************************************/
/* Key Server related functions                                         */
/************************************************************************/
typedef PGPError (__cdecl *funcPGPsdkNetworkLibInit)(PGPFlags options);

typedef PGPError (__cdecl *funcPGPsdkNetworkLibCleanup)(void);

typedef PGPError (__cdecl *funcPGPKeyServerInit)();

typedef PGPError (__cdecl *funcPGPKeyServerCleanup)();

typedef PGPError
(__cdecl *funcPGPNewKeyServer)(
				PGPContextRef 			context,
				PGPKeyServerClass		serverClass,
				PGPKeyServerRef 		*outKeyServerRef,
				PGPOptionListRef		firstOption,
				... );
typedef PGPError
(__cdecl *funcPGPFreeKeyServer)(
				 PGPKeyServerRef	inKeyServerRef);
typedef PGPError
(__cdecl *funcPGPSetKeyServerEventHandler)(
							PGPKeyServerRef			inKeyServerRef,
							PGPEventHandlerProcPtr	inCallback,
							PGPUserValue			inUserData);
typedef PGPError
(__cdecl *funcPGPNewTLSContext)(
				 PGPContextRef		context,
				 PGPtlsContextRef *	outRef );
typedef PGPError
(__cdecl *funcPGPFreeTLSContext)(
				  PGPtlsContextRef	ref );
typedef PGPError
(__cdecl *funcPGPNewTLSSession)(
				 PGPtlsContextRef		ref,
				 PGPtlsSessionRef *		outRef );
typedef PGPError
(__cdecl *funcPGPCopyTLSSession)( PGPtlsSessionRef ref, PGPtlsSessionRef *outRef );

typedef PGPError (__cdecl *funcPGPFreeTLSSession)( PGPtlsSessionRef ref );

typedef PGPError
(__cdecl *funcPGPKeyServerOpen)(
				 PGPKeyServerRef		inKeyServerRef,
				 PGPtlsSessionRef	inTLSSessionRef);
typedef PGPError
(__cdecl *funcPGPKeyServerClose)(
				  PGPKeyServerRef	inKeyServerRef);

typedef PGPError
(__cdecl *funcPGPQueryKeyServer)(
				  PGPKeyServerRef	inKeyServerRef, 
				  PGPFilterRef	inFilterRef, 
				  PGPKeyDBRef *	outFoundKeys);
typedef PGPError
(__cdecl *funcPGPUploadToKeyServer)(
					 PGPKeyServerRef	inKeyServerRef, 
					 PGPKeySetRef	inKeysToUpload, 
					 PGPKeySetRef *	outKeysThatFailed);
typedef PGPError
(__cdecl *funcPGPDeleteFromKeyServer)(
					   PGPKeyServerRef	inKeyServerRef,
					   PGPKeySetRef	inKeysToDelete,
					   PGPKeySetRef *	outKeysThatFailed);
typedef PGPError
(__cdecl *funcPGPDisableFromKeyServer)(
						PGPKeyServerRef	inKeyServerRef,
						PGPKeySetRef	inKeysToDisable,
						PGPKeySetRef *	outKeysThatFailed);

typedef PGPOptionListRef	(__cdecl *funcPGPONetURL)(PGPContextRef context, const PGPChar16 *url);

typedef PGPOptionListRef	(__cdecl *funcPGPONetHostName)(PGPContextRef context,
									const PGPChar16 *hostName, PGPUInt16 port);

typedef PGPOptionListRef	(__cdecl *funcPGPONetHostAddress)(PGPContextRef context,
									   PGPUInt32 hostAddress, PGPUInt16 port);

typedef PGPOptionListRef	(__cdecl *funcPGPONetConnectTimeout)(PGPContextRef context,
										  PGPUInt32 timeout);

typedef PGPOptionListRef	(__cdecl *funcPGPONetReadTimeout)(PGPContextRef context,
									   PGPUInt32 timeout);

typedef PGPOptionListRef	(__cdecl *funcPGPONetWriteTimeout)(PGPContextRef context,
										PGPUInt32 timeout);

typedef PGPOptionListRef	(__cdecl *funcPGPOKeyServerProtocol)(PGPContextRef context,
										  PGPKeyServerProtocol serverProtocol);

typedef PGPOptionListRef	(__cdecl *funcPGPOKeyServerKeySpace)(PGPContextRef context,
										  PGPKeyServerKeySpace serverSpace);

typedef PGPOptionListRef	(__cdecl *funcPGPOKeyServerKeyStoreDN)(PGPContextRef context,
											const PGPChar16 *szKeyStoreDn);

typedef PGPOptionListRef	(__cdecl *funcPGPOKeyServerAccessType)(PGPContextRef context,
											PGPKeyServerAccessType accessType);

typedef PGPOptionListRef	(__cdecl *funcPGPOKeyServerCAKey)(PGPContextRef context,
									   PGPKeyDBObjRef caKeyDBObjRef);

typedef PGPOptionListRef	(__cdecl *funcPGPOKeyServerRequestKey)(PGPContextRef context,
											PGPKeyDBObjRef requestKeyDBObjRef);

typedef PGPOptionListRef	(__cdecl *funcPGPOKeyServerSearchKey)(PGPContextRef context,
										   PGPKeyDBObjRef searchKeyDBObjRef);

typedef PGPOptionListRef	(__cdecl *funcPGPOKeyServerSearchFilter)(PGPContextRef context,
											  PGPFilterRef searchFilter);

typedef PGPError			(__cdecl *funcPGPIncKeyServerRefCount)(PGPKeyServerRef inKeyServerRef);

/*	Get keyserver info. */
typedef PGPError			(__cdecl *funcPGPGetKeyServerTLSSession)(PGPKeyServerRef inKeyServerRef,
											  PGPtlsSessionRef * outTLSSessionRef);

typedef PGPError			(__cdecl *funcPGPGetKeyServerProtocol)(PGPKeyServerRef inKeyServerRef,
											PGPKeyServerProtocol * outType);

typedef PGPError			(__cdecl *funcPGPGetKeyServerAccessType)(PGPKeyServerRef inKeyServerRef,
											  PGPKeyServerAccessType * outAccessType);

typedef PGPError			(__cdecl *funcPGPGetKeyServerKeySpace)(PGPKeyServerRef inKeyServerRef,
											PGPKeyServerKeySpace * outKeySpace);

typedef PGPError			(__cdecl *funcPGPGetKeyServerPort)(PGPKeyServerRef inKeyServerRef,
										PGPUInt16 * outPort);

typedef PGPError			(__cdecl *funcPGPGetKeyServerHostName)(PGPKeyServerRef inKeyServerRef,
											PGPChar16 ** outHostName); /* Use PGPFreeData to free */

typedef PGPError			(__cdecl *funcPGPGetKeyServerAddress)(PGPKeyServerRef inKeyServerRef,
										   PGPUInt32 * outAddress);

typedef PGPError			(__cdecl *funcPGPGetKeyServerPath)(PGPKeyServerRef inKeyServerRef,
										PGPChar16 ** outPath); /* Use PGPFreeData to free */

typedef PGPError			(__cdecl *funcPGPGetKeyServerVerificationKeyID)(PGPKeyServerRef inKeyServerRef,
													 PGPKeyID *	outKeyID);

typedef PGPContextRef		(__cdecl *funcPGPGetKeyServerContext)(PGPKeyServerRef inKeyServerRef);

/*	If there was an error string returned from the server, you can get it with
this function. Note that if there is no string, the function will return
kPGPError_NoErr and *outErrorString will be	NULL */
typedef PGPError			(__cdecl *funcPGPGetLastKeyServerErrorString)(
	PGPKeyServerRef inKeyServerRef,
	PGPChar16 ** outErrorString); /* Use PGPFreeData to free */

/* X.509 Certificate Request functions */
typedef PGPError			(__cdecl *funcPGPSendCertificateRequest)( 
	PGPKeyServerRef 	inKeyServerRef,
	PGPOptionListRef	firstOption,
	... );

typedef PGPError			(__cdecl *funcPGPRetrieveCertificate)( 
	PGPKeyServerRef 	inKeyServerRef,
	PGPOptionListRef	firstOption,
	... );

typedef PGPError			(__cdecl *funcPGPRetrieveCertificateRevocationList)( 
	PGPKeyServerRef 	inKeyServerRef,
	PGPOptionListRef	firstOption,
	... );
/* Queries HTTP proxy information. proxyAddress must be freed with PGPFreeData */
typedef PGPError 			(__cdecl *funcPGPGetProxyServer)(
									  PGPContextRef context, PGPProxyServerType type,
									  PGPChar16 **proxyAddress, PGPUInt16 *proxyPort );
typedef PGPError 			(__cdecl *funcPGPGetProxyHost)(
									PGPContextRef context, PGPProxyServerType type,
									const PGPChar16 *destHost, PGPUInt16 destPort,
									PGPChar16 **proxyAddress, PGPUInt16 *proxyPort,
									PGPChar16 **proxyUserName, PGPChar16 **proxyPassword );

typedef PGPError			(__cdecl *funcPGPMakeProxyAuthString)(
	PGPContextRef	context,
	const PGPByte	*responseHeader,
	const PGPChar16   *proxyUserName,
	const PGPChar16   *proxyPassword,
	const PGPChar16   *uri,
	PGPByte		*authString,
	PGPSize		authStringSize);

typedef PGPError			(__cdecl *funcPGPRetrieveOCSPStatus)(
	PGPSocketRef s,
	const PGPChar16  *httpUrl,
	const PGPChar16  *proxyHost,
	const PGPChar16  *proxyUserName,
	const PGPChar16  *proxyPassword,
	PGPKeyDBObjRef   requestCertificate,
	PGPKeyDBObjRef   issuerCertificate,
	PGPKeyDBObjRef	signerKey,
	PGPOCSPStatus *	status,
	PGPOptionListRef		firstOption, ...);

typedef PGPError (__cdecl *funcPGPX509GetCertificationIssuer)( PGPKeyDBObjRef certificate,
									   const PGPChar8  *proxyHost,
									   const PGPChar8  *proxyUserName,
									   const PGPChar8  *proxyPassword,
									   PGPKeyDBRef	   *keyDBOut);

typedef PGPError (__cdecl *funcPGPKeyServerCreateThreadStorage)(
								PGPKeyServerThreadStorageRef *outPreviousStorage);
typedef PGPError (__cdecl *funcPGPKeyServerDisposeThreadStorage)(
								PGPKeyServerThreadStorageRef	inPreviousStorage);

// SDA
typedef PGPError (__cdecl *funcPGPNewSDAContext)(
				 PGPContextRef				context,
				 PGPsdaContextRef *			pSDAContext);

typedef PGPError (__cdecl *funcPGPFreeSDAContext)(
				  PGPsdaContextRef			sdaContext);

typedef PGPError (__cdecl *funcPGPsdaImportObject)(
				   PGPsdaContextRef			sdaContext,
				   PGPChar *					object,
				   PGPChar *					rootPath,
				   PGPBoolean					bRecursive);

typedef PGPError (__cdecl *funcPGPsdaSetAutoLaunchObject)(
						  PGPsdaContextRef			sdaContext,
						  PGPChar *					object,
						  PGPUInt8					flags);

typedef PGPError (__cdecl *funcPGPsdaCreate)(
			 PGPsdaContextRef			sdaContext,
			 PGPCipherAlgorithm			cipherAlgorithm,
			 PGPHashAlgorithm			hashAlgorithm,
			 PGPCompressionAlgorithm	compressionAlgorithm,
			 PGPsdaCompressionLevel		compressionLevel,
			 PGPsdaTargetPlatform		targetPlatform,
			 PGPBoolean					bStripDirectories,
			 PGPKeySetRef				adkKeySet,
			 PGPChar *					passphrase,
			 PGPChar *					outputFilename,
			 PGPsdaEventHandlerProcPtr	handler,
			 PGPUserValue				userValue);

typedef PGPError (__cdecl *funcPGPsdaDecrypt)(
			  PGPChar *					sda,
			  PGPChar8 *					passphrase8,
			  PGPChar *					outputDirectory,
			  PGPBoolean					bStripDirectories,
			  PGPBoolean					bListOnly,
			  PGPsdaEventHandlerProcPtr	sdaEventHandlerFunction,
			  PGPUserValue				sdaEventHandlerUserValue);

typedef PGPError (__cdecl *funcPGPsdaVerify)(
			 PGPChar *					sda);


class CPgpSdkFunc
{
public:
	CPgpSdkFunc()
	{
		m_bValid                         = 0;

		fPGPsdkInit = 0;
		fPGPsdkCleanup = 0;
		fPGPNewContext = 0;
		fPGPNewContextCustom = 0;
		fPGPFreeContext = 0;

		fPGPEncode = 0;
		fPGPDecode = 0;

		fPGPGetPGPsdkVersionString = 0;
		fPGPGetFeatureFlags = 0;
		fPGPGlobalRandomPoolGetEntropy = 0;
		fPGPGlobalRandomPoolGetSize = 0;
		fPGPGlobalRandomPoolGetMinimumEntropy = 0;
		fPGPGlobalRandomPoolHasMinimumEntropy = 0;
		fPGPSetRandSeedFile = 0;
		fPGPGlobalRandomPoolAddKeystroke = 0;
		fPGPGlobalRandomPoolMouseMoved = 0;
		fPGPGlobalRandomPoolAddSystemState = 0;
		fPGPContextReserveRandomBytes = 0;

		fPGPCountPublicKeyAlgorithms = 0;
		fPGPGetIndexedPublicKeyAlgorithmInfo = 0;
		fPGPCountSymmetricCiphers = 0;
		fPGPGetIndexedSymmetricCipherInfo = 0;

		fPGPNewFileSpecFromFullPath = 0;
		fPGPGetFullPathFromFileSpec = 0;
		fPGPCopyFileSpec = 0;
		fPGPFreeFileSpec = 0;
		fPGPRenameFile = 0;
		fPGPDeleteFile = 0;
		fPGPWipeFile = 0;

		fPGPNewKeyDB = 0;
		fPGPOpenKeyDBFile = 0;
		fPGPFreeKeyDB = 0;
		fPGPFlushKeyDB = 0;
		fPGPIncKeyDBRefCount = 0;
		fPGPKeyDBIsMutable = 0;
		fPGPFindKeyByKeyID = 0;
		fPGPFindKeyByV3orV4KeyID = 0;
		fPGPCountKeysInKeyDB = 0;
		fPGPKeyDBIsUpdated = 0;

		fPGPCacheKeyDB = 0;
		fPGPPurgeKeyDBCache = 0;
		fPGPNewKeySet = 0;
		fPGPNewEmptyKeySet = 0;
		fPGPNewOneKeySet = 0;
		fPGPNewEmptyInclusiveKeySet = 0;
		fPGPNewOneInclusiveKeySet = 0;
		fPGPFreeKeySet = 0;
		fPGPIncKeySetRefCount = 0;
		fPGPKeySetIsMember = 0;
		fPGPCountKeys = 0;
		fPGPAddKey = 0;
		fPGPAddKeys = 0;
		fPGPPeekKeySetKeyDB = 0;
		fPGPPeekKeyDBRootKeySet = 0;

		fPGPGetKeyDBObjBooleanProperty = 0;
		fPGPGetKeyDBObjNumericProperty = 0;
		fPGPGetKeyDBObjTimeProperty = 0;
		fPGPGetKeyDBObjDataProperty = 0;
		fPGPGetKeyDBObjAllocatedDataProperty = 0;
		fPGPSetKeyEnabled = 0;
		fPGPSetKeyAxiomatic = 0;

		fPGPGetKeyID = 0;
		fPGPGetPrimaryUserID = 0;
		fPGPGetPrimaryAttributeUserID = 0;
		fPGPGetPrimaryUserIDValidity = 0;
		fPGPGetPrimaryUserIDName = 0;
		fPGPGetKeyForUsage = 0;

		fPGPNewKeyDBObjBooleanFilter = 0;
		fPGPNewKeyDBObjNumericFilter = 0;
		fPGPNewKeyDBObjTimeFilter = 0;
		fPGPNewKeyDBObjDataFilter = 0;
		fPGPFreeFilter = 0;
		fPGPIncFilterRefCount = 0;
		fPGPFilterChildObjects = 0;
		fPGPNegateFilter = 0;
		fPGPIntersectFilters = 0;
		fPGPUnionFilters = 0;
		fPGPFilterKeySet = 0;
		fPGPFilterKeyDB = 0;

		fPGPGenerateKey = 0;
		fPGPGenerateSubKey = 0;
		fPGPGetKeyEntropyNeeded = 0;
		fPGPCopyKeyDBObj = 0;

		fPGPCopyKeys = 0;
		fPGPDeleteKeyDBObj = 0;
		fPGPDeleteKeys = 0;

		fPGPExport = 0;
		fPGPImport = 0;
		fPGPRevokeSig = 0;
		fPGPRevoke = 0;
		fPGPChangePassphrase = 0;
		fPGPPassphraseIsValid = 0;

		fPGPOrderKeySet = 0;
		fPGPIncKeyListRefCount = 0;
		fPGPFreeKeyList = 0;

		fPGPNewKeyIter = 0;
		fPGPNewKeyIterFromKeySet = 0;
		fPGPNewKeyIterFromKeyDB = 0;
		fPGPCopyKeyIter = 0;
		fPGPFreeKeyIter = 0;
		fPGPKeyIterIndex = 0;
		fPGPKeyIterRewind = 0;
		fPGPKeyIterSeek = 0;
		fPGPKeyIterMove = 0;
		fPGPKeyIterNextKeyDBObj = 0;
		fPGPKeyIterPrevKeyDBObj = 0;
		fPGPKeyIterGetKeyDBObj = 0;

		fPGPAddKeyOptions = 0;
		fPGPRemoveKeyOptions = 0;
		fPGPUpdateKeyOptions = 0;

		fPGPNewKeyID = 0;
		fPGPNewKeyIDFromString = 0;
		fPGPGetKeyIDAlgorithm = 0;
		fPGPGetKeyIDString = 0;

		fPGPNewOptionList = 0;
		fPGPAppendOptionList = 0;
		fPGPBuildOptionList = 0;
		fPGPCopyOptionList = 0;
		fPGPFreeOptionList = 0;
		fPGPOLastOption = 0;

		fPGPONullOption = 0;

		fPGPOInputFile = 0;
		fPGPOInputBuffer = 0;

		fPGPOOutputFile = 0;
		fPGPOOutputBuffer = 0;
		fPGPOOutputDirectory = 0;

		fPGPOEncryptToKeyDBObj = 0;
		fPGPOEncryptToKeySet = 0;
		fPGPOIntegrityProtection = 0;
		fPGPOObfuscateRecipients = 0;
		fPGPOSignWithKey = 0;
		fPGPOConventionalEncrypt = 0;
		fPGPOPassphraseBuffer = 0;
		fPGPOPassphrase = 0;
		fPGPOPasskeyBuffer = 0;
		fPGPODetachedSig = 0;

		fPGPOCipherAlgorithm = 0;
		fPGPOHashAlgorithm = 0;
		fPGPOCompressionAlgorithm = 0;

		fPGPOEventHandler = 0;
		fPGPOSendNullEvents = 0;

		fPGPOArmorOutput = 0;
		fPGPODataIsASCII = 0;
		fPGPOClearSign = 0;
		fPGPOKeyDBRef = 0;

		fPGPOExportKeySet = 0;
		fPGPOExportKeyDBObj = 0;
		fPGPOImportKeysTo = 0;
		fPGPOKeyGenParams = 0;

		fPGPOKeyGenName = 0;
		fPGPOCreationDate = 0;
		fPGPOExpiration = 0;

		fPGPOAllocatedOutputBuffer = 0;
		fPGPOAppendOutput = 0;
		fPGPODiscardOutput = 0;
		fPGPOAllocatedOutputKeyContainer = 0;
		fPGPOKeyGenUseExistingEntropy = 0;
		fPGPOKeyGenFast = 0;
		fPGPOKeyFlags = 0;
		fPGPOKeyGenMasterKey = 0;

		fPGPNewData = 0;
		fPGPNewSecureData = 0;
		fPGPReallocData = 0;
		fPGPFreeData = 0;

		fPGPGetErrorString = 0;
		fPGPCheckKeyRingSigs = 0;
		fPGPOExportPrivateKeys = 0;
		fPGPOExportPrivateSubkeys = 0;
		fPGPOVersionString = 0;
		fPGPOExportFormat = 0;
		fPGPOInputFormat = 0;
		fPGPOPreferredCompressionAlgorithms = 0;
		fPGPOPreferredAlgorithms = 0;
		fPGPOPreferredHashAlgorithms = 0;
		fPGPsdkSetLanguage = 0;
		fPGPOOutputFormat = 0;
		fPGPGetDefaultMemoryMgr = 0;
		fPGPPeekKeyDBObjKey = 0;
		fPGPPeekKeyDBObjKeyDB = 0;
		fPGPCompareKeyIDs = 0;
		fPGPAddJobOptions = 0;
		fPGPOCharsetString = 0;
		fPGPORawPGPInput = 0;
		fPGPOCachePassphrase = 0;
		fPGPOPassThroughIfUnrecognized = 0;
		fPGPOPassThroughKeys = 0;
		fPGPPeekContextMemoryMgr = 0;

		fPGPsdkNetworkLibInit = 0;
		fPGPsdkNetworkLibCleanup = 0;
		fPGPKeyServerInit = 0;
		fPGPKeyServerCleanup = 0;
		fPGPNewKeyServer = 0;
		fPGPFreeKeyServer = 0;
		fPGPSetKeyServerEventHandler = 0;
		fPGPNewTLSContext = 0;
		fPGPFreeTLSContext = 0;
		fPGPNewTLSSession = 0;
		fPGPCopyTLSSession = 0;
		fPGPFreeTLSSession = 0;

		fPGPKeyServerOpen = 0;
		fPGPKeyServerClose = 0;

		fPGPQueryKeyServer = 0;
		fPGPUploadToKeyServer = 0;
		fPGPDeleteFromKeyServer = 0;
		fPGPDisableFromKeyServer = 0;

		fPGPONetURL = 0;
		fPGPONetHostName = 0;
		fPGPONetHostAddress = 0;
		fPGPONetConnectTimeout = 0;
		fPGPONetReadTimeout = 0;
		fPGPONetWriteTimeout = 0;
		fPGPOKeyServerProtocol = 0;
		fPGPOKeyServerKeySpace = 0;
		fPGPOKeyServerKeyStoreDN = 0;
		fPGPOKeyServerAccessType = 0;
		fPGPOKeyServerCAKey = 0;
		fPGPOKeyServerRequestKey = 0;
		fPGPOKeyServerSearchKey = 0;
		fPGPOKeyServerSearchFilter = 0;
		fPGPIncKeyServerRefCount = 0;

		/*	Get keyserver info. */
		fPGPGetKeyServerTLSSession = 0;
		fPGPGetKeyServerProtocol = 0;
		fPGPGetKeyServerAccessType = 0;
		fPGPGetKeyServerKeySpace = 0;
		fPGPGetKeyServerPort = 0;
		fPGPGetKeyServerHostName = 0; /* Use PGPFreeData to free */
		fPGPGetKeyServerAddress = 0;
		fPGPGetKeyServerPath = 0; /* Use PGPFreeData to free */
		fPGPGetKeyServerVerificationKeyID = 0;
		fPGPGetKeyServerContext = 0;

		fPGPGetLastKeyServerErrorString = 0; /* Use PGPFreeData to free */

		/* X.509 Certificate Request functions */
		fPGPSendCertificateRequest = 0;
		fPGPRetrieveCertificate = 0;
		fPGPRetrieveCertificateRevocationList = 0;
		fPGPGetProxyServer = 0;
		fPGPGetProxyHost = 0;
		fPGPMakeProxyAuthString = 0;

		fPGPRetrieveOCSPStatus = 0;

		fPGPKeyServerCreateThreadStorage = 0;
		fPGPKeyServerDisposeThreadStorage = 0;

#if 0
		fPGPNewSDAContext = 0;
		fPGPFreeSDAContext = 0;
		fPGPsdaImportObject = 0;
		fPGPsdaSetAutoLaunchObject = 0;
		fPGPsdaCreate = 0;
		fPGPsdaDecrypt = 0;
		fPGPsdaVerify = 0;
#endif
	}

	virtual ~CPgpSdkFunc()
	{
		m_bValid = 0;
	}

	int InitializePgpSdk();

public:
	funcPGPsdkInit fPGPsdkInit;
	funcPGPsdkCleanup fPGPsdkCleanup;
	funcPGPNewContext fPGPNewContext;
	funcPGPNewContextCustom fPGPNewContextCustom;
	funcPGPFreeContext fPGPFreeContext;

	funcPGPEncode fPGPEncode;
	funcPGPDecode fPGPDecode;

	funcPGPGetPGPsdkVersionString fPGPGetPGPsdkVersionString;
	funcPGPGetFeatureFlags fPGPGetFeatureFlags;
	funcPGPGlobalRandomPoolGetEntropy fPGPGlobalRandomPoolGetEntropy;
	funcPGPGlobalRandomPoolGetSize fPGPGlobalRandomPoolGetSize;
	funcPGPGlobalRandomPoolGetMinimumEntropy fPGPGlobalRandomPoolGetMinimumEntropy;
	funcPGPGlobalRandomPoolHasMinimumEntropy fPGPGlobalRandomPoolHasMinimumEntropy;
	funcPGPSetRandSeedFile fPGPSetRandSeedFile;
	funcPGPGlobalRandomPoolAddKeystroke fPGPGlobalRandomPoolAddKeystroke;
	funcPGPGlobalRandomPoolMouseMoved fPGPGlobalRandomPoolMouseMoved;
	funcPGPGlobalRandomPoolAddSystemState fPGPGlobalRandomPoolAddSystemState;
	funcPGPContextReserveRandomBytes fPGPContextReserveRandomBytes;

	funcPGPCountPublicKeyAlgorithms fPGPCountPublicKeyAlgorithms;
	funcPGPGetIndexedPublicKeyAlgorithmInfo fPGPGetIndexedPublicKeyAlgorithmInfo;
	funcPGPCountSymmetricCiphers fPGPCountSymmetricCiphers;
	funcPGPGetIndexedSymmetricCipherInfo fPGPGetIndexedSymmetricCipherInfo;

	funcPGPNewFileSpecFromFullPath fPGPNewFileSpecFromFullPath;
	funcPGPGetFullPathFromFileSpec fPGPGetFullPathFromFileSpec;
	funcPGPCopyFileSpec fPGPCopyFileSpec;
	funcPGPFreeFileSpec fPGPFreeFileSpec;
	funcPGPRenameFile fPGPRenameFile;
	funcPGPDeleteFile fPGPDeleteFile;
	funcPGPWipeFile	fPGPWipeFile;

	funcPGPNewKeyDB fPGPNewKeyDB;
	funcPGPOpenKeyDBFile fPGPOpenKeyDBFile;
	funcPGPFreeKeyDB fPGPFreeKeyDB;
	funcPGPFlushKeyDB fPGPFlushKeyDB;
	funcPGPIncKeyDBRefCount fPGPIncKeyDBRefCount;
	funcPGPKeyDBIsMutable fPGPKeyDBIsMutable;
	funcPGPFindKeyByKeyID fPGPFindKeyByKeyID;
	funcPGPFindKeyByV3orV4KeyID fPGPFindKeyByV3orV4KeyID;
	funcPGPCountKeysInKeyDB fPGPCountKeysInKeyDB;
	funcPGPKeyDBIsUpdated fPGPKeyDBIsUpdated;

	funcPGPCacheKeyDB fPGPCacheKeyDB;
	funcPGPPurgeKeyDBCache fPGPPurgeKeyDBCache;
	funcPGPNewKeySet fPGPNewKeySet;
	funcPGPNewEmptyKeySet fPGPNewEmptyKeySet;
	funcPGPNewOneKeySet fPGPNewOneKeySet;
	funcPGPNewEmptyInclusiveKeySet fPGPNewEmptyInclusiveKeySet;
	funcPGPNewOneInclusiveKeySet fPGPNewOneInclusiveKeySet;
	funcPGPFreeKeySet fPGPFreeKeySet;
	funcPGPIncKeySetRefCount fPGPIncKeySetRefCount;
	funcPGPKeySetIsMember fPGPKeySetIsMember;
	funcPGPCountKeys fPGPCountKeys;
	funcPGPAddKey fPGPAddKey;
	funcPGPAddKeys fPGPAddKeys;
	funcPGPPeekKeySetKeyDB fPGPPeekKeySetKeyDB;
	funcPGPPeekKeyDBRootKeySet fPGPPeekKeyDBRootKeySet;

	funcPGPGetKeyDBObjBooleanProperty fPGPGetKeyDBObjBooleanProperty;
	funcPGPGetKeyDBObjNumericProperty fPGPGetKeyDBObjNumericProperty;
	funcPGPGetKeyDBObjTimeProperty fPGPGetKeyDBObjTimeProperty;
	funcPGPGetKeyDBObjDataProperty fPGPGetKeyDBObjDataProperty;
	funcPGPGetKeyDBObjAllocatedDataProperty fPGPGetKeyDBObjAllocatedDataProperty;
	funcPGPSetKeyEnabled fPGPSetKeyEnabled;
	funcPGPSetKeyAxiomatic fPGPSetKeyAxiomatic;

	funcPGPGetKeyID fPGPGetKeyID;
	funcPGPGetPrimaryUserID fPGPGetPrimaryUserID;
	funcPGPGetPrimaryAttributeUserID fPGPGetPrimaryAttributeUserID;
	funcPGPGetPrimaryUserIDValidity fPGPGetPrimaryUserIDValidity;
	funcPGPGetPrimaryUserIDName fPGPGetPrimaryUserIDName;
	funcPGPGetKeyForUsage fPGPGetKeyForUsage;
	
	funcPGPNewKeyDBObjBooleanFilter fPGPNewKeyDBObjBooleanFilter;
	funcPGPNewKeyDBObjNumericFilter fPGPNewKeyDBObjNumericFilter;
	funcPGPNewKeyDBObjTimeFilter fPGPNewKeyDBObjTimeFilter;
	funcPGPNewKeyDBObjDataFilter fPGPNewKeyDBObjDataFilter;
	funcPGPFreeFilter fPGPFreeFilter;
	funcPGPIncFilterRefCount fPGPIncFilterRefCount;
	funcPGPFilterChildObjects fPGPFilterChildObjects;
	funcPGPNegateFilter fPGPNegateFilter;
	funcPGPIntersectFilters fPGPIntersectFilters;
	funcPGPUnionFilters fPGPUnionFilters;
	funcPGPFilterKeySet fPGPFilterKeySet;
	funcPGPFilterKeyDB fPGPFilterKeyDB;

	funcPGPGenerateKey fPGPGenerateKey;
	funcPGPGenerateSubKey fPGPGenerateSubKey;
	funcPGPGetKeyEntropyNeeded fPGPGetKeyEntropyNeeded;
	funcPGPCopyKeyDBObj fPGPCopyKeyDBObj;

	funcPGPCopyKeys fPGPCopyKeys;
	funcPGPDeleteKeyDBObj fPGPDeleteKeyDBObj;
	funcPGPDeleteKeys fPGPDeleteKeys;

	funcPGPExport fPGPExport;
	funcPGPImport fPGPImport;
	funcPGPRevokeSig fPGPRevokeSig;
	funcPGPRevoke fPGPRevoke;
	funcPGPChangePassphrase fPGPChangePassphrase;
	funcPGPPassphraseIsValid fPGPPassphraseIsValid;

	funcPGPOrderKeySet fPGPOrderKeySet;
	funcPGPIncKeyListRefCount fPGPIncKeyListRefCount;
	funcPGPFreeKeyList fPGPFreeKeyList;

	funcPGPNewKeyIter fPGPNewKeyIter;
	funcPGPNewKeyIterFromKeySet fPGPNewKeyIterFromKeySet;
	funcPGPNewKeyIterFromKeyDB fPGPNewKeyIterFromKeyDB;
	funcPGPCopyKeyIter fPGPCopyKeyIter;
	funcPGPFreeKeyIter fPGPFreeKeyIter;
	funcPGPKeyIterIndex fPGPKeyIterIndex;
	funcPGPKeyIterRewind fPGPKeyIterRewind;
	funcPGPKeyIterSeek fPGPKeyIterSeek;
	funcPGPKeyIterMove fPGPKeyIterMove;
	funcPGPKeyIterNextKeyDBObj fPGPKeyIterNextKeyDBObj;
	funcPGPKeyIterPrevKeyDBObj fPGPKeyIterPrevKeyDBObj;
	funcPGPKeyIterGetKeyDBObj fPGPKeyIterGetKeyDBObj;

	funcPGPAddKeyOptions fPGPAddKeyOptions;
	funcPGPRemoveKeyOptions fPGPRemoveKeyOptions;
	funcPGPUpdateKeyOptions fPGPUpdateKeyOptions;

	funcPGPNewKeyID fPGPNewKeyID;
	funcPGPNewKeyIDFromString fPGPNewKeyIDFromString;
	funcPGPGetKeyIDAlgorithm fPGPGetKeyIDAlgorithm;
	funcPGPGetKeyIDString fPGPGetKeyIDString;

	funcPGPNewOptionList fPGPNewOptionList;
	funcPGPAppendOptionList fPGPAppendOptionList;
	funcPGPBuildOptionList fPGPBuildOptionList;
	funcPGPCopyOptionList fPGPCopyOptionList;
	funcPGPFreeOptionList fPGPFreeOptionList;
	funcPGPOLastOption fPGPOLastOption;

	funcPGPONullOption fPGPONullOption;

	funcPGPOInputFile fPGPOInputFile;
	funcPGPOInputBuffer fPGPOInputBuffer;

	funcPGPOOutputFile fPGPOOutputFile;
	funcPGPOOutputBuffer fPGPOOutputBuffer;
	funcPGPOOutputDirectory fPGPOOutputDirectory;

	funcPGPOEncryptToKeyDBObj fPGPOEncryptToKeyDBObj;
	funcPGPOEncryptToKeySet fPGPOEncryptToKeySet;
	funcPGPOIntegrityProtection fPGPOIntegrityProtection;
	funcPGPOObfuscateRecipients fPGPOObfuscateRecipients;
	funcPGPOSignWithKey fPGPOSignWithKey;
	funcPGPOConventionalEncrypt fPGPOConventionalEncrypt;
	funcPGPOPassphraseBuffer fPGPOPassphraseBuffer;
	funcPGPOPassphrase fPGPOPassphrase;
	funcPGPOPasskeyBuffer fPGPOPasskeyBuffer;
	funcPGPODetachedSig fPGPODetachedSig;

	funcPGPOCipherAlgorithm fPGPOCipherAlgorithm;
	funcPGPOHashAlgorithm fPGPOHashAlgorithm;
	funcPGPOCompressionAlgorithm fPGPOCompressionAlgorithm;

	funcPGPOEventHandler fPGPOEventHandler;
	funcPGPOSendNullEvents fPGPOSendNullEvents;

	funcPGPOArmorOutput fPGPOArmorOutput;
	funcPGPODataIsASCII fPGPODataIsASCII;
	funcPGPOClearSign fPGPOClearSign;
	funcPGPOKeyDBRef fPGPOKeyDBRef;

	funcPGPOExportKeySet fPGPOExportKeySet;
	funcPGPOExportKeyDBObj fPGPOExportKeyDBObj;
	funcPGPOImportKeysTo fPGPOImportKeysTo;
	funcPGPOKeyGenParams fPGPOKeyGenParams;

	funcPGPOKeyGenName fPGPOKeyGenName;
	funcPGPOCreationDate fPGPOCreationDate;
	funcPGPOExpiration fPGPOExpiration;

	funcPGPOAllocatedOutputBuffer fPGPOAllocatedOutputBuffer;
	funcPGPOAppendOutput fPGPOAppendOutput;
	funcPGPODiscardOutput fPGPODiscardOutput;
	funcPGPOAllocatedOutputKeyContainer fPGPOAllocatedOutputKeyContainer;
	funcPGPOKeyGenUseExistingEntropy fPGPOKeyGenUseExistingEntropy;
	funcPGPOKeyGenFast fPGPOKeyGenFast;
	funcPGPOKeyFlags fPGPOKeyFlags;
	funcPGPOKeyGenMasterKey fPGPOKeyGenMasterKey;

	funcPGPNewData fPGPNewData;
	funcPGPNewSecureData fPGPNewSecureData;
	funcPGPReallocData fPGPReallocData;
	funcPGPFreeData fPGPFreeData;

	funcPGPGetErrorString fPGPGetErrorString;
	funcPGPCheckKeyRingSigs fPGPCheckKeyRingSigs;
	funcPGPOExportPrivateKeys fPGPOExportPrivateKeys;
	funcPGPOExportPrivateSubkeys fPGPOExportPrivateSubkeys;
	funcPGPOVersionString fPGPOVersionString;
	funcPGPOExportFormat fPGPOExportFormat;
	funcPGPOInputFormat fPGPOInputFormat;
	funcPGPOPreferredCompressionAlgorithms fPGPOPreferredCompressionAlgorithms;
	funcPGPOPreferredAlgorithms fPGPOPreferredAlgorithms;
	funcPGPOPreferredHashAlgorithms fPGPOPreferredHashAlgorithms;
	funcPGPsdkSetLanguage fPGPsdkSetLanguage;
	funcPGPOOutputFormat fPGPOOutputFormat;
	funcPGPGetDefaultMemoryMgr fPGPGetDefaultMemoryMgr;
	funcPGPPeekKeyDBObjKey fPGPPeekKeyDBObjKey;
	funcPGPPeekKeyDBObjKeyDB fPGPPeekKeyDBObjKeyDB;
	funcPGPCompareKeyIDs fPGPCompareKeyIDs;
	funcPGPAddJobOptions fPGPAddJobOptions;
	funcPGPOCharsetString fPGPOCharsetString;
	funcPGPORawPGPInput fPGPORawPGPInput;
	funcPGPOCachePassphrase fPGPOCachePassphrase;
	funcPGPOPassThroughIfUnrecognized fPGPOPassThroughIfUnrecognized;
	funcPGPOPassThroughKeys fPGPOPassThroughKeys;
	funcPGPPeekContextMemoryMgr fPGPPeekContextMemoryMgr;

	funcPGPsdkNetworkLibInit fPGPsdkNetworkLibInit;
	funcPGPsdkNetworkLibCleanup fPGPsdkNetworkLibCleanup;
	funcPGPKeyServerInit fPGPKeyServerInit;
	funcPGPKeyServerCleanup fPGPKeyServerCleanup;
	funcPGPNewKeyServer fPGPNewKeyServer;
	funcPGPFreeKeyServer fPGPFreeKeyServer;
	funcPGPSetKeyServerEventHandler fPGPSetKeyServerEventHandler;
	funcPGPNewTLSContext fPGPNewTLSContext;
	funcPGPFreeTLSContext fPGPFreeTLSContext;
	funcPGPNewTLSSession fPGPNewTLSSession;
	funcPGPCopyTLSSession fPGPCopyTLSSession;

	funcPGPFreeTLSSession fPGPFreeTLSSession;

	funcPGPKeyServerOpen fPGPKeyServerOpen;
	funcPGPKeyServerClose fPGPKeyServerClose;

	funcPGPQueryKeyServer fPGPQueryKeyServer;
	funcPGPUploadToKeyServer fPGPUploadToKeyServer;
	funcPGPDeleteFromKeyServer fPGPDeleteFromKeyServer;
	funcPGPDisableFromKeyServer fPGPDisableFromKeyServer;

	funcPGPONetURL fPGPONetURL;
	funcPGPONetHostName fPGPONetHostName;
	funcPGPONetHostAddress fPGPONetHostAddress;
	funcPGPONetConnectTimeout fPGPONetConnectTimeout;
	funcPGPONetReadTimeout fPGPONetReadTimeout;
	funcPGPONetWriteTimeout fPGPONetWriteTimeout;
	funcPGPOKeyServerProtocol fPGPOKeyServerProtocol;
	funcPGPOKeyServerKeySpace fPGPOKeyServerKeySpace;
	funcPGPOKeyServerKeyStoreDN fPGPOKeyServerKeyStoreDN;
	funcPGPOKeyServerAccessType fPGPOKeyServerAccessType;
	funcPGPOKeyServerCAKey fPGPOKeyServerCAKey;
	funcPGPOKeyServerRequestKey fPGPOKeyServerRequestKey;
	funcPGPOKeyServerSearchKey fPGPOKeyServerSearchKey;
	funcPGPOKeyServerSearchFilter fPGPOKeyServerSearchFilter;
	funcPGPIncKeyServerRefCount fPGPIncKeyServerRefCount;

	/*	Get keyserver info. */
	funcPGPGetKeyServerTLSSession fPGPGetKeyServerTLSSession;
	funcPGPGetKeyServerProtocol fPGPGetKeyServerProtocol;
	funcPGPGetKeyServerAccessType fPGPGetKeyServerAccessType;
	funcPGPGetKeyServerKeySpace fPGPGetKeyServerKeySpace;
	funcPGPGetKeyServerPort fPGPGetKeyServerPort;
	funcPGPGetKeyServerHostName fPGPGetKeyServerHostName; /* Use PGPFreeData to free */
	funcPGPGetKeyServerAddress fPGPGetKeyServerAddress;
	funcPGPGetKeyServerPath fPGPGetKeyServerPath; /* Use PGPFreeData to free */
	funcPGPGetKeyServerVerificationKeyID fPGPGetKeyServerVerificationKeyID;
	funcPGPGetKeyServerContext fPGPGetKeyServerContext;

	funcPGPGetLastKeyServerErrorString fPGPGetLastKeyServerErrorString; /* Use PGPFreeData to free */

	/* X.509 Certificate Request functions */
	funcPGPSendCertificateRequest fPGPSendCertificateRequest;
	funcPGPRetrieveCertificate fPGPRetrieveCertificate;
	funcPGPRetrieveCertificateRevocationList fPGPRetrieveCertificateRevocationList;
	funcPGPGetProxyServer fPGPGetProxyServer;
	funcPGPGetProxyHost fPGPGetProxyHost;
	funcPGPMakeProxyAuthString fPGPMakeProxyAuthString;

	funcPGPRetrieveOCSPStatus fPGPRetrieveOCSPStatus;

	funcPGPKeyServerCreateThreadStorage fPGPKeyServerCreateThreadStorage;
	funcPGPKeyServerDisposeThreadStorage fPGPKeyServerDisposeThreadStorage;

#if 0
	funcPGPNewSDAContext fPGPNewSDAContext;
	funcPGPFreeSDAContext fPGPFreeSDAContext;
	funcPGPsdaImportObject fPGPsdaImportObject;
	funcPGPsdaSetAutoLaunchObject fPGPsdaSetAutoLaunchObject;
	funcPGPsdaCreate fPGPsdaCreate;
	funcPGPsdaDecrypt fPGPsdaDecrypt;
	funcPGPsdaVerify fPGPsdaVerify;
#endif

private:
	int    m_bValid;
};


#endif  //_PGP_SDK_FUNC_H