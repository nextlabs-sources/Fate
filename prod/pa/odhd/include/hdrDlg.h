
#pragma once
#ifndef _HDR_DIALOG_H_
#define _HDR_DIALOG_H_
#include <Windows.h>
#include <stdio.h>
#include "YWinBase.h"
#include "YViewWnd.h"
#include "YViewBase.h"
#include "YViewCell.h"

/*
WPARAM wParam    HIWORD(wParam) -- the index of this cell item, base on zero
LOWORD(wParam) -- the control index in this cell item, base on zero
LPARAM lParam    -- the hwnd of this control
*/
#define WM_HDR_CELL_CLICK           (WM_USER+102)

#define HDR_WND_WIDTH               465
#define HDR_WND_HEIGHT              400
#define HDR_WND_MARGIN              10
#define HDR_WND_BTN_HEIGHT          22
#define HDR_WND_BTN_WIDTH           70
#define HDR_CLASS_NAME              L"CE HDR DIALOG"
#define HDR_INFO_01                 L"The following hidden information is found. Click Remove All under each type to remove the hidden information from the attachment."
#define HDR_INFO_02                 L"NOTE: Some changes cannot be undone!"
#define HDR_INFO_03                 L"Document inspection complete. No hidden data found. Click Send Email button to send or OK to return to email"
#define HDR_MAX_ICON_SIZE           32
#define HDR_SMALL_ICON_SIZE         16

#define HDR_ATTACHMENT_PREFIX       L"Attachment: "

#define HDR_CELL_BTN_WIDTH          68
#define HDR_CELL_BTN_HEIGHT         20
#define HDR_CELL_MARGIN             8



// From strcount.cpp
extern void WrapTextToFitWidth(LPCWSTR pwzText, std::wstring& strText, int nWidthLimit);

class HdrData
{
public:
    HdrData():m_strHeader(L""),m_strBody(L""),m_nStatus(NOHD){}
    HdrData(LPCWSTR pwzHeader, LPCWSTR pwzBody, int nStatus):m_strHeader(pwzHeader),m_strBody(pwzBody),m_nStatus(nStatus){}
    std::wstring    m_strHeader;
    std::wstring    m_strBody;
    int             m_nStatus;
};
typedef std::vector<YLIB::smart_ptr<HdrData>> HdrDataVector;

class HdrActionBase
{
public:
    HdrActionBase(){}
    virtual ~HdrActionBase(){m_vData.clear();}
    virtual void OnRemove(int nAttachmentIndex, int nItemIndex,
						  std::wstring& strBody, std::wstring& strNote,
						  int* pnStatus,
                          LPVOID pContext) = 0;
    virtual void OnNext(int nAttachmentIndex, std::wstring& strAttach,
                        std::wstring& strHelpUrl, LPVOID pContext) = 0;

    HdrDataVector   m_vData;
};


class HdrViewCell : public YViewCell
{
public:
    HdrViewCell(HWND hViewWnd, HDC hViewDC, int nWidth) : YViewCell(hViewWnd, hViewDC, nWidth){
        m_hBmpFail   = 0;
        m_hBmpHD     = 0;
        m_hBmpNoHD   = 0;
        m_hFontBold  = 0;
        m_hFontNormal= 0;
        m_hButton    = 0;
        m_hButton = CreateWindow(L"BUTTON", L"Remove All", WS_CHILD,
            0,
            0,
            HDR_CELL_BTN_WIDTH,
            HDR_CELL_BTN_HEIGHT,
            hViewWnd, NULL, YBaseWnd::get_Instance(), NULL);
        EnableWindow(m_hButton, FALSE);
    }
    ~HdrViewCell()
    {
        if(m_hButton) SendMessageW(m_hButton, WM_CLOSE, 0, 0);
    }

