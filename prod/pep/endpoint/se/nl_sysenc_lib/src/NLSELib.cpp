#define WINVER _WIN32_WINNT_WINXP
#pragma warning( push )
#pragma warning( disable : 4005 )
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#pragma warning( pop )
#define NTDDI_VERSION NTDDI_WINXPSP2
#include <windows.h>
#pragma warning( push )
#pragma warning( disable : 4005 )
#include <fltuser.h>
#pragma warning( pop )
#include <NTSecAPI.h>
#include <WinCrypt.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "celog.h"
#include "nlthread.h"
#include "nlconfig.hpp"
#include "NLSECommon.h"
#include "nl_sysenc_lib.h"
#include "NLSESDKWrapper.h"
#include "NLSELib.h"
#include "eframework/platform/rights_management_client.hpp"
#include "eframework/platform/rights_management_server.hpp"
#include "AltDataStream.h"
#include "shellapi.h"
#include "nextlabs_features.h"
#include "nextlabs_feature_manager.hpp"

extern bool is_in_DRM_FW_path_list( _In_z_ const wchar_t* in_path );


using std::wstring;
//
// Kernel-mode defines
//

// (Copied from inc/ddk/wdm.h)
// Define the create disposition values
#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005



#define CELOG_CUR_FILE \
  CELOG_FILEPATH_PROD_PEP_ENDPOINT_SE_NL_SYSENC_LIB_SRC_NLSELIB_CPP



#define AES_BLOCK_SIZE 16

#define NLSE_BLK_SIZE_IN_BYTES          512     // Encryption of each block
                                                // will be init'ed using an
                                                // Initial Vector which is the
                                                // file offset of the block.



#define NLSE_CFG_FILE_KEY_SHARED_KEY_RING   L"SharedKeyRing="



typedef struct {
  NextLabsFile_TYPE fileType;
  NLFSE_ENCRYPT_EXTENSION encExt;
  NLFSE_TAG_EXTENSION tagExt;
} NLSEIndicator_t;

typedef struct {
  NextLabsFile_TYPE fileType;
  NLFSE_ENCRYPT_EXTENSION encExt;
} NLSEPartialIndicator_t;

C_ASSERT(offsetof(NLSEPartialIndicator_t, fileType) ==
         offsetof(NLSEIndicator_t, fileType));
C_ASSERT(offsetof(NLSEPartialIndicator_t, encExt) ==
         offsetof(NLSEIndicator_t, encExt));

typedef struct _NLSE_BLOB
{
  BLOBHEADER    hdr;
  DWORD         dwKeySize;
  BYTE          keyData[NLE_KEY_LENGTH_IN_BYTES];
} NLSE_BLOB;

#define ROUND_UP(x, n)  (((x + n - 1) / n) * n)


typedef enum
{
  CEFKO_KEEP,                       // keep key currently in file
  CEFKO_SWITCH_TO_SHARED,           // switch to latest shared key
  CEFKO_SWITCH_TO_LOCAL             // switch to latest local key
} copy_encrypted_file_key_option_t;

#define BUF_SIZE 1048576            // 1MB

class CGlobalTLS
{
public:
    CGlobalTLS(){
        dwTlsIndex = ::TlsAlloc();
        drmPathList = NULL;
        drmPathListLen = 0;
        nlthread_mutex_init(&drmPathListMutex);
    };

    ~CGlobalTLS(){
        nlthread_mutex_destroy(&drmPathListMutex);
        ::TlsFree(dwTlsIndex);
        dwTlsIndex = TLS_OUT_OF_INDEXES;
    };
public:
    DWORD dwTlsIndex;

    WCHAR *drmPathList;             // Blob of DRM paths, concatenated
                                    // together and separated by
                                    // null-terminators
    int drmPathListLen;             // # wchar's in drmPaths
    nlthread_mutex_t drmPathListMutex;
};

static CGlobalTLS _global;

//SetLastError
void  SE_SetLastError(nlse_enc_error_t errorcode)
{
	CELOG_ENTER;
	nlse_enc_error_t *pErrorcode = static_cast<nlse_enc_error_t*>(::TlsGetValue(_global.dwTlsIndex));
	if (pErrorcode != NULL)
	{
		*pErrorcode = errorcode;
	}
	else
	{
		pErrorcode = new nlse_enc_error_t( errorcode );		// Free when the process exit.
	}
	
	if(!::TlsSetValue(_global.dwTlsIndex ,pErrorcode))
	{
		//OutputDebugStringW(L"TlsSetValue Error\n");
	}
	CELOG_RETURN;
}

//GetLastError
nlse_enc_error_t SE_GetLastError( )
{
	CELOG_ENUM_ENTER(nlse_enc_error_t);
	nlse_enc_error_t* pErrorcode = static_cast<nlse_enc_error_t*>(::TlsGetValue(_global.dwTlsIndex));
	if (pErrorcode != NULL)
	{
		nlse_enc_error_t errorCode= *pErrorcode;
		CELOG_ENUM_RETURN_VAL(errorCode);
	}
	CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_SUCCESS);
}
//DisplayErrorMessage
void SE_DisplayErrorMessage(_In_ nlse_enc_error_t errorcode)
{
	CELOG_ENTER;
	switch(errorcode)
	{
	case NLSE_ENC_ERROR_INVALID_KEY_RING:
		MessageBoxW(NULL, L"Error: You do not have access to the correct key to decrypt this file. Please contact your system administrator.", L"Error", MB_OK|MB_ICONERROR);
		break;
	case NLSE_ENC_ERROR_CANT_CONNECT_PC:
		MessageBoxW(NULL, L"Error: Key Management needs to be running in order to perform this function.", L"Error", MB_OK|MB_ICONERROR);
		break;
	case NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE:  //Delete localkeyring When call SE_Unwrapxxx() funtion fail it alway return errorcode 13
		MessageBoxW(NULL, L"Error: Local key is not available.", L"Error", MB_OK|MB_ICONERROR);
		break;
	case NLSE_ENC_ERROR_CANT_GET_LOCAL_KEY:
		MessageBoxW(NULL, L"Error: Local key is not available.", L"Error", MB_OK|MB_ICONERROR);
		break;
	case NLSE_ENC_ERROR_CANT_GET_SHARED_KEY:
		MessageBoxW(NULL,  L"Error: You do not have access to the correct key to decrypt this file. Please contact your system administrator.", L"Error", MB_OK|MB_ICONERROR);
		break;
	case NLSE_ENC_ERROR_INVALID_FEATURE_ENCRYPTION_SYSTEM:
		MessageBoxW(NULL, L"Error: System Encyption is required in order to perform this fuction.", L"Error", MB_OK|MB_ICONERROR);
		break;
	default:
		break;
	}
	CELOG_RETURN;
}

/************************************************************************/
// Define DEBUG Flags
/************************************************************************/
#ifdef _DEBUG

/************************************************************************/
// In debug mode, developer can enable/disable these flags
/************************************************************************/
#define NLSE_DEBUG_FAKE_FILE_KEY            FALSE
#define NLSE_DEBUG_FAKE_PC_KEY              FALSE
#define NLSE_DEBUG_CRYPTO_PASSTHROUGH       FALSE

#else

/************************************************************************/
// In release mode, these flags should always be FALSE
// DON'T change them
/************************************************************************/
#define NLSE_DEBUG_FAKE_FILE_KEY            FALSE
#define NLSE_DEBUG_FAKE_PC_KEY              FALSE
#define NLSE_DEBUG_CRYPTO_PASSTHROUGH       FALSE

#endif /* _DEBUG */

#if (NLSE_DEBUG_FAKE_FILE_KEY || NLSE_DEBUG_FAKE_PC_KEY || NLSE_DEBUG_CRYPTO_PASSTHROUGH)
#  ifndef _DEBUG
#    error NLSE ERROR - Cannot set debug options for a release library.
#  endif
#endif



static nlse_enc_error_t isFileNLSEEncrypted(LPCWSTR fileName, PBOOL pEncrypted);
static BOOL nlse_findstream(_In_z_ const wchar_t* file);

static bool seLibInitialized = false;

static const WCHAR wildcardStr[] = L"*";
static bool sharedKeyRingRetrieved = false;
static char sharedKeyRing[NLE_KEY_RING_NAME_MAX_LEN];
#ifndef MAX_TIMEELAPSED 
#define MAX_TIMEELAPSED  300000
#endif
static DWORD dTimeElapsed = 0 ;

// ====================================================
// BEGIN OF: Declare Local Routines
_Check_return_
static
BOOL
SE_GetNxlFileAttributes(
                        _In_z_ LPCWSTR wzFile,
                        _Out_ PDWORD pdwAttrs
                        );

_Check_return_
static
BOOL
SE_SetNxlFileAttributes(
                        _In_z_ LPCWSTR wzFile,
                        _In_ DWORD dwAttrs
                        );

_Check_return_
static
BOOL
SE_SetWrappedFileFlags(
                       _In_z_ LPCWSTR wzFile
                       );

_Check_return_
static
BOOL
SE_ClearWrappedFileFlags(
                       _In_z_ LPCWSTR wzFile
                       );
// END OF: Declare Local Routines
// ====================================================



/*
 * Ensure that module is initialized
 */
int ensureInited(void)
{
  CELOG_ENTER;
  if (seLibInitialized) {
    CELOG_RETURN_VAL(0);
  }

  seLibInitialized = true;
  CELOG_LOG(CELOG_INFO, L"NLSELib log initialized.\n");
  CELOG_LOGA(CELOG_DEBUG, "%d.%d.%d.%d (Built %s %s)\n",
               VERSION_MAJOR, VERSION_MINOR,
               VERSION_MAINTENANCE, VERSION_PATCH,
               __DATE__, __TIME__);

#ifdef _DEBUG
  CELOG_LOG(CELOG_WARNING, L"********************************************************************************\n");
  CELOG_LOG(CELOG_WARNING, L"* NextLabs System Encryption Library\n");
  CELOG_LOG(CELOG_WARNING, L"*\n");
  CELOG_LOG(CELOG_WARNING, L"* WARNING: The library is compiled with debug parameters\n");
  CELOG_LOG(CELOG_WARNING, L"*\n");

#if NLSE_DEBUG_FAKE_FILE_KEY
  CELOG_LOG(CELOG_WARNING, L"*   NLSE_DEBUG_FAKE_FILE_KEY on\n");
#else
  CELOG_LOG(CELOG_WARNING, L"*   NLSE_DEBUG_FAKE_FILE_KEY off\n");
#endif

#if NLSE_DEBUG_FAKE_PC_KEY
  CELOG_LOG(CELOG_WARNING, L"*   NLSE_DEBUG_FAKE_PC_KEY on\n");
#else
  CELOG_LOG(CELOG_WARNING, L"*   NLSE_DEBUG_FAKE_PC_KEY off\n");
#endif

  CELOG_LOG(CELOG_WARNING, L"*\n");
  CELOG_LOG(CELOG_WARNING, L"********************************************************************************\n");
#endif /* _DEBUG */

  CELOG_RETURN_VAL(0);
} /* ensureInited */

static int ensureDrmPathListRetrieved(void)
{
  CELOG_ENTER;
  int drmPathListSize;
  CEResult_t result;
  int ret = -1;

  nlthread_mutex_lock(&_global.drmPathListMutex);

  if (_global.drmPathList != NULL)
  {
    ret = 0;
    goto exit;
  }

  result = CESDKDrmGetPaths(&_global.drmPathList, &drmPathListSize);
  if (result != CE_RESULT_SUCCESS)
  {
    goto exit;
  }

  _global.drmPathListLen = drmPathListSize / sizeof *_global.drmPathList;

  if (_global.drmPathList[_global.drmPathListLen - 1] != L'\0')
  {
    // The last path is not null-terminated.  The list is corrupted.
    CELOG_LOG(CELOG_ERROR, L"NLSELib!ensureDrmPathListRetrieved: path list is corrutped\n");
    free(_global.drmPathList);
    _global.drmPathList = NULL;
    _global.drmPathListLen = 0;
    goto exit;
  }

  ret = 0;

exit:
  nlthread_mutex_unlock(&_global.drmPathListMutex);
  CELOG_RETURN_VAL(ret);
} /* ensureDrmPathListRetrieved */

static int ensureSharedKeyRingNameRetrieved(void)
{
  CELOG_ENTER;
	DWORD dTick = GetTickCount() ;
	if (sharedKeyRingRetrieved&& (dTimeElapsed-dTick)<MAX_TIMEELAPSED)
	{
		CELOG_RETURN_VAL(0);
	}
	dTimeElapsed = dTick ;
	// If any error occurs below while reading "SharedKeyRing=xxx" from the
  // config file, we will use the default shared key ring name.
  sharedKeyRingRetrieved = true;
  C_ASSERT(sizeof NLE_KEY_RING_SHARE_DEFAULT - sizeof '\0' <=
           sizeof sharedKeyRing);
  memset(sharedKeyRing, '\0', sizeof sharedKeyRing);
  memcpy(sharedKeyRing, NLE_KEY_RING_SHARE_DEFAULT,
         strlen(NLE_KEY_RING_SHARE_DEFAULT));

  char file[MAX_PATH] = {0};
  WCHAR nlse_root[MAX_PATH] = {0};
  if( NLConfig::GetComponentInstallPath(L"Enterprise DLP\\System Encryption",nlse_root,_countof(nlse_root)) == false )
  {
    wcsncpy_s(nlse_root,_countof(nlse_root),L"C:\\Program Files\\NextLabs\\System Encryption",_TRUNCATE);
  }
  _snprintf_s(file,_countof(file), _TRUNCATE,"%ws\\config\\SystemEncryption.cfg",nlse_root);

  FILE *f;
  errno_t err;

  err = fopen_s(&f, file, "rt, ccs=UTF-8");
  if (err != 0 || f == NULL) {
    CELOG_LOGA(CELOG_WARNING, "NLSELib!ensureSharedKeyRingNameRetrieved: cannot open %s\n", file);
    goto exit;
  }

  for (;;)
  {
	  wchar_t buf[1024]={0};

    // Read one line from file.
    if (fgetws(buf, _countof(buf), f) == NULL)
    {
      break;
    }

    // Remove newline at the end, if any.
    if (buf[wcslen(buf) - 1] == L'\n')
    {
      buf[wcslen(buf) - 1] = L'\0';
    }
        
    // Discard the line if we don't recognize the key.
    if (wcsncmp(buf, NLSE_CFG_FILE_KEY_SHARED_KEY_RING,
                wcslen(NLSE_CFG_FILE_KEY_SHARED_KEY_RING)) != 0)
    {
      continue;
    }

    // Convert the key ring name to chars.
    const wchar_t *name = buf + wcslen(NLSE_CFG_FILE_KEY_SHARED_KEY_RING);
	char tempSharedKeyRing[NLE_KEY_RING_NAME_MAX_LEN] ={0} ;
    size_t i;

    if (wcslen(name) > sizeof(sharedKeyRing))
    {
      CELOG_LOG(CELOG_ERROR, L"NLSELib!ensureSharedKeyRingNameRetrieved: shared key ring name more than %u characters\n", sizeof(sharedKeyRing));
      break;
    }
    else if (wcslen(name) == 0)
    {
      CELOG_LOG(CELOG_ERROR, L"NLSELib!ensureSharedKeyRingNameRetrieved: shared key ring name is empty\n");
      break;
    }

    memset(tempSharedKeyRing, '\0', sizeof tempSharedKeyRing);
    for (i = 0; i < wcslen(name); i++)
    {
      int c = wctob(name[i]);
      if (c == -1)
      {
        CELOG_LOG(CELOG_ERROR, L"NLSELib!ensureSharedKeyRingNameRetrieved: shared key ring name contains non-SBCS characters\n");
        break;
      }
      tempSharedKeyRing[i] = (char) c;
    }

    if (i == wcslen(name))
    {
      // Shared key ring name in cfg file is good.  Store it.
      memcpy(sharedKeyRing, tempSharedKeyRing, sizeof sharedKeyRing);
    }
  }

exit:
    if (f != NULL)
    {
      fclose(f);
    }
    CELOG_RETURN_VAL(0);
} /* ensureSharedKeyRingNameRetrieved */
/*
*\brief : judge if current os is 64bit
* \return TRUE is 64bit otherwise, it is 32bit
*/
BOOL Is64bitSystem()
{
	CELOG_ENTER;
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);

	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||    
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
	{
		CELOG_RETURN_VAL(TRUE);
	}
	else
	{
		CELOG_RETURN_VAL(FALSE);
	} 
}
/** is_protected_path
 *
 *  \brief Determine if the given path is protected from being encrypted.  For example,
 *         "Program Files" is protected.
 *
 *  \retrun true if the path is protected, otherwise false.
 */
