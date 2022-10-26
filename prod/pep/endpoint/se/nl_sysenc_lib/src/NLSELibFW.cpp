#define WINVER _WIN32_WINNT_WINXP
#pragma warning( push )
#pragma warning( disable : 4005 )
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#pragma warning( pop )
#define NTDDI_VERSION NTDDI_WINXPSP2

#include <windows.h>
#include <cassert>
#pragma warning( push )
#pragma warning( disable : 4005 )
#include <fltuser.h>
#pragma warning( pop )
#include <WinCrypt.h>
#include <shlwapi.h>
#include "celog.h"
#include "nlthread.h"
#include "nl_sysenc_lib.h"
#include "nl_sysenc_lib_fw.h"
#include "NLSESDKWrapper.h"
#include "NLSELib.h"



#define CELOG_CUR_FILE \
  CELOG_FILEPATH_PROD_PEP_ENDPOINT_SE_NL_SYSENC_LIB_SRC_NLSELIBFW_CPP



#define AES_BLOCK_SIZE 16

#define NLSE_BLK_SIZE_IN_BYTES          512     // Encryption of each block
                                                // will be init'ed using an
                                                // Initial Vector which is the
                                                // file offset of the block.

// The following struct needs to be unpadded (i.e. the 64-bit fileRealLength
// member in NextLabsEncryptionFile_Header_1_0_t will be un-aligned) in order
// to match the indicator stream created by the NLSE FW filter driver.
#pragma pack(push, 1)
typedef struct {
  NextLabsFile_Header_1_0_t fileType;
  NextLabsEncryptionFile_Header_1_0_t encExt;
} NLSEIndicator10_t;
#pragma pack(pop)

#define ROUND_UP(x, n)  (((x + n - 1) / n) * n)

class CFWGlobalTLS
{
public:
    CFWGlobalTLS(){
        dwFWTlsIndex = ::TlsAlloc();
        drmFWPathList = NULL;
        drmFWPathListLen = 0;
        nlthread_mutex_init(&drmFWPathListMutex);
    };

    ~CFWGlobalTLS(){
        nlthread_mutex_destroy(&drmFWPathListMutex);
        ::TlsFree(dwFWTlsIndex);
        dwFWTlsIndex = TLS_OUT_OF_INDEXES;
    };
public:
    DWORD dwFWTlsIndex;

    WCHAR *drmFWPathList;             // Blob of DRM paths, concatenated
                                    // together and separated by
                                    // null-terminators
    int drmFWPathListLen;             // # wchar's in drmPaths
    nlthread_mutex_t drmFWPathListMutex;
};

static CFWGlobalTLS _fwglobal;

static bool getNLSEIndicatorName(LPCWSTR fileName, BOOL needPrefix,std::wstring& indicatorName);


static int ensureDrmFWPathListRetrieved(void)
{
  CELOG_ENTER;
  int drmFWPathListSize;
  CEResult_t result;
  int nRet = 0;

  nlthread_mutex_lock(&_fwglobal.drmFWPathListMutex);

  if (_fwglobal.drmFWPathList != NULL)
  {
    nRet = 0;
    goto _exit;
  }

  result = CESDKDrmGetFwPaths(&_fwglobal.drmFWPathList, &drmFWPathListSize);
  if (result != CE_RESULT_SUCCESS)
  {
    nRet = -1;
    goto _exit;
  }

  _fwglobal.drmFWPathListLen = drmFWPathListSize / sizeof *_fwglobal.drmFWPathList;

  if (_fwglobal.drmFWPathList[_fwglobal.drmFWPathListLen - 1] != L'\0')
  {
    // The last path is not null-terminated.  The list is corrupted.
    CELOG_LOG(CELOG_ERROR, L"NLSELib!ensureDrmFWPathListRetrieved: path list is corrutped\n");
    free(_fwglobal.drmFWPathList);
    _fwglobal.drmFWPathList = NULL;
    _fwglobal.drmFWPathListLen = 0;
    nRet = -1;
    goto _exit;
  }

  nRet = 0;

_exit:  
  nlthread_mutex_unlock(&_fwglobal.drmFWPathListMutex);
  CELOG_RETURN_VAL(0);
}

/** is_in_DRM_FW_path_list
 *
 *  \brief Determine if the given path matches a path in the DRM path list
 *
 *  \retrun true if the path matches, otherwise false.
 */
bool is_in_DRM_FW_path_list( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  if (ensureDrmFWPathListRetrieved() != 0)
  {
    // Error.
    CELOG_RETURN_VAL(false);
  }

  nlthread_mutex_lock(&_fwglobal.drmFWPathListMutex);

  // Get head of list.
  WCHAR *entry = _fwglobal.drmFWPathList;

  // Stop if end of list is reached.
  while (entry != _fwglobal.drmFWPathList + _fwglobal.drmFWPathListLen)
  {
    if (_wcsicmp(in_path, entry) == 0 ||
        is_matching_wildcard_path(in_path, entry))
    {
      nlthread_mutex_unlock(&_fwglobal.drmFWPathListMutex);
      CELOG_RETURN_VAL(true);
    }

    // Get next list entry.
    entry += wcslen(entry) + 1;
  }

  nlthread_mutex_unlock(&_fwglobal.drmFWPathListMutex);
  CELOG_RETURN_VAL(false);
}/* is_in_DRM_FW_path_list */

BOOL SE_IsEncryptedFW( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls \n", in_path);
  
  assert( in_path != NULL );
  if( in_path == NULL )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }
  BOOL is_encrypted = FALSE, is_nonExisting = FALSE;
  DWORD attrs = GetFileAttributesW(in_path);
  if( attrs == INVALID_FILE_ATTRIBUTES )
  {
    is_nonExisting = TRUE;
  }

  if ( !is_nonExisting )
  {
    std::wstring ind_name;
    if( getNLSEIndicatorName(in_path,TRUE,ind_name) == false )
    {
      SetLastError(ERROR_INVALID_PARAMETER);
      CELOG_RETURN_VAL(FALSE);
    }

    HANDLE hFile = CreateFileW(ind_name.c_str(),
                   GENERIC_READ,FILE_SHARE_READ,NULL,
                   OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if( hFile != INVALID_HANDLE_VALUE )
    {
      is_encrypted = TRUE;
      CloseHandle(hFile);
    }
  }

  bool is_directory = (attrs & FILE_ATTRIBUTE_DIRECTORY) || is_nonExisting;

  /* Check DRM path list if this is a directory and is not encrypted. */
  if( is_directory == true && is_encrypted == FALSE )
  {  
    is_encrypted = is_in_DRM_FW_path_list(in_path);
  }

  /* Check parent if this is a directory and is not encrypted. */
  if( is_directory == true && is_encrypted == FALSE )
  {
    std::wstring dir(in_path);
    std::wstring::size_type off = dir.find_last_of(L"\\/");
    if( off != std::wstring::npos )
    {
      dir.erase(off,dir.length());  /* trim leading directory to get the file name only */
      BOOL is_enc = SE_IsEncryptedFW(dir.c_str());

      /* If director is SE encrypted return true to avoid walking up to another
	 path which may not be encrypted.  If any of the parents is encrypted,
	 then this directory should be reported as encrypted.
       */
      if( is_enc == TRUE )
      {
	CELOG_RETURN_VAL(TRUE);
      }
    }
  }

  SetLastError(ERROR_SUCCESS);
  CELOG_RETURN_VAL(is_encrypted);
}/* SE_IsEncryptedFW */

