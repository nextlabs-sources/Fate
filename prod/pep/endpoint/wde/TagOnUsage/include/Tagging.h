#pragma once

class CPromptDlg;
typedef struct struTaggingParam
{
	std::wstring strFilePath;
	std::map<std::wstring, std::wstring> mapTags;
	CPromptDlg* pDlg;
	int	nResult;
}TAGGINGPARAM, *LPTAGGINGPARAM;


class CTagging
{
public:
	CTagging(void);
	~CTagging(void);

public:
	static BOOL AddTag(LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags, HWND hParentWnd = NULL);
	static BOOL GetAllTags(LPCWSTR lpszFilePath, OUT std::map<std::wstring, std::wstring>& mapTags);
	static void UpdateIndexTag(LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags);
	static BOOL Split_NXT_Tags(IN std::wstring strIndexTag, OUT std::vector<std::wstring>& listTags);
	static BOOL ReadTags(LPCWSTR lpszFilePath, OUT std::map<std::wstring, std::wstring>& mapTags, HWND hParentWnd = NULL);
};

void TaggingThread( LPVOID pParam );
