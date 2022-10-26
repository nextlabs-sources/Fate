#ifndef _NXL_ATTRS_H_
#define _NXL_ATTRS_H_

#include "resattrlib.h"



/** IsNXLFile
 *
 *  \brief Check if the file is a NextLabs file (i.e. either local-encrypted
 *         file or wrapped file.)
 *
 *  \param filename (in)        Path for the file
 *
 *  \return true if file is NextLabs file.
 */
bool IsNXLFile(const wchar_t *fileName);

/** GetNXLFileProps
 *
 *  \brief Get the file attributes from the NextLabs file.
 *
 *  \param filename (in)        Path for the file.
 *  \param attrs (inout)        Attribute storage (may be non-empty) for
 *                              returning attributes from file.  Any existing
 *                              attributes in the storage are preserved.
 *
 *  \return zero if success.
 */
int GetNXLFileProps(const wchar_t *filename, ResourceAttributes *attrs);

/** SetNXLFileProps
 *
 *  \brief Write the file attributes to the NextLabs file.
 *
 *  \param filename (in)        Path for the file
 *  \param attrs (in)           Attributes to write to file
 *
 *  \return zero if success.
 */
int SetNXLFileProps(const wchar_t *filename, const ResourceAttributes *attrs);

/** SetNXLFileProps
 *
 *  \brief Remove the file attributes from the NextLabs file.
 *
 *  \param filename (in)        Path for the file
 *  \param attrs (in)           Attributes to write to file
 *
 *  \return zero if success.
 */
int RemoveNXLFileProps(const wchar_t *filename, const ResourceAttributes *attrs);

/** IsNXL10File
 *
 *  \brief Check if the file is a NextLabs 1.0 file (i.e. heavy-write
 *         local-encrypted file.)
 *
 *  \param filename (in)        Path for the file
 *
 *  \return true if file is NextLabs 1.0 file.
 */
bool IsNXL10File(const wchar_t *fileName);

/** GetNXL10FileProps
 *
 *  \brief Get the file attributes from the NextLabs 1.0 file.
 *
 *  \param filename (in)        Path for the file.
 *  \param attrs (inout)        Attribute storage (may be non-empty) for
 *                              returning attributes from file.  Any existing
 *                              attributes in the storage are preserved.
 *
 *  \return zero if success.
 */
int GetNXL10FileProps(const wchar_t *filename, ResourceAttributes *attrs);



#endif