    int  get_Height()
    {
        return m_nHeight;
    }
    int  get_Width()
    {
        return m_nWidth;
    }
    int  prepare_Cell(int cyOffset)
    {
        int nHeaderHeight = 0;
        int nBodyHeight   = 0;
        m_nOffset = cyOffset;

        int nTextWidth= m_nWidth-HDR_CELL_MARGIN*3-HDR_SMALL_ICON_SIZE-HDR_CELL_BTN_WIDTH-YScrollBar::get_CXVScroll();
        nHeaderHeight = YLIB::YWinUtility::CountTextAreaHeightByWidth(m_strHeader.c_str(), m_hViewDC, m_hFontBold, nTextWidth, 0);
        nBodyHeight   = YLIB::YWinUtility::CountTextAreaHeightByWidth(m_strBody.c_str(), m_hViewDC, m_hFontBold, nTextWidth-HDR_CELL_MARGIN, 0);
        m_nHeight     = nHeaderHeight + HDR_CELL_MARGIN + nBodyHeight + HDR_CELL_MARGIN*2;

        // Icon rect
        m_rcFlag.left = HDR_CELL_MARGIN;            m_rcFlag.right = m_rcFlag.left + HDR_SMALL_ICON_SIZE;
        m_rcFlag.top  = cyOffset+HDR_CELL_MARGIN*2; m_rcFlag.bottom = m_rcFlag.top + HDR_SMALL_ICON_SIZE;

        // Header rect
        m_rcHeader.left = m_rcFlag.right+HDR_CELL_MARGIN;   m_rcHeader.right = m_rcHeader.left + nTextWidth;
        m_rcHeader.top  = m_rcFlag.top;                     m_rcHeader.bottom= m_rcHeader.top + nHeaderHeight;

        // Body rect
        m_rcBody.left = m_rcHeader.left+HDR_CELL_MARGIN;    m_rcBody.right = m_rcHeader.right;
        m_rcBody.top  = m_rcHeader.bottom+HDR_CELL_MARGIN/4;  m_rcBody.bottom= m_rcBody.top + nBodyHeight;

        // Button rect
        m_rcButton.right = m_nWidth - HDR_CELL_MARGIN - YScrollBar::get_CXVScroll() - HDR_CELL_MARGIN/2;
        m_rcButton.left  = m_rcButton.right - HDR_CELL_BTN_WIDTH;
        m_rcButton.top   = cyOffset+HDR_CELL_MARGIN*3;
        m_rcButton.bottom= m_rcButton.top + HDR_CELL_BTN_HEIGHT;
        MoveWindow(m_hButton, m_rcButton.left, m_rcButton.top, m_rcButton.right-m_rcButton.left, m_rcButton.bottom-m_rcButton.top, TRUE);
        EnableWindow(m_hButton, (NOHD==m_nStatus||REMOVEOK==m_nStatus||REMOVEFAIL==m_nStatus||INSPECTFAIL==m_nStatus)?FALSE:TRUE);
        ShowWindow(m_hButton, (NOHD==m_nStatus||INSPECTFAIL==m_nStatus)?FALSE:TRUE);
        if(m_hFontNormal) SendMessage(m_hButton, WM_SETFONT, (WPARAM)m_hFontNormal, (LPARAM)TRUE);

        return m_nHeight;
    }
    void update_button_status()
    {
        EnableWindow(m_hButton, (NOHD==m_nStatus||REMOVEOK==m_nStatus||REMOVEFAIL==m_nStatus||INSPECTFAIL==m_nStatus)?FALSE:TRUE);
    }
    void render_Cell()
    {
        render_Cell(FALSE);
    }
    void render_Cell(BOOL bRedrawBody)
    {
        HBITMAP hFlag    = NULL;
        HFONT   hOldFont = NULL;

        // draw splitter line
        if(!bRedrawBody)
        {
            HPEN  hPen = CreatePen(PS_SOLID, 1, RGB(220,220,220));
            POINT ptMove;
            HPEN  hOldPen = (HPEN)SelectObject(m_hViewDC, hPen);
            MoveToEx(m_hViewDC, HDR_CELL_MARGIN, m_nOffset, &ptMove);
            LineTo(m_hViewDC, m_nWidth-HDR_CELL_MARGIN*3-4, m_nOffset);
            SelectObject(m_hViewDC, hOldPen);
            DeleteObject(hPen);
        }

        // render Icon
        if(NOHD==m_nStatus || REMOVEOK==m_nStatus) hFlag = m_hBmpNoHD;
        else if(HAVEHD==m_nStatus||INSPECTFAIL==m_nStatus) hFlag = m_hBmpHD;
        else if(REMOVEFAIL==m_nStatus) hFlag = m_hBmpFail;
        else hFlag=0;
        if(NULL!=hFlag)
        {
            HDC     hFlagDc = CreateCompatibleDC(m_hViewDC);
            HBITMAP hOldFlag= (HBITMAP)SelectObject(hFlagDc, hFlag);
            BitBlt(m_hViewDC, m_rcFlag.left, m_rcFlag.top, HDR_SMALL_ICON_SIZE, HDR_SMALL_ICON_SIZE, hFlagDc, 0, 0, SRCCOPY);
            SelectObject(hFlagDc, hOldFlag);
            DeleteObject(hFlagDc);
        }

        // render header
        if(!bRedrawBody)
        {
            hOldFont = (HFONT)SelectObject(m_hViewDC, m_hFontBold);
            SetBkMode(m_hViewDC, TRANSPARENT);
            SetTextColor(m_hViewDC, RGB(64, 64, 64));
            DrawTextEx(m_hViewDC, (LPWSTR)m_strHeader.c_str(), -1, &m_rcHeader, DT_LEFT|DT_NOPREFIX/*|DT_WORDBREAK*/, NULL);
            SelectObject(m_hViewDC, hOldFont);
        }

        // render body
        if(bRedrawBody)
        {
            HBRUSH hb = ::CreateSolidBrush(RGB(255,255,255));
            if(hb)
            {
                HBRUSH hbold = (HBRUSH)::SelectObject(m_hViewDC, (HGDIOBJ)hb);
                ::FillRect(m_hViewDC, &m_rcBody, hb);
                SelectObject(m_hViewDC, (HGDIOBJ)hbold);
                ::DeleteObject((HGDIOBJ)hb);
            }
        }
        hOldFont = (HFONT)SelectObject(m_hViewDC, m_hFontNormal);
        SetBkMode(m_hViewDC, TRANSPARENT);
        if(REMOVEFAIL == m_nStatus)
            SetTextColor(m_hViewDC, RGB(192, 0, 0));
        else
            SetTextColor(m_hViewDC, RGB(64, 64, 64));
        DrawTextEx(m_hViewDC, (LPWSTR)m_strBody.c_str(), -1, &m_rcBody, DT_LEFT|DT_NOPREFIX|DT_BOTTOM|DT_WORDBREAK, NULL);
        SelectObject(m_hViewDC, hOldFont);
    }
    int  locate_Cell(int cxOffset, int cyOffset)
    {
        RECT rcPos; memset(&rcPos, 0, sizeof(RECT));
        if(m_hButton)
        {
            memcpy(&rcPos, &m_rcButton, sizeof(RECT));
            OffsetRect(&rcPos, 0-cxOffset, 0-cyOffset);
            SetWindowPos(m_hButton, HWND_BOTTOM, rcPos.left, rcPos.top, 0, 0, SWP_NOSIZE);
        }
        return m_nHeight;
    }
    WORD find_cellctrl(HWND hWnd)
    {
        if(m_hButton == hWnd)
            return 2;
        else
            return 0xFFFF;
    }

