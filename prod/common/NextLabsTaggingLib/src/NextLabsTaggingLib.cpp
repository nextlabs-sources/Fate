#define WINVER _WIN32_WINNT_WINXP
#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#endif

#include "celog.h"
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"
#include "nlconfig.hpp"
#include "NextLabsEncryption_Types.h"
#include "NextLabsTagging_Types.h"
#include "NextLabsTaggingLib.h"
#include "nl_sysenc_lib.h"
#include "nl_sysenc_lib_fw.h"
#include "NextLabsTagMap.h"



static CELog nltLog;
static bool nltLogInitialized = false;



/*
 * Ensure that file logging is initialized
 */
static int ensureLogInited(void)
{
  if (nltLogInitialized) {
    return 0;
  }

  nltLog.SetLevel(CELOG_INFO); /* default */
  if( NLConfig::IsDebugMode() == true ) {
    nltLog.SetLevel(CELOG_DEBUG);
    nltLog.SetPolicy( new CELogPolicy_WinDbg() );
  }

  char file[MAX_PATH] = {0};
  WCHAR pc_root[MAX_PATH] = {0};
  if( NLConfig::GetComponentInstallPath(L"Compliant Enterprise\\Policy Controller",pc_root,_countof(pc_root)) == false )
  {
    wcsncpy_s(pc_root,_countof(pc_root),L"C:\\Program Files\\NextLabs\\Policy Controller",_TRUNCATE);
  }
  _snprintf_s(file,_countof(file), _TRUNCATE,"%ws\\diags\\logs\\nlt.log",pc_root);
  CELogPolicy_File* file_policy = new CELogPolicy_File(file);
  nltLog.SetPolicy(file_policy);

  nltLogInitialized = true;
  nltLog.Log(CELOG_INFO, L"NextLabsTaggingLib log initialized.\n");

  return 0;
} /* ensureLogInited */

_Check_return_
static BOOL isTagStreamValid(_In_ const NextLabsTaggingFile_Header_t *pTagStream)
{
  // Make sure tags stream exists at the right offset.
  if (strcmp((const char *) pTagStream->sh.stream_name, NLT_STREAM_NAME) != 0)
  {
    nltLog.Log(CELOG_ERR, L"NLT!isTagStreamValid: NLT stream not found\n");
    return FALSE;
  }

  // Make sure tagsSize is within bounds.
  if (pTagStream->tagsSize % sizeof(wchar_t) != 0 ||
      pTagStream->tagsSize > sizeof pTagStream->tag_data)
  {
    nltLog.Log(CELOG_ERR, L"NLT!isTagStreamValid: tagsSize corrupted\n");
    return FALSE;
  }

  unsigned long numStrings = 0;
  unsigned int i;

  // Count the number of strings.
  for (i = 0; i < pTagStream->tagsSize / sizeof(wchar_t); i++)
  {
    if (pTagStream->tag_data[i] == L'\0')
    {
      numStrings++;
    }
  }

  // Make sure that the last string ends with a '\0' and there are an even
  // number of strings.
  if ((i != 0 && pTagStream->tag_data[i - 1] != L'\0') ||
      numStrings % 2 != 0)
  {
    nltLog.Log(CELOG_ERR, L"NLT!isTagStreamValid: tag_data corrupte\n");
    return FALSE;
  }

  return TRUE;
} /* isTagStreamValid */

