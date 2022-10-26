/* Copyright 2005-2006, Voltage Security, all rights reserved.
 */

#include "vibe.h"

#ifndef _VIBE_SAMPLE_IMPL_H
#define _VIBE_SAMPLE_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Set these #defines to the memory Impl and associated info you want
 * to use.
 * <p>You may have to change these values based on the platform (Windows
 * vs. Linux, for example) or you may want to change it based on debug
 * vs. production.
 * <p>Make sure the MEMORY_INFO is defined and that it matches the
 * MEMORY_IMPL you choose.
 * <p>If you use debug memory, you may want to reset the first field of
 * the gDebugMemInfo. That field is the amount of memory set aside for
 * the Impl's use (not the total memory the Impl is allowed to
 * allocate). The more you do, the more space the Impl will need to do
 * its work. For ZDM operations, it may need up to 100K.
 *
 * <pre>
 * <code>
 *
 * Suggested definitions:
 *
 * 1. Debug Memory
 *   #define MEMORY_IMPL VtMemoryImplDebug
 *   VtDebugMemoryInfo gDebugMemInfo = { 8192, VOLT_DEBUG_MEM_FLAG_PRINTF };
 *   #define MEMORY_INFO (Pointer)&gDebugMemInfo
 * 2. Windows Production Memory
 *   #define MEMORY_IMPL VtMemoryImplWin32
 *   #define MEMORY_INFO (Pointer)0
 * 3. Available for Linux
 *   #define MEMORY_IMPL VtMemoryImplDefault
 *   #define MEMORY_INFO (Pointer)0
 *
 * </code>
 * </pre>
 */
#define MEMORY_IMPL VtMemoryImplDebug
VtDebugMemoryInfo gDebugMemInfo = {262144, VOLT_DEBUG_MEM_FLAG_PRINTF };
#define MEMORY_INFO (Pointer)&gDebugMemInfo

/* Set these #defines to the thread Impl and associated info you want
 * to use.
 * <p>You may have to change these values based on the platform
 * (Windows vs. Linux, for example).
 * <p>Make sure the THREAD_INFO is defined and that it matches the
 * THREAD_IMPL you choose.
 *
 * <pre>
 * <code>
 *
 * Suggested definitions:
 *
 * 1. Single Threaded
 *   #define THREAD_IMPL VtThreadImplDefaultSingle
 *   #define THREAD_INFO (Pointer)0
 * 2. Windows Multi-threaded
 *   #define THREAD_IMPL VtThreadImplWin32Multi
 *   #define THREAD_INFO (Pointer)0
 * 3. Multi-threaded available for Linux
 *   #define THREAD_IMPL VtThreadImplPThread
 *   #define THREAD_INFO (Pointer)0
 *
 * </code>
 * </pre>
 */
#define THREAD_IMPL VtThreadImplWin32Multi
#define THREAD_INFO (Pointer)0

/* Set these #defines to the Autoseed Impl and associated info you want
 * to use.
 * <p>If possible, Voltage recommends using the CAPI autoseed Impl. If
 * you are on a Windows environment, both VtRandomImplCAPIAutoSeed and
 * VtRandomImplAutoSeed will use CAPI. However, if you are not on a
 * Windows environment, or you are but CAPI is not available, you will
 * want to use VtRandomImplOpenSSLAutoSeed.
 * <p>You may have to change these values based on the platform
 * (Windows vs. Linux, for example).
 * <p>Make sure the AUTOSEED_INFO is defined and that it matches the
 * AUTOSEED_IMPL you choose.
 *
 * <pre>
 * <code>
 *
 * Suggested definitions:
 *
 * 1. Platform default (Windows: CAPI; Linux: OpenSSL)
 *   #define AUTOSEED_IMPL VtRandomImplAutoSeed
 *   #define AUTOSEED_INFO (Pointer)0
 * 2. CAPI
 *   #define AUTOSEED_IMPL VtRandomImplCAPIAutoSeed
 *   #define AUTOSEED_INFO (Pointer)0
 * 3. OpenSSL
 *   #define AUTOSEED_IMPL VtRandomImplOpenSSLAutoSeed
 *   #define AUTOSEED_INFO (Pointer)0
 *
 * </code>
 * </pre>
 */
#define AUTOSEED_IMPL VtRandomImplAutoSeed
#define AUTOSEED_INFO (Pointer)0