static bool is_protected_path( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  wchar_t prog_files[MAX_PATH] = {0};
  wchar_t sys_root[MAX_PATH] = {0};
  wchar_t tmp_path[MAX_PATH] = {0};

  if( GetEnvironmentVariableW(L"ProgramFiles",prog_files,_countof(prog_files)) == 0 )
  {
    CELOG_RETURN_VAL(false);
  }
  if( GetWindowsDirectoryW(sys_root,_countof(sys_root)) == 0 )
  {
    CELOG_RETURN_VAL(false);
  }
  if( GetTempPath(_countof(tmp_path),tmp_path) == 0 )
  {
    CELOG_RETURN_VAL(false);
  }
  else if( tmp_path[wcslen(tmp_path) - 1] == L'\\' )
  {
    // Remove the '\' that was added by GetTempPath().
    tmp_path[wcslen(tmp_path) - 1] = L'\0';
  }

  wchar_t in_path_long[MAX_PATH] = {0};
  wchar_t prog_files_long[MAX_PATH] = {0};
  wchar_t sys_root_long[MAX_PATH] = {0};
  wchar_t tmp_path_long[MAX_PATH] = {0};

  if( GetLongPathName(in_path,in_path_long,_countof(in_path_long)) == 0 )
  {
    CELOG_RETURN_VAL(false);
  }
  if( GetLongPathName(prog_files,prog_files_long,_countof(prog_files_long)) == 0 )
  {
    CELOG_RETURN_VAL(false);
  }
  if( GetLongPathName(sys_root,sys_root_long,_countof(sys_root_long)) == 0 )
  {
    CELOG_RETURN_VAL(false);
  }
  if( GetLongPathName(tmp_path,tmp_path_long,_countof(tmp_path_long)) == 0 )
  {
    CELOG_RETURN_VAL(false);
  }

  /* Pattern of root path such as { C: , C:\ } */
  if( wcslen(in_path) <= 3 && in_path[1] == L':' )
  {
    CELOG_RETURN_VAL(true);
  }

  /* Both the protected and input paths must contain a trailing slash when they
   * are directories.  This will prevent a false positive for matching if a path
   * exists in a protected directory such as:
   *
   *  C:\Windows Fake\ vs. C:\Windows
   */

  wcsncat_s(prog_files_long,MAX_PATH, L"\\", _TRUNCATE);
  wcsncat_s(sys_root_long,MAX_PATH,L"\\", _TRUNCATE);
  wcsncat_s(tmp_path_long,MAX_PATH,L"\\", _TRUNCATE);

  bool result = false;
  std::wstring in_path_cpp(in_path_long);

  DWORD attrs = GetFileAttributesW(in_path);
  if( attrs == INVALID_FILE_ATTRIBUTES )
  {
    CELOG_RETURN_VAL(true);
  }
  bool is_directory = (attrs & FILE_ATTRIBUTE_DIRECTORY);
  if( is_directory && boost::algorithm::iends_with(in_path_cpp,L"\\") == false )
  {
    in_path_cpp.append(L"\\");
  }

  // here we have no API to get the wow6432 program path, so we 
  // fake create it for bug of 14379
  if(Is64bitSystem())
  {
	wstring strwow6432prog_path(prog_files_long);
#define PROGRAM_WOW6432	L"Program Files (x86)"
#define PROGEAM_64		L"Program Files"
	boost::algorithm::ireplace_first(strwow6432prog_path,PROGEAM_64,PROGRAM_WOW6432);
	if(PathFileExists(strwow6432prog_path.c_str()))
	{
		if(boost::algorithm::iequals(in_path,strwow6432prog_path.c_str()) ||
			boost::algorithm::istarts_with(in_path_cpp,strwow6432prog_path.c_str()))
		{
			CELOG_RETURN_VAL(true);
		}
	}
  }


  /* Exact match for protected path such as C:\Windows */
  if( boost::algorithm::iequals(in_path,prog_files_long) == true ||
      boost::algorithm::iequals(in_path,sys_root_long) == true ||
      boost::algorithm::iequals(in_path,tmp_path_long) == true )
  {
    CELOG_RETURN_VAL(true);
  }

  /* Check to see if this path resides in a protected directory.
   */
  if( boost::algorithm::istarts_with(in_path_cpp,prog_files_long) == true ||
      boost::algorithm::istarts_with(in_path_cpp,sys_root_long) == true ||
      boost::algorithm::istarts_with(in_path_cpp,tmp_path_long) == true )
  {
    CELOG_RETURN_VAL(true);
  }

  CELOG_RETURN_VAL(result);
}/* is_protected_path */

/** get_next_path_component
 *
 *  \brief Get the next component in the path, starting at the passed index.
 *
 *  \param in_path (in)     Path from which to extract the next component
 *  \param index (inout)    Pointer to index.  On entry, it contains the index
 *                          at which the next compoment is to be eextracted.
 *  \param component (out)  String to store the next component, or empty string
 *                          if the next component is empty (e.g.
 *                          C:\Dir1\Dir2\\Dir4).  Caller needs to free the
 *                          memory allocated for this string.
 *
 *  \return TRUE on success (including the case where the next component is
 *                          empty), otherwise FALSE.
 */
bool get_next_path_component(_In_z_ const wchar_t* in_path,
                             _Inout_ size_t *index,
                             _Out_ wchar_t** component)
{
  CELOG_ENTER;
  size_t i;
  size_t len = wcslen(in_path);

  if (*index >= len)
  {
    // Invalid parameter error.
    CELOG_RETURN_VAL(false);
  }

  for (i = *index; i < len; i++)
  {
    if (in_path[i] == L'\\')
    {
      break;
    }
  }

  // We have reached either a '\' or the end of the string.  Return the
  // (maybe empty) component.

  if (i == *index)
  {
    // Component is empty.  Return empty component.
    *component = NULL;
    CELOG_RETURN_VAL(true);
  }

  // Return non-empty component.
  *component = (wchar_t *) malloc((i - *index + 1) * sizeof **component);
  if (*component == NULL)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSELib!get_next_path_component: cannot allocate memory\n");
    CELOG_RETURN_VAL(false);
  }
  wcsncpy_s(*component, i - *index + 1, &in_path[*index], _TRUNCATE);

  *index = i;

  CELOG_RETURN_VAL(true);
} /* get_next_path_component */

/** is_matching_wildcard_path
 *
 *  \brief Check is the path matches a path containing wildcards.
 *
 *  \param in_path (in)         The path to check.
 *  \param wildcard_path (in)   The wildcard of which to check against.
 *
 *  \return TRUE if match, FALSE if not match or error.
 */
bool is_matching_wildcard_path(_In_z_ const wchar_t* in_path,
                               _In_ const wchar_t* wildcard_path)
{
  CELOG_ENTER;
  size_t index1 = 0, index2 = 0;

  while (1)
  {
    bool endReached1, endReached2;
    wchar_t *component1, *component2;
    bool componentMatch;

    endReached1 = (index1 >= wcslen(in_path));
    endReached2 = (index2 >= wcslen(wildcard_path));

    if (endReached1 && endReached2)
    {
      // Both paths have reached the end.  The paths match.
      CELOG_RETURN_VAL(true);
    }
    else if (endReached1 || endReached2)
    {
      // One, but not both, path has reached the end.  The paths don't match.
      CELOG_RETURN_VAL(false);
    }

    // Neither paths have reached the end.  See if the components between the
    // two paths match.

    if (!get_next_path_component(in_path, &index1, &component1))
    {
      // Error.
      CELOG_RETURN_VAL(false);
    }
 
    if (!get_next_path_component(wildcard_path, &index2, &component2))
    {
      // Error.
      if (component1 != NULL)
      {
        free(component1);
      }
      CELOG_RETURN_VAL(false);
    }

    componentMatch = ((component2 != NULL &&
                       _wcsicmp(component2, wildcardStr) == 0) ||
                      ((component2 == NULL && component1 == NULL) ||
                       (component2 != NULL && component1 != NULL &&
                        _wcsicmp(component2, component1) == 0)));

    // Free the components if they are not empty.
    if (component2 != NULL)
    {
      free(component2);
    }
    if (component1 != NULL)
    {
      free(component1);
    }

    if (!componentMatch)
    {
      // This component doesn't match.  The paths don't match.
      CELOG_RETURN_VAL(false);
    }

    // Skip the component separator (either a '\' or the wchar after the last
    // wchar in the paths).
    index1++;
    index2++;
  }
} /* is_matching_wildcard_path */

/** is_in_DRM_path_list
 *
 *  \brief Determine if the given path matches a path in the DRM path list
 *
 *  \retrun true if the path matches, otherwise false.
 */
static bool is_in_DRM_path_list( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  if(in_path == NULL || in_path[0] == NULL) CELOG_RETURN_VAL(false);

  if (ensureDrmPathListRetrieved() != 0)
  {
    // Error.
    CELOG_RETURN_VAL(false);
  }

  nlthread_mutex_lock(&_global.drmPathListMutex);

  // Get head of list.
  WCHAR *entry = _global.drmPathList;

  // Stop if end of list is reached.
  while (entry != _global.drmPathList + _global.drmPathListLen)
  {
    if (_wcsicmp(in_path, entry) == 0 ||
        is_matching_wildcard_path(in_path, entry))
    {
      nlthread_mutex_unlock(&_global.drmPathListMutex);
      CELOG_RETURN_VAL(true);
    }

    // Get next list entry.
    entry += wcslen(entry) + 1;
  }

  nlthread_mutex_unlock(&_global.drmPathListMutex);
  CELOG_RETURN_VAL(false);
}/* is_in_DRM_path_list */

//Send user mode command to kernel
static int WriteCommand( NLSE_USER_COMMAND * cmd )
{
  CELOG_ENTER;
  HRESULT           hr;
  HANDLE            port;
  NLSE_PORT_CONTEXT portCtx;
  int               result = 0;

  if(cmd->type == NLSE_USER_COMMAND_ENABLE_FILTER ||
     cmd->type == NLSE_USER_COMMAND_DISABLE_FILTER) {
    portCtx.portTag=NLSE_PORT_TAG_MAIN_CMD;
  } else {
    portCtx.portTag=NLSE_PORT_TAG_DRM;
  }
  hr = FilterConnectCommunicationPort(NLSE_PORT_NAME,0,
				      &portCtx,
				      sizeof(portCtx),
				      NULL,
				      &port);
  if( !IS_ERROR(hr) && port != NULL) {
    DWORD result_size;
    hr = FilterSendMessage(port,cmd,sizeof(*cmd),NULL,0,&result_size);
    if( !IS_ERROR(hr) ) {
      result = 1;
    }
  } else {
    CELOG_LOG(CELOG_CRITICAL, L"NLSELib!WriteCommand: ConnectCommunicationPort failed 0x%08X\n", hr);
  }    

  if( port != INVALID_HANDLE_VALUE ) {
    CloseHandle(port);
  }

  if(result == 0) {
    CELOG_LOG(CELOG_ERROR, L"NLSELib!WriteCommand failed: 0x%08X\n", hr);
  }

  CELOG_RETURN_VAL(result);
}/*WriteCommand */

void init_cmd( NLSE_USER_COMMAND *cmd )
{
  CELOG_ENTER;
  memset(cmd,0x00,sizeof(NLSE_USER_COMMAND));
  CELOG_RETURN;
}/* init_cmd */

BOOL SE_IsWrappedFile( _In_z_ const wchar_t* path )
{
  CELOG_ENTER;
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: path=%ls \n", path);
  if (!SE_IsNXLFile(path))
  {
    CELOG_RETURN_VAL(FALSE);
  }

  NextLabsFile_Header_t info;

  if (!SE_GetFileInfo(path, SE_FileInfo_NextLabs, &info))
  {
    CELOG_RETURN_VAL(FALSE);
  }

  SetLastError(ERROR_SUCCESS);
  CELOG_RETURN_VAL((info.flags & NLF_WRAPPED) != 0);
}/* SE_IsWrappedFile */

BOOL SE_IsNXLFile( _In_z_ const wchar_t* path )
{
  CELOG_ENTER;
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: path=%ls \n", path);
  // Since NLSE 5.0, the only NextLabs files that NLSE/NLE/RM supports are the
  // NextLabs encrypted ones.  (This may change in the future.)  So here we
  // just call SE_IsEncrypted() to do the checking.
  CELOG_RETURN_VAL(SE_IsEncrypted(path));
}/* SE_IsNXLFile */

BOOL SE_IsEncrypted( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls \n", in_path);
  CELOG_RETURN_VAL(SE_IsEncryptedEx(in_path, TRUE));
}/* SE_IsEncrypted */

BOOL SE_IsEncryptedEx( _In_z_ const wchar_t* in_path,
                       _In_ BOOL skip_if_rm_not_installed )
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls, skip_if_rm_not_installed=%ls \n", in_path,skip_if_rm_not_installed?L"TRUE":L"FALSE");
  if (skip_if_rm_not_installed &&
      !(nextlabs::rmc::is_rmc_installed() || nextlabs::rms::is_rms_installed()))
  {
    CELOG_LOG(CELOG_INFO, L"NLSE!SE_IsEncryptedEx: Neither RMC nor RMS is installed\n");
    SetLastError(ERROR_NOT_SUPPORTED);
    CELOG_RETURN_VAL(FALSE);
  }

  assert( in_path != NULL );
  if( in_path == NULL )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  if(in_path[0]==NULL || _wcsicmp(in_path,L"\\") == 0 ||
	  _wcsicmp(in_path,L"\\\\") == 0 )
    CELOG_RETURN_VAL(FALSE);

  BOOL is_encrypted = FALSE, is_nonExisting = FALSE;
  DWORD attrs = GetFileAttributesW(in_path);
  if( attrs == INVALID_FILE_ATTRIBUTES )
  {
	  if(GetLastError() == ERROR_ACCESS_DENIED)
      {
          // Try to verify this file using driver
          if( isFileNLSEEncrypted(in_path, &is_encrypted) == NLSE_ENC_ERROR_SUCCESS)
              goto success;
          else
              CELOG_RETURN_VAL(FALSE);
      }
    is_nonExisting = TRUE;
  }

  bool is_directory = (attrs & FILE_ATTRIBUTE_DIRECTORY) || is_nonExisting;

  if ( !is_directory )
  {
    if( isFileNLSEEncrypted(in_path, &is_encrypted) == NLSE_ENC_ERROR_SUCCESS)
    {
      goto success;
    }
  }

  /* Check DRM path list if this is a directory and is not encrypted. */
  if( is_directory == true && is_encrypted == FALSE )
  {  
    is_encrypted = is_in_DRM_path_list(in_path) || is_in_DRM_FW_path_list(in_path);
  }

  /* Check parent if this is a directory and is not encrypted. */
  if( is_directory == true && is_encrypted == FALSE )
  {
    std::wstring dir(in_path);
    std::wstring::size_type off = dir.find_last_of(L"\\/");
    if( off != std::wstring::npos )
    {
      dir.erase(off,dir.length());  /* trim leading directory to get the file name only */
      BOOL is_enc = SE_IsEncrypted(dir.c_str());

      /* If director is SE encrypted return true to avoid walking up to another
	 path which may not be encrypted.  If any of the parents is encrypted,
	 then this directory should be reported as encrypted.
       */
      if( is_enc == TRUE )
      {
        is_encrypted = TRUE;
        goto success;
      }
    }
  }

success:
  CELOG_LOG(CELOG_DUMP, L"Local variables are: is_encrypted=%ls, is_nonExisting=%ls, attrs=%lu \n", is_encrypted?L"TRUE":L"FALSE",is_nonExisting?L"TRUE":L"FALSE",attrs );
  SetLastError(ERROR_SUCCESS);
  CELOG_RETURN_VAL(is_encrypted);
}/* SE_IsEncryptedEx */

/* Determine if the given path resides on a local disk.
 */
static bool is_local_disk( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  if( wcslen(in_path) < 3 )
  {
    CELOG_RETURN_VAL(false);
  }

  wchar_t dletter[32] = {0};
  wcsncpy_s(dletter,_countof(dletter),in_path,3);

  UINT dt = GetDriveType(dletter);

  bool result = false;
  switch( dt )
  {
  case DRIVE_UNKNOWN:
  case DRIVE_NO_ROOT_DIR:
  case DRIVE_REMOTE:
    result = false;
    break;

  case DRIVE_REMOVABLE:
  case DRIVE_FIXED:
  case DRIVE_CDROM:
  case DRIVE_RAMDISK:
    result = true;
    break;
  }
  CELOG_RETURN_VAL(result);
}/* is_local_disk */