/** SE_EncryptDirectoryFW
 *
 *  \brief Set Fast-Write encryption on a given directory.
 *
 *  \return TRUE on success.
 */
BOOL SE_EncryptDirectoryFW( _In_z_ const wchar_t* dName )
{
  CELOG_ENTER;
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: dName=%ls \n", dName);
  BOOL status = FALSE;
  CEResult_t rv;

  assert( dName != NULL );
  if( dName == NULL )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  /* Can encrypt this dir? */
  if( can_encrypt(dName) == false )
  {
    SetLastError(ERROR_NOT_SUPPORTED);
    CELOG_RETURN_VAL(FALSE);
  }

  rv = CESDKDrmAddFwPath(dName);
  if( rv == CE_RESULT_SUCCESS )
  {
    status = TRUE;
  }

  CELOG_RETURN_VAL(status);

}/* SE_EncryptDirectoryFW */

/** SE_DecryptDirectoryFW
 *
 *  \brief Clear Fast-Write encryption on a given directory.
 *
 *  \return TRUE on success.
 */
BOOL SE_DecryptDirectoryFW( _In_z_ const wchar_t* dName,
                            _In_z_ const wchar_t* password )
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: dName=%ls, password=%ls \n", dName,password );
	
  BOOL status = FALSE;
  nlse_enc_error_t ret;

  assert( dName != NULL );
  if( dName == NULL )
  {
	CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  if (is_NLSE_service_running())
  {
	CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
    SetLastError(ERROR_ACCESS_DENIED);
    CELOG_RETURN_VAL(FALSE);
  }

  ret = checkPassword(password);
  if (ret == NLSE_ENC_ERROR_INVALID_PASSWORD)
  {
	CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
    SetLastError(ERROR_INVALID_PASSWORD);
    CELOG_RETURN_VAL(FALSE);
  }
  else if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
	CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
    SetLastError(ERROR_ACCESS_DENIED);
    CELOG_RETURN_VAL(FALSE);
  }

  CEResult_t rv;
  rv = CESDKDrmRemoveFwPath(dName);
  if( rv == CE_RESULT_SUCCESS )
  {
    status = TRUE;
  }
  CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
  CELOG_RETURN_VAL(status);
}/* SE_DecryptDirectoryFW */

/** getNLSEIndicatorName
 *
 *  \brief Get the NLSE indicator full path name for the passed file.
 *
 *  \param fileName full path name of a local file.
 *  \param needPrefix TRUE if "\\?\" prefix should be added in case the indicator path name is over MAX_PATH.
 *
 *  \return full path name of the indicator, prefixed as requested.
 */
static bool getNLSEIndicatorName(LPCWSTR fileName, BOOL needPrefix, std::wstring& indicatorName)
{
  CELOG_ENTER;
  // We cannot handle relative paths, since relative paths cannot be prefixed
  // with "\\?\" which is the only way to go over the MAX_PATH limit when the
  // indicator path name is too long.
  // We don't handle network paths either, since NLSE as a whole doesn't
  // support it.
  // Hence fileName must be in the form of either "C:\..." or "\\?\C:\...".

  if( wcslen(fileName) < 4 )
  {
    CELOG_RETURN_VAL(false);
  }

  indicatorName.clear();

  if( wcsncmp(fileName + 1, L":\\", wcslen(L":\\")) == 0 &&
      wcsncmp(fileName, L"\\\\?\\", wcslen(L"\\\\?\\")) != 0)
  {
    indicatorName = ((needPrefix &&
		      wcsncmp(fileName, L"\\\\?\\", wcslen(L"\\\\?\\")) != 0) ?
		     L"\\\\?\\" : L"");
  }
  
  indicatorName += fileName;
  indicatorName += NLFSE_ADS_SUFFIX;

  CELOG_RETURN_VAL(true);
} /* getNLSEIndicatorName */

static nlse_enc_error_t isFileNLSEEncryptedFW(LPCWSTR fileName, PBOOL pEncrypted)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  HANDLE han = INVALID_HANDLE_VALUE;
  nlse_enc_error_t ret;
  
  std::wstring ind_name;
  if( getNLSEIndicatorName(fileName,TRUE,ind_name) == false )
  {
    CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_CANT_READ_INPUT_FILE);
  }

  han = CreateFile(ind_name.c_str(), GENERIC_READ,
                   FILE_SHARE_READ, NULL, OPEN_EXISTING,
                   FILE_FLAG_SEQUENTIAL_SCAN, NULL);

  if (han == INVALID_HANDLE_VALUE)
  {
    // Cannot open the indicator stream.  Return error if the error was
    // something other than stream-not-found.
    if (GetLastError() != ERROR_FILE_NOT_FOUND)
    {
      ret = NLSE_ENC_ERROR_CANT_READ_INDICATOR;
      goto exit;
    }

    // Indicator stream was not found.  See if the file itself exists.
    if (GetFileAttributes(fileName) == INVALID_FILE_ATTRIBUTES &&
        GetLastError() == ERROR_FILE_NOT_FOUND)
    {
      // Either the file itself doesn't exist or it can't be read.  Return
      // error in both cases.
      ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
    }
    else
    {
      // The file itself exists.  Return "not encrypted".
      *pEncrypted = FALSE;
      ret = NLSE_ENC_ERROR_SUCCESS;
    }

    goto exit;
  }

  // Read the stream.
  NLSEIndicator10_t indicatorBuf;
  DWORD bytesRead;

  if (!ReadFile(han, &indicatorBuf, sizeof indicatorBuf, &bytesRead, NULL))
  {
    // Can't read the indicator stream.  Return error.
    ret = NLSE_ENC_ERROR_CANT_READ_INDICATOR;
    goto exit;
  }
  else if (bytesRead < sizeof indicatorBuf)
  {
    // Stream too short.  Return "not encrypted".
    *pEncrypted = FALSE;
    ret = NLSE_ENC_ERROR_SUCCESS;
    goto exit;
  }

  if (memcmp(indicatorBuf.fileType.cookie, NL_FILE_1_0_COOKIE,
             sizeof indicatorBuf.fileType.cookie) != 0)
  {
    // Cookie doesn't match.  Return "not encrypted".
    *pEncrypted = FALSE;
    ret = NLSE_ENC_ERROR_SUCCESS;
    goto exit;
  }

  if (indicatorBuf.fileType.version_major != NL_FILE_VERSION_1_0_MAJOR ||
      indicatorBuf.fileType.header_size != sizeof(NextLabsFile_Header_1_0_t) ||
      indicatorBuf.fileType.stream_count != 1)
  {
    // Filetype version not supported or more than one stream.  Return error.
    ret = NLSE_ENC_ERROR_UNSUPPORTED_FILETYPE;
    goto exit;
  }

  if (strcmp((const char *) indicatorBuf.encExt.sh.stream_name,
             NLE_STREAM_1_0_NAME) != 0)
  {
    // Stream name doesn't match.  Return "not encrypted".
    *pEncrypted = FALSE;
    ret = NLSE_ENC_ERROR_SUCCESS;
    goto exit;
  }

  if (indicatorBuf.encExt.version_major != NLE_FILE_VERSION_1_0_MAJOR ||
      indicatorBuf.encExt.sh.stream_size !=
      sizeof(NextLabsEncryptionFile_Header_1_0_t))
  {
    // Crypto version not supported.  Return error.
    // Encryption Filetype version or header size not supported.  Return
    // error.
    ret = NLSE_ENC_ERROR_UNSUPPORTED_ENCRYPTION_FILETYPE;
    goto exit;
  }

  *pEncrypted = TRUE;
  ret = NLSE_ENC_ERROR_SUCCESS;

