#ifndef FTAG_H
#define FTAG_H
BOOL IsPDFFile(LPCWSTR pwzFile);
void GetPDFFileProps(const WCHAR *fileName, std::map<std::wstring, std::wstring> &results);
void SetPDFFileProp(const WCHAR *fileName, const WCHAR *key, const WCHAR *value);
#endif
