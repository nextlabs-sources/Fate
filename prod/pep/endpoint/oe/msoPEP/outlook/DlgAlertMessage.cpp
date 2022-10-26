#include "StdAfx.h"
#include "DlgAlertMessage.h"
#include <windowsx.h>
#include "../adaptercomm/include/adaptercomm.h"
#include "DataType.h"

#pragma comment(lib, "comctl32.lib")

// Enable Visual Style [Getting alpha blending to work with CImageList](https://stackoverflow.com/questions/40097312/getting-alpha-blending-to-work-with-cimagelist)
//[Why am I getting black background when displaying a list-view icon?](https://stackoverflow.com/questions/24269418/why-am-i-getting-black-background-when-displaying-a-list-view-icon#answer-41557745)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

extern HINSTANCE g_hInstance;

CDlgAlertMessage::CDlgAlertMessage(DWORD dwDlgID, bool bAllow, const AlertMsgs& alertMsg)
{
	m_dwDlgID = dwDlgID;
	m_bAllow = bAllow;
	hSmallImageList = NULL;

	m_alertMessages = &alertMsg;
}

CDlgAlertMessage::~CDlgAlertMessage()
{
}

int CDlgAlertMessage::DoModal(HWND hParent /* = NULL */)
{
	return (int)::DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(m_dwDlgID), hParent, DlgProc, (LPARAM)this);
}

INT_PTR CDlgAlertMessage::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDlgAlertMessage*   pDlg = NULL;

	if (WM_INITDIALOG == uMsg)
	{
#if defined(_WIN64)
	  SetWindowLongPtrW(hWnd, DWLP_USER, (LONG_PTR)lParam);
#else
       SetWindowLongW(hWnd, DWL_USER, (LONG)lParam);
#endif

		pDlg = reinterpret_cast<CDlgAlertMessage*>(lParam);
		if (NULL == pDlg)
		{
			return FALSE;
		}
		pDlg->SetHWnd(hWnd);
	}
	else
	{
#if defined(_WIN64)
		pDlg = reinterpret_cast<CDlgAlertMessage*>(GetWindowLongPtrW(hWnd, DWLP_USER));
#else
		pDlg = reinterpret_cast<CDlgAlertMessage*>((LONG_PTR)GetWindowLongW(hWnd, DWL_USER));
#endif

	}

	if (NULL == pDlg)
	{
		return FALSE;
	}

	return pDlg->MessageHandler(hWnd, uMsg, wParam, lParam);
}

INT_PTR CDlgAlertMessage::MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	hWnd = hWnd;// remove warning message
	INT_PTR nRet = 0;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		OnInitDialog();
		nRet = 1;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			OnOK();
			nRet = 1;
			break;
		case IDCANCEL:
			::EndDialog(m_hWnd, IDCANCEL);
			nRet = 0;
			break;
		case IDCLOSE:
			::EndDialog(m_hWnd, IDCLOSE);
			nRet = 1;
			break;
		}
		break;
	case WM_DESTROY:
		if (hSmallImageList)
		{
			ImageList_Destroy(hSmallImageList);
		}
		break;
	case WM_DRAWITEM:
		nRet = OnOwnerDraw((DRAWITEMSTRUCT*)lParam);
		break;

	//case WM_MEASUREITEM:
	//{
	//					   MEASUREITEMSTRUCT* pMeasureItem = (MEASUREITEMSTRUCT*)lParam;
	//					   if (IDC_LIST_MESSAGE == pMeasureItem->CtlID)
	//					   {
	//						   return   m_ListViewMessage.OnMeasureItem(pMeasureItem);
	//					   }
	//}
		break;
	case WM_CTLCOLORDLG: // fall through
	case WM_CTLCOLORSTATIC:
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
	}

	return nRet;
}

BOOL CDlgAlertMessage::OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct)
{
	 if(IDC_STATIC_HEADER == lpDrawItemStruct->CtlID)
	{
		m_staticHeader.OnDrawItem(lpDrawItemStruct);
		return TRUE;
	}
	 else if(IDC_STATIC_TIP == lpDrawItemStruct->CtlID)
	 {
		 m_staticTip.OnDrawItem(lpDrawItemStruct);
		 return TRUE;
	 }
	 //else if (IDC_LIST_MESSAGE == lpDrawItemStruct->CtlID)
	 //{
		// return TRUE;
	 //}

	return FALSE;

}

