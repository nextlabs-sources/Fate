#ifndef _NTFS_ATTRS_H_
#define _NTFS_ATTRS_H_

#include "resattrlib.h"



void InitializeNTFSStreamFunctions(void);

BOOL IsNTFSFile(LPCWSTR pszFileName);

BOOL GetNTFSFileProps(const WCHAR *filename, ResourceAttributes *attrs);
BOOL SetNTFSFileProps(const WCHAR *filename, ResourceAttributes *attrs);
BOOL RemoveNTFSFileProps(const WCHAR *filename, ResourceAttributes *attrs);



#endif
