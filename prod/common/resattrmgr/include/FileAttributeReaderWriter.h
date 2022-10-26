#ifndef CEFILEATTRIBUTEREADERWRITER_H
#define CEFILEATTRIBUTEREADERWRITER_H
#include "resattrlib.h"
#include "resattrmgr.h"

namespace GenericNextLabsTagging {
    BOOL TagExists(ResourceAttributes *attrs, LPCWSTR pszTagName, BOOL bCaseSensitive = FALSE);
    void AddKeyValueHelperA(ResourceAttributes *attrs, const char *key, const char *value);
    void AddKeyValueHelperW(ResourceAttributes *attrs, const WCHAR *key, const WCHAR *value);
    BOOL GetFileCustomAttributes(const WCHAR *name, ResourceAttributes *attrs, TagType);
    BOOL SetFileCustomAttributes(const WCHAR *name, ResourceAttributes *attrs, TagType);
    BOOL RemoveFileCustomAttributes(const WCHAR *name, ResourceAttributes *attrs);
}
#endif