    //
public:
    void put_header(LPCWSTR  pwzHeader){
        m_strHeader = pwzHeader;
    }
    void put_body(LPCWSTR  pwzBody){
        m_strBody = pwzBody;
    }
    void put_status(int nStatus){
        m_nStatus = nStatus;
    }
    void put_normalfont(HFONT hFont){m_hFontNormal=hFont;}
    void put_boldlfont(HFONT hFont){m_hFontBold=hFont;}
    void put_bmpfail(HBITMAP hBmp){m_hBmpFail=hBmp;}
    void put_bmphd(HBITMAP hBmp){m_hBmpHD=hBmp;}
    void put_bmpnohd(HBITMAP hBmp){m_hBmpNoHD=hBmp;}

private:
    HBITMAP         m_hBmpFail;
    HBITMAP         m_hBmpHD;
    HBITMAP         m_hBmpNoHD;
    HFONT           m_hFontBold;
    HFONT           m_hFontNormal;
    HWND            m_hButton;
    std::wstring    m_strHeader;
    std::wstring    m_strBody;
    int             m_nStatus;

    RECT            m_rcFlag;
    RECT            m_rcHeader;
    RECT            m_rcBody;
    RECT            m_rcButton;
};

class HdrViewWnd : public YViewWnd<HdrViewCell>
{
public:
    HdrViewWnd()
    {
        m_hWhiteBrush = CreateSolidBrush(RGB(255,255,255));
    }
    virtual ~HdrViewWnd()
    {
        if(m_hWhiteBrush) DeleteObject(m_hWhiteBrush);
    }
    void AddItem(HBITMAP hBmpFail,
                HBITMAP hBmpHD,
                HBITMAP hBmpNoHD,
                HFONT hFontBold,
                HFONT hFontNormal,
                LPCWSTR pwzHeader,
                LPCWSTR pwzBody,
                int nStatus)
    {
        YLIB::smart_ptr<HdrViewCell> spItem( new HdrViewCell(m_hViewWnd, m_offscreenDC, HDR_WND_WIDTH-20));
        if(0==spItem.get()) return;
        spItem->put_bmpfail(hBmpFail);
        spItem->put_bmphd(hBmpHD);
        spItem->put_bmpnohd(hBmpNoHD);
        spItem->put_boldlfont(hFontBold);
        spItem->put_normalfont(hFontNormal);
        spItem->put_header(pwzHeader);
        spItem->put_body(pwzBody);
        spItem->put_status(nStatus);
        m_cells.push_back(spItem);
    }
    LRESULT OnCtrlClick(HWND hWnd, BOOL& bCalldefaultHandler)
    {
        DWORD wParam = FindCellCtrl(hWnd);
        if(0xFFFFFFFF!=wParam)
            return SendMessageW(m_hParent, WM_HDR_CELL_CLICK, (WPARAM)wParam, (LPARAM)hWnd);
        return 0L;
    }
    void UpdateItemStatus(int nItem, LPCWSTR pwzNewBody, int nNewStatus)
    {
        if(nItem >= (int)m_cells.size())
            return;

        RECT rc;
        YLIB::smart_ptr<HdrViewCell> spItem = m_cells[nItem];
        spItem->put_body(pwzNewBody);
        spItem->put_status(nNewStatus);
        spItem->render_Cell(TRUE);
        spItem->update_button_status();
        GetClientRect(m_hWnd, &rc);
        ::InvalidateRect(m_hWnd, &rc, TRUE);
    }

private:
    HBRUSH                  m_hWhiteBrush;
};

template<class T>
class HdrDialog : public YBaseWnd
{
public:
    HdrDialog(int nAttach, LPCWSTR pwzCaption=NULL,
              LPVOID pHdrActionContext=NULL):
        m_nAllAttach(nAttach), m_uResult(IDCANCEL), m_uMainIconID(0),
        m_pHdrActionContext(pHdrActionContext)
    {        
        if(NULL!=pwzCaption) m_strCaption=pwzCaption;
        m_uMainIconID   = 0;
        m_uExtMailID    = 0;
        m_uWarningID    = 0;
    }
    ~HdrDialog(){
    }

    void AddData(LPCWSTR pwzHeader, LPCWSTR pwzBody, int nStatus)
    {
        m_hdrAction.m_vData.push_back(YLIB::smart_ptr<HdrData>(new HdrData(pwzHeader, pwzBody, nStatus)));
    }

