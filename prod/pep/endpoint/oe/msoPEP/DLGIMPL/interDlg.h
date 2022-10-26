
#pragma once
#ifndef _INTER_DIALOG_H_
#define _INTER_DIALOG_H_
#include <Windows.h>
#include <stdio.h>
#include "common.h"
#include "YWinBase.h"
#include "YViewWnd.h"
#include "YViewBase.h"
#include "YViewCell.h"

#define INTER_DLG_WIDTH               465
#define INTER_DLG_HEIGHT              400
#define INTER_DLG_MARGIN              10
#define INTER_DLG_BTN_HEIGHT          22
#define INTER_DLG_BTN_WIDTH           70
#define INTER_DLG_CLASS_NAME          L"MORGAN STANLEY DEMO DIALOG"
#define INTER_DLG_INFO_01             L"The following attachments are for INTERNAL USE ONLY. Click on OK to return to the email."
#define INTER_DLG_BIG_ICON_SIZE       32
#define INTER_DLG_SMALL_ICON_SIZE     16

#define INTER_CELL_MARGIN             8

class InterViewCell : public YViewCell
{
public:
    InterViewCell(HWND hViewWnd, HDC hViewDC, int nWidth) : YViewCell(hViewWnd, hViewDC, nWidth)
    {
        m_hFlag     = 0;
        m_hText     = 0;
    }
    ~InterViewCell()
    {
        SendMessageW(m_hFlag, WM_CLOSE, 0, 0);
        SendMessageW(m_hText, WM_CLOSE, 0, 0);
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
        m_nOffset = cyOffset;

        // Icon rect
        m_rcFlag.left = INTER_CELL_MARGIN; m_rcFlag.right = m_rcFlag.left + INTER_DLG_SMALL_ICON_SIZE;
        m_rcFlag.top  = cyOffset+INTER_CELL_MARGIN*2;           m_rcFlag.bottom = m_rcFlag.top + INTER_DLG_SMALL_ICON_SIZE;

       // text rect
        int nTextWidth = m_nWidth-INTER_CELL_MARGIN-INTER_DLG_SMALL_ICON_SIZE-2*YScrollBar::get_CXVScroll();
        m_nHeight = YLIB::YWinUtility::CountTextAreaHeightByWidth(m_strAttachment.c_str(), m_hViewDC, m_hFont, nTextWidth, 0);
        if(m_nHeight<(INTER_DLG_SMALL_ICON_SIZE*2)) m_nHeight=INTER_DLG_SMALL_ICON_SIZE*2;
        m_nHeight += INTER_CELL_MARGIN*2;
        m_rcText.left   = m_rcFlag.right+INTER_CELL_MARGIN;
        m_rcText.right  = m_rcText.left + nTextWidth;
        m_rcText.top    = cyOffset+INTER_CELL_MARGIN*2;
        m_rcText.bottom = m_rcText.top + m_nHeight;

        return m_nHeight;
    }
    void render_Cell()

    {

        if(0!=m_hBmp)

        {

            HDC     hFlagDc = CreateCompatibleDC(m_hViewDC);

            HBITMAP hOldFlag= (HBITMAP)SelectObject(hFlagDc, m_hBmp);

            BitBlt(m_hViewDC, m_rcFlag.left, m_rcFlag.top, INTER_DLG_SMALL_ICON_SIZE, INTER_DLG_SMALL_ICON_SIZE, hFlagDc, 0, 0, SRCCOPY);

            SelectObject(hFlagDc, hOldFlag);

            DeleteObject(hFlagDc);

        }



        HFONT hOldFont = (HFONT)SelectObject(m_hViewDC, m_hFont);

        SetBkMode(m_hViewDC, TRANSPARENT);

        SetTextColor(m_hViewDC, RGB(64, 64, 64));

        DrawTextEx(m_hViewDC, (LPWSTR)m_strAttachment.c_str(), -1, &m_rcText, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK, NULL);

        SelectObject(m_hViewDC, hOldFont);

    }

