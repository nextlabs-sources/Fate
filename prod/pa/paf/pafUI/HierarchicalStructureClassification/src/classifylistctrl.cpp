

#include <Windows.h>
#include <Winuser.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <assert.h>

#include <list>

//#include "commonui.hpp"
#include <atlbase.h>
#include <atlcomcli.h>
#include "classifylistctrl.hpp"

#include <boost/scoped_array.hpp>

#define UNSELECTED_STR  L" "


LRESULT CALLBACK NewListProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// simply check the display text of a window is clipped
/*

If the end of the text of a control does not fit in the rectangle, it is truncated and ellipses are added.

return the length of words need chopping of at the end of the text of a control

e.g. it changes window text and returns 25

  |-------------49 characters---------------------|
  A long label string that will usually get clipped
  A long label string that...                     |
  |-----24 characters----||-----25 characters-----|
*/
// 
//int EllipsizeWindowText(HWND hwnd)
//{
//    BOOL bRet;
//
//    SIZE dealSize = { 0 };
//    // BCM_GETIDEALSIZE: Gets the size of the button that best fits its text and image, if an image list is present.
//    BOOL bGetDealSize = SendMessage(hwnd, BCM_GETIDEALSIZE, 0, (LPARAM)&dealSize);
//
//    RECT rcClient;
//    bRet = ::GetClientRect(hwnd, &rcClient);
//    const int nClientWidth = rcClient.right - rcClient.left;
//
//	{
//		wchar_t sz[1024];
//		wsprintfW(sz, L"EllipsizeWindowText, nClientWidth=%d, dealSize.cx=%d, bGetDealSize=%d, GetLastError=%#x\n",
//			nClientWidth, dealSize.cx, bGetDealSize, GetLastError());
//		OutputDebugStringW(sz);
//	}
//
//    // If the end of a string does not fit in the rectangle, it is truncated.
//    if (nClientWidth < dealSize.cx) //
//    {
//        //calculate how to add ellipses (...) at the end.
//
//        const int nTCharsOfBuffer = GetWindowTextLengthW(hwnd);
//        boost::scoped_array<wchar_t> sazBuffer(new wchar_t[nTCharsOfBuffer + 1]); // RAII 
//        int nTCharsOfText = GetWindowTextW(hwnd, sazBuffer.get(), nTCharsOfBuffer);
//
//		{
//			wchar_t sz[1024];
//			wsprintfW(sz, L"EllipsizeWindowText, GetWindowTextW=%s\n", sazBuffer.get());
//			OutputDebugStringW(sz);
//		}
//
//        int end = nTCharsOfText - 3;
//        if (end > 0)
//        {
//            do
//            {
//                sazBuffer[end] = '.';
//                sazBuffer[end + 1] = '.';
//                sazBuffer[end + 2] = '.';
//                sazBuffer[end + 3] = '\0';
//
//                SetWindowTextW(hwnd, sazBuffer.get());
//
//                dealSize.cx = 0;
//                dealSize.cy = 0;
//                SendMessage(hwnd, BCM_GETIDEALSIZE, 0, (LPARAM)&dealSize);
//
//                --end;
//            } while (nClientWidth < dealSize.cx && end > 0);
//
//            return nTCharsOfText - end;
//        }
//    }
//    return 0;
//}

