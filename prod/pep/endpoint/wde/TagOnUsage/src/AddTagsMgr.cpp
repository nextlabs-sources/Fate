#include "StdAfx.h"
#include "AddTagsMgr.h"
#include "AddTagsDlg.h"
#include "Tagging.h"

#define TAG_SEPARATOR			L"|"
#define TAG_NAME_SEPARATOR		L"="
#define VALUE_SEPARATOR			L";"

#define PARAM_TAG_NAME_SEPARATOR		L"Tag Name"
#define PARAM_TAG_VALUE_SEPARATOR		L"Tag Value"
#define PARAM_OPTIONAL_SEPARATOR		L"Optional"
#define PARAM_PATH_SEPARATOR			L"-file"
#define PARAM_APPLICATION_SEPARATOR		L"-app"
#define PARAM_ACTION_SEPARATOR			L"-action"
#define PARAM_OPENPROMPT_SEPARATOR		L"-promptOnOpen"
#define PARAM_DESCRIPTION_SEPARATOR		L"Description"
#define PARAM_AUTO_SEPARATOR            L"-auto"
#define DELAY_TIME						5000	

using namespace std;

BOOL g_bFinded = FALSE;
BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam)
{
	wchar_t buffer[MAX_PATH * 2 + 1] = {0};
	GetWindowText(hwndChild, buffer, MAX_PATH * 2);

	if(wcslen(buffer) == 0)
	{
		g_bFinded = FALSE;
		return TRUE;
	}

	wchar_t* p = (wchar_t*)lParam;//file name.
	if(!p)
		return TRUE;

	if(IsTemplateFile(p))
	{
// 		wchar_t aa[1000] = {0};
// 		swprintf_s(aa, 1000, L"EnumChildWindow, '%s', '%s',", buffer, p);
// 		OutputDebugStringW(aa);
		std::wstring strProbableName = GetProbableFileName(p);
		if(_wcsicmp(strProbableName.c_str(), p) != 0)
		{
			if(wcslen(buffer) >= strProbableName.length() && _memicmp(buffer, strProbableName.c_str(), strProbableName.length() * sizeof(wchar_t)) == 0)
			{
				g_bFinded = TRUE;
				return FALSE;
			}
		}
	}

	//Try to compare the full file name
	if(_wcsicmp(buffer, p) == 0)
	{
		g_bFinded = TRUE;
		return FALSE;
	}
	else
	{
		std::wstring strFileName2(p);
		strFileName2 += L" ";
		if(wcslen(buffer) >= strFileName2.length() && _memicmp(buffer, strFileName2.c_str(), strFileName2.length() * sizeof(wchar_t)) == 0)
		{
			g_bFinded = TRUE;
			return FALSE;
		}
	}

	//try to compare the file name without extension name
	const wchar_t* ext = wcsrchr(p, '.');
	if(ext)
	{
		std::wstring strFileNameWithoutExt(p);
		strFileNameWithoutExt = strFileNameWithoutExt.substr(0, strFileNameWithoutExt.rfind('.'));

		if(_wcsicmp(buffer, strFileNameWithoutExt.c_str()) == 0)
		{
			g_bFinded = TRUE;
			return FALSE;
		}
		else
		{
			std::wstring strFileName2 = strFileNameWithoutExt;
			strFileName2 += L" ";
			if(wcslen(buffer) >= strFileName2.length() && _memicmp(buffer, strFileName2.c_str(), strFileName2.length() * sizeof(wchar_t)) == 0)
			{
				g_bFinded = TRUE;
				return FALSE;
			}
		}
	}

	return TRUE;
}


