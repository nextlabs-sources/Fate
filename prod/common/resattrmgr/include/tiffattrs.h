#ifndef _TIFF_ATTRS_H_
#define _TIFF_ATTRS_H_



BOOL IsTIFFFile(LPCWSTR pwzFile);

BOOL GetTIFFFileProps(const WCHAR *fileName, ResourceAttributes *attrs);
BOOL SetTIFFFileProps(const WCHAR *fileName, ResourceAttributes *attrs);
BOOL RemoveTIFFFileProps(const WCHAR *fileName, ResourceAttributes *attrs);

#endif