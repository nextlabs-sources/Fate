#include "stdafx.h"
#include "tabcfg.h"
#include "IniFile.h"


#define EDLP_MGR_TAB_BASE 300

BOOL CTabCfg::ParseConfig(const char* pszConfigName)
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

	//	read tab count

	char szSectionCnt[256] = {0};
	pIni->ReadString("Section Count", "Count", "0", szSectionCnt, 256);

	DWORD dwSectionCnt = atoi(szSectionCnt);


	//	read information of each tab
	for ( DWORD i = 0; i < dwSectionCnt; i++ )
	{
		char szItem[256] = {0};
		_snprintf_s(szItem, 256, _TRUNCATE, "%s%d", "tab", i + 1);

		TabsInfo itemInfo;

		//	set item name
		pIni->ReadString(szItem, "Name", NULL, itemInfo.szName, sizeof(itemInfo.szName));

		//	set item API and index
		char szAPI[256] = {0};
		pIni->ReadString(szItem, "API", szAPI, szAPI, sizeof(szAPI));

		char* pComma = strchr(szAPI, ',');
		if (!pComma)
		{
			delete pIni;
			return FALSE;
		}


		*pComma = 0;

		pComma++;

		//	set item handler function index

		itemInfo.dwIndex = atoi(pComma);

		//Try to get full path of the current DLL
		char szDir[MAX_PATH + 1] = {0};
		GetCurrentDirectory(szDir, MAX_PATH);
		strncat_s(szDir, MAX_PATH, szAPI, _TRUNCATE);


		HMODULE hPlugin = LoadLibraryA(szDir);
		if (hPlugin)
		{
			//	set item handler function address

			itemInfo.pFun = (DoTabType)GetProcAddress(hPlugin, "DoTab");

			if (!itemInfo.pFun)
			{
				delete pIni;
				return FALSE;
			}
		}
		else
		{
			delete pIni;
			return FALSE;
		}


		//	finish item parsing

		m_TabInfo.push_back(itemInfo);

	}

	delete pIni;

	return TRUE;
}