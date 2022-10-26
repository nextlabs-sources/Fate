#ifndef __NL_SYSENC_LIB_FW_H__
#define __NL_SYSENC_LIB_FW_H__

#include <windows.h>

/** SE_EncryptFileFW
 *
 *  \brief Encrypt a Fast-Write file using the current key.
 *
 *  \param path (in) Path to file which should be encrypted.
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return ERROR_NOT_SUPPORT
 *          when the target file system does not support System Encryption
 */
extern "C"
BOOL SE_EncryptFileFW( _In_z_ const wchar_t* path );
extern "C"
BOOL SE_EncryptFileFWEx( _In_z_ const wchar_t* path, _In_ BOOL force );

/** SE_DecryptFileFW
 *
 *  \brief Decrypt a Fast-Write file using the current key.
 *
 *  \param in_path (in) Path to file which should be decrypted.
 *  \param out_path (in) (optional) Path to decrypted file.
 *  \param password (in) Password that allows decryption.
 *
 *  \return TRUE on success, otherwise FALSE.
 */
extern "C"
BOOL SE_DecryptFileFW( _In_z_ const wchar_t* in_path,
                       _In_opt_z_ const wchar_t* out_path,
                       _In_z_ const wchar_t* password );

/** SE_EncryptDirectoryFW
 *
 *  \brief Set Fast-Write encryption on a given directory.
 *
 *  \param path (in) Path to directory which should be encrypted.
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return ERROR_NOT_SUPPORT
 *          when the target file system does not support System Encryption
 */
extern "C"
BOOL SE_EncryptDirectoryFW( _In_z_ const wchar_t* path );

/** SE_DecryptDirectoryFW
 *
 *  \brief Clear Fast-Write encryption on a given directory.
 *
 *  \param path (in) Path to directory which should be decrypted.
 *  \param password (in) Password that allows decryption.
 *
 *  \return TRUE on success, otherwise FALSE.
 */
extern "C"
BOOL SE_DecryptDirectoryFW( _In_z_ const wchar_t* path,
                          _In_z_ const wchar_t* password );

/** SE_IsEncryptedFW
 *
 *  \brief Determine if the given object (file or directory) is Fast-Write-
 *         encrypted.
 *
 *  \param path (in) Path to directory or file to check for encryption.
 *
 *  \return TRUE if encrypted, otherwise FALSE.  If SE_IsEncrypted fails
 *          GetLastError() will have a value other than ERROR_SUCCESS.
 */
extern "C"
BOOL SE_IsEncryptedFW( _In_z_ const wchar_t* path );

/** SE_GetFileInfoFW
 *
 *  \brief Read file information from a Fast-Write Encrypted file.
 *
 *  \param in_file (in)      Path to file which should be encrypted.
 *  \param in_file_info (in) File information type to read.
 *  \param info (out)        Buffer for file info.
 *
 *  \return TRUE on success, otherwise FALSE.
 */
_Check_return_
extern "C"
BOOL SE_GetFileInfoFW( _In_z_ const wchar_t* in_file ,
		     _In_ NextLabs_FileInfo_t in_file_info ,
		     _Out_bytecapcount_x_(
		     			in_file_info == SE_FileInfo_NextLabs ?
                      	sizeof(NextLabsFile_Header_1_0_t) : NextLabsEncryptionFile_Header_1_0_t)
                      			  void* info );

/** SE_PurgeDRMListFW
 *
 *  \brief Purge DRM list in SELib
 */
extern "C"
void SE_PurgeFWDRMList();

#endif /* __NLSE_LIB_FW_H__ */
