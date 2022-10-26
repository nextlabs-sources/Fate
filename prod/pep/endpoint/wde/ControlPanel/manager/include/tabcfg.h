#pragma once


#include <vector>

class CTabCfg
{

public:

	/*
	
	public type definition can be used by user.

	*/

	typedef HWND (*DoActionType) (DWORD dwIndex, HWND hWnd, LPRECT pRc);

	typedef struct _tabsInfo
	{
		//	function and parameter
		DoActionType pFun;
		DWORD dwIndex;

		//	name of menu item
		char szName[256];


	}TabsInfo;


public:

	/*

	parse tabcfg.ini file getting menu item information.

	*/

	BOOL ParseConfig(const char* pszConfigName);


	/*
	
	get tab information.
	
	*/

	BOOL GetConfig(std::vector<TabsInfo>& tabinfo)
	{
		tabinfo = m_TabInfo;

		return TRUE;
	}


private:

	std::vector<TabsInfo> m_TabInfo;

};