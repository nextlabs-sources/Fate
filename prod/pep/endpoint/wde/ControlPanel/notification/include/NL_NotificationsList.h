#pragma once

#include "NL_ListView.h"
#include <vector>
#include <string>
#include <map>
#include "smart_ptr.h"

#define NL_COL1_WIDTH		150
#define NL_ICON_WIDTH		16
#define NL_ICON_HEIGHT		16
#define NL_LEFT_MARGIN		2
#define NL_VALUE_MARGIN		10
#define NL_DETAILS_MARGIN	50
#define NL_KEY_SEPARATOER	L":"

using namespace std;
using namespace NL_SP;

typedef struct struSubItem
{
	wstring strKey;
	wstring strValue;
}SUBITEM, *LPSUBITEM;

typedef struct struNL_Notifications_Details
{
	wstring strName;
	vector<struSubItem> vValues;
}NL_NOTIFICATION_DETAILS, *LPNL_NOTIFICATION_DETAILS;

enum NL_NOTIFICATION_STATUS{NL_EXPAND = 0, NL_SHRINK};
enum NL_NOTIFICATION_TYPE{NL_SUBITEM = 0, NL_DETAIL_NAME, NL_DETAILS, NL_DETAILS_LAST};

typedef struct struNL_NotificationsItem
{
	unsigned uType;//Determine if the current item is expanded or shrunk.
	wstring strTime;
	vector<struSubItem> vSubItems;
	NL_NOTIFICATION_DETAILS detail;
	vector<int> vSubItemIndex;
}NL_NOTIFICATIONS_ITEM, *LPNL_NOTIFICATIONS_ITEM;

class CNL_NotificationsList: public CNL_ListView
{
public:
	CNL_NotificationsList(void);
	~CNL_NotificationsList(void);

	BEGIN_MSG_MAP(CNL_NotificationsList)
		MESSAGE_HANDLER( OCM_DRAWITEM, OnDrawItem )
		MESSAGE_HANDLER( OCM_MEASUREITEM, OnMeasureItem )
		MESSAGE_HANDLER( OCM_NOTIFY, OnNotify)
		
	//	DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

public:
	void Initialize(int nWidth);
	void SetLinesInItem(int nLines){m_nLinesInItem = nLines;}
	BOOL InsertItemEx( LPARAM lParam, int nIndex);
	void SetExpandedBmp(HBITMAP hBmp) {m_hExpanded = hBmp;}
	void SetShrunkBmp(HBITMAP hBmp) {m_hShunk = hBmp;}
	void SetGridColor(COLORREF clr) {m_clrGrid = clr;}

	BOOL DeleteAllItems();

	void ShowDetails(int nIndex);

	//Add icon functions
	void SetShrunkIcon(HICON hIcon, HICON hIcoHover){m_hShrunkIcon = hIcon; m_hShrunkHoverIcon = hIcoHover;}
	void SetExpandedIcon(HICON hIcon, HICON hIcoHover){m_hExpandedIcon = hIcon; m_hExpandedHoverIcon = hIcoHover;}
protected:
	BOOL GetNotification(int nIndex, smart_ptr<NL_NOTIFICATIONS_ITEM>& spNotification);//nIndex: line index in list view
	void AddSubItem(struSubItem& item, NL_NOTIFICATION_TYPE nType, int nIndex);
	void AddSubItem(wstring& strText, NL_NOTIFICATION_TYPE nType, int nIndex);
	void DrawMainItem(HDC dc, smart_ptr<NL_NOTIFICATIONS_ITEM> spNotification, LPRECT lpRect);
	void AdjustWidth(LPCWSTR lpszText, int nLen, int nMargin = 0);
protected:
	map<int, smart_ptr<NL_NOTIFICATIONS_ITEM>> m_mapNotifications;
	HBITMAP m_hExpanded;
	HBITMAP m_hShunk;

	HFONT m_hBold;
	COLORREF m_clrGrid;

	int m_nCurColWidth;

	HICON m_hShrunkIcon;
	HICON m_hShrunkHoverIcon;
	HICON m_hExpandedIcon;
	HICON m_hExpandedHoverIcon;
};
