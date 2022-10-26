
#ifndef __NL_SYSENC_LIB_H__
#define __NL_SYSENC_LIB_H__

#include <windows.h>

/** NextLabs_FileInfo_t
 *
 *  \brief File information type.
 */
typedef enum
{
  SE_FileInfo_NextLabs,             /* NextLabs File Information
                                     * Type: NextLabsFile_Header_t
                                     */
  SE_FileInfo_NextLabsEncryption,   /* NextLabs Encryption File Information
                                     * Type: NextLabsEncryptionFile_Header_t
                                     */
  SE_FileInfo_NextLabsTagging       /* NextLabs Tagging File Information
                                     * Type: NextLabsTaggingFile_Header_t
                                     */
} NextLabs_FileInfo_t;
//Return values for functions used by SE_EncryptFile and SE_DecryptFile.
typedef enum
{
  NLSE_ENC_ERROR_SUCCESS = 0,
  NLSE_ENC_ERROR_CANT_READ_INDICATOR,
  NLSE_ENC_ERROR_CANT_WRITE_INDICATOR,
  NLSE_ENC_ERROR_INDICATOR_CORRUPTED,
  NLSE_ENC_ERROR_UNSUPPORTED_FILETYPE,
  NLSE_ENC_ERROR_UNSUPPORTED_ENCRYPTION_FILETYPE,
  NLSE_ENC_ERROR_CANT_ACQUIRE_CRYPT_CONTEXT,
  NLSE_ENC_ERROR_CANT_RELEASE_CRYPT_CONTEXT,
  NLSE_ENC_ERROR_CANT_INIT_KEY,
  NLSE_ENC_ERROR_CANT_DESTROY_KEY,
  NLSE_ENC_ERROR_INPUT_FILE_ENCRYPTED,
  NLSE_ENC_ERROR_INPUT_FILE_NOT_ENCRYPTED,
  NLSE_ENC_ERROR_CANT_READ_INPUT_FILE,
  NLSE_ENC_ERROR_CANT_WRITE_OUTPUT_FILE,
  NLSE_ENC_ERROR_CANT_GENERATE_PADDING_BYTES,
  NLSE_ENC_ERROR_CANT_ENCRYPT_DATA,
  NLSE_ENC_ERROR_CANT_DECRYPT_DATA,
  NLSE_ENC_ERROR_CANT_INIT_CESDK,
  NLSE_ENC_ERROR_CANT_CONNECT_PC,
  NLSE_ENC_ERROR_CANT_GET_PC_KEY,
  NLSE_ENC_ERROR_CANT_GENERATE_DATA_KEY,
  NLSE_ENC_ERROR_CANT_ENCRYPT_DATA_KEY,
  NLSE_ENC_ERROR_CANT_DECRYPT_DATA_KEY,
  NLSE_ENC_ERROR_PLUGIN_COMM,
  NLSE_ENC_ERROR_INVALID_PASSWORD,
  NLSE_ENC_ERROR_NOT_ENOUGH_MEMORY,
  NLSE_ENC_ERROR_CANT_GET_SHARED_KEY,
  NLSE_ENC_ERROR_CANT_GET_LOCAL_KEY,
  NLSE_ENC_ERROR_INVALID_KEY_RING,
  NLSE_ENC_ERROR_INVALID_FEATURE_ENCRYPTION_SYSTEM,
  NLSE_ENC_ERROR_CANT_DELETE_INDICATOR
} nlse_enc_error_t;

/** SE_EncryptFile
 *
 *  \brief Encrypt a file using the current key.
 *
 *  \param path (in) Fully-qualified path to file which should be encrypted.
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return ERROR_NOT_SUPPORT
 *          when the target file system does not support System Encryption
 */
extern "C"
BOOL SE_EncryptFile( _In_z_ const wchar_t* path );

extern "C"
BOOL SE_EncryptFileForce( _In_z_ const wchar_t* path );

/** SE_DecryptFile
 *
 *  \brief Decrypt a file using the current key.
 *
 *  \param in_path (in) Path to file which should be decrypted.
 *  \param out_path (in) (optional) Path to decrypted file.
 *  \param password (in) Password that allows decryption.
 *
 *  \return TRUE on success, otherwise FALSE.
 */
