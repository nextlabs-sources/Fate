

#pragma once
#ifndef _DM_DIALOG_H_
#define _DM_DIALOG_H_
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
#define WM_CELLITEM_CLICK               (WM_USER+100)

#define DM_WND_WIDTH               465
#define DM_WND_HEIGHT              400
#define DM_WND_MARGIN              10
#define DM_WND_BTN_HEIGHT          22
#define DM_WND_BTN_WIDTH           70
#define DM_CLASS_NAME                  L"MORGAN STANLEY DEMO DIALOG"
#define DM_INFO_01                     L"Verify the following external recipients. Select the checkbox for each recipient confirmed."
#define DM_INFO_02                     L"indicates sending a file to an inappropriate client"
#define DM_INFO_03                     L"indicates sending email to multiple clients"
#define DM_MAX_ICON_SIZE                   32
#define DM_SMALL_ICON_SIZE                 16

#define ATTACHMENT_PREFIX               L"Attachment: "
#define CLIENT_PREFIX                   L"Client Name: "

#define DM_CELL_BTN_WIDTH          (103+17) // text width + left/right margins
#define DM_CELL_BTN_HEIGHT         20
#define DM_CELL_MARGIN             8

typedef std::vector<std::pair<std::wstring, std::wstring>>  ATTACHMENTINFO;   

class DmData
{
public:
    DmData(BOOL bDomainMismatch, BOOL bMultiClient, LPCWSTR pwzRecipient, ATTACHMENTINFO& vecAttachInfo)
    {
        m_bDomainMismatch = bDomainMismatch;
        m_bMultiClient = bMultiClient;
        m_strRecipient = pwzRecipient;
        for(ATTACHMENTINFO::iterator it=vecAttachInfo.begin(); it!=vecAttachInfo.end(); ++it)
            m_vecAttachInfo.push_back(*it);
    }
    BOOL         m_bDomainMismatch;
    BOOL         m_bMultiClient;
    std::wstring m_strRecipient;
    ATTACHMENTINFO m_vecAttachInfo;
};

typedef std::vector<smart_ptr<DmData>>    DATAVECTOR;
typedef std::vector<int>                        CHECKEDLIST;

class DmActionBase
{
public:
    DmActionBase(){m_hBtnSend=NULL; m_hBtnCancel=NULL;}
    virtual ~DmActionBase(){m_vData.clear();}
    virtual void OnRemove(int nIndex, HWND hWnd, LPVOID pContext, std::vector<ATTACHMENTINFO>& vecvecAttachInfo) = 0;
    virtual void OnSend(CHECKEDLIST& checkedList, LPVOID pContext) = 0;
    inline void put_SendButtonHwnd(HWND hWnd){m_hBtnSend=hWnd;}
    inline void put_CancelButtonHwnd(HWND hWnd){m_hBtnCancel=hWnd;}
    inline void enable_SendButton(BOOL bEnable){EnableWindow(m_hBtnSend, bEnable);}
    inline void enable_CancelButton(BOOL bEnable){EnableWindow(m_hBtnCancel, bEnable);}
    inline void put_SendButtonText(LPCWSTR pwzText){if(m_hBtnSend && pwzText) SetWindowTextW(m_hBtnSend, pwzText);}
    inline void put_CancelButtonText(LPCWSTR pwzText){if(m_hBtnCancel && pwzText) SetWindowTextW(m_hBtnCancel, pwzText);}

    DATAVECTOR  m_vData;
    HWND        m_hBtnSend;
    HWND        m_hBtnCancel;
};

class DmViewCell : public YViewCell
{
public:
    DmViewCell(HWND hViewWnd, HDC hViewDC, int nWidth) : YViewCell(hViewWnd, hViewDC, nWidth){
        m_hCheck = 0;
        m_hButton= 0;
        m_bDomainMismatch = FALSE;
        m_bMultiClient = FALSE;
        m_bExpand      = FALSE;
        m_nHiddenTextHeight = 0;
        m_nRecipientHeight  = 0;
    }
    ~DmViewCell()
    {
        if(m_hCheck) SendMessageW(m_hCheck, WM_CLOSE, 0, 0);
        if(m_hButton) SendMessageW(m_hButton, WM_CLOSE, 0, 0);
    }

    void  put_DomainMismatch(BOOL bDomainMismatch){m_bDomainMismatch=bDomainMismatch;}

    void  put_MultiClient(BOOL bMultiClient){m_bMultiClient=bMultiClient;}

