
#include <Windows.h>
#include <Windowsx.h>
#include <assert.h>
#include <Prsht.h>

#include <list>

//#include "commonui.hpp"
#include "dlgtemplate.hpp"

extern HINSTANCE g_hInst;

CDlgTemplate::CDlgTemplate() : _hWnd(NULL), _AutoRelease(FALSE), _dlgId(0)
{
}

CDlgTemplate::CDlgTemplate(_In_ UINT uDlgId) : _hWnd(NULL), _AutoRelease(FALSE), _dlgId(uDlgId)
{
}

CDlgTemplate::~CDlgTemplate()
{
}

int CDlgTemplate::DoModal(_In_opt_ HWND hParent)
{
	return (int)::DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(_dlgId), hParent, DlgProc, (LPARAM)this);
}

INT_PTR CDlgTemplate::MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR nRet = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        return OnInitialize();

    case WM_DESTROY:
        OnDestroy();
        SetWindowLongPtrW(_hWnd, DWLP_USER, NULL);
        if (AutoRelease()) {
            delete this;
        }
        break;

    case WM_CLOSE:
        nRet = OnClose();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            OnOk();
            return TRUE;
        case IDCANCEL:
            OnCancel();
            return TRUE;
        default:
            nRet = OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
            break;
        }
        break;

    case WM_NOTIFY:
        nRet = OnNotify((LPNMHDR)lParam);
        break;

    case WM_ERASEBKGND:
        nRet = OnEraseBkGnd((HDC)wParam);
        break;

    case WM_DRAWITEM:
        nRet = OnOwnerDraw((DRAWITEMSTRUCT*)lParam);
        break;

    case WM_MEASUREITEM:
        nRet = OnMeasureItem((MEASUREITEMSTRUCT*)lParam);
        break;

    default:
        nRet = FALSE;
        break;
    }

    return nRet;
}

INT_PTR WINAPI CDlgTemplate::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDlgTemplate*   pDlg = NULL;

    if (WM_INITDIALOG == uMsg) {
        SetWindowLongPtrW(hWnd, DWLP_USER, (LONG_PTR)lParam);
        pDlg = reinterpret_cast<CDlgTemplate*>(lParam);
        if(NULL == pDlg) {
            return FALSE;
        }
        pDlg->SetHwnd(hWnd);
    }
    else {
        pDlg = reinterpret_cast<CDlgTemplate*>(GetWindowLongPtrW(hWnd, DWLP_USER));
    }

    if(NULL == pDlg) {
        return FALSE;
    }

    return pDlg->MessageHandler(hWnd, uMsg, wParam, lParam);
}

INT_PTR CPropPageDlgTemplate::MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR nRet = CDlgTemplate::MessageHandler(hWnd, uMsg, wParam, lParam);
    
    switch (uMsg)
    {
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
            OnPsnSetActive((LPPSHNOTIFY)lParam);
            break;
        case PSN_KILLACTIVE:
            OnPsnKillActive((LPPSHNOTIFY)lParam);
            break;
        case PSN_APPLY:
            OnPsnApply((LPPSHNOTIFY)lParam);
            break;
        case PSN_RESET:
            OnPsnReset((LPPSHNOTIFY)lParam);
            break;
        case PSN_HELP:
            OnPsnHelp((LPPSHNOTIFY)lParam);
            break;
        case PSN_WIZBACK:
            OnPsnWizBack((LPPSHNOTIFY)lParam);
            break;
        case PSN_WIZNEXT:
            OnPsnWizNext((LPPSHNOTIFY)lParam);
            break;
        case PSN_WIZFINISH:
            OnPsnWizFinish((LPPSHNOTIFY)lParam);
            break;
        case PSN_QUERYCANCEL:
            OnPsnQueryCancel((LPPSHNOTIFY)lParam);
            break;
        case PSN_GETOBJECT:
            OnPsnGetObject((LPPSHNOTIFY)lParam);
            break;
        case PSN_TRANSLATEACCELERATOR:
            OnPsnTranslateAccelerator((LPPSHNOTIFY)lParam);
            break;
        case PSN_QUERYINITIALFOCUS:
            OnPsnQueryInitialFocus((LPPSHNOTIFY)lParam);
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    return nRet;
}

INT_PTR WINAPI CPropPageDlgTemplate::PropPageDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CPropPageDlgTemplate*   pDlg = NULL;
    
    if (WM_INITDIALOG == uMsg) {
        pDlg = reinterpret_cast<CPropPageDlgTemplate*>(((const PROPSHEETPAGEW*)lParam)->lParam);
        if(NULL == pDlg) {
            return FALSE;
        }
        SetWindowLongPtrW(hWnd, DWLP_USER, (LONG_PTR)pDlg);
        pDlg->SetHwnd(hWnd);
    }
    else {
        pDlg = reinterpret_cast<CPropPageDlgTemplate*>(GetWindowLongPtrW(hWnd, DWLP_USER));
    }

    if(NULL == pDlg) {
        return FALSE;
    }

    return (INT_PTR)pDlg->MessageHandler(hWnd, uMsg, wParam, lParam);
}