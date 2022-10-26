#include "stdafx.h"
#include "actionMenu.h"
#include "SysUtils.h"
#include "NotifyMgr.h"
#include "policy_controller.hpp"

#define EDLP_MGR_MENU_ITEM_BASE 200

#define PC_MENUITEM				L"Start/Stop"

CActionMenu::CActionMenu()
{
	m_hMenu = NULL;
}

CActionMenu::~CActionMenu()
{
}


BOOL CActionMenu::Create(const string& stMenuFile)
{
	
	if (m_hMenu)
	{
		//	only create once.
		return TRUE;
	}


	//	read menu information from configuration file
	char pszFullFileName[1024] = {0};
	GetModuleFileNameA(
			NULL,                   // handle to module
			pszFullFileName,        // file name of module
			MAX_PATH                // size of buffer
			);
	char* pTemp = strrchr(pszFullFileName, '\\');
	if(pTemp)
	{
		*(++pTemp) = 0;
	}
	strncat_s(pszFullFileName, 1024, stMenuFile.c_str(), _TRUNCATE);

	if ( !ParseConfig(pszFullFileName) )
	{
		return FALSE;
	}
	

	//	create action menu according to configuration
	if ( !CreateMenuByConfig() )
	{
		return FALSE;
	}

	
	return TRUE;
}



BOOL CActionMenu::CreateMenuByConfig()
{

	m_hMenu = ::CreatePopupMenu ();

	if (!m_hMenu)
	{
		//	create menu failed.
		return FALSE;
	}

	for (DWORD i = 0; i < m_MenuInfos.size(); i++)
	{
		HMENU hMenu = NULL;
		BOOL bPopup = FALSE;
		std::wstring sName;
		if ( !CreateMenuItem(m_MenuInfos[i], hMenu, bPopup, sName) )
		{
			return FALSE;
		}
		
		DWORD uFlag = 0;
		//	check if current menu item is a separator
		if (L"separator" == sName)
		{
			//	yes, it is a separator,
			//	append a separator menu item
			uFlag = MF_SEPARATOR;
			AppendMenu(m_hMenu, uFlag, (UINT_PTR)hMenu, NULL);
		}
		else
		{
			uFlag = bPopup ? (MF_STRING | MF_POPUP) : MF_STRING;
			AppendMenu(m_hMenu, uFlag, (UINT_PTR)hMenu, sName.c_str());
		}
	}
	
	return TRUE;
}



BOOL CActionMenu::Popup(int x, int y, HWND hWnd)
{

	if (!m_hMenu)
	{
		//	the menu is not created well, user should create action menu first
		return FALSE;
	}

	ShowPCMenuItemState();

	BOOL res = TrackPopupMenu (m_hMenu,
					TPM_LEFTBUTTON,
					x,
					y,
					0,
					hWnd,
					NULL);

	return res;
}