/* Determine if the given path resides on a local fixed disk.
 */
static bool is_local_fixed_disk( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  if( wcslen(in_path) < 3 )
  {
    CELOG_RETURN_VAL(false);
  }

  wchar_t dletter[32] = {0};
  wcsncpy_s(dletter,_countof(dletter),in_path,3);

  UINT dt = GetDriveType(dletter);

  bool result = false;
  switch( dt )
  {
  case DRIVE_UNKNOWN:
  case DRIVE_NO_ROOT_DIR:
  case DRIVE_REMOVABLE:
  case DRIVE_REMOTE:
  case DRIVE_CDROM:
  case DRIVE_RAMDISK:
    result = false;
    break;

  case DRIVE_FIXED:
    result = true;
    break;
  }
  CELOG_RETURN_VAL(result);
}/* is_local_fixed_disk */

/* Determine if the given path is EFS-encrypted
 */
static bool is_efs_encrypted( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  DWORD attrs = GetFileAttributesW(in_path);

  CELOG_RETURN_VAL( attrs != INVALID_FILE_ATTRIBUTES &&
           (attrs & FILE_ATTRIBUTE_ENCRYPTED) != 0 );
}/* is_efs_encrypted */

/* Determine if the given path is NTFS-compressed
 */
static bool is_ntfs_compressed( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  DWORD attrs = GetFileAttributesW(in_path);

  CELOG_RETURN_VAL( attrs != INVALID_FILE_ATTRIBUTES &&
           (attrs & FILE_ATTRIBUTE_COMPRESSED) != 0 );
}/* is_ntfs_compressed */

/* Determine if the given path is read-only
 */
static bool is_read_only( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  DWORD attrs = GetFileAttributesW(in_path);

  CELOG_RETURN_VAL( attrs != INVALID_FILE_ATTRIBUTES &&
           (attrs & FILE_ATTRIBUTE_READONLY) != 0 );
}/* is_read_only */

bool real_can_encrypt( _In_z_ const wchar_t* in_path, _In_ bool Force = false )
{
	CELOG_ENTER;
	if( is_local_disk(in_path) == false ||
		is_efs_encrypted(in_path) == true ||
		is_ntfs_compressed(in_path) == true ||
		is_read_only(in_path) == true )
	{
		CELOG_RETURN_VAL(false);
	}

	/* Restricted path such as Windows root? */
	if (!Force)
	{
		if( is_protected_path(in_path) == true )
		{
			CELOG_RETURN_VAL(false);
		}
	}

	CELOG_RETURN_VAL(true);
}

bool can_encryptForce( _In_z_ const wchar_t* in_path)
{
	CELOG_ENTER;
	CELOG_RETURN_VAL(real_can_encrypt(in_path, true));
}/* can_encrypt */

/* Determine if the given path can be NLSE-encrypted
 */
bool can_encrypt( _In_z_ const wchar_t* in_path )
{
	CELOG_ENTER;
	CELOG_RETURN_VAL(real_can_encrypt(in_path));
}/* can_encrypt */

/** SE_EncryptDirectory
 *
 *  \brief attach DRM attribute to  the input directory 
 *
 *  \return TRUE on success.
 */
BOOL SE_EncryptDirectory( _In_z_ const wchar_t* dName )
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: dName=%ls \n", dName);
  BOOL status = FALSE;
  CEResult_t rv;

  assert( dName != NULL );
  if( dName == NULL )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
    CELOG_RETURN_VAL(FALSE);
  }

  /* Can encrypt this dir? */
  if( can_encrypt(dName) == false )
  {
    SetLastError(ERROR_NOT_SUPPORTED);
    CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
    CELOG_RETURN_VAL(FALSE);
  }

  rv = CESDKDrmAddPath(dName);
  if( rv == CE_RESULT_SUCCESS )
  {
    status = TRUE;
  }
  CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
  CELOG_RETURN_VAL(status);

}/* SE_EncryptDirectory */

/** SE_DecryptDirectory
 *
 *  \brief remove DRM attribute from the input directory 
 *
 *  \return TRUE on success.
 */
BOOL SE_DecryptDirectory( _In_z_ const wchar_t* dName,
                          _In_z_ const wchar_t* password )
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: dName=%ls, password=%ls \n", dName,password);
  
  BOOL status = FALSE;
  nlse_enc_error_t ret;

  assert( dName != NULL );
  if( dName == NULL )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
    CELOG_RETURN_VAL(FALSE);
  }

  if (is_NLSE_service_running())
  {
    SetLastError(ERROR_ACCESS_DENIED);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
    CELOG_RETURN_VAL(FALSE);
  }

  ret = checkPassword(password);
  if (ret == NLSE_ENC_ERROR_INVALID_PASSWORD)
  {
    SetLastError(ERROR_INVALID_PASSWORD);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
    CELOG_RETURN_VAL(FALSE);
  }
  else if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    SetLastError(ERROR_ACCESS_DENIED);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
    CELOG_RETURN_VAL(FALSE);
  }

  CEResult_t rv;
  rv = CESDKDrmRemovePath(dName);
  if( rv == CE_RESULT_SUCCESS )
  {
    status = TRUE;
  }
  CELOG_LOG(CELOG_DUMP, L"Local variables are: status=%ls \n", status?L"TRUE":L"FALSE" );
  CELOG_RETURN_VAL(status);
}/* SE_DecryptDirectory */

bool is_NLSE_service_running(void)
{
  CELOG_ENTER;
  SERVICE_STATUS_PROCESS ssStatus; 
  SC_HANDLE schSCManager;
  SC_HANDLE schService;
  DWORD dwBytesNeeded;

  // Get a handle to the SCM database. 
  schSCManager = OpenSCManager(NULL,    // local computer
			       NULL,    // servicesActive database 
			       SC_MANAGER_ENUMERATE_SERVICE|SC_MANAGER_QUERY_LOCK_STATUS);
  if (NULL == schSCManager) {
    CELOG_RETURN_VAL(false);
  }

  // Get a handle to the service.
  schService = OpenService(schSCManager,         // SCM database 
			   L"nlsysencryption",   // name of service 
			   SERVICE_QUERY_STATUS);
  if (schService == NULL) {
    CloseServiceHandle(schSCManager);
    CELOG_RETURN_VAL(false);
  }   

  // Check the service status 
  if (!QueryServiceStatusEx(schService, // handle to service 
			    SC_STATUS_PROCESS_INFO,     // information level
			    (LPBYTE) &ssStatus,         // address of structure
			    sizeof(SERVICE_STATUS_PROCESS), //size of structure
			    &dwBytesNeeded )) {
    // size needed if buffer is too small
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
    CELOG_RETURN_VAL(false); 
  }

  CloseServiceHandle(schService); 
  CloseServiceHandle(schSCManager);

  // STOPPED and PAUSED are considered not running.  All other states,
  // including all PENDING ones, are considered running.
  CELOG_RETURN_VAL(!(ssStatus.dwCurrentState == SERVICE_STOPPED ||
           ssStatus.dwCurrentState == SERVICE_PAUSED));
} /* is_NLSE_service_running */

nlse_enc_error_t checkPassword(_In_z_ const wchar_t *password)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  CEResult_t result;
  nlse_enc_error_t ret;

  result = CESDKMakeProcessTrusted(password);
  if (result != CE_RESULT_SUCCESS)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!checkPassword: CESDKMakeProcessTrusted() returned %d\n",
                 result);
  }

  switch (result)
  {
  case CE_RESULT_SUCCESS:
    ret = NLSE_ENC_ERROR_SUCCESS;
    break;
  case CE_RESULT_PERMISSION_DENIED:
    ret = NLSE_ENC_ERROR_INVALID_PASSWORD;
    break;
  default:
	SE_SetLastError(NLSE_ENC_ERROR_CANT_CONNECT_PC);
    ret = NLSE_ENC_ERROR_CANT_CONNECT_PC;
    break;
  }

  CELOG_ENUM_RETURN_VAL(ret);
} /* checkPassword */

nlse_enc_error_t initCryptoContext(HCRYPTPROV *phProv)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  // Acquire crypto context for AES Encryption, any provider.
  if (!CryptAcquireContext(phProv, NULL, NULL, PROV_RSA_AES,
                           CRYPT_VERIFYCONTEXT))
  {
    CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_CANT_ACQUIRE_CRYPT_CONTEXT);
  }

  CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_SUCCESS);
} /* initCryptoContext */

nlse_enc_error_t freeCryptoContext(HCRYPTPROV hProv)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  if (!CryptReleaseContext(hProv, 0))
  {
    CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_CANT_RELEASE_CRYPT_CONTEXT);
  }

  CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_SUCCESS);
} /* freeCryptoContext */

nlse_enc_error_t initCryptoKey(HCRYPTPROV hProv,
                                      const BYTE key[NLE_KEY_LENGTH_IN_BYTES],
                                      ULONGLONG initVector, HCRYPTKEY *phKey)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  NLSE_BLOB blob;
  BYTE iv[AES_BLOCK_SIZE];
  nlse_enc_error_t ret;

  // Set up blob and import key.
  blob.hdr.bType = PLAINTEXTKEYBLOB;
  blob.hdr.bVersion = CUR_BLOB_VERSION;
  blob.hdr.reserved = 0;
  blob.hdr.aiKeyAlg = CALG_AES_128;
  blob.dwKeySize = sizeof blob.keyData;
  memcpy(blob.keyData, key, sizeof blob.keyData);

  if (!CryptImportKey(hProv, (BYTE *) &blob, sizeof blob, 0, 0, phKey))
  {
    ret = NLSE_ENC_ERROR_CANT_INIT_KEY;
    goto cleanup;
  }

  // Set up initialization vector in key.
  memset(iv, 0x00, sizeof iv);
  *((ULONGLONG *) iv) = initVector;

  if (!CryptSetKeyParam(*phKey, KP_IV, iv, 0))
  {
    CryptDestroyKey(*phKey);
    *phKey = NULL;
    ret = NLSE_ENC_ERROR_CANT_INIT_KEY;
    goto cleanup;
  }

  ret = NLSE_ENC_ERROR_SUCCESS;

cleanup:
  SecureZeroMemory(blob.keyData, sizeof blob.keyData);
  CELOG_ENUM_RETURN_VAL(ret);
} /* initCryptoKey */

nlse_enc_error_t freeCryptoKey(HCRYPTKEY hKey)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  if (!CryptDestroyKey(hKey))
  {
    CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_CANT_DESTROY_KEY);
  }

  CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_SUCCESS);
} /* freeCryptoKey */

static nlse_enc_error_t getSharedKeyRingName(char pcKeyRingName[NLE_KEY_RING_NAME_MAX_LEN])
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  if (ensureSharedKeyRingNameRetrieved() != 0)
  {
	SE_SetLastError(NLSE_ENC_ERROR_CANT_GET_SHARED_KEY);
    CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_CANT_GET_SHARED_KEY);
  }

  memcpy(pcKeyRingName, sharedKeyRing, NLE_KEY_RING_NAME_MAX_LEN);
  CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_SUCCESS);
} /* getSharedKeyRingName */

nlse_enc_error_t getPCKey(const char *pcKeyRingName,
                          NLSE_KEY_ID *pcKeyId,
                          unsigned char pcKey[NLE_KEY_LENGTH_IN_BYTES])
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
#if NLSE_DEBUG_FAKE_PC_KEY

  memset(pcKey, 'a', NLE_KEY_LENGTH_IN_BYTES);
  CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_SUCCESS);

#else


  NLSE_MESSAGE inMsg, outMsg;

  int len = (int)strlen(pcKeyRingName);

  if (len > NLE_KEY_RING_NAME_MAX_LEN)
  {
	SE_SetLastError(NLSE_ENC_ERROR_INVALID_KEY_RING);
    CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_INVALID_KEY_RING);
  }

  memset(inMsg.keyRingName, '\0', NLE_KEY_RING_NAME_MAX_LEN);
  memcpy(inMsg.keyRingName, pcKeyRingName, len);

  inMsg.keyID = *pcKeyId;
  inMsg.pid = GetCurrentProcessId();
  CEResult_t result;
  nlse_enc_error_t ret;

  result = CESDKKeyManagementAPI(&inMsg, &outMsg);
  if (result != CE_RESULT_SUCCESS)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!getPCKey: CESDKKeyManagementAPI() returned %d\n",
                 result);
    ret = NLSE_ENC_ERROR_CANT_GET_PC_KEY;
    goto cleanup;
  }

  CopyMemory(pcKey, outMsg.key, NLE_KEY_LENGTH_IN_BYTES);
  *pcKeyId = outMsg.keyID;
  ret = NLSE_ENC_ERROR_SUCCESS;

cleanup: 
  SecureZeroMemory(outMsg.key, sizeof outMsg.key);
  CELOG_ENUM_RETURN_VAL(ret);

#endif /* NLSE_DEBUG_FAKE_PC_KEY */
} /* getPCKey */

static BOOL nlse_findstream(_In_z_ const wchar_t* file)
{
    CELOG_ENTER;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD  dwDesiredAccess = FILE_READ_ATTRIBUTES|STANDARD_RIGHTS_READ;
    DWORD  dwShareMode     = FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE;
    std::wstring strStream  = file;    

    strStream += L":nlse_stream";
    hFile = ::CreateFileW(strStream.c_str(), dwDesiredAccess, dwShareMode, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE!=hFile)
    {
        CloseHandle(hFile);
        CELOG_RETURN_VAL(TRUE);
    }

    CELOG_RETURN_VAL(FALSE);
}

static nlse_enc_error_t isFileNLSEEncrypted(LPCWSTR fileName, PBOOL pEncrypted)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  HANDLE han = INVALID_HANDLE_VALUE;
  nlse_enc_error_t ret;

  if(nlse_findstream(fileName))
  {
      *pEncrypted = TRUE;
      CELOG_ENUM_RETURN_VAL(NLSE_ENC_ERROR_SUCCESS);
  }
  
  BOOL useRawRead = is_NLSE_service_running() && is_local_disk(fileName);

  if (useRawRead)
  {
    wchar_t indicatorName[MAX_PATH] = {0};
    wcsncpy_s(indicatorName,MAX_PATH, fileName, _TRUNCATE);

    if (!SE_CreateFileRaw(indicatorName, wcslen(indicatorName), GENERIC_READ,
                          0, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING,
                          &han))
    {
      goto commError;
    }

    // If the volume is not attached by the filter driver, switch back to Win32
    // File API.
    // (In WinXP and Win2003, LsaNtStatusToWinError() translates
    // STATUS_FLT_INSTANCE_NOT_FOUND to ERROR_MR_MID_NOT_FOUND.  So we have to
    // check for that also.)
    if (han == INVALID_HANDLE_VALUE &&
        (GetLastError() == ERROR_FLT_INSTANCE_NOT_FOUND ||
         GetLastError() == ERROR_MR_MID_NOT_FOUND))
    {
      useRawRead = FALSE;
    }
  }

  if (!useRawRead)
  {
    han = CreateFile(fileName, GENERIC_READ,
                     FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                     0, NULL);
  }

  if (han == INVALID_HANDLE_VALUE)
  {
    // Either the file itself doesn't exist or it can't be read.  Return
    // error in both cases.
    ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
    goto exit;
  }

  // Read the indicator.
  NLSEPartialIndicator_t indicatorBuf;
  DWORD bytesRead;

  if (useRawRead)
  {
    if (!SE_ReadFileRaw(han, 0, sizeof indicatorBuf, &indicatorBuf,
                        sizeof indicatorBuf, &bytesRead))
    {
      goto commError;
    }
  }
  else
  {
    if (!ReadFile(han, &indicatorBuf, sizeof indicatorBuf, &bytesRead, NULL))
    {
      // Can't read the indicator stream.  Return error.
      ret = NLSE_ENC_ERROR_CANT_READ_INDICATOR;
      goto exit;
    }
  }

  if (bytesRead == 0 && GetLastError() != ERROR_HANDLE_EOF)
  {
    // Can't read the indicator.  Return error.
    ret = NLSE_ENC_ERROR_CANT_READ_INDICATOR;
    goto exit;
  }
  else if (bytesRead < sizeof indicatorBuf)
  {
    // File too short.  Return "not encrypted".
    *pEncrypted = FALSE;
    ret = NLSE_ENC_ERROR_SUCCESS;
    goto exit;
  }

  if (memcmp(indicatorBuf.fileType.cookie, NL_FILE_COOKIE,
             sizeof indicatorBuf.fileType.cookie) != 0)
  {
    // Cookie doesn't match.  Return "not encrypted".
    *pEncrypted = FALSE;
    ret = NLSE_ENC_ERROR_SUCCESS;
    goto exit;
  }

  if (indicatorBuf.fileType.version_major != NL_FILE_VERSION_MAJOR ||
      indicatorBuf.fileType.header_size != sizeof(NextLabsFile_TYPE) ||
      indicatorBuf.fileType.stream_count != 2)
  {
    // Filetype version or header size or stream count not supported.  Return
    // error.
    ret = NLSE_ENC_ERROR_UNSUPPORTED_FILETYPE;
    goto exit;
  }

  if (strcmp((const char *) indicatorBuf.encExt.sh.stream_name,
             NLE_STREAM_NAME) != 0)
  {
    // Stream name doesn't match.  Return "not encrypted".
    *pEncrypted = FALSE;
    ret = NLSE_ENC_ERROR_SUCCESS;
    goto exit;
  }

  if (indicatorBuf.encExt.version_major != NLE_FILE_VERSION_MAJOR ||
      indicatorBuf.encExt.sh.stream_size != sizeof(NLFSE_ENCRYPT_EXTENSION))
  {
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
    if (useRawRead)
    {
      SE_CloseFileRaw(han);
    }
    else
    {
      CloseHandle(han);
    }
  }

  CELOG_ENUM_RETURN_VAL(ret);

