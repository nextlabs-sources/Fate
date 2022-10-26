// vibe_adapter.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "vibe_adapter.h"

// #define CRTDBG_MAP_ALLOC
// #include <stdlib.h>
// #include <crtdbg.h>

#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

/* Any app that uses the VIBE Toolkit must include either vibecrypto.h
* or vibe.h (vibe.h includes vibecrypto.h).
*/
#include "vibe.h"

/* For platform specific implementations and their associated infos.
*/
// #include "sampleimpl.h"

/* For the sample code, there are several utilities to perform platform
* specific operations.
*/
#include "sampleutil.h"

#include "log.h"

#define EXCLUDED_LIST_COUNT 5
int list[EXCLUDED_LIST_COUNT] = {
	VT_ERROR_BUFFER_TOO_SMALL,
	VT_ERROR_CHOOSE_RECIPIENT,
	VT_ERROR_ASYNC_DOWNLOAD,
	VT_ERROR_DOWNLOAD_PENDING,
	VT_ERROR_DOWNLOAD_PREVIOUS
};

#define POLICY_XML_PATH		"Voltage\\ClientPolicy\\clientPolicy.xml"
#define KEY_STORE_PATH		"Voltage\\Store"

static int BuildSecureFileByParts ( VtLibCtx libCtx, char *srcDirectory, char *dstDirectory, 
								   char *szSrcFileName, char *szDstBaseFileName, char *extention, 
								   std::wstring &wstrSenderEmail, StringVector &vecEmails);
static int ValidateEmailAddress(LPEncryptionAdapterData lpData);
static LPWSTR GetBaseFileName(LPWSTR lpwzFileName);

#ifdef _MANAGED
#pragma managed(push, off)
#endif

HINSTANCE g_hInstance = NULL;
WCHAR g_wzVoltageAdapterPath[MAX_PATH];
static CRITICAL_SECTION s_criticalSection;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	UNUSED(lpReserved);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hInstance = (HINSTANCE)hModule;
			GetModuleFileNameW(hModule, g_wzVoltageAdapterPath, MAX_PATH);
			InitializeCriticalSection(&s_criticalSection);
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		g_hInstance = NULL;
		DeleteCriticalSection(&s_criticalSection);
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