    int  get_Height()
    {
        return m_nHeight;
    }
    int  get_Width()
    {
        return m_nWidth;
    }
    void UpdateAttachment(ATTACHMENTINFO& vecAttachInfo)
    {
        put_AttachInfo(vecAttachInfo);
        RedrawText();
    }
    void SetRemoved()
    {
        ATTACHMENTINFO info;

        info.push_back(std::pair<std::wstring, std::wstring>
                       (L"Removed", L"Removed"));
        put_AttachInfo(info);

        // We don't use text control any more
        // So re-draw the text on offscreen
        // by Gavin
        RedrawText();
    }
    BOOL PtInExpand(POINT& pt)
    {
        return PtInRect(&m_rcExpand, pt);
    }
    void ExpandCell()
    {
        m_bExpand = TRUE;
        if(m_hButton) ShowWindow(m_hButton, SW_SHOW);
    }
    int  resetCell(int cyOffset)
    {
        m_rcFlag01.top  = cyOffset+DM_CELL_MARGIN*2;    m_rcFlag01.bottom = m_rcFlag01.top + DM_SMALL_ICON_SIZE;
        m_rcFlag02.top  = m_rcFlag01.top;               m_rcFlag02.bottom = m_rcFlag02.top + DM_SMALL_ICON_SIZE;
        m_rcCheckBox.top    = cyOffset+DM_CELL_MARGIN*2;m_rcCheckBox.bottom = m_rcCheckBox.top+DM_CELL_MARGIN*2;
        MoveWindow(m_hCheck, m_rcCheckBox.left, m_rcCheckBox.top, m_rcCheckBox.right-m_rcCheckBox.left, m_rcCheckBox.bottom-m_rcCheckBox.top, TRUE);
        m_rcRecipient.top  = m_rcCheckBox.top - 1;  m_rcRecipient.bottom = m_rcRecipient.top + m_nRecipientHeight;
        m_rcExpand.top   = m_rcFlag02.top;
        m_rcExpand.bottom= m_rcExpand.top + DM_SMALL_ICON_SIZE;
        if(m_bExpand && m_vecAttachInfo.size())
        {
            memset(&m_rcExpand, 0, sizeof(RECT));
            m_rcHiddenText.top  = m_rcRecipient.bottom - DM_CELL_MARGIN/2;   m_rcHiddenText.bottom = m_rcHiddenText.top + m_nHiddenTextHeight;
            m_rcButton.top   = m_rcHiddenText.bottom;         m_rcButton.bottom = m_rcButton.top + DM_CELL_BTN_HEIGHT;
            MoveWindow(m_hButton, m_rcButton.left, m_rcButton.top, m_rcButton.right-m_rcButton.left, m_rcButton.bottom-m_rcButton.top, TRUE);
            m_nHeight = (DM_CELL_MARGIN + m_nHiddenTextHeight+DM_CELL_MARGIN*2+DM_CELL_BTN_HEIGHT);
        }
        return m_nHeight;
    }
    int  prepare_Cell(int cyOffset)
    {
        int i = 0;
        m_nOffset = cyOffset;

        // Icon rect
        memset(&m_rcFlag01, 0, sizeof(RECT));
        memset(&m_rcFlag02, 0, sizeof(RECT));
        m_rcFlag01.left = DM_CELL_MARGIN;              m_rcFlag01.right  = m_rcFlag01.left + DM_SMALL_ICON_SIZE;
        m_rcFlag01.top  = cyOffset+DM_CELL_MARGIN*2;   m_rcFlag01.bottom = m_rcFlag01.top + DM_SMALL_ICON_SIZE;
        m_rcFlag02.left = m_rcFlag01.right + DM_CELL_MARGIN/2;        m_rcFlag02.right  = m_rcFlag02.left + DM_SMALL_ICON_SIZE;
        m_rcFlag02.top  = m_rcFlag01.top;              m_rcFlag02.bottom = m_rcFlag02.top + DM_SMALL_ICON_SIZE;

        // check box rect
        m_rcCheckBox.left   = m_rcFlag02.right+DM_CELL_MARGIN*2;
        m_rcCheckBox.right  = m_rcCheckBox.left+DM_CELL_MARGIN*2;
        m_rcCheckBox.top    = cyOffset+DM_CELL_MARGIN*2;
        m_rcCheckBox.bottom = m_rcCheckBox.top+DM_CELL_MARGIN*2;
        m_hCheck = CreateWindowW(L"BUTTON", L"", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_MULTILINE|BS_TOP,
            m_rcCheckBox.left,
            m_rcCheckBox.top,
            (m_rcCheckBox.right-m_rcCheckBox.left),
            (m_rcCheckBox.bottom-m_rcCheckBox.top),
            m_hViewWnd, NULL, YBaseWnd::get_Instance(), NULL);
        if(m_hFont) SendMessage(m_hCheck, WM_SETFONT, (WPARAM)m_hFont, (LPARAM)TRUE);

        // Expand RECT
        m_rcExpand.right = m_nWidth - YScrollBar::get_CXVScroll();
        m_rcExpand.left  = m_rcExpand.right - DM_CELL_BTN_WIDTH;
        m_rcExpand.top   = m_rcFlag02.top;
        m_rcExpand.bottom= m_rcExpand.top + DM_SMALL_ICON_SIZE;

        int nTextWidth = m_nWidth-DM_CELL_MARGIN*6-DM_SMALL_ICON_SIZE-DM_CELL_BTN_WIDTH-2*YScrollBar::get_CXVScroll();
        // recipient RECT
        m_nRecipientHeight = YLIB::YWinUtility::CountTextAreaHeightByWidth(m_strRecipient.c_str(), m_hViewDC, m_hFont, nTextWidth, 0);
        m_nRecipientHeight = max(m_nRecipientHeight, DM_SMALL_ICON_SIZE);
        m_rcRecipient.left = m_rcCheckBox.right;    m_rcRecipient.right = m_rcRecipient.left + nTextWidth;
        m_rcRecipient.top  = m_rcCheckBox.top - 1;  m_rcRecipient.bottom = m_rcRecipient.top + m_nRecipientHeight;

        m_nHeight = DM_CELL_MARGIN*2 + m_nRecipientHeight;

        // Hidden Text RECT
        int nVecSize = (int)m_vecAttachInfo.size();
        m_strHiddenText = L"\n";                // Increase the space, Gavin, 07/18/08
        if(nVecSize)
        {
            for(i=0; i<nVecSize; i++)
            {
                WCHAR wzIndex[10]; memset(wzIndex, 0, sizeof(wzIndex));
                _itow_s(i+1, wzIndex, 9, 10);
                m_strHiddenText += wzIndex;
                m_strHiddenText += i<10?L".  ":L". ";
                m_strHiddenText += m_vecAttachInfo[i].first.c_str();
                m_strHiddenText += L"\n     Client: ";
                m_strHiddenText += m_vecAttachInfo[i].second.c_str();
                if(i != nVecSize-1)
                    m_strHiddenText += L"\n\n"; // Increase the space, Gavin, 07/18/08
            }
            nTextWidth = m_nWidth-DM_CELL_MARGIN*6-DM_SMALL_ICON_SIZE-2*YScrollBar::get_CXVScroll();
            m_nHiddenTextHeight = YLIB::YWinUtility::CountTextAreaHeightByWidth(m_strHiddenText.c_str(), m_hViewDC, m_hFont, nTextWidth, 0);
            m_rcHiddenText.left = m_rcRecipient.left + DM_CELL_MARGIN;       m_rcHiddenText.right = m_rcHiddenText.left + nTextWidth;
            m_rcHiddenText.top  = m_rcRecipient.bottom - DM_CELL_MARGIN/2;   m_rcHiddenText.bottom = m_rcHiddenText.top + m_nHiddenTextHeight;

            // button rect
            m_rcButton.right = m_nWidth - DM_CELL_MARGIN*2 - 2*YScrollBar::get_CXVScroll();
            m_rcButton.left  = m_rcButton.right - DM_CELL_BTN_WIDTH;
            m_rcButton.top   = m_rcHiddenText.bottom-DM_CELL_MARGIN;
            m_rcButton.bottom = m_rcButton.top + DM_CELL_BTN_HEIGHT;
            m_hButton = CreateWindow(L"BUTTON", L"Remove Attachments", WS_CHILD,
                    m_rcButton.left,
                    m_rcButton.top,
                    DM_CELL_BTN_WIDTH,
                    DM_CELL_BTN_HEIGHT,
                    m_hViewWnd, NULL, YBaseWnd::get_Instance(), NULL);
            if(m_hFont) SendMessage(m_hButton, WM_SETFONT, (WPARAM)m_hFont, (LPARAM)TRUE);

            if(m_bExpand) m_nHeight += DM_CELL_MARGIN + m_nHiddenTextHeight+DM_CELL_MARGIN+DM_CELL_BTN_HEIGHT;
        }

        m_nHeight += DM_CELL_MARGIN;

        return m_nHeight;
    }
    int get_ExpandHeight()
    {
        return (DM_CELL_MARGIN + m_nHiddenTextHeight+DM_CELL_MARGIN*3+DM_CELL_BTN_HEIGHT);
    }
    void render_Cell()
    {
        if(m_bMultiClient && 0!=m_rcFlag01.left && 0!=m_rcFlag01.top && 0!=m_rcFlag01.right && 0!=m_rcFlag01.bottom)
        {
            HDC     hFlagDc = CreateCompatibleDC(m_hViewDC);
            HBITMAP hOldFlag= (HBITMAP)SelectObject(hFlagDc, m_hBmpMultiClient);
            BitBlt(m_hViewDC, m_rcFlag01.left, m_rcFlag01.top, DM_SMALL_ICON_SIZE, DM_SMALL_ICON_SIZE, hFlagDc, 0, 0, SRCCOPY);
            SelectObject(hFlagDc, hOldFlag);
            DeleteObject(hFlagDc);
        }

        if(m_bDomainMismatch && 0!=m_rcFlag02.left && 0!=m_rcFlag02.top && 0!=m_rcFlag02.right && 0!=m_rcFlag02.bottom)
        {
            HDC     hFlagDc = CreateCompatibleDC(m_hViewDC);
            HBITMAP hOldFlag= (HBITMAP)SelectObject(hFlagDc, m_hBmpDomainMismatch);
            BitBlt(m_hViewDC, m_rcFlag02.left, m_rcFlag02.top, DM_SMALL_ICON_SIZE, DM_SMALL_ICON_SIZE, hFlagDc, 0, 0, SRCCOPY);
            SelectObject(hFlagDc, hOldFlag);
            DeleteObject(hFlagDc);
        }

        // Draw Recipient
        HFONT hOldFont = (HFONT)SelectObject(m_hViewDC, m_hFont);
        SetBkMode(m_hViewDC, TRANSPARENT);
        SetTextColor(m_hViewDC, RGB(0, 0, 210));

        wchar_t wzAttachInfoTitle[] = {L'A', L't', L't', L'a', L'c', L'h', L'm', L'e', L'n', L't', L'I', L'n', L'f', L'o', L'\0' };
        if(!m_bExpand && m_vecAttachInfo.size()) DrawTextExW(m_hViewDC, wzAttachInfoTitle, -1, &m_rcExpand, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK, NULL);
        SetTextColor(m_hViewDC, RGB(64, 64, 64));
        DrawTextEx(m_hViewDC, (LPWSTR)m_strRecipient.c_str(), -1, &m_rcRecipient, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK, NULL);
        if(m_bExpand) DrawTextEx(m_hViewDC, (LPWSTR)m_strHiddenText.c_str(), -1, &m_rcHiddenText, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK, NULL);
        SelectObject(m_hViewDC, hOldFont);
    }
    // This function will update the text
    void RedrawText()
    {
        int nVecSize = (int)m_vecAttachInfo.size();
        m_strHiddenText = L"\n";                // Increase the space, Gavin, 07/18/08
        if(nVecSize)
        {
            for(int i=0; i<nVecSize; i++)
            {
                WCHAR wzIndex[10]; memset(wzIndex, 0, sizeof(wzIndex));
                _itow_s(i+1, wzIndex, 9, 10);
                m_strHiddenText += wzIndex;
                m_strHiddenText += i<10?L".  ":L". ";
                m_strHiddenText += m_vecAttachInfo[i].first.c_str();
                m_strHiddenText += L"\n     Client: ";
                m_strHiddenText += m_vecAttachInfo[i].second.c_str();
                if(i != nVecSize-1)
                    m_strHiddenText += L"\n\n"; // Increase the space, Gavin, 07/18/08
            }
        }

		if(!m_bExpand)
			return;

        HBRUSH hWhiteBrush = CreateSolidBrush(RGB(255,255,255));
        if(NULL != hWhiteBrush)
        {
            FillRect(m_hViewDC, &m_rcHiddenText, hWhiteBrush);
            DeleteObject(hWhiteBrush);
        }
        
        HFONT hOldFont = (HFONT)SelectObject(m_hViewDC, m_hFont);
        SetBkMode(m_hViewDC, TRANSPARENT);
        SetTextColor(m_hViewDC, RGB(64, 64, 64));
        DrawTextEx(m_hViewDC, (LPWSTR)m_strHiddenText.c_str(), -1, &m_rcHiddenText, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK, NULL);
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

        memcpy(&rcPos, &m_rcCheckBox, sizeof(RECT));
        OffsetRect(&rcPos, 0-cxOffset, 0-cyOffset);
        SetWindowPos(m_hCheck, HWND_BOTTOM, rcPos.left, rcPos.top, 0, 0, SWP_NOSIZE);
        return m_nHeight;
    }
    WORD find_cellctrl(HWND hWnd)
    {
        if(m_hCheck == hWnd)
            return 1;
        else if(m_hButton == hWnd)
            return 2;
        else
            return 0xFFFF;
    }
    BOOL is_checked()
    {
        if(BST_CHECKED == SendMessageW(m_hCheck, BM_GETCHECK, 0,0))
            return TRUE;
        return FALSE;
    }
    void EnableRemoveButton(BOOL bEnable)
    {
        if (NULL != m_hButton)
            EnableWindow(m_hButton, bEnable);
    }