exit:
  if (han != INVALID_HANDLE_VALUE)
  {
    CloseHandle(han);
  }

  CELOG_ENUM_RETURN_VAL(ret);
} /* isFileNLSEEncryptedFW */

//Initialize NextLabs File Type v1.0 structure
static VOID initializeNLFileType10(NextLabsFile_Header_1_0_t *t)
{
  CELOG_ENTER;
  ZeroMemory(t, sizeof *t);
  t->version_major=NL_FILE_VERSION_1_0_MAJOR;
  t->version_minor=NL_FILE_VERSION_1_0_MINOR;
  t->header_size = sizeof *t;
  t->stream_count=0;
  CopyMemory(t->cookie, NL_FILE_1_0_COOKIE, sizeof t->cookie);
  CELOG_RETURN;
}

static VOID initializeNLFSEEncryptExtension10(NextLabsEncryptionFile_Header_1_0_t __unaligned *pExt)
{
  CELOG_ENTER;
  ZeroMemory(pExt, sizeof *pExt);  
  pExt->sh.stream_size = sizeof *pExt;
  strncpy_s((char *) pExt->sh.stream_name, sizeof pExt->sh.stream_name,
           NLE_STREAM_1_0_NAME, _TRUNCATE);
  pExt->version_major=NLE_FILE_VERSION_1_0_MAJOR;
  pExt->version_minor=NLE_FILE_VERSION_1_0_MINOR;
  CELOG_RETURN;
}/*NLFSEAllocateEncryptExtension*/

static nlse_enc_error_t readNLSEIndicatorFW
    (HCRYPTPROV hProv, LPCWSTR fileName, PULONGLONG pFileRealLength,
     BYTE key[NLE_KEY_LENGTH_IN_BYTES], PULONG pPaddingLen,
     PBYTE pPaddingData)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  NLSEIndicator10_t indicatorBuf;
  HANDLE indicatorStream = INVALID_HANDLE_VALUE;
  HCRYPTKEY hPCKey = NULL;
  DWORD bytesRead;
  nlse_enc_error_t ret;

  std::wstring ind_name;
  if( getNLSEIndicatorName(fileName,TRUE,ind_name) == false )
  {
    CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_CANT_READ_INDICATOR);
  }

  indicatorStream =
    CreateFile(ind_name.c_str(), GENERIC_READ,
               FILE_SHARE_READ, NULL, OPEN_EXISTING,
               FILE_FLAG_SEQUENTIAL_SCAN, NULL);

  if (indicatorStream == INVALID_HANDLE_VALUE ||
      !ReadFile(indicatorStream, &indicatorBuf, sizeof indicatorBuf,
                &bytesRead, NULL) ||
      bytesRead != sizeof indicatorBuf)
  {
    ret = NLSE_ENC_ERROR_CANT_READ_INDICATOR;
    goto cleanup;
  }

  *pFileRealLength = indicatorBuf.encExt. fileRealLength;
  *pPaddingLen = indicatorBuf.encExt.paddingLen;
  CopyMemory(pPaddingData, indicatorBuf.encExt.paddingData, *pPaddingLen);

  // Decrypt data encryption key with the PC key that was used to encrypt it.

  CopyMemory(key, indicatorBuf.encExt.key, NLE_KEY_LENGTH_IN_BYTES);

  unsigned char pcKey[NLE_KEY_LENGTH_IN_BYTES];

  ret = getPCKey(NLE_KEY_RING_LOCAL, &indicatorBuf.encExt.pcKeyID, pcKey);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto cleanup;
  }

  ret = initCryptoKey(hProv, pcKey, 0, &hPCKey);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto cleanup;
  }

  DWORD encDataSize = NLE_KEY_LENGTH_IN_BYTES;

  if (!CryptDecrypt(hPCKey, NULL, FALSE, 0, key, &encDataSize))
  {
    ret = NLSE_ENC_ERROR_CANT_DECRYPT_DATA_KEY;
    goto cleanup;
  }

  ret = NLSE_ENC_ERROR_SUCCESS;

cleanup:
  if (hPCKey != NULL)
  {
    freeCryptoKey(hPCKey);
  }

  SecureZeroMemory(pcKey, sizeof pcKey);

  if (indicatorStream != INVALID_HANDLE_VALUE)
  {
    CloseHandle(indicatorStream);
  }

  CELOG_ENUM_RETURN_VAL(ret);
}

static nlse_enc_error_t writeNLSEIndicatorFW(
    HCRYPTPROV hProv, LPCWSTR fileName, ULONGLONG fileRealLength,
    const BYTE key[NLE_KEY_LENGTH_IN_BYTES], ULONG paddingLen,
    const BYTE *paddingData)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  NLSEIndicator10_t indicatorBuf;
  HANDLE indicatorStream = INVALID_HANDLE_VALUE;
  HCRYPTKEY hPCKey = NULL;
  DWORD bytesWritten;
  nlse_enc_error_t ret;
  std::wstring ind_name;

  initializeNLFileType10(&indicatorBuf.fileType);
  indicatorBuf.fileType.stream_count = 1;
  initializeNLFSEEncryptExtension10(&indicatorBuf.encExt);
  strncpy_s(indicatorBuf.encExt.pcKeyRingName, NLE_KEY_RING_NAME_MAX_LEN, NLE_KEY_RING_LOCAL, _TRUNCATE);
  indicatorBuf.encExt.fileRealLength = fileRealLength;
  indicatorBuf.encExt.paddingLen = paddingLen;
  CopyMemory(indicatorBuf.encExt.paddingData, paddingData, paddingLen);

  // Encrypt data encryption key with latest PC key, and store the
  // corresponding key ID.

  CopyMemory(indicatorBuf.encExt.key, key, sizeof indicatorBuf.encExt.key);

  unsigned char pcKey[NLE_KEY_LENGTH_IN_BYTES];

  ZeroMemory(&indicatorBuf.encExt.pcKeyID, sizeof indicatorBuf.encExt.pcKeyID);
  ret = getPCKey(NLE_KEY_RING_LOCAL, &indicatorBuf.encExt.pcKeyID, pcKey);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto cleanup;
  }

  ret = initCryptoKey(hProv, pcKey, 0, &hPCKey);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto cleanup;
  }

  DWORD encDataSize = sizeof indicatorBuf.encExt.key;

  if (!CryptEncrypt(hPCKey, NULL, FALSE, 0, indicatorBuf.encExt.key,
                    &encDataSize, sizeof indicatorBuf.encExt.key))
  {
    ret = NLSE_ENC_ERROR_CANT_ENCRYPT_DATA_KEY;
    goto cleanup;
  }

  wchar_t indicatorName[NLSE_MAX_PATH] = {0};
  if( getNLSEIndicatorName(fileName, FALSE, ind_name) == false )
  {
    ret = NLSE_ENC_ERROR_CANT_ENCRYPT_DATA_KEY;
    goto cleanup;
  }
  wcsncpy_s(indicatorName,NLSE_MAX_PATH,ind_name.c_str(), _TRUNCATE);
  if (!SE_CreateFileRaw(indicatorName, wcslen(indicatorName), GENERIC_WRITE,
                        FILE_ATTRIBUTE_NORMAL, 0, CREATE_ALWAYS,
                        &indicatorStream))
  {
    goto commError;
  }
  else if (indicatorStream == INVALID_HANDLE_VALUE)
  {
    ret = NLSE_ENC_ERROR_CANT_WRITE_INDICATOR;
    goto cleanup;
  }

  if (!SE_WriteFileRaw(indicatorStream, 0, sizeof indicatorBuf, &indicatorBuf,
                       sizeof indicatorBuf, &bytesWritten))
  {
    goto commError;
  }
  else if (bytesWritten != sizeof indicatorBuf)
  {
    ret = NLSE_ENC_ERROR_CANT_WRITE_INDICATOR;
    goto cleanup;
  }

  ret = NLSE_ENC_ERROR_SUCCESS;



