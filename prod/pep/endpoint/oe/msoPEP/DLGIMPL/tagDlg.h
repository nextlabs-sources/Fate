
#pragma once
#ifndef _TAG_DIALOG_H_
#define _TAG_DIALOG_H_
#include <Windows.h>
#include <stdio.h>
#include "common.h"
#include "YWinBase.h"
#include "YViewWnd.h"
#include "YViewBase.h"
#include "YViewCell.h"

/*
WPARAM wParam    HIWORD(wParam) -- the index of this cell item, base on zero
LOWORD(wParam) -- the control index in this cell item, base on zero
LPARAM lParam    -- the hwnd of this control
*/
#define TAG_DLG_WIDTH               465
#define TAG_DLG_HEIGHT              400
#define TAG_DLG_MARGIN              10
#define TAG_DLG_BTN_HEIGHT          22
#define TAG_DLG_BTN_WIDTH           70
#define TAG_COMBOBOX_HEIGHT         25
#define TAG_COMBOBOX_WIDTH          150
#define TAG_DLG_CLASS_NAME          L"MORGAN STANLEY DEMO DIALOG"
#define TAG_DLG_INFO_01             L"Sending documents to external recipients requires associating a client to the documents. Select a client from the list or Not Client related."
#define TAG_NOT_CLIENT_RELATED      L"Not Client Related"
#define TAG_NOT_CLIENT_RELATED_INDEX 0

#define TAG_DLG_BIG_ICON_SIZE       32
#define TAG_DLG_SMALL_ICON_SIZE     16

#define TAG_CELL_MARGIN             8

class TagActionBase
{
public:
    virtual void OnOK(std::vector<int>& vTagIndices, LPVOID pContext) = 0;
};

class TagData
{
public:
    TagData(LPCWSTR pwzAttachment, LPCWSTR pwzClients)
    {
        m_strAttachment = pwzAttachment;
        m_vClients.clear();
        /* Place the "Not Client Related" client name at the beginning of the
           list. */
        C_ASSERT(TAG_NOT_CLIENT_RELATED_INDEX == 0);
        m_vClients.push_back(TAG_NOT_CLIENT_RELATED);
        if (wcslen(pwzClients) > 0)
        {
            ParseClients(pwzClients, m_vClients);
        }

    }
    static void ParseClients(LPCWSTR pwzClients, STRINGS& vClients)
    {
        const WCHAR* pwzStart = pwzClients;
        const WCHAR* pwzEnd   = wcsstr(pwzStart, L";");
        do 
        {
            if(NULL==pwzEnd)
            {
                vClients.push_back(pwzStart);
                break;
            }
            if(pwzEnd != pwzStart)
            {
                std::wstring strTemp(pwzStart, (pwzEnd-pwzStart));
                vClients.push_back(strTemp);
            }
            pwzStart = ++pwzEnd;
            pwzEnd = wcsstr(pwzStart, L";");
        } while(1);
    }
    std::wstring m_strAttachment;
    STRINGS      m_vClients;
};
typedef std::vector<smart_ptr<TagData>>    TAGDATALIST;

