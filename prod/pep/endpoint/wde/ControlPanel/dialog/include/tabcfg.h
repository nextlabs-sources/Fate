#pragma once


#include <vector>

class CTabCfg
{

public:

	/*
	
	public type definition can be used by user.

	*/

	typedef HWND (*DoTabType) (DWORD dwIndex, HWND hWnd, LPRECT pRc);

	typedef struct _tabsInfo
	{
		//	function and parameter
		DoTabType pFun;
		DWORD dwIndex;

		//	name of menu item
		char szName[256];


	}TabsInfo;


public:

	/*

	parse tabcfg.ini file getting tab item information.

	*/

	BOOL ParseConfig(const char* pszConfigName);


	/*
	
	get tab information into parameter tabinfo.

	parameter:

	tabinfo	---		user call this function to get tab information and fill tab information into this parameter.
	
	*/

	BOOL GetConfig(std::vector<TabsInfo>& tabinfo)
	{
		tabinfo = m_TabInfo;

		return TRUE;
	}


private:

	std::vector<TabsInfo> m_TabInfo;

};