cleanup:
  if (indicatorStream != INVALID_HANDLE_VALUE)
  {
    SE_CloseFileRaw(indicatorStream);
  }

  if (hPCKey != NULL)
  {
    freeCryptoKey(hPCKey);
  }

  SecureZeroMemory(pcKey, sizeof pcKey);
  CELOG_ENUM_RETURN_VAL(ret);

commError:
  ret = NLSE_ENC_ERROR_PLUGIN_COMM;
  goto cleanup;
} /* writeNLSEIndicatorFW */

static nlse_enc_error_t deleteNLSEIndicator(LPCWSTR fileName)
{
    CELOG_ENUM_ENTER(nlse_enc_error_t);
    std::wstring ind_name;
    if( getNLSEIndicatorName(fileName,TRUE,ind_name) == false )
    {
      CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_CANT_DELETE_INDICATOR);
    }
    if (!DeleteFile(ind_name.c_str()))
    {
      CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_CANT_DELETE_INDICATOR);
    }

    CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_SUCCESS);
} /* deleteNLSEIndicator */

/** doEncryptFileFW
 *
 * \brief Encrypt a file, either in-place or to a different file name.
 */
static nlse_enc_error_t doEncryptFileFW(LPCWSTR inFileName,
                                        LPCWSTR outFileName OPTIONAL)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  HANDLE inFile = INVALID_HANDLE_VALUE, outFile = INVALID_HANDLE_VALUE;
  nlse_enc_error_t ret;
  BOOL encrypted;
  LARGE_INTEGER fileRealLength;
  HCRYPTPROV hProv = NULL;
  unsigned char key[NLE_KEY_LENGTH_IN_BYTES];
  HCRYPTKEY hKey = NULL;
  ULONGLONG readRoundCount = 0;

  ret = isFileNLSEEncryptedFW(inFileName, &encrypted);

  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto exit;
  }

  if (encrypted)
  {
    ret = NLSE_ENC_ERROR_INPUT_FILE_ENCRYPTED;
    goto exit;
  }

  ret = initCryptoContext(&hProv);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto exit;
  }

  // Generate random data key.
  if (!CryptGenRandom(hProv, sizeof key, key))
  {
    ret = NLSE_ENC_ERROR_CANT_GENERATE_DATA_KEY;
    goto exit;
  }

  if (outFileName == NULL || outFileName[0] == L'\0')
  {
    // Open the input file for read/write.
    inFile = CreateFile(inFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (inFile == INVALID_HANDLE_VALUE)
    {
      ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
      goto exit;
    }

    outFile = inFile;
  }
  else
  {
    // Encrypt the input file to the output file.
    inFile = CreateFile(inFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (inFile == INVALID_HANDLE_VALUE)
    {
      ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
      goto exit;
    }

    // Delete the file before we use Raw R/W API to create the file.  This
    // ensures that any outdated data encryption key stored in filter driver's
    // memory for this file will be purged.  Then the driver will use the data
    // encryption key that we generate and store in the ADS stream.
    if (!DeleteFile(outFileName) && GetLastError() != ERROR_FILE_NOT_FOUND)
    {
      ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
      goto exit;
    }

    // Create the output file.
    wchar_t outFileName2[MAX_PATH];

    wcsncpy_s(outFileName2, MAX_PATH, outFileName, _TRUNCATE);
    if (!SE_CreateFileRaw(outFileName2, wcslen(outFileName), GENERIC_WRITE,
                          FILE_ATTRIBUTE_NORMAL, 0, CREATE_NEW, &outFile))
    {
      goto commError;
    }
    else if (outFile == INVALID_HANDLE_VALUE)
    {
      ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
      goto exit;
    }
  }

  if (!GetFileSizeEx(inFile, &fileRealLength))
  {
    ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
    goto exit;
  }

  BYTE paddingData[NLE_PADDING_DATA_LEN];
  DWORD paddingLen = 0;

  // Loop through each block of file content to encrypt.
  while (1)
  {
    BYTE buf[NLSE_BLK_SIZE_IN_BYTES];
    DWORD bytesRead, bytesWritten;
  KeepProcessTrusted();
    ret = initCryptoKey(hProv, key, readRoundCount * sizeof buf, &hKey);
    if (ret != NLSE_ENC_ERROR_SUCCESS)
    {
      goto exit;
    }

    // Read one block of data from inFile.
    //
    // About checking for EOF:
    // MSDN doc says that "If the ReadFile function attempts to read past the
    // end of the file, the function returns zero, and GetLastError returns
    // ERROR_HANDLE_EOF."  However, my observations on WinXP Pro SP3, Win2003
    // R2 SE SP2 and Win2008 EE SP1 is that ReadFile() returns non-zero and
    // bytesRead = 0.  So here we check for both cses when checking for EOF.
    if (!ReadFile(inFile, buf, sizeof buf, &bytesRead, NULL))
    {
      if (GetLastError() == ERROR_HANDLE_EOF)
      {
        // No more bytes to encrypt.
        break;
      }
      else
      {
        ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
        goto exit;
      }
    }
    else if (bytesRead == 0)
    {
      // No more bytes to encrypt.
      break;
    }

    // Pad the data with random bytes, if needed.
    DWORD paddedSize = ROUND_UP(bytesRead, AES_BLOCK_SIZE);

    paddingLen = paddedSize - bytesRead;

    if (paddingLen > 0)
    {
      if (!CryptGenRandom(hProv, paddingLen, buf + bytesRead))
      {
        ret = NLSE_ENC_ERROR_CANT_GENERATE_PADDING_BYTES;
        goto exit;
      }
    }

    // Encrypt the data.
    DWORD encDataSize = paddedSize;

    // Temporarily disable compiler warning 6385, which is:
    //     "warning C6385: Invalid data: accessing 'argument 5', the readable
    //     size is 'bytesRead' bytes, but '16' bytes might be read:"
    // It is complaining that CryptEncrypt() below might be reading more bytes
    // in buf than were returned by ReadFile() above.  This would never happen,
    // however, since the padding logic above would have called
    // CryptGenRandom() to init the extra bytes.  Hence we disable the warning
    // here.
#pragma warning(push)
#pragma warning(disable:6385)
    if (!CryptEncrypt(hKey, NULL, FALSE, 0, buf, &encDataSize, paddedSize) ||
        encDataSize != paddedSize)
#pragma warning(pop)
    {
      ret = NLSE_ENC_ERROR_CANT_ENCRYPT_DATA;
      goto exit;
    }

    if (paddingLen > 0)
    {
      CopyMemory(paddingData, buf + bytesRead, paddingLen);
    }

    // Rewind the file pointer if we are reading from and writing to the same
    // file.
    if (outFile == inFile)
    {
      LARGE_INTEGER dist;

      dist.QuadPart = - (LONGLONG) bytesRead;
      if (SetFilePointerEx(outFile, dist, NULL, FILE_CURRENT) ==
          INVALID_SET_FILE_POINTER)
      {
        ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
        goto exit;
      }
    }

    // Write one block of data to outFile.
    if (!SE_WriteFileRaw(outFile, readRoundCount * sizeof buf, bytesRead, buf,
                         sizeof buf, &bytesWritten))
    {
      goto commError;
    }
    else if (bytesWritten != bytesRead)
    {
      ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
      goto exit;
    }

    freeCryptoKey(hKey);
    hKey = NULL;
    readRoundCount++;
  }
   KeepProcessTrusted();
  if (outFile != INVALID_HANDLE_VALUE && outFile != inFile)
  {
	  SE_CloseFileRaw(outFile);
	  outFile = INVALID_HANDLE_VALUE;
  }
  // Write key and padding data into indicator.
  ret = writeNLSEIndicatorFW(hProv, outFile == inFile ? inFileName : outFileName,
                             fileRealLength.QuadPart, key, paddingLen,
                             paddingData);

exit:  
  if (hKey != NULL)
  {
    freeCryptoKey(hKey);
  }

  if (outFile != INVALID_HANDLE_VALUE && outFile != inFile)
  {
    SE_CloseFileRaw(outFile);
  }

  if (inFile != INVALID_HANDLE_VALUE)
  {
    CloseHandle(inFile);
  }

  SecureZeroMemory(key, sizeof key);

  if (hProv != NULL)
  {
    freeCryptoContext(hProv);
  }

  CELOG_ENUM_RETURN_VAL(ret);

commError:
  ret = NLSE_ENC_ERROR_PLUGIN_COMM;
  goto exit;
} /* doEncryptFileFW */

/** doDecryptFileFW
 *
 * \brief Decrypt a file, either in-place or to a different file name.
 */
static nlse_enc_error_t doDecryptFileFW(LPCWSTR inFileName,
                                        LPCWSTR outFileName OPTIONAL)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  HANDLE inFile = INVALID_HANDLE_VALUE, outFile = INVALID_HANDLE_VALUE;
  nlse_enc_error_t ret;
  BOOL encrypted;
  ULONGLONG fileRealLength;
  HCRYPTPROV hProv = NULL;
  BYTE key[NLE_KEY_LENGTH_IN_BYTES];
  HCRYPTKEY hKey = NULL;
  ULONGLONG writeRoundCount = 0;

  ret = isFileNLSEEncryptedFW(inFileName, &encrypted);

  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto exit;
  }

  if (!encrypted)
  {
    ret = NLSE_ENC_ERROR_INPUT_FILE_NOT_ENCRYPTED;
    goto exit;
  }

  ret = initCryptoContext(&hProv);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto exit;
  }

  if (outFileName == NULL || outFileName[0] == L'\0')
  {
    // Open the input file for read/write to decrypt in-place.

    // Open the input file for read/write.
    // OPEN_EXISTING does not delete existing streams including our NLSE
    // indicator.
    inFile = CreateFile(inFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (inFile == INVALID_HANDLE_VALUE)
    {
      ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
      goto exit;
    }

    outFile = inFile;
  }
  else
  {
	//Check if PC is running by asking it for the latest PC key.
    NLSE_KEY_ID dummyPcKeyId;
	unsigned char dummyPcKey[NLE_KEY_LENGTH_IN_BYTES];

    ZeroMemory(&dummyPcKeyId, sizeof dummyPcKeyId);
	ret = getPCKey(NLE_KEY_RING_LOCAL, &dummyPcKeyId, dummyPcKey);
	if (ret != NLSE_ENC_ERROR_SUCCESS)
	{
		goto exit;
	}
    // Decrypt the input file to the output file.
    inFile = CreateFile(inFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (inFile == INVALID_HANDLE_VALUE)
    {
      ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
      goto exit;
    }

    // Create the output file or open the file for overwrite.
    // If the file already exists, CREATE_ALWAYS deletes any existing streams
    // including our NLSE indicator in the file.
    outFile = CreateFile(outFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                         NULL);

    if (outFile == INVALID_HANDLE_VALUE)
    {
      ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
      goto exit;
    }

  }

  // Read key and padding data from indicator.
  BYTE paddingData[NLE_PADDING_DATA_LEN];
  DWORD paddingLen;

  ret = readNLSEIndicatorFW(hProv, inFileName, &fileRealLength, key, &paddingLen,
                            paddingData);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto exit;
  }

  // Verify the indicator.
  LARGE_INTEGER fileLengthOnDisk;

  if (!GetFileSizeEx(inFile, &fileLengthOnDisk))
  {
    ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
    goto exit;
  }
  else if (((ULONGLONG) fileLengthOnDisk.QuadPart != fileRealLength) ||
           (fileRealLength + paddingLen !=
            ROUND_UP(fileRealLength, AES_BLOCK_SIZE)))
  {
    ret = NLSE_ENC_ERROR_INDICATOR_CORRUPTED;
    goto exit;
  }

  // Loop through each block of file content to decrypt.
  while (1)
  {
    BYTE buf[NLSE_BLK_SIZE_IN_BYTES];
    DWORD bytesRead, bytesWritten;

    ret = initCryptoKey(hProv, key, writeRoundCount * sizeof buf, &hKey);
    if (ret != NLSE_ENC_ERROR_SUCCESS)
    {
      goto exit;
    }

    // Read one block of data from inFile.
    //
    // About checking for EOF:
    // MSDN doc says that "If the ReadFile function attempts to read past the
    // end of the file, the function returns zero, and GetLastError returns
    // ERROR_HANDLE_EOF."  However, my observations on WinXP Pro SP3, Win2003
    // R2 SE SP2 and Win2008 EE SP1 is that ReadFile() returns non-zero and
    // bytesRead = 0.  So here we check for both cses when checking for EOF.
    if (!ReadFile(inFile, buf, sizeof buf, &bytesRead, NULL))
    {
      if (GetLastError() == ERROR_HANDLE_EOF)
      {
        // No more bytes to decrypt.  Done.
        break;
      }
      else
      {
        ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
        goto exit;
      }
    }
    else if (bytesRead == 0)
    {
      // No more bytes to decrypt.  Done.
      break;
    }

    // Pad the data with padding bytes, if needed.
    DWORD paddedSize = ROUND_UP(bytesRead, AES_BLOCK_SIZE);

    if (paddedSize > bytesRead)
    {
      CopyMemory(buf + bytesRead, paddingData, paddingLen);
    }

    // Decrypt the data.
    DWORD decDataSize = paddedSize;

    // Temporarily disable compiler warning 6385, which is:
    //     "warning C6385: Invalid data: accessing 'argument 5', the readable
    //     size is 'bytesRead' bytes, but '16' bytes might be read:"
    // It is complaining that CryptDecrypt() below might be reading more bytes
    // in buf than were returned by ReadFile() above.  This would never happen,
    // however, since the padding logic above would have copied the padding
    // data to the extra bytes.  Hence we disable the warning here.
#pragma warning(push)
#pragma warning(disable:6385)
    if (!CryptDecrypt(hKey, NULL, FALSE, 0, buf, &decDataSize) ||
        decDataSize != paddedSize)
#pragma warning(pop)
    {
      ret = NLSE_ENC_ERROR_CANT_DECRYPT_DATA;
      goto exit;
    }

    // Rewind the file pointer if we are reading from and writing to the same
    // file.
    if (outFile == inFile)
    {
      LARGE_INTEGER dist;

      dist.QuadPart = - (LONGLONG) bytesRead;
      if (SetFilePointerEx(outFile, dist, NULL, FILE_CURRENT) ==
          INVALID_SET_FILE_POINTER)
      {
        ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
        goto exit;
      }
    }

    // Write one block of data to outFile.
    if (!WriteFile(outFile, buf, bytesRead, &bytesWritten, NULL))
    {
      ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
      goto exit;
    }

    freeCryptoKey(hKey);
    hKey = NULL;
    writeRoundCount++;
  }

  // If we decrypted the input file in-place, delete the NLSE indicator in the
  // input file.
  if (outFile == inFile)
  {
    ret = deleteNLSEIndicator(inFileName);

    if (ret != NLSE_ENC_ERROR_SUCCESS)
    {
      goto exit;
    }
  }

  ret = NLSE_ENC_ERROR_SUCCESS;

exit:  
  if (hKey != NULL)
  {
    freeCryptoKey(hKey);
  }

  SecureZeroMemory(key, sizeof key);

  if (outFile != INVALID_HANDLE_VALUE && outFile != inFile)
  {
    CloseHandle(outFile);
  }

  if (inFile != INVALID_HANDLE_VALUE)
  {
    CloseHandle(inFile);
  }

  if (hProv != NULL)
  {
    freeCryptoContext(hProv);
  }

  CELOG_ENUM_RETURN_VAL(ret);
} /* doDecryptFileFW */

