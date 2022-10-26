#pragma once
#include <vector>

typedef struct struTagInfo
{
	std::wstring strTagName;
	std::vector<std::wstring>  vTagValues;
}TAGINFO, *LPTAGINFO;

typedef struct struItemInfo
{
	std::wstring strFilePath;
	std::wstring strFileName;
	std::wstring strTags;
	std::wstring strApplication;
	std::wstring strApplicationName;
	std::wstring strAction;
	std::vector<TAGINFO> vTags;
	std::wstring strOriCmdLine;
       bool bShowGUI;
	BOOL		 bNeedCheckWindow;
	BOOL		 bWndFound;//it depends on bNeedCheckWindow. it only works when bNeedCheckWindow is TRUE;
	std::vector<std::wstring> vWndTitle;
	std::vector<std::wstring> vAppTxt;

	std::wstring strPrompt;

	BOOL		bOptional;
	DWORD	dwEntryTime;//This is used as an identify.
}ITEMINFO, *LPITEMINFO;

class CAddTagsDlg;
class CAddTagsMgr
{
protected:
	CAddTagsMgr(void);
	~CAddTagsMgr(void);

	BOOL ParseTags(LPCWSTR lpszTags, OUT std::vector<TAGINFO>& vTags);
	BOOL ParseCmdLine(LPCWSTR lpszCmdLine, std::wstring& strTags, std::wstring& strPath, std::wstring& strApplication, std::wstring& strAction, 
                          std::wstring& strPrompt, bool& showGUI, bool& bOptional);
	BOOL Item_exists(LPCWSTR lpszCmdLine);
	BOOL Item_exists_PathTags(LPCWSTR lpszPath, LPCWSTR lpszTags);
	
public:
	//singleton
	static CAddTagsMgr* GetInstance()
	{
		static CAddTagsMgr mgr;
		return &mgr;
	}

	void AddItem(LPCWSTR pszCmdLine);
	void RemoveItem(LPCWSTR pszCmdLine, DWORD dwEntryTime);
	void SetCheckedFlag(LPCWSTR pszCmdLine, DWORD dwEntryTime, BOOL bFlag);

	void RemoveFirstShownItem();

	unsigned GetShownItem();
	BOOL NoMoreShownItems();
//	BOOL NoMoreItems();

	void StartWork();
	void EndWork();

	BOOL IsValid(LPCWSTR lpszCmdLine);

	void SetApplicationInstance(HINSTANCE hInstance) {m_hApplicationInstance = hInstance;}
	HINSTANCE GetApplicationInstance(){return m_hApplicationInstance;}

	void UpdateDialog();
public:
	std::list<ITEMINFO> m_listItems;//all the items
	std::list<ITEMINFO> m_listItems_Show;//all the items which is writable
	HANDLE m_hShowEvent;
	HANDLE m_hWorkingThread;
	BOOL m_bEnd;
protected:
	CAddTagsDlg* m_pDlg;
	HINSTANCE m_hApplicationInstance;
	
};

void WorkingThread( LPVOID pParam );
void ReadSharedMemoryThread( LPVOID pParam );

