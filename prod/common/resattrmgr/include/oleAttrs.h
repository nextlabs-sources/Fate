#ifndef _OLE_ATTRS_H_
#define _OLE_ATTRS_H_

#include "resattrlib.h"



BOOL IsOLEFile(LPCWSTR pszFileName);

HRESULT GetOLEFileProps(const WCHAR *filename, ResourceAttributes *attrs);
HRESULT SetOLEFileProps(const WCHAR *filename, ResourceAttributes *attrs);
HRESULT RemoveOLEFileProps(const WCHAR *filename, ResourceAttributes *attrs);
HRESULT GetOLEFileSummaryProps(const WCHAR *filename, ResourceAttributes *attrs);



#endif