BOOL g_bFindedTemp = FALSE;
BOOL CALLBACK EnumWindowsProc( HWND hwnd, LPARAM lParam)
{
	wchar_t buffer[MAX_PATH * 2 + 1] = {0};
	GetWindowText(hwnd, buffer, MAX_PATH * 2);

	if(wcslen(buffer) == 0)
	{
		g_bFindedTemp = FALSE;
		return TRUE;
	}

	ITEMINFO* pInfo = (ITEMINFO*)lParam;

	if(pInfo)
	{
		std::wstring strCaption(buffer);
		std::transform(strCaption.begin(), strCaption.end(), strCaption.begin(), towlower);

		//Filter some windows.
		if(_wcsicmp(pInfo->strApplicationName.c_str(), L"acad.exe") == 0)
		{
			if(strCaption.find(L"text window") != std::wstring::npos)
			{
				g_bFindedTemp = FALSE;
				return TRUE;
			}
		}

		//Try to find the window with whole caption
		std::vector<std::wstring>::iterator itr;
		for(itr = pInfo->vWndTitle.begin(); itr != pInfo->vWndTitle.end(); itr++)
		{
			if(_wcsicmp(strCaption.c_str(), (*itr).c_str()) == 0)
			{
				g_bFindedTemp = TRUE;
				return FALSE;
			}
		}

		//Try to find the "application text" part.
		BOOL bFind = FALSE;
		for(itr = pInfo->vAppTxt.begin(); itr != pInfo->vAppTxt.end(); itr++)
		{
			std::wstring strAppTxt = *itr;
			std::transform(strAppTxt.begin(), strAppTxt.end(), strAppTxt.begin(), towlower);

			if( strCaption.find(strAppTxt) != std::wstring::npos)
			{
				bFind = TRUE;
				break;
			}
		}
		if(!bFind)
		{
			g_bFindedTemp = FALSE;
			return TRUE;
		}

		wstring strFilePath = pInfo->strFileName;
		std::transform(strFilePath.begin(), strFilePath.end(), strFilePath.begin(), towlower);
		if (strFilePath.rfind(L".pot") == strFilePath.length() - 4)
		{
			g_bFindedTemp = TRUE;
			return FALSE;
		}

		//Try to find the related windows in the child windows
		g_bFinded = FALSE;
		EnumChildWindows(hwnd, EnumChildProc, (LPARAM)pInfo->strFileName.c_str());
		if(g_bFinded)
		{
			g_bFindedTemp = TRUE;
			return FALSE;
		}
		
	}

	return TRUE;
}

BOOL CheckWindowWithFileName(ITEMINFO itemInfo)
{
	g_bFindedTemp = FALSE;
	EnumWindows(EnumWindowsProc, (LPARAM)&itemInfo);
	if(g_bFindedTemp)
	{
		return TRUE;
	}		
	
	return FALSE;
}

CAddTagsMgr::CAddTagsMgr(void)
{
	m_hApplicationInstance = NULL;
	m_pDlg = NULL;
	m_bEnd = FALSE;
	m_hShowEvent = CreateEvent( 
							NULL,               // default security attributes
							TRUE,               // manual-reset event
							FALSE,               // initial state is signaled
							NULL  // object name
							); 
}

CAddTagsMgr::~CAddTagsMgr(void)
{
	if(m_hShowEvent)
	{
		CloseHandle(m_hShowEvent);
		m_hShowEvent = NULL;
	}

	if(m_pDlg)
	{
		delete m_pDlg;
		m_pDlg = NULL;
	}
}