/***********************************************
@brief: Check if PC is running or not.
		Enable/Disable the related menu items 
		according to PC status.
************************************************/
void CActionMenu::ShowPCMenuItemState()
{
	if(m_hMenu)
	{
		int nCount = GetMenuItemCount(m_hMenu);
		for ( int i = 0; i < nCount; i++)
		{
			//Try to get menu item info.
			MENUITEMINFOW info;
			info.cbSize = sizeof(MENUITEMINFOW);
			info.fMask = MIIM_STRING | MIIM_SUBMENU;
			wchar_t buf[1024] = {0};
			info.dwTypeData = buf;
			info.cch = sizeof(buf)/sizeof(wchar_t);

			if( GetMenuItemInfoW(m_hMenu, i, TRUE, &info) )
			{
				if ( _wcsicmp(info.dwTypeData, PC_MENUITEM) == 0 )//Check if the current menu item is "Enforcers"
				{
					HMENU  hSub = info.hSubMenu;

					int nSubCount = GetMenuItemCount(hSub);

					if(nSubCount == 3)
					{
						BOOL bUp = nextlabs::policy_controller::is_up();//Get PC state

						if(bUp)
						{
							::EnableMenuItem(hSub, 0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
							::EnableMenuItem(hSub, 2, MF_BYPOSITION | MF_ENABLED);
						}
						else
						{
							::EnableMenuItem(hSub, 0, MF_BYPOSITION | MF_ENABLED);
							::EnableMenuItem(hSub, 2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
						}
					}
					break;
				}
			}


		}
	}
}

BOOL CActionMenu::HandleClick(DWORD dwItemID)
{
	DoActionType pFunc = NULL;
	DWORD dwIndex = 0;

	CEDPMgr& edpMgr = CEDPMgr::GetInstance();

	char szErrorMsg[1024] = {0};
	if ( !GetProcByItemId(dwItemID, pFunc, dwIndex, szErrorMsg, 1024) )
	{
		//	the function call failed,
		//	some error happened.
		edpMgr.GetCELog().Log(CELOG_DEBUG, L"CActionMenu GetProcByItemId failed\n");
		return FALSE;
	}

	if(pFunc)
	{
		pFunc(dwIndex);
	}
	else
	{
		edpMgr.GetCELog().Log(CELOG_DEBUG, L"CActionMenu HandleClick pFunc is NULL\n");
	}

	return TRUE;
}


/*
parse configuration file. result is set into m_MenuInfos.


configuration file example 1.

[Section Count]
Count = 3
[Item1]
Name = EDLP Manager
Count = 0
API = app,0
[Item2]
Name = separator
[Item3]
Name = Diagnostic
Count = 0
API = emgrdgnstc.dll,0


configuration file example 2.

[Section Count]
Count = 5
[Item1]
Name = EDLP Manager
Count = 0
API = app, 0
[Item2]
Name = separator
[Item3]
Name = Diagnostic
Count = 0
API = emgrdgnstc.dll, 0
[Item4]
Name = separator
[Item5]
Name = Update
Count = 2
Name1= policy
Count1 = 0
Api1 = updatePolicy.dll, 0
Name2 = Enforcer
Count2 = 2
Name2.1 = all enforcers
Count2.1 = 0
Api2.1 = updateEnforcer.dll, 0
Name2.2 = windows Enforcer
Count2.2 = 0
Api2.2 = updateEnforcer.dll, 1

*/


/*

parse menu item and all subitem specified by pszParantID ,dwIndex and pszSection.


const char* pszParentID		--	input, parent item id.

DWORD dwIndex	--	input, index related with its parent item.

const char* pszSection	--	input, section name in ini file of this item

CIniFile& iniTool	--	input, ini configuration tool

MenuItemInfo& itemInfo	--	output, menu item and sub menu item information.

DWORD& dwItemID	--	input and output, this ID is based on EDLP_MGR_MENU_ITEM_BASE, can be used by AppendMenu / InsertMenu, 


*/
BOOL CActionMenu::ParseSubMenu(const char* pszParentID, DWORD dwIndex, const char* pszSection, CIniFile& iniTool, MenuItemInfo& itemInfo, DWORD& dwItemID)
{
	if (!pszSection)
	{
		//	pszSection is not allowed to be NULL
		return FALSE;
	}
		

	//	combine pszParantID and dwIndex to get menu item ID.
	char szID[256] = {0};


	//	check if parent id is NULLL
	if (!pszParentID)
	{
		//	yes, parent id is NULL, this means item is root item,
		//	in this case, szID should be ""
		*szID = 0;
	}
	else
	{
		//	parent id is not NULL, check if parent id is too long, we can not support too long.
		if ( strlen(pszParentID) > 200 )
		{
			//	parent id is too long, do not support, return error
			return FALSE;
		}

		//	check if parent ID is ""
		if ( 0 == *pszParentID )
		{
			//	yes, parent ID is "", this means parent item is root item,
			//	in this case, directly use dwIndex as item ID.
			_snprintf_s(szID, 256, _TRUNCATE, "%d", dwIndex);
		}
		else
		{
			//	combine pszParantID and dwIndex to get menu item ID.
			_snprintf_s(szID, 256, _TRUNCATE, "%s.%d", pszParentID, dwIndex);
		}
	}

	//	get item name according to ID and dwSectionID
	char szNameKey[256] = {0};
	_snprintf_s(szNameKey, 256, _TRUNCATE, "Name%s", szID);

	char szNameValue[256] = {0};
	iniTool.ReadString(pszSection, szNameKey, "", szNameValue, sizeof(szNameValue));

	strncpy_s(itemInfo.szName, 256, szNameValue, _TRUNCATE);


	//	check if this item is a separator
	if ( !strcmp("separator", itemInfo.szName) )
	{
		//	yes, it is a separator, the separator has only a hard code name and has no API and count, and so, it has no sub menu item,
		//	so, we finish parsing work.
		return TRUE;
	}


	//	get item count also according to ID and dwSectionID
	char szCountKey[256] = {0};
	_snprintf_s(szCountKey, 256, _TRUNCATE, "Count%s", szID);

	char szCountValue[256] = {0};
	iniTool.ReadString(pszSection, szCountKey, "", szCountValue, sizeof(szCountValue));

	//	check if count value is not zero.
	DWORD dwCnt = atoi(szCountValue);
	if ( !dwCnt )
	{
		//	yes, count value is zero,
		//	means item has no sub item, so it has API.
		//	parse API getting API address and index value.
		char szAPIKey[256] = {0};
		_snprintf_s(szAPIKey, 256, _TRUNCATE, "API%s", szID);

		char szAPIValue[256] = {0};
		iniTool.ReadString(pszSection, szAPIKey, "", szAPIValue, sizeof(szAPIValue));

		char* pComma = strchr(szAPIValue, ',');
		if (!pComma)
		{
			return FALSE;
		}

		*pComma = 0;

		pComma++;

		//	get index value
		itemInfo.dwIndex = atoi(pComma);

		
		//	it is exported from library, load it and get API address out.
		HMODULE hPlugin = LoadLibraryA(szAPIValue);
		if (hPlugin)
		{
			itemInfo.pFun = (DoActionType)GetProcAddress(hPlugin, "DoAction");

			if (!itemInfo.pFun)
			{
				_snprintf_s(itemInfo.szErrorMsg, sizeof(itemInfo.szErrorMsg), _TRUNCATE, "Failed to get address \"DoAction\" of %s", szAPIValue);
				return TRUE;
			}
		}
		else
		{
			_snprintf_s(itemInfo.szErrorMsg, sizeof(itemInfo.szErrorMsg), _TRUNCATE, "Failed to Load %s", szAPIValue);
			return TRUE;
		}
		
	}
	else
	{
		//	no, count value is not zero,
		//	means item has sub item, so it has no API, but has sub menu item.

		for (DWORD i = 0; i < dwCnt; i++)
		{
			//	parse sub item one by one

			//	parse
			MenuItemInfo subItemInfo;
			if ( !ParseSubMenu(szID, i + 1, pszSection, iniTool, subItemInfo, dwItemID))
			{
				//	parse failed
				return FALSE;
			}
			//	insert
			itemInfo.vecSubMenu.push_back(subItemInfo);
		}
	}

	return TRUE;
}

BOOL CActionMenu::ParseConfig(const char* pszConfigName)
{

	if (!pszConfigName)
	{
		return FALSE;
	}


	CIniFile* pIni = new CIniFile(pszConfigName);

	if (!pIni)
	{
		return FALSE;
	}

	char szSectionCnt[256] = {0};
	pIni->ReadString("Section Count", "Count", "0", szSectionCnt, sizeof(szSectionCnt));
	

	DWORD dwMenuItemID = EDLP_MGR_MENU_ITEM_BASE;

	DWORD dwSectionCnt = atoi(szSectionCnt);
	for ( DWORD i = 0; i < dwSectionCnt; i++ )
	{
		//	get section name for each root item
		char szItem[256] = {0};
		_snprintf_s(szItem, 256, _TRUNCATE, "%s%d", "Item", i + 1);

		//	parse item and all its sub item
		MenuItemInfo itemInfo;
		if ( !ParseSubMenu(NULL, 0, szItem, *pIni, itemInfo, dwMenuItemID) )
		{
			//	error
			delete pIni;
			return FALSE;
		}

		//	push into m_MenuInfos
		m_MenuInfos.push_back(itemInfo);
	}

	delete pIni;
	return TRUE;
}

BOOL CActionMenu::CreateMenuItem(MenuItemInfo& itemInfo, HMENU& hMenu, BOOL& bPopUP, std::wstring& sMenuItemName)
{
	/*
	
	fake code

	if itemInfo.vecSub.count = 0
		hmenu = CreatePopupMenu
		return hmenu tell caller this item has no submenu
	else
		for(i = 0; i < itemInfo.vecSub.count; i++)
			call CreateMenuItem with this subitem info,
			call AppendMenu to hMenu with handle to above sub menu.
		return hmenu

	also we need to build a data structure to store the mapping of menuitemid and func.
		
	*/


	//	tell caller current menu item name
	WCHAR wszName[256] = {0};
	AnsiToUnicode(itemInfo.szName, wszName, 256);
	sMenuItemName = wszName;


	//	create a menu for current item
	hMenu = CreatePopupMenu();


	//	check if current menu item has sub menu item
	if (!itemInfo.vecSubMenu.size())
	{
		//	no, it has no sub menu item
		//	so, it is not a pop up menu item
		bPopUP = FALSE;
		

		//	check if current item is a separator
		if ( L"separator" != sMenuItemName )
		{
			//	no, it is not a separator.
			//	and as current menu item has no sub menu item,
			//	so it has API. associating API with menu handle,
			//	so when process windows notification by menu handle, we can find related API
			SetProcByItemId( LOWORD((DWORD_PTR)hMenu), itemInfo.pFun, itemInfo.dwIndex, itemInfo.szErrorMsg);	
		}
		
		return TRUE;
	}

	//	current menu item has sub menu item
	bPopUP = TRUE;

	for (DWORD i = 0; i < itemInfo.vecSubMenu.size(); i++)
	{
		HMENU hSubMenu = NULL;
		BOOL bSubPopup = FALSE;
		std::wstring sSubItemName;

		//	create sub menu item
		if ( !CreateMenuItem(itemInfo.vecSubMenu[i], hSubMenu, bSubPopup, sSubItemName) )
		{
			return FALSE;
		}

		//	append sub menu item to current menu item
		DWORD uFlag = 0;
		if (L"separator" == sSubItemName)
		{
			//	yes, it is a separator,
			//	append a separator menu item
			uFlag = MF_SEPARATOR;
			AppendMenu(hMenu, uFlag, (UINT_PTR)hSubMenu, NULL);
		}
		else
		{
			uFlag = bSubPopup ? (MF_STRING | MF_POPUP) : MF_STRING;
			AppendMenu(hMenu, uFlag, (UINT_PTR)hSubMenu, sSubItemName.c_str());
		}
	}

	return TRUE;
}

BOOL CActionMenu::GetProcByItemId(DWORD dwMenuItemId, DoActionType& pFunc, DWORD& dwIndex, char* pszErrorMsg, int nLen)
{
	for(DWORD i = 0; i < m_MenuAPIMapping.size(); i++)
	{
		if (dwMenuItemId == m_MenuAPIMapping[i].dwMenuID)
		{
			pFunc = m_MenuAPIMapping[i].pFun;
			dwIndex = m_MenuAPIMapping[i].dwIndex;

			if(pszErrorMsg)
			{
				strncpy_s(pszErrorMsg, nLen, m_MenuAPIMapping[i].szErrorMsg, _TRUNCATE);
			}
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CActionMenu::SetProcByItemId(DWORD dwMenuItemId, DoActionType pFunc, DWORD dwIndex, char* pszErrorMsg)
{
	API_ID_MAPPING ApiIDPair;
	ApiIDPair.dwMenuID = dwMenuItemId;
	ApiIDPair.pFun = pFunc;
	ApiIDPair.dwIndex = dwIndex;

	if(pszErrorMsg)
	{
		strncpy_s(ApiIDPair.szErrorMsg, sizeof(ApiIDPair.szErrorMsg), pszErrorMsg, _TRUNCATE);
	}

	m_MenuAPIMapping.push_back(ApiIDPair);

	return TRUE;
}

