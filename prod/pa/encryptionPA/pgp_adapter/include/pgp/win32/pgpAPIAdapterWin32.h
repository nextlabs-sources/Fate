/*____________________________________________________________________________
 *	Copyright (C) 2003 PGP Corporation
 *	All rights reserved.
 *            
 *	$Id$
 *____________________________________________________________________________*/
#ifndef Included_pgpAPIAdapterWin32_h   /* [ */
#define Included_pgpAPIAdapterWin32_h

#include "pgpKeys.h"
#include "pgpKeyServer.h"

#include "pgpMemoryMgr.h"
#include "pgpUtilities.h"
#include "pgpPFLErrors.h"
#include "pgpFileSpec.h"

#include "pgpAPIAdapter.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if !PGP_WIN32
#error	"This file is intended for Win32 (the only platform with sizeof(wchar_t)==2)"
#endif

#if PGP_UNICODE
#define SDK_UNICODE 1
#define SDK_UI_UNICODE 1
#define SDK_NET_UNICODE 1
#endif

PGP_BEGIN_C_DECLARATIONS

#ifndef SDK_UNICODE
#define PGPGetErrorString PGPGetErrorStringU8
#else
#define PGPGetErrorString PGPGetErrorStringU16
#endif

#ifndef SDK_UNICODE
#define PGPGetPGPsdkVersionString PGPGetPGPsdkVersionStringU8 
#else
#define PGPGetPGPsdkVersionString PGPGetPGPsdkVersionStringU16
#endif

#ifndef SDK_UNICODE
#define PGPSetContextTempDirectory PGPSetContextTempDirectoryU8
#else
#define PGPSetContextTempDirectory PGPSetContextTempDirectoryU16
#endif

#ifndef SDK_UNICODE
#define PGPGetContextTempDirectory PGPGetContextTempDirectoryU8
#else
#define PGPGetContextTempDirectory PGPGetContextTempDirectoryU16
#endif

#ifndef SDK_UNICODE
#define PGPNewFileSpecFromFullPath PGPNewFileSpecFromFullPathU8 
#else
#define PGPNewFileSpecFromFullPath PGPNewFileSpecFromFullPathU16
#endif

#ifndef SDK_UNICODE
#define PGPGetFullPathFromFileSpec PGPGetFullPathFromFileSpecU8
#else
#define PGPGetFullPathFromFileSpec PGPGetFullPathFromFileSpecU16
#endif

#ifndef SDK_UNICODE
#define PGPRenameFile PGPRenameFileU8
#else
#define PGPRenameFile PGPRenameFileU16
#endif

#ifndef SDK_UNICODE
#define PGPOPassphraseBuffer PGPOPassphraseBufferU8
#else
#define PGPOPassphraseBuffer PGPOPassphraseBufferU16
#endif

#ifndef SDK_UNICODE
#define PGPOPassphrase PGPOPassphraseU8 
#else
#define PGPOPassphrase PGPOPassphraseU16
#endif

#ifndef SDK_UNICODE
#define PGPOPGPMIMEEncoding PGPOPGPMIMEEncodingU8 
#else
#define PGPOPGPMIMEEncoding PGPOPGPMIMEEncodingU16
#endif

#ifndef SDK_UNICODE
#define PGPOKeyGenName PGPOKeyGenNameU8 
#else
#define PGPOKeyGenName PGPOKeyGenNameU16
#endif

#ifndef SDK_UNICODE
#define PGPOPreferredKeyServer PGPOPreferredKeyServerU8 
#else
#define PGPOPreferredKeyServer PGPOPreferredKeyServerU16
#endif

#ifndef SDK_UNICODE
#define PGPOCommentString PGPOCommentStringU8 
#else
#define PGPOCommentString PGPOCommentStringU16
#endif

#ifndef SDK_UNICODE
#define PGPOVersionString PGPOVersionStringU8 
#else
#define PGPOVersionString PGPOVersionStringU16
#endif

#ifndef SDK_UNICODE
#define PGPOFileNameString PGPOFileNameStringU8
#else
#define PGPOFileNameString PGPOFileNameStringU16
#endif