    //
public:
    void put_recipient(LPCWSTR  pwzRecipient){
        m_strRecipient = pwzRecipient;
    }
    void put_AttachInfo(ATTACHMENTINFO& vecAttachInfo)
    {
        m_vecAttachInfo.clear();
        for(ATTACHMENTINFO::iterator it=vecAttachInfo.begin(); it!=vecAttachInfo.end(); ++it)
            m_vecAttachInfo.push_back(*it);
    }
    void put_font(HFONT hFont){m_hFont=hFont;}
    void put_DomainMismatchIcon(HBITMAP hBmp){m_hBmpDomainMismatch=hBmp;}
    void put_MultiClientIcon(HBITMAP hBmp){m_hBmpMultiClient=hBmp;}

private:
    HBITMAP  m_hBmpDomainMismatch;
    HBITMAP  m_hBmpMultiClient;

    RECT            m_rcFlag01;
    RECT            m_rcFlag02;
    RECT            m_rcExpand;
    RECT            m_rcCheckBox;
    RECT            m_rcRecipient;
    RECT            m_rcHiddenText;
    RECT            m_rcButton;

    int             m_nRecipientHeight;
    int             m_nHiddenTextHeight;

    HFONT           m_hFont;
    std::wstring    m_strRecipient;
    std::wstring    m_strHiddenText;
    ATTACHMENTINFO  m_vecAttachInfo;