// update or delete tagInfo(key and value pair)
// input: 
//		pTagStream: header for tagging info
//      taggingData: array of strings(tag/value pair)
//      key: tag name
//      value: tag value
//      deletedFlag: 0(add or update), 1(delete)
// output:
//      taggingData: new array of tag/value pair
static BOOL updateOrDeleteDupTags( _In_ NextLabsTaggingFile_Header_t *pTagStream,
								 _Inout_ wchar_t* taggingData,
								 _In_z_  const wchar_t* key,
								 _In_z_  const wchar_t* value,
								 BOOL deletedFlag = 0)  
{
    NL_Tagging_Map tagMap;
	size_t newNoTags; // new number of tags
	size_t tagSize = 0;
	size_t numTags = 0;
	int maxDataSize = sizeof(wchar_t)*MAX_TAG_DATA_SIZE;
	tagMap.import_keys(taggingData, pTagStream->tagsSize, &numTags);
	// delete or add based on input flag
	if( deletedFlag == 0 )
		tagMap.add_key(key,value);
	else
	{
		if( tagMap.delete_key(key) == 1 )
		{
			// Not Found
			nltLog.Log(CELOG_ERR, L"NLT!updateOrDeleteDupTags: tag name %s does not exist\n", key);
			SetLastError(ERROR_TAG_NOT_FOUND);
			return FALSE;
		};
	}
	ZeroMemory(taggingData, maxDataSize);
	tagMap.export_keys(taggingData, &newNoTags, &tagSize);
	pTagStream->tagsSize = static_cast<int> (tagSize);
	return TRUE;
} /* updateOrDeleteDupTags */

_Check_return_
_Success_(return == TRUE)
static BOOL addStringHelper(
    _Deref_pre_opt_cap_x_(*pNumStrings) _Deref_post_opt_count_x_(*pNumStrings) wchar_t ***pStrArray,
    _Inout_                                     unsigned long *pNumStrings,
    _In_z_                                      const wchar_t *str)
{
  wchar_t ** const newArray = (wchar_t **)
    realloc(*pStrArray, (*pNumStrings + 1) * sizeof *newArray);
  if (newArray == NULL)
  {
    return FALSE;
  }
  *pStrArray = newArray;

  const size_t len = wcslen(str);

  (*pStrArray)[*pNumStrings] = (wchar_t*) malloc((len + 1) * sizeof(wchar_t));
  if ((*pStrArray)[*pNumStrings] == NULL)
  {
    return FALSE;
  }

  wcsncpy_s((*pStrArray)[*pNumStrings], len + 1, str, _TRUNCATE);
  (*pNumStrings)++;
  return TRUE;
} /* addStringHelper */

static VOID removeStringHelper(
    _Deref_pre_opt_cap_x_(*pNumStrings) _Deref_post_opt_count_x_(*pNumStrings) wchar_t ***pStrArray,
    _Inout_                                     unsigned long *pNumStrings
)
{
  if (*pNumStrings == 0 || *pStrArray == NULL)
  {
    return;
  }

  free((*pStrArray)[*pNumStrings - 1]);

  // Even though realloc never returns error when shrinking a block, and only
  // returns NULL when we are shrinking to zero bytes which is what we want
  // anyway, here we still need to use a temp variable just to keep PREfast
  // happy.
  wchar_t ** const tempArray = (wchar_t **)
    realloc(*pStrArray, (*pNumStrings - 1) * sizeof *tempArray);
  *pStrArray = tempArray;

  (*pNumStrings)--;
} /* removeStringHelper */

_Check_return_
_Success_(return == TRUE)
static BOOL getTagsFromNLTHeader(
    _In_z_                                      const wchar_t* in_path,
    _Inout_                                     PULONG pCount,
    _Deref_pre_opt_cap_x_((*pCount)*2) _Deref_post_opt_count_x_((*pCount)*2)               wchar_t ***pTagKeyValuePairs)
{
  BOOL ret = FALSE;
  DWORD lastErr = ERROR_SUCCESS;
  unsigned long numStrings = *pCount * 2;

  // Read the Tagging stream.
  NextLabsTaggingFile_Header_t tagStream;  

  if (!SE_GetFileInfo(in_path, SE_FileInfo_NextLabsTagging, &tagStream))
  {
    lastErr = GetLastError();
    goto exit;
  }

  if (!isTagStreamValid(&tagStream))
  {
    lastErr = ERROR_FILE_CORRUPT;
    goto exit;
  }

  // Copy all the tag strings.
  ULONG i = 0, iPrev = 0;

  while (i < tagStream.tagsSize / sizeof(wchar_t))
  {
    if (tagStream.tag_data[i++] == L'\0')
    {
      if (!addStringHelper(pTagKeyValuePairs, &numStrings,
                           &tagStream.tag_data[iPrev]))
      {
        lastErr = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
      }

      iPrev = i;
    }
  }

  *pCount = numStrings / 2;
  ret = TRUE;

exit:
  if (!ret)
  {
    // Free all the strings that were added during this call, but not the ones
    // that were passed in.
    while (numStrings > *pCount * 2)
    {
      removeStringHelper(pTagKeyValuePairs, &numStrings);
    }

    SetLastError(lastErr);
  }

  return ret;
} /* getTagsFromNLTHeader */

