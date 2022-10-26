#ifndef CEPDFATTRS_H
#define CEPDFATTRS_H

#include "resattrmgr.h"
#include "resattrlib.h"
#ifndef TAGLIB_WINDOWS
#include "utils.h"
#include <errno.h>
#endif

#ifdef TAGLIB_WINDOWS
#define TAGCHAR wchar_t
#else
#define TAGCHAR char
#endif

#ifdef __cplusplus
extern "C" {
#endif

RESATTRMGR_EXPORT BOOL IsPDFFile(const TAGCHAR * pwzFile);
RESATTRMGR_EXPORT BOOL GetPDFFileProps(const TAGCHAR * fileName, ResourceAttributes *attrs);
RESATTRMGR_EXPORT BOOL SetPDFFileProps(const TAGCHAR * fileName, ResourceAttributes *attrs);
RESATTRMGR_EXPORT BOOL RemovePDFFileProps(const TAGCHAR * fileName, ResourceAttributes *attrs);
RESATTRMGR_EXPORT BOOL GetPDFSummaryProps(const TAGCHAR * fileName, ResourceAttributes *attrs);

#ifdef __cplusplus
}
#endif

#endif
