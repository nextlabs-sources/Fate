#pragma once
#include <windows.h>
#include <string>
#include "HyperLinkStatic.h"
#include "resource.h"

class CDlgTagError
{
public:
	CDlgTagError(DWORD dwDlgID, const wchar_t* wszMessage);
	~CDlgTagError();
	int DoModal(HWND hParent = NULL);

protected:
	static INT_PTR WINAPI DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnOK();
	void OnInitDialog();
	BOOL OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct);
	void SetHWnd(HWND hWnd) { m_hWnd = hWnd; }

private:
	std::wstring m_strMessage;
	CHyperLinkStatic m_staticMessage;
	DWORD m_dwDlgID;
	HWND m_hWnd;
};