/***********************************command line*******************************************************************************************
Interactive:
wdeaddTags.exe Tag Name company Tag Value IBM;MS Optional No Description "The following file needs to be tagged:" -file "c:/kaka/1.doc" -app "C:\WINDOWS\winword.EXE" -action "OPEN"

Automatic:
wdeaddTags.exe Tag Name class Tag Value public; Optional No Description "The following file needs to be tagged:" -file "c:/kaka/1.doc" -app "C:\WINDOWS\winword.EXE" -action "OPEN" -auto yes
********************************************************************************************************************************************/
BOOL CAddTagsMgr::ParseCmdLine(LPCWSTR lpszCmdLine, std::wstring& strTags, std::wstring& strPath, std::wstring& strApplication, 
							   std::wstring& strAction, std::wstring& strPrompt, bool &showGUI, bool& bOptional)
{
	if(!lpszCmdLine)
	{
		return FALSE;
	}

	std::wstring strCmdLine(lpszCmdLine);
	std::wstring::size_type uTagNameIndex = strCmdLine.find(PARAM_TAG_NAME_SEPARATOR);
	std::wstring::size_type uTagValueIndex = strCmdLine.find(PARAM_TAG_VALUE_SEPARATOR);
	std::wstring::size_type uOptionalIndex = strCmdLine.find(PARAM_OPTIONAL_SEPARATOR);
	std::wstring::size_type uDescriptionIndex = strCmdLine.find(PARAM_DESCRIPTION_SEPARATOR);
	std::wstring::size_type uFilePathIndex = strCmdLine.find(PARAM_PATH_SEPARATOR);
	std::wstring::size_type uAppIndex = strCmdLine.find(PARAM_APPLICATION_SEPARATOR);
	std::wstring::size_type uActionIndex = strCmdLine.find(PARAM_ACTION_SEPARATOR);
	std::wstring::size_type uAutoIndex = strCmdLine.find(PARAM_AUTO_SEPARATOR);

	if(uTagNameIndex == wstring::npos || uTagValueIndex == wstring::npos || uFilePathIndex == wstring::npos || uAppIndex == wstring::npos || uAppIndex == wstring::npos)
	{
		return FALSE;
	}

	BOOL bAuto = FALSE;
	if(uAutoIndex != wstring::npos)
	{
		bAuto = TRUE;
	}

	showGUI = !bAuto;

	wstring strTagName;
	wstring strTagValue;
	wstring strOptional = L"no";
	if(!bAuto)//Interactive tagging
	{
		if( !(uTagValueIndex > uTagNameIndex && uOptionalIndex > uTagValueIndex && uDescriptionIndex > uOptionalIndex 
			&& uFilePathIndex > uDescriptionIndex && uAppIndex > uFilePathIndex && uActionIndex > uAppIndex) )
		{
			g_log.Log(CELOG_DEBUG, L"The command line of interactive tagging is not correct. %s\r\n", strCmdLine.c_str());
			return FALSE;
		}

		strTagName = strCmdLine.substr(uTagNameIndex + wcslen(PARAM_TAG_NAME_SEPARATOR) + 1, uTagValueIndex - uTagNameIndex - wcslen(PARAM_TAG_NAME_SEPARATOR) - 2);
		strTagValue = strCmdLine.substr(uTagValueIndex + wcslen(PARAM_TAG_VALUE_SEPARATOR) + 1, uOptionalIndex - uTagValueIndex - wcslen(PARAM_TAG_VALUE_SEPARATOR) - 2);
		strOptional = strCmdLine.substr(uOptionalIndex + wcslen(PARAM_OPTIONAL_SEPARATOR) + 1, uDescriptionIndex - uOptionalIndex - wcslen(PARAM_OPTIONAL_SEPARATOR) - 2);
		strPrompt = strCmdLine.substr(uDescriptionIndex + wcslen(PARAM_DESCRIPTION_SEPARATOR) + 1, uFilePathIndex - uDescriptionIndex - wcslen(PARAM_DESCRIPTION_SEPARATOR) - 2);
		strPath = strCmdLine.substr(uFilePathIndex + wcslen(PARAM_PATH_SEPARATOR) + 1, uAppIndex - uFilePathIndex - wcslen(PARAM_PATH_SEPARATOR) - 2);
		strApplication = strCmdLine.substr(uAppIndex + wcslen(PARAM_APPLICATION_SEPARATOR) + 1, uActionIndex - uAppIndex - wcslen(PARAM_APPLICATION_SEPARATOR) - 2);
		strAction = strCmdLine.substr(uActionIndex + wcslen(PARAM_ACTION_SEPARATOR) + 1, strCmdLine.length() - uActionIndex - wcslen(PARAM_ACTION_SEPARATOR) - 1);
	}
	else//automatic tagonusage
	{
		if( !(uTagValueIndex > uTagNameIndex && uDescriptionIndex > uTagValueIndex && uFilePathIndex > uDescriptionIndex 
			&& uAppIndex > uFilePathIndex && uActionIndex > uAppIndex && uAutoIndex > uActionIndex) )
		{
			g_log.Log(CELOG_DEBUG, L"The command line of automatic tagging is not correct. %s\r\n", strCmdLine.c_str());
			return FALSE;
		}

		strTagName = strCmdLine.substr(uTagNameIndex + wcslen(PARAM_TAG_NAME_SEPARATOR) + 1, uTagValueIndex - uTagNameIndex - wcslen(PARAM_TAG_NAME_SEPARATOR) - 2);
		strTagValue = strCmdLine.substr(uTagValueIndex + wcslen(PARAM_TAG_VALUE_SEPARATOR) + 1, uDescriptionIndex - uTagValueIndex - wcslen(PARAM_TAG_VALUE_SEPARATOR) - 2);
		strPrompt = strCmdLine.substr(uDescriptionIndex + wcslen(PARAM_DESCRIPTION_SEPARATOR) + 1, uFilePathIndex - uDescriptionIndex - wcslen(PARAM_DESCRIPTION_SEPARATOR) - 2);
		strPath = strCmdLine.substr(uFilePathIndex + wcslen(PARAM_PATH_SEPARATOR) + 1, uAppIndex - uFilePathIndex - wcslen(PARAM_PATH_SEPARATOR) - 2);
		strApplication = strCmdLine.substr(uAppIndex + wcslen(PARAM_APPLICATION_SEPARATOR) + 1, uActionIndex - uAppIndex - wcslen(PARAM_APPLICATION_SEPARATOR) - 2);
		strAction = strCmdLine.substr(uActionIndex + wcslen(PARAM_ACTION_SEPARATOR) + 1, uAutoIndex - uActionIndex - wcslen(PARAM_ACTION_SEPARATOR) - 2);
	}

	if(_wcsicmp(strOptional.c_str(), L"yes") == 0)
	{
		bOptional = true;
	}

	if(strPath.find(L"\"") != std::wstring::npos)//remove "" for path
	{
		strPath = strPath.substr(1, strPath.length() - 2);
	}

	for(unsigned i = 0; i < strPath.length(); i++)
	{
		if(strPath[i] == '/')
		{
			strPath[i] = '\\';
		}
	}

	if(strAction[0] == '"' && strAction[strAction.length() - 1] == '"')//remove "" for action
	{
		strAction = strAction.substr(1, strAction.length() - 2);
	}

	if(strApplication.find(L"\"") != std::wstring::npos)//remove "" for application
	{
		strApplication = strApplication.substr(1, strApplication.length() - 2);
	}

	strTags = strTagName + L"=" + strTagValue;

	return TRUE;
}