void EllipsizeWindowText(HWND hwnd, const std::wstring &text)
{
	RECT rcClient;
	BOOL bRet = ::GetClientRect(hwnd, &rcClient);

	// If the end of a string does not fit in the rectangle, it is truncated.

	const int nTCharsOfBuffer = GetWindowTextLengthW(hwnd);
	const int nTCharsOfText = text.length();
	boost::scoped_array<wchar_t> sazBuffer(new wchar_t[nTCharsOfText + 1]); // RAII 
	wcscpy_s(sazBuffer.get(), nTCharsOfText + 1, text.c_str());

	HDC hdc = ::GetDC(hwnd);
	HFONT hOldFont = SelectFont(hdc, GetWindowFont(hwnd));

	//SIZE stringSize;
	//bRet = GetTextExtentPointW(hdc, sazBuffer.get(), nTCharsOfText, &stringSize);
	//int totalWidth = stringSize.cx + GetSystemMetrics(SM_CXMENUCHECK) + 7;
	rcClient.right = rcClient.right - GetSystemMetrics(SM_CXMENUCHECK) - 7;

	// calculate how to add ellipses (...) at the end.
	int iHeightOrTop = DrawText(hdc, sazBuffer.get(), nTCharsOfText, &rcClient, DT_MODIFYSTRING | DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);

	if (0 != text.compare(sazBuffer.get()))
	{
		SetWindowText(hwnd, sazBuffer.get());
	}

	(HFONT)SelectObject(hdc, hOldFont);
	::ReleaseDC(hwnd, hdc);
}

//
//  class CListItem
//

CListItem::CListItem() : _level(-1), _clsitem(NULL)
{
    _margin_unit.SetDluUnits(7, 7);
    _caption_unit.SetDluUnits(8, 8);
    _checkbox_unit.SetDluUnits(10, 10);
    _combobox_unit.SetDluUnits(14, 14);
}

CListItem::CListItem(int level, const CDlgUnit& margin_unit, const CDlgUnit& caption_unit, const CDlgUnit& checkbox_unit, const CDlgUnit& combobox_unit) : _level(level), _clsitem(NULL)
{
    _margin_unit = margin_unit;
    _caption_unit = caption_unit;
    _checkbox_unit = checkbox_unit;
    _combobox_unit = combobox_unit;
}

CListItem::~CListItem()
{
    Destroy();
}

BOOL CListItem::Create(HWND hListCtrl, UINT& uId, const RECT& rc, classify::CItem* clsItem)
{
    RECT rcCtrl;
    rcCtrl.top = rc.top + _margin_unit.GetPixelCy();
    rcCtrl.bottom = rcCtrl.top + _caption_unit.GetPixelCy();
    rcCtrl.left = rc.left + _margin_unit.GetPixelCx();
    rcCtrl.right = rc.right - 2 * _margin_unit.GetPixelCx();

    _hCaption = CreateStaticControl(hListCtrl, uId++, rcCtrl);
    if(_hCaption == NULL) {
        return FALSE;
    }
    std::wstring wsCaption = clsItem->GetDisplayName();
    if(!clsItem->IsMandatory()) {
        wsCaption += L" (Optional)";
    }
    SetWindowTextW(_hCaption, wsCaption.c_str());

    if(clsItem->IsMultiSelection()) {
        for(int i=0; i<(int)clsItem->GetValues().size(); i++) {
            HWND hSubItem = NULL;
            UINT uSubItemId = uId++;
            rcCtrl.top    = rcCtrl.bottom + _margin_unit.GetPixelCy()/2;
            rcCtrl.bottom = rcCtrl.top + _checkbox_unit.GetPixelCy();

			const std::wstring &wsText = clsItem->GetValues()[i].GetData();

            hSubItem = CreateCheckBoxControl(hListCtrl, wsText.c_str(), uSubItemId, rcCtrl);
            if(NULL == hSubItem) {
                return FALSE;
            }

			{
				wchar_t sz[1024];
				wsprintfW(sz, L"CListItem::CreateCheckBoxControl, %s\n", wsText.c_str());
				OutputDebugStringW(sz);
			}

			EllipsizeWindowText(hSubItem, wsText);

            _vItems.push_back(std::pair<UINT,HWND>(uSubItemId, hSubItem));
            if(clsItem->IsSelected(i)) {
                Button_SetCheck(hSubItem, BST_CHECKED);
            }
			unsigned long uDefaultValue = 0x00000001 << i;		
			if (clsItem->GetDefaultValue()&uDefaultValue)
			{
				Button_SetCheck(hSubItem, BST_CHECKED);
				clsItem->Select(i);
			}
        }
    }
    else {
        HWND hSubItem = NULL;
        UINT uSubItemId = uId++;
        rcCtrl.top    = rcCtrl.bottom + _margin_unit.GetPixelCy()/2;
        rcCtrl.bottom = rcCtrl.top + _combobox_unit.GetPixelCy();
        hSubItem = CreateComboBoxControl(hListCtrl, uSubItemId, rcCtrl);
        if(NULL == hSubItem) {
            return FALSE;
        }

		clsItem->SetComboBox(hSubItem);
        _vItems.push_back(std::pair<UINT,HWND>(uSubItemId, hSubItem));

	    ComboBox_AddString(hSubItem, UNSELECTED_STR);

        for(int i=0; i<(int)clsItem->GetValues().size(); i++) {
            ComboBox_AddString(hSubItem, clsItem->GetValues()[i].GetData().c_str());
        }

		//int value
		int nInitValue= clsItem->GetSelectId();
		int nDefaultValue = clsItem->GetDefaultValue();

		nDefaultValue += 1; //because the first value of the commobox is  UNSELECTED_STR
		if (nInitValue != -1)
		{
			nInitValue += 1; ////because the first value of the commobox is  UNSELECTED_STR
		}
		
		ComboBox_SetCurSel(hSubItem, nInitValue==-1 ? nDefaultValue : nInitValue );
		PostMessage(hListCtrl, WM_COMMAND, MAKELPARAM(uSubItemId,CBN_SELCHANGE), (LPARAM)hSubItem);
		//std::wstring strDefauleValue = clsItem->GetValues().at(nDefaultValue).GetData();
	//	ComboBox_SelectString(hSubItem, 0, strDefauleValue.c_str() );
    }

    _clsitem = clsItem;
    return TRUE;
}