BOOL SE_EncryptFileFW( _In_z_ const wchar_t* in_path )
{
    CELOG_ENTER;
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls \n", in_path);
    CELOG_RETURN_VAL(SE_EncryptFileFWEx(in_path, FALSE));
}

BOOL SE_EncryptFileFWEx( _In_z_ const wchar_t* in_path, _In_ BOOL force )
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls, force=%ls \n", in_path, force?L"TRUE":L"FALSE");
	
  assert( in_path != NULL );
  if( in_path == NULL )
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFileFW: in_path is NULL\n");
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  /* Can encrypt this file? */
  if( can_encrypt(in_path) == false )
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFileFW: can_encrypt(%s) returned false\n",
                 in_path);
    SetLastError(ERROR_NOT_SUPPORTED);
    CELOG_RETURN_VAL(FALSE);
  }

  WCHAR parent_dir[MAX_PATH];
  PWCHAR pLastBS;

  wcsncpy_s(parent_dir, MAX_PATH, in_path, _TRUNCATE);
  pLastBS = wcsrchr(parent_dir, L'\\');
  if (pLastBS == NULL)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFileFW: can't find parent dir in %s\n",
                 in_path);
    SetLastError(ERROR_BAD_PATHNAME);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: parent_dir=%ls \n", parent_dir );
    CELOG_RETURN_VAL(FALSE);
  }
  *pLastBS = L'\0';

  // Determine the directory for creating the temp file.
  //
  // If the system temp directory exists and is on the same volume as the
  // original file, create the temp file in the system temp directory.
  // Otherwise, create the temp file in the directory of the original file.
  // This is because ReplaceFile() works only if the temp file is on the same
  // volume.
  WCHAR tmp_path[MAX_PATH] = L"", tmp_vol_path[MAX_PATH], parent_vol_path[MAX_PATH];
  DWORD dwRet;
  dwRet = GetTempPath(_countof(tmp_path), tmp_path);
  CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_EncryptFileFW: GetTempPath returned \"%s\"\n",
               tmp_path);

  if (dwRet == 0 ||
      !GetVolumePathName(tmp_path, tmp_vol_path,
                         _countof(tmp_vol_path)) ||
      !GetVolumePathName(parent_dir, parent_vol_path,
                         _countof(parent_vol_path)) ||
      _wcsicmp(tmp_vol_path, parent_vol_path) != 0)
  {
    wcsncpy_s(tmp_path, MAX_PATH, parent_dir, _TRUNCATE);
    CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_EncryptFileFW: using \"%s\" as temp path instead.\n",
                 tmp_path);
  }
  WCHAR tmp_file[MAX_PATH];   
  
  if (GetTempFileName(tmp_path, L"SE_", 0, tmp_file) == 0)
  {
    DWORD lastError = GetLastError();

    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFileFW: cannot create temp file (le %d)\n", 
                 lastError);
    SetLastError(lastError);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: parent_dir=%ls, tmp_path=%ls, dwRet=%lu, tmp_file=%ls \n", parent_dir,tmp_path,dwRet,tmp_file );
    CELOG_RETURN_VAL(FALSE);
  }

  // Create a unique empty backup file ourselves instead of letting
  // ReplaceFile() create it.  This way we don't have to worry about whether a
  // backup file name that we choose collides with any existing file in the
  // directory.
  WCHAR bak_file[MAX_PATH];
  
  if (GetTempFileName(parent_dir, L"SE_", 0, bak_file) == 0)
  {
    DWORD lastError = GetLastError();

    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFileFW: cannot create bak file (le %d)\n", 
                 lastError);
    DeleteFile(tmp_file);
	
	CELOG_LOG(CELOG_DUMP, L"Local variables are: parent_dir=%ls, tmp_path=%ls, dwRet=%lu, tmp_file=%ls, bak_file=%ls \n", parent_dir,tmp_path,dwRet,tmp_file,bak_file );
    SetLastError(lastError);
    CELOG_RETURN_VAL(FALSE);
  }

  // Starting from the introduction of the White-List feature in RMC 6.1, we
  // only support Heavy-Write-encrypying existing files that are already under
  // HW-DRM folders.  This usually occurs when the user runs
  // nlSysEncryption.exe to HW-encrypt a folder and then the files underneath
  // immediately afterwards.
  //
  // Here, the temp file that we create in the system temp directory will be
  // moved to the original file's directory.  Since we only support the case
  // where the original file's directory is already marked as HW-DRM, we can
  // simply create a non-encrypted temp file, and then let the SE HW filter
  // driver encrypt it when we move it to the original file's directory.
  if(!force)
  {
      CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_EncryptFileFW: About to call CopyFile().  in_path=\"%s\", tmp_file=\"%s\", bak_file=\"%s\"\n",
          in_path, tmp_file, bak_file);
      if (!CopyFile(in_path, tmp_file, FALSE))
      {
          DWORD lastError = GetLastError();

          CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFileFW: CopyFile(%s, %s) failed (le %lu)\n",
              in_path, tmp_file, lastError);
          DeleteFile(bak_file);
          DeleteFile(tmp_file);
		  CELOG_LOG(CELOG_DUMP, L"Local variables are: parent_dir=%ls, tmp_path=%ls, dwRet=%lu \n", parent_dir,tmp_path,dwRet );
          SetLastError(lastError);
          CELOG_RETURN_VAL(FALSE);
      }
  }
  else
  {
      CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_EncryptFileFW: About to call doEncryptFileFW().  in_path=\"%s\", tmp_file=\"%s\", bak_file=\"%s\"\n",
          in_path, tmp_file, bak_file);

      nlse_enc_error_t ret;

      ret = doEncryptFileFW(in_path, tmp_file);
      if (ret != NLSE_ENC_ERROR_SUCCESS)
      {
          CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFileFW: doEncryptFileFW(%s, %s) returned %d\n",
              in_path, tmp_file, ret);
          DeleteFile(bak_file);
          DeleteFile(tmp_file);
		  CELOG_LOG(CELOG_DUMP, L"Local variables are: parent_dir=%ls, tmp_path=%ls, dwRet=%lu \n", parent_dir,tmp_path,dwRet );
          SetLastError(ERROR_ACCESS_DENIED);
          CELOG_RETURN_VAL(FALSE);
      }
  }

  if (ReplaceFile(in_path, tmp_file, bak_file,
                  REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL))
  {
    // ReplaceFile() succeeded.

    // Delete the backup.  There should be no permission problem since
    // ReplaceFile() successfully opened this file with DELETE access rights.
    if (!DeleteFile(bak_file))
    {
      // This should never happen.
      DWORD lastError = GetLastError();

      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFileFW: Cannot delete %s (le %d)\n",
                   bak_file, lastError);
	
	  CELOG_LOG(CELOG_DUMP, L"Local variables are: parent_dir=%ls, tmp_path=%ls, dwRet=%lu \n", parent_dir,tmp_path,dwRet );
      SetLastError(lastError);
      CELOG_RETURN_VAL(FALSE);
    }
    else
    {
	  CELOG_LOG(CELOG_DUMP, L"Local variables are: parent_dir=%ls, tmp_path=%ls, dwRet=%lu \n", parent_dir,tmp_path,dwRet );
      CELOG_RETURN_VAL(TRUE);
    }
  }
  else
  {
    // ReplaceFile() failed.
    DWORD lastError = GetLastError();

    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFileFW: Cannot replace %s with %s (le %d)\n",
                 in_path, tmp_file, lastError);

    switch (lastError) {
    case ERROR_UNABLE_TO_MOVE_REPLACEMENT_2:
      // Original file has been renamed to backup file name.
      // Temp encrypted file is still under temp file name.
      //
      // Rename the backup file back to the original file name if possible.
      MoveFile(bak_file, in_path);
      break;

    case ERROR_UNABLE_TO_MOVE_REPLACEMENT:
    case ERROR_UNABLE_TO_REMOVE_REPLACED:
    default:
      // Original file is still under original name.
      // Temp encrypted file is still under temp file name.
      // Backup file may or may not exist (since we created the backup file
      // ourselves before calling ReplaceFile()).
      //
      // Delete the backup file if possible.
      DeleteFile(bak_file);
      break;
    }

    // For all error cases, delete the temp encrypted file if possible.
    DeleteFile(tmp_file);
	
	CELOG_LOG(CELOG_DUMP, L"Local variables are: parent_dir=%ls, tmp_path=%ls, dwRet=%lu \n", parent_dir,tmp_path,dwRet );
    // Restore the last-error from ReplaceFile().
    SetLastError(lastError);
    CELOG_RETURN_VAL(FALSE);
  }
}/* SE_EncryptFileFW */

