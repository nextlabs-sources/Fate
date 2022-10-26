#pragma once
#include <string>
#include <list>


typedef struct tagStaticContentInfo
{
	std::wstring strText; //contains only a word or a space
	BOOL bIsHyperLink;
	BOOL bIsClicked;
	std::wstring strLinkUrl;
	RECT rcText;
}StaticContentInfo;

//class for change DC object
template<class HOBJ> class ChangeDCObj
{
public:
	ChangeDCObj(HDC hDC, HOBJ newObj)
	{
		m_hDC = hDC;
		m_hOldObj = (HOBJ)::SelectObject(hDC, newObj);
	}
	~ChangeDCObj()
	{
		::SelectObject(m_hDC, m_hOldObj);
	}

protected:
	HDC m_hDC;
	HOBJ m_hOldObj;
};
template<> class ChangeDCObj<COLORREF>
{
public:
	ChangeDCObj(HDC hDC, COLORREF newColor)
	{
		m_hDC = hDC;
		m_OldColor = ::SetTextColor(hDC, newColor);
	}
	~ChangeDCObj()
	{
		::SetTextColor(m_hDC, m_OldColor);
	}

protected:
	HDC m_hDC;
	COLORREF m_OldColor;
};

class CHyperLinkStatic
{
public:
	CHyperLinkStatic();
	~CHyperLinkStatic();

	//public method
public:
	BOOL Attach(HWND hWnd);
	BOOL Create(HWND hParent, const WCHAR* wszText, int nX, int nY, int nWidth, int nHeight);
	void SetWindowText(const WCHAR* wszText);
	void MoveWindow(int nX, int nY, int nWidth, int nHeight);
	HWND GetHWND(){ return m_hWnd; }
	LRESULT OnDrawItem(DRAWITEMSTRUCT* pDrawStruct);

	//protected method
protected:
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT ProcessWndMessage(UINT message, WPARAM wParam, LPARAM lParam);
	void OnMoseMove(WPARAM wParam, LPARAM lParam);
	void OnLButtonDown(WPARAM wParam, LPARAM lParam);
	void CreateFont(HWND hParent);
	void ExtractContentInfo();
	void ExtractContentInfo(const std::wstring& strText, BOOL bIsHyperLink=FALSE, const std::wstring& strUrl=L"");
	void ExtractHyperlinkContentInfo(const std::wstring& strText);
	void AddContentInfo(const std::wstring& strText, BOOL bHyperLink, const std::wstring& strLinkUrl=L"");
	void ClearContentInfo();
	StaticContentInfo* GetContentInfoByMousePos(int nX, int nY);
	void SetNeighborContentToClicked(StaticContentInfo* pContentInfo);


protected:
	HWND m_hWnd;
	std::wstring m_strText; 
	WNDPROC m_wndProcOld;
	RECT m_rcWindow;
	std::list<StaticContentInfo*> m_lstContentInfo;

	static HFONT m_hFontDefault; 
	static COLORREF m_clrDefault;
	static HFONT m_hFontLinkInit;
	static COLORREF m_clrLinkInit;
	static COLORREF m_clrLinkClicked;
};