extern "C"
BOOL SE_DecryptFile( _In_z_ const wchar_t* in_path,
                     _In_opt_z_ const wchar_t* out_path,
                     _In_z_ const wchar_t* password );

/** SE_EncryptDirectory
 *
 *  \brief Set encryption on a given directory.
 *
 *  \param path (in) Path to directory which should be encrypted.
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return ERROR_NOT_SUPPORT
 *          when the target file system does not support System Encryption
 */
extern "C"
BOOL SE_EncryptDirectory( _In_z_ const wchar_t* path );

/** SE_DecryptDirectory
 *
 *  \brief Clear encryption on a given directory.
 *
 *  \param path (in) Path to directory which should be decrypted.
 *  \param password (in) Password that allows decryption.
 *
 *  \return TRUE on success, otherwise FALSE.
 */
extern "C"
BOOL SE_DecryptDirectory( _In_z_ const wchar_t* path,
                          _In_z_ const wchar_t* password );

/** SE_IsWrappedFile
 *
 *  \brief Determine if the given file is a wrapped file.
 *
 *  \param path (in) Path to file to check.
 *
 *  \return TRUE if wrapped file, otherwise FALSE.  If SE_IsWrappedFile fails,
 *          GetLastError() will have a value other than ERROR_SUCCESS.
 *          It skips checking the path and returns error with
 *          ERROR_NOT_SUPPORTED when neither RMC nor RMS is installed.
 */
extern "C"
BOOL SE_IsWrappedFile( _In_z_ const wchar_t* path );

/** SE_IsNXLFile
 *
 *  \brief Determine if the given file is a NextLabs file.
 *
 *  \param path (in) Path to file to check.
 *
 *  \return TRUE if NextLabs file, otherwise FALSE.  If SE_IsNXLFile fails,
 *          GetLastError() will have a value other than ERROR_SUCCESS.
 *          It skips checking the path and returns error with
 *          ERROR_NOT_SUPPORTED when neither RMC nor RMS is installed.
 */
extern "C"
BOOL SE_IsNXLFile( _In_z_ const wchar_t* path );

/** SE_IsEncrypted
 *
 *  \brief Determine if the given object (file or directory) is encrypted.
 *
 *  \param path (in) Path to directory or file to check for encryption.
 *
 *  \return TRUE if encrypted, otherwise FALSE.  If SE_IsEncrypted fails,
 *          GetLastError() will have a value other than ERROR_SUCCESS.
 *          It skips checking the path and returns error with
 *          ERROR_NOT_SUPPORTED when neither RMC nor RMS is installed.
 */
extern "C"
BOOL SE_IsEncrypted( _In_z_ const wchar_t* path );

/** SE_IsEncryptedEx
 *
 *  \brief Determine if the given object (file or directory) is encrypted.
 *
 *  \param path (in) Path to directory or file to check for encryption.
 *  \param skip_if_rm_not_installed (in) TRUE if it should skip checking the
 *         path and return error with ERROR_NOT_SUPPORTED when neither RMC nor
 *         RMS is installed.
 *
 *  \return TRUE if encrypted, otherwise FALSE.  If SE_IsEncrypted fails,
 *          GetLastError() will have a value other than ERROR_SUCCESS.
 */
extern "C"
BOOL SE_IsEncryptedEx( _In_z_ const wchar_t* path,
                       _In_ BOOL skip_if_rm_not_installed );

