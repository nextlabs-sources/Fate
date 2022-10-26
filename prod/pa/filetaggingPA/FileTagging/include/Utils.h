#ifndef _FIELTAGGING_UTILS_H_
#define _FIELTAGGING_UTILS_H_

#ifdef _UNICODE
#define DP(x)   PrintLogW x
#define NLPRINT_DEBUGVIEWLOG( ... ) DP((__VA_ARGS__))
#else
#define DP(x)   PrintLogA x
#endif

extern BOOL g_bLoadByOE;
#define  FILETAGGING_DESCRIPTION_OB				L"Description"
#define  FILETAGGING_TARGET_OB					L"Target"
#define  FILETAGGING_TAGONERROR                 L"Decision on Tag Error"
#define  FILETAGGING_TAGONUNSUPPORTTYPE         L"Decision on Tag to Non-Supported Document"
#define  FILETAGGING_BLOCK_MESSAGE              L"Message for block reason"
#define  FILETAGGING_XML_FORMAT                 L"Classification Data(XML file format)"
#define  FILETAGGING_LOGID						L"LogId"


// For check last modify time
// Obligation name and value
#define OBCHECKMODIFIEDTIMENAME                 L"CheckModifiedTime"
#define OBCHECKMODIFIEDTIMEVALUE                L"True"

// Tag name
#define NLLASTMODIFYTIMETAGNAME                 L"NLLastModifyTime"

#define  ERROR_ACTION_CONTINUE  L"continue"
#define  ERROR_ACTION_BLOCK     L"block"

extern const DWORD g_kdwLastModifyTimeInterval;	// UTC time, unit: 100ns

void PrintLogA(const char* _Fmt, ...);
void PrintLogW(const WCHAR* _Fmt, ...);

BOOL CheckFileTaggingError(LPCWSTR pszFileName, HWND hParentWnd, DWORD& dwErrorID, int nType = 0);/*0: add, 1:remove, 2: read*/
void PopupMessageBox(DWORD dwStringID, LPCWSTR pszFileName, HWND hParentWnd);
BOOL NeedPopupPromptDlg(LPCWSTR pszFileName);
BOOL AddTagEx(std::list<smart_ptr<FILETAG_PAIR>>* pList, LPCWSTR pszFileName, CFileTagMgr* pMgr, LPCWSTR szErrorAction=NULL/*block or continue*/, LPCWSTR szMessageIfBlcok=NULL);
BOOL AddTagEx(LPCWSTR pszTagName, LPCWSTR pszTagValue, LPCWSTR pszFileName, CFileTagMgr* pMgr);
BOOL RemoveTagEx(std::list<std::wstring>* pList, LPCWSTR pszFileName, CFileTagMgr* pMgr);
BOOL GetAllTagsEx( LPCWSTR pszFileName, CFileTagMgr* pMgr, OUT std::list<smart_ptr<FILETAG_PAIR>>* pList);


std::wstring FormatCurrentTime();

BOOL Split_NXT_Tags(IN std::wstring strIndexTag, OUT std::list<std::wstring>& listTags);
BOOL GetIndexTag(std::list<smart_ptr<FILETAG_PAIR>>* pList, LPCWSTR pszFileName, OUT std::wstring& strValue);
BOOL TagExists(IN LPCWSTR pszTagName, IN std::list<std::wstring>& listTags, OUT std::list<std::wstring>::iterator& n_itr);

BOOL GetFileNameFromPath(LPCWSTR pszPath, std::wstring& strFileName);
BOOL CheckAndCreateRandTemp( std::wstring strFilePath, std::wstring &strTempFolder ) ;

std::wstring GetInstallPath(HMODULE hModule);

BOOL DoAutoTag(LPAUTOTHREADPARAM& pParam, HWND hParentWnd, LPCWSTR szErrorAction = NULL/*block or continue*/, LPCWSTR szMessageIfBlcok = NULL);

int ShowTagErrorBlockMessage(HWND hParent, LPCWSTR szFileName, LPCWSTR szMessage);

BOOL IsLoadByOE();

/////////////////////////For HSC last modify time begin/////////////////////////////////////////////////
// Only support dec now
std::wstring DwordToString(_In_ const DWORD kdwIn);

DWORD StringToDword(_In_ const std::wstring& kwstrIn);

bool GetFileTimeByType(_In_ const std::wstring& kwstrFileFullPath, _In_ const int knType, _Out_ FILETIME* pstuFileTime);

bool AreAllDigits(_In_ const std::wstring& kwstrIn);

bool ComvertStrTimeToFileTime(_In_ const std::wstring& kwstrFileTime, _Out_ FILETIME* pstuFileTime, _In_ const wchar_t kwchSep, _In_ const bool kbStrict);

std::wstring ComvertFileTimeToString(_In_ const FILETIME& kstuFileTime, _In_ const wchar_t kwchSep);

FILETIME NLGetAbsDiffFileTime(_In_ const FILETIME& kstuFirst, _In_ const FILETIME& kstuSecond);

void NLAddFileTime(_In_ FILETIME& kstuIn, _In_ const DWORD kdwIn);

bool NLIsUNCPath(_In_ const wstring& wstrFilePath);

// kwstrNLLastModifyTime: a string file time which record in the file by NEXTLABS HSC obligation, format is "high32-low32
// stuFileLastModifyTime: current file last modify time
bool IsTheFileModified(_In_ const std::wstring& kwstrNLLastModifyTime, _In_ const FILETIME& stuFileLastModifyTime, _In_ const DWORD knTimeInterval);

bool AddNLFileLastModifyTime(_Inout_ std::list<smart_ptr<FILETAG_PAIR> >& lstFileTag, _In_ std::wstring& wstrFilePath);

bool SetTagValueToTagPair(_Inout_ std::list<smart_ptr<FILETAG_PAIR> >& lstFileTag, _In_ const smart_ptr<FILETAG_PAIR>& spPairTagVlaue);

std::wstring GetTagValueFromTagPair(_In_ const std::list<smart_ptr<FILETAG_PAIR> >& lstFileTag, _In_ const std::wstring& kwstrTagName);
/////////////////////////For HSC last modify time end/////////////////////////////////////////////////

#endif