/* Set these #defines to the mpCtx Impl and associated info you want
 * to use.
 * <p>You may have to change these values based on the platform
 * (Windows vs. Linux, for example).
 * <p>Make sure the MP_INT_INFO is defined and that it matches the
 * MP_INT_IMPL you choose.
 *
 * <pre>
 * <code>
 *
 * Suggested definitions:
 *
 * 1. OpenSSL BigNum
 *   #define MP_INT_IMPL VtMpIntImplOpenSSL
 *   #define MP_INT_INFO (Pointer)0
 * 2. Gnu's GMP
 *   #define MP_INT_IMPL VtMpIntImplGMP
 *   #define MP_INT_INFO (Pointer)0
 *
 * </code>
 * </pre>
 */
#define MP_INT_IMPL VtMpIntImplOpenSSL
#define MP_INT_INFO (Pointer)0

/* Set these #defines to the file Impl and associated info you want to
 * use.
 * <p>You may have to change these values based on the platform
 * (Windows vs. Linux, for example).
 * <p>Make sure the FILE_INFO is defined and that it matches the
 * FILE_IMPL you choose.
 *
 * <pre>
 * <code>
 *
 * Suggested definitions:
 *
 * 1. Windows
 *   #define FILE_IMPL VtFileImplWin32
 *   #define FILE_INFO (Pointer)0
 * 2. Linux
 *   #define FILE_IMP VtFileImplPOSIX
 *   #define FILE_INFO (Pointer)0
 * 3. Locking, for multithreaded apps.
 *   #define FILE_IMPL VtFileImplLocking
 *   VtFileImplInfoLocking lockingInfo =
 *     { VtFileImplWin32, (Pointer)0, VT_FILE_IMPL_LOCKING_SHARED, 120000 };
 *   #define FILE_INFO (Pointer)&lockingInfo
 *
 * </code>
 * </pre>
 */
#ifdef WIN32
#define FILE_IMPL VtFileImplWin32
#define FILE_INFO (Pointer)0
#else
#define FILE_IMPL VtFileImplPOSIX
#define FILE_INFO (Pointer)0
#endif

/* Set these #defines to the policy Impl and associated info you want
 * to use.
 * <p>You may have to change these values based on the platform
 * (Windows vs. Linux, for example).
 * <p>Make sure the POLICY_INFO is defined and that it matches the
 * POLICY_IMPL you choose.
 *
 * <pre>
 * <code>
 *
 * Suggested definitions:
 *
 * 1. Download policy: Windows
 *   #define POLICY_IMPL VtPolicyImplXmlURLWinINetTime
 *   VtWinINetHttpTimeInfo gPolicyInfo =
 *     { "https://voltage-ps-0000.developer.voltage.com/v2/clientPolicy.xml",
 *       10000 };
 *   #define POLICY_INFO (Pointer)&gPolicyInfo
 *
 * 2. Download policy: Linux
 *    NOTE: The Curl package needs a store of trusted certs
 *    (trustStore). The toolkit comes with a possible trustStore in the
 *    support directory. This policyInfo expects the working directory
 *    of the sample to be the directory where the sample code is
 *    located.
 *   #define POLICY_IMPL VtPolicyImplXmlURLCurlTime
 *   VtCurlHttpTimeInfo gPolicyInfo =
 *    {"https://voltage-ps-0000.developer.voltage.com/v2/clientPolicy.xml",
 *     ../support/trustStore, 10000};      
 *   #define POLICY_INFO (Pointer)&gPolicyInfo;
 *
 * 3. Load policy from a file
 *    NOTE: These #defines assume you have built a fileCtx and have
 *    loaded it into the libCtx. Furthermore, it assumes the working
 *    directory of the project is the sample directory.
 *   #define POLICY_IMPL VtPolicyImplXmlFile
 *   VtFileCtxUseInfo gPolicyInfo =
 *     { (VtFileCtx)0, "sampleclientpolicy.xml" };
 *   #define POLICY_INFO (Pointer)&gPolicyInfo
 *
 * </code>
 * </pre>
 */
#ifdef WIN32
#define POLICY_IMPL VtPolicyImplXmlFile
VtFileCtxUseInfo gPolicyInfo = {
  (VtFileCtx)0, (unsigned char *)"..\\sampleclientpolicy.xml"};
#define POLICY_INFO (Pointer)&gPolicyInfo
#else
#define POLICY_IMPL VtPolicyImplXmlFile
VtFileCtxUseInfo gPolicyInfo = {
  (VtFileCtx)0, "sampleclientpolicy.xml"};