/** SE_CreateFileRaw
 *
 *  \brief create or open a file in raw mode
 *
 *  \param fName The fully-qualified file specification of the file to be created or opened.
 *  \param nameLen Number of wide characters in fName, excluding null terminator.
 *  \param desiredAccess The requested access to the file.  (See CreateFile.)  FILE_READ_DATA is automatically set if FILE_WRITE_DATA or FILE_APPEND_DATA is passed.
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
extern "C" 
BOOL SE_CreateFileRaw(
    _In_z_count_(nameLen)    WCHAR *fName,
    _In_                    size_t nameLen,
    _In_                    ACCESS_MASK desiredAccess,
    _In_                    ULONG fileAttributes,
    _In_                    ULONG shareAccess,
    _In_                    ULONG createDisposition,
    _Deref_out_             PHANDLE pHandle);

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
extern "C" 
BOOL SE_ReadFileRaw(
    _In_                                    HANDLE handle,
    _In_                                    ULONGLONG offset,
    _In_                                    ULONG len,
    _Out_bytecap_post_bytecount_(bufSize, *bytesRead)  PVOID buf,
    _In_                                    ULONG bufSize,
    _Out_                                   PULONG bytesRead);

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
extern "C" 
BOOL SE_WriteFileRaw(
    _In_                HANDLE handle,
    _In_                ULONGLONG offset,
    _In_                ULONG len,
    _In_bytecount_(len)    PVOID buf,
    _In_                ULONG bufSize,
    _Out_               PULONG bytesWritten);

/** NLSEUserCmd_CloseFileRaw
 *
 *  \brief close a file opened in raw mode
 *
 *  \param handle A handle to the file.
 *
 *  \return TRUE on communication success.
 *  \return     To get extended error information, call GetLastError().
 *  \return FALSE on communication error.
 */
extern "C" 
BOOL SE_CloseFileRaw(
    _In_    HANDLE handle);

/** SE_GetFileInfo
 *
 *  \brief Read file information from a System Encryption file.
 *
 *  \param in_path (in)      Path to file which should be encrypted.
 *  \param in_file_info (in) File information type to read.
 *  \param info (out)        Buffer for info to read.
 *
 *  \return TRUE on success, otherwise FALSE.
 */
_Check_return_
extern "C"
BOOL SE_GetFileInfo(
    _In_z_                              const wchar_t* in_file,
    _In_                                NextLabs_FileInfo_t in_file_info ,
    _Out_bytecapcount_x_(in_file_info == SE_FileInfo_NextLabs ?
                      sizeof(NextLabsFile_Header_t) :
                      (in_file_info == SE_FileInfo_NextLabsEncryption ?
                       sizeof(NextLabsEncryptionFile_Header_t) :
                       sizeof(NextLabsTaggingFile_Header_t)))
                                        void* info);

/** SE_SetFileInfo
 *
 *  \brief Write file information to a System Encryption file.
 *
 *  \param in_path (in)      Path to file which should be encrypted.
 *  \param in_file_info (in) File information type to write.
 *  \param info (in)         Buffer for info to write.
 *
 *  \return TRUE on success, otherwise FALSE.
 */
extern "C"
BOOL SE_SetFileInfo(
    _In_z_                              const wchar_t* in_file,
    _In_                                NextLabs_FileInfo_t in_file_info ,
    _In_bytecount_x_(in_file_info == SE_FileInfo_NextLabs ?
                sizeof(NextLabsFile_Header_t) :
                (in_file_info == SE_FileInfo_NextLabsEncryption ?
                 sizeof(NextLabsEncryptionFile_Header_t) :
                 sizeof(NextLabsTaggingFile_Header_t)))
                                        const void* info);

/** SE_WrapPlainFile
 *
 *  \brief Wrap a plain file using a shared key.
 *
 *  \param in_path (in) Fully-qualified path to plain file.
 *  \param out_path (in) Fully-qualified path to the wrapped file.
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return 
 *          appropriate error code
 */
extern "C"
BOOL SE_WrapPlainFile( _In_z_ const wchar_t* in_path,
                       _In_z_ const wchar_t* out_path);

/** SE_UnwrapToPlainFile
 *
 *  \brief Unwrap a file to a plain file.
 *
 *  \param in_path (in) Fully-qualified path to the wrapped file.
 *  \param out_path (in) Fully-qualified path to the plain file.
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return 
 *          appropriate error code
 */
extern "C"
BOOL SE_UnwrapToPlainFile( _In_z_ const wchar_t* in_path,
                           _In_z_ const wchar_t* out_path);

/** SE_WrapEncryptedFile
 *
 *  \brief Wrap a local-encrypted file using a shared key.
 *
 *  \param in_path (in) Fully-qualified path to local-encrypted file.
 *  \param out_path (in) Fully-qualified path to the wrapped file (must have .nxl extension)
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return 
 *          appropriate error code
 */