    HWND            m_hCheck;
    HWND            m_hButton;
    BOOL            m_bDomainMismatch;
    BOOL            m_bMultiClient;
    BOOL            m_bExpand;
};

class DmViewWnd : public YViewWnd<DmViewCell>
{
public:
    DmViewWnd()
    {
        m_hWhiteBrush = CreateSolidBrush(RGB(255,255,255));
    }

    virtual ~DmViewWnd()
    {
        if(m_hWhiteBrush) DeleteObject(m_hWhiteBrush);
    }
    void AddItem(BOOL isDomainMismatch, BOOL isMultiClient, HBITMAP hDomainMismatchIcon, HBITMAP hMultiClientIcon, HFONT hFont, LPCWSTR pwzRecipient, ATTACHMENTINFO& vecAttachInfo)
    {
        smart_ptr<DmViewCell> spItem( new DmViewCell(m_hViewWnd, m_offscreenDC, DM_WND_WIDTH-20));
        if(0==spItem.get()) return;
        spItem->put_DomainMismatch(isDomainMismatch);
        spItem->put_MultiClient(isMultiClient);
        spItem->put_recipient(pwzRecipient);
        spItem->put_AttachInfo(vecAttachInfo);
        spItem->put_font(hFont);
        spItem->put_DomainMismatchIcon(hDomainMismatchIcon);
        spItem->put_MultiClientIcon(hMultiClientIcon);
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
    virtual LRESULT OnClickWnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(uMsg);
        bCalldefaultHandler = FALSE;
        if(MK_LBUTTON==wParam)
        {
            POINT ptClick;            
            ptClick.x = LOWORD(lParam) + m_viewOffset.cx;
            ptClick.y = HIWORD(lParam) + m_viewOffset.cy;
            int nCellCount = (int)m_cells.size();
            for (int i=0; i<nCellCount; i++)
            {
                if(m_cells[i]->PtInExpand(ptClick))
                {
                    m_cells[i]->ExpandCell();
                    break;
                }
            }
            int cy = 0;
            for (int i=0; i<nCellCount; i++)
            {
                cy += m_cells[i]->resetCell(cy);
            }
            renderView();
            UpdateWindow(m_hWnd);
            UpdateView();
            RelocateCtrls(m_viewOffset.cx, m_viewOffset.cy);
        }
        return 0L;
    }
    LRESULT OnCtrlClick(HWND hWnd, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(bCalldefaultHandler);
        DWORD wParam = FindCellCtrl(hWnd);
        if(0xFFFFFFFF!=wParam)
            return SendMessageW(m_hParent, WM_CELLITEM_CLICK, (WPARAM)wParam, (LPARAM)hWnd);
        return 0L;
    }
    BOOL IsAnyCellSelected()
    {
        for(std::vector<smart_ptr<DmViewCell>>::iterator it=m_cells.begin();
            it!=m_cells.end(); ++it)
        {
            smart_ptr<DmViewCell> spCell = *it;
            if(spCell->is_checked())
                return TRUE;
        }
        return FALSE;
    }
    void GetCheckedList(CHECKEDLIST& checkedList)
    {
        checkedList.clear();
        int nIndex = 0;
        for(std::vector<smart_ptr<DmViewCell>>::iterator it=m_cells.begin();
            it!=m_cells.end(); ++it)
        {
            smart_ptr<DmViewCell> spCell = *it;
            if(spCell->is_checked())
            {
                checkedList.push_back(nIndex);
            }
            nIndex++;
        }
    }
    void SetAttachRemoved(int nIndex)
    {
        int nCount = (int)m_cells.size();
        if(nIndex>=0 && nIndex<nCount)
        {
            smart_ptr<DmViewCell> spItem = m_cells[nIndex];
            spItem->SetRemoved();
        }
    }