commError:
  ret = NLSE_ENC_ERROR_PLUGIN_COMM;
  goto exit;
} /* isFileNLSEEncrypted */

//Initialize NextLabs File Type structure
static VOID initializeNLFileType(NextLabsFile_TYPE *t)
{
  CELOG_ENTER;
  ZeroMemory(t, sizeof(NextLabsFile_TYPE));
  t->version_major=NL_FILE_VERSION_MAJOR;
  t->version_minor=NL_FILE_VERSION_MINOR;
  t->header_size = sizeof *t;
  t->stream_count=0;
  CopyMemory(t->cookie, NL_FILE_COOKIE, sizeof t->cookie);
  CELOG_RETURN;
} /* initializeNLFileType */

static VOID initializeNLFSEEncryptExtension(PNLFSE_ENCRYPT_EXTENSION pExt)
{
  CELOG_ENTER;
  ZeroMemory(pExt, sizeof(NLFSE_ENCRYPT_EXTENSION));  
  pExt->sh.stream_size = sizeof(NLFSE_ENCRYPT_EXTENSION);
  strncpy_s((char *) pExt->sh.stream_name, sizeof NLE_STREAM_NAME,
           NLE_STREAM_NAME, _TRUNCATE);
  pExt->version_major=NLE_FILE_VERSION_MAJOR;
  pExt->version_minor=NLE_FILE_VERSION_MINOR;
  pExt->flags=0;
  CELOG_RETURN;
}/* initializeNLFSEEncryptExtension */

static VOID initializeNLFSETagExtension(PNLFSE_TAG_EXTENSION pExt)
{
  CELOG_ENTER;
  ZeroMemory(pExt, sizeof(NLFSE_TAG_EXTENSION));  
  pExt->sh.stream_size = sizeof(NLFSE_TAG_EXTENSION);
  strncpy_s((char *) pExt->sh.stream_name, sizeof NLT_STREAM_NAME,
           NLT_STREAM_NAME, _TRUNCATE);
  pExt->version_major=NLT_FILE_VERSION_MAJOR;
  pExt->version_minor=NLT_FILE_VERSION_MINOR;
  pExt->tagsSize = 0;
  CELOG_RETURN;
}/* initializeNLFSETagExtension */

static nlse_enc_error_t readNLSEIndicator(
    HCRYPTPROV hProv, LPCWSTR fileName, PULONGLONG pFileRealLength,
    BYTE key[NLE_KEY_LENGTH_IN_BYTES], PULONG pPaddingLen,
    PBYTE pPaddingData)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  NLSEPartialIndicator_t indicatorBuf;
  HANDLE indicatorStream = INVALID_HANDLE_VALUE;
  HCRYPTKEY hPCKey = NULL;
  DWORD bytesRead;
  nlse_enc_error_t ret;

  BOOL useRawRead = is_NLSE_service_running() && is_local_disk(fileName);

  if (useRawRead)
  {
    wchar_t indicatorName[MAX_PATH] = {0};
    wcsncpy_s(indicatorName,MAX_PATH,fileName, _TRUNCATE);

    if (!SE_CreateFileRaw(indicatorName, wcslen(indicatorName), GENERIC_READ,
                          0, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING,
                          &indicatorStream))
    {
      goto commError;
    }

    // If the volume is not attached by the filter driver, switch back to Win32
    // File API.
    // (In WinXP and Win2003, LsaNtStatusToWinError() translates
    // STATUS_FLT_INSTANCE_NOT_FOUND to ERROR_MR_MID_NOT_FOUND.  So we have to
    // check for that also.)
    if (indicatorStream == INVALID_HANDLE_VALUE &&
        (GetLastError() == ERROR_FLT_INSTANCE_NOT_FOUND ||
         GetLastError() == ERROR_MR_MID_NOT_FOUND))
    {
      useRawRead = FALSE;
    }
  }

  if (!useRawRead)
  {
    indicatorStream = CreateFile(fileName, GENERIC_READ,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                 OPEN_EXISTING, 0, NULL);
  }

  if (indicatorStream == INVALID_HANDLE_VALUE)
  {
    ret = NLSE_ENC_ERROR_CANT_READ_INDICATOR;
    goto cleanup;
  }

  if (useRawRead)
  {
    if (!SE_ReadFileRaw(indicatorStream, 0, sizeof indicatorBuf, &indicatorBuf,
                        sizeof indicatorBuf, &bytesRead))
    {
      goto commError;
    }
  }
  else
  {
    if (!ReadFile(indicatorStream, &indicatorBuf, sizeof indicatorBuf,
                  &bytesRead, NULL))
    {
      ret = NLSE_ENC_ERROR_CANT_READ_INDICATOR;
      goto cleanup;
    }
  }

  if (bytesRead != sizeof indicatorBuf)
  {
    ret = NLSE_ENC_ERROR_CANT_READ_INDICATOR;
    goto cleanup;
  }

  *pFileRealLength = indicatorBuf.encExt. fileRealLength;
  *pPaddingLen = indicatorBuf.encExt.paddingLen;
  CopyMemory(pPaddingData, indicatorBuf.encExt.paddingData, *pPaddingLen);

  // Decrypt data encryption key with the PC key that was used to encrypt it.

  CopyMemory(key, indicatorBuf.encExt.key, NLE_KEY_LENGTH_IN_BYTES);

  char tempPcKeyRingName[NLE_KEY_RING_NAME_MAX_LEN + 1];
  unsigned char pcKey[NLE_KEY_LENGTH_IN_BYTES];

  CopyMemory(tempPcKeyRingName, indicatorBuf.encExt.pcKeyRingName,
             NLE_KEY_RING_NAME_MAX_LEN);
  tempPcKeyRingName[NLE_KEY_RING_NAME_MAX_LEN] = '\0';
  ret = getPCKey(tempPcKeyRingName,
                 &indicatorBuf.encExt.pcKeyID, pcKey);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
	if(ret == NLSE_ENC_ERROR_CANT_GET_PC_KEY)
	{
		//To tell the difference required key
		if(_stricmp(tempPcKeyRingName,NLE_KEY_RING_LOCAL) == 0)
		{
			SE_SetLastError(NLSE_ENC_ERROR_CANT_GET_LOCAL_KEY);
		}
		else
		{
			SE_SetLastError(NLSE_ENC_ERROR_CANT_GET_SHARED_KEY);
		}
	}
    goto cleanup;
  }

  ret = initCryptoKey(hProv, pcKey, 0, &hPCKey);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto cleanup;
  }

  DWORD encDataSize = NLE_KEY_LENGTH_IN_BYTES;

#if NLSE_DEBUG_CRYPTO_PASSTHROUGH
  /* Do nothing*/
#else
  if (!CryptDecrypt(hPCKey, NULL, FALSE, 0, key, &encDataSize))
  {
    ret = NLSE_ENC_ERROR_CANT_DECRYPT_DATA_KEY;
    goto cleanup;
  }
#endif /* NLSE_DEBUG_CRYPTO_PASSTHROUGH */

  ret = NLSE_ENC_ERROR_SUCCESS;

cleanup:
  if (hPCKey != NULL)
  {
    freeCryptoKey(hPCKey);
  }

  SecureZeroMemory(pcKey, sizeof pcKey);

  if (indicatorStream != INVALID_HANDLE_VALUE)
  {
    if (useRawRead)
    {
      SE_CloseFileRaw(indicatorStream);
    }
    else
    {
      CloseHandle(indicatorStream);
    }
  }

  CELOG_ENUM_RETURN_VAL(ret);

commError:
  ret = NLSE_ENC_ERROR_PLUGIN_COMM;
  goto cleanup;
} /* readNLSEIndicator */

static nlse_enc_error_t writeNLSEIndicator(
    HCRYPTPROV hProv, LPCWSTR fileName, ULONGLONG fileRealLength,
    const BYTE key[NLE_KEY_LENGTH_IN_BYTES], ULONG paddingLen,
    const BYTE *paddingData, BOOL useLocalKey, BOOL writeNLEOnly)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  NLSEIndicator_t indicatorBuf;
  HANDLE indicatorStream = INVALID_HANDLE_VALUE;
  HCRYPTKEY hPCKey = NULL;
  DWORD bytesWritten;
  nlse_enc_error_t ret;

  const BOOL useRawWrite = nextlabs::rmc::is_rmc_installed();

  if (!writeNLEOnly)
  {
    initializeNLFileType(&indicatorBuf.fileType);
    indicatorBuf.fileType.stream_count = 2;
    initializeNLFSETagExtension(&indicatorBuf.tagExt);
  }

  initializeNLFSEEncryptExtension(&indicatorBuf.encExt);

  if (useLocalKey)
  {
    strncpy_s(indicatorBuf.encExt.pcKeyRingName, NLE_KEY_RING_NAME_MAX_LEN, NLE_KEY_RING_LOCAL, _TRUNCATE);
  }
  else
  {
    ret = getSharedKeyRingName(indicatorBuf.encExt.pcKeyRingName);
    if (ret != NLSE_ENC_ERROR_SUCCESS)
    {
      goto cleanup;
    }
  }

  indicatorBuf.encExt.fileRealLength = fileRealLength;
  indicatorBuf.encExt.paddingLen = paddingLen;
  CopyMemory(indicatorBuf.encExt.paddingData, paddingData, paddingLen);

  // Encrypt data encryption key with latest PC key, and store the
  // corresponding key ID.

  CopyMemory(indicatorBuf.encExt.key, key, sizeof indicatorBuf.encExt.key);

  char tempPcKeyRingName[NLE_KEY_RING_NAME_MAX_LEN + 1];
  unsigned char pcKey[NLE_KEY_LENGTH_IN_BYTES];

  CopyMemory(tempPcKeyRingName, indicatorBuf.encExt.pcKeyRingName,
             NLE_KEY_RING_NAME_MAX_LEN);
  tempPcKeyRingName[NLE_KEY_RING_NAME_MAX_LEN] = '\0';
  ZeroMemory(&indicatorBuf.encExt.pcKeyID, sizeof indicatorBuf.encExt.pcKeyID);
  ret = getPCKey(tempPcKeyRingName,
                 &indicatorBuf.encExt.pcKeyID, pcKey);
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

#if NLSE_DEBUG_CRYPTO_PASSTHROUGH
  /* Do nothing*/
#else
  if (!CryptEncrypt(hPCKey, NULL, FALSE, 0, indicatorBuf.encExt.key,
                    &encDataSize, sizeof indicatorBuf.encExt.key))
  {
    ret = NLSE_ENC_ERROR_CANT_ENCRYPT_DATA_KEY;
    goto cleanup;
  }