#define POLICY_INFO (Pointer)&gPolicyInfo
#endif

/* Set these #defines to the storage Impl and associated info you want
 * to use.
 * <p>There is currently only one storage Impl available (although you
 * will need to set the storage ctx with at least one provider).
 *
 * <pre>
 * <code>
 *
 * Suggested definitions:
 *
 * 1. Basic
 *   #define STORAGE_IMPL VtStorageImplBasic
 *   #define STORAGE_INFO (Pointer)0
 *
 * </code>
 * </pre>
 */
#define STORAGE_IMPL VtStorageImplBasic
#define STORAGE_INFO (Pointer)0

/* Set these #defines to the storage Provider and associated info you
 * want to use.
 * <p>You may have to change these values based on the platform
 * (Windows vs. Linux, for example).
 * <p>Make sure the STORAGE_PROVIDER_INFO is defined and that it
 * matches the STORAGE_PROVIDER you choose.
 *
 * <pre>
 * <code>
 *
 * Suggested definitions:
 *
 * 1. Windows file (store encrypted using Windows protected storage)
 *    NOTE: These definitions assume you have a fileCtx built and it is
 *    loaded into the libCtx.
 *   #define STORAGE_PROVIDER VtStorageFileWin32
 *   VtFileCtxUseInfo gStorageInfo = { (VtFileCtx)0, ".\\samplestore" };
 *   #define STORAGE_PROVIDER_INFO (Pointer)&gStorageInfo
 * 2. Windows file compatible with current client (stored encrypted
 *    using Windows protected storage, default location)
 *    NOTE: These definitions assume you have a fileCtx built and it is
 *    loaded into the libCtx.
 *   #define STORAGE_PROVIDER_CLIENT VtStorageFileWin32Client
 *   VtWin32ClientStorageInfo gStorageInfo =
 *     { (VtFileCtx)0, (VtPasswordManager)0, (Pointer)0 };
 *   #define STORAGE_PROVIDER_INFO_CLIENT (Pointer)&gStorageInfo
 *
 * 3. Linux (items not stored encrypted, uses OS file permissions)
 *    NOTE: These definitions assume you have a fileCtx built and it is
 *    loaded into the libCtx.
 *   #define STORAGE_PROVIDER VtStorageFileUnix
 *   VtFileCtxUseInfo gStorageInfo = { (VtFileCtx)0, "./samplestore" };
 *   #define STORAGE_PROVIDER_INFO (Pointer)&gStorageInfo
 *
 * </code>
 * </pre>
 */
#ifdef WIN32
#define STORAGE_PROVIDER VtStorageFileWin32
VtFileCtxUseInfo gStorageInfo = { (VtFileCtx)0, (unsigned char *)".\\samplestore" };
#else
#define STORAGE_PROVIDER VtStorageFileUnix
VtFileCtxUseInfo gStorageInfo = { (VtFileCtx)0, "./samplestore" };
#endif

#define STORAGE_PROVIDER_INFO (Pointer)&gStorageInfo