    void UpdateAllAttachments(std::vector<ATTACHMENTINFO>& vecvecAttachInfo)
    {
        int nIndex = 0;

        for(std::vector<smart_ptr<DmViewCell>>::iterator it=m_cells.begin();
            it!=m_cells.end(); ++it)
        {
            smart_ptr<DmViewCell> spCell = *it;
            BOOL bHasAttachment = (vecvecAttachInfo[nIndex].size() > 0);

            if (bHasAttachment)
            {
                spCell->UpdateAttachment(vecvecAttachInfo[nIndex]);
            }
            else
            {
                spCell->SetRemoved();
            }

            spCell->EnableRemoveButton(bHasAttachment);
            nIndex++;
        }

        // Re-draw the cell text
        RECT rect; memset(&rect, 0, sizeof(RECT));
        GetClientRect(m_hWnd, &rect);
        InvalidateRect(m_hWnd, &rect, TRUE);
    }

private:
    HBRUSH                  m_hWhiteBrush;
};



template<class T>
class DmDialog : public YBaseWnd
{
public:
    DmDialog(LPCWSTR pwzCaption=NULL, LPVOID pDmActionContext=NULL):
        m_uResult(IDCANCEL), m_uMainIconID(0),
        m_pDmActionContext(pDmActionContext)
    {        
        if(NULL!=pwzCaption) m_strCaption=pwzCaption;
        m_strDlgInfo    = DM_INFO_01;
        m_uMainIconID   = 0;
        m_uExtMailID    = 0;
        m_uDomainMismatchID = 0;
        m_uMultiClientID = 0;
        m_uDomainMismatchBmpID = 0;
        m_uMultiClientBmpID = 0;
        m_multiClientBmp  = 0;
        m_domainMismatchBmp = 0;
        m_strNextOKText = L"Send Email";
        m_strCancelText = L"Cancel";
    }
    ~DmDialog(){
        //m_vData.clear();
    }