class TagViewCell : public YViewCell
{
public:
    TagViewCell(HWND hViewWnd, HDC hViewDC, int nWidth) : YViewCell(hViewWnd, hViewDC, nWidth){
        m_hFlag     = 0;
        m_hText     = 0;
        m_hComboBox = 0;
    }
    ~TagViewCell()
    {
        SendMessageW(m_hFlag, WM_CLOSE, 0, 0);
        SendMessageW(m_hText, WM_CLOSE, 0, 0);
        SendMessageW(m_hComboBox, WM_CLOSE, 0, 0);
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

        int nTextWidth = m_nWidth-TAG_CELL_MARGIN*3-TAG_DLG_SMALL_ICON_SIZE-TAG_COMBOBOX_WIDTH-2*YScrollBar::get_CXVScroll();
        m_nHeight = YLIB::YWinUtility::CountTextAreaHeightByWidth(m_strAttachment.c_str(), m_hViewDC, m_hFont, nTextWidth/*m_nWidth*/, 0);
        if(m_nHeight<(TAG_DLG_SMALL_ICON_SIZE*2)) m_nHeight=TAG_DLG_SMALL_ICON_SIZE*2;
        m_nHeight += TAG_CELL_MARGIN*2;

        // Icon rect
        m_rcFlag.left = TAG_CELL_MARGIN;            m_rcFlag.right = m_rcFlag.left + TAG_DLG_SMALL_ICON_SIZE;
        m_rcFlag.top  = cyOffset+TAG_CELL_MARGIN*2; m_rcFlag.bottom = m_rcFlag.top + TAG_DLG_SMALL_ICON_SIZE;

        // combo box rect
        m_rcComboBox.right = m_nWidth /*- TAG_CELL_MARGIN*/ - 2*YScrollBar::get_CXVScroll();
        m_rcComboBox.left  = m_rcComboBox.right - TAG_COMBOBOX_WIDTH;
        m_rcComboBox.top   = cyOffset+TAG_CELL_MARGIN*2;
        m_rcComboBox.bottom= m_rcComboBox.top + TAG_COMBOBOX_HEIGHT;
        m_hComboBox = CreateWindow(L"COMBOBOX", L"", WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST,
            m_rcComboBox.left,
            m_rcComboBox.top,
            TAG_COMBOBOX_WIDTH,
            TAG_COMBOBOX_HEIGHT*8,
            m_hViewWnd, NULL, YBaseWnd::get_Instance(), NULL);
        if(m_hFont) SendMessage(m_hComboBox, WM_SETFONT, (WPARAM)m_hFont, (LPARAM)TRUE);
        // add strings
        for(STRINGS::iterator it=m_vClients.begin(); it!=m_vClients.end(); ++it)
        {
            SendMessageW(m_hComboBox, CB_ADDSTRING, 0, (LPARAM)(*it).c_str());
        }
        SendMessageW(m_hComboBox, CB_SETCURSEL, 0, 0);

        // text rect
        m_rcText.left   = m_rcFlag.right+TAG_CELL_MARGIN;
        m_rcText.right  = m_rcComboBox.left - TAG_CELL_MARGIN;
        m_rcText.top    = cyOffset+TAG_CELL_MARGIN*2;
        m_rcText.bottom = m_rcText.top + m_nHeight;

        return m_nHeight;
    }
    void render_Cell()
    {
        if(0!=m_hBmp)
        {
            HDC     hFlagDc = CreateCompatibleDC(m_hViewDC);
            HBITMAP hOldFlag= (HBITMAP)SelectObject(hFlagDc, m_hBmp);
            BitBlt(m_hViewDC, m_rcFlag.left, m_rcFlag.top, TAG_DLG_SMALL_ICON_SIZE, TAG_DLG_SMALL_ICON_SIZE, hFlagDc, 0, 0, SRCCOPY);
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
        RECT rcPos; memset(&rcPos, 0, sizeof(RECT));

        memcpy(&rcPos, &m_rcComboBox, sizeof(RECT));
        OffsetRect(&rcPos, 0-cxOffset, 0-cyOffset);
        SetWindowPos(m_hComboBox, HWND_BOTTOM, rcPos.left, rcPos.top, 0, 0, SWP_NOSIZE);

        return m_nHeight;
    }
    WORD find_cellctrl(HWND hWnd)
    {
        UNREFERENCED_PARAMETER(hWnd);
        return 0xFFFF;
    }
    int get_selected()
    {
        int nSel = (int)SendMessageW(m_hComboBox, CB_GETCURSEL, 0, 0);
        if(nSel>=0 && nSel<(int)m_vClients.size()) return nSel;
        else return -1;
    }

    //
public:
    void put_attachment(LPCWSTR  pwzAttachment){
        m_strAttachment = pwzAttachment?pwzAttachment:L"";
    }
    void put_clients(STRINGS& vClients){
        m_vClients = vClients;
    }
    void put_font(HFONT hFont){m_hFont=hFont;}
    void put_bmp(HBITMAP hBmp){m_hBmp=hBmp;}

private:
    HBITMAP         m_hBmp;
    HFONT           m_hFont;
    std::wstring    m_strAttachment;
    STRINGS         m_vClients;

    RECT            m_rcFlag;
    RECT            m_rcText;
    RECT            m_rcComboBox;

    HWND            m_hFlag;
    HWND            m_hText;
    HWND            m_hComboBox;
};

class TagViewWnd : public YViewWnd<TagViewCell>
{
public:
    TagViewWnd()
    {
        m_hWhiteBrush = CreateSolidBrush(RGB(255,255,255));
    }
    virtual ~TagViewWnd()
    {
        if(m_hWhiteBrush) DeleteObject(m_hWhiteBrush);
    }
    void AddItem(HBITMAP hBmp, HFONT hFont, LPCWSTR pwzAttachment, STRINGS& vClients)
    {
        smart_ptr<TagViewCell> spItem( new TagViewCell(m_hViewWnd, m_offscreenDC, TAG_DLG_WIDTH-20));
        if(0==spItem.get()) return;
        spItem->put_attachment(pwzAttachment);
        spItem->put_clients(vClients);
        spItem->put_font(hFont);
        spItem->put_bmp(hBmp);
        m_cells.push_back(spItem);
    }
    LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(lParam);
        HDC  hDC   = (HDC)wParam;
        //HWND hCtrl = (HWND)lParam;
        bCalldefaultHandler = FALSE;
        SetBkColor(hDC, RGB(255,255,255));
        return (LRESULT)m_hWhiteBrush;
    }
    void GetTagList(std::vector<int>& vTagIndices)
    {
        for(std::vector<smart_ptr<TagViewCell>>::iterator it=m_cells.begin();
            it!=m_cells.end(); ++it)
        {
            smart_ptr<TagViewCell> spCell = *it;
            vTagIndices.push_back(spCell->get_selected());
        }
    }

private:
    HBRUSH                  m_hWhiteBrush;
};