ADAPTER_BASE_API EA_Error WINAPI Encrypt(EncryptionAdapterData *lpData)
{
	int status = 0;
	VtLibCtx libCtx = (VtLibCtx)0;
	VtIBECacheCtx ibeCacheCtx = (VtIBECacheCtx)0;
	VtMpIntCtx mpCtx = (VtMpIntCtx)0;
	VtFileCtx fileCtx = (VtFileCtx)0;
	VtPolicyCtx policyCtx = (VtPolicyCtx)0;
	VtStorageCtx storageCtx = (VtStorageCtx)0;
	VtTransportCtx transportCtx = (VtTransportCtx)0;
	VtCertVerifyCtx certVerifyCtx = (VtCertVerifyCtx)0;
	VtRandomObject random = (VtRandomObject)0;
	VtErrorCtx errorCtx = (VtErrorCtx)0;

	VtFileCtxUseInfo gStorageInfo;
	VtWinHttpTransportInfo gTransportInfo;
	VtFileCtxUseInfo gPolicyInfo;

	VtExcludedErrors excludedErrors;
	char szDstDirectory[MAX_PATH];
	char szSrcDirectory[MAX_PATH];
	char szFileName[MAX_PATH];
	char szBaseFileName[MAX_PATH];
	char *pszFileName = NULL;
	char szAppData[MAX_PATH];
	char szPolicyXmlPath[MAX_PATH];
	char szKeyStore[MAX_PATH];
	struct _stat stat_buffer;
	BOOL bInited = FALSE;
	WCHAR wzActualDstFile[MAX_PATH];
	char szActualDstFile[MAX_PATH];
	WCHAR wzStringResource[512];

	if (!lpData)
	{
		return EA_E_NULLPTR;
	}

	if (lpData->wstrSrcFile.empty() || lpData->wstrBaseFileName.empty())
	{
		LoadStringW(g_hInstance, IDS_STR_NOSRC, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_BADPARAM;
	}

	lpData->wstrActualDstFile = L"";

	if (lpData->encryptContext.bSymm)
	{
		LoadStringW(g_hInstance, IDS_STR_SYMM_UNSUPPORTED, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_UNSUPPORTED;
	}

	if (lpData->encryptContext.wstrEncrypterInfo.empty())
	{
		DPW((L"EncryptionAdapter: sender's email should not be empty!\n"));
		LoadStringW(g_hInstance, IDS_STR_NO_ENCRYPTER, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_BADPARAM;
	}

	if (ValidateEmailAddress(lpData) != 0)
	{
		DPW((L"EncryptionAdapter: Contain invalid email address!\n"));
		LoadStringW(g_hInstance, IDS_STR_WRONG_EMAIL, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_BADEMAIL;
	}

	if (_wstat(lpData->wstrSrcFile.c_str(), &stat_buffer) != 0)
	{
		DPW((L"EncryptionAdapter: source file: %s does not exist!\n", lpData->wstrSrcFile));

		WCHAR wzError[1024];

		LoadStringW(g_hInstance, IDS_STR_SRC_NOTEXIST, wzStringResource, 512);
		_snwprintf_s(wzError, 1024, _TRUNCATE, wzStringResource, lpData->wstrSrcFile.c_str());
		lpData->wstrErrorInfo = wzError;

		return EA_E_WRONGPATH;
	}

	WideCharToMultiByte(CP_ACP, 0, lpData->wstrSrcFile.c_str(), -1, szSrcDirectory, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, lpData->wstrDstFolder.c_str(), -1, szDstDirectory, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, lpData->wstrBaseFileName.c_str(), -1, szBaseFileName, MAX_PATH, NULL, NULL);

	pszFileName = strrchr(szSrcDirectory, '\\');
	if (!pszFileName)
	{
		pszFileName = strrchr(szSrcDirectory, '/');
	}

	if (!pszFileName)
	{
		strncpy_s(szFileName, MAX_PATH, szSrcDirectory, _TRUNCATE);
		memset(szSrcDirectory, 0, MAX_PATH);
	}
	else
	{
		strncpy_s(szFileName, MAX_PATH, pszFileName+1, _TRUNCATE);
		*pszFileName = 0;
	}

	memset(szAppData, 0, MAX_PATH);
	SHGetSpecialFolderPathA(NULL, szAppData, CSIDL_APPDATA, FALSE);
	_snprintf_s(szPolicyXmlPath, MAX_PATH, _TRUNCATE, "%s\\%s", szAppData, POLICY_XML_PATH);
	_snprintf_s(szKeyStore, MAX_PATH, _TRUNCATE, "%s\\%s", szAppData, KEY_STORE_PATH);
	if (_stat(szPolicyXmlPath, &stat_buffer) != 0)
	{
		DPW((L"EncryptionAdapter: Voltage Application should be installed first!\n"));

		LoadStringW(g_hInstance, IDS_STR_NOTINSTALLED, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;

		return EA_E_UNSUPPORTED;
	}

	int iCondition = 0;
	do
	{
		/* The first thing you need to do is build a library context.
		*/
		status = VtCreateLibCtx (
			VtMemoryImplWin32, (Pointer)0, VtThreadImplWin32Multi, (Pointer)0, &libCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtCreateLibCtx failed(status=%d)!\n", status));
			break;
		}
		/* Build an error ctx to help trace any errors.
		*/
		status = VtCreateErrorCtx (
			libCtx, VtErrorCtxImplBasic, (Pointer)0, &errorCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtCreateErrorCtx failed(status=%d)!\n", status));
			break;
		}

		/* Some errors we don't want to log.
		*/
		excludedErrors.excludedErrors = list;
		excludedErrors.count = EXCLUDED_LIST_COUNT;
		status = VtSetErrorCtxParam (
			errorCtx, VtErrorCtxParamExcludedErrors, (Pointer)&excludedErrors);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtSetErrorCtxParam failed(status=%d)!\n", status));
			break;
		}

		/* The only way to take advantage of the errorCtx is to load it
		* into the libCtx.
		*/
		status = VtSetLibCtxParam (
			libCtx, VtLibCtxParamErrorCtx, (Pointer)errorCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtSetLibCtxParam failed(status=%d)!\n", status));
			break;
		}

		/* We're going to be doing IBE work, so we might want to load the
		* IBE Cache Ctx. Loading this increases code size and might
		* increase memory usage. But it will speed up IBE operations, and
		* if we do lots of IBE operations, it might actually reduce memory
		* size.
		*/
		status = VtCreateIBECacheCtx (
			libCtx, VtIBECacheCtxImplBasic, (Pointer)0, &ibeCacheCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtCreateIBECacheCtx failed(status=%d)!\n", status));
			break;
		} 

		/* The only way to take advantage of the IBE Cache is to load it
		* into the libCtx.
		*/
		status = VtSetLibCtxParam (
			libCtx, VtLibCtxParamIBECacheCtx, (Pointer)ibeCacheCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtSetLibCtxParam failed(status=%d)!\n", status));
			break;
		}

		/* For this sample we'll need an mpCtx. It will be used to perform
		* operations on IBE public keys, represented as identity objects.
		* Load the mpCtx into the libCtx so we don't have to pass it as an
		* argument to subroutines that will need an mpCtx.
		*/
		status = VtCreateMpIntCtx (
			libCtx, VtMpIntImplOpenSSL, (Pointer)0, &mpCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtCreateMpIntCtx failed(status=%d)!\n", status));
			break;
		}

		status = VtSetLibCtxParam (
			libCtx, VtLibCtxParamMpCtx, (Pointer)mpCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtSetLibCtxParam failed(status=%d)!\n", status));
			break;
		}

		/* For this sample we'll need a random object. The simplest way to
		* build one is to use an autoseed Impl. There are a number of
		* autoseeding Impls, but some may be platform-dependent.
		*/
		status = VtCreateRandomObject (
			libCtx, VtRandomImplAutoSeed, (Pointer)0, &random);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtCreateRandomObject failed(status=%d)!\n", status));
			break;
		}

		status = VtSetLibCtxParam (
			libCtx, VtLibCtxParamRandomObj, (Pointer)random);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtSetLibCtxParam failed(status=%d)!\n", status));
			break;
		}

		/* Our policy and storage providers will need a file context. Build
		* one and load it into the libCtx.
		*/
		status = VtCreateFileCtx (
			libCtx, VtFileImplWin32, (Pointer)0, &fileCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtCreateFileCtx failed(status=%d)!\n", status));
			break;
		}

		status = VtSetLibCtxParam (
			libCtx, VtLibCtxParamFileCtx, (Pointer)fileCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtSetLibCtxParam failed(status=%d)!\n", status));
			break;
		}

		/* We will need a client policy to work with identities.
		* Load it into the libCtx so we don't have to pass it around as an
		* argument.
		*/
		gPolicyInfo.fileCtx = (VtFileCtx)0;
		gPolicyInfo.path = (unsigned char *)szPolicyXmlPath;
		status = VtCreatePolicyCtx (
			libCtx, VtPolicyImplXmlFile, (Pointer)&gPolicyInfo, &policyCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtCreatePolicyCtx failed(status=%d)!\n", status));
			break;
		}

		status = VtSetLibCtxParam (
			libCtx, VtLibCtxParamPolicyCtx, (Pointer)policyCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtSetLibCtxParam failed(status=%d)!\n", status));
			break;
		}

		/* For this sample, we'll use a storage ctx.
		*/
		status = VtCreateStorageCtx (
			libCtx, VtStorageImplBasic, (Pointer)0, &storageCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtCreateStorageCtx failed(status=%d)!\n", status));
			break;
		}

		/* Once we build the storage ctx, we have to add one or more
		* providers. Each storage ctx can contain multiple providers. In
		* this way, when searching for keys or other elements, the ctx can
		* look in more than one location.
		*/
		gStorageInfo.fileCtx = (VtFileCtx)0;
		gStorageInfo.path = (unsigned char *)szKeyStore;
		status = VtAddStorageProvider (
			storageCtx, VtStorageFileWin32, (Pointer)&gStorageInfo);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtAddStorageProvider failed(status=%d)!\n", status));
			break;
		}

		status = VtSetLibCtxParam (
			libCtx, VtLibCtxParamStorageCtx, (Pointer)storageCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtSetLibCtxParam failed(status=%d)!\n", status));
			break;
		}

		/* We'll need a transport ctx. Once again, load it into the libCtx
		* so we don't have to worry about it any more.
		*/
		gTransportInfo.uiHandle = (Pointer)0;
		gTransportInfo.asyncFlag = VT_ASYNC_RESPONSE_CONTINUE | VT_PREVIOUS_RESPONSE_CONTINUE;
		gTransportInfo.fileCtx = (VtFileCtx)0;
		gTransportInfo.timeout = 10000;
		
		status = VtCreateTransportCtx (
			libCtx, VtTransportImplHttpsWinHttp, (Pointer)&gTransportInfo, &transportCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtCreateTransportCtx failed(status=%d)!\n", status));
			break;
		}

		status = VtSetLibCtxParam (
			libCtx, VtLibCtxParamTransportCtx, (Pointer)transportCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtSetLibCtxParam failed(status=%d)!\n", status));
			break;
		}

		/* Later on we'll verify a signature on a message. The public key
		* we'll use to verify the signature will come from a cert. In
		* order to verify the validity of the cert, we'll need a
		* CertVerifyCtx.
		*/
		status = VtCreateCertVerifyCtx (
			libCtx, VtCertVerifyImplBasic, (Pointer)0, &certVerifyCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtCreateCertVerifyCtx failed(status=%d)!\n", status));
			break;
		}

		status = VtSetLibCtxParam (
			libCtx, VtLibCtxParamCertVerifyCtx, (Pointer)certVerifyCtx);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: VtSetLibCtxParam failed(status=%d)!\n", status));
			break;
		}

		bInited = TRUE;

#if 1
		status = BuildSecureFileByParts(libCtx, szSrcDirectory, szDstDirectory, szFileName, szBaseFileName, ".vsf",
				lpData->encryptContext.wstrEncrypterInfo, lpData->encryptContext.vecDecrypterInfos);
		if (status != 0)
		{
			DPW((L"EncryptionAdapter: BuildSecureFileByParts(status=%d)!\n", status));
			break;
		}

		_snprintf_s(szActualDstFile, MAX_PATH, _TRUNCATE, "%s\\%s.vsf", szDstDirectory, szBaseFileName);
		MultiByteToWideChar(CP_ACP, 0, szActualDstFile, -1, wzActualDstFile, MAX_PATH);
		lpData->wstrActualDstFile = wzActualDstFile;
		lpData->wstrEncryptionType = L"Voltage Identity-Based Encryption";
#endif

	} while (iCondition);

#if 1
 	VtDestroyCertVerifyCtx (&certVerifyCtx);
	VtDestroyTransportCtx (&transportCtx);
	VtDestroyStorageCtx (&storageCtx);
	VtDestroyPolicyCtx (&policyCtx);
	VtDestroyFileCtx (&fileCtx);
	VtDestroyRandomObject (&random);
	VtDestroyMpIntCtx (&mpCtx);
	VtDestroyIBECacheCtx (&ibeCacheCtx);
	VtDestroyErrorCtx (&errorCtx);

	/* The last toolkit operation is destroy the libCtx.
	*/
	VtDestroyLibCtx (&libCtx);
#endif

// 	_CrtDumpMemoryLeaks();

	if (!bInited)
	{
		LoadStringW(g_hInstance, IDS_STR_INIT_ERROR, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_INIT;
	}
	else if (status == VT_ERROR_DOWNLOAD_PENDING)
	{
		LoadStringW(g_hInstance, IDS_STR_UNAUTHORIZED, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_KEYUNAUTHORIZED;
	}
	else if (status != 0)
	{
		WCHAR wzError[1024];

		LoadStringW(g_hInstance, IDS_STR_ENCRYPT_FAILED, wzStringResource, 512);
		_snwprintf_s(wzError, 1024, _TRUNCATE, wzStringResource, lpData->wstrSrcFile.c_str());
		lpData->wstrErrorInfo = wzError;
		return EA_E_ENCRYPT;
	}

	return EA_OK;
}

#define INPUT_DATA_SIZE 1024

static int BuildSecureFileByParts ( VtLibCtx libCtx, char *srcDirectory, char *dstDirectory, 
								   char *szSrcFileName, char *szDstBaseFileName, char *extention, 
								   std::wstring &wstrSenderEmail, StringVector &vecEmails)
{
	VtIdentityObject encryptorId = (VtIdentityObject)0;
	VtIdentityList decryptorList = (VtIdentityList)0;
	VtSecureFileObject sfWriter = (VtSecureFileObject)0;
	unsigned char *encrypted = (unsigned char *)0;
	Pointer fileHandleIn = (Pointer)0;
	Pointer fileHandleOut = (Pointer)0;

	unsigned char inputData[INPUT_DATA_SIZE];

	unsigned int fileSize, bytesRead, endOfFile, listIndex;
	unsigned int bufferSize, encryptedLen;

	VtEmailInfo emailInfo;
	int status = 0;
	char szEmailString[256];

	int iCondition = 0;
	do
	{
		/* Open the file we want to encrypt.
		*/
		status = SampleUtilOpenFile (
			libCtx, srcDirectory, szSrcFileName, (char *)0, SAMPLE_UTIL_OPEN_FILE_READ,
			&fileHandleIn);
		if (status != 0)
		{
			DPW((L"BuildSecureFileByParts: SampleUtilOpenFile failed(status=%d)!\n", status));
			break;
		}

		/* Get the file size. We'll need to know later exactly how many
		* bytes we'll be processing.
		*/
		status = SampleUtilFileSize (libCtx, fileHandleIn, &fileSize);
		if (status != 0)
		{
			DPW((L"BuildSecureFileByParts: SampleUtilFileSize failed(status=%d)!\n", status));
			break;
		}

		/* Open the file into which we'll place the encrypted contents.
		*/
		status = SampleUtilOpenFile (
			libCtx, dstDirectory, szDstBaseFileName, extention, SAMPLE_UTIL_OPEN_FILE_WRITE,
			&fileHandleOut);
		if (status != 0)
		{
			DPW((L"BuildSecureFileByParts: SampleUtilOpenFile failed(status=%d)!\n", status));
			break;
		}

		/* Now move on to the SecureFile format work.
		* First, the encryptor is represented as an identity
		* object.
		*/

		/* Build the empty identity object.
		* An identity object will need an mpCtx (for IBE public key
		* operations), so either use VtIdentityImplMpCtx with an mpCtx as
		* the associated info (or NULL if the mpCtx to use is loaded in
		* the libCtx).
		*/
		status = VtCreateIdentityObject (
			libCtx, VtIdentityImplMpCtx, (Pointer)0, &encryptorId);
		if (status != 0)
		{
			DPW((L"BuildSecureFileByParts: VtCreateIdentityObject encryptorId failed(status=%d)!\n", status));
			break;
		}

		/* Set the identity object with the info.
		* It's possible to explicitly set the object with the district,
		* the IBE params, and the encoded identity. Or you can set the
		* object with the schema (the foundation of the identity, such as
		* the email address) and let the toolkit do the rest of the work
		* (using the policy, storage, and transport contexts).
		* To set an identity object, use an IdentityParam with its
		* associated info. For this sample we'll set it with the email
		* address. That Param actually sets with an email address and a
		* time. You can set the emailTime field explicitly, or just call
		* the toolkit's GetTime function for the current time.
		*/
		WideCharToMultiByte(CP_ACP, 0, wstrSenderEmail.c_str(), -1, szEmailString, 256, NULL, NULL);
		emailInfo.emailAddress = (unsigned char *)szEmailString;
		VtGetTime (libCtx, &(emailInfo.emailTime));
		status = VtSetIdentityParam (
			encryptorId, VtIdentityParam822Email, (Pointer)&emailInfo);
		if (status != 0)
		{
			DPW((L"BuildSecureFileByParts: VtSetIdentityParam failed(status=%d)!\n", status));
			break;
		}

		/* Who will be allowed to read the file? There can be more than one
		* decryptor, so the decryptors are represented as an IdentityList
		* (even if there is only one decryptor).
		*/
		status = VtCreateIdentityList (
			libCtx, VtIdentityListImplMpCtx, (Pointer)0, &decryptorList);
		if (status != 0)
		{
			DPW((L"BuildSecureFileByParts: VtCreateIdentityList decryptorList failed(status=%d)!\n", status));
			break;
		}

		/* There are two ways to add an identity to an identity list.
		* First, there's VtAddNewIdToIdentityList. This is just like
		* setting an identity object with a schema, but without having to
		* build the object first.
		*/
		StringVector::iterator iterEmail;
		for (iterEmail= vecEmails.begin(); iterEmail != vecEmails.end(); iterEmail++)
		{
			WideCharToMultiByte(CP_ACP, 0, (*iterEmail).c_str(), -1, szEmailString, 256, NULL, NULL);
			emailInfo.emailAddress = (unsigned char *)szEmailString;
			status = VtAddNewIdToIdentityList (
				decryptorList, VtIdentityParam822Email, (Pointer)&emailInfo,
				&listIndex);
			if (status != 0)
				break;
		}
		if (status != 0)
		{
			DPW((L"BuildSecureFileByParts: VtAddNewIdToIdentityList failed(status=%d)!\n", status));
			break;
		}

		/* If we want to encrypt the file so the encryptor can read it as
		* well, add the encryptor to the decryptor list.
		* Because we already have an identity object built for the
		* encryptor, we can add it to the list by using the second method
		* of adding an identity: VtAddIdObjectToIdentityList.
		*/
		EnterCriticalSection(&s_criticalSection);
		status = VtAddIdObjectToIdentityList (
			decryptorList, encryptorId, &listIndex);
		if (status != 0)
		{
			LeaveCriticalSection(&s_criticalSection);
			DPW((L"BuildSecureFileByParts: VtAddIdObjectToIdentityList failed(status=%d)!\n", status));
			break;
		}

		/* Now build the SecureFile object that will create the encrypted
		* data. Create the object with an Impl that can write the
		* SecureFile format.
		*/
		status = VtCreateSecureFileObject (
			libCtx, VtSecureFileImplWrite, (Pointer)0, &sfWriter);
		if (status != 0)
		{
			LeaveCriticalSection(&s_criticalSection);
			DPW((L"BuildSecureFileByParts: VtCreateSecureFileObject failed(status=%d)!\n", status));
			break;
		}

		/* Set the object with the encryptor and decryptors.
		*/
		status = VtSetSecureFileParam (
			sfWriter, VtSecureFileParamSenderId, (Pointer)encryptorId);
		if (status != 0)
		{
			LeaveCriticalSection(&s_criticalSection);
			DPW((L"BuildSecureFileByParts: VtSetSecureFileParam VtSecureFileParamSenderId failed(status=%d)!\n", status));
			break;
		}

		status = VtSetSecureFileParam (
			sfWriter, VtSecureFileParamRecipientList, (Pointer)decryptorList);
		if (status != 0)
		{
			LeaveCriticalSection(&s_criticalSection);
			DPW((L"BuildSecureFileByParts: VtSetSecureFileParam VtSecureFileParamRecipientList failed(status=%d)!\n", status));
			break;
		}

		/* What's the file name?
		* NOTE!!! You must set the file name if you want the file you
		* encrypt to be readable by the current client.
		*/
		status = VtSetSecureFileParam (
			sfWriter, VtSecureFileParamFileName, (Pointer)szSrcFileName);
		if (status != 0)
		{
			LeaveCriticalSection(&s_criticalSection);
			DPW((L"BuildSecureFileByParts: VtSetSecureFileParam VtSecureFileParamFileName failed(status=%d)!\n", status));
			break;
		}

		/* Which symmetric algorithm should the object use to encrypt the
		* bulk data?
		*/
		status = VtSetSecureFileParam (
			sfWriter, VtSecureFileParamAES128CBC, (Pointer)0);
		if (status != 0)
		{
			LeaveCriticalSection(&s_criticalSection);
			DPW((L"BuildSecureFileByParts: VtSetSecureFileParam VtSecureFileParamAES128CBC failed(status=%d)!\n", status));
			break;
		}

		/* At this point we have a choice, either call WriteInit and
		* WriteFinal if we have all the data in one block, or set the
		* SecureFile object with the data length, then call WriteInit,
		* WriteUpdate (as many times as necessary to input all the data),
		* and WriteFinal. The Toolkit User's Manual has a discussion on
		* why it is necessary to add the data length as a Param to a
		* SecureFile object if you'll be using the Update function (look
		* for the discussion on streaming a message).
		* This subroutine will operate by parts, so we need to set the
		* DataLen param.
		*/
		status = VtSetSecureFileParam (
			sfWriter, VtSecureFileParamDataLen, (Pointer)&fileSize);
		if (status != 0)
		{
			LeaveCriticalSection(&s_criticalSection);
			DPW((L"BuildSecureFileByParts: VtSetSecureFileParam VtSecureFileParamDataLen failed(status=%d)!\n", status));
			break;
		}

		/* Because we're passing NULLs for the contexts and random object,
		* this function will expect to find the necessary elements in the
		* libCtx.
		*/
		status = VtSecureFileWriteInit (
			sfWriter, (VtPolicyCtx)0, (VtStorageCtx)0, (VtTransportCtx)0,
			(VtRandomObject)0);
		LeaveCriticalSection(&s_criticalSection);
		if (status != 0)
		{
			DPW((L"BuildSecureFileByParts: VtSecureFileWriteInit failed(status=%d)!\n", status));
			break;

			/* During WriteInit (when writing SecureFile data), the toolkit
			* will obtain the private signing key and signing cert. If those
			* values are not in storage, it will generate a new DSA key pair
			* and download a new cert. The download might be asynchronous.
			* To handle Async, call the WriteInit function again, after the
			* authentication has completed.
			* Of course, if the error is not related to ASYNC, we'll want to
			* quit.
			* If you are using the VtTransportImplHttpsWinINet provider
			* along with VtStorageFileWin32, you must have the toolkit's
			* token handler installed. If you have the Voltage Client token
			* handler installed (installed automatically when you installed
			* the Voltage email client), you must use
			* VtStorageFileWin32Client.
			* Also, this sample expects the transport provider to have been
			* set to both VT_ASYNC_RESPONSE_CONTINUE and
			* VT_PREVIOUS_RESPONSE_CONTINUE.
			*/
#if 0
			if (status != VT_ERROR_DOWNLOAD_PENDING)
				break;

			label = "Write SecureFile: Type Return/Enter after auth completes";
			SampleUtilWaitForResponse (2, label);

			/* Once the signer has been authenticated, call WriteInit again.
			* This time (if auth succeeded), the function will be able to
			* obtain the IBE private key and the signing cert.
			*/
			status = VtSecureFileWriteInit (
				sfWriter, (VtPolicyCtx)0, (VtStorageCtx)0, (VtTransportCtx)0,
				(VtRandomObject)0);
			if (status != 0)
				break;
#endif
		}

#if 1

		/* Read part of the file, operate on that data, placing any output
		* into the output file.
		*/
		bufferSize = 0;
		do
		{
			status = SampleUtilReadFile (
				libCtx, fileHandleIn, inputData, INPUT_DATA_SIZE, &bytesRead,
				&endOfFile);
			if (status != 0)
				break;

			/* Call WriteUpdate on this new input.
			*/
			EnterCriticalSection(&s_criticalSection);
			status = VtSecureFileWriteUpdate (
				sfWriter, (VtRandomObject)0, inputData, bytesRead,
				encrypted, bufferSize, &encryptedLen);
			LeaveCriticalSection(&s_criticalSection);
			if (status != 0)
			{
				/* If we get an error, it should be BUFFER_TOO_SMALL.
				*/
				DPW((L"BuildSecureFileByParts: VtSecureFileWriteUpdate failed(status=%d)!\n", status));

				if (status != VT_ERROR_BUFFER_TOO_SMALL)
					break;

				/* The toolkit offers the VtRealloc routine. You don't have to
				* use it, but it may be helpful. This call will use the memory
				* provider loaded when the libCtx was created. That provider
				* might be something you want to use for all your memory
				* because it overwrites memory before freeing (for security)
				* or because it's the debug memory and you want it to help
				* check for memory leaks in your code as well as the toolkit
				* code.
				*/
				bufferSize = encryptedLen;
				status = VtRealloc (libCtx, bufferSize, 0, (Pointer *)&encrypted);
				if (status != 0)
					break;

				EnterCriticalSection(&s_criticalSection);
				status = VtSecureFileWriteUpdate (
					sfWriter, (VtRandomObject)0, inputData, bytesRead,
					encrypted, bufferSize, &encryptedLen);
				LeaveCriticalSection(&s_criticalSection);
				if (status != 0)
				{
					DPW((L"BuildSecureFileByParts: VtSecureFileWriteUpdate failed(status=%d)!\n", status));
					break;
				}
			}

			/* Now write this new part of the SecureFile contents to the
			* output file.
			*/
			status = SampleUtilWriteFile (
				libCtx, fileHandleOut, encrypted, encryptedLen);
			if (status != 0)
				break;

		} while (endOfFile == 0);
		/* If there was an error in the do-while loop, we broke out to this
		* point. We'll need to break out of the do-while(0), though.
		*/
		if (status != 0)
			break;
		/* Now call WriteFinal. There's no more input but there can be more
		* output.
		*/
		EnterCriticalSection(&s_criticalSection);
		status = VtSecureFileWriteFinal (
			sfWriter, (VtRandomObject)0, (unsigned char *)0, 0,
			encrypted, bufferSize, &encryptedLen);
		LeaveCriticalSection(&s_criticalSection);
		if (status == VT_ERROR_BUFFER_TOO_SMALL)
		{
			bufferSize = encryptedLen;
			status = VtRealloc (libCtx, bufferSize, 0, (Pointer *)&encrypted);
			if (status != 0)
				break;

			EnterCriticalSection(&s_criticalSection);
			status = VtSecureFileWriteFinal (
				sfWriter, (VtRandomObject)0, (unsigned char *)0, 0,
				encrypted, bufferSize, &encryptedLen);
			LeaveCriticalSection(&s_criticalSection);
		}
		if (status != 0)
		{
			DPW((L"BuildSecureFileByParts: VtSecureFileWriteFinal failed(status=%d)!\n", status));
			break;
		}

		status = SampleUtilWriteFile (
			libCtx, fileHandleOut, encrypted, encryptedLen);
#endif

	} while (iCondition);

	/* If you have some sensitive information that you "allocated" on the
	* stack, overwrite it when you're done in order to prevent the
	* memory reconstruction attack.
	*/
	VtMemset (libCtx, (Pointer)inputData, 0, sizeof (inputData));

	VtFree (libCtx, (Pointer *)&encrypted);

	/* If you call create, you must call destroy.
	* If the object to destroy is NULL, the Destroy function will do
	* nothing. That means that if we initialize the object variable to
	* NULL and the object was never created in the first place (there was
	* some error before creation and we quit), the VtDestroy function
	* will recognize that there's nothing to destroy. Hence, it's easy
	* to simply always call the VtDestroy.
	*/
	EnterCriticalSection(&s_criticalSection);
	VtDestroySecureFileObject (&sfWriter);
	LeaveCriticalSection(&s_criticalSection);

	VtDestroyIdentityList (&decryptorList);
	VtDestroyIdentityObject (&encryptorId);

	/* Close any files we opened. This function will do nothing if a file
	* was previously closed or never opened.
	*/
	SampleUtilCloseFile (libCtx, &fileHandleIn);
	SampleUtilCloseFile (libCtx, &fileHandleOut);

	return (status);
}

// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	DWORD   dwDisposition	= 0;
	LONG    lResult			= 0;
	HKEY	hKeyNextlabs = NULL;
	HKEY    hKeyEncryption  = NULL;
	HKEY	hKeyVoltageAdapter	= NULL;
	WCHAR   wzKeyName[MAX_PATH+1];memset(wzKeyName, 0, sizeof(wzKeyName));
// 	DWORD   dwVal			= 0;
// 	WCHAR   wzVal[MAX_PATH+1];    memset(wzVal, 0, sizeof(wzVal));
	LPWSTR	lpwzVal = NULL;

		_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Nextlabs");

		lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE, wzKeyName,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyNextlabs,&dwDisposition);
		if(ERROR_SUCCESS != lResult)
		{
			DP((L"RegisterDll::Fail to open key: Nextlabs\n"));
			return E_UNEXPECTED;
		}

		lResult = RegCreateKeyEx( hKeyNextlabs, L"Encryption",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyEncryption,&dwDisposition);
		if(ERROR_SUCCESS != lResult)
		{
			DP((L"RegisterDll::Fail to open key: Encryption\n"));

		if(NULL != hKeyNextlabs) RegCloseKey(hKeyNextlabs);
			return E_UNEXPECTED;
		}

		lResult = RegCreateKeyEx( hKeyEncryption, L"VoltageAdapter",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyVoltageAdapter,&dwDisposition);
		if(ERROR_SUCCESS != lResult)
		{
			DP((L"RegisterDll::Fail to open key: VoltageAdapter\n"));

		if(NULL != hKeyEncryption) RegCloseKey(hKeyEncryption);
		if(NULL != hKeyNextlabs) RegCloseKey(hKeyNextlabs);
			return E_UNEXPECTED;
		}

// 		dwVal = 0;
// 		RegSetValueExW(hKeyVoltageAdapter, L"Symmetric", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
// 		dwVal = 1;
// 		RegSetValueExW(hKeyVoltageAdapter, L"Asymmetric", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
		lpwzVal = GetBaseFileName(g_wzVoltageAdapterPath);
		RegSetValueExW(hKeyVoltageAdapter, L"DLLName", 0, REG_SZ, (const BYTE*)lpwzVal, (1+(DWORD)wcslen(lpwzVal))*sizeof(WCHAR));
		lpwzVal = L".vsf";
		RegSetValueExW(hKeyVoltageAdapter, L"Extension", 0, REG_SZ, (const BYTE*)lpwzVal, (1+(DWORD)wcslen(lpwzVal))*sizeof(WCHAR));

		if(NULL != hKeyVoltageAdapter) RegCloseKey(hKeyVoltageAdapter);
		if(NULL != hKeyEncryption) RegCloseKey(hKeyEncryption);
		if(NULL != hKeyNextlabs) RegCloseKey(hKeyNextlabs);

		hKeyVoltageAdapter   = NULL;
		hKeyEncryption = NULL;
	hKeyNextlabs = NULL;

	return S_OK;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));

	_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Nextlabs\\Encryption\\VoltageAdapter");
	RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
	return S_OK;
}