    void AddData(BOOL bMultiClient, LPCWSTR pwzRecipient, ATTACHMENTINFO& vecAttachInfo)
    {
        BOOL bDomainMismatch = (0 < vecAttachInfo.size());
        m_dmAction.m_vData.push_back(smart_ptr<DmData>(new DmData(bDomainMismatch, bMultiClient, pwzRecipient, vecAttachInfo)));
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
        RECT    rcWinPos = {0, 0, DM_WND_WIDTH, DM_WND_HEIGHT};
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

        if(!Create(DM_CLASS_NAME, m_strCaption.length()?m_strCaption.c_str():DM_CLASS_NAME, rcWinPos, NULL, WS_EX_CONTEXTHELP, dwStyle, hParent, m_uMainIconID))
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
    inline void put_DomainMismatchID(UINT uID){m_uDomainMismatchID=uID;}
    inline void put_MultiClientID(UINT uID){m_uMultiClientID=uID;}
    inline void put_DomainMismatchBmpID(UINT uID){m_uDomainMismatchBmpID=uID;}
    inline void put_MultiClientBmpID(UINT uID){m_uMultiClientBmpID=uID;}
    inline void put_SendButtonText(LPCWSTR pwzText){if(pwzText) m_strNextOKText=pwzText;}
    inline void put_CancelButtonText(LPCWSTR pwzText){if(pwzText) m_strCancelText=pwzText;}
    inline void put_HelpUrl(LPCWSTR pwzHelpUrl){if(pwzHelpUrl)m_strHelpUrl=pwzHelpUrl;}

protected:
    MESSAGE_PROCESS_START()
        HANDLE_MESSAGE(WM_CREATE, OnCreate)
        HANDLE_MESSAGE(WM_PAINT, OnPaint)
        HANDLE_MESSAGE(WM_ERASEBKGND, OnErasBkGnd)
        HANDLE_MESSAGE(WM_CELLITEM_CLICK, OnCtrlClick)
        HANDLE_CTRL_CLICK_MESSAGE(m_hCancel, OnCancel)
        HANDLE_CTRL_CLICK_MESSAGE(m_hNextOK, OnNextOK)
        HANDLE_SYSCOMMAND_MESSAGE(SC_CONTEXTHELP, OnHelpContext)
        MESSAGE_PROCESS_END()

protected:
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        RECT rcClient; memset(&rcClient, 0, sizeof(RECT));
        RECT rcView;   memset(&rcView, 0, sizeof(RECT));
        int  i=0, nInfoHeight = 0;
        YLIB::WindowDC wndDC(m_hWnd);

        // Set Window's body font
        if(m_hNormalFont) SendMessage(m_hWnd, WM_SETFONT, (WPARAM)m_hNormalFont, (LPARAM)TRUE);

        // Get current RECT
        GetClientRect(m_hWnd, &rcClient);
        rcClient.left  += DM_WND_MARGIN;
        rcClient.right -= DM_WND_MARGIN;
        rcClient.top   += DM_WND_MARGIN;
        rcClient.bottom-= DM_WND_MARGIN;

        int cyOffset = rcClient.top;
        // Create External Mail Icon
        m_hExtMailIcon = CreateWindowW(L"STATIC", L"STATIC", WS_CHILD|WS_VISIBLE|SS_ICON|SS_REALSIZEIMAGE,
            rcClient.left,
            cyOffset,
            DM_MAX_ICON_SIZE,
            DM_MAX_ICON_SIZE,
            m_hWnd, NULL, m_hInst, NULL);
        m_extMailIcon = LoadIconW(m_hInst, MAKEINTRESOURCEW(m_uExtMailID));
        if(m_uExtMailID && m_hExtMailIcon && m_extMailIcon)
            SendMessage(m_hExtMailIcon, STM_SETIMAGE, (WPARAM)IMAGE_ICON , (LPARAM)m_extMailIcon);
        UpdateWindow(m_hExtMailIcon);

		// Create External Mail Info
        int nTextWidth = rcClient.right-rcClient.left-DM_MAX_ICON_SIZE-DM_WND_MARGIN;
        int nTextHeight= YLIB::YWinUtility::CountTextAreaHeightByWidth(m_strDlgInfo.c_str(), wndDC.get_DC(), m_hBoldFont, nTextWidth, 0);
        if(nTextHeight<DM_MAX_ICON_SIZE) nTextHeight=DM_MAX_ICON_SIZE;
        m_hExtMailInfo = CreateWindow(L"STATIC", m_strDlgInfo.c_str(), WS_CHILD|WS_VISIBLE|SS_LEFT,
            rcClient.left+DM_MAX_ICON_SIZE+DM_WND_MARGIN,
            cyOffset,
            (rcClient.right-rcClient.left-DM_MAX_ICON_SIZE-DM_WND_MARGIN),
            nTextHeight,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hBoldFont) SendMessage(m_hExtMailInfo, WM_SETFONT, (WPARAM)m_hBoldFont, (LPARAM)TRUE);

        // Create Line // height = 10
        cyOffset += DM_MAX_ICON_SIZE+DM_WND_MARGIN/2;
        CreateWindowW(L"STATIC", L"STATIC", WS_CHILD|WS_VISIBLE|SS_ETCHEDHORZ,
            rcClient.left,
            cyOffset-DM_WND_MARGIN/2,
            (rcClient.right-rcClient.left),
            2,
            m_hWnd, NULL, m_hInst, NULL);

        // Create Domain Mismatch icon   // height = 16
        m_hDomainMismatchIcon = CreateWindowW(L"STATIC", L"STATIC", WS_CHILD|WS_VISIBLE|SS_ICON|SS_REALSIZEIMAGE,
            rcClient.left+DM_WND_MARGIN,
            cyOffset,
            DM_SMALL_ICON_SIZE,
            DM_SMALL_ICON_SIZE,
            m_hWnd, NULL, m_hInst, NULL);

        m_domainMismatchIcon = (HICON)LoadImage(m_hInst, MAKEINTRESOURCEW(m_uDomainMismatchID), IMAGE_ICON, DM_SMALL_ICON_SIZE, DM_SMALL_ICON_SIZE, 0);
        if(m_uDomainMismatchID && m_hDomainMismatchIcon && m_domainMismatchIcon)
            SendMessage(m_hDomainMismatchIcon, STM_SETIMAGE, (WPARAM)IMAGE_ICON , (LPARAM)m_domainMismatchIcon);
        UpdateWindow(m_hDomainMismatchIcon);

        // Create External Mail Info
        m_hDomainMismatchInfo = CreateWindow(L"STATIC", DM_INFO_02, WS_CHILD|WS_VISIBLE|SS_LEFT,
            rcClient.left+DM_WND_MARGIN*2 + DM_SMALL_ICON_SIZE,
            cyOffset+2,
            (rcClient.right-rcClient.left-DM_WND_MARGIN*2-DM_SMALL_ICON_SIZE),
            DM_SMALL_ICON_SIZE,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hNormalFont) SendMessage(m_hDomainMismatchInfo, WM_SETFONT, (WPARAM)m_hNormalFont, (LPARAM)TRUE);

        // Create Multi Client icon   // height = 16
        cyOffset += DM_SMALL_ICON_SIZE;
        m_hMultiClientIcon = CreateWindowW(L"STATIC", L"STATIC", WS_CHILD|WS_VISIBLE|SS_ICON,//|SS_REALSIZEIMAGE,
            rcClient.left+DM_WND_MARGIN,
            cyOffset,
            DM_SMALL_ICON_SIZE,
            DM_SMALL_ICON_SIZE,
            m_hWnd, NULL, m_hInst, NULL);
        m_multiClientIcon = (HICON)LoadImage(m_hInst, MAKEINTRESOURCEW(m_uMultiClientID), IMAGE_ICON, 16, 16, 0);
        if(m_uMultiClientID && m_hMultiClientIcon && m_multiClientIcon)
            SendMessage(m_hMultiClientIcon, STM_SETIMAGE, (WPARAM)IMAGE_ICON , (LPARAM)m_multiClientIcon);
        UpdateWindow(m_hMultiClientIcon);

        // Create External Mail Info
        m_hMultiClientInfo = CreateWindow(L"STATIC", DM_INFO_03, WS_CHILD|WS_VISIBLE|SS_LEFT,
            rcClient.left+DM_WND_MARGIN*2 + DM_SMALL_ICON_SIZE,
            cyOffset+2,
            (rcClient.right-rcClient.left-DM_WND_MARGIN*2-DM_SMALL_ICON_SIZE),
            DM_SMALL_ICON_SIZE,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hNormalFont) SendMessage(m_hMultiClientInfo, WM_SETFONT, (WPARAM)m_hNormalFont, (LPARAM)TRUE);

        // Load BMP
        m_multiClientBmp  = ::LoadBitmapW(m_hInst, MAKEINTRESOURCEW(m_uMultiClientBmpID));
        m_domainMismatchBmp = ::LoadBitmapW(m_hInst, MAKEINTRESOURCEW(m_uDomainMismatchBmpID));

        // Create View
        cyOffset += DM_WND_MARGIN*2+DM_WND_MARGIN/2;
        rcView.left   = rcClient.left;
        rcView.right  = rcClient.right;
        rcView.top    = cyOffset;
        rcView.bottom = rcClient.bottom-DM_WND_BTN_HEIGHT-DM_WND_MARGIN;
        m_dmView.Create(m_hWnd, rcView);

        // Create Send button
        m_hNextOK = CreateWindow(L"BUTTON", m_strNextOKText.c_str(), WS_CHILD|WS_VISIBLE,
            (rcClient.left + DM_WND_MARGIN),
            (rcClient.bottom-DM_WND_BTN_HEIGHT),
            DM_WND_BTN_WIDTH,
            DM_WND_BTN_HEIGHT,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hButtonFont) SendMessage(m_hNextOK, WM_SETFONT, (WPARAM)m_hButtonFont, (LPARAM)TRUE);
        EnableWindow(m_hNextOK, FALSE);
        m_dmAction.put_SendButtonHwnd(m_hNextOK);

        // Create Cancel button
        m_hCancel = CreateWindow(L"BUTTON", m_strCancelText.c_str(), WS_CHILD|WS_VISIBLE,
            (rcClient.right-DM_WND_BTN_WIDTH-DM_WND_MARGIN),
            (rcClient.bottom-DM_WND_BTN_HEIGHT),
            DM_WND_BTN_WIDTH,
            DM_WND_BTN_HEIGHT,
            m_hWnd, NULL, m_hInst, NULL);
        if(m_hButtonFont) SendMessage(m_hCancel, WM_SETFONT, (WPARAM)m_hButtonFont, (LPARAM)TRUE);
        m_dmAction.put_CancelButtonHwnd(m_hCancel);

        for (DATAVECTOR::iterator it=m_dmAction.m_vData.begin(); it!=m_dmAction.m_vData.end(); ++it)
        {
            smart_ptr<DmData> spData = *it;
            m_dmView.AddItem(spData->m_bDomainMismatch, spData->m_bMultiClient, m_domainMismatchBmp, m_multiClientBmp, m_hNormalFont,
                spData->m_strRecipient.c_str(), spData->m_vecAttachInfo);
        }
        m_dmView.PrepareView();
        m_dmView.CreateView();
        m_dmView.UpdateView();

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
        CHECKEDLIST checkedList;
        m_dmView.GetCheckedList(checkedList);
        m_dmAction.OnSend(checkedList, m_pDmActionContext);
        m_uResult = IDOK;
        return SendMessage(m_hWnd, WM_CLOSE, IDOK, 0);
    }