BOOL SE_DecryptFileFW( _In_z_ const wchar_t* in_path, _In_opt_z_ const wchar_t* out_path,
                       _In_z_ const wchar_t* password )
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls, out_path=%ls, password=%ls \n", in_path,out_path,password);
	
  const BOOL useTempFile = (out_path == NULL || out_path[0] == L'\0');
  nlse_enc_error_t ret;
 
  assert( in_path != NULL );
  if( in_path == NULL )
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFileFW: in_path is NULL\n");
 	CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  if (is_NLSE_service_running())
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFileFW: NLSE service is still running\n");
 	CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
    SetLastError(ERROR_ACCESS_DENIED);
    CELOG_RETURN_VAL(FALSE);
  }

  ret = checkPassword(password);
  if (ret == NLSE_ENC_ERROR_INVALID_PASSWORD)
  {
  	CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
    SetLastError(ERROR_INVALID_PASSWORD);
    CELOG_RETURN_VAL(FALSE);
  }
  else if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFileFW: checkPassword() returned %d\n",
                 ret);
 	CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
    SetLastError(ERROR_ACCESS_DENIED);
    CELOG_RETURN_VAL(FALSE);
  }

  if (useTempFile)
  {
    //
    // Use temporary file for decryption.
    //

    WCHAR parent_dir[MAX_PATH], tmp_file[MAX_PATH], bak_file[MAX_PATH];
    PWCHAR pLastBS;

    wcsncpy_s(parent_dir, MAX_PATH, in_path, _TRUNCATE);
    pLastBS = wcsrchr(parent_dir, L'\\');
    if (pLastBS == NULL)
    {
      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFileFW: can't find parent dir in %s\n",
                   in_path);
	  CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
      SetLastError(ERROR_BAD_PATHNAME);
      CELOG_RETURN_VAL(FALSE);
    }
    *pLastBS = L'\0';

    if (GetTempFileName(parent_dir, L"SE_", 0, tmp_file) == 0)
    {
      DWORD lastError = GetLastError();

      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFileFW: cannot create temp file (le %d)\n", 
                   lastError);
	  CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
      SetLastError(lastError);
      CELOG_RETURN_VAL(FALSE);
    }

    // Create a unique empty backup file ourselves instead of letting
    // ReplaceFile() create it.  This way we don't have to worry about whether
    // a backup file name that we choose collides with any existing file in the
    // directory.
    if (GetTempFileName(parent_dir, L"SE_", 0, bak_file) == 0)
    {
      DWORD lastError = GetLastError();

      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFileFW: cannot create bak file (le %d)\n", 
                   lastError);
      DeleteFile(tmp_file);
	  CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
      SetLastError(lastError);
      CELOG_RETURN_VAL(FALSE);
    }

    CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_DecryptFileFW: About to call doDecryptFileFW().  in_path=\"%s\", tmp_file=\"%s\", bak_file=\"%s\"\n",
                 in_path, tmp_file, bak_file);

    ret = doDecryptFileFW(in_path, tmp_file);
    if (ret != NLSE_ENC_ERROR_SUCCESS)
    {
      DWORD lastError = GetLastError();

      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFileFW: doDecryptFileFW(%s, %s) returned %d\n",
                   in_path, tmp_file, ret);
      DeleteFile(bak_file);
      DeleteFile(tmp_file);
	  CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
      SetLastError(lastError);
      CELOG_RETURN_VAL(FALSE);
    }

    if (ReplaceFile(in_path, tmp_file, bak_file,
                    REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL))
    {
      // ReplaceFile() succeeded.

      // ReplaceFile() preserves the NLSE indicator stream in the original
      // file, which is not what we want.  So we delete the NLSE indicator
      // stream in the original file ourselves.
      ret = deleteNLSEIndicator(in_path);
      if (ret != NLSE_ENC_ERROR_SUCCESS)
      {
        // The original file has become decrypted, but the NLSE indicator can't
        // be deleted.  Just try our best to overwrite the decrypted original
        // file with the backup file.
        DWORD lastError = GetLastError();

        CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFileFW: deleteNLSEIndicator(%s) returned %d\n",
                     in_path, ret);
        ReplaceFile(in_path, bak_file, NULL, REPLACEFILE_IGNORE_MERGE_ERRORS,
                    NULL, NULL);
	 	CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
        SetLastError(lastError);
        CELOG_RETURN_VAL(FALSE);
      }

      // Delete the backup.  There should be no permission problem since
      // ReplaceFile() successfully opened this file with DELETE access rights.
      if (!DeleteFile(bak_file))
      {
        // This should never happen.
        DWORD lastError = GetLastError();

        CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFileFW: Cannot delete %s (le %d)\n",
                     bak_file, lastError);
	 	CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
        SetLastError(lastError);
        CELOG_RETURN_VAL(FALSE);
      }
      else
      {
	  	CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
        CELOG_RETURN_VAL(TRUE);
      }
    }
    else
    {
      // ReplaceFile() failed.
      DWORD lastError = GetLastError();

      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFileFW: Cannot replace %s with %s (le %d)\n",
                   in_path, tmp_file, lastError);

      switch (lastError) {
      case ERROR_UNABLE_TO_MOVE_REPLACEMENT_2:
        // Original file has been renamed to backup file name.
        // Temp encrypted file is still under temp file name.
        //
        // Rename the backup file back to the original file name if possible.
        MoveFile(bak_file, in_path);
        break;

      case ERROR_UNABLE_TO_MOVE_REPLACEMENT:
      case ERROR_UNABLE_TO_REMOVE_REPLACED:
      default:
        // Original file is still under original name.
        // Temp encrypted file is still under temp file name.
        // Backup file may or may not exist (since we created the backup file
        // ourselves before calling ReplaceFile()).
        //
        // Delete the backup file if possible.
        DeleteFile(bak_file);
        break;
      }

      // For all error cases, delete the temp encrypted file if possible.
      DeleteFile(tmp_file);
	  
	  CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
      // Restore the last-error from ReplaceFile().
      SetLastError(lastError);
      CELOG_RETURN_VAL(FALSE);
    }
  }
  else
  {
    //
    // Use file specified by caller for decryption.
    //
    CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_DecryptFileFW: About to call doDecryptFileFW().  in_path=\"%s\", out_path=\"%s\"\n",
                 in_path, out_path);

    ret = doDecryptFileFW(in_path, out_path);
    if (ret != NLSE_ENC_ERROR_SUCCESS)
    {
      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFileFW: doDecryptFileFW(%s, %s) returned %d\n",
                   in_path, out_path, ret);
    }
 	CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
    CELOG_RETURN_VAL(ret == NLSE_ENC_ERROR_SUCCESS);
  }
}/* SE_DecryptFileFW */