    BOOL IsSubWnd(HWND hParent, HWND hWnd)
    {
        HWND hTemp = hWnd;
        if(hWnd == hParent)
            return TRUE;
        while (NULL != (hTemp = GetParent(hTemp)))
        {
            if(hTemp == hParent)
                return TRUE;
        }
        return FALSE;
    }

	static LRESULT DoModalWorkThread(LPVOID lpVoid)
	{
		HdrDialog* lpDlg = (HdrDialog*)lpVoid;
		HWND    hParent = lpDlg->get_ParentHwnd();
		DWORD   dwStyle = WS_DLGFRAME|WS_POPUP|WS_VISIBLE|WS_CAPTION|WS_TILED|WS_SYSMENU;
		RECT    rcWinPos = {0, 0, lpDlg->get_AllAttachment()?HDR_WND_WIDTH:400, lpDlg->get_AllAttachment()?HDR_WND_HEIGHT:120};
		MSG     msg;
		BOOL    fGotMessage;
		BOOL    bParentEnabled = FALSE;

		YLIB::YWinUtility::CenterWindows(rcWinPos);
		HFONT hButtonFont = CreateFontW(14,
			0,
			0,
			0,
			500,
			0,
			0,
			0,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			L"Arial"
			);
		lpDlg->put_BtnFont(hButtonFont);
		HFONT hNormalFont = CreateFontW(14,
			0,
			0,
			0,
			550,
			0,
			0,
			0,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			L"Arial"
			);
		lpDlg->put_NormalFont(hNormalFont);
		HFONT hBoldFont = CreateFontW(14,
			0,
			0,
			0,
			650,
			0,
			0,
			0,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			L"Arial"
			);
		lpDlg->put_BoldFont(hBoldFont);

		if(!lpDlg->Create(HDR_CLASS_NAME, lpDlg->get_Caption().length()?lpDlg->get_Caption().c_str():HDR_CLASS_NAME, rcWinPos, NULL, WS_EX_CONTEXTHELP, dwStyle, hParent, lpDlg->get_MainIconID()))
			goto _exit;

		while((fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0 && fGotMessage != -1) 
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}

_exit:
		if(hBoldFont)   DeleteObject((HGDIOBJ)hBoldFont);
		if(hNormalFont) DeleteObject((HGDIOBJ)hNormalFont);
		if(hButtonFont) DeleteObject((HGDIOBJ)hButtonFont);
		return 0L;
	}

    LRESULT DoModal(HWND hParent = NULL)
    {
		m_hParent = hParent;
		HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DoModalWorkThread, (LPVOID)this, 0, NULL);
		if(NULL == hThread)
			return IDOK;
		if(WAIT_OBJECT_0 == WaitForSingleObject(hThread, INFINITE))
			return m_uResult;
		return IDOK;

//        DWORD   dwStyle = WS_DLGFRAME|WS_POPUP|WS_VISIBLE|WS_CAPTION|WS_TILED|WS_SYSMENU;
//        RECT    rcWinPos = {0, 0, m_nAllAttach?HDR_WND_WIDTH:400, m_nAllAttach?HDR_WND_HEIGHT:120};
//        MSG     msg;
//        BOOL    fGotMessage;
//        BOOL    bParentEnabled = FALSE;
//
//        YLIB::YWinUtility::CenterWindows(rcWinPos);
//        m_hButtonFont = CreateFontW(14,
//            0,
//            0,
//            0,
//            500,
//            0,
//            0,
//            0,
//            DEFAULT_CHARSET,
//            OUT_DEFAULT_PRECIS,
//            CLIP_DEFAULT_PRECIS,
//            CLEARTYPE_QUALITY,
//            DEFAULT_PITCH | FF_DONTCARE,
//            L"Arial"
//            );
//        m_hNormalFont = CreateFontW(14,
//            0,
//            0,
//            0,
//            550,
//            0,
//            0,
//            0,
//            DEFAULT_CHARSET,
//            OUT_DEFAULT_PRECIS,
//            CLIP_DEFAULT_PRECIS,
//            CLEARTYPE_QUALITY,
//            DEFAULT_PITCH | FF_DONTCARE,
//            L"Arial"
//            );
//        m_hBoldFont = CreateFontW(14,
//            0,
//            0,
//            0,
//            650,
//            0,
//            0,
//            0,
//            DEFAULT_CHARSET,
//            OUT_DEFAULT_PRECIS,
//            CLIP_DEFAULT_PRECIS,
//            CLEARTYPE_QUALITY,
//            DEFAULT_PITCH | FF_DONTCARE,
//            L"Arial"
//            );
//
//        if(!Create(HDR_CLASS_NAME, m_strCaption.length()?m_strCaption.c_str():HDR_CLASS_NAME, rcWinPos, NULL, WS_EX_CONTEXTHELP, dwStyle, hParent, m_uMainIconID))
//            goto _exit;
//
//#ifdef WSO2K3
//		if(m_isWordMail==0)
//#endif		
//		{
//			bParentEnabled = IsWindowEnabled(hParent);
//			if(hParent && bParentEnabled) EnableWindow(hParent, FALSE);
//		}
//
//        while((fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0/*, PM_REMOVE*/)) != 0 && fGotMessage != -1) 
//        {
//#ifdef WSO2K3
//			if(m_isWordMail==0)
//#endif	
//			{
//				if(WM_PAINT==msg.message
//					|| WM_QUIT==msg.message)
//					goto _send_msg;
//				if(IsSubWnd(m_hWnd, msg.hwnd))
//					goto _send_msg;
//			}
//            //if(NULL!=hParent && IsSubWnd(hParent, msg.hwnd))
//            //{
//            //    if(WM_LBUTTONDOWN==msg.message
//            //        || WM_LBUTTONDBLCLK==msg.message
//            //        || WM_RBUTTONDOWN==msg.message
//            //        || WM_RBUTTONDBLCLK==msg.message
//            //        )
//            //        continue;
//            //}
//#ifdef WSO2K3
//			if(m_isWordMail==0)
//#endif	
//			
//            continue;
//_send_msg:
//            TranslateMessage(&msg); 
//            DispatchMessage(&msg);
//        }
//#ifdef WSO2K3
//		if(m_isWordMail==0)
//#endif	
//		{
//			if(hParent && bParentEnabled) 
//				EnableWindow(hParent, TRUE);
//		}
//
//_exit:
//        if(m_hBoldFont)   DeleteObject((HGDIOBJ)m_hBoldFont);
//        if(m_hNormalFont) DeleteObject((HGDIOBJ)m_hNormalFont);
//        if(m_hButtonFont) DeleteObject((HGDIOBJ)m_hButtonFont);
//        return m_uResult;
    }
    inline void put_Caption(LPCWSTR pwzCaption){m_strCaption=pwzCaption;}
    inline void put_MainIconID(UINT uID){m_uMainIconID=uID;}
    inline void put_ExtMailID(UINT uID){m_uExtMailID=uID;}
    inline void put_WarningID(UINT uID){m_uWarningID=uID;}
    inline void put_BmpFailIDID(UINT uID){m_uBmpFailID=uID;}
    inline void put_BmpHDID(UINT uID){m_uBmpHDID=uID;}
    inline void put_BmpNoHDID(UINT uID){m_uBmpNoHDID=uID;}
    inline void put_Attachment(LPCWSTR pwzAttachment){m_strAttachment=pwzAttachment;}
    inline void put_HelpUrl(LPCWSTR pwzHelpUrl){if(pwzHelpUrl)m_strHelpUrl=pwzHelpUrl;}
	inline void put_IsWordMail(VARIANT_BOOL isWordMail){m_isWordMail=isWordMail;}
	inline void put_BtnFont(HFONT hFont){m_hButtonFont = hFont;}
	inline void put_NormalFont(HFONT hFont){m_hNormalFont = hFont;}
	inline void put_BoldFont(HFONT hFont){m_hBoldFont = hFont;}

	inline int  get_AllAttachment(){return m_nAllAttach;}
	inline UINT get_MainIconID(){return m_uMainIconID;}
	inline std::wstring get_Caption(){return m_strCaption;}