BOOL CAddTagsMgr::Item_exists(LPCWSTR lpszCmdLine)
{
	if(!lpszCmdLine)
	{
		return FALSE;
	}

	::EnterCriticalSection(&CCriSectionMgr::m_CriItemList);
	std::list<ITEMINFO>::iterator itr;
	BOOL bExists = FALSE;
	for(itr = m_listItems.begin(); itr != m_listItems.end(); itr++)
	{
		if(_wcsicmp((*itr).strOriCmdLine.c_str(), lpszCmdLine) == 0)//exists already
		{
			bExists = TRUE;
			break;
		}
	}
	::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList);

	::EnterCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);
	if(!bExists)
	{
		for(itr = m_listItems_Show.begin(); itr != m_listItems_Show.end(); itr++)
		{
			if(_wcsicmp((*itr).strOriCmdLine.c_str(), lpszCmdLine) == 0)//exists already
			{
				bExists = TRUE;
				break;
			}
		}
	}
	::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);
	

	return bExists;
}

BOOL CAddTagsMgr::Item_exists_PathTags(LPCWSTR lpszPath, LPCWSTR lpszTags)
{
	if(!lpszTags || !lpszPath)
	{
		return FALSE;
	}

	::EnterCriticalSection(&CCriSectionMgr::m_CriItemList);
	std::list<ITEMINFO>::iterator itr;
	BOOL bExists = FALSE;
	for(itr = m_listItems.begin(); itr != m_listItems.end(); itr++)
	{
		if( (_wcsicmp((*itr).strFilePath.c_str(), lpszPath) == 0 || IsSameFile((*itr).strFilePath.c_str(), lpszPath))
			&& (_wcsicmp((*itr).strTags.c_str(), lpszTags) == 0 || (*itr).strTags.find(lpszTags) != wstring::npos))//exists already
		{
			bExists = TRUE;
			break;
		}
	}
	::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList);

	::EnterCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);
	if(!bExists)
	{
		for(itr = m_listItems_Show.begin(); itr != m_listItems_Show.end(); itr++)
		{
			if((_wcsicmp((*itr).strFilePath.c_str(), lpszPath) == 0 || IsSameFile((*itr).strFilePath.c_str(), lpszPath) ) 
				&& (_wcsicmp((*itr).strTags.c_str(), lpszTags) == 0 || (*itr).strTags.find(lpszTags) != wstring::npos))//exists already
			{
				bExists = TRUE;
				break;
			}
		}
	}
	::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);
	

	return bExists;
}

