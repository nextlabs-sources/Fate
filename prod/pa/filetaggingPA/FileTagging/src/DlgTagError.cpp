#include "DlgTagError.h"

extern HINSTANCE g_hInstance;

CDlgTagError::CDlgTagError(DWORD dwDlgID, const wchar_t* wszMessage)
{
	m_dwDlgID = dwDlgID;
	m_strMessage = wszMessage;
}

CDlgTagError::~CDlgTagError()
{

}

int CDlgTagError::DoModal(HWND hParent /* = NULL */)
{
	return (int)::DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(m_dwDlgID), hParent, DlgProc, (LPARAM)this);
}

INT_PTR CDlgTagError::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDlgTagError*   pDlg = NULL;

	if (WM_INITDIALOG == uMsg)
	{
		SetWindowLongPtrW(hWnd, DWLP_USER, (LONG_PTR)lParam);
		pDlg = reinterpret_cast<CDlgTagError*>(lParam);
		if (NULL == pDlg)
		{
			return FALSE;
		}
		pDlg->SetHWnd(hWnd);
	}
	else
	{
		pDlg = reinterpret_cast<CDlgTagError*>(GetWindowLongPtrW(hWnd, DWLP_USER));
	}

	if (NULL == pDlg)
	{
		return FALSE;
	}

	return pDlg->MessageHandler(hWnd, uMsg, wParam, lParam);
}

INT_PTR CDlgTagError::MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
		case IDCLOSE:
			::EndDialog(m_hWnd, IDCLOSE);
			nRet = 1;
			break;
		}
		break;

	case WM_DRAWITEM:
		nRet = OnOwnerDraw((DRAWITEMSTRUCT*)lParam);
		break;
	}

	return nRet;
}

BOOL CDlgTagError::OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct)
{
	if (IDC_STATIC_MESSAGE == lpDrawItemStruct->CtlID)
	{
		m_staticMessage.OnDrawItem(lpDrawItemStruct);
		return TRUE;
	}

	return FALSE;

}

void CDlgTagError::OnInitDialog()
{
	m_staticMessage.Attach(GetDlgItem(m_hWnd, IDC_STATIC_MESSAGE));
	m_staticMessage.SetWindowText(m_strMessage.c_str());
}

void CDlgTagError::OnOK()
{
	::EndDialog(m_hWnd, IDOK);
}