protected:
    MESSAGE_PROCESS_START()
        HANDLE_MESSAGE(WM_CREATE, OnCreate)
        HANDLE_MESSAGE(WM_PAINT, OnPaint)
        HANDLE_MESSAGE(WM_ERASEBKGND, OnErasBkGnd)
        HANDLE_MESSAGE(WM_HDR_CELL_CLICK, OnCtrlClick)
        HANDLE_CTRL_CLICK_MESSAGE(m_hCancel, OnCancel)
        HANDLE_CTRL_CLICK_MESSAGE(m_hNextOK, OnNextOK)
        HANDLE_SYSCOMMAND_MESSAGE(SC_CONTEXTHELP, OnHelpContext)
        MESSAGE_PROCESS_END()

protected:
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        RECT rcClient; memset(&rcClient, 0, sizeof(RECT));
        int  i=0, nInfoHeight = 0;
        YLIB::WindowDC wndDC(m_hWnd);

        m_nCurAttach = 0;

        // Set Window's body font
        if(m_hNormalFont) SendMessage(m_hWnd, WM_SETFONT, (WPARAM)m_hNormalFont, (LPARAM)TRUE);

        // Get current RECT
        GetClientRect(m_hWnd, &rcClient);
        rcClient.left  += HDR_WND_MARGIN;
        rcClient.right -= HDR_WND_MARGIN;
        rcClient.top   += HDR_WND_MARGIN;
        rcClient.bottom-= HDR_WND_MARGIN;

        int cyOffset = rcClient.top;
        // Create External Mail Icon
        m_hExtMailIcon = CreateWindowW(L"STATIC", L"STATIC", WS_CHILD|WS_VISIBLE|SS_ICON|SS_REALSIZEIMAGE,
            rcClient.left,
            (0==m_nAllAttach)?cyOffset:(cyOffset-HDR_WND_MARGIN),
            HDR_MAX_ICON_SIZE,
            HDR_MAX_ICON_SIZE,
            m_hWnd, NULL, m_hInst, NULL);
        m_extMailIcon = LoadIconW(m_hInst, MAKEINTRESOURCEW(m_uExtMailID));
        if(m_uExtMailID && m_hExtMailIcon && m_extMailIcon)
            SendMessage(m_hExtMailIcon, STM_SETIMAGE, (WPARAM)IMAGE_ICON , (LPARAM)m_extMailIcon);
        UpdateWindow(m_hExtMailIcon);
        // Create Attachment Mail Info
        int nAttachWidth  = rcClient.right - rcClient.left - HDR_MAX_ICON_SIZE - HDR_WND_MARGIN*2;
        std::wstring strAttachInfo;
        std::wstring origAttachInfo = HDR_ATTACHMENT_PREFIX; origAttachInfo += m_strAttachment;
        WrapTextToFitWidth(origAttachInfo.c_str(), strAttachInfo, nAttachWidth-2);
        if(0==m_nAllAttach) strAttachInfo = HDR_INFO_03;
        int nAttachHeight = YLIB::YWinUtility::CountTextAreaHeightByWidthNoWdBrk(strAttachInfo.c_str(), wndDC.get_DC(), m_hBoldFont, nAttachWidth, 0);
        nAttachHeight = nAttachHeight>HDR_SMALL_ICON_SIZE?nAttachHeight:HDR_SMALL_ICON_SIZE;
        m_rcAtttachInfo.left  = rcClient.left+HDR_MAX_ICON_SIZE+HDR_WND_MARGIN;
        m_rcAtttachInfo.top   = cyOffset;
        m_rcAtttachInfo.right = m_rcAtttachInfo.left+nAttachWidth;//(rcClient.right-rcClient.left-HDR_MAX_ICON_SIZE-HDR_WND_MARGIN);
        m_rcAtttachInfo.bottom= m_rcAtttachInfo.top + nAttachHeight;
        m_hAttachmentInfo = CreateWindow(L"STATIC", strAttachInfo.c_str(), WS_CHILD|WS_VISIBLE|SS_LEFTNOWORDWRAP,
            m_rcAtttachInfo.left,
            m_rcAtttachInfo.top,
            (m_rcAtttachInfo.right-m_rcAtttachInfo.left),
            (m_rcAtttachInfo.bottom-m_rcAtttachInfo.top),
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hBoldFont) SendMessage(m_hAttachmentInfo, WM_SETFONT, (WPARAM)m_hBoldFont, (LPARAM)TRUE);

        if(m_nAllAttach>0)
        {
            // Create sub information
            cyOffset += nAttachHeight;
            m_rcAtttachSubInfo.left  = rcClient.left + HDR_WND_MARGIN;
            m_rcAtttachSubInfo.top   = cyOffset;
            m_rcAtttachSubInfo.right = rcClient.right;
            m_rcAtttachSubInfo.bottom= m_rcAtttachSubInfo.top + HDR_MAX_ICON_SIZE;
            m_hAttachSubInfo = CreateWindow(L"STATIC", HDR_INFO_01, WS_CHILD|WS_VISIBLE|SS_LEFT,
                m_rcAtttachSubInfo.left,
                m_rcAtttachSubInfo.top,
                (m_rcAtttachSubInfo.right-m_rcAtttachSubInfo.left),
                (m_rcAtttachSubInfo.bottom-m_rcAtttachSubInfo.top),
                m_hWnd, NULL, m_hInst, NULL);
            if(m_hNormalFont) SendMessage(m_hAttachSubInfo, WM_SETFONT, (WPARAM)m_hNormalFont, (LPARAM)TRUE);
        }

        // Create View
        cyOffset += HDR_MAX_ICON_SIZE + HDR_WND_MARGIN/2;
        m_rcHdrView.left  = rcClient.left;
        m_rcHdrView.top   = cyOffset;
        m_rcHdrView.right = rcClient.right;
        m_rcHdrView.bottom= rcClient.bottom-HDR_WND_BTN_HEIGHT-HDR_WND_MARGIN*2;
        m_hdrView.Create(m_hWnd, m_rcHdrView);
        if(0 == m_nAllAttach) ShowWindow(m_hdrView.get_Hwnd(), SW_HIDE);

        if(m_nAllAttach>0)
        {
            // Create warning
            m_warningIcon  = (HICON)LoadImage(YBaseWnd::get_Instance(), MAKEINTRESOURCEW(m_uWarningID), IMAGE_ICON, HDR_SMALL_ICON_SIZE, HDR_SMALL_ICON_SIZE, 0);
            m_hWarningIcon = CreateWindowW(L"STATIC", L"STATIC", WS_CHILD|WS_VISIBLE|SS_ICON|SS_REALSIZEIMAGE,
                rcClient.left,
                m_rcHdrView.bottom+7,
                HDR_SMALL_ICON_SIZE,
                HDR_SMALL_ICON_SIZE,
                m_hWnd, NULL, YBaseWnd::get_Instance(), NULL);
            if(m_hWarningIcon) SendMessage(m_hWarningIcon, STM_SETIMAGE, (WPARAM)IMAGE_ICON , (LPARAM)m_warningIcon);
            UpdateWindow(m_hWarningIcon);
            // Create warning Info
            m_hWarningInfo = CreateWindow(L"STATIC", HDR_INFO_02, WS_CHILD|WS_VISIBLE|SS_LEFT,
                rcClient.left+HDR_SMALL_ICON_SIZE+HDR_WND_MARGIN/2,
                m_rcHdrView.bottom+HDR_WND_MARGIN,
                (rcClient.right-rcClient.left-HDR_SMALL_ICON_SIZE-HDR_WND_MARGIN)-160,
                HDR_MAX_ICON_SIZE,
                m_hWnd, NULL, m_hInst, NULL);
            if(m_hNormalFont) SendMessage(m_hWarningInfo, WM_SETFONT, (WPARAM)m_hNormalFont, (LPARAM)TRUE);
        }

        // Create Cancel button
        RECT rcButtonCancel, rcButtonOK;
        if(0==m_nAllAttach)
        {
            rcButtonCancel.bottom = rcClient.bottom-HDR_WND_MARGIN/2;
            rcButtonCancel.top    = rcButtonCancel.bottom-HDR_WND_BTN_HEIGHT;
            rcButtonOK.bottom     = rcClient.bottom-HDR_WND_MARGIN/2;
            rcButtonOK.top        = rcButtonOK.bottom-HDR_WND_BTN_HEIGHT;
            rcButtonOK.right = rcClient.left + (rcClient.right-rcClient.left)/2 - HDR_WND_BTN_WIDTH/2;
            rcButtonOK.left  = rcButtonOK.right - HDR_WND_BTN_WIDTH;
            rcButtonCancel.left  = rcButtonOK.right + HDR_WND_BTN_WIDTH;
            rcButtonCancel.right = rcClient.right + HDR_WND_BTN_WIDTH;
        }
        else
        {
            rcButtonCancel.top    = rcClient.bottom-HDR_WND_BTN_HEIGHT;
            rcButtonCancel.bottom = rcClient.bottom;
            rcButtonOK.top        = rcClient.bottom-HDR_WND_BTN_HEIGHT;
            rcButtonOK.bottom     = rcClient.bottom;
            rcButtonCancel.right = rcClient.right-HDR_WND_MARGIN;
            rcButtonCancel.left  = rcButtonCancel.right-HDR_WND_BTN_WIDTH;
            rcButtonOK.right     = rcButtonCancel.left - HDR_WND_MARGIN*3/2;
            rcButtonOK.left      = rcButtonOK.right - HDR_WND_BTN_WIDTH;
        }
        m_hCancel = CreateWindow(L"BUTTON", m_nAllAttach?L"Cancel":L"OK", WS_CHILD|WS_VISIBLE,
            rcButtonCancel.left,
            rcButtonCancel.top,
            HDR_WND_BTN_WIDTH,
            HDR_WND_BTN_HEIGHT,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hButtonFont) SendMessage(m_hCancel, WM_SETFONT, (WPARAM)m_hButtonFont, (LPARAM)TRUE);

        // Create Send button
        m_hNextOK = CreateWindow(L"BUTTON", m_nAllAttach>1?L"Next":L"Send Email", WS_CHILD|WS_VISIBLE,
            rcButtonOK.left,
            rcButtonOK.top,
            HDR_WND_BTN_WIDTH,
            HDR_WND_BTN_HEIGHT,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hButtonFont) SendMessage(m_hNextOK, WM_SETFONT, (WPARAM)m_hButtonFont, (LPARAM)TRUE);

        if(m_nAllAttach>0)
        {
            // Load Bitmap
            m_hBmpFail = ::LoadBitmapW(m_hInst, MAKEINTRESOURCEW(m_uBmpFailID));
            m_hBmpHD   = ::LoadBitmapW(m_hInst, MAKEINTRESOURCEW(m_uBmpHDID));
            m_hBmpNoHD = ::LoadBitmapW(m_hInst, MAKEINTRESOURCEW(m_uBmpNoHDID));

            // Initialize view
            for (HdrDataVector::iterator it=m_hdrAction.m_vData.begin(); it!=m_hdrAction.m_vData.end(); ++it)
            {
                YLIB::smart_ptr<HdrData> spData = *it;
                m_hdrView.AddItem(m_hBmpFail,
                    m_hBmpHD,
                    m_hBmpNoHD,
                    m_hBoldFont,
                    m_hNormalFont,
                    spData->m_strHeader.c_str(),
                    spData->m_strBody.c_str(),
                    spData->m_nStatus);
            }
            m_hdrView.PrepareView();
            m_hdrView.CreateView();
            m_hdrView.UpdateView();
        }

        return 0L;
    }
    LRESULT OnHelpContext(BOOL& bCalldefaultHandler)
    {
        if(m_strHelpUrl.length())
            ShellExecuteW(GetDesktopWindow(), L"open", m_strHelpUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);

        bCalldefaultHandler = FALSE;
        return 0L;
    }
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        return 0L;
    }
    LRESULT OnErasBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        return 0L;
    }
    LRESULT OnCancel(BOOL& bCalldefaultHandler)
    {
        return SendMessage(m_hWnd, WM_CLOSE, IDCANCEL, 0);
    }
    LRESULT OnNextOK(BOOL& bCalldefaultHandler)
    {
        WCHAR wzText[16]; memset(wzText, 0, sizeof(wzText));
        GetWindowTextW(m_hNextOK, wzText, 15);
        if(0==m_nAllAttach || 0 == wcscmp(wzText, L"Send Email") || m_nCurAttach==m_nAllAttach-1)
        {
            m_uResult = IDOK;
            return SendMessage(m_hWnd, WM_CLOSE, IDOK, 0);
        }
        else
        {
            if(m_nCurAttach==m_nAllAttach-2)
                SetWindowTextW(m_hNextOK, L"Send Email");

            // Move to next attachment
            std::wstring strNewAttachment;
            std::wstring strHelpUrl = m_strHelpUrl;
            m_hdrAction.m_vData.clear();
            m_hdrAction.OnNext(m_nCurAttach, strNewAttachment, strHelpUrl,
                               m_pHdrActionContext);
            // set new help url
            m_strHelpUrl = strHelpUrl;

            // Reset attachment path and adjust position
            YLIB::WindowDC wndDC(m_hWnd);
            // Reset attachment info
            std::wstring strNewAttachInfo = HDR_ATTACHMENT_PREFIX; strNewAttachInfo += strNewAttachment;
            int nAttachWidth  = m_rcAtttachInfo.right - m_rcAtttachInfo.left;
            int nAttachHeight = YLIB::YWinUtility::CountTextAreaHeightByWidth(strNewAttachInfo.c_str(), wndDC.get_DC(), m_hBoldFont, nAttachWidth, 0);
            nAttachHeight = nAttachHeight>HDR_SMALL_ICON_SIZE?nAttachHeight:HDR_SMALL_ICON_SIZE;
            int nOffset = nAttachHeight-(m_rcAtttachInfo.bottom-m_rcAtttachInfo.top);
            if(0!=nOffset)
            {
                m_rcAtttachInfo.bottom += nOffset;
                SetWindowPos(m_hAttachmentInfo, HWND_BOTTOM, 0, 0, nAttachWidth, nAttachHeight, SWP_NOMOVE);
            }
            SetWindowTextW(m_hAttachmentInfo, strNewAttachInfo.c_str());

			//reset note
			SetWindowTextW(m_hWarningInfo, HDR_INFO_02);
            // Reset sub info
            if(0!=nOffset)
            {
                OffsetRect(&m_rcAtttachSubInfo, 0, nOffset);
                SetWindowPos(m_hAttachSubInfo, HWND_BOTTOM, m_rcAtttachSubInfo.left, m_rcAtttachSubInfo.top, 0, 0, SWP_NOSIZE);
            }
            // Reset view
            if(0!=nOffset)
            {
                m_rcHdrView.top += nOffset;
                MoveWindow(m_hdrView.get_Hwnd(), m_rcHdrView.left, m_rcHdrView.top, m_rcHdrView.right-m_rcHdrView.left, m_rcHdrView.bottom-m_rcHdrView.top, TRUE);
            }

            // Reset view
            m_hdrView.DeleteView();
            m_hdrView.CleanView();
            for (HdrDataVector::iterator it=m_hdrAction.m_vData.begin(); it!=m_hdrAction.m_vData.end(); ++it)
            {
                YLIB::smart_ptr<HdrData> spData = *it;
                m_hdrView.AddItem(m_hBmpFail,
                    m_hBmpHD,
                    m_hBmpNoHD,
                    m_hBoldFont,
                    m_hNormalFont,
                    spData->m_strHeader.c_str(),
                    spData->m_strBody.c_str(),
                    spData->m_nStatus);
            }
            m_hdrView.PrepareView();
            m_hdrView.CreateView();
            m_hdrView.UpdateView();
        }
        m_nCurAttach++;
        return 0L;
    }
    LRESULT OnCtrlClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        bCalldefaultHandler = FALSE;
        WORD wCellItem = (WORD)((wParam&0xFFFF0000)>>16);
        WORD wCtrlItem = (WORD)(wParam&0x0000FFFF);
        HWND hCtrl     = (HWND)lParam;

        if(2 == wCtrlItem)     // click remove button
        {
            std::wstring strNewBody = L"";
			std::wstring strNote = L"";
            int          nNewStattus= 0;
            m_hdrAction.OnRemove(m_nCurAttach, wCellItem, strNewBody,strNote,
                                 &nNewStattus, m_pHdrActionContext);
			if(strNote.length()>0)
				SetWindowTextW(m_hWarningInfo, strNote.c_str());
				
            m_hdrView.UpdateItemStatus(wCellItem, strNewBody.c_str(), nNewStattus);
        }
        else                        // unknown click
        {
        }

        return 0L;
    }

