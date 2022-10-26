#include "StdAfx.h"

#include "NL_NotificationsList.h"

#include <time.h>



CNL_NotificationsList::CNL_NotificationsList(void)

{

	m_hExpanded = NULL;

	m_hShunk = NULL;



	m_hBold = CreateFontW(13,

							0,

							0,

							0,

							FW_BOLD,

							0,

							0,

							0,

							DEFAULT_CHARSET,

							OUT_DEFAULT_PRECIS,

							CLIP_DEFAULT_PRECIS,

							CLEARTYPE_QUALITY,

							DEFAULT_PITCH | FF_DONTCARE,

							g_szFont

							);



	m_clrGrid = RGB(182, 215, 252);

}



CNL_NotificationsList::~CNL_NotificationsList(void)

{

	if(m_hBold)

	{

		DeleteObject(m_hBold);

		m_hBold = NULL;

	}



	if(m_hExpandedIcon)

	{

		DestroyIcon(m_hExpandedIcon);

		m_hExpandedIcon = NULL;

	}

	if(m_hExpandedHoverIcon)

	{

		DestroyIcon(m_hExpandedHoverIcon);

		m_hExpandedHoverIcon = NULL;

	}

	if(m_hShrunkIcon)

	{

		DestroyIcon(m_hShrunkIcon);

		m_hShrunkIcon = NULL;

	}

	if(m_hShrunkHoverIcon)

	{

		DestroyIcon(m_hShrunkHoverIcon);

		m_hShrunkHoverIcon = NULL;

	}

}



LRESULT CNL_NotificationsList::OnDrawItem(UINT uMsg, WPARAM , LPARAM lParam, BOOL& )