    LRESULT OnCtrlClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        bCalldefaultHandler = FALSE;
        WORD wCellItem = (WORD)((wParam&0xFFFF0000)>>16);
        WORD wCtrlItem = (WORD)(wParam&0x0000FFFF);
        HWND hCtrl     = (HWND)lParam;

        if(0 == wCtrlItem)      // click image
        {
            // don't handle it
        }
        else if(1 == wCtrlItem)     // click check box
        {
            // get status after user click
            LRESULT dwCheckStatus = SendMessageW(hCtrl, BM_GETCHECK, 0, 0);

            if(BST_CHECKED==dwCheckStatus)
            {
                //MessageBoxW(m_hWnd, L"it is checked!", L"Note", MB_OK);
                EnableWindow(m_hNextOK, TRUE);
            }
            else if(BST_UNCHECKED==dwCheckStatus)

            {
                if(!m_dmView.IsAnyCellSelected())
                    EnableWindow(m_hNextOK, FALSE);
                //MessageBoxW(m_hWnd, L"it is unchecked!", L"Note", MB_OK);
            }
            else if(BST_INDETERMINATE==dwCheckStatus)
                MessageBoxW(m_hWnd, L"it is grayed!", L"Note", MB_OK);
            else
                MessageBoxW(m_hWnd, L"unknown status!", L"Note", MB_OK);
        }
        else if(2 == wCtrlItem)     // click remove button
        {
            // remove
            std::vector<ATTACHMENTINFO> vecvecAttachInfo;
            m_dmAction.OnRemove(wCellItem, hCtrl, m_pDmActionContext,
                                vecvecAttachInfo);
            // reset text
            m_dmView.UpdateAllAttachments(vecvecAttachInfo);
        }
        else                        // unknown click
        {
        }