private:
    UINT                    m_uResult;
    std::wstring            m_strCaption;
    std::wstring            m_strAttachment;
    int                     m_nAllAttach;
    int                     m_nCurAttach;
    std::wstring            m_strHelpUrl;

    // view
    HdrViewWnd              m_hdrView;
    T                       m_hdrAction;
    LPVOID                  m_pHdrActionContext;

    // controls
    HWND                    m_hNextOK;
    HWND                    m_hCancel;
    HWND                    m_hExtMailIcon;
    HWND                    m_hAttachmentInfo;
    HWND                    m_hAttachSubInfo;
    HWND                    m_hWarningIcon;
    HWND                    m_hWarningInfo;

    RECT                    m_rcAtttachInfo;
    RECT                    m_rcAtttachSubInfo;
    RECT                    m_rcHdrView;

    // Resource ID
    UINT                    m_uMainIconID;
    UINT                    m_uExtMailID;
    UINT                    m_uWarningID;
    UINT                    m_uBmpFailID;
    UINT                    m_uBmpHDID;
    UINT                    m_uBmpNoHDID;

    // Font
    HFONT                   m_hButtonFont;
    HFONT                   m_hBoldFont;
    HFONT                   m_hNormalFont;

    // Icon
    HICON                   m_extMailIcon;
    HICON                   m_warningIcon;
    HBITMAP                 m_hBmpFail;
    HBITMAP                 m_hBmpHD;
    HBITMAP                 m_hBmpNoHD;

	VARIANT_BOOL			m_isWordMail;
};



#endif