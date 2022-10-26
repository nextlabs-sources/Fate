#ifndef _UTILS_H_
#define _UTILS_H_

#define ADOBE_ACROBAT_STANDARD			L"Adobe Acrobat Standard - "
#define MAX_LEN_SHAREDMEMORY		409600

BOOL IsWritable(LPCWSTR lpszFilePath);

//shared memory 
BOOL CreateSharedMemory();
void CloseSharedMemory();


BOOL WriteSharedMemory(LPCWSTR lpszInfo, DWORD dwLen);
BOOL ReadSharedMemory(LPWSTR lpszInfo, DWORD dwLen);

BOOL IsCreateBehaviorForExplorer(LPCWSTR lpszFilePath);

BOOL ExistsInROT(LPCWSTR pszFilePath );

BOOL IsOfficeFile(LPCWSTR pszFilePath);

BOOL NeedCheckWnd(LPCWSTR pszAppName, LPCWSTR pszFileName);
BOOL GetRelatedWndTxt(LPCWSTR pszFileName, LPCWSTR pszAppName, std::vector<std::wstring>& vWndTitle, std::vector<std::wstring>& vAppTxt);

BOOL GetOpenSaveAsWnd();

BOOL CanTag(LPCWSTR pszFileName, DWORD& dwErrorID);

void PopupErrorDlg(HWND hParentWnd, HINSTANCE hInstance, DWORD dwErrorID);

std::wstring GetProbableFileName(LPCWSTR lpszFileName);

BOOL IsTemplateFile(LPCWSTR lpszFileName);

BOOL NeedProcessDlg(LPCWSTR lpszFilePath);

BOOL IsReadOnlyFile(LPCWSTR pszFileName);

BOOL IsCATIAFileTypes(LPCWSTR pszFileName);

bool IsSameFile(LPCWSTR szPath1, LPCWSTR szPath2) ;

BOOL InitLog();
#endif