_Check_return_
_Success_(return == TRUE)
static BOOL generateTagsFromNLEHeader(
    _In_z_                                      const wchar_t* in_path,
    _Inout_                                     PULONG pCount,
    _Deref_pre_opt_cap_x_((*pCount)*2) _Deref_post_opt_count_x_((*pCount)*2)  wchar_t ***pTagKeyValuePairs)
{
  BOOL ret = FALSE;
  DWORD lastErr = ERROR_SUCCESS;
  unsigned long numStrings = *pCount * 2;

  // Read the Encryption stream.
  const BOOL isFW = SE_IsEncryptedFW(in_path);
  NextLabsEncryptionFile_Header_t encStream;
  NextLabsEncryptionFile_Header_1_0_t encStream10;

  if (isFW)
  {
    ret = SE_GetFileInfoFW(in_path, SE_FileInfo_NextLabsEncryption,
                           &encStream10);
    // Initialize encStream.flags even though we'd never use it when isFW is
    // TRUE, just to keep PREfast happy.
    encStream.flags = 0;
  }
  else
  {
    ret = SE_GetFileInfo(in_path, SE_FileInfo_NextLabsEncryption,
                         &encStream);
  }

  if (!ret)
  {
      lastErr = GetLastError();
      goto exit;
  }

  lastErr = ERROR_NOT_ENOUGH_MEMORY;

  // NXL_encrypted
  if (!addStringHelper(pTagKeyValuePairs, &numStrings, NLE_ATTR_KEY_ENCRYPTED))
  {
    goto exit;
  }
  if (!addStringHelper(pTagKeyValuePairs, &numStrings, L"TRUE"))
  {
    goto exit;
  }

  if (isFW)
  {
    // NXL_heavyWriteEncrypted
    if (!addStringHelper(pTagKeyValuePairs, &numStrings,
                         NLE_ATTR_KEY_FAST_WRITE_ENCRYPTED))
    {
      goto exit;
    }
    if (!addStringHelper(pTagKeyValuePairs, &numStrings, L"TRUE"))
    {
      goto exit;
    }
  }

  // NXL_keyRingName
  WCHAR wKeyRingName[NLE_KEY_RING_NAME_MAX_LEN + 1] = {0};
  MultiByteToWideChar(CP_UTF8, 0,
                      isFW ? encStream10.pcKeyRingName:encStream.pcKeyRingName,
                      NLE_KEY_RING_NAME_MAX_LEN, wKeyRingName,
                      NLE_KEY_RING_NAME_MAX_LEN);
  wKeyRingName[NLE_KEY_RING_NAME_MAX_LEN] = L'\0';

  if (!addStringHelper(pTagKeyValuePairs, &numStrings,
                       NLE_ATTR_KEY_KEY_RING_NAME))
  {
    goto exit;
  }
  if (!addStringHelper(pTagKeyValuePairs, &numStrings, wKeyRingName))
  {
    goto exit;
  }

  if (!isFW)
  {
    // NXL_requiresLocalEncryption
    if (!addStringHelper(pTagKeyValuePairs, &numStrings,
                         NLE_ATTR_KEY_REQUIRES_LOCAL_ENCRYPTION))
    {
      goto exit;
    }
    if (!addStringHelper(pTagKeyValuePairs, &numStrings,
                         (encStream.flags & NLEF_REQUIRES_LOCAL_ENCRYPTION ?
                          L"TRUE" : L"FALSE")))
    {
      goto exit;
    }
  }

  *pCount = numStrings / 2;
  ret = TRUE;

exit:
  if (!ret)
  {
    // Free all the strings that were added during this call, but not the ones
    // that were passed in.
    while (numStrings > *pCount * 2)
    {
      removeStringHelper(pTagKeyValuePairs, &numStrings);
    }

    SetLastError(lastErr);
  }

  return ret;
} /* generateTagsFromNLEHeader */