// Put #vecMessages->first into #allowedMessages if not exists
static void PutMessages(std::vector<std::wstring> & allowedMessages, AlertMessages::Iteratable & vecMessages)
{
	for(AlertMessages::Iteratable::iterator itAlertMsg = vecMessages.begin(); itAlertMsg != vecMessages.end(); ++itAlertMsg)
	{
		if (std::find(allowedMessages.begin(), allowedMessages.end(), itAlertMsg->first) == allowedMessages.end()) 
		{
			allowedMessages.push_back(itAlertMsg->first);
		}
	}
}

void CDlgAlertMessage::OnInitDialog()
{	
	m_staticTip.Attach(GetDlgItem(m_hWnd, IDC_STATIC_TIP));
	m_staticHeader.Attach(GetDlgItem(m_hWnd, IDC_STATIC_HEADER));
	//m_staticHeader.SetWindowText(m_strHeader.c_str());
	//m_ListViewMessage.Attach(GetDlgItem(m_hWnd, IDC_LIST_MESSAGE));
	//m_ListViewMessage.SetWindowTextW(m_strMessage.c_str());

	if (m_bAllow)
	{
		SetDlgItemText(m_hWnd, IDOK, _T("Proceed"));
		m_staticHeader.SetWindowText(L"<b>Please read the following messages regarding the email you are about to send.</b>");
		m_staticTip.SetWindowText(L"Click <b>Proceed</b> to send the email.\nClick <b>Cancel</b> to stop the transaction.");
	}else
	{
		ShowWindow(GetDlgItem(m_hWnd, IDCANCEL), SW_HIDE);
		m_staticHeader.SetWindowText(L"<b>You are not allowed to send this email message.</b>");
		m_staticTip.SetWindowText(L"Please make the necessary adjustments and try again.");
	}

	HWND hWndListView = GetDlgItem(m_hWnd, IDC_LIST1);

	// long style = ::GetWindowLong(hWndListView, GWL_STYLE);
	// style |= LVS_REPORT | LVS_NOCOLUMNHEADER;
	// ::SetWindowLong(hWndListView,  GWL_STYLE, style);

	// http://stackoverflow.com/questions/2398746/removing-window-border
	// It's also best to remove the extended border styles.
	LONG lExStyle = ::GetWindowLong(hWndListView, GWL_EXSTYLE);
	lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	::SetWindowLong(hWndListView, GWL_EXSTYLE, lExStyle);

	//And finally, to get your window to redraw with the changed styles, you can use SetWindowPos.
	::SetWindowPos(hWndListView, NULL, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);


	PrepareImages();

	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_FMT | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = (LPWSTR)L"Affected Items";
	(int)::SendMessage(hWndListView, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
	
	AddListViewItems(hWndListView, *m_alertMessages);

	////set icon
	//m_hTipIcon = ::LoadIconW(NULL, MAKEINTRESOURCEW(m_bAllow ? IDI_INFORMATION : IDI_ERROR));
	//Static_SetIcon(GetDlgItem(m_hWnd, IDC_TIP_ICON), m_hTipIcon);

	// https://msdn.microsoft.com/en-us/library/windows/desktop/bb761163(v=vs.85).aspx
	(BOOL)::SendMessage(hWndListView, LVM_SETCOLUMNWIDTH, (WPARAM)(int)(0), MAKELPARAM((LVSCW_AUTOSIZE_USEHEADER), 0));
	//ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER );//rc.right - rc.left - nScrollBarWidth
}

void CDlgAlertMessage::PrepareImages()
{
	const int nSM_CXSMICON = 16, nSM_CYSMICON = 16;

	//// Set dialog icon
	HICON hSmallIcon = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, nSM_CXSMICON, nSM_CYSMICON, LR_DEFAULTCOLOR | LR_SHARED);
	if (NULL != hSmallIcon)
	{
		(HICON)::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hSmallIcon);
	}else
	{
		loge(L"Set the small icon in the window caption, GetLastError=%#x", GetLastError());
	}

	HWND hWndListView = GetDlgItem(m_hWnd, IDC_LIST1);

	hSmallImageList = ImageList_Create(nSM_CXSMICON, nSM_CYSMICON, ILC_COLOR32, 2, 1);

	HICON hIconWarn = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_ALERT_WARN), IMAGE_ICON, nSM_CXSMICON, nSM_CYSMICON, LR_DEFAULTCOLOR | LR_SHARED );
	HICON hIconCross = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_ALERT_CROSS), IMAGE_ICON, nSM_CXSMICON, nSM_CYSMICON, LR_DEFAULTCOLOR | LR_SHARED );

	int nWarnIndex = ImageList_AddIcon(hSmallImageList, hIconWarn);
	int nCrossIndex = ImageList_AddIcon(hSmallImageList, hIconCross);

	logd(L"[PrepareImages]Image: Warn{HICON:%#x, Index:%d}, Cross{HICON:%#x, Index:%d}, GetLastError=%#x", hIconWarn, nWarnIndex, hIconCross, nCrossIndex, GetLastError());

	DestroyIcon(hIconWarn);
	DestroyIcon(hIconCross);

	// Assign the image lists to the list-view control. 
	ListView_SetImageList(hWndListView, hSmallImageList, LVSIL_SMALL); 
}

