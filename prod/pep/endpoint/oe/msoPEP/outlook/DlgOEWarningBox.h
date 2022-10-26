#pragma once
#include <windows.h>
#include <string>
#include "HyperLinkStatic.h"
#include "SCrollStatic.h"
class CDlgOEWarningBox
{
public:
	CDlgOEWarningBox(DWORD dwDlgID, const wchar_t* szHeader, const wchar_t* wszMessage, const wchar_t* wszOkButtonTitle, const wchar_t* wszCancelButtonTitle);
	~CDlgOEWarningBox();
	int DoModal(HWND hParent=NULL);

protected:
	static INT_PTR WINAPI DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnOK();
	void OnCancel();
	void OnInitDialog();
	int OnNotify(_In_ LPNMHDR lpnmhdr);
	BOOL OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct);
	void SetMessage(const wchar_t* wszMessage);
	void SetHWnd(HWND hWnd) { m_hWnd = hWnd; }

private:
	std::wstring m_strMessage;
	std::wstring m_strActionDesc;
	std::wstring m_strOKButtonTitle;
	std::wstring m_strCancelButtonTitle;

	CSCrollStatic m_ListViewMessage;
	CHyperLinkStatic m_staticActionDesc;

	DWORD m_dwDlgID;
	HWND m_hWnd;
	HICON m_hTipIcon;
};