{

	LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lParam;



	if (uMsg != OCM_DRAWITEM)

	{

		return S_FALSE;

	}



	if (lpDrawItem->CtlType == ODT_LISTVIEW)

	{

		if (lpDrawItem->itemAction & ODA_DRAWENTIRE)

		{

			RECT rcBk;

			rcBk.left = lpDrawItem->rcItem.left;

			rcBk.top = lpDrawItem->rcItem.top;

			rcBk.right = lpDrawItem->rcItem.right;

			rcBk.bottom = lpDrawItem->rcItem.bottom;





			COLORREF clrHighlight;

			int state = ListView_GetItemState(lpDrawItem->hwndItem, lpDrawItem->itemID, LVIS_FOCUSED | LVIS_SELECTED);

			if (state & LVIS_SELECTED)//Draw highlighted background

			{

				COLORREF color = m_clrBk;

				HBRUSH hbrSEL = CreateSolidBrush(color);

				FillRect(lpDrawItem->hDC, &rcBk, hbrSEL);

				DeleteObject (hbrSEL);

		//		clrHighlight = RGB(255, 255, 255);

				clrHighlight = m_clrText;

			}

			else

			{//draw background

				COLORREF color;

				if(m_clrBk > 0 )

					color = m_clrBk;

				else

					color = GetSysColor(COLOR_BTNFACE);

				HBRUSH hbrBkColor = CreateSolidBrush(color);

				FillRect(lpDrawItem->hDC, &rcBk, hbrBkColor);

				DeleteObject (hbrBkColor);

				clrHighlight = m_clrText;

			}





			//Draw icon "+" "-"		

			smart_ptr<NL_NOTIFICATIONS_ITEM> spNotification;

			if(m_hExpandedIcon && m_hShrunkIcon && GetNotification( lpDrawItem->itemID, spNotification))

			{

				int nBmpLeft = lpDrawItem->rcItem.left + 2;

			//	int nBmpTop = lpDrawItem->rcItem.top + (lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top - 16) / 2;

				int nBmpTop = lpDrawItem->rcItem.top + 2;

				if(spNotification->uType == NL_EXPAND)//The item was expanded, should show "-"

				{

// 					HDC memDC = CreateCompatibleDC ( lpDrawItem->hDC );

// 					HBITMAP hOldBmp = (HBITMAP)SelectObject(memDC, m_hExpanded);

// 

// 					if(!BitBlt(lpDrawItem->hDC, nBmpLeft, nBmpTop, NL_ICON_WIDTH, NL_ICON_HEIGHT, memDC, 0, 0, SRCCOPY))

// 					{

// 						g_log.Log(CELOG_DEBUG, L"BitBlt failed\n");

// 					}

// 

// 					SelectObject(memDC, hOldBmp);

// 					DeleteDC( memDC);



					//use icon

					DrawIconEx(lpDrawItem->hDC, nBmpLeft, nBmpTop, m_hExpandedIcon, 16, 16, 0, 0, DI_NORMAL);

					

				}

				else if(spNotification->uType == NL_SHRINK)//the item was shrunk, should show "+"

				{

// 					HDC memDC = ::CreateCompatibleDC (lpDrawItem->hDC );

// 					HBITMAP hOldBmp = (HBITMAP)SelectObject(memDC, m_hShunk);

// 

// 					BitBlt(lpDrawItem->hDC, nBmpLeft, nBmpTop, NL_ICON_WIDTH, NL_ICON_HEIGHT, memDC, 0, 0, SRCCOPY);

// 

// 					SelectObject(memDC, hOldBmp);

// 					DeleteDC( memDC);



					DrawIconEx(lpDrawItem->hDC, nBmpLeft, nBmpTop, m_hShrunkIcon, 16, 16, 0, 0, DI_NORMAL);

				}

			}



			//try to get the text of current selected item



			WCHAR wzText[MAX_TEXT_LENGTH + 1] = {0};



			LVITEM item;

			memset(&item, 0, sizeof(LVITEM));

			item.mask = LVIF_PARAM | LVIF_TEXT;

			item.iItem = lpDrawItem->itemID;

			item.iSubItem = 0;

			item.pszText = wzText;

			item.cchTextMax = MAX_TEXT_LENGTH;



			//try to get the key with lParam

			ListView_GetItem(lpDrawItem->hwndItem, &item );



			NL_NOTIFICATION_TYPE nType = (NL_NOTIFICATION_TYPE)item.lParam;



			RECT rc;

			rc.left = lpDrawItem->rcItem.left;

			rc.top = lpDrawItem->rcItem.top;

			rc.right = lpDrawItem->rcItem.right;

			rc.bottom = lpDrawItem->rcItem.bottom;



			//Draw line

			HPEN hPen = (HPEN)CreatePen(PS_SOLID, 1, m_clrGrid);

			HPEN hOldPen = (HPEN)SelectObject(lpDrawItem->hDC, hPen );



			//Draw vertical line

			MoveToEx(lpDrawItem->hDC, rc.left, rc.top, (LPPOINT) NULL);

			LineTo(lpDrawItem->hDC, rc.left, rc.bottom);



			MoveToEx(lpDrawItem->hDC, rc.left + NL_ICON_WIDTH, rc.top, (LPPOINT) NULL);

			LineTo(lpDrawItem->hDC, rc.left + NL_ICON_WIDTH, rc.bottom);



			MoveToEx(lpDrawItem->hDC, rc.left + NL_ICON_WIDTH + NL_COL1_WIDTH, rc.top, (LPPOINT) NULL);

			LineTo(lpDrawItem->hDC, rc.left + NL_ICON_WIDTH + NL_COL1_WIDTH, rc.bottom);



			MoveToEx(lpDrawItem->hDC, rc.right - 1, rc.top, (LPPOINT) NULL);

			LineTo(lpDrawItem->hDC,rc.right - 1, rc.bottom);



			//Draw horizonal line

	//		if(lpDrawItem->itemID == 0 )

		

			if(nType == NL_DETAIL_NAME)

			{

				MoveToEx(lpDrawItem->hDC, rc.left, rc.top, (LPPOINT) NULL);

				LineTo(lpDrawItem->hDC, rc.right, rc.top);	

			}



			if( !(nType == NL_DETAIL_NAME || nType == NL_DETAILS) && nType != NL_DETAILS_LAST)

			{

				MoveToEx(lpDrawItem->hDC, rc.left, rc.top, (LPPOINT) NULL);

				LineTo(lpDrawItem->hDC, rc.right, rc.top);	

			}

			

			if(lpDrawItem->itemID == (unsigned)CNL_ListView::GetItemCount() - 1)//the last item, need to draw bottom line

			{

				MoveToEx(lpDrawItem->hDC, rc.left, rc.bottom , (LPPOINT) NULL);

				LineTo(lpDrawItem->hDC, rc.right, rc.bottom);	

			}



			SelectObject(lpDrawItem->hDC, hOldPen);

			DeleteObject(hPen);



			//draw text

			int nOldMode = SetBkMode(lpDrawItem->hDC, TRANSPARENT);

			HFONT oldFont = 0;

			

			COLORREF oldColor = SetTextColor(lpDrawItem->hDC, clrHighlight);



			rc.left = lpDrawItem->rcItem.left + NL_ICON_WIDTH;

			if ( nType > 20)//It means this is the main item, like; 5/1 2010 Notification: It was denied

			{

				DrawMainItem(lpDrawItem->hDC, spNotification, &rc);

			}

			else

			{

				if(nType == NL_DETAIL_NAME)

				{

					oldFont = (HFONT)SelectObject(lpDrawItem->hDC, m_hBold);



					rc.left += NL_COL1_WIDTH + NL_LEFT_MARGIN;

					rc.top += 1;

					DrawTextW(lpDrawItem->hDC, wzText, (int)wcslen(wzText),

							&rc, DT_LEFT | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE);

					SelectObject(lpDrawItem->hDC, oldFont);

				}

				else

				{

					wstring strText(wzText);

					wstring::size_type nIndex = strText.find(NL_KEY_SEPARATOER);

					if( nIndex != wstring::npos)

					{

						wstring strKey = strText.substr(0, nIndex + 1);

						wstring strValue = strText.substr(nIndex + 1, strText.length() - nIndex - 1);

					

						oldFont = (HFONT)SelectObject(lpDrawItem->hDC, m_hBold);



						if(nType == NL_DETAILS || nType == NL_DETAILS_LAST)

						{

							rc.left += NL_DETAILS_MARGIN;

						}



						//draw key

						rc.left += NL_COL1_WIDTH + NL_LEFT_MARGIN;

						rc.top += 1;

						DrawTextW(lpDrawItem->hDC, strKey.c_str(), (int)strKey.length(),

									&rc, DT_LEFT | DT_PATH_ELLIPSIS | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE);



						//draw value

						SelectObject(lpDrawItem->hDC, m_hFont);



						//Compute how long the key uses.

						SIZE len;

						GetTextExtentPoint32W(lpDrawItem->hDC, strKey.c_str(), (int)strKey.length(), &len);



						rc.left += len.cx + NL_VALUE_MARGIN;

						DrawTextW(lpDrawItem->hDC, strValue.c_str(), (int)strValue.length(),

								&rc, DT_LEFT | DT_PATH_ELLIPSIS | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE);



						SelectObject(lpDrawItem->hDC, oldFont);

					}

					

				}



				



			}



			SetBkMode(lpDrawItem->hDC, nOldMode);

			SetTextColor(lpDrawItem->hDC, oldColor);



		}

		

	}



	return S_OK;

}