BOOL SE_GetFileInfoFW( const wchar_t* in_file , NextLabs_FileInfo_t in_file_info , void* info )
{
  CELOG_ENTER;
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_file=%ls \n", in_file);
  
  assert( in_file != NULL );
  assert( info != NULL );
  if( in_file == NULL || info == NULL )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  NextLabsFile_Header_1_0_t* nl        = (NextLabsFile_Header_1_0_t*)info;
  NextLabsEncryptionFile_Header_1_0_t* se = (NextLabsEncryptionFile_Header_1_0_t*)info;

  std::wstring fname(in_file);
  std::wstring::size_type off = fname.find_last_of(L"\\/");

  if( off != std::wstring::npos )
  {
    fname.erase(0,off+1);  /* trim leading directory to get the file name only */
  }

  std::wstring ind_name;
  if( getNLSEIndicatorName(in_file,TRUE,ind_name) == false )
  {
	CELOG_LOG(CELOG_DUMP, L"Local variables are: fname=%ls, ind_name=%ls \n", fname.c_str(),ind_name.c_str() );
    CELOG_RETURN_VAL(FALSE);
  }

  HANDLE hFile = CreateFileW(ind_name.c_str(),
			     GENERIC_READ,FILE_SHARE_READ,NULL,
			     OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if( hFile == INVALID_HANDLE_VALUE )
  {
 	CELOG_LOG(CELOG_DUMP, L"Local variables are: fname=%ls, ind_name=%ls, hFile=%p \n", fname.c_str(),ind_name.c_str(),hFile );
    CloseHandle(hFile);
    CELOG_RETURN_VAL(FALSE);
  }

  DWORD bytes_to_read = sizeof(NextLabsFile_Header_1_0_t) + sizeof(NextLabsEncryptionFile_Header_1_0_t);
  DWORD bytes_read = 0;
  unsigned char* buf = NULL;

  buf = (unsigned char*)malloc(bytes_to_read);
  if( buf == NULL )
  {
 	CELOG_LOG(CELOG_DUMP, L"Local variables are: fname=%ls, ind_name=%ls, hFile=%p, bytes_to_read=%lu, bytes_read=%lu, buf=0x%p\n", fname.c_str(),ind_name.c_str(),hFile,bytes_to_read,bytes_read,buf );
    CloseHandle(hFile);
    SetLastError(ERROR_INSUFFICIENT_BUFFER);
    CELOG_RETURN_VAL(FALSE);
  }
  memset(buf,0x00,bytes_to_read);

  /* Set address of information needed for NL and SE */
  nl = (NextLabsFile_Header_1_0_t*)buf;
  se = (NextLabsEncryptionFile_Header_1_0_t*)((unsigned char*)buf + sizeof(NextLabsFile_Header_1_0_t));

  /* Read the information from the stream */
  BOOL rv = ReadFile(hFile,(LPVOID)buf,bytes_to_read,&bytes_read,NULL);
  if( rv == FALSE && GetLastError() != ERROR_HANDLE_EOF )
  {
 	CELOG_LOG(CELOG_DUMP, L"Local variables are: fname=%ls, ind_name=%ls, hFile=%p, bytes_to_read=%lu, bytes_read=%lu, buf=0x%p, rv=%ls\n", fname.c_str(),ind_name.c_str(),hFile,bytes_to_read,bytes_read,buf,rv?L"TRUE":L"FALSE" );
    free(buf);
    CloseHandle(hFile);
    CELOG_RETURN_VAL(FALSE);
  }

  if( in_file_info == SE_FileInfo_NextLabs )
  {
    memcpy(info,nl,sizeof(NextLabsFile_Header_1_0_t));
  }
  else if( in_file_info == SE_FileInfo_NextLabsEncryption )
  {
    memcpy(info,se,sizeof(NextLabsEncryptionFile_Header_1_0_t));
  }
  CELOG_LOG(CELOG_DUMP, L"Local variables are: fname=%ls, ind_name=%ls, hFile=%p, bytes_to_read=%lu, bytes_read=%lu, buf=0x%p, rv=%ls\n", fname.c_str(),ind_name.c_str(),hFile,bytes_to_read,bytes_read,buf,rv?L"TRUE":L"FALSE" );
  free(buf);
  CloseHandle(hFile);
  SetLastError(ERROR_SUCCESS);
  CELOG_RETURN_VAL(TRUE);
}/* SE_GetFileInfoFW */

void SE_PurgeFWDRMList()
{
  CELOG_ENTER;
  nlthread_mutex_lock(&_fwglobal.drmFWPathListMutex);
  if(NULL != _fwglobal.drmFWPathList) free(_fwglobal.drmFWPathList);
  _fwglobal.drmFWPathList    = NULL;
  _fwglobal.drmFWPathListLen = 0;
  nlthread_mutex_unlock(&_fwglobal.drmFWPathListMutex);
  CELOG_RETURN;
}