_Check_return_
_Success_(return == TRUE)
static BOOL generateTagsFromNLHeader(
    _In_z_                                      const wchar_t* in_path,
    _Inout_                                     PULONG pCount,
    _Deref_pre_opt_cap_x_((*pCount)*2) _Deref_post_opt_count_x_((*pCount)*2)  wchar_t ***pTagKeyValuePairs)
{
  BOOL ret = FALSE;
  DWORD lastErr = ERROR_SUCCESS;
  unsigned long numStrings = *pCount * 2;

  // Read the NextLabs header.
  const BOOL isFW = SE_IsEncryptedFW(in_path);
  NextLabsFile_Header_t nlHeader;

  if (isFW)
  {
    return TRUE;
  }

  ret = SE_GetFileInfo(in_path, SE_FileInfo_NextLabs, &nlHeader);
  if (!ret)
  {
    lastErr = GetLastError();
    goto exit;
  }

  lastErr = ERROR_NOT_ENOUGH_MEMORY;

  // NXL_wrapped
  if (!addStringHelper(pTagKeyValuePairs, &numStrings, NL_ATTR_KEY_WRAPPED))
  {
    goto exit;
  }
  if (!addStringHelper(pTagKeyValuePairs, &numStrings,
                       (nlHeader.flags & NLF_WRAPPED ? L"TRUE" : L"FALSE")))
  {
    goto exit;
  }

  *pCount = numStrings / 2;
  ret = TRUE;

exit:
  if (!ret)
  {
    // Free all the strings that were added during this call, but not the ones
    // that were passed in.
    while (numStrings > *pCount * 2)
    {
      removeStringHelper(pTagKeyValuePairs, &numStrings);
    }

    SetLastError(lastErr);
  }

  return ret;
} /* generateTagsFromNLHeader */

_Check_return_
_Success_(return == TRUE)
extern "C"
BOOL NLT_GetAllTags(
    _In_z_                                      const wchar_t* in_path,
    _Out_                                       PULONG pCount,
    _Deref_post_opt_count_x_((*pCount)*2)               wchar_t ***pTagKeyValuePairs)
{
  ensureLogInited();
  
  assert( in_path != NULL );
  assert( pCount != NULL );
  assert( pTagKeyValuePairs != NULL );
  if( in_path == NULL || pCount == NULL || pTagKeyValuePairs == NULL )
  {
    nltLog.Log(CELOG_ERR, L"NLT!NLT_GetAllTags: in_path or pCount or pTagKeyValuePairs is NULL\n");
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }

  if (!SE_IsNXLFile(in_path))
  {
    if (GetLastError() == ERROR_SUCCESS)
    {
      nltLog.Log(CELOG_ERR, L"NLT!NLT_GetAllTags: %s is not NextLabs file\n",
                 in_path);
      SetLastError(ERROR_BAD_FILE_TYPE);
    }
    else
    {
      nltLog.Log(CELOG_ERR, L"NLT!NLT_GetAllTags: cannot access %s, lastError = %lu\n",
                 in_path, GetLastError());
    }

    return FALSE;
  }

  *pCount = 0;
  *pTagKeyValuePairs = NULL;

  // Get tags from NLT header only if the file is normal-encrypted, because
  // heavy-write encrypted files don't support NLT header.
  if (!SE_IsEncryptedFW(in_path))
  {
    if (GetLastError() != ERROR_SUCCESS)
    {
      // SE_IsEncryptedFW() failed.
      return FALSE;
    }
    else
    {
      // File is not heavy-write-encrypted.  Get tags from NLT header.
      if (!getTagsFromNLTHeader(in_path, pCount, pTagKeyValuePairs))
      {
        return FALSE;
      }
    }
  }

  // Generate tags from NLE header.
  if (!generateTagsFromNLEHeader(in_path, pCount, pTagKeyValuePairs))
  {
    // Free the tags from NLT header, if any.
    if (*pCount != 0)
    {
      NLT_FreeAllTags(*pCount, *pTagKeyValuePairs);
    }

    return FALSE;
  }

  // Generate tags from NL header.
  if (!generateTagsFromNLHeader(in_path, pCount, pTagKeyValuePairs))
  {
    // Free the tags from NLT and NLE headers, if any.
    if (*pCount != 0)
    {
      NLT_FreeAllTags(*pCount, *pTagKeyValuePairs);
    }

    return FALSE;
  }

  return TRUE;
} /* NLT_GetAllTags */