extern "C"
BOOL SE_WrapEncryptedFile( _In_z_ const wchar_t* in_path,
                           _In_z_ const wchar_t* out_path);

/** SE_UnwrapToEncryptedFile
 *
 *  \brief Unwrap a file to a local-encrypted file.
 *
 *  \param in_path (in) Fully-qualified path to the wrapped file (must have .nxl extension)
 *  \param out_path (in) Fully-qualified path to the local-encrypted file.
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return 
 *          appropriate error code
 */
extern "C"
BOOL SE_UnwrapToEncryptedFile( _In_z_ const wchar_t* in_path,
                               _In_z_ const wchar_t* out_path,
                               _In_ BOOL switch_to_local_key);

/** SE_CopyEncryptedFile
 *
 *  \brief Copy a local-encrypted file in an end-to-end-encrypted fashion.
 *
 *  \param in_path (in) Fully-qualified path to src file
 *  \param out_path (in) Fully-qualified path to dest file
 *
 *  \return TRUE on success, otherwise FALSE.  GetLastError will return 
 *          appropriate error code
 */
extern "C"
BOOL SE_CopyEncryptedFile( _In_z_ const wchar_t* in_path,
                           _In_z_ const wchar_t* out_path);

/** SE_SetLastError
 *
 *  \brief Set the errorcode into TLS.
 *
 *  \param errorcode (in) error information
 *
 *  \if the set action isn't success then will write a error information to log
 */
extern "C"
void  SE_SetLastError(_In_ nlse_enc_error_t errorcode);

/** SE_GetLastError
 *
 *  \brief Get the errorcode from TLS.
 *
 *  \return appropriate error code
 */
extern "C"
nlse_enc_error_t SE_GetLastError( );

/** SE_DisplayErrorMessage
 *
 *  \brief According to the errorcode to popup a dialog to inform the user.
 *
 *  \param errorcode (in) error information
 *
 *  \according to the errorcode popup appropriate dialog
 */
extern "C"
void SE_DisplayErrorMessage(_In_ nlse_enc_error_t errorcode);

/** SE_PurgeDRMList
 *
 *  \brief Purge DRM list in SELib
 */
extern "C"
void SE_PurgeDRMList();

/** SE_MarkFileAsDRMOneShot
 *
 *  \brief Mark a file to be encrypted when it is created later by the
 *         current process, taking effect only one time.
 *
 *  \param path (in)    Full path to file.  This can be either a normal path
 *                      or a wildcard path.
 *                      Supported wildcard formats are:
 *                      - "*" (the path component is a "*")
 *                      - "*abc" (the path component starts with a "*")
 *                      Thus "C:\*Dir\barDir\*\*lo.txt" is supported and
 *                      matches "C:\fooDir\barDir\bazDir\hello.txt".
 *
 *  \return TRUE on success, otherwise FALSE.
 */
extern "C"
BOOL SE_MarkFileAsDRMOneShot( _In_z_ const wchar_t* path );

/** SE_UnmarkFileAsDRMOneShot
 *
 *  \brief Un-mark a file that was marked by SE_MarkFileAsDRMOneShot() for the
 *         current process.
 *
 *  \param path (in)    Full path to file.  This path must match exactly the
 *                      path that was passed to SE_MarkFileAsDRMOneShot().
 *                      Thus a normal or wildcard path must be passed
 *                      respectively in order to un-mark a normal or wildcard
 *                      path.
 *
 *  \return TRUE on success, otherwise FALSE.
 */
extern "C"
BOOL SE_UnmarkFileAsDRMOneShot( _In_z_ const wchar_t* path );

/** SE_UnmarkAllFilesAsDRMOneShot
 *
 *  \brief Un-mark all files that were marked by SE_MarkFileAsDRMOneShot() for
 *         the current process.
 *
 *  \return TRUE on success, otherwise FALSE.
 */
extern "C"
BOOL SE_UnmarkAllFilesAsDRMOneShot( void );


#endif /* __NLSE_LIB_H__ */