static int ValidateEmailAddress(LPEncryptionAdapterData lpData)
{
	int ret = lpData->encryptContext.wstrEncrypterInfo.find(L'@');

	if ( ret < 0)
	{
		DPW((L"ValidateEmailAddress: Invalid sender email address(%s)!\n", lpData->encryptContext.wstrEncrypterInfo.c_str()));
		return -1;
	}

	StringVector::iterator iterEmail;
	for (iterEmail = lpData->encryptContext.vecDecrypterInfos.begin(); iterEmail != lpData->encryptContext.vecDecrypterInfos.end(); iterEmail++)
	{
		ret = (*iterEmail).find(L'@');
		if (ret < 0)
		{
			DPW((L"ValidateEmailAddress: Invalid recipient email address(%s)!\n", (*iterEmail).c_str()));
			return -1;
		}
	}

	return 0;
}

static LPWSTR GetBaseFileName(LPWSTR lpwzFileName)
{
	LPWSTR lpwzBaseFileName = NULL;

	lpwzBaseFileName = (LPWSTR)wcsrchr(lpwzFileName, '\\');
	if (!lpwzBaseFileName)
	{
		lpwzBaseFileName = (LPWSTR)wcsrchr(lpwzFileName, '/');
	}

	if (!lpwzBaseFileName)
	{
		lpwzBaseFileName = (LPWSTR)lpwzFileName;
	}
	else
	{
		lpwzBaseFileName++;
	}

	return lpwzBaseFileName;
}