#endif /* NLSE_DEBUG_CRYPTO_PASSTHROUGH */

  if (useRawWrite)
  {
    wchar_t indicatorName[MAX_PATH] = {0};
    wcsncpy_s(indicatorName,MAX_PATH,fileName, _TRUNCATE);

    if (!SE_CreateFileRaw(indicatorName, wcslen(indicatorName), GENERIC_WRITE,
                          FILE_ATTRIBUTE_NORMAL, 0, OPEN_ALWAYS,
                          &indicatorStream))
    {
      goto commError;
    }
  }
  else
  {
    indicatorStream = CreateFile(fileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL, NULL);
	int n=0;
	while(n++ < 5 && indicatorStream == INVALID_HANDLE_VALUE)
	{
		Sleep(500);
		indicatorStream = CreateFile(fileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
	}
  }

  if (indicatorStream == INVALID_HANDLE_VALUE)
  {
    ret = NLSE_ENC_ERROR_CANT_WRITE_INDICATOR;
    goto cleanup;
  }

  PVOID bufToWrite;
  ULONG offsetToWrite;
  ULONG sizeToWrite;

  if (writeNLEOnly)
  {
    bufToWrite = &indicatorBuf.encExt;
    offsetToWrite = offsetof(NLSEIndicator_t, encExt);
    sizeToWrite = sizeof indicatorBuf.encExt;
  }
  else
  {
    bufToWrite = &indicatorBuf;
    offsetToWrite = 0;
    sizeToWrite = sizeof indicatorBuf;
  }

  if (useRawWrite)
  {
    if (!SE_WriteFileRaw(indicatorStream, offsetToWrite, sizeToWrite,
                         bufToWrite, sizeToWrite, &bytesWritten))
    {
      goto commError;
    }
  }
  else
  {
    if ((SetFilePointer(indicatorStream, offsetToWrite, NULL, FILE_BEGIN) ==
         INVALID_SET_FILE_POINTER) ||
        !WriteFile(indicatorStream, bufToWrite, sizeToWrite,
                   &bytesWritten, NULL))
    {
      ret = NLSE_ENC_ERROR_CANT_WRITE_INDICATOR;
      goto cleanup;
    }
  }

  if (bytesWritten != sizeToWrite)
  {
    ret = NLSE_ENC_ERROR_CANT_WRITE_INDICATOR;
    goto cleanup;
  }

  ret = NLSE_ENC_ERROR_SUCCESS;



cleanup:
  if (indicatorStream != INVALID_HANDLE_VALUE)
  {
    if (useRawWrite)
    {
      SE_CloseFileRaw(indicatorStream);
    }
    else
    {
      CloseHandle(indicatorStream);
    }
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
} /* writeNLSEIndicator */

VOID WINAPI KeepProcessTrusted()
{
    CELOG_ENTER;
    HANDLE              hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    wfd   = {0};

    // This function will cause WDE doing evaluation
	hFind = FindFirstFileEx( L"C:\\*", 
		FindExInfoStandard, 
		&wfd, 
		FindExSearchNameMatch, 
		NULL, 
		0 );
    if(INVALID_HANDLE_VALUE != hFind) FindClose(hFind);
    CELOG_RETURN;
}

/** doEncryptFile
 *
 * \brief Encrypt a file to a different file name.
 */
static nlse_enc_error_t doEncryptFile(LPCWSTR inFileName,
                                      LPCWSTR outFileName,
                                      BOOL useLocalKey)
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

  const BOOL useRawWrite = nextlabs::rmc::is_rmc_installed();

  ret = isFileNLSEEncrypted(inFileName, &encrypted);

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

#if NLSE_DEBUG_FAKE_FILE_KEY
  // Use fixed fake data key.
  ((ULONG *) key)[0] = (ULONG)'z';
  ((ULONG *) key)[1] = (ULONG)'z';
  ((ULONG *) key)[2] = (ULONG)'z';
  ((ULONG *) key)[3] = (ULONG)'z';
#else
  // Generate random data key.
  if (!CryptGenRandom(hProv, sizeof key, key))
  {
    ret = NLSE_ENC_ERROR_CANT_GENERATE_DATA_KEY;
    goto exit;
  }
#endif /* NLSE_DEBUG_FAKE_FILE_KEY */

  DWORD dwSourceAttributes = GetFileAttributesW(inFileName);
  if(INVALID_FILE_ATTRIBUTES == dwSourceAttributes)
  {
    ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
    goto exit;
  }

  // Encrypt the input file to the output file.
  inFile = CreateFile(inFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                      OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

  if (inFile == INVALID_HANDLE_VALUE)
  {
    ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
    goto exit;
  }

  // Create the output file.
  if (useRawWrite)
  {
    wchar_t outFileName2[MAX_PATH];
    wcsncpy_s(outFileName2, MAX_PATH, outFileName, _TRUNCATE);

    if (!SE_CreateFileRaw(outFileName2, wcslen(outFileName), GENERIC_WRITE,
                          dwSourceAttributes, 0, CREATE_ALWAYS, &outFile))
    {
      goto commError;
    }
  }
  else
  {
    outFile = CreateFile(outFileName, GENERIC_WRITE, 0, NULL,
                         CREATE_ALWAYS,
                         dwSourceAttributes | FILE_FLAG_SEQUENTIAL_SCAN,
                         NULL);
  }

  if (outFile == INVALID_HANDLE_VALUE)
  {
	ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
    goto exit;
  }

  if (!useRawWrite)
  {
    // Skip the NXL file header.
    LARGE_INTEGER offset;

    offset.QuadPart = sizeof(NLSEIndicator_t);
    if (!SetFilePointerEx(outFile, offset, NULL, FILE_BEGIN))
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

    // Keep this process trusted in the loop
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
    // bytesRead = 0.  So here we check for both cases when checking for EOF.
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

#if NLSE_DEBUG_CRYPTO_PASSTHROUGH
  /* Do nothing*/
#else
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
#endif /* NLSE_DEBUG_CRYPTO_PASSTHROUGH */

    if (paddingLen > 0)
    {
      CopyMemory(paddingData, buf + bytesRead, paddingLen);
    }

    // Write one block of data to outFile.
    if (useRawWrite)
    {
      if (!SE_WriteFileRaw(outFile, sizeof NLSEIndicator_t +
                           readRoundCount * sizeof buf, bytesRead, buf,
                           sizeof buf, &bytesWritten))
      {
        goto commError;
      }
    }
    else
    {
      if (!WriteFile(outFile, buf, bytesRead, &bytesWritten, NULL))
      {
        ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
        goto exit;
      }
    }

    if (bytesWritten != bytesRead)
    {
      ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
      goto exit;
    }

    freeCryptoKey(hKey);
    hKey = NULL;
    readRoundCount++;
  }

  
    // Keep this process trusted after the loop
    KeepProcessTrusted();

  if (useRawWrite)
  {
    SE_CloseFileRaw(outFile);
  }
  else
  {
    CloseHandle(outFile);
  }
  outFile = INVALID_HANDLE_VALUE;

  // Write key and padding data into indicator.
  ret = writeNLSEIndicator(hProv, outFileName,
                           fileRealLength.QuadPart, key, paddingLen,
                           paddingData, useLocalKey, FALSE);

exit:  
  if (hKey != NULL)
  {
    freeCryptoKey(hKey);
  }

  if (outFile != INVALID_HANDLE_VALUE)
  {
    if (useRawWrite)
    {
      SE_CloseFileRaw(outFile);
    }
    else
    {
      CloseHandle(outFile);
    }
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
} /* doEncryptFile */

/** doDecryptFile
 *
 * \brief Decrypt a file to a different file name.
 */
static nlse_enc_error_t doDecryptFile(LPCWSTR inFileName,
                                      LPCWSTR outFileName)
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

  ret = isFileNLSEEncrypted(inFileName, &encrypted);

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
  outFile = CreateFile(outFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                       NULL);

  if (outFile == INVALID_HANDLE_VALUE)
  {
    ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
    goto exit;
  }

  // Read key and padding data from indicator.
  BYTE paddingData[NLE_PADDING_DATA_LEN];
  DWORD paddingLen;

  ret = readNLSEIndicator(hProv, inFileName, &fileRealLength, key, &paddingLen,
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
  else if (((ULONGLONG) fileLengthOnDisk.QuadPart !=
            sizeof(NLSEIndicator_t) + fileRealLength) ||
           (fileRealLength + paddingLen !=
            ROUND_UP(fileRealLength, AES_BLOCK_SIZE)))
  {
    ret = NLSE_ENC_ERROR_INDICATOR_CORRUPTED;
    SetLastError(ERROR_FILE_CORRUPT);
    goto exit;
  }

  // Skip the NXL file header.
  LARGE_INTEGER offset;

  offset.QuadPart = sizeof(NLSEIndicator_t);
  if (!SetFilePointerEx(inFile, offset, NULL, FILE_BEGIN))
  {
    ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
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
    // bytesRead = 0.  So here we check for both cases when checking for EOF.
    if (!ReadFile(inFile, buf, sizeof buf, &bytesRead, NULL))
    {
      if (GetLastError() == ERROR_HANDLE_EOF)
      {
        // No more bytes to decrypt.
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
      // No more bytes to decrypt.
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

#if NLSE_DEBUG_CRYPTO_PASSTHROUGH
  /* Do nothing*/
#else
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
#endif /* NLSE_DEBUG_CRYPTO_PASSTHROUGH */

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

  ret = NLSE_ENC_ERROR_SUCCESS;

exit:  
  if (hKey != NULL)
  {
    freeCryptoKey(hKey);
  }

  SecureZeroMemory(key, sizeof key);

  if (outFile != INVALID_HANDLE_VALUE)
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
} /* doDecryptFile */
static BOOL GetOSInfo(DWORD& dwMajor, DWORD& dwMinor)
{
	CELOG_ENTER;
	static DWORD sMajor = 0;
	static DWORD sMinor = 0;

	if(sMajor == 0 && sMinor == 0)
	{
		OSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx;

		// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
		//
		// If that fails, try using the OSVERSIONINFO structure.

		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
		if( !bOsVersionInfoEx )
		{
			// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.

			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
				CELOG_RETURN_VAL(FALSE);
		}

		sMajor = osvi.dwMajorVersion;
		sMinor = osvi.dwMinorVersion;

		//	g_log.Log(CELOG_DEBUG, L"HTTPE::OS version, Major: %d, Minor: %d", sMajor, sMinor);
	}


	//5,0 win2k, 5,1 winxp
	dwMajor = sMajor;
	dwMinor = sMinor;

	CELOG_RETURN_VAL(TRUE);

}

static BOOL IsWin7(void /*include Vista*/)
{
	CELOG_ENTER;
	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	if(GetOSInfo(dwMajor, dwMinor) && dwMajor == 6)
	{
		CELOG_RETURN_VAL(TRUE);
	}
	CELOG_RETURN_VAL(FALSE);
}
/*
*\ brief: add by Tonny, We only do this if os is win7 and the path is created directly under the system driver
*		: eg: create a.txt under c: driver
*/
static BOOL IsSystemRootPathFile(  LPCWSTR outFileName)
{
	CELOG_ENTER;
	BOOL bTrue = FALSE;
	if(!IsWin7())	CELOG_RETURN_VAL(bTrue);

	// get the file name: y
	wchar_t* pPos = (wchar_t*)wcsrchr(outFileName,L'\\');
	if(pPos == NULL)	CELOG_RETURN_VAL(bTrue);
	// get system driver: x
	wchar_t szSystemPath[MAX_PATH]={0};
	if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL,0, szSystemPath)))	CELOG_RETURN_VAL(bTrue);
	szSystemPath[3]=L'\0';
	// merge the file path
	wcsncat_s(szSystemPath,MAX_PATH,pPos+1, _TRUNCATE);
	// judge do create only if the file path is same as : x:\y 
	if(_wcsicmp(szSystemPath,outFileName) == 0)	bTrue = TRUE;
	CELOG_RETURN_VAL(bTrue);
}

static HANDLE CreateFileByTemp(  LPCWSTR outFileName,BOOL useRawWrite )
{
	CELOG_ENTER;
	HANDLE outFile = INVALID_HANDLE_VALUE, htemp =INVALID_HANDLE_VALUE;

	if(!IsSystemRootPathFile(outFileName))	CELOG_RETURN_VAL(outFile);

	WCHAR tmp_path[MAX_PATH] = {0} ;
	
	DWORD dwRet = GetTempPath(_countof(tmp_path), tmp_path);
	CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_EncryptFile: GetTempPath returned \"%s\"\n",
		tmp_path);

	WCHAR tmp_file[MAX_PATH];                 
	if (GetTempFileName(tmp_path, L"SE_", 0, tmp_file) == 0)
	{
		CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFile: cannot create temp file (le %d)\n", 
			dwRet);
		CELOG_RETURN_VAL(outFile);
	}
	if( useRawWrite )
	{
		if (!SE_CreateFileRaw(tmp_file, wcslen(tmp_file), GENERIC_WRITE,
			FILE_ATTRIBUTE_NORMAL, 0, CREATE_ALWAYS, &htemp))
		{

			CELOG_RETURN_VAL(outFile) ;
		}
	}else
	{
		htemp = CreateFile(tmp_file, GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
	}
	if( htemp!=INVALID_HANDLE_VALUE)
	{
		if (useRawWrite)
		{
			SE_CloseFileRaw(htemp) ;
		}else
		{
			CloseHandle(htemp);
		}
		wchar_t outFileName2[MAX_PATH] = {0};
		wcsncpy_s(outFileName2, MAX_PATH, outFileName, _TRUNCATE);
		SHFILEOPSTRUCT FileOp;
		ZeroMemory((void*)&FileOp,sizeof(SHFILEOPSTRUCT));
		FileOp.fFlags = FOF_NOCONFIRMATION ;
		FileOp.hNameMappings = NULL;
		FileOp.hwnd = NULL;
		FileOp.lpszProgressTitle = NULL;
		FileOp.pFrom = tmp_file;
		FileOp.pTo = outFileName2;
		FileOp.wFunc = FO_MOVE;
		if(  SHFileOperation(&FileOp)!= 0)
		{
			CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_EncryptFile: =copy faileu=======%s===%s===%d====",tmp_file,outFileName,GetLastError());
			CELOG_RETURN_VAL(outFile) ;
		}
		else
		{
			CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_EncryptFile: ========%s===%s=======",tmp_file,outFileName);
			if (useRawWrite)
			{
				wchar_t outFileName2_l[MAX_PATH] = {0};
				wcsncpy_s(outFileName2_l, MAX_PATH, outFileName, _TRUNCATE);

				if (!SE_CreateFileRaw(outFileName2_l, wcslen(outFileName2_l), GENERIC_WRITE,
					FILE_ATTRIBUTE_NORMAL, 0, CREATE_ALWAYS, &outFile))
				{
					CELOG_RETURN_VAL(outFile) ;
				}
			}
			else
			{
				outFile = CreateFile(outFileName, GENERIC_WRITE, 0, NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
					NULL);
			}
			if (outFile == INVALID_HANDLE_VALUE)
			{
				CELOG_LOG(CELOG_DEBUG, L"Create file [%s] failure in CreateFileByTemp , so delete the 0 size temp file........\n",outFileName);
				if(!DeleteFile(outFileName))
				{
					CELOG_LOG(CELOG_DEBUG, L"Delete the 0 size temp file [%s] failed in CreateFileByTemp, error is : [%d]........\n",
						outFileName,GetLastError());
				}
				CELOG_RETURN_VAL(outFile) ;
			}
		}
	}
	CELOG_RETURN_VAL(outFile) ;
}

/** doCopyEncryptedFile
 *
 * \brief Copy an encrypted file in an end-to-end-encrypted fashion.  The
 * \brief source file and the destination file can be local-encrypted,
 * \brief wrapped, or mixed.
 */
static nlse_enc_error_t doCopyEncryptedFile(
    LPCWSTR inFileName,
    LPCWSTR outFileName,
    copy_encrypted_file_key_option_t keyOpt)
{
  CELOG_ENUM_ENTER(nlse_enc_error_t);
  BOOL encrypted;
  UCHAR *buf = NULL;
  HANDLE inFile = INVALID_HANDLE_VALUE, outFile = INVALID_HANDLE_VALUE;
  nlse_enc_error_t ret;
  HCRYPTPROV hProv = NULL;

  // For performance reason, we use Raw R/W only for files that are not wrapped.
  const BOOL isDriverRunning = is_NLSE_service_running();
  const BOOL isLocalFixedInFile = is_local_fixed_disk(inFileName);
  const BOOL isLocalFixedOutFile = is_local_fixed_disk(outFileName);
  const BOOL useRawRead = isDriverRunning && isLocalFixedInFile && !SE_IsWrappedFile(inFileName);
  const BOOL useRawWrite = isDriverRunning && isLocalFixedOutFile;

  ret = isFileNLSEEncrypted(inFileName, &encrypted);

  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto exit;
  }

  if (!encrypted)
  {
    ret = NLSE_ENC_ERROR_INPUT_FILE_NOT_ENCRYPTED;
    goto exit;
  }

  //in order to check required key move these ahead
  ULONGLONG fileRealLength;
  BYTE key[NLE_KEY_LENGTH_IN_BYTES];
  BYTE paddingData[NLE_PADDING_DATA_LEN];
  DWORD paddingLen;

  ret = initCryptoContext(&hProv);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto exit;
  }

  ret = readNLSEIndicator(hProv, inFileName, &fileRealLength, key, &paddingLen,
                          paddingData);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto exit;
  }

  // Allocate copy buffer.
  buf = (UCHAR *) malloc(BUF_SIZE);
  if (buf == NULL)
  {
    ret = NLSE_ENC_ERROR_NOT_ENOUGH_MEMORY;
    goto exit;
  }

  // Open input file.
  //
  // When a file is already opened by another application for write access,
  // allowing the copying to proceed can end up with an inconsistent
  // destination file if the app is writing to the source file at the same
  // time.  Hence the NLSE code originally allow only share-read.  However,
  // since Microsoft allows Explorer to perform such copying in this situation
  // when no NLSE is installed, we will change NLSE code to allow
  // share-read-write for encrypted file instead.  This will keep our product
  // behavior consistent with Microsoft.
  if (useRawRead)
  {
    wchar_t inFileName2[MAX_PATH] = {0};
    wcsncpy_s(inFileName2, MAX_PATH, inFileName, _TRUNCATE);

    if (!SE_CreateFileRaw(inFileName2, wcslen(inFileName2), GENERIC_READ,
                          0, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING,
                          &inFile))
    {
      goto commError;
    }
  }
  else
  {
    inFile = CreateFile(inFileName, GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);
  }

  if (inFile == INVALID_HANDLE_VALUE)
  {
    ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
    goto exit;
  }

  //If the out folder is not existing, create it. Otherwise the encrypted file will be copied as unencrypted
  {
	  std::wstring strOrigOutPath=outFileName;
	  if(strOrigOutPath[strOrigOutPath.length()-1]==L'\\'||strOrigOutPath[strOrigOutPath.length()-1]==L'/')
		SHCreateDirectoryExW(NULL, strOrigOutPath.c_str(), NULL);
	  else
	  {
		  std::wstring::size_type pos=strOrigOutPath.rfind(L"\\");
		  if(pos!=std::wstring::npos)
		  {
			  strOrigOutPath=strOrigOutPath.substr(0,pos);
			  SHCreateDirectoryExW(NULL, strOrigOutPath.c_str(), NULL);
		  }
	  }
  }
  // Open output file.
  if (useRawWrite)
  {
    wchar_t outFileName2[MAX_PATH] = {0};
    wcsncpy_s(outFileName2, MAX_PATH, outFileName, _TRUNCATE);

    if (!SE_CreateFileRaw(outFileName2, wcslen(outFileName2), GENERIC_WRITE,
                          FILE_ATTRIBUTE_NORMAL, 0, CREATE_ALWAYS, &outFile))
    {
      goto commError;
    }
  }
  else
  {
    outFile = CreateFile(outFileName, GENERIC_WRITE, 0, NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                         NULL);
  }

  if (outFile == INVALID_HANDLE_VALUE)
  {
	outFile = CreateFileByTemp(outFileName,useRawWrite ) ;
	 if (outFile == INVALID_HANDLE_VALUE)
	 {
		  SE_SetLastError(NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE);
		  ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
		  goto exit;
	 }
  }

  // Loop through each block of file content to encrypt.
  ULONGLONG curOffset = 0;

  while (1)
  {
    DWORD bytesRead, bytesWritten;

    // Read one block of data from inFile.
    if (useRawRead)
    {
      if (!SE_ReadFileRaw(inFile, curOffset, BUF_SIZE, buf, BUF_SIZE,
                          &bytesRead))
      {
        goto commError;
      }
      else if (bytesRead == 0)
      {
        if (GetLastError() == ERROR_HANDLE_EOF)
        {
          // No more bytes to copy.
          break;
        }
        else
        {
          ret = NLSE_ENC_ERROR_CANT_READ_INPUT_FILE;
          goto exit;
        }
      }
    }
    else
    {
      // About checking for EOF:
      // MSDN doc says that "If the ReadFile function attempts to read past the
      // end of the file, the function returns zero, and GetLastError returns
      // ERROR_HANDLE_EOF."  However, my observations on WinXP Pro SP3, Win2003
      // R2 SE SP2 and Win2008 EE SP1 is that ReadFile() returns non-zero and
      // bytesRead = 0.  So here we check for both cases when checking for EOF.
      if (!ReadFile(inFile, buf, BUF_SIZE, &bytesRead, NULL))
      {
        if (GetLastError() == ERROR_HANDLE_EOF)
        {
          // No more bytes to copy.
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
        // No more bytes to copy.
        break;
      }

    }

    // Write one block of data to outFile.
    if (useRawWrite)
    {
      if (!SE_WriteFileRaw(outFile, curOffset, bytesRead, buf, BUF_SIZE,
                           &bytesWritten))
      {
        goto commError;
      }
    }
    else
    {
      if (!WriteFile(outFile, buf, bytesRead, &bytesWritten, NULL))
      {
		SE_SetLastError(NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE);
        ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
        goto exit;
      }
    }

    if (bytesWritten != bytesRead)
    {
	  SE_SetLastError(NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE);
      ret = NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE;
	  goto exit;
	}

	curOffset += bytesRead ;//BUF_SIZE;
	if (curOffset >= fileRealLength + sizeof(NLSEIndicator_t))
	{
		break ;
	}
  }

  // Close input file.
  if (useRawRead)
  {
    SE_CloseFileRaw(inFile);
  }
  else
  {
    CloseHandle(inFile);
  }
  inFile = INVALID_HANDLE_VALUE;

  // Close output file.
  if (useRawWrite)
  {
    SE_CloseFileRaw(outFile);
  }
  else
  {
    CloseHandle(outFile);
  }

  outFile = INVALID_HANDLE_VALUE;
  // Switch key if necessary.
  if (keyOpt == CEFKO_KEEP)
  {
    ret = NLSE_ENC_ERROR_SUCCESS;
    goto exit;
  }

  ret = writeNLSEIndicator(hProv, outFileName,
                           fileRealLength, key, paddingLen,
                           paddingData, keyOpt == CEFKO_SWITCH_TO_LOCAL, TRUE);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    goto exit;
  }

  ret = NLSE_ENC_ERROR_SUCCESS;
