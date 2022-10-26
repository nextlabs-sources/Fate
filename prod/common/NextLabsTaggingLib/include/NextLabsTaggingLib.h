#ifndef __NEXTLABS_TAGGING_LIB_H__
#define __NEXTLABS_TAGGING_LIB_H__

#include <Windows.h>



#define NL_ATTR_KEY_WRAPPED                     L"NXL_wrapped"
#define NLE_ATTR_KEY_ENCRYPTED                  L"NXL_encrypted"
#define NLE_ATTR_KEY_FAST_WRITE_ENCRYPTED       L"NXL_heavyWriteEncrypted"
#define NLE_ATTR_KEY_KEY_RING_NAME              L"NXL_keyRingName"
#define NLE_ATTR_KEY_REQUIRES_LOCAL_ENCRYPTION  L"NXL_requiresLocalEncryption"



/** NLT_GetAllTags
 *
 *  \brief Get all tags in a NextLabs file.  Both System-encrypted
 *         files and .nxl files are supported.
 *         NextLabs files:          Both "NXL_xxx" encryption-info tags
 *                                  generated from NextLabsEncryption header
 *                                  and other tags (if any) in NextLabsTagging
 *                                  header are returned.
 *         other files:             No tags are returned.  (This is an error.)
 *
 *  \param in_path (in) Fully-qualified path to NextLabs Tagging file
 *  \param pCount (out) Pointer to tag count to return
 *  \param pTagKeyValuePairs (out) Pointer to array of key/value string pairs.
 *         Caller should call NLT_FreeAllTags to free the memory afterwards
 *         (if tags are returned).
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return
 *          appropriate error code.
 */
_Check_return_
_Success_(return == TRUE)
extern "C"
BOOL NLT_GetAllTags(
    _In_z_                                      const wchar_t* in_path,
    _Out_                                       PULONG pCount,
    _Deref_post_opt_count_x_((*pCount)*2)				wchar_t ***pTagKeyValuePairs);

/** NLT_FreeAllTags
 *
 *  \brief Free all tag memory returned by NLT_GetAllTags.
 *
 *  \param count (in) Tag count
 *  \param tagKeyValuePairs (in) Array of key/value string pairs.
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return
 *          appropriate error code.
 */
extern "C"
BOOL NLT_FreeAllTags(
    _In_                                        ULONG count,
    _In_count_x_(count * 2)                      wchar_t **tagKeyValuePairs);

/** NLT_AddTag
 *
 *  \brief Add a tag to a NextLabs Tagging file.
 *         NextLabs files:          "NXL_xxx" encryption-info tags are silently
 *                                  ignored.  Other tags are added to
 *                                  NextLabsTagging header.
 *         other files:             No tags are added.  (This is an error.)
 *
 *  \param in_path (in) Fully-qualified path to NextLabs Tagging file
 *  \param key (in) Tag key
 *  \param value (in) Tag value
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return
 *          appropriate error code.
 */
extern "C"
BOOL NLT_AddTag(
    _In_z_                          const wchar_t* in_path,
    _In_z_                          const wchar_t* key,
    _In_z_                          const wchar_t* value);



/** NLT_DeleteTag
 *
 *  \brief Delete a tag to a NextLabs Tagging file.
 *
 *  \param in_path (in) Fully-qualified path to NextLabs Tagging file
 *  \param key (in) Tag key
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return
 *          appropriate error code.
 */
extern "C"
BOOL NLT_DeleteTag(
    _In_z_                          const wchar_t* in_path,
    _In_z_                          const wchar_t* key);

#endif /* __NEXTLABS_TAGGING_LIB_H__ */