void CNL_NotificationsList::AddSubItem(struSubItem& item, NL_NOTIFICATION_TYPE nType, int nIndex)

{

	wstring strText = item.strKey + wstring(NL_KEY_SEPARATOER) + item.strValue;



	AddSubItem(strText, nType, nIndex);

}



void CNL_NotificationsList::AddSubItem(std::wstring &strText, NL_NOTIFICATION_TYPE nType, int nIndex)

{

	LVITEM subItem;

	memset(&subItem, 0, sizeof(LVITEM));

	subItem.mask = LVIF_TEXT | LVIF_PARAM;

	subItem.iItem = nIndex;

	subItem.iSubItem = 0;

	subItem.cchTextMax = 500;

	subItem.lParam = (LPARAM)nType;



	wchar_t szTemp[4096] = {0};

	memcpy_s(szTemp, 4096, strText.c_str(), strText.length() * sizeof(wchar_t));

	subItem.pszText = szTemp;



	CNL_ListView::InsertItem(&subItem);



	//Adjust width base on TEXT

	wstring strTemp = strText;

	int nMargin = 0;

	if(nType == NL_DETAILS || nType == NL_DETAILS_LAST)

	{

		nMargin = NL_DETAILS_MARGIN;

	}



	AdjustWidth(strTemp.c_str(), (int)strTemp.length(), nMargin);

}



