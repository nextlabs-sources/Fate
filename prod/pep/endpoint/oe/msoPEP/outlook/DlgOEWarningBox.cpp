#include "stdafx.h"
#include "DlgOEWarningBox.h"
#include <windowsx.h>
#include "resource.h"
#include "../adaptercomm/include/adaptercomm.h"

extern HINSTANCE g_hInstance;

CDlgOEWarningBox::CDlgOEWarningBox(DWORD dwDlgID, const wchar_t* szHeader, const wchar_t* wszMessage, const wchar_t* wszOkButtonTitle, const wchar_t* wszCancelButtonTitle):
m_dwDlgID(dwDlgID), m_strOKButtonTitle(wszOkButtonTitle), m_strCancelButtonTitle(wszCancelButtonTitle)
{
	m_hWnd = NULL;
	m_strActionDesc = szHeader;
	AdapterCommon::StringReplace(m_strActionDesc, L"<br>", L"\n");
	SetMessage(wszMessage);
}


CDlgOEWarningBox::~CDlgOEWarningBox()
{
}

void CDlgOEWarningBox::SetMessage(const wchar_t* wszMessage)
{
   m_strMessage = wszMessage;
   AdapterCommon::StringReplace(m_strMessage, L"<br>", L"\n");
   if (std::wstring::npos == m_strMessage.find('\n')){
	   logd(L"[CDlgOEWarningBox::SetMessage]m_strMessage=%s", m_strMessage.c_str());
	   m_strMessage += '\n';
   }
}

int CDlgOEWarningBox::DoModal(HWND hParent/* =NULL */)
{
	return (int)::DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(m_dwDlgID), hParent, DlgProc, (LPARAM)this);
}

INT_PTR CDlgOEWarningBox::MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd);
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
			OnCancel();
			nRet = 0;
			break;
		default:
			break;
		}
		break;

	case WM_NOTIFY:
		nRet = OnNotify((LPNMHDR)lParam);
		break;

	case WM_DRAWITEM:
		nRet = OnOwnerDraw((DRAWITEMSTRUCT*)lParam);
		break;

	case WM_MEASUREITEM:
	{
						   MEASUREITEMSTRUCT* pMeasureItem = (MEASUREITEMSTRUCT*)lParam;
						   if (IDC_LIST_MESSAGE == pMeasureItem->CtlID)
						   {
							   return   m_ListViewMessage.OnMeasureItem(pMeasureItem);
						   }
	}
		break;
	}

	return nRet;
}

INT_PTR CDlgOEWarningBox::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDlgOEWarningBox*   pDlg = NULL;

	if (WM_INITDIALOG == uMsg)
	{
#if defined(_WIN64)
		SetWindowLongPtrW(hWnd, DWLP_USER, (LONG_PTR)lParam);
#else
		SetWindowLongPtrW(hWnd, DWL_USER, (LONG)lParam);
#endif
	   pDlg = reinterpret_cast<CDlgOEWarningBox*>(lParam);
		if (NULL == pDlg)
		{
			return FALSE;
		}
		pDlg->SetHWnd(hWnd);
	}
	else
	{
#if defined(_WIN64)
		pDlg = reinterpret_cast<CDlgOEWarningBox*>(GetWindowLongPtrW(hWnd, DWLP_USER));
#else
		pDlg = reinterpret_cast<CDlgOEWarningBox*>((INT_PTR)GetWindowLongPtrW(hWnd, DWL_USER));
#endif
	}

	if (NULL == pDlg)
	{
		return FALSE;
	}

	return pDlg->MessageHandler(hWnd, uMsg, wParam, lParam);
}

void CDlgOEWarningBox::OnOK()
{
	::EndDialog(m_hWnd, IDOK);
}

void CDlgOEWarningBox::OnCancel()
{
	::EndDialog(m_hWnd, IDCANCEL);
}

void CDlgOEWarningBox::OnInitDialog()
{	m_ListViewMessage.Attach(GetDlgItem(m_hWnd, IDC_LIST_MESSAGE));
	m_ListViewMessage.SetWindowTextW(m_strMessage.c_str());
	
	m_staticActionDesc.Attach(GetDlgItem(m_hWnd, IDC_STATIC_ACTION_DESC));
	m_staticActionDesc.SetWindowTextW(m_strActionDesc.c_str());

	SetDlgItemTextW(m_hWnd, IDOK, m_strOKButtonTitle.c_str());
	SetDlgItemTextW(m_hWnd, IDCANCEL, m_strCancelButtonTitle.c_str());

	//set icon
	m_hTipIcon = ::LoadIconW(NULL, MAKEINTRESOURCEW(IDI_WARNING));
	Static_SetIcon(GetDlgItem(m_hWnd, IDC_TIP_ICON), m_hTipIcon);
}


int CDlgOEWarningBox::OnNotify(_In_ LPNMHDR lpnmhdr)
{
	UNREFERENCED_PARAMETER(lpnmhdr);
	return 1;
}

BOOL CDlgOEWarningBox::OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct)
{
  if (IDC_LIST_MESSAGE == lpDrawItemStruct->CtlID)
	{
		return TRUE;
	}
	else if (IDC_STATIC_ACTION_DESC == lpDrawItemStruct->CtlID)
	{
		m_staticActionDesc.OnDrawItem(lpDrawItemStruct);
	}

	return FALSE;
}