    int  locate_Cell(int cxOffset, int cyOffset)
    {
        UNREFERENCED_PARAMETER(cxOffset);
        UNREFERENCED_PARAMETER(cyOffset);
        RECT rcPos; memset(&rcPos, 0, sizeof(RECT));

        //memcpy(&rcPos, &m_rcFlag, sizeof(RECT));
        //OffsetRect(&rcPos, 0-cxOffset, 0-cyOffset);
        //SetWindowPos(m_hFlag, HWND_BOTTOM, rcPos.left, rcPos.top, 0, 0, SWP_NOSIZE);

        //memcpy(&rcPos, &m_rcText, sizeof(RECT));
        //OffsetRect(&rcPos, 0-cxOffset, 0-cyOffset);
        //SetWindowPos(m_hText, HWND_BOTTOM, rcPos.left, rcPos.top, 0, 0, SWP_NOSIZE);

        return m_nHeight;

    }

    WORD find_cellctrl(HWND hWnd)
    {
        UNREFERENCED_PARAMETER(hWnd);
        return 0;
    }

    //
public:
    void put_attachment(LPCWSTR  pwzAttachment){
        m_strAttachment = pwzAttachment?pwzAttachment:L"";
    }
    void put_font(HFONT hFont){m_hFont=hFont;}
    void put_bmp(HBITMAP hBmp){m_hBmp=hBmp;}

private:
    HBITMAP         m_hBmp;
    HFONT           m_hFont;
    std::wstring    m_strAttachment;

    RECT            m_rcFlag;
    RECT            m_rcText;

    HWND            m_hFlag;
    HWND            m_hText;
};

class InterViewWnd : public YViewWnd<InterViewCell>
{
public:
    InterViewWnd()
    {
        m_hWhiteBrush = CreateSolidBrush(RGB(255,255,255));
    }

    virtual ~InterViewWnd()
    {
        if(m_hWhiteBrush) DeleteObject(m_hWhiteBrush);
    }
    void AddItem(HBITMAP hBmp, HFONT hFont, LPCWSTR pwzAttachment)
    {
        smart_ptr<InterViewCell> spItem( new InterViewCell(m_hViewWnd, m_offscreenDC, INTER_DLG_WIDTH-20));
        if(0==spItem.get()) return;
        spItem->put_attachment(pwzAttachment);
        spItem->put_font(hFont);
        spItem->put_bmp(hBmp);
        m_cells.push_back(spItem);
    }
    LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bCalldefaultHandler);
        HDC  hDC   = (HDC)wParam;
        bCalldefaultHandler = FALSE;
        SetBkColor(hDC, RGB(255,255,255));
        return (LRESULT)m_hWhiteBrush;
    }

private:
    HBRUSH                  m_hWhiteBrush;
};

class InterDialog : public YBaseWnd
{
public:
    InterDialog(LPCWSTR pwzCaption=NULL):m_uMainIconID(0)
    {        
        if(NULL!=pwzCaption) m_strCaption=pwzCaption;
        m_strDlgInfo    = INTER_DLG_INFO_01;
        m_uMainIconID  = 0;
        m_uExtMailID   = 0;
        m_uWordBmpID   = 0;
        m_uExcelBmpID  = 0;
        m_uPwptBmpID   = 0;
        m_uPdfBmpID    = 0;
        m_uUnkBmpID    = 0;
       m_strSendNextBtn = L"OK";
    }
    ~InterDialog(){
        m_vInterData.clear();
    }