BOOL CAddTagsMgr::IsValid(LPCWSTR lpszCmdLine)
{
	if(!lpszCmdLine)
	{
		return FALSE;
	}

	std::wstring strTags;
	std::wstring strPath;
	std::wstring strApplication;
	std::wstring strAction;
	std::wstring strPrompt;
    bool showGUI;
	bool bOptional = false;

	if(!ParseCmdLine(lpszCmdLine, strTags, strPath, strApplication, strAction, strPrompt, showGUI, bOptional))
	{
		return FALSE;
	}
	
	if(strPath.find(L"*") != std::wstring::npos)
	{
		return FALSE;
	}

	if(IsReadOnlyFile(strPath.c_str()))//We don't care read-only file
	{
		return FALSE;
	}

	if(Item_exists_PathTags(strPath.c_str(), strTags.c_str()))
	{
		return FALSE;
	}

	const wchar_t* pApplication = wcsrchr(strApplication.c_str(), '\\');
	if(pApplication)
	{
		if( _wcsicmp(pApplication + 1, L"Explorer.exe") == 0 && _wcsicmp(strAction.c_str(), L"OPEN") == 0 && !IsCATIAFileTypes(strPath.c_str()))
		{
			return FALSE;
		}

		if( _wcsicmp(pApplication + 1, L"Explorer.exe") == 0 && _wcsicmp(strAction.c_str(), L"EDIT") == 0)
		{
			/***********************************************
			We only handle the files which modified time is 
			close to current time for Explorer.
			***********************************************/
			if( !IsCreateBehaviorForExplorer(strPath.c_str()) )
			{
				g_log.Log(CELOG_DEBUG, L"WdeAddTags, ignored for \"CREATE\" because of the different time zone, %s\n", strPath.c_str());
				return FALSE;
			}
		}
	}

// 	if(_wcsicmp(strAction.c_str(), L"OPEN") == 0)
// 	{
// 		if(GetOpenSaveAsWnd())
// 		{
// 			return FALSE;
// 		}
// 	}
	
	return TRUE;
}

/************************************************************************************
function name: AddItem
feature: Parse the "tag" parameter from command line and add a new 
item to the list.
command line format:
addTags.exe -m Tag1Name=Tag1Value1,Tag1Value2,Tag1Value3;Tag2Name=Tag2Value1 -f c:\foo.doc
************************************************************************************/
void CAddTagsMgr::AddItem(LPCWSTR pszCmdLine)
{
	if(!pszCmdLine)
	{
		return;
	}

	std::wstring strTags;
	std::wstring strPath;
	std::wstring strApplication;
	std::wstring strAction;
	std::wstring strPrompt;
    bool showGUI;
	bool bOptional = false;

	if(!ParseCmdLine(pszCmdLine, strTags, strPath, strApplication, strAction, strPrompt, showGUI, bOptional))
	{
		return;
	}

	if(Item_exists_PathTags(strPath.c_str(), strTags.c_str()))
	{
		return;
	}

	ITEMINFO iteminfo;
	iteminfo.strFilePath = strPath;
	std::wstring::size_type uIndex = iteminfo.strFilePath.rfind(L"\\");
	if( uIndex != std::wstring::npos)
	{
		iteminfo.strFileName = iteminfo.strFilePath.substr(uIndex + 1, iteminfo.strFilePath.length() - uIndex - 1);
	}
	
	iteminfo.strTags = strTags;
	iteminfo.strApplication = strApplication;
	uIndex = iteminfo.strApplication.rfind(L"\\");
	if( uIndex != std::wstring::npos)
	{
		iteminfo.strApplicationName = iteminfo.strApplication.substr(uIndex + 1, iteminfo.strApplication.length() - uIndex - 1);
	}

	iteminfo.strAction = strAction;
	ParseTags( strTags.c_str(), iteminfo.vTags);
	iteminfo.strOriCmdLine = std::wstring(pszCmdLine);

	//Check if the current ITEM needs "window check" (It means if the window with the file name exists on desktop) 
	iteminfo.bNeedCheckWindow = FALSE;
	iteminfo.bWndFound = FALSE;
	if(_wcsicmp(iteminfo.strAction.c_str(), L"open") == 0)
	{
		iteminfo.bNeedCheckWindow = NeedCheckWnd(iteminfo.strApplicationName.c_str(), iteminfo.strFileName.c_str());
		if(iteminfo.bNeedCheckWindow)
		{
			GetRelatedWndTxt(iteminfo.strFileName.c_str(), iteminfo.strApplicationName.c_str(), iteminfo.vWndTitle, iteminfo.vAppTxt);
		}
	}

	//Add prompt paramteter
	iteminfo.strPrompt = strPrompt;
	iteminfo.dwEntryTime = GetTickCount();
	iteminfo.bShowGUI = showGUI;
	iteminfo.bOptional = bOptional;


	::EnterCriticalSection(&CCriSectionMgr::m_CriItemList);
	std::list<ITEMINFO>::iterator itr;
	bool bUpdated = false;
	for(itr = m_listItems.begin(); itr != m_listItems.end(); itr++)
	{
		if(_wcsicmp((*itr).strFilePath.c_str(), iteminfo.strFilePath.c_str()) == 0 && (*itr).bShowGUI == iteminfo.bShowGUI)//can't merge for "interactive" and "auto"
		{//merge tags base on file path
			std::vector<TAGINFO>::iterator it;
			for(it = iteminfo.vTags.begin(); it != iteminfo.vTags.end(); it++)
			{
				TAGINFO info;
				info.strTagName = (*it).strTagName;
				info.vTagValues = (*it).vTagValues;
				(*itr).vTags.push_back(info);

				g_log.Log(CELOG_DEBUG, L"Add a new tag into an existing item, tag: %s\n", info.strTagName.c_str());
			}

			(*itr).strTags += TAG_SEPARATOR + iteminfo.strTags;
			bUpdated = true;
			
			break;
		}
	}
	if(!bUpdated)
	{
		m_listItems.push_back(iteminfo);
		g_log.Log(CELOG_DEBUG, L"Add a new item, tag: %s\n", iteminfo.strTags.c_str());
	}
	::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList);

}