void CDlgAlertMessage::AddListViewItems(HWND hWndListView, const AlertMsgs &msgs)
{
	const std::vector<AlertMsg> & vecMessages = msgs.Messages();
	logd(L"[FillListItems] message amount is %d", msgs.size());
	int nItem = ListView_GetItemCount(hWndListView), nRet;
	std::wstring itemObjectText;
	for(std::vector<AlertMsg>::const_iterator itAlertMsg = vecMessages.begin(); itAlertMsg != vecMessages.end(); ++itAlertMsg)
	{
		const AlertMsg & message = *itAlertMsg;
		const std::vector<std::wstring> objects = message.items;
		LV_ITEMW lvitem = { 0 };

		lvitem.mask = LVIF_IMAGE|LVIF_TEXT;

		if (nItem > 0)
		{
			lvitem.iImage = -2;
			lvitem.iItem = nItem;
			lvitem.iSubItem = 0;
			lvitem.pszText = L"";
			nRet = (int)::SendMessage(hWndListView, LVM_INSERTITEMW, 0, (LPARAM)(const LV_ITEM *)(&lvitem));

			logd(L"[InsertListItems](LVM_INSERTITEMW returned %d) separator nItem=%d", nRet, nItem);

			++nItem;
		}

		lvitem.iImage = message.allowed ? 0 : 1;
		lvitem.iItem = nItem;
		lvitem.iSubItem = 0;
		lvitem.pszText = const_cast<wchar_t*>(message.text.c_str());
		//ListView_InsertItem(hWndListView, &lvitem);
		nRet = (int)::SendMessage(hWndListView, LVM_INSERTITEMW, 0, (LPARAM)(const LV_ITEM *)(&lvitem));
		logd(L"[InsertListItems](LVM_INSERTITEMW returned %d)Alert Obigation: %s", nRet, lvitem.pszText);

		++nItem;
		for (std::vector<std::wstring>::const_iterator itObject = objects.begin(); objects.end() != itObject; ++itObject)
		{
			itemObjectText = L"- ";
			itemObjectText.append(*itObject);

			lvitem.iImage = -2;
			lvitem.iItem = nItem;
			lvitem.iSubItem = 0;
			lvitem.pszText = const_cast<wchar_t*>(itemObjectText.c_str());
			// ListView_InsertItem(hWndListView, &lvitem);
			nRet = (int)::SendMessage(hWndListView, LVM_INSERTITEMW, 0, (LPARAM)(const LV_ITEM *)(&lvitem));

			++nItem;

			logd(L"[InsertListItems](LVM_INSERTITEMW returned %d)Detail object: %s", nRet, lvitem.pszText);
		}
	}
}

void CDlgAlertMessage::OnOK()
{
	::EndDialog(m_hWnd, IDOK);
}