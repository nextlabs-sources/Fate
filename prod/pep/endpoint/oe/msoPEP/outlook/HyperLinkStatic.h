#pragma once
#include <string>
#include <list>


typedef struct tagStaticContentInfo
{
	std::wstring strText; //contains only a word or a space
	BOOL bIsHyperLink;
	BOOL bIsClicked;
	BOOL bBold;
	std::wstring strLinkUrl;
	RECT rcText;
	HFONT hFont;
	COLORREF colorText;
	tagStaticContentInfo()
	{
		bIsHyperLink = FALSE;
		bIsClicked = FALSE;
		bBold = FALSE;
		rcText.left = rcText.top = rcText.right = rcText.bottom = 0;
		hFont = NULL;
		colorText = RGB(0, 0, 0);
	}
}StaticContentInfo;

enum TEXT_FORMAT
{
	TEXT_FORMAT_NONE = 0,
	TEXT_FORMAT_HYPERLINK=1,
	TEXT_FORMAT_BOLD,
};

typedef struct tagTextFormatInfo
{
	TEXT_FORMAT fmtType;
	std::wstring wstrFormatBegin;
	std::wstring wstrFormatEnd; 
}TextFormatInfo;

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
	void SetFirstWordBoldFont(BOOL bBold);
	int  GetTextLineCount(){ return m_nLineCount; }
	void SetYExtend(BOOL bExtend){ m_bYExtend = bExtend; };
	BOOL GetYExtend() { return m_bYExtend; }
	int GetLineHeight(){return m_nLineHeight;}
	//protected method
protected:
	void Init();
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT ProcessWndMessage(UINT message, WPARAM wParam, LPARAM lParam);
	void OnMoseMove(WPARAM wParam, LPARAM lParam);
	void OnLButtonDown(WPARAM wParam, LPARAM lParam);
	void CreateFont(HWND hParent);
	void ExtractContentInfo();
	void ExtractContentInfo(const std::wstring& strText, BOOL bIsHyperLink=FALSE, const std::wstring& strUrl=L"", BOOL bBold=FALSE);
	void ExtractHyperlinkContentInfo(const std::wstring& strText);
	void ExtractBoldFontContentInfo(const std::wstring& strText);
	void AddContentInfo(const std::wstring& strText, BOOL bHyperLink, const std::wstring& strLinkUrl = L"", BOOL bBold = FALSE);
	void ClearContentInfo();
	StaticContentInfo* GetContentInfoByMousePos(int nX, int nY);
	void SetNeighborContentToClicked(StaticContentInfo* pContentInfo);
	void CalculateContnetPosition();
	BOOL FindFormatText(const std::wstring& refWstrText, size_t& nFindPos, std::wstring& refWstrTextBeforeFmtText, std::wstring& refFmtText, TEXT_FORMAT& refTextFmt);

protected:
	HWND m_hWnd;
	std::wstring m_strText; 
	int m_nLineCount;
	int m_nLineHeight;
	WNDPROC m_wndProcOld;
	RECT m_rcWindow;
	std::list<StaticContentInfo*> m_lstContentInfo;
    BOOL m_bFirstWordBold;
	BOOL m_bYExtend; //is the Line count of the content can be extend or limited by windows height;
    HBRUSH m_hbrBackground;

	static HFONT m_hFontDefault; 
	static HFONT m_hFontDefaultBold;
	static COLORREF m_clrDefault;
	static HFONT m_hFontLinkInit;
	static COLORREF m_clrLinkInit;
	static COLORREF m_clrLinkClicked;
	const static TextFormatInfo m_textFormatInfo[];
};