exit:
  SecureZeroMemory(key, sizeof key);

  if (hProv != NULL)
  {
    freeCryptoContext(hProv);
  }

  if (outFile != INVALID_HANDLE_VALUE)
  {
    if (useRawWrite)
    {
      SE_CloseFileRaw(outFile);
    }
    else
    {
      CloseHandle(outFile);
    }
  }

  if (inFile != INVALID_HANDLE_VALUE)
  {
    if (useRawRead)
    {
      SE_CloseFileRaw(inFile);
    }
    else
    {
      CloseHandle(inFile);
    }
  }

  if (buf != NULL)
  {
    free(buf);
  }

  if (ret == NLSE_ENC_ERROR_SUCCESS)
  {
	  // Copy the NTFS Alternate Data Streams from source to destination
	  NxtCopyNtfsDataStreams(inFileName, outFileName);
  }
  CELOG_ENUM_RETURN_VAL(ret);

commError:
  ret = NLSE_ENC_ERROR_PLUGIN_COMM;
  goto exit;
} /* doCopyEncryptedFile */

BOOL Real_EncryptFile( _In_z_ const wchar_t* in_path, _In_ bool Force = false )
{
	CELOG_ENTER;
	ensureInited();

	assert( in_path != NULL );
	if( in_path == NULL )
	{
		CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFile: in_path is NULL\n");
		SetLastError(ERROR_INVALID_PARAMETER);
		CELOG_RETURN_VAL(FALSE);
	}

	/* Can encrypt this file? */
	if (Force)
	{
		if( can_encryptForce(in_path) == false )
		{
			CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFile: can_encrypt(%s) returned false\n",
				in_path);
			SetLastError(ERROR_NOT_SUPPORTED);
			CELOG_RETURN_VAL(FALSE);
		}
	}
	else
	{
		if( can_encrypt(in_path) == false )
		{
			CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFile: can_encrypt(%s) returned false\n",
				in_path);
			SetLastError(ERROR_NOT_SUPPORTED);
			CELOG_RETURN_VAL(FALSE);
		}
	}

	WCHAR parent_dir[MAX_PATH];
	PWCHAR pLastBS;

	wcsncpy_s(parent_dir, MAX_PATH, in_path, _TRUNCATE);
	pLastBS = wcsrchr(parent_dir, L'\\');
	if (pLastBS == NULL)
	{
		CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFile: can't find parent dir in %s\n",
			in_path);
		SetLastError(ERROR_BAD_PATHNAME);
		CELOG_RETURN_VAL(FALSE);
	}
	*pLastBS = L'\0';

	// Determine the directory for creating the temp file.
	//
	// We use the system temp directory for our temp file, regardless of whether
	// it is on the same volume or different volume as the original file.  The
	// system temp directory is guaranteed by is_protected_path() to be non-DRM.
	WCHAR tmp_path[MAX_PATH] = L"";
	DWORD dwRet;

	dwRet = GetTempPath(_countof(tmp_path), tmp_path);
	if (dwRet == 0)
	{
		DWORD lastError = GetLastError();

		CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFile: GetTempPath failed (le %d)\n",
			lastError);
		SetLastError(lastError);
		CELOG_RETURN_VAL(FALSE);
	}

	CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_EncryptFile: GetTempPath returned \"%s\"\n",
		tmp_path);

	WCHAR tmp_file[MAX_PATH];                 
	if (GetTempFileName(tmp_path, L"SE_", 0, tmp_file) == 0)
	{
		DWORD lastError = GetLastError();

		CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFile: cannot create temp file (le %d)\n", 
			lastError);
		SetLastError(lastError);
		CELOG_RETURN_VAL(FALSE);
	}

	CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_EncryptFile: About to call doEncryptFile().  in_path=\"%s\", tmp_file=\"%s\"\n",
		in_path, tmp_file);

	nlse_enc_error_t ret;

	//
	//
	// Fix bug 16769:
	//
	// To encrypt a file (c:\test\foo.txt), the original design is:
	//    a. Use raw create file to generate a tmp file (c:\users\...\temp\SE_XXX.tmp)
	//    b. Use raw API to write encrypted data into that file
	//    c. Replace original file using that tmp file
	//
	// The problem happens in step #a -- the tmp files doesn't always contains the
	// same security attributes as the original file. So when another user log on, the
	// file cannot be opened correctly.
	//
	// To fix this problem, we need to change original design. The new design is:
	//    a. Backup original file to temp directory (c:\users\...\temp\SEXXX.tmp)
	//    b. Encrypt original file
	//    c. If there is any error in step #b, recover original file from the
	//       backup file. Otherwsie, delete the backup file.
	//
	//  In this new design, the function doesn't delete original file, so it will keep its
	//  original security attributes.

	DWORD dwLastError = 0;
	BOOL  bRet        = FALSE;

	// Step a. Backup original file to tmp_file
	if(!::CopyFileW(in_path, tmp_file, FALSE))
	{
		dwLastError = GetLastError();
		goto _exit;
	}

	// Step b. Encrypt original file
	ret = doEncryptFile(tmp_file, in_path, TRUE);
	if (ret != NLSE_ENC_ERROR_SUCCESS)
	{
		dwLastError = GetLastError();
		CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFile: doEncryptFile(%s, %s, TRUE) returned %d\n", tmp_file, in_path, ret);
		// Step c. Recover the original file if any error happens
		if (!::CopyFileW(tmp_file, in_path, FALSE))
		{
			CELOG_LOG(CELOG_ERROR, L"NLSE!SE_EncryptFile: doEncryptFile failed. Also fail to recover original file (%d)\n", GetLastError());
		}
		dwLastError = ERROR_ACCESS_DENIED;
		goto _exit;
	}

	// Succeed
	bRet = TRUE;

_exit:
	// Remove all the temp file generated by us
	DeleteFile(tmp_file);
	SetLastError(dwLastError);
	CELOG_RETURN_VAL(bRet);
}

BOOL SE_EncryptFileForce( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls \n", in_path);
  CELOG_RETURN_VAL(Real_EncryptFile(in_path, true));
}

BOOL SE_EncryptFile( _In_z_ const wchar_t* in_path )
{
  CELOG_ENTER;
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls \n", in_path);
  CELOG_RETURN_VAL(Real_EncryptFile(in_path));
}/* SE_EncryptFile */

BOOL SE_DecryptFile( _In_z_ const wchar_t* in_path,
                     _In_opt_z_ const wchar_t* out_path,
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
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFile: in_path is NULL\n");
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  if (is_NLSE_service_running())
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFile: NLSE service is still running\n");
    SetLastError(ERROR_ACCESS_DENIED);
    CELOG_RETURN_VAL(FALSE);
  }

  ret = checkPassword(password);
  if (ret == NLSE_ENC_ERROR_INVALID_PASSWORD)
  {
    SetLastError(ERROR_INVALID_PASSWORD);
    CELOG_RETURN_VAL(FALSE);
  }
  else if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFile: checkPassword() returned %d\n",
                 ret);
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
      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFile: can't find parent dir in %s\n",
                   in_path);
      SetLastError(ERROR_BAD_PATHNAME);
      CELOG_RETURN_VAL(FALSE);
    }
    *pLastBS = L'\0';

    if (GetTempFileName(parent_dir, L"SE_", 0, tmp_file) == 0)
    {
      DWORD lastError = GetLastError();

      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFile: cannot create temp file (le %d)\n", 
                   lastError);
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

      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFile: cannot create bak file (le %d)\n", 
                   lastError);
      DeleteFile(tmp_file);
      SetLastError(lastError);
      CELOG_RETURN_VAL(FALSE);
    }

    CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_DecryptFile: About to call doDecryptFile().  in_path=\"%s\", tmp_file=\"%s\", bak_file=\"%s\"\n",
                 in_path, tmp_file, bak_file);

    ret = doDecryptFile(in_path, tmp_file);
    if (ret != NLSE_ENC_ERROR_SUCCESS)
    {
      DWORD lastError = GetLastError();

      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFile: doDecryptFile(%s, %s) returned %d\n",
                   in_path, tmp_file, ret);
      DeleteFile(bak_file);
      DeleteFile(tmp_file);
      SetLastError(lastError);
      CELOG_RETURN_VAL(FALSE);
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

        CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFile: Cannot delete %s (le %d)\n",
                     bak_file, lastError);
        SetLastError(lastError);
        CELOG_RETURN_VAL(FALSE);
      }
      else
      {
        CELOG_RETURN_VAL(TRUE);
      }
    }
    else
    {
      // ReplaceFile() failed.
      DWORD lastError = GetLastError();

      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFile: Cannot replace %s with %s (le %d)\n",
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
    CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_DecryptFile: About to call doDecryptFile().  in_path=\"%s\", out_path=\"%s\"\n",
                 in_path, out_path);

    ret = doDecryptFile(in_path, out_path);
    if (ret != NLSE_ENC_ERROR_SUCCESS)
    {
      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_DecryptFile: doDecryptFile(%s, %s) returned %d\n",
                   in_path, out_path, ret);
    }
    CELOG_LOG(CELOG_DUMP, L"Local variables are: useTempFile=%ls \n", useTempFile?L"TRUE":L"FALSE" );
    CELOG_RETURN_VAL(ret == NLSE_ENC_ERROR_SUCCESS);
  }
}/* SE_DecryptFile */

/** SE_CreateFileRaw
 *
 *  \brief create or open a file in raw mode
 *
 *  \param fName The fully-qualifed file specification of the file to be created or opened.
 *  \param nameLen Number of wide characters in fName, excluding null terminator.
 *  \param desiredAccess The requested access to the file.  (See CreateFile.)
 *  \param fileAttributes One or more of the FILE_ATTRIBUTE_XXX flags.  (See CreateFile.)
 *  \param shareAccess The requested sharing mode of the file.  (See CreateFile.)
 *  \param createDisposition An action to take on a file that exists or does not exist.  (See CreateFile.)
 *  \param pHandle A pointer to a caller-allocated variable that receives the file handle if the call is successful.
 *
 *  \return TRUE on success.
 *  \return     *pHandle = file handle on create success.
 *  \return                INVALID_HANDLE_VALUE on create error.  To get
 *  \return                extended error information, call GetLastError().
 *  \return FALSE on communication error.
 */
_Check_return_
BOOL SE_CreateFileRaw(
		      _In_z_count_(nameLen)    WCHAR *fName,
		      _In_                    size_t nameLen,
		      _In_                    ACCESS_MASK desiredAccess,
		      _In_                    ULONG fileAttributes,
		      _In_                    ULONG shareAccess,
		      _In_                    ULONG createDisposition,
		      _Deref_out_             PHANDLE pHandle)
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: fName=%ls, nameLen=%u, fileAttributes=%lu, shareAccess=%lu, createDisposition=%lu \n", fName,nameLen,fileAttributes, shareAccess,createDisposition);
  NLSE_USER_COMMAND cmd;
  NTSTATUS status;

  init_cmd(&cmd);

  cmd.type=NLSE_USER_COMMAND_CREATE_FILE_RAW;
  wcsncpy_s(cmd.msg.fname, NLSE_MAX_PATH, fName, _TRUNCATE);
  cmd.msg.fname[NLSE_MAX_PATH-1]=L'\0';
  cmd.msg.params.createFileRaw.desiredAccess=desiredAccess;
  cmd.msg.params.createFileRaw.fileAttributes=fileAttributes;
  cmd.msg.params.createFileRaw.shareAccess=shareAccess;

  switch (createDisposition)
  {
  case CREATE_NEW:
    cmd.msg.params.createFileRaw.createDisposition=FILE_CREATE;
    break;
  case CREATE_ALWAYS:
    cmd.msg.params.createFileRaw.createDisposition=FILE_OVERWRITE_IF;
    break;
  case OPEN_EXISTING:
    cmd.msg.params.createFileRaw.createDisposition=FILE_OPEN;
    break;
  case OPEN_ALWAYS:
    cmd.msg.params.createFileRaw.createDisposition=FILE_OPEN_IF;
    break;
  case TRUNCATE_EXISTING:
    CELOG_LOG(CELOG_ERROR,L"NLSELib!NLSEUserCmd_CreateFileRaw: TRUNCATE_EXISTING is not supported\n");
    SetLastError(ERROR_NOT_SUPPORTED);
    CELOG_RETURN_VAL(TRUE);
  default:
    // Unknown createDisposition.
    CELOG_LOG(CELOG_ERROR,L"NLSELib!NLSEUserCmd_CreateFileRaw: createDisposition is invalid\n");
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(TRUE);
  }

  cmd.msg.params.createFileRaw.pHandle=pHandle;
  cmd.msg.params.createFileRaw.status = &status;

  if(WriteCommand(&cmd) == 0) {
    //write command failed
    CELOG_RETURN_VAL(FALSE);
  }

  DWORD lastErr = LsaNtStatusToWinError(status);
  if (lastErr != ERROR_SUCCESS)
  {
    *pHandle = INVALID_HANDLE_VALUE;
  }
  CELOG_LOG(CELOG_DUMP, L"Local variables are: lastErr=%lu \n", lastErr );
  SetLastError(lastErr);
  CELOG_RETURN_VAL(TRUE);
} /* SE_CreateFileRaw */

/** SE_ReadFileRaw
 *
 *  \brief read a file in raw mode
 *
 *  \param handle A handle to the file.
 *  \param offset The byte offset within the file where the read operation is to begin.
 *  \param len Size, in bytes, to read.
 *  \param buf Pointer to a caller-allocated buffer that receives the data that is read from the file.
 *  \param bufSize Size, in bytes, of buffer.
 *  \param bytesRead Pointer to a caller-allocated variable that receives the number of bytes read from the fiel.
 *
 *  \return TRUE on communication success.
 *  \return     *bytesRead = non-zero on read success.
 *  \return                  zero on read error.  To get extended error
 *  \return                  information, call GetLastError().
 *  \return FALSE on communication error.
 */
