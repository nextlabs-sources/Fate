#pragma once
#include"HyperLinkStatic.h"
// CSCrollStatic is a List Box control that contains a static window.
class CSCrollStatic
{
public:
	CSCrollStatic();
	~CSCrollStatic();
	BOOL Attach(HWND hWnd);
	BOOL Create(HWND hParent, int nX, int nY, int nWidth, int nHeight);
	void SetWindowText(const wchar_t* wszText);
	LRESULT OnMeasureItem(MEASUREITEMSTRUCT* pMeasureItem);

protected:
	BOOL CreateEmbedStatic();
	static LRESULT CALLBACK ListControlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT ProcessWndMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void PositionTextWindow();
	LRESULT OnOwneDraw(DRAWITEMSTRUCT* pDrawItemStruct);


private:
	CHyperLinkStatic m_hyperSatic;
	HWND m_hWnd;
	std::wstring m_wstrText;
	WNDPROC m_wndProcOld;
};