void CListItem::Destroy()
{
    if(NULL != _hCaption) {
        ::ShowWindow(_hCaption, SW_HIDE);
        ::DestroyWindow(_hCaption);
        _hCaption = NULL;
    }
    for(int i=0; i<(int)_vItems.size(); i++) {
        ::ShowWindow(_vItems[i].second, SW_HIDE);
        ::DestroyWindow(_vItems[i].second);
    }
    _vItems.clear();
}

void CListItem::Show(BOOL bShow, RECT& rc)
{
    RECT rcCtrl;
    rcCtrl.top = rc.top + _margin_unit.GetPixelCy();
    rcCtrl.bottom = rcCtrl.top + _caption_unit.GetPixelCy();
    rcCtrl.left = rc.left + _margin_unit.GetPixelCx();
    rcCtrl.right = rc.right - 2 * _margin_unit.GetPixelCx();

    SetWindowPos(_hCaption, NULL, rcCtrl.left, rcCtrl.top, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE | (bShow ? 0 :SWP_HIDEWINDOW));

    if(_clsitem->IsMultiSelection()) {
        for(int i=0; i<(int)_vItems.size(); i++) {
            rcCtrl.top    = rcCtrl.bottom + _margin_unit.GetPixelCy()/2;
            rcCtrl.bottom = rcCtrl.top + _checkbox_unit.GetPixelCy();
			SetWindowPos(_vItems[i].second, NULL, rcCtrl.left, rcCtrl.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | (bShow ? 0 : SWP_HIDEWINDOW));
        }
    }
    else {
        if(_vItems.size() > 0) {
            rcCtrl.top    = rcCtrl.bottom + _margin_unit.GetPixelCy()/2;
            rcCtrl.bottom = rcCtrl.top + _combobox_unit.GetPixelCy();
			SetWindowPos(_vItems[0].second, NULL, rcCtrl.left, rcCtrl.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | (bShow ? 0 : SWP_HIDEWINDOW));
        }
    }
}

void CListItem::SetReadOnly(bool readonly)
{
    for(std::vector<std::pair<UINT,HWND>>::iterator it=_vItems.begin(); it!= _vItems.end(); ++it) {
        EnableWindow((*it).second, readonly?FALSE:TRUE);
    }
}