void CAddTagsMgr::RemoveItem(LPCWSTR pszCmdLine, DWORD dwEntryTime)
{
	if(!pszCmdLine )
		return;

	std::list<ITEMINFO>::iterator itr;
	::EnterCriticalSection(&CCriSectionMgr::m_CriItemList);
	for(itr = m_listItems.begin(); itr != m_listItems.end(); itr++)
	{
		if(_wcsicmp((*itr).strOriCmdLine.c_str(), pszCmdLine) == 0 && (*itr).dwEntryTime == dwEntryTime)
		{
			m_listItems.erase(itr);
			break;
		}
	}
	::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList);
	return;
}

void CAddTagsMgr::SetCheckedFlag(LPCWSTR pszCmdLine, DWORD dwEntryTime, BOOL bFlag)
{
	if(!pszCmdLine )
		return;

	std::list<ITEMINFO>::iterator itr;
	::EnterCriticalSection(&CCriSectionMgr::m_CriItemList);
	for(itr = m_listItems.begin(); itr != m_listItems.end(); itr++)
	{
		if(_wcsicmp((*itr).strOriCmdLine.c_str(), pszCmdLine) == 0 && (*itr).dwEntryTime == dwEntryTime)
		{
			(*itr).bWndFound = bFlag;
			break;
		}
	}
	::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList);
	return;
}

/***************************************************************
function name: ParseTags
feature: parse the string of tags, 
like:
Tag1Name=Tag1Value1,Tag1Value2,Tag1Value3;Tag2Name=Tag2Value1
****************************************************************/
BOOL CAddTagsMgr::ParseTags(LPCWSTR lpszTags, OUT std::vector<TAGINFO>& vTags)
{
	if(!lpszTags)
	{
		return FALSE;
	}

	std::wstring strTagInfo(lpszTags);
	//make sure the last character of tag string is ";" 
	if(strTagInfo.substr(strTagInfo.length() - 1, 1) != TAG_SEPARATOR)
	{
		strTagInfo += TAG_SEPARATOR;
	}

	wstring::size_type uIndex;

	while((uIndex = strTagInfo.find(TAG_SEPARATOR) ) != std::wstring::npos)
	{
		std::wstring tag = strTagInfo.substr(0, uIndex);
		strTagInfo = strTagInfo.substr(uIndex + 1, strTagInfo.length() - uIndex - 1);
		
		wstring::size_type uTagIndex = tag.find(TAG_NAME_SEPARATOR);
		if(uTagIndex == std::wstring::npos) //the format is not correct. it should like: document class=A,B,C
		{
			continue;
		}

		std::wstring strTagName = tag.substr(0, uTagIndex);//get tag name
		std::wstring strTagValues = tag.substr(uTagIndex + 1, tag.length() - uTagIndex - 1);

		TAGINFO taginfo;
		taginfo.strTagName = strTagName;

		//try to get all the tag values
		wstring::size_type uTagValueIndex;
		while( (uTagValueIndex = strTagValues.find(VALUE_SEPARATOR) ) != std::wstring::npos)
		{
			std::wstring strTagValue = strTagValues.substr(0, uTagValueIndex);
			taginfo.vTagValues.push_back(strTagValue);

			strTagValues = strTagValues.substr(uTagValueIndex + 1, strTagValues.length() - uTagValueIndex - 1);
		}
		if(!strTagValues.empty())
		{
			taginfo.vTagValues.push_back(strTagValues);
		}

		vTags.push_back(taginfo);
	}

	return TRUE;
}

