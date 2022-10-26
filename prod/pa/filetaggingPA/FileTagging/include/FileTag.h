#ifndef _FILE_TAG_H_
#define _FILE_TAG_H_

#include "datatype.h"
class CFileTagMgr;
class CFileTag
{
public:
	CFileTag();

public:
	DWORD AddTag(LPCWSTR pszTagName, LPCWSTR pszTagValue, LPCWSTR pszFile);
	DWORD AddTag(std::list<smart_ptr<FILETAG>>* pListTags, LPCWSTR pszFileName);
	DWORD AddTag(std::list<smart_ptr<FILETAG_PAIR>>* pListTags, LPCWSTR pszFileName);

	DWORD RemoveTag(LPCWSTR pszTagName, LPCWSTR pszFile);
	DWORD RemoveTag(std::list<std::wstring>* pListTags, LPCWSTR pszFileName);

	DWORD GetTagValueByName(LPCWSTR pszTagName, LPCWSTR pszFile, OUT std::wstring& strTagValue);
	DWORD GetAllTags(LPCWSTR pszFile, std::list<smart_ptr<FILETAG_PAIR>>* pList);

	BOOL UpdateIndexTag(std::list<smart_ptr<FILETAG_PAIR>>* pListTags, LPCWSTR pszFileName, BOOL bAdd = TRUE);
	BOOL UpdateIndexTag(std::list<std::wstring>* pListTags, LPCWSTR pszFileName, BOOL bAdd = TRUE);

	void SetParentWnd(HWND hWnd){m_hWnd = hWnd;}
	HWND GetParentWnd(){return m_hWnd;};
	void SetFileTagMgr(CFileTagMgr* pMgr){m_pMgr = pMgr;};
protected:
	HWND m_hWnd;
	CFileTagMgr* m_pMgr;
};

#endif