HWND CListItem::CreateStaticControl(HWND hListCtrl, UINT uId, const RECT& rc)
{
    HWND hWnd = NULL;
    int x  = rc.left;
    int y  = rc.top;
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;

    
    hWnd = CreateWindowExW(0,
                           L"STATIC",       // predefined class 
                           NULL,            // no window name 
                           WS_CHILD | SS_SIMPLE | WS_VISIBLE,
                           x, y, cx, cy,
                           hListCtrl,       // parent window 
                           (HMENU)(ULONG_PTR)uId,      // this control ID 
                           (HINSTANCE)(ULONG_PTR)GetWindowLongW(hListCtrl, GWLP_HINSTANCE),
                           NULL);           // pointer not needed
    
    HFONT hFont = _hTitleFont ? _hTitleFont : GetWindowFont(hListCtrl);
    SetWindowFont(hWnd, hFont, TRUE);
    return hWnd;

}

HWND CListItem::CreateComboBoxControl(HWND hListCtrl, UINT uId, const RECT& rc)
{
    HWND hWnd = NULL;
    int x = rc.left;
    int y = rc.top;
    int cx = rc.right - rc.left;
    int cy = 10 * (rc.bottom - rc.top);
    
    hWnd = CreateWindowExW(0,
                           WC_COMBOBOXW,    // predefined class 
                           NULL,            // no window name 
                           CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | ES_AUTOVSCROLL | WS_VISIBLE,
                           x, y, cx, cy,
                           hListCtrl,       // parent window 
                           (HMENU)(ULONG_PTR)uId,      // this control ID 
                           (HINSTANCE)(ULONG_PTR)GetWindowLongW(hListCtrl, GWLP_HINSTANCE),
                           NULL);           // pointer not needed

    HFONT hFont = GetWindowFont(hListCtrl);
    SetWindowFont(hWnd, hFont, TRUE);

    return hWnd;

}

HWND CListItem::CreateCheckBoxControl(HWND hListCtrl, LPCWSTR wzText, UINT uId, const RECT& rc)
{
    HWND hWnd = NULL;
    int x = rc.left;
    int y = rc.top;
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;
    
    hWnd = CreateWindowExW(WS_EX_TRANSPARENT,
                           L"Button",   // predefined class 
                           wzText,      // Text
                           WS_CHILD | BS_AUTOCHECKBOX | WS_VISIBLE,
                           x, y, cx, cy,
                           hListCtrl,   // parent window 
                           (HMENU)(ULONG_PTR)uId,  // this control ID 
                           (HINSTANCE)(ULONG_PTR)GetWindowLongW(hListCtrl, GWLP_HINSTANCE),
                           NULL);       // pointer not needed
    
    HFONT hFont = GetWindowFont(hListCtrl);
    SetWindowFont(hWnd, hFont, TRUE);
    return hWnd;

}



//
//  CClassifyListCtrl
//

CClassifyListCtrl::CClassifyListCtrl() : _hWnd(NULL), _uNextSubCtrlId(5000), pInsertingItem(NULL)
{
    _hbrWhite = CreateSolidBrush(RGB(255,255,255));
}

CClassifyListCtrl::~CClassifyListCtrl()
{
    if(NULL != _hbrWhite) {
        ::DeleteObject(_hbrWhite);
        _hbrWhite = NULL;
    }
}

void CClassifyListCtrl::Attach(_In_ HWND hWnd)
{
    _hWnd = hWnd;
    SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)this);
    _OldListProc = (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)NewListProc);
}

void CClassifyListCtrl::Detach()
{
    Clear();
    _hWnd = NULL;
}