void CAddTagsMgr::RemoveFirstShownItem()
{
	::EnterCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);
	if(m_listItems_Show.size() > 0)
	{
		m_listItems_Show.pop_front();
	}
	::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);
}

BOOL CAddTagsMgr::NoMoreShownItems()
{
	return GetShownItem() == 0? TRUE: FALSE;
}

unsigned CAddTagsMgr::GetShownItem()
{
	::EnterCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);
	unsigned nCount = static_cast<unsigned>(m_listItems_Show.size());
	::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);

	return nCount;
}
/*
BOOL CAddTagsMgr::NoMoreItems()
{
	::EnterCriticalSection(&CCriSectionMgr::m_CriItemList);
	unsigned nCount = m_listItems.size();
	::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList);

	return nCount == 0? TRUE: FALSE;
}
*/
void CAddTagsMgr::StartWork()
{
	_beginthread(ReadSharedMemoryThread, 0, this);
	m_hWorkingThread = (HANDLE)_beginthread(WorkingThread, 0, this);

	WaitForSingleObject(m_hShowEvent, INFINITE);

	if(!m_pDlg)
	{
		m_pDlg = new CAddTagsDlg();
		if(m_pDlg)
		{
			m_pDlg->DoModal();
		}
	}

	m_bEnd = TRUE;
	WaitForSingleObject(m_hWorkingThread, INFINITE);
}

void CAddTagsMgr::UpdateDialog()
{
	if(m_pDlg && m_pDlg->m_hWnd)
	{
		PostMessage(m_pDlg->m_hWnd, WM_UPDATE_ITEM_COUNT, NULL, NULL);
	}
}

