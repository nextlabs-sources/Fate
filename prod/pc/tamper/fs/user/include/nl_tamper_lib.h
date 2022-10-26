
#ifndef __NL_TAMPER_LIB_H__
#define __NL_TAMPER_LIB_H__

#include <windows.h>
#include <tchar.h>

/** NLTamper_ProtectFile
 *
 *  \brief Protect a given file from access for all but the current process.
 *
 *  \param file (in)  File path to protect.
 *  \param flags (in) Flags to apply to action evaluation.
 *  \return Non-zero value on success.
 */
int NLTamper_ProtectFile( const WCHAR* file , int flags );

/** NLTamper_ExemptProcessName
 *
 *  \brief Exempt a process name from tamper proofing.
 *
 *  \param pname (in) Full path of the image file.
 *  \return Non-zero value on success.
 */
int NLTamper_ExemptProcessName( const WCHAR* pname );

/** NLTamper_ExemptProcessName
 *
 *  \brief Exempt a process name from tamper proofing on a protected file.
 *
 *  \param pname (in) Full path of the image file.
 *  \param fname (in)  File path to protect. The file has to be set to be protected by NLTamper_ProtectFile
 *  \return Non-zero value on success.
 */
int NLTamper_FileExemptProcessName( const WCHAR* pname, const WCHAR* fname );

/** NLTamper_ExemptProcessId
 *
 *  \brief Ignore operations from a given process ID.
 *
 *  \param pid (in) Process ID to ignore (exempt).
 *  \return Non-zero value on success.
 */
int NLTamper_ExemptProcessId( ULONG pid );

/** NLTamper_TranslatePath
 *
 *  Translate between a letter based drive path and its fully qualified
 *  volume path name.
 *
 *  Example:  \Device\HarddiskVolume1 -> C:
 *            C: -> \Device\HarddiskVolume1
 *
 *  Calling this function twice will yield the first path given to it.
 *  The path will be translated either way depending on the root of the
 *  path.
 *
 *  Returns the length in characters of the new path or zero on failure.
 */
int NLTamper_TranslatePath( WCHAR* path , int path_size );

#endif /* __NL_TAMPER_LIB_H__ */
