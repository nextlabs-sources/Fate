#ifndef _OFFICE2K7_H_
#define _OFFICE2K7_H_

#include "resattrlib.h"



void LoadOffice2k7Dll();
void FreeOffice2k7Dll();

int GetO2K7FileProps(const wchar_t* pszFileName, ResourceAttributes *attrs);
int SetO2K7FileProps(const wchar_t* pszFileName, ResourceAttributes *attrs);
int RemoveO2K7FileProps(const wchar_t* pszFileName, ResourceAttributes *attrs);
int GetO2K7FileSummaryProps(const wchar_t* pszFileName, ResourceAttributes *attrs);

BOOL IsOffice2k7FileType(const wchar_t* pszFileName);

#endif