_Check_return_
BOOL SE_ReadFileRaw(
		    _In_                                    HANDLE handle,
		    _In_                                    ULONGLONG offset,
		    _In_                                    ULONG len,
		    _Out_bytecap_post_bytecount_(bufSize, *bytesRead)  PVOID buf,
		    _In_                                    ULONG bufSize,
		    _Out_                                   PULONG bytesRead)
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: handle=%p, len=%lu, bufSize=%lu \n",handle, len, bufSize);
  
  NLSE_USER_COMMAND cmd;
  NTSTATUS status;

  init_cmd(&cmd);

  cmd.type=NLSE_USER_COMMAND_READ_FILE_RAW;
  cmd.msg.params.readFileRaw.handle=handle;
  cmd.msg.params.readFileRaw.offset=offset;
  cmd.msg.params.readFileRaw.len=len;
  cmd.msg.params.readFileRaw.buf=buf;
  cmd.msg.params.readFileRaw.bufSize=bufSize;
  cmd.msg.params.readFileRaw.bytesRead=bytesRead;
  cmd.msg.params.readFileRaw.status=&status;

  if(WriteCommand(&cmd) == 0) {
    //write command failed
    CELOG_RETURN_VAL(FALSE);
  }

  SetLastError(LsaNtStatusToWinError(status));
  CELOG_RETURN_VAL(TRUE);
} /* SE_ReadFileRaw */

/** SE_WriteFileRaw
 *
 *  \brief write to a file in raw mode
 *
 *  \param handle A handle to the file.
 *  \param offset The byte offset within the file where the write operation is to begin.
 *  \param len Size, in bytes, to write.
 *  \param buf Pointer to a buffer that contains the data to be written to the file.
 *  \param bufSize Size, in bytes, of buffer.
 *  \param bytesWritten Pointer to a caller-allocated variable that receives the number of bytes written to the file.
 *
 *  \return TRUE on communication success.
 *  \return     *bytesWritten = len on write success.
 *  \return                     other value on write error.  To get extended
 *  \return                     error information, call GetLastError().
 *  \return FALSE on communication error.
 */
_Check_return_
BOOL SE_WriteFileRaw(
            _In_                HANDLE handle,
            _In_                ULONGLONG offset,
            _In_                ULONG len,
            _In_bytecount_(len)    PVOID buf,
            _In_                ULONG bufSize,
            _Out_               PULONG bytesWritten)
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: handle=%p, len=%lu, bufSize=%lu \n",handle, len, bufSize);

  NLSE_USER_COMMAND cmd;
  NTSTATUS status;

  init_cmd(&cmd);

  cmd.type=NLSE_USER_COMMAND_WRITE_FILE_RAW;
  cmd.msg.params.writeFileRaw.handle=handle;
  cmd.msg.params.writeFileRaw.offset=offset;
  cmd.msg.params.writeFileRaw.len=len;
  cmd.msg.params.writeFileRaw.buf=buf;
  cmd.msg.params.writeFileRaw.bufSize=bufSize;
  cmd.msg.params.writeFileRaw.bytesWritten=bytesWritten;
  cmd.msg.params.writeFileRaw.status=&status;

  if(WriteCommand(&cmd) == 0) {
    //write command failed
    CELOG_RETURN_VAL(FALSE);
  }

  SetLastError(LsaNtStatusToWinError(status));
  CELOG_RETURN_VAL(TRUE);
} /* SE_WriteFileRaw */

/** SE_CloseFileRaw
 *
 *  \brief close a file opened in raw mode
 *
 *  \param handle A handle to the file.
 *
 *  \return TRUE on communication success.
 *  \return     To get extended error information, call GetLastError().
 *  \return FALSE on communication error.
 */
BOOL SE_CloseFileRaw(
		     _In_    HANDLE handle)
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: handle=%p \n", handle);
	
  NLSE_USER_COMMAND cmd;
  NTSTATUS status;

  init_cmd(&cmd);

  cmd.type=NLSE_USER_COMMAND_CLOSE_FILE_RAW;
  cmd.msg.params.closeFileRaw.handle=handle;
  cmd.msg.params.closeFileRaw.status=&status;

  if(WriteCommand(&cmd) == 0) {
    //write command failed
    CELOG_RETURN_VAL(FALSE);
  }

  SetLastError(LsaNtStatusToWinError(status));
  CELOG_RETURN_VAL(TRUE);
} /* SE_CloseFileRaw */

_Check_return_
BOOL SE_GetFileInfo(
    _In_z_                              const wchar_t* in_file,
    _In_                                NextLabs_FileInfo_t in_file_info ,
    _Out_bytecapcount_x_(in_file_info == SE_FileInfo_NextLabs ?
                      sizeof(NextLabsFile_Header_t) :
                      (in_file_info == SE_FileInfo_NextLabsEncryption ?
                       sizeof(NextLabsEncryptionFile_Header_t) :
                       sizeof(NextLabsTaggingFile_Header_t)))
                                        void* info)
{
  CELOG_ENTER;
  HANDLE    hFile   = INVALID_HANDLE_VALUE;
  BOOL      ret     = FALSE;
  BOOL      is_driver_running;
  BOOL      is_local_file;
  BOOL      useRawRead;

  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_file=%ls \n", in_file);
	
  assert( in_file != NULL );
  assert( info != NULL );

  //
  //  Sanity check
  //
  if( in_file == NULL || info == NULL )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  if( in_file_info != SE_FileInfo_NextLabs &&
      in_file_info != SE_FileInfo_NextLabsEncryption &&
      in_file_info != SE_FileInfo_NextLabsTagging )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  //
  //  Get environment state
  //
  is_driver_running = is_NLSE_service_running();
  is_local_file     = is_local_disk(in_file);
  useRawRead        = (is_driver_running && is_local_file) ? TRUE : FALSE;

  if (useRawRead)
  {
    wchar_t indicatorName[MAX_PATH] = {0};
    wcsncpy_s(indicatorName,MAX_PATH, in_file, _TRUNCATE);

    if (!SE_CreateFileRaw(indicatorName, wcslen(indicatorName), GENERIC_READ,
                          0, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING,
                          &hFile))
    {
      goto commError;
    }

    // If the volume is not attached by the filter driver, switch back to Win32
    // File API.
    // (In WinXP and Win2003, LsaNtStatusToWinError() translates
    // STATUS_FLT_INSTANCE_NOT_FOUND to ERROR_MR_MID_NOT_FOUND.  So we have to
    // check for that also.)
    if (hFile == INVALID_HANDLE_VALUE &&
        (GetLastError() == ERROR_FLT_INSTANCE_NOT_FOUND ||
         GetLastError() == ERROR_MR_MID_NOT_FOUND))
    {
      useRawRead = FALSE;
    }
  }

  if (!useRawRead)
  {
    hFile = CreateFile(in_file, GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                       OPEN_EXISTING, 0, NULL);
  }

  if (hFile == INVALID_HANDLE_VALUE)
  {
    goto exit;
  }

  ULONGLONG offset_to_read = 0;
  DWORD bytes_to_read = 0;
  DWORD bytes_read;

  /* Set address of information needed for NL and SE */
  switch( in_file_info )
  {
  case SE_FileInfo_NextLabs:
    offset_to_read = offsetof(NLSEIndicator_t, fileType);
    bytes_to_read = sizeof(NextLabsFile_Header_t);
    break;
  case SE_FileInfo_NextLabsEncryption:
    offset_to_read = offsetof(NLSEIndicator_t, encExt);
    bytes_to_read = sizeof(NextLabsEncryptionFile_Header_t);
    break;
  case SE_FileInfo_NextLabsTagging:
    offset_to_read = offsetof(NLSEIndicator_t, tagExt);
    bytes_to_read = sizeof(NextLabsTaggingFile_Header_t);
    break;
  }

  /* Read the information from the file */
  if (useRawRead)
  {
    if (!SE_ReadFileRaw(hFile, offset_to_read, bytes_to_read, info,
                        bytes_to_read, &bytes_read))
    {
      goto commError;
    }
  }
  else
  {
    LARGE_INTEGER offset;

    offset.QuadPart = offset_to_read;
    if (!SetFilePointerEx(hFile, offset, NULL, FILE_BEGIN))
    {
      goto exit;
    }
    if (!ReadFile(hFile, info, bytes_to_read, &bytes_read, NULL))
    {
      goto exit;
    }
  }

  if (bytes_read != bytes_to_read)
  {
    goto exit;
  }

  ret = TRUE;

exit:  
  CELOG_LOG(CELOG_DUMP, L"Local variables are: hFile=%p, ret=%ls, is_driver_running=%ls, is_local_file=%ls, useRawRead=%ls \n", hFile,ret?L"TRUE":L"FALSE",is_driver_running?L"TRUE":L"FALSE",is_local_file?L"TRUE":L"FALSE",useRawRead?L"TRUE":L"FALSE" );
  if (hFile != INVALID_HANDLE_VALUE)
  {
    if (useRawRead)
    {
      SE_CloseFileRaw(hFile);
    }
    else
    {
      CloseHandle(hFile);
    }
  }

  CELOG_RETURN_VAL(ret);

commError:
  goto exit;
}/* SE_GetFileInfo */

BOOL SE_SetFileInfo(
    _In_z_                              const wchar_t* in_file,
    _In_                                NextLabs_FileInfo_t in_file_info ,
    _In_bytecount_x_(in_file_info == SE_FileInfo_NextLabs ?
                sizeof(NextLabsFile_Header_t) :
                (in_file_info == SE_FileInfo_NextLabsEncryption ?
                 sizeof(NextLabsEncryptionFile_Header_t) :
                 sizeof(NextLabsTaggingFile_Header_t)))
                                        const void* info)
{
  CELOG_ENTER;
  
  HANDLE    hFile   = INVALID_HANDLE_VALUE;
  BOOL      ret     = FALSE;
  BOOL      is_driver_running;
  BOOL      is_local_file;
  BOOL      useRawWrite;

  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_file=%ls \n", in_file);
  
  assert( in_file != NULL );
  assert( info != NULL );

  //
  //  Sanity check
  //
  if( in_file == NULL || info == NULL )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  if( in_file_info != SE_FileInfo_NextLabs &&
      in_file_info != SE_FileInfo_NextLabsEncryption &&
      in_file_info != SE_FileInfo_NextLabsTagging )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  //
  //  Get environment state
  //
  is_driver_running = is_NLSE_service_running();
  is_local_file     = is_local_disk(in_file);
  useRawWrite       = (is_driver_running && is_local_file) ? TRUE : FALSE;

  if (useRawWrite)
  {
    wchar_t indicatorName[MAX_PATH] = {0};
    wcsncpy_s(indicatorName,MAX_PATH, in_file, _TRUNCATE);

    if (!SE_CreateFileRaw(indicatorName, wcslen(indicatorName), GENERIC_WRITE,
                          FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, OPEN_ALWAYS,
                          &hFile))
    {
      goto commError;
    }

    // If the volume is not attached by the filter driver, switch back to Win32
    // File API.
    // (In WinXP and Win2003, LsaNtStatusToWinError() translates
    // STATUS_FLT_INSTANCE_NOT_FOUND to ERROR_MR_MID_NOT_FOUND.  So we have to
    // check for that also.)
    if (hFile == INVALID_HANDLE_VALUE &&
        (GetLastError() == ERROR_FLT_INSTANCE_NOT_FOUND ||
         GetLastError() == ERROR_MR_MID_NOT_FOUND))
    {
      useRawWrite = FALSE;
    }
  }

  if (!useRawWrite)
  {
    hFile = CreateFile(in_file, GENERIC_WRITE,
                       FILE_SHARE_READ, NULL,
                       OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  }

  if (hFile == INVALID_HANDLE_VALUE)
  {
    goto exit;
  }

  ULONGLONG offset_to_write = 0;
  DWORD bytes_to_write = 0;
  DWORD bytes_written = 0;

  /* Non-const buffers for passing to SE_WriteFileRaw(). */
  NextLabsFile_Header_t nl;
  NextLabsEncryptionFile_Header_t nle;
  NextLabsTaggingFile_Header_t nlt;
  void *buf = NULL;

  /* Set address of information needed for NL and SE */
  switch( in_file_info )
  {
  case SE_FileInfo_NextLabs:
    offset_to_write = offsetof(NLSEIndicator_t, fileType);
    bytes_to_write = sizeof(NextLabsFile_Header_t);
    nl = * (const NextLabsFile_Header_t *) info;
    buf = &nl;
    break;
  case SE_FileInfo_NextLabsEncryption:
    offset_to_write = offsetof(NLSEIndicator_t, encExt);
    bytes_to_write = sizeof(NextLabsEncryptionFile_Header_t);
    nle = * (const NextLabsEncryptionFile_Header_t *) info;
    buf = &nle;
    break;
  case SE_FileInfo_NextLabsTagging:
    offset_to_write = offsetof(NLSEIndicator_t, tagExt);
    bytes_to_write = sizeof(NextLabsTaggingFile_Header_t);
    nlt = * (const NextLabsTaggingFile_Header_t *) info;
    buf = &nlt;
    break;
  }

  /* Write the information to the file */
  if (useRawWrite)
  {
    if (!SE_WriteFileRaw(hFile, offset_to_write, bytes_to_write, buf,
                         bytes_to_write, &bytes_written))
    {
      goto commError;
    }
  }
  else
  {
    LARGE_INTEGER offset;

    offset.QuadPart = offset_to_write;
    if (!SetFilePointerEx(hFile, offset, NULL, FILE_BEGIN))
    {
      goto exit;
    }
    if (!WriteFile(hFile, info, bytes_to_write, &bytes_written, NULL))
    {
      goto exit;
    }
  }

  if (bytes_written != bytes_to_write)
  {
    goto exit;
  }

  ret = TRUE;

exit:
  CELOG_LOG(CELOG_DUMP, L"Local variables are: hFile=%p, ret=%ls, is_driver_running=%ls, is_local_file=%ls, useRawWrite=%ls \n", hFile,ret?L"TRUE":L"FALSE",is_driver_running?L"TRUE":L"FALSE",is_local_file?L"TRUE":L"FALSE",useRawWrite?L"TRUE":L"FALSE" );

  if (hFile != INVALID_HANDLE_VALUE)
  {
    if (useRawWrite)
    {
      SE_CloseFileRaw(hFile);
    }
    else
    {
      CloseHandle(hFile);
    }
  }

  CELOG_RETURN_VAL(ret);

commError:
  goto exit;
}/* SE_SetFileInfo */

BOOL SE_WrapPlainFile( _In_z_ const wchar_t* in_path,
                       _In_z_ const wchar_t* out_path)
{
  CELOG_ENTER;
  DWORD dwAttrs = INVALID_FILE_ATTRIBUTES;
  ensureInited();
  
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls, out_path=%ls \n", in_path,out_path);
	
  assert( in_path != NULL );
  assert( out_path != NULL );
  if( in_path == NULL || out_path == NULL)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_WrapPlainFile: in_path or out_path is NULL\n");
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }
  else if( _wcsicmp(out_path + wcslen(out_path) - wcslen(NL_FILE_EXT),
                    NL_FILE_EXT) != 0 )
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_WrapPlainFile: %s does not have %s extension\n",
                 out_path, NL_FILE_EXT);
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  nlse_enc_error_t ret;

  // Get original file attributes
  dwAttrs = GetFileAttributes(in_path);

  ret = doEncryptFile(in_path, out_path, FALSE);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_WrapPlainFile: doEncryptFile(%s, %s, FALSE) returned %d\n",
                 in_path, out_path, ret);
    SetLastError(ERROR_ACCESS_DENIED);
	if(ret == NLSE_ENC_ERROR_CANT_GET_PC_KEY)
	{
		CELOG_LOG(CELOG_ERROR,L"Error: Shared key is not available.\n");
	}
	CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
    CELOG_RETURN_VAL(FALSE);
  }

  if (!SE_SetWrappedFileFlags(out_path))
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_WrapPlainFile: fail to set flags in NXL file(%s)\n", out_path);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
    CELOG_RETURN_VAL(FALSE);
  }

  // The file is wrapped successfully, we need to save its original attributes in wrapped NXL file
  if(!SE_SetNxlFileAttributes(out_path, dwAttrs))
  {
    // We just record that set file attributes failed
    // The wrap action still succeed
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_WrapPlainFile: fail to save file attributes in NXL file(%s)\n", out_path);
    SetLastError(0);
  }
  CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
  CELOG_RETURN_VAL(TRUE);
} /* SE_WrapPlainFile */