#ifndef SDK_UNICODE
#define PGPOSigRegularExpression PGPOSigRegularExpressionU8
#else
#define PGPOSigRegularExpression PGPOSigRegularExpressionU16 
#endif

#ifndef SDK_UNICODE
#define PGPGetKeyDBObjDataProperty PGPGetKeyDBObjDataPropertyU8
#else
#define PGPGetKeyDBObjDataProperty PGPGetKeyDBObjDataPropertyU16 
#endif

#ifndef SDK_UNICODE
#define PGPGetKeyDBObjAllocatedDataProperty PGPGetKeyDBObjAllocatedDataPropertyU8
#else
#define PGPGetKeyDBObjAllocatedDataProperty PGPGetKeyDBObjAllocatedDataPropertyU16 
#endif

#ifndef SDK_UNICODE
#define PGPNewKeyDBObjDataFilter PGPNewKeyDBObjDataFilterU8
#else
#define PGPNewKeyDBObjDataFilter PGPNewKeyDBObjDataFilterU16 
#endif

#ifndef SDK_UNICODE
#define PGPLDAPQueryFromFilter PGPLDAPQueryFromFilterU8
#else
#define PGPLDAPQueryFromFilter PGPLDAPQueryFromFilterU16 
#endif

#ifndef SDK_UNICODE
#define PGPLDAPX509QueryFromFilter PGPLDAPX509QueryFromFilterU8
#else
#define PGPLDAPX509QueryFromFilter PGPLDAPX509QueryFromFilterU16
#endif

#ifndef SDK_UNICODE
#define PGPHKSQueryFromFilter PGPHKSQueryFromFilterU8
#else
#define PGPHKSQueryFromFilter PGPHKSQueryFromFilterU16
#endif

#ifndef SDK_UNICODE
#define PGPNetToolsCAHTTPQueryFromFilter PGPNetToolsCAHTTPQueryFromFilterU8
#else
#define PGPNetToolsCAHTTPQueryFromFilter PGPNetToolsCAHTTPQueryFromFilterU16
#endif

#ifndef SDK_UNICODE
#define PGPAddUserID PGPAddUserIDU8
#else
#define PGPAddUserID PGPAddUserIDU16 
#endif

#ifndef SDK_UNICODE
#define PGPCompareUserIDStrings PGPCompareUserIDStringsU8
#else
#define PGPCompareUserIDStrings PGPCompareUserIDStringsU16
#endif

#ifndef SDK_UNICODE
#define PGPGetKeyIDString PGPGetKeyIDStringU8
#else
#define PGPGetKeyIDString PGPGetKeyIDStringU16 
#endif

#ifndef SDK_UNICODE
#define PGPNewKeyIDFromString PGPNewKeyIDFromStringU8
#else
#define PGPNewKeyIDFromString PGPNewKeyIDFromStringU16 
#endif

#ifndef SDK_UNICODE
#define PGPSetPKCS11DrvFile PGPSetPKCS11DrvFileU8
#else
#define PGPSetPKCS11DrvFile PGPSetPKCS11DrvFileU16
#endif

#ifndef SDK_UNICODE
#define PGPGetTokenInfoDataProperty PGPGetTokenInfoDataPropertyU8
#else
#define PGPGetTokenInfoDataProperty PGPGetTokenInfoDataPropertyU16
#endif

#ifndef SDK_UNICODE
#define PGPCreateDistinguishedName PGPCreateDistinguishedNameU8
#else
#define PGPCreateDistinguishedName PGPCreateDistinguishedNameU16
#endif

#ifndef SDK_UNICODE
#define PGPGetPrimaryUserIDName PGPGetPrimaryUserIDNameU8
#else
#define PGPGetPrimaryUserIDName PGPGetPrimaryUserIDNameU16
#endif

#ifndef SDK_UNICODE
#define PGPGetHashWordString PGPGetHashWordStringU8
#else
#define PGPGetHashWordString PGPGetHashWordStringU16
#endif

#ifndef SDK_UNICODE
#define PGPSetShareFileUserID PGPSetShareFileUserIDU8
#else
#define PGPSetShareFileUserID PGPSetShareFileUserIDU16
#endif