template<class T>
class TagDialog : public YBaseWnd
{
public:
    TagDialog(LPCWSTR pwzCaption=NULL, LPVOID pTagActionContext=NULL):
        m_uMainIconID(0), m_pTagActionContext(pTagActionContext)
    {        
        if(NULL!=pwzCaption) m_strCaption=pwzCaption;
        m_strDlgInfo   = TAG_DLG_INFO_01;
        m_uMainIconID  = 0;
        m_uExtMailID   = 0;
        m_uWordBmpID   = 0;
        m_uExcelBmpID  = 0;
        m_uPwptBmpID   = 0;
        m_uPdfBmpID    = 0;
        m_strSendNextBtn = L"OK";
    }
    ~TagDialog(){
        m_vTagData.clear();
    }

    void AddData(LPCWSTR pwzAttachment, LPCWSTR pwzClients)
    {

        m_vTagData.push_back(smart_ptr<TagData>(new TagData(pwzAttachment, pwzClients)));

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
        DWORD   dwStyle = WS_DLGFRAME|WS_POPUP|WS_VISIBLE|WS_CAPTION|WS_TILED|WS_SYSMENU;
        RECT    rcWinPos = {0, 0, TAG_DLG_WIDTH, TAG_DLG_HEIGHT};
        MSG     msg;
        BOOL    fGotMessage;
        m_uResult = IDCANCEL;

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

        if(!Create(TAG_DLG_CLASS_NAME, m_strCaption.length()?m_strCaption.c_str():TAG_DLG_CLASS_NAME, rcWinPos, NULL, WS_EX_CONTEXTHELP, dwStyle, hParent, m_uMainIconID))
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

        return m_uResult;

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
        HANDLE_CTRL_CLICK_MESSAGE(m_hCancel, OnCancel)
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
        RECT rcClient; memset(&rcClient, 0, sizeof(RECT));
        RECT rcView;   memset(&rcView, 0, sizeof(RECT));
        int  i=0, nInfoHeight = 0;
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
        rcClient.left  += TAG_DLG_MARGIN;
        rcClient.right -= TAG_DLG_MARGIN;
        rcClient.top   += TAG_DLG_MARGIN;
        rcClient.bottom-= TAG_DLG_MARGIN;

        int cyOffset = rcClient.top;
        // Create External Mail Icon
        m_hExtMailIcon = CreateWindowW(L"STATIC", L"STATIC", WS_CHILD|WS_VISIBLE|SS_ICON|SS_REALSIZEIMAGE,
            rcClient.left,
            cyOffset,
            TAG_DLG_BIG_ICON_SIZE,
            TAG_DLG_BIG_ICON_SIZE,
            m_hWnd, NULL, m_hInst, NULL);
        m_extMailIcon = LoadIconW(m_hInst, MAKEINTRESOURCEW(m_uExtMailID));
        if(m_uExtMailID && m_hExtMailIcon && m_extMailIcon)
            SendMessage(m_hExtMailIcon, STM_SETIMAGE, (WPARAM)IMAGE_ICON , (LPARAM)m_extMailIcon);
        UpdateWindow(m_hExtMailIcon);

        // Create External Mail Info
        int nTextWidth = rcClient.right-rcClient.left-TAG_DLG_BIG_ICON_SIZE-TAG_DLG_MARGIN;
        int nTextHeight= YLIB::YWinUtility::CountTextAreaHeightByWidth(m_strDlgInfo.c_str(), wndDC.get_DC(), m_hBoldFont, nTextWidth, 0);
        if(nTextHeight<TAG_DLG_BIG_ICON_SIZE) nTextHeight=TAG_DLG_BIG_ICON_SIZE;
        m_hExtMailInfo = CreateWindow(L"STATIC", m_strDlgInfo.c_str(), WS_CHILD|WS_VISIBLE|SS_LEFT,
            rcClient.left+TAG_DLG_BIG_ICON_SIZE+TAG_DLG_MARGIN,
            cyOffset,
            (rcClient.right-rcClient.left-TAG_DLG_BIG_ICON_SIZE-TAG_DLG_MARGIN),
            nTextHeight,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hBoldFont) SendMessage(m_hExtMailInfo, WM_SETFONT, (WPARAM)m_hBoldFont, (LPARAM)TRUE);
        
        // Create View
        cyOffset += TAG_DLG_BIG_ICON_SIZE+TAG_DLG_MARGIN;
        rcView.left   = rcClient.left;
        rcView.right  = rcClient.right;
        rcView.top    = cyOffset;
        rcView.bottom = rcClient.bottom-TAG_DLG_BTN_HEIGHT-TAG_DLG_MARGIN;
        m_tagView.Create(m_hWnd, rcView);

        // Create Send button
        m_hOK = CreateWindow(L"BUTTON", m_strSendNextBtn.c_str()/*L"OK"*//*Text is decided by msoPEP, Gavin, 07/18/08*/, WS_CHILD|WS_VISIBLE,
            (rcClient.left + TAG_DLG_MARGIN),
            (rcClient.bottom-TAG_DLG_BTN_HEIGHT),
            TAG_DLG_BTN_WIDTH,
            TAG_DLG_BTN_HEIGHT,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hButtonFont) SendMessage(m_hOK, WM_SETFONT, (WPARAM)m_hButtonFont, (LPARAM)TRUE);

        // Create Cancel button
        m_hCancel = CreateWindow(L"BUTTON", L"Cancel", WS_CHILD|WS_VISIBLE,
            (rcClient.right-TAG_DLG_BTN_WIDTH-TAG_DLG_MARGIN),
            (rcClient.bottom-TAG_DLG_BTN_HEIGHT),
            TAG_DLG_BTN_WIDTH,
            TAG_DLG_BTN_HEIGHT,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hButtonFont) SendMessage(m_hCancel, WM_SETFONT, (WPARAM)m_hButtonFont, (LPARAM)TRUE);

        for(TAGDATALIST::iterator it=m_vTagData.begin(); it!=m_vTagData.end(); ++it)
        {
            smart_ptr<TagData> spItem = *it;
            const WCHAR* pExt = wcsrchr(spItem->m_strAttachment.c_str(), L'.');
            HBITMAP      hBmp = NULL;
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
            m_tagView.AddItem(hBmp, m_hNormalFont, spItem->m_strAttachment.c_str(), spItem->m_vClients);
        }
        m_tagView.PrepareView();
        m_tagView.CreateView();
        m_tagView.UpdateView();

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
    LRESULT OnOK(BOOL& bCalldefaultHandler)
    {
        std::vector<int> vTagIndices;
        m_tagView.GetTagList(vTagIndices);
        m_tagAction.OnOK(vTagIndices, m_pTagActionContext);

        m_uResult = IDOK;
        return SendMessage(m_hWnd, WM_CLOSE, IDOK, 0);
    }

    LRESULT OnCancel(BOOL& bCalldefaultHandler)
    {
        return SendMessage(m_hWnd, WM_CLOSE, IDCANCEL, 0);
    }

private:
    UINT                    m_uResult;
    std::wstring            m_strCaption;

    std::wstring            m_strDlgInfo;
    std::wstring            m_strHelpUrl;
    std::wstring            m_strSendNextBtn;

    // view
    TagViewWnd              m_tagView;
    T                       m_tagAction;
    LPVOID                  m_pTagActionContext;

    // controls
    HWND                    m_hOK;
    HWND                    m_hCancel;
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

    TAGDATALIST             m_vTagData;
};

#endif