/* Set these #defines to the transport Impl and associated info you
 * want to use.
 * <p>You may have to change these values based on the platform
 * (Windows vs. Linux, for example).
 * <p>Make sure the TRANSPORT_INFO is defined and that it matches the
 * TRANSPORT_IMPL you choose.
 *
 * <pre>
 * <code>
 *
 * Suggested definitions:
 *
 * 1. Windows Standard: WinHttp
 *   #define TRANSPORT_IMPL VtTransportImplHttpsWinHttp
 *   VtWinHttpTransportInfo gTransportInfo =
 *   { (Pointer)0,
 *     VT_ASYNC_RESPONSE_CONTINUE | VT_PREVIOUS_RESPONSE_CONTINUE,
 *     (VtFileCtx)0, 10000 };
 *   #define TRANSPORT_INFO (Pointer)&gTransportInfo
 *
 * 2. Windows Standard: WinINet
 *   #define TRANSPORT_IMPL VtTransportImplHttpsWinINet
 *   VtWinHttpTransportInfo gTransportInfo =
 *   { (Pointer)0,
 *     VT_ASYNC_RESPONSE_CONTINUE | VT_PREVIOUS_RESPONSE_CONTINUE,
 *     (VtFileCtx)0 };
 *   #define TRANSPORT_INFO (Pointer)&gTransportInfo
 *
 * 3. Windows Delegated Adapter
 *   #define TRANSPORT_IMPL VtTransportImplDelegatedWinINet
 *   #define TRANSPORT_INFO (Pointer)0
 *
 * 4. Linux Standard
 *    NOTE: The Curl package needs a store of trusted certs
 *    (trustStore). The toolkit comes with a possible trustStore in the
 *    support directory. This policyInfo expects the working directory
 *    of the sample to be the directory where the sample code is
 *    located.
 *   #define TRANSPORT_IMPL VtTransportImplHttpsCurl
 *   VtCurlTransportInfo gTransportInfo =
 *   { (Pointer)0, 
 *     VT_ASYNC_RESPONSE_CONTINUE | VT_PREVIOUS_RESPONSE_CONTINUE,
 *     (VtFileCtx)0, "../support/trustStore" };
 *   #define TRANSPORT_INFO (Pointer)&gTransportInfo
 *
 * 5. Linux Delegated Adapter
 *    NOTE: The Curl package needs a store of trusted certs
 *    (trustStore). The toolkit comes with a possible trustStore in the
 *    support directory. This policyInfo expects the working directory
 *    of the sample to be the directory where the sample code is
 *    located.
 *   #define TRANSPORT_IMPL VtTransportImplDelegatedCurl
 *   char *gTrustStore = "../support/trustStore";
 *   #define DELEGATED_TRANSPORT_INFO (Pointer)gTrustStore
 *
 * </code>
 * </pre>
 */
#ifdef WIN32
#define TRANSPORT_IMPL VtTransportImplHttpsWinHttp
   VtWinHttpTransportInfo gTransportInfo =
   { (Pointer)0,
     VT_ASYNC_RESPONSE_CONTINUE | VT_PREVIOUS_RESPONSE_CONTINUE,
     (VtFileCtx)0, 10000 };
#else
#define TRANSPORT_IMPL VtTransportImplHttpsCurl
VtCurlTransportInfo gTransportInfo =
  { (Pointer)0, 
    VT_ASYNC_RESPONSE_CONTINUE | VT_PREVIOUS_RESPONSE_CONTINUE,
    (VtFileCtx)0, "../support/trustStore" };
#endif

#define TRANSPORT_INFO (Pointer)&gTransportInfo

/* Add these #defines, one of the sample programs uses the delegated
 * transport, and no other.
 */
#ifdef WIN32
#define DELEGATED_TRANSPORT_IMPL VtTransportImplDelegatedWinHttp
#define DELEGATED_TRANSPORT_INFO (Pointer)0
#else
#define DELEGATED_TRANSPORT_IMPL VtTransportImplDelegatedCurl
char *gTrustStore = "../support/trustStore";
#define DELEGATED_TRANSPORT_INFO (Pointer)gTrustStore
#endif

/* Set these #defines to the cert verify Impl and associated info you
 * want to use.
 * <p>You may have to change these values based on the platform
 * (Windows vs. Linux, for example).
 * <p>Make sure the CERT_VERIFY_INFO is defined and that it matches the
 * CERT_VERIFY_IMPL you choose.
 *
 * <pre>
 * <code>
 *
 * Suggested definitions:
 *
 * 1. All platforms
 *   #define CERT_VERIFY_IMPL VtCertVerifyImplBasic
 *   #define CERT_VERIFY_INFO (Pointer)0
 *
 * </code>
 * </pre>
 */
#define CERT_VERIFY_IMPL VtCertVerifyImplBasic
#define CERT_VERIFY_INFO (Pointer)0

/* Set these #defines to the VerifyFailureList Impl and associated info
 * you want to use.
 * <p>You may have to change these values based on the platform
 * (Windows vs. Linux, for example).
 * <p>Make sure the VFY_FAIL_LIST_INFO is defined and that it matches the
 * VFY_FAIL_LIST_IMPL you choose.
 *
 * <pre>
 * <code>
 *
 * Suggested definitions:
 *
 * 1. All platforms
 *   #define VFY_FAIL_LIST_IMPL VtVerifyFailureListImplBasic
 *   #define VFY_FAIL_LIST_INFO (Pointer)0
 *
 * </code>
 * </pre>
 */
#define VFY_FAIL_LIST_IMPL VtVerifyFailureListImplBasic
#define VFY_FAIL_LIST_INFO (Pointer)0

#ifdef __cplusplus
}
#endif

#endif /* _VIBE_SAMPLE_IMPL_H */