    void AddData(LPCWSTR pwzAttachment)
    {
        m_vInterData.push_back(pwzAttachment);
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
    LRESULT DoModal(HWND hParent = NULL, VARIANT_BOOL bIsWdEdit=0)
    {
		bIsWdEdit ;
        DWORD   dwStyle = WS_DLGFRAME|WS_POPUP|WS_VISIBLE|WS_CAPTION|WS_TILED|WS_SYSMENU;
        RECT    rcWinPos = {0, 0, INTER_DLG_WIDTH, INTER_DLG_HEIGHT};

        MSG     msg;

        BOOL    fGotMessage;



        YLIB::YWinUtility::CenterWindows(rcWinPos);

        m_hButtonFont = CreateFontW(14,

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

        m_hNormalFont = CreateFontW(14,

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

        m_hBoldFont = CreateFontW(14,

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



        if(!Create(INTER_DLG_CLASS_NAME, m_strCaption.length()?m_strCaption.c_str():INTER_DLG_CLASS_NAME, rcWinPos, NULL, WS_EX_CONTEXTHELP, dwStyle, hParent, m_uMainIconID))

            goto _exit;




        BOOL    bParentEnabled = TRUE;
#ifdef WSO2K3
        if(0==bIsWdEdit)
#endif		
        {
            bParentEnabled = IsWindowEnabled(hParent);
            if(hParent && bParentEnabled) EnableWindow(hParent, FALSE);
        }

        while((fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0 && fGotMessage != -1) 
        {
#ifdef WSO2K3
            if(0==bIsWdEdit)
#endif	
            {
                if(WM_PAINT==msg.message
                    || WM_QUIT==msg.message)
                    goto _send_msg;
                if(IsSubWnd(m_hWnd, msg.hwnd))
                    goto _send_msg;
            }
#ifdef WSO2K3
            if(0==bIsWdEdit)
#endif
                continue;

_send_msg:
            TranslateMessage(&msg); 
            DispatchMessage(&msg);
        }
#ifdef WSO2K3
        if(0==bIsWdEdit)
#endif	
        {
            if(hParent && bParentEnabled) 
                EnableWindow(hParent, TRUE);
        }


_exit:

        if(m_hBoldFont)   DeleteObject((HGDIOBJ)m_hBoldFont);

        if(m_hNormalFont) DeleteObject((HGDIOBJ)m_hNormalFont);

        if(m_hButtonFont) DeleteObject((HGDIOBJ)m_hButtonFont);

        return IDOK;

    }

    inline void put_Caption(LPCWSTR pwzCaption){m_strCaption=pwzCaption;}

    inline void put_DlgInfo(LPCWSTR pwzDlgInfo){m_strDlgInfo=pwzDlgInfo;}

    inline void put_MainIconID(UINT uID){m_uMainIconID=uID;}

    inline void put_ExtMailID(UINT uID){m_uExtMailID=uID;}

    inline void put_WordBmpID(UINT uID){m_uWordBmpID=uID;}

    inline void put_ExcelBmpID(UINT uID){m_uExcelBmpID=uID;}

    inline void put_PwptBmpID(UINT uID){m_uPwptBmpID=uID;}

    inline void put_PdfBmpID(UINT uID){m_uPdfBmpID=uID;}

    inline void put_UnkBmpID(UINT uID){m_uUnkBmpID=uID;}
    inline void put_HelpUrl(LPCWSTR pwzHelpUrl){if(pwzHelpUrl)m_strHelpUrl=pwzHelpUrl;}
    inline void put_SendNextBtnText(LPCWSTR pwzSendNext){if(pwzSendNext)m_strSendNextBtn=pwzSendNext;}



protected:

    MESSAGE_PROCESS_START()

        HANDLE_MESSAGE(WM_CREATE, OnCreate)

        HANDLE_MESSAGE(WM_PAINT, OnPaint)

        HANDLE_MESSAGE(WM_ERASEBKGND, OnErasBkGnd)

        HANDLE_CTRL_CLICK_MESSAGE(m_hOK, OnOK)

        HANDLE_SYSCOMMAND_MESSAGE(SC_CONTEXTHELP, OnHelpContext)

        MESSAGE_PROCESS_END()



protected:

    BOOL IsWordDoc(LPCWSTR pwzExt)

    {

        if(NULL==pwzExt) return FALSE;

        if(0==_wcsicmp(pwzExt, L".DOC")

            || 0==_wcsicmp(pwzExt, L".DOCX")

            )

            return TRUE;

        return FALSE;

    }

    BOOL IsExcelDoc(LPCWSTR pwzExt)

    {

        if(NULL==pwzExt) return FALSE;

        if(0==_wcsicmp(pwzExt, L".XLS")

            || 0==_wcsicmp(pwzExt, L".XLSX")

            )

            return TRUE;

        return FALSE;

    }

    BOOL IsPwptDoc(LPCWSTR pwzExt)

    {

        if(NULL==pwzExt) return FALSE;

        if(0==_wcsicmp(pwzExt, L".PPT")

            || 0==_wcsicmp(pwzExt, L".PPTX")

            || 0==_wcsicmp(pwzExt, L".PPTM")

            )

            return TRUE;

        return FALSE;

    }

    BOOL IsPdfDoc(LPCWSTR pwzExt)

    {

        if(NULL==pwzExt) return FALSE;

        if(0==_wcsicmp(pwzExt, L".PDF"))

            return TRUE;

        return FALSE;

    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)

    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bCalldefaultHandler);
        RECT rcClient; memset(&rcClient, 0, sizeof(RECT));
        RECT rcView;   memset(&rcView, 0, sizeof(RECT));

        YLIB::WindowDC wndDC(m_hWnd);
        // Get all icons
        m_wordBmp  = ::LoadBitmapW(m_hInst, MAKEINTRESOURCEW(m_uWordBmpID));
        m_excelBmp = ::LoadBitmapW(m_hInst, MAKEINTRESOURCEW(m_uExcelBmpID));
        m_pwptBmp  = ::LoadBitmapW(m_hInst, MAKEINTRESOURCEW(m_uPwptBmpID));
        m_pdfBmp   = ::LoadBitmapW(m_hInst, MAKEINTRESOURCEW(m_uPdfBmpID));
        m_unkBmp   = ::LoadBitmapW(m_hInst, MAKEINTRESOURCEW(m_uUnkBmpID));

        // Set Window's body font
        if(m_hNormalFont) SendMessage(m_hWnd, WM_SETFONT, (WPARAM)m_hNormalFont, (LPARAM)TRUE);

        // Get current RECT
        GetClientRect(m_hWnd, &rcClient);
        rcClient.left  += INTER_DLG_MARGIN;
        rcClient.right -= INTER_DLG_MARGIN;
        rcClient.top   += INTER_DLG_MARGIN;
        rcClient.bottom-= INTER_DLG_MARGIN;

        int cyOffset = rcClient.top;
        // Create External Mail Icon
        m_hExtMailIcon = CreateWindowW(L"STATIC", L"STATIC", WS_CHILD|WS_VISIBLE|SS_ICON|SS_REALSIZEIMAGE,
            rcClient.left,
            cyOffset,
            INTER_DLG_BIG_ICON_SIZE,
            INTER_DLG_BIG_ICON_SIZE,
            m_hWnd, NULL, m_hInst, NULL);
        m_extMailIcon = LoadIconW(m_hInst, MAKEINTRESOURCEW(m_uExtMailID));
        if(m_uExtMailID && m_hExtMailIcon && m_extMailIcon)
            SendMessage(m_hExtMailIcon, STM_SETIMAGE, (WPARAM)IMAGE_ICON , (LPARAM)m_extMailIcon);
        if(NULL!=m_hExtMailIcon)
            UpdateWindow(m_hExtMailIcon);

        // Create External Mail Info
        int nTextWidth = rcClient.right-rcClient.left-INTER_DLG_BIG_ICON_SIZE-INTER_DLG_MARGIN;
        int nTextHeight= YLIB::YWinUtility::CountTextAreaHeightByWidth(m_strDlgInfo.c_str(), wndDC.get_DC(), m_hBoldFont, nTextWidth, 0);
        if(nTextHeight<INTER_DLG_BIG_ICON_SIZE) nTextHeight=INTER_DLG_BIG_ICON_SIZE;
        m_hExtMailInfo = CreateWindow(L"STATIC", m_strDlgInfo.c_str(), WS_CHILD|WS_VISIBLE|SS_LEFT,
            rcClient.left+INTER_DLG_BIG_ICON_SIZE+INTER_DLG_MARGIN,
            cyOffset,
            (rcClient.right-rcClient.left-INTER_DLG_BIG_ICON_SIZE-INTER_DLG_MARGIN),
            nTextHeight,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hBoldFont) SendMessage(m_hExtMailInfo, WM_SETFONT, (WPARAM)m_hBoldFont, (LPARAM)TRUE);
    
        // Create View
        cyOffset += INTER_DLG_BIG_ICON_SIZE+INTER_DLG_MARGIN;
        rcView.left   = rcClient.left;
        rcView.right  = rcClient.right;
        rcView.top    = cyOffset;
        rcView.bottom = rcClient.bottom-INTER_DLG_BTN_HEIGHT-INTER_DLG_MARGIN;
        m_interView.Create(m_hWnd, rcView);

        // Create Send button
        m_hOK = CreateWindow(L"BUTTON", m_strSendNextBtn.c_str()/*L"OK"*/, WS_CHILD|WS_VISIBLE,
            (rcClient.right-INTER_DLG_BTN_WIDTH-INTER_DLG_MARGIN),
            (rcClient.bottom-INTER_DLG_BTN_HEIGHT),
            INTER_DLG_BTN_WIDTH,
            INTER_DLG_BTN_HEIGHT,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hButtonFont) SendMessage(m_hOK, WM_SETFONT, (WPARAM)m_hButtonFont, (LPARAM)TRUE);

        for(STRINGS::iterator it=m_vInterData.begin(); it!=m_vInterData.end(); ++it)
        {
            const WCHAR* pExt = wcsrchr((*it).c_str(), L'.');
            HBITMAP      hBmp= NULL;
            if(IsWordDoc(pExt))
                hBmp = m_wordBmp;
            else if(IsExcelDoc(pExt))
                hBmp = m_excelBmp;
            else if(IsPwptDoc(pExt))
                hBmp = m_pwptBmp;
            else if(IsPdfDoc(pExt))
                hBmp = m_pdfBmp;
            else
                hBmp = m_unkBmp;
            m_interView.AddItem(hBmp, m_hNormalFont, (*it).c_str());
        }

        m_interView.PrepareView();
        m_interView.CreateView();
        m_interView.UpdateView();
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
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bCalldefaultHandler);

        return 0L;

    }

    LRESULT OnErasBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)

    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bCalldefaultHandler);
        return 0L;

    }

    LRESULT OnOK(BOOL& bCalldefaultHandler)

    {
        UNREFERENCED_PARAMETER(bCalldefaultHandler);
        return SendMessage(m_hWnd, WM_CLOSE, IDOK, 0);

    }



private:

    std::wstring            m_strCaption;

    std::wstring            m_strDlgInfo;
    std::wstring            m_strHelpUrl;
    std::wstring            m_strSendNextBtn;



    // view

    InterViewWnd            m_interView;



    // controls

    HWND                    m_hOK;

    HWND                    m_hExtMailIcon;

    HWND                    m_hExtMailInfo;



    // Resource ID

    UINT                    m_uMainIconID;

    UINT                    m_uExtMailID;

    UINT                    m_uWordBmpID;

    UINT                    m_uExcelBmpID;

    UINT                    m_uPwptBmpID;

    UINT                    m_uPdfBmpID;

    UINT                    m_uUnkBmpID;



    // Font

    HFONT                   m_hButtonFont;

    HFONT                   m_hBoldFont;

    HFONT                   m_hNormalFont;



    // Icon

    HICON                   m_extMailIcon;

    HBITMAP                 m_wordBmp;

    HBITMAP                 m_excelBmp;

    HBITMAP                 m_pwptBmp;

    HBITMAP                 m_pdfBmp;

    HBITMAP                 m_unkBmp;



    STRINGS                 m_vInterData;

};



#endif