        return 0L;
    }

protected:

private:
    UINT                    m_uResult;
    std::wstring            m_strCaption;
    std::wstring            m_strDlgInfo;

    // view
    DmViewWnd               m_dmView;
    T                       m_dmAction;
    LPVOID                  m_pDmActionContext;

    // controls
    HWND                    m_hNextOK;
    HWND                    m_hCancel;
    HWND                    m_hExtMailIcon;
    HWND                    m_hExtMailInfo;
    HWND                    m_hDomainMismatchIcon;
    HWND                    m_hDomainMismatchInfo;
    HWND                    m_hMultiClientIcon;
    HWND                    m_hMultiClientInfo;
    std::wstring            m_strNextOKText;
    std::wstring            m_strCancelText;

    // Resource ID
    UINT                    m_uMainIconID;
    UINT                    m_uExtMailID;
    UINT                    m_uDomainMismatchID;
    UINT                    m_uMultiClientID;
    UINT                    m_uDomainMismatchBmpID;
    UINT                    m_uMultiClientBmpID;

    // Font
    HFONT                   m_hButtonFont;
    HFONT                   m_hBoldFont;
    HFONT                   m_hNormalFont;

    // Icon
    HICON                   m_domainMismatchIcon;
    HICON                   m_multiClientIcon;
    HICON                   m_extMailIcon;

    HBITMAP                 m_multiClientBmp;
    HBITMAP                 m_domainMismatchBmp;

    std::wstring            m_strHelpUrl;
};

#endif