extern "C"
BOOL NLT_FreeAllTags(
    _In_                                        ULONG count,
    _In_count_x_(count * 2)                      wchar_t **tagKeyValuePairs)
{
  ULONG i;

  if( tagKeyValuePairs == NULL )
  {
    nltLog.Log(CELOG_ERR, L"NLT!NLT_FreeAllTags: tagKeyValuePairs is NULL\n");
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }
  else
  {
    for (i = 0; i < count * 2; i++)
    {
      if (tagKeyValuePairs[i] == NULL)
      {
        nltLog.Log(CELOG_ERR, L"NLT!NLT_FreeAllTags: tagKeyValuePairs[i] is NULL\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
      }
    }
  }

  for (i = 0; i < count * 2; i++)
  {
    free(tagKeyValuePairs[i]);
  }

  free(tagKeyValuePairs);

  return TRUE;
} /* NLT_FreeAllTags */

extern "C"
BOOL NLT_AddTag(
    _In_z_                          const wchar_t* in_path,
    _In_z_                          const wchar_t* key,
    _In_z_                          const wchar_t* value)
{
  WCHAR *newValue = NULL;
  ensureLogInited();

  assert( in_path != NULL );
  assert( key != NULL );
  assert( value != NULL );
  if( in_path == NULL || key == NULL || value == NULL )
  {
    nltLog.Log(CELOG_ERR, L"NLT!NLT_AddTag: in_path or key or value is NULL\n");
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }

  if (!SE_IsNXLFile(in_path))
  {
    if (GetLastError() == ERROR_SUCCESS)
    {
      nltLog.Log(CELOG_ERR, L"NLT!NLT_AddTag: %s is not NextLabs file\n",
                 in_path);
      SetLastError(ERROR_BAD_FILE_TYPE);
    }
    else
    {
      nltLog.Log(CELOG_ERR, L"NLT!NLT_AddTag: cannot access %s, lastError = %lu\n",
                 in_path, GetLastError());
    }

    return FALSE;
  }

  // Silently ignore NXL_xxx encryption-info tags
  if (_wcsicmp(key, NL_ATTR_KEY_WRAPPED) == 0 ||
      _wcsicmp(key, NLE_ATTR_KEY_ENCRYPTED) == 0 ||
      _wcsicmp(key, NLE_ATTR_KEY_FAST_WRITE_ENCRYPTED) == 0 ||
      _wcsicmp(key, NLE_ATTR_KEY_KEY_RING_NAME) == 0 ||
      _wcsicmp(key, NLE_ATTR_KEY_REQUIRES_LOCAL_ENCRYPTION) == 0)
  {
    nltLog.Log(CELOG_WARNING, L"NLT!NLT_AddTag: ignoring %s since it is reserved\n",
               key);
    return TRUE;
  }

  // Read the Tagging stream.
  NextLabsTaggingFile_Header_t tagStream;  

  if (!SE_GetFileInfo(in_path, SE_FileInfo_NextLabsTagging, &tagStream))
  {
    return FALSE;
  }

  if (!isTagStreamValid(&tagStream))
  {
    SetLastError(ERROR_FILE_CORRUPT);
    return FALSE;
  }

  // See if there is enough space left.
  const size_t keyLen = sizeof(wchar_t)*(wcslen(key) + 1);
  size_t valueLen = sizeof(wchar_t)*(wcslen(value) + 1);
  bool newValueFlag = false;
  const size_t sizeLeft = sizeof tagStream.tag_data - tagStream.tagsSize;

  if(keyLen >= sizeLeft)
  {	
	nltLog.Log(CELOG_ERR, L"NLT!NLT_AddTag: %s does not have enought space to add tag (%s,%s)\n",in_path, key, value);
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
  }

  // check if the value to be copied is more than space available and truncate if necessary 
  if(valueLen > sizeLeft - keyLen)
  {		
	valueLen = sizeLeft - keyLen; 
	newValue = (WCHAR*) malloc(valueLen);
	if (newValue != NULL )
	{
		errno_t err = wcsncpy_s(newValue, valueLen/sizeof(WCHAR), value, _TRUNCATE);
		if( err != 0 )
		{
			nltLog.Log(CELOG_ERR, L"NLT!NLT_AddTag: %s may truncate key string to add tag (%s,%s)\n",
				in_path, key, value);
			SetLastError(ERROR_BUFFER_OVERFLOW);
			goto exit;
		}
		newValue[valueLen/sizeof(WCHAR)-1] = L'\0';
		newValueFlag = true;
	} else
	{
		nltLog.Log(CELOG_ERR, L"NLT!NLT_AddTag: %s does not allocate memory space to add tag (%s,%s)\n",in_path, key, value);
		return FALSE;
	}
  }

  if( newValueFlag == true )
  {
	  // remove duplicated tag(same tag key) with truncated value
	  if( (newValue != NULL) && !updateOrDeleteDupTags((NextLabsTaggingFile_Header_t*) &tagStream, tagStream.tag_data, key, newValue) )
	  {
		goto exit;
	  };
  } else
  {
	  // remove duplicated tag(same tag key)
	  if( !updateOrDeleteDupTags((NextLabsTaggingFile_Header_t*) &tagStream, tagStream.tag_data, key, value) )
	  {
		goto exit;
	  };
  }
  // Write the Tagging stream.
  if (!SE_SetFileInfo(in_path, SE_FileInfo_NextLabsTagging, &tagStream))
  {
    goto exit;
  }
  return TRUE;

exit:
  if( newValue != NULL)
	  free(newValue);
  return FALSE;
} /* NLT_AddTag */


extern "C"
BOOL NLT_DeleteTag(
    _In_z_                          const wchar_t* in_path,
    _In_z_                          const wchar_t* key)
{
  const wchar_t* value = L"dummy";
  ensureLogInited();

  assert( in_path != NULL );
  assert( key != NULL );
  assert( value != NULL );
  if( in_path == NULL || key == NULL || value == NULL )
  {
    nltLog.Log(CELOG_ERR, L"NLT!NLT_DeleteTag: in_path or key or value is NULL\n");
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }

  if (!SE_IsNXLFile(in_path))
  {
    if (GetLastError() == ERROR_SUCCESS)
    {
      nltLog.Log(CELOG_ERR, L"NLT!NLT_DeleteTag: %s is not NextLabs file\n",
                 in_path);
      SetLastError(ERROR_BAD_FILE_TYPE);
    }
    else
    {
      nltLog.Log(CELOG_ERR, L"NLT!NLT_DeleteTag: cannot access %s, lastError = %lu\n",
                 in_path, GetLastError());
    }

    return FALSE;
  }

  // Read the Tagging stream.
  NextLabsTaggingFile_Header_t tagStream;  
  if (!SE_GetFileInfo(in_path, SE_FileInfo_NextLabsTagging, &tagStream))
  {
    return FALSE;
  }

  if (!isTagStreamValid(&tagStream))
  {
    SetLastError(ERROR_FILE_CORRUPT);
    return FALSE;
  }

  // Update/delete tag
  if (!updateOrDeleteDupTags((NextLabsTaggingFile_Header_t*) &tagStream, tagStream.tag_data, key, value, 1))
  {
	return FALSE; 
  };

  // Write the Tagging stream.
  if (!SE_SetFileInfo(in_path, SE_FileInfo_NextLabsTagging, &tagStream))
  {
    return FALSE;
  }
  return TRUE;
} /* NLT_DeleteTag */