LRESULT CNL_NotificationsList::OnNotify(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, BOOL& )

{

	if(uMsg == OCM_NOTIFY && NM_CLICK == ((LPNMHDR)lParam)->code)

	{

		int nSel = ListView_GetSelectionMark(m_hWnd);//Get the current selected item



		smart_ptr<NL_NOTIFICATIONS_ITEM> spNotification;



		if(!GetNotification(nSel, spNotification))

		{

			return 0;

		}

		if(spNotification->uType == NL_SHRINK)

		{//Try to show all sub item.s

			spNotification->vSubItemIndex.clear();

			spNotification->uType = NL_EXPAND;



			vector<struSubItem>::iterator itrSubItems = spNotification->vSubItems.begin();

			int nCount = 1;

			//Insert all sub items

			itrSubItems++;//ignore the first one, the first one is main item.

			for( ; itrSubItems != spNotification->vSubItems.end(); itrSubItems++)

			{	

				AddSubItem(*itrSubItems, NL_SUBITEM, nSel + nCount);

				spNotification->vSubItemIndex.push_back(nCount);



				nCount++;



			}



			//add detail name

			if(!spNotification->detail.strName.empty())

			{

				AddSubItem(spNotification->detail.strName, NL_DETAIL_NAME, nSel + nCount);

				spNotification->vSubItemIndex.push_back(nCount);

				nCount++;

			}



			//Add details

			vector<struSubItem>::iterator iterDetail;

			for(iterDetail = spNotification->detail.vValues.begin(); iterDetail != spNotification->detail.vValues.end(); iterDetail++)

			{

				if(iterDetail + 1 == spNotification->detail.vValues.end())

				{

					AddSubItem(*iterDetail, NL_DETAILS_LAST, nSel + nCount);

				}

				else

				{

					AddSubItem(*iterDetail, NL_DETAILS, nSel + nCount);

				}



				spNotification->vSubItemIndex.push_back(nCount);



				nCount++;

			}

			

		}

		else if(spNotification->uType == NL_EXPAND)

		{//Delete all sub items.

			vector<int>::iterator itrIndex;

			int nCount = 0;

			for(itrIndex = spNotification->vSubItemIndex.begin(); itrIndex != spNotification->vSubItemIndex.end(); itrIndex++)

			{

				int nIndex = *itrIndex + nSel - nCount;

				if(nIndex >= 0)

				{

					DeleteItem(nIndex);	

					nCount++;

				}



			}

			spNotification->vSubItemIndex.clear();

			spNotification->uType = NL_SHRINK;

		}

		//Redraw

		Invalidate();

		

	}

	return 0;

}



BOOL CNL_NotificationsList::InsertItemEx( LPARAM lParam, int nIndex)

{

	if(!lParam)

	{

		return FALSE;

	}



	LVITEM item;

	memset(&item, 0, sizeof(LVITEM));

	item.mask = LVIF_PARAM;

	item.iItem = nIndex;

	item.iSubItem = 0;

	

	//Try to get notification information from lParam

	smart_ptr<NL_NOTIFICATIONS_ITEM>* spNotifications = (smart_ptr<NL_NOTIFICATIONS_ITEM>*)( lParam);


#pragma warning(push)
#pragma warning(disable:4311)
	int nKey = (int)((*spNotifications).get());
#pragma warning(pop)


	(*spNotifications)->uType = NL_SHRINK;

	m_mapNotifications[nKey] = *spNotifications;



	item.lParam = (LPARAM)nKey;//Set the key to lParam, so that we can get the "key" in other places with lParam.



	//Computes the length of text

	vector<struSubItem>::iterator iter = (*spNotifications)->vSubItems.begin();



	if(iter != (*spNotifications)->vSubItems.end())

	{

		wstring strTemp = (*iter).strKey + wstring(NL_KEY_SEPARATOER) + (*iter).strValue;



		AdjustWidth(strTemp.c_str(), (int)strTemp.length());

	}



	return CNL_ListView::InsertItem(&item) >= 0? TRUE: FALSE;

}