BOOL SE_UnwrapToPlainFile( _In_z_ const wchar_t* in_path,
                           _In_z_ const wchar_t* out_path)
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls, out_path=%ls \n", in_path,out_path);
	
  assert( in_path != NULL );
  assert( out_path != NULL );
  if( in_path == NULL || out_path == NULL)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToPlainFile: in_path or out_path is NULL\n");
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }
  else if( !SE_IsWrappedFile(in_path) )
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToPlainFile: %s is not a wrapped file\n",
                 in_path);
    SetLastError(ERROR_BAD_FILE_TYPE);
    CELOG_RETURN_VAL(FALSE);
  }
  else if( _wcsicmp(in_path + wcslen(in_path) - wcslen(NL_FILE_EXT),
                    NL_FILE_EXT) != 0 )
  {
    CELOG_LOG(CELOG_WARNING, L"NLSE!SE_UnwrapToPlainFile: %s does not have %s extension\n",
                 in_path, NL_FILE_EXT);
  }

  // Get its original file attributes from NXL file
  DWORD dwAttrs = INVALID_FILE_ATTRIBUTES;
  if(!SE_GetNxlFileAttributes(in_path, &dwAttrs) || INVALID_FILE_ATTRIBUTES==dwAttrs)
  {
    // We just record that set file attributes failed
    // The wrap action still succeed
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToPlainFile: fail to get file attributes from NXL file(%s)\n", in_path);
  }

  // See if NLEF_REQUIRES_LOCAL_ENCRYPTION flag is set.
  NLFSE_ENCRYPT_EXTENSION encExt;
  if (!SE_GetFileInfo(in_path, SE_FileInfo_NextLabsEncryption, &encExt))
  {
    CELOG_RETURN_VAL(FALSE);
  }
  else if (encExt.flags & NLEF_REQUIRES_LOCAL_ENCRYPTION)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToPlainFile: %s requires local encryption\n", in_path);
    SetLastError(ERROR_ACCESS_DENIED);
    CELOG_RETURN_VAL(FALSE);
  }

  nlse_enc_error_t ret;

  ret = doDecryptFile(in_path, out_path);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToPlainFile: doDecryptFile(%s, %s) returned %d\n",
                 in_path, out_path, ret);
    SetLastError(ERROR_ACCESS_DENIED);
    CELOG_RETURN_VAL(FALSE);
  }

  // The file is unwrapped successfully, we need to recover its original file attributes
  if(INVALID_FILE_ATTRIBUTES!=dwAttrs && !::SetFileAttributesW(out_path, dwAttrs))
  {
    // Even it fail, we should still say the unwrap succeed
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToPlainFile: fail to recover file attributes(%s)\n", out_path);
    SetLastError(0);
  }
  CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
  CELOG_RETURN_VAL(TRUE);
} /* SE_UnwrapToPlainFile */

BOOL SE_WrapEncryptedFile( _In_z_ const wchar_t* in_path,
                           _In_z_ const wchar_t* out_path)
{
  CELOG_ENTER;
  DWORD dwAttrs = INVALID_FILE_ATTRIBUTES;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls, out_path=%ls \n", in_path,out_path);
	
  assert( in_path != NULL );
  assert( out_path != NULL );
  if( in_path == NULL || out_path == NULL)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_WrapEncryptedFile: in_path or out_path is NULL\n");
	CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }
  else if( _wcsicmp(out_path + wcslen(out_path) - wcslen(NL_FILE_EXT),
                    NL_FILE_EXT) != 0 )
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_WrapEncryptedFile: %s does not have %s extension\n",
                 out_path, NL_FILE_EXT);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }
  
  // Get its original file attributes
  dwAttrs = GetFileAttributes(in_path);

  const copy_encrypted_file_key_option_t keyOpt = CEFKO_SWITCH_TO_SHARED;
  nlse_enc_error_t ret;

  ret = doCopyEncryptedFile(in_path, out_path, keyOpt);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_WrapEncryptedFile: doCopyEncryptedFile(%s, %s, %d) returned %d\n",
                 in_path, out_path, keyOpt, ret);
    SetLastError(ret == NLSE_ENC_ERROR_NOT_ENOUGH_MEMORY ?
                 ERROR_NOT_ENOUGH_MEMORY : ERROR_ACCESS_DENIED);
	if(ret == NLSE_ENC_ERROR_CANT_GET_PC_KEY)
	{
		CELOG_LOG(CELOG_ERROR,L"Error: Shared key is not available.\n");
	}
	CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
    CELOG_RETURN_VAL(FALSE);
  }

  if (!SE_SetWrappedFileFlags(out_path))
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_WrapEncryptedFile: fail to set flags in NXL file(%s)\n", out_path);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
    CELOG_RETURN_VAL(FALSE);
  }

  // The file is wrapped successfully, we need to save its original attributes in wrapped NXL file
  if(!SE_SetNxlFileAttributes(out_path, dwAttrs))
  {
    // We just record that set file attributes failed
    // The wrap action still succeed
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_WrapEncryptedFile: fail to save file attributes in NXL file(%s)\n", out_path);
    SetLastError(0);
  }
  CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
  CELOG_RETURN_VAL(TRUE);
} /* SE_WrapEncryptedFile */

BOOL SE_UnwrapToEncryptedFile( _In_z_ const wchar_t* in_path,
                               _In_z_ const wchar_t* out_path,
                               _In_ BOOL switch_to_local_key)

{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls, out_path=%ls, switch_to_local_key=%ls \n", in_path,out_path,switch_to_local_key?L"TRUE":L"FALSE");
	
  assert( in_path != NULL );
  assert( out_path != NULL );
  if( in_path == NULL || out_path == NULL)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToEncryptedFile: in_path or out_path is NULL\n");
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }
  else if( !SE_IsWrappedFile(in_path) )
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToEncryptedFile: %s is not a wrapped file\n",
                 in_path);
    SetLastError(ERROR_BAD_FILE_TYPE);
    CELOG_RETURN_VAL(FALSE);
  }
  else if( _wcsicmp(in_path + wcslen(in_path) - wcslen(NL_FILE_EXT),
                    NL_FILE_EXT) != 0 )
  {
    CELOG_LOG(CELOG_WARNING, L"NLSE!SE_UnwrapToEncryptedFile: %s does not have %s extension\n",
                 in_path, NL_FILE_EXT);
  }

  //To Check The License
	nextlabs::feature_manager feat;
	feat.open();
	if( feat.is_enabled(NEXTLABS_FEATURE_ENCRYPTION_SYSTEM) == false )
	{	
		SE_SetLastError(NLSE_ENC_ERROR_INVALID_FEATURE_ENCRYPTION_SYSTEM);	    
    		CELOG_RETURN_VAL(FALSE);
	}

  // If target file is on remote drive, don't do it
  if(!is_local_disk(out_path))
  {
      CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToEncryptedFile: unwrap encrypted file to remote is not allowed (%s -> %s)\n", in_path, out_path);
      SetLastError(ERROR_INVALID_PARAMETER);
      CELOG_RETURN_VAL(FALSE);
  }

  // Get its original file attributes
  DWORD dwAttrs = INVALID_FILE_ATTRIBUTES;
  if(!SE_GetNxlFileAttributes(in_path, &dwAttrs) || INVALID_FILE_ATTRIBUTES==dwAttrs)
  {
    // We just record that set file attributes failed
    // The wrap action still succeed
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToEncryptedFile: fail to get file attributes from NXL file(%s)\n", in_path);
  }

  const copy_encrypted_file_key_option_t keyOpt =
    (switch_to_local_key ? CEFKO_SWITCH_TO_LOCAL : CEFKO_KEEP);
  nlse_enc_error_t ret;

  ret = doCopyEncryptedFile(in_path, out_path, keyOpt);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToEncryptedFile: doCopyEncryptedFile(%s, %s, %d) returned %d\n",
                 in_path, out_path, keyOpt, ret);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
    SetLastError(ret == NLSE_ENC_ERROR_NOT_ENOUGH_MEMORY ?
                 ERROR_NOT_ENOUGH_MEMORY : ERROR_ACCESS_DENIED);
    CELOG_RETURN_VAL(FALSE);
  }

  if (!SE_ClearWrappedFileFlags(out_path))
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToEncryptedFile: fail to clear flags in NXL file(%s)\n", out_path);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
    CELOG_RETURN_VAL(FALSE);
  }

  // The file is unwrapped successfully, we need to recover its original file attributes
  if(INVALID_FILE_ATTRIBUTES!=dwAttrs && !::SetFileAttributesW(out_path, dwAttrs))
  {
    // Even it fail, we should still say the unwrap succeed
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_UnwrapToEncryptedFile: fail to recover file attributes(%s)\n", out_path);
    SetLastError(0);
  }
  CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttrs=%lu \n", dwAttrs );
  CELOG_RETURN_VAL(TRUE);
} /* SE_UnwrapToEncryptedFile */

BOOL SE_CopyEncryptedFile( _In_z_ const wchar_t* in_path,
                           _In_z_ const wchar_t* out_path)
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_path=%ls, out_path=%ls \n", in_path,out_path);
	
  assert( in_path != NULL );
  assert( out_path != NULL );
  if( in_path == NULL || out_path == NULL)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_CopyEncryptedFile: in_path or out_path is NULL\n");
    SetLastError(ERROR_INVALID_PARAMETER);
    CELOG_RETURN_VAL(FALSE);
  }

  const copy_encrypted_file_key_option_t keyOpt = CEFKO_KEEP;
  nlse_enc_error_t ret;

  ret = doCopyEncryptedFile(in_path, out_path, keyOpt);
  if (ret != NLSE_ENC_ERROR_SUCCESS)
  {
    CELOG_LOG(CELOG_ERROR, L"NLSE!SE_CopyEncryptedFile: doCopyEncryptedFile(%s, %s, %d) returned %d\n",
                 in_path, out_path, keyOpt, ret);
    SetLastError(ret == NLSE_ENC_ERROR_NOT_ENOUGH_MEMORY ?
                 ERROR_NOT_ENOUGH_MEMORY : ERROR_ACCESS_DENIED);
    CELOG_RETURN_VAL(FALSE);
  }

  // Copy the file attributres from source to destination
  DWORD dwAttributes = GetFileAttributesW(in_path);
  SetFileAttributesW(out_path, dwAttributes);
  
  CELOG_LOG(CELOG_DUMP, L"Local variables are: dwAttributes=%lu \n", dwAttributes );
  CELOG_RETURN_VAL(TRUE);
} /* SE_CopyEncryptedFile */

_Check_return_
static
BOOL
SE_GetNxlFileAttributes(
                        _In_z_ LPCWSTR wzFile,
                        _Out_ PDWORD pdwAttrs
                        )
{
    CELOG_ENTER;
    NextLabsFile_Header_t info;

    *pdwAttrs = INVALID_FILE_ATTRIBUTES;
    memset(&info, 0, sizeof(NextLabsFile_Header_t));
    if(!SE_GetFileInfo(wzFile, SE_FileInfo_NextLabs, &info))
        CELOG_RETURN_VAL(FALSE);
    if(0==info.attrs || INVALID_FILE_ATTRIBUTES==info.attrs)
        CELOG_RETURN_VAL(FALSE);

    *pdwAttrs = info.attrs;
    CELOG_RETURN_VAL(TRUE);
}

_Check_return_
static
BOOL
SE_SetNxlFileAttributes(
                        _In_z_ LPCWSTR wzFile,
                        _In_ DWORD dwAttrs
                        )
{
    CELOG_ENTER;
    NextLabsFile_Header_t info;

    if(0==dwAttrs || INVALID_FILE_ATTRIBUTES==dwAttrs)
        CELOG_RETURN_VAL(FALSE);

    memset(&info, 0, sizeof(NextLabsFile_Header_t));
    if(!SE_GetFileInfo(wzFile, SE_FileInfo_NextLabs, &info))
        CELOG_RETURN_VAL(FALSE);

    info.attrs = dwAttrs;
    CELOG_RETURN_VAL(SE_SetFileInfo(wzFile, SE_FileInfo_NextLabs, &info));
}

_Check_return_
static
BOOL
SE_SetWrappedFileFlags(
                       _In_z_ LPCWSTR wzFile
                       )
{
    CELOG_ENTER;
    NextLabsFile_Header_t info;

    if(!SE_GetFileInfo(wzFile, SE_FileInfo_NextLabs, &info))
        CELOG_RETURN_VAL(FALSE);

    info.flags |= NLF_WRAPPED;
    if(!SE_SetFileInfo(wzFile, SE_FileInfo_NextLabs, &info))
        CELOG_RETURN_VAL(FALSE);

    NextLabsEncryptionFile_Header_t encExt;

    if(!SE_GetFileInfo(wzFile, SE_FileInfo_NextLabsEncryption, &encExt))
        CELOG_RETURN_VAL(FALSE);

    encExt.flags |= NLEF_REQUIRES_LOCAL_ENCRYPTION;
    CELOG_RETURN_VAL(SE_SetFileInfo(wzFile, SE_FileInfo_NextLabsEncryption, &encExt));
}

_Check_return_
static
BOOL
SE_ClearWrappedFileFlags(
                       _In_z_ LPCWSTR wzFile
                       )
{
    CELOG_ENTER;
    NextLabsFile_Header_t info;

    if(!SE_GetFileInfo(wzFile, SE_FileInfo_NextLabs, &info))
        CELOG_RETURN_VAL(FALSE);

    info.flags &= ~NLF_WRAPPED;
    if(!SE_SetFileInfo(wzFile, SE_FileInfo_NextLabs, &info))
        CELOG_RETURN_VAL(FALSE);

    NextLabsEncryptionFile_Header_t encExt;

    if(!SE_GetFileInfo(wzFile, SE_FileInfo_NextLabsEncryption, &encExt))
        CELOG_RETURN_VAL(FALSE);

    encExt.flags &= ~NLEF_REQUIRES_LOCAL_ENCRYPTION;
    CELOG_RETURN_VAL(SE_SetFileInfo(wzFile, SE_FileInfo_NextLabsEncryption, &encExt));
}

void SE_PurgeDRMList()
{
  CELOG_ENTER;
  nlthread_mutex_lock(&_global.drmPathListMutex);
  if(NULL != _global.drmPathList) free(_global.drmPathList);
  _global.drmPathList    = NULL;
  _global.drmPathListLen = 0;
  nlthread_mutex_unlock(&_global.drmPathListMutex);
  CELOG_RETURN;
}

BOOL SE_MarkFileAsDRMOneShot( _In_z_ const wchar_t* path )
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: path=%ls \n", path);
	
  NLSE_USER_COMMAND cmd;

  init_cmd(&cmd);
  cmd.type = NLSE_USER_COMMAND_ADD_DRM_FILE_ONE_SHOT;
  wcsncpy_s(cmd.msg.fname, NLSE_MAX_PATH, path, _TRUNCATE);
  cmd.msg.pid = GetCurrentProcessId();

  CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_MarkFileAsDRMOneShot: path=\"%s\", PID=%lu\n",
               path, cmd.msg.pid);

  if(WriteCommand(&cmd) == 0) {
    //write command failed
    CELOG_RETURN_VAL(FALSE);
  }

  CELOG_RETURN_VAL(TRUE);
} /* SE_MarkFileAsDRMOneShot */

BOOL SE_UnmarkFileAsDRMOneShot( _In_z_ const wchar_t* path )
{
  CELOG_ENTER;
  ensureInited();
  CELOG_LOG(CELOG_DUMP, L" The Parameters are: path=%ls \n", path);
  
  NLSE_USER_COMMAND cmd;

  init_cmd(&cmd);
  cmd.type = NLSE_USER_COMMAND_REMOVE_DRM_FILE_ONE_SHOT;
  wcsncpy_s(cmd.msg.fname, NLSE_MAX_PATH, path, _TRUNCATE);
  cmd.msg.pid = GetCurrentProcessId();

  CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_UnmarkFileAsDRMOneShot: path=\"%s\", PID=%lu\n",
               path, cmd.msg.pid);

  if(WriteCommand(&cmd) == 0) {
    //write command failed
    CELOG_RETURN_VAL(FALSE);
  }

  CELOG_RETURN_VAL(TRUE);
} /* SE_UnmarkFileAsDRMOneShot */

BOOL SE_UnmarkAllFilesAsDRMOneShot( void )
{
  CELOG_ENTER;
  ensureInited();

  NLSE_USER_COMMAND cmd;

  init_cmd(&cmd);
  cmd.type = NLSE_USER_COMMAND_REMOVE_ALL_DRM_FILES_ONE_SHOT;
  cmd.msg.pid = GetCurrentProcessId();

  CELOG_LOG(CELOG_DEBUG, L"NLSE!SE_UnmarkAllFilesAsDRMOneShot: PID=%lu\n",
               cmd.msg.pid);

  if(WriteCommand(&cmd) == 0) {
    //write command failed
    CELOG_RETURN_VAL(FALSE);
  }

  CELOG_RETURN_VAL(TRUE);
} /* SE_UnmarkAllFilesAsDRMOneShot */