#ifndef SDK_UNICODE
#define PGPGetShareFileUserID PGPGetShareFileUserIDU8
#else
#define PGPGetShareFileUserID PGPGetShareFileUserIDU16
#endif

#ifndef SDK_UNICODE
#define PGPGetTARCacheObjDataProperty PGPGetTARCacheObjDataPropertyU8
#else
#define PGPGetTARCacheObjDataProperty PGPGetTARCacheObjDataPropertyU16
#endif


/* ----- SDK UI ----- */
#ifndef SDK_UI_UNICODE
#define PGPEstimatePassphraseQuality PGPEstimatePassphraseQualityU8
#else
#define PGPEstimatePassphraseQuality PGPEstimatePassphraseQualityU16
#endif

#ifndef SDK_UI_UNICODE
#define PGPOUIDialogPrompt PGPOUIDialogPromptU8 
#else
#define PGPOUIDialogPrompt PGPOUIDialogPromptU16
#endif

#ifndef SDK_UI_UNICODE
#define PGPOUIWindowTitle PGPOUIWindowTitleU8 
#else
#define PGPOUIWindowTitle PGPOUIWindowTitleU16
#endif

#ifndef SDK_UI_UNICODE
#define PGPOUIOutputPassphrase PGPOUIOutputPassphraseU8 
#else
#define PGPOUIOutputPassphrase PGPOUIOutputPassphraseU16
#endif

#ifndef SDK_UI_UNICODE
#define PGPOUICheckbox PGPOUICheckboxU8 
#else
#define PGPOUICheckbox PGPOUICheckboxU16
#endif

#ifndef SDK_UI_UNICODE
#define PGPOUIPopupList PGPOUIPopupListU8 
#else
#define PGPOUIPopupList PGPOUIPopupListU16
#endif


/* ----- SDK Network ----- */
#ifndef SDK_NET_UNICODE
#define PGPONetURL PGPONetURLU8 
#else
#define PGPONetURL PGPONetURLU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPONetHostName PGPONetHostNameU8 
#else
#define PGPONetHostName PGPONetHostNameU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPOKeyServerKeyStoreDN PGPOKeyServerKeyStoreDNU8 
#else
#define PGPOKeyServerKeyStoreDN PGPOKeyServerKeyStoreDNU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPGetKeyServerHostName PGPGetKeyServerHostNameU8 
#else
#define PGPGetKeyServerHostName PGPGetKeyServerHostNameU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPGetKeyServerPath PGPGetKeyServerPathU8 
#else
#define PGPGetKeyServerPath PGPGetKeyServerPathU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPGetLastKeyServerErrorString PGPGetLastKeyServerErrorStringU8 
#else
#define PGPGetLastKeyServerErrorString PGPGetLastKeyServerErrorStringU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPGetProxyServer PGPGetProxyServerU8
#else
#define PGPGetProxyServer PGPGetProxyServerU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPGetProxyHost PGPGetProxyHostU8
#else
#define PGPGetProxyHost PGPGetProxyHostU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPMakeProxyAuthString PGPMakeProxyAuthStringU8
#else
#define PGPMakeProxyAuthString PGPMakeProxyAuthStringU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPRetrieveOCSPStatus PGPRetrieveOCSPStatusU8
#else
#define PGPRetrieveOCSPStatus PGPRetrieveOCSPStatusU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPskepSendShares PGPskepSendSharesU8
#else
#define PGPskepSendShares PGPskepSendSharesU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPskepReceiveShares PGPskepReceiveSharesU8
#else
#define PGPskepReceiveShares PGPskepReceiveSharesU16
#endif

#ifndef SDK_NET_UNICODE
#define PGPMakeReconstruction PGPMakeReconstructionU8
#else
#define PGPMakeReconstruction PGPMakeReconstructionU16
#endif

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpAPIAdapterWin32_h   */


/*__Editor_settings____
 *
 *	Local Variables:
 *	tab-width: 4
 *	End:
 *	vi: ts=4 sw=4
 *	vim: si
 *_____________________*/

