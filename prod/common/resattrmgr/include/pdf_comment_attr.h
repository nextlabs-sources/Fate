#ifndef __PDF_COMMENT_ATTR_H__
#define __PDF_COMMENT_ATTR_H__

#include "resattrmgr.h"

#define PDFTAG_COMMNET_ATTR_LINE3_LEN 32
#define PDFTAG_COMMENT_ATTR_LINE1_HEADER "%%NLTAGVER="

#define PDFTAG_COMMENT_ATTR_VERSION_STR		"1.0"
#define PDFTAG_COMMENT_ATTR_LINE2_HEADER "%%NLTAG="
#define PDFTAG_COMMENT_ATTR_LINE3_HEADER "%%NLTAGLEN="

void InitializePDFTaggingMethod();
BOOL IsTagWithPoDoFo();

BOOL GetPDFFilePropsWithCommentAttr(const WCHAR *fileName, ResourceAttributes *attrs);
BOOL RemovePDFFilePropsWithCommentAttr(const WCHAR* fileName, ResourceAttributes *attrs);

#ifdef __cplusplus
extern "C" {
#endif

    // Why is this the only function exported?
RESATTRMGR_EXPORT BOOL SetPDFFilePropsWithCommentAttr(const WCHAR *fileName, ResourceAttributes *attrs);

#ifdef __cplusplus
}
#endif


#endif //__PDF_COMMENT_ATTR_H__