BOOL CNL_NotificationsList::GetNotification(int nIndex, smart_ptr<NL_NOTIFICATIONS_ITEM>& spNotification)//nIndex: line index in list view

{

	if(nIndex < 0)

	{

		return FALSE;

	}



	LVITEM item;

	memset(&item, 0, sizeof(LVITEM));

	item.mask = LVIF_PARAM;

	item.iItem = nIndex;

	item.iSubItem = 0;



	//try to get the key with lParam

	if( ListView_GetItem(m_hWnd, &item ) )

	{

		int nKey = (int)item.lParam;



		map<int, smart_ptr<NL_NOTIFICATIONS_ITEM>>::iterator iter = m_mapNotifications.find(nKey);//Try to get the related information with key

		if(iter != m_mapNotifications.end())

		{//Succeeded to get the related STRUCT for current item, and then we need to handle its sub items.

			spNotification = (*iter).second;

			return TRUE;

		}

	}

	return FALSE;

}



void CNL_NotificationsList::Initialize(int nWidth)

{

	LVCOLUMN lv;

	memset(&lv, 0, sizeof(LVCOLUMN));

	lv.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT;

	lv.fmt = LVCFMT_IMAGE | LVCFMT_CENTER;

	lv.cx = nWidth;

	lv.cchTextMax = 100;

	wchar_t szHead[] = L"    Date                              Notifications";

	lv.pszText = szHead;

	InsertColumn(0, &lv);



	m_nCurColWidth = nWidth;

}



void CNL_NotificationsList::DrawMainItem(HDC dc, smart_ptr<NL_NOTIFICATIONS_ITEM> spNotification, LPRECT lpRect)

{

	if(!lpRect || !spNotification.valid())

	{

		return;

	}



	RECT rc;

	rc.left = lpRect->left + NL_LEFT_MARGIN;

	rc.top = lpRect->top + 1;

	rc.right = lpRect->right;

	rc.bottom = lpRect->bottom;



	HFONT oldFont = (HFONT)SelectObject(dc, m_hFont);

	

	//draw time

	DrawTextW(dc, spNotification->strTime.c_str(), (int)spNotification->strTime.length(),

		&rc, DT_LEFT | DT_PATH_ELLIPSIS | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE);





	vector<struSubItem>::iterator itrSubItems = spNotification->vSubItems.begin();

	if(itrSubItems !=  spNotification->vSubItems.end())

	{

		//draw key

		SelectObject(dc, m_hBold);



		rc.left = rc.left + NL_COL1_WIDTH;

		

		wstring strKey = (*itrSubItems).strKey + wstring(NL_KEY_SEPARATOER);

		DrawTextW(dc, strKey.c_str(), (int)strKey.length(),

				&rc, DT_LEFT | DT_PATH_ELLIPSIS | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE);



		//draw value

		SelectObject(dc, m_hFont);



		//Compute how long the key uses.

		SIZE len;

		GetTextExtentPoint32W(dc, strKey.c_str(), (int)strKey.length(), &len);



		//Draw value

		rc.left = rc.left + len.cx + NL_VALUE_MARGIN;

		DrawTextW(dc, (*itrSubItems).strValue.c_str(), (int)(*itrSubItems).strValue.length(),

					&rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	}

	

	SelectObject(dc, oldFont);

}



void CNL_NotificationsList::AdjustWidth(LPCWSTR lpszText, int nLen, int nMargin/*0*/)

{

	if(!lpszText)

	{

		return;

	}



	HDC dc = ::GetDC(m_hWnd);

	HFONT oldFont = (HFONT)SelectObject(dc, m_hFont);

	SIZE len;

	GetTextExtentPoint32W(dc, lpszText, nLen, &len);



	int nWidth = len.cx + NL_ICON_WIDTH + NL_COL1_WIDTH + nMargin + 20;

	if(nWidth > m_nCurColWidth)

	{

		CNL_ListView::SetColWidth(0, nWidth);

		m_nCurColWidth = nWidth;

	}



	SelectObject(dc, oldFont);



	::ReleaseDC(m_hWnd, dc);

}



BOOL CNL_NotificationsList::DeleteAllItems()

{

	m_mapNotifications.clear();

	return CNL_ListView::DeleteAllItems();

}