void WorkingThread( LPVOID pParam )
{
	if(!pParam)
	{
		return;
	}

	CAddTagsMgr* pMgr = (CAddTagsMgr*)pParam;

	while(!pMgr->m_bEnd)
	{
		Sleep(200);
		::EnterCriticalSection(&CCriSectionMgr::m_CriItemList);
		std::list<ITEMINFO> listItems = pMgr->m_listItems;
		::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList);

		//Go through all the items, and check if the file is writable.
		std::list<ITEMINFO>::iterator itr;
		for( itr = listItems.begin(); itr != listItems.end(); itr++)//Check all the files in the list. add it to Dialog if it is writable.
		{
			Sleep(20);
			
			ITEMINFO& info = *itr;

			/****************************************
			Try to find the related window for current 
			file.
			****************************************/
			if( info.bNeedCheckWindow && !info.bWndFound )
			{
				if(CheckWindowWithFileName(info))
				{
					pMgr->SetCheckedFlag(info.strOriCmdLine.c_str(), info.dwEntryTime, TRUE);
					wchar_t szTemp[1000] = {0};
					_snwprintf_s(szTemp, 1000, _TRUNCATE, L"addTags::Found window %s", info.strFilePath.c_str());
					g_log.Log(CELOG_DEBUG, szTemp);
				}
				else
				{
					continue;
				}
			}

			if(IsWritable(info.strFilePath.c_str()))//Try to pop up the dialod if the file is "writable".
			{
  	//		OutputDebugStringW(L"This file is writable:");
  	//		OutputDebugStringW(info.strOriCmdLine.c_str());

				if(info.bNeedCheckWindow)
				{
					if(CheckWindowWithFileName(info))//The related window is still there, it mean the current file didn't be closed.
					{
						continue;
					}
				}
				
//  				wchar_t strTemp[1000] = {0};
//  				swprintf_s(strTemp, L"addTags::Push this file to dialog: %s, original command line: %s", info.strFilePath.c_str(), info.strOriCmdLine.c_str());
//  				OutputDebugStringW(strTemp);

                if (info.bShowGUI)//interactive tagging
                {
					::EnterCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);
					std::list<ITEMINFO>::iterator iter;
					bool bUpdated = false;
					for(iter = pMgr->m_listItems_Show.begin(); iter != pMgr->m_listItems_Show.end(); iter++)
					{
						if(_wcsicmp((*iter).strFilePath.c_str(), info.strFilePath.c_str()) == 0)
						{
							std::vector<TAGINFO>::iterator it;
							for(it = info.vTags.begin(); it != info.vTags.end(); it++)
							{
								TAGINFO taginfo;
								taginfo.strTagName = (*it).strTagName;
								taginfo.vTagValues = (*it).vTagValues;
								(*iter).vTags.push_back(taginfo);

								g_log.Log(CELOG_DEBUG, L"add to UI, %s\n", taginfo.strTagName.c_str());
							}
							
							(*iter).strTags += TAG_SEPARATOR + info.strTags;
							bUpdated = true;
							
							break;
						}
					}
					if(!bUpdated)
					{
						pMgr->m_listItems_Show.push_back(info);//push to UI

						g_log.Log(CELOG_DEBUG, L"push to UI, %s\n", info.strTags.c_str());
						
					}
					::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);
					
					pMgr->UpdateDialog();

					pMgr->RemoveItem(info.strOriCmdLine.c_str(), info.dwEntryTime);
					
					//Notify to show tagging dialog
					static BOOL bSet = FALSE;
					if(!bSet)
					{
						SetEvent(pMgr->m_hShowEvent);
						bSet = TRUE;
					}
                }
                else
                {
                    // If we can't show the GUI then we
                    // can't have the user pick between
                    // the various tag values.  Ideally we
                    // shouldn't have been given options,
                    // but if we were, just pick the first
                    std::map<std::wstring, std::wstring> mapTags;

                    for (std::vector<TAGINFO>::const_iterator iter = info.vTags.begin();
                         iter != info.vTags.end();
                         ++iter)
                    {
                        mapTags[iter->strTagName] = iter->vTagValues.front();
                    }

                //    HWND desktopHandle = ::GetDesktopWindow();

                    if(!CTagging::AddTag(info.strFilePath.c_str(), mapTags, NULL))
                    {
                        PopupErrorDlg(::GetDesktopWindow(), pMgr->GetApplicationInstance(), IDS_TAGGING_ERROR);
                    }

                    pMgr->RemoveItem(info.strOriCmdLine.c_str(), info.dwEntryTime);
                }

			}
			
		} 
		

	}
}

void ReadSharedMemoryThread(LPVOID pParam )
{
	if(!pParam)
		return;

	CAddTagsMgr* pMgr = (CAddTagsMgr*)pParam;

	HANDLE hCmdReceived = OpenEvent(EVENT_ALL_ACCESS, NULL, EVENT_CMD_RECEIVED);

	while(hCmdReceived && !pMgr->m_bEnd)
	{
		//Write for "received info" from other instance
		WaitForSingleObject(hCmdReceived, INFINITE);

		//Check the shared memory if there are any new items.
		int nBufLen = MAX_LEN_SHAREDMEMORY/sizeof(wchar_t);
		wchar_t* buffer = new wchar_t[nBufLen];
		
		_ASSERT(buffer);

		

			ReadSharedMemory(buffer, nBufLen - 1);

			g_log.Log(CELOG_DEBUG, L"Read data, cmd: %s\n", buffer);

			//empty the shared memory.
			wchar_t szEmpty[] = {'\0'};
			WriteSharedMemory(szEmpty, 1);

			ResetEvent(hCmdReceived);

			if( wcslen(buffer) > 0)
			{
				pMgr->AddItem(buffer);
			}

			if (buffer != NULL)
			{
				delete [] buffer;
				buffer = NULL;
			}

			
		}

	if(hCmdReceived)
		{
		CloseHandle(hCmdReceived);
		}

	
}