BOOL CClassifyListCtrl::MandatoryCheck(_Out_ const classify::CItem** pEmptyItem)
{
    std::list<std::tr1::shared_ptr<CListItem>>::const_iterator it;
    for(it=_itemList.begin(); it!=_itemList.end(); ++it) {
        if((*it)->GetClassifyData()->IsMandatory()) {
            if((*it)->GetClassifyData()->IsMultiSelection()) {
                if((*it)->GetClassifyData()->GetSelectId() <= 0) {
                    *pEmptyItem = (*it)->GetClassifyData();
                    return FALSE;
                }
            }
            else {
                if((*it)->GetClassifyData()->GetSelectId() < 0) {
                    *pEmptyItem = (*it)->GetClassifyData();
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

void CClassifyListCtrl::GetResult(std::vector<std::pair<std::wstring,std::wstring>>& result)
{
    std::list<std::tr1::shared_ptr<CListItem>>::const_iterator it;
    for(it=_itemList.begin(); it!=_itemList.end(); ++it) {
        std::vector<const classify::CItemValue*> selValues;
        int nSelCount = (*it)->GetClassifyData()->GetSelectedValues(selValues);
        for(int i=0; i<nSelCount; i++) {
            result.push_back(std::pair<std::wstring,std::wstring>((*it)->GetClassifyData()->GetName(), selValues[i]->GetData()));
        }
    }
}

CListItem* CClassifyListCtrl::GetItem(int index)
{
    if(index < 0 || index >= (int)_itemList.size()) {
        return NULL;
    }

    std::list<std::tr1::shared_ptr<CListItem>>::iterator it = _itemList.begin();
    for(int i=0; i<index; i++) {
        ++it;
    }

    return (*it).get();
}

void CClassifyListCtrl::PositionItem(int index)
{
    RECT        rc;

    if(index < 0 || index >= (int)_itemList.size()) {
        return;
    }

    ListBox_GetItemRect(_hWnd, index, &rc);
    GetItem(index)->Show(TRUE, rc);
}

void CClassifyListCtrl::PositionAllItems()
{
    int i=0;
    std::list<std::tr1::shared_ptr<CListItem>>::iterator it = _itemList.begin();
    for(it = _itemList.begin(); it != _itemList.end(); ++it) {
        RECT rc;
        ListBox_GetItemRect(_hWnd, i++, &rc);
        (*it)->Show(rc.bottom==rc.top ? FALSE : TRUE, rc);
    }
    RECT rc;
    GetClientRect(_hWnd, &rc);
    InvalidateRect(_hWnd, &rc, TRUE);
}

void CClassifyListCtrl::OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct)
{
    //CListItem* pItem = GetItem((int)lpDrawItemStruct->itemID);
    //if(pItem) {
    //    pItem->Show(TRUE, lpDrawItemStruct->rcItem);
    //}
}

void CClassifyListCtrl::OnMeasureItem(MEASUREITEMSTRUCT* lpMeasureItem)
{
    if(NULL != pInsertingItem) {

		if (pInsertingItem->IsHideUI())
		{
			lpMeasureItem->itemHeight = 0;
		}
        else if(pInsertingItem->IsMultiSelection()) {
            lpMeasureItem->itemHeight  = _margin_unit.GetPixelCy();     // Top Margin
            lpMeasureItem->itemHeight += _caption_unit.GetPixelCy();    // Caption
            for(int i=0; i<(int)pInsertingItem->GetValues().size(); i++) {
                int inc = _margin_unit.GetPixelCy() / 2;                // Space
                inc += _checkbox_unit.GetPixelCy();                     // CheckBox
                if((lpMeasureItem->itemHeight + inc) > 255) {
                    // Exceed max height
                    break;
                }
                lpMeasureItem->itemHeight += inc;
            }
        }
        else {
				lpMeasureItem->itemHeight = _margin_unit.GetPixelCy();     // Top Margin
				lpMeasureItem->itemHeight += _caption_unit.GetPixelCy();    // Caption
				lpMeasureItem->itemHeight += _margin_unit.GetPixelCy() / 2; // Space
				lpMeasureItem->itemHeight += _combobox_unit.GetPixelCy();   // Combobox      
        }
    }
}

LRESULT CClassifyListCtrl::ListProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0;

    switch(message)
    {
    case WM_CTLCOLORSTATIC:
        {
            HDC hdcStatic = (HDC) wParam;
            SetTextColor(hdcStatic, RGB(0,0,0));
            SetBkColor(hdcStatic, RGB(255,255,255));
            return (INT_PTR)_hbrWhite;

        }
        break;

    case WM_MOUSEWHEEL:
        if(0 != GET_WHEEL_DELTA_WPARAM(wParam)) {
            lRet = CallWindowProcW(_OldListProc, hwnd, message, wParam, lParam);
            PositionAllItems();
            return lRet;
        }
        break;

    case WM_VSCROLL:
        lRet = CallWindowProcW(_OldListProc, hwnd, message, wParam, lParam);
        PositionAllItems();
        return lRet;

    case WM_COMMAND:
        {
            WORD notify = HIWORD(wParam);
            WORD subitem_id = LOWORD(wParam);
            if(CBN_SELCHANGE == notify) {
                lRet = CallWindowProcW(_OldListProc, hwnd, message, wParam, lParam);
                OnComboBoxSelectChange((UINT)subitem_id);
                return lRet;
            }
            else if (BN_CLICKED == notify) {
                lRet = CallWindowProcW(_OldListProc, hwnd, message, wParam, lParam);
                OnCheckBoxStateChange((UINT)subitem_id);
                return lRet;
            }
            else {
                ; // NOTHING
            }
        }
        break;

    case WM_LBUTTONDOWN:
        SetFocus(_hWnd);
        return 0L;

    default:
        break;
    }     

    return CallWindowProcW(_OldListProc, hwnd, message, wParam, lParam);
}

void  CClassifyListCtrl::OnComboBoxSelectChange(UINT dwCtrlId)
{
    int nItem = -1;
    int nSubItem = -1;
    if(!FindItemBySubControlId(dwCtrlId, &nItem, &nSubItem)) {
        return;
    }

    assert(nSubItem == 0);

    // Find target item
	std::list<std::tr1::shared_ptr<CListItem>>::iterator it = _itemList.begin();
    for(int i=0; i<nItem; i++) {
        ++it;
    }

    int nSel = ComboBox_GetCurSel((*it)->GetSubItems()[nSubItem].second);

    assert(it != _itemList.end());

    int nDelItem = nItem + 1;
    while(TRUE) {
		std::list<std::tr1::shared_ptr<CListItem>>::iterator itsub = it;
        ++itsub;
        if(itsub == _itemList.end()) {
            break;
        }
        if((*itsub)->GetLevel() <= (*it)->GetLevel()) {
            break;
        }
        _itemList.erase(itsub);
        ListBox_DeleteString(_hWnd, nDelItem);
    }

    // Okay, Add new sub items if necessary
	const classify::CItemValue* pClsValue = (*it)->GetClassifyData()->Select(nSel-1);
    int nNextItem = nItem + 1;
    if(NULL != pClsValue) {
        for(int i=0; i<(int)pClsValue->GetSubItems().size(); i++) {
            classify::CItem* item = _data->GetItem(pClsValue->GetSubItems()[i]);
            if(item == NULL) {
                // It should never be NULL
                return;
            }
           // item->Select(-1);	 keep old selected value
            InsertItem(nNextItem++, item, (*it)->GetLevel()+1);
        }
    }
    
    RECT rc;
    GetClientRect(_hWnd, &rc);
    InvalidateRect(_hWnd, &rc, TRUE);

    PositionAllItems();
}

void  CClassifyListCtrl::OnCheckBoxStateChange(UINT dwCtrlId)
{
    int nItem = -1;
    int nSubItem = -1;
    if(!FindItemBySubControlId(dwCtrlId, &nItem, &nSubItem)) {
        return;
    }

    assert(-1 != nItem);
    assert(-1 != nSubItem);
    
    // Find target item
	std::list<std::tr1::shared_ptr<CListItem>>::iterator it = _itemList.begin();
    for(int i=0; i<nItem; i++) {
        ++it;
    }

    assert((int)(*it)->GetSubItems().size() > nSubItem);
    assert((*it)->GetClassifyData()->IsMultiSelection());

    if(BST_CHECKED == Button_GetCheck((*it)->GetSubItems()[nSubItem].second)) {
        (*it)->GetClassifyData()->Select(nSubItem);
    }
    else {
        (*it)->GetClassifyData()->Unselect(nSubItem);
    }
}

BOOL CClassifyListCtrl::FindItemBySubControlId(_In_ UINT dwCtrlId, _Out_ int* pnItem, _Out_ int* pnSubItem)
{
    int i = -1;
	for (std::list<std::tr1::shared_ptr<CListItem>>::const_iterator it = _itemList.begin(); it != _itemList.end(); ++it) {
        i++;
        for(int j=0; j<(int)(*it)->GetSubItems().size(); j++) {
            if(dwCtrlId == (*it)->GetSubItems()[j].first) {
                // Good, found this sub-control
                *pnItem  = i;
                *pnSubItem = j;
                return TRUE;
            }
        }
    }

    *pnItem    = -1;
    *pnSubItem = -1;
    return FALSE;
}

BOOL CClassifyListCtrl::Init()
{
    if(NULL == _hWnd) {
        return FALSE;
    }
    
    _margin_unit.SetDluUnits(7, 7);
    _caption_unit.SetDluUnits(8, 8);
    _checkbox_unit.SetDluUnits(10, 10);
    _combobox_unit.SetDluUnits(14, 14);

    // Insert Top Level Item
    int currentIndex = 0;
    for(int i=0; i<(int)_data->GetEntryList().size(); i++) {
        classify::CItem* item = _data->GetItem(_data->GetEntryList()[i]);
        if(item == NULL) {
            continue;
        }
        currentIndex = InsertClassifyItem(currentIndex, 0, item);
    }

    return TRUE;
}

int CClassifyListCtrl::InsertClassifyItem(int index, int level, classify::CItem* pClsItem)
{
    if(InsertItem(index, pClsItem, level)) {
        index++;
        if(!pClsItem->IsMultiSelection() && pClsItem->IsSelected()) {
            const std::vector<std::wstring>& vSubItems = pClsItem->GetFirstSelectedValue()->GetSubItems();
            for(std::vector<std::wstring>::const_iterator it=vSubItems.begin(); it!=vSubItems.end(); ++it) {
                classify::CItem* pSubItem = _data->GetItem(*it);
                level++;
                if(NULL != pSubItem) {
                    index = InsertClassifyItem(index, level, pSubItem);
                }
            }
        }
    }

    return index;
}

BOOL CClassifyListCtrl::InsertItem(int index, classify::CItem* clsItem, int nLevel)
{
    RECT        rc;

	std::tr1::shared_ptr<CListItem> spItem(new CListItem(nLevel, _margin_unit, _caption_unit, _checkbox_unit, _combobox_unit));

    spItem->SetTitleFont(_hTitleFont);

    pInsertingItem = clsItem;
    ListBox_InsertString(_hWnd, index, clsItem->GetDisplayName().c_str());
    pInsertingItem = NULL;
    ListBox_GetItemRect(_hWnd, index, &rc);
    
    // Create SubItem
    if(!spItem->Create(_hWnd, _uNextSubCtrlId, rc, clsItem)) {
        return FALSE;
    }

    //// Insert
	std::list<std::tr1::shared_ptr<CListItem>>::iterator it = _itemList.begin();
    for(int i=0; i<index; i++) it++;
    _itemList.insert(it, spItem);

	if (!clsItem->IsEditable()) {
        spItem->SetReadOnly(true);
    }
    return TRUE;
}

void CClassifyListCtrl::RemoveItem(int index)
{
}

void CClassifyListCtrl::Clear()
{
}

LRESULT CALLBACK NewListProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CClassifyListCtrl* pList = (CClassifyListCtrl*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if(NULL == pList) {
        return FALSE;
    }

    return pList->ListProc(hwnd, message, wParam, lParam);
}
