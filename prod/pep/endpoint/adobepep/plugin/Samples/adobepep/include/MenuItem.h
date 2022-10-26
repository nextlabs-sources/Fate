#pragma once
#pragma warning(push)
#pragma warning(disable: 4819)  // We won't fix code page issue in 3rd party's header file, just ignore it here

// Acrobat Headers.
#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif

#pragma warning(pop)

#include "celog.h"

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE   CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_INCLUDE_MENUITEM_H

class CMenuItem
{
private:
	CMenuItem()
	{
		m_bInit=false;
		m_menuitem_close=NULL;
	}
	~CMenuItem()
	{

	}
public:
	static CMenuItem* GetInstance()
	{
		static CMenuItem ins;
		ins.init();
		return &ins;
	}
	void execute_close()
	{
		if (NULL!=m_menuitem_close)
		{
			CELOG_LOG(CELOG_DEBUG, L"CMenuItem::execute_close done\n");
			AVMenuItemExecute(m_menuitem_close);
		}
	}
private:
	void init()
	{
		if (false==m_bInit)
		{
			AVMenuItemPredicate MyMenuItemPredicatePtr = ASCallbackCreateProto(AVMenuItemPredicate, &MyMenuItemPredicate);
			AVMenubar TheMenubar = AVAppGetMenubar();

			//找到File->Close菜单的句柄
			m_menuitem_close = AVMenubarAcquireMenuItemByPredicate(TheMenubar, MyMenuItemPredicatePtr, "&Close");
			if (m_menuitem_close)
			{
				CELOG_LOG(CELOG_DEBUG, L"CMenuItem::init, have got menu item: &Close\n");
			}

			m_bInit=true;
		}
	}
	static ACCB1 ASBool ACCB2 MyMenuItemPredicate(AVMenuItem menuItem, void *clientData)
	{
		//	Gets the menu item's title as it appears in the user interface
		char buffer[1024] = {0};
		AVTArraySize len = AVMenuItemGetTitle(menuItem, buffer, 1023);
		if(0!=len)
		{
			if(!strcmp(buffer, (char*)clientData))
			{
				return true;
			}
		}
		return false;
	}
public:
	void GetNewWindowMeunItem()
	{
		AVMenuItemPredicate NewWindowMenuItemPredicatePtr = ASCallbackCreateProto(AVMenuItemPredicate, &NewWindowMenuItemPredicate);
		AVMenubar TheMenubar = AVAppGetMenubar();

		
		m_menuitem_new_window = AVMenubarAcquireMenuItemByPredicate(TheMenubar, NewWindowMenuItemPredicatePtr, "&New Window");
		if (m_menuitem_new_window)
		{
			CELOG_LOG(CELOG_DEBUG, L"CMenuItem::init, have got menu item: &New Window\n");
		}

	}


	static ACCB1 ASBool ACCB2 NewWindowMenuItemPredicate(AVMenuItem menuItem, void *clientData)
	{
		//	Gets the menu item's title as it appears in the user interface
		char buffer[1024] = {0};
		AVTArraySize len = AVMenuItemGetTitle(menuItem, buffer, 1023);
		if(0!=len)
		{
			if(!strcmp(buffer, (char*)clientData))
			{
				return true;
			}
		}
		return false;
	}


	void GetAutoScrollMeunItem()
	{
		AVMenuItemPredicate AutoScrollMenuItemPredicatePtr = ASCallbackCreateProto(AVMenuItemPredicate, &AutoScrollMenuItemPredicate);
		AVMenubar TheMenubar = AVAppGetMenubar();

		//找到File->Close菜单的句柄
		m_menuitem_auto_scroll = AVMenubarAcquireMenuItemByPredicate(TheMenubar, AutoScrollMenuItemPredicatePtr, "Automaticall&y Scroll");
		if (m_menuitem_auto_scroll)
		{
			CELOG_LOG(CELOG_DEBUG, L"CMenuItem::init, have got menu item: Automaticall&y Scroll\n");
		}

		m_menuitem_Compare_Documents = AVMenubarAcquireMenuItemByPredicate(TheMenubar, AutoScrollMenuItemPredicatePtr, "&Compare Documents...");
		if (m_menuitem_Compare_Documents)
		{
			CELOG_LOG(CELOG_DEBUG, L"CMenuItem::init, have got menu item: Compare Documents...\n");
		}

	}


	static ACCB1 ASBool ACCB2 AutoScrollMenuItemPredicate(AVMenuItem menuItem, void *clientData)
	{
		//	Gets the menu item's title as it appears in the user interface
		char buffer[1024] = {0};
		AVTArraySize len = AVMenuItemGetTitle(menuItem, buffer, 1023);
		if(0!=len)
		{
			if(!strcmp(buffer, (char*)clientData))
			{
				return true;
			}
		}
		return false;
	}


	void GetSplitMeunItem()
	{
		AVMenuItemPredicate SplitMenuItemPredicatePtr = ASCallbackCreateProto(AVMenuItemPredicate, &SplitMenuItemPredicate);
		AVMenubar TheMenubar = AVAppGetMenubar();

		m_menuitem_split = AVMenubarAcquireMenuItemByPredicate(TheMenubar, SplitMenuItemPredicatePtr, "&Split");
		if (m_menuitem_split)
		{
			CELOG_LOG(CELOG_DEBUG, L"CMenuItem::init, have got menu item: &Split \n");
		}

	}


	static ACCB1 ASBool ACCB2 SplitMenuItemPredicate(AVMenuItem menuItem, void *clientData)
	{
		//	Gets the menu item's title as it appears in the user interface
		char buffer[1024] = {0};
		AVTArraySize len = AVMenuItemGetTitle(menuItem, buffer, 1023);
		if(0!=len)
		{
			if(!strcmp(buffer, (char*)clientData))
			{
				return true;
			}
		}
		return false;
	}

	
	void GetSpreadSplitMeunItem()
	{
		AVMenuItemPredicate SpreadSplitMenuItemPredicatePtr = ASCallbackCreateProto(AVMenuItemPredicate, &SpreadSplitMenuItemPredicate);
		AVMenubar TheMenubar = AVAppGetMenubar();

		//找到File->Close菜单的句柄
		m_menuitem_spread_split = AVMenubarAcquireMenuItemByPredicate(TheMenubar, SpreadSplitMenuItemPredicatePtr, "Sprea&dsheet Split");
		if (m_menuitem_spread_split)
		{
			CELOG_LOG(CELOG_DEBUG, L"CMenuItem::init, have got menu item: Sprea&dsheet Split \n");
		}

	}


	static ACCB1 ASBool ACCB2 SpreadSplitMenuItemPredicate(AVMenuItem menuItem, void *clientData)
	{
		//	Gets the menu item's title as it appears in the user interface
		char buffer[1024] = {0};
		AVTArraySize len = AVMenuItemGetTitle(menuItem, buffer, 1023);
		if(0!=len)
		{
			if(!strcmp(buffer, (char*)clientData))
			{
				return true;
			}
		}
		return false;
	}
	
public:
	AVMenuItem m_menuitem_new_window;
	AVMenuItem m_menuitem_auto_scroll;
	AVMenuItem m_menuitem_split;
	AVMenuItem m_menuitem_spread_split;
	AVMenuItem m_menuitem_Compare_Documents;

private:
	bool m_bInit;
	AVMenuItem m_menuitem_close;
};