#pragma once


#include <vector>
#include "IniFile.h"


class CActionMenu
{
public:

	CActionMenu();
	
	~CActionMenu();

	
	/*

	create our action menu.

	read menu info from configuration, create menu dynamically. 


	*/
	BOOL Create(const string& stMenuFile);



	/*

	pop up our action menu.

	*/
	BOOL Popup(int x, int y, HWND hWnd);




	/*

	handle click event on action menu item.

	*/
	BOOL HandleClick(DWORD dwItemID);


private:


	/*
	
	private type definition used by tool functions.
	
	
	*/



	typedef BOOL (*DoActionType) (DWORD dwIndex);



	//	this structure is constructed by parse configuration file,
	//	ParseConfig() should handle this task.

	typedef struct _MenuItemInfo
	{
		//	function and parameter
		//	these two members are valid if this item has no sub item.
		DoActionType pFun;
		DWORD dwIndex;

		//	name of menu item
		char szName[256];

		//	this member is valid if the item contains sub item.
		std::vector<_MenuItemInfo> vecSubMenu;
		char szErrorMsg[512];

		_MenuItemInfo()
		{
			pFun = NULL;
			dwIndex = 0;
			*szName = 0;
			memset(szErrorMsg, 0, sizeof(szErrorMsg));
		}
		

	}MenuItemInfo;


	typedef struct _API_ID_MAPPING
	{
		DWORD dwMenuID;
		DoActionType pFun;
		DWORD dwIndex;
		char szErrorMsg[512];

		_API_ID_MAPPING()
		{
			dwMenuID = 0;
			pFun = NULL;
			dwIndex = 0;
			memset(szErrorMsg, 0, sizeof(szErrorMsg));
		}
		
	}API_ID_MAPPING;


private:

	/*
	
	tool functions.
	
	
	*/


	BOOL CreateMenuByConfig();


	/*
	
	parse menucfg.ini file getting menu item information.
	
	*/
	BOOL ParseConfig(const char* pszConfigName);


	BOOL GetProcByItemId(DWORD dwMenuItemId, DoActionType& pFunc, DWORD& dwIndex, char* pszErrorMsg, int nLen);

	BOOL SetProcByItemId(DWORD dwMenuItemId, DoActionType pFunc, DWORD dwIndex, char* pszErrorMsg);



	/*
	
	parse menu item and all its sub menu item info.

	menu item is specified by pszSection, pszParentID and dwIndex.

	*/
	BOOL ParseSubMenu(const char* pszParentID, DWORD dwIndex, const char* pszSection, CIniFile& iniTool, MenuItemInfo& itemInfo, DWORD& dwItemID);


	/*
	
	create menu item according to itemInfo.

	
	*/
	BOOL CreateMenuItem(MenuItemInfo& itemInfo, HMENU& hMenu, BOOL& bPopUP, std::wstring& sMenuItemName);


	//Show correct state for "Start/Stop"(Enable or Disable)
	void ShowPCMenuItemState();
private:

	std::vector<MenuItemInfo> m_MenuInfos;

	HMENU m_hMenu;

	std::vector<API_ID_MAPPING> m_MenuAPIMapping;
};
