#pragma once

#include "stdafx.h"

#include "PgpSdkFunc.h"

HINSTANCE g_hPGPSdkDll = NULL;
HINSTANCE g_hPGPSdkNLDll = NULL;

int CPgpSdkFunc::InitializePgpSdk()
{
	if(m_bValid) return TRUE;

	if (!g_hPGPSdkDll || !g_hPGPSdkNLDll)
	{
		return m_bValid;
	}

	if( NULL == (fPGPsdkInit=(funcPGPsdkInit)GetProcAddress(g_hPGPSdkDll, "PGPsdkInit")) )
		goto _exit;
	if( NULL == (fPGPsdkCleanup=(funcPGPsdkCleanup)GetProcAddress(g_hPGPSdkDll, "PGPsdkCleanup")) )
		goto _exit;
	if( NULL == (fPGPNewContext=(funcPGPNewContext)GetProcAddress(g_hPGPSdkDll, "PGPNewContext")) )
		goto _exit;
	if( NULL == (fPGPNewContextCustom=(funcPGPNewContextCustom)GetProcAddress(g_hPGPSdkDll, "PGPNewContextCustom")) )
		goto _exit;
	if( NULL == (fPGPFreeContext=(funcPGPFreeContext)GetProcAddress(g_hPGPSdkDll, "PGPFreeContext")) )
		goto _exit;
	if( NULL == (fPGPEncode=(funcPGPEncode)GetProcAddress(g_hPGPSdkDll, "PGPEncode")) )
		goto _exit;
	if( NULL == (fPGPDecode=(funcPGPDecode)GetProcAddress(g_hPGPSdkDll, "PGPDecode")) )
		goto _exit;
	if( NULL == (fPGPGetPGPsdkVersionString=(funcPGPGetPGPsdkVersionString)GetProcAddress(g_hPGPSdkDll, "PGPGetPGPsdkVersionStringU16")) )
		goto _exit;
	if( NULL == (fPGPGetFeatureFlags=(funcPGPGetFeatureFlags)GetProcAddress(g_hPGPSdkDll, "PGPGetFeatureFlags")) )
		goto _exit;
	if( NULL == (fPGPGlobalRandomPoolGetEntropy=(funcPGPGlobalRandomPoolGetEntropy)GetProcAddress(g_hPGPSdkDll, "PGPGlobalRandomPoolGetEntropy")) )
		goto _exit;
	if( NULL == (fPGPGlobalRandomPoolGetSize=(funcPGPGlobalRandomPoolGetSize)GetProcAddress(g_hPGPSdkDll, "PGPGlobalRandomPoolGetSize")) )
		goto _exit;
	if( NULL == (fPGPGlobalRandomPoolGetMinimumEntropy=(funcPGPGlobalRandomPoolGetMinimumEntropy)GetProcAddress(g_hPGPSdkDll, "PGPGlobalRandomPoolGetMinimumEntropy")) )
		goto _exit;
	if( NULL == (fPGPGlobalRandomPoolHasMinimumEntropy=(funcPGPGlobalRandomPoolHasMinimumEntropy)GetProcAddress(g_hPGPSdkDll, "PGPGlobalRandomPoolHasMinimumEntropy")) )
		goto _exit;
	if( NULL == (fPGPSetRandSeedFile=(funcPGPSetRandSeedFile)GetProcAddress(g_hPGPSdkDll, "PGPSetRandSeedFile")) )
		goto _exit;
	if( NULL == (fPGPGlobalRandomPoolAddKeystroke=(funcPGPGlobalRandomPoolAddKeystroke)GetProcAddress(g_hPGPSdkDll, "PGPGlobalRandomPoolAddKeystroke")) )
		goto _exit;
	if( NULL == (fPGPGlobalRandomPoolMouseMoved=(funcPGPGlobalRandomPoolMouseMoved)GetProcAddress(g_hPGPSdkDll, "PGPGlobalRandomPoolMouseMoved")) )
		goto _exit;
	if( NULL == (fPGPGlobalRandomPoolAddSystemState=(funcPGPGlobalRandomPoolAddSystemState)GetProcAddress(g_hPGPSdkDll, "PGPGlobalRandomPoolAddSystemState")) )
		goto _exit; 
	if( NULL == (fPGPContextReserveRandomBytes=(funcPGPContextReserveRandomBytes)GetProcAddress(g_hPGPSdkDll, "PGPContextReserveRandomBytes")) )
		goto _exit;
	if( NULL == (fPGPCountPublicKeyAlgorithms=(funcPGPCountPublicKeyAlgorithms)GetProcAddress(g_hPGPSdkDll, "PGPCountPublicKeyAlgorithms")) )
		goto _exit;
	if( NULL == (fPGPGetIndexedPublicKeyAlgorithmInfo=(funcPGPGetIndexedPublicKeyAlgorithmInfo)GetProcAddress(g_hPGPSdkDll, "PGPGetIndexedPublicKeyAlgorithmInfo")) )
		goto _exit;
	if( NULL == (fPGPCountSymmetricCiphers=(funcPGPCountSymmetricCiphers)GetProcAddress(g_hPGPSdkDll, "PGPCountSymmetricCiphers")) )
		goto _exit;
	if( NULL == (fPGPGetIndexedSymmetricCipherInfo=(funcPGPGetIndexedSymmetricCipherInfo)GetProcAddress(g_hPGPSdkDll, "PGPGetIndexedSymmetricCipherInfo")) )
		goto _exit;
	if( NULL == (fPGPNewFileSpecFromFullPath=(funcPGPNewFileSpecFromFullPath)GetProcAddress(g_hPGPSdkDll, "PGPNewFileSpecFromFullPathU16")) )
		goto _exit;
	if( NULL == (fPGPGetFullPathFromFileSpec=(funcPGPGetFullPathFromFileSpec)GetProcAddress(g_hPGPSdkDll, "PGPGetFullPathFromFileSpecU16")) )
		goto _exit;
	if( NULL == (fPGPCopyFileSpec=(funcPGPCopyFileSpec)GetProcAddress(g_hPGPSdkDll, "PGPCopyFileSpec")) )
		goto _exit;
	if( NULL == (fPGPFreeFileSpec=(funcPGPFreeFileSpec)GetProcAddress(g_hPGPSdkDll, "PGPFreeFileSpec")) )
		goto _exit;
	if( NULL == (fPGPRenameFile=(funcPGPRenameFile)GetProcAddress(g_hPGPSdkDll, "PGPRenameFileU16")) )
		goto _exit;
	if( NULL == (fPGPDeleteFile=(funcPGPDeleteFile)GetProcAddress(g_hPGPSdkDll, "PGPDeleteFile")) )
		goto _exit;
	if( NULL == (fPGPWipeFile=(funcPGPWipeFile)GetProcAddress(g_hPGPSdkDll, "PGPWipeFile")) )
		goto _exit;
	if( NULL == (fPGPNewKeyDB=(funcPGPNewKeyDB)GetProcAddress(g_hPGPSdkDll, "PGPNewKeyDB")) )
		goto _exit;
	if( NULL == (fPGPOpenKeyDBFile=(funcPGPOpenKeyDBFile)GetProcAddress(g_hPGPSdkDll, "PGPOpenKeyDBFile")) )
		goto _exit;
	if( NULL == (fPGPFreeKeyDB=(funcPGPFreeKeyDB)GetProcAddress(g_hPGPSdkDll, "PGPFreeKeyDB")) )
		goto _exit;
	if( NULL == (fPGPFlushKeyDB=(funcPGPFlushKeyDB)GetProcAddress(g_hPGPSdkDll, "PGPFlushKeyDB")) )
		goto _exit;
	if( NULL == (fPGPIncKeyDBRefCount=(funcPGPIncKeyDBRefCount)GetProcAddress(g_hPGPSdkDll, "PGPIncKeyDBRefCount")) )
		goto _exit;
	if( NULL == (fPGPKeyDBIsMutable=(funcPGPKeyDBIsMutable)GetProcAddress(g_hPGPSdkDll, "PGPKeyDBIsMutable")) )
		goto _exit;
	if( NULL == (fPGPFindKeyByKeyID=(funcPGPFindKeyByKeyID)GetProcAddress(g_hPGPSdkDll, "PGPFindKeyByKeyID")) )
		goto _exit;
	if( NULL == (fPGPFindKeyByV3orV4KeyID=(funcPGPFindKeyByV3orV4KeyID)GetProcAddress(g_hPGPSdkDll, "PGPFindKeyByV3orV4KeyID")) )
		goto _exit;
	if( NULL == (fPGPCountKeysInKeyDB=(funcPGPCountKeysInKeyDB)GetProcAddress(g_hPGPSdkDll, "PGPCountKeysInKeyDB")) )
		goto _exit;
	if( NULL == (fPGPKeyDBIsUpdated=(funcPGPKeyDBIsUpdated)GetProcAddress(g_hPGPSdkDll, "PGPKeyDBIsUpdated")) )
		goto _exit;
	if( NULL == (fPGPCacheKeyDB=(funcPGPCacheKeyDB)GetProcAddress(g_hPGPSdkDll, "PGPCacheKeyDB")) )
		goto _exit;
	if( NULL == (fPGPPurgeKeyDBCache=(funcPGPPurgeKeyDBCache)GetProcAddress(g_hPGPSdkDll, "PGPPurgeKeyDBCache")) )
		goto _exit;
	if( NULL == (fPGPNewKeySet=(funcPGPNewKeySet)GetProcAddress(g_hPGPSdkDll, "PGPNewKeySet")) )
		goto _exit;
	if( NULL == (fPGPNewEmptyKeySet=(funcPGPNewEmptyKeySet)GetProcAddress(g_hPGPSdkDll, "PGPNewEmptyKeySet")) )
		goto _exit;
	if( NULL == (fPGPNewOneKeySet=(funcPGPNewOneKeySet)GetProcAddress(g_hPGPSdkDll, "PGPNewOneKeySet")) )
		goto _exit;
	if( NULL == (fPGPNewEmptyInclusiveKeySet=(funcPGPNewEmptyInclusiveKeySet)GetProcAddress(g_hPGPSdkDll, "PGPNewEmptyInclusiveKeySet")) )
		goto _exit;
	if( NULL == (fPGPFreeKeySet=(funcPGPFreeKeySet)GetProcAddress(g_hPGPSdkDll, "PGPFreeKeySet")) )
		goto _exit;
	if( NULL == (fPGPIncKeySetRefCount=(funcPGPIncKeySetRefCount)GetProcAddress(g_hPGPSdkDll, "PGPIncKeySetRefCount")) )
		goto _exit;
	if( NULL == (fPGPKeySetIsMember=(funcPGPKeySetIsMember)GetProcAddress(g_hPGPSdkDll, "PGPKeySetIsMember")) )
		goto _exit;
	if( NULL == (fPGPCountKeys=(funcPGPCountKeys)GetProcAddress(g_hPGPSdkDll, "PGPCountKeys")) )
		goto _exit;
	if( NULL == (fPGPAddKey=(funcPGPAddKey)GetProcAddress(g_hPGPSdkDll, "PGPAddKey")) )
		goto _exit;
	if( NULL == (fPGPAddKeys=(funcPGPAddKeys)GetProcAddress(g_hPGPSdkDll, "PGPAddKeys")) )
		goto _exit;
	if( NULL == (fPGPPeekKeySetKeyDB=(funcPGPPeekKeySetKeyDB)GetProcAddress(g_hPGPSdkDll, "PGPPeekKeySetKeyDB")) )
		goto _exit;
	if( NULL == (fPGPPeekKeyDBRootKeySet=(funcPGPPeekKeyDBRootKeySet)GetProcAddress(g_hPGPSdkDll, "PGPPeekKeyDBRootKeySet")) )
		goto _exit;
	if( NULL == (fPGPGetKeyDBObjBooleanProperty=(funcPGPGetKeyDBObjBooleanProperty)GetProcAddress(g_hPGPSdkDll, "PGPGetKeyDBObjBooleanProperty")) )
		goto _exit;
	if( NULL == (fPGPGetKeyDBObjNumericProperty=(funcPGPGetKeyDBObjNumericProperty)GetProcAddress(g_hPGPSdkDll, "PGPGetKeyDBObjNumericProperty")) )
		goto _exit;
	if( NULL == (fPGPGetKeyDBObjTimeProperty=(funcPGPGetKeyDBObjTimeProperty)GetProcAddress(g_hPGPSdkDll, "PGPGetKeyDBObjTimeProperty")) )
		goto _exit;
	if( NULL == (fPGPGetKeyDBObjDataProperty=(funcPGPGetKeyDBObjDataProperty)GetProcAddress(g_hPGPSdkDll, "PGPGetKeyDBObjDataPropertyU16")) )
		goto _exit;
	if( NULL == (fPGPGetKeyDBObjAllocatedDataProperty=(funcPGPGetKeyDBObjAllocatedDataProperty)GetProcAddress(g_hPGPSdkDll, "PGPGetKeyDBObjAllocatedDataPropertyU16")) )
		goto _exit;
	if( NULL == (fPGPSetKeyEnabled=(funcPGPSetKeyEnabled)GetProcAddress(g_hPGPSdkDll, "PGPSetKeyEnabled")) )
		goto _exit;
	if( NULL == (fPGPSetKeyAxiomatic=(funcPGPSetKeyAxiomatic)GetProcAddress(g_hPGPSdkDll, "PGPSetKeyAxiomatic")) )
		goto _exit;
	if( NULL == (fPGPGetKeyID=(funcPGPGetKeyID)GetProcAddress(g_hPGPSdkDll, "PGPGetKeyID")) )
		goto _exit;
	if( NULL == (fPGPGetPrimaryUserID=(funcPGPGetPrimaryUserID)GetProcAddress(g_hPGPSdkDll, "PGPGetPrimaryUserID")) )
		goto _exit;
	if( NULL == (fPGPGetPrimaryAttributeUserID=(funcPGPGetPrimaryAttributeUserID)GetProcAddress(g_hPGPSdkDll, "PGPGetPrimaryAttributeUserID")) )
		goto _exit;
	if( NULL == (fPGPGetPrimaryUserIDValidity=(funcPGPGetPrimaryUserIDValidity)GetProcAddress(g_hPGPSdkDll, "PGPGetPrimaryUserIDValidity")) )
		goto _exit;
	if( NULL == (fPGPGetPrimaryUserIDName=(funcPGPGetPrimaryUserIDName)GetProcAddress(g_hPGPSdkDll, "PGPGetPrimaryUserIDNameU16")) )
		goto _exit;
	if( NULL == (fPGPGetKeyForUsage=(funcPGPGetKeyForUsage)GetProcAddress(g_hPGPSdkDll, "PGPGetKeyForUsage")) )
		goto _exit;
	if( NULL == (fPGPNewKeyDBObjBooleanFilter=(funcPGPNewKeyDBObjBooleanFilter)GetProcAddress(g_hPGPSdkDll, "PGPNewKeyDBObjBooleanFilter")) )
		goto _exit;
	if( NULL == (fPGPNewKeyDBObjNumericFilter=(funcPGPNewKeyDBObjNumericFilter)GetProcAddress(g_hPGPSdkDll, "PGPNewKeyDBObjNumericFilter")) )
		goto _exit;
	if( NULL == (fPGPNewKeyDBObjTimeFilter=(funcPGPNewKeyDBObjTimeFilter)GetProcAddress(g_hPGPSdkDll, "PGPNewKeyDBObjTimeFilter")) )
		goto _exit;
	if( NULL == (fPGPNewKeyDBObjDataFilter=(funcPGPNewKeyDBObjDataFilter)GetProcAddress(g_hPGPSdkDll, "PGPNewKeyDBObjDataFilterU16")) )
		goto _exit;
	if( NULL == (fPGPFreeFilter=(funcPGPFreeFilter)GetProcAddress(g_hPGPSdkDll, "PGPFreeFilter")) )
		goto _exit;
	if( NULL == (fPGPIncFilterRefCount=(funcPGPIncFilterRefCount)GetProcAddress(g_hPGPSdkDll, "PGPIncFilterRefCount")) )
		goto _exit;
	if( NULL == (fPGPFilterChildObjects=(funcPGPFilterChildObjects)GetProcAddress(g_hPGPSdkDll, "PGPFilterChildObjects")) )
		goto _exit;
	if( NULL == (fPGPNegateFilter=(funcPGPNegateFilter)GetProcAddress(g_hPGPSdkDll, "PGPNegateFilter")) )
		goto _exit;
	if( NULL == (fPGPIntersectFilters=(funcPGPIntersectFilters)GetProcAddress(g_hPGPSdkDll, "PGPIntersectFilters")) )
		goto _exit;
	if( NULL == (fPGPUnionFilters=(funcPGPUnionFilters)GetProcAddress(g_hPGPSdkDll, "PGPUnionFilters")) )
		goto _exit;
	if( NULL == (fPGPFilterKeySet=(funcPGPFilterKeySet)GetProcAddress(g_hPGPSdkDll, "PGPFilterKeySet")) )
		goto _exit;
	if( NULL == (fPGPFilterKeyDB=(funcPGPFilterKeyDB)GetProcAddress(g_hPGPSdkDll, "PGPFilterKeyDB")) )
		goto _exit;
	if( NULL == (fPGPGenerateKey=(funcPGPGenerateKey)GetProcAddress(g_hPGPSdkDll, "PGPGenerateKey")) )
		goto _exit;
	if( NULL == (fPGPGenerateSubKey=(funcPGPGenerateSubKey)GetProcAddress(g_hPGPSdkDll, "PGPGenerateSubKey")) )
		goto _exit;
	if( NULL == (fPGPGetKeyEntropyNeeded=(funcPGPGetKeyEntropyNeeded)GetProcAddress(g_hPGPSdkDll, "PGPGetKeyEntropyNeeded")) )
		goto _exit;
	if( NULL == (fPGPCopyKeyDBObj=(funcPGPCopyKeyDBObj)GetProcAddress(g_hPGPSdkDll, "PGPCopyKeyDBObj")) )
		goto _exit;
	if( NULL == (fPGPCopyKeys=(funcPGPCopyKeys)GetProcAddress(g_hPGPSdkDll, "PGPCopyKeys")) )
		goto _exit;
	if( NULL == (fPGPDeleteKeyDBObj=(funcPGPDeleteKeyDBObj)GetProcAddress(g_hPGPSdkDll, "PGPDeleteKeyDBObj")) )
		goto _exit;
	if( NULL == (fPGPDeleteKeys=(funcPGPDeleteKeys)GetProcAddress(g_hPGPSdkDll, "PGPDeleteKeys")) )
		goto _exit;
	if( NULL == (fPGPExport=(funcPGPExport)GetProcAddress(g_hPGPSdkDll, "PGPExport")) )
		goto _exit;
	if( NULL == (fPGPImport=(funcPGPImport)GetProcAddress(g_hPGPSdkDll, "PGPImport")) )
		goto _exit;
	if( NULL == (fPGPRevokeSig=(funcPGPRevokeSig)GetProcAddress(g_hPGPSdkDll, "PGPRevokeSig")) )
		goto _exit;
	if( NULL == (fPGPRevoke=(funcPGPRevoke)GetProcAddress(g_hPGPSdkDll, "PGPRevoke")) )
		goto _exit;
	if( NULL == (fPGPChangePassphrase=(funcPGPChangePassphrase)GetProcAddress(g_hPGPSdkDll, "PGPChangePassphrase")) )
		goto _exit;
	if( NULL == (fPGPPassphraseIsValid=(funcPGPPassphraseIsValid)GetProcAddress(g_hPGPSdkDll, "PGPPassphraseIsValid")) )
		goto _exit;
	if( NULL == (fPGPOrderKeySet=(funcPGPOrderKeySet)GetProcAddress(g_hPGPSdkDll, "PGPOrderKeySet")) )
		goto _exit;
	if( NULL == (fPGPIncKeyListRefCount=(funcPGPIncKeyListRefCount)GetProcAddress(g_hPGPSdkDll, "PGPIncKeyListRefCount")) )
		goto _exit;
	if( NULL == (fPGPFreeKeyList=(funcPGPFreeKeyList)GetProcAddress(g_hPGPSdkDll, "PGPFreeKeyList")) )
		goto _exit;
	if( NULL == (fPGPNewKeyIter=(funcPGPNewKeyIter)GetProcAddress(g_hPGPSdkDll, "PGPNewKeyIter")) )
		goto _exit;
	if( NULL == (fPGPNewKeyIterFromKeySet=(funcPGPNewKeyIterFromKeySet)GetProcAddress(g_hPGPSdkDll, "PGPNewKeyIterFromKeySet")) )
		goto _exit;
	if( NULL == (fPGPNewKeyIterFromKeyDB=(funcPGPNewKeyIterFromKeyDB)GetProcAddress(g_hPGPSdkDll, "PGPNewKeyIterFromKeyDB")) )
		goto _exit;
	if( NULL == (fPGPCopyKeyIter=(funcPGPCopyKeyIter)GetProcAddress(g_hPGPSdkDll, "PGPCopyKeyIter")) )
		goto _exit;
	if( NULL == (fPGPFreeKeyIter=(funcPGPFreeKeyIter)GetProcAddress(g_hPGPSdkDll, "PGPFreeKeyIter")) )
		goto _exit;
	if( NULL == (fPGPKeyIterIndex=(funcPGPKeyIterIndex)GetProcAddress(g_hPGPSdkDll, "PGPKeyIterIndex")) )
		goto _exit;
	if( NULL == (fPGPKeyIterRewind=(funcPGPKeyIterRewind)GetProcAddress(g_hPGPSdkDll, "PGPKeyIterRewind")) )
		goto _exit;
	if( NULL == (fPGPKeyIterSeek=(funcPGPKeyIterSeek)GetProcAddress(g_hPGPSdkDll, "PGPKeyIterSeek")) )
		goto _exit;
	if( NULL == (fPGPKeyIterMove=(funcPGPKeyIterMove)GetProcAddress(g_hPGPSdkDll, "PGPKeyIterMove")) )
		goto _exit;
	if( NULL == (fPGPKeyIterNextKeyDBObj=(funcPGPKeyIterNextKeyDBObj)GetProcAddress(g_hPGPSdkDll, "PGPKeyIterNextKeyDBObj")) )
		goto _exit;
	if( NULL == (fPGPKeyIterPrevKeyDBObj=(funcPGPKeyIterPrevKeyDBObj)GetProcAddress(g_hPGPSdkDll, "PGPKeyIterPrevKeyDBObj")) )
		goto _exit;
	if( NULL == (fPGPKeyIterGetKeyDBObj=(funcPGPKeyIterGetKeyDBObj)GetProcAddress(g_hPGPSdkDll, "PGPKeyIterGetKeyDBObj")) )
		goto _exit;
	if( NULL == (fPGPAddKeyOptions=(funcPGPAddKeyOptions)GetProcAddress(g_hPGPSdkDll, "PGPAddKeyOptions")) )
		goto _exit;
	if( NULL == (fPGPRemoveKeyOptions=(funcPGPRemoveKeyOptions)GetProcAddress(g_hPGPSdkDll, "PGPRemoveKeyOptions")) )
		goto _exit;
	if( NULL == (fPGPUpdateKeyOptions=(funcPGPUpdateKeyOptions)GetProcAddress(g_hPGPSdkDll, "PGPUpdateKeyOptions")) )
		goto _exit;
	if( NULL == (fPGPNewKeyID=(funcPGPNewKeyID)GetProcAddress(g_hPGPSdkDll, "PGPNewKeyID")) )
		goto _exit;
	if( NULL == (fPGPNewKeyIDFromString=(funcPGPNewKeyIDFromString)GetProcAddress(g_hPGPSdkDll, "PGPNewKeyIDFromStringU16")) )
		goto _exit;
	if( NULL == (fPGPGetKeyIDAlgorithm=(funcPGPGetKeyIDAlgorithm)GetProcAddress(g_hPGPSdkDll, "PGPGetKeyIDAlgorithm")) )
		goto _exit;
	if( NULL == (fPGPGetKeyIDString=(funcPGPGetKeyIDString)GetProcAddress(g_hPGPSdkDll, "PGPGetKeyIDStringU16")) )
		goto _exit;
	if( NULL == (fPGPNewOptionList=(funcPGPNewOptionList)GetProcAddress(g_hPGPSdkDll, "PGPNewOptionList")) )
		goto _exit;
	if( NULL == (fPGPAppendOptionList=(funcPGPAppendOptionList)GetProcAddress(g_hPGPSdkDll, "PGPAppendOptionList")) )
		goto _exit;
	if( NULL == (fPGPBuildOptionList=(funcPGPBuildOptionList)GetProcAddress(g_hPGPSdkDll, "PGPBuildOptionList")) )
		goto _exit;
	if( NULL == (fPGPCopyOptionList=(funcPGPCopyOptionList)GetProcAddress(g_hPGPSdkDll, "PGPCopyOptionList")) )
		goto _exit;
	if( NULL == (fPGPFreeOptionList=(funcPGPFreeOptionList)GetProcAddress(g_hPGPSdkDll, "PGPFreeOptionList")) )
		goto _exit;
	if( NULL == (fPGPOLastOption=(funcPGPOLastOption)GetProcAddress(g_hPGPSdkDll, "PGPOLastOption")) )
		goto _exit;
	if( NULL == (fPGPONullOption=(funcPGPONullOption)GetProcAddress(g_hPGPSdkDll, "PGPONullOption")) )
		goto _exit;
	if( NULL == (fPGPOInputFile=(funcPGPOInputFile)GetProcAddress(g_hPGPSdkDll, "PGPOInputFile")) )
		goto _exit;
	if( NULL == (fPGPOInputBuffer=(funcPGPOInputBuffer)GetProcAddress(g_hPGPSdkDll, "PGPOInputBuffer")) )
		goto _exit;
	if( NULL == (fPGPOOutputFile=(funcPGPOOutputFile)GetProcAddress(g_hPGPSdkDll, "PGPOOutputFile")) )
		goto _exit;
	if( NULL == (fPGPOOutputBuffer=(funcPGPOOutputBuffer)GetProcAddress(g_hPGPSdkDll, "PGPOOutputBuffer")) )
		goto _exit;
	if( NULL == (fPGPOOutputDirectory=(funcPGPOOutputDirectory)GetProcAddress(g_hPGPSdkDll, "PGPOOutputDirectory")) )
		goto _exit;
	if( NULL == (fPGPOEncryptToKeyDBObj=(funcPGPOEncryptToKeyDBObj)GetProcAddress(g_hPGPSdkDll, "PGPOEncryptToKeyDBObj")) )
		goto _exit;
	if( NULL == (fPGPOEncryptToKeySet=(funcPGPOEncryptToKeySet)GetProcAddress(g_hPGPSdkDll, "PGPOEncryptToKeySet")) )
		goto _exit;
	if( NULL == (fPGPOIntegrityProtection=(funcPGPOIntegrityProtection)GetProcAddress(g_hPGPSdkDll, "PGPOIntegrityProtection")) )
		goto _exit;
	if( NULL == (fPGPOObfuscateRecipients=(funcPGPOObfuscateRecipients)GetProcAddress(g_hPGPSdkDll, "PGPOObfuscateRecipients")) )
		goto _exit;
	if( NULL == (fPGPOSignWithKey=(funcPGPOSignWithKey)GetProcAddress(g_hPGPSdkDll, "PGPOSignWithKey")) )
		goto _exit;
	if( NULL == (fPGPOConventionalEncrypt=(funcPGPOConventionalEncrypt)GetProcAddress(g_hPGPSdkDll, "PGPOConventionalEncrypt")) )
		goto _exit;
	if( NULL == (fPGPOPassphraseBuffer=(funcPGPOPassphraseBuffer)GetProcAddress(g_hPGPSdkDll, "PGPOPassphraseBufferU16")) )
		goto _exit;
	if( NULL == (fPGPOPassphrase=(funcPGPOPassphrase)GetProcAddress(g_hPGPSdkDll, "PGPOPassphraseU16")) )
		goto _exit;
	if( NULL == (fPGPOPasskeyBuffer=(funcPGPOPasskeyBuffer)GetProcAddress(g_hPGPSdkDll, "PGPOPasskeyBuffer")) )
		goto _exit;
	if( NULL == (fPGPODetachedSig=(funcPGPODetachedSig)GetProcAddress(g_hPGPSdkDll, "PGPODetachedSig")) )
		goto _exit;
	if( NULL == (fPGPOCipherAlgorithm=(funcPGPOCipherAlgorithm)GetProcAddress(g_hPGPSdkDll, "PGPOCipherAlgorithm")) )
		goto _exit;
	if( NULL == (fPGPOHashAlgorithm=(funcPGPOHashAlgorithm)GetProcAddress(g_hPGPSdkDll, "PGPOHashAlgorithm")) )
		goto _exit;
	if( NULL == (fPGPOCompressionAlgorithm=(funcPGPOCompressionAlgorithm)GetProcAddress(g_hPGPSdkDll, "PGPOCompressionAlgorithm")) )
		goto _exit;
	if( NULL == (fPGPOEventHandler=(funcPGPOEventHandler)GetProcAddress(g_hPGPSdkDll, "PGPOEventHandler")) )
		goto _exit;
	if( NULL == (fPGPOSendNullEvents=(funcPGPOSendNullEvents)GetProcAddress(g_hPGPSdkDll, "PGPOSendNullEvents")) )
		goto _exit;
	if( NULL == (fPGPOArmorOutput=(funcPGPOArmorOutput)GetProcAddress(g_hPGPSdkDll, "PGPOArmorOutput")) )
		goto _exit;
	if( NULL == (fPGPODataIsASCII=(funcPGPODataIsASCII)GetProcAddress(g_hPGPSdkDll, "PGPODataIsASCII")) )
		goto _exit;
	if( NULL == (fPGPOClearSign=(funcPGPOClearSign)GetProcAddress(g_hPGPSdkDll, "PGPOClearSign")) )
		goto _exit;
	if( NULL == (fPGPOKeyDBRef=(funcPGPOKeyDBRef)GetProcAddress(g_hPGPSdkDll, "PGPOKeyDBRef")) )
		goto _exit;
	if( NULL == (fPGPOExportKeySet=(funcPGPOExportKeySet)GetProcAddress(g_hPGPSdkDll, "PGPOExportKeySet")) )
		goto _exit;
	if( NULL == (fPGPOExportKeyDBObj=(funcPGPOExportKeyDBObj)GetProcAddress(g_hPGPSdkDll, "PGPOExportKeyDBObj")) )
		goto _exit;
	if( NULL == (fPGPOImportKeysTo=(funcPGPOImportKeysTo)GetProcAddress(g_hPGPSdkDll, "PGPOImportKeysTo")) )
		goto _exit;
	if( NULL == (fPGPOKeyGenParams=(funcPGPOKeyGenParams)GetProcAddress(g_hPGPSdkDll, "PGPOKeyGenParams")) )
		goto _exit;
	if( NULL == (fPGPOKeyGenName=(funcPGPOKeyGenName)GetProcAddress(g_hPGPSdkDll, "PGPOKeyGenNameU16")) )
		goto _exit;
	if( NULL == (fPGPOCreationDate=(funcPGPOCreationDate)GetProcAddress(g_hPGPSdkDll, "PGPOCreationDate")) )
		goto _exit;
	if( NULL == (fPGPOExpiration=(funcPGPOExpiration)GetProcAddress(g_hPGPSdkDll, "PGPOExpiration")) )
		goto _exit;
	if( NULL == (fPGPOAllocatedOutputBuffer=(funcPGPOAllocatedOutputBuffer)GetProcAddress(g_hPGPSdkDll, "PGPOAllocatedOutputBuffer")) )
		goto _exit;
	if( NULL == (fPGPOAppendOutput=(funcPGPOAppendOutput)GetProcAddress(g_hPGPSdkDll, "PGPOAppendOutput")) )
		goto _exit;
	if( NULL == (fPGPODiscardOutput=(funcPGPODiscardOutput)GetProcAddress(g_hPGPSdkDll, "PGPODiscardOutput")) )
		goto _exit;
	if( NULL == (fPGPOAllocatedOutputKeyContainer=(funcPGPOAllocatedOutputKeyContainer)GetProcAddress(g_hPGPSdkDll, "PGPOAllocatedOutputKeyContainer")) )
		goto _exit;
	if( NULL == (fPGPOKeyGenUseExistingEntropy=(funcPGPOKeyGenUseExistingEntropy)GetProcAddress(g_hPGPSdkDll, "PGPOKeyGenUseExistingEntropy")) )
		goto _exit;
	if( NULL == (fPGPOKeyGenFast=(funcPGPOKeyGenFast)GetProcAddress(g_hPGPSdkDll, "PGPOKeyGenFast")) )
		goto _exit;
	if( NULL == (fPGPOKeyFlags=(funcPGPOKeyFlags)GetProcAddress(g_hPGPSdkDll, "PGPOKeyFlags")) )
		goto _exit;
	if( NULL == (fPGPOKeyGenMasterKey=(funcPGPOKeyGenMasterKey)GetProcAddress(g_hPGPSdkDll, "PGPOKeyGenMasterKey")) )
		goto _exit;
	if( NULL == (fPGPNewData=(funcPGPNewData)GetProcAddress(g_hPGPSdkDll, "PGPNewData")) )
		goto _exit;
	if( NULL == (fPGPNewSecureData=(funcPGPNewSecureData)GetProcAddress(g_hPGPSdkDll, "PGPNewSecureData")) )
		goto _exit;
	if( NULL == (fPGPReallocData=(funcPGPReallocData)GetProcAddress(g_hPGPSdkDll, "PGPReallocData")) )
		goto _exit;
	if( NULL == (fPGPFreeData=(funcPGPFreeData)GetProcAddress(g_hPGPSdkDll, "PGPFreeData")) )
		goto _exit;
	if( NULL == (fPGPGetErrorString=(funcPGPGetErrorString)GetProcAddress(g_hPGPSdkDll, "PGPGetErrorString")) )
		goto _exit;
	if( NULL == (fPGPCheckKeyRingSigs=(funcPGPCheckKeyRingSigs)GetProcAddress(g_hPGPSdkDll, "PGPCheckKeyRingSigs")) )
		goto _exit;
	if( NULL == (fPGPOExportPrivateKeys=(funcPGPOExportPrivateKeys)GetProcAddress(g_hPGPSdkDll, "PGPOExportPrivateKeys")) )
		goto _exit;
	if( NULL == (fPGPOExportPrivateSubkeys=(funcPGPOExportPrivateSubkeys)GetProcAddress(g_hPGPSdkDll, "PGPOExportPrivateSubkeys")) )
		goto _exit;
	if( NULL == (fPGPOVersionString=(funcPGPOVersionString)GetProcAddress(g_hPGPSdkDll, "PGPOVersionStringU16")) )
		goto _exit;
	if( NULL == (fPGPOExportFormat=(funcPGPOExportFormat)GetProcAddress(g_hPGPSdkDll, "PGPOExportFormat")) )
		goto _exit;
	if( NULL == (fPGPOInputFormat=(funcPGPOInputFormat)GetProcAddress(g_hPGPSdkDll, "PGPOInputFormat")) )
		goto _exit;
	if( NULL == (fPGPOPreferredCompressionAlgorithms=(funcPGPOPreferredCompressionAlgorithms)GetProcAddress(g_hPGPSdkDll, "PGPOPreferredCompressionAlgorithms")) )
		goto _exit;
	if( NULL == (fPGPOPreferredAlgorithms=(funcPGPOPreferredAlgorithms)GetProcAddress(g_hPGPSdkDll, "PGPOPreferredAlgorithms")) )
		goto _exit;
	if( NULL == (fPGPOPreferredHashAlgorithms=(funcPGPOPreferredHashAlgorithms)GetProcAddress(g_hPGPSdkDll, "PGPOPreferredHashAlgorithms")) )
		goto _exit;
	if( NULL == (fPGPsdkSetLanguage=(funcPGPsdkSetLanguage)GetProcAddress(g_hPGPSdkDll, "PGPsdkSetLanguage")) )
		goto _exit;
	if( NULL == (fPGPOOutputFormat=(funcPGPOOutputFormat)GetProcAddress(g_hPGPSdkDll, "PGPOOutputFormat")) )
		goto _exit;
	if( NULL == (fPGPGetDefaultMemoryMgr=(funcPGPGetDefaultMemoryMgr)GetProcAddress(g_hPGPSdkDll, "PGPGetDefaultMemoryMgr")) )
		goto _exit;
	if( NULL == (fPGPPeekKeyDBObjKey=(funcPGPPeekKeyDBObjKey)GetProcAddress(g_hPGPSdkDll, "PGPPeekKeyDBObjKey")) )
		goto _exit;
	if( NULL == (fPGPPeekKeyDBObjKeyDB=(funcPGPPeekKeyDBObjKeyDB)GetProcAddress(g_hPGPSdkDll, "PGPPeekKeyDBObjKeyDB")) )
		goto _exit;
	if( NULL == (fPGPCompareKeyIDs=(funcPGPCompareKeyIDs)GetProcAddress(g_hPGPSdkDll, "PGPCompareKeyIDs")) )
		goto _exit;
	if( NULL == (fPGPAddJobOptions=(funcPGPAddJobOptions)GetProcAddress(g_hPGPSdkDll, "PGPAddJobOptions")) )
		goto _exit;
	if( NULL == (fPGPOCharsetString=(funcPGPOCharsetString)GetProcAddress(g_hPGPSdkDll, "PGPOCharsetString")) )
		goto _exit;
	if( NULL == (fPGPORawPGPInput=(funcPGPORawPGPInput)GetProcAddress(g_hPGPSdkDll, "PGPORawPGPInput")) )
		goto _exit;
	if( NULL == (fPGPOCachePassphrase=(funcPGPOCachePassphrase)GetProcAddress(g_hPGPSdkDll, "PGPOCachePassphrase")) )
		goto _exit;
	if( NULL == (fPGPOPassThroughIfUnrecognized=(funcPGPOPassThroughIfUnrecognized)GetProcAddress(g_hPGPSdkDll, "PGPOPassThroughIfUnrecognized")) )
		goto _exit;
	if( NULL == (fPGPOPassThroughKeys=(funcPGPOPassThroughKeys)GetProcAddress(g_hPGPSdkDll, "PGPOPassThroughKeys")) )
		goto _exit;
	if( NULL == (fPGPPeekContextMemoryMgr=(funcPGPPeekContextMemoryMgr)GetProcAddress(g_hPGPSdkDll, "PGPPeekContextMemoryMgr")) )
		goto _exit;

	// PGPSDKNetworkLib
	if( NULL == (fPGPsdkNetworkLibInit=(funcPGPsdkNetworkLibInit)GetProcAddress(g_hPGPSdkNLDll, "PGPsdkNetworkLibInit")) )
		goto _exit;
	if( NULL == (fPGPsdkNetworkLibCleanup=(funcPGPsdkNetworkLibCleanup)GetProcAddress(g_hPGPSdkNLDll, "PGPsdkNetworkLibCleanup")) )
		goto _exit;
	if( NULL == (fPGPKeyServerInit=(funcPGPKeyServerInit)GetProcAddress(g_hPGPSdkNLDll, "PGPKeyServerInit")) )
		goto _exit;
	if( NULL == (fPGPKeyServerCleanup=(funcPGPKeyServerCleanup)GetProcAddress(g_hPGPSdkNLDll, "PGPKeyServerCleanup")) )
		goto _exit;
	if( NULL == (fPGPNewKeyServer=(funcPGPNewKeyServer)GetProcAddress(g_hPGPSdkNLDll, "PGPNewKeyServer")) )
		goto _exit;
	if( NULL == (fPGPFreeKeyServer=(funcPGPFreeKeyServer)GetProcAddress(g_hPGPSdkNLDll, "PGPFreeKeyServer")) )
		goto _exit;
	if( NULL == (fPGPSetKeyServerEventHandler=(funcPGPSetKeyServerEventHandler)GetProcAddress(g_hPGPSdkNLDll, "PGPSetKeyServerEventHandler")) )
		goto _exit;
	if( NULL == (fPGPNewTLSContext=(funcPGPNewTLSContext)GetProcAddress(g_hPGPSdkNLDll, "PGPNewTLSContext")) )
		goto _exit;
	if( NULL == (fPGPFreeTLSContext=(funcPGPFreeTLSContext)GetProcAddress(g_hPGPSdkNLDll, "PGPFreeTLSContext")) )
		goto _exit;
	if( NULL == (fPGPNewTLSSession=(funcPGPNewTLSSession)GetProcAddress(g_hPGPSdkNLDll, "PGPNewTLSSession")) )
		goto _exit;
	if( NULL == (fPGPCopyTLSSession=(funcPGPCopyTLSSession)GetProcAddress(g_hPGPSdkNLDll, "PGPCopyTLSSession")) )
		goto _exit;
	if( NULL == (fPGPFreeTLSSession=(funcPGPFreeTLSSession)GetProcAddress(g_hPGPSdkNLDll, "PGPFreeTLSSession")) )
		goto _exit;
	if( NULL == (fPGPKeyServerOpen=(funcPGPKeyServerOpen)GetProcAddress(g_hPGPSdkNLDll, "PGPKeyServerOpen")) )
		goto _exit;
	if( NULL == (fPGPKeyServerClose=(funcPGPKeyServerClose)GetProcAddress(g_hPGPSdkNLDll, "PGPKeyServerClose")) )
		goto _exit;
	if( NULL == (fPGPQueryKeyServer=(funcPGPQueryKeyServer)GetProcAddress(g_hPGPSdkNLDll, "PGPQueryKeyServer")) )
		goto _exit;
	if( NULL == (fPGPUploadToKeyServer=(funcPGPUploadToKeyServer)GetProcAddress(g_hPGPSdkNLDll, "PGPUploadToKeyServer")) )
		goto _exit;
	if( NULL == (fPGPDeleteFromKeyServer=(funcPGPDeleteFromKeyServer)GetProcAddress(g_hPGPSdkNLDll, "PGPDeleteFromKeyServer")) )
		goto _exit;
	if( NULL == (fPGPDisableFromKeyServer=(funcPGPDisableFromKeyServer)GetProcAddress(g_hPGPSdkNLDll, "PGPDisableFromKeyServer")) )
		goto _exit;
	if( NULL == (fPGPONetURL=(funcPGPONetURL)GetProcAddress(g_hPGPSdkNLDll, "PGPONetURLU16")) )
		goto _exit;
	if( NULL == (fPGPONetHostName=(funcPGPONetHostName)GetProcAddress(g_hPGPSdkNLDll, "PGPONetHostNameU16")) )
		goto _exit;
	if( NULL == (fPGPONetHostAddress=(funcPGPONetHostAddress)GetProcAddress(g_hPGPSdkNLDll, "PGPONetHostAddress")) )
		goto _exit;
	if( NULL == (fPGPONetConnectTimeout=(funcPGPONetConnectTimeout)GetProcAddress(g_hPGPSdkNLDll, "PGPONetConnectTimeout")) )
		goto _exit;
	if( NULL == (fPGPONetReadTimeout=(funcPGPONetReadTimeout)GetProcAddress(g_hPGPSdkNLDll, "PGPONetReadTimeout")) )
		goto _exit;
	if( NULL == (fPGPONetWriteTimeout=(funcPGPONetWriteTimeout)GetProcAddress(g_hPGPSdkNLDll, "PGPONetWriteTimeout")) )
		goto _exit;
	if( NULL == (fPGPOKeyServerProtocol=(funcPGPOKeyServerProtocol)GetProcAddress(g_hPGPSdkNLDll, "PGPOKeyServerProtocol")) )
		goto _exit;
	if( NULL == (fPGPOKeyServerKeySpace=(funcPGPOKeyServerKeySpace)GetProcAddress(g_hPGPSdkNLDll, "PGPOKeyServerKeySpace")) )
		goto _exit;
	if( NULL == (fPGPOKeyServerKeyStoreDN=(funcPGPOKeyServerKeyStoreDN)GetProcAddress(g_hPGPSdkNLDll, "PGPOKeyServerKeyStoreDNU16")) )
		goto _exit;
	if( NULL == (fPGPOKeyServerAccessType=(funcPGPOKeyServerAccessType)GetProcAddress(g_hPGPSdkNLDll, "PGPOKeyServerAccessType")) )
		goto _exit;
	if( NULL == (fPGPOKeyServerCAKey=(funcPGPOKeyServerCAKey)GetProcAddress(g_hPGPSdkNLDll, "PGPOKeyServerCAKey")) )
		goto _exit;
	if( NULL == (fPGPOKeyServerRequestKey=(funcPGPOKeyServerRequestKey)GetProcAddress(g_hPGPSdkNLDll, "PGPOKeyServerRequestKey")) )
		goto _exit;
	if( NULL == (fPGPOKeyServerSearchKey=(funcPGPOKeyServerSearchKey)GetProcAddress(g_hPGPSdkNLDll, "PGPOKeyServerSearchKey")) )
		goto _exit;
	if( NULL == (fPGPOKeyServerSearchFilter=(funcPGPOKeyServerSearchFilter)GetProcAddress(g_hPGPSdkNLDll, "PGPOKeyServerSearchFilter")) )
		goto _exit;
	if( NULL == (fPGPIncKeyServerRefCount=(funcPGPIncKeyServerRefCount)GetProcAddress(g_hPGPSdkNLDll, "PGPIncKeyServerRefCount")) )
		goto _exit;
	if( NULL == (fPGPGetKeyServerTLSSession=(funcPGPGetKeyServerTLSSession)GetProcAddress(g_hPGPSdkNLDll, "PGPGetKeyServerTLSSession")) )
		goto _exit;
	if( NULL == (fPGPGetKeyServerProtocol=(funcPGPGetKeyServerProtocol)GetProcAddress(g_hPGPSdkNLDll, "PGPGetKeyServerProtocol")) )
		goto _exit;
	if( NULL == (fPGPGetKeyServerAccessType=(funcPGPGetKeyServerAccessType)GetProcAddress(g_hPGPSdkNLDll, "PGPGetKeyServerAccessType")) )
		goto _exit;
	if( NULL == (fPGPGetKeyServerKeySpace=(funcPGPGetKeyServerKeySpace)GetProcAddress(g_hPGPSdkNLDll, "PGPGetKeyServerKeySpace")) )
		goto _exit;
	if( NULL == (fPGPGetKeyServerPort=(funcPGPGetKeyServerPort)GetProcAddress(g_hPGPSdkNLDll, "PGPGetKeyServerPort")) )
		goto _exit;
	if( NULL == (fPGPGetKeyServerHostName=(funcPGPGetKeyServerHostName)GetProcAddress(g_hPGPSdkNLDll, "PGPGetKeyServerHostNameU16")) )
		goto _exit;
	if( NULL == (fPGPGetKeyServerAddress=(funcPGPGetKeyServerAddress)GetProcAddress(g_hPGPSdkNLDll, "PGPGetKeyServerAddress")) )
		goto _exit;
	if( NULL == (fPGPGetKeyServerPath=(funcPGPGetKeyServerPath)GetProcAddress(g_hPGPSdkNLDll, "PGPGetKeyServerPathU16")) )
		goto _exit;
	if( NULL == (fPGPGetKeyServerVerificationKeyID=(funcPGPGetKeyServerVerificationKeyID)GetProcAddress(g_hPGPSdkNLDll, "PGPGetKeyServerVerificationKeyID")) )
		goto _exit;
	if( NULL == (fPGPGetKeyServerContext=(funcPGPGetKeyServerContext)GetProcAddress(g_hPGPSdkNLDll, "PGPGetKeyServerContext")) )
		goto _exit;
	if( NULL == (fPGPGetLastKeyServerErrorString=(funcPGPGetLastKeyServerErrorString)GetProcAddress(g_hPGPSdkNLDll, "PGPGetLastKeyServerErrorStringU16")) )
		goto _exit;
	if( NULL == (fPGPSendCertificateRequest=(funcPGPSendCertificateRequest)GetProcAddress(g_hPGPSdkNLDll, "PGPSendCertificateRequest")) )
		goto _exit;
	if( NULL == (fPGPRetrieveCertificate=(funcPGPRetrieveCertificate)GetProcAddress(g_hPGPSdkNLDll, "PGPRetrieveCertificate")) )
		goto _exit;
	if( NULL == (fPGPRetrieveCertificateRevocationList=(funcPGPRetrieveCertificateRevocationList)GetProcAddress(g_hPGPSdkNLDll, "PGPRetrieveCertificateRevocationList")) )
		goto _exit;
	if( NULL == (fPGPGetProxyServer=(funcPGPGetProxyServer)GetProcAddress(g_hPGPSdkNLDll, "PGPGetProxyServerU16")) )
		goto _exit;
	if( NULL == (fPGPGetProxyHost=(funcPGPGetProxyHost)GetProcAddress(g_hPGPSdkNLDll, "PGPGetProxyHostU16")) )
		goto _exit;
	if( NULL == (fPGPMakeProxyAuthString=(funcPGPMakeProxyAuthString)GetProcAddress(g_hPGPSdkNLDll, "PGPMakeProxyAuthStringU16")) )
		goto _exit;
	if( NULL == (fPGPRetrieveOCSPStatus=(funcPGPRetrieveOCSPStatus)GetProcAddress(g_hPGPSdkNLDll, "PGPRetrieveOCSPStatusU16")) )
		goto _exit;
	if( NULL == (fPGPKeyServerCreateThreadStorage=(funcPGPKeyServerCreateThreadStorage)GetProcAddress(g_hPGPSdkNLDll, "PGPKeyServerCreateThreadStorage")) )
		goto _exit;
	if( NULL == (fPGPKeyServerDisposeThreadStorage=(funcPGPKeyServerDisposeThreadStorage)GetProcAddress(g_hPGPSdkNLDll, "PGPKeyServerDisposeThreadStorage")) )
		goto _exit;
#if 0
	if( NULL == (fPGPNewSDAContext=(funcPGPNewSDAContext)GetProcAddress(g_hPGPSdkDll, "PGPNewSDAContext")) )
		goto _exit;
	if( NULL == (fPGPFreeSDAContext=(funcPGPFreeSDAContext)GetProcAddress(g_hPGPSdkDll, "PGPFreeSDAContext")) )
		goto _exit;
	if( NULL == (fPGPsdaImportObject=(funcPGPsdaImportObject)GetProcAddress(g_hPGPSdkDll, "PGPsdaImportObject")) )
		goto _exit;
	if( NULL == (fPGPsdaSetAutoLaunchObject=(funcPGPsdaSetAutoLaunchObject)GetProcAddress(g_hPGPSdkDll, "PGPsdaSetAutoLaunchObject")) )
		goto _exit;
	if( NULL == (fPGPsdaCreate=(funcPGPsdaCreate)GetProcAddress(g_hPGPSdkDll, "PGPsdaCreate")) )
		goto _exit;
	if( NULL == (fPGPsdaDecrypt=(funcPGPsdaDecrypt)GetProcAddress(g_hPGPSdkDll, "PGPsdaDecrypt")) )
		goto _exit;
	if( NULL == (fPGPsdaVerify=(funcPGPsdaVerify)GetProcAddress(g_hPGPSdkDll, "PGPsdaVerify")) )
		goto _exit;
#endif

	m_bValid = 1;

_exit:

	return m_bValid;
}